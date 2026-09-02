#!/usr/bin/env python3
"""
Compute the OoTMM XflagID -> check identity mapping (new xflag system, ROM > v32.3 / dev) by
replaying packages/data/build/checks.ts, then stamp the resulting XflagID onto the tracker's
object pool CSVs (matched by Location).

XflagID is a global running counter (OoT + MM in one pass) assigned to every <xflag> element
(scene-level or inside <actor>), walking data/checks/**/*.xml sorted by relative posix path, in
document order. Run against the SAME OoTMM checkout used to build the ROM you play:

    python gen_xflags.py <ootmm_root> --stamp-csv Objects/pool_oot.csv Objects/pool_mm.csv

then re-run gen_objects.py --emit to regenerate OoTObjectScene.cpp / MMObjectScene.cpp.
Add --csv <path> to also dump a full audit table.

<ootmm_root> may be either a LOCAL folder (the OoTMM repo root, holding data/checks and
data/defs/scenes.yml) OR a GitHub reference, which is downloaded once and cached under the temp
folder (only the data/ subtree is extracted):

    python gen_xflags.py https://github.com/OoTMM/OoTMM/tree/master --stamp-csv ...
    python gen_xflags.py OoTMM/OoTMM            --stamp-csv ...   # defaults to the master branch
    python gen_xflags.py OoTMM/OoTMM@v14.0      --stamp-csv ...   # a tag / branch / commit

NB: pick the ref matching YOUR ROM's OoTMM version -- master's HEAD can have XflagIDs that differ
from an older ROM. To force a fresh download, delete the printed cache folder.
"""
import sys, os, glob, io, re, tarfile, tempfile, shutil, urllib.request
import xml.etree.ElementTree as ET

def load_scenes(scenes_yml):
    scenes = {}
    with open(scenes_yml, encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#') or ':' not in line:
                continue
            name, val = line.split(':', 1)
            scenes[name.strip()] = int(val.strip(), 0)
    return scenes

def game_id(game, name, sep='_'):
    for p in ('OOT', 'MM', 'SHARED'):
        if name.startswith(p + sep):
            return name
    return game.upper() + sep + name

def i0(x):
    return int(x, 0)

def enrich(game, old, attrs, scenes):
    ctx = dict(old)
    if 'scene' in attrs:
        sc = attrs['scene']
        if sc == 'NONE':
            ctx['scene'] = 'NONE'
            ctx['sceneId'] = -1
        else:
            ctx['scene'] = game_id(game, sc)
            ctx['sceneId'] = scenes[ctx['scene']]
    if 'setup' in attrs: ctx['setupId'] = i0(attrs['setup'])
    if 'room'  in attrs: ctx['roomId']  = i0(attrs['room'])
    if 'slice' in attrs: ctx['sliceId'] = i0(attrs['slice'])
    if 'actor' in attrs: ctx['actorId'] = i0(attrs['actor'])
    return ctx

class Builder:
    def __init__(self, scenes):
        self.scenes = scenes
        self.next_id = 0
        self.rows = []

    def record(self, game, ctx, attrs):
        xid = self.next_id
        self.next_id += 1
        self.rows.append({
            'id': xid,
            'game': game,
            'sceneId': ctx['sceneId'],
            'setupId': ctx.get('setupId', 0),
            'roomId':  ctx.get('roomId', 0),
            'sliceId': ctx.get('sliceId', 0),
            'actorId': ctx.get('actorId', 0),
            # Prefixed like the tracker's Location column ("OOT ..." / "MM ...").
            'location': game_id(game, attrs.get('location', ''), ' '),
        })

    def walk_scene(self, game, scene):
        init = {'scene': scene.attrib['id']}
        if 'room' in scene.attrib: init['room'] = scene.attrib['room']
        ctx = enrich(game, {}, init, self.scenes)
        for child in scene:
            if child.tag == 'actor':
                actor_ctx = enrich(game, ctx, child.attrib, self.scenes)
                for gc in child:
                    if gc.tag == 'xflag':
                        self.record(game, enrich(game, actor_ctx, gc.attrib, self.scenes), gc.attrib)
                    elif gc.tag == 'match':
                        pass  # match keys only steer in-game resolution, not check identity
                    else:
                        raise SystemExit(f"unexpected <{gc.tag}> in <actor>")
            elif child.tag == 'xflag':
                self.record(game, enrich(game, ctx, child.attrib, self.scenes), child.attrib)
            # chest / collectible / npc / ... consume no XflagID

    def process_file(self, path):
        root = ET.parse(path).getroot()
        if root.tag != 'checks':
            raise SystemExit(f"{path}: root is <{root.tag}> not <checks>")
        game = root.attrib['game']
        for scene in root:
            if scene.tag == 'scene':
                self.walk_scene(game, scene)
            else:
                raise SystemExit(f"{path}: unexpected <{scene.tag}> under <checks>")

def _parse_github(spec):
    """(owner, repo, ref) from 'https://github.com/OWNER/REPO[/tree/REF]', 'OWNER/REPO' or
    'OWNER/REPO@REF'. Defaults ref to 'master'. Returns None if `spec` isn't a GitHub reference."""
    s = spec.strip()
    if s.startswith("http://") or s.startswith("https://"):
        if "github.com/" not in s:
            return None
        path = s.split("github.com/", 1)[1]
    elif re.fullmatch(r"[\w.-]+/[\w.@/-]+", s):   # bare "owner/repo" (no drive letter / backslash)
        path = s
    else:
        return None
    parts = path.rstrip("/").split("/")
    if len(parts) < 2:
        return None
    owner, repo, ref = parts[0], parts[1], "master"
    if "@" in repo:
        repo, ref = repo.split("@", 1)
    elif len(parts) >= 4 and parts[2] == "tree":
        ref = "/".join(parts[3:])                 # branch names may contain '/'
    if repo.endswith(".git"):
        repo = repo[:-4]
    return owner, repo, ref


def _fetch_github(owner, repo, ref):
    """Download OWNER/REPO@REF's source tarball once, extract only its data/ subtree to a temp
    cache, and return that folder (usable as <ootmm_root>). Reuses the cache on later runs."""
    cache = os.path.join(tempfile.gettempdir(), "ootmm_checks_cache",
                         f"{owner}_{repo}_{ref.replace('/', '_')}")
    if os.path.isdir(os.path.join(cache, "data", "checks")):
        print(f"  using cached OoTMM data: {cache}")
        return cache
    url = f"https://github.com/{owner}/{repo}/archive/{ref}.tar.gz"
    print(f"  downloading {url}\n  (full source tarball; only data/ is kept) ...")
    try:
        req = urllib.request.Request(url, headers={"User-Agent": "gen_xflags"})
        with urllib.request.urlopen(req, timeout=180) as resp:
            blob = resp.read()
    except Exception as e:
        raise SystemExit(f"  download failed ({e}). Check the ref, or use a local checkout instead.")
    os.makedirs(cache, exist_ok=True)
    root = os.path.normpath(cache)
    n = 0
    with tarfile.open(fileobj=io.BytesIO(blob), mode="r:gz") as tar:
        for m in tar.getmembers():
            top, _, rel = m.name.partition("/")   # strip the "REPO-REF/" top folder
            if not m.isfile() or not (rel == "data" or rel.startswith("data/")):
                continue
            dest = os.path.normpath(os.path.join(cache, rel))
            if os.path.commonpath([dest, root]) != root:
                continue                          # tarbomb / path-traversal guard
            os.makedirs(os.path.dirname(dest), exist_ok=True)
            with tar.extractfile(m) as src, open(dest, "wb") as out:
                shutil.copyfileobj(src, out)
            n += 1
    if not os.path.isdir(os.path.join(cache, "data", "checks")):
        shutil.rmtree(cache, ignore_errors=True)
        raise SystemExit(f"  '{ref}' has no data/checks (wrong ref?). Extracted {n} data/ file(s).")
    print(f"  extracted {n} file(s) from data/ -> {cache}")
    return cache


def resolve_root(spec):
    """Turn <ootmm_root> into a local folder holding data/checks + data/defs/scenes.yml. Accepts a
    local path (used as-is) or a GitHub URL / owner-repo (downloaded & cached, see _fetch_github)."""
    if os.path.isdir(spec):
        if os.path.isdir(os.path.join(spec, "data", "checks")):
            return spec
        raise SystemExit(f"'{spec}' has no data/checks/ subfolder. Point at the OoTMM repo root "
                         "(the folder containing data/checks and data/defs/scenes.yml).")
    gh = _parse_github(spec)
    if gh is None:
        raise SystemExit(f"'{spec}' is neither a local OoTMM checkout nor a GitHub URL/owner-repo.")
    return _fetch_github(*gh)


def build_rows(root_dir):
    root_dir = resolve_root(root_dir)
    checks_dir = os.path.join(root_dir, 'data', 'checks')
    scenes = load_scenes(os.path.join(root_dir, 'data', 'defs', 'scenes.yml'))
    files = glob.glob(os.path.join(checks_dir, '**', '*.xml'), recursive=True)
    files.sort(key=lambda p: os.path.relpath(p, checks_dir).replace('\\', '/'))
    b = Builder(scenes)
    for f in files:
        b.process_file(f)
    return b.rows

def stamp_csv(path, loc_to_id):
    """Append (or overwrite) an `xflag_id` column matched by the Location column (index 3)."""
    with open(path, 'rb') as f:
        raw = f.read()
    newline = b'\r\n' if b'\r\n' in raw else b'\n'
    trailing = raw.endswith(newline)
    text = raw.decode('utf-8')
    lines = text.split(newline.decode())
    if trailing and lines and lines[-1] == '':
        lines.pop()

    header = lines[0].split(';')
    if 'xflag_id' in header:
        idx = header.index('xflag_id')
    else:
        idx = None
    out = []
    matched = 0
    for i, line in enumerate(lines):
        cols = line.split(';')
        if i == 0:
            if idx is None:
                cols.append('xflag_id')
            out.append(';'.join(cols))
            continue
        loc = cols[3]
        xid = loc_to_id.get(loc)
        val = ('0x%04X' % xid) if xid is not None else '0xFFFF'
        if xid is not None:
            matched += 1
        if idx is None:
            cols.append(val)
        else:
            cols[idx] = val
        out.append(';'.join(cols))

    body = newline.decode().join(out)
    if trailing:
        body += newline.decode()
    with open(path, 'wb') as f:
        f.write(body.encode('utf-8'))
    print(f"  {os.path.basename(path)}: {matched}/{len(out)-1} rows stamped with an XflagID")

def main():
    root_dir = sys.argv[1]
    rows = build_rows(root_dir)
    oot = sum(1 for r in rows if r['game'] == 'oot')
    mm  = sum(1 for r in rows if r['game'] == 'mm')
    print(f"xflags: {len(rows)} (oot={oot} mm={mm}), max XflagID 0x{len(rows)-1:04x}")

    loc_to_id = {r['location']: r['id'] for r in rows}
    if len(loc_to_id) != len(rows):
        print(f"WARNING: {len(rows) - len(loc_to_id)} duplicate location strings")

    args = sys.argv[2:]
    i = 0
    while i < len(args):
        if args[i] == '--stamp-csv':
            i += 1
            while i < len(args) and not args[i].startswith('--'):
                stamp_csv(args[i], loc_to_id)
                i += 1
        elif args[i] == '--csv':
            with open(args[i + 1], 'w', encoding='utf-8') as f:
                f.write("xflagId,game,sceneId,setupId,roomId,sliceId,actorId,location\n")
                for r in rows:
                    g = 0 if r['game'] == 'oot' else 1
                    f.write(f"{r['id']},{g},{r['sceneId']},{r['setupId']},{r['roomId']},"
                            f"{r['sliceId']},{r['actorId']},{r['location']}\n")
            print(f"wrote {args[i + 1]}")
            i += 2
        else:
            i += 1

if __name__ == '__main__':
    main()
