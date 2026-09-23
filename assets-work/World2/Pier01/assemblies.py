"""Exact Dungeon wall/pier transforms with unmodified wall reference models."""
from pathlib import Path
import json
import math
import os
import sys
sys.dont_write_bytecode = True
import bpy
from mathutils import Vector, Matrix, Euler
ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT.parents[1] / 'World1/StaticBatch01'))
from review_scene import mesh_bounds, set_camera, set_lighting, render
CONTEXT_ROOT = Path(os.environ.get('DUNGEON_CONTEXT_ROOT', str(ROOT.parents[3] / 'MuMain-dungeon-remake/assets-work/World2/Readiness01')))


def append_model(record, stage, anchor):
    if record['name'] == 'Object04':
        candidate = ROOT / 'Object04/validation/reimported.blend'
        path = candidate if stage == 'candidate' else ROOT / 'Object04/baseline/source.blend'
    else:
        path = CONTEXT_ROOT / record['name'] / 'baseline.blend'
    assert path.exists(), path
    with bpy.data.libraries.load(str(path)) as (source, loaded):
        loaded.objects = source.objects
    meshes = []
    for obj in loaded.objects:
        if obj.type not in ('ARMATURE', 'MESH') or obj.get('mu_helper') or obj.get('mu_reference'):
            continue
        bpy.context.scene.collection.objects.link(obj)
        if obj.type == 'MESH':
            meshes.append(obj)
        else:
            rotation = Euler([math.radians(v) for v in record['rotation']], 'XYZ').to_matrix().to_4x4()
            obj.matrix_world = Matrix.Translation(Vector(record['position'])-anchor) @ rotation @ Matrix.Scale(record['scale'],4)
    return meshes


def group(config):
    records = config['records']
    out = ROOT / 'review-assemblies' / config['label']
    out.mkdir(parents=True, exist_ok=True)
    stages = ('current','candidate') if (ROOT / 'Object04/validation/reimported.blend').exists() else ('current',)
    for stage in stages:
        bpy.ops.wm.read_factory_settings(use_empty=True)
        bpy.context.scene.world = bpy.data.worlds.new('ReviewWorld')
        meshes = []
        for record in records:
            meshes.extend(append_model(record,stage,Vector(records[0]['position'])))
        bpy.context.scene.frame_set(0)
        bpy.context.view_layer.update()
        bounds = mesh_bounds(meshes)
        set_camera(bounds)
        set_lighting()
        bpy.context.scene.cycles.samples = 16
        center = (Vector(bounds[0])+Vector(bounds[1]))/2
        rotation = Euler([math.radians(v) for v in records[0]['rotation']], 'XYZ').to_matrix()
        camera = bpy.context.scene.camera
        camera.location = center + rotation @ Vector((-.9,-1.8,1.25)) * max(Vector(bounds[1])-Vector(bounds[0]))
        camera.rotation_euler = (center-camera.location).to_track_quat('-Z','Y').to_euler()
        render(out/(stage+'.png'))
        bpy.context.scene.render.resolution_percentage = 35
        render(out/(stage+'-small.png'))
        bpy.context.scene.render.resolution_percentage = 100
        camera.location = center + rotation @ Vector((1.4,-1.5,.8)) * max(Vector(bounds[1])-Vector(bounds[0]))
        camera.rotation_euler = (center-camera.location).to_track_quat('-Z','Y').to_euler()
        render(out/(stage+'-reverse.png'))


if __name__ == '__main__':
    for config in json.loads((ROOT/'context-placements.json').read_text()):
        group(config)
