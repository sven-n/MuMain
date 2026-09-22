"""Rebuild the complete tavern dependency group on immutable imported rigs."""
import json
import math
from pathlib import Path
import sys

sys.dont_write_bytecode = True
import bpy
from mathutils import Vector

ROOT = Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT))
from config import ASSETS, MATERIALS
from model_geometry import bounds, combine, material, preserve_faces, preserve_original, shared
from furniture import chair, shelf, stool
from vessels import vessel

TRIANGLE_BUDGET = 1500


def original_faces(source, part):
    material = next(i for i,m in enumerate(source.data.materials) if m.name == 'REF_'+part['material'])
    bone = source.vertex_groups[part['bone']].index
    return [p for p in source.data.polygons if p.material_index == material and
            all(any(g.group == bone for g in source.data.vertices[v].groups) for v in p.vertices)]


def build_part(name, part, source, context):
    if part['material'] in ('pot3.tga','chair2.tga'):
        return [preserve_faces(part['bone']+'_Cutout',source,original_faces(source,part),part['bone'],context)]
    if part['material'] == 'bookshelf.jpg':
        if name in ('Furniture01','Furniture02'):
            return shelf(part,context)
        return chair(part,source,context) if name == 'Furniture06' else stool(part,source,context)
    if part['material'] in ('winecup.jpg','pot.jpg'):
        indices = {v for p in original_faces(source,part) for v in p.vertices}
        points = [source.matrix_world @ source.data.vertices[v].co for v in indices]
        low = min(p.z for p in points)
        bottom = [p for p in points if abs(p.z-low)<.001]
        center = sum(bottom,Vector())/len(bottom)
        farthest = max(points,key=lambda p:(p.x-center.x)**2+(p.y-center.y)**2)
        part['handle_angle'] = math.atan2(farthest.y-center.y,farthest.x-center.x)-math.pi/2
    objects = vessel(part,context,name.startswith('Beer'))
    return objects


def validate(objects, rig, before):
    triangles = sum(len(o.data.polygons) for o in objects)
    assert triangles <= TRIANGLE_BUDGET, triangles
    for obj in objects:
        assert all(p.area > .0000001 and len(p.vertices)==3 for p in obj.data.polygons)
        for vertex in obj.data.vertices:
            assert len(vertex.groups)==1 and vertex.groups[0].weight==1
            assert obj.vertex_groups[vertex.groups[0].group].name in rig.data.bones
        assert all(math.isfinite(v) for point in obj.data.uv_layers[0].data for v in point.uv)
    after = bounds(objects)
    assert max(abs(a-b) for x,y in zip(before,after) for a,b in zip(x,y)) < .001, (before,after)
    return triangles, after


def build(name):
    folder = ROOT/name
    bpy.ops.wm.open_mainfile(filepath=str(folder/'original/source.blend'))
    bpy.context.scene.frame_set(0)
    rig, reference = preserve_original()
    source = next(iter(reference.objects))
    before = bounds(list(reference.objects))
    target = shared.collection('EXPORT_'+name)
    materials = {n:material(n) for n in MATERIALS[name]}
    parts = json.loads((folder/'original/parts.json').read_text())
    objects = []
    for part in parts:
        objects.extend(build_part(name,part,source,(target,rig,materials[part['material']])))
    shared.retain_high_poly(objects)
    shared.triangulate(objects)
    result = combine(objects,list(materials.values()),target,rig)
    for obj in objects:
        bpy.data.objects.remove(obj,do_unlink=True)
    triangles, after = validate([result],rig,before)
    report = dict(triangles=triangles,bounds_before=before,bounds_after=after,material_order=MATERIALS[name],
                  bone_order=list(rig['mu_bone_order']),action_meta=rig['mu_action_meta'].to_dict(),
                  textures={n:list(next(node.image.size for node in mat.node_tree.nodes if node.type=='TEX_IMAGE')) for n,mat in materials.items()},
                  geometry='Authored vessel profiles and measured carpentry; original seat surfaces and alpha cards retained')
    (folder/'validation/blender.json').write_text(json.dumps(report,indent=2)+'\n')
    (folder/'validation/construction.json').write_text(json.dumps(parts,indent=2)+'\n')
    for image in bpy.data.images:
        if image.source == 'FILE':
            image.pack()
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(folder/'source.blend'))
    print(name,triangles,'triangles',flush=True)


for asset in ASSETS:
    build(asset)
