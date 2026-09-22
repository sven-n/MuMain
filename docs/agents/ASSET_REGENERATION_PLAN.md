# Asset regeneration plan

Brief for the AI (or person) that produces new graphics in Blender for this client. It answers
"can we rebuild all the assets?", lists the rules the game imposes, and sets the order of work.
Formats and tools are documented in [`docs/asset-pipeline.md`](../asset-pipeline.md).

## Answer

Yes. Every visual asset can be regenerated and loaded by the unmodified client, as long as the
replacement respects the engine's data model: one texture per mesh, one bone per vertex,
fixed skeletons and action lists for anything the game animates, power-of-two textures up to
1024x1024, and the folder and file names the code looks up. Extending maps and adding items is
possible too, but needs server-side definitions in OpenMU, so it comes after the visual pass.

The engine renders with per-vertex lighting and plain textures. There are no normal maps, PBR
materials or shadow maps, so quality gains come from better meshes (within the budgets below),
better textures (up to 1024x1024) and better animation, not from new material types.

## Ground rules for every asset

1. **Keep names and paths.** The code refers to files by name (`Data/Player/ArmorClass01.bmd`,
   `Data/World1/TileGrass01.OZJ`). Replace the file, never rename it.
2. **Keep skeletons.** Same bone count, order and names as the original (`bmdconv info`). The
   game addresses bones by index (attachment points, head look-at, feet). Body parts of the
   player use the first 56 bones of `Player.bmd`; new bones may only be appended at the end.
   Work on a `.blend` produced by `mu_bmd_import.py`: it records the original order, and
   `mu_bmd_export.py` restores it even though Blender re-sorts bones internally.
3. **Keep action lists.** Same count and order of actions; frame counts may change. Keep the
   `lock` flags from the manifest. A walk cycle should still cover the same distance per loop.
4. **Scale and orientation.** Z is up, one terrain tile is 100 units, a standing character is
   about 190 units tall with feet at z = 0, facing -Y in the bind pose. Match the original's
   `bmdconv info` bounds within a few percent unless the change is deliberate.
5. **Geometry budgets.** Shipped models average 200 to 1000 triangles and top out at 4591.
   Targets for the remake: props and items up to 1500, monsters and NPCs up to 4000, player
   parts up to 1500 each, effects up to 500. Hard limits: 15000 triangles, 50 materials and
   10000 unique vertices per material per model.
6. **Textures.** Powers of two, at most 1024x1024. `.jpg` for opaque surfaces, `.tga` (32-bit
   uncompressed) where alpha is needed. Material name = texture file name. UI sprites keep
   their exact pixel dimensions. Never use `_` followed by `R`, `H`, `S` or `N` in a texture
   name unless the render flag is intended.
7. **One UV set, triangles only, exactly one vertex group per vertex.** The export script
   trims extra weights, but the result is better when the rig is authored that way.
8. **Validate before handing over.** `bmdconv validate`, `bmdconv compare` (bones and actions
   must be `EQUIVALENT` for a re-skin), `mu_texture.py check`, then a client run.

## Phases

| Phase | Scope | Risk | Output |
|-------|-------|------|--------|
| 0 | Groundwork: converter, texture tool, Blender scripts, this plan | done | `tools/`, docs |
| 1 | **Textures**: repaint or upscale every `.OZJ`/`.OZT` at the same or a higher power-of-two size, same names. Start with World1 (Lorencia) tiles and objects, then player parts, then items, monsters, effects, UI last. | low | new textures in `src/bin/Data/**` |
| 2 | **Static world objects** (`Object<N>/*.bmd`): remodel with the same pivot, footprint and height; no skeleton constraints beyond the single root bone. | low | new `.bmd` + textures |
| 3 | **Items** (`Item/*.bmd`): remodel weapons, shields, wings on the original skeleton (usually one or two bones; weapons attach through the model origin). | medium | new `.bmd` + textures |
| 4 | **Characters and monsters**: new meshes skinned to the untouched skeletons; optional re-animation with identical action lists. Player parts must fit each other across classes (same seams). | high | new `.bmd` + textures |
| 5 | **Terrain polish**: height, tile painting, light map and object placement per world with the Map Editor (Windows build) or scripts; server walk map re-uploaded. "Extend a little" means using blocked areas of the existing 256x256 grid. | medium | `World<N>/` files + server `.att` |
| 6 | **New content**: new items (client `Item.bmd` + OpenMU item definitions), new maps (world enum, terrain, server map definition, gates, spawns). | high | client + server changes |

Work inside a phase in the order players see things: Lorencia (World1/Object1) first, the
starting character parts second, then the rest by frequency of use.

## Per-asset handoff format

For every regenerated asset, deliver a folder `assets-work/<Category>/<Name>/` containing:

- `source.blend` (packed textures) or the Blender project used;
- the exported game files ready to copy: `<Name>.bmd`, `<texture>.OZJ`/`.OZT`;
- `notes.md` with: original `bmdconv info` bounds vs new bounds, `bmdconv compare` output,
  texture sizes, anything intentionally changed.

Copy the game files into `src/bin/Data/<Folder>/` (overwriting the original), rebuild or copy
into the runtime `Data/` folder, start the client and check `MuError.log` for load errors.

## Tracking

Keep a table in [`WORKLOG.md`](WORKLOG.md) or a dedicated sheet with one row per asset:
folder/file, phase, status (todo / in progress / exported / verified in client), who, date,
notes. Verified means: loads without errors, looks right in the client, `compare` shows the
skeleton and actions unchanged.

## Known gaps to plan around

- The in-game editors (map, item, skill) build on Windows only for now; on macOS use the
  scripts and the OpenMU admin panel until the three Win32 file dialogs are ported.
- Sound and music are absent from the repository; supply `Data/Sound/*.wav` and
  `Data/Music/*.mp3` before a release.
- `Data/Object74/` is missing most models; do not ship that world until it is complete.
