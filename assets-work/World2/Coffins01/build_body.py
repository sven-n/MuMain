"""Construct broad bevelled timber boards inside the original coffin wall envelope."""
from pathlib import Path
import sys,json
sys.dont_write_bytecode=True
import bpy
from mathutils import Vector
ROOT=Path(__file__).resolve().parent;sys.path.insert(0,str(ROOT))
from build_lid import read_triangles,replace_mesh,append_triangle
NAME='Object21'
CORNER_STILE_WIDTH=5.0
EDGE_RAIL_HEIGHT=4.0
RECESS_BEVEL_WIDTH=1.4
BOARD_BEVEL_WIDTH=1.3
JOINT_DEPTH=2.6
BOARD_FACE_DEPTH=.6
PAINTED_JOINT_V=(.18359375,.265625,.33984375)


def rectangle(t0,t1,z0,z1):return [(t0,z0),(t1,z0),(t1,z1),(t0,z1)]


def append_quad(output,coordinates,depth,surface):
    values=[surface(t,z,depth) for t,z in coordinates]
    for indices in ((0,1,2),(0,2,3)):
        append_triangle(output,[values[i][0] for i in indices],[values[i][1] for i in indices])


def ring(output,outside,inside,depth_out,depth_in,surface):
    for i in range(4):
        j=(i+1)%4
        points=[(*outside[i],depth_out),(*outside[j],depth_out),(*inside[j],depth_in),(*inside[i],depth_in)]
        values=[surface(*p) for p in points]
        for indices in ((0,1,2),(0,2,3)):
            append_triangle(output,[values[k][0] for k in indices],[values[k][1] for k in indices])


def wall(output,rows):
    a,b,top_b=[Vector(row[1:4]) for row in rows[:3]];top_a=Vector(rows[4][1:4])
    normal=(b-a).cross(top_b-a).normalized();width=(b-a).length;low,high=a.z,top_b.z
    uva,uvb,uvd=[Vector(row[7:9]) for row in rows[:3]];uvc=Vector(rows[4][7:9])
    def surface(t,z,depth):
        p=a.lerp(b,t);p.z=z;p-=normal*depth
        alpha=(z-low)/(high-low)
        uv=uva.lerp(uvb,t).lerp(uvc.lerp(uvd,t),alpha)
        return p,uv
    frame=rectangle(0,1,low,high)
    frame_in=rectangle(CORNER_STILE_WIDTH/width,1-CORNER_STILE_WIDTH/width,low+EDGE_RAIL_HEIGHT,high-EDGE_RAIL_HEIGHT)
    ring(output,frame,frame_in,0,0,surface)
    t0=frame_in[0][0]+RECESS_BEVEL_WIDTH/width;t1=frame_in[1][0]-RECESS_BEVEL_WIDTH/width
    z0=frame_in[0][1]+RECESS_BEVEL_WIDTH;z1=frame_in[2][1]-RECESS_BEVEL_WIDTH
    panel=rectangle(t0,t1,z0,z1);ring(output,frame_in,panel,0,JOINT_DEPTH,surface)
    levels=[z0,z1]
    for v in PAINTED_JOINT_V:
        z=low+(v-uva.y)/(uvc.y-uva.y)*(high-low)
        if z0<z<z1:levels.append(z)
    for bottom,top in zip(sorted(levels),sorted(levels)[1:]):
        perimeter=rectangle(t0,t1,bottom,top)
        inner=rectangle(t0+BOARD_BEVEL_WIDTH/width,t1-BOARD_BEVEL_WIDTH/width,bottom+BOARD_BEVEL_WIDTH,top-BOARD_BEVEL_WIDTH)
        ring(output,perimeter,inner,JOINT_DEPTH,BOARD_FACE_DEPTH,surface)
        append_quad(output,inner,BOARD_FACE_DEPTH,surface)


def build():
    folder=ROOT/NAME;bpy.ops.wm.open_mainfile(filepath=str(folder/'baseline/source.blend'))
    obj=next(o for o in bpy.context.scene.objects if o.type=='MESH' and not o.get('mu_helper'))
    collection=bpy.data.collections.new('REF_ORIGINAL');bpy.context.scene.collection.children.link(collection);collection.hide_render=collection.hide_viewport=True
    clone=obj.copy();clone.data=obj.data.copy();clone['mu_reference']=True;collection.objects.link(clone)
    triangles=read_triangles(folder/'baseline/smd'/f'{NAME}.smd');protected=[*range(4),*range(16,40)]
    output=[([Vector(row[1:4]) for row in triangles[i]],[Vector(row[7:9]) for row in triangles[i]]) for i in protected]
    source_normals=[Vector(row[4:7]) for i in protected for row in triangles[i]]
    for index in range(4,16,2):wall(output,triangles[index]+triangles[index+1])
    replace_mesh(obj,output)
    normals=[Vector() for _ in obj.data.loops];transform=obj.matrix_world.to_3x3().transposed()
    for i,normal in enumerate(source_normals):normals[i]=(transform@normal).normalized()
    for face in obj.data.polygons:face.use_smooth=True
    obj.data.normals_split_custom_set(normals)
    original=[Vector(row[1:4]) for triangle in triangles for row in triangle];actual=[obj.matrix_world@v.co for v in obj.data.vertices]
    anchors={tuple(p) for p in original};error=max(min((Vector(p)-q).length for q in actual) for p in anchors);assert error<.0003
    bound=lambda points:[[min(p[i] for p in points) for i in range(3)],[max(p[i] for p in points) for i in range(3)]]
    before,after=bound(original),bound(actual);assert max(abs(a-b) for aa,bb in zip(before,after) for a,b in zip(aa,bb))<.0003
    bpy.ops.file.pack_all();bpy.context.preferences.filepaths.save_version=0;bpy.ops.wm.save_as_mainfile(filepath=str(folder/'source.blend'))
    (folder/'validation/authored.json').write_text(json.dumps(dict(triangles=len(output),bounds_before=before,bounds_after=after,original_corner_anchors=len(anchors),maximum_anchor_distance=error,protected_original_faces=protected,original_rim_floor_inner_walls_exact=True,construction='Four broad wood board faces perwall, bevelled into joints, cornerstiles and upper/lower rails, entirely inside original outerwall'),indent=2)+'\n')


if __name__=='__main__':build()
