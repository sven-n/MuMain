"""Build, officially export and audit the body before matched directional review."""
from pathlib import Path
import sys,subprocess,os
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parent;sys.path.insert(0,str(ROOT))
import pipeline
name='Object21';folder=ROOT/name
pipeline.blender(['--python',ROOT/'build_body.py'],folder/'validation/build.txt',name)
pipeline.export(name)
env={**os.environ,'COFFIN_NAMES':name}
for script in ('validate.py','contact_proof.py'):subprocess.run([sys.executable,str(ROOT/script)],env=env,check=True)
pipeline.blender(['--python',ROOT/'audit_source.py'],folder/'validation/source-audit.txt',name)
pipeline.blender(['--python',ROOT/'review.py'],folder/'review/render.txt',name)
