# World1 inventory — 2026-09-22

Baseline: `main` at `9a8b2027`; branch `art/world1-pilot`. No game assets changed during inventory.

Unwrapped with `python3 tools/mu_texture.py unwrap-dir src/bin/Data/World1 assets-work/World1/original`: **32 textures, 0 failures**.

The brief says 14 base tiles but explicitly names **17**. The client loads 14 ground slots; the additional three named legacy files are included below. All 17 names remain in the requested repaint scope.

Usage: `MapManager.cpp` loader, `ZzzLodTerrain.cpp` rendering, visual inspection and read-only decoding of `EncTerrain1.map`. Stored layer counts are not visible coverage: alpha, height and occlusion also matter.

| Original file | Dimensions | Payload | Use / constraint |
|---|---:|---|---|
| `TerrainHeight.OZB` | 256×256 | `.bmp` | 8-bit terrain heights. DATA: preserve byte-for-byte. |
| `TerrainLight.OZJ` | 256×256 | `.jpg` | Baked RGB lighting across the map. DATA: preserve byte-for-byte; never repaint. |
| `TileGrass01.OZJ` | 256×256 | `.jpg` | Terrain slot 0; olive grass/moss. Cells: layer 1=53106, layer 2=909. |
| `TileGrass01.OZT` | 256×64 | `.tga` | Alpha grass strip, BITMAP_MAPGRASS+0, terrain slot 0. Preserve 256×64 dimensions. |
| `TileGrass02.OZJ` | 128×128 | `.jpg` | Terrain slot 1; pale gritty earth. Cells: layer 1=2155, layer 2=11184. |
| `TileGrass02.OZT` | 256×64 | `.tga` | Alpha grass strip, BITMAP_MAPGRASS+1, terrain slot 1. Preserve 256×64 dimensions. |
| `TileGrass03.OZT` | 256×128 | `.tga` | Alpha tall dry grass, BITMAP_MAPGRASS+2, terrain slot 2. Preserve 256×128 dimensions. |
| `TileGround01.OZJ` | 256×256 | `.jpg` | Terrain slot 2; irregular town cobbles. Cells: layer 1=65, layer 2=1969. |
| `TileGround01.OZT` | 256×64 | `.tga` | Legacy alpha strip; no current terrain-loader reference. Preserve dimensions. |
| `TileGround01x.OZJ` | 128×128 | `.jpg` | Legacy striated stone; not loaded by current terrain loader. Same bytes as TileMgrass.OZJ. |
| `TileGround02.OZJ` | 128×128 | `.jpg` | Terrain slot 3; worn rectangular slabs. Cells: layer 1=1866, layer 2=387. |
| `TileGround03.OZJ` | 128×128 | `.jpg` | Terrain slot 4; carved gothic paving. Cells: layer 1=194, layer 2=0. |
| `TileMgrass.OZJ` | 128×128 | `.jpg` | Legacy striated stone despite name; not loaded by current terrain loader. |
| `TileMstone.OZJ` | 128×128 | `.jpg` | Legacy bright green moss/grass despite name; not loaded by current terrain loader. |
| `TileRock01.OZJ` | 256×256 | `.jpg` | Terrain slot 7; pale rough rock/gravel. Cells: layer 1=59, layer 2=3673. |
| `TileRock02.OZJ` | 128×128 | `.jpg` | Terrain slot 8; mossy earth/rock. Cells: layer 1=98, layer 2=3340. |
| `TileRock03.OZJ` | 256×256 | `.jpg` | Terrain slot 9; cold grey rock blocks. Cells: layer 1=0, layer 2=0. |
| `TileRock04.OZJ` | 256×256 | `.jpg` | Terrain slot 10; dark granular rock. Cells: layer 1=0, layer 2=0. |
| `TileRock05.OZJ` | 256×256 | `.jpg` | Terrain slot 11; sandstone masonry. Cells: layer 1=0, layer 2=0. |
| `TileRock06.OZJ` | 128×128 | `.jpg` | Terrain slot 12; black marble with gold corners. Cells: layer 1=0, layer 2=0. |
| `TileRock07.OZJ` | 256×256 | `.jpg` | Terrain slot 13; dark rock with inverted legacy logo. Cells: layer 1=0, layer 2=0. |
| `TileWater01.OZJ` | 256×256 | `.jpg` | Terrain slot 5; blue-green water. Cells: layer 1=7939, layer 2=289. |
| `TileWood01.OZJ` | 128×128 | `.jpg` | Terrain slot 6; weathered wood decking. Cells: layer 1=54, layer 2=0. |
| `angeflo_R.OZJ` | 64×64 | `.jpg` | Legacy glow/particle; no matching filename reference found in source. _R is the bright flag. Preserve. |
| `leaf01.OZJ` | 16×16 | `.jpg` | Legacy opaque particle variant; Lorencia loads leaf01.tga. |
| `leaf01.OZT` | 16×16 | `.tga` | Lorencia leaf particle, BITMAP_LEAF1. |
| `leaf02.OZJ` | 16×16 | `.jpg` | Particle texture, BITMAP_LEAF2. |
| `map1.OZJ` | 512×512 | `.jpg` | Legacy map overview; no matching filename reference found in source. Preserve. |
| `mini_map.OZT` | 1024×1024 | `.tga` | World minimap image used by minimap UI. Preserve. |
| `rain01.OZT` | 4×32 | `.tga` | Rain streak, BITMAP_RAIN; shared with other worlds via World1 path. Preserve. |
| `rain011.OZT` | 4×32 | `.tga` | Alternate rain streak loaded by Crywolf. Preserve. |
| `rain02.OZT` | 32×16 | `.tga` | Rain splash ring, BITMAP_RAIN_CIRCLE; shared via World1 path. Preserve. |

## Loose image duplicates

`TerrainHeight.bmp` (256×256), `TileGrass01.tga` (256×64), `rain01.tga` (4×32), and `rain02.tga` (32×16) also exist in the source folder. Height and rain images are byte-identical to their unwrapped payloads; the loose TileGrass01.tga differs from the OZT payload. The client reads the OZT container. Preserve all four loose images. Remaining files are terrain mapping/placement/walk data, BMD minimap models or legacy metadata.

The grass-strip renderer derives world height from image pixel height; keeping dimensions is necessary for this pilot.

## Reference status

`reference/original-texture-contact-sheet.jpg` shows all 32 unwrapped textures. It is an inventory sheet, not a client screenshot.

The three required 1920×1080 client reference captures remain pending because the unchanged client repeatedly crashed during setup. See `validation/client-baseline-crashes.md`.

## Beer01 identity

Import and render confirm a tavern still life (bottle, mug, bowls and food), not a barrel. The user confirmed rebuilding this still life. See `Beer01/original/`: 5 meshes, 216 triangles, 5 bones, one action with one keyframe.
