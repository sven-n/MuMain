"""Import eight original candidates through the official importer, then render and inspect."""

import json
from pathlib import Path
import sys
import traceback

import bpy

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(REPOSITORY / 'tools/blender'))
import mu_bmd_import
from review_scene import mesh_bounds, render, set_camera, set_lighting

SCRATCH = Path('/tmp/astra-world1-candidates')


def inspect_meshes(objects):
    results = []
    for obj in objects:
        groups = {}
        for vertex in obj.data.vertices:
            for group in vertex.groups:
                name = obj.vertex_groups[group.group].name
                groups.setdefault(name, []).append(tuple(obj.matrix_world @ vertex.co))
        bone_bounds = {name: dict(vertices=len(points),
                                 min=[min(p[a] for p in points) for a in range(3)],
                                 max=[max(p[a] for p in points) for a in range(3)])
                       for name, points in groups.items()}
        results.append(dict(name=obj.name, bounds=mesh_bounds([obj]),
                            triangles=sum(len(p.vertices) - 2 for p in obj.data.polygons),
                            materials=[m.name for m in obj.data.materials], bones=bone_bounds))
    return results


def inspect(name):
    folder = SCRATCH / name
    folder.mkdir(parents=True, exist_ok=True)
    sys.argv = ['blender', '--', '--bmd', str(REPOSITORY / f'src/bin/Data/Object1/{name}.bmd'),
                '--out', str(folder / 'source.blend')]
    mu_bmd_import.main()
    objects = [obj for obj in bpy.context.scene.objects if obj.type == 'MESH' and not obj.get('mu_helper')]
    bpy.context.view_layer.update()
    report = dict(meshes=inspect_meshes(objects), images={im.name:list(im.size) for im in bpy.data.images})
    (ROOT / f'candidates/{name}.json').write_text(json.dumps(report, indent=2) + '\n')
    set_camera(mesh_bounds(objects))
    set_lighting()
    render(ROOT / f'candidates/{name}.png')


def main():
    (ROOT / 'candidates').mkdir(exist_ok=True)
    for name in json.loads((ROOT / 'inventory.json').read_text())['candidates']:
        inspect(name)


if __name__ == '__main__':
    try:
        main()
    except Exception:
        traceback.print_exc()
        sys.exit(1)
