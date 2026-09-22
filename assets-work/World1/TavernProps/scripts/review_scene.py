"""Consistent cameras and diffuse-only offline lighting; never client evidence."""

import bpy
from mathutils import Vector

RENDER_SIZE = (960, 760)


def camera(bounds, direction=(1.45, -2.2, 1.45), scale=1.45):
    low, high = map(Vector, bounds)
    center, span = (low + high) / 2, max(high - low)
    existing = bpy.data.objects.get('OfflineCamera')
    if existing:
        bpy.data.objects.remove(existing, do_unlink=True)
    result = bpy.data.objects.new('OfflineCamera', bpy.data.cameras.new('OfflineCamera'))
    bpy.context.scene.collection.objects.link(result)
    result.location = center + Vector(direction) * span
    result.rotation_euler = (center - result.location).to_track_quat('-Z', 'Y').to_euler()
    result.data.type = 'ORTHO'
    result.data.ortho_scale = span * scale
    result.data.clip_end = span * 20
    bpy.context.scene.camera = result
    return result


def lighting():
    scene = bpy.context.scene
    scene.world.use_nodes = True
    background = scene.world.node_tree.nodes.get('Background')
    background.inputs['Color'].default_value = (.32, .36, .40, 1)
    background.inputs['Strength'].default_value = .7
    sun = bpy.data.objects.new('OfflineSun', bpy.data.lights.new('OfflineSun', 'SUN'))
    scene.collection.objects.link(sun)
    sun.rotation_euler = (.45, -.55, -.4)
    sun.data.energy, sun.data.angle = 2.3, .3
    scene.render.engine = 'CYCLES'
    scene.cycles.samples, scene.cycles.use_denoising = 24, True
    scene.render.resolution_x, scene.render.resolution_y = RENDER_SIZE
    scene.render.resolution_percentage = 100
    scene.render.film_transparent = True
    scene.view_settings.view_transform = 'Standard'
    scene['review_status'] = 'OFFLINE BLENDER RENDER; CLIENT VERIFICATION PENDING'
    for material in bpy.data.materials:
        if not material.use_nodes:
            continue
        shader = material.node_tree.nodes.get('Principled BSDF')
        if shader:
            shader.inputs['Roughness'].default_value = 1
            shader.inputs['Specular IOR Level'].default_value = 0


def render(path):
    bpy.context.scene.render.filepath = str(path)
    bpy.ops.render.render(write_still=True)
