"""Archive the full shared-material group with explicit sequential revision provenance."""
import hashlib
import json
import subprocess
import sys
sys.dont_write_bytecode=True
from config import ASSETS,BASELINE,CONTAINERS,CONVERTER,REPOSITORY,ROOT,WELLS_REVISION
WELLS_FILES={'Well01.bmd','well.OZJ','jar_01.OZJ','tub.OZJ'}


def archive(filename,target):
    revision=WELLS_REVISION if filename in WELLS_FILES else BASELINE
    original=subprocess.check_output(['git','show',revision+':src/bin/Data/Object1/'+filename],cwd=REPOSITORY)
    target.parent.mkdir(parents=True,exist_ok=True)
    if target.exists():
        assert target.read_bytes()==original,filename
    else:target.write_bytes(original)
    return dict(source_revision=revision,sha256=hashlib.sha256(original).hexdigest(),archive=str(target.relative_to(ROOT)))


def main():
    provenance={}
    for name in ASSETS:
        folder=ROOT/name/'original'
        provenance[name+'.bmd']=archive(name+'.bmd',folder/(name+'.bmd'))
        info=subprocess.check_output([CONVERTER,'info',str(folder/(name+'.bmd'))],cwd=REPOSITORY,text=True)
        (folder/'info.txt').write_text(info)
        for sub in ('review','exports','validation'):(ROOT/name/sub).mkdir(parents=True,exist_ok=True)
    for filename in CONTAINERS.values():
        target=ROOT/'textures/original'/filename
        provenance[filename]=archive(filename,target)
        subprocess.run([sys.executable,str(REPOSITORY/'tools/mu_texture.py'),'unwrap',str(target)],cwd=REPOSITORY,check=True)
    (ROOT/'original-provenance.json').write_text(json.dumps(provenance,indent=2)+'\n')


if __name__=='__main__':main()
