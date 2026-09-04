#!/usr/bin/env python3
"""
Phase B: augment a pristine OoTMM-identity XML (data/checks schema) with the render layer
(name/xyz + the scene_rendering/rendering_option grouping), joining the current pool CSV by
location. Preserves <xflag> document order (so the XflagID counter is unaffected) and the
<actor>/<match> identity nesting, then rewrites the converted XML in place.

    python augment.py --all                 # (re)augment every New/** file except the HAND set
    python augment.py <src.xml> [dst.xml]    # convert one file (default dst: <src>.aug.xml)

Idempotent: flatten() re-descends already-converted files, and layout-variant checks (one
location placed differently across MM/MM_JP or OoT/OoT_MQ) are collapsed by location then
re-expanded from every matching pool row, so a re-run reproduces the same output.

Also RE-INJECTS version-specific checks the source XML no longer carries: for any pool row whose
`system` column is in AUGMENT_SYNTH_SYSTEMS (checks OoTMM merged / removed upstream, e.g. the legacy
Kokiri crawl grass), the check is synthesized back from the pool (identity rebuilt from the id, render
from the pool row) so overwriting the XML with a fresh OoTMM copy no longer drops it. See REGEN.md.
"""
import sys, os
import xml.etree.ElementTree as ET

HERE = os.path.dirname(os.path.abspath(__file__))          # Resources/
POOL_OOT = os.path.join(HERE, "Objects", "pool_oot.csv")
POOL_MM = os.path.join(HERE, "Objects", "pool_mm.csv")
NEW_DIR = os.path.join(HERE, "Objects", "New")

OBJ_TAGS = {"chest", "collectible", "npc", "gs", "cow", "shop", "scrub",
            "xflag", "sf", "sr", "fish"}
# attribute emission order per identity tag (identity attrs preserved verbatim from source)
ORDER = ["type", "location", "system", "flag", "npc", "slice", "setup", "room", "actor",
         "item", "hint", "name", "xyz", "legacy_scene"]


TYPE_MAP = {"boulder-silver": "silverboulder", "boulder-red": "redboulder",
            "gossip-big": "gossip_big"}

# Pool `system` tokens whose checks are ABSENT from current OoTMM data (an older representation that
# was merged / removed upstream). augment re-injects them from the pool so a fresh OoTMM XML doesn't
# drop them. Extend this set (and gen_objects.SYSTEM_ENUM + the ObjSystem enum) for a new tier.
AUGMENT_SYNTH_SYSTEMS = {"legacy"}

# Direct-identity object tags: the pool `id` is the raw flag / npc id (not a composed xflag key).
# Everything else is an extended overlay authored as <xflag type="...">, whose composed identity we
# rebuild from the pool id when synthesizing.
DIRECT_TAGS = {"chest", "collectible", "npc", "gs", "cow", "shop", "scrub", "sf", "sr", "fish"}


def load_pool(path):
    d = {}
    with open(path, encoding="utf-8") as f:
        hdr = f.readline().rstrip("\n").split(";")
        idx = {n: i for i, n in enumerate(hdr)}
        for line in f:
            line = line.rstrip("\n")
            if not line:
                continue
            c = line.split(";")
            if len(c) < len(hdr):               # tolerate rows missing trailing optional columns
                c += [""] * (len(hdr) - len(c))
            row = {n: c[idx[n]] for n in idx}
            # Keep type=none rows: for a normal check they're render-dup placeholders that
            # pool_rows filters out by type, but for an *unreachable* check (its only pool row)
            # they carry the real map coords -> we reproduce the pool's hidden trace faithfully.
            d.setdefault(c[idx["location"]], []).append(row)
    return d


def esc(s):
    return (s.replace("&", "&amp;").replace('"', "&quot;")
             .replace("<", "&lt;").replace(">", "&gt;"))


class Aug:
    def __init__(self, pool, prefix="OOT_", game="oot"):
        self.pool = pool
        self.prefix = prefix
        self.game = game            # checks-game default for the `layout` attr
        self.warn = []
        self._warned = set()

    def flatten(self, node):
        """Yield object/actor elements in document order, descending render wrappers.
        Makes the augmenter idempotent: it re-parses already-converted files too."""
        for ch in node:
            if ch.tag in ("scene_rendering", "rendering_option"):
                yield from self.flatten(ch)
            elif ch.tag in OBJ_TAGS or ch.tag == "actor":
                yield ch

    def pool_rows(self, el):
        """All pool rows for this check. >1 means layout-variant placements (e.g. Deku Palace
        rupees that sit elsewhere in MM_JP): same location/XflagID, different game_layout+xyz.
        Matched by type, with the big-fairy rename fallback."""
        loc = self.prefix[:-1] + " " + el.attrib.get("location", "")
        otype = _otype(el)
        otype = TYPE_MAP.get(otype, otype)
        cands = self.pool.get(loc)
        # version drift: the big-fairy locations were renamed "Gossip Fairy" -> "Gossip Big Fairy",
        # but the pool still keys both variants under the small-fairy location, split by type.
        if cands is None and "Gossip Big Fairy" in loc:
            cands = self.pool.get(loc.replace("Gossip Big Fairy", "Gossip Fairy"))
        if not cands:
            return []
        matched = [r for r in cands if r["type"] == otype]
        return matched if matched else [cands[0]]

    def pool_row(self, el):
        rows = self.pool_rows(el)
        return rows[0] if rows else None

    def _variant(self, el, p):
        """(renderscene, loc_type, ro_key, name, xyz) from one pool row."""
        # rendering_option attrs only when they deviate from the defaults
        ro = {}
        otype = _otype(el)
        if p["rendertype"] != otype:
            ro["type"] = p["rendertype"]
        if p["icontype"] != "render_type":
            ro["icon"] = p["icontype"]
        if p["context"].lower() != "all":
            ro["context"] = p["context"].lower()
        if str(p["room"]) not in ("0", "0x0", ""):
            ro["room"] = p["room"]
        if p["game_layout"] != self.game:      # per-object layout (all/oot/oot_mq/mm/mm_jp)
            ro["layout"] = p["game_layout"]
        xyz = f'{int(p["x"])};{int(p["y"])};{int(p["z"])}'
        return (p["renderscene"], p["loc_type"], tuple(sorted(ro.items())),
                p["friendly_name"], xyz)

    def renders_of(self, el):
        """List of (rs, lt, ro_key, name, xyz, missing) - one entry per pool layout-variant,
        or a single placeholder when the check is absent from the pool."""
        rows = self.pool_rows(el)
        if rows:
            return [self._variant(el, p) + (False,) for p in rows]
        loc = self.prefix[:-1] + " " + el.attrib.get("location", "")
        if loc not in self._warned:
            self._warned.add(loc)
            self.warn.append(f"not in pool: {loc}")
        # Placeholder: gossip fairies get "Gossip [Big ]Fairy <descriptor>" per the user (xyz 0;0;0
        # for hand-completion); everything else keeps a stub name for tracing (unreachable/OOB).
        raw = el.attrib.get("location", "")
        otype = TYPE_MAP.get(_otype(el), _otype(el))
        if otype in ("gossip", "gossip_big"):
            name = _gossip_name(raw, otype == "gossip_big")
        else:
            name = raw.split()[-1] if raw else ""
        return [(None, "", (), name, "0;0;0", True)]

    def render_of(self, el):
        return self.renders_of(el)[0]

    def obj_xml(self, el, name, xyz, indent, scene_sym=None):
        a = dict(el.attrib)
        a["name"] = name
        a["xyz"] = xyz
        # Auto legacy_scene: when the new (truthful) identity scene differs from the scene the
        # pool (old ROMs) reports for this check, record the old one so FindObject can remap it.
        p = self.pool_row(el)
        if p and scene_sym is None:            # <scene id="NONE"> -> identity is the render scene
            scene_sym = p["renderscene"]
        if p and scene_sym and p["scene"] != scene_sym:
            leg = p["scene"]
            for pre in ("OOT_", "MM_"):
                if leg.startswith(pre):
                    leg = leg[len(pre):]
            a["legacy_scene"] = leg
        # Surface the xflag system (pool `system` column) on the check so the XML is self-documenting
        # and hand-editable; gen_objects reads this attr (falling back to the pool). Empty = "any".
        if p and p.get("system", ""):
            a["system"] = p["system"]
        else:
            a.pop("system", None)              # drop a stale attr if the pool no longer tags it
        parts = []
        for k in ORDER:
            if k in a:
                parts.append(f'{k}="{esc(a[k])}"')
        for k in a:                       # any leftover attrs (rare)
            if k not in ORDER:
                parts.append(f'{k}="{esc(a[k])}"')
        return f'{indent}<{el.tag} ' + " ".join(parts) + "/>"

    def _actor_checks(self, actor):
        """The actor's checks in document order, descending any nested <rendering_option> a
        prior augment left inside the actor (keeps re-augmentation idempotent)."""
        for c in actor:
            if c.tag == "rendering_option":
                yield from self._actor_checks(c)
            elif c.tag in OBJ_TAGS:
                yield c

    def actor_render(self, actor):
        """(rs, lt, ro_key) for placing the actor. rs/lt come from its first check; the ro_key
        is shared only when every check renders the same way. A heterogeneous actor - e.g. a
        gossip stone whose small 'gossip' fairy renders as 'fairy' and its 'gossip-big' as
        'fairy_spot' - returns an empty ro so scene_xml does not wrap it in one shared
        <rendering_option>; emit_actor then gives each check its own nested one instead."""
        rs = lt = None
        ros = []
        for c in self._actor_checks(actor):
            crs, clt, cro, _, _, _ = self.render_of(c)
            if rs is None:
                rs, lt = crs, clt
            ros.append(cro)
        if not ros:
            return None, "", ()
        common = ros[0] if all(r == ros[0] for r in ros) else ()
        return rs, lt, common

    def emit_actor(self, actor, indent, scene_sym=None):
        checks = list(self._actor_checks(actor))
        ros = [self.render_of(c)[2] for c in checks]
        homogeneous = (not ros) or all(r == ros[0] for r in ros)
        lines = [f"{indent}<actor" + _attrs(actor) + ">"]
        for c in actor:                       # matches carry the identity; keep them first
            if c.tag == "match":
                lines.append(f"{indent}  <match" + _attrs(c) + "/>")
        if homogeneous:
            # the actor's single render is carried by the enclosing <rendering_option> (scene_xml)
            for c in checks:
                _, _, _, name, xyz, _ = self.render_of(c)
                lines.append(self.obj_xml(c, name, xyz, indent + "  ", scene_sym))
        else:
            # split: each check keeps its own render type via a nested <rendering_option>
            k = 0
            while k < len(checks):
                ro = ros[k]
                m = k
                while m < len(checks) and ros[m] == ro:
                    m += 1
                pad = indent + "  "
                if ro:
                    roattr = " ".join(f'{kk}="{esc(vv)}"' for kk, vv in ro)
                    lines.append(f"{indent}  <rendering_option {roattr}>")
                    pad = indent + "    "
                for c in checks[k:m]:
                    _, _, _, name, xyz, _ = self.render_of(c)
                    lines.append(self.obj_xml(c, name, xyz, pad, scene_sym))
                if ro:
                    lines.append(f"{indent}  </rendering_option>")
                k = m
        lines.append(f"{indent}</actor>")
        return lines

    def _synth_element(self, p, short):
        """Build a synthetic identity element for a pool check re-injected from the pool (absent from
        the source XML, see AUGMENT_SYNTH_SYSTEMS). gen_objects resolves the ObjectID by Location, so
        the identity attrs are cosmetic; for xflag types we still rebuild the composed key from the
        pool id so the XML stays self-consistent. name/xyz are supplied later from the pool render."""
        ptype = p["type"]
        if ptype in DIRECT_TAGS:
            el = ET.Element(ptype)
            el.set("location", short)
            el.set("npc" if ptype == "npc" else "flag", p["id"])
        else:
            el = ET.Element("xflag")
            el.set("type", ptype)
            el.set("location", short)
            try:
                oid = int(p["id"], 0)
            except ValueError:
                oid = 0
            high = (oid >> 8) & 0xFF
            slc = (oid >> 16) & 0xFF
            el.set("setup", str((high >> 6) & 0x3))
            el.set("room", "0x%x" % (high & 0x3F))
            el.set("actor", "0x%x" % (oid & 0xFF))
            if slc:
                el.set("slice", "0x%x" % slc)
        el.set("item", "RANDOM")
        return el

    def scene_xml(self, scene):
        sid = scene.attrib.get("id")
        scene_sym = None if sid in (None, "NONE") else \
            self.prefix + (sid[:-3] if sid.endswith("_MQ") else sid)
        # 1) flatten scene's top-level items into (kind, payload, rs, lt, ro_key)
        items = []
        seen = set()   # dedupe by location so a re-run over an already-split layout-variant
        for ch in self.flatten(scene):        # (2 elements, same location) collapses back to 1
            if ch.tag == "actor":
                rs, lt, ro = self.actor_render(ch)
                items.append(("actor", ch, rs, lt, ro))
                for c in self._actor_checks(ch):     # record the actor's checks so synthesis skips them
                    seen.add(c.attrib.get("location", ""))
            elif ch.tag in OBJ_TAGS:
                loc = ch.attrib.get("location", "")
                if loc in seen:
                    continue
                seen.add(loc)
                # renders_of expands one check into every pool layout-variant placement.
                for rs, lt, ro, name, xyz, _ in self.renders_of(ch):
                    items.append(("obj", (ch, name, xyz), rs, lt, ro))
        # Re-inject version-specific checks that a fresh OoTMM XML no longer carries (e.g. a legacy
        # form merged upstream) but the pool still marks (system in AUGMENT_SYNTH_SYSTEMS). Their
        # identity/render come from the pool; gen_objects resolves the ObjectID by Location. Appended
        # after the flattened items, grouped like the rest by (rs, lt) then rendering_option.
        for full_loc, prows in self.pool.items():
            p = prows[0]
            if p.get("system", "") not in AUGMENT_SYNTH_SYSTEMS or p["scene"] != scene_sym:
                continue
            short = full_loc[len(self.prefix[:-1]) + 1:]     # strip "OOT "/"MM " -> XML-local location
            if short in seen:
                continue
            seen.add(short)
            el = self._synth_element(p, short)
            for rs, lt, ro, name, xyz, _ in self.renders_of(el):
                items.append(("obj", (el, name, xyz), rs, lt, ro))
        # objects missing from the pool have rs=None: fall back to the scene's own render scene
        # (keeps the XML valid; their xyz stays 0;0;0 for the user to fill in).
        default_rs = next((it[2] for it in items if it[2]), None) or scene_sym
        default_lt = next((it[3] for it in items if it[2]), "") or "overworld"
        items = [(k, p, rs if rs else default_rs, lt if rs else default_lt, ro)
                 for (k, p, rs, lt, ro) in items]
        # 2) emit, grouping consecutive by (rs,lt) -> scene_rendering, then ro -> rendering_option
        out = []
        sattr = _attrs(scene)
        out.append(f"  <scene{sattr}>")
        i = 0
        while i < len(items):
            rs, lt = items[i][2], items[i][3]
            j = i
            while j < len(items) and items[j][2] == rs and items[j][3] == lt:
                j += 1
            block = items[i:j]
            rsattr = f' id="{rs}"' if rs else ""
            if lt:
                rsattr += f' loc_type="{lt}"'
            out.append(f"    <scene_rendering{rsattr}>")
            k = 0
            while k < len(block):
                ro = block[k][4]
                m = k
                while m < len(block) and block[m][4] == ro:
                    m += 1
                run = block[k:m]
                pad = "      "
                if ro:
                    roattr = " ".join(f'{kk}="{esc(vv)}"' for kk, vv in ro)
                    out.append(f"      <rendering_option {roattr}>")
                    pad = "        "
                for kind, payload, *_ in run:
                    if kind == "obj":
                        el, name, xyz = payload
                        out.append(self.obj_xml(el, name, xyz, pad, scene_sym))
                    else:
                        out.extend(self.emit_actor(payload, pad, scene_sym))
                if ro:
                    out.append("      </rendering_option>")
                k = m
            out.append("    </scene_rendering>")
            i = j
        out.append("  </scene>")
        return out


def _gossip_name(loc, big):
    """Display name for a gossip fairy absent from the pool. The descriptor can sit on either
    side of "Fairy" in the location (e.g. "Gossip Outside Fairy" vs "Gossip Fairy Lower"), so
    keep every location word after "Gossip" except the structural "Big"/"Fairy" tokens."""
    tail = loc.split("Gossip", 1)[1] if "Gossip" in loc else loc
    desc = [t for t in tail.split() if t not in ("Big", "Fairy")]
    base = "Gossip Big Fairy" if big else "Gossip Fairy"
    return base + ((" " + " ".join(desc)) if desc else "")


def _otype(el):
    return el.attrib.get("type", el.tag) if el.tag == "xflag" else el.tag


def _attrs(el):
    order = ["name", "id", "room", "setup", "actor", "slice"]
    a = dict(el.attrib)
    parts = []
    for k in order:
        if k in a:
            parts.append(f'{k}="{esc(a[k])}"')
    for k in a:
        if k not in order:
            parts.append(f'{k}="{esc(a[k])}"')
    return (" " + " ".join(parts)) if parts else ""


def augment_file(src, pools, dst=None, quiet=False):
    root = ET.parse(src).getroot()
    game = root.attrib["game"]
    prefix = "OOT_" if game.startswith("oot") else "MM_"
    aug = Aug(pools[prefix], prefix, game)
    out = ['<?xml version="1.0" encoding="UTF-8"?>', f'<checks game="{game}">']
    for scene in root:
        if scene.tag == "scene":
            out.extend(aug.scene_xml(scene))
    out.append("</checks>")
    with open(dst or src, "w", encoding="utf-8") as f:
        f.write("\n".join(out) + "\n")
    if not quiet:
        print(f"wrote {dst or src}")
        for w in aug.warn:
            print("  WARN:", w)
    return aug.warn


def main():
    import glob
    pools = {"OOT_": load_pool(POOL_OOT), "MM_": load_pool(POOL_MM)}

    # Files whose render layer is NOT recoverable from the pool -> never re-augment them (they must
    # be maintained by hand). Currently none: the former examples (oot/special, oot/overworld/kakariko,
    # oot/dungeons_mq/bottom_of_the_well_mq) are now fully backed by the pool, so --all augments them
    # like every other file. Add a "game/path.xml" here only if a file gains hand-only render data.
    HAND = set()
    if sys.argv[1] == "--all":
        total = conv = 0
        report = {}
        errors = []
        for p in sorted(glob.glob(os.path.join(NEW_DIR, "**", "*.xml"), recursive=True)):
            rel = os.path.relpath(p, NEW_DIR).replace("\\", "/")
            if rel in HAND:
                continue
            total += 1
            try:
                warns = augment_file(p, pools, quiet=True)  # flatten() re-parses converted files too
            except ET.ParseError as e:                       # malformed source -> report, keep going
                errors.append(f"{rel}: {e}")
                continue
            conv += 1
            if warns:
                report[os.path.relpath(p, NEW_DIR)] = warns
        print(f"converted {conv}/{total} files in place")
        if errors:
            print(f"\n==== {len(errors)} file(s) FAILED to parse (fix the XML, left untouched) ====")
            for e in errors:
                print("  ", e)
        print(f"\n==== checks not in pool (complete xyz/render by hand): "
              f"{sum(len(v) for v in report.values())} across {len(report)} files ====")
        for f in sorted(report):
            print(f"\n{f}:")
            for w in report[f]:
                print("  ", w.replace("not in pool: ", ""))
        return

    src = sys.argv[1]
    dst = sys.argv[2] if len(sys.argv) > 2 else src.replace(".xml", ".aug.xml")
    augment_file(src, pools, dst)


if __name__ == "__main__":
    main()
