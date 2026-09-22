"""Install only this batch's validated exports into its own worktree source Data."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
sys.dont_write_bytecode = True
from config import PROPS, REPOSITORY, ROOT, ROCKS


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def protected_snapshot(claimed):
    data=REPOSITORY/'src/bin/Data'
    return {str(path.relative_to(data)):digest(path) for folder in ('Object1','World1')
            for path in (data/folder).iterdir() if path.is_file() and path.name not in claimed}


def main():
    branch=subprocess.check_output(['git','branch','--show-current'],cwd=REPOSITORY,text=True).strip()
    assert branch=='codex/lorencia-rocks',branch
    claimed={f'{name}.bmd' for name in ROCKS}|{'ston01.OZJ','ston02.OZT'}
    destination=REPOSITORY/'src/bin/Data/Object1'
    before=protected_snapshot(claimed)
    exports={}
    for name in PROPS:
        report=json.loads((ROOT/name/'validation/summary.json').read_text())
        assert report['status']=='PASS OFFLINE'
        path=ROOT/name/'exports'/f'{name}.bmd'
        assert digest(path)==report['export_sha256'][path.name]
        if name in ROCKS:
            exports[path.name]=path
    for filename in ('ston01.OZJ','ston02.OZT'):
        exports[filename]=ROOT/'textures'/filename
    frozen={}
    for filename in ('Waterspout01.bmd','stone_statue02.OZJ','ston02.OZJ','reagon_waterspout.OZJ'):
        original=ROOT/'Waterspout01/original'/filename if filename.endswith('.bmd') else ROOT/'original'/filename
        assert digest(destination/filename)==digest(original),filename
        frozen[filename]=digest(original)
    for filename,source in exports.items():
        if filename.endswith('.bmd'):
            original=ROOT/Path(filename).stem/'original'/filename
        else:
            original=ROOT/'original'/filename
        assert digest(destination/filename) in (digest(original),digest(source)),filename
    for filename,source in exports.items():
        shutil.copy2(source,destination/filename)
    assert before==protected_snapshot(claimed)
    report={'branch':branch,'runtime_written':False,'client_verified':False,
            'frozen_fountain_files':frozen,
            'protected_file_count':len(before),'protected_hashes':before,
            'installed':{str((destination/name).relative_to(REPOSITORY)):digest(path) for name,path in exports.items()}}
    (ROOT/'installed-files.json').write_text(json.dumps(report,indent=2)+'\n')
    print('Installed',len(exports),'owned files;',len(before),'protected Object1/World1 files unchanged')


if __name__=='__main__':
    main()
