"""Render untouched World1 placement transforms for wall and gate joins, offline only."""
import json
import math
from pathlib import Path
import sys

import bpy
from mathutils import Euler, Matrix, Vector

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT.parent / 'StaticBatch01'))
from review_scene import mesh_bounds, render, set_camera, set_lighting

NAMES = ('StoneWall01', 'StoneWall02', 'StoneWall03', 'StoneWall05', 'StoneWall06')
REVIEW = ROOT / 'review'


def placement_group(anchor_name, radius, anchor_index):
    anchor = json.loads((ROOT / anchor_name / 'original/placements.json').read_text())[anchor_index]['position']
    records = []
    for name in NAMES:
        for index, record in enumerate(json.loads((ROOT / name / 'original/placements.json').read_text())):
            if math.dist(record['position'][:2], anchor[:2]) < radius:
                records.append(dict(name=name, placement_index=index, **record))
    return Vector(anchor), records


def append_asset(record, anchor, stage):
    subfolder = 'original/source.blend' if stage == 'before' else 'validation/reimported.blend'
    with bpy.data.libraries.load(str(ROOT / record['name'] / subfolder)) as (source, loaded):
        loaded.objects = source.objects
    meshes = []
    for obj in loaded.objects:
        if obj.type not in ('MESH', 'ARMATURE') or obj.get('mu_helper'):
            continue
        bpy.context.scene.collection.objects.link(obj)
        if obj.type == 'MESH':
            meshes.append(obj)
        else:
            rotation = Euler([math.radians(value) for value in record['rotation']], 'XYZ').to_matrix().to_4x4()
            obj.matrix_world = Matrix.Translation(Vector(record['position']) - anchor) @ rotation @ Matrix.Scale(record['scale'], 4)
    return meshes


def review_group(label, anchor_name, radius, anchor_index=0):
    anchor, records = placement_group(anchor_name, radius, anchor_index)
    bounds = None
    framing = None
    for stage in ('before', 'after'):
        for obj in list(bpy.data.objects):
            bpy.data.objects.remove(obj, do_unlink=True)
        bpy.context.scene.world = bpy.data.worlds.new('OfflineWorld')
        objects = [obj for record in records for obj in append_asset(record, anchor, stage)]
        bpy.context.scene.frame_set(0)
        bpy.context.view_layer.update()
        if bounds is None:
            evaluated = [obj.evaluated_get(bpy.context.evaluated_depsgraph_get()) for obj in objects]
            bounds = mesh_bounds(evaluated)
        camera = set_camera(bounds)
        set_lighting()
        bpy.context.scene.render.resolution_x = 1440
        bpy.context.scene.render.resolution_y = 840
        if framing is None:
            view = camera.rotation_euler.to_matrix().transposed()
            center = (Vector(bounds[0]) + Vector(bounds[1])) / 2
            evaluated = [obj.evaluated_get(bpy.context.evaluated_depsgraph_get()) for obj in objects]
            points = [view @ (obj.matrix_world @ v.co - center) for obj in evaluated for v in obj.data.vertices]
            lower = Vector((min(p.x for p in points), min(p.y for p in points), 0))
            upper = Vector((max(p.x for p in points), max(p.y for p in points), 0))
            location = camera.location + camera.rotation_euler.to_matrix() @ ((lower + upper) / 2)
            framing = dict(location=list(location), rotation=list(camera.rotation_euler),
                           ortho_scale=max(upper.x-lower.x, (upper.y-lower.y)*1440/840)*1.15)
        camera.location = framing['location']
        camera.rotation_euler = framing['rotation']
        camera.data.ortho_scale = framing['ortho_scale']
        render(REVIEW / f'{label}-{stage}.png')
    report = dict(status='OFFLINE BLENDER PLACEMENT REVIEW; NOT CLIENT EVIDENCE',
                  coordinate_anchor=list(anchor), objects=records,
                  placement_source='Unchanged World1 EncTerrain1.obj, decoded by coordination inventory',
                  caveat='Only owned geometry shown. Original terrain height, lighting, collision and other buildings omitted.',
                  join_checks='Actual World1 position/rotation/scale retained; camera identical before/after')
    report['camera_bounds_from_original_evaluated_meshes'] = bounds
    report['exact_shared_camera'] = framing
    (REVIEW / f'{label}-placements.json').write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    REVIEW.mkdir(exist_ok=True)
    review_group('raised-gates', 'StoneWall01', 850, 1)
    review_group('lowered-gates', 'StoneWall02', 1350, 0)
