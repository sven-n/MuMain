"""Deterministic rigid timber/iron geometry and atlas UVs through Blender's mesh API."""

import math

import bmesh
import bpy

REGIONS = {'top': (.015, .515, .485, .985), 'panel': (.515, .515, .985, .985),
           'wood': (.015, .015, .485, .485), 'iron': (.54, .04, .96, .46)}
AREA_EPSILON = 0.000001


def collection(name):
    target = bpy.data.collections.new(name)
    bpy.context.scene.collection.children.link(target)
    target.vs.export = False
    return target


def bounds(objects):
    points = [o.matrix_world @ v.co for o in objects for v in o.data.vertices]
    return [[min(p[a] for p in points) for a in range(3)],
            [max(p[a] for p in points) for a in range(3)]]


def material(path):
    result = bpy.data.materials.new('desk_big.jpg')
    result.use_nodes = True
    result['mu_texture'] = 'desk_big.jpg'
    shader = result.node_tree.nodes.get('Principled BSDF')
    shader.inputs['Roughness'].default_value = 1
    shader.inputs['Specular IOR Level'].default_value = 0
    texture = result.node_tree.nodes.new('ShaderNodeTexImage')
    texture.image = bpy.data.images.load(str(path), check_existing=False)
    result.node_tree.links.new(texture.outputs['Color'], shader.inputs['Base Color'])
    return result


def map_uv(region, u, v):
    left, bottom, right, top = REGIONS[region]
    return left + u * (right - left), bottom + v * (top - bottom)


def projected_uvs(data, regions):
    """Planar carpentry faces: grain follows their longest dimension, inset atlas islands."""
    low = [min(v.co[a] for v in data.vertices) for a in range(3)]
    size = [max(v.co[a] for v in data.vertices) - low[a] for a in range(3)]
    for layer in list(data.uv_layers):
        data.uv_layers.remove(layer)
    layer = data.uv_layers.new(name='UVMap')
    for polygon, region in zip(data.polygons, regions):
        normal_axis = max(range(3), key=lambda a: abs(polygon.normal[a]))
        axes = [a for a in range(3) if a != normal_axis]
        if region in ('wood', 'iron'):
            axes.sort(key=lambda a: size[a])
        for loop in polygon.loop_indices:
            point = data.vertices[data.loops[loop].vertex_index].co
            u, v = [(point[a] - low[a]) / max(size[a], AREA_EPSILON) for a in axes]
            layer.data[loop].uv = map_uv(region, u, v)


def mesh(name, vertices, faces, regions, context):
    target, rig, surface = context
    data = bpy.data.meshes.new(name)
    data.from_pydata(vertices, [], faces)
    data.update()
    obj = bpy.data.objects.new(name, data)
    target.objects.link(obj)
    data.materials.append(surface)
    projected_uvs(data, regions)
    obj.vertex_groups.new(name=rig['mu_bone_order'][0]).add(list(range(len(vertices))), 1, 'REPLACE')
    obj.parent = rig
    obj.modifiers.new('OriginalRig', 'ARMATURE').object = rig
    obj['mu_component'] = name
    return obj


def bevel(obj, width, segments=1):
    modifier = obj.modifiers.new('AuthoredBevel', 'BEVEL')
    modifier.width = width
    modifier.segments = segments
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.modifier_move_up(modifier=modifier.name)
    bpy.ops.object.modifier_apply(modifier=modifier.name)
    obj.vertex_groups[0].add(list(range(len(obj.data.vertices))), 1, 'REPLACE')
    obj.select_set(False)


def box(name, low, high, context, region='wood', top=None, front=None, chamfer=0):
    vertices = [(x, y, z) for z in (low[2], high[2])
                for y in (low[1], high[1]) for x in (low[0], high[0])]
    faces = [(0, 2, 3, 1), (4, 5, 7, 6), (0, 1, 5, 4),
             (2, 6, 7, 3), (0, 4, 6, 2), (1, 3, 7, 5)]
    regions = [region, top or region, front or region, region, region, region]
    obj = mesh(name, vertices, faces, regions, context)
    if chamfer:
        bevel(obj, chamfer)
        # Interpolated bevel UVs can collapse after SMD's decimal quantization.
        regions = [top if top and p.normal.z > .99 else
                   front if front and p.normal.y < -.99 else region for p in obj.data.polygons]
        projected_uvs(obj.data, regions)
    return obj


def octagon(cx, cy, width, depth, cut):
    x0, x1, y0, y1 = cx - width / 2, cx + width / 2, cy - depth / 2, cy + depth / 2
    return [(x0 + cut, y0), (x1 - cut, y0), (x1, y0 + cut), (x1, y1 - cut),
            (x1 - cut, y1), (x0 + cut, y1), (x0, y1 - cut), (x0, y0 + cut)]


def ring_solid(name, rings, context, region='wood', top=None):
    count = len(rings[0])
    assert all(len(ring) == count for ring in rings)
    vertices = [point for ring in rings for point in ring]
    faces = [list(reversed(range(count)))]
    for level in range(len(rings) - 1):
        for i in range(count):
            nxt = (i + 1) % count
            faces.append((level * count + i, level * count + nxt,
                          (level + 1) * count + nxt, (level + 1) * count + i))
    faces.append(list(range((len(rings) - 1) * count, len(vertices))))
    return mesh(name, vertices, faces, [region] * (len(faces) - 1) + [top or region], context)


def timber(name, sections, context, region='wood', cut=.7):
    rings = [[(x, y, height) for x, y in octagon(cx, cy, width, depth, cut)]
             for cx, cy, height, width, depth in sections]
    return ring_solid(name, rings, context, region)


def rivet(name, center, radius, context, axis='Y'):
    segments, depth = 6, radius * .32
    rings = []
    for offset, scale in ((0, 1), (depth, .6)):
        ring = []
        for i in range(segments):
            a = math.tau * i / segments
            u, v = radius * scale * math.cos(a), radius * scale * math.sin(a)
            delta = (u, -offset, v) if axis == 'Y' else (u, v, offset)
            ring.append(tuple(c + d for c, d in zip(center, delta)))
        rings.append(ring)
    return ring_solid(name, rings, context, 'iron')


def high_poly_and_triangulate(objects):
    source = collection('REF_HIGH_POLY')
    for obj in objects:
        clone = obj.copy()
        clone.data = obj.data.copy()
        clone.name = 'HIGH_' + obj.name
        clone['mu_reference'] = True
        source.objects.link(clone)
        modifier = clone.modifiers.new('EditableFineBevel', 'BEVEL')
        modifier.width = .18
        modifier.segments = 3
        data = bmesh.new()
        data.from_mesh(obj.data)
        bmesh.ops.recalc_face_normals(data, faces=list(data.faces))
        bmesh.ops.triangulate(data, faces=list(data.faces), quad_method='FIXED', ngon_method='EAR_CLIP')
        data.to_mesh(obj.data)
        data.free()
        obj.data.update()
    source.hide_render = True
    source.hide_viewport = True
