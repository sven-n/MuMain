# Furniture05 — Modular tavern counter

**Offline validated and installed in this worktree's source Data. Client verification pending.**

The narrow central body is rebuilt as a continuous back, framed carved front panels and the two original-position splayed supports. All original end-plane corners are retained, including square tabletop ends. Rear frame parts terminate at the backing so there are no coincident exposed back faces.

## Counts, dimensions and material

Original converter report:

```text
model src/bin/Data/Object1/Furniture05.bmd
  name field: Data2\Object1\house_in_05.smd  version: 10
  meshes: 1  bones: 1  actions: 1  triangles: 34
  bounds (bind pose): min -101.78 -34.81 0.00  max 101.69 35.18 105.38  size 203.47 69.99 105.38
  mesh 0: triangles=34 vertices=32 normals=7 uvs=28 texture=desk_big.jpg
  bone 0: "Box13" parent=-1
  action 0: keys=1 lock=0
```

Replacement converter report:

```text
model /Users/webproduktion3/Documents/claude-test-mumain/MuMain-tavern-props/assets-work/World1/TavernProps/Furniture05/exports/Furniture05.bmd
  name field: Furniture05.smd  version: 12
  meshes: 1  bones: 1  actions: 1  triangles: 412
  bounds (bind pose): min -101.78 -34.81 0.00  max 101.69 35.18 105.38  size 203.47 69.99 105.38
  mesh 0: triangles=412 vertices=228 normals=113 uvs=130 texture=desk_big.jpg
  bone 0: "Box13" parent=-1
  action 0: keys=1 lock=0
```

The original and exported SMD bounds match at all six decimal places; maximum component
deviation is 0.000000 units. Each model keeps one mesh, one named root bone,
one one-frame action and lock=0. The original pivot and orientation are retained.
The original 128×128 `desk_big.jpg` becomes a 1024×1024 RGB diffuse atlas, wrapped under the
same `desk_big.OZJ` filename. It is shared exclusively by Furniture03/04/05 in Object1;
all three consumers are replaced together. No new render flags, alpha or shader inputs.

## Placement

World1 type 144; 3 stored instances. Positions are game units (100 per terrain tile).
These records were decoded read-only; nothing in World1 was changed.

| Instance | Position X, Y, Z | Rotation X, Y, Z (degrees) | Scale |
|---|---|---|---:|
| 1 | 12250.000000, 13450.000000, 165.000000 | 0, 0, 0 | 1 |
| 2 | 12450.000000, 13450.000000, 165.000015 | 0, 0, 0 | 1 |
| 3 | 12650.000000, 13450.000000, 165.000000 | 0, 0, 0 | 1 |

## Compare and preservation

Full-model comparison intentionally differs because the geometry was rebuilt:

```text
meshes 1 vs 1, bones 1 vs 1, actions 1 vs 1, triangles 34 vs 412
unmatched triangles: 31  max corner distance: 0.0000  max bone distance: 0.0000  differing bone names: 0
DIFFERENT
```

Isolated original/export rigs, constructed from each BMD's actual node, bind and action data:

```text
meshes 0 vs 0, bones 1 vs 1, actions 1 vs 1, triangles 0 vs 0
unmatched triangles: 0  max corner distance: 0.0000  max bone distance: 0.0000  differing bone names: 0
EQUIVALENT
```

`validation/summary.json` also records zero local bone translation/rotation deviation,
unchanged action index/frame/lock metadata, precise bounds and connection checks. Converter
validation passes both SMDs and actions. Exported triangles have nonzero UV area and matching
winding/normals. `texture-check.txt` passes. Zero corner distance in the full compare is not
claimed as geometric equivalence: unmatched triangles are intentional.

`source.blend` packs original and new textures, preserves original numeric geometry/UVs/skin
and transforms in excluded `REF_ORIGINAL`, and retains the untouched active rig/action.
`REF_HIGH_POLY` contains excluded editable higher-resolution bevel sources. Source audit
results are in `validation/source-audit.json`. The final BMD was exported through
`mu_bmd_export.py` and re-imported through `mu_bmd_import.py` for review.

## Files and review

- `original/`: immutable BMD/OZJ/JPEG, packed original import, SMD/action/manifest,
  geometry inspection, converter info and placements.
- `textures/`: retained imagegen painting, lossless PNG, four-layer editable OpenRaster
  source, final JPEG and exact built-in-tool generation prompt.
- `exports/`: validated BMD and the shared OZJ; installed hashes are in the batch manifest.
- `review/comparison.png`: matching-camera before/after, exported wireframe and reduced
  previews. `reverse-offline.png` and `uv-atlas.png` cover the rear and atlas islands.
- `validation/`: Blender/source audits, actual comparison artifacts, SMD validation,
  exporter log and packed re-import of the game exports.

All renders are **OFFLINE BLENDER**, not client screenshots. Wood grain, carving, silhouette,
UV island separation and reverse surfaces were inspected. Atlas regions intentionally reuse
UVs; they are inset from neighboring materials. The JPEG is fully opaque, so alpha-edge
checks are not applicable. Runtime lighting, culling, filtering, visibility and placement
fit remain pending. See the batch [handoff](../notes.md) for reproduction and client checks.
