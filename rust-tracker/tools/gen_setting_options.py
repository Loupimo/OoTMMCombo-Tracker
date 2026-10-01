#!/usr/bin/env python3
"""Extract per-setting option lists from OoTMM's `settings/data.ts` into a slim
committed snapshot (`tools/ootmm_settings.json`) that `gen_data.py` reads offline.

Each enum setting in OoTMM carries `values: [{ value, name }]` - the real choices the
generator offers for that parameter. We snapshot those so the tracker's ROM-settings
editor can present each parameter's actual options instead of a blanket list.

Usage:
    python gen_setting_options.py                 # fetch data.ts from OoTMM master
    python gen_setting_options.py <path/to/data.ts>
    python gen_setting_options.py <ootmm_root>    # reads <root>/packages/core/src/settings/data.ts

Re-run after an OoTMM update (see REGEN.md). Output is stable-sorted so diffs are clean.
"""
import json
import os
import re
import sys
import urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "ootmm_settings.json")
RAW_URL = "https://raw.githubusercontent.com/OoTMM/OoTMM/master/packages/core/src/settings/data.ts"
REL = os.path.join("packages", "core", "src", "settings", "data.ts")


def load_source(arg):
    """Return the text of OoTMM's settings/data.ts from a file, an ootmm root, or the network."""
    if arg:
        if os.path.isfile(arg):
            return open(arg, encoding="utf-8").read()
        cand = os.path.join(arg, REL)
        if os.path.isfile(cand):
            return open(cand, encoding="utf-8").read()
        if arg.startswith("http"):
            with urllib.request.urlopen(arg) as r:
                return r.read().decode("utf-8")
        sys.exit(f"not a data.ts file, an OoTMM root, or a URL: {arg}")
    print(f"fetching {RAW_URL}", file=sys.stderr)
    with urllib.request.urlopen(RAW_URL) as r:
        return r.read().decode("utf-8")


def _clean(label):
    # OoTMM value names occasionally carry a trailing space or an escaped char.
    return label.replace("\\", "").strip()


def extract(ts):
    """key -> {type, default, values:[[value,label],...]} for every setting entry."""
    key_pos = [(m.start(), m.group(1)) for m in re.finditer(r"\bkey:\s*'([^']+)'", ts)]
    out = {}
    for i, (pos, key) in enumerate(key_pos):
        end = key_pos[i + 1][0] if i + 1 < len(key_pos) else len(ts)
        chunk = ts[pos:end]
        tm = re.search(r"\btype:\s*'([^']+)'", chunk)
        typ = tm.group(1) if tm else "?"
        dm = re.search(r"\bdefault:\s*'([^']*)'", chunk)
        default = dm.group(1) if dm else None
        values = []
        vm = re.search(r"\bvalues:\s*\[", chunk)
        if vm:
            for vv in re.finditer(r"value:\s*'([^']*)'\s*,\s*name:\s*'([^']*)'", chunk[vm.end():]):
                values.append([vv.group(1), _clean(vv.group(2))])
        out[key] = {"type": typ, "default": default, "values": values}
    return out


def main():
    ts = load_source(sys.argv[1] if len(sys.argv) > 1 else None)
    data = extract(ts)
    with open(OUT, "w", encoding="utf-8", newline="\n") as f:
        json.dump(data, f, indent=1, sort_keys=True, ensure_ascii=False)
        f.write("\n")
    n_enum = sum(1 for v in data.values() if v["values"])
    print(f"wrote {OUT}: {len(data)} settings ({n_enum} with option lists)")


if __name__ == "__main__":
    main()
