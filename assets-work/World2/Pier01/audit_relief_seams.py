"""Prove the exported open relief boundary remains on its original registered seam."""
from pathlib import Path
from collections import defaultdict
import itertools
import json
import math
import sys
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parent
from audit_exports import triangles,same
TOLERANCE=.0003


def mesh_edges(faces):
    points=[];buckets=defaultdict(list);edges=defaultdict(int)
    for material,rows in faces:
        indices=[]
        for row in rows:
            point=row[1:4];cell=tuple(math.floor(x/.001) for x in point)
            nearby=(i for offset in itertools.product((-1,0,1),repeat=3)
                    for i in buckets[tuple(a+b for a,b in zip(cell,offset))])
            index=next((i for i in nearby if math.dist(point,points[i])<.0001),None)
            if index is None:
                index=len(points);points.append(point);buckets[cell].append(index)
            indices.append(index)
        for a,b in zip(indices,indices[1:]+indices[:1]):
            assert a!=b
            edges[tuple(sorted((a,b)))]+=1
    assert max(edges.values())<=2
    return points,[edge for edge,count in edges.items() if count==1]


def point_segment(point,start,end):
    edge=[b-a for a,b in zip(start,end)]
    t=sum((p-a)*d for p,a,d in zip(point,start,edge))/sum(d*d for d in edge)
    t=min(1,max(0,t))
    closest=[a+t*d for a,d in zip(start,edge)]
    return math.dist(point,closest),t


def main():
    folder=ROOT/'Object04'
    baseline=triangles(folder/'validation/baseline/Object04.smd')
    actual=triangles(folder/'validation/new/Object04.smd')
    protected=json.loads((folder/'validation/source.json').read_text())['protected_triangles']
    relief=[face for face in actual if not any(same(face,baseline[i]) for i in protected)]
    old_points,old_edges=mesh_edges([face for index,face in enumerate(baseline) if index not in protected]);points,edges=mesh_edges(relief)
    spans=defaultdict(list);maximum=0;maximum_uv=0
    original_relief=[face for index,face in enumerate(baseline) if index not in protected]
    def point_uv(point,faces):
        values=[row[7:9] for _,rows in faces for row in rows if math.dist(row[1:4],point)<.0001]
        assert values
        assert all(math.dist(values[0],value)<1e-6 for value in values)
        return values[0]
    for _,rows in relief:
        assert all(row[0]==0 for row in rows)
        a,b,c=[row[1:4] for row in rows]
        assert (b[0]-a[0])*(c[2]-a[2])-(b[2]-a[2])*(c[0]-a[0])>0, 'Projected winding changed' 
    for a,b in edges:
        matches=[]
        for index,(oa,ob) in enumerate(old_edges):
            da,ta=point_segment(points[a],old_points[oa],old_points[ob])
            db,tb=point_segment(points[b],old_points[oa],old_points[ob])
            if max(da,db)<TOLERANCE:
                matches.append((index,min(ta,tb),max(ta,tb),max(da,db)))
        assert len(matches)==1,(a,b,matches)
        index,start,end,error=matches[0];spans[index].append((start,end));maximum=max(maximum,error)
        oa,ob=old_edges[index];ua=point_uv(old_points[oa],original_relief);ub=point_uv(old_points[ob],original_relief)
        for vertex in (a,b):
            _,factor=point_segment(points[vertex],old_points[oa],old_points[ob])
            expected=[x+(y-x)*factor for x,y in zip(ua,ub)]
            maximum_uv=max(maximum_uv,math.dist(expected,point_uv(points[vertex],relief)))
    assert maximum_uv<1e-6,maximum_uv
    for index in range(len(old_edges)):
        cursor=0
        for start,end in sorted(spans[index]):
            assert abs(start-cursor)<1e-5,(index,start,cursor)
            cursor=end
        assert abs(cursor-1)<1e-5,(index,cursor)
    report=dict(status='PASS',baseline_boundary_segments=len(old_edges),exported_boundary_segments=len(edges),
                maximum_registered_seam_position_error=maximum,maximum_registered_seam_uv_error=maximum_uv,
                binding_and_projected_winding="EXACT",
                criterion='Each new open boundary edge lies on one original seam segment; non-overlapping intervals cover each complete original segment')
    (folder/'validation/relief-seams.json').write_text(json.dumps(report,indent=2))


if __name__=='__main__':main()
