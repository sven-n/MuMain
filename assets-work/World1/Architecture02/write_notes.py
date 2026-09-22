"""Write local production handoffs from the measured final package."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
IDENTITIES = {'House05': 'Animated watermill with raised water flume and turning wheel',
              'HouseWall01': 'Straight masonry wall with timber base and stone foot',
              'HouseWall04': 'L-corner masonry wall module',
              'HouseWall05': 'Corner roof cap with two shingled eaves',
              'HouseWall06': 'Straight roof cap with one shingled eave',
              'Stair01': 'Vertical nine-rung ladder',
              'House04': 'Accepted animated round workshop, readonly compatibility model',
              'HouseWall02': 'Accepted window-wall module, readonly compatibility model',
              'HouseEtc02': 'Accepted low-walled Masonry annex, readonly compatibility model'}
READONLY = ('House04', 'HouseWall02', 'HouseEtc02')
KEYS = {'House05': 30, 'HouseWall01': 1, 'HouseWall04': 1, 'HouseWall05': 1,
        'HouseWall06': 1, 'Stair01': 1, 'House04': 40, 'HouseWall02': 1, 'HouseEtc02': 1}
PROTECTED = {
    'House05': 'All wheel, flume, scrolling water, supporting timber and original wall perimeters remain unchanged. Only six fixed ashlar wall panels are dressed.',
    'HouseWall01': 'Both modular end planes remain original triangles; only front/back masonry panel interiors are dressed.',
    'HouseWall04': 'Both modular end planes and exact L-corner edges remain unchanged; four broad masonry face interiors are dressed.',
    'HouseWall05': 'All sloped shingle faces, original corner outline, roof connection edges and exterior contacts remain unchanged. Only the flat deck and two inward-facing timber trim panels are dressed.',
    'HouseWall06': 'The sloped shingle face and all original roof joining edges remain unchanged. Only the flat deck and two inward-facing timber trim panels are dressed.',
    'Stair01': 'All original ground/top contacts, rail extremities, rung endpoints and tread perimeters are retained. Four broad rail faces and nine tread surfaces gain shallow wear recesses.',
}


def read(folder, path):
    return json.loads((folder / path).read_text())


def write_asset(name, identity):
    folder = ROOT / name
    source = read(folder, 'validation/blender.json')
    summary = read(folder, 'validation/summary.json')
    anchors = read(folder, 'validation/modular-anchors.json')
    motion = read(folder, 'validation/local-motion.json')
    matrices = read(folder, 'validation/matrices-and-posed-bounds.json')
    contract = read(folder, 'validation/final-contract.json')
    placements = read(folder, 'original/placements.json')
    geometry = 'BMD remains byte-identical to its accepted baseline. This deliverable changes only the shared stone/shingle images used by the model.' if name in READONLY else PROTECTED[name]
    comparison = 'EQUIVALENT and exact BMD bytes unchanged' if name in READONLY else 'DIFFERENT because selected panel geometry was remodeled'
    dimensions = '\n'.join(f'- `{file}`: {image["dimensions"]}, {image["mode"]}' for file, image in contract['textures'].items())
    notes = f'''# {name} — {identity}

Owner: ASTRA architecture worker. Branch `codex/lorencia-architecture-timber`.
Date: 2026-09-22. Offline production/validation complete; actual client acceptance pending.

Identity and World1 placement: {identity}. {len(placements)} existing placements; exact positions, XYZ rotations, scale and tile coordinates in `original/placements.json`.

Geometry: {source['original_triangles']} → {source['triangles']} triangles. {geometry} Footprint, origin, orientation, bone pivots and all original corners retained. New recess depths are 0.18 units for ladder wear, 0.4 for roof interiors and 0.65 for stone panels. Explicit modified pairs/perimeters are recorded in `validation/blender.json`. This is conservative architectural edge refinement, with the primary visual gain supplied by the shared material paintings.

Material slots, unchanged order: {', '.join(summary['textures'])}.
Actual material identities differ from filenames: tile_wood01 is coursed stone ashlar; tile_wood03 is green roof shingles. The paintings retain the original block/joint and staggered-shingle layout. The ladder's original reuse of the stone atlas is deliberately retained. Both paintings are 512×512 RGB, packaged through mu_texture as JPEG/OZJ; no material rename or new dependency.

Dimensions of every owned/frozen texture:

{dimensions}

Frozen dependency containers retained byte for byte: {', '.join(anchors['frozen_container_sha256']) or 'None'}.
All other textures, including tile_wood02 timber, ston02 water, tile_ston04 and accepted Architecture01/Masonry paints, are unchanged.

Bind bounds before: `{summary['bounds_before']}`.
Bind bounds after: `{summary['bounds_after']}`.
Every original vertex position/bone survives final conversion within {anchors['max_original_corner_position_component_error']:.9f} units. {anchors['unchanged_triangle_count']} protected triangles retain positions, bone assignments and UVs. Maximum retained UV delta {anchors['max_unchanged_uv_component_error']}; normal component delta {anchors['max_unchanged_normal_component_error']:.9f}; normal direction delta {anchors['max_retained_normal_direction_degrees']:.8f} degrees. Original UV ranges: `{contract['uv_range']}`.

Rig: `{source['bone_order']}`. Names, indices, order, parents and original one-action lock metadata unchanged; {KEYS[name]} action keys. Full local pose evidence: `{motion['samples']}`. All {matrices['action_matrix_samples']} hierarchical action matrices match with maximum component delta {matrices['max_world_matrix_component_delta']}; every action-frame bound matches with maximum component delta {matrices['max_posed_bounds_component_delta']}. Per-frame matrices/bounds evidence is in `validation/matrices-and-posed-bounds.json`.
Full model comparison: {comparison}. Isolated skeleton plus actual action comparison: EQUIVALENT. Geometry and rig equivalence are separate claims.

Special renderer contract: House05 mesh 2 ston02 water and House04 mesh 8 tile_space01 preserve original geometry/UVs and world-time V scroll. HouseWall02 mesh 4 light_02 keeps original additive brightness behavior. HouseWall05/06 keep entire-object HeroTile 4 fade; no new meshes or material slots are introduced. Source excerpts are retained in `../engine-contract.txt`.

Pipeline: official mu_bmd_import.py imports originals. Packed `source.blend` keeps export-excluded REF_ORIGINAL and editable REF_HIGH_POLY candidates, original rig/action and authored mesh. Official mu_bmd_export.py exports all six authored assets. `preserve_contract.py` then restores original protected corners, exact panel-boundary coordinates, bind and action data through supported bmdconv SMD/manifest packaging. This corrects Blender roundtrip quantization; new interior vertices and bevel normals stay authored. Raw official exports and final packing inputs/logs are retained, and raw-versus-final compare EQUIVALENT. Readonly compatibility BMDs are copied exactly and bypass export.

Evidence: full compare, skeleton compare, SMD validation, info, local-motion, matrices-and-posed-bounds, authored-bindings, raw-bone-bindings, modular-anchors, texture-check, final-contract and packed source-audit reports in `validation/`. Original BMD/container bytes independently match pinned Git baseline. Final game hashes are in `summary.json`.

Review: matching original versus actual re-imported final BMD, clear wireframe and reduced-scale previews in `review/comparison.png`. Mill and readonly workshop action sheets use frames 0/14/29 and 0/19/39 respectively. Effect sheets compare neutral diffuse versus labeled Blender additive approximation; illustrative scroll offsets are not engine captures. Actual unchanged World1 assemblies are in `../review/`. The town-interior assembly hides roof caps to illustrate HeroTile 4 target alpha 0; this is an offline approximation.

Sources: immutable game originals, unwrapped textures, official-import blend, bmdconv info/SMD/actions/manifest and placement records in `original/`; editable PNGs and final JPEGs in `textures/`; exact imagegen prompts and untouched generated master PNGs in `../paintings/`; final game files in `exports/`.

Pending: real client loading, lighting/filtering, water blending/scroll, mill motion, roof fade transition, ladder contacts, traversal and matching 1920×1080 screenshots. No runtime/client was used. Terrain placement, height, walk/collision, baked lighting and alpha strips are unchanged.
'''
    (folder / 'notes.md').write_text(notes)
    return f'| {name} | {len(placements)} | {source["original_triangles"]} → {source["triangles"]} | {len(source["bone_order"])} / {KEYS[name]} |'


def main():
    rows = [write_asset(name, identity) for name, identity in IDENTITIES.items()]
    installed = read(ROOT, 'source-installation.json')['installed']
    overview = '''# Architecture02 — Watermill, modular walls, roof caps and ladder

Owner: ASTRA architecture worker. Branch `codex/lorencia-architecture-timber`.
Worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-architecture-timber`.
Date: 2026-09-22. Offline production complete. Client verification pending.

Six new models cover 27 placements. Three accepted compatibility models cover another 17 placements and retain byte-identical BMDs: House04/HouseWall02 from Architecture01 integration 8094616d; HouseEtc02 from Masonry integration f1fbb29c. Original snapshots use current accepted textures at baseline 8094616d.

The two shared filenames are misleading: tile_wood01 is green-weathered coursed stone ashlar, tile_wood03 is green roof shingles. Both are repainted at 512×512 with original layout/material roles retained and previewed across all nine consumers. Static masonry panel edge recesses, roof interior trim and shallow ladder tread wear preserve original silhouettes, openings, joining planes and contacts. Frozen timber, water and all other materials remain unchanged.

| Model | World1 placements | Before → after triangles | Bones / action keys |
| --- | ---: | ---: | ---: |
''' + '\n'.join(rows) + '''

All six remodeled full comparisons are DIFFERENT; the three compatibility models are EQUIVALENT and byte-identical. Every skeleton plus actual action comparison is EQUIVALENT. Full local bind/action translations and Euler rotations, bone names/order/parents and action metadata are retained. Every hierarchical action matrix and every posed bound matches exactly in the explicit SMD matrix audit, including 300 mill action-bone samples and 360 readonly workshop samples. Every original vertex survives within 0.000016 units; protected triangles, normals and UVs pass. No meshes or material slots are added. A separate packed-source versus actual final-BMD audit matches every new panel triangle by material, bone index, position and UV; every new corner retains its intended bone. Only fixed ki12 owns new mill geometry; the five other rebuilt models have one bone each. A direct raw BMD audit confirms no vertex/normal bone mismatch on any owned mesh, including the animated mill; readonly normal bindings are unchanged. The read-only parser from the parallel CartHay worker is retained with its hash.

The official Blender exporter produces each authored mesh. Supported SMD/manifest packaging then restores immutable protected corner records, original panel-boundary coordinates and original bind/action data to avoid Blender roundtrip quantization. New interior geometry and bevel normals remain authored. Raw official and final packaged models compare EQUIVALENT; raw exports and all packing sources/logs remain available. This does not turn the intentional original-versus-remodeled geometry DIFFERENT result into a full-equivalence claim.

House05's complete wheel and ston02 water mesh 2 remain original; its 30-key action and world-time V scroll are preserved. HouseWall05/06 keep HeroTile 4 fade behavior. All frozen container hashes match, including accepted paints and original ston02. Both owned textures have complete consumer ownership recorded in `dependency-ownership.json`.

Exact game files changed (eight total):

''' + '\n'.join(f'- `{file}`' for file in installed) + '''

`source-installation.json` confirms eight isolated-worktree game changes and 316 other World1/Object1 files unchanged. No shared runtime, client, engine, CMake, UI, terrain, main branch or remote changes.

Review images: `review/batch-review.jpg`, `review/texture-comparison.jpg`; all nine per-model `review/comparison.png` sheets; mill/workshop action sheets; indexed-effect neutral/additive sheets. Actual-transform assemblies are `review/town-roof-comparison.jpg`, `town-interior-comparison.jpg` and `west-corners-comparison.jpg`. Companion JSONs retain the exact unchanged World1 placement records. The interior view illustrates the roof target alpha 0 by hiding roof-cap render objects.

All evidence is offline Blender. Grass, 190-unit figure, camera, diffuse lighting, additive shading, V-scroll phases and roof omission are approximations, never client evidence. Actual terrain/collision/baked lighting and unowned buildings are omitted.

Reproduction from this assigned worktree, always explicit cwd, `PYTHONDONTWRITEBYTECODE=1` and the existing converter in `MU_BMDCONV`:

1. `package_textures.py` packages retained built-in imagegen masters with official mu_texture wrapping; copies every frozen dependency unchanged.
2. Blender `-b --python build_source.py` creates packed sources using reviewed read-only helpers from Masonry01/StaticBatch01.
3. `export_all.py` runs official export and preserve_contract supported converter packaging; readonly BMDs are copied exactly.
4. `validate_export.py`, `validate_anchors.py`, `validate_final.py`, `validate_raw_bindings.py`, then Blender `validate_matrices.py` and `validate_authored_bindings.py` check the full engine/material/UV/bone/action/anchor/posed-bound contracts.
5. Blender `render_exports.py`, then `render_joins.py`; bundled Pillow Python `assemble_review.py` produces labeled review sheets.
6. `install_source.py` uses ownership and hash guards for isolated source installation only; `write_notes.py` refreshes local handoffs.

Immutable preservation/import and inspection scripts, original files, packed sources, editable master/final paintings and exact built-in imagegen prompts are retained. Preparation refuses to overwrite original archives. The coordinator alone edits shared handoffs and worklog.

Pending: coordinator and independent acceptance; actual client observation at 1920×1080 when stability permits. No asset is marked verified in client. No unresolved validator rejection remains.
'''
    (ROOT / 'notes.md').write_text(overview)


if __name__ == '__main__':
    main()
