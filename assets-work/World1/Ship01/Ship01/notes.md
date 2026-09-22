# Ship01 — carved river boat

2026-09-22. ASTRA coordinator. Branch `codex/lorencia-ship`; worktree `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-ship`.

Three actual World1 placements. Carved dragon-prow wooden boat with raised stern cabin, furled linen sail and fine rigging. Seven painted surfaces refresh weathered oak, faded green hull decoration, tarnished bronze carving and stained sailcloth. Geometry is deliberately retained: 754 triangles, ten material slots, ten original bones, one 30-key unlocked action. Full geometry comparison and all ten individual material-component comparisons are EQUIVALENT. This is a material art pass with preserved geometry, not a claim of remodeling.

## Geometry and validation

Original and replacement engine bind bounds: `[-581.94,-1166.51,-100.15]` to `[553.46,669.28,937.93]`. All bone names/order/parents and every original local bind/action sample are preserved by final packaging. The official exporter first produced a supported intermediate; `export_validate.py` then retains its triangle stream while restoring the original reference SMD bind header and all original manifest action SMDs through bmdconv. Intermediate files and complete commands/logs remain under validation/official-roundtrip and validation/preserve-bind-actions.txt. This avoids a 0.000129-unit reconstructed bind translation discrepancy in the deeply parented hull.

Final model/action SMD validation passes. Isolated skeleton/actions EQUIVALENT; strict sample proof in validation. Original bind bounds, original material order and every component geometry/UV are retained. Actual final BMD reimport has maximum original anchor drift of 0.00048828125 units due to Blender floating-point reconstruction; engine comparison is EQUIVALENT. Source anchors are unchanged. All vertices rigidly use one original bone. There are 19 original collapsed UV triangles and zero geometric degeneracies before and after; no new defects introduced. Repeating UVs span -7.496 to23.657 by original design.

Packed source.blend includes REF_ORIGINAL and original rig. All original BMD bytes, complete original textures, original info, official imported source and actions are retained. Matching original/final reimport cameras, reverse view, wireframe, 240px readability previews and action keys0/14/29 are in review. The 190-unit bar is a scale proxy. These are offline Blender renders, not client evidence.

## Materials

Five opaque surfaces are512x512 (TileGround03, ship01, ship03, ship04, ship07); linen ship05.jpg and openwork ship05.tga are512x256. Original64x32 ship05 alpha is preserved exactly by bilinear8x enlargement. Tiny ship06.OZJ16x16, ship06.OZT4x16 and ship07.OZT8x32 remain byte-identical, including graded rope alpha45..255. All ten container checks pass. Raw imagegen masters, original image references, generation-prompts.json, editable final PNG color/alpha layers and reproducible wrapping script remain under textures.

`Object1/TileGround03.OZJ` is an exclusive boat decking texture; completed World1 terrain textures are unchanged. Every texture in this batch is exclusive to Ship01. Complete dependencies:

- `TileGround03.jpg`: `src/bin/Data/Object1/TileGround03.OZJ`
- `ship01.jpg`: `src/bin/Data/Object1/ship01.OZJ`
- `ship03.jpg`: `src/bin/Data/Object1/ship03.OZJ`
- `ship04.jpg`: `src/bin/Data/Object1/ship04.OZJ`
- `ship07.jpg`: `src/bin/Data/Object1/ship07.OZJ`
- `ship05.tga`: `src/bin/Data/Object1/ship05.OZT`
- `ship07.tga`: `src/bin/Data/Object1/ship07.OZT`
- `ship06.tga`: `src/bin/Data/Object1/ship06.OZT`
- `ship06.jpg`: `src/bin/Data/Object1/ship06.OZJ`
- `ship05.jpg`: `src/bin/Data/Object1/ship05.OZJ`

## Changed game files

- `src/bin/Data/Object1/Ship01.bmd`
- `src/bin/Data/Object1/TileGround03.OZJ`
- `src/bin/Data/Object1/ship01.OZJ`
- `src/bin/Data/Object1/ship03.OZJ`
- `src/bin/Data/Object1/ship04.OZJ`
- `src/bin/Data/Object1/ship07.OZJ`
- `src/bin/Data/Object1/ship05.OZJ`
- `src/bin/Data/Object1/ship05.OZT`

Only these eight files were installed into this isolated source worktree. All other316 Object1/World1 files are hash-protected. No shared runtime, client, engine/CMake, UI, other map, terrain/lighting/placement or collision changes.

Reproduce in this worktree with bundled Pillow Python: prepare.py once, package_textures.py; Blender build.py; Python export_validate.py; Blender audit_sources.py and render_review.py; Python assemble_review.py and install.py. Raw image generation is described by generation-prompts.json. All scripts derive paths from this deliverable and use the existing official tools. Engine references are recorded in engine-reference.txt.

Pending: independent review, coordinator integration and actual serial1920x1080 client observation once stable. No client verification occurred.
