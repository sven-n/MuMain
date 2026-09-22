"""Archive the reviewed CartHay cask consumer at an explicit accepted commit."""
import json
from pathlib import Path
import subprocess
import sys
from prepare import HERE,ROOT,BLENDER,CONVERTER


def main():
    revision=sys.argv[1]
    name='Carriage03';folder=HERE/name
    for child in ('original','exports','validation','review'):(folder/child).mkdir(parents=True,exist_ok=True)
    dependency=json.loads((ROOT/'assets-work/World1/coordination/dependency-map.json').read_text())['models'][name]
    raw=subprocess.check_output(['git','show',revision+':'+dependency['path']],cwd=ROOT)
    for child in ('original','exports'):(folder/child/(name+'.bmd')).write_bytes(raw)
    for texture,paths in dependency['textures'].items():
        for path in paths:
            (folder/'original'/Path(path).name).write_bytes(subprocess.check_output(['git','show',revision+':'+path],cwd=ROOT))
    (folder/'dependencies.json').write_text(json.dumps(dependency['textures'],indent=2)+'\n')
    (folder/'placements.json').write_text(json.dumps(dependency['placements'],indent=2)+'\n')
    (folder/'original-revision.txt').write_text(revision+'\n')
    (folder/'original/info.txt').write_text(subprocess.check_output([CONVERTER,'info',str(folder/'original'/(name+'.bmd'))],text=True))
    subprocess.run([BLENDER,'-b','--python-exit-code','1','--python-expr','import sys;sys.dont_write_bytecode=True','--python',str(ROOT/'tools/blender/mu_bmd_import.py'),'--','--bmd',str(folder/'original'/(name+'.bmd')),'--out',str(folder/'original/source.blend'),'--textures',str(HERE/'textures/original'),'--bmdconv',CONVERTER],cwd=ROOT,check=True)


if __name__=='__main__':main()
