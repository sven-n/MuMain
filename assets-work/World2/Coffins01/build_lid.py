"""Shape a thick six-sided timber lid with joints registered to painted plank seams."""
from pathlib import Path
import json,sys
sys.dont_write_bytecode=True
import bpy
from mathutils import Vector
ROOT=Path(__file__).resolve().parent
LID='Object22'
SEAMS=(-16.6,17.1)
JOINT_HALF_WIDTH=1.8
JOINT_FLOOR_HALF_WIDTH=.5
JOINT_DEPTH=2.2
LOWER_SHOULDER_Z=4.2
UPPER_SHOULDER_Z=7.9
STATIONS=sorted(y+offset for y in SEAMS for offset in (-JOINT_HALF_WIDTH,-JOINT_FLOOR_HALF_WIDTH,JOINT_FLOOR_HALF_WIDTH,JOINT_HALF_WIDTH))


def read_triangles(path):
    lines=path.read_text().split('triangles\n')[1].splitlines()
    return [[list(map(float,row.split())) for row in lines[i+1:i+4]] for i in range(0,len(lines)-1,4)]


def height(y):
    top=10.227196
    for center in SEAMS:
        distance=abs(y-center)
        if distance<JOINT_HALF_WIDTH:
            slope=max(0,(distance-JOINT_FLOOR_HALF_WIDTH)/(JOINT_HALF_WIDTH-JOINT_FLOOR_HALF_WIDTH))
            return top-JOINT_DEPTH*(1-slope)
    return top


def clip_y(polygon,level,keep_above):
    result=[]
    for p,q in zip(polygon,polygon[1:]+polygon[:1]):
        inside=(p.y>=level) if keep_above else (p.y<=level)
        next_inside=(q.y>=level) if keep_above else (q.y<=level)
        if inside:result.append(p.copy())
        if inside!=next_inside:result.append(p.lerp(q,(level-p.y)/(q.y-p.y)))
    return result


def top_uv(point,triangles):
    for rows in triangles[4:8]:
        a,b,c=[Vector(row[1:3]) for row in rows];p=Vector(point[:2])
        v=b-a;w=c-a;q=p-a;den=v.x*w.y-v.y*w.x
        s=(q.x*w.y-q.y*w.x)/den;t=(v.x*q.y-v.y*q.x)/den
        if min(s,t,1-s-t)>=-.00001:
            return Vector(rows[0][7:9])*(1-s-t)+Vector(rows[1][7:9])*s+Vector(rows[2][7:9])*t
    raise ValueError(('outside original top UV domain',list(point)))


def append_triangle(output,points,uvs):
    if (points[1]-points[0]).cross(points[2]-points[0]).length<.000001:return
    output.append((points,uvs))


def top_boards(output,top,triangles):
    levels=[min(p.y for p in top),*STATIONS,max(p.y for p in top)]
    for low,high in zip(levels,levels[1:]):
        polygon=clip_y(clip_y(top,low,True),high,False)
        if len(polygon)<3:continue
        for p in polygon:p.z=height(p.y)
        for i in range(1,len(polygon)-1):
            points=[polygon[0],polygon[i],polygon[i+1]]
            append_triangle(output,points,[top_uv(p,triangles) for p in points])


def side_uv(rows,point):
    row=min(rows,key=lambda row:(Vector(row[1:4])-point).length)
    assert (Vector(row[1:4])-point).length<.001
    return Vector(row[7:9])


def side_profile(output,bottom,top,triangles):
    side_faces=(18,12,8,10,14,16)
    for edge,index in enumerate(side_faces):
        a,b=bottom[edge],bottom[(edge+1)%6];c,d=top[edge],top[(edge+1)%6]
        rows=triangles[index]+triangles[index+1]
        uva,uvb,uvc,uvd=[side_uv(rows,p) for p in (a,b,c,d)]
        cuts=[0,1]+[(y-c.y)/(d.y-c.y) for y in STATIONS if abs(d.y-c.y)>1e-7 and 0<(y-c.y)/(d.y-c.y)<1]
        cuts=sorted(set(cuts))
        for lo,hi in zip(cuts,cuts[1:]):
            rings=[];uvrings=[]
            for z,blend in ((a.z,0),(LOWER_SHOULDER_Z,.5),(UPPER_SHOULDER_Z,.87),(None,1)):
                points=[];uvs=[]
                for t in (lo,hi):
                    base=a.lerp(b,t);cap=c.lerp(d,t);p=base.lerp(cap,blend);p.z=height(cap.y) if z is None else z
                    alpha=(p.z-a.z)/(c.z-a.z)
                    points.append(p);uvs.append(uva.lerp(uvb,t).lerp(uvc.lerp(uvd,t),alpha))
                rings.append(points);uvrings.append(uvs)
            for r in range(3):
                pts=[rings[r][0],rings[r][1],rings[r+1][1],rings[r+1][0]]
                uvs=[uvrings[r][0],uvrings[r][1],uvrings[r+1][1],uvrings[r+1][0]]
                for indices in ((0,1,2),(0,2,3)):append_triangle(output,[pts[i] for i in indices],[uvs[i] for i in indices])


def replace_mesh(obj,output):
    old=obj.data;groups=[g.name for g in obj.vertex_groups];mesh=bpy.data.meshes.new('PlankLid');inverse=obj.matrix_world.inverted()
    points=[inverse@p for xyz,uv in output for p in xyz]
    mesh.from_pydata(points,[],[(i,i+1,i+2) for i in range(0,len(points),3)])
    mesh.materials.append(old.materials[0]);layer=mesh.uv_layers.new(name=old.uv_layers[0].name)
    obj.data=mesh;obj.vertex_groups.clear()
    for name in groups:obj.vertex_groups.new(name=name)
    for face,(xyz,uvs) in zip(mesh.polygons,output):
        for loop,uv in zip(face.loop_indices,uvs):
            layer.data[loop].uv=uv;obj.vertex_groups[0].add([mesh.loops[loop].vertex_index],1,'REPLACE')
    mesh.update()


def build():
    folder=ROOT/LID;bpy.ops.wm.open_mainfile(filepath=str(folder/'baseline/source.blend'))
    obj=next(o for o in bpy.context.scene.objects if o.type=='MESH' and not o.get('mu_helper'))
    collection=bpy.data.collections.new('REF_ORIGINAL');bpy.context.scene.collection.children.link(collection)
    collection.hide_render=collection.hide_viewport=True;clone=obj.copy();clone.data=obj.data.copy();clone['mu_reference']=True;collection.objects.link(clone)
    triangles=read_triangles(folder/'baseline/smd'/f'{LID}.smd')
    bottom=[Vector(triangles[t][r][1:4]) for t,r in ((2,1),(0,1),(0,0),(1,1),(0,2),(2,2))]
    top=[Vector(triangles[t][r][1:4]) for t,r in ((7,1),(5,1),(4,0),(4,1),(4,2),(6,2))]
    output=[([Vector(r[1:4]) for r in rows],[Vector(r[7:9]) for r in rows]) for rows in triangles[:4]]
    top_boards(output,top,triangles);side_profile(output,bottom,top,triangles)
    replace_mesh(obj,output)
    actual=[obj.matrix_world@v.co for v in obj.data.vertices]
    anchors=[*bottom,*top];error=max(min((p-q).length for q in actual) for p in anchors);assert error<.0003,error
    bounds=lambda pts:[[min(p[i] for p in pts) for i in range(3)],[max(p[i] for p in pts) for i in range(3)]]
    before,after=bounds(anchors),bounds(actual);assert max(abs(a-b) for aa,bb in zip(before,after) for a,b in zip(aa,bb))<.0003
    bpy.ops.file.pack_all();bpy.context.preferences.filepaths.save_version=0;bpy.ops.wm.save_as_mainfile(filepath=str(folder/'source.blend'))
    (folder/'validation/authored.json').write_text(json.dumps(dict(triangles=len(output),bounds_before=before,bounds_after=after,original_corner_anchors=len(anchors),maximum_anchor_distance=error,underside_original_faces=4,joint_centers_y=SEAMS),indent=2)+'\n')


if __name__=='__main__':build()
