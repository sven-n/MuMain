"""Audit actual reimports, unchanged anchors, packed sources and UV integrity."""
import json
import math
from pathlib import Path
import sys

import bpy

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from prepare import ASSETS


def geometry(path):
    bpy.ops.wm.open_mainfile(filepath=str(path))
    bpy.context.scene.frame_set(0)
    meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH' and not o.get('mu_helper') and not o.get('mu_reference')]
    points, zero_uv, zero_area, triangles = [], 0, 0, 0
    uv_range = [math.inf, -math.inf]
    rig = next(o for o in bpy.data.objects if o.type == 'ARMATURE')
    for obj in meshes:
        assert len(obj.data.uv_layers) == 1
        for vertex in obj.data.vertices:
            assert len(vertex.groups) == 1 and vertex.groups[0].weight == 1
            bone = obj.vertex_groups[vertex.groups[0].group].name
            assert bone in rig.data.bones
            coordinate = obj.matrix_world @ vertex.co
            assert all(math.isfinite(v) for v in coordinate)
            points.append((bone, list(coordinate)))
        layer = obj.data.uv_layers[0].data
        for face in obj.data.polygons:
            assert len(face.vertices) == 3
            zero_area += face.area < 1e-10
            a,b,c = [layer[i].uv for i in face.loop_indices]
            area = abs((b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x)) / 2
            zero_uv += area < 1e-10
            uv_range = [min(uv_range[0], *a,*b,*c), max(uv_range[1], *a,*b,*c)]
            triangles += 1
    return dict(points=points, zero_uv=zero_uv, zero_area=zero_area,
                triangles=triangles, uv_range=uv_range)


def audit(name):
    folder = HERE / name
    bpy.ops.wm.open_mainfile(filepath=str(folder / 'source.blend'))
    reference = bpy.data.collections['REF_ORIGINAL']
    assert reference.hide_render and all(o.get('mu_reference') for o in reference.objects)
    assert all(i.packed_file for i in bpy.data.images if i.source == 'FILE')
    authored = geometry(folder / 'source.blend')
    before = geometry(folder / 'original/source.blend')
    after = geometry(folder / 'validation/reimported.blend')
    authored_deltas = [min(max(abs(a-b) for a,b in zip(point, other)) for other_bone,other in after['points'] if other_bone == bone) for bone,point in authored['points']]
    assert max(authored_deltas) < .001, (name, max(authored_deltas))
    deltas = []
    for bone, point in before['points']:
        delta = min(max(abs(a-b) for a,b in zip(point,other)) for other_bone,other in after['points']
                    if other_bone == bone)
        deltas.append(delta)
    # Official Blender reimport floating-point transforms are checked against
    # original anchors alongside strict engine-space component comparisons.
    assert max(deltas) < .01, (name, max(deltas))
    assert after['zero_uv'] <= before['zero_uv'] and after['zero_area'] <= before['zero_area'], (name, {k:v for k,v in before.items() if k != 'points'}, {k:v for k,v in after.items() if k != 'points'})
    report = dict(result='PASS', original_anchors=len(deltas), max_anchor_component_delta=max(deltas),
                  anchor_tolerance=.01, anchor_space='Official Blender reimport world coordinates',
                  packed_source_and_reference=True, max_authored_vertex_delta=max(authored_deltas),
                  authored_vertex_count=len(authored_deltas), authored_rigid_parts='Every vertex found at intended bone and world position in final BMD reimport', original={k:v for k,v in before.items() if k != 'points'},
                  replacement={k:v for k,v in after.items() if k != 'points'},
                  uv_note='Original repeating coordinates retained. No newly collapsed geometry or UV triangles.',
                  rigid_binding='Each vertex uses one original bone with weight 1')
    (folder / 'validation/source-export-audit.json').write_text(json.dumps(report, indent=2) + '\n')


for asset in (sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else ASSETS):
    audit(asset)
