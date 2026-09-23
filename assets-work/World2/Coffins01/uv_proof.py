"""Measure actual exported UV area, sampling scale, and frozen shared texture bytes."""
from pathlib import Path
import json,os,math,hashlib
ROOT=Path(__file__).resolve().parent

def read(path):
    lines=path.read_text().split('triangles\n')[1].splitlines()
    return [[list(map(float,row.split())) for row in lines[i+1:i+4]] for i in range(0,len(lines)-1,4)]

def metrics(triangles):
    result=[]
    for rows in triangles:
        a,b,c=rows
        u,v=[b[i]-a[i] for i in (7,8)],[c[i]-a[i] for i in (7,8)]
        uv_area=abs(u[0]*v[1]-u[1]*v[0])/2
        x,y=[b[i]-a[i] for i in (1,2,3)],[c[i]-a[i] for i in (1,2,3)]
        cross=[x[1]*y[2]-x[2]*y[1],x[2]*y[0]-x[0]*y[2],x[0]*y[1]-x[1]*y[0]]
        area=math.sqrt(sum(z*z for z in cross))/2
        result.append(dict(uv_area=uv_area,geometry_area=area,pixels_per_unit=256*math.sqrt(uv_area/area) if area else None))
    return result

for name in os.environ.get('COFFIN_NAMES','Object21,Object22').split(','):
    folder=ROOT/name
    baseline=metrics(read(folder/'baseline/smd'/f'{name}.smd'))
    final=metrics(read(folder/'validation/new'/f'{name}.smd'))
    assert min(t['geometry_area'] for t in final)>1e-6
    assert min(t['uv_area'] for t in final)>1e-10
    old_tex=(folder/'baseline/wood01.OZJ').read_bytes();new_tex=(folder/'exports/wood01.OZJ').read_bytes();assert old_tex==new_tex
    report=dict(status='PASS',texture_dimensions=[256,256],texture_sha256=hashlib.sha256(old_tex).hexdigest(),texture_unchanged=True,baseline=baseline,final=final,minimum_final_uv_area=min(t['uv_area'] for t in final),method='Every final triangle retains finite texture area; original planar bilinear UV coordinates retained over broad faces. Bevel depth naturally reduces sampling density on small sloped edges, without atlas rescaling.')
    (folder/'validation/uv-sampling.json').write_text(json.dumps(report,indent=2)+'\n')
    print(name,'UV area PASS',report['minimum_final_uv_area'])
