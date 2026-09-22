"""Verify only accepted asset files changed and the combined material graph still resolves."""

import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
CONVERTER = Path('/Users/webproduktion3/Documents/claude-test-mumain/MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv')
BASELINE = 'ac0f6dd8'
ALLOWED_DOCS = {'docs/agents/HANDOFF.md', 'docs/agents/WORKLOG.md'}


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def check_scope(ledger):
    expected = {path for batch in ledger for path in batch['game_files']}
    baseline = json.loads((HERE / 'protected-baseline.json').read_text())
    changed = {p for p, digest in baseline.items() if sha256(ROOT / p) != digest}
    if changed != expected:
        raise ValueError(f'Unexpected/missing game changes: {changed ^ expected}')
    paths = subprocess.check_output(['git', 'diff', '--name-only', BASELINE], cwd=ROOT, text=True).splitlines()
    forbidden = [p for p in paths if not (p.startswith('assets-work/World1/') or p in ALLOWED_DOCS or p in expected)]
    if forbidden:
        raise ValueError(f'Forbidden edits: {forbidden}')
    protected = {p: digest for p, digest in baseline.items() if p not in expected}
    return sorted(changed), protected


def check_exports(ledger):
    result = {}
    for batch in ledger:
        for game, exported in batch['game_files'].items():
            actual, expected = sha256(ROOT / game), sha256(ROOT / exported)
            if actual != expected:
                raise ValueError(f'Installed file does not match reviewed export: {game}')
            result[game] = {'sha256': actual, 'export': exported, 'batch': batch['name']}
    return result



def check_retained(ledger):
    baseline = json.loads((HERE / 'protected-baseline.json').read_text())
    result = {}
    for batch in ledger:
        for game, exported in batch.get('retained_game_files', {}).items():
            actual = sha256(ROOT / game)
            if actual != baseline[game] or actual != sha256(ROOT / exported):
                raise ValueError(f'Retained original BMD changed: {game}')
            result[game] = {'sha256': actual, 'export': exported, 'batch': batch['name']}
    return result


def check_originals(ledger):
    result = {}
    for batch in ledger:
        for name in batch['models']:
            path = ROOT / 'assets-work/World1' / batch['name'] / name / 'original' / (name + '.bmd')
            relative_game = 'src/bin/Data/Object1/' + name + '.bmd'
            revision = batch.get('original_revisions', {}).get(name, BASELINE)
            original = subprocess.check_output(['git', 'show', revision + ':' + relative_game], cwd=ROOT)
            if not path.exists() or path.read_bytes() != original:
                raise ValueError(f'Original archive differs from integration baseline: {name}')
            result[batch['name'] + '/' + name] = {'revision': revision, 'sha256': sha256(path)}
    return result


def check_dependencies(ledger):
    baseline = json.loads((HERE / 'dependency-map.json').read_text())
    for batch in ledger:
        for path in batch['game_files']:
            if Path(path).suffix.lower() not in {'.ozj', '.ozt'}:
                continue
            consumers = set(baseline['texture_consumers'].get(path, []))
            if consumers - set(batch['models']):
                raise ValueError(f'Shared material changed outside owner batch: {path}: {consumers}')
    containers = {p.name.lower(): p for p in (ROOT / 'src/bin/Data/Object1').iterdir()}
    missing, info = [], {}
    for model in sorted((ROOT / 'src/bin/Data/Object1').glob('*.bmd')):
        report = subprocess.check_output([str(CONVERTER), 'info', str(model)], text=True)
        info[model.stem] = report.replace(str(ROOT) + '/', '')
        original_info = baseline['models'][model.stem]['info']
        for pattern in (r'texture=(.*)', r'bone \d+: .*', r'action \d+: .*'):
            if re.findall(pattern, report) != re.findall(pattern, original_info):
                raise ValueError(f'Mesh material order or rig/action metadata changed: {model.stem}: {pattern}')
        for texture in re.findall(r'texture=(.*)', report):
            suffix = {'.jpg': '.ozj', '.tga': '.ozt'}[Path(texture).suffix.lower()]
            if Path(texture).stem.lower() + suffix not in containers:
                missing.append((model.stem, texture))
    if missing:
        raise ValueError(f'Missing materials in combined set: {missing}')
    return info


def check_textures(files):
    textures = [str(ROOT / p) for p in files if Path(p).suffix.lower() in {'.ozj', '.ozt'}]
    if not textures:
        return ''
    return subprocess.check_output([sys.executable, str(ROOT / 'tools/mu_texture.py'), 'check', *textures], text=True)


def main():
    ledger = json.loads((HERE / 'integration-ledger.json').read_text())
    changed, protected = check_scope(ledger)
    exports = check_exports(ledger)
    retained = check_retained(ledger)
    originals = check_originals(ledger)
    info = check_dependencies(ledger)
    texture_report = check_textures(changed)
    report = {'result': 'PASS: offline integration; no client claim',
              'integration_head': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
              'baseline': BASELINE, 'changed_game_files': exports, 'retained_game_files': retained, 'protected_file_count': len(protected),
              'world1_files_unchanged': all(p.startswith('src/bin/Data/Object1/') for p in changed),
              'original_archive_hashes': originals, 'combined_models': len(info), 'missing_textures': [], 'texture_loader_checks': texture_report,
              'client_verified': False, 'runtime_modified': False}
    (HERE / 'combined-validation.json').write_text(json.dumps(report, indent=2) + '\n')
    (HERE / 'combined-model-info.json').write_text(json.dumps(info, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
