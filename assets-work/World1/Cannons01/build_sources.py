"""Author bored swivel barrels, rounded shot and restrained fixed gallows joinery."""
import json
import math
from pathlib import Path
import sys
sys.dont_write_bytecode=True
import bpy
from mathutils import Vector
ROOT=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT))
from config import ASSETS,MATERIALS,PRODUCTION_ASSETS
from model_geometry import bounds,combine,fit,material,preserve_faces,preserve_original,shared
sys.path.insert(0,str(ROOT.parent/'CartHay01'))
# This local implementation avoids running the previous batch's build entry point.
IRON=(.858,.074,.985,.230)
DARK=(.525,.593,.582,.660)


def guard_normals(obj,rig):
    order=list(rig['mu_bone_order']);normals=[]
    for loop,corner in zip(obj.data.loops,obj.data.corner_normals):
        vertex=obj.data.vertices[loop.vertex_index]
        index=order.index(obj.vertex_groups[vertex.groups[0].group].name)+1
        normal=corner.vector.copy();direction=Vector((.00171*index,.00213*index,.00373*index)) if len(order)>1 else Vector((.000071*index,.000113*index,.000173*index))
        normals.append((normal+direction-normal*direction.dot(normal)).normalized())
    obj.data.normals_split_custom_set(normals)


def cannon(name,source,context):
    components=json.loads((ROOT/name/'original/components.json').read_text())
    remodeled={4}|({7,8,9} if name!='Cannon03' else set())
    omitted={i for n in remodeled for i in components[n]['faces']}
    result=[preserve_faces('Retained_Stand_Fuse_Boxes',source,[p for p in source.data.polygons if p.index not in omitted],'Box06',context)]
    # Axis follows the original bore line; separate inner return produces a real muzzle cavity.
    profile=[(0,0),(7,3),(11,9),(13,25),(14,29),(13,33),(9,110),(10,112),(10,116),(7,116),(6.7,99),(0,99)]
    barrel=shared.lathe('Forged_Bored_Barrel',profile,(0,0,0),16,IRON,'Box06',context)
    for polygon in barrel.data.polygons:
        level=polygon.index//16
        if level<8:continue
        rectangle=(480/1024,1-476/1024,556/1024,1-466/1024) if level==8 else (.55,.60,.60,.64)
        polygon.use_smooth=level!=8
        for loop in polygon.loop_indices:
            uv=barrel.data.uv_layers[0].data[loop].uv
            uv[:]=shared.uv(rectangle,(uv.x-IRON[0])/(IRON[2]-IRON[0]),(uv.y-IRON[1])/(IRON[3]-IRON[1]))
    axis=Vector((0,-.643,.766)).normalized();rotation=Vector((0,0,1)).rotation_difference(axis)
    for vertex in barrel.data.vertices:vertex.co=rotation@vertex.co
    fit([barrel],components[4]['bounds']);result.append(barrel)
    for index in sorted(remodeled-{4}):
        profile=[(0,0),(1,0)]+[(math.cos(a),math.sin(a)) for a in (math.pi/8,math.pi/4,3*math.pi/8)]+[(0,1)]
        shot=shared.lathe('Rounded_Shot_'+str(index),profile,(0,0,0),12,DARK,'Box06',context)
        fit([shot],components[index]['bounds']);result.append(shot)
    return result


def retained(name,source,rig,target,materials):
    result=[]
    for surface_index,filename in enumerate(MATERIALS[name]):
        polygons=[p for p in source.data.polygons if p.material_index==surface_index]
        indices=sorted({v for p in polygons for v in p.vertices})
        obj=preserve_faces('Retained_'+str(surface_index),source,polygons,rig['mu_bone_order'][0],(target,rig,materials[filename]))
        obj.vertex_groups.clear()
        for bone in rig['mu_bone_order']:
            group=obj.vertex_groups.new(name=bone)
            owned=[i for i,v in enumerate(indices) if source.vertex_groups[source.data.vertices[v].groups[0].group].name==bone]
            if owned:group.add(owned,1,'REPLACE')
        result.append(obj)
    return result


def build(name):
    folder=ROOT/name;bpy.ops.wm.open_mainfile(filepath=str(folder/'original/source.blend'));bpy.context.scene.frame_set(0)
    rig,reference=preserve_original();source=next(iter(reference.objects));before=bounds([source])
    target=shared.collection('EXPORT_'+name);materials={n:material(n) for n in MATERIALS[name]}
    if name.startswith('Cannon'):objects=cannon(name,source,(target,rig,materials[MATERIALS[name][0]]))
    else:objects=retained(name,source,rig,target,materials)
    if name=='Hanging01':
        context=(target,rig,materials[MATERIALS[name][0]])
        for index,z in enumerate((355,398)):
            objects.append(shared.box('Beam_Iron_Face_'+str(index),(25,92.05,z),(43,92.65,z+8),DARK,'Box08',context,.15))
        for x in (-88,82):
            objects.append(shared.box('Platform_Joint_'+str(x),(x,-97.40,68.8),(x+8,-96.75,80.5),DARK,'Box08',context,.12))
    shared.retain_high_poly(objects)
    # Retained triangles keep their winding and UVs exactly; recalc only authored geometry.
    authored=[o for o in objects if not o.name.startswith('Retained_')]
    shared.triangulate(authored)
    result=combine(objects,list(materials.values()),target,rig)
    for obj in objects:bpy.data.objects.remove(obj,do_unlink=True)
    if name in PRODUCTION_ASSETS:guard_normals(result,rig)
    after=bounds([result]);assert max(abs(a-b) for aa,bb in zip(before,after) for a,b in zip(aa,bb))<.001,(name,before,after)
    assert all(len(v.groups)==1 and v.groups[0].weight==1 for v in result.data.vertices)
    assert all(len(p.vertices)==3 and p.area>0 for p in result.data.polygons)
    assert len(result.data.polygons)<1500
    report=dict(triangles=len(result.data.polygons),bounds_before=before,bounds_after=after,material_order=MATERIALS[name],bone_order=list(rig['mu_bone_order']),action_meta=rig['mu_action_meta'].to_dict(),geometry='Bored 16-sided barrel and rounded shot' if name.startswith('Cannon') else 'Original gallows plus fixed iron joints; animated rope retained' if name=='Hanging01' else 'Readonly original geometry and BMD bytes retained; shared texture compatibility')
    (folder/'validation/blender.json').write_text(json.dumps(report,indent=2)+'\n')
    for image in bpy.data.images:
        if image.source=='FILE':image.pack()
    bpy.context.preferences.filepaths.save_version=0;bpy.ops.wm.save_as_mainfile(filepath=str(folder/'source.blend'))
    print(name,report['triangles'],flush=True)

for name in (sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else ASSETS):build(name)
