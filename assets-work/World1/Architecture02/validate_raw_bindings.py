"""Audit raw BMD normal ownership using the reviewed CartHay read-only parser."""
import hashlib
import json
from pathlib import Path
import sys

sys.dont_write_bytecode = True
from raw_bindings import audit

ROOT = Path(__file__).resolve().parent
READONLY = ('House04', 'HouseWall02', 'HouseEtc02')

for original in sorted(ROOT.glob('*/original')):
    folder, name = original.parent, original.parent.name
    before = audit(original / f'{name}.bmd')
    after = audit(folder / f'exports/{name}.bmd')
    for old, new in zip(before['mesh_bindings'], after['mesh_bindings']):
        assert old['mesh'] == new['mesh'] and old['material'] == new['material']
        mismatch = new['vertex_to_normal_node_mismatch_corner_counts']
        if name in READONLY:
            assert old == new
        else:
            assert not mismatch, (name, mismatch)
    result = dict(status='PASS', before=before, after=after,
                  conclusion='All authored model corners use the intended vertex bone for their raw normal as well. No cross-bone normal deduplication occurred. Readonly bindings are unchanged.',
                  helper='Read-only raw_bindings.py copied from parallel CartHay01 worker; follows engine v10/v12 disk structs and MapFileDecrypt',
                  helper_sha256=hashlib.sha256((ROOT / 'raw_bindings.py').read_bytes()).hexdigest())
    (folder / 'validation/raw-bone-bindings.json').write_text(json.dumps(result, indent=2) + '\n')
    print(name, 'raw vertex/normal ownership PASS')
