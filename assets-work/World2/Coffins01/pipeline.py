"""Official Dungeon coffin import/export with exact raw model identity."""
from pathlib import Path
import os,sys,subprocess,shutil
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parent;REPO=ROOT.parents[2]
NAMES=os.environ.get('COFFIN_NAMES','Object21,Object22').split(',')
BLENDER=os.environ['BLENDER'];CONVERTER=os.environ['MU_BMDCONV']

def blender(arguments,log,name=None,mode=None):
    env={**os.environ,'PYTHONDONTWRITEBYTECODE':'1'}
    if name:env['COFFIN_NAME']=name
    if mode:env['COFFIN_MODE']=mode
    result=subprocess.run([BLENDER,'-b','--python-exit-code','1','--python-expr','import sys;sys.dont_write_bytecode=True',*map(str,arguments)],capture_output=True,env=env)
    log.write_bytes(result.stdout+result.stderr)
    assert result.returncode==0,f'Blender failed; see {log}'


def prepare(name):
    folder=ROOT/name
    for stage in ('baseline','original'):
        path=folder/stage
        blender(['--python',ROOT/'official_adapter.py','--','--bmd',path/f'{name}.bmd','--data',path,'--out',path/'source.blend','--bmdconv',CONVERTER],path/'import.txt',name,'import')


def export(name):
    folder=ROOT/name
    blender([folder/'source.blend','--python',ROOT/'official_adapter.py','--','--out',folder/'exports'/f'{name}.bmd','--bmdconv',CONVERTER],folder/'validation/export.txt',name,'export')
    subprocess.run([CONVERTER,'bmd2smd',str(folder/'exports'/f'{name}.bmd'),str(folder/'validation/new')],check=True,capture_output=True)
    blender(['--python',ROOT/'official_adapter.py','--','--bmd',folder/'exports'/f'{name}.bmd','--data',folder/'exports','--out',folder/'validation/reimported.blend','--bmdconv',CONVERTER],folder/'validation/reimport.txt',name,'import')


if __name__=='__main__':
    stage=sys.argv[1]
    if stage=='prepare':
        for name in NAMES:prepare(name)
    elif stage=='export':
        for name in NAMES:export(name)
    elif stage=='prototype':
        name='Object22'
        blender(['--python',ROOT/'build_lid.py'],ROOT/name/'validation/build.txt',name)
        export(name)
        env={**os.environ,'COFFIN_NAMES':name}
        subprocess.run([sys.executable,str(ROOT/'validate.py')],env=env,check=True)
        subprocess.run([sys.executable,str(ROOT/'contact_proof.py')],env=env,check=True)
        blender(['--python',ROOT/'audit_source.py'],ROOT/name/'validation/source-audit.txt',name)
        blender(['--python',ROOT/'review.py'],ROOT/name/'review/render.txt',name)
