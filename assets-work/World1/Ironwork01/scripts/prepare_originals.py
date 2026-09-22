"""Archive actual originals, all dependencies, placement records and converter metadata."""
import json
from pathlib import Path
import shutil
import subprocess
import sys
sys.dont_write_bytecode=True
from config import CONVERTER, PROPS, REPOSITORY, ROOT


def run(arguments):
    result=subprocess.run(list(map(str,arguments)),cwd=REPOSITORY,capture_output=True,text=True,check=True)
    return result.stdout


def preserve(source,target):
    if target.exists():
        assert source.read_bytes()==target.read_bytes(),target
        return
    shutil.copy2(source,target)


inventory=json.loads((ROOT.parent/'coordination/dependency-map.json').read_text())
models={name:inventory['models'][name] for name in PROPS}
textures={path for model in models.values() for paths in model['textures'].values() for path in paths}
for path in textures:
    target=ROOT/'original'/Path(path).name
    preserve(REPOSITORY/path,target)
    run([sys.executable,REPOSITORY/'tools/mu_texture.py','unwrap',target])
for name,model in models.items():
    folder=ROOT/name/'original'
    folder.mkdir(parents=True,exist_ok=True)
    preserve(REPOSITORY/model['path'],folder/f'{name}.bmd')
    (folder/'info.txt').write_text(run([CONVERTER,'info',folder/f'{name}.bmd']))
    (folder/'placements.json').write_text(json.dumps(model['placements'],indent=2)+'\n')
    run([CONVERTER,'bmd2smd',folder/f'{name}.bmd',folder/'smd'])
    for image,paths in model['textures'].items():
        for filename in (image,*(Path(path).name for path in paths)):
            preserve(ROOT/'original'/filename,folder/filename)
consumers={path:inventory['texture_consumers'][path] for path in textures}
assert all(set(names)<=set(PROPS) for names in consumers.values())
(ROOT/'dependency-inventory.json').write_text(json.dumps({'models':models,'texture_consumers':consumers},indent=2)+'\n')
print('Archived',len(models),'models;',sum(len(m['placements']) for m in models.values()),'placements; all consumers owned')
