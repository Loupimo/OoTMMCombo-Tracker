#!/usr/bin/env python3
"""Generate the src/data/ folder module for the Rust tracker, from the CSV pools.

Safe to re-run: the generator reproduces the committed src/data/ byte-for-byte
(any previously hand-curated name has been ported back into the C++ sources, the
single source of truth). Verify after a run with `git diff -- rust-tracker/src/data`.
It also emits src/data/logic.rs via gen_logic.py (the OoTMM reachability graph).

Same source of truth as the C++ generator (`Resources/Objects/Pool Transform.py`):
  - Resources/Objects/pool_oot.csv, pool_mm.csv   -> objects
  - Resources/Scenes/scenes_oot.csv, scenes_mm.csv -> scenes

The CSVs reference scenes / object-ids by their symbolic names (OOT_KOKIRI_FOREST,
DOG_LADY, ...). To keep those names in the generated Rust, we also read the C++
headers for their numeric values and enum variants, and emit:
  - `mod scenes` / `mod ids`  : the #define constants (readable, editable)
  - ObjectType / ObjectContext / GameLayout / LocType : the C++ enums
  - OOT_SCENES / MM_SCENES / OOT_OBJECTS / MM_OBJECTS  : the data arrays

Stdlib only (no pandas). Re-run after editing a CSV:  python tools/gen_data.py
"""
import csv
import json
import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]          # repository root
# The Qt tracker (Headers/ + Sources/) was relocated under C++-Tracker/, while
# Resources/ stayed at the repo root. `read()` tries C++-Tracker first, so a path
# like "Headers/Combo/Items.h" resolves there and "Resources/..." falls back.
CPP_ROOT = ROOT / "C++-Tracker" if (ROOT / "C++-Tracker" / "Headers").is_dir() else ROOT
# Output is now a folder module (src/data/*.rs). DATA_OUT env var overrides it
# (used to dry-run the generator to a temp dir without touching the tree).
OUTDIR = Path(os.environ.get("DATA_OUT",
              Path(__file__).resolve().parents[1] / "src" / "data"))

# id -> readable constant name, so the emitted arrays reference `iid::NAME` /
# `entr::NAME` instead of raw hex. Populated in main(); first-seen wins.
ITEM_REV = {}
ENT_REV = {}


def iid_ref(k):
    """`iid::NAME` for an item id, else its raw hex (0 / unknown ids)."""
    return f"iid::{ITEM_REV[k]}" if k in ITEM_REV else f"{k:#x}"


def entr_ref(k):
    """`entr::NAME` for an entrance id, else its raw hex."""
    return f"entr::{ENT_REV[k]}" if k in ENT_REV else f"{k:#x}"

# Kept in sync with Pool Transform.py: names stored unprefixed in the CSV that
# must be prefixed with the game (OOT_/MM_) to become the real macro.
COMMON_SCENES = {"FAIRY_FOUNTAIN", "GROTTOS", "GORON_SHOP", "LOST_WOODS",
                 "SHOOTING_GALLERY", "CUTSCENE_MAP", "TREASURE_SHOP",
                 "LABORATORY", "SPIDER_HOUSE_SWAMP", "SPIDER_HOUSE_OCEAN"}
COMMON_ID = {"SONG_STORMS", "SONG_OF_STORMS"}

LITERAL = re.compile(r"0x[0-9a-fA-F]+|\d+")


def resolve(rel):
    """Absolute path of a source file: under C++-Tracker/ if present there, else
    at the repo root (keeps working after the Qt tracker was moved)."""
    cand = CPP_ROOT / rel
    return cand if cand.exists() else ROOT / rel


def read(rel):
    return resolve(rel).read_text(encoding="utf-8", errors="replace")


def read_csv(rel):
    with resolve(rel).open(newline="", encoding="utf-8", errors="replace") as f:
        return list(csv.DictReader(f, delimiter=";"))


# --- Header parsing --------------------------------------------------------
def parse_defines(text, seed=None):
    """Evaluate the numeric `#define NAME <expr>` of a header, in order.

    @param seed  Pre-existing symbols (lets several headers chain).
    @return dict name -> int. Function-like / non-numeric macros are skipped.
    """
    sym = dict(seed or {})
    # The \s+ after the name means function macros `NAME(x)` never match.
    for line in text.splitlines():
        m = re.match(r"\s*#define\s+([A-Za-z_]\w*)\s+(.+)", line)
        if not m:
            continue
        name, expr = m.group(1), m.group(2)
        expr = re.sub(r"/\*.*?\*/", "", expr)
        expr = re.sub(r"//.*", "", expr).strip()
        if not expr:
            continue
        try:
            sym[name] = int(eval(expr, {"__builtins__": {}}, sym))
        except Exception:
            pass
    return sym


def parse_enum(text, name):
    """Return the ordered variant names of a C/C++ `enum name { ... }`."""
    m = re.search(
        r"enum(?:\s+class)?\s+%s\s*(?::\s*\w+\s*)?\{(.*?)\}" % re.escape(name), text, re.S)
    variants = []
    for part in m.group(1).split(","):
        part = re.sub(r"/\*.*?\*/", "", part)
        part = re.sub(r"//.*", "", part)
        tok = part.strip().split("=")[0].strip()
        if tok:
            variants.append(tok)
    return variants


def parse_enum_values(text, name):
    """Return (variant, value) pairs of a C/C++ enum, honouring `= expr` and the
    implicit +1 increment between entries."""
    m = re.search(r"enum(?:\s+class)?\s+%s(?:\s*:\s*\w+)?\s*\{(.*?)\}" % re.escape(name), text, re.S)
    out = []
    val = 0
    for part in m.group(1).split(","):
        part = re.sub(r"/\*.*?\*/", "", part)
        part = re.sub(r"//.*", "", part).strip()
        if not part:
            continue
        if "=" in part:
            nm, expr = part.split("=", 1)
            nm = nm.strip()
            val = int(eval(expr.strip(), {"__builtins__": {}}, {}))
        else:
            nm = part
        out.append((nm, val))
        val += 1
    return out


def parse_regions(text):
    """Region enum name -> {variant: [id, display name, icon path]}, per game."""
    out = {}
    for enum_name in ("OoTRegions", "MMRegions"):
        ids = {name: i for i, name in enumerate(parse_enum(text, enum_name))}
        out[enum_name] = {name: [rid, name.replace("_", " "), ""] for name, rid in ids.items()}
    for arr, enum_name in (("OoTRegionsMetaInfo", "OoTRegions"),
                           ("MMRegionsMetaInfo", "MMRegions")):
        block = re.search(r"%s\s*\[[^\]]*\]\s*=\s*\{(.*?)\n\};" % arr, text, re.S).group(1)
        for name, icon, en in re.findall(
            r'\{\s*"((?:[^"\\]|\\.)*)"\s*,\s*"([^"]*)"\s*,\s*\(uint8_t\)\s*\w+::(\w+)', block
        ):
            if en in out[enum_name]:
                out[enum_name][en][1] = name
                out[enum_name][en][2] = icon
    return out


def emit_region_icons(name, region_dict):
    """A `&[&str]` of region icon paths indexed by region id (0 = None = empty)."""
    by_id = {v[0]: (v[2] if len(v) > 2 else "") for v in region_dict.values()}
    n = (max(by_id) + 1) if by_id else 0
    cells = ", ".join('"{}"'.format(esc(by_id.get(i, ""))) for i in range(n))
    return f"pub static {name}: &[&str] = &[{cells}];\n"


def parse_entrance_costs(text, arr):
    """Parse an EntranceCostMeasurement[] array (OoT/MMEntranceCosts.cpp):
    each `{ scene, from, to, cost }` -> (scene, from, to, cost) ints, in order."""
    block = re.search(r"%s\s*\[\s*\]\s*=\s*\{(.*?)\n\};" % arr, text, re.S).group(1)
    rows = []
    for sc, fr, to, cost in re.findall(
        r"\{\s*(0x[0-9a-fA-F]+|\d+)\s*,\s*(0x[0-9a-fA-F]+|\d+)\s*,"
        r"\s*(0x[0-9a-fA-F]+|\d+)\s*,\s*(\d+)\s*\}",
        block,
    ):
        rows.append((int(sc, 0), int(fr, 0), int(to, 0), int(cost, 0)))
    return rows


def emit_entrance_costs(name, rows):
    """A `&[(u16, u32, u32, u32)]` of measured intra-scene walk times
    (scene, from-entrance, to-entrance, cost seconds)."""
    cells = ", ".join(f"({s}, {f}, {t}, {c})" for s, f, t, c in rows)
    return f"pub static {name}: &[(u16, u32, u32, u32)] = &[{cells}];\n"


def emit_region_names(name, region_dict):
    """A `&[&str]` of region display names indexed by region id. Unlike the icon
    table, warp regions (OoT Songs / MM Owls) carry a name here even though no
    ordinary scene lives in them, so the entrance nav can label them instead of
    deriving the name from a SceneDef.region_name that does not exist."""
    by_id = {v[0]: (v[1] if len(v) > 1 else "") for v in region_dict.values()}
    n = (max(by_id) + 1) if by_id else 0
    # Region 0 is the "None" bucket: keep it empty like the icon table.
    cells = ", ".join('"{}"'.format(esc(by_id.get(i, "")) if i else "") for i in range(n))
    return f"pub static {name}: &[&str] = &[{cells}];\n"


# --- CSV -> rows -----------------------------------------------------------
def prefix_scene(name, prefix):
    name = name.strip()
    return prefix + name if name in COMMON_SCENES else name


def load_objects(rel, prefix, scene_sym, id_sym, used_ids, missing, missing_ids):
    objs = []
    for row in read_csv(rel):
        scene = prefix_scene(row["scene"], prefix)
        render = prefix_scene(row["renderscene"], prefix)
        if scene not in scene_sym:
            missing.add(scene)
            continue
        if render not in scene_sym:
            missing.add(render)
            render = scene

        idtok = row["id"].strip()
        if idtok in COMMON_ID:
            idtok = prefix + idtok
        if LITERAL.fullmatch(idtok):
            id_ref = idtok                       # literal, e.g. 0x0002
        elif idtok in id_sym:
            used_ids.add(idtok)
            id_ref = f"ids::{idtok}"             # named constant
        else:
            missing_ids.add(idtok)
            id_ref = "0xFFFF_FFFF"

        def as_int(key):
            try:
                return int(str(row[key]).strip())
            except ValueError:
                return 0

        objs.append({
            "id_ref": id_ref,
            "scene": scene,
            "name": row["friendly_name"],
            "location": row["location"],
            "type": row["type"].strip(),
            "x": as_int("x"), "y": as_int("y"), "z": as_int("z"),
            "render": render,
            "render_type": row["rendertype"].strip(),
            "map_icon": row["icontype"].strip(),
            "context": row["context"].strip(),
            "room": as_int("room"),
            "layout": row["game_layout"].strip(),
            "loc_type": row["loc_type"].strip(),
        })
    return objs


# --- New-system objects: reuse the C++ generator (single source = New/** XML) ----
_GEN_OBJECTS = None


def gen_object_rows():
    """Import the C++ object generator (Resources/gen_objects.py) and build every check once from
    the New/** XML: legacy ObjectID pool-preferred, XflagID by Location, plus the legacy-scene
    remap. Shared so the Rust tracker and the Qt tracker resolve checks identically."""
    global _GEN_OBJECTS
    if _GEN_OBJECTS is None:
        sys.path.insert(0, str(CPP_ROOT / "Resources"))
        import gen_objects
        _GEN_OBJECTS = gen_objects.build_objects()
    return _GEN_OBJECTS


def _id_ref(idtok, id_sym, used_ids, missing_ids):
    idtok = str(idtok).strip()
    if LITERAL.fullmatch(idtok):
        return idtok
    if idtok in id_sym:
        used_ids.add(idtok)
        return f"ids::{idtok}"
    missing_ids.add(idtok)
    return "0xFFFF_FFFF"


# Overlay checks whose in-game flag the DLL reports as (OvType, id). The runtime
# resolves these globally by (type, object_id), so object_id MUST equal that flag.
_OVERLAY_TAGS = ("gs", "scrub", "cow", "shop", "sr", "fish")
_OVERLAY_FLAGS = None


def overlay_flags():
    """`{game-prefixed location -> flag}` for the overlay checks, parsed from the
    OoTMM checks XML — the authoritative id the game / DLL sends when one is collected.
    Master-Quest objects are stored in the pool under their base-dungeon slot id, which
    does NOT match this reported flag; the caller uses this map to restamp the MQ
    overlay object_id so it resolves. Cached (parsed once)."""
    global _OVERLAY_FLAGS
    if _OVERLAY_FLAGS is None:
        _OVERLAY_FLAGS = {}
        base = resolve("Resources/OoTMM-master/data/checks")
        pat = re.compile(
            r'<(' + '|'.join(_OVERLAY_TAGS) + r')\s+location="([^"]+)"[^>]*?\bflag="([^"]+)"')
        for path in Path(base).rglob("*.xml"):
            p = str(path).replace("\\", "/")
            prefix = "OOT " if "/oot/" in p else ("MM " if "/mm/" in p else "")
            if not prefix:
                continue
            for m in pat.finditer(path.read_text(encoding="utf-8", errors="replace")):
                try:
                    _OVERLAY_FLAGS[prefix + m.group(2)] = (m.group(1), int(m.group(3), 0))
                except ValueError:
                    pass
    return _OVERLAY_FLAGS


def objects_from_rows(bucket, scene_sym, id_sym, used_ids, missing, missing_ids):
    """gen_objects rows for one game -> ObjectDef dicts (adds xflag_id + legacy_scene). Scenes and
    the legacy ObjectID come already game-prefixed from gen_objects, so no re-prefixing here."""
    def i0(v, d=0):
        try:
            return int(str(v), 0)
        except (ValueError, TypeError):
            return d
    rows = gen_object_rows().rows      # puts Resources/ on sys.path + imports gen_objects
    import gen_objects                 # now resolvable (for SYSTEM_ENUM: token -> ObjSystem variant)
    objs = []
    for r in rows:
        if r["bucket"] != bucket:
            continue
        scene, render = r["scene"], r["renderscene"]
        if scene not in scene_sym:
            missing.add(scene)
            continue
        if render not in scene_sym:
            missing.add(render)
            render = scene
        legacy = r.get("legacy_scene")
        id_ref = _id_ref(r["id"], id_sym, used_ids, missing_ids)
        # Master-Quest overlay objects are stored under their base-dungeon slot id, but
        # the game reports the MQ-specific flag (e.g. MQ Dodongo scrub 0x9d, not the base
        # 0x1d), so a global (type, id) lookup never matches them. Restamp the object_id
        # to that reported flag (gs is kept at flag+8 to stay consistent with the pool's
        # base gs and the runtime resolve_raw_gs; the other overlays use the flag as-is).
        if r["game_layout"].strip() in ("oot_mq", "mm_jp") and r["type"] in _OVERLAY_TAGS:
            hit = overlay_flags().get(r["location"].strip())
            if hit is not None:
                _, flag = hit
                id_ref = f"{flag + 8 if r['type'] == 'gs' else flag:#x}"
        objs.append({
            "id_ref": id_ref,
            "scene": scene, "name": r["friendly_name"], "location": r["location"],
            "type": r["type"], "x": r["x"], "y": r["y"], "z": r["z"],
            "render": render, "render_type": r["rendertype"], "map_icon": r["icontype"],
            "context": r["context"], "room": i0(r["room"]), "layout": r["game_layout"],
            "loc_type": r["loc_type"], "xflag_id": i0(r["xflag_id"], 0xFFFF),
            "system": gen_objects.SYSTEM_ENUM.get(r.get("system", ""), "Any"),
            "legacy_scene": legacy if legacy in scene_sym else None,
        })
    return objs


def emit_legacy_remap(name, bucket, id_sym, used_ids, missing_ids):
    """The per-game legacy-scene remap table for the Rust runtime's find_object fallback."""
    out = [f"pub static {name}: &[LegacySceneRemap] = &["]
    for bk, legacy, objid, otype, true_scene, loc in gen_object_rows().remap:
        if bk != bucket:
            continue
        out.append(
            f"    LegacySceneRemap {{ legacy_scene: scenes::{legacy}, "
            f"object_id: {_id_ref(objid, id_sym, used_ids, missing_ids)}, "
            f"type_: ObjectType::{rust_ident(otype)}, true_scene: scenes::{true_scene} }}, // {loc}")
    out.append("];\n")
    return "\n".join(out)


# --- CSV-independent: room tables from RoomRenderer.h ----------------------
ROOM_ENTRY = re.compile(
    r"(\w+)\s*,\s*std::vector<RoomInfo>\s*\(\s*\{(.*?)\}\s*\)", re.S
)
ROOM_ITEM = re.compile(
    r'\{\s*(\d+)\s*,\s*"((?:[^"\\]|\\.)*)"\s*,\s*"((?:[^"\\]|\\.)*)"\s*\}'
)


def parse_rooms(text, scene_sym):
    """Per-scene room lists (RoomRenderer.h) -> (oot_entries, mm_entries)."""
    oot, mm = [], []
    for m in ROOM_ENTRY.finditer(text):
        macro, block = m.group(1), m.group(2)
        if macro not in scene_sym:
            continue
        rooms = [(int(r.group(1)), r.group(2), r.group(3)) for r in ROOM_ITEM.finditer(block)]
        (oot if macro.startswith("OOT_") else mm).append((macro, rooms))
    return oot, mm


def build_entrance_code_map(rows):
    """Code_Name -> To_ID value (Pool Transform #defines each code = its To_ID),
    used to resolve the named From_ID of the reverse entrance."""
    m = {}
    for r in rows:
        try:
            m[r["Code_Name"].strip()] = int(r["To_ID"].strip(), 0)
        except ValueError:
            pass
    return m


def load_entrances(rel, scene_sym, missing, missing_ent):
    rows = read_csv(rel)
    code_map = build_entrance_code_map(rows)

    def resolve_id(tok):
        tok = tok.strip()
        if LITERAL.fullmatch(tok):
            return int(tok, 0)
        if tok in code_map:
            return code_map[tok]
        missing_ent.add(tok)
        return 0

    def as_int(v):
        try:
            return int(str(v).strip())
        except ValueError:
            return 0

    out = []
    for r in rows:
        to_scene = r["To_Scene"].strip()
        from_scene = r["From_Scene"].strip()
        if to_scene not in scene_sym:
            missing.add(to_scene)
            continue
        if from_scene not in scene_sym:
            from_scene = to_scene
        out.append({
            "to_id": resolve_id(r["To_ID"]),
            "from_id": resolve_id(r["From_ID"]),
            "from_scene": from_scene,
            "to_scene": to_scene,
            "from_name": r["From_Name"],
            "to_name": r["To_Name"],
            "type": r["Type"].strip() or "None",
            "ax": as_int(r["Anchor_X"]), "ay": as_int(r["Anchor_Y"]),
            "tx": as_int(r["Text_X"]), "ty": as_int(r["Text_Y"]),
            "icon": r["Entrance_Icon"].strip(),
            "layout": r["Active_Layout"].strip(),
        })
    return out


def load_scenes(rel, region_map, scene_sym, missing):
    scenes = []
    for row in read_csv(rel):
        sid = row["scene_id"].strip()
        if sid not in scene_sym:
            missing.add(sid)
            continue
        region = region_map.get(row["parent_region"].strip())
        rid, rname = (region[0], region[1]) if region else (0, row["parent_region"].strip())
        scenes.append({
            "id_name": sid,
            "name": row["scene_name"],
            # image_rel = artistic map (items) ; minimap_rel = minimap (entrances).
            "image_rel": row["image_path"].strip(),
            "minimap_rel": row["minimap_path"].strip(),
            "region_id": rid,
            "region_name": rname,
            "has_context": str(row["has_context"]).strip().lower() == "true",
            "layout": row["active_layout"].strip(),
            # Optional per-context map (Spring / Adult); missing column -> "".
            "context_image_rel": (row.get("context_image_path") or "").strip(),
        })
    return scenes


# --- Rust emission ---------------------------------------------------------
# `crate`/`self`/`super`/`Self` cannot be identifiers in Rust (not even raw), so
# any C++ enum value colliding with a Rust keyword gets a trailing underscore.
RUST_KEYWORDS = {
    "crate", "self", "super", "Self", "as", "break", "const", "continue", "dyn",
    "else", "enum", "extern", "false", "fn", "for", "if", "impl", "in", "let",
    "loop", "match", "mod", "move", "mut", "pub", "ref", "return", "static",
    "struct", "trait", "true", "type", "unsafe", "use", "where", "while", "async",
    "await", "box",
}


def rust_ident(name):
    return name + "_" if name in RUST_KEYWORDS else name


def esc(s):
    return str(s).replace("\\", "\\\\").replace('"', '\\"')


def emit_enum(name, variants, comment, repr_u8=False):
    body = ",\n    ".join(rust_ident(v) for v in variants)
    # repr(u8) lets the type be compared to the raw OvType bytes (same numbering
    # for values 0..=fish), which is how FindObject matches objects.
    repr_attr = "#[repr(u8)]\n" if repr_u8 else ""
    return (f"/// Mirror of {comment}.\n"
            f"{repr_attr}"
            "#[derive(Clone, Copy, PartialEq, Eq, Hash)]\n"
            f"pub enum {name} {{\n    {body},\n}}\n\n")


def emit_name_id_table(name, entries):
    """`pub static NAME: &[(&str, u32)]` sorted by key, for runtime binary_search
    (the spoiler `Entrances` section joins entrance const-names to ids)."""
    lines = [f"pub static {name}: &[(&str, u32)] = &["]
    for n, v in sorted(entries):
        lines.append(f'    ("{n}", {v:#x}),')
    lines.append("];\n")
    return "\n".join(lines)


def emit_consts(mod, entries, ty, use_scene_id=False):
    lines = [f"pub mod {mod} {{"]
    if use_scene_id:
        lines.append("    use super::SceneId;")
    for name, val in entries:
        lines.append(f"    pub const {name}: {ty} = {val:#x};")
    lines.append("}\n")
    return "\n".join(lines)


def emit_scenes(name, scenes):
    out = [f"pub static {name}: &[SceneDef] = &["]
    for s in scenes:
        out.append(
            "    SceneDef {{ id: scenes::{id}, name: \"{nm}\", image_rel: \"{img}\", "
            "minimap_rel: \"{mini}\", region_id: {rid}, region_name: \"{rn}\", "
            "has_context: {ctx}, layout: GameLayout::{lay}, context_image_rel: \"{cimg}\" }},".format(
                id=s["id_name"], nm=esc(s["name"]), img=esc(s["image_rel"]),
                mini=esc(s["minimap_rel"]), rid=s["region_id"], rn=esc(s["region_name"]),
                ctx=str(s["has_context"]).lower(), lay=rust_ident(s["layout"]),
                cimg=esc(s["context_image_rel"])))
    out.append("];\n")
    return "\n".join(out)


def emit_rooms(name, entries):
    out = [f"pub static {name}: &[(SceneId, &[RoomDef])] = &["]
    for macro, rooms in entries:
        cells = ", ".join(
            'RoomDef {{ id: {i}, name: "{n}", image_rel: "{p}" }}'.format(
                i=i, n=esc(n), p=esc(p))
            for (i, n, p) in rooms
        )
        out.append(f"    (scenes::{macro}, &[{cells}]),")
    out.append("];\n")
    return "\n".join(out)


# --- Grotto position table (Entrances.cpp GrottoEntrances) -----------------
GROTTO_KEY = re.compile(r"(\w+)\s*,\s*std::vector<GrottoEntrance>")
GROTTO_ITEM = re.compile(
    r"\{\s*(\w+)\s*,\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*\}"
)


def resolve_entr(tok, ent_sym, missing):
    tok = tok.strip()
    if LITERAL.fullmatch(tok):
        return int(tok, 0)
    if tok in ent_sym:
        return ent_sym[tok]
    missing.add(tok)
    return None


def parse_grottos(text, ent_sym, missing):
    m = re.search(r"GrottoEntrances\s*=\s*\{(.*?)\n\};", text, re.S)
    if not m:
        return []
    block = m.group(1)
    keys = list(GROTTO_KEY.finditer(block))
    out = []
    for i, km in enumerate(keys):
        end = keys[i + 1].start() if i + 1 < len(keys) else len(block)
        span = block[km.end():end]
        items = []
        for it in GROTTO_ITEM.finditer(span):
            eid = resolve_entr(it.group(1), ent_sym, missing)
            if eid is not None:
                items.append((eid, float(it.group(2)), float(it.group(3)), float(it.group(4))))
        key = resolve_entr(km.group(1), ent_sym, missing)
        if key is not None:
            out.append((key, items))
    return out


def emit_grottos(entries):
    lines = ["pub static GROTTO_ENTRANCES: &[(u32, &[GrottoPos])] = &["]
    for key_val, items in entries:
        cells = ", ".join(
            "GrottoPos {{ id: {:#x}, pos: [{:.1f}, {:.1f}, {:.1f}] }}".format(i, x, y, z)
            for (i, x, y, z) in items
        )
        lines.append(f"    ({key_val:#x}, &[{cells}]),")
    lines.append("];\n")
    return "\n".join(lines)


# --- CheckGrottoSpawn: generate the case-group -> canonical map from C++ -----
SPAWN_GROUP = re.compile(r"((?:case\s+\w+\s*:\s*)+)\{\s*Message\.EntranceID\s*=\s*(\w+)\s*;", re.S)
CASE_NAME = re.compile(r"case\s+(\w+)\s*:")


def parse_check_grotto_spawn(text, ent_sym, missing):
    m = re.search(
        r"CheckGrottoSpawn\(EntranceMessage& Message\)(.*?)\nuint32_t EntranceHelper::",
        text, re.S,
    )
    body = re.sub(r"/\*.*?\*/", "", m.group(1), flags=re.S)
    body = re.sub(r"//.*", "", body)
    oot, mm = [], []
    for g in SPAWN_GROUP.finditer(body):
        canon = resolve_entr(g.group(2), ent_sym, missing)
        if canon is None:
            continue
        target = oot if g.group(2).startswith("OOT_") else mm
        for cm in CASE_NAME.finditer(g.group(1)):
            src = resolve_entr(cm.group(1), ent_sym, missing)
            if src is not None:
                target.append((src, canon))
    oot.sort()
    mm.sort()
    return oot, mm


def emit_pairs(name, pairs):
    cells = ", ".join(f"({a:#x}, {b:#x})" for (a, b) in pairs)
    return f"pub static {name}: &[(u32, u32)] = &[{cells}];\n"


# --- SceneEntranceMeta: scene -> entrances rendered on that scene -------------
# The hand-authored map in SceneOoT/MMEntrances.cpp groups entrance ids by the
# scene they belong to. Scene structs are anchored on the `(uint8_t)Region` cast;
# entrance entries on the default `{ { { UINT32_MAX, NO_GAME } }, UINT32_MAX }`.
# The scene anchor also carries the entrance-side region (the `(uint8_t)Region`
# cast), which is DISTINCT from the scene's own ParentRegion — e.g. the Market /
# ToT Entryway hub scenes are ParentRegion None but entrance-region Market. Group
# 2 captures that region variant so the entrance nav can group by it.
SEMETA_SCENE = re.compile(r"(\w+)\s*,\s*\(uint8_t\)(?:\w+::)?(\w+)")
SEMETA_ENTR = re.compile(r"\{\s*(\w+)\s*,\s*\{\s*\{\s*\{\s*UINT32_MAX")


def parse_scene_entrances(text, scene_sym, ent_sym, missing, missing_ent, region_map):
    """SceneOoT/MMEntrances.cpp -> [(scene_id, region_id, [entrance_id, ...]), ...]."""
    events = [(m.start(), "s", (m.group(1), m.group(2))) for m in SEMETA_SCENE.finditer(text)]
    events += [(m.start(), "e", m.group(1)) for m in SEMETA_ENTR.finditer(text)]
    events.sort(key=lambda e: e[0])  # by source position (unique) -> avoids tok type-compare
    out, cur = [], None
    for _, kind, tok in events:
        if kind == "s":
            scene_name, region_name = tok
            if scene_name in scene_sym:
                region = region_map.get(region_name)
                rid = region[0] if region else 0
                cur = (scene_sym[scene_name], rid, [])
                out.append(cur)
            else:
                missing.add(scene_name)
                cur = None
        elif cur is not None:
            eid = resolve_entr(tok, ent_sym, missing_ent)
            if eid is not None:
                cur[2].append(eid)
    out.sort(key=lambda kv: kv[0])  # sorted by scene id for binary search
    return out


def emit_scene_entrances(name, entries):
    lines = [f"pub static {name}: &[(SceneId, u8, &[u32])] = &["]
    for sid, rid, ents in entries:
        cells = ", ".join(entr_ref(e) for e in ents)
        lines.append(f"    ({sid:#x}, {rid}, &[{cells}]),")
    lines.append("];\n")
    return "\n".join(lines)


ICON_ENTRY = re.compile(r'\{\s*"((?:[^"\\]|\\.)*)"')
# An IconsMetaInfo row: {"path", {w, h}, ...}. Captures the default render size.
ICON_SIZE_ENTRY = re.compile(r'\{\s*"(?:[^"\\]|\\.)*"\s*,\s*\{\s*(\d+)\s*,\s*(\d+)\s*\}')


def parse_icons(text, n):
    """First `n` icon paths of IconsMetaInfo (indexed by ObjectType)."""
    return [m.group(1) for m in ICON_ENTRY.finditer(text)][:n]


def parse_icon_sizes(text, n):
    """First `n` icon default sizes [w, h] of IconsMetaInfo (by ObjectType)."""
    return [(int(m.group(1)), int(m.group(2))) for m in ICON_SIZE_ENTRY.finditer(text)][:n]


def emit_icon_sizes(sizes):
    cells = ", ".join(f"[{w}, {h}]" for w, h in sizes)
    return (
        "/// Default icon render size [w, h] per ObjectType (Icons.cpp).\n"
        f"pub static ICON_TYPE_SIZES: &[[u16; 2]] = &[{cells}];\n"
    )


def parse_icon_by_name(icons_h, icons_cpp):
    """Map each EGameIcon variant name to its IconsMetaInfo path (by index)."""
    names = parse_enum(icons_h, "EGameIcon")
    paths = [m.group(1) for m in ICON_ENTRY.finditer(icons_cpp)]
    out = []
    for name, path in zip(names, paths):
        if path and name not in ("type", "render_type"):
            out.append((name, path))
    out.sort()
    return out


def emit_icon_by_name(entries):
    cells = ",\n    ".join('("{}", "{}")'.format(esc(n), esc(p)) for n, p in entries)
    return (
        "/// EGameIcon name -> image path (Icons.cpp), for per-object MapIcon "
        "rendering (sorted).\n"
        f"pub static ICON_BY_NAME: &[(&str, &str)] = &[\n    {cells},\n];\n"
    )


def parse_entrance_icons(icons_h, icons_cpp):
    """Map each EntranceIcons variant name to its EntranceIconsMetaInfo path."""
    names = [n for n in parse_enum(icons_h, "EntranceIcons") if n != "Entrance_Last"]
    m = re.search(r"EntranceIconsMetaInfo\[[^\]]*\]\s*=\s*\{(.*?)\n\};", icons_cpp, re.S)
    paths = [e.group(1) for e in ICON_ENTRY.finditer(m.group(1))]
    out = [(n, p) for n, p in zip(names, paths) if p]  # drop the empty No_Entry
    out.sort()
    return out


def emit_entrance_icons(entries):
    cells = ",\n    ".join('("{}", "{}")'.format(esc(n), esc(p)) for n, p in entries)
    return (
        "/// EntranceIcons name -> image path (Icons.cpp EntranceIconsMetaInfo), "
        "sorted for a binary search.\n"
        f"pub static ENTRANCE_ICONS: &[(&str, &str)] = &[\n    {cells},\n];\n"
    )


def emit_icons(paths):
    cells = ", ".join('"{}"'.format(esc(p)) for p in paths)
    return (
        "/// Icon path per ObjectType (index = `ObjectType as usize`), from "
        "Icons.cpp.\n"
        f"pub static ICON_PATHS: &[&str] = &[{cells}];\n"
    )


# --- Object-type display names + per-game filterable type sets ----------------
def parse_obj_type_names(objects_h, n):
    """The ObjTypeName[] display strings (index = ObjectType value), from Objects.h."""
    m = re.search(r"ObjTypeName\[[^\]]*\]\s*=\s*\{(.*?)\};", objects_h, re.S)
    names = [mm.group(1) for mm in re.finditer(r'"((?:[^"\\]|\\.)*)"', m.group(1))]
    return names[:n]


def parse_type_set(objects_h, name):
    """The ObjectType members of a `const QSet<ObjectType> <name> = { ... };`."""
    m = re.search(r"QSet<ObjectType>\s*" + name + r"\s*=\s*\{(.*?)\};", objects_h, re.S)
    return re.findall(r"ObjectType::(\w+)", m.group(1))


def emit_obj_type_names(names):
    cells = ", ".join('"{}"'.format(esc(p)) for p in names)
    return (
        "/// Display name per ObjectType (index = `ObjectType as usize`), from "
        "Objects.h ObjTypeName.\n"
        f"pub static OBJ_TYPE_NAMES: &[&str] = &[{cells}];\n"
    )


def emit_type_set(name, types):
    cells = ", ".join(f"ObjectType::{rust_ident(t)}" for t in types)
    return f"pub static {name}: &[ObjectType] = &[{cells}];\n"


# --- ROM settings parameters (Settings.cpp FilterSettings / ItemSettings) -----
# Display-name overrides, keyed by setting key: patch known Settings.cpp typos here
# so a regen keeps the corrected label without editing the C++ source. The MM shovel
# toggle is copy-pasted from the OoT one and still reads "Shovel - OoT" in Settings.cpp.
SETTING_NAME_FIX = {
    "shovelMm": "Shovel - MM",
}
SETTING_ENTRY = re.compile(
    r'\{\s*"([^"]+)"\s*,\s*\{\s*"([^"]*)"\s*,\s*ParamType::(\w+)\s*,'
    r"\s*ParamCategory::(\w+)\s*,\s*ShuffleSetting::(\w+)\s*,\s*\{([^}]*)\}",
    re.S,
)


def parse_settings_map(text, member, id_sym, missing_ids):
    """One of the `this-><member> = QMap<...>({ ... });` parameter maps."""
    m = re.search(
        member + r"\s*=\s*QMap<QString,\s*Parameter>\(\{(.*?)\n\s*\}\);", text, re.S)
    out = []
    for e in SETTING_ENTRY.finditer(m.group(1)):
        key, name, ptype, pcat, shuf, aff = e.groups()
        name = SETTING_NAME_FIX.get(key, name)
        ids = []
        for tok in aff.split(","):
            tok = tok.strip()
            if not tok:
                continue
            if tok in id_sym:
                ids.append(id_sym[tok])
            elif LITERAL.fullmatch(tok):
                ids.append(int(tok, 0))
            else:
                missing_ids.add(tok)
        out.append((key, name, ptype, pcat, shuf, ids))
    return out


# --- Item table (Items.cpp ItemList) + progressive families ------------------
# One ItemList row: { ITEM_ID, "Name", EGameIcon::icon, canBeShared }.
ITEM_ENTRY = re.compile(
    r'\{\s*(\w+)\s*,\s*"((?:[^"\\]|\\.)*)"\s*,\s*EGameIcon::\w+\s*,\s*(true|false)\s*\}')
# One ProgressiveFamilies row: { base, upgrade1, upgrade2 } (0 pads a single-upgrade family).
FAMILY_ROW = re.compile(r"\{\s*(\w+)\s*,\s*(\w+)\s*,\s*(\w+)\s*\}")


def _resolve_id(tok, id_sym, missing_ids):
    tok = tok.strip()
    if tok in id_sym:
        return id_sym[tok]
    if LITERAL.fullmatch(tok):
        return int(tok, 0)
    missing_ids.add(tok)
    return None


# Items defined in Items.h (and referenced by progression / settings) that are not
# yet present in the Items.cpp ItemList, so parse_items can't find a display name
# for them. They are spliced in at their id position below so ITEMS stays dense
# (position == id - 1, the FindItem fast path). Remove an entry here once it has
# been added to the C++ ItemList (then the parsed name / flag wins).
EXTRA_ITEMS = [
    ("OOT_CLOCK", "Clock (OoT)", False),
    ("OOT_SHOVEL", "Shovel (OoT)", True),
    ("MM_SHOVEL", "Shovel (MM)", True),
    ("SHARED_SHOVEL", "Shovel", False),
]


def parse_items(items_cpp, id_sym, missing_ids):
    """The ItemList[] table (Items.cpp): [(id, name, can_be_shared), ...] in file order."""
    m = re.search(r"ItemList\[NUM_ITEM\]\s*=\s*\{(.*?)\n\};", items_cpp, re.S)
    out = []
    for e in ITEM_ENTRY.finditer(m.group(1)):
        iid = _resolve_id(e.group(1), id_sym, missing_ids)
        if iid is not None:
            out.append((iid, e.group(2), e.group(3) == "true"))
    # Splice in the not-yet-in-ItemList items at their id position (keeps ITEMS
    # dense). Skipped if the symbol is unknown or already parsed from the list.
    have = {iid for iid, _, _ in out}
    for sym, name, shared in EXTRA_ITEMS:
        iid = id_sym.get(sym)
        if iid is None or iid in have:
            continue
        pos = next((k for k, o in enumerate(out) if o[0] > iid), len(out))
        out.insert(pos, (iid, name, shared))
    return out


def emit_items(items):
    lines = ["pub static ITEMS: &[ItemDef] = &["]
    for iid, name, shared in items:
        lines.append(
            f'    ItemDef {{ id: {iid:#x}, name: "{esc(name)}", '
            f"can_be_shared: {str(shared).lower()} }},")
    lines.append("];\n")
    # Lowercased name -> id, first ItemList occurrence wins (mirrors FindItemByName's
    # top-down scan), sorted for a binary search.
    seen = {}
    for iid, name, _ in items:
        seen.setdefault(name.lower(), iid)
    cells = ",\n    ".join(f'("{esc(k)}", {v:#x})' for k, v in sorted(seen.items()))
    lines.append(
        "/// Lowercased item name -> id (FindItemByName), sorted for binary search.")
    lines.append(f"pub static ITEM_BY_NAME_LC: &[(&str, u32)] = &[\n    {cells},\n];\n")
    return "\n".join(lines)


def parse_progressive_families(items_cpp, id_sym, missing_ids):
    """The ProgressiveFamilies[][3] table (Items.cpp) -> [[base, up1, up2], ...]."""
    m = re.search(r"ProgressiveFamilies\[\]\[3\]\s*=\s*\{(.*?)\n\};", items_cpp, re.S)
    out = []
    for r in FAMILY_ROW.finditer(m.group(1)):
        vals = [_resolve_id(t, id_sym, missing_ids) or 0 for t in r.groups()]
        out.append(vals)
    return out


def emit_progressive_families(fams):
    cells = ", ".join("[{}, {}, {}]".format(*(iid_ref(k) for k in f)) for f in fams)
    return (
        "/// Progressive capacity families [base, upgrade1, upgrade2] (Items.cpp).\n"
        f"pub static PROGRESSIVE_FAMILIES: &[[u32; 3]] = &[{cells}];\n")


# --- Progression dashboard tables (ProgressionEntry.cpp) ----------------------
# One ProgEntry: { EGameIcon::icon, "Name", { id, ... }, isCounter [, maxCount [, maxFromSpoiler]] }.
PROG_ENTRY = re.compile(
    r'\{\s*EGameIcon::(\w+)\s*,\s*"((?:[^"\\]|\\.)*)"\s*,\s*\{([^}]*)\}\s*,\s*'
    r"(true|false)\s*(?:,\s*(-?\d+)\s*)?(?:,\s*(true|false)\s*)?\}", re.S)


def parse_prog_arrays(prog_cpp, id_sym, missing_ids):
    """Every `const ProgEntry NAME[] = { ... };` -> {NAME: [entry-dict, ...]}."""
    out = {}
    for m in re.finditer(r"const\s+ProgEntry\s+(\w+)\s*\[\]\s*=\s*\{(.*?)\n\};", prog_cpp, re.S):
        body = re.sub(r"/\*.*?\*/", "", m.group(2), flags=re.S)
        body = re.sub(r"//.*", "", body)
        entries = []
        for e in PROG_ENTRY.finditer(body):
            icon, disp, keys, counter, maxc, maxspoil = e.groups()
            ids = [_resolve_id(t, id_sym, missing_ids) for t in keys.split(",") if t.strip()]
            entries.append({
                "icon": icon, "name": disp,
                "keys": [i for i in ids if i is not None],
                "counter": counter == "true",
                "max": int(maxc) if maxc else 0,
                "max_spoiler": maxspoil == "true",
            })
        out[m.group(1)] = entries
    return out


def parse_prog_sections(prog_cpp):
    """Every `const ProgSection NAME[...] = { MAKE_SECTION(...) };` -> {NAME: [(title, arr)]}."""
    out = {}
    for m in re.finditer(r"const\s+ProgSection\s+(\w+)\s*\[[^\]]*\]\s*=\s*\{(.*?)\n\};", prog_cpp, re.S):
        out[m.group(1)] = re.findall(
            r'MAKE_SECTION\(\s*"((?:[^"\\]|\\.)*)"\s*,\s*(\w+)\s*\)', m.group(2))
    return out


# The four dashboard pages (title from ProgressionTab.cpp) and their ProgSection array.
PROG_PAGES = [
    ("Ocarina of Time", "OoTSections"),
    ("Majora's Mask", "MMSections"),
    ("Souls", "SoulsSections"),
    ("Collectibles", "CollectiblesSections"),
]


def emit_prog(sections_map, arrays_map):
    lines = ["pub static PROG_PAGES: &[ProgPage] = &["]
    for page_title, sec_arr in PROG_PAGES:
        lines.append(f'    ProgPage {{ title: "{esc(page_title)}", sections: &[')
        for sec_title, arr_name in sections_map[sec_arr]:
            lines.append(f'        ProgSection {{ title: "{esc(sec_title)}", entries: &[')
            for e in arrays_map[arr_name]:
                keys = ", ".join(iid_ref(k) for k in e["keys"])
                lines.append(
                    f'            ProgEntry {{ icon: "{esc(e["icon"])}", '
                    f'name: "{esc(e["name"])}", lookup_keys: &[{keys}], '
                    f'is_counter: {str(e["counter"]).lower()}, '
                    f'max_count: {e["max"]}, '
                    f'max_from_spoiler: {str(e["max_spoiler"]).lower()} }},')
            lines.append("        ] },")
        lines.append("    ] },")
    lines.append("];\n")
    return "\n".join(lines)


def emit_settings(name, entries, setting_opts):
    """`setting_opts` is the OoTMM snapshot (tools/ootmm_settings.json): each setting's
    real per-parameter option list. We stamp it onto SettingMeta so the ROM-settings
    editor offers exactly the choices OoTMM's generator does for that key."""
    lines = [f"pub static {name}: &[SettingMeta] = &["]
    for key, disp, ptype, pcat, shuf, ids in entries:
        cells = ", ".join(f"{i:#x}" for i in ids)
        vals = setting_opts.get(key, {}).get("values", [])
        if vals:
            opts = "&[" + ", ".join(
                f'SettingOption {{ value: "{esc(v)}", label: "{esc(lbl)}" }}'
                for v, lbl in vals) + "]"
        else:
            opts = "&[]"
        lines.append(
            f'    SettingMeta {{ key: "{esc(key)}", name: "{esc(disp)}", '
            f"type_: ParamType::{rust_ident(ptype)}, "
            f"cat: ParamCategory::{rust_ident(pcat)}, "
            f"default: ShuffleSetting::{rust_ident(shuf)}, affected: &[{cells}], "
            f"options: {opts} }},")
    lines.append("];\n")
    return "\n".join(lines)


# Access / win-condition settings the reachability logic reads but the tracker's
# FilterSettings / ItemSettings do not expose. Each key is a real logic setting
# (present in SETTING_KEYS) whose value only affects reachability (open dungeons,
# door of time, bridge / moon / LACS conditions, …). Curated, ordered for the UI;
# type / default / option list come from the OoTMM snapshot. Set-valued keys
# (openDungeons*, ganonTrials) list their members and drive `setting(k, member)`.
ACCESS_KEYS = [
    ("doorOfTime", "Door of Time"),
    ("beneathWell", "Beneath the Well"),
    ("gerudoFortress", "Gerudo Fortress"),
    ("zoraKing", "Zora King"),
    ("openZdShortcut", "Open Zora's Domain Shortcut"),
    ("openMaskShop", "Open Mask Shop"),
    ("crossAge", "Cross-Age Items"),
    ("freeScarecrowOot", "Free Scarecrow (OoT)"),
    ("freeScarecrowMm", "Free Scarecrow (MM)"),
    ("openMoon", "Open Moon"),
    ("moonCrash", "Moon Crash"),
    ("majoraChild", "Majora as Child"),
    ("bossWarpPads", "Boss Warp Pads"),
    ("openDungeonsOot", "Open Dungeons (OoT)"),
    ("openDungeonsMm", "Open Dungeons (MM)"),
    ("clearStateDungeonsMm", "Clear State Dungeons (MM)"),
    ("rainbowBridge", "Rainbow Bridge"),
    ("moon", "Moon Access"),
    ("lacs", "Light Arrow Cutscene"),
    ("ganonTrials", "Ganon's Trials"),
]
# `ganonBossKey` and `skipZelda` are also FilterSettings (they gate map visibility), so
# they stay on their filter pages; the filter edit is synced into raw_settings there so
# they drive the logic too. Do not duplicate them here.


def emit_access_settings(setting_opts):
    """Emit ACCESS_SETTINGS from the OoTMM snapshot: kind (enum / bool / set), default
    value, and member/option list per curated key (see ACCESS_KEYS)."""
    kinds = {"enum": "Enum", "boolean": "Bool", "set": "Set"}
    lines = ["pub static ACCESS_SETTINGS: &[AccessSetting] = &["]
    for key, disp in ACCESS_KEYS:
        s = setting_opts.get(key, {})
        typ = s.get("type", "boolean")
        kind = kinds.get(typ, "Bool")
        default = s.get("default")
        if default is None:
            default = "false" if typ == "boolean" else ""
        vals = s.get("values", [])
        if vals:
            opts = "&[" + ", ".join(
                f'SettingOption {{ value: "{esc(v)}", label: "{esc(lbl)}" }}'
                for v, lbl in vals) + "]"
        else:
            opts = "&[]"
        lines.append(
            f'    AccessSetting {{ key: "{esc(key)}", name: "{esc(disp)}", '
            f"kind: AccessKind::{kind}, default: \"{esc(default)}\", options: {opts} }},")
    lines.append("];\n")
    return "\n".join(lines)


def emit_entrances(name, entrs):
    out = [f"pub static {name}: &[EntranceDef] = &["]
    for e in entrs:
        out.append(
            "    EntranceDef {{ to_id: {ti}, from_id: {fi}, from_scene: scenes::{fs}, "
            "to_scene: scenes::{ts}, from_name: \"{fn}\", to_name: \"{tn}\", "
            "type_: EntranceType::{ty}, anchor: [{ax}, {ay}], text: [{tx}, {tyy}], "
            "icon: \"{ic}\", layout: GameLayout::{lay} }},".format(
                ti=entr_ref(e["to_id"]), fi=entr_ref(e["from_id"]),
                fs=e["from_scene"], ts=e["to_scene"],
                fn=esc(e["from_name"]), tn=esc(e["to_name"]), ty=rust_ident(e["type"]),
                ax=e["ax"], ay=e["ay"], tx=e["tx"], tyy=e["ty"],
                ic=esc(e["icon"]), lay=rust_ident(e["layout"])))
    out.append("];\n")
    return "\n".join(out)


def emit_objects(name, objs):
    out = [f"pub static {name}: &[ObjectDef] = &["]
    for o in objs:
        out.append(
            "    ObjectDef {{ object_id: {oid}, scene: scenes::{sc}, name: \"{nm}\", "
            "location: \"{loc}\", type_: ObjectType::{ty}, x: {x}, y: {y}, z: {z}, "
            "render_scene: scenes::{rs}, render_type: ObjectType::{rt}, map_icon: \"{mi}\", "
            "context: ObjectContext::{cx}, room: {rm}, layout: GameLayout::{lay}, "
            "loc_type: LocType::{lt}, xflag_id: {xf}, system: ObjSystem::{sys} }},".format(
                oid=o["id_ref"], sc=o["scene"], nm=esc(o["name"]), loc=esc(o["location"]),
                ty=rust_ident(o["type"]), x=o["x"], y=o["y"], z=o["z"], rs=o["render"],
                rt=rust_ident(o["render_type"]), mi=esc(o["map_icon"]),
                cx=rust_ident(o["context"]), rm=o["room"],
                lay=rust_ident(o["layout"]), lt=rust_ident(o["loc_type"]),
                xf=f'0x{o["xflag_id"]:04X}', sys=o["system"]))
    out.append("];\n")
    return "\n".join(out)


# --- split the generated source into a src/data/ folder module ---------------
_CORE = {'ObjectType', 'ObjectContext', 'ObjSystem', 'GameLayout', 'LocType', 'EntranceType',
         'ParamType', 'ParamCategory', 'ShuffleSetting', 'SceneId', 'scenes',
         'SceneDef', 'ObjectDef', 'LegacySceneRemap', 'RoomDef', 'GrottoPos', 'EntranceDef',
         'SettingMeta', 'SettingOption', 'AccessSetting', 'AccessKind',
         'ItemDef', 'ProgEntry', 'ProgSection', 'ProgPage'}
_CONSTS = {'ids', 'iid', 'entr', 'song_oot', 'song_mm', 'owl'}
_TARGET = {'PROGRESSIVE_FAMILIES': 'prog', 'PROG_PAGES': 'prog',
           'OOT_OBJECTS': 'oot_items', 'MM_OBJECTS': 'mm_items',
           'OOT_LEGACY_SCENE_REMAP': 'oot_items', 'MM_LEGACY_SCENE_REMAP': 'mm_items',
           'OOT_SCENES': 'oot_world', 'OOT_ROOMS': 'oot_world',
           'OOT_ENTRANCES': 'oot_world', 'OOT_SCENE_ENTRANCES': 'oot_world',
           'OOT_ENTRANCE_COSTS': 'oot_world',
           'MM_SCENES': 'mm_world', 'MM_ROOMS': 'mm_world',
           'MM_ENTRANCES': 'mm_world', 'MM_SCENE_ENTRANCES': 'mm_world',
           'MM_ENTRANCE_COSTS': 'mm_world'}
_SUB = [('consts', 'id / entrance / song constants'),
        ('misc', 'items, icons, settings, grottos - misc tables'),
        ('prog', 'progression dashboard'),
        ('oot_items', 'OoT object/check pool'),
        ('oot_world', 'OoT scenes / regions / entrances'),
        ('mm_items', 'MM object/check pool'),
        ('mm_world', 'MM scenes / regions / entrances')]
_ALLOW = '#![allow(dead_code, non_camel_case_types, non_upper_case_globals)]'


def write_split(full_text, outdir, extra_mods=()):
    """Segment the generated source into top-level blocks and route each to its
    category file (mirrors the tree layout: mod.rs + per-category submodules).

    `extra_mods` are submodules written by another generator (e.g. `logic` from
    gen_logic.py): they are declared in mod.rs but their .rs file is left alone."""
    lines = full_text.split("\n")
    blocks, lead, i, n = [], [], 0, len(lines)
    while i < n:
        s = lines[i]
        if re.match(r'(pub (?:enum|mod|struct|static|type|const|fn)\b|#!\[)', s):
            m = re.match(r'pub (?:enum|mod|struct|static|type|const|fn)\s+(\w+)', s)
            name = m.group(1) if m else None
            kind = s.split()[1] if s.startswith('pub ') else 'attr'
            st = s.rstrip()
            if kind == 'attr' or st.endswith(';') or st.endswith('}'):
                end = i
            else:
                j = i + 1
                while j < n and lines[j].rstrip() not in ('}', '];'):
                    j += 1
                end = j
            blocks.append((name, kind, lead + lines[i:end + 1]))
            lead, i = [], end + 1
        else:
            lead.append(s)
            i += 1

    buckets = {k: [] for k, _ in _SUB}
    core, header = [], None
    for name, kind, blk in blocks:
        if kind == 'attr':
            header = blk
        elif kind in ('enum', 'struct', 'type') or name in _CORE:
            core.append(blk)
        elif kind == 'mod' and name in _CONSTS:
            buckets['consts'].append(blk)
        elif name in _TARGET:
            buckets[_TARGET[name]].append(blk)
        else:
            buckets['misc'].append(blk)

    os.makedirs(outdir, exist_ok=True)

    def flat(chunks):
        out = []
        for c in chunks:
            out += c + ['']
        return out

    for key, cat in _SUB:
        head = [f'//! GENERATED by tools/gen_data.py - DO NOT EDIT BY HAND. ({cat})', _ALLOW]
        if key != 'consts':                       # consts needs nothing from super
            head.append('use super::*;')
        head.append('')
        Path(outdir, f'{key}.rs').write_text("\n".join(head + flat(buckets[key])), encoding="utf-8")

    mod = list(header or [])                        # //! header + #![allow(...)]
    mod.append('')
    for key, _ in _SUB:
        mod += [f'mod {key};', f'pub use {key}::*;']
    # Submodules generated separately (their .rs is written by another script).
    # `allow(unused_imports)` keeps a clean build until the runtime consumes them.
    for key in extra_mods:
        mod += [f'mod {key};', '#[allow(unused_imports)]', f'pub use {key}::*;']
    mod.append('')
    Path(outdir, 'mod.rs').write_text("\n".join(mod + flat(core)), encoding="utf-8")


def main():
    scene_sym = parse_defines(read("Headers/Combo/Scenes.h"))
    id_sym = parse_defines(read("Headers/Combo/Items.h"))
    id_sym = parse_defines(read("Headers/Combo/NPC.h"), seed=id_sym)

    # Entrance id constants: generated OoT/MM entrances + the special ones.
    ent_sym = parse_defines(read("Headers/Combo/OoTEntrances.h"))
    ent_sym = parse_defines(read("Headers/Combo/MMEntrances.h"), seed=ent_sym)
    ent_sym = parse_defines(read("Headers/Combo/Entrances.h"), seed=ent_sym)

    objects_h = read("Headers/Combo/Objects.h")
    obj_types = [v for v in parse_enum(objects_h, "ObjectType") if v != "last"]
    obj_ctx = parse_enum(objects_h, "ObjectContext")
    layouts = parse_enum(read("Headers/Multi/Game.h"), "GameLayout")
    loc_types = parse_enum(read("Headers/Combo/Scenes.h"), "LocType")
    ent_types = parse_enum(read("Headers/Combo/Entrances.h"), "EntranceType")
    settings_h = read("Headers/UI/Settings.h")
    param_types = parse_enum(settings_h, "ParamType")
    param_cats = parse_enum(settings_h, "ParamCategory")
    shuffle_settings = parse_enum(settings_h, "ShuffleSetting")
    entrances_h = read("Headers/Combo/Entrances.h")
    song_oot = parse_enum_values(entrances_h, "OoTSongs")
    song_mm = parse_enum_values(entrances_h, "MMSongs")
    owl_vals = parse_enum_values(entrances_h, "OwlScene")
    regions = parse_regions(read("Headers/Combo/Regions.h"))

    missing, missing_ids, missing_ent, used_ids = set(), set(), set(), set()
    oot_objs = objects_from_rows("OoT", scene_sym, id_sym, used_ids, missing, missing_ids)
    mm_objs = objects_from_rows("MM", scene_sym, id_sym, used_ids, missing, missing_ids)
    oot_scenes = load_scenes("Resources/Scenes/scenes_oot.csv",
                             regions["OoTRegions"], scene_sym, missing)
    mm_scenes = load_scenes("Resources/Scenes/scenes_mm.csv",
                            regions["MMRegions"], scene_sym, missing)
    oot_rooms, mm_rooms = parse_rooms(read("Headers/UI/RoomRenderer.h"), scene_sym)
    oot_entr = load_entrances("Resources/Scenes/entrances_oot.csv", scene_sym, missing, missing_ent)
    mm_entr = load_entrances("Resources/Scenes/entrances_mm.csv", scene_sym, missing, missing_ent)
    icons_cpp = read("Sources/UI/Icons.cpp")
    icons = parse_icons(icons_cpp, len(obj_types))
    icon_sizes = parse_icon_sizes(icons_cpp, len(obj_types))
    icons_h = read("Headers/UI/Icons.h")
    icon_by_name = parse_icon_by_name(icons_h, icons_cpp)
    entrance_icons = parse_entrance_icons(icons_h, icons_cpp)
    type_names = parse_obj_type_names(objects_h, len(obj_types))
    oot_filter_types = parse_type_set(objects_h, "OoTTypes")
    mm_filter_types = parse_type_set(objects_h, "MMTypes")
    entrances_cpp = read("Sources/Combo/Entrances.cpp")
    grottos = parse_grottos(entrances_cpp, ent_sym, missing_ent)
    spawn_oot, spawn_mm = parse_check_grotto_spawn(entrances_cpp, ent_sym, missing_ent)
    oot_scene_entr = parse_scene_entrances(
        read("Sources/UI/SceneOoTEntrances.cpp"), scene_sym, ent_sym, missing, missing_ent,
        regions["OoTRegions"])
    mm_scene_entr = parse_scene_entrances(
        read("Sources/UI/SceneMMEntrances.cpp"), scene_sym, ent_sym, missing, missing_ent,
        regions["MMRegions"])
    settings_cpp = read("Sources/UI/Settings.cpp")
    filter_settings = parse_settings_map(
        settings_cpp, r"this->FilterSettings", id_sym, missing_ids)
    item_settings = parse_settings_map(
        settings_cpp, r"this->ItemSettings", id_sym, missing_ids)
    # OoTMM's real per-parameter option lists (tools/ootmm_settings.json snapshot).
    setting_opts = json.load(open(
        os.path.join(os.path.dirname(os.path.abspath(__file__)), "ootmm_settings.json"),
        encoding="utf-8"))
    items_cpp = read("Sources/Combo/Items.cpp")
    items = parse_items(items_cpp, id_sym, missing_ids)
    prog_families = parse_progressive_families(items_cpp, id_sym, missing_ids)
    prog_cpp = read("Sources/UI/ProgressionEntry.cpp")
    prog_arrays = parse_prog_arrays(prog_cpp, id_sym, missing_ids)
    prog_sections = parse_prog_sections(prog_cpp)

    scene_consts = sorted(scene_sym.items(), key=lambda kv: (kv[1], kv[0]))
    id_consts = sorted(((n, id_sym[n]) for n in used_ids), key=lambda kv: (kv[1], kv[0]))
    all_id_consts = sorted(id_sym.items(), key=lambda kv: (kv[1], kv[0]))
    ent_consts = sorted(ent_sym.items(), key=lambda kv: (kv[1], kv[0]))

    # Reverse maps for readable id references (iid::NAME / entr::NAME) in the
    # emitted arrays. Items.h only (avoids NPC/song aliases sharing a value);
    # first-seen wins, matching file order.
    for nm, val in parse_defines(read("Headers/Combo/Items.h")).items():
        ITEM_REV.setdefault(val, nm)
    for nm, val in ent_sym.items():
        ENT_REV.setdefault(val, nm)

    parts = [
        "//! GENERATED by tools/gen_data.py - DO NOT EDIT BY HAND.\n"
        "//! Source of truth: Resources/Objects/pool_*.csv + Resources/Scenes/scenes_*.csv\n"
        "//! Symbol values: Headers/Combo/{Scenes,NPC,Items,Objects}.h, Headers/Multi/Game.h\n"
        f"//! Scenes OoT={len(oot_scenes)} MM={len(mm_scenes)} | "
        f"Objects OoT={len(oot_objs)} MM={len(mm_objs)}\n\n"
        "#![allow(dead_code, non_camel_case_types, non_upper_case_globals)]\n\n"
        "/// Scene identifier (matches the width of the C++ scene ids).\n"
        "pub type SceneId = u16;\n\n",
        emit_enum("ObjectType", obj_types, "ObjectType (Headers/Combo/Objects.h)", repr_u8=True),
        emit_enum("ObjectContext", obj_ctx, "ObjectContext (Headers/Combo/Objects.h)"),
        emit_enum("ObjSystem", ["Any", "Legacy", "New"],
                  "ObjSystem (Headers/Combo/Objects.h)", repr_u8=True),
        emit_enum("GameLayout", layouts, "GameLayout (Headers/Multi/Game.h)"),
        emit_enum("LocType", loc_types, "LocType (Headers/Combo/Scenes.h)"),
        emit_enum("EntranceType", ent_types, "EntranceType (Headers/Combo/Entrances.h)"),
        emit_enum("ParamType", param_types, "ParamType (Headers/UI/Settings.h)"),
        emit_enum("ParamCategory", param_cats, "ParamCategory (Headers/UI/Settings.h)"),
        emit_enum("ShuffleSetting", shuffle_settings, "ShuffleSetting (Headers/UI/Settings.h)"),
        "/// Scene id constants (mirror of the #defines in Scenes.h).\n",
        emit_consts("scenes", scene_consts, "SceneId", use_scene_id=True),
        "\n/// Named object-id constants used by the pools (from NPC.h / Items.h).\n",
        emit_consts("ids", id_consts, "u32"),
        "\n/// Every item / NPC id constant (Items.h + NPC.h), for the ROM settings.\n",
        emit_consts("iid", all_id_consts, "u32"),
        "\n/// Entrance id constants (OoT/MM entrance headers).\n",
        emit_consts("entr", ent_consts, "u32"),
        "\n/// Entrance const-name -> id, sorted by name for the spoiler `Entrances`\n"
        "/// join (progressive discovery). Some names carry a trailing `_ENTR`.\n",
        emit_name_id_table("ENTRANCE_ID_BY_NAME", ent_sym.items()),
        "\n/// Song / owl id values (Entrances.h enums).\n",
        emit_consts("song_oot", song_oot, "u8"),
        emit_consts("song_mm", song_mm, "u8"),
        emit_consts("owl", owl_vals, "u8"),
        "\npub struct SceneDef {\n"
        "    pub id: SceneId,\n    pub name: &'static str,\n"
        "    /// Artistic map (items). May be empty (dungeons use rooms/minimap).\n"
        "    pub image_rel: &'static str,\n"
        "    /// Minimap (entrances).\n    pub minimap_rel: &'static str,\n"
        "    pub region_id: u8,\n    pub region_name: &'static str,\n"
        "    pub has_context: bool,\n    pub layout: GameLayout,\n"
        "    /// Alternate map shown when the context toggle is ON (Spring / Adult);\n"
        "    /// empty => image_rel is used for both contexts.\n"
        "    pub context_image_rel: &'static str,\n}\n\n"
        "pub struct ObjectDef {\n"
        "    pub object_id: u32,\n    pub scene: SceneId,\n    pub name: &'static str,\n"
        "    pub location: &'static str,\n    pub type_: ObjectType,\n"
        "    pub x: i32,\n    pub y: i32,\n    pub z: i32,\n    pub render_scene: SceneId,\n"
        "    pub render_type: ObjectType,\n    pub map_icon: &'static str,\n"
        "    pub context: ObjectContext,\n    pub room: u16,\n    pub layout: GameLayout,\n"
        "    pub loc_type: LocType,\n"
        "    /// Compact XflagID (new xflag ROMs > v32.3) stamped by Location; 0xFFFF = none.\n"
        "    pub xflag_id: u16,\n"
        "    /// Which ROM xflag system this object exists under (legacy / new / both). A few\n"
        "    /// checks changed representation across OoTMM versions; gated by uses_legacy at\n"
        "    /// display / resolution time (object_active).\n"
        "    pub system: ObjSystem,\n}\n\n"
        "/// A check whose true scene differs from the one pre-migration (<= v32.3) ROMs report.\n"
        "/// Keyed by the reported (legacy) scene + the check's unchanged legacy ObjectID.\n"
        "pub struct LegacySceneRemap {\n"
        "    pub legacy_scene: SceneId,\n    pub object_id: u32,\n"
        "    pub type_: ObjectType,\n    pub true_scene: SceneId,\n}\n\n"
        "pub struct RoomDef {\n"
        "    pub id: u32,\n    pub name: &'static str,\n    pub image_rel: &'static str,\n}\n\n"
        "pub struct GrottoPos {\n    pub id: u32,\n    pub pos: [f32; 3],\n}\n\n"
        "pub struct EntranceDef {\n"
        "    pub to_id: u32,\n    pub from_id: u32,\n    pub from_scene: SceneId,\n"
        "    pub to_scene: SceneId,\n    pub from_name: &'static str,\n"
        "    pub to_name: &'static str,\n    pub type_: EntranceType,\n"
        "    pub anchor: [i32; 2],\n    pub text: [i32; 2],\n    pub icon: &'static str,\n"
        "    pub layout: GameLayout,\n}\n\n"
        "/// One selectable value of a setting: the raw OoTMM value plus its display label\n"
        "/// (from OoTMM's settings/data.ts, snapshotted in tools/ootmm_settings.json).\n"
        "pub struct SettingOption {\n"
        "    pub value: &'static str,\n    pub label: &'static str,\n}\n\n"
        "/// A ROM build parameter (mirror of Settings.cpp Parameter, minus the\n"
        "/// runtime shuffle value which lives in the Settings struct). `options` is the\n"
        "/// real per-parameter choice list OoTMM's generator offers (empty = fall back to\n"
        "/// the type's generic list).\n"
        "pub struct SettingMeta {\n"
        "    pub key: &'static str,\n    pub name: &'static str,\n"
        "    pub type_: ParamType,\n    pub cat: ParamCategory,\n"
        "    pub default: ShuffleSetting,\n    pub affected: &'static [u32],\n"
        "    pub options: &'static [SettingOption],\n}\n\n"
        "/// The shape of an access / win-condition setting's value (see ACCESS_SETTINGS).\n"
        "#[derive(Clone, Copy, PartialEq, Eq)]\n"
        "pub enum AccessKind {\n    /// One value out of `options`.\n    Enum,\n"
        "    /// On / off (`true` / `false`).\n    Bool,\n"
        "    /// Any subset of `options` (members joined into the raw value).\n    Set,\n}\n\n"
        "/// A logic-only ROM setting (open dungeons, door of time, bridge / moon / LACS\n"
        "/// conditions, …) the reachability solver reads from the spoiler's raw settings\n"
        "/// but the tracker's filter / item settings do not expose. Edited straight into\n"
        "/// `Settings::raw_settings` (the raw OoTMM value string), so the solver picks it\n"
        "/// up with or without a spoiler. `default` is the OoTMM default value string.\n"
        "pub struct AccessSetting {\n"
        "    pub key: &'static str,\n    pub name: &'static str,\n"
        "    pub kind: AccessKind,\n    pub default: &'static str,\n"
        "    pub options: &'static [SettingOption],\n}\n\n"
        "/// A tracked item (mirror of Items.cpp ItemInfo, minus the render icon).\n"
        "pub struct ItemDef {\n"
        "    pub id: u32,\n    pub name: &'static str,\n    pub can_be_shared: bool,\n}\n\n"
        "/// One progression-dashboard entry (mirror of ProgressionEntry.h ProgEntry).\n"
        "/// `icon` is the EGameIcon variant name (resolve the image via ICON_BY_NAME).\n"
        "pub struct ProgEntry {\n"
        "    pub icon: &'static str,\n    pub name: &'static str,\n"
        "    pub lookup_keys: &'static [u32],\n    pub is_counter: bool,\n"
        "    pub max_count: i32,\n    pub max_from_spoiler: bool,\n}\n\n"
        "pub struct ProgSection {\n"
        "    pub title: &'static str,\n    pub entries: &'static [ProgEntry],\n}\n\n"
        "pub struct ProgPage {\n"
        "    pub title: &'static str,\n    pub sections: &'static [ProgSection],\n}\n\n",
        "\n" + emit_icons(icons),
        "\n" + emit_icon_sizes(icon_sizes),
        "\n" + emit_icon_by_name(icon_by_name),
        "\n" + emit_entrance_icons(entrance_icons),
        "\n" + emit_obj_type_names(type_names),
        "\n/// Region icon path per region id (Regions.h RegionsMetaInfo).\n"
        + emit_region_icons("OOT_REGION_ICONS", regions["OoTRegions"])
        + emit_region_icons("MM_REGION_ICONS", regions["MMRegions"]),
        "\n/// Region display name per region id (Regions.h RegionsMetaInfo).\n"
        + emit_region_names("OOT_REGION_NAMES", regions["OoTRegions"])
        + emit_region_names("MM_REGION_NAMES", regions["MMRegions"]),
        "\n/// Measured intra-scene walk times (scene, from-entrance, to-entrance,\n"
        "/// cost seconds), imported from OoT/MMEntranceCosts.cpp. Feeds the GPS.\n"
        + emit_entrance_costs("OOT_ENTRANCE_COSTS",
                              parse_entrance_costs(read("Sources/Combo/OoTEntranceCosts.cpp"), "OoTMeasuredCosts"))
        + emit_entrance_costs("MM_ENTRANCE_COSTS",
                              parse_entrance_costs(read("Sources/Combo/MMEntranceCosts.cpp"), "MMMeasuredCosts")),
        "\n/// Filterable object types per game (Objects.h OoTTypes / MMTypes).\n"
        + emit_type_set("OOT_FILTER_TYPES", oot_filter_types)
        + emit_type_set("MM_FILTER_TYPES", mm_filter_types),
        "\n/// ROM build parameters (Settings.cpp FilterSettings / ItemSettings).\n"
        + emit_settings("FILTER_SETTINGS", filter_settings, setting_opts)
        + emit_settings("ITEM_SETTINGS", item_settings, setting_opts),
        "\n/// Logic-only access / win-condition settings (open dungeons, door of time,\n"
        "/// bridge / moon / LACS), edited into Settings::raw_settings for the solver.\n"
        + emit_access_settings(setting_opts),
        "\n/// Item table + name lookup (Items.cpp ItemList / FindItemByName).\n"
        + emit_items(items),
        "\n" + emit_progressive_families(prog_families),
        "\n/// Progression dashboard pages (ProgressionEntry.cpp).\n"
        + emit_prog(prog_sections, prog_arrays),
        "\n" + emit_scenes("OOT_SCENES", oot_scenes),
        "\n" + emit_scenes("MM_SCENES", mm_scenes),
        "\n" + emit_rooms("OOT_ROOMS", oot_rooms),
        "\n" + emit_rooms("MM_ROOMS", mm_rooms),
        "\n" + emit_entrances("OOT_ENTRANCES", oot_entr),
        "\n" + emit_entrances("MM_ENTRANCES", mm_entr),
        "\n/// Grotto entrance spawn positions (Entrances.cpp GrottoEntrances).\n"
        + emit_grottos(grottos),
        "\n/// CheckGrottoSpawn: entrance id -> canonical spawn (sorted, per game).\n"
        + emit_pairs("CHECK_GROTTO_SPAWN_OOT", spawn_oot)
        + emit_pairs("CHECK_GROTTO_SPAWN_MM", spawn_mm),
        "\n/// Per-scene entrance ids (SceneOoT/MMEntrances.cpp SceneEntranceMeta).\n"
        + emit_scene_entrances("OOT_SCENE_ENTRANCES", oot_scene_entr)
        + emit_scene_entrances("MM_SCENE_ENTRANCES", mm_scene_entr),
        "\n" + emit_objects("OOT_OBJECTS", oot_objs),
        "\n" + emit_objects("MM_OBJECTS", mm_objs),
        "\n" + emit_legacy_remap("OOT_LEGACY_SCENE_REMAP", "OoT", id_sym, used_ids, missing_ids),
        "\n" + emit_legacy_remap("MM_LEGACY_SCENE_REMAP", "MM", id_sym, used_ids, missing_ids),
    ]
    write_split("".join(parts), OUTDIR, extra_mods=["logic"])

    # Compile the OoTMM logic graph (root Logic/ folder) into src/data/logic.rs.
    # Deferred import to avoid a circular import (gen_logic imports from us).
    import gen_logic
    gen_logic.generate(OUTDIR / "logic.rs", id_sym=id_sym)

    print(f"OK -> {OUTDIR}/ (mod + {len(_SUB)} submodules + logic)")
    print(f"  scenes  : OoT={len(oot_scenes)}  MM={len(mm_scenes)}")
    print(f"  objects : OoT={len(oot_objs)}  MM={len(mm_objs)}")
    print(f"  named ids used: {len(used_ids)}  | scene consts: {len(scene_consts)}")
    print(f"  rooms   : OoT scenes={len(oot_rooms)}  MM scenes={len(mm_rooms)}")
    print(f"  entrances: OoT={len(oot_entr)}  MM={len(mm_entr)}")
    print(f"  settings : filter={len(filter_settings)}  item={len(item_settings)}")
    n_sec = sum(len(prog_sections[s]) for _, s in PROG_PAGES)
    n_prog = sum(len(prog_arrays[arr])
                 for _, s in PROG_PAGES for _, arr in prog_sections[s])
    print(f"  items    : {len(items)}  | progressive families: {len(prog_families)}")
    print(f"  prog     : {n_sec} sections, {n_prog} entries")
    print(f"  scene-entrances: OoT scenes={len(oot_scene_entr)} "
          f"({sum(len(e) for _, _, e in oot_scene_entr)} entr)  "
          f"MM scenes={len(mm_scene_entr)} "
          f"({sum(len(e) for _, _, e in mm_scene_entr)} entr)")
    if missing_ent:
        print(f"  WARN {len(missing_ent)} entrance ids unresolved: "
              f"{sorted(missing_ent)[:12]}", file=sys.stderr)
    if missing:
        print(f"  WARN unresolved scenes: {sorted(missing)[:20]}", file=sys.stderr)
    if missing_ids:
        print(f"  WARN unresolved object-ids: {sorted(missing_ids)[:20]}", file=sys.stderr)


if __name__ == "__main__":
    main()
