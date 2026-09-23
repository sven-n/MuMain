"""Rebuild only exposed creature relief, preserving the structural pier surfaces."""
from pathlib import Path
import json
import math
import sys
sys.dont_write_bytecode = True
import bpy
from mathutils import Vector, Euler
ROOT = Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT))
from audit_exports import triangles
from relief_surface import sculpt, RELIEF_RANGE
PROTECTED = list(range(10))+list(range(82,90))


def support_contract(points, placements):
    planes,anchors,seen = [],[],set()
    for record in placements:
        rotation=Euler([math.radians(v) for v in record['rotation']],'XYZ').to_matrix()
        for axis in range(3):
            for sign in (-1,1):
                normal=Vector(rotation[axis])*sign
                key=tuple(round(v,7) for v in normal)
                if key in seen:
                    continue
                seen.add(key)
                limit=max(normal.dot(p) for p in points)
                planes.append((normal,limit))
                anchors.extend(p.copy() for p in points if abs(normal.dot(p)-limit)<.00001)
    return planes,anchors


def reference(obj):
    collection=bpy.data.collections.new('REF_ORIGINAL')
    bpy.context.scene.collection.children.link(collection)
    copy=obj.copy();copy.data=obj.data.copy();copy['mu_reference']=True
    collection.objects.link(copy)
    collection.hide_render=collection.hide_viewport=True


def make_faces(rows,planes,anchors):
    faces=sculpt(rows,planes,anchors)
    for index in PROTECTED:
        material,values=rows[index]
        corners=[(Vector(row[1:4]),Vector(row[7:9])) for row in values]
        faces.append((0 if material=='deep_wall01.jpg' else 1,corners,index))
    faces.sort(key=lambda f:f[0])
    return faces


def mesh_geometry(obj,faces):
    original=obj.data
    vertices,polygons,lookup=[],[],{}
    inverse=obj.matrix_world.inverted()
    for material,corners,index in faces:
        polygon=[]
        for point,uv in corners:
            key=tuple(round(v,6) for v in point)
            if key not in lookup:
                lookup[key]=len(vertices);vertices.append(inverse@point)
            polygon.append(lookup[key])
        polygons.append(polygon)
    mesh=bpy.data.meshes.new('CarvedCreatureContinuousBrowCheekMuzzle')
    mesh.from_pydata(vertices,[],polygons)
    for material in original.materials:
        mesh.materials.append(material)
    assert [group.name for group in obj.vertex_groups]==['Box01']
    obj.data=mesh
    obj.vertex_groups.clear()
    obj.vertex_groups.new(name='Box01').add(list(range(len(vertices))),1,'REPLACE')
    mesh.update()
    assert all(face.area>1e-8 for face in mesh.polygons)
    assert len(mesh.polygons)<=1500
    return mesh


def material_normals(mesh,faces,rows):
    uv_layer=mesh.uv_layers.new(name='UVMap')
    normals=[Vector() for _ in mesh.loops]
    neighbors={}
    for face,(material,corners,old_index) in zip(mesh.polygons,faces):
        face.material_index=material;face.use_smooth=True
        for loop,(_,uv) in zip(face.loop_indices,corners):
            uv_layer.data[loop].uv=uv
            if old_index is None:
                neighbors.setdefault(mesh.loops[loop].vertex_index,[]).append(face.normal.copy())
    for face,(_,corners,old_index) in zip(mesh.polygons,faces):
        for corner,loop in enumerate(face.loop_indices):
            if old_index is not None:
                normals[loop]=Vector(rows[old_index][1][corner][4:7])
            else:
                nearby=neighbors[mesh.loops[loop].vertex_index]
                normals[loop]=sum((n for n in nearby if n.dot(face.normal)>.65),Vector()).normalized()
    mesh.normals_split_custom_set(normals)


def main():
    folder=ROOT/'Object04'
    rows=triangles(folder/'validation/baseline/Object04.smd')
    bpy.ops.wm.open_mainfile(filepath=str(folder/'baseline/source.blend'))
    obj=next(o for o in bpy.context.scene.objects if o.type=='MESH' and not o.get('mu_helper'))
    bpy.context.scene.frame_set(0)
    points=[Vector(row[1:4]) for _,values in rows for row in values]
    placements=json.loads((folder/'placements.json').read_text())
    planes,anchors=support_contract(points,placements)
    reference(obj)
    faces=make_faces(rows,planes,anchors)
    mesh=mesh_geometry(obj,faces)
    material_normals(mesh,faces,rows)
    bpy.ops.file.pack_all();bpy.context.preferences.filepaths.save_version=0
    bpy.ops.wm.save_as_mainfile(filepath=str(folder/'source.blend'))
    (folder/'validation/source.json').write_text(json.dumps(dict(triangles=len(mesh.polygons),
        protected_triangles=PROTECTED,original_relief_triangles=list(RELIEF_RANGE),
        support_planes=len(planes),actual_placements=len(placements),
        design='Adaptive cubic mask surfaces, broad brow and cheek curves, fixed structural block and relief seams'),indent=2))


if __name__=='__main__':
    main()
