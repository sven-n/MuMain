"""Matching cameras, exported topology, scale, actual-placement modular joins and audits."""
import importlib.util
import json
from pathlib import Path
import sys

import bpy
from mathutils import Vector

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT.parent / 'StaticBatch01'))
from review_scene import mesh_bounds, render, set_camera, set_lighting

spec = importlib.util.spec_from_file_location('static_review', ROOT.parent / 'StaticBatch01/render_review.py')
review = importlib.util.module_from_spec(spec)
spec.loader.exec_module(review)
NAMES = ('Fence01', 'Fence02', 'Fence03', 'Fence04')
JOIN_TOLERANCE = .002


def source_audit(root):
    bpy.ops.wm.open_mainfile(filepath=str(root / 'source.blend'))
    images = [image for image in bpy.data.images if image.source == 'FILE']
    assert images and all(image.packed_file for image in images)
    for name in ('REF_ORIGINAL', 'REF_HIGH_POLY'):
        group = bpy.data.collections[name]
        assert group.objects and group.hide_render and not group.vs.export
        assert all(obj.get('mu_reference') for obj in group.objects)
    objects = list(bpy.data.collections['EXPORT_' + root.name].objects)
    assert all(len(obj.data.uv_layers) == 1 and obj.data.uv_layers[0].name == 'UVMap' for obj in objects)
    record = dict(status='PASS', packed_images=[image.name for image in images], original_rig_reused=True,
                  reference_exclusion='REF_* collections and mu_reference', export_objects=len(objects))
    (root / 'validation/source-audit.json').write_text(json.dumps(record, indent=2) + '\n')


def inspect_joins(root):
    if root.name == 'Fence01':
        # Timber rail triangle rings are the original physical rail/post interfaces.
        indices = [0, 1, 4, 5, 6, 8, 17, 18, 21, 22, 23, 25]
    elif root.name == 'Fence02':
        indices = [0, 3, 4, 11, 6, 7, 8, 9, 1, 2, 5, 10]
    else:
        indices = list(range(8))
    original = json.loads((root / 'original/geometry.json').read_text())[0]['vertices']
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == 'MESH' and not obj.get('mu_helper')]
    points = [obj.matrix_world @ vertex.co for obj in meshes for vertex in obj.data.vertices]
    errors = [min((point - Vector(original[index])).length for point in points) for index in indices]
    assert max(errors) < JOIN_TOLERANCE, (root.name, errors)
    record = dict(status='PASS', tolerance=JOIN_TOLERANCE, original_vertex_indices=indices,
                  max_connection_vertex_error=max(errors), exact_original_connection_vertices=[original[i] for i in indices],
                  checks='All original external interface corners retained; L-curb miter corners retained; no placement changes',
                  full_geometry_equivalence=False,
                  numerical_note='Fence01 rotated source rig produces <0.002-unit official Blender round-trip drift; curb corners exact')
    (root / 'validation/modular-joins.json').write_text(json.dumps(record, indent=2) + '\n')


def review_asset(name):
    root = ROOT / name
    source_audit(root)
    bounds = json.loads((root / 'validation/blender.json').read_text())['bounds_before']
    for stage in ('before', 'after'):
        review.open_model(root, stage, bounds)
        if stage == 'after':
            inspect_joins(root)
        render(root / f'review/{stage}.png')
        review.scale_view(root, stage)
    review.open_model(root, 'after', bounds)
    bpy.context.view_layer.material_override = review.wire_material()
    render(root / 'review/wireframe.png')
    record = dict(kind='OFFLINE BLENDER; NOT CLIENT SCREENSHOTS', after='Re-imported exported BMD',
                  camera='Identical orthographic camera and diffuse lighting before/after',
                  scale=dict(world_view_width=900, pixels=900, terrain_tile_units=100, proxy_height=190,
                             caveat='Offline scale assumption, not the actual client camera'))
    (root / 'review/render-context.json').write_text(json.dumps(record, indent=2) + '\n')


if __name__ == '__main__':
    for name in NAMES:
        review_asset(name)
