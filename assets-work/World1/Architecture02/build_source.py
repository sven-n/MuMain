"""Refine masonry panels, roof interiors and ladder wear with the reviewed geometry helpers."""
import importlib.util
import json
from pathlib import Path
import sys
import traceback

sys.dont_write_bytecode = True
import bpy

ROOT = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('masonry_build', ROOT.parent / 'Masonry01/build_source.py')
builder = importlib.util.module_from_spec(spec)
spec.loader.exec_module(builder)
builder.ROOT = ROOT
builder.PAIRS = {
    'House05': [(i, i + 1) for i in range(228, 240, 2)],
    'HouseWall01': [(0, 1), (2, 3)],
    'HouseWall04': [(0, 1), (2, 3), (6, 7), (8, 9)],
    'HouseWall05': [(0, 1), (18, 19), (22, 23)],
    'HouseWall06': [(0, 1), (18, 19), (22, 23)],
    'Stair01': [(4, 5), (8, 9), (70, 71), (74, 75)] + [(i, i + 1) for i in range(14, 63, 6)],
    'House04': [], 'HouseWall02': [], 'HouseEtc02': [],
}
DEPTHS = {'House05': .65, 'HouseWall01': .65, 'HouseWall04': .65,
          'HouseWall05': .4, 'HouseWall06': .4, 'Stair01': .18,
          'House04': 0, 'HouseWall02': 0, 'HouseEtc02': 0}
PERMITTED = {'tile_wood01.jpg', 'tile_wood02.jpg'}
READONLY = ('House04', 'HouseWall02', 'HouseEtc02')


def perimeter(original, pair):
    faces = [original.data.polygons[i] for i in pair]
    assert len({face.material_index for face in faces}) == 1
    assert original.data.materials[faces[0].material_index].name.removeprefix('REF_') in PERMITTED
    edges, points = [], {}
    for face in faces:
        for loop in face.loop_indices:
            index = original.data.loops[loop].vertex_index
            value = original.data.uv_layers[0].data[loop].uv.copy()
            if index in points:
                assert (points[index] - value).length < .00001, (original.name, pair, 'UV seam')
            points[index] = value
        edges += list(zip(face.vertices, list(face.vertices[1:]) + [face.vertices[0]]))
    edges = [edge for edge in edges if (edge[1], edge[0]) not in edges]
    assert len(edges) == 4
    boundary = [edges[0][0], edges[0][1]]
    while len(boundary) < 4:
        boundary.append(next(b for a, b in edges if a == boundary[-1]))
    return boundary, points, faces[0].material_index


def finish_source(name):
    path = ROOT / name / 'validation/blender.json'
    record = json.loads(path.read_text())
    record['alpha'] = 'All textures opaque RGB; water and accepted effects retain original indexed geometry and materials'
    record['protected'] = 'Every original corner retained. Modular end faces, roof edges, ladder contacts, waterwheel and scrolling water geometry remain unchanged; only selected fixed panel interiors refined.'
    record['readonly_compatibility'] = name in READONLY
    path.write_text(json.dumps(record, indent=2) + '\n')
    bpy.ops.wm.save_as_mainfile(filepath=str(ROOT / name / 'source.blend'))


if __name__ == '__main__':
    try:
        builder.perimeter = perimeter
        for name in builder.PAIRS:
            builder.RECESS_DEPTH = DEPTHS[name]
            builder.PANEL_BEVEL_WIDTH = .45 if name == 'Stair01' else 2
            builder.build(name)
            finish_source(name)
    except Exception:
        traceback.print_exc()
        sys.exit(1)
