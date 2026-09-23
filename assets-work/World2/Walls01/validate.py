"""Frozen Dungeon identity/material/rig/motion validation using the official converter."""
from pathlib import Path
import hashlib,json,os,subprocess,sys,struct
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parent;REPO=ROOT.parents[2]
sys.path.insert(0,str(REPO/'assets-work/World1/Architecture03'))
from raw_bindings import payload
CONVERTER=os.environ['MU_BMDCONV']

def run(*args,check=True):
    result=subprocess.run([CONVERTER,*map(str,args)],capture_output=True,check=check)
    return result.stdout.decode('utf8','backslashreplace')+result.stderr.decode('utf8','backslashreplace')


def motion_rows(path):
    return [list(map(float,line.split())) for line in path.read_text().split('skeleton\n')[1].split('end')[0].splitlines() if line and not line.startswith('time')]


def audit(name):
    folder=ROOT/name;old=folder/'baseline/smd';new=folder/'validation/new'
    original=payload(folder/'baseline'/f'{name}.bmd');final=payload(folder/'exports'/f'{name}.bmd')
    assert original[:32]==final[:32]
    assert struct.unpack_from('<3h',original,32)==struct.unpack_from('<3h',final,32)==(2,1,1)
    logs=[]
    logs.append(run('validate',new/f'{name}.smd'))
    for root in (old,new):
        text=(root/f'{name}.smd').read_text();assert all(m in text for m in ('deep_wall01.jpg','deep_wall02.jpg'))
        assert text.split('nodes\n')[1].split('end')[0]==(old/f'{name}.smd').read_text().split('nodes\n')[1].split('end')[0]
        action=[line for line in (root/f'{name}.actions.txt').read_bytes().splitlines() if line.startswith(b'action ')]
        assert len(action)==1 and b'lock=0' in action[0]
    a,b=motion_rows(old/f'{name}_a00.smd'),motion_rows(new/f'{name}_a00.smd');assert len(a)==len(b)==1
    difference=max(abs(x-y) for x,y in zip(a[0],b[0]));assert difference<.0001
    for stage,path in (('baseline',old),('candidate',new)):
        skeleton=folder/'validation'/f'{stage}-skeleton.smd'
        skeleton.write_text((path/f'{name}.smd').read_text().split('triangles\n')[0]+'triangles\nend\n')
        run('smd2bmd',skeleton,skeleton.with_suffix('.bmd'),'--manifest',path/f'{name}.actions.txt')
    comparison=run('compare',folder/'validation/baseline-skeleton.bmd',folder/'validation/candidate-skeleton.bmd');assert 'EQUIVALENT' in comparison
    (folder/'validation/skeleton-compare.txt').write_text(comparison)
    (folder/'validation/validate.txt').write_text('\n'.join(logs))
    (folder/'validation/compare.txt').write_text(run('compare',folder/'baseline'/f'{name}.bmd',folder/'exports'/f'{name}.bmd',check=False))
    hashes=json.loads((ROOT/'frozen-textures.json').read_text())
    for path,value in hashes.items():
        assert hashlib.sha256((REPO/'src/bin/Data/Object2'/path).read_bytes()).hexdigest()==value
        assert hashlib.sha256((folder/'exports'/Path(path).name).read_bytes()).hexdigest()==value
    report=dict(status='PASS',raw32byte_name_equal=True,name_hex=original[:32].hex(),mesh_bone_action_counts=[2,1,1],keys=1,maximum_motion_component_delta=difference,texture_frozen=True,sha256=hashlib.sha256((folder/'exports'/f'{name}.bmd').read_bytes()).hexdigest())
    (folder/'validation/summary.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))


for name in os.environ.get('WALL_NAMES','Object01').split(','):audit(name)
