# Tree09 — Five tapered tall-grass tufts — 2026-09-22

Status: exported and validated offline. Client verification pending.
Owner: ASTRA Scrub01 artist, `codex/lorencia-scrub`.

- Game file: `src/bin/Data/Object1/Tree09.bmd`.
- World1 placements: **42**; full original transforms in `original/placements.json`.
- Mesh-slot order / complete diffuse dependencies: `tree_07.tga` → `src/bin/Data/Object1/tree_07.OZT`.
- Triangles: **240 → 240**. Retained authored forms, UVs and skinning; this is a material pass.
- Texture dimensions: original **32×32**, replacement **512×512 RGBA**. TGA is uncompressed 32-bit bottom-left; original graded alpha field enlarged with bilinear filtering.
- Bone count: **5**, names/order `['Cylinder04', 'Cylinder05', 'Cylinder06', 'Cylinder07', 'Cylinder08']`. All parent relationships unchanged; complete proof in `validation/summary.json`.
- One unchanged action: index 0, one keyframe, `lock=0`.
- Original bind bounds: `[[-123.817307, -129.511993, 0.5619], [156.321899, 111.420807, 192.241302]]`.
- Exported bind bounds: `[[-123.817307, -129.512115, 0.5619], [156.321915, 111.420807, 192.241302]]`.
- Maximum bound component round-trip drift: **0.000122 units**; source geometry/UV/weights/transforms remain exactly identical.
- Maximum local pose translation drift: **0.000002 units**; rotation drift: **0.000000 radians**. These are float serialization differences through Blender/SMD.

`validation/compare.txt` is the complete full-model converter output: **EQUIVALENT** at the converter's default 0.05-unit tolerance. Original and final model and action SMDs pass the authoritative validator. Mesh count/order and texture tokens are preserved. No collapsed UV triangles, invalid weights or nonfinite values were introduced.

`source.blend` is packed with excluded `REF_ORIGINAL`; `original/source.blend`, original BMD/TGA/OZT, original converter info/SMDs and placements are retained. Saved-source audit verifies numeric reference geometry, UVs, skinning and transforms plus rig metadata and packed images. There is no new high-poly geometry because the inherited shell/card topology is appropriate for these frequently repeated props.

Shared editable artwork is in `../textures/`: raw imagegen paintings, unmasked diffuse, original alpha mask, final PNG/TGA/OZT and layered OpenRaster files. Prompts are in `../generation-prompts.json`. Preserve shared texture ownership across all listed consumers in `../dependency-inventory.json`.

`review/comparison.png` contains matching-camera before/after views, actual reimported BMD/OZT after image, wireframe and reduced-scale previews. Reverse view and camera settings are retained. `../alpha-review.png` shows light and dark fringe checks. All are **OFFLINE BLENDER**, never client evidence.

Client review still needs real loading, per-vertex light, alpha filtering/testing, culling and placement-scale checks at 1920×1080. No runtime files or terrain data were changed. Coordinator owns final integration, shared handoff and worklog.
