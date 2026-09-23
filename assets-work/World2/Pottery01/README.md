# Dungeon pottery

Editable sources and offline validation for Object2/Object28 and Object29; Object30 remains the original broken-pot companion. Read `notes.md` for the design, protected geometry and limitations. `candidate-manifest.json` binds final sources, exports, audit reports and review images by SHA-256. `independent-final-review.json` accepts both replacements and explicitly retains Object30. The reviewed manifest remains byte-identical to its pre-review snapshot; the independent report is the final decision.

## Reproduction

Run from the repository root with official Blender + enabled Blender Source Tools and the built converter:

```sh
export MU_BLENDER=/path/to/Blender
export MU_BMDCONV=/path/to/bmdconv
export BLENDER_USER_SCRIPTS=/path/to/isolated/scripts
export BLENDER_USER_CONFIG=/path/to/isolated/config
export PYTHONDONTWRITEBYTECODE=1
python3 assets-work/World2/Pottery01/pipeline.py prepare
python3 assets-work/World2/Pottery01/pipeline.py build
"$MU_BLENDER" -b --factory-startup --python-exit-code 1 --python assets-work/World2/Pottery01/assemblies.py
python3 assets-work/World2/Pottery01/make_comparisons.py
python3 assets-work/World2/Pottery01/make_manifest.py
```

Comparison generation requires Pillow; component-fit audits require NumPy (also present in the Blender runtime). Prepare imports the retained baseline BMDs using official tools. Build reconstructs the authored meshes, saves packed source files, exports through official tools, validates actual BMDs, reimports them and checks source correspondence before rendering. `official_io.py` only adapts metadata transport for the original CP949 name bytes; it does not modify the importer, exporter or converter.

Sources contain hidden `REF_ORIGINAL` collections and exact packed original texture payloads. Do not paint the frozen shared textures. Game installation is deliberately separate from reproduction; the pipeline writes only this deliverable folder.

## Evidence

- `Object28/review` and `Object29/review`: matching original/current/candidate main, reverse, reduced and wireframe views; labeled comparison sheets.
- `review-assemblies`: five actual placement groups, including 65-degree tilted Object29 and 25-degree tilted Object28 variants.
- Each `validation` folder: source/export triangle correspondence; raw normal world directions; raw bindings; exact rig/action/name/material contracts; packed texture payloads; UV/winding and protected surfaces; all actual rotated placement bounds.
- `shared-whole-pot-proof.json`: original triangle/corner affine correspondence, including the narrow anisotropic pot.
- `exported-variant-proof.json`: every final composite triangle accounted, with separate position and UV errors and exact material/bone/cyclic winding.
- `shared-broken-pot-proof.json`: retained broken-component correspondence and explicit six-triangle difference.
- `studies`: earlier 782-triangle and first 398-triangle canonical prototypes, preserved independently of final constrained family.

Renders are offline approximations using the exact decoded frozen textures. No new Dungeon client verification is claimed.
