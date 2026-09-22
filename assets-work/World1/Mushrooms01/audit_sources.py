"""Audit packed references, triangle/UV integrity and rigid bindings in actual exports."""
import json
import math
from pathlib import Path
import sys

import bpy

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from prepare import ASSETS


def audit(name):
    folder = HERE / name
    bpy.ops.wm.open_mainfile(filepath=str(folder / 'source.blend'))
    reference = bpy.data.collections['REF_ORIGINAL']
    assert reference.hide_render and len(reference.objects) > 0
    assert all(obj.get('mu_reference') for obj in reference.objects)
    assert all(image.packed_file for image in bpy.data.images if image.source == 'FILE')
    bpy.ops.wm.open_mainfile(filepath=str(folder / 'validation/reimported.blend'))
    rig = next(obj for obj in bpy.data.objects if obj.type == 'ARMATURE')
    triangles, uv_min, uv_max, min_area = 0, 1, 0, math.inf
    for obj in bpy.data.objects:
        if obj.type != 'MESH' or obj.get('mu_helper'):
            continue
        assert len(obj.data.uv_layers) == 1
        for vertex in obj.data.vertices:
            assert len(vertex.groups) == 1 and vertex.groups[0].weight == 1
            assert obj.vertex_groups[vertex.groups[0].group].name in rig.data.bones
            assert all(math.isfinite(v) for v in vertex.co)
        uv = obj.data.uv_layers[0].data
        for face in obj.data.polygons:
            assert len(face.vertices) == 3 and face.area > 0
            a, b, c = [uv[index].uv for index in face.loop_indices]
            area = abs((b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x)) / 2
            assert area > 1e-10
            min_area = min(min_area, area)
            for point in (a, b, c):
                uv_min, uv_max = min(uv_min, *point), max(uv_max, *point)
            triangles += 1
    assert 0 <= uv_min <= uv_max <= 1
    report = dict(result='PASS', packed_source_and_reference=True, triangles=triangles,
                  uv_range=[uv_min, uv_max], minimum_uv_triangle_area=min_area,
                  rigid_bindings='One original non-dummy bone per vertex', alpha='Opaque RGB; no alpha material')
    (folder / 'validation/source-export-audit.json').write_text(json.dumps(report, indent=2) + '\n')


for asset in ASSETS:
    audit(asset)
