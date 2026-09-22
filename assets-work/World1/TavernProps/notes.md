# Lorencia tavern furniture — ASTRA — 2026-09-22

**Exported, validated offline and installed in this worktree's source Data.
Client verification remains pending under the user's explicit authorization.**

Branch: `art/lorencia-tavern-props`, based on `main` at `9a8b2027`.
Worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-tavern-props`.
Start the visual review with [review-overview.jpg](review-overview.jpg).

## Selection and exact game changes

| File under `src/bin/Data/Object1/` | Observed identity | Triangles before → after | World1 instances |
|---|---|---:|---:|
| `Furniture03.bmd` | Four-legged rectangular tavern table | 42 → 680 | 5 |
| `Furniture04.bmd` | Half-round table on a flared pedestal | 38 → 324 | 3 |
| `Furniture05.bmd` | Repeated tavern counter section | 34 → 412 | 3 |
| `desk_big.OZJ` | Shared opaque carved-oak/iron diffuse atlas | 128² → 1024² pixels | All three models |

These four files are the entire game-data change. All 115 Object1 models were inventoried;
`desk_big.jpg` has exactly these three consumers in that folder. All three are included
so the shared texture can be repainted safely. Their geometry was inspected through Blender
imports and original renders, and their placements were decoded read-only from
`World1/EncTerrain1.obj`. See [inventory.json](inventory.json) and [claims.md](claims.md).

Ownership checks covered every active worktree and the World1/UI handoffs. The static
artist explicitly deferred this furniture set. The World1 pilot's handoff, Beer01 notes,
source script and comparison were read as references without importing its changeset.
Beer01 is the bottle/mug/bowls/grapes still life; neither it nor `plate2` was edited.

The Lorencia loader uses the existing Furniture model series in `MapManager.cpp:1091`;
the enum maps these models to types 142–144. Source searches found furniture interaction
handling for types 145–146 (chairs), not this batch, and no explicit independent `desk_big`
texture loads. Each replacement nevertheless retains its original single mesh and material.

## Art and preservation

Aged warm grey-brown oak, restrained wrought iron and the original interlaced diamond/scroll
identity form one material family. Carving is painted into diffuse color; no normal maps,
PBR, glow or new material flags are required. Timber bevels, structural rails, a smoother
half-round silhouette, iron foot bindings and inset counter panels account for the added
geometry. All props remain below the documented 1,500-triangle budget.

The original model origin, orientation and measured outer bounds are retained. Furniture03
keeps its four measured splayed-leg centers; Furniture04 keeps its flared pedestal footprint
and all straight joining-edge corners. Furniture05 retains every original end-plane corner,
including the unchamfered tabletop connections. Its 200-unit repetition spacing and existing
3.4696-unit overlap remain unchanged. The paired half tables retain their original offset.
The [joining comparisons](Furniture05/review/joining-comparison.png) use the actual stored
transforms. No placement, terrain, collision or walk data was edited.

## Delivered sources and evidence

Each asset directory contains original BMD/OZJ/JPEG files, original converter info/SMDs,
placements and packed original import; packed `source.blend` with `REF_ORIGINAL`, original
active rig/action, export geometry and excluded editable `REF_HIGH_POLY` bevel sources;
generated painting, lossless PNG, four-layer OpenRaster `.ora`, final JPEG/OZJ and exact
generation prompt; BMD exports; and validation/review folders.

The atlas was created with the **built-in imagegen tool** using the original texture as an
ornament/material reference. [generation-prompts.json](generation-prompts.json) records
the full prompt and retained input/output. Each prop's `textures/` contains the same shared
atlas source for a self-contained handoff. Treat the shared painting as one dependency when
editing further. Open the ORA in an OpenRaster editor; the four editable layers are tabletop,
panel, plain oak and iron. The final JPEG is RGB, quality 97, 4:4:4, wrapped by `mu_texture.py`.

Matching-camera before/after renders, wireframes, reverse views, exported UV overlays and
reduced-scale comparisons are labeled **OFFLINE BLENDER**. The final after views were made
by re-importing the actual BMD/OZJ exports, not merely rendering the working meshes.
They use diffuse-only lighting, but do not emulate the client's per-vertex renderer.

## Validation results

- Original and replacement SMDs/actions pass `bmdconv validate`; all three delivered texture
  copies pass `mu_texture.py check`. Wrap/unwrap preserves the exact JPEG bytes.
- Each model retains one mesh, one original named root bone (same index and parent), one
  action, one keyframe and `lock=0`. No secondary vertex weights or dummy bindings.
- Full-model `compare` reports `DIFFERENT`, intentionally: the geometry is remodeled.
  Rig-only BMD comparisons report **EQUIVALENT**, with zero bone deviation. Independent
  original/export SMD comparisons also report zero local translation/rotation deviation.
- Original/export bounds are identical at the SMD's six-decimal precision. All original
  joining-edge corner samples for Furniture04/05 have zero coordinate deviation.
- Blender checks pass finite coordinates, one UV set, manifold closed component meshes,
  nonzero triangle areas and identity transforms. Export checks pass UV area and winding:
  **zero collapsed UV triangles and zero winding/normal disagreements**.
- Source audits prove unchanged numeric original geometry, UVs, skinning and transforms in
  `REF_ORIGINAL`; both the original and replacement images are packed. The high-poly
  collection and original references are excluded from game export.
- [installed-files.json](installed-files.json) records exact installed hashes. All **320**
  non-claimed Object1/World1 files match the preserved baseline. No UI/engine/CMake change,
  shared runtime write or client launch was made.

Visual review checked the single and paired silhouettes, carved grain at reduced scale,
atlas-region separation, end alignment and reverse-facing surfaces. A collapsed table
bevel UV issue and a coincident rear counter face were corrected before final installation.
The new atlas is opaque: there are no alpha edges to validate. Intentional UV reuse occurs
within wood/iron fields; the atlas islands are inset from neighboring materials.

## Reproduce

Run from this worktree with Blender 5.2 + Source Tools. Set `BMD_CONVERTER` to a built
`bmdconv` if the default sibling `MuMain/out/build/...` tool is unavailable. The converter
is used read-only except for outputs below this batch. Packaging and sheet assembly require
Pillow; the bundled interpreter on this Mac is
`/Users/webproduktion3/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/bin/python3`.

```sh
python3 assets-work/World1/TavernProps/scripts/package_texture.py
/Applications/Blender.app/Contents/MacOS/Blender -b --python-exit-code 1 --python assets-work/World1/TavernProps/scripts/build_sources.py
python3 assets-work/World1/TavernProps/scripts/export_models.py
python3 assets-work/World1/TavernProps/scripts/validate_exports.py
/Applications/Blender.app/Contents/MacOS/Blender -b --python-exit-code 1 --python assets-work/World1/TavernProps/scripts/audit_sources.py
/Applications/Blender.app/Contents/MacOS/Blender -b --python-exit-code 1 --python assets-work/World1/TavernProps/scripts/render_review.py
python3 assets-work/World1/TavernProps/scripts/assemble_review.py
python3 assets-work/World1/TavernProps/scripts/install.py --install
```

Inventory and original-import scripts are guarded first-run steps; do not run them over the
preserved originals. Generation itself is not deterministic; packaging reuses the retained
painting. The installer checks the branch, all export hashes and protected baseline before
copying only the four claimed source Data files. It never targets the runtime.

## Pending client checks

When a stable, coordinated client session is available, capture the original and replacement
assets at the same camera/zoom and 1920×1080 resolution. Inspect the table cluster near tiles
(122–128, 125.5–129.5), the paired half tables near (124.9, 122.35), and the counter near
(122.5–126.5, 134.5). The additional table at (79.5, 153) needs review too. Verify loading,
actual visibility/occlusion, per-vertex lighting, readable timber contrast, texture filtering,
backface culling, tabletop repetition/overlap and silhouette fit among existing props.
Check MuError.log for replacement-related load errors. Do not mark this batch client-verified
until those checks pass.
