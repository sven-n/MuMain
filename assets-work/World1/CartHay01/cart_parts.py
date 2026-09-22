"""Small useful iron hub profiles, inside unchanged cart axle and wheel contacts."""
from mathutils import Vector
from model_geometry import shared

IRON_RECT=(.85,.075,.96,.21)
HUB_SEGMENTS=12


def axle_sleeves(name,part,context):
    axis=1 if name=='Carriage01' else 0
    low,high=part['bounds']
    if axis==1:
        center=Vector((.4834,0,57.2819))
    else:
        offset=1.8518 if name in ('Carriage03','Carriage04') else 0
        center=Vector((0,82.0943+offset,64.5782))
    profiles=((0,0),(13,0),(15,1.2),(15,7.5),(12,9),(0,9))
    objects=[]
    for side,position in ((1,low[axis]+.001),(-1,high[axis]-.001)):
        obj=shared.lathe(part['bone']+'_IronHub',profiles,(0,0,0),HUB_SEGMENTS,IRON_RECT,part['bone'],context)
        for vertex in obj.data.vertices:
            x,y,z=vertex.co
            vertex.co=(position+side*z,center.y+x,center.z+y) if axis==0 else (center.x+x,position+side*z,center.z+y)
        obj.data.update()
        objects.append(obj)
    return objects
