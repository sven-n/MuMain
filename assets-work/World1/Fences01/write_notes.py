"""Write local deliverable notes from retained engine and Blender validation evidence."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
NAMES = ('Fence01', 'Fence02', 'Fence03', 'Fence04')
IDENTITIES = {'Fence01': 'Two-post rustic timber fence with two sagging rails',
              'Fence02': 'Shallow ornamental curb, L-shaped corner',
              'Fence03': 'Shallow ornamental curb, 200-unit straight module',
              'Fence04': 'Shallow ornamental curb, 100-unit straight module'}


def format_bounds(bounds):
    return ' / '.join(', '.join(f'{value:.5f}' for value in point) for point in bounds)


def asset_note(name):
    root = ROOT / name
    data = json.loads((root / 'validation/blender.json').read_text())
    summary = json.loads((root / 'validation/summary.json').read_text())
    joins = json.loads((root / 'validation/modular-joins.json').read_text())
    placements = json.loads((root / 'original/placements.json').read_text())
    old_triangles = {'Fence01': 36, 'Fence02': 16, 'Fence03': 10, 'Fence04': 10}[name]
    texture_note = ('`src/bin/Data/Object1/tile_wood02.OZJ` (128x128) is frozen, byte-identical. '
                    'Only geometry and UVs changed; no painted replacement was allowed for this shared texture.'
                    if name == 'Fence01' else '`src/bin/Data/Object1/joint.OZJ` is an exclusive Fence02/03/04 dependency, '
                    'repainted from 32x32 to 512x512 RGB. All three consumers belong to this batch. '
                    'Stone upper field and dark gothic metal lower band remain visible on both front and rear faces and end caps.')
    geometry = ('Chamfered post caps and bases, correct longitudinal timber UV direction, and solid closed triangular rails. '
                'Original sag/bend points, rail/post interfaces, opening, dimensions and pivot retained.' if name == 'Fence01'
                else 'Separate dressed-stone modules with narrow inset joints and joint-edge bevels. '
                'Original external connection cross sections, height, taper and end planes retained; the L-corner miter is exact. '
                'No placement or collision data changed.')
    text = f'''# {name} — {IDENTITIES[name]}

Status: exported and validated offline; not verified in client. Owner: ASTRA fence worker,
branch `codex/lorencia-fences`; worktree `MuMain-lorencia-fences`.

{geometry}

## Actual World1 placement

{len(placements)} instances, object type {81 + NAMES.index(name)}. Full untouched placement records are in
`original/placements.json`. Example tile: {placements[0]['tile']}; world position:
{placements[0]['position']}; rotation degrees: {placements[0]['rotation']}; scale {placements[0]['scale']}.

The names are loader identifiers, not inferred identities: Fence02–04 are 22.6887-unit-high
curb pieces. `MapManager.cpp:1059` loads Fence01–04 through MODEL_FENCE01+i; the four enums
are at `_enum.h:790`. Source search found no fence-specific mesh-index render code. One
material/mesh remains in all four, in the original slot.

## Counts, bounds and dependencies

- Triangles: {old_triangles} → {data['triangles']} (prop budget 1500).
- Before bind minimum / maximum: {format_bounds(data['bounds_before'])}.
- Authored minimum / maximum: {format_bounds(data['bounds_after'])}.
- Engine-reported before bounds: {format_bounds(summary['bounds_before'])}.
- Engine-reported after bounds: {format_bounds(summary['bounds_after'])}.
- One original root bone, `{data['bone_order'][0]}`, index 0, parent -1. Names, order and parents identical.
- One action, index 0, one key, lock=0. Bind and every local action translation/rotation unchanged.
- {texture_note}

## Validation

`validation/smd-validation.txt` retains full original/replacement reference and animation
validation output. `validation/info-after.txt`, `compare.txt`, `skeleton-compare.txt` and
`local-motion.json` retain engine output and every local-transform comparison.

Full model comparison (geometry intentionally differs):

```text
{(root / 'validation/compare.txt').read_text().strip()}
```

Isolated skeleton plus actual action comparison:

```text
{(root / 'validation/skeleton-compare.txt').read_text().strip()}
```

Full-model equivalence is not claimed. All local position and Euler component deviations
are zero in the retained converter output. One valid full-weight bone per vertex, no dummy
binding, one UVMap in [0,1], one opaque diffuse material, finite nondegenerate triangles,
identity export transforms and packed image dependencies pass. Texture loader checks pass.
No alpha texture or render flag was introduced.

`validation/modular-joins.json` checks original interface corners against re-imported final
BMD vertices: maximum error {joins['max_connection_vertex_error']:.9f} units, tolerance 0.002.
The three curb assets retain their connection vertices exactly. Fence01's rotated imported
rig incurs below 0.002-unit Blender conversion drift, far below 0.00002 terrain tiles.
The source/export bounds match at bmdconv's printed precision.

`source.blend` retains the original rig/action, hidden `REF_ORIGINAL`, excluded bevel-ready
`REF_HIGH_POLY`, export geometry and packed images. `validation/source-audit.json` proves
packed images and reference exclusion. `original/` retains untouched BMD/container, decoded
texture, converter SMD/actions/info, official imported blend and inspection geometry/render.

## Review and pending checks

`review/comparison.png`: same-camera original versus re-imported game BMD, wireframe and
reduced-scale proxy. All images are **offline Blender evidence**, never client screenshots.
The scale study uses 100-unit ground repeats and a 190-unit proxy; it is not a measured
client camera. `../review/modular-joins.png` shows real unchanged World1 placement transforms
for a town corner and straight run, without unrelated terrain/buildings.

No runtime or client was operated. Client loading/logs, actual diffuse lighting, contact,
collision readability and 1920x1080 before/after screenshots remain pending under the user's
offline authorization. See `../notes.md` for reproduction and batch dependency scope.
'''
    (root / 'notes.md').write_text(text)


def batch_note():
    dependency = json.loads((ROOT.parent / 'coordination/dependency-map.json').read_text())
    consumers = dependency['texture_consumers']['src/bin/Data/Object1/tile_wood02.OZJ']
    text = '''# Fences01 — Lorencia timber fence and ornamental curbs

2026-09-22. Owner: ASTRA fence worker. Branch `codex/lorencia-fences`, isolated worktree
`/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-fences`.
Status: four assets exported and validated offline; coordinator review/integration and
client verification pending. This directory is the worker's complete deliverable.

| Asset | Actual identity | Placements | Triangles before → after | Texture |
|---|---|---:|---:|---|
| Fence01 | Two-post, two-rail timber fence | 180 | 36 → 116 | tile_wood02, frozen 128² |
| Fence02 | Low ornamental curb, L corner | 33 | 16 → 112 | joint, 512² |
| Fence03 | Low ornamental curb, 200-unit straight | 38 | 10 → 112 | joint, 512² |
| Fence04 | Low ornamental curb, 100-unit straight | 17 | 10 → 56 | joint, 512² |

The three shallow curb models are not standing fences. Their 22.6887-unit height,
sloped faces, exact external end profiles and L-corner miter are preserved. Individual
stone joints are inset within that original envelope. The old material's upper stone and
lower decorated dark band remain on front, rear and ends. Timber retains its recognizable
bent rails and openings; frozen shared timber limits this asset to geometry/UV improvement.

## Exact authorized game changes

- `src/bin/Data/Object1/Fence01.bmd`
- `src/bin/Data/Object1/Fence02.bmd`
- `src/bin/Data/Object1/Fence03.bmd`
- `src/bin/Data/Object1/Fence04.bmd`
- `src/bin/Data/Object1/joint.OZJ`

The complete dependency group for joint.OZJ is Fence02/Fence03/Fence04. Its original 32²
texture was repainted through the built-in imagegen tool. Exact prompt, raw editable PNG
master, provenance and deterministic size/wrap script are retained in `paintings/`.
Final diffuse is 512² RGB JPEG/OZJ; no alpha, PBR, normal map or render-flag change.

`tile_wood02.OZJ` is frozen because it is shared with these other consumers:
''' + ', '.join(consumers) + '''.
It is retained byte-identically in Fence01's original/export folders, and is not a game
replacement. No texture renaming or new game filename avoids ownership boundaries.

## Evidence

All four assets pass bmdconv reference/animation validation and texture loader checks.
All retain one original mesh/material, one original bone and one one-frame action, lock=0.
Bone names/order/parents and every bind/action local translation/rotation are unchanged;
isolated rig/action BMDs compare EQUIVALENT. Full compares are DIFFERENT for deliberate
geometry changes. All before/after engine bind bounds agree at printed precision.
New vertices each have one valid full-weight bone; UVs/materials/export exclusion checked.
Original curb connection vertices are exact; timber interfaces differ by at most 0.00135
units after the official Blender conversion, within a documented 0.002-unit tolerance.

Each asset folder includes packed source.blend with REF_ORIGINAL and excluded high-poly
source, untouched originals, exported game files, final editable textures, validation
logs, notes and labeled before/after/wireframe/reduced-scale comparison. All final review
images use a re-import of the exported BMD, not the authoring mesh.

Batch sheet: `review/batch-review.jpg`; per-asset sheets: `<asset>/review/comparison.png`.
`review/modular-joins.png` uses the original World1 position/rotation/scale of actual town
corner and straight-run instances. Full placement lists and render caveats are retained.
These are Blender renders, **not client evidence**. No runtime or client was touched.

## Reproduce

Run commands from this worktree or the integrated checkout. Set `MU_BMDCONV` to the built
converter and `PYTHONDONTWRITEBYTECODE=1`. Blender requires enabled Source Tools.
Python packaging/review assembly requires Pillow.

```sh
export PYTHONDONTWRITEBYTECODE=1
export MU_BMDCONV=/path/to/bmdconv
python3 assets-work/World1/Fences01/package_textures.py
/Applications/Blender.app/Contents/MacOS/Blender -b --python assets-work/World1/Fences01/build_source.py
python3 assets-work/World1/Fences01/export_all.py
python3 assets-work/World1/Fences01/validate_export.py
/Applications/Blender.app/Contents/MacOS/Blender -b --python assets-work/World1/Fences01/render_exports.py
/Applications/Blender.app/Contents/MacOS/Blender -b --python assets-work/World1/Fences01/render_joins.py
python3 assets-work/World1/Fences01/assemble_review.py
python3 assets-work/World1/Fences01/write_notes.py
python3 assets-work/World1/Fences01/install_source.py
```

`prepare_originals.py` is baseline-only and refuses to overwrite original folders.
`install_source.py` writes only the listed five owned source game files; it never touches
the shared runtime. It checks all other World1/Object1 files and frozen timber unchanged.
The installation report is `source-installation.json`.

Pending: coordinator integration and combined dependency verification; actual client load,
lighting, zoom/readability, contacts/collision and serial 1920x1080 before/after captures.
No engine/CMake, UI, character, terrain placement/height/walk/light/alpha-strip files changed.
'''
    (ROOT / 'notes.md').write_text(text)


if __name__ == '__main__':
    for name in NAMES:
        asset_note(name)
    batch_note()
