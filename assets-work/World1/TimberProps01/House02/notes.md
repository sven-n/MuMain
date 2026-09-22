# House02 — Open sloped-rim weapons bin

2026-09-22. Owner: ASTRA timber-props worker. Branch `codex/lorencia-timber-props`.
Status: exported and validated offline; not verified in client.

Rebuilt plank walls, sloped display bed, bevels and raised iron bands. The 147-triangle oversized blade/shield display is retained with original positions, UVs and one-bone assignment; normals are recalculated. This is placed scenery, not an equipment-file edit.

## Geometry, placement and textures

- 173 → 1215 triangles, under the 1500-triangle prop budget.
- Before authored bind min / max: -114.88810, -64.12230, -0.16240 / 89.38980, 72.77970, 188.55130.
- After authored bind min / max: -114.88810, -64.12230, -0.16155 / 89.38980, 72.77970, 188.55130.
- Engine before min / max: -114.89000, -64.12000, -0.16000 / 89.39000, 72.78000, 188.55000.
- Engine after min / max: -114.89000, -64.12000, -0.16000 / 89.39000, 72.78000, 188.55000.
- one original root bone; one one-key action, lock=0; original bone order: Box02.
- Textures: drum.OZJ (shared only with TreasureDrum01), steel.OZJ (exclusive).
- Export material order: drum.jpg, steel.jpg.
- Texture dimensions: {'drum.jpg': [512, 512], 'steel.jpg': [512, 512]}.

7 unchanged World1 instances. Full position/rotation/scale records are in
`original/placements.json`. First instance tile: [7.0, 148.5]; world position:
[700.0, 14850.0, 161.6428680419922]; rotation: [-5.0, 0.0, 2190.0]; scale 0.820000171661377.
Actual identity was checked in imported geometry/materials and the World1 inventory, not
inferred from the filename. House02 is a weapons bin, not a building.

## Validation

Original and replacement SMDs/actions pass `bmdconv validate`. Texture containers pass
`mu_texture.py check`. Before/after engine bounds agree to printed precision. All original
bone names, indices, parents, action order and lock flags remain. Every bind/action local
transform was compared: maximum position component 0.000000000 units and Euler
component 0.000000000 radians modulo 2*pi. No action timing was changed.

Full-model comparison reports intentional geometry differences:

```text
meshes 2 vs 2, bones 1 vs 1, actions 1 vs 1, triangles 173 vs 1215
unmatched triangles: 95  max corner distance: 0.0000  max bone distance: 0.0000  differing bone names: 0
DIFFERENT
```

The isolated original/replacement skeletons include their actual action data:

```text
meshes 0 vs 0, bones 1 vs 1, actions 1 vs 1, triangles 0 vs 0
unmatched triangles: 0  max corner distance: 0.0000  max bone distance: 0.0000  differing bone names: 0
EQUIVALENT
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
