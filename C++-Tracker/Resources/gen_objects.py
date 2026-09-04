#!/usr/bin/env python3
"""
Generate OoT/MMObjectScene.cpp from the object XML files in Resources/Objects/New/**.

The XML is OoTMM's native data/checks schema (<scene>, <actor>, <xflag slice/setup/actor>,
<chest/gs/... flag>, verbatim `location`) plus a render layer (<scene_rendering id loc_type>,
<rendering_option type/icon/context/room/layout>, and name/xyz/item on each check). This script
resolves every check to the same ObjectInfo row Pool Transform.py emits, computing:

  * ObjectID (legacy matching):
      chest/collectible/cow/shop/scrub  -> `flag`
      npc / song                        -> npc symbol (SONG_* get the game prefix)
      xflag                             -> slice<<16 | (room|(setup<<6))<<8 | actor
      gs                                -> reindexed, not derivable -> gs_ids.csv (from the pool)
  * XflagID (new xflag ROMs > v32.3): looked up by Location from gen_xflags.build_rows(<ootmm_root>)
  * an optional legacy-scene remap (legacy_scene="...") for the few checks whose truthful game
    scene differs from the one old ROMs report (cow, granny potions, hatch eggs, ...).

Usage:
    python gen_objects.py <ootmm_root> --diff      # compare against the current pool CSVs
    python gen_objects.py <ootmm_root> --emit       # (re)write OoT/MMObjectScene.cpp
    python gen_objects.py <ootmm_root> --dump-gs     # (re)extract Objects/gs_ids.csv from the pool

<ootmm_root> must be the SAME OoTMM checkout used to build the ROM you play (its data/checks and
data/defs/scenes.yml drive the XflagID counter).
"""
import sys, os, re, glob, argparse
import xml.etree.ElementTree as ET

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from gen_xflags import build_rows  # noqa: E402  (XflagID replay of packages/data/build/checks.ts)

NEW_DIR = os.path.join(HERE, "Objects", "New")
POOL_OOT = os.path.join(HERE, "Objects", "pool_oot.csv")
POOL_MM = os.path.join(HERE, "Objects", "pool_mm.csv")
GS_IDS = os.path.join(HERE, "Objects", "gs_ids.csv")
OUT_OOT = os.path.join(HERE, "..", "Sources", "Combo", "OoTObjectScene.cpp")
OUT_MM = os.path.join(HERE, "..", "Sources", "Combo", "MMObjectScene.cpp")
HDR_OOT = os.path.join(HERE, "..", "Headers", "Combo", "OoTObjectScene.h")
HDR_MM = os.path.join(HERE, "..", "Headers", "Combo", "MMObjectScene.h")
OBJ_CPP = os.path.join(HERE, "..", "Sources", "Combo", "Objects.cpp")

OBJ_TAGS = {"chest", "collectible", "npc", "gs", "cow", "shop", "scrub",
            "xflag", "sf", "sr", "fish"}

# xflag `type` attr -> ObjectType enum name (only where they differ from the attr)
XFLAG_TYPE = {
    "boulder-silver": "silverboulder", "boulder-red": "redboulder",
    "gossip-big": "gossip_big", "fairy-spot": "fairy_spot",
}

# xflag `system` token (pool CSV `system` column) -> ObjSystem C++/Rust enum variant. Tags the few
# checks whose representation changed across OoTMM versions: "legacy" objects exist only on the older
# ROMs, "new" only on the current ones; an empty / unknown token means "any" (both). The token is the
# durable authority for the `system` field, stored in the pool by Location (see load_pool_system);
# augment.py re-injects the legacy-tagged checks that vanish from a fresh OoTMM XML. Runtime gate:
# UsesLegacyXflags() (ObjectInfo::HasCorrectLayout / Rust object_active).
#   To add a future version tier, add its token here + a matching ObjSystem variant + a runtime case.
SYSTEM_ENUM = {"legacy": "Legacy", "new": "New"}

# Shared object-id symbols the pool stores bare but whose C++ define is game-prefixed (OoT and MM
# hold different values, e.g. OOT_SONG_STORMS 0x06 vs MM_SONG_STORMS 0x0d) -> prefix per game.
COMMON_ID = {"SONG_STORMS", "SONG_OF_STORMS"}

GAME_META = {  # <checks game> -> (scene prefix, GameLayout, cpp bucket)
    "oot": ("OOT_", "oot", "OoT"), "oot_mq": ("OOT_", "oot_mq", "OoT"),
    "mm": ("MM_", "mm", "MM"), "mm_jp": ("MM_", "mm_jp", "MM"),
}


def i0(x):
    return int(str(x), 0)


def fix_malformed(text):
    # tolerate the hand-authoring slip <scene_rendering="X"> -> <scene_rendering id="X">
    return re.sub(r"<scene_rendering=", r"<scene_rendering id=", text)


def scene_symbol(scene_id, prefix):
    if scene_id in (None, "NONE"):
        return None
    sid = scene_id[:-3] if scene_id.endswith("_MQ") else scene_id
    for p in ("OOT_", "MM_", "SHARED_"):
        if sid.startswith(p):
            return sid
    return prefix + sid


def compose_xflag_id(idc):
    s = i0(idc.get("slice", "0")); r = i0(idc.get("room", "0"))
    setup = i0(idc.get("setup", "0")); a = i0(idc.get("actor", "0"))
    return (s << 16) | (((r | (setup << 6)) & 0xFF) << 8) | (a & 0xFF)


def parse_xyz(xyz):
    if not xyz:
        return (0, 0, 0)
    parts = [p for p in xyz.split(";") if p != ""]
    vals = [int(p) for p in parts] + [0, 0, 0]
    return (vals[0], vals[1], vals[2])


class Gen:
    def __init__(self, gs_map, xflags, pool_ids, pool_system=None):
        self.gs_map = gs_map
        self.xflags = xflags
        self.pool_ids = pool_ids
        self.pool_system = pool_system or {}
        self.rows = []
        self.remap = []   # (bucket, legacy_scene, objid, type, true_scene, location)
        self.warn = []

    def walk(self, el, game, ident, render):
        for child in el:
            tag, a = child.tag, child.attrib
            if tag == "scene_rendering":
                r = dict(render)
                if "id" in a:
                    r["renderscene"] = a["id"]
                if "loc_type" in a:
                    r["loc_type"] = a["loc_type"]
                self.walk(child, game, ident, r)
            elif tag == "rendering_option":
                r = dict(render)
                for k_xml, k_row in (("type", "rtype"), ("icon", "icon"),
                                     ("context", "context"), ("room", "rroom"),
                                     ("layout", "layout")):
                    if k_xml in a:
                        r[k_row] = a[k_xml]
                self.walk(child, game, ident, r)
            elif tag == "actor":
                id2 = dict(ident)
                for k in ("setup", "room", "actor", "slice"):
                    if k in a:
                        id2[k] = a[k]
                # New OoTMM format: an <actor> without setup/actor carries them on its <match>
                # children; the FIRST match is the primary (== legacy identity we reproduce).
                fm = next((c for c in child if c.tag == "match"), None)
                if fm is not None:
                    for k in ("setup", "room", "actor", "slice"):
                        if k not in a and k in fm.attrib:
                            id2[k] = fm.attrib[k]
                self.walk(child, game, id2, render)
            elif tag in OBJ_TAGS:
                id2 = dict(ident)
                for k in ("scene", "room", "setup", "actor", "slice"):
                    if k in a:
                        id2[k] = a[k]
                self.emit(tag, child, game, id2, render)
            elif tag == "match":
                pass
            else:
                self.warn.append(f"unexpected <{tag}>")

    def emit(self, tag, el, game, idc, render):
        a = el.attrib
        prefix, layout, bucket = GAME_META[game]
        loc = f"{prefix[:-1]} {a.get('location', '')}"

        if tag == "xflag":
            t = a.get("type")
            otype = XFLAG_TYPE.get(t, t)
            if not otype:
                self.warn.append(f"xflag missing type @ {loc}")
        else:
            otype = tag

        if tag == "npc":
            objid = a.get("npc", "?")
        elif tag == "gs":
            objid = self.gs_map.get(loc)
            if objid is None:
                objid = "0x0"
                self.warn.append(f"gs not in gs_ids.csv @ {loc}")
        elif tag in ("chest", "collectible", "cow", "shop", "scrub", "sf", "sr", "fish"):
            objid = a.get("flag", "?")
        elif tag == "xflag":
            objid = "0x%05X" % compose_xflag_id(idc)
        else:
            objid = "?"

        # The legacy ObjectID is what <= v32.3 ROMs emit, so the pool (a snapshot of that era) is
        # authoritative: prefer its value by Location, keeping the composed id only as the fallback
        # for checks the pool never had (new gossips, ...). New ROMs (> v32.3) use the XflagID and
        # ignore this entirely.
        objid = self.pool_ids.get(loc, objid)
        if objid in COMMON_ID:                  # bare shared symbol -> its game-prefixed C++ define
            objid = prefix + objid

        x, y, z = parse_xyz(a.get("xyz", ""))
        xid = self.xflags.get(loc)

        # A render override of type="none" marks an unreachable check kept only as a data trace:
        # hide it exactly like the pool did (both Type and RenderType = none), coords preserved.
        if render.get("rtype") == "none":
            otype = "none"

        scene = scene_symbol(idc.get("scene"), prefix)
        renderscene = render.get("renderscene")
        if scene is None:                       # <scene id="NONE"> -> identity is the render scene
            scene = renderscene
        legacy = scene_symbol(a.get("legacy_scene"), prefix) if a.get("legacy_scene") else None

        row = {
            "id": objid, "scene": scene, "friendly_name": a.get("name", ""),
            "location": loc, "type": otype, "x": x, "y": y, "z": z,
            "renderscene": renderscene, "rendertype": render.get("rtype", otype),
            "icontype": render.get("icon", "render_type"),
            "context": render.get("context", "all").capitalize(),
            "room": render.get("rroom", "0"), "game_layout": render.get("layout", layout),
            "loc_type": render.get("loc_type", ""),
            "xflag_id": ("0x%04X" % xid) if xid is not None else "0xFFFF",
            # xflag system: prefer the self-documenting XML attr (written by augment / hand-editable),
            # fall back to the pool `system` column (authority) for a not-yet-augmented raw XML.
            "system": a.get("system") or self.pool_system.get(loc, ""),
            "bucket": bucket, "tag": tag, "legacy_scene": legacy,
        }
        self.rows.append(row)
        if legacy and legacy != scene:
            self.remap.append((bucket, legacy, objid, otype, scene, loc))

    def process(self, path):
        text = fix_malformed(open(path, encoding="utf-8").read())
        root = ET.fromstring(text)
        game = root.attrib["game"]
        _, layout, _ = GAME_META[game]
        for scene in root:
            if scene.tag != "scene":
                self.warn.append(f"{os.path.basename(path)}: <{scene.tag}> under <checks>")
                continue
            ident = {}
            if "id" in scene.attrib:
                ident["scene"] = scene.attrib["id"]
            if "room" in scene.attrib:
                ident["room"] = scene.attrib["room"]
            render = {"layout": layout}
            if "loc_type" in scene.attrib:
                render["loc_type"] = scene.attrib["loc_type"]
            self.walk(scene, game, ident, render)


# ---------------------------------------------------------------- pool / gs
def load_pool(path):
    rows, idx = {}, None
    with open(path, encoding="utf-8") as f:
        header = f.readline().rstrip("\n").split(";")
        idx = {n: i for i, n in enumerate(header)}
        for line in f:
            line = line.rstrip("\n")
            if line:
                c = line.split(";")
                if len(c) < len(header):        # tolerate rows missing trailing optional columns
                    c += [""] * (len(header) - len(c))
                rows.setdefault(c[idx["location"]], []).append(c)
    return rows, idx


def build_gs_ids():
    """Extract the reindexed gs ObjectIDs (location -> id) from both pool CSVs."""
    m = {}
    for p in (POOL_OOT, POOL_MM):
        if not os.path.exists(p):
            continue
        rows, idx = load_pool(p)
        for loc, plist in rows.items():
            for c in plist:
                if c[idx["type"]] == "gs":
                    m[loc] = c[idx["id"]]
                    break
    return m


def load_gs_ids():
    if not os.path.exists(GS_IDS):
        m = build_gs_ids()
        with open(GS_IDS, "w", encoding="utf-8") as f:
            f.write("location;id\n")
            for loc in sorted(m):
                f.write(f"{loc};{m[loc]}\n")
        return m
    m = {}
    with open(GS_IDS, encoding="utf-8") as f:
        f.readline()
        for line in f:
            line = line.rstrip("\n")
            if line:
                loc, i = line.split(";")
                m[loc] = i
    return m


def load_pool_ids():
    """location -> legacy ObjectID from the pool (authoritative for <= v32.3 ROMs). Prefer a
    non-none row (the real check) over its type=none render-dup placeholders."""
    m = {}
    for p in (POOL_OOT, POOL_MM):
        if not os.path.exists(p):
            continue
        rows, idx = load_pool(p)
        for loc, plist in rows.items():
            real = next((c for c in plist if c[idx["type"]] != "none"), plist[0])
            m[loc] = real[idx["id"]]
    return m


def load_pool_system():
    """location -> xflag `system` token (pool `system` column). The pool is the durable authority
    for the version tag (immune to augment.py, which re-injects the legacy-tagged checks). Absent /
    empty column -> "" (any)."""
    m = {}
    for p in (POOL_OOT, POOL_MM):
        if not os.path.exists(p):
            continue
        rows, idx = load_pool(p)
        if "system" not in idx:
            continue
        for loc, plist in rows.items():
            for c in plist:
                v = c[idx["system"]].strip()
                if v:
                    m[loc] = v
                    break
    return m


def load_xflags(ootmm_root):
    rows = build_rows(ootmm_root)
    return {r["location"]: r["id"] for r in rows}


def load_xflags_from_pool():
    """Fallback when no OoTMM checkout is on hand: reuse the XflagID already stamped on the pool
    CSVs (column `xflag_id`, previously computed by gen_xflags against the matching checkout)."""
    m = {}
    for p in (POOL_OOT, POOL_MM):
        if not os.path.exists(p):
            continue
        rows, idx = load_pool(p)
        if "xflag_id" not in idx:
            continue
        for loc, plist in rows.items():
            for c in plist:
                v = c[idx["xflag_id"]]
                if v and v != "0xFFFF":
                    m[loc] = int(v, 0)
                    break
    return m


# ---------------------------------------------------------------- cpp emit
def emit_struct(r):
    return ("\t{ " + str(r["id"]) + ", " + str(r["scene"]) + ", \"" + r["friendly_name"]
            + "\", \"" + r["location"] + "\", ObjectType::" + r["type"] + ", {"
            + str(r["x"]) + ", " + str(r["y"]) + ", " + str(r["z"]) + "}, "
            + str(r["renderscene"]) + ", ObjectType::" + r["rendertype"] + ", EGameIcon::"
            + r["icontype"] + ", ObjectContext::" + r["context"] + ", " + str(r["room"])
            + ", GameLayout::" + r["game_layout"] + ", LocType::" + r["loc_type"]
            + ", NULL, " + r["xflag_id"] + ", ObjSystem::" + SYSTEM_ENUM.get(r.get("system", ""), "Any")
            + " }")


def wrap_cpp(bucket, content):
    inc = "OoT" if bucket == "OoT" else "MM"
    return ("#include \"Combo/" + inc + "ObjectScene.h\"\n#include \"Combo/Objects.h\"\n"
            "#include \"Combo/Scenes.h\"\n\n#pragma region " + bucket.upper()
            + "\n" + content + "\n#pragma endregion")


def emit_remap(remap, bucket):
    """C++ for the per-game legacy-scene remap table (consulted by FindObjectByLegacyScene)."""
    name = bucket + "LegacySceneRemap"
    entries = [e for e in remap if e[0] == bucket]     # (bucket, legacy, objid, otype, scene, loc)
    if not entries:
        return f"\nconst size_t {name}Count = 0;\nLegacySceneRemap* {name} = nullptr;\n"
    lines = [f"\t{{ {legacy}, {objid}, ObjectType::{otype}, {scene} }}, // {loc}"
             for (_, legacy, objid, otype, scene, loc) in entries]
    return (f"\nconst size_t {name}Count = {len(entries)};\n"
            f"static LegacySceneRemap {name}_Data [{name}Count] =\n{{\n"
            + "\n".join(lines) + f"\n}};\nLegacySceneRemap* {name} = {name}_Data;\n")


def header_scene_set(bucket):
    """The scenes the matching ObjectScene.h declares extern (defined in the generated cpp)."""
    path = HDR_OOT if bucket == "OoT" else HDR_MM
    return set(re.findall(r"extern ObjectInfo\*\s*([A-Za-z0-9_]+)SceneObjects;",
                          open(path, encoding="utf-8").read()))


def objects_cpp_registration():
    """(CreateEmptyScene set, CreateSceneObjects set) from Objects.cpp: the empty scenes defined
    locally there, and every scene wired into the OoT/MMSceneObjects aggregate arrays."""
    txt = open(OBJ_CPP, encoding="utf-8").read()
    # Anchor at line start (after indent) so commented-out "//CreateEmptyScene(...)" lines,
    # which the user disables when a scene gains render-dups, are correctly ignored.
    return (set(re.findall(r"^[ \t]*CreateEmptyScene\(([A-Za-z0-9_]+)\)", txt, re.M)),
            set(re.findall(r"^[ \t]*CreateSceneObjects\(([A-Za-z0-9_]+)\)", txt, re.M)))


def emit_cpp(rows, remap, bucket, out_path):
    """Replicate Pool Transform.py::parse_file2 scene bucketing + render duplication.
    Returns a registration-mismatch report (scenes whose empty/non-empty status changed)."""
    fin = {}
    for r in rows:
        if r["bucket"] != bucket:
            continue
        s = emit_struct(r)
        fin.setdefault(r["scene"], []).append(s)
        if r["renderscene"] != r["scene"] and r["type"] != "none":
            fin.setdefault(r["renderscene"], [])
            if s not in fin[r["renderscene"]]:
                fin[r["renderscene"]].append(s)

    header = header_scene_set(bucket)
    empty_local, aggregate = objects_cpp_registration()

    content = ""
    for scene, structs in fin.items():
        content += ("\nconst size_t " + scene + "NumOfObjs = " + str(len(structs))
                    + ";\nstatic ObjectInfo " + scene + "SceneObjects_Data [" + scene
                    + "NumOfObjs] =\n{\n" + ",\n".join(structs)
                    + "\n};\nObjectInfo * " + scene + "SceneObjects = " + scene + "SceneObjects_Data;\n")
    # Extern-declared scenes with no objects (parent/render scenes that only held type=none
    # render-dups, or checks that moved away) still need a definition so the aggregate links.
    # Skip the ones Objects.cpp already defines itself via CreateEmptyScene.
    for scene in sorted(header - set(fin) - empty_local):
        content += ("\nconst size_t " + scene + "NumOfObjs = 0;\n"
                    + "ObjectInfo * " + scene + "SceneObjects = nullptr;\n")
    content += emit_remap(remap, bucket)
    with open(out_path, "w", encoding="utf-8") as f:
        f.write(wrap_cpp(bucket, content))
    n = sum(1 for r in rows if r["bucket"] == bucket)
    nr = sum(1 for e in remap if e[0] == bucket)
    print(f"  wrote {os.path.basename(out_path)} ({n} objects, {len(fin)} scene arrays, {nr} legacy remaps)")

    # Registration drift the user must reconcile in Objects.cpp / ObjectScene.h (the aggregate
    # array is indexed by scene id, so these edits are left manual on purpose):
    report = {
        # now has objects but Objects.cpp still defines it empty -> remove its CreateEmptyScene
        "was_empty_now_has_objects": sorted(set(fin) & empty_local),
        # defined here but not declared extern in ObjectScene.h -> add the extern
        "missing_extern": sorted(set(fin) - header),
        # reachable objects need the scene in the aggregate array
        "missing_from_aggregate": sorted(set(fin) - aggregate),
    }
    return report


# ---------------------------------------------------------------- diff
CMP_FIELDS = ["id", "scene", "type", "x", "y", "z", "renderscene", "rendertype",
              "icontype", "context", "room", "game_layout", "loc_type", "xflag_id"]


def norm(field, v):
    v = "" if v is None else str(v)
    if field in ("id", "xflag_id", "room"):
        try:
            return str(int(v, 0))
        except ValueError:
            return v
    return v


def run_diff(g):
    pools = {}
    for p in (POOL_OOT, POOL_MM):
        if os.path.exists(p):
            rows, idx = load_pool(p)
            pools[p] = (rows, idx)
    identical = differing = new = 0
    field_deltas = {f: 0 for f in CMP_FIELDS}
    details = []
    for r in g.rows:
        prim = pidx = None
        for rows, idx in pools.values():
            plist = rows.get(r["location"])
            if plist:
                prim = next((c for c in plist if c[idx["type"]] != "none"), plist[0])
                pidx = idx
                break
        if prim is None:
            new += 1
            details.append(f"NEW  {r['location']}  (type={r['type']} id={r['id']} xflag={r['xflag_id']})")
            continue
        diffs = []
        for f in CMP_FIELDS:
            if norm(f, r[f]) != norm(f, prim[pidx[f]]):
                if (f == "scene" and r.get("legacy_scene")
                        and norm("scene", r["legacy_scene"]) == norm(f, prim[pidx[f]])):
                    continue
                diffs.append(f"{f}: xml={r[f]!r} pool={prim[pidx[f]]!r}")
                field_deltas[f] += 1
        if diffs:
            differing += 1
            details.append(f"DIFF {r['location']}\n      " + "\n      ".join(diffs))
        else:
            identical += 1
    print(f"\n==== {identical} identical, {differing} differing, {new} new ====")
    print("field deltas:", {k: v for k, v in field_deltas.items() if v})
    for d in details:
        print(d)


def sync_pool(g, write=False):
    """Back-fill the pool CSVs with checks that live in the New/** XML render layer but are absent
    from the pool (typically freshly-added gossips). The pool is the DURABLE store of the render
    layer (xyz / name / icon / layout), so `augment.py --all` or overwriting the XML from a fresh
    OoTMM checkout would otherwise drop those hand-entered coordinates. Dry-run by default (lists
    what it would add); pass write=True to append the rows (columns in the pool's own order;
    `requierements` / `tooltip` left empty; `xflag_id` = whatever the build carries, so run WITH an
    <ootmm_root> for real ids, or fill them later with `gen_xflags.py --stamp-csv`)."""
    targets = {"OoT": POOL_OOT, "MM": POOL_MM}
    total = 0
    for bucket, path in targets.items():
        if not os.path.exists(path):
            print(f"  {bucket}: pool absent ({os.path.relpath(path, HERE)}) - skipped")
            continue
        pool, idx = load_pool(path)
        header = [n for n, _ in sorted(idx.items(), key=lambda kv: kv[1])]
        existing = set(pool.keys())               # Locations already stored (any layout row)
        missing = [r for r in g.rows if r["bucket"] == bucket and r["location"] not in existing]

        lines, skipped = [], []
        for r in missing:
            cells = ["" if r.get(c) is None else str(r.get(c, "")) for c in header]
            if any(";" in c or "\n" in c for c in cells):
                skipped.append(r["location"])       # a ';' would corrupt the row - never write it
                continue
            lines.append(";".join(cells))

        print(f"  {bucket}: {len(lines)} check(s) missing from {os.path.basename(path)}"
              + (" (dry-run)" if not write else ""))
        for r in missing:
            if r["location"] not in skipped:
                print(f"     + {r['location']}  [{r['type']}] id={r['id']} "
                      f"xyz={r['x']};{r['y']};{r['z']} xflag={r['xflag_id']}")
        if skipped:
            print(f"     ! {len(skipped)} skipped (a field contains ';' or a newline): {skipped[:5]}")
        total += len(lines)

        if write and lines:
            with open(path, "rb") as f:
                raw = f.read()
            nl = b"\r\n" if b"\r\n" in raw else b"\n"
            if raw and not raw.endswith(nl):
                raw += nl                            # ensure we append on a fresh line
            raw += nl.join(l.encode("utf-8") for l in lines) + nl
            with open(path, "wb") as f:
                f.write(raw)
            print(f"     -> appended {len(lines)} row(s) to {os.path.basename(path)}")

    if write:
        print(f"\n  done: {total} row(s) added. Next: `gen_xflags.py <ootmm_root> --stamp-csv "
              "Objects/pool_oot.csv Objects/pool_mm.csv` to fill the XflagID, then re-run the "
              "generators.")
    else:
        print(f"\n  (dry-run) {total} row(s) would be added. Re-run with --sync-pool --write to apply.")


# ---------------------------------------------------------------- reusable build
def build_objects(ootmm_root=None, quiet=True):
    """Parse every converted New/** file into the object rows + legacy-scene remap. Shared entry
    point so other generators (the Rust tracker's tools/gen_data.py) consume the same New/ XML.
    Returns the populated Gen (`.rows`, `.remap`, `.warn`). XflagID comes from the OoTMM checkout
    when given, else from the pool CSVs; the legacy ObjectID is always pool-preferred."""
    gs_map = load_gs_ids()
    xflags = load_xflags(ootmm_root) if ootmm_root else load_xflags_from_pool()
    g = Gen(gs_map, xflags, load_pool_ids(), load_pool_system())
    skipped_files = []
    for path in sorted(glob.glob(os.path.join(NEW_DIR, "**", "*.xml"), recursive=True)):
        # Only converted files carry the render layer; identity-only originals are skipped.
        if "<scene_rendering" not in open(path, encoding="utf-8").read():
            skipped_files.append(os.path.relpath(path, NEW_DIR).replace("\\", "/"))
            continue
        g.process(path)
    g.skipped_files = skipped_files
    g.skipped = len(skipped_files)
    if not quiet:
        src = f"checkout {ootmm_root}" if ootmm_root else "pool CSVs"
        print(f"XflagIDs from {src}; parsed {len(g.rows)} objects ({g.skipped} not-yet-converted "
              f"skipped); {len(g.remap)} legacy-scene remaps; {len(g.warn)} warnings")
        for sf in skipped_files:
            print(f"  un-converted (no <scene_rendering>): {sf}")
        for w in g.warn:
            print("  WARN:", w)
    return g


# ---------------------------------------------------------------- main
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("ootmm_root", nargs="?", help="OoTMM checkout matching the ROM (for XflagID)")
    ap.add_argument("--emit", action="store_true", help="write OoT/MMObjectScene.cpp")
    ap.add_argument("--force", action="store_true",
                    help="allow --emit while files are still un-converted (partial cpp!)")
    ap.add_argument("--diff", action="store_true", help="compare rows against the pool CSVs")
    ap.add_argument("--dump-gs", action="store_true", help="(re)write Objects/gs_ids.csv from the pool")
    ap.add_argument("--sync-pool", action="store_true",
                    help="propose pool rows for checks present in New/** but missing from the pool "
                         "(e.g. new gossips); dry-run unless --write")
    ap.add_argument("--write", action="store_true", help="with --sync-pool: append the missing rows")
    args = ap.parse_args()

    if args.dump_gs:
        if os.path.exists(GS_IDS):
            os.remove(GS_IDS)
        load_gs_ids()
        print(f"wrote {os.path.relpath(GS_IDS, HERE)}")
        if not (args.emit or args.diff):
            return

    g = build_objects(args.ootmm_root, quiet=False)
    skipped = getattr(g, "skipped", 0)

    if args.sync_pool:
        sync_pool(g, write=args.write)
        return

    if args.emit:
        if skipped and not args.force:
            print(f"\nREFUSING --emit: {skipped} file(s) still un-converted (no <scene_rendering>) -> "
                  "the generated cpp would DROP those scenes. Convert them first, or pass --force:")
            for sf in getattr(g, "skipped_files", []):
                print("     ", sf)
        else:
            rep_oot = emit_cpp(g.rows, g.remap, "OoT", os.path.normpath(OUT_OOT))
            rep_mm = emit_cpp(g.rows, g.remap, "MM", os.path.normpath(OUT_MM))
            drift = {k: rep_oot[k] + rep_mm[k] for k in rep_oot}
            if any(drift.values()):
                print("\n==== scene registration to reconcile by hand (Objects.cpp / ObjectScene.h) ====")
                if drift["was_empty_now_has_objects"]:
                    print("  remove CreateEmptyScene(...) for (now non-empty):")
                    for s in drift["was_empty_now_has_objects"]:
                        print("     ", s)
                if drift["missing_extern"]:
                    print("  add 'extern ObjectInfo* <scene>SceneObjects;' to ObjectScene.h for:")
                    for s in drift["missing_extern"]:
                        print("     ", s)
                if drift["missing_from_aggregate"]:
                    print("  add CreateSceneObjects(<scene>) to the OoT/MMSceneObjects aggregate for:")
                    for s in drift["missing_from_aggregate"]:
                        print("     ", s)
    if args.diff or not args.emit:
        run_diff(g)


if __name__ == "__main__":
    main()
