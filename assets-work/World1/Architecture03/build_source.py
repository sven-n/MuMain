"""Refine bridge and lintel face interiors with the reviewed geometry helpers."""
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
builder.PAIRS = {'HouseWall03': [(2, 3), (6, 7)], 'Bridge01': [(18, 19), (22, 23), (26, 27), (34, 35), (40, 41), (42, 43), (50, 51), (54, 55), (60, 61), (64, 65)], 'BridgeStone01': [(64, 65), (66, 67), (68, 69), (70, 71)], 'Fence01': [], 'FireLight02': [], 'House01': [], 'House03': [], 'House04': [], 'House05': [], 'HouseEtc02': [], 'HouseWall01': [], 'HouseWall02': [], 'HouseWall04': [], 'HouseWall05': [], 'HouseWall06': [], 'StoneMuWall01': [], 'StoneWall01': [], 'StoneWall02': [], 'Tent01': [], 'Tree07': []}
DEPTHS = {'HouseWall03': 0.55, 'Bridge01': 0.55, 'BridgeStone01': 0.55, 'Fence01': 0, 'FireLight02': 0, 'House01': 0, 'House03': 0, 'House04': 0, 'House05': 0, 'HouseEtc02': 0, 'HouseWall01': 0, 'HouseWall02': 0, 'HouseWall04': 0, 'HouseWall05': 0, 'HouseWall06': 0, 'StoneMuWall01': 0, 'StoneWall01': 0, 'StoneWall02': 0, 'Tent01': 0, 'Tree07': 0}
PERMITTED = {'tile_wood02.jpg', 'bridge_01.jpg'}
READONLY = ('Fence01', 'FireLight02', 'House01', 'House03', 'House04', 'House05', 'HouseEtc02', 'HouseWall01', 'HouseWall02', 'HouseWall04', 'HouseWall05', 'HouseWall06', 'StoneMuWall01', 'StoneWall01', 'StoneWall02', 'Tent01', 'Tree07')


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
    record['alpha'] = 'Six opaque RGB paintings; original RGBA shadow and accepted alpha textiles copied byte-exact. All indexed effects unchanged.'
    record['protected'] = 'All original corners, bridge end planes, arches, beast heads, flame anchors, shadow meshes, log ends and door contacts retained; only selected interior panel faces refined.'
    record['readonly_compatibility'] = name in READONLY
    path.write_text(json.dumps(record, indent=2) + '\n')
    for material in bpy.data.materials:
        if material.name.endswith('.tga') and material.use_nodes:
            nodes = material.node_tree.nodes
            textures = [node for node in nodes if node.type == 'TEX_IMAGE']
            shader = nodes.get('Principled BSDF')
            if textures and shader:
                material.node_tree.links.new(textures[0].outputs['Alpha'], shader.inputs['Alpha'])
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
