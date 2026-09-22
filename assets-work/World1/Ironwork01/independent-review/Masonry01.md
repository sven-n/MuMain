# Masonry01 independent integration review

Date: 2026-09-22. Reviewer: ASTRA independent review / ironwork worker.
Recommendation: **ACCEPT for offline integration**. Client verification remains pending.
Reviewed masonry commit: `5180fd6d88386527ef187f71a75342939dcce249` on
`codex/lorencia-masonry`; its worktree was clean and remained read-only.

Scope: HouseEtc01, HouseEtc02 and StoneMuWall01–04, representing 137 World1
placements. Final triangle counts are respectively 50, 250, 398, 76, 84 and 317.
The source-installation ledger contains exactly six BMDs and four OZJ containers.
Independent SHA-256 checks matched all ten actual installed source files to that
ledger. No runtime installation or client observation was performed for this review.

The coordinator dependency map independently confirms that every consumer of the
four changed textures belongs to this batch: c_wall04 is used by all six models;
c_wall05 by StoneMuWall01/02; c_wall06 by HouseEtc01 and StoneMuWall01–04;
tile_ston01 by HouseEtc02. Eight frozen texture containers were independently
hashed against the anchor evidence and matched. Stone, carved monster/dragon
relief, timber, portcullis and siege details retain their distinct material roles.
All four new paintings are opaque RGB at 512 × 512; no alpha convention changes.

Read all six summary, modular-anchor, final-contract and local-motion reports,
plus the actual panel-construction and anchor-check scripts. Every original
vertex/bone anchor survives export; maximum reported component drift is 0.000001
units. Original panel boundary edges are retained; added geometry recesses only
panel interiors by 0.65 units. All triangles outside the explicit panel selection
retain positions, bone bindings and UVs. This protects gate inward jamb planes,
portcullis, hut doorway, carved relief silhouettes and frozen siege geometry.
Printed bind bounds and UV ranges agree before/after. The maximum retained-normal
direction difference is 0.081719 degrees, explicitly reported as import/export
normalization rather than exact normal equivalence; this is not a blocker.

Independently reran bmdconv validation on all six final model SMDs and all six
action SMDs: 12 passed. Reran all six skeleton/action BMD comparisons: EQUIVALENT.
Evidence records identical node names, indices and parents, one original action
with one key and unchanged lock metadata; local translation and wrapped Euler
component differences are zero for every bind/action sample. Full model compares
correctly report DIFFERENT for the intentional geometry changes. Mesh material
ordering, rigid binding, finite geometry and original source archive checks pass.

Visually inspected `review/batch-review.jpg`,
`review/south-gate-comparison.jpg` and `review/siege-wall-comparison.jpg` using the
image viewer. Weathered tan stone and dark relief painting form a coherent group;
the carved faces remain recognizable. Matched assembly images show unchanged
openings, wall joins and fixture silhouettes, with readable dark gate and timber
details. No visible new seam, filled opening or material-role regression found.
These are offline reconstruction renders, not evidence from the running client.

Evidence paths above are relative to `assets-work/World1/Masonry01/` in the
masonry worktree. The review changes only this document in the reviewer's own
ironwork worktree. No blocking finding; no change request.
