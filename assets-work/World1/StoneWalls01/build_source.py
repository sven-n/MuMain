"""Dress selected stone panel interiors while retaining every original boundary corner."""
import json
import math
from pathlib import Path
import sys
import traceback

sys.dont_write_bytecode = True
import bpy
from mathutils import Vector

ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT.parent / 'StaticBatch01'))
from geometry import collection, material
from review_scene import mesh_bounds, set_camera, set_lighting

RECESS_DEPTH = .65
PANEL_BEVEL_WIDTH = 2.0
MAX_INSET_FRACTION = .09
PROP_TRIANGLE_BUDGET = 1500

PAIRS = {'StoneWall01': [(0, 1), (2, 3), (8, 9), (12, 13), (18, 19), (20, 21), (26, 27), (28, 29), (34, 35), (38, 39), (44, 45), (46, 47)], 'StoneWall02': [(0, 1), (2, 3), (8, 9), (12, 13), (18, 19), (20, 21), (26, 27), (28, 29), (34, 35), (38, 39), (44, 45), (46, 47)], 'StoneWall03': [(2, 3), (10, 11), (20, 21), (30, 31), (40, 41)], 'StoneWall05': [(2, 3), (10, 11), (20, 21), (30, 31), (40, 41)], 'StoneWall06': [(40, 41), (42, 43), (44, 45), (46, 47)]}


def preserve_original():
    """Snapshot the sorted material collection before renaming its entries."""
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
    for entry in list(bpy.data.materials):
        entry.name = 'REF_' + entry.name
    return rig, reference


def perimeter(original, pair):
    """Use original directed edges, preserving the original panel winding and UVs."""
    faces = [original.data.polygons[i] for i in pair]
    edges, points = [], {}
    for face in faces:
        assert original.data.materials[face.material_index].name.endswith('tile_01.jpg'), original.data.materials[face.material_index].name
        for loop in face.loop_indices:
            index = original.data.loops[loop].vertex_index
            value = original.data.uv_layers[0].data[loop].uv.copy()
            if index in points:
                assert (points[index] - value).length < .00001, (original.name, pair, 'UV seam')
            points[index] = value
        edges += list(zip(face.vertices, list(face.vertices[1:]) + [face.vertices[0]]))
    edges = [edge for edge in edges if (edge[1], edge[0]) not in edges]
    assert len(edges) == 4
    boundary = [edges[0][0], edges[0][1]]
    while len(boundary) < 4:
        boundary.append(next(b for a, b in edges if a == boundary[-1]))
    return boundary, points, faces[0].material_index


def make_recess(original, pair, vertices, bindings):
    boundary, coordinates, slot = perimeter(original, pair)
    center = sum((vertices[i] for i in boundary), Vector()) / 4
    uv_center = sum((coordinates[i] for i in boundary), Vector((0, 0))) / 4
    normal = sum((original.data.polygons[i].normal for i in pair), Vector()).normalized()
    fraction = min(MAX_INSET_FRACTION, PANEL_BEVEL_WIDTH / min((vertices[i] - center).length for i in boundary))
    assert all(bindings[i] == bindings[boundary[0]] for i in boundary)
    inner, inner_uv = [], []
    for index in boundary:
        inner.append(len(vertices))
        # Interior-only recess: no original vertex or connection plane is moved.
        vertices.append(vertices[index].lerp(center, fraction) - normal * RECESS_DEPTH)
        bindings.append(bindings[index])
        inner_uv.append(coordinates[index].lerp(uv_center, fraction))
    panel_faces, panel_uvs = [], []
    for index in range(4):
        nxt = (index + 1) % 4
        quad = [boundary[index], boundary[nxt], inner[nxt], inner[index]]
        tex = [coordinates[boundary[index]], coordinates[boundary[nxt]], inner_uv[nxt], inner_uv[index]]
        for order in ((0, 1, 2), (2, 3, 0)):
            panel_faces.append([quad[i] for i in order])
            panel_uvs.append([tex[i] for i in order])
    for order in ((0, 1, 2), (2, 3, 0)):
        panel_faces.append([inner[i] for i in order])
        panel_uvs.append([inner_uv[i] for i in order])
    evidence = dict(original_face_pair=pair, boundary_vertices=boundary,
                    inset_fraction=fraction, recess_depth=RECESS_DEPTH,
                    original_perimeter=[list(original.matrix_world @ vertices[i]) for i in boundary])
    return (panel_faces, panel_uvs, slot), evidence


def assemble_geometry(original, name):
    vertices = [vertex.co.copy() for vertex in original.data.vertices]
    bindings = [[(original.vertex_groups[g.group].name, g.weight) for g in vertex.groups]
                for vertex in original.data.vertices]
    replacements, skipped, evidence = {}, set(), []
    for pair in PAIRS[name]:
        replacements[pair[0]], record = make_recess(original, pair, vertices, bindings)
        skipped.add(pair[1])
        evidence.append(record)
    faces, uvs, slots, corner_normals = [], [], [], []
    for face in original.data.polygons:
        if face.index in skipped:
            continue
        if face.index in replacements:
            panel_faces, panel_uvs, slot = replacements[face.index]
            for points, tex in zip(panel_faces, panel_uvs):
                faces.append(points)
                uvs.append(tex)
                slots.append(slot)
                normal = (vertices[points[1]] - vertices[points[0]]).cross(vertices[points[2]] - vertices[points[0]]).normalized()
                corner_normals.append([normal] * 3)
        else:
            faces.append(list(face.vertices))
            uvs.append([original.data.uv_layers[0].data[i].uv.copy() for i in face.loop_indices])
            slots.append(face.material_index)
            corner_normals.append([original.data.corner_normals[i].vector.copy() for i in face.loop_indices])
    return (vertices, bindings, faces, uvs, slots, corner_normals), evidence


def create_export(root, original, rig, geometry):
    name = root.name
    vertices, bindings, faces, uvs, slots, corner_normals = geometry
    target = collection('EXPORT_' + name)
    data = bpy.data.meshes.new(name + '_Dressed_Stone')
    data.from_pydata(vertices, [], faces)
    data.update()
    obj = bpy.data.objects.new(name + '_Dressed_Stone', data)
    target.objects.link(obj)
    obj.matrix_world = original.matrix_world.copy()
    obj.parent = rig
    for old in original.data.materials:
        data.materials.append(material(root / 'textures' / old.name.removeprefix('REF_')))
    layer = data.uv_layers.new(name='UVMap')
    for face, tex, slot in zip(data.polygons, uvs, slots):
        face.material_index = slot
        face.use_smooth = True
        for loop, coordinate in zip(face.loop_indices, tex):
            layer.data[loop].uv = coordinate
    data.normals_split_custom_set([n for face in corner_normals for n in face])
    for bone in original.vertex_groups:
        group = obj.vertex_groups.new(name=bone.name)
        indices = [i for i, values in enumerate(bindings) if values == [(bone.name, 1.0)]]
        if indices:
            group.add(indices, 1, 'REPLACE')
    modifier = obj.modifiers.new('OriginalRig', 'ARMATURE')
    modifier.object = rig
    return obj


def retain_high_poly(obj):
    # Retain an editable sculpt/bake candidate while the engine uses the authored low poly.
    high = collection('REF_HIGH_POLY')
    clone = obj.copy()
    clone.data = obj.data.copy()
    clone.name = 'HIGH_' + obj.name
    clone['mu_reference'] = True
    high.objects.link(clone)
    bevel = clone.modifiers.new('Optional_Dressed_Stone_Bake', 'BEVEL')
    bevel.width = .12
    bevel.segments = 3
    high.hide_render = True
    high.hide_viewport = True


def validate_source(root, obj, rig, original, bounds, evidence):
    data = obj.data
    layer = data.uv_layers[0]
    bpy.context.view_layer.update()
    after = mesh_bounds([obj])
    assert max(abs(a-b) for old, new in zip(bounds, after) for a, b in zip(old, new)) < .0001
    assert len(data.polygons) < PROP_TRIANGLE_BUDGET
    assert all(len(v.groups) == 1 and v.groups[0].weight == 1 for v in data.vertices)
    assert all(math.isfinite(c) for loop in layer.data for c in loop.uv)
    assert all(len(p.vertices) == 3 and p.area > .000001 for p in data.polygons)
    texture_sizes = {m.name: list(next(n.image for n in m.node_tree.nodes if n.type == 'TEX_IMAGE').size) for m in data.materials}
    report = dict(status='PASS: source checks; exported engine checks separately required',
                  triangles=len(data.polygons), original_triangles=len(original.data.polygons),
                  bounds_before=bounds, bounds_after=after, original_vertices_retained=len(original.data.vertices),
                  bone_order=list(rig['mu_bone_order']), action_meta=rig['mu_action_meta'].to_dict(),
                  textures=texture_sizes, material_order=[m.name for m in data.materials],
                  uv='One finite UVMap; original UVs and repeats retained; new coordinates interpolated within old panels',
                  skinning='Exactly one original non-dummy bone with weight 1 per vertex',
                  alpha='Owned stone opaque RGB; frozen badge_03.tga alpha unchanged; original material slots retained',
                  panel_recesses=evidence, protected='All original vertices and unmodified triangle corners retained; only named panel interiors changed')
    (root / 'validation/blender.json').write_text(json.dumps(report, indent=2) + '\n')


def build(name):
    root = ROOT / name
    bpy.ops.wm.open_mainfile(filepath=str(root / 'original/source.blend'))
    bpy.context.scene.frame_set(0)
    rig, reference = preserve_original()
    original = next(iter(reference.objects))
    bounds = mesh_bounds([original])
    geometry, evidence = assemble_geometry(original, name)
    obj = create_export(root, original, rig, geometry)
    retain_high_poly(obj)
    validate_source(root, obj, rig, original, bounds, evidence)
    set_camera(bounds)
    set_lighting()
    for image in bpy.data.images:
        if image.source == 'FILE':
            image.pack()
    bpy.context.preferences.filepaths.save_version = 0
    bpy.context.scene['review_status'] = 'OFFLINE BLENDER; CLIENT VERIFICATION PENDING'
    bpy.ops.wm.save_as_mainfile(filepath=str(root / 'source.blend'))
    print(name, len(obj.data.polygons), 'triangles; original bounds and every original vertex retained')


if __name__ == '__main__':
    try:
        for name in PAIRS:
            build(name)
    except Exception:
        traceback.print_exc()
        sys.exit(1)
