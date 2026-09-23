"""Read-only all-material and targeted shared-strip consumer previews."""
from pathlib import Path
import sys
sys.dont_write_bytecode=True
import bpy
from mathutils import Vector
ROOT=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT.parents[1]/'World1/StaticBatch01'))
from review_scene import mesh_bounds,set_camera,set_lighting,render


def show(name):
    folder=ROOT/'context'/name
    bpy.ops.wm.open_mainfile(filepath=str(folder/'baseline/source.blend'))
    meshes=[o for o in bpy.context.scene.objects if o.type=='MESH' and not o.get('mu_helper')]
    bounds=mesh_bounds(meshes);set_camera(bounds);set_lighting();bpy.context.scene.cycles.samples=16
    for label,direction in [('front',(-1,-2,1.25)),('reverse',(1,2,1.25))]:
        camera=bpy.context.scene.camera;center=(Vector(bounds[0])+Vector(bounds[1]))/2
        camera.location=center+Vector(direction)*max(Vector(bounds[1])-Vector(bounds[0]))
        camera.rotation_euler=(center-camera.location).to_track_quat('-Z','Y').to_euler()
        render(folder/(label+'.png'))
    for obj in meshes:
        for material in obj.data.materials:
            if material.get('mu_texture')=='deep_wall03.jpg':
                continue
            material.use_nodes=True
            node=material.node_tree.nodes.get('Principled BSDF')
            if node:
                for link in list(material.node_tree.links):
                    if link.to_node==node and link.to_socket.name=='Base Color':material.node_tree.links.remove(link)
                node.inputs['Base Color'].default_value=(.08,.08,.08,1)
    camera=bpy.context.scene.camera;camera.location=center+Vector((-.5,-2,1))*max(Vector(bounds[1])-Vector(bounds[0]))
    camera.rotation_euler=(center-camera.location).to_track_quat('-Z','Y').to_euler()
    render(folder/'shared-strip-location.png')


if __name__=='__main__':
    for name in ('Object16','Object17'):show(name)
