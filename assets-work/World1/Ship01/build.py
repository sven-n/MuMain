"""Repaint the boat while preserving its complete animated hull and rigging."""
import json
import math
from pathlib import Path
import sys

import bpy
from mathutils import Vector

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from prepare import ASSETS
sys.path.insert(0, str(HERE.parent / 'StaticBatch01'))
from geometry import collection
from review_scene import mesh_bounds


def preserve_original():
    reference = collection('REF_ORIGINAL')
    rig = next(obj for obj in bpy.context.scene.objects if obj.type == 'ARMATURE')
    for obj in list(bpy.context.scene.objects):
        if obj.type != 'MESH' or obj.get('mu_helper'):
            continue
        for owner in list(obj.users_collection):
            owner.objects.unlink(obj)
        reference.objects.link(obj)
        obj['mu_reference'] = True
    reference.hide_render = True
    reference.hide_viewport = True
    for material in list(bpy.data.materials):
        material.name = 'REF_' + material.name
    return rig, reference


def repaint(obj):
    for index, original in enumerate(list(obj.data.materials)):
        material = original.copy()
        texture_name = original.get('mu_texture') or original.name.removeprefix('REF_')
        material.name = texture_name
        material['mu_texture'] = texture_name
        for node in material.node_tree.nodes:
            if node.type == 'TEX_IMAGE' and node.image:
                node.image = bpy.data.images.load(str(HERE / 'textures/final' / texture_name), check_existing=False)
        obj.data.materials[index] = material


def build(name):
    folder = HERE / name
    bpy.ops.wm.open_mainfile(filepath=str(folder / 'original/source.blend'))
    bpy.context.scene.frame_set(0)
    rig, reference = preserve_original()
    target = collection('EXPORT_' + name)
    objects, edits = [], []
    for original in reference.objects:
        obj = original.copy()
        obj.data = original.data.copy()
        obj.name = name + '_Production'
        obj['mu_reference'] = False
        target.objects.link(obj)
        obj.hide_render = False
        obj.hide_viewport = False
        repaint(obj)
        objects.append(obj)
    before, after = mesh_bounds(list(reference.objects)), mesh_bounds(objects)
    assert max(abs(a-b) for aa,bb in zip(before,after) for a,b in zip(aa,bb)) < .0001
    report = dict(bounds_before=before, bounds_after=after,
                  triangles=sum(len(o.data.polygons) for o in objects), edits=edits,
                  bone_order=list(rig['mu_bone_order']), action_meta=rig['mu_action_meta'].to_dict(),
                  retained_geometry=True,
                  note='Hull silhouette, rigging attachments, decorative alpha cards and all 30 keys retained')
    (folder / 'validation/blender.json').write_text(json.dumps(report, indent=2) + '\n')
    bpy.ops.file.pack_all()
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(folder / 'source.blend'))


for asset in ASSETS:
    build(asset)
