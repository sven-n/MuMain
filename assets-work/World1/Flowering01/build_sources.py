"""Preserve economical tree geometry and all sway keys while replacing owned paintings."""
import json
import math
import sys

sys.dont_write_bytecode = True
import bpy

sys.path.insert(0, __import__('os').path.dirname(__file__))
from config import ASSETS, MATERIALS, ROOT
sys.path.insert(0, str(ROOT.parent / 'StaticBatch01'))
from geometry import collection, material, preserve_original
from review_scene import mesh_bounds


def painted_material(filename):
    result = material(ROOT / 'textures/final' / filename)
    if filename.endswith('.tga'):
        nodes = result.node_tree.nodes
        image = next(node for node in nodes if node.type == 'TEX_IMAGE')
        shader = nodes.get('Principled BSDF')
        result.node_tree.links.new(image.outputs['Alpha'],shader.inputs['Alpha'])
    return result


def validate_objects(objects, rig):
    triangles = 0
    uv_limits = []
    for obj in objects:
        assert len(obj.data.uv_layers) == 1
        for vertex in obj.data.vertices:
            assert len(vertex.groups) == 1 and vertex.groups[0].weight == 1
            bone = obj.vertex_groups[vertex.groups[0].group].name
            assert bone in rig.data.bones and not bone.startswith('Du')
            assert all(math.isfinite(value) for value in vertex.co)
        assert all(len(face.vertices) == 3 and face.area > 0 for face in obj.data.polygons)
        values = [value for point in obj.data.uv_layers[0].data for value in point.uv]
        assert all(math.isfinite(value) for value in values)
        uv_limits.append([min(values),max(values)])
        triangles += len(obj.data.polygons)
    return triangles, uv_limits


def copy_geometry(reference, target, materials):
    objects = []
    for source in reference.objects:
        obj = source.copy()
        obj.data = source.data.copy()
        obj.name = source.name + '_REPAINT'
        if 'mu_reference' in obj:
            del obj['mu_reference']
        # Replace slots in place: clearing the list resets polygon material indices.
        for index, original in enumerate(obj.data.materials):
            obj.data.materials[index] = materials[original.name.removeprefix('REF_')]
        target.objects.link(obj)
        objects.append(obj)
    return objects


def build(name):
    folder = ROOT / name
    bpy.ops.wm.open_mainfile(filepath=str(folder / 'original/source.blend'))
    bpy.context.scene.frame_set(0)
    rig, reference = preserve_original()
    materials = {filename:painted_material(filename) for filename in MATERIALS[name]}
    objects = copy_geometry(reference,collection('EXPORT_'+name),materials)
    triangles, uv_limits = validate_objects(objects,rig)
    before, after = mesh_bounds(list(reference.objects)), mesh_bounds(objects)
    assert before == after
    textures = {filename:list(next(n.image.size for n in mat.node_tree.nodes if n.type=='TEX_IMAGE'))
                for filename,mat in materials.items()}
    result = dict(triangles=triangles, mesh_count=len(materials), bounds_before=before, bounds_after=after,
                  geometry='Original trunk/crown geometry retained; surface artwork rebuilt', textures=textures,
                  material_order=MATERIALS[name], bone_order=list(rig['mu_bone_order']),
                  action_meta=rig['mu_action_meta'].to_dict(), legacy_uv_limits=uv_limits,
                  checks='Finite positions/UVs, one UV set, nondegenerate triangles, one valid rigid bone per vertex')
    (folder/'validation/blender.json').write_text(json.dumps(result,indent=2)+'\n')
    for image in bpy.data.images:
        if image.source == 'FILE':
            image.pack()
    bpy.context.preferences.filepaths.save_version = 0
    bpy.context.scene['review_status'] = 'OFFLINE: client review pending'
    bpy.context.scene['geometry_note'] = 'Original low-cost geometry and full sway animation retained'
    bpy.ops.wm.save_as_mainfile(filepath=str(folder/'source.blend'))


for asset in ASSETS:
    build(asset)
