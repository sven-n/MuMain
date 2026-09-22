# TimberProps01 — barrel, weapons bin and hanging lantern

2026-09-22. Owner: ASTRA timber-props worker. Isolated branch `codex/lorencia-timber-props`,
worktree `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-timber-props`.
Status: three models exported and validated offline; coordinator integration/client
acceptance pending. No runtime or client session was operated.

| Asset | Actual identity | Placements | Triangles | Materials/rig |
|---|---|---:|---:|---|
| House02 | Open weapons bin with blade/shield display | 7 | 173 → 1215 | 2 meshes; 1 bone; 1 key |
| TreasureDrum01 | Sealed coopered oak barrel | 64 | 60 → 1456 | 1 mesh; 1 bone; 1 key |
| StreetLight01 | Braced timber hanging lantern | 10 | 84 → 572 | 2 meshes; 11 bones; 21 keys |

## Game files changed

- `src/bin/Data/Object1/House02.bmd`
- `src/bin/Data/Object1/TreasureDrum01.bmd`
- `src/bin/Data/Object1/StreetLight01.bmd`
- `src/bin/Data/Object1/drum.OZJ`
- `src/bin/Data/Object1/steel.OZJ`
- `src/bin/Data/Object1/streetlight.OZJ`
- `src/bin/Data/Object1/streetlight_brightness2.OZJ`

The model-to-texture inventory identifies drum.OZJ consumers as only House02 and
TreasureDrum01. Both are rebuilt together. Steel is exclusive to House02; both lantern
textures are exclusive to StreetLight01. No outside consumer or game filename changed.
Only these seven owned source files are installed; source-installation.json hashes them
and proves all other World1/Object1 files unchanged.

The imagegen skill and built-in imagegen tool produced retained paintings and exact
prompts in paintings/. Drum, steel and streetlight are 512² RGB; lantern brightness is
128² RGB. Editable masters/final PNGs remain alongside JPEG/OZJ exports. The barrel atlas
uses wood, blackened iron and two plank lids; existing UV roles are preserved. New UVs
avoid the lowest unneeded atlas border. Lantern material slot1 remains the original
additive JPEG convention; no new alpha, render flags, PBR or normal maps were introduced.

## Evidence and limitations

Each model passes bmdconv reference/action validation, loader texture checks, geometry,
rigid skinning, UV/material and packed-source audits. All original bone names/order/parents,
action count/order/locks and key counts are retained. Isolated skeleton/actions compare
EQUIVALENT. Full model compares are DIFFERENT for intended remodels. Engine bind bounds
agree at printed precision. Lantern all 231 bone/key samples differ by at most 0.000034 units
per local translation component and 0 radians per Euler component. Dummy bones remain
unbound. The other two rigs' local transforms are exact in converter output.

The static blade/shield display's 147 triangles retain positions, UVs and bones exactly.
The original lantern glow's 36 triangles retain positions within 0.000008 units, UVs and
bone assignments. Those checks do not claim unchanged normals; Blender recomputes them.
Original mesh/material slot order is proven by the engine info validator, especially
StreetLight01's BlendMesh=1 and 21-key action. Source render references and all reports are
retained in each notes.md and validation/ folder.

Every asset retains untouched game originals, official imported blend/info/SMD/actions,
packed rebuilt source.blend with REF_ORIGINAL and excluded high-poly geometry, export BMD,
editable diffuse art, imagegen prompt, matching-camera before/after BMD reimport renders,
wireframes and a reduced-scale proxy. StreetLight01 additionally retains frames 0/10/20
comparison renders plus neutral diffuse before/after views without additive approximation. Batch sheet: review/batch-review.jpg; per-model review/comparison.png;
lantern review/action-comparison.png.

**All review images are offline Blender evidence.** The lantern additive shader is a
preview approximation; opaque camera background preserves emitted light in the comparison
sheets. The actual game uses its unchanged diffuse/additive renderer and warm dynamic
terrain light. None of the new assets has been verified in client. No engine, CMake, UI,
equipment, character, other-map, terrain placement/height/walk/light/alpha-strip file changed.

## Reproduce

Run from the assigned or integrated checkout. Python needs Pillow; Blender needs Source
Tools. Set MU_BMDCONV to the built converter. Scripts leave original folders untouched.

```sh
export PYTHONDONTWRITEBYTECODE=1
export MU_BMDCONV=/path/to/bmdconv
python3 assets-work/World1/TimberProps01/package_textures.py
/Applications/Blender.app/Contents/MacOS/Blender -b --python assets-work/World1/TimberProps01/build_source.py
python3 assets-work/World1/TimberProps01/export_all.py
python3 assets-work/World1/TimberProps01/validate_export.py
python3 assets-work/World1/TimberProps01/validate_preserved.py
/Applications/Blender.app/Contents/MacOS/Blender -b --python assets-work/World1/TimberProps01/render_exports.py
python3 assets-work/World1/TimberProps01/assemble_review.py
python3 assets-work/World1/TimberProps01/write_notes.py
python3 assets-work/World1/TimberProps01/install_source.py
```

prepare_originals.py is baseline-only and refuses to replace saved originals. Import and
export use the official repository tools; reproduction disables Python cache writes in
those subprocesses. install_source.py is confined to the seven owned source game files.
Pending coordinator integration checks, then serial real client load/log, material/light,
lantern animation/blending, contact and 1920x1080 before/after review when stability permits.
