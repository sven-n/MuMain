"""Three-arm bronze candelabrum; keep the original animated flame geometry and rig."""

from pathlib import Path
import sys
import traceback

import bmesh
import bpy

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT.parent / 'StaticBatch01'))
from build_support import finish, start
from geometry import lathe, material, tube

BRONZE = (.02, .52, .48, .98)
WAX = (.53, .53, .97, .98)
IRON = (.02, .02, .48, .48)
ENGRAVING = (.52, .02, .98, .48)
STAND_BONE = 'Cylinder05'
CENTER = (.225, -.02435, .0057)
CANDLES = [('Left', -15.1763, 44.9, 66.35), ('Center', 0, 50.5, 71.69),
           ('Right', 15.1459, 45.0, 66.3941)]


def base_and_stem(context):
    lathe('OctagonalFoot', [(0, 0), (11.23585, 0), (11.23585, 1.5),
                          (9.4, 2.2), (5.5, 3.7), (3.2, 6.4), (0, 6.4)],
          CENTER, 8, ENGRAVING, STAND_BONE, context, smooth=False)
    profile = [(0, 4.8), (3.1, 4.8), (3.1, 7), (1.65, 9), (1.65, 26),
               (2.9, 28), (2.9, 30.5), (1.6, 32.5), (1.5, 49), (0, 49)]
    lathe('TurnedBronzeStem', profile, CENTER, 8, BRONZE, STAND_BONE, context)
    lathe('DarkCollar', [(0, 27), (3.15, 27), (3.15, 28.7), (0, 28.7)],
          CENTER, 8, IRON, STAND_BONE, context)


def branching_arms(context):
    for side in (-1, 1):
        centers = [(CENTER[0], CENTER[1], 29), (side * 6, CENTER[1], 32.5),
                   (side * 11.8, CENTER[1], 37.3), (side * 15.1, CENTER[1], 41.2),
                   (side * 15.1, CENTER[1], 44.8)]
        tube(f'ForgedArm{side}', centers, [2, 1.7, 1.5, 1.4, 1.5],
             6, BRONZE, STAND_BONE, context)
        scroll = [(side * 5, CENTER[1], 31.8), (side * 8.7, CENTER[1], 31),
                  (side * 12.2, CENTER[1], 32.5), (side * 12.7, CENTER[1], 35.2),
                  (side * 10.8, CENTER[1], 36.1)]
        tube(f'GothicScroll{side}', scroll, [.8, .8, .75, .65, .4],
             4, BRONZE, STAND_BONE, context)


def candle(name, x, bottom, top, context):
    center = (x, -.02435, bottom)
    lathe(name + 'DripCup', [(0, -2.5), (2.2, -2.5), (4.0, -1.0),
                           (4.0, -.2), (3.85, .35), (3.3, .35), (0, -.1)],
          center, 8, ENGRAVING, STAND_BONE, context)
    height = top - bottom
    lathe(name + 'Beeswax', [(0, 0), (3.5, 0), (3.65, .8), (3.65, height - .9),
                           (3.4, height), (2.65, height - .45), (0, height - .55)],
          center, 10, WAX, STAND_BONE, context)
    lathe(name + 'Wick', [(0, height - .55), (.3, height - .55), (.25, height + .3), (0, height + .3)],
          center, 4, IRON, STAND_BONE, context, smooth=False)


def preserve_flames(reference, target, flame_material):
    source = next(iter(reference.objects))
    obj = source.copy()
    obj.data = source.data.copy()
    obj.name = 'OriginalAnimatedFlames'
    obj['mu_reference'] = False
    obj['mu_preserved_flames'] = True
    target.objects.link(obj)
    data = bmesh.new()
    data.from_mesh(obj.data)
    bmesh.ops.delete(data, geom=[face for face in data.faces if face.material_index == 0], context='FACES')
    bmesh.ops.delete(data, geom=[vertex for vertex in data.verts if not vertex.link_faces], context='VERTS')
    for face in data.faces:
        face.material_index = 0
    data.to_mesh(obj.data)
    data.free()
    # Joining meshes merges UV sets by name; retain the coordinates in the shared set.
    obj.data.uv_layers[0].name = 'UVMap'
    obj.data.materials.clear()
    obj.data.materials.append(flame_material)


def main():
    rig, reference, target, bounds = start(ROOT)
    context = target, rig, material(ROOT / 'textures/candle.jpg')
    base_and_stem(context)
    branching_arms(context)
    for specification in CANDLES:
        candle(*specification, context)
    preserve_flames(reference, target, material(ROOT / 'textures/candle2.jpg'))
    finish(ROOT, rig, target, bounds)


if __name__ == '__main__':
    try:
        main()
    except Exception:
        traceback.print_exc()
        sys.exit(1)
