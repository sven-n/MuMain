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


def raw_normals(actual,meshes,world):
    lookup={};offset=0
    assert [m['material'] for m in meshes]==['deep_wall01.jpg','deep_wall02.jpg']
    for mesh in meshes:
        for material_rows,(vertices,normals) in zip(actual[offset:offset+len(mesh['triangles'])],mesh['triangles']):
            material,rows=material_rows;assert material==mesh['material']
            for row,vertex,normal in zip(rows,vertices,normals):
                record=mesh['normals'][normal];assert record[0]==mesh['vertices'][vertex][0]==0
                lookup[id(row)]=(world.to_3x3()@Vector(record[1:4])).normalized()
        offset+=len(mesh['triangles'])
    assert offset==len(actual)
    return lookup


def authored_object(obj,rig,candidates,lookup):
    assert len(obj.data.uv_layers)==1
    errors=[];normals=[];uv_errors=[]
    for face in obj.data.polygons:
        rows=[];source_normals=[]
        for loop in face.loop_indices:
            vertex=obj.data.vertices[obj.data.loops[loop].vertex_index];assert len(vertex.groups)==1 and vertex.groups[0].weight==1
            assert obj.vertex_groups[vertex.groups[0].group].name==rig['mu_bone_order'][0]
            normal=(obj.matrix_world.to_3x3().inverted().transposed()@obj.data.corner_normals[loop].vector).normalized()
            rows.append([0,*(obj.matrix_world@vertex.co),*normal,*obj.data.uv_layers[0].data[loop].uv]);source_normals.append(normal)
        result=match(obj.data.materials[face.material_index].name,rows,candidates)
        errors.extend(abs(a[i]-b[i]) for a,b in zip(rows,result) for i in range(1,4))
        uv_errors.extend(abs(a[i]-b[i]) for a,b in zip(rows,result) for i in (7,8))
        normals.extend((n-lookup[id(row)]).length for n,row in zip(source_normals,result))
    return max(errors),max(normals),max(uv_errors)


def triangle_quality(actual,lookup):
    areas=[];uv_areas=[];normal_dots=[]
    for material,rows in actual:
        a,b,c=[Vector(r[1:4]) for r in rows];cross=(b-a).cross(c-a);areas.append(cross.length/2)
        assert cross.length>1e-6
        normal_dots.extend(cross.normalized().dot(lookup[id(row)]) for row in rows)
        a,b,c=[Vector(r[7:9]) for r in rows];u,v=b-a,c-a;uv_areas.append(abs(u.x*v.y-u.y*v.x)/2)
    assert min(areas)>1e-6 and min(uv_areas)>1e-10 and min(normal_dots)>0
    return dict(minimum_triangle_area=min(areas),minimum_UV_area=min(uv_areas),minimum_corner_normal_face_dot=min(normal_dots))


def audit(name):
    folder=ROOT/name;actual,world=read_smd(folder/'validation/new'/f'{name}.smd');original,_=read_smd(folder/'baseline/smd'/f'{name}.smd')
    lookup=raw_normals(actual,raw_bindings.meshes(folder/'exports'/f'{name}.bmd'),world)
    candidates=actual.copy();bpy.ops.wm.open_mainfile(filepath=str(folder/'source.blend'))
    rig=next(o for o in bpy.context.scene.objects if o.type=='ARMATURE');assert len(rig.data.bones)==1
    objects=[o for o in bpy.context.scene.objects if o.type=='MESH' and not o.get('mu_helper') and not o.get('mu_reference')];assert objects and sum(len(o.data.materials) for o in objects)==2
    errors=[authored_object(obj,rig,candidates,lookup) for obj in objects];assert not candidates
    assert max(e[1] for e in errors)<NORMAL_TOLERANCE
    metadata=json.loads((folder/'validation/authored.json').read_text());protected=actual.copy();protected_normal=[]
    for index in metadata['protected_original_faces']:
        material,rows=original[index];after=match(material,rows,protected)
        protected_normal.extend((Vector(a[4:7]).normalized()-Vector(b[4:7]).normalized()).length for a,b in zip(rows,after))
    assert max(protected_normal)<NORMAL_TOLERANCE
    images=[image for image in bpy.data.images if image.source=='FILE'];assert len(images)==2 and all(image.packed_file for image in images)
    assert bpy.data.collections['REF_ORIGINAL'].hide_render
    report=dict(status='PASS',triangles=len(actual),position_tolerance=POSITION_TOLERANCE,maximum_position_delta=max(e[0] for e in errors),UV_tolerance=UV_TOLERANCE,maximum_UV_delta=max(e[2] for e in errors),normal_tolerance=NORMAL_TOLERANCE,maximum_authored_raw_world_normal_delta=max(e[1] for e in errors),protected_original_triangles=len(metadata['protected_original_faces']),maximum_protected_normal_delta=max(protected_normal),packed_images=[image.name for image in images],winding='Every actual triangle matches authored cyclic order',**triangle_quality(actual,lookup))
    (folder/'validation/authored-export-contract.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))

for name in os.environ.get('WALL_NAMES',os.environ.get('WALL_NAME','Object01')).split(','):audit(name)
