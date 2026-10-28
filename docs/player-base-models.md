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
Do not omit the Updated folder or its textures when packaging the client.

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
`body_hq_class301`–`305` and shared `hair_hq_player`, all with `.OZJ` extensions.
The `hq` names are internal compatibility aliases, not a promise of HD texture resolution.

This is a body/face mesh and appearance improvement, not an armor-set upgrade. Equipped armor,
Summoner/Rage Fighter bodies, masks, animations, class IDs and attachment rules are unchanged.
Fully armored characters still use their existing armor art. The setting is independent of
Classic/Enhanced graphics and texture packs.

## Validation

Focused `player_model_selector_tests` cover the default and saved choices, exactly 13 selected
body-family paths, unchanged excluded paths and native loading from both packaged model sets.
Existing native body tests cover the retained skeleton, mesh/index/UV bounds, palette skinning
and texture roles. CPU tests skip GPU uploads; they do not replace native appearance checks.

```sh
cmake --build out/build/linux-x64 --config Release --target player_model_selector_tests
ctest --test-dir out/build/linux-x64 -C Release -R '\[player-selector\]' --output-on-failure
```

## Attribution

Original and improved models and textures belong to **Webzen**. Attribution does not transfer
ownership or grant redistribution rights. Anyone distributing the packaged assets must have the
necessary permission.
