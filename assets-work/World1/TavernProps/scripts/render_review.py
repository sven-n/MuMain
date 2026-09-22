"""Render original and exported BMDs, topology, reverse views and placed repetitions offline."""

import json
import math
from pathlib import Path
import sys

import bpy
from mathutils import Euler, Matrix, Vector

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))
from config import CONVERTER, PROPS, REPOSITORY, ROOT
from geometry import bounds
from review_scene import camera, lighting, render
sys.path.insert(0, str(REPOSITORY / 'tools/blender'))
import mu_bmd_import


def meshes():
    return [o for o in bpy.context.scene.objects if o.type == 'MESH' and not o.get('mu_helper')]


def import_export(root):
    bpy.context.preferences.filepaths.save_version = 0
    sys.argv = ['blender', '--', '--bmd', str(root / f'exports/{root.name}.bmd'),
                '--out', str(root / 'validation/reimported.blend'), '--bmdconv', str(CONVERTER)]
    mu_bmd_import.main()
    return meshes()


def wire_material():
    result = bpy.data.materials.new('OFFLINE_WIREFRAME_ONLY')
    result.use_nodes = True
    nodes = result.node_tree.nodes
    nodes.clear()
    output = nodes.new('ShaderNodeOutputMaterial')
    shader = nodes.new('ShaderNodeBsdfDiffuse')
    wire = nodes.new('ShaderNodeWireframe')
    wire.inputs['Size'].default_value = .21
    mix = nodes.new('ShaderNodeMixRGB')
    mix.inputs[1].default_value = (.32, .40, .39, 1)
    mix.inputs[2].default_value = (.012, .018, .018, 1)
    result.node_tree.links.new(wire.outputs[0], mix.inputs[0])
    result.node_tree.links.new(mix.outputs[0], shader.inputs['Color'])
    result.node_tree.links.new(shader.outputs[0], output.inputs['Surface'])
    return result


def uv_data(objects):
    return [[list(obj.data.uv_layers[0].data[i].uv) for i in polygon.loop_indices]
            for obj in objects for polygon in obj.data.polygons]


def arrangement(root, objects, state):
    records = json.loads((root / 'original/placements.json').read_text())
    if root.name == 'Furniture04':
        records = records[:2]
    elif root.name != 'Furniture05':
        return
    origin = Vector(records[0]['position'])
    originals = list(objects)
    placed = []
    for record in records:
        translation = Vector(record['position']) - origin
        angles = Euler(tuple(math.radians(v) for v in record['rotation']), 'XYZ')
        transform = Matrix.Translation(translation) @ angles.to_matrix().to_4x4()
        transform = transform @ Matrix.Scale(record['scale'], 4)
        for obj in originals:
            clone = obj.copy()
            clone.data = obj.data.copy()
            clone.parent = None
            clone.modifiers.clear()
            bpy.context.scene.collection.objects.link(clone)
            clone.matrix_world = transform
            placed.append(clone)
    for obj in originals:
        obj.hide_render = True
    bpy.context.view_layer.update()
    camera(bounds(placed), scale=1.45)
    render(root / f'review/arrangement-{state}-offline.png')
    for obj in placed:
        bpy.data.objects.remove(obj, do_unlink=True)
    for obj in originals:
        obj.hide_render = False


def review(name):
    root = ROOT / name
    bpy.ops.wm.open_mainfile(filepath=str(root / 'original/source.blend'))
    bpy.context.scene.frame_set(0)
    original_bounds = bounds(meshes())
    camera(original_bounds)
    lighting()
    render(root / 'review/before-offline.png')
    arrangement(root, meshes(), 'before')
    objects = import_export(root)
    camera(original_bounds)
    lighting()
    render(root / 'review/after-offline.png')
    (root / 'review/exported-uv.json').write_text(json.dumps(uv_data(objects)) + '\n')
    arrangement(root, objects, 'after')
    camera(original_bounds, direction=(-1.4, 2.2, 1.1))
    render(root / 'review/reverse-offline.png')
    camera(original_bounds)
    topology = wire_material()
    for obj in objects:
        obj.data.materials.clear()
        obj.data.materials.append(topology)
    render(root / 'review/wireframe-offline.png')
    (root / 'review/render-context.json').write_text(json.dumps(dict(
        status='OFFLINE BLENDER; NOT CLIENT EVIDENCE', after_source='reimported exported BMD/OZJ',
        camera_bounds=original_bounds, projection='orthographic', direction=[1.45, -2.2, 1.45],
        scale_factor=1.45, lighting='diffuse-only Cycles; Standard transform; 24 samples',
        camera_and_lighting_matched=True, arrangements='exact original positions/rotations/scales'), indent=2) + '\n')


for prop in PROPS:
    review(prop)
