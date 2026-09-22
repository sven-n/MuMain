"""Measured rigid modeling helpers; all coordinates remain in original game units."""
import math
import sys
from pathlib import Path

import bpy
from mathutils import Vector

ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT.parent / 'StaticBatch01'))
import geometry as shared

REGIONS = {'top': (.02,.52,.48,.98), 'panel': (.52,.52,.98,.98),
           'wood': (.02,.02,.48,.48), 'iron': (.54,.04,.96,.46)}
FULL = (.02,.02,.98,.98)


def preserve_original():
    reference = shared.collection('REF_ORIGINAL')
    rig = next(obj for obj in bpy.context.scene.objects if obj.type == 'ARMATURE')
    for obj in list(bpy.context.scene.objects):
        if obj.type != 'MESH' or obj.get('mu_helper'):
            continue
        for owner in list(obj.users_collection):
            owner.objects.unlink(obj)
        reference.objects.link(obj)
        obj['mu_reference'] = True
    reference.hide_render = reference.hide_viewport = True
    # Material names reorder this Blender collection; snapshot before renaming.
    for surface in list(bpy.data.materials):
        surface.name = 'REF_' + surface.name
    return rig, reference


def bounds(objects):
    points = [o.matrix_world @ v.co for o in objects for v in o.data.vertices]
    return [[min(p[a] for p in points) for a in range(3)],
            [max(p[a] for p in points) for a in range(3)]]


def fit(objects, target):
    low, high = bounds(objects)
    for obj in objects:
        for vertex in obj.data.vertices:
            vertex.co = [target[0][a] + (vertex.co[a]-low[a]) / (high[a]-low[a]) *
                         (target[1][a]-target[0][a]) for a in range(3)]
        obj.data.update()


def material(filename):
    result = shared.material(ROOT / 'textures/final' / filename)
    if filename.endswith('.tga'):
        nodes = result.node_tree.nodes
        texture = next(n for n in nodes if n.type == 'TEX_IMAGE')
        result.node_tree.links.new(texture.outputs['Alpha'], nodes['Principled BSDF'].inputs['Alpha'])
    return result


def planar_uv(obj, region='wood', top=None):
    low, high = bounds([obj])
    for polygon in obj.data.polygons:
        axis = max(range(3), key=lambda a: abs(polygon.normal[a]))
        axes = [a for a in range(3) if a != axis]
        selected = top if top and polygon.normal.z > .99 else region
        if selected in ('wood', 'panel'):
            axes.sort(key=lambda a: high[a]-low[a])
        for loop in polygon.loop_indices:
            point = obj.data.vertices[obj.data.loops[loop].vertex_index].co
            uv = [(point[a]-low[a]) / max(high[a]-low[a], .000001) for a in axes]
            obj.data.uv_layers[0].data[loop].uv = shared.uv(REGIONS[selected], *uv)


def box(name, low, high, bone, context, region='wood', top=None, bevel=0):
    obj = shared.box(name, low, high, FULL, bone, context, bevel)
    planar_uv(obj, region, top)
    return obj


def preserve_faces(name, source, polygons, bone, context, project=None):
    indices = sorted({v for p in polygons for v in p.vertices})
    remap = {v:i for i,v in enumerate(indices)}
    vertices = [list(source.matrix_world @ source.data.vertices[v].co) for v in indices]
    faces = [[remap[v] for v in p.vertices] for p in polygons]
    uvs = [[list(source.data.uv_layers[0].data[loop].uv) for loop in p.loop_indices] for p in polygons]
    obj = shared.mesh(name, vertices, faces, uvs, bone, context)
    if project:
        planar_uv(obj, project, 'top')
    return obj


def lathe(name, profile, segments, bone, context):
    return shared.lathe(name, profile, (0,0,0), segments, FULL, bone, context)


def combine(objects, materials, target, rig):
    vertices, faces, coordinates, bones, indices = [], [], [], [], []
    for material_index, surface in enumerate(materials):
        for obj in [o for o in objects if o.data.materials[0] == surface]:
            offset = len(vertices)
            vertices.extend(list(obj.matrix_world @ v.co) for v in obj.data.vertices)
            bones.extend(obj.vertex_groups[v.groups[0].group].name for v in obj.data.vertices)
            for polygon in obj.data.polygons:
                faces.append([offset+v for v in polygon.vertices])
                coordinates.append([list(obj.data.uv_layers[0].data[loop].uv) for loop in polygon.loop_indices])
                indices.append((material_index, polygon.use_smooth))
    result = shared.mesh('EXPORT_MESH', vertices, faces, coordinates, bones[0], (target, rig, materials[0]))
    result.vertex_groups.clear()
    for bone in rig['mu_bone_order']:
        group = result.vertex_groups.new(name=bone)
        owned = [i for i, name in enumerate(bones) if name == bone]
        if owned:
            group.add(owned, 1, 'REPLACE')
    for surface in materials[1:]:
        result.data.materials.append(surface)
    for polygon, (index, smooth) in zip(result.data.polygons, indices):
        polygon.material_index, polygon.use_smooth = index, smooth
    return result
