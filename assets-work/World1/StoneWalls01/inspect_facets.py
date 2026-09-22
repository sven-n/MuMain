"""Document masonry face planes before choosing conservative interior recesses."""
from pathlib import Path
import sys
sys.dont_write_bytecode = True
import bpy
ROOT = Path(__file__).resolve().parent
for root in sorted(ROOT.glob('*/original')):
    bpy.ops.wm.open_mainfile(filepath=str(root / 'source.blend'))
    for obj in bpy.context.scene.objects:
        if obj.type != 'MESH' or obj.get('mu_helper'):
            continue
        print(root.parent.name)
        for face in obj.data.polygons:
            if obj.data.materials[face.material_index].name != 'tile_01.jpg':
                continue
            positions = [obj.matrix_world @ obj.data.vertices[i].co for i in face.vertices]
            print(face.index, 'verts', list(face.vertices), 'normal', tuple(round(x, 2) for x in face.normal), 'center', tuple(round(sum(p[a] for p in positions) / len(positions), 2) for a in range(3)))
