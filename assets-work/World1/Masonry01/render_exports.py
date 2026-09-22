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
NAMES = ('HouseEtc01', 'StoneMuWall01', 'StoneMuWall02', 'StoneMuWall03', 'StoneMuWall04', 'HouseEtc02')


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
    figure.location.x = 330
    figure.location.y = -100
    camera = bpy.context.scene.camera
    center = Vector((60, 40, 150))
    camera.location = center + Vector((600, -900, 900))
    camera.rotation_euler = (center - camera.location).to_track_quat('-Z', 'Y').to_euler()
    camera.data.ortho_scale = 1450
    bpy.context.scene.render.resolution_x = 900
    bpy.context.scene.render.resolution_y = 550
    render(root / f'review/gameplay-{stage}.png')


def review_asset(name):
    root = ROOT / name
    source_audit(root)
    bounds = json.loads((root / 'validation/blender.json').read_text())['bounds_before']
    for stage in ('before', 'after'):
        review.open_model(root, stage, bounds)
        render(root / f'review/{stage}.png')
        scale_view(root, stage)
    review.open_model(root, 'after', bounds)
    bpy.context.view_layer.material_override = review.wire_material()
    render(root / 'review/wireframe.png')
    (root / 'review/render-context.json').write_text(json.dumps(dict(
        kind='OFFLINE BLENDER; NOT CLIENT SCREENSHOTS', after='Re-imported exported BMD',
        camera='Identical orthographic camera and diffuse lighting before/after',
        scale=dict(world_view_width=1450, pixels=900, terrain_tile_units=100, proxy_height=190,
                   caveat='Offline scale assumption with flat grass proxy, not actual terrain or client camera')),
        indent=2) + '\n')


if __name__ == '__main__':
    try:
        for name in NAMES:
            review_asset(name)
    except Exception:
        traceback.print_exc()
        sys.exit(1)
