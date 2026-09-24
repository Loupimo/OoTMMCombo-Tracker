#!/usr/bin/env python3
"""Snapshot OoTMM's renewable check locations, with their vanilla item, into
`tools/ootmm_renewables.tsv`, which `gen_logic.py` compiles offline into
`RENEWABLE_LOCATIONS` (logic.rs).

The logic's `renewable(ITEM)` holds once a *renewable* location that holds ITEM is
reachable (OoTMM pathfind: `isLocationRenewable` -> `ws.renewables`). Which
locations are renewable is code, not data, upstream
(`packages/logic/src/locations.ts`, `RENEWABLE_LOCATIONS`): every check whose type
is in a fixed list (shop, cow, scrub, fairy, fish, fairy-spot, gossip, gossip-big)
minus the one-time shop buys, plus explicit lists (MM scrub shops, OoT / MM
merchants, Tingle maps). This script re-reads those rules from `locations.ts` (so an
upstream change is picked up, and a refactor fails loudly instead of silently) and
takes each check's type and vanilla item from `data/checks/**/*.xml`.

The vanilla item matters because the spoiler only lists SHUFFLED locations: in a
seed with vanilla shops / cows the tracker must still know that the Market Potion
Shop sells a Red Potion and that a cow gives Milk.

Usage:
    python gen_renewables.py                # fetch from OoTMM master (GitHub)
    python gen_renewables.py <ootmm_root>   # local OoTMM checkout

Re-run after an OoTMM update (see REGEN.md). Output is sorted so diffs are clean.
"""
import json
import os
import re
import sys
import urllib.request
import xml.etree.ElementTree as ET

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "ootmm_renewables.tsv")
REPO = "OoTMM/OoTMM"
BRANCH = "master"
RAW = f"https://raw.githubusercontent.com/{REPO}/{BRANCH}/"
TREE_API = f"https://api.github.com/repos/{REPO}/git/trees/{BRANCH}?recursive=1"
LOCATIONS_TS = "packages/logic/src/locations.ts"
CHECKS_DIR = "data/checks/"

GAME_PREFIX = {"oot": "OOT", "mm": "MM"}
# Vanilla item values that are not a fixed item (no renewable source to record).
NO_ITEM = {None, "", "NOTHING", "RANDOM"}


def fetch(url):
    with urllib.request.urlopen(url) as r:
        return r.read().decode("utf-8")


def load_sources(root):
    """Return (locations.ts text, {xml path: xml text}) from a checkout or GitHub."""
    if root:
        if not os.path.isdir(root):
            sys.exit(f"not an OoTMM root: {root}")
        ts = open(os.path.join(root, LOCATIONS_TS), encoding="utf-8").read()
        xmls = {}
        base = os.path.join(root, CHECKS_DIR)
        for dirpath, _, files in os.walk(base):
            for name in files:
                if name.endswith(".xml"):
                    path = os.path.join(dirpath, name)
                    rel = os.path.relpath(path, root).replace(os.sep, "/")
                    xmls[rel] = open(path, encoding="utf-8").read()
        return ts, xmls
    print(f"fetching {REPO}@{BRANCH}", file=sys.stderr)
    ts = fetch(RAW + LOCATIONS_TS)
    tree = json.loads(fetch(TREE_API))["tree"]
    paths = sorted(e["path"] for e in tree
                   if e["path"].startswith(CHECKS_DIR) and e["path"].endswith(".xml"))
    return ts, {p: fetch(RAW + p) for p in paths}


def quoted(text):
    """The single-quoted TS string literals of `text`, unescaped."""
    return [s.replace("\\'", "'") for s in re.findall(r"'((?:[^'\\]|\\.)*)'", text)]


def renewable_rules(ts):
    """(check types, excluded locations, explicit locations) from `RENEWABLE_LOCATIONS`."""
    m = re.search(r"const RENEWABLE_LOCATIONS = new Set\(\[(.*?)\]\);", ts, re.S)
    if not m:
        sys.exit("RENEWABLE_LOCATIONS not found in locations.ts (upstream refactor?)")
    block = m.group(1)
    tm = re.search(r"\[([^\]]*)\]\.includes\(x\.type\)", block)
    if not tm:
        sys.exit("renewable check-type list not found in RENEWABLE_LOCATIONS")
    types = set(quoted(tm.group(1)))

    def const_list(name):
        cm = re.search(rf"(?:export\s+)?const {name}\s*=\s*\[(.*?)\];", ts, re.S)
        if not cm:
            sys.exit(f"list {name} not found in locations.ts")
        return quoted(cm.group(1))

    excluded = set()
    for name in re.findall(r"!([A-Z_]+)\.includes\(x\.location\)", block):
        excluded.update(const_list(name))
    explicit = set()
    for name in re.findall(r"\.\.\.([A-Z_]+)\s*,", block):
        if name != "CHECKS":
            explicit.update(const_list(name))
    return types, excluded, explicit


def parse_checks(xmls):
    """location -> (type, vanilla item symbol or None), game-prefixed like the spoiler."""
    checks = {}
    for text in xmls.values():
        root = ET.fromstring(text.encode("utf-8"))
        pre = GAME_PREFIX[root.get("game")]
        for el in root.iter():
            loc = el.get("location")
            if not loc:
                continue
            item = el.get("item")
            checks[f"{pre} {loc}"] = (
                el.get("type") or el.tag,
                None if item in NO_ITEM else f"{pre}_{item}",
            )
    return checks


def main():
    ts, xmls = load_sources(sys.argv[1] if len(sys.argv) > 1 else None)
    types, excluded, explicit = renewable_rules(ts)
    checks = parse_checks(xmls)
    missing = sorted(explicit - checks.keys())
    if missing:
        sys.exit(f"explicit renewable locations with no check data: {missing}")
    locs = {l for l, (t, _) in checks.items() if t in types and l not in excluded} | explicit
    with open(OUT, "w", encoding="utf-8", newline="\n") as f:
        f.write("# OoTMM renewable check locations (packages/logic/src/locations.ts\n")
        f.write("# RENEWABLE_LOCATIONS) and their vanilla item (data/checks/**/*.xml).\n")
        f.write("# Generated by tools/gen_renewables.py - do not edit by hand.\n")
        f.write(f"# types: {', '.join(sorted(types))}\n")
        f.write("# location\tvanilla item (empty = random / nothing)\n")
        for loc in sorted(locs):
            f.write(f"{loc}\t{checks[loc][1] or ''}\n")
    print(f"wrote {OUT}: {len(locs)} renewable locations "
          f"({len(explicit)} explicit, {len(excluded)} one-time excluded)")


if __name__ == "__main__":
    main()
