"""Read-only Lorencia inventory and dependency groups; never modifies game data."""

import hashlib
import importlib.util
import json
from pathlib import Path
import re
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[3]
OUTPUT = Path(__file__).resolve().parent
DATA = ROOT / 'src/bin/Data'
CONVERTER = Path('/Users/webproduktion3/Documents/claude-test-mumain/MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv')
GROUPS = {'Tree': (0, 13), 'Grass': (20, 8), 'Stone': (30, 5),
          'StoneStatue': (40, 3), 'SteelStatue': (43, 1), 'Tomb': (44, 3),
          'FireLight': (50, 2), 'Bonfire': (52, 1), 'DoungeonGate': (55, 1),
          'MerchantAnimal': (56, 2), 'TreasureDrum': (58, 1), 'TreasureChest': (59, 1),
          'Ship': (60, 1), 'SteelWall': (65, 3), 'SteelDoor': (68, 1),
          'StoneWall': (69, 6), 'StoneMuWall': (75, 4), 'Bridge': (80, 1),
          'Fence': (81, 4), 'BridgeStone': (85, 1), 'StreetLight': (90, 1),
          'Cannon': (91, 3), 'Curtain': (95, 1), 'Sign': (96, 2),
          'Carriage': (98, 4), 'Straw': (102, 2), 'Waterspout': (105, 1),
          'Well': (106, 4), 'Hanging': (110, 1), 'Stair': (111, 1),
          'House': (115, 5), 'Tent': (120, 1), 'HouseWall': (121, 6),
          'HouseEtc': (127, 3), 'Light': (130, 3), 'PoseBox': (133, 1),
          'Furniture': (140, 7), 'Candle': (150, 1), 'Beer': (151, 3)}
EXCLUDED = {'Bird01': 'animated fauna; outside static environment scope',
            'Butterfly01': 'animated fauna; outside static environment scope',
            'Fish01': 'animated fauna; outside static environment scope',
            'MerchantAnimal01': 'merchant creature; character/animal scope excluded',
            'MerchantAnimal02': 'merchant creature; character/animal scope excluded',
            'Light01': 'engine light marker; preserve', 'Light02': 'engine light marker; preserve',
            'Light03': 'engine light marker; preserve', 'PoseBox01': 'hidden pose marker; preserve'}


def read_placements():
    spec = importlib.util.spec_from_file_location('world_inventory', OUTPUT.parent / 'inventory.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    raw = module.decode_map((DATA / 'World1/EncTerrain1.obj').read_bytes())
    count = struct.unpack_from('<h', raw, 2)[0]
    assert len(raw) == 4 + 30 * count
    result = {}
    for offset in range(4, len(raw), 30):
        kind, *values = struct.unpack_from('<h7f', raw, offset)
        result.setdefault(kind, []).append({'position': values[:3], 'rotation': values[3:6],
                                           'scale': values[6], 'tile': [v / 100 for v in values[:2]]})
    return result


def texture_paths(texture, files):
    suffix = {'.jpg': '.ozj', '.tga': '.ozt', '.bmp': '.ozb'}[Path(texture).suffix.lower()]
    requested = Path(texture).stem.lower() + suffix
    return [str(p.relative_to(ROOT)) for p in files if p.name.lower() == requested]


def inventory_models():
    ids = {f'{prefix}{i + 1:02}': start + i for prefix, (start, count) in GROUPS.items() for i in range(count)}
    placements = read_placements()
    files = list((DATA / 'Object1').iterdir())
    models, users = {}, {}
    for model in sorted((DATA / 'Object1').glob('*.bmd')):
        info = subprocess.check_output([str(CONVERTER), 'info', str(model)], text=True)
        textures = re.findall(r'texture=(.*)', info)
        dependencies = {t: texture_paths(t, files) for t in textures}
        kind = ids.get(model.stem)
        models[model.stem] = {'path': str(model.relative_to(ROOT)), 'type': kind,
                             'placements': placements.get(kind, []), 'textures': dependencies,
                             'info': info.replace(str(ROOT) + '/', ''), 'scope_exclusion': EXCLUDED.get(model.stem)}
        for paths in dependencies.values():
            for path in paths:
                users.setdefault(path, []).append(model.stem)
    return models, users


def dependency_groups(models, users):
    pending, groups = set(models), []
    while pending:
        frontier, members = {min(pending)}, set()
        while frontier:
            name = frontier.pop()
            if name in members:
                continue
            members.add(name)
            for paths in models[name]['textures'].values():
                for path in paths:
                    frontier.update(set(users[path]) - members)
        pending -= members
        groups.append(sorted(members))
    return sorted(groups, key=lambda g: (-sum(len(models[n]['placements']) for n in g), g))


def source_references():
    result = []
    for path in (ROOT / 'src/source').rglob('*'):
        if path.suffix not in {'.cpp', '.h'}:
            continue
        for line_number, line in enumerate(path.read_text(errors='replace').splitlines(), 1):
            if re.search(r'Object1(?:\\|/|\")', line, re.I):
                result.append({'path': str(path.relative_to(ROOT)), 'line': line_number, 'text': line.strip()})
    return result


def main():
    if (OUTPUT / 'dependency-map.json').exists():
        raise SystemExit('Immutable dependency baseline already exists; use validate_integration.py for current state.')
    models, users = inventory_models()
    result = {'baseline': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
              'models': models, 'texture_consumers': users,
              'dependency_groups': dependency_groups(models, users),
              'source_references': source_references(),
              'missing_textures': [(n, t) for n, m in models.items() for t, paths in m['textures'].items() if not paths]}
    (OUTPUT / 'dependency-map.json').write_text(json.dumps(result, indent=2) + '\n')
    hashes = {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
              for folder in ('World1', 'Object1') for p in (DATA / folder).iterdir() if p.is_file()}
    target = OUTPUT / 'protected-baseline.json'
    if not target.exists():
        target.write_text(json.dumps(hashes, indent=2) + '\n')
    print(f'{len(models)} models, {len(users)} textures, {len(result["dependency_groups"])} dependency groups')
    for group in result['dependency_groups']:
        print(sum(len(models[n]['placements']) for n in group), ', '.join(group))
    print('Missing textures:', result['missing_textures'])


if __name__ == '__main__':
    main()
