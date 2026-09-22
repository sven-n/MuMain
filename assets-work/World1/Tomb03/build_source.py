"""Rebuild the original round-topped grave marker with bevels and a recessed inscription."""

import math
from pathlib import Path
import sys
import traceback

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT.parent / 'StaticBatch01'))
from build_support import finish, start
from geometry import material, mesh, uv

FRONT = (.015, .015, .73, .985)
STONE = (.785, .03, .97, .97)
ARCH_SEGMENTS = 14
BONE = 'Cylinder01'


def outline(bounds):
    low, high = bounds
    center = (low[0] + high[0]) / 2
    radius = (high[0] - low[0]) / 2
    spring = high[2] - radius
    points = [(low[0], low[2]), (high[0], low[2])]
    for index in range(ARCH_SEGMENTS + 1):
        angle = math.pi * index / ARCH_SEGMENTS
        # Two small worn notches add age without changing the footprint extrema.
        wear = .7 if index in (3, 11) else 0
        points.append((center + (radius - wear) * math.cos(angle),
                       spring + (radius - wear) * math.sin(angle)))
    return points


def inset(points, amount, bounds):
    low, high = bounds
    center = ((low[0] + high[0]) / 2, (low[2] + high[2]) / 2)
    size = (high[0] - low[0], high[2] - low[2])
    return [(center[0] + (x - center[0]) * (1 - 2 * amount / size[0]),
             center[1] + (z - center[1]) * (1 - 2 * amount / size[1])) for x, z in points]


def face_uv(point, bounds, front=True):
    low, high = bounds
    u = (point[0] - low[0]) / (high[0] - low[0])
    v = (point[2] - low[2]) / (high[2] - low[2])
    return uv(FRONT if front else STONE, u, v)


def gravestone(context, bounds):
    low, high = bounds
    contour = outline(bounds)
    # Rim, chamfer, inscription recess and rear bevel are real geometry, not shader effects.
    layers = [(2.2, high[1]), (0, high[1] - 2.2), (0, low[1] + 2.2),
              (2.2, low[1]), (5.8, low[1]), (7.0, low[1] + 1.1)]
    vertices = [(x, depth, z) for shrink, depth in layers for x, z in inset(contour, shrink, bounds)]
    count = len(contour)
    faces, coordinates = [], []
    for level in range(len(layers) - 1):
        for corner in range(count):
            nxt = (corner + 1) % count
            face = (level * count + corner, level * count + nxt,
                    (level + 1) * count + nxt, (level + 1) * count + corner)
            faces.append(face)
            if level >= 3:
                coordinates.append([face_uv(vertices[i], bounds) for i in face])
            else:
                coordinates.append([uv(STONE, u, v) for u, v in
                                    ((.1, corner / count), (.9, corner / count),
                                     (.9, (corner + 1) / count), (.1, (corner + 1) / count))])
    for level, front in ((0, False), (len(layers) - 1, True)):
        face = list(range(level * count, (level + 1) * count))
        faces.append(face if front else face[::-1])
        coordinates.append([face_uv(vertices[i], bounds, front) for i in faces[-1]])
    mesh('CarvedRoundHeadstone', vertices, faces, coordinates, BONE, context)


def main():
    rig, reference, target, bounds = start(ROOT)
    context = target, rig, material(ROOT / 'textures/tombstone.jpg')
    gravestone(context, bounds)
    finish(ROOT, rig, target, bounds)


if __name__ == '__main__':
    try:
        main()
    except Exception:
        traceback.print_exc()
        sys.exit(1)
