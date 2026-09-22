"""Write deliverable notes from converter, Blender and preservation checks."""
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parent
NAMES=('House02','TreasureDrum01','StreetLight01')
IDENTITIES={'House02':'Open sloped-rim weapons bin','TreasureDrum01':'Sealed coopered oak barrel',
            'StreetLight01':'Animated timber hanging lantern'}
COUNTS={'House02':173,'TreasureDrum01':60,'StreetLight01':84}
CHANGES={'House02':'Rebuilt plank walls, sloped display bed, bevels and raised iron bands. The 147-triangle oversized blade/shield display is retained with original positions, UVs and one-bone assignment; normals are recalculated. This is placed scenery, not an equipment-file edit.',
         'TreasureDrum01':'Rebuilt sixteen individual thick staves, rounded barrel silhouette, two raised iron hoops, inset planked lid, stave lip and bottom closure. Higher triangle use represents actual stave thickness and silhouette; no normal maps or PBR are required.',
         'StreetLight01':'Beveled the braced timber post, replaced its foot with a fitted iron shoe, added crossbeam straps and six narrow lantern stays, and rebuilt tiered bronze caps. All original glass positions, triangles, UVs and bone assignments are retained. New glass remains the existing additive JPEG material in mesh slot 1.'}


def format_bounds(bounds):
    return ' / '.join(', '.join(f'{value:.5f}' for value in point) for point in bounds)


def asset_note(name):
    folder=ROOT/name
    data=json.loads((folder/'validation/blender.json').read_text())
    summary=json.loads((folder/'validation/summary.json').read_text())
    placements=json.loads((folder/'original/placements.json').read_text())
    motion=json.loads((folder/'validation/local-motion.json').read_text())
    max_position=max(item['max_position_component'] for item in motion['samples'])
    max_rotation=max(item['max_euler_component_radians'] for item in motion['samples'])
    dependencies={'House02':'drum.OZJ (shared only with TreasureDrum01), steel.OZJ (exclusive)',
                  'TreasureDrum01':'drum.OZJ (shared only with House02)',
                  'StreetLight01':'streetlight.OZJ and streetlight_brightness2.OZJ (both exclusive)'}[name]
    metadata='11 bones including three original dummy bones; one 21-key action, lock=0' if name=='StreetLight01' else 'one original root bone; one one-key action, lock=0'
    text=f'''# {name} — {IDENTITIES[name]}

2026-09-22. Owner: ASTRA timber-props worker. Branch `codex/lorencia-timber-props`.
Status: exported and validated offline; not verified in client.

{CHANGES[name]}

## Geometry, placement and textures

- {COUNTS[name]} → {data['triangles']} triangles, under the 1500-triangle prop budget.
- Before authored bind min / max: {format_bounds(data['bounds_before'])}.
- After authored bind min / max: {format_bounds(data['bounds_after'])}.
- Engine before min / max: {format_bounds(summary['bounds_before'])}.
- Engine after min / max: {format_bounds(summary['bounds_after'])}.
- {metadata}; original bone order: {', '.join(data['bone_order'])}.
- Textures: {dependencies}.
- Export material order: {', '.join(summary['textures'])}.
- Texture dimensions: {data['textures']}.

{len(placements)} unchanged World1 instances. Full position/rotation/scale records are in
`original/placements.json`. First instance tile: {placements[0]['tile']}; world position:
{placements[0]['position']}; rotation: {placements[0]['rotation']}; scale {placements[0]['scale']}.
Actual identity was checked in imported geometry/materials and the World1 inventory, not
inferred from the filename. House02 is a weapons bin, not a building.

## Validation

Original and replacement SMDs/actions pass `bmdconv validate`. Texture containers pass
`mu_texture.py check`. Before/after engine bounds agree to printed precision. All original
bone names, indices, parents, action order and lock flags remain. Every bind/action local
transform was compared: maximum position component {max_position:.9f} units and Euler
component {max_rotation:.9f} radians modulo 2*pi. No action timing was changed.

Full-model comparison reports intentional geometry differences:

```text
{(folder/'validation/compare.txt').read_text().strip()}
```

The isolated original/replacement skeletons include their actual action data:

```text
{(folder/'validation/skeleton-compare.txt').read_text().strip()}
```

No full-model equivalence claim is made. `validation/` contains complete comparison,
info, SMD/action validation, local-transform, source-audit and texture-check output.
New mesh geometry has one finite UVMap, nondegenerate triangles and exactly one valid
full-weight non-dummy bone per vertex. Original lantern UV repeat to -0.0936 is retained.
No texture filename, alpha convention or render flag changed.

`source.blend` reopens with packed images, untouched original rig/action, hidden
`REF_ORIGINAL` and excluded `REF_HIGH_POLY`. All originals, unwrapped textures, converter
info/SMD/actions, official imported source and geometry/placement inspection are retained.
Raw built-in imagegen paintings, exact prompts and packaging provenance are in
`../paintings/`; per-model final PNG/JPEG and game exports remain editable.

## Offline review and pending client checks

`review/comparison.png` presents matching-camera original and re-imported BMD export,
wireframe and reduced-scale preview. Its 190-unit proxy and 100-unit ground repeats use
an assumed offline camera. They are **not client screenshots**.

StreetLight01 retains its two mesh slots: streetlight.jpg first and
streetlight_brightness2.jpg second, required by `ZzzObject.cpp:4646` (`BlendMesh=1`,
`Velocity=0.3`). Existing terrain-light behavior at `ZzzObject.cpp:3844` is unchanged.
`StreetLight01/review/action-comparison.png` compares frames 0/10/20; all 21 keys and all
11 bones were checked numerically. The additive preview uses transparent plus image
emission against an opaque camera backdrop; Blender light is an approximation, not proof
of the client's blend/light response. The game export itself has only original diffuse
materials. Separate `StreetLight01/review/neutral-before.png` and `neutral-after.png` show diffuse-only materials without the preview blend. House02's retained steel and the lantern's 36 glow triangles have additional
position/UV/skin checks in `validation/preserved-geometry.json` where applicable.

No client or shared runtime was operated. Pending: real asset loading/logs, light and
additive response, lamp motion, contact/orientation, gameplay zoom, and serial matching
1920x1080 before/after screenshots. See `../notes.md` for reproduction and batch scope.
'''
    (folder/'notes.md').write_text(text)


def batch_note():
    text='''# TimberProps01 — barrel, weapons bin and hanging lantern

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
agree at printed precision. Lantern all231 bone/key samples differ by at most0.000034 units
per local translation component and0 radians per Euler component. Dummy bones remain
unbound. The other two rigs' local transforms are exact in converter output.

The static blade/shield display's147 triangles retain positions, UVs and bones exactly.
The original lantern glow's36 triangles retain positions within0.000008 units, UVs and
bone assignments. Those checks do not claim unchanged normals; Blender recomputes them.
Original mesh/material slot order is proven by the engine info validator, especially
StreetLight01's BlendMesh=1 and21-key action. Source render references and all reports are
retained in each notes.md and validation/ folder.

Every asset retains untouched game originals, official imported blend/info/SMD/actions,
packed rebuilt source.blend with REF_ORIGINAL and excluded high-poly geometry, export BMD,
editable diffuse art, imagegen prompt, matching-camera before/after BMD reimport renders,
wireframes and a reduced-scale proxy. StreetLight01 additionally retains frames0/10/20
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
lantern animation/blending, contact and1920x1080 before/after review when stability permits.
'''
    # Keep measurement prose legible after interpolation and hand-written technical lists.
    for old,new in [('all231','all 231'),('most0.','most 0.'),('and0 ','and 0 '),('display\'s147','display\'s 147'),
                    ('glow\'s36','glow\'s 36'),('within0.','within 0.'),('and21','and 21'),('frames0/','frames 0/'),('and1920','and 1920')]:
        text=text.replace(old,new)
    (ROOT/'notes.md').write_text(text)


if __name__=='__main__':
    for name in NAMES:
        asset_note(name)
    batch_note()
