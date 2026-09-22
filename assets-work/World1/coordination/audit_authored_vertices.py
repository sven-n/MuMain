"""Cross-check authored rigid-part ownership against final BMD reimports."""
import json
from pathlib import Path
import sys
import bpy
from mathutils import kdtree

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
TOLERANCE = .01  # Covers measured legacy float reconstruction; report exact maxima.


def points(path):
    bpy.ops.wm.open_mainfile(filepath=str(path))
    bpy.context.scene.frame_set(0)
    result = {}
    for obj in bpy.context.scene.objects:
        if obj.type != 'MESH' or obj.get('mu_helper') or obj.get('mu_reference'):
            continue
        if any(c.name.startswith('REF_') for c in obj.users_collection):
            continue
        for vertex in obj.data.vertices:
            weights = [g for g in vertex.groups if g.weight > .999]
            assert len(weights) == 1, (path, obj.name, vertex.index)
            bone = obj.vertex_groups[weights[0].group].name
            result.setdefault(bone, []).append(obj.matrix_world @ vertex.co)
    assert result, str(path)
    return result


def distances(source, target):
    maximum = 0
    for bone, vertices in source.items():
        assert bone in target, bone
        tree = kdtree.KDTree(len(target[bone]))
        for index, point in enumerate(target[bone]):
            tree.insert(point, index)
        tree.balance()
        maximum = max(maximum, *(tree.find(point)[2] for point in vertices))
    return maximum


def candidates():
    latest = {}
    for batch in json.loads((HERE/'integration-ledger.json').read_text()):
        for game, export in batch['game_files'].items():
            if game.endswith('.bmd'):
                latest[Path(game).stem] = ROOT/Path(export).parent.parent
    for name, folder in latest.items():
        comparisons = list((folder/'validation').glob('*compare*.txt'))
        full = next((p for p in comparisons if p.name in ('compare.txt','comparison.txt','full-compare.txt')), None)
        if full and 'DIFFERENT' not in full.read_text():
            continue
        yield name, folder


def main():
    reports = []
    for name, folder in candidates():
        authored = points(folder/'source.blend')
        exported = points(folder/'validation/reimported.blend')
        forward, reverse = distances(authored, exported), distances(exported, authored)
        reports.append(dict(model=name, source=str(folder.relative_to(ROOT)),
                            authored_vertices=sum(map(len,authored.values())),
                            reimport_vertices=sum(map(len,exported.values())),
                            authored_to_export_max=forward, export_to_authored_max=reverse,
                            bone_names=sorted(authored), result='PASS' if max(forward,reverse)<TOLERANCE else 'REVIEW'))
    record = dict(result='PASS' if all(r['result']=='PASS' for r in reports) else 'REVIEW',
                  tolerance=TOLERANCE, space='Blender bind/world coordinates, matched only within identical bone names',
                  tested='Both directions, every authored vertex and every final-reimport vertex of remodeled models',
                  client_verified=False, models=reports)
    (HERE/'authored-vertex-audit.json').write_text(json.dumps(record,indent=2)+'\n')
    print(record['result'],len(reports),'models')
    for report in reports:
        if report['result']!='PASS': print(report)


if __name__ == '__main__':
    main()
