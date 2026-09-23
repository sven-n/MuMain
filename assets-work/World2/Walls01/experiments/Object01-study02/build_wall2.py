"""Dress the central crown with broad stone courses; preserve module ends and plaque."""
from pathlib import Path
import sys,json
sys.dont_write_bytecode=True
import bpy
from mathutils import Vector
from mathutils.geometry import tessellate_polygon
ROOT=Path(__file__).resolve().parent
NAME='Object01'
REPLACED_FACES={2,3,18,19}
CROWN_STATIONS=((221.0,-44.0),(233.0,-40.0),(240.0,-47.0),(278.0,-39.5),(284.0,-39.2))


def read_triangles():
    lines=(ROOT/NAME/'baseline/smd'/f'{NAME}.smd').read_text().split('triangles\n')[1].splitlines()
    return [(lines[i],[list(map(float,row.split())) for row in lines[i+1:i+4]]) for i in range(0,len(lines)-1,4)]


def sample_uv(point,triangles,axes):
    for _,rows in triangles:
        a,b,c=[Vector((r[axes[0]],r[axes[1]])) for r in rows];p=Vector((point[axes[0]-1],point[axes[1]-1]))
        u,v,q=b-a,c-a,p-a;den=u.x*v.y-u.y*v.x
        if abs(den)<1e-9:continue
        s=(q.x*v.y-q.y*v.x)/den;t=(u.x*q.y-u.y*q.x)/den
        if min(s,t,1-s-t)<-1e-5:continue
        return Vector(rows[0][7:9])*(1-s-t)+Vector(rows[1][7:9])*s+Vector(rows[2][7:9])*t
    raise AssertionError(('Outside original UV patch',point,axes))


def append_triangle(output,points,uvs):
    normal=(points[1]-points[0]).cross(points[2]-points[0]).normalized()
    assert normal.length>.99
    output.append([[0,*p,*normal,*uv] for p,uv in zip(points,uvs)])


def crown(original,output):
    a,b,c=[Vector(row[1:4]) for row in original[18][1]];d=Vector(original[19][1][1][1:4])
    back_left=Vector(original[3][1][1][1:4]);back_right=Vector(original[2][1][2][1:4])
    upper_y=-19.5
    top_z=d.z+(back_left.z-d.z)*(upper_y-d.y)/(back_left.y-d.y)
    interior=[(a.z,a.y),(233.0,-42.0),(242.0,-46.0),(253.0,-46.0),(279.0,-24.0),(top_z,upper_y)]
    columns=[]
    for x,inside in ((a.x,False),(a.x+12,True),(b.x-12,True),(b.x,False)):
        column=[]
        for index,(z,y) in enumerate(interior):
            if not inside:
                if index==len(interior)-1:z=d.z
                y=a.y+(d.y-a.y)*(z-a.z)/(d.z-a.z)
            column.append(Vector((x,y,z)))
        columns.append(column)
    for left,right in zip(columns,columns[1:]):
        for index in range(len(interior)-1):
            points=[left[index],right[index],right[index+1],left[index+1]]
            uvs=[sample_uv(p,[original[18],original[19]],(1,3)) for p in points]
            for ids in ((0,1,2),(0,2,3)):append_triangle(output,[points[i] for i in ids],[uvs[i] for i in ids])
    roof=[column[-1] for column in columns]+[back_right,back_left]
    for indices in tessellate_polygon([roof]):
        points=[roof[value] if isinstance(value,int) else value for value in indices]
        if (points[1]-points[0]).cross(points[2]-points[0]).z<0:points.reverse()
        append_triangle(output,points,[sample_uv(p,[original[2],original[3]],(1,2)) for p in points])


def replace_mesh(obj,rows):
    old=obj.data;groups=[g.name for g in obj.vertex_groups];inverse=obj.matrix_world.inverted();mesh=bpy.data.meshes.new('DressedCentralCrown')
    mesh.from_pydata([inverse@Vector(row[1:4]) for face in rows for row in face],[],[(i,i+1,i+2) for i in range(0,len(rows)*3,3)])
    [mesh.materials.append(material) for material in old.materials];layer=mesh.uv_layers.new(name=old.uv_layers[0].name);obj.data=mesh;obj.vertex_groups.clear()
    for name in groups:obj.vertex_groups.new(name=name)
    normals=[];transform=obj.matrix_world.to_3x3().transposed()
    for face,source in zip(mesh.polygons,rows):
        face.use_smooth=True;face.material_index=1 if face.index>=len(rows)-2 else 0
        for loop,row in zip(face.loop_indices,source):
            layer.data[loop].uv=row[7:9];obj.vertex_groups[0].add([mesh.loops[loop].vertex_index],1,'REPLACE');normals.append((transform@Vector(row[4:7])).normalized())
    mesh.normals_split_custom_set(normals);mesh.update()


def build():
    folder=ROOT/NAME;bpy.ops.wm.open_mainfile(filepath=str(folder/'baseline/source.blend'));original=read_triangles()
    objects=[o for o in bpy.context.scene.objects if o.type=='MESH' and not o.get('mu_helper')]
    reference=bpy.data.collections.new('REF_ORIGINAL');bpy.context.scene.collection.children.link(reference);reference.hide_render=reference.hide_viewport=True
    for obj in objects:
        clone=obj.copy();clone.data=obj.data.copy();clone['mu_reference']=True;reference.objects.link(clone)
    obj=next(o for o in objects if o.data.materials[0].name=='deep_wall01.jpg')
    output=[rows for i,(material,rows) in enumerate(original[:56]) if i not in REPLACED_FACES];crown(original,output);output.extend(rows for material,rows in original[56:]);replace_mesh(obj,output)
    old_points=[r[1:4] for _,rows in original for r in rows];new_points=[r[1:4] for rows in output for r in rows]
    bounds=lambda points:[[min(p[k] for p in points) for k in range(3)],[max(p[k] for p in points) for k in range(3)]]
    assert max(abs(a-b) for aa,bb in zip(bounds(old_points),bounds(new_points)) for a,b in zip(aa,bb))<.00003
    bpy.ops.file.pack_all();bpy.context.preferences.filepaths.save_version=0;bpy.ops.wm.save_as_mainfile(filepath=str(folder/'source.blend'))
    (folder/'validation/authored.json').write_text(json.dumps(dict(triangles=len(output),replaced_original_faces=sorted(REPLACED_FACES),protected_original_faces=[i for i in range(58) if i not in REPLACED_FACES],bounds_before=bounds(old_points),bounds_after=bounds(new_points),construction='Broad sloped upper stone face across143-unit central span with12-unit end transitions; short bed/neck courses beneath; exact existing planar UV projection. Entire crown side profiles, module ends, ground and plaque remain original'),indent=2)+'\n')

if __name__=='__main__':build()
