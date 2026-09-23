"""Verify appended actual-placement scene geometry against transformed raw SMD."""
from pathlib import Path
import json
import math
import sys
sys.dont_write_bytecode=True
import bpy
from mathutils import Vector,Euler
ROOT=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT))
from assemblies import append_model
from audit_exports import triangles


def main():
    records=json.loads((ROOT/'context-placements.json').read_text())
    points=[Vector(row[1:4]) for _,rows in triangles(ROOT/'Object04/validation/baseline/Object04.smd') for row in rows]
    report=[]
    for group in records:
        record=group['records'][0]
        bpy.ops.wm.read_factory_settings(use_empty=True)
        meshes=append_model(record,'current',Vector(record['position']))
        bpy.context.scene.frame_set(0);bpy.context.view_layer.update()
        actual=[]
        for obj in meshes:
            evaluated=obj.evaluated_get(bpy.context.evaluated_depsgraph_get())
            mesh=evaluated.to_mesh()
            actual.extend(evaluated.matrix_world@vertex.co for vertex in mesh.vertices)
            evaluated.to_mesh_clear()
        rotation=Euler([math.radians(v) for v in record['rotation']],'XYZ').to_matrix()
        expected=[rotation@point*record['scale'] for point in points]
        error=max(abs(operation(p[k] for p in actual)-operation(p[k] for p in expected)) for k in range(3) for operation in (min,max))
        report.append(dict(group=group['label'],maximum_evaluated_bound_error=error,
                           armatures=[dict(name=o.name,matrix=[list(row) for row in o.matrix_world]) for o in bpy.context.scene.objects if o.type=='ARMATURE']))
    (ROOT/'context-transform-proof.json').write_text(json.dumps(report,indent=2))
    assert max(item['maximum_evaluated_bound_error'] for item in report)<.001,report


if __name__=='__main__':main()
