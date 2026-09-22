# Lorencia scrub and tall grass — ASTRA — 2026-09-22

**Four assets exported and validated offline; client acceptance pending.**
Owner: ASTRA Scrub01 artist, branch `codex/lorencia-scrub`, isolated worktree
`/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-scrub`.
The coordinator owns the asset board, consolidated handoff and worklog.

Start with [review-overview.jpg](review-overview.jpg) and [alpha-review.png](alpha-review.png).
This is a painted material pass retaining the existing efficient authored vegetation
geometry, UVs, footprint, orientation, origins and rigs. No subdivision or new collision
geometry was introduced.

## Actual inventory and exact game changes

| Model under `src/bin/Data/Object1/` | Observed identity | Placements | Triangles | Bones | Mesh-slot textures |
|---|---|---:|---:|---:|---|
| `Tree09.bmd` | Five tapered tall-grass tuft shells | 42 | 240 → 240 | 5 | `tree_07.tga` |
| `Tree10.bmd` | Ten tapered tall-grass tuft shells | 250 | 480 → 480 | 10 | `tree_07.tga` |
| `Grass03.bmd` | Four layered upright scrub clumps | 119 | 128 → 128 | 4 | `tree_01.tga`, `tree_02.tga` |
| `Grass04.bmd` | Four layered upright scrub clumps, spread variant | 119 | 128 → 128 | 4 | `tree_01.tga`, `tree_02.tga` |

The remaining three changed game files are `src/bin/Data/Object1/tree_07.OZT`,
`src/bin/Data/Object1/tree_01.OZT` and `src/bin/Data/Object1/tree_02.OZT`.
These seven paths are the entire source Data change. All **317** non-owned
Object1/World1 files retain their pre-install hashes, including terrain, height,
walk data, placements, lighting, terrain alpha strips and earlier artists' assets.
The source Data installation and protected hashes are recorded in
[installed-files.json](installed-files.json). No runtime Data files were touched.

The inspection board's initial two/four-tussock description was corrected by
examining the actual vertices assigned to five/ten separate Cylinder roots.
Some tufts overlap in the initial viewing direction. Grass03/04 each have four
Sphere roots and four layered clumps. Complete World1 transforms are preserved
in each `original/placements.json`, covering **530 existing placements**.

The complete 115-model inventory establishes two independent texture groups:
`tree_07.OZT` has only Tree09/10 consumers; `tree_01.OZT` and `tree_02.OZT` have
only Grass03/04 consumers. The differently typed files `tree_01.OZJ` and
`tree_02.OZJ` belong to unrelated tree models and remain untouched. No texture
was renamed. [dependency-inventory.json](dependency-inventory.json) retains the
assigned models, actual placement records and complete texture-consumer lists.

The Lorencia loader references the original Tree/Grass series in
`src/source/World/MapInfra/MapManager.cpp:1029–1031`, then loads Object1 textures
at line 1097. Lorencia-specific object setup in `ZzzObject.cpp:4611–4674` has
special cases for other Tree indices, with no targeted slot mutation found for
these four models. The original mesh count/order and names are retained anyway.

## Art and alpha handling

The three originals were only **32×32 RGBA**. Built-in imagegen repainted their
color roles at high resolution; final packaged textures are **512×512 RGBA**.
The dark foliage has grouped pointed leaves with restrained olive/sage highlights.
Tall grass retains olive upper blades and dry brown stalk bases, with clearer
fibers and overlapping strands. This muted palette follows the completed terrain
and Beer01 references without adding glow or renderer features.

The original graded alpha fields are retained and enlarged with bilinear sampling;
there are no new silhouette cutouts or opaque rectangles. Alpha mean differs by at
most 0.031 on a 0–255 scale due to 8-bit interpolation rounding. The color painting
continues beneath every transparent texel, avoiding a separate background matte.
The alpha audit records 23 fully black RGB pixels in tree_01's natural shadow
painting, none in the other paintings, and no white pixels; these are sparse
painted recesses, not a black border. Light/dark composite inspection found no
introduced continuous fringe. The low-resolution original soft alpha edge remains
recognizable; the renderer's actual alpha testing/filtering must still be reviewed
in the client.

Every TGA is 32-bit, uncompressed, bottom-left origin and wraps successfully into
its original OZT name. No PBR, normal map or new material render flag is required.
The batch does not touch World1's alpha strips.

Raw imagegen outputs, final PNG/TGA/OZT, unmasked diffuse PNG, alpha PNG and layered
OpenRaster sources are retained in `textures/`. Each `.ora` provides visible final
RGBA and hidden editable color and alpha layers. Prompts and built-in tool provenance
are recorded in [generation-prompts.json](generation-prompts.json); packaging is
reproducible from the retained generated paintings. Generation itself is not
bit-for-bit deterministic.

## Validation and retained sources

All four assets were imported through the official `mu_bmd_import.py`, retain
packed `source.blend` with excluded `REF_ORIGINAL`, and were exported through
`mu_bmd_export.py`. The unchanged authored shell/card geometry already meets the
budget and has no new high-poly counterpart. Original packed imports and untouched
BMD/TGA/OZT files, converter info, model/action SMDs and placements are retained per
asset. Original texture copies are also retained in the shared `original/` folder.

- All **8 model SMDs and 8 action SMDs** (original plus final) pass the authoritative
  converter validator. All final textures pass `mu_texture.py check`.
- Every full original/export BMD comparison reports **EQUIVALENT** at the tool's
  default 0.05-unit tolerance: zero unmatched triangles; maximum corner drift
  0.0003 units. This batch preserves geometry, so full equivalence is appropriate.
- Bone names, order, parents, action order/count, frame counts and `lock=0` are
  preserved. Explicit original/export local-pose samples are in each validation
  summary. Maximum serialization translation drift is 0.000016 units; rotation
  drift is zero at the converter's six-decimal precision.
- Maximum bind-bound component drift is 0.000122 units through Blender/SMD float
  serialization. Full before/after bounds are retained in every asset's notes
  and validation summary. Source geometry/UVs/weights/transforms are exactly equal.
- Mesh material order is unchanged. No invalid weights, nonfinite data or collapsed
  UV triangles. Saved-source audit verifies untouched reference geometry, UVs,
  skinning, transforms, rig metadata and packed original/final images.
- Exporter source collections are disabled as independent Source Tools groups;
  the official exporter alone creates and exports its joined mesh. Final logs
  contain no Source Tools export errors or validator rejections.

Each asset's `review/comparison.png` shows matching-camera original and reimported
final BMD/OZT renders, wireframe and reduced-scale readability. Reverse views and
camera/lighting records are retained. Offline renders use diffuse-only Cycles
lighting; they do not emulate the client's per-vertex renderer. The review found
clearer blade/leaf detail, preserved cluster shapes and no opaque alpha plates.

## Reproduce and pending review

Run commands with this worktree as the explicit working directory. Pillow commands
can use `/Users/webproduktion3/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/bin/python3`.
Blender is `/Applications/Blender.app/Contents/MacOS/Blender`.

```sh
python3 assets-work/World1/Scrub01/scripts/package_textures.py
blender -b --python-exit-code 1 --python assets-work/World1/Scrub01/scripts/import_originals.py
blender -b --python-exit-code 1 --python assets-work/World1/Scrub01/scripts/build_and_export.py
python3 assets-work/World1/Scrub01/scripts/validate_exports.py
blender -b --python-exit-code 1 --python assets-work/World1/Scrub01/scripts/audit_sources.py
blender -b --python-exit-code 1 --python assets-work/World1/Scrub01/scripts/render_review.py
python3 assets-work/World1/Scrub01/scripts/assemble_review.py
python3 assets-work/World1/Scrub01/scripts/install.py
```

Imports are guarded and never overwrite original packed imports. The installer
requires this owned production branch, verifies original/expected destination hashes,
installs only seven owned source Data files, and checks all protected hashes.
It never targets the shared runtime. Coordinator integration should retain both
texture groups atomically with their respective models.

**Actually verified in client: nothing.** The coordinator still needs a stable
serial session for real 1920×1080 before/after captures, loading, map lighting,
alpha testing/filtering, culling and existing placed scale. No blocked validator
or remaining production defect is known in this batch; client acceptance is open.
