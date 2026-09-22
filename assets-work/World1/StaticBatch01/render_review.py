"""Offline review of re-imported game exports, matched originals, wireframes and scale."""

import json
import math
from pathlib import Path
import sys
import traceback

import bpy
from mathutils import Vector

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(REPOSITORY / 'tools/blender'))
import mu_bmd_import
from geometry import material
from review_scene import render, set_camera, set_lighting

PROPS = ('Candle01', 'TreasureChest01', 'Tomb03')
SCALE_WIDTH = 900
SCALE_HEIGHT = 400
SCALE_WORLD_WIDTH = 900
CHARACTER_HEIGHT = 190


def preview_flames():
    """Approximate the existing additive mesh for inspection only, never export a shader."""
    for mat in bpy.data.materials:
        if not mat.name.endswith('candle2.jpg'):
            continue
        nodes, links = mat.node_tree.nodes, mat.node_tree.links
        texture = next(n for n in nodes if n.type == 'TEX_IMAGE')
        transparent = nodes.new('ShaderNodeBsdfTransparent')
        emission = nodes.new('ShaderNodeEmission')
        emission.inputs['Strength'].default_value = 2
        mix = nodes.new('ShaderNodeMixShader')
        links.new(texture.outputs['Color'], mix.inputs[0])
        links.new(transparent.outputs[0], mix.inputs[1])
        links.new(texture.outputs['Color'], emission.inputs['Color'])
        links.new(emission.outputs[0], mix.inputs[2])
        links.new(mix.outputs[0], nodes.get('Material Output').inputs['Surface'])


def open_model(root, stage, bounds):
    if stage == 'before':
        bpy.ops.wm.open_mainfile(filepath=str(root / 'original/source.blend'))
    else:
        bpy.context.preferences.filepaths.save_version = 0
        sys.argv = ['blender', '--', '--bmd', str(root / f'exports/{root.name}.bmd'),
                    '--out', str(root / 'validation/reimported.blend'), '--textures', str(root / 'textures')]
        mu_bmd_import.main()
    bpy.context.scene.frame_set(0)
    set_camera(bounds)
    set_lighting()
    preview_flames()


def wire_material():
    mat = bpy.data.materials.new('REVIEW_WIREFRAME')
    mat.use_nodes = True
    nodes, links = mat.node_tree.nodes, mat.node_tree.links
    wire = nodes.new('ShaderNodeWireframe')
    wire.inputs['Size'].default_value = .09
    mix = nodes.new('ShaderNodeMixRGB')
    mix.inputs[1].default_value = (.22, .33, .34, 1)
    mix.inputs[2].default_value = (.005, .009, .01, 1)
    emission = nodes.new('ShaderNodeEmission')
    links.new(wire.outputs[0], mix.inputs[0])
    links.new(mix.outputs[0], emission.inputs[0])
    links.new(emission.outputs[0], nodes.get('Material Output').inputs['Surface'])
    return mat


def plain_mesh(name, vertices, faces, mat):
    data = bpy.data.meshes.new(name)
    data.from_pydata(vertices, [], faces)
    data.materials.append(mat)
    obj = bpy.data.objects.new(name, data)
    bpy.context.scene.collection.objects.link(obj)
    return obj


def scale_figure():
    mat = bpy.data.materials.new('REVIEW_190_UNIT_FIGURE')
    mat.diffuse_color = (.11, .15, .17, 1)
    profile = [(16, 0), (20, 70), (26, 130), (31, 142), (15, 153), (14, 176), (0, CHARACTER_HEIGHT)]
    vertices = [(150 + radius * math.cos(i * math.tau / 8), 40 + radius * math.sin(i * math.tau / 8), z)
                for radius, z in profile for i in range(8)]
    faces = [(level * 8 + i, level * 8 + (i + 1) % 8,
              (level + 1) * 8 + (i + 1) % 8, (level + 1) * 8 + i)
             for level in range(len(profile) - 1) for i in range(8)]
    plain_mesh('ScaleProxy_NotGameCharacter', vertices, faces, mat)


def scale_ground(root):
    tile = 'TileGrass01.jpg' if root.name == 'Tomb03' else 'TileGround02.jpg'
    mat = material(ROOT.parent / 'GroundTiles/textures' / tile)
    obj = plain_mesh('OfflineScaleGround_100UnitRepeats',
                     [(-250, -200, -.5), (350, -200, -.5), (350, 300, -.5), (-250, 300, -.5)],
                     [(0, 1, 2, 3)], mat)
    layer = obj.data.uv_layers.new(name='UVMap')
    for point, coordinate in zip(layer.data, ((0, 0), (6, 0), (6, 5), (0, 5))):
        point.uv = coordinate


def scale_view(root, stage):
    scale_ground(root)
    scale_figure()
    scene = bpy.context.scene
    camera = scene.camera
    center = Vector((55, 30, 45))
    camera.location = center + Vector((500, -750, 800))
    camera.rotation_euler = (center - camera.location).to_track_quat('-Z', 'Y').to_euler()
    camera.data.ortho_scale = SCALE_WORLD_WIDTH
    scene.render.resolution_x, scene.render.resolution_y = SCALE_WIDTH, SCALE_HEIGHT
    render(root / f'review/gameplay-{stage}.png')


def action_views(root, stage):
    if root.name not in ('TreasureChest01', 'Candle01'):
        return
    bpy.context.scene.camera.data.ortho_scale *= 1.25
    for frame in (0, 3, 6):
        bpy.context.scene.frame_set(frame)
        render(root / f'review/action-{stage}-{frame}.png')
    bpy.context.scene.frame_set(0)
    bpy.context.scene.camera.data.ortho_scale /= 1.25


def review(root):
    bounds = json.loads((root / 'validation/blender.json').read_text())['bounds_before']
    for stage in ('before', 'after'):
        open_model(root, stage, bounds)
        render(root / f'review/{stage}.png')
        action_views(root, stage)
        scale_view(root, stage)
    # Reopen to avoid including proxy geometry in the wireframe.
    open_model(root, 'after', bounds)
    bpy.context.view_layer.material_override = wire_material()
    render(root / 'review/wireframe.png')
    record = dict(kind='OFFLINE BLENDER RENDERS; NOT CLIENT SCREENSHOTS', after='Re-imported exported BMD',
                  closeup='Identical orthographic camera and diffuse lighting for before/after',
                  scale=dict(world_view_width=SCALE_WORLD_WIDTH, pixels=SCALE_WIDTH,
                             terrain_tile_units=100, proxy_height=CHARACTER_HEIGHT,
                             caveat='Assumed gameplay scale, not a measured client camera or actual placement'),
                  flame_preview='Approximation of existing additive mesh; client result remains unverified')
    (root / 'review/render-context.json').write_text(json.dumps(record, indent=2) + '\n')


def main():
    for prop in PROPS:
        review(ROOT.parent / prop)


if __name__ == '__main__':
    try:
        main()
    except Exception:
        traceback.print_exc()
        sys.exit(1)
