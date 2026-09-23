"""Rebuild the accepted Object01 study using retained baseline blobs and official tools."""
from pathlib import Path
import hashlib,json,os,subprocess,sys
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parent;sys.path.insert(0,str(ROOT))
import pipeline
name='Object01';folder=ROOT/name
expected=json.loads((ROOT/'baseline-evidence.json').read_text())[name]['sha256']
assert hashlib.sha256((folder/'baseline'/f'{name}.bmd').read_bytes()).hexdigest()==expected
for texture,digest in json.loads((ROOT/'frozen-textures.json').read_text()).items():
    assert hashlib.sha256((folder/'baseline'/texture).read_bytes()).hexdigest()==digest
pipeline.prepare(name)
pipeline.blender(['--python',ROOT/'build_wall3.py'],folder/'validation/build.txt',name)
pipeline.export(name)
for script in ('validate.py','contact_proof.py','affine_uv_proof.py'):
    subprocess.run([sys.executable,str(ROOT/script)],env={**os.environ,'WALL_NAMES':name},check=True)
pipeline.blender(['--python',ROOT/'audit_source.py'],folder/'validation/source-audit.txt',name)
pipeline.blender(['--python',ROOT/'review.py'],folder/'review/render.txt',name)
pipeline.blender(['--python',ROOT/'final_evidence.py'],folder/'review/final-evidence.txt',name)
for stage in ('baseline','candidate'):
    os.environ['WALL_STAGE']=stage;pipeline.blender(['--python',ROOT/'context_review.py'],ROOT/'review'/f'{stage}-render.txt',name)
