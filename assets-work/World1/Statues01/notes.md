# Statues01 offline handoff

2026-09-22. ASTRA reviewer/production worker. Branch `codex/lorencia-monuments`.
Worktree `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-monuments`.
Six newly completed monuments/graves cover 30 World1 placements; one additional fountain compatibility placement.

| Model | Actual identity | Placements | Triangles | Bones | bmdconv compare |
|---|---|---:|---:|---:|---|
| StoneStatue01 | Carved robed-relief pillar | 7 | 92 → 100 | 1 | DIFFERENT |
| StoneStatue02 | Moai-like stone head | 7 | 107 → 107 | 1 | EQUIVALENT |
| StoneStatue03 | Winged angel on a memorial plinth | 4 | 248 → 248 | 1 | EQUIVALENT |
| SteelStatue01 | Capped carved memorial with ochre plaque | 4 | 66 → 98 | 1 | DIFFERENT |
| Tomb01 | Arched sarcophagus with offset lid | 4 | 44 → 76 | 2 | DIFFERENT |
| Tomb02 | Cross-topped carved grave | 4 | 40 → 72 | 1 | DIFFERENT |
| Waterspout01 | Accepted dragon fountain; shared basin material compatibility only | 1 | 639 → 639 | 11 | EQUIVALENT |

Six stone atlases sharpen the original reliefs, symbols and material roles. The shared relief-pillar and basin
retain warmer limestone; the Moai head, angel and cemetery monuments use muted cool granite. Painted edge
wear and readable carving complement the accepted Masonry01, Rocks01 and Tomb03 palette. Four models receive
shallow 0.4-unit stone panel recesses, keeping every original boundary. Head and angel sculptural geometry
remain intact. Six head-cap triangle UVs are repaired to remove the old narrow-strip stretch. The sarcophagus
paint was refined after render review to remove blocky grain while retaining its original abstract medallions.

## Exact changed game files

- `src/bin/Data/Object1/StoneStatue01.bmd`
- `src/bin/Data/Object1/stone_statue02.OZJ`
- `src/bin/Data/Object1/StoneStatue02.bmd`
- `src/bin/Data/Object1/stone_statue01.OZJ`
- `src/bin/Data/Object1/StoneStatue03.bmd`
- `src/bin/Data/Object1/angel_stone_statue.OZJ`
- `src/bin/Data/Object1/tombstone_big.OZJ`
- `src/bin/Data/Object1/SteelStatue01.bmd`
- `src/bin/Data/Object1/Tomb01.bmd`
- `src/bin/Data/Object1/grave_02.OZJ`
- `src/bin/Data/Object1/Tomb02.bmd`
- `src/bin/Data/Object1/grave_01.OZJ`

`dependency-ownership.json` verifies the complete six-texture/seven-model group. stone_statue02 serves the
relief pillar and fountain basin; tombstone_big serves angel base and memorial; other paintings are exclusive.
The fountain BMD, reagon_waterspout.OZJ, accepted ston01.OZJ and ston02.OZJ water remain byte-identical.

## Validation

- Seven official imports, packed source files with exact original references, official exports and actual-BMD reimports.
- All seven model SMDs and seven action SMDs validate; full comparisons retained. Four remodeled models DIFFERENT.
- Head, angel and unchanged fountain compare EQUIVALENT for geometry/rig. Head cap UVs deliberately differ;
  converter equivalence is not claimed as whole-asset identity. All seven skeleton/action comparisons EQUIVALENT.
- Bone names/order/parents and all keys are checked. Six production models retain one key; fountain retains 21.
- Every authored and reimported vertex checked bidirectionally for position and intended named bone, including new
  inset vertices and the two Tomb01 roots. No root-ownership deduplication defect; source references exact and packed.
- Tomb01 original slanted root has measured 0.002639771-unit Blender reconstruction drift / 0.002633 SMD drift;
  explicit 0.005 tolerance, matching printed bounds, exact named bindings/UVs, local matrix checks pass. Other
  assets use 0.001 positional tolerance; all original source anchors unchanged. No unresolved validator rejection.
- No new collapsed UVs, finite geometry, one original bone per vertex, original material order and names retained.
- stone_statue02 is 256×512 RGB; the five other paintings are 512×512 RGB. Official wrapping/loader checks pass.
- Only twelve owned game files installed into this isolated source checkout; 312 other World1/Object1 files hash-protected.
- No runtime/client, shared-document, engine/CMake, UI, terrain, placement, collision or other-map changes.

## Evidence and reproduction

Review `review/batch-review.jpg`, all seven per-model `review/comparison.png` sheets, fountain
`review/action-comparison.jpg`, and `paintings/texture-comparison.jpg`. These use actual original/exported BMDs
with matching cameras and lighting. Flat grass and the 190-unit figure are offline scale proxies. Fountain
water scrolling, particles, additive blending and actual client lighting are not emulated or verified.

Set MU_BMDCONV to the supplied converter. Run commands explicitly from this worktree using bundled Python and Blender.
Initial archive/import: prepare.py. Artwork: builtin imagegen prompts/raw references/masters under paintings; the
grave_02-refine-prompt.txt records its second pass. Package: package_textures.py. Build/export: Blender build_source.py,
Python export_validate.py. Check validate_anchors.py, validate_final.py, Blender audit_bindings.py, audit_materials.py.
Render: Blender render_exports.py then render_actions.py; assemble_review.py and assemble_actions.py. Install only
owned isolated-source paths with install_source.py. write_notes.py reproduces measured handoffs. No shared runtime writes.
Reviewed SignsBanners01, StoneWalls01 and StaticBatch01 helpers are reused read-only.

**Pending:** coordinator review/integration and serial in-client observation. No client asset is marked verified.
