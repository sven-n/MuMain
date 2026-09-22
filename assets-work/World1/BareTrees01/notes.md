# BareTrees01 — shared charcoal bark and dead trees

Owner: ASTRA groundcover/tree worker. Branch `codex/lorencia-bare-trees`.
Offline production complete; client verification pending. Four new production assets cover
93 World1 placements. The accepted conifer is a fifth shared-texture compatibility consumer,
covering 165 previously completed placements; it is not counted as a newly completed model.

| Model | Actual identity | Placements | Triangles retained | Bones / action keys |
|---|---|---:|---:|---:|
| Tree03 | Broad upright dead tree | 22 | 273 | 1 / 1 |
| Tree04 | Crooked dead tree | 15 | 273 | 1 / 1 |
| Tree05 | Narrow forked snag | 20 | 85 | 1 / 1 |
| Tree07 | Root-flared seating stump | 36 | 90 | 1 / 1 |
| Tree11 | Accepted conifer, compatibility only | 165 | 169 | 10 / 31 |

The economical branch shapes and low stump platform were retained intentionally. This is a
surface-art rebuild, with no high-poly/remodel claim. Sharper layered bark and longitudinal
fissures retain the dark gray material role and fit the accepted muted foliage/terrain.
The new opaque bark is 512x256, preserving the original 2:1 aspect and UV direction. Imagegen
produced the retained raw/editable PNG; the exact prompt and technical packaging script remain.

Exact changed game files, all under `src/bin/Data/Object1/`:
`Tree03.bmd`, `Tree04.bmd`, `Tree05.bmd`, `Tree07.bmd`, `tree_03.OZJ`.

The complete tree_03.OZJ dependency group is those four models plus Tree11. Tree07 also uses
tree_04.OZJ, a frozen cut-face material shared with BridgeStone01. Tree11 uses accepted
tree_06.OZT foliage. Both containers and the installed Tree11.bmd remain byte-identical;
frozen-files.json and protected-check.json prove that. Tree11 originals come from accepted
revision ac16ffeb; the four bare originals come from ac0f6dd8. Per-model original-provenance.json
proof is consolidated at the batch root and independently checked against starting HEAD.
Original model/texture containers are untouched archives, with info, official imported packed
source and preserved source geometry in REF_ORIGINAL. Tree11 additionally archives its current
accepted texture dependencies within Tree11/original/.

All five official roundtrips pass full BMD comparison EQUIVALENT. Bind bounds match to printed
converter precision; UVs, material ordering, original rigid bones and every original corner
are checked after export. Rig/action-only comparisons also pass. Translation discrepancies
are at most 0.000008 units for the four bare models and 0.000092 units for the conifer's
separate roundtrip. Every local rotation matrix is identical in these samples, including all
310 conifer action bone samples. All names, parents, action/key ordering and lock=0 remain.
The installed Tree11 BMD is not replaced by that roundtrip.

Per-model review/comparison.jpg sheets include matching before/after cameras, completed
terrain proxies, a 190-unit scale figure and 240px readability previews. Wireframes are saved.
Tree11 action-comparison.jpg shows frames 0, 15 and 30 using the unchanged accepted game BMD
and final bark. Terrain staging is illustrative at the origin, not client evidence or an
actual placement capture. Every real placement is recorded in the model's placements.json.
Tree07's CreateOperate seating hook and exact top, root footprint and cut-face mesh are retained.

Reproduction, from this checkout: bundled Python runs prepare.py, import_originals.py and
package_textures.py; Blender runs build_sources.py with `--python-exit-code 1`; Python runs
export_validate.py; Blender runs audit_sources.py and render_review.py; Python runs finalize.py.
The prepare step refuses to overwrite a different archived original. All scripts resolve and
execute against their repository worktree. Source installation is limited to the five files
above; the protected files are checked before completion. No shared runtime was touched.

Pending client checks: lighting, actual placed contacts/occlusion, sitting on Tree07 and
conifer sway under engine timing. No model has been observed in client for this batch.
Independent acceptance of coordinator Mushrooms01 is retained in independent-review/.
