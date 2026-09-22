"""Rebuild cart/hay details while keeping immutable imported rigs and cart contacts."""
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
from bundles import bundle
from cart_parts import axle_sleeves

TRIANGLE_BUDGET = 1500


def original_faces(source, part):
    material = next(i for i,m in enumerate(source.data.materials) if m.name == 'REF_'+part['material'])
    bone = source.vertex_groups[part['bone']].index
    return [p for p in source.data.polygons if p.material_index == material and
            all(any(g.group == bone for g in source.data.vertices[v].groups) for v in p.vertices)]


def repair_collapsed_original_uvs(obj):
    repaired=0
    layer=obj.data.uv_layers[0]
    for face in obj.data.polygons:
        coordinates=[layer.data[i].uv.copy() for i in face.loop_indices]
        a,b,c=coordinates
        area=abs((b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x))
        if area>1e-10:
            continue
        # Repair only existing collapsed wood UVs, within a quarter original atlas texel.
        edge=b-a
        if edge.length<1e-8:
            layer.data[face.loop_indices[1]].uv.x+=1/1024
        layer.data[face.loop_indices[2]].uv+=Vector((-edge.y,edge.x)).normalized()/1024 if edge.length else Vector((0,1/1024))
        repaired+=1
    obj['mu_original_uv_repairs']=repaired


def build_part(name, part, source, context):
    if part['material']=='grass_01.tga':
        return bundle(part,context)
    original=preserve_faces(part['bone']+'_Retained',source,original_faces(source,part),part['bone'],context)
    original['mu_preserved_flames']=True
    if part['material']=='horse_drawn_01.jpg':
        repair_collapsed_original_uvs(original)
    objects=[original]
    wheel_bone='Bone05' if name=='Carriage01' else 'Cylinder03'
    if part['bone']==wheel_bone and part['material']=='horse_drawn_01.jpg':
        objects.extend(axle_sleeves(name,part,context))
    return objects


def distinguish_rigid_normals(obj,rig):
    """Avoid legacy converter merging normals across animated bone indices.

    SMD2BMD deduplicates equal local normals without their bone. A tiny tangent
    offset per rigid part keeps numerical identity separate without visible shading change.
    """
    order=list(rig['mu_bone_order'])
    original=[normal.vector.copy() for normal in obj.data.corner_normals]
    normals=[]
    for loop,normal in zip(obj.data.loops,original):
        vertex=obj.data.vertices[loop.vertex_index]
        bone=obj.vertex_groups[vertex.groups[0].group].name
        index=order.index(bone)+1
        direction=Vector((.000071*index,.000113*index,.000173*index))
        tangent=direction-normal*direction.dot(normal)
        normals.append((normal+tangent).normalized())
    obj.data.normals_split_custom_set(normals)
    obj['mu_normal_binding_guard']='Per-bone tangent offset <0.004 radians prevents cross-node normal merging'


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
    distinguish_rigid_normals(result,rig)
    triangles, after = validate([result],rig,before)
    report = dict(triangles=triangles,bounds_before=before,bounds_after=after,material_order=MATERIALS[name],
                  bone_order=list(rig['mu_bone_order']),action_meta=rig['mu_action_meta'].to_dict(),
                  textures={n:list(next(node.image.size for node in mat.node_tree.nodes if node.type=='TEX_IMAGE')) for n,mat in materials.items()},
                  geometry='Rounded closed tied-straw bundles and iron axle sleeves; original cart surfaces, wheel contacts, lamp/effect geometry and UVs retained')
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
