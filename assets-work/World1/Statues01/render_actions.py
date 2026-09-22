"""Compare every animated identity at matching beginning, middle and ending keys."""
import json
from pathlib import Path
import sys
sys.dont_write_bytecode = True
import bpy
ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT))
from render_exports import review, render, hide_import_helpers, preview_alpha_test

SAMPLES = {'Waterspout01': (0,10,20)}

for name, frames in SAMPLES.items():
    folder = ROOT / name
    bounds = json.loads((folder / 'validation/blender.json').read_text())['bounds_before']
    for stage in ('before', 'after'):
        source = folder / ('original/source.blend' if stage == 'before' else 'validation/reimported.blend')
        bpy.ops.wm.open_mainfile(filepath=str(source))
        bpy.context.scene.frame_set(0)
        hide_import_helpers()
        preview_alpha_test()
        review.set_camera(bounds)
        review.set_lighting()
        bpy.context.scene.camera.data.ortho_scale *= 1.2
        for frame in frames:
            bpy.context.scene.frame_set(frame)
            render(folder / f'review/action-{stage}-{frame}.png')
    (folder / 'review/action-context.json').write_text(json.dumps(dict(
        kind='OFFLINE BLENDER; NOT CLIENT EVIDENCE', samples=frames,
        camera='Identical original-bound orthographic camera before/after',
        after='Actual installed-shape BMD reimported with final textures'), indent=2)+'\n')
