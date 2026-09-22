"""Read BMD mesh bindings directly; do not infer normal nodes from SMD expansion.

Read-only audit matching ZzzBMD.h disk structures and MapFileDecrypt. Usage:
python -B raw_bindings.py model.bmd [another.bmd ...]
A normal/vertex node mismatch requires comparing both bones' rotations for every key;
shared normals on bones with identical world rotations can be harmless.
"""
import json
from pathlib import Path
import struct
import sys
sys.dont_write_bytecode = True

XOR_KEY=bytes.fromhex('D1 73 52 F6 D2 9A CB 27 3E AF 59 31 37 B3 E7 A2')
VERTEX_SIZE=16
NORMAL_SIZE=20
UV_SIZE=8
TRIANGLE_SIZE=64


def payload(path):
    data=Path(path).read_bytes()
    assert data[:3]==b'BMD'
    if data[3]==10:
        return data[4:]
    assert data[3]==12, 'Only official plain/encrypted BMD versions are supported'
    length=struct.unpack_from('<I',data,4)[0]
    source=data[8:]
    assert len(source)==length
    key=0x5E
    result=bytearray()
    for index,value in enumerate(source):
        result.append(((value ^ XOR_KEY[index%16])-key)&255)
        key=(value+0x3D)&255
    return bytes(result)


def meshes(path):
    data=payload(path)
    count,bones,actions=struct.unpack_from('<3h',data,32)
    cursor=38
    result=[]
    for index in range(count):
        nv,nn,nu,nt,texture=struct.unpack_from('<5h',data,cursor)
        cursor+=10
        vertices=[struct.unpack_from('<h2x3f',data,cursor+i*VERTEX_SIZE) for i in range(nv)]
        cursor+=nv*VERTEX_SIZE
        normals=[struct.unpack_from('<h2x3fh2x',data,cursor+i*NORMAL_SIZE) for i in range(nn)]
        cursor+=nn*NORMAL_SIZE
        cursor+=nu*UV_SIZE
        triangles=[]
        for i in range(nt):
            offset=cursor+i*TRIANGLE_SIZE
            assert data[offset]==3
            v=struct.unpack_from('<4h',data,offset+2)[:3]
            n=struct.unpack_from('<4h',data,offset+10)[:3]
            assert max(v)<nv and max(n)<nn
            triangles.append((v,n))
        cursor+=nt*TRIANGLE_SIZE
        material=data[cursor:cursor+32].split(b'\0')[0].decode('ascii')
        cursor+=32
        result.append(dict(index=index,material=material,vertices=vertices,normals=normals,triangles=triangles))
    return result


def audit(path):
    records=[]
    for mesh in meshes(path):
        pairs={}
        for vi,ni in mesh['triangles']:
            for vertex,normal in zip(vi,ni):
                vnode=mesh['vertices'][vertex][0]
                nnode=mesh['normals'][normal][0]
                if vnode!=nnode:
                    key=f'{vnode}:{nnode}'
                    pairs[key]=pairs.get(key,0)+1
        records.append(dict(mesh=mesh['index'],material=mesh['material'],vertex_to_normal_node_mismatch_corner_counts=pairs))
    return dict(file=str(path),mesh_bindings=records)


if __name__=='__main__':
    print(json.dumps([audit(p) for p in sys.argv[1:]],indent=2))
