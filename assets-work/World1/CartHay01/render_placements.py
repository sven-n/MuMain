"""Matching offline comparisons at selected untouched World1 object transforms.

Uses actual translation/rotation/scale and stack spacing; terrain and surroundings
are absent. This is placement evidence in Blender, never a client capture.
"""
import json
import math
from pathlib import Path
import sys
sys.dont_write_bytecode=True
import bpy
from mathutils import Euler,Matrix,Vector
ROOT=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT))
from config import REPOSITORY
sys.path.insert(0,str(ROOT.parent/'StaticBatch01'))
from review_scene import mesh_bounds,render,set_camera,set_lighting

GROUPS={'paired-carts':(('Carriage01',0),('Carriage02',0)),
        'stacked-hay':(('Carriage02',3),('Straw01',2),('Straw01',3)),
        'loaded-cart':(('Carriage04',0),),'crossed-hay':(('Straw02',1),)}


def load_placed(name,stage,record,origin):
    suffix='original/source.blend' if stage=='before' else 'validation/reimported.blend'
    with bpy.data.libraries.load(str(ROOT/name/suffix),link=False) as (source,target):
        target.objects=source.objects
    for obj in target.objects:
        bpy.context.scene.collection.objects.link(obj)
    bpy.context.scene.frame_set(0)
    bpy.context.view_layer.update()
    angles=Euler(tuple(math.radians(value) for value in record['rotation']),'XYZ')
    transform=Matrix.Translation(Vector(record['position'])-origin) @ angles.to_matrix().to_4x4() @ Matrix.Scale(record['scale'],4)
    placed=[]
    for obj in target.objects:
        if obj.type!='MESH' or obj.get('mu_helper'):
            continue
        evaluated=obj.evaluated_get(bpy.context.evaluated_depsgraph_get())
        mesh=bpy.data.meshes.new_from_object(evaluated,preserve_all_data_layers=True,depsgraph=bpy.context.evaluated_depsgraph_get())
        clone=bpy.data.objects.new('PLACED_'+name,mesh)
        bpy.context.scene.collection.objects.link(clone)
        clone.matrix_world=transform @ obj.matrix_world
        placed.append(clone)
    for obj in target.objects:
        bpy.data.objects.remove(obj,do_unlink=True)
    return placed


def light_pane_preview():
    for mat in bpy.data.materials:
        if not mat.name.startswith('horse_drawn_03.jpg') or not mat.use_nodes:
            continue
        nodes,links=mat.node_tree.nodes,mat.node_tree.links
        texture=next(n for n in nodes if n.type=='TEX_IMAGE')
        output=next(n for n in nodes if n.type=='OUTPUT_MATERIAL')
        emission=nodes.new('ShaderNodeEmission');transparent=nodes.new('ShaderNodeBsdfTransparent');add=nodes.new('ShaderNodeAddShader')
        links.new(texture.outputs['Color'],emission.inputs['Color'])
        links.new(transparent.outputs[0],add.inputs[0]);links.new(emission.outputs[0],add.inputs[1]);links.new(add.outputs[0],output.inputs['Surface'])


def review(group,selection):
    records=[dict(model=name,placement=json.loads((ROOT/name/'original/placements.json').read_text())[index]) for name,index in selection]
    origin=Vector(records[0]['placement']['position'])
    bounds=None
    for stage in ('before','after'):
        for obj in list(bpy.data.objects):
            bpy.data.objects.remove(obj,do_unlink=True)
        bpy.context.scene.world=bpy.data.worlds.new('OfflineWorld')
        placed=[]
        for record in records:
            placed.extend(load_placed(record['model'],stage,record['placement'],origin))
        bpy.context.view_layer.update()
        if bounds is None:
            bounds=mesh_bounds(placed)
        set_camera(bounds,1.5);set_lighting();light_pane_preview()
        bpy.context.scene.cycles.samples=8
        bpy.context.scene.render.resolution_x=1200;bpy.context.scene.render.resolution_y=850
        render(ROOT/f'review/placements/{group}-{stage}.png')
    report=dict(evidence='OFFLINE Blender; not client',models=records,world_origin_subtracted=list(origin),
                world_transform='Translation @ EulerXYZ(rotation degrees) @ uniform scale; original records unchanged',
                camera_bounds=bounds,terrain='Terrain and surrounding architecture omitted',
                light='Carriage01 mesh2 uses render-only additive approximation')
    (ROOT/f'review/placements/{group}.json').write_text(json.dumps(report,indent=2)+'\n')


(ROOT/'review/placements').mkdir(parents=True,exist_ok=True)
for group,selection in GROUPS.items():review(group,selection)
