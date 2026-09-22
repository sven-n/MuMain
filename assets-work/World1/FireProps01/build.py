"""Refine the standing brazier support; retain all flame, basket and gateway anchors."""
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
from geometry import collection, preserve_original
from review_scene import mesh_bounds


def rebuild_stem(obj):
    source = obj.data
    group_names = [group.name for group in obj.vertex_groups]
    vertices = [v.co.copy() for v in source.vertices]
    weights = [[(g.group, g.weight) for g in v.groups] for v in source.vertices]
    faces, uvs, materials, normals = [], [], [], []
    selected = {v.index for v in source.vertices if abs(v.co.x) < 5 and abs(v.co.y) < 5
                and (abs(v.co.z + .3591) < .001 or abs(v.co.z - 136.5269) < .001)}
    assert len(selected) == 8
    removed = 0
    for face in source.polygons:
        if face.material_index == 1 and set(face.vertices) <= selected:
            removed += 1
            continue
        faces.append(tuple(face.vertices))
        materials.append(face.material_index)
        uvs.append([tuple(source.uv_layers.active.data[i].uv) for i in face.loop_indices])
        normals.extend([tuple(source.corner_normals[i].vector) for i in face.loop_indices])
    assert removed == 8
    center = Vector((-.064, -.1792, 0))
    low = sorted((i for i in selected if vertices[i].z < 0),
                 key=lambda i: math.atan2(vertices[i].y-center.y, vertices[i].x-center.x))
    high = sorted((i for i in selected if vertices[i].z > 100),
                  key=lambda i: math.atan2(vertices[i].y-center.y, vertices[i].x-center.x))
    low_z, high_z = vertices[low[0]].z, vertices[high[0]].z
    start = math.atan2(vertices[low[0]].y-center.y, vertices[low[0]].x-center.x)
    middle = []
    for height in (low_z + 1.6, high_z - 1.6):
        ring = []
        for i in range(8):
            angle = start + math.tau * i / 8
            ring.append(len(vertices))
            vertices.append(Vector((center.x + 4.6567 * math.cos(angle),
                                    center.y + 4.6567 * math.sin(angle), height)))
            weights.append(weights[low[0]])
        middle.append(ring)
    authored = []
    for end, mid in ((low, middle[0]), (high, middle[1])):
        for i in range(4):
            nxt = (i+1) % 4
            a, b, c = mid[2*i], mid[(2*i+1) % 8], mid[(2*i+2) % 8]
            authored.extend([(end[i], end[nxt], c), (end[i], c, b), (end[i], b, a)])
    for i in range(8):
        nxt = (i+1) % 8
        authored.extend([(middle[0][i], middle[0][nxt], middle[1][nxt]),
                         (middle[0][i], middle[1][nxt], middle[1][i])])
    for face in authored:
        points = [vertices[i] for i in face]
        normal = (points[1]-points[0]).cross(points[2]-points[0]).normalized()
        radial = sum(points, Vector()) / 3 - center
        radial.z = 0
        if normal.dot(radial) < 0:
            face = face[::-1]
            normal = -normal
        coords = []
        for index in face:
            point = vertices[index]
            angle = (math.atan2(point.y-center.y, point.x-center.x) - start) % math.tau
            coords.append([angle / math.tau, (point.z-low_z) / (high_z-low_z)])
        if max(v[0] for v in coords) - min(v[0] for v in coords) > .5:
            coords = [[u+1 if u < .5 else u, v] for u,v in coords]
        faces.append(face)
        uvs.append(coords)
        materials.append(1)
        normals.extend([tuple(normal)] * 3)
    data = bpy.data.meshes.new('StandingBrazier_OctagonalSupport')
    data.from_pydata(vertices, [], faces)
    for mat in source.materials:
        data.materials.append(mat)
    layer = data.uv_layers.new(name='UVMap')
    for face, coords, material in zip(data.polygons, uvs, materials):
        face.material_index = material
        for loop, point in zip(face.loop_indices, coords):
            layer.data[loop].uv = point
    data.normals_split_custom_set(normals)
    obj.data = data
    for name in group_names:
        if name not in obj.vertex_groups:
            obj.vertex_groups.new(name=name)
    for index, groups in enumerate(weights):
        for group, weight in groups:
            obj.vertex_groups[group].add([index], weight, 'REPLACE')
    return dict(original_stem_triangles=removed, new_stem_triangles=len(authored),
                endpoints='All eight original end corners retained exactly',
                protected_materials=['light.tga', 'light3.jpg'])


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
        if name == 'FireLight01':
            edits.append(rebuild_stem(obj))
        repaint(obj)
        objects.append(obj)
    before, after = mesh_bounds(list(reference.objects)), mesh_bounds(objects)
    assert max(abs(a-b) for aa,bb in zip(before,after) for a,b in zip(aa,bb)) < .0001
    report = dict(bounds_before=before, bounds_after=after,
                  triangles=sum(len(o.data.polygons) for o in objects), edits=edits,
                  bone_order=list(rig['mu_bone_order']), action_meta=rig['mu_action_meta'].to_dict(),
                  retained_geometry=name != 'FireLight01',
                  note='Gateway opening, fire offsets and effect mesh geometry/UVs retained')
    (folder / 'validation/blender.json').write_text(json.dumps(report, indent=2) + '\n')
    bpy.ops.file.pack_all()
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(folder / 'source.blend'))


for asset in ASSETS:
    build(asset)
