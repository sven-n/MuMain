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

sys.path.insert(0, str(ROOT))
from config import ASSETS, KEYS
VIEW_WIDTH = 800


def additive_preview():
    """Render-only approximation of the existing additive mesh2; never exported."""
    for mat in bpy.data.materials:
        if not mat.name.startswith('horse_drawn_03.jpg') or not mat.use_nodes:
            continue
        nodes,links=mat.node_tree.nodes,mat.node_tree.links
        texture=next(n for n in nodes if n.type=='TEX_IMAGE')
        output=next(n for n in nodes if n.type=='OUTPUT_MATERIAL')
        emission=nodes.new('ShaderNodeEmission')
        transparent=nodes.new('ShaderNodeBsdfTransparent')
        add=nodes.new('ShaderNodeAddShader')
        links.new(texture.outputs['Color'],emission.inputs['Color'])
        links.new(transparent.outputs[0],add.inputs[0])
        links.new(emission.outputs[0],add.inputs[1])
        links.new(add.outputs[0],output.inputs['Surface'])


def open_stage(name, stage, bounds):
    folder = ROOT / name
    if stage == 'before':
        bpy.ops.wm.open_mainfile(filepath=str(folder / 'original/source.blend'))
    else:
        bpy.context.preferences.filepaths.save_version = 0
        bmd = folder / f'exports/{name}.bmd'
        sys.argv = ['blender', '--', '--bmd', str(bmd),
                    '--out', str(folder / 'validation/reimported.blend'), '--textures',
                    str(ROOT / 'textures/final'), '--bmdconv', CONVERTER]
        mu_bmd_import.main()
    bpy.context.scene.frame_set(0)
    set_camera(bounds)
    bpy.context.scene.camera.data.ortho_scale *= 1.18
    set_lighting()
    additive_preview()
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
    figure.location.x = 130
    scene = bpy.context.scene
    center = Vector((30,0,100))
    scene.camera.location = center + Vector((1300,-1900,2000))
    scene.camera.rotation_euler = (center-scene.camera.location).to_track_quat('-Z','Y').to_euler()
    scene.camera.data.ortho_scale = VIEW_WIDTH
    scene.camera.data.clip_end = 10000
    scene.render.resolution_x, scene.render.resolution_y = 900, 600
    render(folder / f'review/terrain-{stage}.png')


def review(name):
    folder = ROOT / name
    bounds = json.loads((folder / 'validation/blender.json').read_text())['bounds_before']
    for stage in ('before','after'):
        open_stage(name, stage, bounds)
        render(folder / f'review/{stage}.png')
        camera = bpy.context.scene.camera
        location, rotation = camera.location.copy(), camera.rotation_euler.copy()
        center = Vector([(low+high)/2 for low,high in zip(*bounds)])
        delta = location-center
        camera.location = center+Vector((-delta.x,-delta.y,delta.z))
        camera.rotation_euler = (center-camera.location).to_track_quat('-Z','Y').to_euler()
        render(folder / f'review/reverse-{stage}.png')
        camera.location, camera.rotation_euler = location, rotation
        if KEYS[name] > 1:
            for frame in (KEYS[name]//2,KEYS[name]-1):
                bpy.context.scene.frame_set(frame)
                render(folder / f'review/action-{stage}-{frame:02d}.png')
            bpy.context.scene.frame_set(0)
        ground_view(folder, stage)
    open_stage(name, 'after', bounds)
    bpy.context.view_layer.material_override = wire_material()
    render(folder / 'review/wireframe.png')
    report = dict(evidence='OFFLINE Blender renders, not client screenshots',
                  replacement='Reimported exported BMD; mesh2 light uses render-only additive approximation', camera='Matching camera, lighting and scale before/after',
                  terrain='Completed TileGrass01, 100-unit repeats as staging proxy; engine density depends on texture width',
                  scale_proxy_height=190, terrain_view_width=VIEW_WIDTH,
                  animation='Hanging01 all25 keys; Carriage01 all21 keys; sample first/middle/last, all other actions original one frame.',
                  placement='Preview at origin, not an actual World1 placement or measured client camera')
    (folder / 'review/render-context.json').write_text(json.dumps(report,indent=2)+'\n')


selected=sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else ASSETS
for asset in selected:
    assert asset in ASSETS
    review(asset)
