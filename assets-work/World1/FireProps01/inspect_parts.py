"""Record original rigid geometry before choosing edits."""
import json
import sys
from pathlib import Path
import bpy

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from prepare import ASSETS

for name in ASSETS:
    bpy.ops.wm.open_mainfile(filepath=str(HERE / name / 'original/source.blend'))
    bpy.context.scene.frame_set(0)
    report = []
    for obj in bpy.context.scene.objects:
        if obj.type != 'MESH' or obj.get('mu_helper'):
            continue
        parts = {}
        for vertex in obj.data.vertices:
            bone = obj.vertex_groups[vertex.groups[0].group].name
            parts.setdefault(bone, []).append(list(obj.matrix_world @ vertex.co))
        report.append(dict(mesh=obj.name, materials=[m.name for m in obj.data.materials],
                           points=parts, triangles=len(obj.data.polygons)))
    (HERE / name / 'original/parts.json').write_text(json.dumps(report, indent=2) + '\n')
