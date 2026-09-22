"""Compare actual exported rigid bounds at every original action key."""
import json
import re
import sys
sys.dont_write_bytecode=True
import numpy as np
from config import ASSETS,ROOT
from audit_bindings import rotation


def poses(path):
    text=path.read_text();nodes=text.split('nodes\n')[1].split('\nend')[0].splitlines()
    parents=[int(re.search(r'(-?\d+)$',line)[1]) for line in nodes]
    result=[]
    for line in text.split('skeleton\n')[1].split('\nend')[0].splitlines():
        if line.startswith('time '):result.append([]);continue
        values=list(map(float,line.split()));index=int(values[0]);matrix=np.eye(4)
        matrix[:3,:3]=rotation(values[4:]);matrix[:3,3]=values[1:4]
        if parents[index]>=0:matrix=result[-1][parents[index]]@matrix
        result[-1].append(matrix)
    return result


def posed_bounds(folder,name):
    bind=poses(folder/(name+'.smd'))[0]
    action=poses(folder/(name+'_a00.smd'))
    lines=(folder/(name+'.smd')).read_text().split('triangles\n')[1].splitlines()[:-1]
    vertices=[list(map(float,lines[i].split()[:4])) for i in range(len(lines)) if i%4]
    grouped={bone:np.array([row[1:]+[1] for row in vertices if int(row[0])==bone]).T for bone in set(int(row[0]) for row in vertices)}
    inverse=[np.linalg.inv(matrix) for matrix in bind];bounds=[]
    for pose in action:
        points=np.concatenate([(pose[bone]@inverse[bone]@array)[:3] for bone,array in grouped.items()],axis=1)
        bounds.append([points.min(axis=1).tolist(),points.max(axis=1).tolist()])
    return bounds


for name in ASSETS:
    folder=ROOT/name/'validation';before=posed_bounds(folder/'original',name);after=posed_bounds(folder/'new',name)
    difference=float(np.abs(np.array(before)-np.array(after)).max());assert difference<.005,(name,difference)
    (folder/'all-key-posed-bounds.json').write_text(json.dumps(dict(status='PASS',keys=len(before),max_bounds_delta=difference,original_bounds=before,replacement_bounds=after),indent=2)+'\n')
    print(name,len(before),difference)
