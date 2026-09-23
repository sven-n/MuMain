"""Prove actual exported composite instances against the canonical exported pot."""
from pathlib import Path
import json
import sys
sys.dont_write_bytecode = True
import numpy as np
from audit_exports import triangles, same
ROOT = Path(__file__).resolve().parent
POSITION_TOLERANCE = .0003
UV_TOLERANCE = 1e-6


def closest(expected, candidates, used):
    material, rows = expected
    matches = []
    for index, (other_material, other_rows) in enumerate(candidates):
        if index in used or material != other_material:
            continue
        for shift in range(3):
            corners = other_rows[shift:] + other_rows[:shift]
            if any(a[0] != b[0] for a, b in zip(rows, corners)):
                continue
            uv_error = max(np.linalg.norm(np.array(a[7:9])-b[7:9]) for a,b in zip(rows,corners))
            if uv_error >= UV_TOLERANCE:
                continue
            position_error = max(np.linalg.norm(np.array(a[1:4])-b[1:4]) for a,b in zip(rows,corners))
            if position_error < POSITION_TOLERANCE:
                matches.append((index, float(position_error), float(uv_error)))
    assert len(matches) == 1, (material, len(matches))
    return matches[0]


def main():
    canonical = triangles(ROOT / 'Object29/validation/new/Object29.smd')
    composite = triangles(ROOT / 'Object28/validation/new/Object28.smd')
    baseline = triangles(ROOT / 'Object28/validation/baseline/Object28.smd')
    parts = json.loads((ROOT / 'shared-whole-pot-proof.json').read_text())['parts']
    used, results = set(), []
    for number, part in enumerate(parts):
        matrix = np.asarray(part['affine_row_vector_4x3'])
        maximum_position = maximum_uv = 0
        indices = []
        for material, rows in canonical:
            expected = [row[:1] + (np.asarray(row[1:4]+[1]) @ matrix).tolist() + row[4:] for row in rows]
            index, position_error, uv_error = closest((material, expected), composite, used)
            used.add(index)
            indices.append(index)
            maximum_position = max(maximum_position, position_error)
            maximum_uv = max(maximum_uv, uv_error)
        results.append(dict(part=number, triangles=len(indices), maximum_position_error=maximum_position,
                            maximum_uv_error=maximum_uv, material_bone_cyclic_winding='EXACT', exported_indices=indices))
    original_whole = {index for part in parts for index in part['original_indices']}
    broken_indices = [index for index in range(len(baseline)) if index not in original_whole]
    for index in broken_indices:
        candidates = [i for i, face in enumerate(composite) if i not in used and same(baseline[index], face)]
        assert len(candidates) == 1, (index, candidates)
        used.add(candidates[0])
    assert len(used) == len(composite)
    assert '0 "Cylinder05" -1' in (ROOT / 'Object29/validation/new/Object29.smd').read_text()
    assert '0 "Cylinder03" -1' in (ROOT / 'Object28/validation/new/Object28.smd').read_text()
    (ROOT / 'exported-variant-proof.json').write_text(json.dumps(dict(status='PASS', parts=results,
        preserved_broken_triangles=len(broken_indices), all_candidate_triangles_accounted=len(used),
        bone_mapping='Original Cylinder05 canonical to original Cylinder03 composite, unchanged index 0'), indent=2))


if __name__ == '__main__':
    main()
