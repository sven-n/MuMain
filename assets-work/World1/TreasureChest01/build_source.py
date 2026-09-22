"""Walnut and antique-brass chest, modeled around the untouched seven-frame lid rig."""

import math
from pathlib import Path
import sys
import traceback

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT.parent / 'StaticBatch01'))
from build_support import finish, start
from geometry import box, material, mesh, uv

WOOD = (.02, .02, .48, .98)
BRASS = (.52, .52, .98, .98)
IRON = (.52, .02, .98, .48)
BODY_BONE, LID_BONE = 'Bone01', 'Bone02'
LEFT, RIGHT = -57.7335, 56.8326
FRONT, BACK = -29.5275, 29.9587
BOTTOM, HINGE, TOP = .0538, 47.5454, 84.0915
CENTER_Y = (FRONT + BACK) / 2
RADIUS_Y = (BACK - FRONT) / 2
RADIUS_Z = TOP - HINGE
ARC_SEGMENTS = 12


def wooden_body(context):
    panels = [((LEFT + 1, FRONT + 1, BOTTOM + 1), (RIGHT - 1, BACK - 1, BOTTOM + 4)),
              ((LEFT + 1, FRONT + 1, BOTTOM + 2), (RIGHT - 1, FRONT + 4, HINGE - .65)),
              ((LEFT + 1, BACK - 4, BOTTOM + 2), (RIGHT - 1, BACK - 1, HINGE - .65)),
              ((LEFT + 1, FRONT + 3, BOTTOM + 2), (LEFT + 4, BACK - 3, HINGE - .65)),
              ((RIGHT - 4, FRONT + 3, BOTTOM + 2), (RIGHT - 1, BACK - 3, HINGE - .65))]
    for index, (low, high) in enumerate(panels):
        box(f'WalnutPanel{index}', low, high, WOOD, BODY_BONE, context, bevel=.5)


def body_straps(context):
    for x in (LEFT, RIGHT - 4):
        for y in (FRONT, BACK - 4):
            box(f'CornerPost{x}_{y}', (x, y, BOTTOM), (x + 4, y + 4, HINGE - .3),
                BRASS, BODY_BONE, context, bevel=.45)
    for y in (FRONT, BACK - 2.4):
        for z in (BOTTOM, HINGE - 4.2):
            box(f'Rim{y}_{z}', (LEFT + 3.5, y, z), (RIGHT - 3.5, y + 2.4, z + 3.7),
                BRASS, BODY_BONE, context, bevel=.35)
    for x in (LEFT, RIGHT - 2.4):
        for z in (BOTTOM, HINGE - 4.2):
            box(f'SideRim{x}_{z}', (x, FRONT + 4, z), (x + 2.4, BACK - 4, z + 3.7),
                BRASS, BODY_BONE, context)


def arch_points(x, inset):
    return [(x, CENTER_Y - (RADIUS_Y - inset) * math.cos(math.pi * i / ARC_SEGMENTS),
             HINGE + (RADIUS_Z - inset) * math.sin(math.pi * i / ARC_SEGMENTS))
            for i in range(ARC_SEGMENTS + 1)]


def curved_shell(name, x_left, x_right, outer, inner, rectangle, context):
    """Four connected arch strips make a solid, hollow hood or a thin metal binding."""
    rows = [arch_points(x_left, outer), arch_points(x_right, outer),
            arch_points(x_right, inner), arch_points(x_left, inner)]
    vertices = [point for row in rows for point in row]
    stride = ARC_SEGMENTS + 1
    faces, coordinates = [], []
    for band in range(4):
        nxt = (band + 1) % 4
        for segment in range(ARC_SEGMENTS):
            faces.append((band * stride + segment, nxt * stride + segment,
                          nxt * stride + segment + 1, band * stride + segment + 1))
            coordinates.append([uv(rectangle, u, v) for u, v in
                                ((0, segment / ARC_SEGMENTS), (1, segment / ARC_SEGMENTS),
                                 (1, (segment + 1) / ARC_SEGMENTS), (0, (segment + 1) / ARC_SEGMENTS))])
    for end in (0, ARC_SEGMENTS):
        faces.append(tuple(row * stride + end for row in range(4)))
        coordinates.append([uv(rectangle, u, v) for u, v in ((0, 0), (1, 0), (1, 1), (0, 1))])
    return mesh(name, vertices, faces, coordinates, LID_BONE, context)


def lid_end(name, x_outer, x_inner, context):
    contour = arch_points(x_outer, 1.1)
    vertices = contour + [(x_inner, y, z) for _, y, z in contour]
    count = len(contour)
    faces = [list(range(count - 1, -1, -1)), list(range(count, count * 2))]
    for corner in range(count):
        nxt = (corner + 1) % count
        faces.append((corner, nxt, nxt + count, corner + count))
    coords = [[uv(WOOD, (vertices[i][1] - FRONT) / (BACK - FRONT),
                  (vertices[i][2] - HINGE) / (TOP - HINGE)) for i in face] for face in faces]
    mesh(name, vertices, faces, coords, LID_BONE, context)


def domed_lid(context):
    curved_shell('TwelveSlatHollowLid', LEFT + .8, RIGHT - .8, .8, 3.1, WOOD, context)
    lid_end('LeftLidEnd', LEFT + .8, LEFT + 3.2, context)
    lid_end('RightLidEnd', RIGHT - .8, RIGHT - 3.2, context)
    for index, x in enumerate((LEFT, RIGHT - 4.5)):
        curved_shell(f'EngravedArchBinding{index}', x, x + 4.5, 0, .75, BRASS, context)


def rivet(name, x, y, z, bone, context):
    radius, depth = .95, .6
    vertices = [(x - radius, y, z), (x, y, z - radius), (x + radius, y, z),
                (x, y, z + radius), (x, y - depth, z)]
    faces = [(0, 1, 4), (1, 2, 4), (2, 3, 4), (3, 0, 4)]
    coordinates = [[uv(BRASS, .35, .6), uv(BRASS, .55, .7), uv(BRASS, .45, .9)]] * 4
    mesh(name, vertices, faces, coordinates, bone, context)


def hardware(context):
    center = (LEFT + RIGHT) / 2
    box('LockReceiver', (center - 5.5, -34.3614, 32.8), (center + 5.5, FRONT + 1, 44.5),
        BRASS, BODY_BONE, context, bevel=.6)
    box('HingedHasp', (center - 3.8, -31.4, HINGE + .1), (center + 3.8, -28.5, 58),
        BRASS, LID_BONE, context, bevel=.45)
    box('KeyholeStem', (center - .45, -34.365, 36), (center + .45, -34.34, 39),
        IRON, BODY_BONE, context)
    vertices = [(center, -34.365, 41), (center + 1.1, -34.365, 39.6),
                (center, -34.365, 38.8), (center - 1.1, -34.365, 39.6)]
    mesh('KeyholeEye', vertices, [(0, 1, 2, 3)], [[uv(IRON, .5, .5)] * 4], BODY_BONE, context)
    for index, x in enumerate((LEFT + 2, LEFT + 20, center - 12, center + 12, RIGHT - 20, RIGHT - 2)):
        for z in (BOTTOM + 2, HINGE - 2.3):
            rivet(f'FrontRivet{index}_{z}', x, FRONT - .05, z, BODY_BONE, context)
    for index, x in enumerate((LEFT + 2.25, RIGHT - 2.25)):
        rivet(f'LidRivet{index}', x, FRONT + .1, HINGE + 2, LID_BONE, context)


def main():
    rig, reference, target, bounds = start(ROOT)
    context = target, rig, material(ROOT / 'textures/treasure_chest.jpg')
    wooden_body(context)
    body_straps(context)
    domed_lid(context)
    hardware(context)
    finish(ROOT, rig, target, bounds)


if __name__ == '__main__':
    try:
        main()
    except Exception:
        traceback.print_exc()
        sys.exit(1)
