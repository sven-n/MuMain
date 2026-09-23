"""Untouched wall/pillar identity and actual neighborhood before remodeling."""
from pathlib import Path
import json,sys,math,os
sys.dont_write_bytecode=True
import bpy
from mathutils import Vector,Euler,Matrix
ROOT=Path(__file__).resolve().parent;REPO=ROOT.parents[2]
sys.path.insert(0,str(REPO/'assets-work/World1/StaticBatch01'))
from review_scene import mesh_bounds,set_camera,set_lighting,render

sys.path.insert(0,str(ROOT))
from inspect_baseline import setup,views

def context(stage):
    evidence=json.loads((ROOT/'baseline-evidence.json').read_text());anchor=Vector((6300,12100,169.5));objects=[];records=[]
    for name,indices in [('Object01',[958,959,960]),('Object03',[964])]:
        for index in indices:
            record=next(p for p in evidence[name]['placements'] if p['index']==index)
            path=ROOT/name/'validation/reimported.blend' if stage=='candidate' and (ROOT/name/'validation/reimported.blend').exists() else ROOT/name/'baseline/source.blend'
            with bpy.data.libraries.load(str(path)) as (source,loaded):loaded.objects=source.objects
            for obj in loaded.objects:
                if obj.type not in ('MESH','ARMATURE') or obj.get('mu_helper') or obj.get('mu_reference'):continue
                bpy.context.scene.collection.objects.link(obj)
                if obj.type=='ARMATURE':obj.matrix_world=Matrix.Translation(Vector(record['position'])-anchor)@Euler([math.radians(a) for a in record['rotation']]).to_matrix().to_4x4()@Matrix.Scale(record['scale'],4)
                else:objects.append(obj)
            records.append(dict(name=name,source=str(path.relative_to(ROOT)),**record))
    bpy.context.scene.frame_set(0);bpy.context.view_layer.update();graph=bpy.context.evaluated_depsgraph_get();bounds=mesh_bounds([o.evaluated_get(graph) for o in objects]);setup(bounds)
    output=ROOT/'review';output.mkdir(exist_ok=True);views(bounds,output,'wall960-pillar964-'+stage)
    (output/(stage+'-context.json')).write_text(json.dumps(dict(kind='OFFLINE_ACTUAL_PLACEMENTS',records=records,bounds=bounds,anchor=list(anchor)),indent=2)+'\n')

context(os.environ.get('WALL_STAGE','candidate'))
