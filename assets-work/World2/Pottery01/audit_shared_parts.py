"""Read-only affine/UV correspondence of whole Object28 pots to Object29."""
from pathlib import Path
import json
import sys
sys.dont_write_bytecode = True
import numpy as np
from audit_exports import triangles
ROOT = Path(__file__).resolve().parent


def signature(face):
    material, rows = face
    values = [tuple(row[7:9]) for row in rows]
    shift = min(range(3), key=lambda k: tuple(values[k:] + values[:k]))
    return (material, tuple(values[shift:] + values[:shift])), rows[shift:] + rows[:shift]


def main():
    reference = triangles(ROOT / 'Object29/validation/baseline/Object29.smd')
    composite = triangles(ROOT / 'Object28/validation/baseline/Object28.smd')
    components = json.loads((ROOT / 'independent-component-inspection.json').read_text())['Object28']['components_at_1e-3']
    lookup = {}
    for index, face in enumerate(reference):
        key, rows = signature(face)
        assert key not in lookup, 'Ambiguous canonical UV topology'
        lookup[key] = (index, rows)
    results = []
    for part in components:
        if part['triangles'] != len(reference):
            continue
        before, after, matches = [], [], []
        used = set()
        for index in part['indices']:
            key, rows = signature(composite[index])
            assert key in lookup
            old_index, old_rows = lookup[key]
            assert old_index not in used
            used.add(old_index)
            assert all(row[0] == old[0] for row, old in zip(rows, old_rows))
            before.extend([row[1:4] + [1] for row in old_rows])
            after.extend([row[1:4] for row in rows])
            matches.append(dict(canonical_triangle=old_index, composite_triangle=index))
        transform = np.linalg.lstsq(np.asarray(before), np.asarray(after), rcond=None)[0]
        errors = np.linalg.norm(np.asarray(before) @ transform - np.asarray(after), axis=1)
        assert errors.max() < .0003
        results.append(dict(original_indices=part['indices'], affine_row_vector_4x3=transform.tolist(),
                            maximum_position_fit_error=float(errors.max()), maximum_uv_error=0,
                            material_bone_cyclic_winding='EXACT', matches=matches))
    assert len(results) == 3
    (ROOT / 'shared-whole-pot-proof.json').write_text(json.dumps(dict(status='PASS', parts=results,
        criterion='Each whole-pot triangle matches unique canonical material/UV/bone/cyclic topology, then all corners fit one affine transform'), indent=2))


if __name__ == '__main__':
    main()
