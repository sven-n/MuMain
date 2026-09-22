"""Authored furniture forms, using measured original coordinates in game units."""

import math

from geometry import box, ring_solid, rivet, timber

TABLE_BOUNDS = ((-101.684799, -51.504002, 0), (101.784798, 51.387402, 86.580902))
HALF_BOUNDS = ((-106.607697, -99.685699, 0), (97.078102, -.396301, 86.327499))
BAR_BOUNDS = ((-101.784302, -34.814301, 0), (101.685303, 35.179798, 105.375504))


def splayed_leg(name, x, bottom_y, top_y, height, depth, context):
    def section(z, width=10, extra=0):
        y = bottom_y + (top_y - bottom_y) * z / height
        return x, y, z, width + extra, depth + extra
    timber(name, [section(0), section(1), section(height - 1), section(height)], context)
    timber(name + '_IronFoot', [section(4, extra=.3), section(9, extra=.3)], context, 'iron')


def table(context):
    low, high = TABLE_BOUNDS
    box('CarvedTabletop', (low[0], low[1], 77.765701), high,
        context, top='top', chamfer=.65)
    for x in (-65.5837, 65.048601):
        for bottom_y, top_y in ((-29.713201, -9.8501), (28.6983, 8.8352)):
            splayed_leg(f'Leg_{x}_{bottom_y}', x, bottom_y, top_y, 77.346901, 12.7078, context)
        box(f'EndStretcher_{x}', (x - 4, -26, 20), (x + 4, 25, 26), context, chamfer=.6)
    for y in (-13, 10):
        box(f'TopApron_{y}', (-73, y - 3, 69), (73, y + 3, 77.765701), context, chamfer=.5)
    box('LongStretcher', (-68, -3.5, 22), (68, 2.5, 29), context, chamfer=.6)
    for x in (-65.5837, 65.048601):
        box(f'ApronBinding_{x}', (x - 3, -16.25, 66), (x + 3, -15.75, 77), context, 'iron')
        rivet(f'ApronPin_{x}', (x, -16.35, 72), 1.4, context)


def half_top(context):
    low, high = HALF_BOUNDS
    center_x = (low[0] + high[0]) / 2
    radius_x, radius_y = (high[0] - low[0]) / 2, high[1] - low[1]
    segments, lower_z, bevel = 18, 77.709396, .65
    rings = []
    for z, inset in ((lower_z, bevel), (lower_z + bevel, 0),
                     (high[2] - bevel, 0), (high[2], bevel)):
        # The flat joining edge always lies at the measured Y, including bevel levels.
        rings.append([(center_x + (radius_x - (0 if i in (0, segments) else inset)) * math.cos(math.pi + math.pi * i / segments),
                       high[1] + (radius_y - inset) * math.sin(math.pi + math.pi * i / segments), z)
                      for i in range(segments + 1)])
    return ring_solid('HalfRoundCarvedTop', rings, context, 'wood', top='top')


def half_table(context):
    half_top(context)
    cx, cy = .135201, -25.904099
    sections = [(cx, cy, 0, 50, 40), (cx, cy, 3, 50, 40),
                (cx, cy, 12.9834, 31.8301, 25.4641), (cx, cy, 16, 31.8301, 25.4641),
                (cx, cy, 69, 16.9014, 13.5211), (cx, cy, 77.709396, 26, 21),
                (cx, cy, 79.488998, 26, 21)]
    timber('FlaredPedestal', sections, context, cut=1.1)
    timber('PedestalIronCollar', [(cx, cy, 14, 32.1, 25.8),
                                (cx, cy, 17, 31.8, 25.5)], context, 'iron', cut=1)
    # Small tenon/binding details remain subordinate to the original flared silhouette.
    for x in (-8.0, 8.27):
        rivet(f'PedestalPin_{x}', (x, cy - 12.95, 15.5), 1.05, context)


def bar_top(context):
    low, high = BAR_BOUNDS
    # Retain all eight original tabletop corners for the three repeated sections.
    return box('ContinuousCountertop', (low[0], low[1], 94.663597), high, context, top='top')


def bar_panel(context):
    low, high = BAR_BOUNDS
    panel_y, back_y, bottom, top = -8.5842, 6.7758, 1.064201, 95.136497
    rear_join = 4.5
    box('ContinuousBack', (low[0], rear_join, bottom), (high[0], back_y, top), context)
    for name, z0, z1 in (('Sill', bottom, 12), ('Lintel', 84, top)):
        box(name, (low[0], panel_y, z0), (high[0], rear_join, z1), context)
    for x0, x1 in ((low[0], low[0] + 10), (-4, 4), (high[0] - 10, high[0])):
        box(f'FrameStile_{x0}', (x0, panel_y, 12), (x1, rear_join, 84), context)
    for x0, x1 in ((low[0] + 10, -4), (4, high[0] - 10)):
        box(f'CarvedInset_{x0}', (x0, panel_y + .5, 12), (x1, rear_join, 84),
            context, front='panel', chamfer=.5)


def counter(context):
    bar_top(context)
    bar_panel(context)
    for x in (-65.683199, 64.949097):
        splayed_leg(f'CounterBrace_{x}', x, -19.99065, -6.4784, 80, 8.6447, context)
        box(f'BraceBinding_{x}', (x - 3, -12.8, 67), (x + 3, -12.3, 78), context, 'iron')
        rivet(f'BracePin_{x}', (x, -12.9, 73), 1.4, context)


BUILDERS = {'Furniture03': table, 'Furniture04': half_table, 'Furniture05': counter}
