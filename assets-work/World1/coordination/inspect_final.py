"""Render the actual integrated BMD/material combinations for offline review."""

import hashlib
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


def inspect(name, model):
    output = HERE / 'final-inspection' / name
    output.mkdir(parents=True, exist_ok=True)
    dependencies = [model['path']]
    dependencies += [p for paths in model['textures'].values() for p in paths]
    hashes = {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in dependencies}
    manifest = output / 'provenance.json'
    if manifest.exists() and (output / 'final-offline.png').exists():
        if json.loads(manifest.read_text())['game_files'] == hashes:
            return
    sys.argv = ['blender', '--', '--bmd', str(ROOT / model['path']),
                '--data', str(ROOT / 'src/bin/Data'), '--bmdconv', CONVERTER,
                '--out', str(output / 'import.blend')]
    mu_bmd_import.main()
    scene = bpy.context.scene
    scene.frame_set(0)
    objects = [o for o in scene.objects if o.type == 'MESH' and not o.get('mu_helper')]
    bpy.context.view_layer.update()
    set_camera(mesh_bounds(objects), scale_factor=1.8)
    set_lighting()
    scene.cycles.samples = 8
    scene.render.threads_mode = 'FIXED'
    scene.render.threads = 2
    scene.render.resolution_x, scene.render.resolution_y = 480, 440
    scene.render.filepath = str(output / 'final-offline.png')
    bpy.ops.render.render(write_still=True)
    report = dict(model=name, game_files=hashes, client_verified=False,
                  method='Actual combined BMD and wrapped textures; Blender diffuse preview at bind pose. Engine additive, scrolling, lighting and placement effects are not client evidence.',
                  triangles=sum(len(p.vertices)-2 for o in objects for p in o.data.polygons),
                  bind_bounds=mesh_bounds(objects))
    manifest.write_text(json.dumps(report, indent=2) + '\n')
    print('COMBINED RENDER', name, flush=True)


def main():
    bpy.context.preferences.filepaths.save_version = 0
    data = json.loads((HERE / 'dependency-map.json').read_text())
    for name, model in sorted(data['models'].items()):
        if not model['scope_exclusion']:
            inspect(name, model)


if __name__ == '__main__':
    main()
