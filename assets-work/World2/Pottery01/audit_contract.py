"""Protect the original throat, base and all actually placed orientation extents."""
from pathlib import Path
import json
import math
import sys
sys.dont_write_bytecode = True
import bpy
from mathutils import Vector, Euler
ROOT = Path(__file__).resolve().parent
from audit_exports import triangles, same


def check(folder):
    name = folder.name
    old = triangles(folder / 'validation/baseline' / (name + '.smd'))
    new = triangles(folder / 'validation/new' / (name + '.smd'))
    indices = json.loads((folder / 'validation/source.json').read_text())['protected_triangles']
    normal_error = 0
    for index in indices:
        matches = [face for face in new if same(old[index], face)]
        assert len(matches) == 1, (index, len(matches))
        face = matches[0]
        shift = min(range(3), key=lambda s: max(math.dist(x[1:4], face[1][(i+s)%3][1:4]) for i,x in enumerate(old[index][1])))
        normal_error = max(normal_error, max(math.dist(x[4:7], face[1][(i+shift)%3][4:7]) for i,x in enumerate(old[index][1])))
    assert normal_error < .0003
    old_points = [Vector(row[1:4]) for _, rows in old for row in rows]
    new_points = [Vector(row[1:4]) for _, rows in new for row in rows]
    records = []
    for placement in json.loads((folder / 'placements.json').read_text()):
        matrix = Euler([math.radians(v) for v in placement['rotation']], 'XYZ').to_matrix()
        before = [matrix @ p * placement['scale'] for p in old_points]
        after = [matrix @ p * placement['scale'] for p in new_points]
        error = max(abs(operation(p[k] for p in before) - operation(p[k] for p in after))
                    for operation in (min, max) for k in range(3))
        assert error < .001, (placement['index'], error)
        records.append(dict(index=placement['index'], maximum_placed_bound_error=error))
    (folder / 'validation/protected-contacts.json').write_text(json.dumps(dict(status='PASS',
        protected_triangles=indices, maximum_protected_corner_normal_error=normal_error,
        actual_placement_bounds=records), indent=2))
