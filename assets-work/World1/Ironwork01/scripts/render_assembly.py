"""Offline modular assemblies from actual BMD SMDs and unchanged World1 records."""
import json
import math
from pathlib import Path
import sys
import bpy
from mathutils import Euler, Matrix, Vector
sys.dont_write_bytecode=True
sys.path.insert(0,str(Path(__file__).resolve().parent))
from config import PROPS,ROOT
sys.path.insert(0,str(ROOT.parent/'StaticBatch01'))
from review_scene import mesh_bounds,set_camera,set_lighting

ASSEMBLIES={
    'north-gate':{'SteelWall01':[0,1],'SteelWall02':[3,12,13,16,18],'SteelDoor01':[2,3]},
    'prison-corner':{'SteelWall02':[0,1,2],'SteelWall03':[0,1,2,3],
                     'SteelDoor01':[0,1],'HouseEtc03':[4,5]},
}


def material(texture,state):
    key=state+'_'+texture
    found=bpy.data.materials.get(key)
    if found:return found
    material=bpy.data.materials.new(key)
    material.use_nodes=True
    nodes,links=material.node_tree.nodes,material.node_tree.links
    nodes.clear()
    output=nodes.new('ShaderNodeOutputMaterial')
    diffuse=nodes.new('ShaderNodeBsdfDiffuse')
    transparent=nodes.new('ShaderNodeBsdfTransparent')
    mix=nodes.new('ShaderNodeMixShader')
    image=nodes.new('ShaderNodeTexImage')
    image.image=bpy.data.images.load(str(ROOT/('original' if state=='original' else 'textures')/texture))
    links.new(image.outputs['Color'],diffuse.inputs['Color'])
    links.new(image.outputs['Alpha'],mix.inputs[0])
    links.new(transparent.outputs[0],mix.inputs[1])
    links.new(diffuse.outputs[0],mix.inputs[2])
    links.new(mix.outputs[0],output.inputs['Surface'])
    return material


def parsed_mesh(name,state):
    path=ROOT/name/'validation'/state/f'{name}.smd'
    lines=path.read_text().split('triangles\n')[1].splitlines()[:-1]
    textures=[]
    vertices=[]
    uvs=[]
    slots=[]
    for start in range(0,len(lines),4):
        texture=lines[start]
        if texture not in textures:textures.append(texture)
        slots.append(textures.index(texture))
        for line in lines[start+1:start+4]:
            row=list(map(float,line.split()))
            vertices.append(row[1:4]);uvs.append(row[7:9])
    mesh=bpy.data.meshes.new(state+'_'+name)
    mesh.from_pydata(vertices,[],[list(range(i,i+3)) for i in range(0,len(vertices),3)])
    layer=mesh.uv_layers.new(name='UVMap')
    for face,slot in zip(mesh.polygons,slots):
        face.material_index=slot
        for index in face.loop_indices:layer.data[index].uv=uvs[index]
    for texture in textures:mesh.materials.append(material(texture,state))
    return mesh


def assemble(state,selected):
    meshes={name:parsed_mesh(name,state) for name in selected}
    objects=[]
    for name,indices in selected.items():
        placements=json.loads((ROOT/name/'original/placements.json').read_text())
        for index in indices:
            placement=placements[index]
            obj=bpy.data.objects.new(f'{name}_World1_{index}',meshes[name])
            bpy.context.scene.collection.objects.link(obj)
            rotation=Euler([math.radians(value) for value in placement['rotation']],'XYZ').to_matrix().to_4x4()
            obj.matrix_world=Matrix.Translation(Vector(placement['position']))@rotation@Matrix.Scale(placement['scale'],4)
            objects.append(obj)
    return objects


def review(label,selected):
    camera_bounds=None
    framing=None
    for state in ('original','replacement'):
        for obj in list(bpy.data.objects):bpy.data.objects.remove(obj,do_unlink=True)
        if bpy.context.scene.world is None:bpy.context.scene.world=bpy.data.worlds.new('OfflineWorld')
        objects=assemble(state,selected)
        if camera_bounds is None:camera_bounds=mesh_bounds(objects)
        camera=set_camera(camera_bounds,scale_factor=.72)
        center=(Vector(camera_bounds[0])+Vector(camera_bounds[1]))/2
        span=max(Vector(camera_bounds[1])-Vector(camera_bounds[0]))
        camera.location=center+Vector((.35,-2,1))*span
        camera.rotation_euler=(center-camera.location).to_track_quat('-Z','Y').to_euler()
        view=camera.rotation_euler.to_matrix().transposed()
        projected=[view@(obj.matrix_world@vertex.co-center) for obj in objects for vertex in obj.data.vertices]
        projected_width=max(p.x for p in projected)-min(p.x for p in projected)
        projected_height=max(p.y for p in projected)-min(p.y for p in projected)
        if framing is None:
            offset=Vector(((max(p.x for p in projected)+min(p.x for p in projected))/2,
                           (max(p.y for p in projected)+min(p.y for p in projected))/2,0))
            location=camera.location+camera.rotation_euler.to_matrix()@offset
            framing={'location':list(location),'rotation':list(camera.rotation_euler),
                     'ortho_scale':max(projected_width,projected_height*1600/700)*1.15}
        camera.location=framing['location']
        camera.rotation_euler=framing['rotation']
        camera.data.ortho_scale=framing['ortho_scale']
        set_lighting()
        scene=bpy.context.scene
        scene.render.resolution_x=1600
        scene.render.resolution_y=700
        scene.cycles.samples=24
        scene.render.threads_mode='FIXED';scene.render.threads=2
        scene.render.filepath=str(ROOT/'review'/f'{label}-{state}.png')
        bpy.ops.render.render(write_still=True)
    return {'actual_World1_record_indices':selected,'camera_bounds':camera_bounds,'exact_shared_camera':framing,
            'placement_changes':False,'status':'OFFLINE XYZ transform reconstruction; no terrain/client simulation'}


(ROOT/'review').mkdir(exist_ok=True)
report={label:review(label,selected) for label,selected in ASSEMBLIES.items()}
(ROOT/'review/assembly-context.json').write_text(json.dumps(report,indent=2)+'\n')
