"""Adaptive planar brow and socket patches; the cubic study is archived."""
import math
from collections import defaultdict
from mathutils import Vector
from mathutils.geometry import delaunay_2d_cdt
RELIEF_RANGE = tuple(range(16,22))+tuple(range(40,50))+tuple(range(54,58))+tuple(range(62,66))
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


def barycentric(point, corners):
    a,b,c = [Vector((p.x,p.z)) for p in corners]
    ab,ac,ap = b-a,c-a,point-a
    determinant = ab.x*ac.y-ab.y*ac.x
    second = (ap.x*ac.y-ap.y*ac.x)/determinant
    third = (ab.x*ap.y-ab.y*ap.x)/determinant
    return (1-second-third,second,third)


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
    # One connected arch, with a broad crest and deliberately planar slopes.
    ridge_z = 254-.18*abs(x)
    ridge = max(0,min(1,(13-abs(z-ridge_z))/10))
    side_fade = max(0,min(1,(39-abs(x))/8))
    brow = -14*ridge*side_fade
    socket = 0
    for center in (-23,23):
        rake = .15*(abs(x)-23)
        diamond = abs(x-center)/11+abs(z-(233+rake))/10
        socket += 15*max(0,min(1,(1.45-diamond)))
    blend = min(1,seam_distance/7)
    return (brow+socket)*blend


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
        target=9 if (a.z+b.z)/2>208 else 20
        count=max(1,math.ceil(Vector((b.x-a.x,b.z-a.z)).length/target))
        boundary.extend([Vector((a.x,a.z)).lerp(Vector((b.x,b.z)),i/count) for i in range(count)])
    vertices=list(boundary)
    center=sum((Vector((p.x,p.z)) for p in corners),Vector((0,0)))/3
    vertices.append(center)
    # Seed the actual ridge and angular eye-ring landmarks, rather than uniformly
    # increasing the old polygon count.
    features = [(x,254-.18*abs(x)+dz) for x in range(-32,33,8) for dz in (-13,-3,3,13)]
    for cx in (-23,23):
        for radius in (.45,1.0,1.45):
            for side in range(4):
                diamond = [(cx+11*radius,233),(cx,233+10*radius),
                           (cx-11*radius,233),(cx,233-10*radius)]
                start,end = Vector(diamond[side]),Vector(diamond[(side+1)%4])
                features.extend(tuple(start.lerp(end,t)) for t in (0,.5))
    for feature in features:
        point=Vector(feature)
        if min(barycentric(point,corners))>1e-5:
            if all((point-old).length>.001 for old in vertices):vertices.append(point)
    edges=[(i,(i+1)%len(boundary)) for i in range(len(boundary))]
    output=delaunay_2d_cdt(vertices,edges,[list(range(len(boundary)))],1,1e-6)
    return output[0],output[2]


def sculpt(rows, planes, anchors):
    points,faces,boundary=topology(rows)
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
                point.y+=anatomy(point,seam_distance)
                point.y=min(point.y,-51.0)
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
