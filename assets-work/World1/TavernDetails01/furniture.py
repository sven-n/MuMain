"""Measured shelf joinery and seating furniture; original seat planes remain exact."""
import math

from model_geometry import FULL, box, planar_uv, preserve_faces, shared


def ordered_chunks(part, count=8):
    vertices = part['vertices']
    keys = sorted(map(int, vertices))
    return [[vertices[str(k)] for k in keys[i:i+count]] for i in range(0,len(keys),count)]


def extremes(points):
    return [[min(p[a] for p in points) for a in range(3)],
            [max(p[a] for p in points) for a in range(3)]]


def shelf(part, context):
    objects = []
    for index, points in enumerate(ordered_chunks(part)):
        low, high = extremes(points)
        vertical = high[2]-low[2] > high[0]-low[0]
        objects.append(box(f'ShelfBoard_{index}',low,high,part['bone'],context,
                           region='panel' if vertical else 'wood', top='top',
                           bevel=0 if vertical else .35))
        if not vertical:
            rail_low = (low[0],high[1]-3,low[2]-5)
            rail_high = (high[0],high[1],low[2])
            objects.append(box(f'ShelfRearRail_{index}',rail_low,rail_high,part['bone'],context))
    return objects


def chair(part, source, context):
    objects = []
    bone = part['bone']
    for index, points in enumerate(ordered_chunks(part)):
        low, high = extremes(points)
        if index == 4:
            ids = set(range(32,40))
            faces = [p for p in source.data.polygons if set(p.vertices).issubset(ids)]
            objects.append(preserve_faces('ExactOriginalSeat',source,faces,bone,context,'wood'))
            continue
        objects.append(box(f'ChairTimber_{index}',low,high,bone,context))
    # Aprons attach below the original platform and stay inside its measured edges.
    for x in (-15.9,16.4):
        objects.append(box(f'SideApron_{x}',(x-2,-16,34),(x+2,21,41.07),bone,context))
        objects.append(box(f'LowerStretcher_{x}',(x-1.5,-17,14),(x+1.5,21,18),bone,context))
    objects.append(box('FrontApron',(-12.19,-22.6,34),(12.68,-18.6,41.07),bone,context))
    return objects


def ring_geometry(name, rings, bone, context, cap=True):
    count = len(rings[0])
    vertices = [p for ring in rings for p in ring]
    faces = []
    if cap:
        faces.append(list(reversed(range(count))))
    for level in range(len(rings)-1):
        for i in range(count):
            nxt = (i+1)%count
            faces.append((level*count+i,level*count+nxt,(level+1)*count+nxt,(level+1)*count+i))
    if cap:
        faces.append(list(range((len(rings)-1)*count,len(vertices))))
    obj = shared.mesh(name,vertices,faces,[[(0,0)]*len(f) for f in faces],bone,context)
    planar_uv(obj)
    return obj


def stool_leg(index, points, bone, context):
    lower, upper = extremes(points)
    bottom = [p for p in points if abs(p[2]-lower[2]) < .001]
    top = [p for p in points if abs(p[2]-upper[2]) < .001]
    centers = [[sum(p[a] for p in ring)/4 for a in range(3)] for ring in (bottom,top)]
    for ring, center in zip((bottom,top),centers):
        ring.sort(key=lambda p:math.atan2(p[1]-center[1],p[0]-center[0]))
    rings = []
    for fraction, scale in ((0,1),(.04,1),(.12,.92),(.8,.92),(.95,1),(1,1)):
        center = [centers[0][a]*(1-fraction)+centers[1][a]*fraction for a in range(3)]
        points = [[a*(1-fraction)+b*fraction for a,b in zip(low,high)] for low,high in zip(bottom,top)]
        rings.append([[center[a]+(p[a]-center[a])*(scale if a<2 else 1) for a in range(3)] for p in points])
    return ring_geometry(f'SplayedStoolLeg_{index}',rings,bone,context)


def stool(part, source, context):
    chunks = ordered_chunks(part)
    objects = [stool_leg(i,points,part['bone'],context) for i,points in enumerate(chunks[:3])]
    seat_points = [p for k,p in part['vertices'].items() if int(k) >= 24]
    low, high = extremes(seat_points)
    center = [(low[a]+high[a])/2 for a in range(2)]
    top = [p for k,p in part['vertices'].items() if 24 <= int(k) < 36 and abs(p[2]-high[2]) < .001]
    top.sort(key=lambda p:math.atan2(p[1]-center[1],p[0]-center[0]))
    rings = [[[center[a]+(p[a]-center[a])*scale for a in range(2)]+[low[2]+(high[2]-low[2])*fraction]
             for p in top] for fraction,scale in ((0,.94),(.1,1),(.86,1),(1,1))]
    objects.append(ring_geometry('StoolSeatSidewall',rings,part['bone'],context,False))
    faces = [p for p in source.data.polygons if all(abs(source.data.vertices[v].co.z-high[2])<.001 for v in p.vertices)]
    objects.append(preserve_faces('ExactOriginalStoolSeat',source,faces,part['bone'],context,'top'))
    return objects
