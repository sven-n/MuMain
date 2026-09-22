"""Read-only original geometry previews for the independent cannon review."""
import sys
from pathlib import Path
sys.dont_write_bytecode = True
import bpy
REPOSITORY = Path.cwd()
SOURCE = REPOSITORY.parent / 'MuMain-lorencia-cannons/assets-work/World1/Cannons01'
OUTPUT = Path(__file__).resolve().parent
sys.path.insert(0, str(REPOSITORY / 'assets-work/World1/StaticBatch01'))
from review_scene import mesh_bounds, set_camera, set_lighting, render

for name in ('Cannon01', 'Cannon02', 'Cannon03', 'Hanging01'):
    bpy.ops.wm.open_mainfile(filepath=str(SOURCE / name / 'original/source.blend'))
    bpy.context.scene.frame_set(0)
    bpy.context.view_layer.update()
    objects = [obj for obj in bpy.context.scene.objects if obj.type == 'MESH' and not obj.get('mu_helper')]
    set_camera(mesh_bounds(objects))
    set_lighting()
    bpy.context.scene.render.resolution_x = 700
    bpy.context.scene.render.resolution_y = 650
    render(OUTPUT / (name + '-original.png'))
