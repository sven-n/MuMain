"""New surfaces must have finite UV area and consistent normals; document exact legacy faces."""
from pathlib import Path
import json
import math
ROOT=Path(__file__).resolve().parent

def sub(a,b):return [x-y for x,y in zip(a,b)]
def cross(a,b):return [a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]]
def triangles(path):
    lines=path.read_text().split('triangles\n')[1].splitlines()[:-1]
    return [(lines[i],[list(map(float,line.split())) for line in lines[i+1:i+4]]) for i in range(0,len(lines),4)]
def same(a,b):
    if a[0]!=b[0]:return False
    return any(all(x[0]==y[0] and math.dist(x[1:4],y[1:4])<.0003 and math.dist(x[7:9],y[7:9])<1e-6
                   for x,y in zip(a[1],b[1][shift:]+b[1][:shift])) for shift in range(3))
def inspect(name):
    f=ROOT/name;old=triangles(f/'validation/baseline'/f'{name}.smd');new=triangles(f/'validation/new'/f'{name}.smd')
    min_area=float('inf');legacy=[]
    for index,face in enumerate(new):
        rows=face[1];assert all(len(r)==9 and all(math.isfinite(v) for v in r) for r in rows)
        normal=cross(sub(rows[1][1:4],rows[0][1:4]),sub(rows[2][1:4],rows[0][1:4]));area=math.sqrt(sum(v*v for v in normal))/2
        assert area>1e-8,(name,index,area);min_area=min(area,min_area)
        u,v=sub(rows[1][7:9],rows[0][7:9]),sub(rows[2][7:9],rows[0][7:9]);uv_area=abs(u[0]*v[1]-u[1]*v[0])/2
        dots=[sum(a*b for a,b in zip(normal,row[4:7]))/(2*area) for row in rows]
        if uv_area<1e-10 or min(dots)<=0:
            matches=[i for i,previous in enumerate(old) if same(face,previous)]
            assert len(matches)==1,(name,index,uv_area,min(dots),matches)
            legacy.append(dict(triangle=index,baseline_triangle=matches[0],material=face[0],uv_area=uv_area,min_normal_dot=min(dots)))
    (f/'validation/uv-winding.json').write_text(json.dumps(dict(status='PASS',triangles=len(new),minimum_area=min_area,unchanged_legacy_faces=legacy),indent=2))
def main(names):
    for name in names:inspect(name)
if __name__=='__main__':main(('Object04',))
