"""Audit visible imported objects and texture wiring to diagnose offline artifacts."""
import bpy
import json
from pathlib import Path
root = Path(__file__).resolve().parent / 'Sign01'
result = {}
for stage, relative in [('before', 'original/source.blend'), ('after', 'validation/reimported.blend')]:
    bpy.ops.wm.open_mainfile(filepath=str(root / relative))
    result[stage] = [dict(name=obj.name, type=obj.type, helper=bool(obj.get('mu_helper')),
        hide_render=obj.hide_render, location=list(obj.location),
        materials=[dict(name=m.name, textures=[n.image.filepath for n in m.node_tree.nodes
            if n.type=='TEX_IMAGE' and n.image]) for m in obj.data.materials if m]
            if obj.type=='MESH' else []) for obj in bpy.data.objects]
(root / 'validation/render-object-audit.json').write_text(json.dumps(result, indent=2)+'\n')
