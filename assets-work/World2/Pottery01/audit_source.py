"""Match authored triangles to actual BMD reimport, including material, UV and binding."""
import sys
sys.dont_write_bytecode=True
import json
from pathlib import Path

import bpy
from mathutils import kdtree

TOLERANCE = .0003
UV_TOLERANCE = .000001


def triangles(path):
    bpy.ops.wm.open_mainfile(filepath=str(path))
    bpy.context.scene.frame_set(0)
    result = []
    for obj in bpy.context.scene.objects:
        if obj.type != 'MESH' or obj.get('mu_helper') or obj.get('mu_reference'):
            continue
        if any(c.name.startswith('REF_') for c in obj.users_collection):
            continue
        for face in obj.data.polygons:
            material = obj.data.materials[face.material_index].get('mu_texture', obj.data.materials[face.material_index].name)
            corners = []
            for loop in face.loop_indices:
                vertex = obj.data.vertices[obj.data.loops[loop].vertex_index]
                bone = obj.vertex_groups[vertex.groups[0].group].name
                corners.append((obj.matrix_world @ vertex.co, obj.data.uv_layers[0].data[loop].uv.copy(), bone))
            result.append((material, corners))
    return result


def corner_error(a, b):
    if a[2] != b[2]:
        return float('inf')
    return max((a[0]-b[0]).length, (a[1]-b[1]).length)


def check(folder):
    authored = triangles(folder / 'source.blend')
    actual = triangles(folder / 'validation/reimported.blend')
    assert len(authored) == len(actual)
    maximum = 0
    position_max = uv_max = 0
    unused = set(range(len(actual)))
    for material, corners in authored:
        best_error, best_index = float('inf'), None
        for i in unused:
            if actual[i][0] != material:
                continue
            candidate = actual[i][1]
            error = min(max(corner_error(corners[j], candidate[(j+shift)%3]) for j in range(3)) for shift in range(3))
            if error < best_error:
                best_error, best_index = error, i
            if error < TOLERANCE:
                break
        assert best_index is not None
        assert best_error < .01, (folder.name, best_error)
        selected = actual[best_index][1]
        shift = min(range(3), key=lambda shift: max(corner_error(corners[j], selected[(j+shift)%3]) for j in range(3)))
        position_max = max(position_max, max((corners[j][0]-selected[(j+shift)%3][0]).length for j in range(3)))
        uv_max = max(uv_max, max((corners[j][1]-selected[(j+shift)%3][1]).length for j in range(3)))
        maximum = max(maximum, best_error)
        unused.remove(best_index)
    (folder / 'validation/authored-triangle-match.json').write_text(json.dumps(dict(
        status='PASS' if maximum<TOLERANCE else 'PRECISION_REVIEW', maximum_position_error=position_max, maximum_uv_error=uv_max, triangles=len(authored), maximum_position_or_uv_error=maximum,
        criteria='Every triangle has same material, winding, bone ownership, positions and UV corners',
        position_tolerance=TOLERANCE, uv_tolerance=UV_TOLERANCE,
        precision='Six-decimal SMD roundtrip and float32 coordinates; maximum measured position and UV errors are separately reported against 0.0003 world-unit and 0.000001 UV-unit budgets'), indent=2))
    assert position_max < TOLERANCE and uv_max < UV_TOLERANCE, (folder.name, position_max, uv_max)
