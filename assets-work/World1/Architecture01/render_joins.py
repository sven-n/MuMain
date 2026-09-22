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
sys.path.insert(0, str(ROOT))
from render_exports import opaque_camera_background

NAMES = ('House01', 'House03', 'House04', 'Tent01', 'HouseWall02', 'HouseEtc02')
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


def effect_preview():
    opaque_camera_background()
    for material in bpy.data.materials:
        if not material.name.startswith('light_02.jpg'):
            continue
        nodes, links = material.node_tree.nodes, material.node_tree.links
        textures = [node for node in nodes if node.type == 'TEX_IMAGE']
        if not textures:
            continue
        transparent = nodes.new('ShaderNodeBsdfTransparent')
        emission = nodes.new('ShaderNodeEmission')
        emission.inputs['Strength'].default_value = .55
        add = nodes.new('ShaderNodeAddShader')
        links.new(textures[0].outputs['Color'], emission.inputs['Color'])
        links.new(transparent.outputs[0], add.inputs[0])
        links.new(emission.outputs[0], add.inputs[1])
        links.new(add.outputs[0], nodes.get('Material Output').inputs['Surface'])


def review_group(label, anchor_name, radius, anchor_index=0):
    anchor, records = placement_group(anchor_name, radius, anchor_index)
    bounds = None
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
        set_camera(bounds)
        set_lighting()
        effect_preview()
        bpy.context.scene.render.resolution_x = 1440
        bpy.context.scene.render.resolution_y = 840
        render(REVIEW / f'{label}-{stage}.png')
    report = dict(status='OFFLINE BLENDER PLACEMENT REVIEW; NOT CLIENT EVIDENCE',
                  coordinate_anchor=list(anchor), objects=records,
                  placement_source='Unchanged World1 EncTerrain1.obj, decoded by coordination inventory',
                  caveat='Only owned geometry shown. Original terrain height, lighting, collision and other buildings omitted. Light beams approximate additive rendering at0.55 emission, not engine evidence.',
                  join_checks='Actual World1 position/rotation/scale retained; camera identical before/after')
    report['camera_bounds_from_original_evaluated_meshes'] = bounds
    (REVIEW / f'{label}-placements.json').write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    REVIEW.mkdir(exist_ok=True)
    review_group('house-pair', 'House01', 1100)
    review_group('town-wall', 'HouseWall02', 1200, 4)
