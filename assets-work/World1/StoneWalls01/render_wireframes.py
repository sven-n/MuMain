"""Render legible wireframes from the actual BMD exports without proxy scenery."""
import json
from pathlib import Path
import sys
sys.dont_write_bytecode = True
import bpy
ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT))
from render_exports import NAMES, review, render

for name in NAMES:
    folder = ROOT / name
    bounds = json.loads((folder / 'validation/blender.json').read_text())['bounds_before']
    review.open_model(folder, 'after', bounds)
    wire = review.wire_material()
    next(node for node in wire.node_tree.nodes if node.type == 'WIREFRAME').inputs['Size'].default_value = .65
    bpy.context.view_layer.material_override = wire
    render(folder / 'review/wireframe.png')
