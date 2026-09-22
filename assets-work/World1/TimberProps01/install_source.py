"""Install only this batch's owned game files into the assigned source worktree."""
import hashlib
import json
from pathlib import Path
import shutil

ROOT=Path(__file__).resolve().parent
REPOSITORY=ROOT.parents[2]
DATA=REPOSITORY/'src/bin/Data/Object1'
OWNERS={'House02':('House02.bmd','drum.OZJ','steel.OZJ'),
        'TreasureDrum01':('TreasureDrum01.bmd',),
        'StreetLight01':('StreetLight01.bmd','streetlight.OZJ','streetlight_brightness2.OZJ')}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    replacements={DATA/name:ROOT/owner/'exports'/name for owner,names in OWNERS.items() for name in names}
    baseline={path:digest(path) for group in ('World1','Object1') for path in (DATA.parent/group).iterdir() if path.is_file()}
    report_path=ROOT/'source-installation.json'
    previous=json.loads(report_path.read_text()).get('installed',{}) if report_path.exists() else {}
    for owner,names in OWNERS.items():
        for name in names:
            relative=str((DATA/name).relative_to(REPOSITORY))
            allowed=(digest(ROOT/owner/'original'/name),digest(ROOT/owner/'exports'/name),previous.get(relative))
            assert digest(DATA/name) in allowed, name
    assert digest(ROOT/'House02/exports/drum.OZJ')==digest(ROOT/'TreasureDrum01/exports/drum.OZJ')
    for destination,source in replacements.items():
        shutil.copy2(source,destination)
    assert all(digest(path)==sha for path,sha in baseline.items() if path not in replacements)
    report=dict(status='PASS; isolated source worktree only',runtime_written=False,client_verified=False,
                unchanged_world1_object1_files=len(baseline)-len(replacements),
                installed={str(path.relative_to(REPOSITORY)):digest(path) for path in replacements})
    (ROOT/'source-installation.json').write_text(json.dumps(report,indent=2)+'\n')


if __name__=='__main__':
    main()
