"""Small deterministic Blender mesh helpers for the Lorencia prop batch."""

import math

import bmesh
import bpy
from mathutils import Vector

TAU = math.tau


def collection(name):
    result = bpy.data.collections.new(name)
    bpy.context.scene.collection.children.link(result)
    result.vs.export = False
    return result


def preserve_original():
    reference = collection('REF_ORIGINAL')
    armature = next(obj for obj in bpy.context.scene.objects if obj.type == 'ARMATURE')
    for obj in list(bpy.context.scene.objects):
        if obj.type != 'MESH' or obj.get('mu_helper'):
            continue
        for owner in list(obj.users_collection):
            owner.objects.unlink(obj)
        reference.objects.link(obj)
        obj['mu_reference'] = True
    reference.hide_render = True
    reference.hide_viewport = True
    for material in bpy.data.materials:
        material.name = 'REF_' + material.name
    return armature, reference


def material(path):
    result = bpy.data.materials.new(path.name)
    result.use_nodes = True
    result['mu_texture'] = path.name
    shader = result.node_tree.nodes.get('Principled BSDF')
    shader.inputs['Roughness'].default_value = 1
    shader.inputs['Specular IOR Level'].default_value = 0
    texture = result.node_tree.nodes.new('ShaderNodeTexImage')
    texture.image = bpy.data.images.load(str(path), check_existing=False)
    result.node_tree.links.new(texture.outputs['Color'], shader.inputs['Base Color'])
    return result


def uv(rect, u, v):
    left, bottom, right, top = rect
    return left + u * (right - left), bottom + v * (top - bottom)


def mesh(name, vertices, faces, coordinates, bone, context, smooth=False):
    target, rig, texture = context
    data = bpy.data.meshes.new(name)
    data.from_pydata(vertices, [], faces)
    data.update()
    obj = bpy.data.objects.new(name, data)
    target.objects.link(obj)
    data.materials.append(texture)
    layer = data.uv_layers.new(name='UVMap')
    for polygon, face_uvs in zip(data.polygons, coordinates):
        polygon.use_smooth = smooth
        for loop, value in zip(polygon.loop_indices, face_uvs):
            layer.data[loop].uv = value
    obj.vertex_groups.new(name=bone).add(list(range(len(vertices))), 1, 'REPLACE')
    obj.parent = rig
    modifier = obj.modifiers.new('OriginalRig', 'ARMATURE')
    modifier.object = rig
    return obj


def box(name, low, high, rect, bone, context, bevel=0):
    vertices = [(x, y, z) for z in (low[2], high[2])
                for y in (low[1], high[1]) for x in (low[0], high[0])]
    faces = [(0, 2, 3, 1), (4, 5, 7, 6), (0, 1, 5, 4),
             (2, 6, 7, 3), (0, 4, 6, 2), (1, 3, 7, 5)]
    coordinates = [[uv(rect, u, v) for u, v in ((0, 0), (1, 0), (1, 1), (0, 1))]] * len(faces)
    obj = mesh(name, vertices, faces, coordinates, bone, context)
    if bevel:
        apply_bevel(obj, bevel)
        # Bevel interpolates weights with float rounding; this authored part is rigid.
        obj.vertex_groups[bone].add(list(range(len(obj.data.vertices))), 1, 'REPLACE')
    return obj


def apply_bevel(obj, width, segments=1):
    """Bake only the authored bevel; retain the original armature modifier untouched."""
    modifier = obj.modifiers.new('AuthoredEdgeBevel', 'BEVEL')
    modifier.width = width
    modifier.segments = segments
    modifier.affect = 'EDGES'
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.modifier_move_up(modifier=modifier.name)
    bpy.ops.object.modifier_apply(modifier=modifier.name)
    obj.select_set(False)


def lathe(name, profile, center, segments, rect, bone, context, aspect=1, smooth=True):
    vertices, rings = [], []
    for radius, height in profile:
        ring = []
        for index in range(segments if radius else 1):
            angle = TAU * index / segments
            ring.append(len(vertices))
            vertices.append((center[0] + radius * math.cos(angle),
                             center[1] + aspect * radius * math.sin(angle), center[2] + height))
        rings.append(ring)
    faces, coordinates = [], []
    for level, (lower, upper) in enumerate(zip(rings, rings[1:])):
        for index in range(segments):
            nxt = (index + 1) % segments
            face = [lower[index % len(lower)], lower[nxt % len(lower)],
                    upper[nxt % len(upper)], upper[index % len(upper)]]
            points = [(index / segments, level / (len(rings) - 1)),
                      ((index + 1) / segments, level / (len(rings) - 1)),
                      ((index + 1) / segments, (level + 1) / (len(rings) - 1)),
                      (index / segments, (level + 1) / (len(rings) - 1))]
            if len(lower) == 1:
                del face[0]
                del points[0]
            if len(upper) == 1:
                del face[-1]
                del points[-1]
            faces.append(face)
            coordinates.append([uv(rect, *point) for point in points])
    return mesh(name, vertices, faces, coordinates, bone, context, smooth)


def tube(name, centers, radii, segments, rect, bone, context):
    """Sweep a low-sided metal arm through an explicit designer-authored centerline."""
    vertices = []
    for index, (center, radius) in enumerate(zip(centers, radii)):
        tangent = Vector(centers[min(index + 1, len(centers) - 1)]) - Vector(centers[max(0, index - 1)])
        tangent.normalize()
        side = tangent.cross(Vector((0, 1, 0))).normalized()
        second = tangent.cross(side).normalized()
        for corner in range(segments):
            angle = TAU * corner / segments
            vertices.append(tuple(Vector(center) + radius * (math.cos(angle) * side + math.sin(angle) * second)))
    faces, coords = [], []
    for level in range(len(centers) - 1):
        for corner in range(segments):
            nxt = (corner + 1) % segments
            faces.append((level * segments + corner, level * segments + nxt,
                          (level + 1) * segments + nxt, (level + 1) * segments + corner))
            coords.append([uv(rect, u, v) for u, v in ((corner / segments, level / len(centers)),
                          ((corner + 1) / segments, level / len(centers)),
                          ((corner + 1) / segments, (level + 1) / len(centers)),
                          (corner / segments, (level + 1) / len(centers)))])
    for level, reverse in ((0, True), (len(centers) - 1, False)):
        face = list(range(level * segments, (level + 1) * segments))
        faces.append(face[::-1] if reverse else face)
        coords.append([uv(rect, .5 + .4 * math.cos(TAU * i / segments),
                          .5 + .4 * math.sin(TAU * i / segments)) for i in range(segments)])
    return mesh(name, vertices, faces, coords, bone, context, True)


def triangulate(objects):
    for obj in objects:
        if obj.get('mu_preserved_flames'):
            continue
        data = bmesh.new()
        data.from_mesh(obj.data)
        bmesh.ops.recalc_face_normals(data, faces=list(data.faces))
        bmesh.ops.triangulate(data, faces=list(data.faces), quad_method='FIXED', ngon_method='EAR_CLIP')
        data.to_mesh(obj.data)
        data.free()
        obj.data.update()


def retain_high_poly(objects):
    high_poly = collection('REF_HIGH_POLY')
    for obj in objects:
        if obj.get('mu_preserved_flames'):
            continue
        clone = obj.copy()
        clone.data = obj.data.copy()
        clone.name = 'HIGH_' + obj.name
        clone['mu_reference'] = True
        high_poly.objects.link(clone)
        modifier = clone.modifiers.new('FutureBakeBevel', 'BEVEL')
        modifier.width = .12
        modifier.segments = 3
    high_poly.hide_render = True
    high_poly.hide_viewport = True
