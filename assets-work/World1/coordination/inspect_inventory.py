"""Import and render the integration baseline inventory for offline identity and dependency review."""

import json
from pathlib import Path
import sys

import bpy

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
sys.path.insert(0, str(ROOT / 'tools/blender'))
sys.path.insert(0, str(HERE.parent / 'StaticBatch01'))
import mu_bmd_import
from review_scene import mesh_bounds, set_camera, set_lighting

CONVERTER = '/Users/webproduktion3/Documents/claude-test-mumain/MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv'
RENDER_SIZE = (400, 360)


def inspect(name, record):
    output = HERE / 'inspection' / name
    output.mkdir(parents=True, exist_ok=True)
    if (output / 'baseline-offline.png').exists():
        return
    archived = HERE / 'baseline-validation' / name / (name + '.bmd')
    sys.argv = ['blender', '--', '--bmd', str(archived), '--textures', str(HERE / 'texture-baseline'), '--bmdconv', CONVERTER,
                '--out', str(output / 'import.blend')]
    mu_bmd_import.main()
    bpy.context.scene.frame_set(0)
    objects = [o for o in bpy.context.scene.objects if o.type == 'MESH' and not o.get('mu_helper')]
    bpy.context.view_layer.update()
    report = []
    for obj in objects:
        report.append({'mesh': obj.name, 'bounds': mesh_bounds([obj]),
                       'triangles': sum(len(p.vertices) - 2 for p in obj.data.polygons),
                       'materials': [m.name for m in obj.data.materials]})
    (output / 'geometry.json').write_text(json.dumps(report, indent=2) + '\n')
    set_camera(mesh_bounds(objects))
    set_lighting()
    scene = bpy.context.scene
    scene.cycles.samples = 8
    scene.render.threads_mode = 'FIXED'
    scene.render.threads = 2
    scene.render.resolution_x, scene.render.resolution_y = RENDER_SIZE
    scene.render.filepath = str(output / 'baseline-offline.png')
    bpy.ops.render.render(write_still=True)


def main():
    data = json.loads((HERE / 'dependency-map.json').read_text())
    for name, record in sorted(data['models'].items()):
        if record['scope_exclusion']:
            continue
        inspect(name, record)


if __name__ == '__main__':
    main()
