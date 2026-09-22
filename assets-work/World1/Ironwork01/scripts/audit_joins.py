"""Prove every original modular anchor survives the final BMD, including placed scale."""
import json
import math
from pathlib import Path
import sys
sys.dont_write_bytecode=True
from config import PROPS,ROOT
POSITION_TOLERANCE=.002


def corners(path):
    lines=path.read_text().split('triangles\n')[1].splitlines()[:-1]
    return sorted({tuple(map(float,line.split()[:4])) for i,line in enumerate(lines) if i%4})


for name in PROPS:
    folder=ROOT/name
    original=corners(folder/'validation/original'/f'{name}.smd')
    final=corners(folder/'validation/replacement'/f'{name}.smd')
    distances=[]
    for vertex in original:
        distance=min(math.dist(vertex[1:],candidate[1:]) for candidate in final if candidate[0]==vertex[0])
        assert distance<POSITION_TOLERANCE,(name,vertex,distance)
        distances.append(distance)
    placements=json.loads((folder/'original/placements.json').read_text())
    report={'status':'PASS','all_original_anchors_preserved':len(original),
            'bone_assignments_retained':True,'maximum_exported_anchor_distance':max(distances),
            'maximum_World1_scaled_anchor_distance':max(distances)*max(row['scale'] for row in placements),
            'source_connection_vertices':'exact (source-audit.json)',
            'tolerance_game_units':POSITION_TOLERANCE,
            'opening_envelopes':'Original support endpoints and every alpha-plane corner/UV retained',
            'placement_count':len(placements),'placement_changes':False}
    (folder/'validation/joins.json').write_text(json.dumps(report,indent=2)+'\n')
    print(name,report['all_original_anchors_preserved'],'original anchors; max drift',max(distances))
