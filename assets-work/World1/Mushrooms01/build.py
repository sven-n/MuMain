"""Round the original mushroom silhouettes while retaining each rigid part's footprint."""
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
from geometry import collection, material, mesh, preserve_original, triangulate
from review_scene import mesh_bounds

SEGMENTS = 12
PROFILE = ((.23, 0), (.20, .25), (.17, .49), (.88, .56), (1, .62), (.90, .77), (.62, .94), (0, 1))


def bounds(points):
    return [Vector(min(v[i] for v in points) for i in range(3)),
            Vector(max(v[i] for v in points) for i in range(3))]


def source_parts(reference, rig):
    parts = {}
    for obj in reference.objects:
        for vertex in obj.data.vertices:
            assert len(vertex.groups) == 1
            bone = obj.vertex_groups[vertex.groups[0].group].name
            parts.setdefault(bone, []).append(obj.matrix_world @ vertex.co)
    return [(name, points, rig.matrix_world @ rig.data.bones[name].matrix_local)
            for name, points in parts.items()]


def create_profile(local_bounds, matrix):
    low, high = local_bounds
    center = (low + high) / 2
    radius = (high - low) / 2
    vertices, rings, polar = [], [], []
    for r, height in PROFILE:
        ring = []
        for index in range(SEGMENTS if r else 1):
            angle = math.tau * index / SEGMENTS
            dx, dy = math.cos(angle), math.sin(angle)
            point = Vector((center.x + radius.x * r * dx, center.y + radius.y * r * dy,
                            low.z + (high.z - low.z) * height))
            ring.append(len(vertices))
            vertices.append(matrix @ point)
            polar.append((dx * r, dy * r, index / SEGMENTS, height))
        rings.append(ring)
    return vertices, rings, polar


def profile_faces(rings, polar, variant):
    faces, uvs = [], []
    cap_center = .25 if variant % 3 else .75
    for level, (lower, upper) in enumerate(zip(rings, rings[1:])):
        for index in range(SEGMENTS):
            nxt = (index + 1) % SEGMENTS
            face = [lower[index], lower[nxt], upper[nxt % len(upper)], upper[index % len(upper)]]
            if len(upper) == 1:
                face.pop()
            coords = []
            for corner in face:
                dx, dy, u, height = polar[corner]
                if level >= 4:
                    coords.append((cap_center + .232 * dx, .75 + .232 * dy))
                else:
                    if index == SEGMENTS - 1 and u == 0:
                        u = 1
                    coords.append((.02 + .96 * u, .02 + .44 * min(height / .62, 1)))
            faces.append(tuple(face))
            uvs.append(coords)
    faces.append(tuple(reversed(rings[0])))
    uvs.append([(.1 + .02 * polar[i][0], .03 + .02 * polar[i][1]) for i in reversed(rings[0])])
    return faces, uvs


def remodel_part(index, name, points, matrix, context):
    local = [matrix.inverted() @ point for point in points]
    vertices, rings, polar = create_profile(bounds(local), matrix)
    before, generated = bounds(points), bounds(vertices)
    # Preserve each mushroom's world-axis contact/extent anchors, including its original tilt.
    vertices = [tuple(before[0][axis] + (v[axis] - generated[0][axis]) *
                     (before[1][axis] - before[0][axis]) / (generated[1][axis] - generated[0][axis])
                     for axis in range(3)) for v in vertices]
    faces, uvs = profile_faces(rings, polar, index)
    result = mesh('Mushroom_' + name, vertices, faces, uvs, name, context, smooth=True)
    proof = {'bone': name, 'bounds_before': [list(v) for v in before],
             'bounds_after': [list(v) for v in bounds([Vector(v) for v in vertices])],
             'original_vertices': len(points)}
    return result, proof


def build(name):
    folder = HERE / name
    bpy.ops.wm.open_mainfile(filepath=str(folder / 'original/source.blend'))
    bpy.context.scene.frame_set(0)
    rig, reference = preserve_original()
    context = (collection('EXPORT_' + name), rig, material(HERE / 'textures/final/mushroom.jpg'))
    objects, proof = [], []
    for index, (bone, points, matrix) in enumerate(source_parts(reference, rig)):
        obj, record = remodel_part(index, bone, points, matrix, context)
        objects.append(obj)
        proof.append(record)
    triangulate(objects)
    before, after = mesh_bounds(list(reference.objects)), mesh_bounds(objects)
    assert max(abs(a-b) for aa,bb in zip(before,after) for a,b in zip(aa,bb)) < .0001
    report = dict(triangles=sum(len(obj.data.polygons) for obj in objects),
                  bounds_before=before, bounds_after=after, parts=proof,
                  bone_order=list(rig['mu_bone_order']), action_meta=rig['mu_action_meta'].to_dict(),
                  materials=['mushroom.jpg'], texture_dimensions=[512, 512])
    (folder / 'validation/blender.json').write_text(json.dumps(report, indent=2) + '\n')
    for obj in objects:
        assert all(len(v.groups) == 1 and v.groups[0].weight == 1 for v in obj.data.vertices)
        assert all(len(face.vertices) == 3 and face.area > 0 for face in obj.data.polygons)
        assert len(obj.data.uv_layers) == 1
    bpy.ops.file.pack_all()
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(folder / 'source.blend'))


for asset in ASSETS:
    build(asset)
