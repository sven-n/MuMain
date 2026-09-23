"""Actual Dungeon pottery placement transforms; no terrain or runtime mutation."""
from pathlib import Path
import json
import math
import sys
sys.dont_write_bytecode=True
import bpy
from mathutils import Vector,Matrix,Euler
ROOT=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT.parents[1]/'World1/StaticBatch01'))
from review_scene import mesh_bounds,set_camera,set_lighting,render

def placement(name,index):
    records=json.loads((ROOT/name/'placements.json').read_text())
    return dict(name=name,**next(record for record in records if record['index']==index))

def group(label,records,scale_reference=False):
    anchor=Vector(records[0]['position']);out=ROOT/'review-assemblies'/label;out.mkdir(parents=True,exist_ok=True)
    (out/'placements.json').write_text(json.dumps(dict(records=records,scale_reference=190 if scale_reference else None),indent=2))
    for stage in ('current','candidate'):
        bpy.ops.wm.read_factory_settings(use_empty=True);bpy.context.scene.world=bpy.data.worlds.new('ReviewWorld');meshes=[]
        for record in records:
            folder=ROOT/record['name'];candidate=folder/'validation/reimported.blend'
            path=candidate if stage=='candidate' and candidate.exists() else folder/'baseline/source.blend'
            with bpy.data.libraries.load(str(path)) as (source,loaded):loaded.objects=source.objects
            for obj in loaded.objects:
                if obj.type not in ('ARMATURE','MESH') or obj.get('mu_helper'):continue
                bpy.context.scene.collection.objects.link(obj)
                if obj.type=='MESH':meshes.append(obj)
                else:
                    rot=Euler([math.radians(v) for v in record['rotation']],'XYZ').to_matrix().to_4x4()
                    obj.matrix_world=Matrix.Translation(Vector(record['position'])-anchor)@rot@Matrix.Scale(record['scale'],4)
        bpy.context.scene.frame_set(0);bpy.context.view_layer.update()
        if stage=='current':
            bounds=mesh_bounds(meshes)
            if scale_reference:bounds[1][0]+=25;bounds[1][2]=max(bounds[1][2],bounds[0][2]+190)
        if scale_reference:
            bpy.ops.mesh.primitive_cube_add(size=1,location=(bounds[1][0],bounds[0][1],bounds[0][2]+95))
            bar=bpy.context.object;bar.name='REVIEW_ONLY_190_UNIT_REFERENCE';bar.scale=(3,3,190)
            mat=bpy.data.materials.new('ReviewScale');mat.diffuse_color=(.45,.49,.5,1);bar.data.materials.append(mat)
        set_camera(bounds);set_lighting();bpy.context.scene.cycles.samples=16
        render(out/(stage+'.png'))
        bpy.context.scene.render.resolution_percentage=40;render(out/(stage+'-small.png'))
        camera=bpy.context.scene.camera;center=(Vector(bounds[0])+Vector(bounds[1]))/2
        camera.location=center+Vector((-1.35,2,1.3))*max(Vector(bounds[1])-Vector(bounds[0]));camera.rotation_euler=(center-camera.location).to_track_quat('-Z','Y').to_euler()
        bpy.context.scene.render.resolution_percentage=100;render(out/(stage+'-reverse.png'))

if __name__=='__main__':
    group('tilted-entry',[placement('Object29',75),placement('Object30',76)])
    group('northern-pot-group',[placement('Object29',522),placement('Object28',520),placement('Object28',521),placement('Object29',518)])
    group('sloped-pair',[placement('Object29',3362),placement('Object29',3363)])

    group('steep-composite-pair',[placement('Object28',265),placement('Object28',266)])
    group('broken-family-context',[placement('Object28',3556),placement('Object28',3557),placement('Object30',3560),placement('Object29',3558)])
