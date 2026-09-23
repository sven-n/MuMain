"""Matched actual exports under neutral diffuse lighting, including reversed tilted lid."""
from pathlib import Path
import json,os,sys
sys.dont_write_bytecode=True
import bpy
from mathutils import Vector
ROOT=Path(__file__).resolve().parent;REPO=ROOT.parents[2]
sys.path.insert(0,str(REPO/'assets-work/World1/StaticBatch01'))
from review_scene import set_camera,set_lighting,render
name=os.environ.get('COFFIN_NAME','Object22');folder=ROOT/name
bounds=json.loads((folder/'validation/authored.json').read_text())['bounds_before']
for stage,path in [('baseline','baseline/source.blend'),('candidate','validation/reimported.blend')]:
    bpy.ops.wm.open_mainfile(filepath=str(folder/path));bpy.context.scene.frame_set(0)
    set_camera(bounds,1.35);set_lighting();scene=bpy.context.scene
    scene.render.film_transparent=False;scene.render.resolution_x=1100;scene.render.resolution_y=700;scene.cycles.samples=16
    render(folder/'review'/f'{stage}.png')
    scene.render.resolution_percentage=30;render(folder/'review'/f'{stage}-small.png');scene.render.resolution_percentage=100
    center=(Vector(bounds[0])+Vector(bounds[1]))/2;camera=scene.camera
    camera.location=center+Vector((-1.8,1.8,1.4))*max(Vector(bounds[1])-Vector(bounds[0]));camera.rotation_euler=(center-camera.location).to_track_quat('-Z','Y').to_euler()
    render(folder/'review'/f'{stage}-reverse.png')
    camera.location=center+Vector((1.8,-1.8,-.85))*max(Vector(bounds[1])-Vector(bounds[0]));camera.rotation_euler=(center-camera.location).to_track_quat('-Z','Y').to_euler()
    render(folder/'review'/f'{stage}-underside.png')
(folder/'review/context.json').write_text(json.dumps(dict(kind='OFFLINE_DIFFUSE_NOT_CLIENT',baseline='Exact original BMD and frozen wood01',candidate='Official exported/reimported BMD',lighting='Identical standard neutral ambient and diffuse sun',views=['main','330px reduced','reverse','underside']),indent=2)+'\n')
