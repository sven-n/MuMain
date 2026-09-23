"""Design a continuous ceramic shoulder and rolled lip around the frozen throat."""
from pathlib import Path
import json
import math
import sys
sys.dont_write_bytecode = True
import bpy
from mathutils import Vector, Euler, Matrix
ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT))
from audit_exports import triangles
SEGMENTS = 16
SEGMENTS_PER_SECTOR = SEGMENTS // 8


def objects():
    return [o for o in bpy.context.scene.objects if o.type == 'MESH' and not o.get('mu_helper')]


def bounds(points):
    return [[min(p[k] for p in points) for k in range(3)], [max(p[k] for p in points) for k in range(3)]]


def support_planes(points, placements):
    planes = []
    for record in placements:
        rotation = Euler([math.radians(v) for v in record['rotation']], 'XYZ').to_matrix()
        for axis in range(3):
            normal = Vector(rotation[axis])
            planes.extend([(normal, max(normal.dot(p) for p in points)),
                           (-normal, max(-normal.dot(p) for p in points))])
    return planes


def shared_support_planes(points):
    """Canonical support halfspaces for each transformed whole-pot instance."""
    parts = json.loads((ROOT / 'shared-whole-pot-proof.json').read_text())['parts']
    placements = json.loads((ROOT / 'Object28/placements.json').read_text())
    planes = []
    for part in parts:
        transform = part['affine_row_vector_4x3']
        matrix = Matrix(transform[:3]).transposed()
        for record in placements:
            rotation = Euler([math.radians(v) for v in record['rotation']], 'XYZ').to_matrix()
            for axis in range(3):
                normal = (matrix.transposed() @ Vector(rotation[axis])).normalized()
                planes.extend([(normal, max(normal.dot(p) for p in points)),
                               (-normal, max(-normal.dot(p) for p in points))])
    return planes


def constrain(point, planes):
    result = point.copy()
    for iteration in range(100):
        maximum = 0
        for normal, limit in planes:
            excess = normal.dot(result) - limit
            if excess > 0:
                result -= normal * excess
                maximum = max(maximum, excess)
        if maximum < .000001:
            return result
    raise AssertionError('Placement support projection did not converge')


def reference(obj):
    collection = bpy.data.collections.new('REF_ORIGINAL')
    bpy.context.scene.collection.children.link(collection)
    duplicate = obj.copy()
    duplicate.data = obj.data.copy()
    duplicate['mu_reference'] = True
    collection.objects.link(duplicate)
    collection.hide_render = collection.hide_viewport = True


def build():
    folder = ROOT / 'Object29'
    bpy.ops.wm.open_mainfile(filepath=str(folder / 'baseline/source.blend'))
    bpy.context.scene.frame_set(0)
    obj = objects()[0]
    original = obj.data
    group_names = [group.name for group in obj.vertex_groups]
    assert group_names == ['Cylinder05'], group_names
    reference(obj)
    assert len(original.polygons) == 78
    world = [obj.matrix_world @ v.co for v in original.vertices]
    before = bounds(world)
    planes = support_planes(world, json.loads((folder / 'placements.json').read_text()))
    planes.extend(shared_support_planes(world))
    rows = triangles(folder / 'validation/baseline/Object29.smd')
    levels = sorted(set(round(p.z, 5) for p in world))
    base_z, top_z = min(p.z for p in world), max(p.z for p in world)
    center = Vector(((before[0][0] + before[1][0]) / 2, (before[0][1] + before[1][1]) / 2, 0))
    rings = {}
    for face in original.polygons:
        if face.material_index != 0:
            continue
        for loop in face.loop_indices:
            point = world[original.loops[loop].vertex_index]
            uv = original.uv_layers[0].data[loop].uv.copy()
            rings.setdefault(round(point.z, 4), {})[round(uv.x % 1, 5)] = (point.copy(), uv.copy())
    heights = sorted(rings)
    assert len(heights) == 5
    radius = [(max(p.x for p, uv in rings[z].values()) - min(p.x for p, uv in rings[z].values())) / 2 for z in heights]
    # The existing painted bands remain at their original heights. Intermediate
    # rows use a designed rounded lower belly and eased shoulder, not linear facets.
    profile = [(heights[0], radius[0], .0005), (7, 12.9, .144),
               (heights[1], radius[1], .4023), (24, 18.08, .497),
               (heights[2], radius[2], .6261),
               (33, 17.70, .685), (36, 14.60, .747), (39, 10.65, .810),
               (heights[3], radius[3], .8636),
               (46.0, 9.50, .946), (47.2, 10.15, .970),
               (top_z - .65, 9.7, .987), (top_z, radius[4], .9995)]
    generated = []
    for ring_index, (z, r, v) in enumerate(profile):
        values = []
        for segment in range(SEGMENTS + 1):
            u = segment / SEGMENTS
            angle = 2 * math.pi * u
            point = Vector((center.x - math.sin(angle) * r, center.y + math.cos(angle) * r, z))
            # Actual base polygon and throat interface remain exactly registered.
            if ring_index in (0, len(profile) - 1):
                source = rings[heights[0 if ring_index == 0 else 4]]
                cell = segment / SEGMENTS_PER_SECTOR
                left = int(cell) % 8
                factor = cell - int(cell)
                point = source[round(left / 8, 5)][0].lerp(source[round(((left + 1) % 8) / 8, 5)][0], factor)
            elif round(z, 4) in rings and segment % SEGMENTS_PER_SECTOR == 0:
                point = rings[round(z, 4)][round((u % 1), 5)][0].copy()
            values.append((constrain(point, planes), Vector((u, v))))
        generated.append(values)
    faces = []
    for low, high in zip(generated, generated[1:]):
        for i in range(SEGMENTS):
            faces.extend([(0, [low[i], low[i + 1], high[i]], None),
                          (0, [high[i], low[i + 1], high[i + 1]], None)])
    preserved = []
    for face in original.polygons:
        points = [world[original.loops[i].vertex_index] for i in face.loop_indices]
        if face.material_index != 1 and not all(abs(p.z - base_z) < .00001 for p in points):
            continue
        corners = [(p, original.uv_layers[0].data[loop].uv.copy()) for p, loop in zip(points, face.loop_indices)]
        faces.append((face.material_index, corners, face.index))
        preserved.append(face.index)
    faces.sort(key=lambda f: f[0])
    mesh = bpy.data.meshes.new('DungeonCeramicBodyWithFrozenThroat')
    vertices, indices, lookup = [], [], {}
    for material, corners, old_index in faces:
        polygon = []
        for point, uv in corners:
            key = tuple(round(v, 6) for v in point)
            if key not in lookup:
                lookup[key] = len(vertices)
                vertices.append(obj.matrix_world.inverted() @ point)
            polygon.append(lookup[key])
        indices.append(polygon)
    mesh.from_pydata(vertices, [], indices)
    for material in original.materials:
        mesh.materials.append(material)
    uv_layer = mesh.uv_layers.new(name='UVMap')
    obj.data = mesh
    obj.vertex_groups.clear()
    for name in group_names:
        obj.vertex_groups.new(name=name)
    obj.vertex_groups[0].add(list(range(len(vertices))), 1, 'REPLACE')
    mesh.update()
    assert all(face.area > 1e-8 for face in mesh.polygons), 'Authored degenerate triangle'
    normals = [Vector() for loop in mesh.loops]
    neighbors = {}
    for face, (material, corners, old_index) in zip(mesh.polygons, faces):
        face.material_index = material
        face.use_smooth = True
        for loop, (_, uv) in zip(face.loop_indices, corners):
            uv_layer.data[loop].uv = uv
            neighbors.setdefault(mesh.loops[loop].vertex_index, []).append(face.normal.copy())
    for face, (_, corners, old_index) in zip(mesh.polygons, faces):
        for corner, loop in enumerate(face.loop_indices):
            if old_index is not None:
                normals[loop] = Vector(rows[old_index][1][corner][4:7])
            else:
                candidates = neighbors[mesh.loops[loop].vertex_index]
                normals[loop] = sum((n for n in candidates if n.dot(face.normal) > .72), Vector()).normalized()
    mesh.normals_split_custom_set(normals)
    after = bounds([obj.matrix_world @ v.co for v in mesh.vertices])
    assert max(abs(a - b) for aa, bb in zip(before, after) for a, b in zip(aa, bb)) < .0003
    assert len(mesh.polygons) <= 1500
    bpy.ops.file.pack_all()
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(folder / 'source.blend'))
    (folder / 'validation/source.json').write_text(json.dumps(dict(triangles=len(mesh.polygons),
        bounds_before=before, bounds_after=after, protected_triangles=preserved,
        design='Continuous ceramic belly/shoulder, thick rolled lip; original octagonal throat and bottom surfaces exact',
        actual_placement_support_planes=len(planes)), indent=2))


if __name__ == '__main__':
    build()
    from build_composite import build as build_composite
    build_composite()
