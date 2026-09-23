"""Adaptive cubic relief patches with broad anatomical stone planes."""
import math
from collections import defaultdict
from mathutils import Vector
from mathutils.geometry import delaunay_2d_cdt
RELIEF_RANGE = range(10,82)
PLANE_MARGIN = 1e-6


def topology(rows):
    points, faces, lookup = [], [], {}
    for index in RELIEF_RANGE:
        polygon = []
        for row in rows[index][1]:
            key = tuple(row[1:4])
            if key not in lookup:
                lookup[key] = len(points)
                points.append(Vector(key))
            polygon.append(lookup[key])
        faces.append(polygon)
    edges = defaultdict(list)
    for index, face in enumerate(faces):
        for a,b in zip(face,face[1:]+face[:1]):
            edges[tuple(sorted((a,b)))].append(index)
    boundary = {edge for edge, faces in edges.items() if len(faces)==1}
    return points, faces, boundary


def gradients(points, faces):
    sums = [Vector((0,0)) for _ in points]
    weights = [0.0 for _ in points]
    for face in faces:
        a,b,c = [points[i] for i in face]
        normal = (b-a).cross(c-a)
        weight = abs(normal.y)
        assert weight > 1e-7
        gradient = Vector((-normal.x/normal.y,-normal.z/normal.y))
        if gradient.length > 3:
            gradient *= 3/gradient.length
        for index in face:
            sums[index] += gradient*weight
            weights[index] += weight
    return [gradient/weight for gradient,weight in zip(sums,weights)]


def barycentric(point, corners):
    a,b,c = [Vector((p.x,p.z)) for p in corners]
    ab,ac,ap = b-a,c-a,point-a
    determinant = ab.x*ac.y-ab.y*ac.x
    second = (ap.x*ac.y-ap.y*ac.x)/determinant
    third = (ab.x*ap.y-ab.y*ap.x)/determinant
    return (1-second-third,second,third)


def cubic_height(weights, indices, points, derivatives, boundary):
    corners = [points[i] for i in indices]
    controls = {}
    for i in range(3):
        for j in range(3):
            if i==j:
                continue
            vector = Vector((corners[j].x-corners[i].x,corners[j].z-corners[i].z))
            slope = derivatives[indices[i]].dot(vector)
            if tuple(sorted((indices[i],indices[j]))) in boundary:
                slope = corners[j].y-corners[i].y
            controls[i,j] = corners[i].y+slope/3
    edge_mean = sum(controls.values())/6
    center = edge_mean+(edge_mean-sum(p.y for p in corners)/3)/2
    value = sum(w**3*p.y for w,p in zip(weights,corners))
    value += sum(3*weights[i]**2*weights[j]*h for (i,j),h in controls.items())
    value += 6*math.prod(weights)*center
    active = [p for w,p in zip(weights,corners) if w > 1e-7]
    return min(max(value,min(p.y for p in active)-2),max(p.y for p in active)+2)


def distance_to_boundary(point, points, boundary):
    distance = float('inf')
    for a,b in boundary:
        start,end = Vector((points[a].x,points[a].z)),Vector((points[b].x,points[b].z))
        edge = end-start
        factor = min(1,max(0,(point-start).dot(edge)/edge.length_squared))
        distance = min(distance,(point-start-edge*factor).length)
    return distance


def anatomy(point, seam_distance):
    x,z = point.x+.29,point.z
    def mound(cx,cz,sx,sz):
        return math.exp(-(((x-cx)/sx)**2+((z-cz)/sz)**2)/2)
    # Wide connected brow and cheek forms, not isolated polygon shelves.
    brows = 5.0*(mound(-19,240,10,12)+mound(19,240,10,12))
    sockets = -2.0*(mound(-24,220,9,11)+mound(24,220,9,11))
    cheeks = 4.0*(mound(-24,176,10,22)+mound(24,176,10,22))
    chin = 2.0*mound(0,153,13,20)
    blend = min(1,seam_distance/12)
    blend = blend*blend*(3-2*blend)
    return -(brows+sockets+cheeks+chin)*blend


def depth_constraint(point, planes):
    minimum,maximum = -float('inf'),float('inf')
    for normal,limit in planes:
        if abs(normal.y)<1e-8:
            assert normal.dot(point)<=limit+.0001
            continue
        bound = (limit-normal.x*point.x-normal.z*point.z)/normal.y
        if normal.y>0:
            maximum=min(maximum,bound)
        else:
            minimum=max(minimum,bound)
    assert minimum<=maximum+.0001
    point.y=min(max(point.y,minimum),maximum)
    return point


def patch_points(indices, points):
    corners=[points[i] for i in indices]
    boundary=[]
    for a,b in zip(corners,corners[1:]+corners[:1]):
        # The expressive upper mask gets shorter spans than the long lower carving.
        target=12 if (a.z+b.z)/2>185 else 22
        count=max(1,math.ceil(Vector((b.x-a.x,b.z-a.z)).length/target))
        boundary.extend([Vector((a.x,a.z)).lerp(Vector((b.x,b.z)),i/count) for i in range(count)])
    vertices=list(boundary)
    center=sum((Vector((p.x,p.z)) for p in corners),Vector((0,0)))/3
    vertices.append(center)
    edges=[(i,(i+1)%len(boundary)) for i in range(len(boundary))]
    output=delaunay_2d_cdt(vertices,edges,[list(range(len(boundary)))],1,1e-6)
    return output[0],output[2]


def sculpt(rows, planes, anchors):
    points,faces,boundary=topology(rows)
    derivatives=gradients(points,faces)
    result=[]
    for old_index,indices in zip(RELIEF_RANGE,faces):
        corners=[points[i] for i in indices]
        coordinates,triangles=patch_points(indices,points)
        values=[]
        for coordinate in coordinates:
            weights=barycentric(coordinate,corners)
            point=sum((w*p for w,p in zip(weights,corners)),Vector())
            uv=sum((w*Vector(row[7:9]) for w,row in zip(weights,rows[old_index][1])),Vector((0,0)))
            seam_distance=distance_to_boundary(coordinate,points,boundary)
            is_anchor=any((point-anchor).length<.0001 for anchor in anchors)
            if not is_anchor and seam_distance>1e-5:
                point.y=cubic_height(weights,indices,points,derivatives,boundary)
                point.y+=anatomy(point,seam_distance)
                point=depth_constraint(point,planes)
            values.append((point,uv))
        for triangle in triangles:
            assert len(triangle)==3
            corners=[values[i] for i in triangle]
            normal=(corners[1][0]-corners[0][0]).cross(corners[2][0]-corners[0][0])
            if normal.y>0:
                corners.reverse()
            result.append((1,corners,None))
    return result
