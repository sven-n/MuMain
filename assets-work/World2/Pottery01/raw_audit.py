"""Assess raw normal node sharing by its actual world direction at every key."""
from pathlib import Path
import importlib.util
import json
import math
import sys
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('raw',ROOT.parents[1]/'World1/Architecture03/raw_bindings.py')
raw=importlib.util.module_from_spec(spec);spec.loader.exec_module(raw)
def rotation(values):
    x,y,z=values;cx,cy,cz=math.cos(x),math.cos(y),math.cos(z);sx,sy,sz=math.sin(x),math.sin(y),math.sin(z)
    return ((cz*cy,cz*sy*sx-sz*cx,cz*sy*cx+sz*sx),(sz*cy,sz*sy*sx+cz*cx,sz*sy*cx-cz*sx),(-sy,cy*sx,cy*cx))
def mul(a,b):return [[sum(a[i][k]*b[k][j] for k in range(3)) for j in range(3)] for i in range(3)]
def apply(a,v):return [sum(a[i][k]*v[k] for k in range(3)) for i in range(3)]
def poses(path):
    text=path.read_text();parents=[int(line.rsplit(' ',1)[1]) for line in text.split('nodes\n')[1].split('\nend')[0].splitlines()]
    result=[]
    for line in text.split('skeleton\n')[1].split('\nend')[0].splitlines():
        if line.startswith('time '):result.append([]);continue
        row=list(map(float,line.split()));index=int(row[0]);matrix=rotation(row[4:])
        if parents[index]>=0:matrix=mul(result[-1][parents[index]],matrix)
        result[-1].append(matrix)
    return result

def check(folder,stage):
    source='exports' if stage=='new' else stage
    frames=poses(folder/'validation'/stage/(folder.name+'_a00.smd'));records={}
    for mesh in raw.meshes(folder/source/(folder.name+'.bmd')):
        for vi,ni in mesh['triangles']:
            for vertex,normal in zip(vi,ni):
                vbone=mesh['vertices'][vertex][0];n=mesh['normals'][normal];nbone=n[0]
                if vbone==nbone:continue
                errors=[math.dist(apply(frame[vbone],n[1:4]),apply(frame[nbone],n[1:4])) for frame in frames]
                key=f'{vbone}:{nbone}';entry=records.setdefault(key,dict(corners=0,max_world_normal_direction_error=0))
                entry['corners']+=1;entry['max_world_normal_direction_error']=max(entry['max_world_normal_direction_error'],*errors)
    result=dict(stage=stage,keys=len(frames),normal_aliases=records,criterion='Normal world direction differs by less than1e-5 at every action key',status='PASS' if all(x['max_world_normal_direction_error']<1e-5 for x in records.values()) else 'FAIL')
    (folder/'validation'/f'raw-world-{stage}.json').write_text(json.dumps(result,indent=2))
    return result
if __name__=='__main__':
    for name in ('Object28','Object29','Object30'):print(name,check(ROOT/name,'baseline'))
