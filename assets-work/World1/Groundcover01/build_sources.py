"""Retain card geometry and rigid skeletons; bind the rebuilt alpha paintings."""
from pathlib import Path
import json
import math
import sys

sys.dont_write_bytecode = True

import bpy

ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
sys.path.insert(0, str(ROOT.parent / 'StaticBatch01'))
from geometry import collection, material, preserve_original
from review_scene import mesh_bounds

ASSETS = ('Grass01', 'Grass02', 'Grass05', 'Grass06')
TEXTURES = {'Grass01': 'tree_08', 'Grass02': 'tree_08', 'Grass05': 'tree_09', 'Grass06': 'tree_09'}


def validate_objects(objects, rig):
    triangles = 0
    for obj in objects:
        assert len(obj.data.uv_layers) == 1
        assert len(obj.data.materials) == 1
        for vertex in obj.data.vertices:
            assert len(vertex.groups) == 1 and vertex.groups[0].weight == 1
            bone = obj.vertex_groups[vertex.groups[0].group].name
            assert bone in rig.data.bones and not bone.startswith('Du')
            assert all(math.isfinite(value) for value in vertex.co)
        for polygon in obj.data.polygons:
            assert len(polygon.vertices) == 3 and polygon.area > 0
        assert all(0 <= value <= 1 for point in obj.data.uv_layers[0].data for value in point.uv)
        triangles += len(obj.data.polygons)
    return triangles


def copy_export_geometry(reference, target, texture):
    objects = []
    for source in reference.objects:
        obj = source.copy()
        obj.data = source.data.copy()
        obj.name = source.name + '_REPAINT'
        if 'mu_reference' in obj:
            del obj['mu_reference']
        obj.data.materials.clear()
        obj.data.materials.append(texture)
        target.objects.link(obj)
        objects.append(obj)
    return objects


def build(name):
    folder = ROOT / name
    bpy.ops.wm.open_mainfile(filepath=str(folder / 'original/source.blend'))
    bpy.context.scene.frame_set(0)
    rig, reference = preserve_original()
    texture = material(ROOT / f'textures/final/{TEXTURES[name]}.tga')
    shader = texture.node_tree.nodes.get('Principled BSDF')
    image = next(node for node in texture.node_tree.nodes if node.type == 'TEX_IMAGE')
    texture.node_tree.links.new(image.outputs['Alpha'], shader.inputs['Alpha'])
    target = collection('EXPORT_' + name)
    objects = copy_export_geometry(reference, target, texture)
    triangles = validate_objects(objects, rig)
    before, after = mesh_bounds(list(reference.objects)), mesh_bounds(objects)
    assert before == after
    result = dict(triangles=triangles, mesh_count=1, geometry='Retained unchanged; repaint only',
                  bounds_before=before, bounds_after=after, texture=list(image.image.size),
                  bone_order=list(rig['mu_bone_order']), action_meta=rig['mu_action_meta'].to_dict(),
                  checks='One UV set, finite coordinates, nondegenerate triangles, one valid bone per vertex, identical bounds')
    (folder / 'validation/blender.json').write_text(json.dumps(result, indent=2) + '\n')
    for image in bpy.data.images:
        if image.source == 'FILE':
            image.pack()
    bpy.context.scene['review_status'] = 'OFFLINE SOURCE: CLIENT REVIEW PENDING'
    bpy.context.scene['geometry_note'] = 'Original economical cards retained. Surface art rebuilt.'
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(folder / 'source.blend'))


for asset in ASSETS:
    build(asset)
