"""Selective rock ridge chamfers; preserve extreme anchors and all vegetation cards."""
import math
import bmesh
import bpy

ANGLE_LIMIT=math.radians(25)
EXTREME_TOLERANCE=0.0001
RELATIVE_BEVEL_WIDTH=.004
MIN_BEVEL_WIDTH=.25
MAX_BEVEL_WIDTH=1.4
WEIGHT_ATTRIBUTE='bevel_weight_edge'
MIN_UV_AREA=1e-8
UV_REPAIR_RADIUS=.002


def select_ridges(obj):
    data=bmesh.new()
    data.from_mesh(obj.data)
    data.edges.ensure_lookup_table()
    points=[vertex.co for vertex in obj.data.vertices]
    low=[min(point[a] for point in points) for a in range(3)]
    high=[max(point[a] for point in points) for a in range(3)]
    protected={v.index for v in data.verts if any(abs(v.co[a]-limit[a])<EXTREME_TOLERANCE
               for a in range(3) for limit in (low,high))}
    selected=[]
    for edge in data.edges:
        if not edge.is_manifold or not edge.is_convex:
            continue
        if any(vertex.index in protected for vertex in edge.verts):
            continue
        if any(obj.data.materials[face.material_index].name!='ston01.jpg' for face in edge.link_faces):
            continue
        if edge.calc_face_angle() < ANGLE_LIMIT:
            continue
        selected.append(edge.index)
    data.free()
    width=min(MAX_BEVEL_WIDTH,max(MIN_BEVEL_WIDTH,max(b-a for a,b in zip(low,high))*RELATIVE_BEVEL_WIDTH))
    return selected,protected,width


def make_modifier(obj,width,segments):
    modifier=obj.modifiers.new('AuthoredRockRidgeChamfers','BEVEL')
    modifier.limit_method='WEIGHT'
    modifier.edge_weight=WEIGHT_ATTRIBUTE
    modifier.width=width
    modifier.segments=segments
    modifier.use_clamp_overlap=True
    modifier.affect='EDGES'
    return modifier


def keep_editable_high_poly(obj,width):
    collection=bpy.data.collections.get('REF_HIGH_POLY')
    if collection is None:
        collection=bpy.data.collections.new('REF_HIGH_POLY')
        bpy.context.scene.collection.children.link(collection)
        collection.hide_viewport=collection.hide_render=True
        collection.vs.export=False
    high=obj.copy()
    high.data=obj.data.copy()
    high.name=obj.name+'_EditableChamfers'
    high['mu_reference']=True
    collection.objects.link(high)
    make_modifier(high,width,3)


def refine(obj):
    project_rock_uvs(obj)
    selected,protected,width=select_ridges(obj)
    assert selected, obj.name+' has no suitable ridge edges'
    attribute=obj.data.attributes.get(WEIGHT_ATTRIBUTE)
    if attribute is None:
        attribute=obj.data.attributes.new(WEIGHT_ATTRIBUTE,'FLOAT','EDGE')
    for index in selected:
        attribute.data[index].value=1
    keep_editable_high_poly(obj,width)
    modifier=make_modifier(obj,width,1)
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active=obj
    bpy.ops.object.modifier_move_to_index(modifier=modifier.name,index=0)
    bpy.ops.object.modifier_apply(modifier=modifier.name)
    data=bmesh.new()
    data.from_mesh(obj.data)
    bmesh.ops.triangulate(data,faces=list(data.faces),quad_method='FIXED',ngon_method='EAR_CLIP')
    data.to_mesh(obj.data)
    data.free()
    # All five rock assets have a single original root; bevel interpolation keeps that binding.
    assert len(obj.vertex_groups)==1
    obj.vertex_groups[0].add(list(range(len(obj.data.vertices))),1,'REPLACE')
    obj.data.update()
    projected=project_rock_uvs(obj)
    repaired=repair_collapsed_uv(obj)
    return {'selected_convex_ridges':len(selected),'protected_extreme_vertices':len(protected),
            'width_game_units':width,'segments_export':1,'segments_editable_high_poly':3,
            'selection':'Only ston01 convex manifold ridges above 25 degrees; no grass or bounds-extreme edges',
            'collapsed_new_bevel_uvs_repaired':repaired,
            'rock_faces_box_projected':projected,
            'uv_projection':'Dominant face axis, uniform world density; grass UVs untouched'}


def repair_collapsed_uv(obj):
    """Give collapsed chamfer corners a small local texel footprint, leaving grass UVs intact."""
    coordinates=obj.data.uv_layers.active.data
    repaired=0
    for face in obj.data.polygons:
        if obj.data.materials[face.material_index].name!='ston01.jpg':
            continue
        a,b,c=[coordinates[index].uv.copy() for index in face.loop_indices]
        area=abs((b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x))
        if area > MIN_UV_AREA:
            continue
        center=(a+b+c)/3
        center.x=min(1-UV_REPAIR_RADIUS,max(UV_REPAIR_RADIUS,center.x))
        center.y=min(1-UV_REPAIR_RADIUS,max(UV_REPAIR_RADIUS,center.y))
        for index,offset in zip(face.loop_indices,((-1,-1),(1,-1),(0,1))):
            coordinates[index].uv=(center.x+offset[0]*UV_REPAIR_RADIUS,center.y+offset[1]*UV_REPAIR_RADIUS)
        repaired+=1
    return repaired


def project_rock_uvs(obj):
    """Remove the legacy top-projection streaks from steep rock faces."""
    faces=[face for face in obj.data.polygons
           if obj.data.materials[face.material_index].name=='ston01.jpg']
    points=[obj.data.vertices[index].co for face in faces for index in face.vertices]
    low=[min(point[axis] for point in points) for axis in range(3)]
    high=[max(point[axis] for point in points) for axis in range(3)]
    scale=.96/max(high[axis]-low[axis] for axis in range(3))
    coordinates=obj.data.uv_layers.active.data
    for face in faces:
        normal_axis=max(range(3),key=lambda axis:abs(face.normal[axis]))
        axes=[axis for axis in range(3) if axis!=normal_axis]
        for loop_index in face.loop_indices:
            point=obj.data.vertices[obj.data.loops[loop_index].vertex_index].co
            coordinates[loop_index].uv=[.02+(point[axis]-low[axis])*scale for axis in axes]
    return len(faces)
