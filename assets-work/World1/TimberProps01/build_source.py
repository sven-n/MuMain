"""Build the owned barrel, sloped weapons bin and animated hanging lantern."""
import json
import math
from pathlib import Path
import sys
import traceback

import bmesh
import bpy
from mathutils import Vector

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT.parent / 'StaticBatch01'))
from geometry import apply_bevel, box, collection, lathe, material, mesh, preserve_original, retain_high_poly, triangulate, tube, uv
from review_scene import mesh_bounds, set_camera, set_lighting

NAMES = ('House02', 'TreasureDrum01', 'StreetLight01')
OAK = (.02, .085, .98, .555)
IRON = (.02, .578, .98, .62)
LAMP_OAK = (.02, .03, .69, .97)
LAMP_IRON = (.75, .54, .97, .96)
LAMP_BRONZE = (.75, .04, .97, .46)


def copy_original_faces(original, material_index, name, bone, context, required_group=None):
    vertices, faces, coordinates = [], [], []
    for face in original.data.polygons:
        if face.material_index != material_index:
            continue
        if required_group and not all(original.vertex_groups[original.data.vertices[i].groups[0].group].name == required_group for i in face.vertices):
            continue
        indices = []
        for index in face.vertices:
            indices.append(len(vertices))
            vertices.append(tuple(original.matrix_world @ original.data.vertices[index].co))
        faces.append(indices)
        coordinates.append([tuple(original.data.uv_layers[0].data[loop].uv) for loop in face.loop_indices])
    return mesh(name, vertices, faces, coordinates, bone, context)


def wedge(name, low, high, top_front, top_back, rect, bone, context, bevel=.25):
    vertices = [(x, y, low[2]) for y in (low[1], high[1]) for x in (low[0], high[0])]
    vertices += [(x, y, z) for y, z in ((low[1], top_front), (high[1], top_back)) for x in (low[0], high[0])]
    faces = [(0, 2, 3, 1), (4, 5, 7, 6), (0, 1, 5, 4), (2, 6, 7, 3), (0, 4, 6, 2), (1, 3, 7, 5)]
    coordinates = [[uv(rect, u, v) for u, v in ((0,0),(1,0),(1,1),(0,1))]] * len(faces)
    obj = mesh(name, vertices, faces, coordinates, bone, context)
    if bevel:
        apply_bevel(obj, bevel)
        obj.vertex_groups[bone].add(list(range(len(obj.data.vertices))), 1, 'REPLACE')
    return obj


def crate_height(y):
    return 117.976 + (y + 41.262) / (40.075 + 41.262) * (143.143 - 117.976)


def crate_walls(bone, context):
    x0, x1, y0, y1, z0 = -76.385, 75.995, -41.262, 40.075, -.161553
    for side, (low_y, high_y) in enumerate(((y0+.35, -31.403), (30.216, y1-.35))):
        for index in range(5):
            left, right = x0 + (x1-x0)*index/5, x0 + (x1-x0)*(index+1)/5
            rect = (.02+index*.19, OAK[1], .20+index*.19, OAK[3])
            wedge(f'Crate_{side}_Plank_{index}', (left+.12,low_y,z0), (right-.12,high_y,0),
                  crate_height(low_y), crate_height(high_y), rect, bone, context)
    for side, (low_x, high_x) in enumerate(((x0+.35,-68.304), (67.915,x1-.35))):
        for index in range(3):
            front, back = y0+(y1-y0)*index/3, y0+(y1-y0)*(index+1)/3
            wedge(f'Crate_Side_{side}_Plank_{index}', (low_x,front+.12,z0), (high_x,back-.12,0),
                  crate_height(front+.12), crate_height(back-.12), OAK, bone, context)
    wedge('Crate_Original_Sloped_Display_Bed', (-68.304,-31.403,92), (67.915,30.216,0), 96.12,115.331,OAK,bone,context,bevel=0)


def crate_bands(bone, context):
    x0, x1, y0, y1, z0 = -76.385, 75.995, -41.262, 40.075, -.161553
    for y in (y0, y1-.60):
        for top in (False, True):
            low = crate_height(y)-7 if top else z0
            box('Crate_FrontBack_Iron_Band', (x0,y,low), (x1,y+.6,low+7), IRON,bone,context,bevel=.12)
    for x in (x0, x1-.60):
        box('Crate_Side_Lower_Band',(x,y0,z0),(x+.6,y1,z0+7),IRON,bone,context,bevel=.12)
        # Sloped side rails use a translated wedge, keeping their bottom edge parallel to their top.
        obj = wedge('Crate_Sloped_Iron_Rim',(x,y0,0),(x+.6,y1,0),7,7,IRON,bone,context,bevel=0)
        for vertex in obj.data.vertices:
            vertex.co.z += crate_height(vertex.co.y)-7
        apply_bevel(obj,.12)
        obj.vertex_groups[bone].add(list(range(len(obj.data.vertices))),1,'REPLACE')


def build_crate(original, rig, target, root):
    bone = 'Box02'
    context = (target,rig,material(root/'textures/drum.jpg'))
    crate_walls(bone,context)
    crate_bands(bone,context)
    metal = copy_original_faces(original,1,'Preserved_Static_Blades_And_Shields',bone,
                                (target,rig,material(root/'textures/steel.jpg')))
    metal['mu_preserved_geometry'] = True


def barrel_staves(center, z0, z1, bone, context):
    levels = [(33.0012,z0),(34.0,z0+7),(40.6342,34.3277),(40.6342,68.6534),(34.0,z1-7),(33.0012,z1)]
    segments, gap = 16, .002
    for index in range(segments):
        angle = index*math.tau/segments
        angles = (angle-math.pi/segments+gap,angle,angle+math.pi/segments-gap)
        vertices = []
        for radius,z in levels:
            vertices += [(center[0]+radius*math.cos(a),center[1]+radius*math.sin(a),z) for a in angles]
        faces,coords = [],[]
        for level in range(len(levels)-1):
            for edge in range(2):
                faces.append((level*3+edge,level*3+edge+1,(level+1)*3+edge+1,(level+1)*3+edge))
                u0,u1 = (index+edge/2)/segments,(index+(edge+1)/2)/segments
                v0,v1 = (levels[level][1]-z0)/(z1-z0),(levels[level+1][1]-z0)/(z1-z0)
                coords.append([uv(OAK,u,v) for u,v in ((u0,v0),(u1,v0),(u1,v1),(u0,v1))])
        obj = mesh(f'Coopered_Stave_{index:02}',vertices,faces,coords,bone,context)
        modifier = obj.modifiers.new('Authored_Stave_Thickness','SOLIDIFY')
        modifier.thickness = .7
        modifier.offset = -1
        bpy.context.view_layer.objects.active=obj
        bpy.ops.object.modifier_move_up(modifier=modifier.name)
        bpy.ops.object.modifier_apply(modifier=modifier.name)
        obj.vertex_groups[bone].add(list(range(len(obj.data.vertices))),1,'REPLACE')


def barrel_lid(center,z,bone,context):
    count,radius = 32,32.4
    vertices = [(center[0],center[1],z)] + [(center[0]+radius*math.cos(i*math.tau/count),center[1]+radius*math.sin(i*math.tau/count),z) for i in range(count)]
    faces = [(0,i+1,(i+1)%count+1) for i in range(count)]
    def lid_uv(index):
        if index==0: return (.25,.812)
        angle=(index-1)*math.tau/count
        return (.25+.172*math.cos(angle),.812+.15*math.sin(angle))
    mesh('Inset_Planked_Barrel_Lid',vertices,faces,[[lid_uv(i) for i in face] for face in faces],bone,context)


def build_barrel(rig,target,root):
    bone,center,z0,z1='Cylinder03',(.0669,.4184),.001923,102.9792
    context=(target,rig,material(root/'textures/drum.jpg'))
    barrel_staves(center,z0,z1,bone,context)
    for low,high in ((2,9),(94,101)):
        profile=[(33.4,low),(34.6,low+.65),(35.15,high-.65),(33.8,high)]
        if low>50: profile=[(r,low+high-z) for r,z in profile][::-1]
        lathe('Raised_Forged_Hoop',profile,(center[0],center[1],0),16,IRON,bone,context,smooth=False)
    barrel_lid(center,z1-.40,bone,context)
    lathe('Top_Stave_Lip',[(32.4,z1-.4),(33.0012,z1),(31.9,z1)],(center[0],center[1],0),32,OAK,bone,context,smooth=False)
    lathe('Bottom_Closure',[(0,z0),(33.0012,z0)],(center[0],center[1],0),16,OAK,bone,context,smooth=False)


def remap_timber(obj):
    for face in obj.data.polygons:
        points=[obj.data.vertices[i].co for i in face.vertices]
        spans=[max(p[a] for p in points)-min(p[a] for p in points) for a in range(3)]
        long_axis=max(range(3),key=lambda a:spans[a])
        short_axis=max((a for a in range(3) if a!=long_axis),key=lambda a:spans[a])
        low=[min(p[a] for p in points) for a in range(3)]
        for loop,index in zip(face.loop_indices,face.vertices):
            point=obj.data.vertices[index].co
            u=(point[short_axis]-low[short_axis])/max(spans[short_axis],.001)
            v=(point[long_axis]-low[long_axis])/max(spans[long_axis],.001)
            obj.data.uv_layers[0].data[loop].uv=uv(LAMP_OAK,u,v)


def build_lantern(original,rig,target,root):
    context=(target,rig,material(root/'textures/streetlight.jpg'))
    timber=copy_original_faces(original,0,'Braced_Timber_Post','Mesh02',context,required_group='Mesh02')
    bm=bmesh.new(); bm.from_mesh(timber.data)
    bmesh.ops.remove_doubles(bm,verts=list(bm.verts),dist=.0001)
    bmesh.ops.dissolve_limit(bm,angle_limit=.01,verts=list(bm.verts),edges=list(bm.edges))
    bm.to_mesh(timber.data); bm.free()
    base = [v.co.copy() for v in timber.data.vertices if v.co.z < 1]
    foot_low = tuple(min(point[axis] for point in base) for axis in range(3))
    foot_high = (max(point.x for point in base), max(point.y for point in base), 10.02)
    for vertex in timber.data.vertices:
        if vertex.co.z < 10:
            vertex.co.z = 10
    remap_timber(timber)
    apply_bevel(timber,.22)
    timber.vertex_groups['Mesh02'].add(list(range(len(timber.data.vertices))),1,'REPLACE')
    box('Iron_Post_Foot',foot_low,foot_high,LAMP_IRON,'Mesh02',context,bevel=.22)
    for y in (-69,-12):
        box('Iron_Crossbeam_Strap',(-6.70,y,245.40),(6.66,y+3,258.317),LAMP_IRON,'Mesh02',context,bevel=.2)
    center=(.2635,-62.284,0)
    lathe('Forged_Lantern_Foot',[(0,206.39),(7.7,206.39),(8.05,208),(8.05,212.9),(6.9,214.0)],center,12,LAMP_BRONZE,'Bone02',context,smooth=False)
    lathe('Tiered_Lantern_Cap',[(9.2,238.5),(9.4,239.1),(5.6,242.0),(3.6,242.2),(.08,245.45)],center,12,LAMP_BRONZE,'Bone02',context,smooth=False)
    for index in range(6):
        angle = math.pi / 6 + index * math.tau / 6
        profile = [(8.05,213.145),(11.35,218.14),(12.369,225.42),(9.333,238.55)]
        centers = [(center[0]+(radius*math.cos(math.pi/6)+.05)*math.cos(angle), center[1]+(radius*math.cos(math.pi/6)+.05)*math.sin(angle), z) for radius,z in profile]
        tube(f'Forged_Glass_Stay_{index}', centers, [.15]*4, 4, LAMP_IRON, 'Bone02', context)
    # Bright mesh is renderer-addressed by index and retained exactly, including its legacy repeating UVs.
    glow=copy_original_faces(original,1,'Preserved_Additive_Glass','Bone02',
                             (target,rig,material(root/'textures/streetlight_brightness2.jpg')))
    glow['mu_preserved_glow']=True


def validate_source(root,objects,rig,bounds):
    triangles=0
    for obj in objects:
        assert len(obj.data.materials)==1 and len(obj.data.uv_layers)==1
        for vertex in obj.data.vertices:
            assert len(vertex.groups)==1 and vertex.groups[0].weight==1
            name=obj.vertex_groups[vertex.groups[0].group].name
            assert name in rig.data.bones and not name.startswith('Du')
        lower=-.1 if obj.get('mu_preserved_glow') else 0
        assert all(math.isfinite(value) and lower<=value<=1 for point in obj.data.uv_layers[0].data for value in point.uv)
        assert all(len(face.vertices)==3 and face.area>.000001 for face in obj.data.polygons)
        triangles+=len(obj.data.polygons)
    assert triangles<=1500,triangles
    current=mesh_bounds(objects)
    assert max(abs(a-b) for old,new in zip(bounds,current) for a,b in zip(old,new))<.03,(root.name,bounds,current)
    textures={mat.name:list(node.image.size) for obj in objects for mat in obj.data.materials for node in mat.node_tree.nodes if node.type=='TEX_IMAGE'}
    report=dict(status='PASS: authored geometry; engine validation separately required',triangles=triangles,
                bounds_before=bounds,bounds_after=current,bone_order=list(rig['mu_bone_order']),
                action_meta=rig['mu_action_meta'].to_dict(),textures=textures,
                skinning='Exactly one valid non-dummy full-weight bone per vertex',
                uv='One UVMap set, finite; new UVs [0,1]; original lantern glow repeat to -0.0936 retained',
                materials='Original order and texture filenames; one diffuse image per mesh; no alpha or flags added')
    (root/'validation/blender.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report))


def build(name):
    root=ROOT/name
    bpy.ops.wm.open_mainfile(filepath=str(root/'original/source.blend'))
    bpy.context.scene.frame_set(0)
    rig,reference=preserve_original()
    original=next(iter(reference.objects)); bounds=mesh_bounds(list(reference.objects))
    target=collection('EXPORT_'+name)
    if name=='House02': build_crate(original,rig,target,root)
    elif name=='TreasureDrum01': build_barrel(rig,target,root)
    else: build_lantern(original,rig,target,root)
    objects=list(target.objects)
    retain_high_poly(objects); triangulate(objects)
    bpy.context.view_layer.update(); validate_source(root,objects,rig,bounds)
    set_camera(bounds); set_lighting()
    for image in bpy.data.images:
        if image.source=='FILE': image.pack()
    bpy.context.preferences.filepaths.save_version=0
    bpy.context.scene['review_status']='OFFLINE BLENDER; CLIENT VERIFICATION PENDING'
    bpy.ops.wm.save_as_mainfile(filepath=str(root/'source.blend'))


if __name__=='__main__':
    try:
        for name in NAMES: build(name)
    except Exception:
        traceback.print_exc(); sys.exit(1)
