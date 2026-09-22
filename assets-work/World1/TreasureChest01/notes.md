# TreasureChest01 — arched timber chest

**2026-09-22 / ASTRA — exported, validated offline and installed. Client verification pending.**

Rebuilt a hollow walnut chest with beveled panels, separate corner straps and binding rails, a twelve-section arched lid, end caps, rivets and a split hasp/lock. The dark wood and brass identity, closed silhouette and original rear hinge are retained. Body components remain bound to Bone01, lid components to Bone02; unused Bone03 and Box12 remain in their original indices.

## Geometry and unchanged contract

| Measure | Original | Replacement |
|---|---:|---:|
| Triangles | 66 | 1174 |
| Meshes | 1 | 1 |
| Bones | 4 | 4 |
| Actions / frames / lock | 1 / 7 / 0 | 1 / 7 / 0 |
| Bind minimum | -57.73 -34.36 0.05 | -57.73 -34.37 0.05 |
| Bind maximum | 56.83 29.96 84.09 | 56.83 29.96 84.09 |
| Bind size | 114.57 64.32 84.04 | 114.57 64.32 84.04 |

Bounds are the engine's bind-pose reports, rounded to two decimals. Full precision Blender
bounds are in `validation/blender.json`. All props stay below the 1,500-triangle target.

One mesh remains one mesh. The engine gives this object zero animation velocity in Lorencia, but all seven opening frames are preserved. Offline poses at frames 0, 3 and 6 show the new hollow body and lid following the original motion. The only footprint change is 0.0036 units of additional front projection for the keyhole, below 0.006% of depth; the other extents match.

## Textures and sources

treasure_chest.jpg / treasure_chest.OZJ: 128×128 → 512×512 RGB JPEG. Exclusive to this model among all 115 Object1 models. One diffuse atlas supplies walnut, brass and dark iron.

The texture-sharing decision is recorded in `original/textures.json` and the complete
[batch inventory](../StaticBatch01/inventory.json). No shared textures were repainted.
The built-in imagegen tool produced the paintings; exact generation/refinement prompts,
initial variants where applicable and the selected full-resolution masters are retained in
`paintings/`. Engine-sized PNG/JPEGs are in `textures/`, wrapped containers in `exports/`.
The final game appearance uses diffuse images only, with no PBR or normal maps.

`source.blend` packs both original and replacement textures, reuses the original rig/actions,
and contains `REF_ORIGINAL`, `EXPORT_TreasureChest01` and excluded `REF_HIGH_POLY` sources.
`original/source.blend` is the untouched official importer result. Original BMD, wrapped and
unwrapped textures, SMD/action files, bmdconv info and placements are preserved in `original/`.
The packed source audit passes; no external cached image is needed to open either source.

## Offline validation

`bmdconv validate` passes the reference mesh and every action. Blender checks pass finite
non-degenerate triangles, one UV set, one diffuse texture per mesh, identity transforms,
power-of-two textures and exactly one valid full-weight bone assignment per vertex.
`mu_texture.py check` passes every exported container. The exported BMD was re-imported with
`mu_bmd_import.py` for the final review renders.

Full-model comparison reports the intended topology change:

```text
meshes 1 vs 1, bones 4 vs 4, actions 1 vs 1, triangles 66 vs 1174
unmatched triangles: 66  max corner distance: 0.0000  max bone distance: 0.0000  differing bone names: 0
DIFFERENT
```

The zero corner-distance value is not a geometry-similarity measurement for unmatched
triangles. Separate zero-mesh BMDs made from the actual original/exported node and action
blocks compare as follows:

```text
meshes 0 vs 0, bones 4 vs 4, actions 1 vs 1, triangles 0 vs 0
unmatched triangles: 0  max corner distance: 0.0000  max bone distance: 0.0000  differing bone names: 0
EQUIVALENT
```

Node indices, names and parents, action order, all 7 frame indices and lock=0 match.
Additional local-pose checks include rotations (the converter itself compares bone origins):
maximum translation component difference 0.00000800 units, maximum Euler component difference
0.0000000000 radians modulo 2π. Both are below 0.0001 and reflect exchange-format precision.
See `validation/local-motion.json`, the raw SMDs and `validation/skeleton-compare.txt`.

## Review and client follow-up

[Comparison sheet](review/comparison.png) contains matched before/after renders, the
exported wireframe and an offline gameplay-scale preview with 100-unit terrain repeats and
a 190-unit figure proxy. The camera scale is an assumption, not a measured client camera.
These are **Blender renders, not client screenshots**. Candle additive rendering is
approximated for visual review; the actual client still needs to confirm it.

[Action sheet](review/action-comparison.png) compares original/exported poses at frames 0, 3 and 6.

The user authorized offline continuation after repeated baseline client crashes. Pending:
load/error-log checks, actual lighting/contrast, texture seams at real zoom, orientation,
footprint/contact with surrounding assets, and screenshots from matched client viewpoints.
Do not mark this asset verified in client before those checks pass.

World1 placements are unchanged; all records are in `original/placements.json`:

| Tile X | Tile Y | World Z | Z rotation | Scale |
|---:|---:|---:|---:|---:|
| 3.000 | 41.500 | 187.562 | 2310.000 | 0.680 |
| 1.812 | 41.689 | 170.446 | -660.000 | 1.000 |
| 185.166 | 140.065 | 165.000 | 1860.000 | 1.000 |

## Installation and reproduction

Exports retain all original filenames. They are installed into the `art/world1-pilot`
worktree's source Data and the existing macOS runtime Data. Checksums are in
[installed-files.json](../StaticBatch01/installed-files.json). All 317 non-batch files in
World1/Object1 match the session baseline in both destinations, including finished terrain,
TerrainLight, original alpha strips and Beer01. The original shared checkout was moved to
main externally during the session; this batch continued in an isolated worktree and did
not write replacement files or commits to main.

See the [batch reproduction instructions](../StaticBatch01/README.md). `build_source.py`
builds this asset on its preserved original import; export uses the unmodified official
`tools/blender/mu_bmd_export.py`. Validation, rendering and installation helpers are shared
under `StaticBatch01/`.
