"""Show the actual adjacent Lorencia accent-tree transforms, with offline terrain proxy."""
import json
import math
import sys

sys.dont_write_bytecode = True
import bpy
from mathutils import Vector

sys.path.insert(0,__import__('os').path.dirname(__file__))
from config import REPOSITORY, ROOT
sys.path.insert(0,str(ROOT.parent/'StaticBatch01'))
from geometry import material
from render_review import plain_mesh, scale_figure
from review_scene import render, set_lighting

ORIGIN = Vector((11500,10850,165))
VIEW_WIDTH = 1100


def load_tree(name,stage,placement):
    source=ROOT/name/('original/source.blend' if stage=='before' else 'validation/reimported.blend')
    with bpy.data.libraries.load(str(source),link=False) as (available,loaded):
        loaded.objects=list(available.objects)
    anchor=bpy.data.objects.new(name+'_ActualPlacement',None)
    bpy.context.scene.collection.objects.link(anchor)
    anchor.location=Vector(placement['position'])-ORIGIN
    anchor.rotation_euler=tuple(math.radians(value) for value in placement['rotation'])
    anchor.scale=(placement['scale'],)*3
    for obj in loaded.objects:
        if not obj or obj.get('mu_helper') or obj.type not in ('MESH','ARMATURE'):
            continue
        bpy.context.scene.collection.objects.link(obj)
        if obj.parent is None:
            obj.parent=anchor


def setup():
    texture=material(ROOT.parent/'GroundTiles/textures/TileGrass01.jpg')
    obj=plain_mesh('REVIEW_ONLY_TERRAIN_PROXY',
                   [(-700,-600,-1),(700,-600,-1),(700,600,-1),(-700,600,-1)],[(0,1,2,3)],texture)
    uv=obj.data.uv_layers.new(name='UVMap')
    for point,coordinate in zip(uv.data,((0,0),(14,0),(14,12),(0,12))):point.uv=coordinate
    scale_figure()
    bpy.data.objects['ScaleProxy_NotGameCharacter'].location.x=310
    camera=bpy.data.objects.new('OfflineCamera',bpy.data.cameras.new('OfflineCamera'))
    bpy.context.scene.collection.objects.link(camera)
    center=Vector((40,0,150));camera.location=center+Vector((900,-1400,1100))
    camera.rotation_euler=(center-camera.location).to_track_quat('-Z','Y').to_euler()
    camera.data.type='ORTHO';camera.data.ortho_scale=VIEW_WIDTH;camera.data.clip_end=10000
    bpy.context.scene.camera=camera
    set_lighting();bpy.context.scene.cycles.samples=8
    bpy.context.scene.render.resolution_x=1000;bpy.context.scene.render.resolution_y=750


def main():
    inventory=json.loads((ROOT.parent/'coordination/dependency-map.json').read_text())
    placements={name:inventory['models'][name]['placements'][0] for name in ('Tree12','Tree13')}
    (ROOT/'review').mkdir(exist_ok=True)
    for stage in ('before','after'):
        for obj in list(bpy.data.objects):bpy.data.objects.remove(obj,do_unlink=True)
        for name,placement in placements.items():load_tree(name,stage,placement)
        bpy.context.scene.frame_set(0);setup()
        render(ROOT/f'review/actual-accent-pair-{stage}.png')
    context=dict(evidence='OFFLINE: actual object transforms, proxy ground and lighting, not client screenshot',
                 placements=placements,normalization_origin=list(ORIGIN),terrain_units_per_repeat=100,
                 figure_proxy_height=190,world_view_width=VIEW_WIDTH,
                 omitted='Actual terrain height, baked lighting, surrounding town/buildings and other objects')
    (ROOT/'review/actual-pair-context.json').write_text(json.dumps(context,indent=2)+'\n')


main()
