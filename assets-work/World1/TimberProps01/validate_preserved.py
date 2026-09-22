"""Prove preserved display/glow positions, UVs and bone assignments in final BMD extracts."""
import json
import math
from pathlib import Path

ROOT=Path(__file__).resolve().parent
POSITION_TOLERANCE=.002
UV_TOLERANCE=.0000011
SPECS={'House02':('steel.jpg',147),'StreetLight01':('streetlight_brightness2.jpg',36)}


def triangles(path,material):
    lines=path.read_text().split('triangles\n')[1].splitlines()
    result=[]
    for index in range(0,len(lines)-1,4):
        if lines[index]==material:
            result.append([list(map(float,row.split())) for row in lines[index+1:index+4]])
    return result


def corner_error(old,new):
    if old[0]!=new[0]:
        return None
    distance=math.dist(old[1:4],new[1:4])
    uv=max(abs(a-b) for a,b in zip(old[7:9],new[7:9]))
    return (distance,uv) if distance<POSITION_TOLERANCE and uv<UV_TOLERANCE else None


def validate(name,material,count):
    root=ROOT/name
    original=triangles(root/f'validation/original/{name}.smd',material)
    replacement=triangles(root/f'validation/new/{name}.smd',material)
    assert len(original)==len(replacement)==count
    remaining=list(replacement); maximum=0
    for source in original:
        matched=None
        for index,candidate in enumerate(remaining):
            errors=[next((error for target in candidate if (error:=corner_error(corner,target)) is not None),None) for corner in source]
            if all(error is not None for error in errors):
                matched=index; maximum=max(maximum,*(error[0] for error in errors)); break
        assert matched is not None,(name,source)
        remaining.pop(matched)
    report=dict(status='PASS',material=material,triangles=count,
                positions_uv_and_one_bone_assignment_preserved=True,max_corner_distance=maximum,
                position_tolerance=POSITION_TOLERANCE,uv_tolerance=UV_TOLERANCE,
                normals='Recalculated by Blender export; normal equality is not claimed',
                renderer='StreetLight01 keeps original BlendMesh=1 material slot and 36 glow triangles' if name=='StreetLight01' else 'Static scenery blade/shield display remains in original material slot1')
    (root/'validation/preserved-geometry.json').write_text(json.dumps(report,indent=2)+'\n')
    print(name,report)


if __name__=='__main__':
    for name,(material,count) in SPECS.items():
        validate(name,material,count)
