"""First crown direction study through the unchanged official game export pipeline."""
from pathlib import Path
import sys
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parent;sys.path.insert(0,str(ROOT))
import pipeline
name='Object01';folder=ROOT/name
pipeline.blender(['--python',ROOT/'build_wall.py'],folder/'validation/build.txt',name)
pipeline.export(name)
pipeline.blender(['--python',ROOT/'review.py'],folder/'review/render.txt',name)
