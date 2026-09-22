"""Render actual exported BMDs beside originals with fixed cameras and offline scale proxies."""
import importlib.util
import json
from pathlib import Path
import sys
import traceback

sys.dont_write_bytecode = True
import bpy
from mathutils import Vector

ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT.parent / 'StaticBatch01'))
from geometry import material
from review_scene import render

spec = importlib.util.spec_from_file_location('static_review', ROOT.parent / 'StaticBatch01/render_review.py')
review = importlib.util.module_from_spec(spec)
spec.loader.exec_module(review)
NAMES = ('House01', 'House03', 'House04', 'Tent01', 'HouseWall02', 'HouseEtc02')
EFFECT_NAMES = ('House03', 'House04', 'HouseWall02')
ACTION_FRAMES = {'House04': (0, 19, 39), 'Tent01': (0, 17, 35)}


def opaque_camera_background():
    scene = bpy.context.scene
    scene.render.film_transparent = False
    nodes, links = scene.world.node_tree.nodes, scene.world.node_tree.links
    background = nodes.new('ShaderNodeBackground')
    background.inputs['Color'].default_value = (.009, .012, .014, 1)
    camera_ray = nodes.new('ShaderNodeLightPath')
    mix = nodes.new('ShaderNodeMixShader')
    links.new(camera_ray.outputs['Is Camera Ray'], mix.inputs[0])
    links.new(nodes.get('Background').outputs[0], mix.inputs[1])
    links.new(background.outputs[0], mix.inputs[2])
    links.new(mix.outputs[0], nodes.get('World Output').inputs['Surface'])


def approximate_effects(name):
    if name not in EFFECT_NAMES:
        return
    opaque_camera_background()
    filename = 'tile_space01.jpg' if name == 'House04' else 'light_02.jpg'
    for material in bpy.data.materials:
        if material.name != filename:
            continue
        nodes, links = material.node_tree.nodes, material.node_tree.links
        texture = next(node for node in nodes if node.type == 'TEX_IMAGE')
        transparent = nodes.new('ShaderNodeBsdfTransparent')
        emission = nodes.new('ShaderNodeEmission')
        emission.inputs['Strength'].default_value = 1 if name == 'House04' else .55
        add = nodes.new('ShaderNodeAddShader')
        links.new(texture.outputs['Color'], emission.inputs['Color'])
        links.new(transparent.outputs[0], add.inputs[0])
        links.new(emission.outputs[0], add.inputs[1])
        links.new(add.outputs[0], nodes.get('Material Output').inputs['Surface'])
        if name == 'House04':
            coordinates = nodes.new('ShaderNodeTexCoord')
            offset = nodes.new('ShaderNodeVectorMath')
            offset.name = 'OFFLINE_V_SCROLL'
            offset.operation = 'ADD'
            links.new(coordinates.outputs['UV'], offset.inputs[0])
            links.new(offset.outputs[0], texture.inputs['Vector'])


def action_views(root, stage):
    for index, frame in enumerate(ACTION_FRAMES.get(root.name, ())):
        bpy.context.scene.frame_set(frame)
        if root.name == 'House04':
            bpy.data.materials['tile_space01.jpg'].node_tree.nodes['OFFLINE_V_SCROLL'].inputs[1].default_value[1] = -index / 3
        render(root / f'review/action-{stage}-{frame}.png')
    bpy.context.scene.frame_set(0)
    if root.name == 'House04':
        bpy.data.materials['tile_space01.jpg'].node_tree.nodes['OFFLINE_V_SCROLL'].inputs[1].default_value[1] = 0


def source_audit(root):
    bpy.ops.wm.open_mainfile(filepath=str(root / 'source.blend'))
    images = [image for image in bpy.data.images if image.source == 'FILE']
    assert images and all(image.packed_file for image in images)
    for name in ('REF_ORIGINAL', 'REF_HIGH_POLY'):
        group = bpy.data.collections[name]
        assert group.objects and group.hide_render and not group.vs.export
        assert all(obj.get('mu_reference') for obj in group.objects)
    objects = list(bpy.data.collections['EXPORT_' + root.name].objects)
    assert all(len(obj.data.uv_layers) == 1 and obj.data.uv_layers[0].name == 'UVMap' for obj in objects)
    (root / 'validation/source-audit.json').write_text(json.dumps(dict(
        status='PASS', packed_images=[image.name for image in images], original_rig_reused=True,
        reference_exclusion='REF_* collections and mu_reference', export_objects=len(objects)), indent=2) + '\n')


def scale_view(root, stage):
    mat = material(ROOT.parent / 'GroundTiles/textures/TileGrass01.jpg')
    ground = review.plain_mesh('OfflineGround_100UnitRepeats',
        [(-600, -500, -.5), (650, -500, -.5), (650, 600, -.5), (-600, 600, -.5)], [(0, 1, 2, 3)], mat)
    layer = ground.data.uv_layers.new(name='UVMap')
    for point, coord in zip(layer.data, ((0, 0), (12.5, 0), (12.5, 11), (0, 11))):
        point.uv = coord
    review.scale_figure()
    figure = bpy.data.objects['ScaleProxy_NotGameCharacter']
    figure.location.x = 410
    figure.location.y = -100
    camera = bpy.context.scene.camera
    center = Vector((60, 40, 150))
    camera.location = center + Vector((600, -900, 900))
    camera.rotation_euler = (center - camera.location).to_track_quat('-Z', 'Y').to_euler()
    camera.data.ortho_scale = 1800
    bpy.context.scene.render.resolution_x = 900
    bpy.context.scene.render.resolution_y = 550
    render(root / f'review/gameplay-{stage}.png')


def exported_wire_material():
    material = review.wire_material()
    wire = next(node for node in material.node_tree.nodes if node.type == 'WIREFRAME')
    wire.use_pixel_size = True
    wire.inputs['Size'].default_value = 1.25
    return material


def review_asset(name):
    root = ROOT / name
    source_audit(root)
    bounds = json.loads((root / 'validation/blender.json').read_text())['bounds_before']
    for stage in ('before', 'after'):
        review.open_model(root, stage, bounds)
        if name in EFFECT_NAMES:
            render(root / f'review/neutral-{stage}.png')
        approximate_effects(name)
        render(root / f'review/{stage}.png')
        action_views(root, stage)
        scale_view(root, stage)
    review.open_model(root, 'after', bounds)
    bpy.context.view_layer.material_override = exported_wire_material()
    render(root / 'review/wireframe.png')
    (root / 'review/render-context.json').write_text(json.dumps(dict(
        kind='OFFLINE BLENDER; NOT CLIENT SCREENSHOTS', after='Re-imported exported BMD',
        camera='Identical orthographic camera and diffuse lighting before/after',
        additive='OFFLINE APPROXIMATION: transparent plus image emission; light_02 midpoint brightness0.55, tile_space01 brightness1. Neutral diffuse images also retained.',
        animation_frames=ACTION_FRAMES.get(name, ()),
        porthole_scroll='House04 pose columns pair original frames0/19/39 with illustrative V offsets0/-1/3/-2/3. Actual engine scroll is world-time driven; mesh UVs are unmodified.',
        scale=dict(world_view_width=1800, pixels=900, terrain_tile_units=100, proxy_height=190,
                   caveat='Offline scale assumption with flat grass proxy, not actual terrain or client camera')),
        indent=2) + '\n')


if __name__ == '__main__':
    try:
        for name in NAMES:
            review_asset(name)
    except Exception:
        traceback.print_exc()
        sys.exit(1)
