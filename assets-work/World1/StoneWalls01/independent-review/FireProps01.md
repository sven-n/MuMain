# FireProps01 independent integration review

2026-09-22. Reviewer: ASTRA independent review / stone-walls worker.
Recommendation: **ACCEPT for offline integration**. No blocking finding.
Reviewed commit: `967dc888cf281f219c5738935d062fde97593083` in the clean
`MuMain-lorencia-fire-props` worktree, which remained read-only during review.

Scope: FireLight01, FireLight02, Bonfire01 and DoungeonGate01; 54 placements.
Final triangles: 80, 34, 110 and 333. Only the standing brazier's support was
remodeled; the other three full comparisons correctly report EQUIVALENT.
FireLight01 correctly reports DIFFERENT. Skeleton/action comparisons pass for
all four, with original bone names/order/parents and one-key action metadata.

Read the handoff, all four summaries, local-motion and source/export audits,
plus texture packaging and protected-component validation implementation.
Independently reran eight final model/action SMD validations: all passed.
Independently reran twelve skeleton/protected-material BMD comparisons: all
EQUIVALENT. Also matched every protected triangle corner by its original bone,
exact UV pair and position within 0.005 units; no protected UV/binding loss.
This includes basket/coal, wall-torch geometry, the 38-triangle bonfire effect
shell and the cobra entrance. Mesh material order and rigid skinning are retained.

The original gateway collapsed UV triangle remains the sole pre-existing case;
no new collapsed geometry or UV triangles are introduced. Printed bind bounds
match for all models. Reported maximum official Blender reimport anchor drift is
0.004524231 units on FireLight02 and 0.001060993 on DoungeonGate01. These small
legacy-format reconstruction differences are disclosed and pass engine geometry
comparison; they are not a claim of byte-identical geometry. Local translations
remain within 0.000031 units, with rotation matrices within 0.000000470. Matrix
comparison appropriately handles the bonfire's equivalent Euler representation.

Independently verified all nine installed source game-file hashes and all 315
protected World1/Object1 file hashes. The coordinator dependency map confirms
exclusive ownership of light.tga/light2.jpg (FireLight01), fire_light_01.tga
(FireLight02), copra_gate.jpg (FireLight02 and DoungeonGate01) and fire_01.jpg
(Bonfire01). light3.OZJ, fire_02.OZJ and shared tile_02.OZJ remain unchanged.
Both final 256-square alpha fields independently match the original scalar masks
resized bilinearly, pixel for pixel; the paintings alter color only. The color
extends through transparent areas, avoiding a newly introduced black fringe.

Visually inspected all four `review/comparison.jpg` sheets and
`textures/alpha-review.jpg`. Dark iron, charred timber and weathered sculpted stone
remain coherent with the Lorencia direction. Basket holes and brackets remain
open; the cobra silhouette, entrance clearance and fire plinths stay recognizable.
The new standing support is restrained and its endpoint footprint stays intact.
The bonfire correctly uses the same documented additive-material approximation
in both views; the preserved effect shell remains visible as the legacy shape.
Reduced previews and visible wireframes support the comparison. Runtime particles
are absent and the sheets explicitly remain offline evidence.

No runtime installation or client observation was performed. Serial in-client
review of particle attachment, additive flame behavior and final alpha readability
remains pending. This review changes only this document in the reviewer's own
stone-walls worktree; no production asset or shared handoff was modified.
