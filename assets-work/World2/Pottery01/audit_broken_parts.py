"""Read-only exact shared UV topology and affine mapping of the broken variants."""
from pathlib import Path
import json
import sys
sys.dont_write_bytecode = True
import numpy as np
from audit_exports import triangles
from audit_shared_parts import signature
ROOT = Path(__file__).resolve().parent


def component(name, count, parts):
    faces = triangles(ROOT / name / 'validation/baseline' / (name + '.smd'))
    indices = next(part['indices'] for part in parts[name]['components_at_1e-3'] if part['triangles'] == count)
    lookup = {}
    for index in indices:
        key, rows = signature(faces[index])
        assert key not in lookup
        lookup[key] = (index, rows)
    return lookup


def match(left, right):
    keys = left.keys() & right.keys()
    a, b, records = [], [], []
    for key in sorted(keys):
        old_index, old_rows = left[key]
        new_index, new_rows = right[key]
        assert all(old[0] == new[0] for old, new in zip(old_rows, new_rows))
        a.extend([row[1:4] + [1] for row in old_rows])
        b.extend([row[1:4] for row in new_rows])
        records.append(dict(object28_triangle=old_index, object30_triangle=new_index))
    transform = np.linalg.lstsq(a, b, rcond=None)[0]
    errors = np.linalg.norm(np.asarray(a) @ transform - np.asarray(b), axis=1)
    return dict(shared_triangles=len(keys), maximum_affine_position_error=float(errors.max()),
                affine_row_vector_4x3=transform.tolist(), matches=records,
                only_object28=[left[key][0] for key in left.keys() - keys],
                only_object30=[right[key][0] for key in right.keys() - keys])


def main():
    parts = json.loads((ROOT / 'independent-component-inspection.json').read_text())
    shared = match(component('Object28', 174, parts), component('Object30', 174, parts))
    assert shared['shared_triangles'] == 174 and shared['maximum_affine_position_error'] < .0003
    different = match(component('Object28', 108, parts), component('Object30', 114, parts))
    (ROOT / 'shared-broken-pot-proof.json').write_text(json.dumps(dict(status='PASS',
        complete_shared_part=shared, different_variant=different,
        restriction='Different variant is not interchangeable; unmatched triangles require preservation/individual design'), indent=2))


if __name__ == '__main__':
    main()
