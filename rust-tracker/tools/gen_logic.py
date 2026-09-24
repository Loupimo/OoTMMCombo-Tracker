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

# Renewable check locations + their vanilla item, snapshotted from OoTMM by
# gen_renewables.py (the renewable rule is upstream code, not part of `Logic/`).
RENEWABLES_TSV = Path(__file__).resolve().parent / "ootmm_renewables.tsv"

# The C++ headers (item / NPC symbols) moved under C++-Tracker/ when the Qt
# tracker was relocated; fall back to the repo root for older checkouts.
CPP_ROOT = ROOT / "C++-Tracker" if (ROOT / "C++-Tracker" / "Headers").is_dir() else ROOT

OOT, MM = 0, 1
GAME_PREFIX = {OOT: "OOT ", MM: "MM "}
ITEM_PREFIX = {OOT: "OOT_", MM: "MM_"}

# Song of Storms name collision (OoTMM overloads SONG_STORMS for two things):
#   * the ITEM   -> tracker Items.h: OOT_SONG_STORMS 0x8E / MM_SONG_STORMS 0x293,
#                   and the SHARED item is SHARED_SONG_OF_STORMS 0x411.
#   * the NPC    -> tracker NPC.h:  OOT_SONG_OF_STORMS 0x06 / MM_SONG_OF_STORMS 0x0d
#                   (renamed with the _OF_ infix to disambiguate from the item; 0x06
#                   also happens to be OOT_BOOMERANG).
# The logic's has(SONG_STORMS) means the ITEM, so per game it must resolve DIRECTLY to
# OOT_/MM_SONG_STORMS -- no alias. Aliasing it to SONG_OF_STORMS (as an earlier naming
# generation did) now points it at the NPC id 0x06 == Boomerang, so a collected
# Boomerang wrongly satisfied can_play_storms (gossip "big fairy" lit with no Song of
# Storms). Only the SHARED item still needs the alias, since it is named
# SHARED_SONG_OF_STORMS and there is no SHARED_SONG_STORMS define to resolve to.
# Keep in sync with the Qt Items.h / NPC.h naming.
LOGIC_ITEM_ALIASES = {
    "SHARED_SONG_STORMS": "SHARED_SONG_OF_STORMS",
}

# Native primitives (called but never defined as a macro). Each compiles to a
# dedicated op. Any *other* native still becomes a generic `Builtin` leaf,
# recorded in `builtins_seen` (should be empty now — a non-empty set flags a new
# upstream primitive to model).
#
# CORE: modelled precisely by the M2 evaluator.
CORE_NATIVES = {
    "has", "event", "trick", "setting", "cond", "age", "oot_time", "mm_time",
    "renewable", "license", "masks",
    # has_pond_fish(kind, lo, hi): with pondFishShuffle the weighted fish are real
    # uniquely-ided shuffled items (OOT_FISHING_POND_<kind>_<w>LBS, one per pound),
    # so this desugars to an OR of has() over the weight range (see compile_native)
    # -- modelled precisely, no dedicated op.
    "has_pond_fish",
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
# OPTIMISTIC: modelled as satisfiable at compile time when the tracker has no
# per-seed data source, so a "show only reachable" tracker does not hide for them.
# `price` is handled specially in `compile_native` (budget-aware: a POSITIVE budget
# stays true, but the 0-budget "free" branch is false so the wallet/rupee tiers of
# `wallet_price` actually gate shop / scrub / merchant checks — otherwise every one
# shows even with no wallet). This set is the generic blanket-true fallback.
OPTIMISTIC_NATIVES = {"price"}
NATIVES = CORE_NATIVES | OPTIMISTIC_NATIVES

# Logic `var(NAME)` values: a per-seed number that OoTMM binds from a setting
# (`world/builder.ts addVar`). Used only as the count of a `has(item, var(NAME))`
# (currently just the stray-fairy Great Fairy rewards). Maps NAME -> (spoiler
# setting key, OoTMM default) so the solver reads the real threshold per seed and
# falls back to the default when the spoiler omits it. Without this the count arg
# fell back to 1, so a single stray fairy wrongly satisfied the Great Fairy.
VAR_INFO = {
    "STRAY_FAIRY_COUNT": ("strayFairyRewardCount", 15),
}


# ── MM time-of-day model ─────────────────────────────────────────────────────
# Ordered MM time slices (vendored from OoTMM packages/logic/src/expr/data.ts).
# `at/after/before/between(slice)` compile to a bitmask over these; the solver
# tests it against the set of slices the player can reach (`mm_time_slices`), so
# a check gated on e.g. `after(NIGHT3_AM_12_00)` needs the Night 3 clock, not
# merely *some* clock. Index i -> bit (1 << i); 47 slices fit a u64.
MM_TIME_SLICES = [
    "DAY1_AM_06_00", "DAY1_AM_07_00", "DAY1_AM_08_00", "DAY1_AM_10_00",
    "DAY1_PM_01_45", "DAY1_PM_03_00", "DAY1_PM_04_00",
    "NIGHT1_PM_06_00", "NIGHT1_PM_08_00", "NIGHT1_PM_09_00", "NIGHT1_PM_10_00",
    "NIGHT1_PM_11_00", "NIGHT1_AM_12_00", "NIGHT1_AM_02_30", "NIGHT1_AM_04_00",
    "NIGHT1_AM_05_00",
    "DAY2_AM_06_00", "DAY2_AM_07_00", "DAY2_AM_08_00", "DAY2_AM_10_00",
    "DAY2_AM_11_30", "DAY2_PM_02_00", "DAY2_PM_04_00",
    "NIGHT2_PM_06_00", "NIGHT2_PM_08_00", "NIGHT2_PM_09_00", "NIGHT2_PM_10_00",
    "NIGHT2_PM_11_00", "NIGHT2_AM_12_00", "NIGHT2_AM_04_00", "NIGHT2_AM_05_00",
    "NIGHT2_AM_05_30",
    "DAY3_AM_06_00", "DAY3_AM_07_00", "DAY3_AM_08_00", "DAY3_AM_10_00",
    "DAY3_AM_11_30", "DAY3_PM_01_00",
    "NIGHT3_PM_06_00", "NIGHT3_PM_08_00", "NIGHT3_PM_09_00", "NIGHT3_PM_10_00",
    "NIGHT3_PM_11_00", "NIGHT3_AM_12_00", "NIGHT3_AM_04_00", "NIGHT3_AM_05_00",
]

# The six MM day/night periods, in the same order as the `clock_day1 ..
# clock_night3` macros and `MM_CLOCK_PERIOD_MACROS` below. A slice belongs to a
# period by its name prefix (`DAY1_...` -> DAY1).
MM_PERIODS = ["DAY1", "NIGHT1", "DAY2", "NIGHT2", "DAY3", "NIGHT3"]

# The macro whose truth (pure has/setting: clock items + progressiveClocks mode)
# says whether that period is reachable, one per `MM_PERIODS` entry.
MM_CLOCK_PERIOD_MACROS = [
    "clock_day1", "clock_night1", "clock_day2",
    "clock_night2", "clock_day3", "clock_night3",
]


def mm_time_mask(operator, slice_names):
    """Port of OoTMM `exprMmTime`: the bitmask of MM time slices that satisfy a
    `before/after/at/between(slice[, slice2])` requirement. `before` = strictly
    earlier slices, `after` = this slice and all later, `at` = this slice only,
    `between(a, b)` = `[a, b)`. Unknown slice names raise (fail loud)."""
    idx = [MM_TIME_SLICES.index(s) for s in slice_names]
    n = len(MM_TIME_SLICES)
    v = 0
    if operator == "before":
        for i in range(0, idx[0]):
            v |= (1 << i)
    elif operator == "after":
        for i in range(idx[0], n):
            v |= (1 << i)
    elif operator == "at":
        v |= (1 << idx[0])
    elif operator == "between":
        for i in range(idx[0], idx[1]):
            v |= (1 << i)
    else:
        raise ValueError(f"bad mm_time operator {operator!r}")
    return v


def mm_period_slices():
    """The slice bitmask of each `MM_PERIODS` entry (by slice-name prefix)."""
    masks = [0] * len(MM_PERIODS)
    for i, name in enumerate(MM_TIME_SLICES):
        masks[MM_PERIODS.index(name.split("_", 1)[0])] |= (1 << i)
    return masks


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
    body_indent = None  # indent of a region's direct children (attrs / subsections)
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
            body_indent = None
        else:
            # A region's direct children (attributes + subsection headers) sit at
            # the first non-zero indent seen after its header; entries inside a
            # subsection are deeper. The body indent is detected (not hard-coded to
            # 2) so regions indented with 4 spaces parse the same as 2-space ones.
            if body_indent is None:
                body_indent = indent
            if indent <= body_indent:
                # `key:` (maybe with a trailing comment) with no value is a
                # subsection header; `key: value` is a scalar attribute.
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


def load_renewables(path=RENEWABLES_TSV):
    """[(location, vanilla item symbol or None)] from the gen_renewables snapshot."""
    rows = []
    for line in read_text(path).splitlines():
        if not line.strip() or line.startswith("#"):
            continue
        loc, _, item = line.partition("	")
        rows.append((loc.strip(), item.strip() or None))
    return rows


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
        self.vars, self.vars_l = {}, []
        self.builtins, self.builtins_l = {}, []
        # Diagnostics.
        self.missing_items = set()
        self.missing_renewable_items = set()
        self.builtins_seen = {}
        # Expression dedup: op-tuple -> index.
        self.expr_pool, self.expr_index = [], {}
        # EXPRS indices of the six MM clock-period rules (filled in generate()).
        self.mm_clock_period_exprs = [0] * 6
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
        t = LOGIC_ITEM_ALIASES.get(tok, tok)
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
            if item is None:
                return [("const", False)]
            # `has(item, var(NAME))`: the count is a per-seed number the solver
            # reads from a setting (see VAR_INFO). Without this it fell through to
            # the `else 1` below, so one item satisfied a threshold of N.
            a1 = args[1] if len(args) > 1 else None
            if a1 is not None and a1[0] in ("builtin", "call") and a1[1] == "var":
                vname = self._lit(a1[2][0])
                if vname not in VAR_INFO:
                    raise ValueError(f"unknown var({vname}) — add it to VAR_INFO")
                return [("has_var", item, self._intern(self.vars, self.vars_l, vname))]
            cnt = a1[1] if a1 is not None and a1[0] == "num" else 1
            return [("has", item, cnt)]
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
            # `mm_time(operator, slice[, slice2])`: keep the actual time bound by
            # compiling it to a slice bitmask (interned by value), not just the
            # operator name -- the old code dropped the bound entirely.
            operator = self._lit(args[0])
            slice_names = [self._lit(a) for a in args[1:]]
            mask = mm_time_mask(operator, slice_names)
            return [("mm_time", self._intern(self.mm_time, self.mm_time_l, mask))]
        if name == "masks":
            return [("masks", args[0][1] if args[0][0] == "num" else 0)]
        if name == "special":
            return [("special", self._intern(self.specials, self.specials_l, self._lit(args[0])))]
        if name in ("flag_on", "flag_off"):
            idx = self._intern(self.flags, self.flags_l, self._lit(args[0]))
            return [("flag", idx, name == "flag_on")]
        if name == "has_pond_fish":
            return self.compile_pond_fish(args)
        if name in ("_song_event_oot", "_song_event_mm"):
            # (slot, songIndex) both fold to numeric literals after macro
            # expansion; if either is non-constant (shouldn't happen), stay
            # optimistic so we never hide the check.
            if args[0][0] != "num" or args[1][0] != "num":
                return [("const", True)]
            game = OOT if name.endswith("oot") else MM
            return [("song_event", game, args[0][1], args[1][1])]
        if name == "price":
            # `price(range, id, budget)`: the item's shop/scrub/merchant cost is <= budget.
            # We have no per-seed price data (randomised), so we stay optimistic — but only
            # for a POSITIVE budget. `wallet_price` is
            #   price(_, _, 0) || (has_rupees && ((price(_, _, 99) && has_wallet(1)) || ...))
            # so treating price(_, _, 0) as true (the old blanket-true `price`) short-circuits
            # the whole OR and shows every shop/scrub/merchant check even with NO wallet and
            # NO rupees (reported: shop items visible without a wallet). The 0-budget branch
            # means "the item is free"; assume it is NOT (returns false), so the wallet tiers
            # `has_rupees && has_wallet(n)` gate the check. Any positive budget stays true
            # (affordable if the player can hold that many rupees).
            budget = args[2] if len(args) > 2 else None
            free = budget is not None and budget[0] == "num" and budget[1] == 0
            return [("const", not free)]
        if name in OPTIMISTIC_NATIVES:
            # A modelled-as-satisfiable native with no per-seed data source.
            return [("const", True)]
        # Should be unreachable (only NATIVES reach here).
        return [("builtin", self.intern_builtin(name))]

    def compile_pond_fish(self, args):
        """`has_pond_fish(kind, lo, hi)` (pondFishShuffle). With the setting on, the
        weighted fish become real, uniquely-ided shuffled items
        `OOT_FISHING_POND_<kind>_<w>LBS` (one per pound), collectable anywhere; the
        check is satisfied by owning any fish of that kind weighing in [lo, hi].
        Desugar to an OR of `has()` over the weight range so the ordinary inventory
        path (find_item_id crediting a collected fish) drives it. Terms whose weight
        has no item id (a partial range) drop out; an empty range is const-false.
        When pondFishShuffle is off, the enclosing rule short-circuits this via
        `!setting(pondFishShuffle) ||` and none of these items are in the pool. A
        non-constant bound (shouldn't happen) stays optimistic so nothing is hidden."""
        kind = self._lit(args[0])
        if args[1][0] != "num" or args[2][0] != "num":
            return [("const", True)]
        lo, hi = args[1][1], args[2][1]
        terms = [("has", item, 1)
                 for w in range(lo, hi + 1)
                 if (item := self.resolve_item(f"FISHING_POND_{kind}_{w}LBS")) is not None]
        if not terms:
            return [("const", False)]
        ops = [terms[0]]
        for t in terms[1:]:
            ops += [t, ("or",)]
        return ops

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
    if k == "has_var":
        return f"Op::HasVar({op[1]:#x}, {op[2]})"
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
    /// `has(item, var(NAME))`: at least a per-seed count of the item; the count
    /// is read from a setting at solve time (index into `VAR_NAMES`).
    HasVar(u32, u8),
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
    # `var(NAME)` tables (`Op::HasVar` count sources), indexed by var id: the name,
    # the spoiler setting the solver reads the count from, and the OoTMM default.
    parts.append("/// `var(NAME)` names, indexed by `Op::HasVar` var id.\n"
                 + str_arr("VAR_NAMES", compiler.vars_l))
    parts.append("/// Spoiler setting key each var's count is read from, same order.\n"
                 + str_arr("VAR_SETTING_KEYS", [VAR_INFO[v][0] for v in compiler.vars_l]))
    vd_cells = ", ".join(str(VAR_INFO[v][1]) for v in compiler.vars_l)
    parts.append("/// OoTMM default count for each var (fallback when the spoiler\n"
                 "/// omits the setting), same order.\n"
                 f"pub static VAR_DEFAULTS: &[u16] = &[{vd_cells}];\n")
    parts.append("/// OoT time-of-day values, indexed by id.\n"
                 + str_arr("OOT_TIME_VALUES", compiler.oot_time_l))
    # MM time is a slice bitmask (`Op::MmTime` indexes this): a check is time-
    # reachable iff this mask intersects the player's reachable slices.
    mm_mask_cells = ",\n    ".join(f"0x{m:x}" for m in compiler.mm_time_l)
    parts.append(
        "/// MM time requirement bitmasks, indexed by `Op::MmTime` id. Bit i =\n"
        "/// `MM_TIME_SLICES[i]`; the check holds iff this intersects the reachable\n"
        "/// slice set (see `MM_CLOCK_PERIOD_EXPRS` / `MM_PERIOD_SLICES`).\n"
        f"pub static MM_TIME_MASKS: &[u64] = &[\n    {mm_mask_cells},\n];\n")
    # Per-period slice masks (DAY1, NIGHT1, DAY2, NIGHT2, DAY3, NIGHT3) and the
    # EXPRS index of each period's reachability rule, in the same order. The
    # solver ORs a period's slices into `mm_time_slices` when its rule evaluates
    # true (all-slices when clock shuffle is off, since the rules short-circuit).
    ps_cells = ",\n    ".join(f"0x{m:x}" for m in mm_period_slices())
    parts.append(
        "/// MM day/night period slice masks: [DAY1, NIGHT1, DAY2, NIGHT2, DAY3,\n"
        "/// NIGHT3]. OR'd into the reachable slice set per period whose rule holds.\n"
        f"pub static MM_PERIOD_SLICES: [u64; 6] = [\n    {ps_cells},\n];\n")
    cpe_cells = ", ".join(str(i) for i in compiler.mm_clock_period_exprs)
    parts.append(
        "/// EXPRS index of each MM period's reachability rule, matching\n"
        "/// `MM_PERIOD_SLICES` order (clock_day1 .. clock_night3).\n"
        f"pub static MM_CLOCK_PERIOD_EXPRS: [u32; 6] = [{cpe_cells}];\n")
    # Renewable sources (`Op::Renewable`): location -> vanilla item id (0 = none /
    # unresolved), sorted by location for binary search.
    ren_cells = []
    for loc, item in sorted(load_renewables()):
        iid = 0
        if item:
            iid = compiler.id_sym.get(LOGIC_ITEM_ALIASES.get(item, item))
            if iid is None:
                compiler.missing_renewable_items.add(item)
                iid = 0
        ren_cells.append(f'("{esc(loc)}", {iid:#x}),')
    body = "&[]" if not ren_cells else "&[\n    " + "\n    ".join(ren_cells) + "\n]"
    parts.append(
        "/// OoTMM renewable check locations (`isLocationRenewable`) and their vanilla\n"
        "/// item id (0 = random / nothing / unresolved). Reaching one that holds X makes\n"
        "/// `renewable(X)` hold. Sorted by location for binary search. Snapshotted from\n"
        "/// OoTMM via tools/ootmm_renewables.tsv (tools/gen_renewables.py).\n"
        f"pub static RENEWABLE_LOCATIONS: &[(&str, u32)] = {body};\n")
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

    # Tracker-specific MM age model (deliberate deviation from OoTMM's macros_mm.yml).
    # OoTMM defines `is_adult`/`is_child` as
    #   cond(setting(crossAge), age(<x>), setting(startingAgeMm, <x>) || has_mask_adult)
    # relying on a pathfinder that tracks Link's *physical* age per node and only lets it
    # flip at an age-change node when you own MM_MASK_ADULT (crossAge). This solver has no
    # such model: it seeds MM at BOTH ages unconditionally, so `age(adult)` is trivially
    # true and every adult-only MM check lights up even with no way to become adult
    # (reported: the Doggy Racetrack Chest showing via is_tall -> is_adult without the
    # mask). In MM the age is fully determined by the adult mask (crossAge) or the starting
    # age, so drop the age() dependency entirely: adult iff you own the mask or start adult;
    # child otherwise. `!setting(startingAgeMm, adult)` yields the child default (MM starts
    # child) without needing the setting seeded, and both branches of the original `cond`
    # collapse to this (with crossAge off, has(MM_MASK_ADULT) is 0 — the mask is not in the
    # pool — so it reduces to setting(startingAgeMm, <x>), matching the OoTMM off-branch).
    mm_macros["is_adult"] = ([], "has(MM_MASK_ADULT) || setting(startingAgeMm, adult)")
    mm_macros["is_child"] = ([], "has(MM_MASK_ADULT) || !setting(startingAgeMm, adult)")

    compiler = Compiler(id_sym)
    regions = []

    compiler.macros = bind_macro_bodies(oot_macros)
    compiler.game = OOT
    used_oot = collect(OOT, compiler, regions)

    compiler.macros = bind_macro_bodies(mm_macros)
    compiler.game = MM
    used_mm = collect(MM, compiler, regions)

    # Compile the six clock-period reachability macros (pure clock-item / mode
    # logic) while the MM macro table + game are still bound; the solver ORs in a
    # period's slices when its rule holds to build `mm_time_slices`.
    compiler.mm_clock_period_exprs = [
        compiler.expr_index_of(m) for m in MM_CLOCK_PERIOD_MACROS
    ]

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
        # A vanilla item with no tracker id (the generic weightless pond fish) can
        # only matter if the logic asks `renewable()` of it, and any such symbol is
        # already reported by the `unresolved items` warning below — so just count.
        renewables = {loc for loc, _ in load_renewables()}
        print(f"  renewables     : {len(renewables)} locations "
              f"({len(renewables - logic_locs)} not in logic; vanilla item without "
              f"tracker id: {sorted(compiler.missing_renewable_items)})")
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
