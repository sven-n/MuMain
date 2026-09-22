"""Restore original archives from the reviewed starting revision without overwriting changes."""
from pathlib import Path
import subprocess
import sys

sys.dont_write_bytecode = True
from config import ASSETS, BASELINE, CONTAINERS, CONVERTER, REPOSITORY, ROOT


def archive(filename, target):
    original = subprocess.check_output(['git','show',BASELINE+':src/bin/Data/Object1/'+filename],cwd=REPOSITORY)
    target.parent.mkdir(parents=True,exist_ok=True)
    if target.exists():
        assert target.read_bytes() == original, 'Original archive differs: '+filename
    else:
        target.write_bytes(original)


def main():
    for name in ASSETS:
        folder = ROOT/name/'original'
        archive(name+'.bmd',folder/(name+'.bmd'))
        result = subprocess.check_output([CONVERTER,'info',str(folder/(name+'.bmd'))],cwd=REPOSITORY,text=True)
        (folder/'info.txt').write_text(result)
        for sub in ('review','exports','validation'):
            (ROOT/name/sub).mkdir(parents=True,exist_ok=True)
    for filename in CONTAINERS.values():
        target = ROOT/'textures/original'/filename
        archive(filename,target)
        subprocess.run([sys.executable,str(REPOSITORY/'tools/mu_texture.py'),'unwrap',str(target)],cwd=REPOSITORY,check=True)


if __name__ == '__main__':
    main()
