"""Round only the middle of solid rectangular bars, preserving every endpoint vertex."""
import math
import bmesh
import bpy
from mathutils import Vector
from mathutils.geometry import barycentric_transform

OPAQUE_MATERIAL='steel_barred_b.jpg'
END_TRANSITION=2.0
CHAMFER_WIDTH=.35
AXIS_TOLERANCE=.002


def connected_groups(mesh):
    remaining=set(range(len(mesh.vertices)))
    neighbors={index:set() for index in remaining}
    for edge in mesh.edges:
        a,b=edge.vertices
        neighbors[a].add(b)
        neighbors[b].add(a)
    while remaining:
        visited=set()
        pending=[next(iter(remaining))]
        while pending:
            index=pending.pop()
            if index in visited:continue
            visited.add(index)
            pending.extend(neighbors[index]-visited)
        remaining-=visited
        yield visited


def rectangular_bars(obj):
    for indices in connected_groups(obj.data):
        faces=[face for face in obj.data.polygons if face.vertices[0] in indices]
        if len(indices)!=8 or len(faces)!=8:
            continue
        if any(obj.data.materials[face.material_index].name!=OPAQUE_MATERIAL for face in faces):
            continue
        points=[obj.data.vertices[index].co for index in indices]
        low=[min(p[a] for p in points) for a in range(3)]
        high=[max(p[a] for p in points) for a in range(3)]
        axis=max(range(3),key=lambda a:high[a]-low[a])
        ends=[[index for index in indices if abs(obj.data.vertices[index].co[axis]-limit[axis])<AXIS_TOLERANCE]
              for limit in (low,high)]
        if not all(len(end)==4 for end in ends):
            continue
        yield faces,ends,axis


def face_uvs(obj,face,points):
    vertices=[obj.data.vertices[index].co for index in face.vertices]
    uv=[Vector((*obj.data.uv_layers.active.data[index].uv,0)) for index in face.loop_indices]
    return [barycentric_transform(point,*vertices,*uv)[:2] for point in points]


def append_surface(obj,vertices,faces,uvs,materials,original_faces,indices):
    points=[Vector(vertices[index]) for index in indices]
    center=sum(points,Vector())/len(points)
    normal=(points[1]-points[0]).cross(points[2]-points[0]).normalized()
    candidates=sorted(original_faces,key=lambda face:normal.dot(face.normal),reverse=True)
    reference=candidates[0]
    faces.append(indices)
    uvs.append(face_uvs(obj,reference,points))
    materials.append(reference.material_index)


def profile_bar(obj,record,vertices,faces,uvs,materials,weights):
    original_faces,ends,axis=record
    axes=[a for a in range(3) if a!=axis]
    center=sum((obj.data.vertices[index].co for index in ends[0]),Vector())/4
    order=lambda index:math.atan2(obj.data.vertices[index].co[axes[1]]-center[axes[1]],
                                 obj.data.vertices[index].co[axes[0]]-center[axes[0]])
    ends=[sorted(end,key=order) for end in ends]
    rings=[]
    for side,end in enumerate(ends):
        ring=[]
        for offset,index in enumerate(end):
            point=obj.data.vertices[index].co.copy()
            for neighbor in (end[(offset-1)%4],end[(offset+1)%4]):
                toward=obj.data.vertices[neighbor].co-point
                corner=point+toward.normalized()*min(CHAMFER_WIDTH,toward.length*.15)
                corner[axis]+=END_TRANSITION if side==0 else -END_TRANSITION
                ring.append(len(vertices))
                vertices.append(list(corner))
                weights.append(weights[index].copy())
        rings.append(ring)
    new_faces=[]
    for side,end in enumerate(ends):
        ring=rings[side]
        for index in range(4):
            next_index=(index+1)%4
            new_faces.append([end[index],ring[index*2],ring[index*2+1]])
            new_faces.append([end[index],end[next_index],ring[next_index*2],ring[index*2+1]])
    for index in range(8):
        next_index=(index+1)%8
        new_faces.append([rings[0][index],rings[0][next_index],rings[1][next_index],rings[1][index]])
    midpoint=sum((obj.data.vertices[index].co for end in ends for index in end),Vector())/8
    for indices in new_faces:
        points=[Vector(vertices[index]) for index in indices]
        normal=(points[1]-points[0]).cross(points[2]-points[0])
        if normal.dot(sum(points,Vector())/len(points)-midpoint)<0:
            indices.reverse()
        append_surface(obj,vertices,faces,uvs,materials,original_faces,indices)


def keep_high_poly(obj):
    collection=bpy.data.collections.new('REF_HIGH_POLY')
    bpy.context.scene.collection.children.link(collection)
    collection.hide_viewport=collection.hide_render=True
    collection.vs.export=False
    high=obj.copy()
    high.data=obj.data.copy()
    high.name='Editable_Forged_Edge_Profile'
    high['mu_reference']=True
    collection.objects.link(high)
    modifier=high.modifiers.new('Fine_Edge_Rounding','BEVEL')
    modifier.width=.05
    modifier.segments=3
    modifier.limit_method='ANGLE'
    modifier.angle_limit=math.radians(20)


def refine(obj):
    bars=list(rectangular_bars(obj))
    if not bars:return {'bars_refined':0,'geometry':'Original alpha planes retained exactly'}
    removed={face.index for faces,_,_ in bars for face in faces}
    vertices=[list(vertex.co) for vertex in obj.data.vertices]
    weights=[[(group.group,group.weight) for group in vertex.groups] for vertex in obj.data.vertices]
    faces=[]
    uvs=[]
    materials=[]
    for face in obj.data.polygons:
        if face.index in removed:continue
        faces.append(list(face.vertices))
        uvs.append([list(obj.data.uv_layers.active.data[index].uv) for index in face.loop_indices])
        materials.append(face.material_index)
    for record in bars:
        profile_bar(obj,record,vertices,faces,uvs,materials,weights)
    ordered=sorted(zip(materials,faces,uvs),key=lambda record:record[0])
    materials,faces,uvs=map(list,zip(*ordered))
    mesh=bpy.data.meshes.new(obj.data.name+'_RoundedProfiles')
    mesh.from_pydata(vertices,[],faces)
    for material in obj.data.materials:mesh.materials.append(material)
    layer=mesh.uv_layers.new(name='UVMap')
    for face,coordinates,material in zip(mesh.polygons,uvs,materials):
        face.material_index=material
        for index,uv in zip(face.loop_indices,coordinates):layer.data[index].uv=uv
    group_names=[group.name for group in obj.vertex_groups]
    obj.data=mesh
    for group_name in group_names:
        if group_name not in obj.vertex_groups:obj.vertex_groups.new(name=group_name)
    for index,groups in enumerate(weights):
        for group,weight in groups:obj.vertex_groups[group].add([index],weight,'REPLACE')
    mesh.update()
    repaired=repair_uv_triangles(obj)
    keep_high_poly(obj)
    return {'collapsed_chamfer_uvs_repaired':repaired,'bars_refined':len(bars),'chamfer_width':CHAMFER_WIDTH,'original_end_vertices_exact':True,
            'transition_length':END_TRANSITION,'alpha_planes':'geometry UVs and bones unchanged'}


def repair_uv_triangles(obj):
    """Give narrow chamfers a stable texel footprint after deterministic triangulation."""
    mesh=bmesh.new()
    mesh.from_mesh(obj.data)
    bmesh.ops.triangulate(mesh,faces=list(mesh.faces),quad_method='FIXED',ngon_method='EAR_CLIP')
    mesh.to_mesh(obj.data)
    mesh.free()
    uv=obj.data.uv_layers.active.data
    repaired=0
    for face in obj.data.polygons:
        if obj.data.materials[face.material_index].name!=OPAQUE_MATERIAL:continue
        a,b,c=[uv[index].uv.copy() for index in face.loop_indices]
        if abs((b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x))>1e-6 and min((a-b).length,(a-c).length,(b-c).length)>1e-4:continue
        center=(a+b+c)/3
        for index,offset in zip(face.loop_indices,((-1,-1),(1,-1),(0,1))):
            uv[index].uv=(center.x+offset[0]/512,center.y+offset[1]/512)
        repaired+=1
    return repaired
