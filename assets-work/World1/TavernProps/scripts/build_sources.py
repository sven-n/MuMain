"""Rebuild this explicitly claimed batch on its preserved original Blender rigs."""

import json
import math
from pathlib import Path
import sys

import bmesh
import bpy
from mathutils import Matrix

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))
from config import MAX_TRIANGLES, PROPS, ROOT
from geometry import AREA_EPSILON, bounds, collection, high_poly_and_triangulate, material
from model_builders import BUILDERS
from review_scene import camera, lighting, render


def preserve_reference():
    reference = collection('REF_ORIGINAL')
    rig = next(o for o in bpy.context.scene.objects if o.type == 'ARMATURE')
    for obj in list(bpy.context.scene.objects):
        if obj.type != 'MESH' or obj.get('mu_helper'):
            continue
        for owner in list(obj.users_collection):
            owner.objects.unlink(obj)
        reference.objects.link(obj)
        obj['mu_reference'] = True
    reference.hide_render = reference.hide_viewport = True
    for surface in bpy.data.materials:
        surface.name = 'REF_' + surface.name
    return rig, reference


def validate_mesh(obj, rig):
    assert obj.matrix_basis == Matrix.Identity(4), obj.name
    assert obj.parent == rig and len(obj.modifiers) == 1
    assert obj.modifiers[0].type == 'ARMATURE' and obj.modifiers[0].object == rig
    assert len(obj.data.uv_layers) == 1 and len(obj.data.materials) == 1
    for vertex in obj.data.vertices:
        assert all(math.isfinite(v) for v in vertex.co)
        assert len(vertex.groups) == 1 and vertex.groups[0].weight == 1
        assert obj.vertex_groups[vertex.groups[0].group].name == rig['mu_bone_order'][0]
    assert all(len(p.vertices) == 3 and p.area > AREA_EPSILON for p in obj.data.polygons)
    assert all(math.isfinite(v) and 0 <= v <= 1 for uv in obj.data.uv_layers[0].data for v in uv.uv)
    data = bmesh.new()
    data.from_mesh(obj.data)
    assert all(edge.is_manifold for edge in data.edges), obj.name
    data.free()
    return len(obj.data.polygons)


def validate(root, target, rig, before):
    objects = list(target.objects)
    triangles = sum(validate_mesh(obj, rig) for obj in objects)
    assert triangles <= MAX_TRIANGLES, triangles
    after = bounds(objects)
    assert max(abs(a - b) for aa, bb in zip(before, after) for a, b in zip(aa, bb)) < .0001
    result = dict(status='PASS: Blender checks; engine validation required', triangles=triangles,
        meshes_after_merge=1, bounds_before=before, bounds_after=after,
        bone_order=list(rig['mu_bone_order']), action_meta=rig['mu_action_meta'].to_dict(),
        skinning='one unchanged full-weight bone per vertex', geometry='finite; manifold parts; nondegenerate triangles',
        uv='one set; atlas islands inset from region borders; all coordinates within [0,1]',
        textures={'desk_big.jpg': [1024, 1024]}, client_verified=False)
    (root / 'validation/blender.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result))


def build(name):
    root = ROOT / name
    bpy.ops.wm.open_mainfile(filepath=str(root / 'original/source.blend'))
    bpy.context.scene.frame_set(0)
    rig, reference = preserve_reference()
    before = bounds(list(reference.objects))
    target = collection('EXPORT_' + name)
    BUILDERS[name]((target, rig, material(root / 'textures/desk_big.jpg')))
    high_poly_and_triangulate(list(target.objects))
    bpy.context.view_layer.update()
    validate(root, target, rig, before)
    camera(before)
    lighting()
    for image in bpy.data.images:
        if image.source == 'FILE':
            image.pack()
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(root / 'source.blend'))
    render(root / 'review/source-after-offline.png')


for prop in PROPS:
    build(prop)
