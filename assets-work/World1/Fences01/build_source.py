"""Rebuild the timber fence and modular curb group, keeping exact original join profiles."""
import json
from pathlib import Path
import sys

import bpy
from mathutils import Vector

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT.parent / 'StaticBatch01'))
from geometry import collection, material, mesh, preserve_original, retain_high_poly, triangulate
from build_support import validate
from review_scene import mesh_bounds, set_camera, set_lighting

NAMES = ('Fence01', 'Fence02', 'Fence03', 'Fence04')
CURB_LENGTH = 50
JOINT_HALF_GAP = .10
JOINT_BEVEL_WIDTH = .50
TOP_RECESS = .42


def original_vertices(root):
    return [Vector(v) for v in json.loads((root / 'original/geometry.json').read_text())[0]['vertices']]


def loft(name, rings, ring_uv, bone, context, caps=True, cap_uv=None):
    vertices = [tuple(v) for ring in rings for v in ring]
    count = len(rings[0])
    faces, uvs = [], []
    for level in range(len(rings) - 1):
        for edge in range(count):
            nxt = (edge + 1) % count
            faces.append((level * count + edge, level * count + nxt,
                          (level + 1) * count + nxt, (level + 1) * count + edge))
            uvs.append(ring_uv(level, edge))
    if caps:
        for level, reverse in ((0, True), (len(rings) - 1, False)):
            face = list(range(level * count, (level + 1) * count))
            faces.append(face[::-1] if reverse else face)
            coordinates = cap_uv or [(0.08, .67), (.45, .67), (.45, .94), (.08, .94)][:count]
            uvs.append(coordinates[::-1] if reverse else coordinates)
    return mesh(name, vertices, faces, uvs, bone, context)


def timber_post(name, corners, low, high, bone, context):
    center = sum(corners, Vector()) / len(corners)
    heights = [low, low + 2, low + 8, high - 7, high - 1.4, high]
    scales = [.94, 1, 1, 1, .95, .76]
    rings = []
    for height, scale in zip(heights, scales):
        rings.append([Vector((center.x + (p.x - center.x) * scale,
                              center.y + (p.y - center.y) * scale, height)) for p in corners])
    def coordinates(level, edge):
        u0, u1 = .06 + edge * .22, .06 + (edge + 1) * .22
        v0, v1 = (heights[level] - low) / (high - low), (heights[level + 1] - low) / (high - low)
        return [(v0, u0), (v0, u1), (v1, u1), (v1, u0)]
    return loft(name, rings, coordinates, bone, context)


def timber_rail(name, vertices, indices, bone, context):
    rings = [[vertices[index] for index in group] for group in indices]
    def coordinates(level, edge):
        u0, u1 = level * .48 + .02, (level + 1) * .48 + .02
        v0, v1 = .12 + edge * .22, .12 + (edge + 1) * .22
        return [(u0, v0), (u0, v1), (u1, v1), (u1, v0)]
    return loft(name, rings, coordinates, bone, context)


def build_timber(vertices, bone, context):
    timber_post('West_Chamfered_Timber_Post', [vertices[i] for i in (26, 27, 28, 29)],
                vertices[30].z, vertices[26].z, bone, context)
    timber_post('East_Chamfered_Timber_Post', [vertices[i] for i in (9, 10, 11, 12)],
                vertices[13].z, vertices[9].z, bone, context)
    timber_rail('Upper_Closed_Sagging_Rail', vertices, ((5, 4, 8), (3, 2, 7), (0, 1, 6)), bone, context)
    timber_rail('Lower_Closed_Sagging_Rail', vertices, ((22, 21, 25), (20, 19, 24), (17, 18, 23)), bone, context)


def contract_profile(profile):
    """Inset only stone joints, leaving external modular interfaces completely unchanged."""
    result = [v.copy() for v in profile]
    across = (profile[3] - profile[0]).normalized()
    for index in (0, 1):
        result[index] += across * .20
    for index in (2, 3):
        result[index] -= across * .20
    for index in (1, 2):
        result[index].z -= TOP_RECESS
    return result


def curb_uv(level, edge, ring_distances, block_index):
    u0, u1 = [(distance / CURB_LENGTH) * .5 + (block_index % 2) * .5
              for distance in ring_distances[level:level + 2]]
    if edge == 0:
        v0, v1 = .015, .985
    elif edge == 1:
        v0, v1 = .68, .92
    elif edge == 2:
        v0, v1 = .985, .015
    else:
        v0, v1 = .62, .68
    return [(u0, v0), (u0, v1), (u1, v1), (u1, v0)]


def build_curb_arm(name, start, end, count, bone, context):
    length = (end[0] - start[0]).length
    for block in range(count):
        start_distance, end_distance = length * block / count, length * (block + 1) / count
        left_gap = JOINT_HALF_GAP if block else 0
        right_gap = JOINT_HALF_GAP if block != count - 1 else 0
        distances = [start_distance + left_gap, start_distance + JOINT_BEVEL_WIDTH,
                     end_distance - JOINT_BEVEL_WIDTH, end_distance - right_gap]
        rings = [[a.lerp(b, distance / length) for a, b in zip(start, end)] for distance in distances]
        if block:
            rings[0] = contract_profile(rings[0])
        if block != count - 1:
            rings[-1] = contract_profile(rings[-1])
        local = [(distance - start_distance) * CURB_LENGTH / (length / count) for distance in distances]
        loft(f'{name}_Dressed_Block_{block + 1}', rings,
             lambda level, edge: curb_uv(level, edge, local, block), bone, context,
             cap_uv=[(.05, .015), (.05, .985), (.30, .985), (.30, .015)])


def build_curbs(name, vertices, bone, context):
    if name == 'Fence02':
        # Exact cross sections at west, original miter and north connection planes.
        west = [vertices[i] for i in (4, 0, 3, 11)]
        miter = [vertices[i] for i in (5, 1, 2, 10)]
        north = [vertices[i] for i in (6, 9, 8, 7)]
        build_curb_arm('West_Arm', west, miter, 2, bone, context)
        build_curb_arm('North_Arm', miter, north, 2, bone, context)
        return
    west = [vertices[i] for i in (4, 0, 3, 7)]
    east = [vertices[i] for i in (5, 1, 2, 6)]
    build_curb_arm('Straight', west, east, 4 if name == 'Fence03' else 2, bone, context)


def save_source(root, rig, target, bounds):
    objects = list(target.objects)
    retain_high_poly(objects)
    triangulate(objects)
    bpy.context.view_layer.update()
    validate(root, objects, rig, bounds)
    report_path = root / 'validation/blender.json'
    report = json.loads(report_path.read_text())
    report['uv'] = 'one UVMap set; all finite coordinates in [0,1]'
    report_path.write_text(json.dumps(report, indent=2) + '\n')
    set_camera(bounds)
    set_lighting()
    for image in bpy.data.images:
        if image.source == 'FILE':
            image.pack()
    bpy.context.preferences.filepaths.save_version = 0
    bpy.context.scene['review_status'] = 'OFFLINE BLENDER; CLIENT VERIFICATION PENDING'
    bpy.ops.wm.save_as_mainfile(filepath=str(root / 'source.blend'))


def build(name):
    root = ROOT / name
    bpy.ops.wm.open_mainfile(filepath=str(root / 'original/source.blend'))
    bpy.context.scene.frame_set(0)
    rig, reference = preserve_original()
    bounds = mesh_bounds(list(reference.objects))
    target = collection('EXPORT_' + name)
    filename = 'tile_wood02.jpg' if name == 'Fence01' else 'joint.jpg'
    context = (target, rig, material(root / 'textures' / filename))
    bone = list(rig['mu_bone_order'])[0]
    vertices = original_vertices(root)
    if name == 'Fence01':
        build_timber(vertices, bone, context)
    else:
        build_curbs(name, vertices, bone, context)
    save_source(root, rig, target, bounds)


if __name__ == '__main__':
    for name in NAMES:
        build(name)
