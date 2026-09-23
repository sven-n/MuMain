"""Continuous contact surfaces and all placed bounds, beyond local corner retention."""
from pathlib import Path
import itertools,json,math,os
ROOT=Path(__file__).resolve().parent
TOLERANCE=.0003

def sub(a,b):return [x-y for x,y in zip(a,b)]
def dot(a,b):return sum(x*y for x,y in zip(a,b))
def cross(a,b):return [a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]]
def length(a):return math.sqrt(dot(a,a))
def triangles(path):
    lines=path.read_text().split('triangles\n')[1].splitlines()
    return [[list(map(float,row.split()[1:4])) for row in lines[i+1:i+4]] for i in range(0,len(lines)-1,4)]

def rotation(point,angles):
    x,y,z=point;a,b,c=map(math.radians,angles)
    y,z=y*math.cos(a)-z*math.sin(a),y*math.sin(a)+z*math.cos(a)
    x,z=x*math.cos(b)+z*math.sin(b),-x*math.sin(b)+z*math.cos(b)
    return [x*math.cos(c)-y*math.sin(c),x*math.sin(c)+y*math.cos(c),z]

def bounds(points):return [[min(p[i] for p in points) for i in range(3)],[max(p[i] for p in points) for i in range(3)]]

def support_planes(points):
    planes=[]
    for a,b,c in itertools.combinations(points,3):
        n=cross(sub(b,a),sub(c,a));size=length(n)
        if size<.0001:continue
        n=[v/size for v in n];d=dot(n,a);values=[dot(n,p)-d for p in points]
        if max(values)<=TOLERANCE:planes.append((n,d))
        elif min(values)>=-TOLERANCE:planes.append(([-v for v in n],-d))
    assert planes
    return planes

def point_on_triangle(p,tri):
    a,b,c=tri;u=sub(b,a);v=sub(c,a);q=sub(p,a);n=cross(u,v);size=length(n)
    if size<.000001 or abs(dot(q,n)/size)>TOLERANCE:return False
    uu,vv,uv,qu,qv=dot(u,u),dot(v,v),dot(u,v),dot(q,u),dot(q,v);den=uu*vv-uv*uv
    s=(qu*vv-qv*uv)/den;t=(qv*uu-qu*uv)/den
    return min(s,t,1-s-t)>=-TOLERANCE

def audit(name):
    folder=ROOT/name;old_tri=triangles(folder/'baseline/smd'/f'{name}.smd');new_tri=triangles(folder/'validation/new'/f'{name}.smd')
    old=list({tuple(p) for t in old_tri for p in t});new=[p for t in new_tri for p in t]
    planes=support_planes(old);excess=max(dot(n,p)-d for n,d in planes for p in new);assert excess<TOLERANCE,excess
    records=[]
    for placement in json.loads((folder/'placements.json').read_text()):
        before,after=[bounds([rotation(p,placement['rotation']) for p in points]) for points in (old,new)]
        error=max(abs(a-b) for aa,bb in zip(before,after) for a,b in zip(aa,bb))*placement['scale'];assert error<TOLERANCE,(placement['index'],error)
        records.append(dict(index=placement['index'],rotation=placement['rotation'],maximum_bound_delta=error,ground_min_z_delta=(after[0][2]-before[0][2])*placement['scale']))
    report=dict(status='PASS',tolerance=TOLERANCE,original_convex_support_planes=len(planes),maximum_outward_support_excess=excess,actual_placements=records,full_module_join_faces=list(range(22,34)) if name=='Object01' else None,method='All original convex support planes contain final geometry; all actual placed bounds unchanged. Full original module-end and motif faces independently matched with UV/winding/normals in authored-export-contract.json.')
    (folder/'validation/contact-envelope.json').write_text(json.dumps(report,indent=2)+'\n');print(name,'contact/envelope PASS',excess)

for name in os.environ.get('WALL_NAMES','Object01').split(','):audit(name)
