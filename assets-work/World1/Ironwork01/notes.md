# Ironwork01 — modular iron fencing, gate and cage

2026-09-22. Owner: `/root/reviewer`. Branch: `codex/lorencia-ironwork`. Worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-ironwork`.

**Five assets / 44 World1 placements rebuilt and validated offline. No client verification.** The three shared textures form one complete dependency group; no consumer outside this batch is affected. `dependency-inventory.json` retains actual identities, all placements and complete texture consumers.

| Model | Inspected identity | Placements | Triangles | Bones | Full compare |
|---|---|---:|---:|---:|---|
| SteelWall01 | Corner post with two return rails | 2 | 52 → 84 | 1 | DIFFERENT |
| SteelWall02 | Four-section straight fence run | 21 | 72 → 328 | 4 | DIFFERENT |
| SteelWall03 | Single straight fence module | 5 | 18 → 82 | 1 | DIFFERENT |
| SteelDoor01 | Tall ornate gothic iron gate | 8 | 58 → 218 | 1 | DIFFERENT |
| HouseEtc03 | Freestanding iron cage built from transparent panels | 8 | 16 → 16 | 1 | EQUIVALENT |

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
