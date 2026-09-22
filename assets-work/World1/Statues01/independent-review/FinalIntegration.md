# Final Lorencia integration independent audit

2026-09-22. Reviewer: ASTRA independent reviewer, `codex/lorencia-monuments`.
Integration worktree inspected read-only:
`/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-rebuild`.

**ACCEPT for offline integration.** Final game revision:
`973ab58db8c5bd57237b063711c41f89223a824e` on `art/lorencia-rebuild`.
No missing production batch, deliverable gap, scope violation or visual integration blocker found.
Client verification remains pending; this is not acceptance of runtime behavior.

## Inventory and evidence coverage

The final board accounts for all 115 Object1 candidates: 106 accepted static assets and nine explicitly
excluded fauna, merchant creatures and engine markers. The latter retain the board's `blocked` status
with scope reasons; they are not unfinished authorized production. The 25 ledger batches cover 102
distinct models, with protected Beer01, Candle01, TreasureChest01 and Tomb03 supplying four earlier
completed models. Repeated models in compatibility groups are not counted twice.

All 106 accepted assets have retained original BMDs, packed source.blend with REF_ORIGINAL, validation
reports and matching-camera comparison/wireframe/reduced-scale evidence. Some early batches embed
reduced-scale images in comparison.png rather than storing a separately named file. Independent
read-only Blender inspection opened 142 production and compatibility sources across the initial audit
and final delta: every inspected source has REF_ORIGINAL and packed file-backed images. All 102
non-pilot originals match ac0f6dd8 in at least one initial production archive; accepted compatibility
archives additionally preserve the accepted versions they consumed. Earlier pilot originals remain.

Waterspout01 is intentionally a shared-material compatibility pass: its original BMD remains byte-exact,
with packed sources and comparison/action evidence in Rocks01 and Statues01. Grass02, Tree12 and
Tree13 retain original BMD bytes after NormalBindings01; their accepted paints, authored sources and
prior official roundtrip evidence remain. Those cases are not counted as new geometry rebuilds, and
legacy Tree12/Tree13 normal sharing is preserved rather than claimed repaired. All four exceptions
were independently rechecked against ac0f6dd8 after final integration.

## Combined game files and boundaries

All 183 active ledger game exports match installed integration files exactly: 98 BMDs and 85 texture
containers. There are no duplicate active path owners or unledgered game changes. The complete branch
diff is confined to assets-work/World1, authorized Object1 files and coordinator HANDOFF/WORKLOG
documents. No engine, CMake, UI, other-map or placement mutation appears. No runtime installation or
client operation was performed for this audit.

All 52 tracked World1 files are byte-identical to reviewed baseline ac0f6dd8, including completed terrain
paintings, TerrainLight, alpha strips, height, walk data and placements. All 13 protected/excluded model
BMDs and their 25 distinct BMD/texture dependency paths match that baseline. The three reviewed pilot
commits 2e2ed427, b232470c and 8d22a912 are integration ancestors. Beer01 remains the 784-triangle
still life with its accepted plate2 atlas. The final combined validator reports 141 protected game files.

Fresh independent conversion of the initially accepted 102 installed BMDs plus the final four-model
delta yields 212 valid reference/action SMDs. Their 98 distinct consumed texture containers resolve
using the complete filename suffix and pass fresh mu_texture loader checks; the replaced final shared
wood container was checked again. Same-stem OZJ/OZT families remain distinct. Per-batch comparisons
properly distinguish intentional geometry DIFFERENT from preserved skeleton/action EQUIVALENT.

Final coordination reports inspected: combined-validation.json PASS at 973ab58d with zero missing
textures; normal-binding-audit.json PASS across 101 models; authored-vertex-audit.json PASS across
72 remodeled models with bidirectional matching constrained to identical bone names. The latter
uses its stated 0.01-unit tolerance, not an assertion of bit-exact geometry. Earlier documented legacy
float drift and retained original normal sharing remain explicitly qualified in their batch evidence.

## Final Cannons01 delta

Freshly extracted original and installed/exported reference/action SMDs for Cannon01, Cannon02,
Cannon03 and Hanging01 all validate. All four full geometry comparisons correctly report DIFFERENT.
Final counts are 707, 681, 367 and 338 triangles respectively; Hanging01 retains its 25-key animation,
and each cannon retains one key. Bind bounds, original anchors, bone names/order/parents and action
contracts pass the retained batch checks. Hanging01's measured local translation drift is below
0.000045 units; no byte-exact action-payload claim is made.

Independent raw BMD inspection finds zero cross-node normal/vertex corner bindings on all four final
models, including all Hanging01 keys. Their final bidirectional authored-point errors are below
0.000031 units. All eight compatibility BMDs are byte-identical to the accepted snapshots, and all
12 source.blends contain packed artwork and REF_ORIGINAL. The sole changed shared atlas is
horse_drawn_01.OZJ, a 1024-square fixed-layout painting. Its original material roles and complete
12-model consumer set remain intact.

Viewed the four matching-camera comparison sheets and complete consumer overview, followed by the
current combined gallery. Round cannon bores, timber stands, ammunition boxes and gallows/noose
retain identity and readable silhouettes. Dark walnut atlas padding avoids the first draft's pale edge
halos. Carts, masonry mortar pieces, annex and well remain coherent with the revised shared wood.

## Combined visual review and limits

Viewed refreshed final-sheet-01.jpg through final-sheet-06.jpg across all 106 final inventory previews.
All 106 per-model provenance records were independently rehashed against actual current BMDs and
texture containers after the final gallery refresh. The integration game files still match 973ab58d.
The gallery therefore uses the actual combined exports, including current shared materials, rather
than stale per-batch copies.

No unexpected shared-material replacement, missing texture, silhouette loss or opaque foliage-card
regression was found. Muted timber, worn iron, masonry and vegetation remain coherent while preserving
material and heraldic distinctions. Combined architecture, pottery/casks, stump/end grain, statues,
fountain and hay/cart consumers retain their expected identities. Modular connection/opening checks
and assembly views remain available in the accepted architecture batch evidence.

The neutral diffuse gallery visibly retains black effect-card faces on some additive effects, including
HouseWall02, Bonfire01 and Candle01. This is a documented preview limitation, not a new asset regression;
per-batch matched effect studies contain the relevant additive approximations. Offline images do not
prove engine blending, scrolling, particles, lighting, collision or play-scale performance. No client
captures or client verification were performed. Serial in-client observation is the remaining review
step once the client's pre-existing stability problem is resolved.

Review evidence is in assets-work/World1/coordination/final-inspection/*/provenance.json and the six
final-sheet-*.jpg files, with full technical reports beside them and detailed evidence in each batch.
Only this independent review document was modified by the reviewer; shared handoff and integration
ownership remain with the coordinator.
