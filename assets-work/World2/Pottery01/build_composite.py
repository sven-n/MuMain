"""Propagate the canonical ceramic exterior into three proven affine instances."""
from pathlib import Path
import json
import sys
sys.dont_write_bytecode = True
import bpy
from mathutils import Vector, Matrix
from audit_exports import triangles
from build_source import objects, reference, bounds
ROOT = Path(__file__).resolve().parent


def canonical_faces():
    bpy.ops.wm.open_mainfile(filepath=str(ROOT / 'Object29/source.blend'))
    obj = objects()[0]
    mesh = obj.data
    protected = triangles(ROOT / 'Object29/validation/baseline/Object29.smd')
    base_z = min(row[3] for _, rows in protected for row in rows)
    result = []
    for face in mesh.polygons:
        points = [obj.matrix_world @ mesh.vertices[mesh.loops[i].vertex_index].co for i in face.loop_indices]
        if face.material_index == 1 or all(abs(p.z-base_z) < .00001 for p in points):
            continue
        result.append((face.material_index, [(point.copy(), mesh.uv_layers[0].data[loop].uv.copy(),
            (obj.matrix_world.to_3x3().inverted().transposed() @ mesh.corner_normals[loop].vector).normalized())
            for point, loop in zip(points, face.loop_indices)]))
    assert len(result) == 384
    return result


def assemble_faces(canonical, baseline, parts):
    replaced = {match['composite_triangle'] for part in parts for match in part['matches']
                if 6 <= match['canonical_triangle'] < 70}
    preserved = [index for index in range(len(baseline)) if index not in replaced]
    faces = []
    for index in preserved:
        material, rows = baseline[index]
        material_index = 0 if material == 'flower_vase.jpg' else 1
        assert material in ('flower_vase.jpg', 'deep_wall04.jpg')
        faces.append((material_index, [(Vector(row[1:4]), Vector(row[7:9]), Vector(row[4:7])) for row in rows]))
    for part in parts:
        transform = part['affine_row_vector_4x3']
        matrix = Matrix(transform[:3]).transposed()
        offset = Vector(transform[3])
        normal_matrix = matrix.inverted().transposed()
        assert matrix.determinant() > 0
        for material, corners in canonical:
            faces.append((material, [(matrix @ p + offset, uv.copy(), (normal_matrix @ normal).normalized())
                                     for p, uv, normal in corners]))
    faces.sort(key=lambda item: item[0])
    return faces, preserved


def replace_mesh(obj, faces):
    old = obj.data
    group_names = [group.name for group in obj.vertex_groups]
    assert group_names == ['Cylinder03'], group_names
    vertices, polygons, lookup = [], [], {}
    inverse = obj.matrix_world.inverted()
    for material, corners in faces:
        polygon = []
        for point, uv, normal in corners:
            key = tuple(round(v, 6) for v in point)
            if key not in lookup:
                lookup[key] = len(vertices)
                vertices.append(inverse @ point)
            polygon.append(lookup[key])
        polygons.append(polygon)
    mesh = bpy.data.meshes.new('ThreeAffineWholePotsWithExactBrokenParts')
    mesh.from_pydata(vertices, [], polygons)
    for material in old.materials:
        mesh.materials.append(material)
    uv_layer = mesh.uv_layers.new(name='UVMap')
    obj.data = mesh
    obj.vertex_groups.clear()
    obj.vertex_groups.new(name=group_names[0]).add(list(range(len(vertices))), 1, 'REPLACE')
    normals = [Vector() for _ in mesh.loops]
    normal_to_local = obj.matrix_world.to_3x3().transposed()
    for face, (material, corners) in zip(mesh.polygons, faces):
        face.material_index = material
        face.use_smooth = True
        for loop, (_, uv, normal) in zip(face.loop_indices, corners):
            uv_layer.data[loop].uv = uv
            normals[loop] = (normal_to_local @ normal).normalized()
    mesh.update()
    mesh.normals_split_custom_set(normals)
    assert len(mesh.polygons) == 1476
    assert all(face.area > 1e-8 for face in mesh.polygons)


def build():
    canonical = canonical_faces()
    folder = ROOT / 'Object28'
    parts = json.loads((ROOT / 'shared-whole-pot-proof.json').read_text())['parts']
    baseline = triangles(folder / 'validation/baseline/Object28.smd')
    faces, preserved = assemble_faces(canonical, baseline, parts)
    bpy.ops.wm.open_mainfile(filepath=str(folder / 'baseline/source.blend'))
    obj = objects()[0]
    before = bounds([obj.matrix_world @ v.co for v in obj.data.vertices])
    reference(obj)
    replace_mesh(obj, faces)
    after = bounds([obj.matrix_world @ v.co for v in obj.data.vertices])
    assert max(abs(a-b) for aa, bb in zip(before, after) for a,b in zip(aa,bb)) < .0003
    bpy.ops.file.pack_all()
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(folder / 'source.blend'))
    (folder / 'validation/source.json').write_text(json.dumps(dict(triangles=len(obj.data.polygons),
        protected_triangles=preserved, bounds_before=before, bounds_after=after,
        design='Three affine canonical exteriors; all broken components, three bases and throats preserved'), indent=2))


if __name__ == '__main__':
    build()
