"""Compare Blender skin evaluation independently of the SMD contract checks."""
import bpy
import json
from pathlib import Path
root=Path(__file__).resolve().parent/'Sign01'
result={}
for stage,relative in [('before','original/source.blend'),('after','validation/reimported.blend')]:
    bpy.ops.wm.open_mainfile(filepath=str(root/relative))
    bpy.context.scene.frame_set(0)
    objects=[]
    for obj in bpy.data.objects:
        if obj.type!='MESH' or obj.get('mu_helper'): continue
        evaluated=obj.evaluated_get(bpy.context.evaluated_depsgraph_get())
        mesh=evaluated.to_mesh()
        for poly in mesh.polygons:
            name=obj.data.materials[poly.material_index].name
            if name=='notice.jpg': continue
            objects.append(dict(material=name,vertices=[list(evaluated.matrix_world@mesh.vertices[i].co) for i in poly.vertices]))
        evaluated.to_mesh_clear()
    result[stage]=objects
(root/'validation/evaluated-alpha-cards.json').write_text(json.dumps(result,indent=2)+'\n')
