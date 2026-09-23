"""Independently recover baseline stone UV projection on every changed final corner."""
from pathlib import Path
import json,math
ROOT=Path(__file__).resolve().parent;FOLDER=ROOT/'Object01'

def read(path):
    lines=path.read_text().split('triangles\n')[1].splitlines()
    return [(lines[i],[list(map(float,r.split())) for r in lines[i+1:i+4]]) for i in range(0,len(lines)-1,4)]

def matches(a,b):
    return any(max(abs(x[k]-y[k]) for x,y in zip(a,b[shift:]+b[:shift]) for k in (1,2,3,7,8))<.000003 for shift in range(3))

def interpolate(row,patch,axes):
    p=[row[k] for k in axes]
    for _,rows in patch:
        a,b,c=[[r[k] for k in axes] for r in rows];u=[b[k]-a[k] for k in (0,1)];v=[c[k]-a[k] for k in (0,1)];q=[p[k]-a[k] for k in (0,1)];den=u[0]*v[1]-u[1]*v[0]
        if abs(den)<1e-10:continue
        s=(q[0]*v[1]-q[1]*v[0])/den;t=(u[0]*q[1]-u[1]*q[0])/den
        if min(s,t,1-s-t)<-.00001:continue
        return [rows[0][k]*(1-s-t)+rows[1][k]*s+rows[2][k]*t for k in (7,8)]
    raise AssertionError(('Outside baseline projection',row,axes))

old=read(FOLDER/'baseline/smd/Object01.smd');new=read(FOLDER/'validation/new/Object01.smd');meta=json.loads((FOLDER/'validation/authored.json').read_text());remaining=new.copy()
for index in meta['protected_original_faces']:
    material,rows=old[index];match=next(i for i,(m,r) in enumerate(remaining) if material==m and matches(rows,r));remaining.pop(match)
front_a=old[2][1][0];back_a=old[2][1][2]
def top_height(y):return front_a[3]+(back_a[3]-front_a[3])*(y-front_a[2])/(back_a[2]-front_a[2])
errors=[];classified={'top':0,'front':0,'new_plain_stone':0};scale_errors=[]
DENSITY=(1.1062+.1062)/167.0
for material,rows in remaining:
    assert material=='deep_wall01.jpg'
    top=all(abs(r[3]-top_height(r[2]))<.00003 for r in rows)
    lower=all(r[3]<=273.50003 for r in rows)
    if top or lower:
        axes=(1,2) if top else (1,3);patch=[old[i] for i in ((2,3) if top else (18,19))];classified['top' if top else 'front']+=1
        for row in rows:
            uv=interpolate(row,patch,axes);errors.extend(abs(a-b) for a,b in zip(uv,row[7:9]))
        continue
    classified['new_plain_stone']+=1
    assert all(.429999<=r[8]<=.71 for r in rows)
    for a,b in zip(rows,rows[1:]+rows[:1]):
        distance=math.sqrt(sum((a[k]-b[k])**2 for k in (1,2,3)))
        uv_distance=math.sqrt(sum((a[k]-b[k])**2 for k in (7,8)))
        scale_errors.append(abs(uv_distance/distance-DENSITY))
assert errors and max(errors)<.000001,max(errors)
assert scale_errors and max(scale_errors)<.0000001,max(scale_errors)
report=dict(status='PASS',maximum_original_UV_projection_delta=max(errors),original_projection_tolerance=.000001,changed_triangles=len(remaining),classified=classified,new_plain_stone_UV_per_world_unit=DENSITY,maximum_new_stone_density_delta=max(scale_errors),new_stone_density_tolerance=.0000001,method='Original lower ornament and top corners recover original affine mapping. Every new upper stone edge independently has uniform physical UV density in the existing plain stone V .43-.71 patch; no texture edits.')
(FOLDER/'validation/affine-UV.json').write_text(json.dumps(report,indent=2)+'\n');print(report)
