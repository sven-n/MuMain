"""Check raw normal-node sharing over every exported action key.

bmdconv SMD expansion uses the vertex bone for normals, while the runtime stores
normal-node indices separately. Sharing is safe only when the normal's direction
is unchanged by choosing either bone across all keys.
"""
import importlib.util
import json
import math
from pathlib import Path
import shlex
import sys
import numpy as np

sys.dont_write_bytecode=True
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[2]
HELPER=ROOT/'assets-work/World1/CartHay01/raw_bindings.py'
SPEC=importlib.util.spec_from_file_location('raw_bindings',HELPER)
RAW=importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(RAW)
TOLERANCE=.0001


def rotation(values):
    x,y,z=values
    cx,cy,cz=map(math.cos,(x,y,z));sx,sy,sz=map(math.sin,(x,y,z))
    return np.array(((cz*cy,cz*sy*sx-sz*cx,cz*sy*cx+sz*sx),
                     (sz*cy,sz*sy*sx+cz*cx,sz*sy*cx-cz*sx),(-sy,cy*sx,cy*cx)))


def poses(path):
    text=path.read_text()
    nodes=text.split('nodes\n')[1].split('\nend')[0].splitlines()
    parents={int(parts[0]):int(parts[2]) for parts in map(shlex.split,nodes)}
    frames=[]
    for line in text.split('skeleton\n')[1].split('\nend')[0].splitlines():
        if line.startswith('time '):
            frames.append({})
        else:
            parts=line.split();frames[-1][int(parts[0])]=rotation(tuple(map(float,parts[4:7])))
    result=[]
    for frame in frames:
        world={}
        def resolve(index):
            if index not in world:
                world[index]=frame[index] if parents[index]<0 else resolve(parents[index])@frame[index]
            return world[index]
        for index in frame:resolve(index)
        result.append(world)
    return result


def model(name,folder,game,stage=None):
    records=[]
    keys=[]
    smd = folder/'validation'/stage if stage else next((folder/'validation'/choice for choice in ('new','replacement') if (folder/'validation'/choice/f'{name}.smd').exists()))
    for path in [smd/f'{name}.smd',*sorted(smd.glob(f'{name}_a*.smd'))]:
        keys.extend(poses(path))
    for mesh in RAW.meshes(game):
        sharing=set()
        for vertices,normals in mesh['triangles']:
            for vi,ni in zip(vertices,normals):
                vnode=mesh['vertices'][vi][0];normal=mesh['normals'][ni];nnode=normal[0]
                if vnode!=nnode:sharing.add((vnode,nnode,*normal[1:4]))
        worst=0
        for vnode,nnode,*normal in sharing:
            if vnode<0 or nnode<0:
                worst=math.inf;continue
            vector=np.array(normal)
            worst=max(worst,*(float(np.max(np.abs((key[vnode]-key[nnode])@vector))) for key in keys))
        records.append(dict(material=mesh['material'],shared_normal_cases=len(sharing),max_direction_delta=worst,
                            result='PASS' if worst<TOLERANCE else 'REVIEW'))
    preserved = game.read_bytes() == (folder/'original'/f'{name}.bmd').read_bytes()
    safe = all(r['result']=='PASS' for r in records)
    return dict(model=name,frames_including_bind=len(keys),meshes=records,original_bytes_preserved=preserved,
                acceptance_basis='Normal direction invariant across every key' if safe else 'Exact original BMD retains pre-existing normal behavior' if preserved else 'Unresolved normal-node reinterpretation',
                result='PASS' if safe or preserved else 'REVIEW')


def main():
    latest={}
    for batch in json.loads((HERE/'integration-ledger.json').read_text()):
        for game,export in {**batch['game_files'],**batch.get('retained_game_files',{})}.items():
            if game.endswith('.bmd'):latest[game]=ROOT/Path(export).parent.parent
    reports=[model(Path(game).stem,folder,ROOT/game) for game,folder in latest.items()]
    result=dict(result='PASS' if all(r['result']=='PASS' for r in reports) else 'REVIEW',
                normal_direction_tolerance=TOLERANCE,models=reports,client_verified=False)
    (HERE/'normal-binding-audit.json').write_text(json.dumps(result,indent=2)+'\n')
    print(result['result'],len(reports),'models')
    for record in reports:
        if record['result']!='PASS':print(record)


if __name__=='__main__':main()
