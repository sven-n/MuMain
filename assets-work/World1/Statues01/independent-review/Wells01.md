# Wells01 independent review

2026-09-22. Reviewer: ASTRA independent reviewer, `codex/lorencia-monuments`.
Reviewed coordinator commit `46daddf273bcf0b60e28bf7068d44291642819b2` read-only in
`/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-wells`.

**ACCEPT for offline integration.** The well structure, bucket, rope and casks preserve their original
geometry. Pottery receives constrained curved edges and smooth shading while retaining its original
corners and part bounds. The three paintings preserve the existing timber, fieldstone, cream/slate
pottery and oak/iron material roles. Client verification remains pending.

## Independent technical checks

Fresh converter extraction and twenty reference/action SMD validations pass across original and final
versions of all five models. Fresh complete comparisons reproduce the intended results:

| Model | Triangles before / after | Bones | Complete comparison | Maximum authored/final position component delta |
|---|---:|---:|---|---:|
| Well01 | 405 / 915 | 7 | DIFFERENT | 0.000072479248 |
| Well02 | 123 / 123 | 1 | EQUIVALENT | 0 |
| Well03 | 194 / 704 | 4 | DIFFERENT | 0.000099182129 |
| Well04 | 388 / 1408 | 8 | DIFFERENT | 0.000057220459 |
| Carriage03 compatibility | 538 / 538 | 4 | EQUIVALENT | 0 |

- Fresh reference SMD headers and complete one-key action SMD files are byte-identical for every pair.
  Bone names, order, parents, bind transforms and action records are unchanged. The supported original
  SMD header/action packaging follows the official Blender export; its raw intermediate evidence remains.
- Read-only Blender checks independently verify every authored vertex against the final reimport and
  every final vertex against the authored source, requiring the intended named bone in both directions.
  Every original corner is also present on its original bone. The table gives measured maximum errors,
  all below 0.0001 units. There is no cross-bone vertex deduplication regression.
- Packed sources contain hidden, correctly marked REF_ORIGINAL collections; all file-backed images
  are packed. Original/final printed bind bounds and material ordering match. Source geometry and
  actual reimports were inspected separately.
- Collapsed UV triangle counts remain exactly 24, 6, 18 and 36 for Well01 through Well04, and zero for
  the accepted cart. Zero geometric degeneracies. The build explicitly excludes original constant-color
  cap faces and their edges from the pottery subdivision. No additional collapsed UV triangles appear.
- Raw BMD normal records were independently parsed using CartHay01/raw_bindings.py and checked
  against fresh action rotations. Original and final models have zero unsafe normal-sharing corners.
  Final cross-node normal references remain on 66 Well01 corners, 12 Well04 corners and three accepted
  cart corners; all have exactly zero transformed-normal direction delta at the action key. Well02
  and Well03 have no final cross-node normal references. This check does not infer normal node indices
  from expanded SMD normals.

## Ownership and compatibility

The dependency map confirms the complete groups: well.jpg serves Well01/02; jar_01.jpg serves
Well01/03/04; tub.jpg serves Well01 and Carriage03. The unrelated horse_drawn_01 container remains frozen.
All four final dependency containers pass a fresh mu_texture loader check. The two square paintings
are 512 by 512 RGB; tub is 512 by 256 RGB. No alpha or filename convention changed.

Exactly seven game paths differ: Object1/Well01.bmd, Well02.bmd, Well03.bmd, Well04.bmd, well.OZJ,
jar_01.OZJ and tub.OZJ. All 317 other tracked World1/Object1 files independently match the reviewed
commit's parent byte for byte. Four archived source BMDs match ac0f6dd8; four final exports match their
installed source-checkout BMDs. The cart compatibility export is byte-identical to accepted 93ca193e,
SHA-256 `47d5c3c47defb5a1ff2a56ab8d4748c4ddb59e449e7a953f99857eef47bdb296`.
The newer cart was deliberately not installed into this batch's older frozen game path.

## Visual review and limits

Viewed all five matching-camera review/comparison.jpg sheets, including wireframes and reduced-scale
views, plus the pottery reverse view and textures/paint-comparison.jpg. The roof and masonry remain
recognizable; restrained timber wear and cask bands agree across the well and accepted cart. The cream
and slate vessels keep their openings, handles, individual silhouettes and relative arrangement.
Pottery curvature improves the silhouette without shifting original assembly contacts. No blocker found.

Review evidence resides under assets-work/World1/Wells01 in the reviewed worktree. Fresh independent
converter files used a temporary directory under this reviewer's deliverable folder and were removed
after checks. All source worktrees were inspected read-only. No engine, runtime installation or client
session was used. Offline renders are not client evidence; actual serial client observation remains pending.
Only this review document is committed by the reviewer.
