"""Corner-exact authored/export, raw normal and original contact proof."""
from pathlib import Path
import json,sys,os,math
sys.dont_write_bytecode=True
import bpy
from mathutils import Vector,Euler,Matrix
ROOT=Path(__file__).resolve().parent;REPO=ROOT.parents[2]
sys.path.insert(0,str(REPO/'assets-work/World1/Architecture03'))
import raw_bindings
POSITION_TOLERANCE=.0003
UV_TOLERANCE=.000001
NORMAL_TOLERANCE=.001


def read_smd(path):
    text=path.read_text();lines=text.split('triangles\n')[1].splitlines()
    triangles=[(lines[i],[list(map(float,row.split())) for row in lines[i+1:i+4]]) for i in range(0,len(lines)-1,4)]
    root=text.split('skeleton\n')[1].split('end')[0].splitlines()[1].split()
    values=list(map(float,root[1:]));world=Matrix.Translation(Vector(values[:3]))@Euler(values[3:]).to_matrix().to_4x4()
    return triangles,world


def match(material,rows,candidates):
    for index,(m,other) in enumerate(candidates):
        if material!=m:continue
        for shift in range(3):
            ordered=other[shift:]+other[:shift]
            if any(int(a[0])!=int(b[0]) for a,b in zip(rows,ordered)):continue
            if max(abs(a[i]-b[i]) for a,b in zip(rows,ordered) for i in range(1,4))>POSITION_TOLERANCE:continue
            if max(abs(a[i]-b[i]) for a,b in zip(rows,ordered) for i in (7,8))>UV_TOLERANCE:continue
            candidates.pop(index);return ordered
    raise AssertionError(('No cyclic triangle match',material,rows))


def audit(name):
    folder=ROOT/name;actual,final_world=read_smd(folder/'validation/new'/f'{name}.smd');original,old_world=read_smd(folder/'baseline/smd'/f'{name}.smd')
    raw=raw_bindings.meshes(folder/'exports'/f'{name}.bmd');assert len(raw)==1 and raw[0]['material']=='wood01.jpg'
    mesh=raw[0];lookup={}
    for (material,rows),(vertices,normals) in zip(actual,mesh['triangles']):
        for row,vertex,normal in zip(rows,vertices,normals):
            record=mesh['normals'][normal];assert record[0]==mesh['vertices'][vertex][0]==0
            lookup[id(row)]=(final_world.to_3x3()@Vector(record[1:4])).normalized()
    assert len(actual)==len(mesh['triangles'])
    candidates=actual.copy();bpy.ops.wm.open_mainfile(filepath=str(folder/'source.blend'))
    obj=next(o for o in bpy.context.scene.objects if o.type=='MESH' and not o.get('mu_helper') and not o.get('mu_reference'))
    rig=next(o for o in bpy.context.scene.objects if o.type=='ARMATURE');assert len(rig.data.bones)==1
    assert len(obj.data.uv_layers)==1
    errors=[];areas=[];normals=[]
    for face in obj.data.polygons:
        rows=[];source_normals=[]
        for loop in face.loop_indices:
            vertex=obj.data.vertices[obj.data.loops[loop].vertex_index];assert len(vertex.groups)==1 and vertex.groups[0].weight==1
            assert obj.vertex_groups[vertex.groups[0].group].name==rig['mu_bone_order'][0]
            normal=(obj.matrix_world.to_3x3().inverted().transposed()@obj.data.corner_normals[loop].vector).normalized()
            rows.append([0,*(obj.matrix_world@vertex.co),*normal,*obj.data.uv_layers[0].data[loop].uv]);source_normals.append(normal)
        result=match(obj.data.materials[face.material_index].name,rows,candidates)
        errors.extend(abs(a[i]-b[i]) for a,b in zip(rows,result) for i in range(1,4))
        normals.extend((n-lookup[id(row)]).length for n,row in zip(source_normals,result))
        a,b,c=[Vector(row[1:4]) for row in result];areas.append((b-a).cross(c-a).length/2)
    assert not candidates and min(areas)>.000001 and max(normals)<NORMAL_TOLERANCE
    anchors={tuple(row[1:4]) for m,rows in original for row in rows};points=[Vector(row[1:4]) for m,rows in actual for row in rows]
    anchor_error=max(min((Vector(p)-q).length for q in points) for p in anchors);assert anchor_error<POSITION_TOLERANCE
    protected=actual.copy()
    metadata=json.loads((folder/'validation/authored.json').read_text())
    protected_indices=list(range(4)) if name=='Object22' else metadata['protected_original_faces']
    if protected_indices:
        for material,rows in [original[i] for i in protected_indices]:
            after=match(material,rows,protected)
            assert max((Vector(a[4:7]).normalized()-Vector(b[4:7]).normalized()).length for a,b in zip(rows,after))<NORMAL_TOLERANCE
    images=[image for image in bpy.data.images if image.source=='FILE'];assert images and all(image.packed_file for image in images)
    assert bpy.data.collections['REF_ORIGINAL'].hide_render
    report=dict(status='PASS',triangles=len(actual),position_tolerance=POSITION_TOLERANCE,maximum_position_delta=max(errors),UV_tolerance=UV_TOLERANCE,normal_tolerance=NORMAL_TOLERANCE,maximum_authored_raw_world_normal_delta=max(normals),minimum_triangle_area=min(areas),original_contact_corners=len(anchors),maximum_contact_distance=anchor_error,protected_original_triangles=len(protected_indices),underside_exact_triangles=4 if name=='Object22' else None,packed_images=[image.name for image in images],winding='Every actual triangle matches authored cyclic order, no reversed match allowed')
    (folder/'validation/authored-export-contract.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))


for name in os.environ.get('COFFIN_NAMES',os.environ.get('COFFIN_NAME','Object22')).split(','):audit(name)
