"""Measure original rigid parts and disconnected wooden construction pieces."""
from pathlib import Path
import json
import sys
sys.dont_write_bytecode = True
import bpy
ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT))
from config import ASSETS


def bounds(points):
    return [[min(p[a] for p in points) for a in range(3)],
            [max(p[a] for p in points) for a in range(3)]]


for name in ASSETS:
    bpy.ops.wm.open_mainfile(filepath=str(ROOT / name / 'original/source.blend'))
    result = []
    for obj in bpy.context.scene.objects:
        if obj.type != 'MESH' or obj.get('mu_helper'):
            continue
        for group in obj.vertex_groups:
            for index, material in enumerate(obj.data.materials):
                polygons = [p for p in obj.data.polygons if p.material_index == index
                            and all(any(g.group == group.index for g in obj.data.vertices[v].groups)
                                    for v in p.vertices)]
                indices = {v for p in polygons for v in p.vertices}
                if not indices:
                    continue
                points = {i: list(obj.matrix_world @ obj.data.vertices[i].co) for i in indices}
                record = dict(bone=group.name, material=material.name, triangles=len(polygons),
                              bounds=bounds(list(points.values())))
                if material.name in ('bookshelf.jpg', 'chair2.tga'):
                    record['vertices'] = points
                    record['faces'] = [list(p.vertices) for p in polygons]
                result.append(record)
    (ROOT / name / 'original/parts.json').write_text(json.dumps(result, indent=2) + '\n')
