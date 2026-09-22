"""Load artists' packed sources read-only; save only independent review evidence."""

import hashlib
import json
from pathlib import Path

import bpy

OUTPUT = Path(__file__).resolve().parent
REVIEW_REPO = OUTPUT.parents[3]
ASSETS = REVIEW_REPO.parent / 'MuMain-tavern-props/assets-work/World1/TavernProps'
NAMES = ('Furniture03', 'Furniture04', 'Furniture05')


def mesh_snapshot(objects):
    return sorted([{'name': obj.name,
                    'vertices': [list(vertex.co) for vertex in obj.data.vertices],
                    'polygons': [list(face.vertices) for face in obj.data.polygons],
                    'uv': [[list(corner.uv) for corner in layer.data] for layer in obj.data.uv_layers],
                    'transform': [list(row) for row in obj.matrix_world],
                    'weights': [[(obj.vertex_groups[group.group].name, group.weight)
                                 for group in vertex.groups] for vertex in obj.data.vertices]}
                   for obj in objects], key=lambda record: record['name'])


def rig_snapshot():
    rigs = [obj for obj in bpy.context.scene.objects if obj.type == 'ARMATURE']
    return [{'bones': [(bone.name, bone.parent.name if bone.parent else None)
                       for bone in rig.data.bones],
             'mu_bone_order': list(rig['mu_bone_order']),
             'mu_action_meta': rig['mu_action_meta'].to_dict(),
             'transform': [list(row) for row in rig.matrix_world]} for rig in rigs]


def packed_images():
    images = [image for image in bpy.data.images if image.source == 'FILE']
    assert all(image.packed_file for image in images)
    return {image.name: {'size': list(image.size),
                         'sha256': hashlib.sha256(image.packed_file.data).hexdigest()}
            for image in images}


def inspect(name):
    root = ASSETS / name
    bpy.ops.wm.open_mainfile(filepath=str(root / 'original/source.blend'))
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == 'MESH' and not obj.get('mu_helper')]
    original, original_rig = mesh_snapshot(meshes), rig_snapshot()
    bpy.ops.wm.open_mainfile(filepath=str(root / 'source.blend'))
    reference = bpy.data.collections['REF_ORIGINAL']
    high_poly = bpy.data.collections['REF_HIGH_POLY']
    assert reference.hide_render and reference.hide_viewport
    assert high_poly.hide_render and high_poly.hide_viewport
    assert all(obj.get('mu_reference') for obj in [*reference.objects, *high_poly.objects])
    assert mesh_snapshot(reference.objects) == original
    assert rig_snapshot() == original_rig, (original_rig, rig_snapshot())
    active = [obj for obj in bpy.context.scene.objects if obj.type == 'MESH'
              and not obj.get('mu_reference') and not obj.get('mu_helper')]
    assert active
    assert all(len(obj.data.uv_layers) == 1 for obj in active)
    assert all(len(vertex.groups) == 1 and vertex.groups[0].weight == 1
               for obj in active for vertex in obj.data.vertices)
    material_names = sorted({material.name for obj in active for material in obj.data.materials})
    assert material_names == ['desk_big.jpg']
    images = packed_images()
    assert len(images) == 2
    return {'status': 'PASS', 'ref_original_numeric_geometry_uv_weights_transform': 'identical',
            'original_snapshot_sha256': hashlib.sha256(json.dumps(original, sort_keys=True).encode()).hexdigest(),
            'reference_and_high_poly_excluded': True, 'packed_images': images,
            'rig_and_metadata': original_rig, 'active_mesh_objects': len(active),
            'materials': material_names, 'uv_layers_per_mesh': 1, 'bone_influences_per_vertex': 1}


report = {name: inspect(name) for name in NAMES}
(OUTPUT / 'source-audit.json').write_text(json.dumps(report, indent=2) + '\n')
print(json.dumps(report, indent=2))
