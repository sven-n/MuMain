# Independent Ship01 review — ACCEPT

2026-09-22. Reviewer: ASTRA reviewer/SignsBanners01 worker. Reviewed coordinator commit
`e687807a88ff23a95add92707be6fb00a0d7ac37` read-only in `MuMain-lorencia-ship`.
All review commands ran explicitly from the separate `MuMain-lorencia-signs-banners` checkout.
No ship files, shared documents or runtime files were changed by this review.

**Accept for offline integration. No blocking defect found. Client observation remains pending.**

Ship01 is the carved dragon-prow river boat with raised stern, furled linen sail and thin rigging;
three World1 placements. This is a material pass retaining the recognizable 754-triangle geometry.
Viewed `Ship01/Ship01/review/comparison.jpg` and `action-reverse-comparison.jpg` using the image tool:
matched forward/reverse views, wireframe, reduced previews and action keys 0/14/29. Dark oak and
restrained green hull decoration remain distinct; painted linen keeps the pale mast/sail landmark.
Thin ropes and open stern/deck spaces remain readable. The quieter, weathered palette fits the
accepted Lorencia wood/iron assets. No placement or client-rendering claim is inferred from Blender.

Independent technical checks:

- Fresh bmdconv extraction and validation of both original/final model SMDs and their 30-key action
  SMDs: four validations OK. Fresh full BMD comparison: EQUIVALENT, 754/754 triangles, ten meshes,
  ten bones, one action, zero unmatched triangles, maximum reported corner distance 0.0005 units.
- Freshly extracted node/bind headers and complete 30-key action SMD bytes match exactly. Bone names,
  order, parents, key counts and local transforms are therefore unchanged in the final packaged BMD.
- Independently compared all 2,262 triangle corners in original material order: bone IDs exact;
  maximum position-component difference 0.000489 units; UV coordinates exact. Official Blender
  reimport audit separately reports maximum world-coordinate anchor drift 0.00048828125 units.
  This is declared numerical roundtrip precision, not a structural remodel.
- Reviewed official export intermediate and supported SMD/manifest packaging: only the original bind
  header and original action records replace reconstructed transform records; the final triangle
  stream comes from the official exporter. The raw intermediate and packaging script are retained.
- Ten original material roles/order remain present. Full dependency-map check confirms every one
  of the ten textures is exclusive to Ship01. `Object1/TileGround03.OZJ` is boat decking and does
  not affect the similarly named protected World1 terrain texture.
- All eleven untouched original game files match pinned original baseline `ac0f6dd8`. All eight
  changed installed files match the validated exports. All 316 other World1/Object1 files match
  this production commit's parent `1fbe3db685db307a7b28a0346740cf384d49e408` byte-for-byte.
- `ship05.tga` final alpha is the exact bilinear enlargement of the original scalar field;
  `ship06.OZJ`, `ship06.OZT`, `ship07.OZT` remain byte-identical. Seven painted images and three
  frozen small rope/endcap containers resolve; wrapper checks pass. No new opaque rigging planes
  appear in the matched render evidence.
- Opened the packed editable source read-only in Blender: all 20 image datablocks packed;
  `REF_ORIGINAL` contains the preserved original, is render-hidden and excluded from Source Tools
  export, with the reference marker set. Source audit confirms rigid original bone binding.
- The original 19 collapsed UV triangles remain 19, with zero geometric degeneracies; repeating
  UV extent is preserved. This pre-existing limitation is accurately disclosed in the handoff.

Reviewed final BMD SHA256:
`26c2e0309638634f24cf2396f26464afe587236de2e264ea54567cbdbfdf6921`.

Evidence and production handoff: `assets-work/World1/Ship01/notes.md`, per-model `validation/`,
`review/comparison.jpg`, `review/action-reverse-comparison.jpg`, texture masters/prompts and alpha
records. Only serial in-client placement, shading and playback checks remain; none were performed.
