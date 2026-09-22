# Candle01 — three-candle bronze stand

**2026-09-22 / ASTRA — exported, validated offline and installed. Client verification pending.**

Rebuilt the turned stem, octagonal foot, forged branching arms, small gothic scrolls, drip cups, rounded beeswax candles and wicks. The original three-candle silhouette, different candle heights, origin and orientation remain. The six original crossed flame triangles retain their geometry, bone assignments and UV coordinates.

## Geometry and unchanged contract

| Measure | Original | Replacement |
|---|---:|---:|
| Triangles | 116 | 1018 |
| Meshes | 2 | 2 |
| Bones | 9 | 9 |
| Actions / frames / lock | 1 / 7 / 0 | 1 / 7 / 0 |
| Bind minimum | -19.32 -11.26 0.01 | -19.32 -11.26 0.01 |
| Bind maximum | 19.35 11.21 87.07 | 19.35 11.21 87.07 |
| Bind size | 38.67 22.47 87.06 | 38.67 22.47 87.06 |

Bounds are the engine's bind-pose reports, rounded to two decimals. Full precision Blender
bounds are in `validation/blender.json`. All props stay below the 1,500-triangle target.

Two mesh slots are preserved in their original order: candle.jpg (1,012 triangles), candle2.jpg (6 triangles). ZzzObject.cpp uses BlendMesh=1 and velocity 0.3; no code or effect placement changed. The original flame UV range extends to 1.0074 and intentionally remains unchanged. The replacement sprite has a dark border appropriate to the existing additive blend. No filename render flags were added.

## Textures and sources

candle.jpg / candle.OZJ: 32×32 → 512×512. candle2.jpg / candle2.OZJ: 16×16 → 128×128. Both are RGB JPEG textures exclusive to Candle01 among all 115 Object1 models.

The texture-sharing decision is recorded in `original/textures.json` and the complete
[batch inventory](../StaticBatch01/inventory.json). No shared textures were repainted.
The built-in imagegen tool produced the paintings; exact generation/refinement prompts,
initial variants where applicable and the selected full-resolution masters are retained in
`paintings/`. Engine-sized PNG/JPEGs are in `textures/`, wrapped containers in `exports/`.
The final game appearance uses diffuse images only, with no PBR or normal maps.

`source.blend` packs both original and replacement textures, reuses the original rig/actions,
and contains `REF_ORIGINAL`, `EXPORT_Candle01` and excluded `REF_HIGH_POLY` sources.
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
meshes 2 vs 2, bones 9 vs 9, actions 1 vs 1, triangles 116 vs 1018
unmatched triangles: 110  max corner distance: 0.0000  max bone distance: 0.0000  differing bone names: 0
DIFFERENT
```

The zero corner-distance value is not a geometry-similarity measurement for unmatched
triangles. Separate zero-mesh BMDs made from the actual original/exported node and action
blocks compare as follows:

```text
meshes 0 vs 0, bones 9 vs 9, actions 1 vs 1, triangles 0 vs 0
unmatched triangles: 0  max corner distance: 0.0000  max bone distance: 0.0000  differing bone names: 0
EQUIVALENT
```

Node indices, names and parents, action order, all 7 frame indices and lock=0 match.
Additional local-pose checks include rotations (the converter itself compares bone origins):
maximum translation component difference 0.00001500 units, maximum Euler component difference
0.0000000000 radians modulo 2π. Both are below 0.0001 and reflect exchange-format precision.
See `validation/local-motion.json`, the raw SMDs and `validation/skeleton-compare.txt`.

The preserved flame-only BMD also compares **EQUIVALENT**, 6 vs 6 triangles, zero unmatched
triangles, zero reported corner/bone deviation. `validation/flame-uv.txt` confirms all 18
corners retain their positions, rigid bones and UVs. The imported UV layer was renamed from
Float2 to UVMap on the replacement copy so the exporter joins it without losing coordinates;
`REF_ORIGINAL` is untouched.

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
| 62.000 | 131.500 | 165.000 | 2430.000 | 1.040 |
| 125.310 | 122.532 | 270.000 | 270.000 | 1.000 |
| 122.466 | 134.418 | 275.000 | 240.000 | 1.000 |
| 126.615 | 134.489 | 275.000 | 210.000 | 1.000 |
| 124.554 | 134.646 | 275.000 | 390.000 | 1.000 |
| 126.584 | 128.254 | 250.000 | 60.000 | 1.000 |

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
