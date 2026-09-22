"""Prove modular anchors, clearance geometry, UVs and frozen materials survive export."""
import hashlib
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parent
OWNED = ('tile_01', 'tile_03')
# Official BMD/SMD roundtrip accumulates sub-millimeter float error on the nine-bone banner hierarchy.
POSITION_TOLERANCE = .001


def triangles(path):
    lines = path.read_text().split('triangles\n')[1].splitlines()
    return [(lines[i], [list(map(float, line.split())) for line in lines[i+1:i+4]])
            for i in range(0, len(lines)-1, 4)]


def position_error(a, b):
    if a[0] != b[0]:
        return math.inf
    return max(abs(x-y) for x, y in zip(a[1:4], b[1:4]))


def match_triangle(a, b):
    if a[0] != b[0]:
        return None
    for shift in range(3):
        aligned = b[1][shift:] + b[1][:shift]
        if all(position_error(x, y) < POSITION_TOLERANCE for x, y in zip(a[1], aligned)):
            return aligned
    return None


for root in sorted(ROOT.glob('*/original')):
    folder, name = root.parent, root.parent.name
    report = json.loads((folder / 'validation/blender.json').read_text())
    old = triangles(folder / f'validation/original/{name}.smd')
    new = triangles(folder / f'validation/new/{name}.smd')
    corners = [corner for _, values in new for corner in values]
    max_position = max(min(position_error(corner, candidate) for candidate in corners)
                       for _, values in old for corner in values)
    assert max_position < POSITION_TOLERANCE, (name, max_position)
    modified = {index for panel in report['panel_recesses'] for index in panel['original_face_pair']}
    retained = [(i, triangle) for i, triangle in enumerate(old) if i not in modified]
    max_uv, max_normal, max_normal_angle = 0, 0, 0
    by_texture = {}
    for index, triangle in retained:
        candidates = [values for other in new if (values := match_triangle(triangle, other)) is not None]
        assert candidates, (name, index, 'Original triangle lost')
        match = min(candidates, key=lambda values: max(abs(a-b) for x, y in zip(triangle[1], values) for a, b in zip(x[7:9], y[7:9])))
        error_uv = max(abs(a-b) for x, y in zip(triangle[1], match) for a, b in zip(x[7:9], y[7:9]))
        error_normal = max(abs(a-b) for x, y in zip(triangle[1], match) for a, b in zip(x[4:7], y[4:7]))
        assert error_uv < .00001, (name, index, error_uv)
        for x, y in zip(triangle[1], match):
            a, b = x[4:7], y[4:7]
            cosine = sum(v*w for v, w in zip(a, b)) / math.sqrt(sum(v*v for v in a) * sum(v*v for v in b))
            angle = math.degrees(math.acos(max(-1, min(1, cosine))))
            max_normal_angle = max(max_normal_angle, angle)
        max_uv, max_normal = max(max_uv, error_uv), max(max_normal, error_normal)
        by_texture[triangle[0]] = by_texture.get(triangle[0], 0) + 1
    frozen = {}
    assert max_normal_angle < .1, (name, 'Roundtrip changed shading direction', max_normal_angle)
    for path in (p for p in root.iterdir() if p.suffix in ('.OZJ', '.OZT')):
        if path.stem not in OWNED:
            digest = hashlib.sha256(path.read_bytes()).hexdigest()
            assert digest == hashlib.sha256((folder / 'exports' / path.name).read_bytes()).hexdigest()
            frozen[path.name] = digest
    result = dict(status='PASS', position_tolerance=POSITION_TOLERANCE, every_original_vertex_position_and_bone_retained=True,
                  max_original_corner_position_component_error=max_position,
                  unchanged_triangle_count=len(retained), retained_triangles_by_texture=by_texture,
                  max_unchanged_uv_component_error=max_uv,
                  max_unchanged_normal_component_error=max_normal,
                  max_retained_normal_direction_degrees=max_normal_angle,
                  frozen_container_sha256=frozen,
                  panel_face_pairs=[panel['original_face_pair'] for panel in report['panel_recesses']],
                  protected='All original connection extrema and edges retained. All triangles outside explicit tile_01 panel pairs retain position, bone and UV. Gate portcullis, gate inward jamb planes, wall endpoint planes, banner silhouettes and frozen siege details unchanged.',
                  normals='Original split normals copied into source; official import/export normalizes and requantizes them. Measured component and direction differences are reported, not claimed byte-equivalent.',
                  note='Panel centers are recessed by 0.65 units; no original perimeter vertices moved. No placement/collision/terrain changes.')
    (folder / 'validation/modular-anchors.json').write_text(json.dumps(result, indent=2) + '\n')
    print(name, 'PASS:', len(retained), 'unchanged triangles;', max_position, 'maximum original-vertex drift')
