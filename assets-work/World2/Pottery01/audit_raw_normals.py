"""Match authored corner normals against raw BMD normal-node world transforms."""
from pathlib import Path
import json
import struct
import sys
sys.dont_write_bytecode=True
import bpy
from mathutils import Vector,Matrix,Euler,kdtree
ROOT=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT))
from raw_audit import raw

def actual(folder):
    text=(folder/'validation/new'/f'{folder.name}.smd').read_text()
    nodes=[line.split('"')[1] for line in text.split('nodes\n')[1].split('\nend')[0].splitlines()]
    poses=[]
    for line in text.split('skeleton\n')[1].split('\nend')[0].splitlines()[1:]:
        row=list(map(float,line.split()));poses.append(Matrix.Translation(Vector(row[1:4]))@Euler(row[4:],'XYZ').to_matrix().to_4x4())
    data=raw.payload(folder/'exports'/f'{folder.name}.bmd');count=struct.unpack_from('<h',data,32)[0];cursor=38;result=[]
    for mesh in range(count):
        nv,nn,nu,nt,texture=struct.unpack_from('<5h',data,cursor);cursor+=10
        vertices=[struct.unpack_from('<h2x3f',data,cursor+i*16) for i in range(nv)];cursor+=nv*16
        normals=[struct.unpack_from('<h2x3fh2x',data,cursor+i*20) for i in range(nn)];cursor+=nn*20
        uvs=[struct.unpack_from('<2f',data,cursor+i*8) for i in range(nu)];cursor+=nu*8
        triangles=[]
        for index in range(nt):
            offset=cursor+index*64;vi=struct.unpack_from('<4h',data,offset+2)[:3];ni=struct.unpack_from('<4h',data,offset+10)[:3];ui=struct.unpack_from('<4h',data,offset+18)[:3]
            corners=[]
            for v,n,u in zip(vi,ni,ui):
                vertex=vertices[v];normal=normals[n];uv=uvs[u]
                corners.append((poses[vertex[0]]@Vector(vertex[1:4]),Vector((uv[0],1-uv[1])),nodes[vertex[0]],(poses[normal[0]].to_3x3()@Vector(normal[1:4])).normalized()))
            triangles.append(corners)
        cursor+=nt*64;material=data[cursor:cursor+32].split(b'\0')[0].decode();cursor+=32
        result.extend((material,corners) for corners in triangles)
    return result

def check(folder):
    exported=actual(folder);tree=kdtree.KDTree(len(exported))
    for i,(material,corners) in enumerate(exported):tree.insert(sum((c[0] for c in corners),Vector())/3,i)
    tree.balance();used=set();maximum=0;count=0
    bpy.ops.wm.open_mainfile(filepath=str(folder/'source.blend'));bpy.context.scene.frame_set(0)
    for obj in bpy.context.scene.objects:
        if obj.type!='MESH' or obj.get('mu_reference') or obj.get('mu_helper') or any(c.name.startswith('REF_') for c in obj.users_collection):continue
        normal_matrix=obj.matrix_world.to_3x3().inverted().transposed()
        for face in obj.data.polygons:
            material=obj.data.materials[face.material_index].get('mu_texture');corners=[]
            for loop in face.loop_indices:
                v=obj.data.vertices[obj.data.loops[loop].vertex_index]
                corners.append((obj.matrix_world@v.co,obj.data.uv_layers[0].data[loop].uv.copy(),obj.vertex_groups[v.groups[0].group].name,(normal_matrix@obj.data.corner_normals[loop].vector).normalized()))
            center=sum((c[0] for c in corners),Vector())/3;matches=[]
            for point,i,distance in tree.find_range(center,.0005):
                if i in used or exported[i][0]!=material:continue
                candidate=exported[i][1]
                for shift in range(3):
                    if all(corners[j][2]==candidate[(j+shift)%3][2] and (corners[j][0]-candidate[(j+shift)%3][0]).length<.0003 and (corners[j][1]-candidate[(j+shift)%3][1]).length<.000001 for j in range(3)):
                        error=max((corners[j][3]-candidate[(j+shift)%3][3]).length for j in range(3));matches.append((error,i))
            assert matches,(folder.name,face.index,'no raw triangle correspondence')
            error,i=min(matches);used.add(i);maximum=max(maximum,error);count+=1
    result=dict(status='PASS' if maximum<.001 and count==len(exported) else 'FAIL',triangles=count,maximum_world_normal_vector_distance=maximum,tolerance=.001,criterion='Every authored triangle corner normal compared with actual raw normal Node rotation, after exact material/bone/position/UV/cyclic-winding match')
    (folder/'validation/authored-raw-normal.json').write_text(json.dumps(result,indent=2))
    assert result['status']=='PASS',result
if __name__=='__main__':
    for name in ('Object29','Object28'):check(ROOT/name)
