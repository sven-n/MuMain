# Asset pipeline

How the client's game assets are organised, what the file formats accept, and the tools in
this repository for getting assets out of the game and back in. This is the reference for
anyone (human or AI) who wants to replace models, textures, maps or tables. The plan for
regenerating the shipped assets lives in
[`docs/agents/ASSET_REGENERATION_PLAN.md`](agents/ASSET_REGENERATION_PLAN.md).

## 1. Where assets live

Everything ships in `src/bin/Data/` and is copied next to the executable at build time
(`Main.app/Contents/MacOS/Data` on macOS, `<build>/src/<Config>/Data` elsewhere). Edit the
files under `src/bin/Data/`; the next build copies them.

| Folder | Content |
|--------|---------|
| `Player/` | The character skeleton with all animations (`Player.bmd`, no mesh) and the body parts per class: `ArmorClass01.bmd`, `HelmClass01.bmd`, `PantClass01.bmd`, `GloveClass01.bmd`, `BootClass01.bmd`, ... plus their textures. |
| `Monster/`, `NPC/` | One `.bmd` per creature (mesh, skeleton and animations in one file) plus textures. |
| `Item/` | Weapons, shields, wings, jewels: `Sword01.bmd` ... with textures. Items render as 3D models in the inventory, so no icon sprites are needed. |
| `Skill/`, `Effect/` | Skill and particle effect models and textures. |
| `Object<N>/` | Static world objects of world `N`: `Object01.bmd` ... (`N` = world enum + 1, so Lorencia is `Object1`). |
| `World<N>/` | Terrain of world `N`: tile mapping, walk attributes, object placement, height map, light map, tile textures, minimap. See [`src/MuEditor/UI/MapEditor/MAP_EDITOR.md`](../src/MuEditor/UI/MapEditor/MAP_EDITOR.md) for the exact file formats. |
| `Interface/` | UI sprites, cursors, fonts as `.OZT`/`.OZJ`. Pixel layouts are hard-coded in the UI code, so keep dimensions when repainting. |
| `Local/` | Game data tables (`Item.bmd`, `Skill.bmd`, `Quest.bmd`, ...). Despite the extension these are not models; the in-game Item and Skill editors (editor build) write them. |
| `Sound/`, `Music/` | Not in the repository. The client expects `Data\Sound\*.wav` and `data\music\*.mp3` and logs load errors until they are supplied. |

## 2. File formats and what they accept

### Models (`.bmd`)

A BMD holds meshes, a skeleton and animations:

- **Meshes**: one texture per mesh, up to 50 meshes per model, up to 10000 unique vertices
  (positions, normals or UVs) per mesh when imported through SMD, one UV set, triangles only.
- **Skeleton**: up to 200 bones, parent-first order, names up to 31 characters. Bones whose
  name starts with `Du` are *dummy* bones: the game drops their keyframes and treats them as
  identity, so never bind vertices to them. Every vertex is bound to exactly one bone (no
  weights).
- **Animations**: a list of actions, each a list of keyframes with a position and an Euler
  rotation (radians) per bone. The game addresses actions **by index**, so the count and order
  of a character's or monster's actions must stay as they are. Actions with locked root motion
  (`lock=1` in the manifest, see below) keep the model in place while it walks.
- **Render flags** come from the texture name: an underscore followed by letters from `RHSN`
  (up to four) sets bright (`R`), hidden mesh (`H`), scrolling texture (`S`) or no-blend (`N`),
  for example `body_R.jpg`. Avoid `_` followed by those letters unless you want the flag.
- **Versions**: `0xA` plain and `0xC` encrypted are readable; the tools always write `0xC`.
  Files in `Data/Local` share the extension but are tables, not models.

Shipped models for scale: the largest has 4591 triangles (`NPC/gambler_moss.bmd`), the
largest skeleton 183 bones (`Monster/Monster215.bmd`), the longest animation 105 frames
(`NPC/barnert.bmd`), and `Player.bmd` has 284 actions. Averages are around 200 to 1000
triangles per model.

### Textures (`.OZJ`, `.OZT`, `.OZB`)

Thin wrappers around ordinary images; the game code refers to `.jpg`/`.tga` names and swaps
the extension:

| Container | Payload | Rules |
|-----------|---------|-------|
| `.OZJ` | JPEG, 24-byte prefix | RGB (no CMYK), used for opaque textures. |
| `.OZT` | TGA, 4-byte prefix | 32-bit uncompressed true color, bottom-left origin, no RLE; used when alpha is needed. |
| `.OZB` | BMP, 4-byte prefix | 8-bit 256x256 height maps (`TerrainHeight.OZB`). |

Dimensions must be powers of two and at most 1024x1024; the loader pads other sizes and the
UVs no longer cover the image.

### Terrain, walk attributes and object placement

Per world folder: `EncTerrain<N>.map` (tile texture indices, two layers plus alpha),
`EncTerrain<N>.att` (walkability, one byte per tile), `EncTerrain<N>.obj` (placed objects with
position, rotation, scale), `TerrainHeight.OZB`, `TerrainLight.OZJ`, `Tile*.OZJ` (30 tile
slots) and `mini_map.OZT`. The `.map`/`.att`/`.obj` files are encrypted with the map cipher
(`MapFileEncrypt` in `src/source/Render/Terrain/ZzzLodTerrain.h`); the in-game Map Editor
reads and writes them. The server keeps its own copy of the walk map: after editing a client
`.att`, upload the server variant through the OpenMU admin panel as described in the Map
Editor guide.

## 3. Tools

### `bmdconv` (models)

Built with the project (`MU_BUILD_ASSET_TOOLS`, on by default); on macOS the binary is
`out/build/macos-arm64/tools/bmdconv/Release/bmdconv`. It uses the engine's own SMD parser
and BMD writer, so what it produces is exactly what the game reads.

```bash
bmdconv info Data/Monster/Monster01.bmd            # meshes, textures, bones, actions, bounds
bmdconv bmd2smd Data/Item/Sword01.bmd work/Sword01 # -> Sword01.smd, Sword01_a00.smd, Sword01.actions.txt
bmdconv bmd2smd-dir Data/Monster work/monsters     # every model, one folder each
bmdconv validate work/Sword01/Sword01.smd          # check an SMD against the engine limits
bmdconv smd2bmd work/Sword01/Sword01.smd out/Sword01.bmd --manifest work/Sword01/Sword01.actions.txt
bmdconv smd2bmd mesh.smd out/Model.bmd --anim a00_stand.smd --anim a01_walk.smd:lock
bmdconv compare Data/Item/Sword01.bmd out/Sword01.bmd   # geometry + bone motion difference report
```

SMD is the Valve text format that [Blender Source Tools](https://github.com/Artfunkel/BlenderSourceTools)
imports and exports. The `.actions.txt` manifest records the action order and lock flags so a
re-import keeps them. Texture names in the SMD material lines must start with a letter and use
only letters, digits, `.` and `_`.

### `tools/mu_texture.py` (textures)

```bash
python3 tools/mu_texture.py unwrap Data/Item/Axes02.OZJ           # -> Axes02.jpg next to it
python3 tools/mu_texture.py unwrap-dir Data work/textures         # every .OZJ/.OZT/.OZB, same tree
python3 tools/mu_texture.py wrap work/textures/Item/Axes02.jpg --out Data/Item/Axes02.OZJ
python3 tools/mu_texture.py check Data/Item/*.ozt                 # validate against the loader rules
```

### Blender scripts (`tools/blender/`)

Require Blender 4.1+ with Blender Source Tools enabled (headless install:
`blender -b --python-expr "import bpy; bpy.ops.preferences.addon_install(filepath='blender_source_tools.zip'); bpy.ops.preferences.addon_enable(module='io_scene_valvesource'); bpy.ops.wm.save_userpref()"`).

```bash
# BMD -> .blend with mesh, armature, one action per animation, textures wired and packed
blender -b --python tools/blender/mu_bmd_import.py -- --bmd Data/Monster/Monster01.bmd --out work/Monster01.blend

# .blend -> BMD (joins meshes, one bone per vertex, actions ordered by mu_index / leading number)
blender -b work/Monster01.blend --python tools/blender/mu_bmd_export.py -- --out out/Monster01.bmd
```

The import stores two things on the armature that the export needs to reproduce a file the
game accepts: `mu_bone_order` (the original bone order; Blender re-sorts bones depth-first, and
the game addresses bones by index) and `mu_action_meta` (game action index and lock flag per
animation clip; Blender 4.4+ keeps the clips as slots of one action). New bones or clips are
appended after the recorded ones. For a model built from scratch, name the clips with a
leading number (`00_stand`, `01_walk`) to fix their order, and pass `--lock` for clips whose
root motion must stay in place.

## 4. Typical workflows

**Inspect** what a model contains: `bmdconv info`. Use the bounds to match scale (one terrain
tile is 100 units; a standing character is about 190 units tall with the feet at z = 0).

**Replace a texture**: unwrap it, repaint at the same dimensions (or a larger power of two
for world textures), `mu_texture.py check`, wrap it back under the same name.

**Edit or replace a model**: import into Blender, change geometry and materials, keep the
armature untouched (same bones, same order, same names), export, then `bmdconv compare`
against the original to confirm the bone motion is unchanged and inspect the triangle report.
Copy the result over the original in `src/bin/Data/...`, rebuild (or copy into the runtime
`Data/` folder) and run the client.

**Add a new item**: needs a model in `Data/Item`, an entry in `Data/Local/Item.bmd` (Item
Editor in the editor build) and the matching item definition on the OpenMU server. Adding
maps needs a new world enum, terrain files, and the server's map definition. Both are later
phases in the regeneration plan.

## 5. Limits that reject a model

The validator in `bmdconv` reports these before the engine parser sees the file:

- more than 200 bones, 50 materials (meshes) or 15000 triangles per model;
- more than 10000 unique positions, normals or UVs per material;
- more than 256 keyframes in one action (`TIME_MAX` in `src/source/Render/Models/SMD.h`);
- bone or texture names longer than 31 characters, texture names that are not engine tokens;
- numbers in scientific notation, frames that skip a bone, non-sequential frame numbers.
