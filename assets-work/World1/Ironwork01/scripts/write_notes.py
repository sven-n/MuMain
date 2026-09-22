"""Generate asset handoff notes from the final ironwork validation results."""
import hashlib
import json
from pathlib import Path
import sys
sys.dont_write_bytecode=True
from config import PROPS,REPOSITORY,ROOT

IDENTITIES={'SteelWall01':'Corner post with two return rails',
            'SteelWall02':'Four-section straight fence run',
            'SteelWall03':'Single straight fence module',
            'SteelDoor01':'Tall ornate gothic iron gate',
            'HouseEtc03':'Freestanding iron cage built from transparent panels'}


def asset_notes(name):
    folder=ROOT/name
    summary=json.loads((folder/'validation/summary.json').read_text())
    source=json.loads((folder/'validation/source.json').read_text())
    joins=json.loads((folder/'validation/joins.json').read_text())
    placements=json.loads((folder/'original/placements.json').read_text())
    before,after=summary['geometry']['triangles_before_after']
    bounds=summary['geometry']['bounds_before_after']
    lines=[f'# {name} — {IDENTITIES[name]}','',
           'Owner `/root/reviewer`; branch `codex/lorencia-ironwork`.',
           '**Status: validated offline; no client observation.**','',
           f'World1 placements: {len(placements)}. Exact position/rotation/scale/tile records: `original/placements.json`.',
           f'Game model: `src/bin/Data/Object1/{name}.bmd`.',
           'Complete ordered texture dependencies: '+', '.join('`'+t+'`' for t in summary['mesh_texture_order'])+'.','',
           f'Triangles: **{before} → {after}**. Full original/final BMD compare: **{summary["full_compare"]}**.',
           'Skeleton/actions-only BMD compare: **EQUIVALENT**. Names/order/parents, material order and one-key action metadata retained.',
           f'Original bone order: `{source["bone_order"]}`.',f'Action metadata: `{source["action_meta"]}`.','',
           'Actual converted BMD bind bounds:','',f'- Before: `{bounds[0]}`',f'- After: `{bounds[1]}`',
           f'- Largest component drift: `{summary["geometry"]["max_bound_component_difference"]:.6f}` game units.',
           f'- All {joins["all_original_anchors_preserved"]} original position/bone anchors retained; maximum final distance `{joins["maximum_exported_anchor_distance"]:.6f}` units.',
           '- Saved-source original connection vertices and bind extrema remain exactly equal.','',
           'Physical bar middles use narrow eight-sided profiles, transitioning back to every original endpoint over 2 game units. '
           'The 0.35-unit corner cuts stay inside the original section. Endpoints, spearheads, nonrectangular corner rail joints and alpha panels are protected. '
           'No placement, collision, orientation or opening adjustment compensates for the model. HouseEtc03 is a deliberate texture-only pass because all of its geometry is alpha panels.','',
           'Shared artwork: `steel_barred_b.jpg` 512×512 opaque RGB; `steel_barred_a.tga` and `steel_barred_door.tga` 256×512 RGBA where present. '
           'The exact original 64×128 alpha fields are enlarged 4× bilinearly. Continuous dark metal RGB underneath zero-alpha pixels prevents new black/white matte fringes. '
           'BMD material filenames and OZJ/OZT conventions are unchanged.','',
           'Evidence: `validation/compare.txt`, `commands.json`, `skeleton-compare.txt`, `preservation.json`, `summary.json`, `joins.json`, and `source-audit.json`. '
           'Each protected alpha material is separately compared EQUIVALENT, including all corners, original rigid bone and UV coordinates. '
           'All final triangles have nonzero area, one UV set and a single original bone binding; no collapsed UVs. '
           'Small newly created chamfer UV degeneracies are corrected locally and counted in `validation/source.json`.','',
           'Untouched BMDs, all texture dependencies, info, official imported source, SMD/actions and placement records remain in `original/`. '
           'Packed `source.blend` contains exact `REF_ORIGINAL`, export geometry and excluded editable `REF_HIGH_POLY` for remodeled support profiles. '
           'The cage needs no high-poly source. Official Blender import/export tools and `mu_texture.py` are used.','',
           '`review/comparison.png` contains matching-camera original/final actual BMD renders, wireframe and reduced-scale views; '
           '`review/reverse-offline.png` checks the opposite side. Parent `review/` contains two modular assemblies reconstructed from the recorded World1 transforms. '
           'Those reconstructions are offline XYZ transform studies, with no client, terrain, collision or lighting simulation.','',
           'Pending: serial client review at actual scale, per-vertex lighting, alpha testing/sorting and real 1920×1080 capture. '
           'No shared runtime folder or client session was used.']
    (folder/'notes.md').write_text('\n'.join(lines)+'\n')
    return f'| {name} | {IDENTITIES[name]} | {len(placements)} | {before} → {after} | {len(source["bone_order"])} | {summary["full_compare"]} |'


rows=[asset_notes(name) for name in PROPS]
notes='''# Ironwork01 — modular iron fencing, gate and cage

2026-09-22. Owner: `/root/reviewer`. Branch: `codex/lorencia-ironwork`. Worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-ironwork`.

**Five assets / 44 World1 placements rebuilt and validated offline. No client verification.** The three shared textures form one complete dependency group; no consumer outside this batch is affected. `dependency-inventory.json` retains actual identities, all placements and complete texture consumers.

| Model | Inspected identity | Placements | Triangles | Bones | Full compare |
|---|---|---:|---:|---:|---|
'''+ '\n'.join(rows) + '''

## Exact game files changed

- `src/bin/Data/Object1/SteelWall01.bmd`
- `src/bin/Data/Object1/SteelWall02.bmd`
- `src/bin/Data/Object1/SteelWall03.bmd`
- `src/bin/Data/Object1/SteelDoor01.bmd`
- `src/bin/Data/Object1/HouseEtc03.bmd`
- `src/bin/Data/Object1/steel_barred_b.OZJ` — 32² → 512² opaque RGB support atlas.
- `src/bin/Data/Object1/steel_barred_a.OZT` — 64×128 → 256×512 RGBA bar panels.
- `src/bin/Data/Object1/steel_barred_door.OZT` — 64×128 → 256×512 RGBA gothic ornament.

Installed only into this worktree's source Data. `installed-files.json` records these eight hashes and proves 316 other Object1/World1 files unchanged. No runtime/client, terrain placement/height/walk/light/alpha strip, engine, CMake, UI, other-map or other-worker asset changes.

## Art and modular preservation

Original brass-tinted bars become aged blackened iron with cool gray worn edges, sparse rusty collars, hammered surface detail and recognizable gothic scrollwork. The three continuous color masters were painted with built-in imagegen using original texture references; exact prompts and raw files are retained in `generation-prompts.json` and `textures/*-generated.png`. Editable layered OpenRaster masters are `textures/*.ora`; final color, alpha, PNG/JPEG/TGA and containers remain alongside them.

Solid support profiles are remodeled locally: one bar in SteelWall01, eight in SteelWall02, two in SteelWall03 and five in SteelDoor01. Each new middle has eight edges, then transitions to the exact four original end corners within 2 units. Original spearheads, irregular corner returns, every endpoint and every alpha panel remain. No blind subdivision, opaque fill planes, filename changes or placement compensation. HouseEtc03 is entirely transparent panels and deliberately retains its original geometry.

All saved-source original positions remain present exactly, not merely the global bounds. Final engine-parsed original anchors match within **0.000578** game units, including bone assignment; every original alpha plane's positions/UV/bones compare equivalent. Maximum bind-bound component drift is **0.000287** units from ordinary floating-point conversion. Original modular end profiles and gate/cage opening dimensions therefore remain intact. `validation/joins.json` and `source-audit.json` give per-model proof.

Both original alpha fields are retained and enlarged bilinearly, without repainting negative spaces. At alpha ≥128, bar coverage is 14.099% → 14.030% and door coverage 56.067% → 56.122%. Sampling at alpha ≥64 changes texel counts due to interpolation, 14.319% → 16.516% and 58.630% → 61.779%; this is explicitly recorded in `textures/alpha-preservation.json`. The scalar masks are exact bilinear resamples, and the light/dark `alpha-review.png` exposes their retained holes and edges. Original low-resolution mask irregularities are not silently replaced by the new color painting.

## Technical evidence

All actual exported model/action SMDs pass bmdconv validation; all final containers pass mu_texture checks. Model full compares report DIFFERENT for the four intentional support remodels and EQUIVALENT for the cage. All five isolated skeleton/actions comparisons are EQUIVALENT. Do not call the remodeled geometry fully equivalent.

Original bone names/order/parents and one-key action index/order/lock metadata are retained, including SteelWall02's four independent roots. Every bind/action local pose sample is compared. Maximum local translation delta is 0.000023 units, and maximum wrapped Euler component difference is 0.0000003072 radians. Full node lists and values are retained in each summary. Source rigs were reused and their source metadata audited.

Mesh/material order is exact. Initial exporter order and UV corner issues were corrected before this final validated package; no rejection remains unresolved. Final geometry has finite values, one rigid bone per vertex, one UV set, no zero-area triangles and no collapsed UV triangles. New narrow chamfer UVs receive a small local texel footprint where projection was degenerate, leaving all alpha UVs untouched. The exporter derives normals for remodeled support faces.

Each asset retains untouched originals and bmdconv info/SMD/actions, packed original and edited source.blend with `REF_ORIGINAL`, excluded editable high-poly support source where useful, complete game exports, full comparisons, texture checks, source/skin/UV checks, bind bounds and local-pose proof. `build-export-output.txt`, `validation-output.txt`, `preservation-output.txt`, `source-audit-output.txt`, `joins-output.txt` and `texture-packaging-output.txt` retain command evidence.

## Review images

Start with `review-overview.jpg`, `alpha-review.png` and each model's `review/comparison.png`. Matching cameras/light compare original and actual reimported final BMDs; wireframes, reverse views and reduced-scale images are retained. `review/north-gate-comparison.jpg` and `review/prison-corner-comparison.jpg` reconstruct selected actual World1 placement records, with identical transforms before/after. `review/assembly-context.json` lists every chosen record. These are offline XYZ placement reconstructions; terrain, client projection, collision and runtime lighting are not simulated.

The independent review of Trees01 and TimberProps01 requested before production is isolated in `independent-review/`, committed separately as `8788f85c17046a5ba95dfc45af9208f9b09f4bba`. That review accepted both batches offline after viewing their sheets and independently validating the actual source Data BMDs/action SMDs and hashes. It edits no production assets from those batches.

## Reproduction and pending checks

Run from the assigned checkout. Pillow Python: `/Users/webproduktion3/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/bin/python3`. Blender: `/Applications/Blender.app/Contents/MacOS/Blender -b --python-exit-code 1 --python`. Prefix the following scripts with `assets-work/World1/Ironwork01/scripts/`.

1. Python `package_textures.py` packages the retained raw artwork, no new generation needed.
2. Blender `import_originals.py`, then `build_and_export.py`.
3. Python `validate_exports.py`, `validate_preservation.py`, `audit_joins.py`, `audit_alpha.py`; Blender `audit_sources.py`.
4. Blender `render_review.py` and `render_assembly.py`; Python `assemble_review.py`.
5. Python `install.py` and `write_notes.py`.

`prepare_originals.py` is baseline-only and refuses to replace preserved originals after installation. The guarded installer writes only the eight owned source Data paths and verifies original/intended hashes. All output paths are scoped to this deliverable; prior common scene helpers are reused read-only.

Pending: coordinator acceptance/integration and serial real-client alpha, light, placed-scale, gate clearance and 1920×1080 visual evidence when stable. **No asset in this batch is verified in client.** No runtime install occurred and no engine code was changed.
'''
(ROOT/'notes.md').write_text(notes)
source=REPOSITORY/'src/source/World/MapInfra/MapManager.cpp'
lines=source.read_text().splitlines()
report=['Read-only engine reference inspection: MapManager.cpp',
        'sha256 '+hashlib.sha256(source.read_bytes()).hexdigest(),
        'Steel walls/door and cage use original model constants and filenames. No model-specific render switch was found by named-symbol search in ZzzObject.cpp.','']
for low,high in ((1049,1056),(1082,1087)):
    report.extend(f'{i}: {lines[i-1]}' for i in range(low,high+1))
    report.append('')
(ROOT/'engine-reference-audit.txt').write_text('\n'.join(report).rstrip()+'\n')
