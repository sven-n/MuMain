"""Actual separated/tilted coffin and lid pairs; never snap their distinct origins."""
from pathlib import Path
import json,sys,math,os
sys.dont_write_bytecode=True
import bpy
from mathutils import Vector,Euler,Matrix
ROOT=Path(__file__).resolve().parent;REPO=ROOT.parents[2]
sys.path.insert(0,str(REPO/'assets-work/World1/StaticBatch01'))
from review_scene import mesh_bounds,set_camera,set_lighting,render
PAIRS=((574,575),(806,808),(4122,4123),(4264,4265),(683,685))
EVIDENCE=json.loads((ROOT/'baseline-evidence.json').read_text())


def append(name,index,anchor,stage):
    record=next(p for p in EVIDENCE[name]['placements'] if p['index']==index)
    candidate=ROOT/name/'validation/reimported.blend'
    path=candidate if stage=='candidate' and candidate.exists() else ROOT/name/'baseline/source.blend'
    with bpy.data.libraries.load(str(path)) as (source,loaded):loaded.objects=source.objects
    meshes=[]
    for obj in loaded.objects:
        if obj.type not in ('MESH','ARMATURE') or obj.get('mu_helper') or obj.get('mu_reference'):continue
        bpy.context.scene.collection.objects.link(obj)
        if obj.type=='ARMATURE':
            rotation=Euler([math.radians(value) for value in record['rotation']]).to_matrix().to_4x4()
            obj.matrix_world=Matrix.Translation(Vector(record['position'])-anchor)@rotation@Matrix.Scale(record['scale'],4)
        else:meshes.append(obj)
    return meshes,dict(name=name,source=str(path.relative_to(ROOT)),**record)


def pair(body,lid):
    anchor=Vector(next(p for p in EVIDENCE['Object21']['placements'] if p['index']==body)['position']);bounds=None
    for stage in ('baseline','candidate'):
        for obj in list(bpy.data.objects):bpy.data.objects.remove(obj,do_unlink=True)
        bpy.context.scene.world=bpy.data.worlds.new('ContextWorld');meshes=[];records=[]
        for name,index in (('Object21',body),('Object22',lid)):
            objects,record=append(name,index,anchor,stage);meshes+=objects;records.append(record)
        bpy.context.scene.frame_set(0);bpy.context.view_layer.update()
        if bounds is None:bounds=mesh_bounds([obj.evaluated_get(bpy.context.evaluated_depsgraph_get()) for obj in meshes])
        set_camera(bounds,1.4);set_lighting();scene=bpy.context.scene;scene.render.film_transparent=False;scene.cycles.samples=16
        scene.render.resolution_x=1100;scene.render.resolution_y=850
        label=f'{body}-{lid}-{stage}'
        render(ROOT/'review'/f'{label}.png');scene.render.resolution_percentage=30;render(ROOT/'review'/f'{label}-small.png');scene.render.resolution_percentage=100
        center=(Vector(bounds[0])+Vector(bounds[1]))/2;camera=scene.camera
        camera.location=center+Vector((-1.7,1.7,1.4))*max(Vector(bounds[1])-Vector(bounds[0]));camera.rotation_euler=(center-camera.location).to_track_quat('-Z','Y').to_euler()
        render(ROOT/'review'/f'{label}-reverse.png')
        (ROOT/'review'/f'{label}-context.json').write_text(json.dumps(dict(kind='OFFLINE_ACTUAL_PLACEMENTS_NOT_CLIENT',records=records,anchor=list(anchor),bounds=bounds,excluded='Terrain, lighting bake and unrelated environment'),indent=2)+'\n')


(ROOT/'review').mkdir(exist_ok=True)
selected=os.environ.get('COFFIN_PAIRS')
pairs=[tuple(map(int,item.split(':'))) for item in selected.split(',')] if selected else PAIRS
for body,lid in pairs:pair(body,lid)
