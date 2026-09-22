"""Inspect actual vertices and render original silhouettes with a shared offline camera."""
import json
from pathlib import Path
import sys

sys.dont_write_bytecode = True
import bpy

ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT.parent / 'StaticBatch01'))
from review_scene import mesh_bounds, render, set_camera, set_lighting

for name in ('HouseWall03', 'Bridge01', 'BridgeStone01', 'Fence01', 'FireLight02', 'House01', 'House03', 'House04', 'House05', 'HouseEtc02', 'HouseWall01', 'HouseWall02', 'HouseWall04', 'HouseWall05', 'HouseWall06', 'StoneMuWall01', 'StoneWall01', 'StoneWall02', 'Tent01', 'Tree07'):
    root = ROOT / name / 'original'
    bpy.ops.wm.open_mainfile(filepath=str(root / 'source.blend'))
    bpy.context.scene.frame_set(0)
    objects = [obj for obj in bpy.context.scene.objects if obj.type == 'MESH' and not obj.get('mu_helper')]
    report = []
    for obj in objects:
        report.append(dict(name=obj.name, vertices=[list(obj.matrix_world @ v.co) for v in obj.data.vertices],
                           polygons=[list(p.vertices) for p in obj.data.polygons],
                           materials=[m.name for m in obj.data.materials],
                           groups=[g.name for g in obj.vertex_groups],
                           vertex_groups=[[(obj.vertex_groups[g.group].name, g.weight) for g in v.groups] for v in obj.data.vertices],
                           face_materials=[p.material_index for p in obj.data.polygons]))
    (root / 'geometry.json').write_text(json.dumps(report, indent=2) + '\n')
    set_camera(mesh_bounds(objects))
    set_lighting()
    render(root / 'inspection.png')
