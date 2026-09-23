# Dungeon carved pier: rejected studies and retention assessment

`independent-final-direction.json` rejects the487-triangle shelf and explicitly retains baseline Object04. Read `notes.md` for the artistic decision, complete texture dependency analysis, contract evidence and limitations. No game BMD or texture has been changed.

The current `Object04/source.blend` is the second, deliberately planar brow/socket study (487 triangles), not an accepted replacement. It includes packed original textures and hidden REF_ORIGINAL geometry. The first cubic study (576 triangles) is preserved in `studies/geometry01`. The 90-triangle original/current model remains under `Object04/original` and `Object04/baseline`.

`geometry02-manifest.json` binds the final second study's actual export, source, audits and images. Its matching full/reverse/reduced and three actual-placement comparison sheets are under `Object04/review` and `review-assemblies`. Full Data atlas-consumer scan, periodic UV masks, read-only Object16/17 previews and all three rejected imagegen portrait masters remain available for future reference. None of the paintings passed identity/registration review; no replacement texture was packaged.

## Reproduction of the current study

From the repository root, set `MU_BLENDER`, `MU_BMDCONV`, `BLENDER_USER_SCRIPTS` and `BLENDER_USER_CONFIG` to the official configured Blender/SourceTools and converter paths, then run:

```sh
python3 assets-work/World2/Pier01/pipeline.py prepare
python3 assets-work/World2/Pier01/pipeline.py build
"$MU_BLENDER" -b --factory-startup --python-exit-code 1 --python assets-work/World2/Pier01/audit_context.py
"$MU_BLENDER" -b --factory-startup --python-exit-code 1 --python assets-work/World2/Pier01/assemblies.py
python3 assets-work/World2/Pier01/make_comparisons.py
```

The assembly script's unmodified Object01 context source defaults to the sibling integration worktree `MuMain-dungeon-remake/assets-work/World2/Readiness01`; override `DUNGEON_CONTEXT_ROOT` if that reference folder moves. Its provenance is retained in the integration readiness records. Context mesh transforms are independently checked against raw SMD bounds.

Consumer imports use `import_consumers.py` with `-- import Object16 baseline` or Object17. `render_consumers.py` creates all-material and isolated shared-strip previews without saving material changes into their baseline sources. `texture_masks.py` requires Pillow and NumPy and reproduces the periodic protected-region masks. `inspect_texture_consumers.py` reads the full current Data inventory; unsupported model headers are verified using the converter.

Prepared coefficient-patching scripts are adaptations of the accepted Fountain02 method, limited to this atlas's 64×128 RGB 4:4:4 layout. They are unused because all portrait candidates failed registration; their presence is not a packaging validation claim or texture-change authorization.

All evidence is offline. No new client verification is claimed.
