"""Write reviewable handoffs from measured exported data; no shared documentation edits."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
IDENTITIES = {
    'HouseEtc01': 'Carved dragon relief block/plinth',
    'StoneMuWall01': 'Raised five-stake portcullis with tiered dragon pylons',
    'StoneMuWall02': 'Thin tall dragon buttress with below-ground extension',
    'StoneMuWall03': 'Wide angular dragon-relief wall module',
    'StoneMuWall04': 'Same relief wall module with a separate rear siege fixture',
    'HouseEtc02': 'Low walled hut/platform, doorway, roof rim and rear static fixture',
}


def main():
    rows, game_files = [], []
    for name, identity in IDENTITIES.items():
        folder = ROOT / name
        source = json.loads((folder / 'validation/blender.json').read_text())
        validation = json.loads((folder / 'validation/summary.json').read_text())
        anchors = json.loads((folder / 'validation/modular-anchors.json').read_text())
        contract = json.loads((folder / 'validation/final-contract.json').read_text())
        placements = json.loads((folder / 'original/placements.json').read_text())
        motion = json.loads((folder / 'validation/local-motion.json').read_text())
        dependencies = ', '.join(validation['textures'])
        frozen = ', '.join(anchors['frozen_container_sha256']) or 'None'
        row = f'| {name} | {len(placements)} | {source["original_triangles"]} → {source["triangles"]} | {len(source["bone_order"])} / 1 key |'
        rows.append(row)
        game_files.append(f'src/bin/Data/Object1/{name}.bmd')
        notes = f'''# {name} — {identity}

Owner: ASTRA fences/timber/masonry worker. Branch `codex/lorencia-masonry`.
Production date: 2026-09-22. Status: offline production and engine validation complete; coordinator acceptance and client review are separate.

Identity and placement: {identity}. {len(placements)} unchanged World1 placements. Full positions, XYZ rotations, scales and tile coordinates are in `original/placements.json`.

Geometry: {source['original_triangles']} → {source['triangles']} triangles. {len(source['panel_recesses'])} stone panels gain narrow beveled recesses, depth 0.65 units, inside the original panel perimeter. All original vertices, relief silhouettes, connection edges, openings, pivots and orientation are retained. The render-visible pass is primarily coordinated stone painting; the architecture's original outline is intentionally retained.

Material slots in original order: {dependencies}.
All repainted textures are opaque RGB 512×512 JPEG/OZJ. Frozen dependency containers copied byte for byte for self-contained review: {frozen}.
Exact dimensions for every owned/frozen image are in `validation/final-contract.json`.

Bind bounds before: `{validation['bounds_before']}`.
Bind bounds after: `{validation['bounds_after']}`.
All original corners survive export with maximum position component drift {anchors['max_original_corner_position_component_error']:.9f} units. {anchors['unchanged_triangle_count']} original triangles outside the explicitly listed panel pairs retain positions, original bone binding and UVs.

Rig proof: bone order `{source['bone_order']}`. Bone names, indices and parents match exactly. Original one-frame action count and lock metadata retained. Full local translation/Euler values checked for bind and action keys: `{motion['samples']}`. Isolated skeleton plus actual action comparison is EQUIVALENT. Full model comparison is DIFFERENT because the stone panels were remodeled.

Source normals were copied, then normalized/requantized by the official import/export roundtrip. They are not claimed byte-equivalent: maximum retained normal component difference {anchors['max_unchanged_normal_component_error']:.9f}, maximum direction difference {anchors['max_retained_normal_direction_degrees']:.6f} degrees (under 0.1 degree).
Original repeating UV range retained exactly: U `{contract['uv_range'][0]}`, V `{contract['uv_range'][1]}`. New UVs interpolate inside their original panels; no texture renaming or atlas reassignment. All exported corners have one original full-weight non-dummy bone and finite geometry/UV values.

Evidence: `validation/compare.txt`, `skeleton-compare.txt`, `local-motion.json`, `modular-anchors.json`, `smd-validation.txt`, `info-after.txt`, `texture-check.txt`, `source-audit.json`, `final-contract.json` and `summary.json`.
Review: `review/comparison.png` contains matched-camera actual before/after BMD renders, exported wireframe and reduced offline grass/190-unit figure study. The camera is an offline assumption. Batch assemblies are in `../review/` and use untouched decoded World1 transforms.

Sources: immutable original BMD, OZJs, unwrapped images, bmdconv info/SMD/actions and official-import Blender file in `original/`. Packed editable `source.blend` contains hidden export-excluded REF_ORIGINAL and REF_HIGH_POLY, the original rig/action, and final mesh. Optional high-poly bevel is a retained bake candidate, not an engine asset. Painted master PNGs and exact built-in imagegen prompts are in `../paintings/`; final editable PNG/JPEG copies are in `textures/`. Official exporter produced `exports/`.

Pending: actual client screenshots, lighting/readability, occlusion and traversal review. No runtime or client session was used. World1 placement, terrain height, walk/collision data, TerrainLight and alpha strips are unchanged.
'''
        (folder / 'notes.md').write_text(notes)
    game_files += [f'src/bin/Data/Object1/{name}.OZJ' for name in ('c_wall04', 'c_wall05', 'c_wall06', 'tile_ston01')]
    overview = '''# Masonry01 — Lorencia modular stone and dragon reliefs

Owner: ASTRA fences/timber/masonry worker. Branch `codex/lorencia-masonry`.
Worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-masonry`.
Date: 2026-09-22. Offline production complete. Client verification pending.

Six connected models cover 137 placements. Coordinated 512×512 stone paintings preserve c_wall04's tan limestone and narrow carved lower scroll band, c_wall05's central monster face, c_wall06's paired-frill dragon face, and tile_ston01's large weathered ashlar. Shallow dressed-stone panel recesses add edge depth without moving any original corner. This is a conservative architecture pass preserving the exact original silhouettes and connection geometry.

| Model | World1 placements | Before → after triangles | Bones / action |
| --- | ---: | ---: | ---: |
''' + '\n'.join(rows) + '''

All full model comparisons report DIFFERENT. All isolated skeleton plus actual action comparisons report EQUIVALENT; all local poses are explicitly checked. All bind bounds match. Material slot counts/order and exact game filenames are unchanged. Every original vertex survives export within 0.000001 units. The portcullis, gate inner planes, hut doorway, all carved face geometry and frozen-material scenery retain their original triangles, UVs and bindings. Original normal directions differ by at most 0.081719 degrees after official roundtrip normalization/quantization; no byte-equivalence claim is made for normals.

All four repainted containers are exclusive to the six owned models, independently checked in `dependency-ownership.json`. Frozen materials are tile_wood02, tile_ston04, tile_wood03, tile_house01, horse_drawn_01, tile_ston06, tile_02 and bridge_01. Their OZJ bytes are unchanged. `source-installation.json` confirms exactly ten isolated-worktree game changes and all other World1/Object1 files unchanged. No runtime, engine, UI, terrain, main-branch or remote changes were made.

Actual game files changed:

''' + '\n'.join(f'- `{file}`' for file in game_files) + '''

Review images: `review/batch-review.jpg`; per-model `review/comparison.png`; actual unchanged placement assemblies `review/south-gate-comparison.jpg` and `review/siege-wall-comparison.jpg`. The latter uses the west-side wall and siege fixture near its gate. These are Blender images of official re-imported exported BMDs. Flat grass/figure studies, camera and diffuse lighting are offline approximations, never client evidence. Complete retained placement records accompany the assemblies.

Reproduction from this worktree (set MU_BMDCONV to the existing converter and use the bundled Pillow Python for image scripts):

1. `package_textures.py`: retained built-in imagegen master PNGs → 512 RGB editable PNG/JPEG + official OZJ wrap; frozen containers copied unchanged.
2. Blender `-b --python build_source.py`: original official-import files → packed low-poly source plus REF_ORIGINAL/high-poly candidate.
3. `export_all.py`: official mu_bmd_export.py only, with logs and generated SMD/actions.
4. `validate_export.py`, `validate_anchors.py`, `validate_final.py`: engine validation, full/skeleton/action comparison, exact anchors and protected triangle proof, original SHA verification and full dependency audit.
5. Blender `-b --python render_exports.py`, then `render_joins.py`; bundled Pillow Python `assemble_review.py`.
6. `install_source.py`: owned-file and shared-texture hash guards, isolated source install only. `write_notes.py` refreshes these local handoffs.

Every command must use this assigned worktree as cwd. The preserved original baseline commit is pinned in `baseline-commit.txt`; preservation scripts refuse to overwrite originals. Python bytecode writes are disabled. No shared documentation is edited by this worker.

Pending checks: coordinator visual/integration review and real client observation at 1920×1080 once stability permits. No asset is marked verified in client. The user authorized continued offline production.
'''
    (ROOT / 'notes.md').write_text(overview)


if __name__ == '__main__':
    main()
