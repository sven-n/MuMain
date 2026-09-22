"""Review final re-imported BMDs with matching cameras, retained animation and diffuse materials."""
import importlib.util
import json
from pathlib import Path
import sys
import traceback

import bpy

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT.parent / 'StaticBatch01'))
from review_scene import render

spec = importlib.util.spec_from_file_location('static_review', ROOT.parent / 'StaticBatch01/render_review.py')
review = importlib.util.module_from_spec(spec)
spec.loader.exec_module(review)
NAMES = ('House02', 'TreasureDrum01', 'StreetLight01')


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
    record = dict(status='PASS', packed_images=[image.name for image in images], original_rig_reused=True,
                  reference_exclusion='REF_* collections and mu_reference', export_objects=len(objects))
    (root / 'validation/source-audit.json').write_text(json.dumps(record, indent=2) + '\n')


def opaque_camera_background():
    scene = bpy.context.scene
    scene.render.film_transparent = False
    nodes, links = scene.world.node_tree.nodes, scene.world.node_tree.links
    lighting = nodes.get('Background')
    background = nodes.new('ShaderNodeBackground')
    background.inputs['Color'].default_value = (.009, .012, .014, 1)
    camera_ray = nodes.new('ShaderNodeLightPath')
    mix = nodes.new('ShaderNodeMixShader')
    links.new(camera_ray.outputs['Is Camera Ray'], mix.inputs[0])
    links.new(lighting.outputs[0], mix.inputs[1])
    links.new(background.outputs[0], mix.inputs[2])
    links.new(mix.outputs[0], nodes.get('World Output').inputs['Surface'])


def approximate_additive_lantern():
    for material in bpy.data.materials:
        if not material.name.endswith('streetlight_brightness2.jpg'):
            continue
        opaque_camera_background()
        nodes, links = material.node_tree.nodes, material.node_tree.links
        texture = next(node for node in nodes if node.type == 'TEX_IMAGE')
        transparent = nodes.new('ShaderNodeBsdfTransparent')
        emission = nodes.new('ShaderNodeEmission')
        emission.inputs['Strength'].default_value = 1
        add = nodes.new('ShaderNodeAddShader')
        links.new(texture.outputs['Color'], emission.inputs['Color'])
        links.new(transparent.outputs[0], add.inputs[0])
        links.new(emission.outputs[0], add.inputs[1])
        links.new(add.outputs[0], nodes.get('Material Output').inputs['Surface'])


def action_views(root,stage):
    if root.name != 'StreetLight01':
        return
    for frame in (0,10,20):
        bpy.context.scene.frame_set(frame)
        render(root / f'review/action-{stage}-{frame}.png')
    bpy.context.scene.frame_set(0)


def review_asset(name):
    root = ROOT / name
    source_audit(root)
    bounds = json.loads((root / 'validation/blender.json').read_text())['bounds_before']
    for stage in ('before', 'after'):
        review.open_model(root, stage, bounds)
        approximate_additive_lantern()
        render(root / f'review/{stage}.png')
        action_views(root,stage)
        review.scale_view(root, stage)
    if name == 'StreetLight01':
        for stage in ('before', 'after'):
            review.open_model(root, stage, bounds)
            render(root / f'review/neutral-{stage}.png')
    review.open_model(root, 'after', bounds)
    bpy.context.view_layer.material_override = review.wire_material()
    render(root / 'review/wireframe.png')
    record = dict(kind='OFFLINE BLENDER; NOT CLIENT SCREENSHOTS', after='Re-imported exported BMD',
                  camera='Identical orthographic camera and diffuse lighting before/after',
                  lantern='Transparent + diffuse-image emission approximates original additive mesh; opaque camera backdrop preserves emitted pixels; client result unverified',
                  neutral='StreetLight01 neutral-before/after use diffuse-only image material without additive approximation',
                  action_frames=[0,10,20] if name=='StreetLight01' else [],
                  scale=dict(world_view_width=900,pixels=900,terrain_tile_units=100,proxy_height=190,
                             caveat='Offline scale assumption, not the actual client camera'))
    (root / 'review/render-context.json').write_text(json.dumps(record, indent=2) + '\n')


if __name__ == '__main__':
    try:
        for name in NAMES:
            review_asset(name)
    except Exception:
        traceback.print_exc(); sys.exit(1)
