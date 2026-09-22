"""Import immutable BMD backups through the repository importer in Blender."""

import json
import sys

import bpy

sys.dont_write_bytecode = True
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from config import CONVERTER, PROPS, REPOSITORY, ROOT
sys.path.insert(0, str(REPOSITORY / 'tools/blender'))
import mu_bmd_import


def import_one(name):
    root = ROOT / name / 'original'
    target = root / 'source.blend'
    if target.exists():
        raise RuntimeError(f'Original already preserved: {target}')
    sys.argv = ['blender', '--', '--bmd', str(root / f'{name}.bmd'),
                '--out', str(target), '--bmdconv', str(CONVERTER)]
    mu_bmd_import.main()
    objects = [o for o in bpy.context.scene.objects if o.type == 'MESH' and not o.get('mu_helper')]
    data = [dict(name=o.name, vertices=[list(o.matrix_world @ v.co) for v in o.data.vertices],
                 triangles=sum(len(p.vertices) - 2 for p in o.data.polygons),
                 materials=[m.name for m in o.data.materials]) for o in objects]
    (root / 'geometry.json').write_text(json.dumps(data, indent=2) + '\n')


for prop in PROPS:
    import_one(prop)
