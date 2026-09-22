# Beer01 — tavern still life

**Exported and validated offline; client verification pending.** The user confirmed the
actual still life after the brief's barrel identification proved incorrect.

Rebuilt bottle, hollow bowl, serving platter, oak tankard with metal bands/handle, clustered
purple grapes and vine leaves. Model origin remains unchanged. `source.blend` contains the
untouched imported rig and action, the original geometry in `REF_ORIGINAL`, export geometry
in `EXPORT_Beer01`, and excluded higher-resolution sources in `REF_HIGH_POLY` for future
baking. The latter retains subdivision modifiers on the new authored geometry; it is not
part of the game export. Texture appearance requires only one diffuse image, with no PBR,
normal map or extra render flag.

## Geometry and bounds

| | Original | Replacement |
|---|---:|---:|
| Meshes | 5 | 1 |
| Triangles | 216 | 784 |
| Bones | 5 | 5 |
| Actions / keys / lock | 1 / 1 / 0 | 1 / 1 / 0 |
| Bind minimum | -50.14, -29.63, -0.34 | -50.13, -29.63, -0.34 |
| Bind maximum | 41.07, 28.94, 61.46 | 41.06, 28.85, 61.46 |
| Bind size | 91.20 × 58.57 × 61.80 | 91.19 × 58.48 × 61.80 |

Bounds differ by less than 0.2%. Raw reports: `original/info.txt`, `validation/info-after.txt`.
Original bone order: Cylinder09, Cylinder11, Cylinder08, Sphere05, Cylinder31; all roots.
The original action metadata and single keyframe are retained.

## Textures and mesh consolidation

The existing `plate2.jpg` material now contains the sole 512×512 RGB diffuse atlas, exported
as `plate2.OZJ` with the existing exact filename. Original prop textures were 32×32. The
other four texture names are shared with Beer02/03 or Furniture01/02, so their game files
were left unchanged (`original/shared-textures.json`). No new game filenames were added.

Static Beer01 has no special mesh-index render handling in the source references inspected.
Its geometry is therefore consolidated to one material/mesh, while retaining all original
bones, instead of modifying shared textures. Collections are disabled as Source Tools export
groups; the normal MU exporter joins only the intended export objects.

## Full bmdconv comparison

```text
meshes 5 vs 1, bones 5 vs 5, actions 1 vs 1, triangles 216 vs 784
unmatched triangles: 216  max corner distance: 0.0000  max bone distance: 0.0000  differing bone names: 0
note: mesh count differs (meshes sharing a texture are merged by the SMD path)
DIFFERENT
```

`DIFFERENT` is the expected full-model result for remodeled geometry; it is not claimed as a
geometry round trip. The zero corner-distance figure is not a bounds or mesh-similarity test
when the original triangles are unmatched.

To isolate the required rig check, `validate_export.py` extracts the original and replacement
BMDs with bmdconv, retains their respective node and bind-pose blocks plus actual action
SMDs, and reconstructs zero-mesh comparison BMDs. Both manifests must contain exactly one
one-frame action with lock=0. `bmdconv compare` then reports:

```text
meshes 0 vs 0, bones 5 vs 5, actions 1 vs 1, triangles 0 vs 0
unmatched triangles: 0  max corner distance: 0.0000  max bone distance: 0.0000  differing bone names: 0
EQUIVALENT
```

Reports and the actual comparison artifacts are under `validation/`. `bmdconv validate`
passes the replacement reference SMD (784 triangles, one material, five bones) and action.
The empty-triangle-block warning for the isolated rig models is expected and does not
apply to the delivered Beer01 game model. Blender validation also checks non-degenerate
triangles, one UV set, one material, valid atlas coordinates and exactly one full-weight
bone assignment per vertex.

## Visual review and client follow-up

`review/comparison.png` is a 1920×1080 **offline Blender sheet**, with matching camera and
lighting, before/after, wireframe and a reduced-scale study. Individual renders are retained.
The diffuse material has no specular contribution in the preview. Blender illumination
does not duplicate the client renderer, so in-game readability remains unverified.

The unchanged World1 object placement has one Beer01 instance: tile `(127.386, 128.381)`,
world position `(12738.604, 12838.121, 260.000)`, z rotation 180°, scale 1.
Capture it with the baseline assets and again with this replacement once the client is
stable. The user's approved offline continuation leaves that check and screenshots pending.
