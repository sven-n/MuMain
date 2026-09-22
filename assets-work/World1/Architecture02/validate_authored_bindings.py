"""Compare authored Blender triangle corners to the final BMD's re-extracted geometry."""
import json
from pathlib import Path
import sys

sys.dont_write_bytecode = True
import bpy

ROOT = Path(__file__).resolve().parent
OWNED = ('House05', 'HouseWall01', 'HouseWall04', 'HouseWall05', 'HouseWall06', 'Stair01')
POSITION_TOLERANCE = .002
UV_TOLERANCE = .00001


def final_triangles(folder):
    lines = (folder / f'validation/new/{folder.name}.smd').read_text().split('triangles\n')[1].splitlines()
    return [(lines[i], [list(map(float, row.split())) for row in lines[i+1:i+4]]) for i in range(0, len(lines)-1, 4)]


def aligned_error(expected, actual):
    best = None
    for offset in range(3):
        aligned = actual[offset:] + actual[:offset]
        if any(old[0] != new[0] or max(abs(a-b) for a, b in zip(old[7:9], new[7:9])) >= UV_TOLERANCE for old, new in zip(expected, aligned)):
            continue
        error = max(abs(a-b) for old, new in zip(expected, aligned) for a, b in zip(old[1:4], new[1:4]))
        best = min(error, best) if best is not None else error
    return best


def source_triangle(obj, face, bone_indices):
    result = []
    for loop in face.loop_indices:
        vertex = obj.data.vertices[obj.data.loops[loop].vertex_index]
        assert len(vertex.groups) == 1 and vertex.groups[0].weight == 1
        bone = bone_indices[obj.vertex_groups[vertex.groups[0].group].name]
        position = obj.matrix_world @ vertex.co
        normal = obj.data.corner_normals[loop].vector
        uv = obj.data.uv_layers[0].data[loop].uv
        result.append([bone, *position, *normal, *uv])
    return result


def validate(name):
    folder = ROOT / name
    bpy.ops.wm.open_mainfile(filepath=str(folder / 'source.blend'))
    bpy.context.scene.frame_set(0)
    rig = next(obj for obj in bpy.context.scene.objects if obj.type == 'ARMATURE')
    bones = {name: index for index, name in enumerate(rig['mu_bone_order'])}
    mesh = next(iter(bpy.data.collections['EXPORT_' + name].objects))
    actual = final_triangles(folder)
    source = json.loads((folder / 'validation/blender.json').read_text())
    original_count = source['original_vertices_retained']
    new_bones = sorted({mesh.vertex_groups[v.groups[0].group].name for v in mesh.data.vertices[original_count:]})
    maximum, new_triangles = 0, 0
    for face in mesh.data.polygons:
        if all(index < original_count for index in face.vertices):
            continue
        expected = source_triangle(mesh, face, bones)
        texture = mesh.data.materials[face.material_index].name
        matches = [error for material, rows in actual if material == texture and (error := aligned_error(expected, rows)) is not None]
        assert matches and min(matches) < POSITION_TOLERANCE, (name, face.index, min(matches) if matches else 'No triangle with correct bones/UVs')
        maximum, new_triangles = max(maximum, min(matches)), new_triangles + 1
    report = dict(status='PASS', authored_new_vertices=len(mesh.data.vertices)-original_count,
                  authored_panel_triangles_checked=new_triangles, intended_bones=new_bones,
                  every_new_triangle_retains_intended_bone_indices=True,
                  maximum_source_to_final_position_component_delta=maximum, position_tolerance=POSITION_TOLERANCE,
                  uv_tolerance=UV_TOLERANCE,
                  method='Packed Blender authored triangles versus final BMD re-extracted SMD; each cyclically aligned corner must retain material, intended bone index, position and UV. Original corner normalization is audited separately.')
    (folder / 'validation/authored-bindings.json').write_text(json.dumps(report, indent=2) + '\n')
    print(name, 'authored binding PASS', new_triangles, 'triangles;', new_bones, 'maximum drift', maximum)


if __name__ == '__main__':
    for name in OWNED:
        validate(name)
