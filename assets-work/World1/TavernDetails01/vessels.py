"""Rounded bottle, ceramic, fruit and tankard profiles fitted to original rigid-part bounds."""
import math
from mathutils import Vector

from model_geometry import FULL, fit, lathe, shared

BOTTLE = [(0,0),(.98,0),(1,.03),(1,.48),(.32,.66),(.32,.95),(.37,1),(0,1)]
BOTTLE_UV_LEVELS = [.01,.02,.04,.48,.70,.90,.94,.96]
BOWL = [(0,0),(.45,0),(.9,.72),(1,.94),(.97,1),(.84,.85),(.4,.16),(0,.16)]
APPLE = [(0,.08),(.35,0),(.8,.13),(1,.48),(.95,.74),(.68,.98),(.3,1),(0,.9)]
MUG = [(0,0),(.92,0),(1,.08),(.94,.14),(.94,.88),(1,.96),(.91,1),(.78,.89),(.78,.15),(0,.15)]
JUG = [(0,0),(.55,0),(.9,.16),(1,.48),(.62,.75),(.67,.95),(.7,1),(.55,1),(.5,.76),(0,.25)]
POT = [(0,0),(.58,0),(.84,.13),(1,.46),(.89,.67),(.61,.8),(.69,.95),(.67,1),(.54,1),(.48,.8),(0,.18)]


def handle_tube(name, centers, radius, bone, context):
    vertices, faces, uvs = [], [], []
    sides = 4
    for i, center in enumerate(centers):
        tangent = Vector(centers[min(i+1,len(centers)-1)]) - Vector(centers[max(i-1,0)])
        side = Vector((1,0,0))
        second = tangent.normalized().cross(side)
        for corner in range(sides):
            angle = math.tau*corner/sides
            vertices.append(list(Vector(center)+radius*(math.cos(angle)*side+math.sin(angle)*second)))
    for level in range(len(centers)-1):
        for corner in range(sides):
            nxt = (corner+1)%sides
            faces.append((level*sides+corner,level*sides+nxt,(level+1)*sides+nxt,(level+1)*sides+corner))
            uvs.append([shared.uv(FULL,u,v) for u,v in ((corner/sides,level/len(centers)),((corner+1)/sides,level/len(centers)),((corner+1)/sides,(level+1)/len(centers)),(corner/sides,(level+1)/len(centers)))])
    for ring in (list(reversed(range(sides))),list(range((len(centers)-1)*sides,len(vertices)))):
        faces.append(ring)
        uvs.append([shared.uv(FULL,.5+.4*math.cos(math.tau*i/sides),.5+.4*math.sin(math.tau*i/sides)) for i in range(sides)])
    return shared.mesh(name,vertices,faces,uvs,bone,context,True)


def handle(name, bone, context, mug=False):
    centers = [(0,.87,.8),(0,1.3,.8),(0,1.47,.64),(0,1.47,.28),(0,.86,.22)]
    if not mug:
        centers = [(0,.62,.8),(0,1.15,.86),(0,1.38,.63),(0,1.2,.34),(0,.86,.3)]
    return handle_tube(name, centers, .08, bone, context)


def vessel(part, context, dense):
    bone, material = part['bone'], part['material']
    segments = 12 if dense else 8
    profiles = {'bottle.tga': BOTTLE, 'plate.jpg': BOWL, 'apple.jpg': APPLE,
                'winecup.jpg': MUG, 'pot.jpg': JUG, 'pot2.jpg': POT}
    profile = profiles[material]
    objects = [lathe(bone + '_Body', profile, segments, bone, context)]
    if material == 'bottle.tga':
        # Height-aware label placement; keep distinct cap V values for valid UV fans.
        for point in objects[0].data.uv_layers[0].data:
            level = round((point.uv.y-FULL[1])/(FULL[3]-FULL[1])*(len(profile)-1))
            point.uv.y = FULL[1]+BOTTLE_UV_LEVELS[level]*(FULL[3]-FULL[1])
    if material in ('winecup.jpg', 'pot.jpg'):
        objects.append(handle(bone + '_Handle', bone, context, material == 'winecup.jpg'))
        angle = part['handle_angle']
        for obj in objects:
            for vertex in obj.data.vertices:
                x,y = vertex.co.x,vertex.co.y
                vertex.co.x = x*math.cos(angle)-y*math.sin(angle)
                vertex.co.y = x*math.sin(angle)+y*math.cos(angle)
    fit(objects, part['bounds'])
    return objects
