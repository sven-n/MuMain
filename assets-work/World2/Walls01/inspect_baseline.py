"""Untouched wall/pillar identity and actual neighborhood before remodeling."""
from pathlib import Path
import json,sys,math,os
sys.dont_write_bytecode=True
import bpy
from mathutils import Vector,Euler,Matrix
ROOT=Path(__file__).resolve().parent;REPO=ROOT.parents[2]
sys.path.insert(0,str(REPO/'assets-work/World1/StaticBatch01'))
from review_scene import mesh_bounds,set_camera,set_lighting,render

def setup(bounds):
    set_camera(bounds,1.75);set_lighting();scene=bpy.context.scene
    scene.render.film_transparent=False;scene.render.resolution_x=1100;scene.render.resolution_y=850;scene.cycles.samples=16

def views(bounds,folder,label):
    render(folder/(label+'.png'));scene=bpy.context.scene
    scene.render.resolution_percentage=30;render(folder/(label+'-small.png'));scene.render.resolution_percentage=100
    center=(Vector(bounds[0])+Vector(bounds[1]))/2;span=max(Vector(bounds[1])-Vector(bounds[0]));camera=scene.camera
    camera.location=center+Vector((-1.6,1.8,1.2))*span;camera.rotation_euler=(center-camera.location).to_track_quat('-Z','Y').to_euler()
    render(folder/(label+'-reverse.png'))

def individual(name):
    folder=ROOT/name;bpy.ops.wm.open_mainfile(filepath=str(folder/'baseline/source.blend'));bpy.context.scene.frame_set(0)
    bounds=mesh_bounds([o for o in bpy.context.scene.objects if o.type=='MESH' and not o.get('mu_helper')]);setup(bounds);views(bounds,folder/'review','baseline')
    (folder/'baseline/bounds.json').write_text(json.dumps(bounds,indent=2)+'\n')

def context():
    evidence=json.loads((ROOT/'baseline-evidence.json').read_text());anchor=Vector((6300,12100,169.5));objects=[];records=[]
    for name,indices in [('Object01',[958,959,960]),('Object03',[964])]:
        for index in indices:
            record=next(p for p in evidence[name]['placements'] if p['index']==index)
            with bpy.data.libraries.load(str(ROOT/name/'baseline/source.blend')) as (source,loaded):loaded.objects=source.objects
            for obj in loaded.objects:
                if obj.type not in ('MESH','ARMATURE') or obj.get('mu_helper') or obj.get('mu_reference'):continue
                bpy.context.scene.collection.objects.link(obj)
                if obj.type=='ARMATURE':obj.matrix_world=Matrix.Translation(Vector(record['position'])-anchor)@Euler([math.radians(a) for a in record['rotation']]).to_matrix().to_4x4()@Matrix.Scale(record['scale'],4)
                else:objects.append(obj)
            records.append(dict(name=name,**record))
    bpy.context.scene.frame_set(0);bpy.context.view_layer.update();graph=bpy.context.evaluated_depsgraph_get();bounds=mesh_bounds([o.evaluated_get(graph) for o in objects]);setup(bounds)
    output=ROOT/'review';output.mkdir(exist_ok=True);views(bounds,output,'wall960-pillar964-baseline')
    (output/'baseline-context.json').write_text(json.dumps(dict(kind='OFFLINE_ACTUAL_PLACEMENTS',records=records,bounds=bounds,anchor=list(anchor)),indent=2)+'\n')

if __name__=='__main__':
    if os.environ.get('WALL_INSPECT')=='context':context()
    else:
        for name in ('Object01','Object03'):individual(name)
