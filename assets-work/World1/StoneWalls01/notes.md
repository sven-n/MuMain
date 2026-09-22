# StoneWalls01 offline handoff

Owner: ASTRA review/ironwork/stone-wall worker. Branch: `codex/lorencia-stone-walls`.
Worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-stone-walls`.
Five assets complete for coordinator review; 18 actual World1 placements. Client verification pending.

| Model | Actual identity | Placements | Triangles before → after | Bones |
|---|---|---:|---:|---:|
| StoneWall01 | Raised portcullis gateway with rough masonry piers | 8 | 318 → 414 | 1 |
| StoneWall02 | Lowered portcullis gateway with rough masonry piers | 6 | 318 → 414 | 1 |
| StoneWall03 | Rough stone wall with preserved siege fixture | 2 | 293 → 333 | 4 |
| StoneWall05 | Plain rough stone wall module; no current World1 placement | 0 | 60 → 100 | 2 |
| StoneWall06 | Rough masonry pier with animated heraldic banner | 2 | 56 → 88 | 9 |

StoneWall06 retains its one 25-key action; other models retain one action with one key.
Rough charcoal/olive stone preserves the original two-course layout and remains distinct from the
warmer carved limestone of accepted Masonry01. Broad chipped facets sharpen material readability.
Only selected stone-panel interiors receive shallow 0.65-unit recesses; all original vertices and
perimeter edges survive, including exact source gateway clearance and wall connection geometry.

## Changed game files

- `src/bin/Data/Object1/StoneWall01.bmd`
- `src/bin/Data/Object1/tile_01.OZJ`
- `src/bin/Data/Object1/tile_03.OZJ`
- `src/bin/Data/Object1/StoneWall02.bmd`
- `src/bin/Data/Object1/StoneWall03.bmd`
- `src/bin/Data/Object1/StoneWall05.bmd`
- `src/bin/Data/Object1/StoneWall06.bmd`

The two 512 × 512 RGB paintings, tile_01 and tile_03, have no consumers outside this owned group;
`dependency-ownership.json` records the full inventory check. Frozen containers remain byte-identical:
`tile_02.OZJ`, `bridge_01.OZJ`, `horse_drawn_01.OZJ`, `badge_01.OZJ`, `badge_03.OZT`.
The banner alpha, hidden RGB fringe, original material order and filenames remain unchanged.

## Validation

- Official Blender import/export; packed source with excluded original and high-poly references.
- All model/action SMDs validate. Five full geometry compares report DIFFERENT as intended.
- Five skeleton/action comparisons EQUIVALENT; exact bone names/order/parents and action metadata.
- Seven frozen-material components independently EQUIVALENT, with matching bone/position/UV corners.
- All original source vertices/boundaries unchanged. Actual exports: max corner drift 0.000366 units
  on the nine-bone banner hierarchy, zero for the four other models. Printed bind bounds agree.
- All 25 banner keys checked: max local translation drift 0.000078 units; rotations unchanged.
- Retained normals are normalized/requantized by the official pipeline and measured explicitly;
  they are not claimed byte-equivalent. All material/UV/skin/finite-geometry checks pass.
- Five frozen containers hash-identical; original game archives match the pinned baseline commit.
- Isolated source installation changed only seven owned paths; 317 other World1/Object1 files unchanged.

## Review evidence

`review/batch-review.jpg`, each asset’s `review/comparison.png`, `review/texture-comparison.jpg`,
`review/texture-repeat.jpg`, `review/banner-action-comparison.jpg`, and
`review/raised-gates-comparison.jpg` / `review/lowered-gates-comparison.jpg`.
Assembly sheets use actual untouched World1 placement transforms with identical fitted cameras.
They reconstruct only owned geometry in Blender; actual terrain, collision and baked lighting are
omitted. Some original placements are tilted/buried; these are preserved. No client evidence is claimed.

## Reproduction

Run all commands explicitly from this worktree. Set `MU_BMDCONV` to the coordinator-supplied
`/Users/webproduktion3/Documents/claude-test-mumain/MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv`.
Blender is `/Applications/Blender.app/Contents/MacOS/Blender`; packaging/validation uses the supplied
Pillow Python runtime. `prepare_originals.py` is initial-only and refuses to overwrite originals.
Built-in imagegen prompts and raw/full-resolution editable PNG masters are in `paintings/`.
Run `package_textures.py`, Blender `build_source.py`, `export_all.py`, `validate_export.py`,
`validate_anchors.py`, `validate_final.py`, `validate_protected.py`, Blender `render_exports.py`,
`render_joins.py`, `render_actions.py`, then `assemble_review.py`, `assemble_extra.py` and
`install_source.py`. `write_notes.py` regenerates this measured handoff.
Reviewed StaticBatch01 and Masonry01 helpers are reused read-only within the same worktree.

**Pending:** coordinator review/integration and actual client observation. No known blocking defect;
no runtime/client, engine/CMake, UI, placement/collision/terrain or shared-document changes.
