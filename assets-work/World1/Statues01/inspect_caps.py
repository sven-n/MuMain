"""Inspect the head cap UVs before a narrowly scoped mapping repair."""
from pathlib import Path
import bpy
root=Path(__file__).resolve().parent
bpy.ops.wm.open_mainfile(filepath=str(root/'StoneStatue02/original/source.blend'))
obj=next(o for o in bpy.data.objects if o.type=='MESH' and not o.get('mu_helper'))
for p in obj.data.polygons:
    if min(obj.data.vertices[i].co.z for i in p.vertices)>420:
        print(p.index, list(p.vertices), [list(obj.data.vertices[i].co) for i in p.vertices], [list(obj.data.uv_layers[0].data[i].uv) for i in p.loop_indices])
