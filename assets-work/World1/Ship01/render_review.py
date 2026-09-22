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
    wire.use_pixel_size = True
    wire.inputs['Size'].default_value = 1.1
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


def preview_additive_fire():
    """Offline approximation of the documented BlendMesh=1 additive material."""
    for material in bpy.data.materials:
        if material.get('mu_texture') != 'fire_02.jpg':
            continue
        nodes, links = material.node_tree.nodes, material.node_tree.links
        texture = next(n for n in nodes if n.type == 'TEX_IMAGE')
        output = next(n for n in nodes if n.type == 'OUTPUT_MATERIAL')
        transparent = nodes.new('ShaderNodeBsdfTransparent')
        emission = nodes.new('ShaderNodeEmission')
        add = nodes.new('ShaderNodeAddShader')
        links.new(texture.outputs['Color'], emission.inputs['Color'])
        links.new(transparent.outputs[0], add.inputs[0])
        links.new(emission.outputs[0], add.inputs[1])
        links.new(add.outputs[0], output.inputs['Surface'])


def scale_proxy(bounds):
    bpy.ops.mesh.primitive_cube_add(size=1, location=(bounds[1][0] + 35, 0, 95))
    obj = bpy.context.object
    obj.name = 'OFFLINE_190_UNIT_HEIGHT_PROXY'
    obj.dimensions = (10, 10, 190)
    obj['mu_reference'] = True
    set_camera((bounds[0], (bounds[1][0] + 55, bounds[1][1], max(190, bounds[1][2]))), 1.4)


def review(name):
    folder = HERE / name
    reference_bounds = json.loads((folder / 'validation/blender.json').read_text())['bounds_before']
    for stage, source in [('before', 'original/source.blend'), ('after', 'validation/reimported.blend')]:
        bpy.ops.wm.open_mainfile(filepath=str(folder / source))
        bpy.context.scene.frame_set(0)
        if name == 'Bonfire01':
            preview_additive_fire()
        set_lighting()
        set_camera(reference_bounds, 1.3)
        scene = bpy.context.scene
        scene.render.resolution_x, scene.render.resolution_y = 900, 680
        scene.render.threads_mode, scene.render.threads = 'FIXED', 2
        scene.cycles.samples = 16
        render(folder / 'review' / (stage + '.png'))
        camera = scene.camera
        camera.location.x *= -1
        camera.location.y *= -1
        from mathutils import Vector
        target = (Vector(reference_bounds[0]) + Vector(reference_bounds[1])) / 2
        camera.rotation_euler = (target-camera.location).to_track_quat('-Z','Y').to_euler()
        render(folder / 'review' / ('reverse-' + stage + '.png'))
        set_camera(reference_bounds,1.3)
        for key in (14,29):
            scene.frame_set(key)
            render(folder / 'review' / (f'pose-{key}-' + stage + '.png'))
        scene.frame_set(0)
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
        scale_proxy(reference_bounds)
        render(folder / 'review' / ('scale-' + stage + '.png'))
    (folder / 'review/camera-context.json').write_text(json.dumps(dict(
        baseline_bounds=reference_bounds, camera='Matching orthographic; direction (1.35,-2,1.3)',
        scale_proxy_height=190, client_screenshot=False, effect_shader='Standard diffuse/alpha offline approximation; no runtime client claim'), indent=2) + '\n')


for asset in ASSETS:
    review(asset)
