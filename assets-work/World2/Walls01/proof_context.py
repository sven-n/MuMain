"""Strict exported wall checks and fresh-process actual placement comparisons."""
from pathlib import Path
import sys,os,subprocess
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parent;sys.path.insert(0,str(ROOT))
import pipeline
name='Object01';folder=ROOT/name
for script in ('validate.py','contact_proof.py'):
    subprocess.run([sys.executable,str(ROOT/script)],env={**os.environ,'WALL_NAMES':name},check=True)
pipeline.blender(['--python',ROOT/'audit_source.py'],folder/'validation/source-audit.txt',name)
for stage in ('baseline','candidate'):
    os.environ['WALL_STAGE']=stage
    pipeline.blender(['--python',ROOT/'context_review.py'],ROOT/'review'/f'{stage}-render.txt',name)
