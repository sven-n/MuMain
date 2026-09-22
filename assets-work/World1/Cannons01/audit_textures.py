"""Check every copied atlas/dependency byte and every readonly model provenance."""
import hashlib
import json
import sys
sys.dont_write_bytecode=True
from PIL import Image
from config import ASSETS,COMPATIBILITY_ASSETS,CONTAINERS,FROZEN,MATERIALS,ROOT
result={}
for filename,container in CONTAINERS.items():
    path=ROOT/'textures/final'/container;digest=hashlib.sha256(path.read_bytes()).hexdigest()
    if container in FROZEN:assert path.read_bytes()==(ROOT/'textures/original'/container).read_bytes(),container
    consumers=[name for name in ASSETS if filename in MATERIALS[name]]
    for name in consumers:assert (ROOT/name/'exports'/container).read_bytes()==path.read_bytes()
    painting=Image.open(ROOT/'textures/final'/filename)
    result[container]=dict(sha256=digest,dimensions=list(painting.size),mode=painting.mode,consumers=consumers,frozen=container in FROZEN)
assert len(result['horse_drawn_01.OZJ']['consumers'])==12
for name in COMPATIBILITY_ASSETS:
    assert (ROOT/name/f'exports/{name}.bmd').read_bytes()==(ROOT/name/f'original/{name}.bmd').read_bytes()
(ROOT/'texture-dependency-audit.json').write_text(json.dumps(dict(status='PASS',readonly_BMDs_byte_identical=list(COMPATIBILITY_ASSETS),textures=result),indent=2)+'\n')
print('PASS all12 shared atlas consumers,16 frozen containers,8 readonly BMDs')
