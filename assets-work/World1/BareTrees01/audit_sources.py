"""Audit saved packed projects rather than only the in-memory build result."""
import json
import math
from pathlib import Path
import sys

sys.dont_write_bytecode = True
import bpy

ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT))
from config import ASSETS, MATERIALS


def audit(name):
    folder = ROOT / name
    bpy.ops.wm.open_mainfile(filepath=str(folder / 'source.blend'))
    reference = bpy.data.collections['REF_ORIGINAL']
    assert reference.objects and reference.hide_render
    assert all(image.packed_file for image in bpy.data.images if image.source == 'FILE')
    rig = next(obj for obj in bpy.data.objects if obj.type == 'ARMATURE')
    meshes = list(bpy.data.collections['EXPORT_' + name].objects)
    assert len(meshes) == len(reference.objects)
    triangles = sum(len(obj.data.polygons) for obj in meshes)
    for obj in meshes:
        assert not obj.get('mu_reference', False)
        assert tuple(mat['mu_texture'] for mat in obj.data.materials) == MATERIALS[name]
        for vertex in obj.data.vertices:
            assert len(vertex.groups) == 1 and vertex.groups[0].weight == 1
            bone = obj.vertex_groups[vertex.groups[0].group].name
            assert bone in rig.data.bones and not bone.startswith('Du')
        assert all(math.isfinite(value) for loop in obj.data.uv_layers[0].data for value in loop.uv)
    report = {'status': 'PASS', 'saved_packed_source': True, 'REF_ORIGINAL': True,
              'triangles': triangles, 'rigid_bindings': 'One full-weight original real bone per vertex',
              'materials': list(MATERIALS[name]), 'bone_order': list(rig['mu_bone_order']),
              'UVs': 'Finite; original projection compared separately after official export'}
    (folder / 'validation/source-audit.json').write_text(json.dumps(report, indent=2) + '\n')


for asset in ASSETS:
    audit(asset)
