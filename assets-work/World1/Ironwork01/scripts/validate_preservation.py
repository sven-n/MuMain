"""Prove unchanged rigs and protected material geometry using engine-parsed components."""
import hashlib
import json
from pathlib import Path
import subprocess
import sys
sys.dont_write_bytecode=True
from config import CONVERTER,PROPS,ROOT
POSITION_TOLERANCE=.005


def run(*arguments):
    result=subprocess.run([str(CONVERTER),*map(str,arguments)],capture_output=True,text=True)
    if result.returncode:
        raise RuntimeError(result.stdout+result.stderr)
    return result.stdout+result.stderr


def write_component(folder,name,label,texture):
    text=(folder/f'{name}.smd').read_text()
    header,body=text.split('triangles\n')
    lines=body.splitlines()[:-1]
    selected=['\n'.join(lines[i:i+4]) for i in range(0,len(lines),4) if lines[i]==texture]
    (folder/f'{label}.smd').write_text(header+'triangles\n'+'\n'.join(selected)+'\nend\n')
    run('smd2bmd',folder/f'{label}.smd',folder/f'{label}.bmd','--anim',folder/f'{name}_a00.smd')
    return [list(map(float,line.split())) for triangle in selected for line in triangle.splitlines()[1:]]


def component(root,label,texture=None):
    folders=[root/'validation'/state for state in ('original','replacement')]
    groups=[write_component(folder,root.name,label,texture) for folder in folders]
    text=run('compare',folders[0]/f'{label}.bmd',folders[1]/f'{label}.bmd')
    assert 'EQUIVALENT' in text
    (root/'validation'/f'{label}-compare.txt').write_text(text)
    assert len(groups[0])==len(groups[1])
    for old in groups[0]:
        candidates=[new for new in groups[1] if old[0]==new[0] and max(abs(a-b) for a,b in zip(old[1:4],new[1:4])) < POSITION_TOLERANCE]
        assert any(old[7:9]==new[7:9] for new in candidates), (root.name,label,old)
    return {'component':label,'texture':texture,'triangles':len(groups[0])//3,
            'compare':'EQUIVALENT','bindings_and_uvs':'all original corners matched'}


def validate(name):
    root=ROOT/name
    report={'rig':component(root,'skeleton')}
    summary=json.loads((root/'validation/summary.json').read_text())
    for texture in summary['mesh_texture_order']:
        if texture.endswith('.tga'):
            label=Path(texture).stem
            report[label]=component(root,label,texture)
    (root/'validation/preservation.json').write_text(json.dumps(report,indent=2)+'\n')
    print(name,'rig/protected alpha-plane preservation PASS')


for prop in PROPS:
    validate(prop)
