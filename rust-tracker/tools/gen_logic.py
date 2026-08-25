#!/usr/bin/env python3
"""Compile the OoTMM logic (root `Logic/` folder) into `src/data/logic.rs`.

M1 scope: parse the YAML region files + the macro files, fully inline the macros
(with parameter substitution), compile every access expression to a small
stack (RPN) bytecode, intern the symbols (events / tricks / settings / setting
values / regions / locations) and emit static Rust tables. No evaluator yet —
that is M2 (src/logic/). This module only produces the DATA the solver will run.

Source of truth (maintained upstream by OoTMM Combo):
  Logic/macros_common.yml, macros_oot.yml, macros_mm.yml   -> macros
  Logic/oot/**/*.yml, Logic/mm/**/*.yml                     -> region graph

The join with the tracker: a logic location key "Deku Tree Map Chest" (in an oot
file) maps to the pool/spoiler location "OOT Deku Tree Map Chest" — i.e. the game
prefix ("OOT "/"MM ") + the yml key. We emit the ALREADY-PREFIXED string so the
runtime can match it against `ObjectDef.location` with no transformation.

Stdlib only (PyYAML is not assumed — the region/macro YAML is regular enough to
parse directly). Invoked by gen_data.py after it writes the data folder, or run
standalone:  python tools/gen_logic.py
"""
import csv
import os
import re
import sys
from pathlib import Path

# Reuse gen_data's header helpers (side-effect free import: its work is guarded
# behind `if __name__ == "__main__"`).
from gen_data import parse_defines, read, resolve, ROOT

LOGIC_DIR = ROOT / "Logic"
OUT_DEFAULT = Path(__file__).resolve().parents[1] / "src" / "data" / "logic.rs"

# Trick id <-> display-name table, vendored from OoTMM
# `packages/core/src/settings/tricks.ts` (the spoiler lists tricks by display
# name, the logic references them by id). Not part of the `Logic/` drop, so it is
# a separate file to refresh when OoTMM adds tricks.
TRICKS_TSV = Path(__file__).resolve().parent / "tricks.tsv"

# The C++ headers (item / NPC symbols) moved under C++-Tracker/ when the Qt
# tracker was relocated; fall back to the repo root for older checkouts.
CPP_ROOT = ROOT / "C++-Tracker" if (ROOT / "C++-Tracker" / "Headers").is_dir() else ROOT

OOT, MM = 0, 1
GAME_PREFIX = {OOT: "OOT ", MM: "MM "}
ITEM_PREFIX = {OOT: "OOT_", MM: "MM_"}

# Native primitives (called but never defined as a macro). Each compiles to a
# dedicated op. Any *other* native still becomes a generic `Builtin` leaf,
# recorded in `builtins_seen` (should be empty now — a non-empty set flags a new
# upstream primitive to model).
#
# CORE: modelled precisely by the M2 evaluator.
CORE_NATIVES = {
    "has", "event", "trick", "setting", "cond", "age", "oot_time", "mm_time",
    "renewable", "license", "masks",
    # Win-condition gates (BRIDGE / GANON_BK / LACS / MAJORA / MOON) and the MM
    # region-state flags (temple cleared / cursed): settings- / progress-driven,
    # kept as real ops so the solver can evaluate them.
    "special", "flag_on", "flag_off",
    # song_event(x) resolution: `_song_event_<game>(slot, songIndex)` is true iff
    # the seed placed that song at that event slot. The spoiler's `Song Events`
    # section gives the slot->song map, so this is a real, precise op (the
    # `&& can_play_<song>` half of the macro compiles to ordinary item leaves).
    "_song_event_oot", "_song_event_mm",
}
# OPTIMISTIC: folded to `true` at compile time. These gate on data the tracker
# has no source for (randomised shop prices) or on skill (fishing weight), so a
# "show only reachable" tracker must not hide for them — hiding a reachable check
# is worse UX than showing an unreachable one.
#   price(range, id, max)        -> shops/scrubs always affordable
#   has_pond_fish(kind, lo, hi)  -> catchable if you can fish
OPTIMISTIC_NATIVES = {"price", "has_pond_fish"}
NATIVES = CORE_NATIVES | OPTIMISTIC_NATIVES


# ── YAML-ish loaders ─────────────────────────────────────────────────────────
def _strip_value(val):
    """A scalar value: unquote a `"..."` (ignoring a trailing comment), else take
    the bare token up to a ` #` comment."""
    val = val.strip()
    if val.startswith('"'):
        end = val.index('"', 1)
        return val[1:end]
    return val.split("#", 1)[0].strip()


def _split_kv(content):
    """`key: value` where key may be a `"quoted string"` or a bare identifier and
    value may be quoted / bare. Returns (key, value)."""
    content = content.strip()
    if content.startswith('"'):
        end = content.index('"', 1)
        key = content[1:end]
        rest = content[end + 1:].lstrip()
        assert rest.startswith(":"), content
        val = rest[1:]
    else:
        idx = content.index(":")
        key, val = content[:idx], content[idx + 1:]
    return key.strip(), _strip_value(val)


def load_macros(path):
    """`macros_*.yml` -> {name: (params, expr_str)}. Keys look like `name` or
    `name(a, b, c)`; values are quoted expression strings."""
    out = {}
    for raw in read_text(path).splitlines():
        line = raw.rstrip()
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        key, val = _split_kv(line)
        m = re.match(r"^([A-Za-z_]\w*)\s*(?:\(([^)]*)\))?$", key)
        if not m:
            raise ValueError(f"bad macro key in {path}: {key!r}")
        name = m.group(1)
        params = [p.strip() for p in (m.group(2) or "").split(",") if p.strip()]
        out[name] = (params, val)
    return out


SUBSECTIONS = ("exits", "events", "locations")


def _region_header(content):
    """The region name on a `"Name": [# comment]` (or bare `Name:`) header line,
    tolerating an inline comment after the colon."""
    content = content.strip()
    if content.startswith('"'):
        return content[1:content.index('"', 1)]
    return content.split(":", 1)[0].strip()


def load_regions(path, game):
    """One region YAML file -> [ (region_name, attrs, {sub: {name: expr}}) ]."""
    regions = []
    cur = None
    cur_sub = None
    for raw in read_text(path).splitlines():
        line = raw.rstrip()
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        indent = len(line) - len(line.lstrip(" "))
        content = line.strip()
        if indent == 0:
            cur = {"name": _region_header(content), "attrs": {},
                   "exits": {}, "events": {}, "locations": {}}
            regions.append(cur)
            cur_sub = None
        elif indent <= 2:
            # `key:` (maybe with a trailing comment) with no value is a subsection
            # header; `key: value` is a scalar attribute (dungeon / region / ...).
            key, _, rest = content.partition(":")
            key = key.strip()
            if key in SUBSECTIONS and not rest.split("#", 1)[0].strip():
                cur_sub = key
            else:
                k, v = _split_kv(content)
                cur["attrs"][k] = v
                cur_sub = None
        else:  # entry inside a subsection
            if cur is None or cur_sub is None:
                continue
            k, v = _split_kv(content)
            cur[cur_sub][k] = v
    return [(r["name"], r["attrs"], {"exits": r["exits"], "events": r["events"],
                                     "locations": r["locations"]}) for r in regions]


def read_text(path):
    return Path(path).read_text(encoding="utf-8", errors="replace")


def load_trick_names(path=TRICKS_TSV):
    """`id<ws>display name` per line -> {id: display_name}. Split on the first run
    of whitespace (ids are single tokens, names may contain spaces) so the file is
    tolerant of tabs or spaces."""
    out = {}
    if not Path(path).exists():
        return out
    for raw in read_text(path).splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split(None, 1)
        if len(parts) != 2:
            continue
        out[parts[0].strip()] = parts[1].strip()
    return out


def region_files(game):
    root = LOGIC_DIR / ("oot" if game == OOT else "mm")
    return sorted(p for p in root.rglob("*.yml"))


def path_layout(path):
    """The `GameLayout` a region file belongs to — the axis on which the base and
    alternate dungeon layouts are mutually exclusive (mirror of ObjectDef.layout):
      oot/dungeons_mq/ -> oot_mq   oot/dungeons/ -> oot
      mm/jp/           -> mm_jp    mm/us/        -> mm
      everything else (overworld / boss / MM dungeons / system) -> all
    A region tagged `all` is always active; `oot`/`mm` only when its dungeon runs
    the base layout, `oot_mq`/`mm_jp` only when it runs the alternate one."""
    p = str(path).replace("\\", "/")
    if "/dungeons_mq/" in p:
        return "oot_mq"
    if "/oot/dungeons/" in p:
        return "oot"
    if "/mm/jp/" in p:
        return "mm_jp"
    if "/mm/us/" in p:
        return "mm"
    return "all"


# ── Expression parser (recursive descent) ────────────────────────────────────
TOKEN_RE = re.compile(r"\s*(&&|\|\||[()!,+\-*]|[A-Za-z_]\w*|0x[0-9a-fA-F]+|\d+)")
NUM_RE = re.compile(r"0x[0-9a-fA-F]+|\d+")

# Count of stray trailing ')' tolerated across all parsed expressions (report).
TRAILING_FIXES = 0


def tokenize(expr):
    toks, i, n = [], 0, len(expr)
    while i < n:
        m = TOKEN_RE.match(expr, i)
        if not m:
            if expr[i].isspace():
                i += 1
                continue
            raise ValueError(f"cannot tokenize {expr!r} at {i}: {expr[i:i+10]!r}")
        toks.append(m.group(1))
        i = m.end()
    return toks


class Parser:
    def __init__(self, toks):
        self.t = toks
        self.i = 0

    def peek(self):
        return self.t[self.i] if self.i < len(self.t) else None

    def eat(self, expect=None):
        tok = self.peek()
        if expect is not None and tok != expect:
            raise ValueError(f"expected {expect!r} got {tok!r}")
        self.i += 1
        return tok

    def parse(self):
        node = self.parse_or()
        # A few upstream logic files carry a stray unbalanced trailing ')'
        # (e.g. `A && (B) || (C))`). Tolerate it so a raw upstream drop still
        # compiles; count it for the report.
        while self.peek() == ")":
            self.eat()
            global TRAILING_FIXES
            TRAILING_FIXES += 1
        if self.peek() is not None:
            raise ValueError(f"trailing tokens: {self.t[self.i:]}")
        return node

    def parse_or(self):
        parts = [self.parse_and()]
        while self.peek() == "||":
            self.eat()
            parts.append(self.parse_and())
        return parts[0] if len(parts) == 1 else ("or", parts)

    def parse_and(self):
        parts = [self.parse_unary()]
        while self.peek() == "&&":
            self.eat()
            parts.append(self.parse_unary())
        return parts[0] if len(parts) == 1 else ("and", parts)

    def parse_unary(self):
        if self.peek() == "!":
            self.eat()
            return ("not", self.parse_unary())
        return self.parse_arith()

    def parse_arith(self):
        """Arithmetic on numeric counts (e.g. `has(WALLET, n - 1)`); folded to a
        constant at expansion time once params are bound."""
        node = self.parse_primary()
        while self.peek() in ("+", "-", "*"):
            op = self.eat()
            node = ("arith", op, node, self.parse_primary())
        return node

    def parse_primary(self):
        tok = self.peek()
        if tok == "(":
            self.eat("(")
            node = self.parse_or()
            self.eat(")")
            return node
        if tok is None:
            raise ValueError("unexpected end of expression")
        if NUM_RE.fullmatch(tok):
            self.eat()
            return ("num", int(tok, 0))
        if re.fullmatch(r"[A-Za-z_]\w*", tok):
            self.eat()
            if self.peek() == "(":
                self.eat("(")
                args = []
                if self.peek() != ")":
                    args.append(self.parse_or())
                    while self.peek() == ",":
                        self.eat()
                        args.append(self.parse_or())
                self.eat(")")
                return ("call", tok, args)
            return ("id", tok)
        raise ValueError(f"unexpected token {tok!r}")


def parse_expr(expr):
    return Parser(tokenize(expr)).parse()


# ── Macro expansion (produce a param-free, macro-free AST) ───────────────────
class Compiler:
    def __init__(self, id_sym):
        self.id_sym = id_sym
        # Interners (name -> index), each with a parallel ordered list.
        self.events, self.events_l = {}, []
        self.tricks, self.tricks_l = {}, []
        self.skeys, self.skeys_l = {}, []
        self.svals, self.svals_l = {}, []
        self.oot_time, self.oot_time_l = {}, []
        self.mm_time, self.mm_time_l = {}, []
        self.flags, self.flags_l = {}, []
        self.specials, self.specials_l = {}, []
        self.builtins, self.builtins_l = {}, []
        # Diagnostics.
        self.missing_items = set()
        self.builtins_seen = {}
        # Expression dedup: op-tuple -> index.
        self.expr_pool, self.expr_index = [], {}
        self.macros = None
        self.game = None

    # -- interner helpers --
    def _intern(self, d, lst, key):
        idx = d.get(key)
        if idx is None:
            idx = len(lst)
            d[key] = idx
            lst.append(key)
        return idx

    def intern_event(self, tok, game):
        g, name = game, tok
        if tok.startswith("OOT_"):
            g, name = OOT, tok[4:]
        elif tok.startswith("MM_"):
            g, name = MM, tok[3:]
        return self._intern(self.events, self.events_l, (g, name))

    def intern_trick(self, tok):
        return self._intern(self.tricks, self.tricks_l, tok)

    def intern_skey(self, tok):
        return self._intern(self.skeys, self.skeys_l, tok)

    def intern_sval(self, tok):
        return self._intern(self.svals, self.svals_l, tok)

    def intern_builtin(self, name):
        self.builtins_seen[name] = self.builtins_seen.get(name, 0) + 1
        return self._intern(self.builtins, self.builtins_l, name)

    def resolve_item(self, tok):
        t = tok
        if not (t.startswith("OOT_") or t.startswith("MM_") or t.startswith("SHARED_")):
            t = ITEM_PREFIX[self.game] + t
        v = self.id_sym.get(t)
        if v is None:
            self.missing_items.add(t)
            return None
        return v

    # -- expansion --
    def expand(self, ast, env, depth=0):
        if depth > 200:
            raise RecursionError("macro expansion too deep (cycle?)")
        kind = ast[0]
        if kind == "num":
            return ast
        if kind == "arith":
            op, a, b = ast[1], self.expand(ast[2], env, depth), self.expand(ast[3], env, depth)
            if a[0] == "num" and b[0] == "num":
                val = {"+": a[1] + b[1], "-": a[1] - b[1], "*": a[1] * b[1]}[op]
                return ("num", val)
            raise ValueError(f"non-constant arithmetic {op} on {a} {b}")
        if kind == "not":
            return ("not", self.expand(ast[1], env, depth))
        if kind in ("and", "or"):
            return (kind, [self.expand(x, env, depth) for x in ast[1]])
        if kind == "id":
            name = ast[1]
            if name in env:
                return env[name]
            if name in ("true", "false"):
                return ("const", name == "true")
            m = self.macros.get(name)
            if m is not None and not m[0]:  # 0-arg macro
                return self.expand(m[1], {}, depth + 1)
            return ("bareid", name)
        if kind == "call":
            name, args = ast[1], ast[2]
            eargs = [self.expand(a, env, depth) for a in args]
            m = self.macros.get(name)
            if m is not None and len(m[0]) == len(eargs):
                newenv = dict(zip(m[0], eargs))
                return self.expand(m[1], newenv, depth + 1)
            if name in NATIVES:
                return ("call", name, eargs)
            return ("builtin", name, eargs)
        raise ValueError(f"cannot expand {ast!r}")

    # -- macro bodies are stored as strings: parse+cache on first use --
    def get_macro_ast(self, name):
        params, body = self.macros[name]
        if isinstance(body, str):
            self.macros[name] = (params, parse_expr(body))
        return self.macros[name]

    # -- compile a clean AST to RPN ops --
    def compile(self, ast):
        kind = ast[0]
        if kind == "const":
            return [("const", ast[1])]
        if kind == "bareid":
            # A bare boolean identifier that was not a macro (world flag / typo).
            return [("builtin", self.intern_builtin(ast[1]))]
        if kind == "not":
            return self.compile(ast[1]) + [("not",)]
        if kind in ("and", "or"):
            parts = ast[1]
            ops = self.compile(parts[0])
            for p in parts[1:]:
                ops += self.compile(p) + [(kind,)]
            return ops
        if kind == "builtin":
            return [("builtin", self.intern_builtin(ast[1]))]
        if kind == "call":
            return self.compile_native(ast[1], ast[2])
        if kind == "num":
            raise ValueError("number in boolean position")
        raise ValueError(f"cannot compile {ast!r}")

    def _lit(self, node):
        """The literal token of a name argument (bareid) or number."""
        if node[0] == "bareid":
            return node[1]
        if node[0] == "id":
            return node[1]
        if node[0] == "const":
            return "true" if node[1] else "false"
        raise ValueError(f"expected literal arg, got {node!r}")

    def compile_native(self, name, args):
        if name == "has":
            item = self.resolve_item(self._lit(args[0]))
            cnt = args[1][1] if len(args) > 1 and args[1][0] == "num" else 1
            return [("const", False)] if item is None else [("has", item, cnt)]
        if name in ("renewable", "license"):
            item = self.resolve_item(self._lit(args[0]))
            return [("const", False)] if item is None else [(name, item)]
        if name == "event":
            return [("event", self.intern_event(self._lit(args[0]), self.game))]
        if name == "trick":
            return [("trick", self.intern_trick(self._lit(args[0])))]
        if name == "setting":
            k = self.intern_skey(self._lit(args[0]))
            if len(args) == 1:
                return [("setting", k)]
            return [("setting_eq", k, self.intern_sval(self._lit(args[1])))]
        if name == "cond":
            return (self.compile(args[0]) + self.compile(args[1])
                    + self.compile(args[2]) + [("cond",)])
        if name == "age":
            return [("age", 0 if self._lit(args[0]) == "child" else 1)]
        if name == "oot_time":
            return [("oot_time", self._intern(self.oot_time, self.oot_time_l, self._lit(args[0])))]
        if name == "mm_time":
            return [("mm_time", self._intern(self.mm_time, self.mm_time_l, self._lit(args[0])))]
        if name == "masks":
            return [("masks", args[0][1] if args[0][0] == "num" else 0)]
        if name == "special":
            return [("special", self._intern(self.specials, self.specials_l, self._lit(args[0])))]
        if name in ("flag_on", "flag_off"):
            idx = self._intern(self.flags, self.flags_l, self._lit(args[0]))
            return [("flag", idx, name == "flag_on")]
        if name in ("_song_event_oot", "_song_event_mm"):
            # (slot, songIndex) both fold to numeric literals after macro
            # expansion; if either is non-constant (shouldn't happen), stay
            # optimistic so we never hide the check.
            if args[0][0] != "num" or args[1][0] != "num":
                return [("const", True)]
            game = OOT if name.endswith("oot") else MM
            return [("song_event", game, args[0][1], args[1][1])]
        if name in OPTIMISTIC_NATIVES:
            # price / _song_event_* / has_pond_fish: no data source -> satisfiable.
            return [("const", True)]
        # Should be unreachable (only NATIVES reach here).
        return [("builtin", self.intern_builtin(name))]

    # -- expression -> deduped index --
    def expr_index_of(self, expr_str):
        ast = self.expand(parse_expr(expr_str), {})
        ops = tuple(self.compile(ast))
        idx = self.expr_index.get(ops)
        if idx is None:
            idx = len(self.expr_pool)
            self.expr_index[ops] = idx
            self.expr_pool.append(ops)
        return idx


# ── driver ───────────────────────────────────────────────────────────────────
def build_id_sym():
    items = (CPP_ROOT / "Headers/Combo/Items.h").read_text(encoding="utf-8", errors="replace")
    npc = (CPP_ROOT / "Headers/Combo/NPC.h").read_text(encoding="utf-8", errors="replace")
    return parse_defines(npc, seed=parse_defines(items))


def tracker_locations():
    """The set of pool/spoiler location strings (already game-prefixed)."""
    locs = set()
    for rel in ("Resources/Objects/pool_oot.csv", "Resources/Objects/pool_mm.csv"):
        # resolve() (from gen_data) tries C++-Tracker/ first, so the CSVs keep
        # resolving after the Qt tracker (and its Resources/) was relocated there.
        with resolve(rel).open(newline="", encoding="utf-8", errors="replace") as f:
            for row in csv.DictReader(f, delimiter=";"):
                loc = (row.get("location") or "").strip()
                if loc:
                    locs.add(loc)
    return locs


def bind_macro_bodies(macros):
    """Turn each stored `(params, body_str)` into `(params, body_ast)` lazily by
    pre-parsing all bodies once (surfaces parse errors up front)."""
    for name, (params, body) in list(macros.items()):
        if isinstance(body, str):
            macros[name] = (params, parse_expr(body))
    return macros


def collect(game, compiler, regions_out):
    """Parse + compile every region of one game into `regions_out`, using the
    game's macro table (common + game). Returns per-game location keys used."""
    used_locs = set()
    for path in region_files(game):
        layout = path_layout(path)
        for name, attrs, subs in load_regions(path, game):
            exits, events, locs = [], [], []
            for tgt, expr in subs["exits"].items():
                exits.append((tgt, compiler.expr_index_of(expr)))
            for ev, expr in subs["events"].items():
                ev_idx = compiler.intern_event(ev, game)
                events.append((ev_idx, compiler.expr_index_of(expr)))
            for loc, expr in subs["locations"].items():
                full = GAME_PREFIX[game] + loc
                used_locs.add(full)
                locs.append((full, compiler.expr_index_of(expr)))
            age_change = attrs.get("age_change", "true").strip().lower() != "false"
            regions_out.append({
                "game": game,
                "name": name,
                "layout": layout,
                "dungeon": attrs.get("dungeon", ""),
                "area": attrs.get("region", ""),
                "age_change": age_change,
                "exits": exits,
                "events": events,
                "locations": locs,
            })
    return used_locs


def resolve_region_targets(regions):
    """Map exit target names to region indices. Cross-game targets carry an
    `OOT `/`MM ` prefix; same-game targets are bare.

    A dungeon has a base and an alternate (MQ / JP) layout, so several regions can
    share a name across the two variants. Resolution keeps the same variant for an
    intra-dungeon exit; an exit from an `all` region into such a dungeon (the
    overworld -> dungeon entrance) fans out to *every* variant — the solver only
    activates the one matching the seed's layout, so the dead edges are harmless.
    Returns the set of unresolved target names."""
    from collections import defaultdict

    by_name = defaultdict(list)
    for i, r in enumerate(regions):
        by_name[(r["game"], r["name"])].append(i)
    unresolved = set()

    def candidates(game, target):
        g, name = game, target
        if target.startswith("OOT "):
            g, name = OOT, target[4:]
        elif target.startswith("MM "):
            g, name = MM, target[3:]
        return by_name.get((g, name), [])

    for r in regions:
        new_exits = []
        for tgt, expr in r["exits"]:
            cands = candidates(r["game"], tgt)
            if not cands:
                unresolved.add(f"{GAME_PREFIX[r['game']]}{r['name']} -> {tgt}")
                continue
            if len(cands) > 1:
                same = [i for i in cands if regions[i]["layout"] == r["layout"]]
                cands = same or cands
            for ti in cands:
                new_exits.append((ti, expr))
        r["exits"] = new_exits
    return unresolved


# ── Rust emission ────────────────────────────────────────────────────────────
def esc(s):
    return str(s).replace("\\", "\\\\").replace('"', '\\"')


def op_to_rust(op):
    k = op[0]
    if k == "const":
        return f"Op::Const({str(op[1]).lower()})"
    if k == "has":
        return f"Op::Has({op[1]:#x}, {op[2]})"
    if k == "renewable":
        return f"Op::Renewable({op[1]:#x})"
    if k == "license":
        return f"Op::License({op[1]:#x})"
    if k == "event":
        return f"Op::Event({op[1]})"
    if k == "trick":
        return f"Op::Trick({op[1]})"
    if k == "setting":
        return f"Op::Setting({op[1]})"
    if k == "setting_eq":
        return f"Op::SettingEq({op[1]}, {op[2]})"
    if k == "age":
        return f"Op::Age({op[1]})"
    if k == "oot_time":
        return f"Op::OotTime({op[1]})"
    if k == "mm_time":
        return f"Op::MmTime({op[1]})"
    if k == "masks":
        return f"Op::Masks({op[1]})"
    if k == "special":
        return f"Op::Special({op[1]})"
    if k == "flag":
        return f"Op::Flag({op[1]}, {str(op[2]).lower()})"
    if k == "song_event":
        return f"Op::SongEvent({op[1]}, {op[2]}, {op[3]})"
    if k == "builtin":
        return f"Op::Builtin({op[1]})"
    if k == "not":
        return "Op::Not"
    if k == "and":
        return "Op::And"
    if k == "or":
        return "Op::Or"
    if k == "cond":
        return "Op::Cond"
    raise ValueError(op)


def str_arr(name, items):
    if not items:
        return f"pub static {name}: &[&str] = &[];\n"
    cells = ",\n    ".join(f'"{esc(s)}"' for s in items)
    return f"pub static {name}: &[&str] = &[\n    {cells},\n];\n"


HEADER = """\
//! GENERATED by tools/gen_logic.py - DO NOT EDIT BY HAND.
//! Source of truth: the root `Logic/` folder (macros_*.yml + oot/**, mm/**).
//! Access rules compiled to a small stack bytecode (`Op`); the solver (M2) runs
//! it against the player's inventory / settings / tricks to compute reachability.
#![allow(dead_code, non_camel_case_types, non_upper_case_globals)]

use super::GameLayout;

/// One instruction of an access expression, evaluated on a boolean stack.
/// Leaves push a value; `Not`/`And`/`Or`/`Cond` consume operands from the stack.
#[derive(Clone, Copy, Debug)]
pub enum Op {
    Const(bool),
    /// `has(item, n)`: at least `n` of the item.
    Has(u32, u16),
    Renewable(u32),
    License(u32),
    Event(u32),
    Trick(u32),
    /// `setting(key)` enabled (boolean form).
    Setting(u32),
    /// `setting(key, value)`.
    SettingEq(u32, u32),
    /// Age: 0 = child, 1 = adult.
    Age(u8),
    OotTime(u8),
    MmTime(u8),
    Masks(u16),
    /// Win-condition gate (`special(x)`); index into `SPECIAL_NAMES`
    /// (BRIDGE / GANON_BK / LACS / MAJORA / MOON).
    Special(u32),
    /// MM region-state flag; index into `FLAG_NAMES`, `true` = `flag_on` (set),
    /// `false` = `flag_off` (clear).
    Flag(u32, bool),
    /// `_song_event_<game>(slot, song)`: true iff the seed placed song index
    /// `song` at event slot `slot`. Fields: `(game, slot, song)`, game 0 = OoT,
    /// 1 = MM. The slot->song map comes from the spoiler's `Song Events` section.
    SongEvent(u8, u8, u8),
    /// Native primitive not yet modelled; index into `BUILTIN_NAMES`. Should be
    /// unused (a leftover flags a new upstream primitive to model).
    Builtin(u32),
    Not,
    And,
    Or,
    Cond,
}

pub struct LogicEdge { pub to: u32, pub expr: u32 }
pub struct LogicEvent { pub event: u32, pub expr: u32 }
pub struct LogicLoc { pub loc: &'static str, pub expr: u32 }

/// A world region: its access rules to neighbours (`exits`), the events it can
/// set, and the checks it hosts (`locations`, already game-prefixed to match
/// `ObjectDef.location`). `game`: 0 = OoT, 1 = MM.
pub struct LogicRegion {
    pub game: u8,
    pub name: &'static str,
    /// Mutually-exclusive dungeon layout this region belongs to. `all` regions
    /// are always active; `oot`/`mm` only under the base layout, `oot_mq`/`mm_jp`
    /// only under the alternate one (mirror of ObjectDef.layout).
    pub layout: GameLayout,
    pub dungeon: &'static str,
    pub area: &'static str,
    pub age_change: bool,
    pub exits: &'static [LogicEdge],
    pub events: &'static [LogicEvent],
    pub locations: &'static [LogicLoc],
}
"""


def emit(out_path, compiler, regions):
    parts = [HEADER, ""]

    # Expression bytecode pool.
    lines = ["pub static EXPRS: &[&[Op]] = &["]
    for ops in compiler.expr_pool:
        cells = ", ".join(op_to_rust(o) for o in ops)
        lines.append(f"    &[{cells}],")
    lines.append("];\n")
    parts.append("\n".join(lines))

    # Regions.
    lines = ["pub static LOGIC_REGIONS: &[LogicRegion] = &["]
    for r in regions:
        ex = ", ".join(f"LogicEdge {{ to: {t}, expr: {e} }}" for t, e in r["exits"])
        ev = ", ".join(f"LogicEvent {{ event: {i}, expr: {e} }}" for i, e in r["events"])
        lo = ", ".join(
            f'LogicLoc {{ loc: "{esc(l)}", expr: {e} }}' for l, e in r["locations"])
        lines.append(
            "    LogicRegion {{ game: {g}, name: \"{nm}\", layout: GameLayout::{lay}, "
            "dungeon: \"{dg}\", area: \"{ar}\", age_change: {ac}, exits: &[{ex}], "
            "events: &[{ev}], locations: &[{lo}] }},".format(
                g=r["game"], nm=esc(r["name"]), lay=r["layout"], dg=esc(r["dungeon"]),
                ar=esc(r["area"]), ac=str(r["age_change"]).lower(),
                ex=ex, ev=ev, lo=lo))
    lines.append("];\n")
    parts.append("\n".join(lines))

    # Symbol name tables (index == the id used in the bytecode / regions).
    parts.append("/// Event names, indexed by event id. Prefixed OOT_/MM_ shows the game.\n"
                 + str_arr("EVENT_NAMES",
                           [f"{GAME_PREFIX[g]}{n}".strip() for (g, n) in compiler.events_l]))
    parts.append("/// Trick names (OoTMM trick ids), indexed by trick id.\n"
                 + str_arr("TRICK_NAMES", compiler.tricks_l))
    # Display name -> trick id, for the ids the logic actually references. Sorted
    # by name so the runtime can binary-search the spoiler's `Tricks:`/`Glitches:`
    # entries back to ids.
    names = load_trick_names()
    pairs = sorted((names[t], t) for t in compiler.tricks_l if t in names)
    cells = "\n    ".join(f'("{esc(nm)}", "{esc(tid)}"),' for nm, tid in pairs)
    body = "&[]" if not pairs else f"&[\n    {cells}\n]"
    parts.append(
        "/// Spoiler trick/glitch display name -> OoTMM trick id (a subset of\n"
        "/// `TRICK_NAMES`: only ids the logic references). Sorted by name for\n"
        "/// binary search. Vendored from OoTMM tricks.ts via tools/tricks.tsv.\n"
        f"pub static TRICK_NAME_TO_ID: &[(&str, &str)] = {body};\n")
    parts.append("/// Setting keys, indexed by setting id.\n"
                 + str_arr("SETTING_KEYS", compiler.skeys_l))
    parts.append("/// Setting values referenced by `setting(k, v)`, indexed by value id.\n"
                 + str_arr("SETTING_VALUES", compiler.svals_l))
    parts.append("/// OoT time-of-day values, indexed by id.\n"
                 + str_arr("OOT_TIME_VALUES", compiler.oot_time_l))
    parts.append("/// MM time values, indexed by id.\n"
                 + str_arr("MM_TIME_VALUES", compiler.mm_time_l))
    parts.append("/// Win-condition gate names (`Op::Special`), indexed by id.\n"
                 + str_arr("SPECIAL_NAMES", compiler.specials_l))
    parts.append("/// MM region-state flag names (`Op::Flag`), indexed by id.\n"
                 + str_arr("FLAG_NAMES", compiler.flags_l))
    parts.append("/// Native primitives not yet modelled (see `Op::Builtin`).\n"
                 + str_arr("BUILTIN_NAMES", compiler.builtins_l))

    Path(out_path).write_text("\n".join(parts), encoding="utf-8")


def generate(out_path=OUT_DEFAULT, id_sym=None, verbose=True):
    id_sym = id_sym or build_id_sym()
    common = load_macros(LOGIC_DIR / "macros_common.yml")
    oot_macros = dict(common); oot_macros.update(load_macros(LOGIC_DIR / "macros_oot.yml"))
    mm_macros = dict(common); mm_macros.update(load_macros(LOGIC_DIR / "macros_mm.yml"))

    compiler = Compiler(id_sym)
    regions = []

    compiler.macros = bind_macro_bodies(oot_macros)
    compiler.game = OOT
    used_oot = collect(OOT, compiler, regions)

    compiler.macros = bind_macro_bodies(mm_macros)
    compiler.game = MM
    used_mm = collect(MM, compiler, regions)

    unresolved = resolve_region_targets(regions)
    emit(out_path, compiler, regions)

    # ── Location coverage report (the M1 acceptance check) ──
    tracker = tracker_locations()
    logic_locs = used_oot | used_mm
    in_logic_not_tracker = sorted(logic_locs - tracker)
    in_tracker_not_logic = sorted(tracker - logic_locs)
    matched = logic_locs & tracker

    if verbose:
        print(f"OK -> {out_path}")
        print(f"  regions        : {len(regions)} (OoT+MM)")
        print(f"  expressions    : {len(compiler.expr_pool)} (deduped)")
        print(f"  events/tricks  : {len(compiler.events_l)} / {len(compiler.tricks_l)}")
        print(f"  settings/vals  : {len(compiler.skeys_l)} / {len(compiler.svals_l)}")
        print(f"  specials/flags : {len(compiler.specials_l)} / {len(compiler.flags_l)}")
        trick_names = load_trick_names()
        no_display = [t for t in compiler.tricks_l if t not in trick_names]
        print(f"  tricks named   : {len(compiler.tricks_l) - len(no_display)}"
              f"/{len(compiler.tricks_l)} (display name known)")
        if no_display:
            print(f"  WARN tricks referenced by logic with no display name "
                  f"({len(no_display)}): {sorted(no_display)[:15]}", file=sys.stderr)
        print(f"  locations      : logic={len(logic_locs)} tracker={len(tracker)} "
              f"matched={len(matched)}")
        print(f"  loc in logic not in tracker: {len(in_logic_not_tracker)}")
        print(f"  loc in tracker not in logic: {len(in_tracker_not_logic)}")
        if compiler.missing_items:
            print(f"  WARN unresolved items ({len(compiler.missing_items)}): "
                  f"{sorted(compiler.missing_items)[:15]}", file=sys.stderr)
        if unresolved:
            print(f"  WARN unresolved exit targets ({len(unresolved)}): "
                  f"{sorted(unresolved)[:15]}", file=sys.stderr)
        if compiler.builtins_seen:
            top = sorted(compiler.builtins_seen.items(), key=lambda kv: -kv[1])[:20]
            print(f"  WARN unmodelled natives ({len(compiler.builtins_seen)}): {top}",
                  file=sys.stderr)
        if in_logic_not_tracker:
            print(f"  sample logic-only locations: {in_logic_not_tracker[:15]}",
                  file=sys.stderr)
    return {
        "regions": regions, "compiler": compiler,
        "logic_only": in_logic_not_tracker, "tracker_only": in_tracker_not_logic,
        "matched": matched, "unresolved_targets": unresolved,
    }


if __name__ == "__main__":
    generate()
