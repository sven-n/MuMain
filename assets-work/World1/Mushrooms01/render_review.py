"""Matched cameras on original and reimported BMDs, with wireframe and scale evidence."""
import json
from pathlib import Path
import sys

import bpy

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from prepare import ASSETS
sys.path.insert(0, str(HERE.parent / 'StaticBatch01'))
from review_scene import mesh_bounds, set_camera, set_lighting


def meshes():
    return [obj for obj in bpy.context.scene.objects if obj.type == 'MESH' and not obj.get('mu_helper')]


def wire_material():
    result = bpy.data.materials.new('OFFLINE_WIREFRAME')
    result.use_nodes = True
    nodes, links = result.node_tree.nodes, result.node_tree.links
    wire = nodes.new('ShaderNodeWireframe')
    wire.inputs['Size'].default_value = .05
    mix = nodes.new('ShaderNodeMixRGB')
    mix.inputs[1].default_value = (.34, .48, .47, 1)
    mix.inputs[2].default_value = (.02, .03, .03, 1)
    links.new(wire.outputs['Fac'], mix.inputs[0])
    links.new(mix.outputs[0], nodes.get('Principled BSDF').inputs['Base Color'])
    return result


def render(path):
    scene = bpy.context.scene
    scene.render.filepath = str(path)
    bpy.ops.render.render(write_still=True)


def scale_proxy():
    bpy.ops.mesh.primitive_cube_add(size=1, location=(115, 20, 95))
    obj = bpy.context.object
    obj.name = 'OFFLINE_190_UNIT_HEIGHT_PROXY'
    obj.dimensions = (10, 10, 190)
    obj['mu_reference'] = True
    set_camera(((-70, -60, 0), (135, 70, 190)), 1.4)


def review(name):
    folder = HERE / name
    reference_bounds = json.loads((folder / 'validation/blender.json').read_text())['bounds_before']
    for stage, source in [('before', 'original/source.blend'), ('after', 'validation/reimported.blend')]:
        bpy.ops.wm.open_mainfile(filepath=str(folder / source))
        bpy.context.scene.frame_set(0)
        set_lighting()
        set_camera(reference_bounds, 1.3)
        scene = bpy.context.scene
        scene.render.resolution_x, scene.render.resolution_y = 900, 680
        scene.render.threads_mode, scene.render.threads = 'FIXED', 2
        scene.cycles.samples = 16
        render(folder / 'review' / (stage + '.png'))
        if stage == 'after':
            original = [(obj, list(obj.data.materials)) for obj in meshes()]
            wire = wire_material()
            for obj, mats in original:
                for index in range(len(mats)):
                    obj.data.materials[index] = wire
            render(folder / 'review/wireframe.png')
            for obj, mats in original:
                for index, mat in enumerate(mats):
                    obj.data.materials[index] = mat
        scale_proxy()
        render(folder / 'review' / ('scale-' + stage + '.png'))
    (folder / 'review/camera-context.json').write_text(json.dumps(dict(
        baseline_bounds=reference_bounds, camera='Matching orthographic; direction (1.35,-2,1.3)',
        scale_proxy_height=190, world_units_per_terrain_tile=100, client_screenshot=False), indent=2) + '\n')


for asset in ASSETS:
    review(asset)
