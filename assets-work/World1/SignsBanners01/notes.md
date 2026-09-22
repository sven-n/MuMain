# SignsBanners01 offline handoff

Owner: ASTRA reviewer/production worker; branch `codex/lorencia-signs-banners`.
Worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-signs-banners`.
Four newly completed models cover 26 World1 placements. StoneWall06 is a read-only compatibility model
covering two additional placements of shared heraldry. Coordinator integration and client verification pending.

| Model | Placements | Triangles before → after | Bones | Original action | Full comparison |
|---|---:|---:|---:|---|---|
| Sign01 | 3 | 30 → 46 | 4 | action 0 keys=21 lock=0 | DIFFERENT |
| Sign02 | 5 | 30 → 46 | 1 | action 0 keys=1 lock=0 | DIFFERENT |
| Curtain01 | 6 | 68 → 68 | 7 | action 0 keys=26 lock=0 | EQUIVALENT |
| StoneWall04 | 12 | 36 → 36 | 8 | action 0 keys=25 lock=0 | EQUIVALENT |
| StoneWall06 | 2 | 88 → 88 | 9 | action 0 keys=25 lock=0 | EQUIVALENT |

The two notice boards keep their ochre plaque, original abstract pseudo-lettering, fasteners and oak utility
atlas. A shallow 0.25-unit panel recess adds a clear edge without moving any original boundary. Muted forged
iron supports and tarnished pole hardware match the accepted ironwork/tavern palette. Blue laurel and yellow
mounted-rider symbols retain their identities, layout and cloth topology. No new insignia or readable text.

## Changed game files

- `src/bin/Data/Object1/Sign01.bmd`
- `src/bin/Data/Object1/notice.OZJ`
- `src/bin/Data/Object1/signboard.OZT`
- `src/bin/Data/Object1/doorknob.OZT`
- `src/bin/Data/Object1/Sign02.bmd`
- `src/bin/Data/Object1/Curtain01.bmd`
- `src/bin/Data/Object1/badge_01.OZJ`
- `src/bin/Data/Object1/badge_02.OZT`
- `src/bin/Data/Object1/StoneWall04.bmd`
- `src/bin/Data/Object1/badge_03.OZT`

The complete dependency map assigns notice to Sign01/02, signboard and doorknob to Sign01, badge_01 to
Curtain01/StoneWall04/StoneWall06, badge_02 to Curtain01, and badge_03 to StoneWall04/StoneWall06.
`dependency-ownership.json` verifies these are all consumers. StoneWall06.bmd and tile_01.OZJ remain
byte-identical to the accepted baseline, both in Data and compatibility exports.

## Validation and limits

- Official importer/exporter used for all five sources; original imports and packed editable sources retained.
- All five model SMDs and five action SMDs validate; actual BMD info and full comparison logs retained.
- Sign01/02 full comparison is DIFFERENT for explicit geometry remodeling. Curtain01, StoneWall04 and the
  accepted StoneWall06 compatibility export compare EQUIVALENT. All five rig/action comparisons EQUIVALENT.
- Every local transform for all original keys passes translation <0.0001 and matrix component <0.00001.
- Every original vertex/bone and all unchanged triangle UVs retained. Maximum original-corner export drift
  is 0.000130 units in the animated StoneWall04 hierarchy; source anchors are exactly unchanged.
- Material order, one rigid original bone per vertex, finite geometry/UVs, alpha convention and dimensions pass.
- notice512²; signboard512×256; doorknob128²; badge_01 64×512; badge_02 256×512; badge_03 512².
- Four final alpha textures are exact bilinear resamples of the original scalar masks. Filtering RGB is padded
  from nearest alpha>=128 texels; masks themselves are never thresholded or repainted. Editable OpenRaster
  files retain generated diffuse, mask and composited final layers; full generated PNG masters and prompts remain.
- Legacy signboard has a detached ghost ring at alpha<=10/255. Raw Cycles blending initially exaggerated this
  low-alpha debris after painting. Both review sides now approximate the documented RGBA >0.25 alpha test
  with scalar blending above threshold. Game alpha is unchanged. See engine-contract-audit.txt and alpha-audit.json.
- Source installation changed only ten owned game paths; 314 other World1/Object1 files were hash-protected.
- No runtime/client, engine/CMake, UI, terrain, collision, placement or shared-document edits.

## Evidence and reproduction

Start with `review/batch-review.jpg`, all five `review/comparison.png` sheets, four `review/action-comparison.jpg`
sheets and `paintings/alpha-review.jpg`. Cameras and lighting match before/after; previews use actual exported
BMD imports. Flat grass and 190-unit figure are scale proxies, not actual World1 terrain or client observations.

Set `MU_BMDCONV` to the supplied absolute bmdconv binary. Use the supplied Blender and Pillow Python.
`prepare.py` is initial-only; `paintings/*-prompt.txt` records builtin imagegen edits of the inspected originals.
Run package_textures.py; Blender build_source.py and export_validate.py; validate_anchors.py, validate_final.py,
audit_alpha.py; Blender render_exports.py followed sequentially by render_actions.py; assemble_review.py,
assemble_actions.py; install_source.py; write_notes.py. All commands must use this worktree explicitly.
Reviewed scripts from StoneWalls01 and StaticBatch01 are reused read-only; own scripts and outputs stay here.

**Pending:** coordinator visual acceptance/integration and actual serial client review. No known blocking defect.
