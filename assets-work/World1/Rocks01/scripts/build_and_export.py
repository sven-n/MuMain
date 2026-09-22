"""Refine rock ridges and shared paint while preserving fountain geometry and animation."""
import hashlib
import json
from pathlib import Path
import shutil
import sys
import bpy
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))
from config import CONVERTER, PROPS, REPOSITORY, ROOT, ROCKS
from refine_edges import refine
sys.path.insert(0, str(REPOSITORY / 'tools/blender'))
import mu_bmd_export


def new_material(original):
    name = original.name.removeprefix('REF_')
    material = bpy.data.materials.new(name)
    material.use_nodes = True
    material['mu_texture'] = name
    nodes, links = material.node_tree.nodes, material.node_tree.links
    nodes.clear()
    output = nodes.new('ShaderNodeOutputMaterial')
    diffuse = nodes.new('ShaderNodeBsdfDiffuse')
    transparent = nodes.new('ShaderNodeBsdfTransparent')
    mix = nodes.new('ShaderNodeMixShader')
    texture = nodes.new('ShaderNodeTexImage')
    path = ROOT / 'textures' / name
    if not path.exists():
        path = ROOT / 'original' / name
    texture.image = bpy.data.images.load(str(path))
    texture.image.pack()
    links.new(texture.outputs['Color'], diffuse.inputs['Color'])
    links.new(texture.outputs['Alpha'], mix.inputs[0])
    links.new(transparent.outputs[0], mix.inputs[1])
    links.new(diffuse.outputs[0], mix.inputs[2])
    links.new(mix.outputs[0], output.inputs['Surface'])
    return material


def snapshot(objects):
    return [{'positions': [list(vertex.co) for vertex in obj.data.vertices],
             'polygons': [list(face.vertices) for face in obj.data.polygons],
             'uv': [[list(corner.uv) for corner in layer.data] for layer in obj.data.uv_layers],
             'weights': [[(obj.vertex_groups[group.group].name, group.weight) for group in vertex.groups]
                         for vertex in obj.data.vertices],
             'matrix': [list(row) for row in obj.matrix_world]} for obj in objects]


def preserve_and_duplicate(originals):
    reference = bpy.data.collections.new('REF_ORIGINAL')
    bpy.context.scene.collection.children.link(reference)
    target = bpy.data.collections.new('EXPORT_PAINTED')
    bpy.context.scene.collection.children.link(target)
    # The official exporter creates its own joined object outside these source groups.
    reference.vs.export = target.vs.export = False
    originals_materials = {material for obj in originals for material in obj.data.materials}
    for material in originals_materials:
        material.name = 'REF_' + material.name
    replacement_materials = {material.name: new_material(material) for material in originals_materials}
    for obj in originals:
        for owner in list(obj.users_collection):
            owner.objects.unlink(obj)
        reference.objects.link(obj)
        obj['mu_reference'] = True
        clone = obj.copy()
        clone.data = obj.data.copy()
        clone.name = 'Painted_' + obj.name
        del clone['mu_reference']
        target.objects.link(clone)
        for index, material in enumerate(clone.data.materials):
            clone.data.materials[index] = replacement_materials[material.name]
    reference.hide_render = reference.hide_viewport = True
    return list(target.objects)


def build(name):
    root = ROOT / name
    bpy.ops.wm.open_mainfile(filepath=str(root / 'original/source.blend'))
    bpy.context.scene.frame_set(0)
    originals = [obj for obj in bpy.context.scene.objects if obj.type == 'MESH' and not obj.get('mu_helper')]
    original_state = snapshot(originals)
    active = preserve_and_duplicate(originals)
    assert snapshot(active) == original_state
    refinement = [refine(obj) for obj in active] if name in ROCKS else []
    assert all(len(vertex.groups) == 1 and vertex.groups[0].weight == 1
               for obj in active for vertex in obj.data.vertices)
    assert all(len(obj.data.uv_layers) == 1 for obj in active)
    rig = next(obj for obj in bpy.context.scene.objects if obj.type == 'ARMATURE')
    report = {'original_reference_unchanged': True, 'geometry_unchanged': name not in ROCKS,
              'ridge_refinement': refinement,
              'numeric_snapshot_sha256': hashlib.sha256(json.dumps(original_state, sort_keys=True).encode()).hexdigest(),
              'triangles': sum(len(face.vertices)-2 for obj in active for face in obj.data.polygons),
              'bone_order': list(rig['mu_bone_order']), 'action_meta': rig['mu_action_meta'].to_dict(),
              'material_order': [material.name for obj in active for material in obj.data.materials],
              'packed_images': {image.name: list(image.size) for image in bpy.data.images if image.packed_file},
              'client_verified': False}
    (root / 'validation').mkdir(exist_ok=True)
    (root / 'validation/source.json').write_text(json.dumps(report, indent=2)+'\n')
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(root / 'source.blend'))
    (root / 'exports').mkdir(exist_ok=True)
    for material_name in report['material_order']:
        extension = '.OZT' if material_name.endswith('.tga') else '.OZJ'
        texture_name = Path(material_name).stem + extension
        path = ROOT / 'textures' / texture_name
        if not path.exists():
            path = ROOT / 'original' / texture_name
        shutil.copy2(path, root / 'exports' / texture_name)
    sys.argv = ['blender','--','--bmdconv',str(CONVERTER),'--out',str(root / 'exports' / f'{name}.bmd')]
    mu_bmd_export.main()
    if name == 'Waterspout01':
        # Only the shared base texture changes. Preserve exact animated game bytes.
        shutil.copy2(root / 'exports' / f'{name}.bmd', root / 'validation/official-roundtrip.bmd')
        shutil.copy2(root / 'original' / f'{name}.bmd', root / 'exports' / f'{name}.bmd')


for prop in PROPS:
    build(prop)
