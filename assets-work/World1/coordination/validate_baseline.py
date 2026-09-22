"""Read-only engine validation of every in-scope integration-baseline model and action."""

import json
from pathlib import Path
import subprocess

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
BASELINE = 'ac0f6dd8'
CONVERTER = '/Users/webproduktion3/Documents/claude-test-mumain/MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv'


def validate_model(name, record):
    folder = HERE / 'baseline-validation' / name
    folder.mkdir(parents=True, exist_ok=True)
    original = folder / (name + '.bmd')
    original.write_bytes(subprocess.check_output(['git', 'show', BASELINE + ':' + record['path']], cwd=ROOT))
    converted = subprocess.run([CONVERTER, 'bmd2smd', str(original), str(folder / 'smd')], capture_output=True, text=True)
    results = [{'command': 'bmd2smd', 'exit_code': converted.returncode, 'output': converted.stdout + converted.stderr}]
    if converted.returncode:
        return {'status': 'needs inspection', 'commands': results}
    for smd in sorted((folder / 'smd').glob('*.smd')):
        arguments = [CONVERTER, 'validate', str(smd)]
        if smd.stem != name:
            arguments.append('--animation')
        result = subprocess.run(arguments, capture_output=True, text=True)
        results.append({'command': 'validate ' + smd.name + (' --animation' if smd.stem != name else ''), 'exit_code': result.returncode,
                        'output': result.stdout + result.stderr})
    status = 'PASS' if all(r['exit_code'] == 0 for r in results) else 'needs inspection'
    (folder / 'results.json').write_text(json.dumps(results, indent=2) + '\n')
    return {'status': status, 'commands': results}


def main():
    data = json.loads((HERE / 'dependency-map.json').read_text())
    results = {name: validate_model(name, record) for name, record in data['models'].items() if not record['scope_exclusion']}
    report = {'baseline': BASELINE, 'client_verified': False, 'models': results}
    (HERE / 'baseline-validation.json').write_text(json.dumps(report, indent=2) + '\n')
    failures = [name for name, result in results.items() if result['status'] != 'PASS']
    print(f'{len(results)} baseline models inspected; {len(failures)} need inspection: {failures}')


if __name__ == '__main__':
    main()
