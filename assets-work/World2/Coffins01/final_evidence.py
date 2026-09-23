"""Matched untouched original and actual exported topology; offline evidence only."""
from pathlib import Path
import json,os,sys
sys.dont_write_bytecode=True
import bpy
from mathutils import Vector
ROOT=Path(__file__).resolve().parent;REPO=ROOT.parents[2]
sys.path.insert(0,str(REPO/'assets-work/World1/StaticBatch01'))
from review_scene import set_camera,set_lighting,render

def wire_material():
    material=bpy.data.materials.new('REVIEW_EXPORTED_TOPOLOGY');material.use_nodes=True
    nodes,links=material.node_tree.nodes,material.node_tree.links
    wire=nodes.new('ShaderNodeWireframe');wire.inputs['Size'].default_value=.10
    mix=nodes.new('ShaderNodeMixRGB');mix.inputs[1].default_value=(.3,.36,.34,1);mix.inputs[2].default_value=(.005,.009,.01,1)
    emission=nodes.new('ShaderNodeEmission')
    links.new(wire.outputs[0],mix.inputs[0]);links.new(mix.outputs[0],emission.inputs[0]);links.new(emission.outputs[0],nodes.get('Material Output').inputs['Surface'])
    return material

for name in os.environ.get('COFFIN_NAMES','Object21,Object22').split(','):
    folder=ROOT/name;bounds=json.loads((folder/'validation/authored.json').read_text())['bounds_before']
    for stage,path in [('original','original/source.blend'),('wireframe','validation/reimported.blend')]:
        bpy.ops.wm.open_mainfile(filepath=str(folder/path));bpy.context.scene.frame_set(0)
        set_camera(bounds,1.35);set_lighting();scene=bpy.context.scene
        scene.render.film_transparent=False;scene.render.resolution_x=1100;scene.render.resolution_y=700;scene.cycles.samples=16
        if stage=='wireframe':
            material=wire_material()
            for obj in scene.objects:
                if obj.type=='MESH' and not obj.get('mu_helper'):
                    obj.data.materials.clear();obj.data.materials.append(material)
        render(folder/'review'/f'{stage}.png')
        center=(Vector(bounds[0])+Vector(bounds[1]))/2;camera=scene.camera
        camera.location=center+Vector((-1.8,1.8,1.4))*max(Vector(bounds[1])-Vector(bounds[0]));camera.rotation_euler=(center-camera.location).to_track_quat('-Z','Y').to_euler()
        render(folder/'review'/f'{stage}-reverse.png')
