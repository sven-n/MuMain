"""Write the asset handoff, comparison sheets and owned source-game files."""
from pathlib import Path
import hashlib
import json
import shutil

import numpy as np
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
ASSETS = ('Grass01', 'Grass02', 'Grass05', 'Grass06')
TEXTURES = {'Grass01':'tree_08','Grass02':'tree_08','Grass05':'tree_09','Grass06':'tree_09'}
IDENTITIES = {'Grass01':'wide low ivy/bramble leaf mat', 'Grass02':'spread low ivy/bramble leaf mat',
              'Grass05':'four bent broad-leaf ground clumps', 'Grass06':'narrow low broad-leaf cluster'}
BACKGROUND = (29, 33, 34)


def smd_corners(path):
    text = path.read_text().split('triangles\n')[1].splitlines()
    return [(text[index], list(map(float, row.split())))
            for index in range(0,len(text)-1,4) for row in text[index+1:index+4]]


def uv_audit(name):
    folder = ROOT / name / 'validation'
    old = smd_corners(folder / f'original/{name}.smd')
    new = smd_corners(folder / f'new/{name}.smd')
    maximum_uv = 0
    for material, corner in old:
        matches = [row for mat,row in new if mat == material and row[0] == corner[0]
                   and np.max(np.abs(np.array(row[1:4])-corner[1:4])) < .001]
        assert matches, 'Missing rigid-bound corner'
        difference = min(max(abs(a-b) for a,b in zip(corner[7:9],row[7:9])) for row in matches)
        assert difference < .000002
        maximum_uv = max(maximum_uv, difference)
    result = dict(status='PASS', corners=len(old), maximum_uv_component_difference=maximum_uv,
                  material=f'{TEXTURES[name]}.tga', mesh_order='Single original material/mesh retained',
                  binding='Every original corner retains matching bone, position and UV')
    (folder / 'uv-material.json').write_text(json.dumps(result,indent=2)+'\n')


def panel(image_path, size):
    image = Image.open(image_path).convert('RGBA')
    image.thumbnail(size, Image.Resampling.LANCZOS)
    canvas = Image.new('RGB',size,BACKGROUND)
    canvas.paste(image,((size[0]-image.width)//2,(size[1]-image.height)//2),image)
    return canvas


def review_sheet(name):
    folder = ROOT / name / 'review'
    sheet = Image.new('RGB',(1440,1100),BACKGROUND)
    draw = ImageDraw.Draw(sheet)
    draw.text((20,12),f'{name} | {IDENTITIES[name]} | OFFLINE - client review pending',fill='white')
    for index,stage in enumerate(('before','after')):
        x=index*720
        draw.text((x+20,38),stage.upper()+' - matching camera',fill='white')
        sheet.paste(panel(folder/f'{stage}.png',(720,460)),(x,58))
        sheet.paste(panel(folder/f'terrain-{stage}.png',(720,420)),(x,525))
        thumb=panel(folder/f'terrain-{stage}.png',(240,160))
        thumb.save(folder/f'readability-{stage}.png')
        sheet.paste(thumb,(x+30,940))
    draw.text((1000,980),'190-unit proxy; 100-unit terrain repeats',fill='white')
    sheet.save(folder/'comparison.jpg',quality=94)


def write_notes(name, inventory):
    folder = ROOT / name
    result = json.loads((folder/'validation/summary.json').read_text())
    blender = json.loads((folder/'validation/blender.json').read_text())
    texture=TEXTURES[name]
    (folder/'placements.json').write_text(json.dumps(inventory,indent=2)+'\n')
    text=f'''# {name} — {IDENTITIES[name]}

Status: exported and validated offline. Client verification pending; no client was operated.
Owner: ASTRA groundcover worker, branch `codex/lorencia-groundcover`.

World1 model type {inventory['type']}; {len(inventory['placements'])} actual placements.
The exact placement records, tile coordinates, scales and rotations are in `placements.json`.
Game BMD: `src/bin/Data/Object1/{name}.bmd`.
Complete texture dependency: `src/bin/Data/Object1/{texture}.OZT` (material `{texture}.tga`).

The low-cost original vegetation cards, footprint, pivot and orientation are retained. This
is a texture art rebuild, with no additional triangles. Original reference meshes remain in
`REF_ORIGINAL` inside packed `source.blend`; untouched BMD, converter info, SMD and packed
original source remain in `original/`. A high-poly mesh would add no value for these cutouts.

Triangles: {blender['triangles']} before and after. One mesh. Texture: {blender['texture'][0]}×{blender['texture'][1]}.
Bind bounds before: {result['bounds_before']}.
Bind bounds after: {result['bounds_after']}.

Full BMD comparison: EQUIVALENT; all original triangles match. Full output:

```
{(folder/'validation/compare.txt').read_text().strip()}
```

Bone names, original indices, parents, action order, one-frame action and lock=0 are retained.
`validation/local-motion.json` compares every local position and rotation matrix. Grass05/06
can encode the same rotation using a different Euler branch after Blender; equivalence is
proved as rotation matrices, not by assuming identical Euler components. The skeleton-only
BMD comparison is independently EQUIVALENT. `validation/uv-material.json` checks every
original triangle corner's rigid binding, UV and material against the exported BMD.

All exported SMD references/actions pass bmdconv validation. Final 32-bit uncompressed
bottom-left TGA/OZT passes mu_texture check. Alpha artwork is retained, including irregular
cutouts; RGB beneath fully transparent texels is padded from adjacent visible leaf colors.
Raw paintings, rejected density study, final editable PNG/TGA and prompts live in the parent
batch `textures/` and `prompts.md`. The two paired models share only their assigned texture.

Review images are in `review/comparison.jpg`, with individual before/after, wireframe and
240px readability images. After images come from a fresh re-import of the exported BMD.
Terrain previews use the completed pilot TileGrass01 at 100 units per repeat and a 190-unit
figure proxy. These are matched offline previews at the origin, not actual client captures.

Engine inspection: MapManager.cpp loads Grass01 through the Grass model range;
_enum.h assigns MODEL_GRASS01=20. Lorencia switches in ZzzObject.cpp contain no special
mesh-index handling for types 20/21/24/25. Single material and mesh order remain unchanged.

Reproduction: run the parent batch `package_textures.py`, Blender `build_sources.py`,
`export_validate.py`, Blender `render_review.py`, then `finalize.py`, with commands executed
from the assigned worktree. No runtime installation is performed by this batch.
'''
    (folder/'notes.md').write_text(text)


def main():
    inventory=json.loads((ROOT.parent/'coordination/dependency-map.json').read_text())
    for name in ASSETS:
        uv_audit(name)
        review_sheet(name)
        write_notes(name,inventory['models'][name])
        shutil.copy2(ROOT/name/f'exports/{name}.bmd',REPOSITORY/f'src/bin/Data/Object1/{name}.bmd')
    for texture in set(TEXTURES.values()):
        shutil.copy2(ROOT/f'textures/final/{texture}.OZT',REPOSITORY/f'src/bin/Data/Object1/{texture}.OZT')
    files=[REPOSITORY/f'src/bin/Data/Object1/{name}.bmd' for name in ASSETS]
    files += [REPOSITORY/f'src/bin/Data/Object1/{texture}.OZT' for texture in sorted(set(TEXTURES.values()))]
    manifest={str(path.relative_to(REPOSITORY)):hashlib.sha256(path.read_bytes()).hexdigest() for path in files}
    (ROOT/'installed-source-files.json').write_text(json.dumps(manifest,indent=2)+'\n')


if __name__ == '__main__':
    main()
