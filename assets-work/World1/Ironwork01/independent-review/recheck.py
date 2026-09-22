"""Read-only independent validator check of accepted trees and timber exports."""
import hashlib
import json
from pathlib import Path
import subprocess

REPOSITORY=Path(__file__).resolve().parents[4]
OUTPUT=Path(__file__).resolve().parent
CONVERTER=REPOSITORY.parent/'MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv'
GROUPS={'Trees01':('Tree01','Tree02','Tree06','Tree11'),
        'TimberProps01':('House02','TreasureDrum01','StreetLight01')}


def run(arguments,allowed=(0,)):
    result=subprocess.run(list(map(str,arguments)),cwd=REPOSITORY,text=True,capture_output=True)
    assert result.returncode in allowed,result.stdout+result.stderr
    return {'command':list(map(str,arguments)),'returncode':result.returncode,'stdout':result.stdout,'stderr':result.stderr}


reports={}
for group,names in GROUPS.items():
    for name in names:
        folder=REPOSITORY/'assets-work/World1'/group/name
        target=OUTPUT/name
        target.mkdir(exist_ok=True)
        original=folder/'original'/f'{name}.bmd'
        actual=REPOSITORY/'src/bin/Data/Object1'/f'{name}.bmd'
        commands=[run([CONVERTER,'bmd2smd',actual,target])]
        for smd in sorted(target.glob(name+'*.smd')):
            commands.append(run([CONVERTER,'validate',smd,*(['--animation'] if smd.stem!=name else [])]))
        commands.append(run([CONVERTER,'compare',original,actual],allowed=(0,) if group=='Trees01' else (2,)))
        summary=json.loads((folder/'validation/summary.json').read_text())
        hashes=summary.get('exports',summary.get('export_sha256'))
        for filename,sha in hashes.items():
            assert hashlib.sha256((REPOSITORY/'src/bin/Data/Object1'/filename).read_bytes()).hexdigest()==sha
        reports[name]={'status':'PASS','commands':commands,'source_game_hashes_match_validation':True}
(OUTPUT/'checks.json').write_text(json.dumps(reports,indent=2)+'\n')
print('All seven installed models/action SMDs pass independent converter checks; recorded game hashes match.')
