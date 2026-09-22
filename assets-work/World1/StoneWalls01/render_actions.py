"""Render matching samples of the preserved 25-key heraldic banner action."""
import json
from pathlib import Path
import sys

sys.dont_write_bytecode = True
import bpy
ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT))
import render_exports

folder = ROOT / 'StoneWall06'
bounds = json.loads((folder / 'validation/blender.json').read_text())['bounds_before']
for stage in ('before', 'after'):
    render_exports.review.open_model(folder, stage, bounds)
    for frame in (0, 12, 24):
        bpy.context.scene.frame_set(frame)
        render_exports.render(folder / f'review/action-{stage}-{frame}.png')
(folder / 'review/action-context.json').write_text(json.dumps(dict(
    status='OFFLINE BLENDER; CLIENT VERIFICATION PENDING',
    samples=[0, 12, 24], total_original_keys=25,
    source='Original BMD and reimported actual exported BMD; same camera and diffuse lighting'), indent=2) + '\n')
