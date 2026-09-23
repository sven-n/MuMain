"""Require valid decoded images and byte-identical frozen payloads in the editable source."""
from pathlib import Path
import hashlib
import json
import sys
sys.dont_write_bytecode=True
import bpy
ROOT=Path(__file__).resolve().parent


def check(folder):
    bpy.ops.wm.open_mainfile(filepath=str(folder/'source.blend'))
    report=[]
    for obj in bpy.context.scene.objects:
        if obj.type!='MESH' or obj.get('mu_reference') or obj.get('mu_helper'):continue
        for material in obj.data.materials:
            texture=material.get('mu_texture')
            image=next(node.image for node in material.node_tree.nodes if node.type=='TEX_IMAGE')
            dimensions=list(image.size)
            decoded_sample=list(image.pixels[:4])
            assert image.packed_file and image.has_data and min(dimensions)>0 and len(decoded_sample)==4,texture
            expected=(folder/'textures'/texture).read_bytes()
            assert image.packed_file.data==expected,texture
            report.append(dict(texture=texture,size=dimensions,packed_payload_sha256=hashlib.sha256(expected).hexdigest()))
    assert report
    assert any(c.name=='REF_ORIGINAL' and c.hide_render for c in bpy.data.collections)
    (folder/'validation/packed-textures.json').write_text(json.dumps(dict(status='PASS',images=report,reference_collection='REF_ORIGINAL hidden and retained'),indent=2))
if __name__=='__main__':check(ROOT/'Bonfire01')
