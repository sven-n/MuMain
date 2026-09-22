# Cannons01 independent review — accepted offline

Reviewer: ASTRA `/root/fences`, 2026-09-22. Producer: `/root/groundcover`.
Reviewed immutable production commit: **ab832f5de4cea0dcaee144dce3df77870e565815**
on `codex/lorencia-cannons`. No outstanding technical or visual rejection remains.
This review does not claim client verification. No client or shared runtime was operated.

## Identity and scope

Original geometry, original renders, placement/dependency records and engine references
confirm three inclined swivel guns on timber stands, with two/one/no ammunition boxes;
Hanging01 is a timber gallows with stairs, a platform and animated rope/noose.
These are static World1 scenery. Cannon01/02/03 have 1/2/3 placements and Hanging01 has 1.
`MapManager.cpp` loads the Cannon, Carriage and Hanging model families; `ZzzObject.cpp`
sets Carriage01 BlendMesh 2. That additive lantern mesh retains its material index and
unchanged BMD. Rendered lantern glow is explicitly an offline approximation.

Exactly these five game files change, all below `src/bin/Data/Object1/`:

- `Cannon01.bmd`
- `Cannon02.bmd`
- `Cannon03.bmd`
- `Hanging01.bmd`
- `horse_drawn_01.OZJ`

The pinned diff contains only those files and Cannons01 deliverables. All 319 other
tracked World1/Object1 game files match the protected baseline. No engine, terrain,
lighting, placement, collision, alpha-strip, UI or filename changes occur.

The exact `horse_drawn_01.jpg` dependency has twelve consumers. The eight compatibility
BMDs are byte-identical to their accepted originals: Carriage01–04, HouseEtc02,
StoneMuWall04, StoneWall03 and Well01. All 29 archived original BMDs/containers match
their declared source revisions and SHA-256 values. Well01 and well/jar/tub dependencies
correctly use **46daddf2**; the other snapshots use **0bb84512**.

## Independent technical results

Fresh Git blobs were extracted with bmdconv into reviewer-owned temporary directories.
Every original and exported mesh/action SMD validates. The audit recomputes skeleton
hierarchies, all local and world action matrices, posed bounds and raw BMD normal ownership;
it does not rely solely on producer PASS reports. Packed source geometry is also compared
bidirectionally against exported positions on the intended bone.

| Asset | Before → after triangles | Bones / keys | Full compare |
| --- | ---: | ---: | --- |
| Cannon01 | 181 → 707 | 1 / 1 | DIFFERENT |
| Cannon02 | 155 → 681 | 1 / 1 | DIFFERENT |
| Cannon03 | 75 → 367 | 1 / 1 | DIFFERENT |
| Hanging01 | 162 → 338 | 4 / 25 | DIFFERENT |
| Carriage01 | 472 → 472 | 14 / 21 | EQUIVALENT; identical bytes |
| Carriage02 | 450 → 450 | 2 / 1 | EQUIVALENT; identical bytes |
| Carriage03 | 538 → 538 | 4 / 1 | EQUIVALENT; identical bytes |
| Carriage04 | 882 → 882 | 5 / 1 | EQUIVALENT; identical bytes |
| HouseEtc02 | 250 → 250 | 3 / 1 | EQUIVALENT; identical bytes |
| StoneMuWall04 | 317 → 317 | 3 / 1 | EQUIVALENT; identical bytes |
| StoneWall03 | 333 → 333 | 4 / 1 | EQUIVALENT; identical bytes |
| Well01 | 915 → 915 | 7 / 1 | EQUIVALENT; identical bytes |

Full DIFFERENT is expected for the four remodeled assets; it is not presented as full
equivalence. Bone names, order, parents, action count, frame count, locks and material
slot order are unchanged. The separate producer skeleton/action comparisons are EQUIVALENT.
Cannon bind/posed bounds are unchanged. Hanging01 maximum local matrix delta is
0.0000457764, hierarchical matrix delta 0.0000305176 and all-key bounds delta 0.0000152588
engine units. All other assets have zero matrix/bounds delta in the independent audit.
All 25 gallows keys and all 21 cart keys were included, rather than only bind pose.

The new gallows exports have **zero cross-node vertex/normal pairs** after the custom-normal
correction. The readonly Carriage03 3:0, StoneMuWall04 2:1, StoneWall03 2:1 and Well01 6:0
pairs retain identical world rotations throughout their clips, so no normal-motion discrepancy
is introduced. Every exported corner and every authored source vertex matches the intended
bone and position within 0.005 units; all twelve source files contain packed images and
hidden, marked REF_ORIGINAL/REF_HIGH_POLY collections.

The painted atlas is 1024×1024, with unchanged filename/layout and no alpha conversion.
All seventeen dependency containers pass `mu_texture.py check`; all frozen containers
remain byte-identical. Each consumer export contains the same final atlas bytes. Production
UVs have positive area, and no new zero-area geometry, winding disagreement or collapsed
UVs are reported. Existing readonly constant-color collapsed cap UVs remain unchanged.
Retained timber contacts, steps and rope geometry keep their positions, bones and UVs.

## Visual review

Reviewed all twelve matching-camera comparisons and reduced-scale views, the four new
wireframes, gallows reverse view, gallows frames 0/12/24, cart frames 0/10/20 and actual
World1 cannon-row/gallows placement comparisons. The 138 delivered review images were
byte-checked against the pinned commit and the final artifact hash manifest.

The weathered oak/iron palette is consistent across the guns, gallows, carts and architectural
consumers. The standalone gun variants retain their identifiable ammunition-box layouts and
inclined barrel silhouettes. The rounded barrels and recessed bores improve their small-scale
readability. The gallows preserves its narrow noose silhouette, staircase and platform footprint.
The eight readonly consumers keep their accepted shapes and material roles.

Three findings were resolved before this acceptance: an animated gallows normal-node merge,
bright atlas gutters visible as off-white platform/cart edge blocks, and insufficient separation
between the cannon bore and rim. Fresh pinned images show dark timber edge coverage and
readable gray iron muzzle rims around dark bores. No retained UVs or unrelated atlas islands
were moved to make the muzzle correction.

All imagery is offline Blender evidence. Grass/figure staging and lantern emission are proxies;
actual client lighting, blending and world appearance remain pending coordinator client review.
Compatibility previews use the pinned package's declared dependency revisions; the coordinator
separately checks the combined integration set including concurrent Architecture03 paints.

## Evidence and reproduction

- [Independent immutable-binary audit](Cannons01-evidence/completed-commit-audit.json)
- [Audit execution log](Cannons01-evidence/completed-commit-audit.log)
- [Pinned images, scope and producer-report cross-check](Cannons01-evidence/pinned-artifact-audit.json)
- [Original provenance and exact consumer audit](Cannons01-evidence/original-audit.json)
- [Independent binary audit script](Cannons01-evidence/audit_completed_commit.py)
- [Review artifact audit script](Cannons01-evidence/audit_review_artifacts.py)
- Production visual index: `assets-work/World1/Cannons01/review/all-consumers.jpg`.
- Per-model comparison: `assets-work/World1/Cannons01/<model>/review/comparison.jpg`.

Run the binary audit with Blender from the review checkout, passing the full reviewed commit
after `--`; run the artifact audit with Python from that same checkout. Both write only this
review evidence directory. Production scripts, packed sources, raw/editable artwork, prompts,
full comparisons and per-asset notes remain in the pinned Cannons01 package.
