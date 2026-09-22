"""Consistent diffuse-only offline Blender lighting and orthographic review cameras."""

import bpy
from mathutils import Vector

RENDER_SIZE = (900, 780)
CAMERA_DIRECTION = Vector((1.35, -2, 1.3))


def mesh_bounds(objects):
    points = [obj.matrix_world @ vertex.co for obj in objects for vertex in obj.data.vertices]
    return ([min(point[axis] for point in points) for axis in range(3)],
            [max(point[axis] for point in points) for axis in range(3)])


def set_camera(bounds, scale_factor=1.4):
    low, high = map(Vector, bounds)
    center = (low + high) / 2
    span = max(high - low)
    camera = bpy.data.objects.new('OfflineCamera', bpy.data.cameras.new('OfflineCamera'))
    bpy.context.scene.collection.objects.link(camera)
    camera.location = center + CAMERA_DIRECTION * span
    camera.rotation_euler = (center - camera.location).to_track_quat('-Z', 'Y').to_euler()
    camera.data.type = 'ORTHO'
    camera.data.ortho_scale = span * scale_factor
    camera.data.clip_end = span * 20
    bpy.context.scene.camera = camera
    return camera


def set_lighting():
    scene = bpy.context.scene
    scene.world.use_nodes = True
    background = scene.world.node_tree.nodes.get('Background')
    background.inputs['Color'].default_value = (.32, .36, .40, 1)
    background.inputs['Strength'].default_value = .7
    sun = bpy.data.objects.new('OfflineDiffuseSun', bpy.data.lights.new('OfflineDiffuseSun', 'SUN'))
    scene.collection.objects.link(sun)
    sun.rotation_euler = (.45, -.55, -.4)
    sun.data.energy = 2.3
    sun.data.angle = .3
    scene.render.engine = 'CYCLES'
    scene.cycles.samples = 24
    scene.cycles.use_denoising = True
    scene.render.resolution_x, scene.render.resolution_y = RENDER_SIZE
    scene.render.resolution_percentage = 100
    scene.render.film_transparent = True
    scene.view_settings.view_transform = 'Standard'
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
