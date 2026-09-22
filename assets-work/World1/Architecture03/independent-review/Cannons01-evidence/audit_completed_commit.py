"""Independently validate immutable Cannon batch Git blobs, never its live worktree."""
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import bpy

sys.dont_write_bytecode = True
REPOSITORY = Path.cwd()
OUTPUT = Path(__file__).resolve().parent
BATCH = 'assets-work/World1/Cannons01'
CONVERTER = REPOSITORY.parent / 'MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv'
OWNED = ('Cannon01', 'Cannon02', 'Cannon03', 'Hanging01')
READONLY = ('Carriage01', 'Carriage02', 'Carriage03', 'Carriage04', 'HouseEtc02', 'StoneMuWall04', 'StoneWall03', 'Well01')
KEYS = {'Hanging01': 25, 'Carriage01': 21}
POSITION_TOLERANCE = .005
MATRIX_TOLERANCE = .0002
NORMAL_ROTATION_TOLERANCE = .00001
REVISION = sys.argv[sys.argv.index('--') + 1]


def module(name, filename):
    specification = importlib.util.spec_from_file_location(name, OUTPUT.parents[1] / filename)
    result = importlib.util.module_from_spec(specification)
    specification.loader.exec_module(result)
    return result


matrices = module('independent_matrices', 'validate_matrices.py')
raw = module('independent_raw', 'raw_bindings.py')


def git(*args):
    return subprocess.check_output(['git', *args], cwd=REPOSITORY)


def blob(path):
    return git('show', REVISION + ':' + path)


def converter(*args, check=True):
    process = subprocess.run([str(CONVERTER), *map(str, args)], cwd=REPOSITORY, text=True, capture_output=True)
    if check and process.returncode:
        raise RuntimeError(process.stdout + process.stderr)
    return process.stdout + process.stderr


def extract(name, stage, temporary):
    directory = temporary / name / stage
    directory.mkdir(parents=True)
    relative = f'{BATCH}/{name}/{"original" if stage == "original" else "exports"}/{name}.bmd'
    model = directory / (name + '.bmd')
    model.write_bytes(blob(relative))
    converter('bmd2smd', model, directory)
    validations = [converter('validate', path, *(['--animation'] if path.stem != name else []))
                   for path in sorted(directory.glob('*.smd'))]
    info = converter('info', model)
    return directory, model, info, validations


def action_audit(name, old, new):
    old_path, new_path = old / (name + '.smd'), new / (name + '.smd')
    parents, old_bind = matrices.parse_smd(old_path)
    new_parents, new_bind = matrices.parse_smd(new_path)
    assert parents == new_parents
    old_nodes = old_path.read_text().split('nodes\n')[1].split('\nend')[0]
    assert old_nodes == new_path.read_text().split('nodes\n')[1].split('\nend')[0]
    bind = [matrices.world_matrices(parents, values[0]) for values in (old_bind, new_bind)]
    points = [matrices.mesh_points(path) for path in (old_path, new_path)]
    clips = [matrices.parse_smd(path.with_name(name + '_a00.smd'))[1] for path in (old_path, new_path)]
    assert clips[0].keys() == clips[1].keys() and len(clips[0]) == KEYS.get(name, 1)
    maximum_matrix, maximum_bounds, maximum_local = 0, 0, 0
    for frame in clips[0]:
        local_delta = max(abs(a-b) for bone in parents for aa, bb in zip(clips[0][frame][bone], clips[1][frame][bone]) for a, b in zip(aa, bb))
        maximum_local = max(maximum_local, local_delta)
        worlds = [matrices.world_matrices(parents, clip[frame]) for clip in clips]
        delta = max(abs(a-b) for bone in parents for aa, bb in zip(worlds[0][bone], worlds[1][bone]) for a, b in zip(aa, bb))
        bounds = [matrices.posed_bounds(points[i], bind[i], worlds[i]) for i in range(2)]
        difference = max(abs(a-b) for aa, bb in zip(*bounds) for a, b in zip(aa, bb))
        maximum_matrix, maximum_bounds = max(maximum_matrix, delta), max(maximum_bounds, difference)
    assert maximum_local < MATRIX_TOLERANCE and maximum_matrix < MATRIX_TOLERANCE and maximum_bounds < POSITION_TOLERANCE, (name, maximum_local, maximum_matrix, maximum_bounds)
    return dict(keys=len(clips[0]), bones=len(parents), samples=len(clips[0])*len(parents), max_hierarchical_matrix_delta=maximum_matrix,
                max_local_matrix_delta=maximum_local, max_posed_bound_delta=maximum_bounds), clips[1], parents


def raw_audit(model, frames, parents):
    pairs = set()
    for mesh in raw.meshes(model):
        for vertices, normals in mesh['triangles']:
            for vi, ni in zip(vertices, normals):
                vertex, normal = mesh['vertices'][vi][0], mesh['normals'][ni][0]
                if vertex != normal:
                    pairs.add((vertex, normal))
    rotations = [matrices.world_matrices(parents, frame) for frame in frames.values()]
    differences = {f'{a}:{b}': max(abs(world[a][i][j]-world[b][i][j]) for world in rotations for i in range(3) for j in range(3))
                   for a, b in pairs}
    assert all(value < NORMAL_ROTATION_TOLERANCE for value in differences.values()), differences
    return dict(status='PASS', unequal_node_pair_rotation_deltas=differences)


def packed_source_audit(name, folder, replacement):
    source = folder / 'source.blend'
    source.write_bytes(blob(f'{BATCH}/{name}/source.blend'))
    bpy.ops.wm.open_mainfile(filepath=str(source))
    bpy.context.scene.frame_set(0)
    rig = next(obj for obj in bpy.context.scene.objects if obj.type == 'ARMATURE')
    bones = {bone: index for index, bone in enumerate(rig['mu_bone_order'])}
    images = [image for image in bpy.data.images if image.source == 'FILE']
    assert images and all(image.packed_file for image in images)
    for label in ('REF_ORIGINAL', 'REF_HIGH_POLY'):
        collection = bpy.data.collections[label]
        assert collection.hide_render and all(obj.get('mu_reference') for obj in collection.objects)
    source_points = {}
    for obj in bpy.data.collections['EXPORT_' + name].objects:
        for vertex in obj.data.vertices:
            assert len(vertex.groups) == 1 and vertex.groups[0].weight == 1
            bone = bones[obj.vertex_groups[vertex.groups[0].group].name]
            source_points.setdefault(bone, []).append(obj.matrix_world @ vertex.co)
    final = matrices.mesh_points(replacement / (name + '.smd'))
    maximum = max(min(max(abs(a-b) for a, b in zip(point, candidate)) for candidate in source_points[bone]) for bone, point in final)
    exported = {}
    for bone, point in final:
        exported.setdefault(bone, []).append(point)
    reverse = max(min(max(abs(a-b) for a, b in zip(point, candidate)) for candidate in exported[bone])
                  for bone, points in source_points.items() for point in points)
    assert max(maximum, reverse) < POSITION_TOLERANCE, (name, maximum, reverse)
    return dict(status='PASS', packed_images=len(images), reference_collections_hidden_and_marked=True,
                every_exported_corner_and_source_vertex_matches_intended_bone=True,
                max_export_to_authored_position_delta=maximum, max_authored_to_export_position_delta=reverse)


def audit_asset(name, temporary):
    old, before, old_info, old_validation = extract(name, 'original', temporary)
    new, after, new_info, new_validation = extract(name, 'new', temporary)
    old_contract = [line.strip() for line in old_info.splitlines() if line.strip().startswith(('bone ', 'action '))]
    new_contract = [line.strip() for line in new_info.splitlines() if line.strip().startswith(('bone ', 'action '))]
    assert old_contract == new_contract
    assert re.findall(r'texture=(.*)', old_info) == re.findall(r'texture=(.*)', new_info)
    count = int(re.search(r'triangles: (\d+)', new_info)[1])
    assert count <= 1500
    comparison = converter('compare', before, after, check=False)
    expected = 'DIFFERENT' if name in OWNED else 'EQUIVALENT'
    assert expected in comparison
    if name in READONLY:
        assert before.read_bytes() == after.read_bytes()
    else:
        assert after.read_bytes() == blob(f'src/bin/Data/Object1/{name}.bmd')
    actions, clips, parents = action_audit(name, old, new)
    result = dict(status='PASS', triangles=count, original_sha256=hashlib.sha256(before.read_bytes()).hexdigest(),
                  export_sha256=hashlib.sha256(after.read_bytes()).hexdigest(), full_compare=expected,
                  full_compare_output=comparison, info_before=old_info, info_after=new_info,
                  contract=old_contract, motion=actions, raw_bindings=raw_audit(after, clips, parents),
                  packed_source=packed_source_audit(name, temporary / name, new),
                  converter_validations=old_validation+new_validation)
    print(name, expected, actions, flush=True)
    return result


def texture_audit(provenance, temporary):
    final_atlas = blob('src/bin/Data/Object1/horse_drawn_01.OZJ')
    records, containers = {}, []
    for name, record in provenance.items():
        original = blob(BATCH + '/' + record['archive'])
        assert original == git('show', record['source_revision'] + ':src/bin/Data/Object1/' + name)
        assert hashlib.sha256(original).hexdigest() == record['sha256']
        if name.endswith('.bmd'):
            continue
        data = blob(BATCH + '/textures/final/' + name)
        assert data == (final_atlas if name == 'horse_drawn_01.OZJ' else original), name
        destination = temporary / name
        destination.write_bytes(data)
        containers.append(str(destination))
        records[name] = dict(sha256=hashlib.sha256(data).hexdigest(), frozen=name != 'horse_drawn_01.OZJ')
    for name in OWNED + READONLY:
        assert blob(f'{BATCH}/{name}/exports/horse_drawn_01.OZJ') == final_atlas
    python = '/Users/webproduktion3/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/bin/python3'
    validation = subprocess.check_output([python, '-B', str(REPOSITORY / 'tools/mu_texture.py'), 'check', *containers], cwd=REPOSITORY, text=True)
    return dict(status='PASS', containers=records, wrapper_validation=validation, all_twelve_consumer_atlases_identical=True)


def main():
    revision = git('rev-parse', REVISION).decode().strip()
    provenance = json.loads(blob(BATCH + '/original-provenance.json'))
    baseline = provenance['Cannon01.bmd']['source_revision']
    changes = git('diff', '--name-only', baseline, revision).decode().splitlines()
    allowed = {f'src/bin/Data/Object1/{name}.bmd' for name in OWNED} | {'src/bin/Data/Object1/horse_drawn_01.OZJ'}
    assert all(path.startswith(BATCH + '/') or path in allowed for path in changes), changes
    assert {path for path in changes if path.startswith('src/')} == allowed
    with tempfile.TemporaryDirectory(prefix='audit-', dir=OUTPUT) as scratch:
        results = {name: audit_asset(name, Path(scratch)) for name in OWNED+READONLY}
        textures = texture_audit(provenance, Path(scratch))
    report = dict(status='PASS', pinned_commit=revision, original_baseline=baseline, game_files=sorted(allowed),
                  method='Fresh converter extraction/validation from immutable Git BMD blobs; direct hierarchical action matrices and every posed mesh corner; direct raw BMD normal ownership; no producer checkout writes',
                  tolerances=dict(matrix=MATRIX_TOLERANCE, posed_bounds=POSITION_TOLERANCE, raw_normal_rotation=NORMAL_ROTATION_TOLERANCE), assets=results, textures=textures)
    (OUTPUT / 'completed-commit-audit.json').write_text(json.dumps(report, indent=2)+'\n')


if __name__ == '__main__':
    main()
