"""Audit retained UVs, assemble offline evidence, document trees and install owned source files."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys

sys.dont_write_bytecode = True
import numpy as np
from PIL import Image, ImageDraw
from config import ASSETS, BASELINE, FROZEN, IDENTITIES, KEYS, MATERIALS, OWNED_TEXTURES, REPOSITORY, ROOT

BACKGROUND = (29,33,34)
CORNER_TOLERANCE = .01  # Sub-centimeter world units; records actual maximum below.
UV_TOLERANCE = .000002


def smd_corners(path):
    lines = path.read_text().split('triangles\n')[1].splitlines()
    return [(lines[index],list(map(float,row.split())))
            for index in range(0,len(lines)-1,4) for row in lines[index+1:index+4]]


def uv_audit(name):
    folder = ROOT/name/'validation'
    old,new = (smd_corners(folder/stage/f'{name}.smd') for stage in ('original','new'))
    maximum_uv, maximum_position = 0,0
    for material,corner in old:
        matches = [row for mat,row in new if mat==material and row[0]==corner[0]
                   and np.max(np.abs(np.array(row[1:4])-corner[1:4])) < CORNER_TOLERANCE]
        matches = [row for row in matches if max(abs(a-b) for a,b in zip(corner[7:9],row[7:9])) < UV_TOLERANCE]
        assert matches, (name,'Missing rigid corner or changed UV/material')
        nearest = min(matches,key=lambda row:np.linalg.norm(np.array(row[1:4])-corner[1:4]))
        maximum_position=max(maximum_position,float(np.linalg.norm(np.array(nearest[1:4])-corner[1:4])))
        maximum_uv=max(maximum_uv,max(abs(a-b) for a,b in zip(corner[7:9],nearest[7:9])))
    result=dict(status='PASS',corners=len(old), maximum_uv_component_difference=maximum_uv,
                maximum_corner_distance=maximum_position,position_tolerance=CORNER_TOLERANCE,
                materials=MATERIALS[name],mesh_order='Original order retained and asserted by converter info',
                binding='Every corner retains matching original rigid bone, material and UV, including legacy repeats')
    (folder/'uv-material.json').write_text(json.dumps(result,indent=2)+'\n')


def panel(path,size):
    image=Image.open(path).convert('RGBA')
    image.thumbnail(size,Image.Resampling.LANCZOS)
    canvas=Image.new('RGB',size,BACKGROUND)
    canvas.paste(image,((size[0]-image.width)//2,(size[1]-image.height)//2),image)
    return canvas


def review_sheet(name):
    folder=ROOT/name/'review'
    sheet=Image.new('RGB',(1440,1120),BACKGROUND)
    draw=ImageDraw.Draw(sheet)
    draw.text((20,12),f'{name} | {IDENTITIES[name]} | OFFLINE - client review pending',fill='white')
    for index,stage in enumerate(('before','after')):
        x=index*720
        draw.text((x+20,38),stage.upper()+' - matching camera',fill='white')
        sheet.paste(panel(folder/f'{stage}.png',(720,460)),(x,58))
        sheet.paste(panel(folder/f'terrain-{stage}.png',(720,420)),(x,525))
        thumb=panel(folder/f'terrain-{stage}.png',(240,160))
        thumb.save(folder/f'readability-{stage}.png')
        sheet.paste(thumb,(x+30,950))
    draw.text((1000,1000),'190-unit proxy / 100-unit terrain repeats',fill='white')
    sheet.save(folder/'comparison.jpg',quality=94)


def action_sheet(name):
    if KEYS[name]==1:
        return
    folder=ROOT/name/'review'
    sheet=Image.new('RGB',(1350,820),BACKGROUND)
    draw=ImageDraw.Draw(sheet)
    draw.text((20,10),f'{name}: preserved sway | matching cameras | OFFLINE, not client',fill='white')
    for row,stage in enumerate(('before','after')):
        for column,frame in enumerate((0,15,30)):
            path=folder/(f'{stage}.png' if frame==0 else f'action-{stage}-{frame:02d}.png')
            sheet.paste(panel(path,(450,360)),(column*450,50+row*385))
            draw.text((column*450+15,35+row*385),f'{stage.upper()} - frame {frame}',fill='white')
    sheet.save(folder/'action-comparison.jpg',quality=94)


def notes_text(name,inventory):
    folder=ROOT/name
    engine=json.loads((folder/'validation/summary.json').read_text())
    blender=json.loads((folder/'validation/blender.json').read_text())
    return f'''# {name} — {IDENTITIES[name]}

Owner: ASTRA groundcover/tree worker, `codex/lorencia-trees`.
Status: exported and validated offline. Client verification pending.
World1 type {inventory['type']}, {len(inventory['placements'])} placements; exact records in `placements.json`.
Game file: `src/bin/Data/Object1/{name}.bmd`.
Complete texture dependencies: {inventory['textures']}.
Original mesh/material order: {list(MATERIALS[name])}.

Geometry retained: {blender['triangles']} triangles, {len(MATERIALS[name])} meshes, {len(blender['bone_order'])} bones.
Surface artwork rebuilt within the original geometry, origins, rigid bindings, footprint,
UV semantics and sway. No subdivision or altered placements/collision was used.
Texture dimensions: {blender['textures']}.
Bind bounds before: {engine['bounds_before']}.
Bind bounds after: {engine['bounds_after']}.

Full converter comparison:

```
{(folder/'validation/compare.txt').read_text().strip()}
```

One original action, {KEYS[name]} keys, lock=0. Names, order, parents, dummy status, action order
and frame indices retained. Every bind/action local key is compared as translation and a
rotation matrix in `validation/local-motion.json`; maximums and tolerances are recorded.
The 0.001-unit local-position tolerance covers Float32 hierarchy round trips and is 50 times
stricter than converter default tolerance. Rotation-matrix tolerance is 0.00001. Numerical
roundtrip deviations are reported honestly; this is semantic equivalence, not binary identity.
Independent skeleton-only BMD compare is EQUIVALENT. Reference and action SMD validation pass.

`validation/uv-material.json` verifies every original corner's material, original rigid bone
and UV, including intentional out-of-range bark repeats. Original material order is asserted
from the exported BMD info. Blender checks finite coordinates, one UV set, nondegenerate
triangles and exactly one valid non-dummy bone per vertex. Original unused dummy bones remain.

Tree01/02 engine collision box and velocity=0.4/scale are unmodified. Original root and trunk
contacts remain. Tree11's shared tree_03.OZJ is frozen and byte-identical in game and deliverable.
Tree06 retains its pale bare snag identity. No filename, alpha convention or render flag changes.

Untouched originals, converter info/SMD and packed original source are in `original/`.
Packed `source.blend` retains original geometry in REF_ORIGINAL and cloned export geometry.
No high-poly mesh is needed for this unchanged low-cost tree/card structure. Painted masters,
editable PNG/JPG/TGA, final wrappers, alpha/fringe audit, tiling previews and imagegen prompts
are in parent `textures/` and `prompts.md`. Final containers pass mu_texture checks.

`review/comparison.jpg` contains matched original/reimported-export views, terrain scale and
240px readability. Animated models additionally include `review/action-comparison.jpg` at
frames0,15,30; all31 keys are audited numerically. Wireframe is retained. Review renders use
completed pilot TileGrass01 at100 units per repeat and a190-unit figure proxy at an invented
origin staging position. These are offline previews, not actual client/placement evidence.

Client alpha sorting, vertex lighting, actual foliage silhouette/contacts and motion under
engine timing remain pending. No shared runtime or client process was touched.
'''


def preserve_hashes():
    originals=sorted(ROOT.glob('*/original/*.bmd'))+sorted((ROOT/'textures/original').glob('*.OZ*'))
    hashes={str(path.relative_to(ROOT)):hashlib.sha256(path.read_bytes()).hexdigest() for path in originals}
    for path,digest in hashes.items():
        filename=Path(path).name
        baseline=subprocess.check_output(['git','show',BASELINE+':src/bin/Data/Object1/'+filename],cwd=REPOSITORY)
        assert hashlib.sha256(baseline).hexdigest()==digest,filename
    (ROOT/'original-hashes.json').write_text(json.dumps(hashes,indent=2)+'\n')


def install():
    files=[]
    for name in ASSETS:
        path=REPOSITORY/f'src/bin/Data/Object1/{name}.bmd'
        shutil.copy2(ROOT/name/f'exports/{name}.bmd',path)
        files.append(path)
    for texture in OWNED_TEXTURES:
        path=REPOSITORY/'src/bin/Data/Object1'/texture
        shutil.copy2(ROOT/'textures/final'/texture,path)
        files.append(path)
    assert (REPOSITORY/'src/bin/Data/Object1'/FROZEN).read_bytes()==(ROOT/'textures/original'/FROZEN).read_bytes()
    manifest={str(path.relative_to(REPOSITORY)):hashlib.sha256(path.read_bytes()).hexdigest() for path in files}
    (ROOT/'installed-source-files.json').write_text(json.dumps(manifest,indent=2)+'\n')


def main():
    preserve_hashes()
    inventory=json.loads((ROOT.parent/'coordination/dependency-map.json').read_text())
    for name in ASSETS:
        uv_audit(name)
        review_sheet(name)
        action_sheet(name)
        (ROOT/name/'placements.json').write_text(json.dumps(inventory['models'][name],indent=2)+'\n')
        (ROOT/name/'notes.md').write_text(notes_text(name,inventory['models'][name]))
    install()


if __name__=='__main__':
    main()
