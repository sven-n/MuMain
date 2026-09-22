"""Shared source preservation, validation and packed-file delivery for the three props."""

import json
import math

import bpy
from mathutils import Matrix

from geometry import collection, preserve_original, retain_high_poly, triangulate
from review_scene import mesh_bounds, render, set_camera, set_lighting

MAX_TRIANGLES = 1500
AREA_EPSILON = 0.000001
BOUND_TOLERANCE = .03


def start(root):
    bpy.ops.wm.open_mainfile(filepath=str(root / 'original/source.blend'))
    bpy.context.scene.frame_set(0)
    rig, reference = preserve_original()
    target = collection('EXPORT_' + root.name)
    bounds = mesh_bounds(list(reference.objects))
    return rig, reference, target, bounds


def inspect_skinning(obj, rig):
    for vertex in obj.data.vertices:
        assert len(vertex.groups) == 1, (obj.name, vertex.index, 'influence count')
        group = vertex.groups[0]
        assert group.weight == 1
        name = obj.vertex_groups[group.group].name
        assert name in rig.data.bones and not name.startswith('Du')


def inspect_geometry(objects, rig):
    triangles = 0
    for obj in objects:
        assert len(obj.data.materials) == 1
        assert len(obj.data.uv_layers) == 1
        assert obj.matrix_basis == Matrix.Identity(4), obj.name
        assert obj.parent == rig
        assert len(obj.modifiers) == 1 and obj.modifiers[0].type == 'ARMATURE'
        inspect_skinning(obj, rig)
        for polygon in obj.data.polygons:
            assert len(polygon.vertices) == 3 and polygon.area > AREA_EPSILON, (obj.name, polygon.index)
        uv_max = 1.01 if obj.get('mu_preserved_flames') else 1
        assert all(math.isfinite(value) and 0 <= value <= uv_max
                   for point in obj.data.uv_layers[0].data for value in point.uv)
        assert all(math.isfinite(value) for v in obj.data.vertices for value in v.co)
        triangles += len(obj.data.polygons)
    assert 0 < triangles <= MAX_TRIANGLES, triangles
    return triangles


def validate(root, objects, rig, original_bounds):
    triangles = inspect_geometry(objects, rig)
    current = mesh_bounds(objects)
    for axis in range(3):
        original_size = original_bounds[1][axis] - original_bounds[0][axis]
        for edge in range(2):
            deviation = abs(current[edge][axis] - original_bounds[edge][axis])
            assert deviation <= max(BOUND_TOLERANCE, original_size * .01), (root.name, axis, deviation)
    textures = {m.name: list(n.image.size) for obj in objects for m in obj.data.materials
                for n in m.node_tree.nodes if n.type == 'TEX_IMAGE'}
    for width, height in textures.values():
        assert 0 < width <= 1024 and 0 < height <= 1024
        assert width & (width - 1) == 0 and height & (height - 1) == 0
    result = dict(status='PASS: Blender checks; engine comparison required', triangles=triangles,
                  meshes_after_texture_merge=len(textures), bounds_before=original_bounds, bounds_after=current,
                  bone_order=list(rig['mu_bone_order']), action_meta=rig['mu_action_meta'].to_dict(),
                  textures=textures, skinning='exactly one valid full-weight bone per vertex',
                  geometry='triangles, finite positions/UVs, nonzero face area, identity transforms',
                  uv='one set; new UVs within [0,1]; unchanged candle flame UVs retain legacy repeat to 1.0074')
    (root / 'validation/blender.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result))


def finish(root, rig, target, bounds):
    objects = list(target.objects)
    retain_high_poly(objects)
    triangulate(objects)
    bpy.context.view_layer.update()
    validate(root, objects, rig, bounds)
    set_camera(bounds)
    set_lighting()
    bpy.context.scene['review_status'] = 'OFFLINE BLENDER PREVIEW; CLIENT VERIFICATION PENDING'
    bpy.context.scene.frame_set(0)
    for image in bpy.data.images:
        if image.source == 'FILE':
            image.pack()
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(root / 'source.blend'))
    render(root / 'review/after.png')
