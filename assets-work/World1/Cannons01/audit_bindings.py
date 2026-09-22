"""Prove actual BMD normal nodes rotate with their vertices at every original key."""
import json
import math
import re
import sys
sys.dont_write_bytecode=True
import numpy as np
from config import ASSETS,ROOT
from raw_bindings import meshes

ROTATION_TOLERANCE=.00001


def rotation(values):
    x,y,z=values
    cx,cy,cz=math.cos(x),math.cos(y),math.cos(z)
    sx,sy,sz=math.sin(x),math.sin(y),math.sin(z)
    return np.array(((cz,-sz,0),(sz,cz,0),(0,0,1))) @ np.array(((cy,0,sy),(0,1,0),(-sy,0,cy))) @ np.array(((1,0,0),(0,cx,-sx),(0,sx,cx)))


def frames(path):
    text=path.read_text()
    nodes=text.split('nodes\n')[1].split('\nend')[0].splitlines()
    parents=[int(re.search(r'(-?\d+)$',line)[1]) for line in nodes]
    rows=text.split('skeleton\n')[1].split('\nend')[0].splitlines()
    result=[]
    for line in rows:
        if line.startswith('time '):
            result.append([])
            continue
        row=list(map(float,line.split()))
        index=int(row[0]);matrix=rotation(row[4:])
        if parents[index]>=0:
            matrix=result[-1][parents[index]] @ matrix
        result[-1].append(matrix)
    return result


def audit(name):
    folder=ROOT/name
    result=[]
    for stage in ('original','new'):
        path=folder/(f'original/{name}.bmd' if stage=='original' else f'exports/{name}.bmd')
        poses=frames(folder/f'validation/{stage}/{name}_a00.smd')
        pairs=set()
        for mesh in meshes(path):
            for vertices,normals in mesh['triangles']:
                for vi,ni in zip(vertices,normals):
                    pair=(mesh['vertices'][vi][0],mesh['normals'][ni][0])
                    if pair[0]!=pair[1]:pairs.add(pair)
        deltas={f'{a}:{b}':max(float(np.abs(frame[a]-frame[b]).max()) for frame in poses) for a,b in pairs}
        if stage=='new':
            assert all(value<ROTATION_TOLERANCE for value in deltas.values()),(name,stage,deltas)
        result.append(dict(stage=stage,keys=len(poses),raw_vertex_normal_node_pair_rotation_delta=deltas))
    (folder/'validation/raw-normal-bindings.json').write_text(json.dumps(dict(status='PASS',criterion='Actual raw normal node has equivalent world rotation to vertex bone for every original action key',stages=result),indent=2)+'\n')
    print(name,result)


if __name__=='__main__':
    for name in ASSETS:audit(name)
