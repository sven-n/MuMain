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
from config import ASSETS, PRODUCTION_ASSETS, BASELINE, FROZEN, IDENTITIES, KEYS, MATERIALS, OWNED_TEXTURES, REPOSITORY, ROOT

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


def notes_text(name, inventory):
    folder = ROOT / name
    report = json.loads((folder / 'validation/summary.json').read_text())
    source = json.loads((folder / 'validation/blender.json').read_text())
    compatibility = name == 'Tree11'
    provenance = 'ac16ffeb (accepted Trees01)' if compatibility else 'ac0f6dd8 (legacy original)'
    status = ('Compatibility consumer: installed BMD remains byte-identical. Official new roundtrip is retained only in exports/.'
              if compatibility else 'Production export installed into this worktree source Data only.')
    return f"""# {name} — {IDENTITIES[name]}

Owner: ASTRA groundcover/tree worker, branch `codex/lorencia-bare-trees`.
Status: offline validated; client verification pending. {status}
Original model provenance: {provenance}; archive also checked against starting integration HEAD {BASELINE}.
World1 type {inventory['type']}; {len(inventory['placements'])} placements recorded in placements.json.
Full texture dependencies: {inventory['textures']}.
Material/mesh order: {list(MATERIALS[name])}.

Geometry retained deliberately: {source['triangles']} triangles, {len(source['bone_order'])} bones,
{len(MATERIALS[name])} material meshes. Original dead-branch/root silhouettes remain recognizable.
The narrow branches, stump seating platform, contact edges, origin, orientation, footprint and
UV projection already suit their roles. No arbitrary subdivision or extra twig geometry was added.
The shared charcoal bark is rebuilt as a 512x256 diffuse painting. Frozen textures retain their
original dimensions: {source['textures']}. There are no new rendering features or filenames.

Bind bounds before: {report['bounds_before']}.
Bind bounds after: {report['bounds_after']}.

Full BMD comparison (geometry retained, equivalent within Float32 export precision):

```
{(folder / 'validation/compare.txt').read_text().strip()}
```

Exact names, bone order, parents, mesh order, action order, key count ({KEYS[name]}), lock=0,
and rigid bindings are preserved. Skeleton-only compare is EQUIVALENT. Every bind/action local
pose is compared numerically in validation/local-motion.json. Rotations use Rz*Ry*Rx matrices
because equivalent Euler branches may differ by pi. Translation tolerance 0.001 engine units
and matrix tolerance 0.00001; actual maxima reported. All original animation keys are checked,
not just the selected visual frames. SMDs pass converter validation.

validation/uv-material.json checks every corner's original UV, bone and material. Finite UVs,
one UV set, nondegenerate faces and one full-weight real bone per vertex pass. Original bark
repeat UVs outside 0..1 are preserved. Original hierarchy and any unused bones are preserved.

Tree07 uses CreateOperate at src/source/Engine/Object/ZzzObject.cpp:4667; its top platform,
cut-face material, footprint, bounds and bone remain unchanged for seating. tree_04.OZJ is a
frozen dependency shared with BridgeStone01. Tree11's accepted BMD and tree_06.OZT foliage
are frozen; SHA-256 proof is in the batch frozen-files.json and protected-check.json.
Only the shared bark changes visually for that compatibility consumer.

Untouched original BMD, info, source.blend and converter extraction are archived. Packed
source.blend contains original geometry in REF_ORIGINAL and a separate export copy using
the painted bark. This is a texture-art rebuild; there is no high-poly remodeling claim.
Editable/raw PNG painting, final PNG/JPG/OZJ, prompt, tiling preview and wrapper checks are
in ../textures and ../prompts.md. Alpha is not edited in this batch; the accepted conifer
alpha container remains byte-identical. Bark remains opaque RGB.

review/comparison.jpg compares original against officially exported/reimported geometry
with matching cameras and lighting. Tree11 additionally has all 31-key proof and selected
frames 0, 15, 30 in action-comparison.jpg. Its matching renders use the unchanged accepted game BMD with final bark.
The separately validated roundtrip is compatibility evidence only, not an installed replacement. Wireframe and 240px readability previews are included. Terrain
views use completed TileGrass01 with 100-unit repeats and a 190-unit scale proxy at an
invented origin staging position. These are offline Blender previews, not client evidence.

Client lighting, actual world placement occlusion, stump sitting contact and conifer
sway/material appearance remain pending. No runtime or client process was touched.
"""


def preserve_hashes():
    originals = sorted(ROOT.glob('*/original/*.bmd')) + sorted((ROOT/'textures/original').glob('*.OZ*'))
    hashes = {str(path.relative_to(ROOT)): hashlib.sha256(path.read_bytes()).hexdigest() for path in originals}
    for path, digest in hashes.items():
        baseline = subprocess.check_output(['git','show',BASELINE+':src/bin/Data/Object1/'+Path(path).name],cwd=REPOSITORY)
        assert hashlib.sha256(baseline).hexdigest() == digest, path
    (ROOT/'original-hashes.json').write_text(json.dumps(hashes,indent=2)+'\n')


def check_protected():
    frozen = json.loads((ROOT/'frozen-files.json').read_text())
    for filename, digest in frozen.items():
        actual = hashlib.sha256((REPOSITORY/'src/bin/Data/Object1'/filename).read_bytes()).hexdigest()
        assert actual == digest, filename
    (ROOT/'protected-check.json').write_text(json.dumps({'status':'PASS','unchanged_SHA256':frozen},indent=2)+'\n')


def install():
    files = []
    for name in PRODUCTION_ASSETS:
        path = REPOSITORY/f'src/bin/Data/Object1/{name}.bmd'
        shutil.copy2(ROOT/name/f'exports/{name}.bmd',path)
        files.append(path)
    for texture in OWNED_TEXTURES:
        path = REPOSITORY/'src/bin/Data/Object1'/texture
        shutil.copy2(ROOT/'textures/final'/texture,path)
        files.append(path)
    check_protected()
    manifest = {str(path.relative_to(REPOSITORY)):hashlib.sha256(path.read_bytes()).hexdigest() for path in files}
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


if __name__ == '__main__':
    main()
