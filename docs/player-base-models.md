# Player models

**S6 player models** in Settings → Interface chooses the player's unequipped body models:

- **On / checked:** original Season 6 models (default).
- **Off / unchecked:** improved models.

The choice is saved in `config.ini` as `[Render] UseS6PlayerModels=1` or `0`.
**Restart the game to apply a change.** Changing the checkbox does not replace models in an
already running game. An explicit saved value takes precedence over the default.

## Included assets

Both sets ship together. The original 65 files remain unchanged in `Data/Player/`.
The improved 65 files are in `Data/Player/Updated/`, with their 14 textures in `Data/Player/`.

With the improved models chosen, the game tries each body part in `Data/Player/Updated/`.
A missing file or one rejected by the model loader's header check uses its original from
`Data/Player/` instead. If an improved part's texture cannot be loaded through the normal
Player texture lookup (including the active texture pack and already cached textures), that
part also uses its original. The failed texture is logged once per affected part, without a
missing-texture dialog for the optional improved part. The original assets must still be complete.
This does not make malformed BMD files safe: the legacy parser does not validate every read.

The improved set is the files the folder holds. Adding a file with the name of an original body
part (for example `HelmClass06.bmd` for the Summoner) adds it without a code change. Its textures
belong in `Data/Player/` (or the corresponding texture-pack folder).

Each selected family contains `HelmClass`, `ArmorClass`, `PantClass`, `GloveClass` and
`BootClass` BMDs with the following suffixes:

| Class | Selected forms / suffixes |
|---|---|
| Dark Wizard | Dark Wizard `01`, Soul Master `201`, Grand Master `301` |
| Dark Knight | Dark Knight `02`, Blade Knight `202`, Blade Master `302` |
| Elf | Fairy Elf `03`, Muse Elf `203`, High Elf `303` |
| Magic Gladiator | Magic Gladiator `04`, Duel Master `304` |
| Dark Lord | Dark Lord `05`, Lord Emperor `305` |

Textures are `skin_hq_class01`–`05`, `skin_hq_class201`–`203`,
`body_hq_class301`–`305` and shared `hairhq_R`, all with `.OZJ` extensions.
The `hq` names are internal compatibility aliases, not a promise of HD texture resolution.
As for every model texture, the part of a name after its first `_` carries drawing flags:
`hairhq_R` keeps the `R` (glowing) flag of the original `hair_R`, so the Dark Lord's hair glows
in both sets.

When updating a texture pack that overrides the previous `Player/hair_hq_player.OZJ`, rename
or copy that override to `Player/hairhq_R.OZJ` inside the pack. The updated Dark Lord and Lord
Emperor heads no longer request the old name. Update the two improved head BMDs and the base
hair texture together; an incremental install may retain the old base alias, but the updated
heads do not use it.

Texture names also determine mesh roles: a lower-case `ski` prefix, or `level` in any case,
marks skin; lower-case `hair` marks hair. Preserve those roles and the original mesh order.
A differently named skin texture may load, but will not be hidden with skin and will receive
specular lighting unlike skin-marked meshes.

This is a body/face mesh and appearance improvement, not an armor-set upgrade. Equipped armor,
masks, animations, class IDs and attachment rules are unchanged. The shipped Summoner/Rage Fighter
bodies retain their original assets. Fully armored characters still use their existing armor art.
The setting is independent of Classic/Enhanced graphics and texture packs.

Player-shaped monsters and NPCs (guards, transformations, the Doppelganger) use the same body
models, so they follow the setting too, in the game and in the editor's Monster Preview. Compare
monster setups before and after a change with the same setting on both runs.

The editor's Item Model file picker does not list `Data/Player/Updated/`: those files are bodies,
not item models, and have the same names as the originals.

## Attribution

Original and improved models and textures belong to **Webzen**. Attribution does not transfer
ownership or grant redistribution rights. Anyone distributing the packaged assets must have the
necessary permission.
