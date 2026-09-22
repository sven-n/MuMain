"""Confirm actual imported animation drives the three fountain review frames."""
import json
from pathlib import Path
import sys
import bpy
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))
from config import ROOT

report = {}
for state, filename in [('original', 'original/source.blend'), ('final', 'validation/reimported.blend')]:
    bpy.ops.wm.open_mainfile(filepath=str(ROOT / 'Waterspout01' / filename))
    rig = next(obj for obj in bpy.context.scene.objects if obj.type == 'ARMATURE')
    assert rig.animation_data and rig.animation_data.action
    frames = {}
    for frame in (0, 10, 20):
        bpy.context.scene.frame_set(frame)
        frames[frame] = {bone.name: [list(row) for row in bone.matrix] for bone in rig.pose.bones}
    assert frames[0] != frames[10]
    report[state] = {'action': rig.animation_data.action.name, 'frames': frames}
assert report['original']['frames'] == report['final']['frames']
report['status'] = 'PASS: active action changes pose; original/final matrices exact at rendered frames'
(ROOT / 'Waterspout01/validation/action-render-audit.json').write_text(json.dumps(report, indent=2) + '\n')
print(report['status'])
