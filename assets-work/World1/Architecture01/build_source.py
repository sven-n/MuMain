"""Conservative roof, shutter, timber-panel refinement using the reviewed masonry helpers."""
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
    'House01': [(0, 1), (2, 3), (6, 7), (8, 9)],
    'House03': [(0, 1), (6, 7), (8, 9)],
    'House04': [(314, 315), (316, 317)],
    'Tent01': [(28, 29), (30, 31), (38, 39), (40, 41),
               (48, 49), (50, 51), (58, 59), (60, 61)],
    'HouseWall02': [(20, 21), (22, 23), (26, 27), (38, 39), (52, 53)],
    'HouseEtc02': [],
}
DEPTHS = {'House01': .65, 'House03': .65, 'House04': .4, 'Tent01': .25, 'HouseWall02': .4, 'HouseEtc02': 0}
PERMITTED = {'tile_ston05.jpg', 'tile_ston04.jpg', 'tile_windows01.jpg', 'tile_ston06.jpg', 'tile_wood01.jpg', 'tile_wood02.jpg'}


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
    for material in bpy.data.materials:
        if material.name != 'tile_ston07.tga':
            continue
        nodes = material.node_tree.nodes
        texture = next(node for node in nodes if node.type == 'TEX_IMAGE')
        material.node_tree.links.new(texture.outputs['Alpha'], nodes.get('Principled BSDF').inputs['Alpha'])
    path = ROOT / name / 'validation/blender.json'
    record = json.loads(path.read_text())
    record['alpha'] = 'House03 awning uses original RGBA cutout mask; all other textures opaque RGB; effects retain original additive indices'
    record['protected'] = 'Every original corner retained; only explicitly selected non-effect panel interiors refined. Footprints, roof perimeters, openings, awning, canopy and effect meshes unchanged.'
    record['readonly_compatibility'] = name == 'HouseEtc02'
    path.write_text(json.dumps(record, indent=2) + '\n')
    bpy.ops.wm.save_as_mainfile(filepath=str(ROOT / name / 'source.blend'))


if __name__ == '__main__':
    try:
        builder.perimeter = perimeter
        for name in builder.PAIRS:
            builder.RECESS_DEPTH = DEPTHS[name]
            builder.PANEL_BEVEL_WIDTH = 1 if name == 'Tent01' else 2
            builder.build(name)
            finish_source(name)
    except Exception:
        traceback.print_exc()
        sys.exit(1)
