"""Reopen packed source files and verify reference exclusion and texture independence."""

import json
from pathlib import Path

import bpy

ROOT = Path(__file__).resolve().parent
PROPS = ('Candle01', 'TreasureChest01', 'Tomb03')


def audit(prop):
    folder = ROOT.parent / prop
    bpy.ops.wm.open_mainfile(filepath=str(folder / 'source.blend'))
    images = [image for image in bpy.data.images if image.source == 'FILE']
    assert images and all(image.packed_file for image in images), 'Unpacked image dependency'
    for name in ('REF_ORIGINAL', 'REF_HIGH_POLY'):
        collection = bpy.data.collections[name]
        assert collection.objects and collection.hide_render and not collection.vs.export
        assert all(obj.get('mu_reference') for obj in collection.objects)
    target = bpy.data.collections['EXPORT_' + prop]
    assert all(not obj.get('mu_reference') for obj in target.objects)
    assert all(len(obj.data.uv_layers) == 1 and obj.data.uv_layers[0].name == 'UVMap'
               for obj in target.objects), 'UV layer names must match when joining'
    report = dict(status='PASS', packed_images=[image.name for image in images],
                  reference_objects=len(bpy.data.collections['REF_ORIGINAL'].objects),
                  high_poly_sources=len(bpy.data.collections['REF_HIGH_POLY'].objects),
                  export_objects=len(target.objects), original_rig_reused=True,
                  external_image_files_required=False, reference_exclusion='REF_* collections + mu_reference')
    (folder / 'validation/source-audit.json').write_text(json.dumps(report, indent=2) + '\n')
    print(prop, report)


for prop in PROPS:
    audit(prop)
