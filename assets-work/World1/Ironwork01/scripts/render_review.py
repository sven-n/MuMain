"""Matching-camera original/export views with diffuse color and preserved alpha."""
import json
from itertools import product
from pathlib import Path
import sys
import bpy
from mathutils import Vector
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))
from config import CONVERTER, PROPS, REPOSITORY, ROOT
sys.path.insert(0, str(REPOSITORY/'tools/blender'))
sys.path.insert(0, str(ROOT.parent/'StaticBatch01'))
import mu_bmd_import
from review_scene import mesh_bounds, set_camera, set_lighting
RENDER_SIZE = (840, 700)


def objects():
    return [obj for obj in bpy.context.scene.objects if obj.type=='MESH' and not obj.get('mu_helper')]


def fit_camera(bounds):
    camera=set_camera(bounds)
    center=(Vector(bounds[0])+Vector(bounds[1]))/2
    view=camera.rotation_euler.to_matrix().transposed()
    corners=[view@(Vector(values)-center) for values in product(*zip(*bounds))]
    width=max(point.x for point in corners)-min(point.x for point in corners)
    height=max(point.y for point in corners)-min(point.y for point in corners)
    camera.data.ortho_scale=max(width,height*RENDER_SIZE[0]/RENDER_SIZE[1])*1.1
    return camera


def setup(bounds):
    fit_camera(bounds)
    set_lighting()
    scene=bpy.context.scene
    scene.render.resolution_x,scene.render.resolution_y=RENDER_SIZE
    scene.cycles.samples=16
    scene.render.threads_mode='FIXED'
    scene.render.threads=2


def render(path):
    bpy.context.scene.render.filepath=str(path)
    bpy.ops.render.render(write_still=True)


def wire_material():
    material=bpy.data.materials.new('OFFLINE_WIREFRAME')
    material.use_nodes=True
    nodes,links=material.node_tree.nodes,material.node_tree.links
    nodes.clear()
    output=nodes.new('ShaderNodeOutputMaterial')
    diffuse=nodes.new('ShaderNodeBsdfDiffuse')
    wire=nodes.new('ShaderNodeWireframe')
    wire.inputs['Size'].default_value=.22
    mix=nodes.new('ShaderNodeMixRGB')
    mix.inputs[1].default_value=(.32,.4,.35,1)
    mix.inputs[2].default_value=(.015,.025,.02,1)
    links.new(wire.outputs[0],mix.inputs[0])
    links.new(mix.outputs[0],diffuse.inputs['Color'])
    links.new(diffuse.outputs[0],output.inputs['Surface'])
    return material


def review(name):
    root=ROOT/name
    folder=root/'review'
    folder.mkdir(exist_ok=True)
    bpy.ops.wm.open_mainfile(filepath=str(root/'original/source.blend'))
    bpy.context.scene.frame_set(0)
    bounds=mesh_bounds(objects())
    setup(bounds)
    render(folder/'before-offline.png')
    bpy.context.preferences.filepaths.save_version=0
    sys.argv=['blender','--','--bmd',str(root/'exports'/f'{name}.bmd'),
              '--bmdconv',str(CONVERTER),'--out',str(root/'validation/reimported.blend')]
    mu_bmd_import.main()
    bpy.context.scene.frame_set(0)
    setup(bounds)
    render(folder/'after-offline.png')
    camera=bpy.context.scene.camera
    # Rotate the entire viewing direction around the same bounds center for a reverse check.
    target=(Vector(bounds[0])+Vector(bounds[1]))/2
    camera.location=target+Vector((-1.35,2,1.3))*max(Vector(bounds[1])-Vector(bounds[0]))
    camera.rotation_euler=(target-camera.location).to_track_quat('-Z','Y').to_euler()
    render(folder/'reverse-offline.png')
    fit_camera(bounds)
    wire=wire_material()
    for obj in objects():
        obj.data.materials.clear()
        obj.data.materials.append(wire)
    render(folder/'wireframe-offline.png')
    (folder/'render-context.json').write_text(json.dumps({'status':'OFFLINE BLENDER ONLY',
        'after_source':'officially reimported exported BMD/OZT','camera_bounds':bounds,
        'camera_direction':[1.35,-2,1.3],'orthographic_scale':bpy.context.scene.camera.data.ortho_scale,
        'resolution':list(RENDER_SIZE),'lighting':'diffuse-only Cycles, Standard, 16 samples',
        'matched_camera_and_lighting':True,'client_verified':False},indent=2)+'\n')


for prop in PROPS:
    review(prop)
