# Independent tavern integration review — 2026-09-22

**Recommendation: ACCEPT OFFLINE** the complete `desk_big` dependency group from
`fe69aa12` and `45c82ea0`. No blocking defect found. Client acceptance remains pending;
this review did not install runtime files or operate the client.

Reviewer: ASTRA independent review agent, branch `codex/lorencia-review`, worktree
`/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-review`.
Production owner: existing ASTRA tavern artist on `art/lorencia-tavern-props` in
`/Users/webproduktion3/Documents/claude-test-mumain/MuMain-tavern-props`.
That worktree was read only. Its current four game files still match `45c82ea0`.

## Accepted scope

| Actual asset | Model path under `src/bin/Data/Object1` | Triangles | World1 placement count | Original root |
|---|---|---:|---:|---|
| Four-legged rectangular tavern table | `Furniture03.bmd` | 42 → 680 | 5 | `Box09` |
| Half-round pedestal table | `Furniture04.bmd` | 38 → 324 | 3 | `Box01` |
| Repeated straight tavern counter | `Furniture05.bmd` | 34 → 412 | 3 | `Box13` |

The fourth and final game change is `src/bin/Data/Object1/desk_big.OZJ`, a
1024×1024 opaque RGB diffuse atlas replacing the original 128×128 texture.
Every model still has one mesh with the original material `desk_big.jpg`.

Independently queried all **115 Object1 BMD files**, rather than trusting the
production inventory: exactly these three models consume `desk_big.jpg`.
[texture-consumers.json](texture-consumers.json) records the complete map and
[object1-info.json](object1-info.json) retains the converter commands and results.
`MapManager.cpp:1091–1097` loads this Furniture series and its textures from Object1.
No explicit `desk_big` source reference was found elsewhere. This group must be
integrated atomically; none of its consumers can retain the old UV/material mapping.

Read-only decoding of the actual `World1/EncTerrain1.obj` independently reproduces
every stored position, rotation and scale in the artist's placement records. See
[placements.json](placements.json). The relevant type indices remain 142–144.
Furniture interaction cases found in engine source concern indices 145–146, outside
this group. No behavior-dependent mesh reordering was found, and the original
one-mesh structure is retained regardless.

## Independently reproduced technical checks

[results.json](results.json) and the per-asset `commands.json` files contain the
fresh results, stdout, stderr, command arguments and return codes.

- Original backups match the untouched BMDs in the reviewed World1 pilot checkout.
- Re-extracted all original and final BMDs through the engine converter. All six
  model SMDs and six action SMDs pass the converter's validation.
- Full-model comparisons report **DIFFERENT**, correctly reflecting new geometry.
  The isolated skeleton/action comparisons report **EQUIVALENT** for all three.
- Bone index 0, name, parent -1, bind pose, action count/order, frame 0 pose and
  `keys=1 lock=0` are unchanged. Numeric poses are exactly equal at the converter's
  six-decimal precision. Some serialized zero rotations change to negative zero;
  this is numerically identical. The informational model-name manifest line changes
  to the exported model name; action and texture metadata are unchanged.
- All three before/after bounds are exactly equal at six-decimal SMD precision.
  [results.json](results.json) retains the complete coordinates.
- Furniture04 retains all **4 unique** original straight-edge corners at
  `y=-0.396301`; Furniture05 retains all **16 unique** original end-plane corners
  at `x=-101.784302` and `x=101.685303`, each with zero coordinate deviation.
  Counts differ from the production report's 19/54 because that report counts
  repeated triangle-corner occurrences; both measurements describe the same anchors.
- Final geometry has finite positions/UVs, one root binding per vertex, nonzero
  triangle and UV areas, consistent winding/normal directions, and UVs within [0,1].
- All three atlas containers pass `mu_texture.py check`; their unwrapped JPEG bytes
  exactly match the delivered final JPEG. Export hashes exactly match the artist's
  installed source Data files. The three atlas copies are identical.
- Blender loaded every original and final packed source read only. `REF_ORIGINAL`
  retains identical numeric geometry, UVs, weights and transforms. Original rig
  bone order, parent relationships, action metadata and object transform match.
  Both original 128² and replacement 1024² images are packed. `REF_ORIGINAL` and
  editable `REF_HIGH_POLY` are hidden and tagged to exclude them from export.
  Active export meshes have one UV set, one material and one full bone influence.
  See [source-audit.json](source-audit.json) and [blender-audit.txt](blender-audit.txt).

## Visual review

Viewed the actual matching-camera comparison sheets for all three assets, including
their wireframes and reduced-scale previews; both modular joining sheets; all three
reverse views; the counter's exported UV atlas; and the shared overview.
Reviewed the scripts and render contexts: after images use re-imported final BMD/OZJ
files, matching the original camera and diffuse-only lighting. These remain offline
Blender evidence, not an emulation or observation of client shading.

The rebuilt table retains its broad plank silhouette and four splayed legs, with
rails and restrained iron adding structure. The half-table preserves its flat join
and flared pedestal while improving its curved rim. The counter's inset fronts keep
the original repeated ornamental identity and match its original end connections.
The actual placement comparison retains the half-tables' original offset and the
counter's existing overlap. No visible new gap or silhouette discontinuity was found.

The brown-grey oak, dark iron and large carved diamond/scroll motifs form a coherent
family. Compared directly with the pilot's Beer01 sheet and GroundTiles repeat sheet,
the subdued wood complements the still life's dark bottle, pewter and tankard and
the terrain's warm stone/olive palette. The broad carving remains legible in the
reduced previews, with fewer conflicting highlights than the original atlas.
Reverse faces are textured. The UV atlas separates the carved tabletop, carved panel,
plain timber and iron regions with visible margins; repeated UV islands are intended.
There is no alpha material or new render-name flag in this group.

Review images remain with the production deliverables after integration:

- `assets-work/World1/TavernProps/review-overview.jpg`
- `assets-work/World1/TavernProps/Furniture03/review/comparison.png`
- `assets-work/World1/TavernProps/Furniture04/review/comparison.png`
- `assets-work/World1/TavernProps/Furniture05/review/comparison.png`
- `assets-work/World1/TavernProps/Furniture04/review/joining-comparison.png`
- `assets-work/World1/TavernProps/Furniture05/review/joining-comparison.png`

## Integration and remaining work

Integrate the baseline commit, production commit and this review evidence sequentially.
The production commit also appends its prior session to `docs/agents/WORKLOG.md`;
the coordinator owns any shared-document conflict resolution. Game changes are exactly
the four files above. The reviewer changed only this review directory.

Actual client checks: **none**. Once the coordinator has a stable serial client
session, review loading, per-vertex illumination, filtering, culling and the placed
table/counter arrangements at matching 1920×1080 cameras. Preserve the offline status
until that evidence exists. There is no technical validator rejection or blocked asset
in this accepted group.

Reproduce from the review worktree:

```sh
python3 assets-work/World1/reviews/tavern-integration/recheck.py
/Applications/Blender.app/Contents/MacOS/Blender -b --python-exit-code 1 --python assets-work/World1/reviews/tavern-integration/audit_blends.py
```

The scripts deliberately read the isolated tavern production worktree and use the
existing primary checkout's converter. All their outputs remain inside this review
directory. The original-BMD guard assumes the review branch's untouched furniture;
after integration, reproduce this review from its retained review worktree.
