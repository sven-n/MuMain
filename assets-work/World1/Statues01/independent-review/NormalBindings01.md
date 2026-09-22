# NormalBindings01 independent review

2026-09-22. Reviewer: ASTRA independent reviewer, `codex/lorencia-monuments`.
Reviewed coordinator commit `f6998b30c1b2a3f74ddbeee28a479d9c85dd6787` read-only in
`/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-normal-bindings`.

**ACCEPT for offline integration.** Restoring the three original BMD containers removes the introduced
Grass02 normal-binding regression and preserves the original Tree12/Tree13 behavior. Accepted paintings
remain unchanged. This is a material-only compatibility correction, not a geometry rebuild or an assertion
that the original Tree12/Tree13 normal sharing has been repaired. Client verification remains pending.

## Independent checks

- Final changed game paths are exactly `Object1/Grass02.bmd`, `Object1/Tree12.bmd` and
  `Object1/Tree13.bmd`. All 321 other tracked World1/Object1 files, including every texture container,
  match the reviewed commit's parent byte for byte.
- Each installed BMD and its delivered original match the original `ac0f6dd8` revision byte for byte.
  This verifies the normal records and bone indices that expanded SMD comparison alone cannot prove.
- Fresh original/final converter extraction and twelve validations pass: reference and action SMDs for
  both stages of all three models. Fresh complete comparisons are EQUIVALENT with zero corner and bone
  deltas and no unmatched triangles. Corresponding complete reference and action SMD files are byte-identical.
- Grass02 retains 120 triangles, one mesh, five bones and one action with one key. Tree12 retains 393
  triangles, two meshes, nine bones and one 31-key action. Tree13 retains 393 triangles, two meshes,
  ten bones and one 31-key action. Bone names, ordering, parents, materials, UVs and actions therefore
  remain the exact original payload, rather than relying on a skeleton-only equivalence claim.
- The retained authored and original source.blend files match their accepted Groundcover01/Flowering01
  sources byte for byte. Independent read-only Blender inspection confirms hidden REF_ORIGINAL objects
  carry the reference marker and all file-backed images are packed (two for Grass02, four for each tree).
  Delivered final texture containers match the unchanged game containers.
- Reviewed prepare.py, audit_sources.py, notes.md, source audits, full comparison logs, and all three
  actual-reimport comparison sheets. Original and accepted-painted views use matching cameras;
  silhouettes, foliage openings, wireframes and reduced-scale readability remain coherent. No opaque
  card-plane regression is visible. Tree12's muted rose and Tree13's autumn canopy retain their identities;
  Grass02 remains subdued olive groundcover.

## Original/final BMD SHA-256

| Model | Original and installed final SHA-256 |
|---|---|
| Grass02 | `1e3f338fbf6d5956bbcc0e194f916f87389ea97a8c0742528e1306a3187de4ea` |
| Tree12 | `f2c6dbc600ebb76c5af82198699ffe83bfb532ba5db0a6f805efb4425545f502` |
| Tree13 | `54fa5331f342977f13dba3e586db8e320c15d4476f30d8ba19369ad3372cf4bf` |

## Limits and evidence

Previous official import/export roundtrip evidence remains available in Groundcover01/Grass02 and
Flowering01/Tree12, Tree13; superseded roundtrip containers are archived within NormalBindings01.
The original Tree12/Tree13 normal sharing is intentionally retained, not declared fixed. Blender's
SMD-based preview cannot establish runtime normal-node behavior; exact original container identity
provides that preservation proof. No client session or runtime installation was performed for this review.

Evidence is under `assets-work/World1/NormalBindings01/` in the reviewed worktree: notes.md, per-model
validation files, packed source files and per-model review/comparison.jpg. Independent converter extracts
were written to a temporary directory inside this reviewer's deliverable directory and removed after checks.
No blocker found. Only this review document is committed by the reviewer.
