"""Matched offline views of original and reimported BMD, plus terrain and wireframes."""
from pathlib import Path
import json
import sys

sys.dont_write_bytecode = True

import bpy
from mathutils import Vector

ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
CONVERTER = '/Users/webproduktion3/Documents/claude-test-mumain/MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv'
sys.path.insert(0, str(ROOT.parent / 'StaticBatch01'))
sys.path.insert(0, str(REPOSITORY / 'tools/blender'))
from review_scene import render, set_camera, set_lighting
from geometry import material
from render_review import plain_mesh, scale_figure, wire_material
import mu_bmd_import

ASSETS = ('Tree01', 'Tree02', 'Tree11', 'Tree06')
VIEW_WIDTH = 2200


def open_stage(name, stage, bounds):
    folder = ROOT / name
    if stage == 'before':
        bpy.ops.wm.open_mainfile(filepath=str(folder / 'original/source.blend'))
    else:
        bpy.context.preferences.filepaths.save_version = 0
        sys.argv = ['blender', '--', '--bmd', str(folder / f'exports/{name}.bmd'),
                    '--out', str(folder / 'validation/reimported.blend'), '--textures',
                    str(ROOT / 'textures/final'), '--bmdconv', CONVERTER]
        mu_bmd_import.main()
    bpy.context.scene.frame_set(0)
    set_camera(bounds)
    set_lighting()
    bpy.context.scene.cycles.samples = 8


def ground_view(folder, stage):
    texture = material(ROOT.parent / 'GroundTiles/textures/TileGrass01.jpg')
    obj = plain_mesh('REVIEW_ONLY_100_UNIT_TERRAIN',
                     [(-1000,-1000,-8),(1000,-1000,-8),(1000,1000,-8),(-1000,1000,-8)], [(0,1,2,3)], texture)
    uv = obj.data.uv_layers.new(name='UVMap')
    for point, coordinate in zip(uv.data, ((0,0),(20,0),(20,20),(0,20))):
        point.uv = coordinate
    scale_figure()
    figure = bpy.data.objects.get('ScaleProxy_NotGameCharacter')
    figure.location.x = 500
    scene = bpy.context.scene
    center = Vector((100,0,240))
    scene.camera.location = center + Vector((1300,-1900,2000))
    scene.camera.rotation_euler = (center-scene.camera.location).to_track_quat('-Z','Y').to_euler()
    scene.camera.data.ortho_scale = VIEW_WIDTH
    scene.render.resolution_x, scene.render.resolution_y = 900, 600
    render(folder / f'review/terrain-{stage}.png')


def review(name):
    folder = ROOT / name
    bounds = json.loads((folder / 'validation/blender.json').read_text())['bounds_before']
    for stage in ('before','after'):
        open_stage(name, stage, bounds)
        render(folder / f'review/{stage}.png')
        if name != 'Tree06':
            for frame in (15,30):
                bpy.context.scene.frame_set(frame)
                render(folder / f'review/action-{stage}-{frame:02d}.png')
            bpy.context.scene.frame_set(0)
        ground_view(folder, stage)
    open_stage(name, 'after', bounds)
    bpy.context.view_layer.material_override = wire_material()
    render(folder / 'review/wireframe.png')
    report = dict(evidence='OFFLINE Blender renders, not client screenshots',
                  replacement='Reimported exported BMD', camera='Matching camera, lighting and scale before/after',
                  terrain='Completed pilot TileGrass01; 100 world units per repeat',
                  scale_proxy_height=190, terrain_view_width=VIEW_WIDTH,
                  animation='Frames 0,15,30 matched; all31 keys validated numerically for animated trees',
                  placement='Preview at origin, not an actual World1 placement or measured client camera')
    (folder / 'review/render-context.json').write_text(json.dumps(report,indent=2)+'\n')


for asset in ASSETS:
    review(asset)
