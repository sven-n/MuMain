"""Strict correspondence checks followed by matching actual-export diffuse views."""
from pathlib import Path
import sys
sys.dont_write_bytecode = True
import bpy
from mathutils import Vector
ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(REPOSITORY / 'assets-work/World1/StaticBatch01'))
from review_scene import mesh_bounds, set_camera, set_lighting, render
from audit_source import check as check_source
from audit_raw_normals import check as check_normals
from audit_packed import check as check_packed
from audit_contract import check as check_contract


def show(folder):
    check_source(folder)
    check_normals(folder)
    check_packed(folder)
    check_contract(folder)
    bpy.ops.wm.open_mainfile(filepath=str(folder / 'baseline/source.blend'))
    bounds = mesh_bounds([o for o in bpy.context.scene.objects if o.type == 'MESH' and not o.get('mu_helper')])
    stages = {'original': 'original/source.blend', 'current': 'baseline/source.blend', 'candidate': 'validation/reimported.blend'}
    for stage, path in stages.items():
        bpy.ops.wm.open_mainfile(filepath=str(folder / path))
        bpy.context.scene.frame_set(0)
        set_camera(bounds)
        set_lighting()
        bpy.context.scene.cycles.samples = 16
        render(folder / 'review' / (stage + '.png'))
        bpy.context.scene.render.resolution_percentage = 25
        render(folder / 'review' / (stage + '-small.png'))
        center = (Vector(bounds[0]) + Vector(bounds[1])) / 2
        camera = bpy.context.scene.camera
        camera.location = center + Vector((-1.35, 2, 1.3)) * max(Vector(bounds[1]) - Vector(bounds[0]))
        camera.rotation_euler = (center-camera.location).to_track_quat('-Z', 'Y').to_euler()
        bpy.context.scene.render.resolution_percentage = 100
        render(folder / 'review' / (stage + '-reverse.png'))
    for obj in bpy.context.scene.objects:
        if obj.type == 'MESH' and not obj.get('mu_helper'):
            wire = obj.modifiers.new('ReviewWire', 'WIREFRAME')
            wire.thickness = .10
    render(folder / 'review/wireframe.png')


if __name__ == '__main__':
    for name in ('Object04',):
        show(ROOT / name)
