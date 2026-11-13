# Graphics Look Data (WIP)

> **Working document.** The graphics look area of the
> [data-driven content roadmap](2026-09-29-data-driven-content-roadmap-design.md)
> (area GL): the data of the Enhanced look's lighting and image values, and its
> editor. It is decision D7 of the lighting plan
> ([#990](https://github.com/sven-n/MuMain/issues/990)), which keeps the order of
> the lighting work itself. Based on MuMain `main` @ `f10bb9313`.

## Goal

The Enhanced look's tuning values are already data, but in two files with two
layerings, keyed one entry per map, and saved in a layout that changes on a
save without changes. Before #990 adds more layers (time of day, zones), the
files get one format that follows the data rules of
[`CODING_RULES.md`](../../CODING_RULES.md#design-principle-data-driven-by-design--data-and-editor):
one layering, named environments that maps refer to, shared entries only for
the variants of one map, and a writer that writes unchanged data as it was.

## Scope

- **GL1 (now):** the format: the light mood's values move into
  `Data/Render/Look.json` as `mood.*` values, on every layer; map entries
  list their maps; the writer is canonical; version 1 files are upgraded when
  read; nothing looks different. The Dev Editor's Look panel and the `$look`
  commands keep working on the new format.
- **GL2:** the Graphics Look tool window, replacing the Dev Editor's Look
  panel: scopes (default, environment, map), the maps an environment is used
  by, Undo/Redo, Data Checks, and edits only through the look's edit
  operations.
- **Later, with the stages of #990 that need them:** the time-of-day and zone
  layers, named grading tables, effect lights on the effect types
  (`dynamicLight`, area FX1), materials per mesh on model entries (items 13,
  MN), the light-map split results and verdict per map, a Lighting tab in
  the Map Editor (after MP2+).

## Current state

- **`Data/Render/Look.json`** (format 1): `default`, `environments` (named
  objects of look values) and `maps`, one entry per map number (`"world"`)
  with an optional `"environment"` name and values. 67 entries, almost all
  only naming an environment. Blood Castle 1–8 are eight entries alike.
  Layers: built-in → default → environment → map.
- **`Data/Maps/LightMood.json`** (format 1): `default` (five values required)
  and `maps`, one entry per map number. No environments, so 28 of the 29
  enclosed maps repeat `base 0.45, shadowKeep 0, sun 1.0`, and all 16 fixed
  maps with a mood entry repeat `sun 1.0`.
- **One tuning surface already:** `Render::Look::LookFiles` edits both
  documents by name (`mood.<key>` for the mood's), validates each edit with
  the files' readers, and saves both together through temporaries and
  backups, with a "files are mixed" state when a restore fails. Data Checks
  has one area for both, "Graphics look".
- **The writer keeps the order it read and writes `Look.json`'s layout for
  both files,** so saving an unchanged `LightMood.json` gives a different
  file. No test checks that an unchanged save gives the same file.
- **Players** tune through `[EnhancedTuning]` in `config.ini`: look value
  names and `mood.strengthPercent`. The Options rows are an explicit list of
  controls (`TuningControlId`), not one per value.

## Decisions

| # | Decision |
|---|---|
| D-GL1 | **One file** (Q-GL1): the light mood's values are look values named `mood.<key>` (`mood.base`, `mood.figures`, ...) in `Data/Render/Look.json`, on every layer. Their built-in values are Classic's (1, `shadowKeep` and `shadeCool` 0); the default no longer has to name any of them. `Data/Maps/LightMood.json` is no longer shipped. |
| D-GL2 | **Maps refer to their environment by name** (Q-GL2): a map entry has `"environment"`; an environment does not list its maps. The editor shows the maps that use an environment (roadmap D26). |
| D-GL3 | **A map entry lists its maps** (Q-GL2): `"maps": [11, 12, 13, 14, 15, 16, 17, 52]`, for the variants of one map (Blood Castle 1–8, Kalima 1–6 and Lost Kalima, Devil Square, Chaos Castle, ...), as `ObjectSounds.json` shares one entry for Kalima. Maps that are only alike in kind share an environment, not an entry. A map is in at most one entry. The numbers are map numbers as the server sends them (today's `"world"`), without stages: lighting is chosen per map. GL1 keeps today's lookup by the loaded world (`gMapManager.WorldActive`), which differs only for Devil Square 5–7 (server map 32, loaded as world 9), so its entry 32 is not read; looking up by the server's map (`GetServerMap()`) is a separate change. Which maps are variants of one map is not checked until the maps area has map groups (MP); the reader checks valid numbers in one entry each. |
| D-GL4 | **Layers:** built-in → default → environment → map, for look and mood values alike. #990 adds zone and time of day after the map. A player's `[EnhancedTuning]` values apply on top, as today. |
| D-GL5 | **Rules between values are checked on the result:** `mood.figures` is never below `mood.base` on the default, on each environment over the default, and on each map's values. |
| D-GL6 | **The canonical writer:** `formatVersion`, `default`, `environments`, `maps`, in that order. Values in the order of the value table (look values, then mood values). Environments sorted by name; map entries by their first map, their keys in the order `maps`, `name`, `environment`, then the values. The layout stays: the default one value per line, each environment and each map entry on one line. A test saves the shipped file unchanged and compares the text. |
| D-GL7 | **Version 1 is upgraded when read:** a version 1 `Look.json` is read with the `LightMood.json` beside it, also version 1, as today (a `mood.*` value in a version 1 `Look.json` is an error): each mood entry becomes the `mood.*` values of its map's entry, every value it named kept, also one equal to the default (Q-GL3). Without a valid `LightMood.json`, the version 1 file applies with the Classic mood and cannot be saved (Q-GL4). Save writes version 2 into `Look.json` only. With a version 2 `Look.json`, a `LightMood.json` beside it is not read, and Data Checks warns that it can be deleted. The same applies to the candidate folders of `$look load`; a version 1 candidate without its own `LightMood.json` shows the shipped moods and cannot be saved. |
| D-GL8 | **Nothing looks different:** the shipped version 2 file is converted from the version 1 pair, and a test resolves every map number from 0 to 255 in both and compares every look and mood value. |
| D-GL9 | **Players keep their controls:** `[EnhancedTuning]` reads the look values a player may change and `mood.strengthPercent`, as today. The `mood.*` values are not player values: a `mood.base` line is ignored with a warning, as an unknown key is. |
| D-GL10 | **The Dev Editor's Look panel and `$look` keep their scopes** (default, current map) until GL2. A change on the current map changes its entry; when the entry lists several maps, the panel names them, and the command's reply too. A map without an entry gets one with only that map. |
| D-GL11 | **Docs:** the file's format, layers and rules are in `docs/enhanced-look.md` ("The Enhanced values"); the mood values' meaning stays in `docs/light-mood.md`; which map has which environment stays in `docs/enhanced-look-map-environments.md`. |

## Steps

| Step | Name | Depends on | Summary |
|---|---|---|---|
| GL1 | One look file | – | D-GL1 to D-GL11, in one PR: reader and writer, the version 1 upgrade, the converted shipped file, the tests (unchanged save, every map alike), Data Checks, docs. |
| GL2 | Graphics Look tool window | GL1 | The editor of the roadmap's shared editor parts: scopes default, environment and map; for an environment, the maps that use it and a warning before a change many maps share; choosing a map's environment; Undo/Redo; problems in Data Checks; changes only through the look's edit operations, which check every change (as `ItemEdits` for items). Replaces the Dev Editor's Look panel. GL1's new possibilities (mood values on environments, shared map entries) are editable only in a file until then; the area is not done before GL2. |
| GL3+ | With #990 | GL1 | Not designed: time of day and zones as layers (zones after MP2+), named grading tables, the light-map split results, and the time-of-day scrubber in the tool window. |

## Agreed questions

| # | Question | Answer |
|---|---|---|
| Q-GL1 | The light mood in `Look.json`, or its own file with environments that name `Look.json`'s? *(2026-11-10)* | One file. The data rules allow names across files; what decides is that every later layer (zone, time of day) is then built once in the reader, the writer and the editor, not twice, and that the editor's undo and save work on one document. Players are not affected: the Options rows are an explicit list. |
| Q-GL2 | Environments listing their maps, or maps naming their environment? *(2026-11-10)* | Maps name their environment: users refer to a shared definition by name and the editor shows its users (roadmap D26). A map's environment is then one field, and a map cannot be in two environments. `maps` lists only for the variants of one map; maps alike in kind share an environment. |
| Q-GL3 | A version 1 import: keep a map's mood value that equals the default, or drop it? *(2026-11-13)* | Keep it: the map named it, so a later change of the default must leave it. Deduplicating the shipped file is a separate, reviewed conversion. |
| Q-GL4 | A version 1 `Look.json` whose `LightMood.json` is missing or invalid: save it? *(2026-11-13)* | No. It applies with the Classic mood, but a save would make that mood the file's and the old file would never be read again. The loader cannot tell a file that never existed from one that was lost, so both block the save. |
