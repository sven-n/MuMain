"""Dense tied straw bundles fitted to the original rigid parts and their orientation."""
import math
import numpy as np
from mathutils import Vector
from model_geometry import fit, shared

SEGMENTS = 12
# Longitudinal cuts put two pinched waists under the painted twine bands.
RINGS = ((0,1),(.31,.89),(.35,.89),(.65,.89),(.69,.89),(1,1))


def basis(part):
    points=np.array(list(part['vertices'].values()))
    center=points.mean(axis=0)
    _,vectors=np.linalg.eigh(np.cov((points-center).T))
    axis=Vector(vectors[:,-1])
    if axis.y<0:
        axis=-axis
    side=axis.cross(Vector((0,0,1))).normalized()
    up=side.cross(axis).normalized()
    length=max(Vector(p).dot(axis) for p in points)-min(Vector(p).dot(axis) for p in points)
    radius=max(abs((Vector(p)-Vector(center)).dot(up)) for p in points)
    return Vector(center),axis,side,up,length,radius


def bundle(part,context):
    center,axis,side,up,length,radius=basis(part)
    variant=int(part['bone'][-2:])
    vertices=[]
    for progress,scale in RINGS:
        for corner in range(SEGMENTS):
            angle=math.tau*corner/SEGMENTS
            radial=side*math.cos(angle)+up*math.sin(angle)
            vertices.append(center+axis*((progress-.5)*length)+radial*(radius*scale*(1-.002*((corner+variant)%5))))
    faces,uvs=[],[]
    for ring in range(len(RINGS)-1):
        for corner in range(SEGMENTS):
            nxt=(corner+1)%SEGMENTS
            faces.append((ring*SEGMENTS+corner,ring*SEGMENTS+nxt,(ring+1)*SEGMENTS+nxt,(ring+1)*SEGMENTS+corner))
            uvs.append(((corner/SEGMENTS*.663,RINGS[ring][0]),((corner+1)/SEGMENTS*.663,RINGS[ring][0]),
                        ((corner+1)/SEGMENTS*.663,RINGS[ring+1][0]),(corner/SEGMENTS*.663,RINGS[ring+1][0])))
    for ring in (0,len(RINGS)-1):
        cap=len(vertices)
        vertices.append(center+axis*((RINGS[ring][0]-.5)*length)+side*(variant*.002)+up*(variant*.003))
        for corner in range(SEGMENTS):
            nxt=(corner+1)%SEGMENTS
            faces.append((cap,ring*SEGMENTS+nxt,ring*SEGMENTS+corner))
            coordinates=[(.834,.75)]
            for index in (nxt,corner):
                angle=math.tau*index/SEGMENTS
                coordinates.append((.834+.15*math.cos(angle),.75+.23*math.sin(angle)))
            uvs.append(coordinates)
    obj=shared.mesh(part['bone']+'_TiedStraw',vertices,faces,uvs,part['bone'],context,True)
    fit([obj],part['bounds'])
    return [obj]
