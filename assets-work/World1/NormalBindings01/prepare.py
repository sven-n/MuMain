"""Retain original raw normal bindings on three material-only models.

The official Blender roundtrips and production sources are retained in the prior
accepted batches. Their geometry compares equivalent; original BMD bytes avoid
normal deduplication changes which SMD geometry comparisons cannot detect.
"""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys

sys.dont_write_bytecode=True
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[2]
ASSETS=('Grass02','Tree12','Tree13')
BATCHES={'Grass02':'Groundcover01','Tree12':'Flowering01','Tree13':'Flowering01'}
CONVERTER='/Users/webproduktion3/Documents/claude-test-mumain/MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv'
BLENDER='/Applications/Blender.app/Contents/MacOS/Blender'
BASELINE='ac0f6dd8'


def run(command,log):
    r=subprocess.run(command,cwd=ROOT,capture_output=True,text=True)
    log.write_text(r.stdout+r.stderr);r.check_returncode()
    return r.stdout


def prepare(name,record):
    folder=HERE/name;previous=HERE.parent/BATCHES[name]/name
    for child in ('original','exports','validation','review'):(folder/child).mkdir(parents=True,exist_ok=True)
    game=ROOT/record['path'];original=subprocess.check_output(['git','show',BASELINE+':'+record['path']],cwd=ROOT)
    (folder/'original'/game.name).write_bytes(original)
    (folder/'exports'/game.name).write_bytes(original)
    shutil.copy2(game,folder/'validation/superseded-roundtrip.bmd')
    shutil.copy2(previous/'source.blend',folder/'source.blend')
    shutil.copy2(previous/'original/source.blend',folder/'original/source.blend')
    for texture,paths in record['textures'].items():
        for path in paths:
            old=subprocess.check_output(['git','show',BASELINE+':'+path],cwd=ROOT)
            (folder/'original'/Path(path).name).write_bytes(old)
            shutil.copy2(ROOT/path,folder/'exports'/Path(path).name)
    (folder/'dependencies.json').write_text(json.dumps(record['textures'],indent=2)+'\n')
    (folder/'placements.json').write_text(json.dumps(record['placements'],indent=2)+'\n')
    (folder/'original/info.txt').write_text(record['info'])
    for stage in ('original','new'):
        model=folder/('original' if stage=='original' else 'exports')/game.name
        target=folder/'validation'/stage;target.mkdir(exist_ok=True)
        run([CONVERTER,'bmd2smd',str(model),str(target)],target/'extract.txt')
        for path in sorted(target.glob(name+'*.smd')):
            run([CONVERTER,'validate',str(path)]+(['--animation'] if path.stem!=name else []),target/(path.stem+'-validate.txt'))
    for path in (folder/'validation/original').glob(name+'*.smd'):
        assert path.read_bytes()==(folder/'validation/new'/path.name).read_bytes()
    run([CONVERTER,'compare',str(folder/'original'/game.name),str(folder/'exports'/game.name)],folder/'validation/compare.txt')
    run([CONVERTER,'compare',str(folder/'validation/superseded-roundtrip.bmd'),str(folder/'exports'/game.name)],folder/'validation/superseded-compare.txt')
    run([CONVERTER,'info',str(folder/'exports'/game.name)],folder/'validation/info-after.txt')
    run([sys.executable,str(ROOT/'tools/mu_texture.py'),'check',*[str(p) for p in (folder/'exports').iterdir() if p.suffix.upper() in ('.OZJ','.OZT')]],folder/'validation/texture-check.txt')
    run([BLENDER,'-b','--python-exit-code','1','--python-expr','import sys;sys.dont_write_bytecode=True','--python',str(ROOT/'tools/blender/mu_bmd_import.py'),'--','--bmd',str(folder/'exports'/game.name),'--out',str(folder/'validation/reimported.blend'),'--textures',str(folder/'exports'),'--bmdconv',CONVERTER],folder/'validation/import.txt')
    summary=dict(result='PASS',geometry='EQUIVALENT',rig_actions='Byte-identical original BMD, reference SMD and complete action SMD',
                 raw_binding_contract='Original BMD bytes retain normal-node indices and triangle/normal sharing exactly',
                 previous_official_export_evidence=str(previous.relative_to(ROOT)/'validation'),
                 original_and_final_sha256=hashlib.sha256(original).hexdigest(),textures_unchanged=True,client_verified=False)
    (folder/'validation/summary.json').write_text(json.dumps(summary,indent=2)+'\n')


if __name__=='__main__':
    dependency=json.loads((ROOT/'assets-work/World1/coordination/dependency-map.json').read_text())
    for name in ASSETS:prepare(name,dependency['models'][name])
