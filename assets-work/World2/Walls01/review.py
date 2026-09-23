"""Actual exported wall against unchanged baseline with matching uncropped cameras."""
from pathlib import Path
import sys,os,json
sys.dont_write_bytecode=True
import bpy
ROOT=Path(__file__).resolve().parent;sys.path.insert(0,str(ROOT))
from inspect_baseline import setup,views
name=os.environ.get('WALL_NAME','Object01');folder=ROOT/name
bounds=json.loads((folder/'validation/authored.json').read_text())['bounds_before']
for stage,path in [('baseline','baseline/source.blend'),('candidate','validation/reimported.blend')]:
    bpy.ops.wm.open_mainfile(filepath=str(folder/path));bpy.context.scene.frame_set(0);setup(bounds);views(bounds,folder/'review',stage)
