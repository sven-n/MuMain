"""Read back saved sources and check reference preservation plus packed material dependencies."""
import hashlib
import json
from pathlib import Path
import sys
import bpy
sys.dont_write_bytecode=True
sys.path.insert(0,str(Path(__file__).resolve().parent))
from config import ROOT,PROPS


def mesh_snapshot(objects):
    return [{'name':obj.name,'positions':[list(v.co) for v in obj.data.vertices],
             'polygons':[list(p.vertices) for p in obj.data.polygons],
             'uv':[[list(u.uv) for u in layer.data] for layer in obj.data.uv_layers],
             'weights':[[(obj.vertex_groups[g.group].name,g.weight) for g in v.groups] for v in obj.data.vertices],
             'matrix':[list(row) for row in obj.matrix_world]} for obj in sorted(objects,key=lambda o:o.name)]


def rig_snapshot():
    rig=next(obj for obj in bpy.context.scene.objects if obj.type=='ARMATURE')
    return {'bones':[(b.name,b.parent.name if b.parent else None) for b in rig.data.bones],
            'bone_order':list(rig['mu_bone_order']),'actions':rig['mu_action_meta'].to_dict(),
            'transform':[list(row) for row in rig.matrix_world]}


def audit(name):
    root=ROOT/name
    bpy.ops.wm.open_mainfile(filepath=str(root/'original/source.blend'))
    original=mesh_snapshot([o for o in bpy.context.scene.objects if o.type=='MESH' and not o.get('mu_helper')])
    original_rig=rig_snapshot()
    bpy.ops.wm.open_mainfile(filepath=str(root/'source.blend'))
    ref=bpy.data.collections['REF_ORIGINAL']
    assert ref.hide_viewport and ref.hide_render and all(obj.get('mu_reference') for obj in ref.objects)
    assert original==mesh_snapshot(ref.objects)
    assert original_rig==rig_snapshot()
    active=bpy.data.collections['EXPORT_PAINTED'].objects
    bounds=lambda objects:[[fn(v.co[axis] for obj in objects for v in obj.data.vertices)
                            for axis in range(3)] for fn in (min,max)]
    assert bounds(ref.objects)==bounds(active),name+' source bounds changed'
    high=bpy.data.collections.get('REF_HIGH_POLY')
    if name!='Waterspout01':
        assert high and high.hide_render and high.hide_viewport
        assert all(obj.get('mu_reference') for obj in high.objects)
    images=[im for im in bpy.data.images if im.source=='FILE']
    assert all(im.packed_file for im in images)
    result={'status':'PASS','reference_geometry_uv_weights_transform':'identical to preserved import',
            'reference_snapshot_sha256':hashlib.sha256(json.dumps(original,sort_keys=True).encode()).hexdigest(),
            'source_bounds_exact':bounds(active),'editable_high_poly_excluded':bool(high),
            'rig_unchanged':original_rig,'packed_images':{im.name:{'size':list(im.size),
            'sha256':hashlib.sha256(im.packed_file.data).hexdigest()} for im in images},'no_external_images_required':True}
    (root/'validation/source-audit.json').write_text(json.dumps(result,indent=2)+'\n')
    print(name,'saved source audit PASS')


for prop in PROPS:
    audit(prop)
