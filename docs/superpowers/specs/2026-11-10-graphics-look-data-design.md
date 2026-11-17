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

- **GL1 (done, #1113):** the format: the light mood's values move into
  `Data/Render/Look.json` as `mood.*` values, on every layer; map entries
  list their maps; the writer is canonical; version 1 files are upgraded when
  read; nothing looks different. The Dev Editor's Look panel and the `$look`
  commands keep working on the new format.
- **GL2 (done, #1126):** the Graphics Look tool window, replacing the Dev Editor's
  Look panel: scopes (default, environment, map), the maps an environment is
  used by, Undo/Redo, Data Checks, and edits only through the look's edit
  operations. Environments are added, renamed, and removed while no map uses
  them; which maps share an entry stays as the file has it until the maps
  area's map groups.
- **GL2.1 (done, #1127):** after a design review of GL1 and GL2: the Player and
  Authoring previews instead of per-value developer marks (D-GL12), a
  slider's value shown live while it is dragged (D-GL13), and the decisions
  the review asked to record before zones and time of day (D-GL4, D-GL14,
  D-GL15).
- **Later, with the stages of #990 that need them:** the time-of-day and zone
  layers, effect lights on the effect types
  (`dynamicLight`, area FX1), materials per mesh on model entries (items 13,
  MN), the light-map split results and verdict per map, a Lighting tab in
  the Map Editor (after MP2+). Colour grading and materials are D1's
  ([linear lighting design](2026-11-16-linear-lighting-design.md)): grading
  values on the look layers, `roughness` and `metallic` on item models.

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
| D-GL4 | **Layers:** built-in → default → environment → map, for look and mood values alike: the baseline an author edits. Zones, time of day and weather are not more first-value-wins layers after the map: a runtime evaluator (#990) will blend states over the baseline with weights, from a context (position, time) kept outside the file, and will keep values for the whole view (exposure, grading) apart from light that depends on where a model stands. A player's `[EnhancedTuning]` value replaces the result, as today, except in the Authoring preview (D-GL12). |
| D-GL5 | **Rules between values are checked on the result:** `mood.figures` is never below `mood.base` on the default, on each environment over the default, and on each map's values. |
| D-GL6 | **The canonical writer:** `formatVersion`, `default`, `environments`, `maps`, in that order. Values in the order of the value table (look values, then mood values). Environments sorted by name; map entries by their first map, their keys in the order `maps`, `name`, `environment`, then the values. The layout stays: the default one value per line, each environment and each map entry on one line. A test saves the shipped file unchanged and compares the text. |
| D-GL7 | **Version 1 is upgraded when read:** a version 1 `Look.json` is read with the `LightMood.json` beside it, also version 1, as today (a `mood.*` value in a version 1 `Look.json` is an error): each mood entry becomes the `mood.*` values of its map's entry, every value it named kept, also one equal to the default (Q-GL3). Without a valid `LightMood.json`, the version 1 file applies with the Classic mood and cannot be saved (Q-GL4). Save writes version 2 into `Look.json` only. With a version 2 `Look.json`, a `LightMood.json` beside it is not read, and Data Checks warns that it can be deleted. The same applies to the candidate folders of `$look load`; a version 1 candidate without its own `LightMood.json` shows the shipped moods and cannot be saved. |
| D-GL8 | **Nothing looks different:** the shipped version 2 file is converted from the version 1 pair, and a test resolves every map number from 0 to 255 in both and compares every look and mood value. |
| D-GL9 | **Players keep their controls:** `[EnhancedTuning]` reads the look values a player may change and `mood.strengthPercent`, as today. The `mood.*` values are not player values: a `mood.base` line is ignored with a warning, as an unknown key is. |
| D-GL10 | **The Dev Editor's Look panel and `$look` keep their scopes** (default, current map) until GL2. A change on the current map changes its entry; when the entry lists several maps, the panel names them, and the command's reply too. A map without an entry gets one with only that map. GL2 replaces the panel with the Graphics Look window; `$look` keeps these scopes. |
| D-GL12 | **Previews, not marks:** the effective values are the Player preview (the profile with the player's `[EnhancedTuning]` values and light mood strength) or the Authoring preview (the profile alone). The first applied edit (`$look set`, `$look reset`, the Graphics Look window, its Undo) switches Player to Authoring and logs it; nothing switches back to Player on its own (not a save, a reset or a load), only `$look preview player` or the window. It replaces GL1's per-value developer marks, which were worked out from what differs from the loaded file and so changed with a save or an edit that matched the file. |
| D-GL13 | **A drag shows live:** the window applies a slider's value whenever the drag changes it and records one undo step when it ends; a drag ended without a change, or whose slider went, puts the profile back. `sun.resolution` (a new shadow map) applies when the drag ends, and the row says so; a value the light mood reads rebuilds the static map light while dragged (a few ms, as Options' Mood strength). |
| D-GL14 | **Identities:** `maps` holds loaded-world numbers (D-GL3); looking up by server map or stage is a separate, reviewed change. An environment's name is its identifier inside the file (renames rewrite its maps' references); before another file refers to an environment, the area decides between a stable identifier apart from the display name and declaring names unchangeable. |
| D-GL15 | **The format keeps evolving** through `formatVersion` and the version upgrade (D-GL7). Values stay one table of named numbers until blending needs more: before zones or time of day blend values, each value gets its unit, its kind and its blend rule (`sun.resolution` is not interpolated, angles blend as angles, a 0 that means "no cap" is not blended); colours, curves and texture references come as typed fields when the first of them arrives. Which values players may set follows the kinds in `docs/enhanced-look.md` ("What the player's controls are"). |
| D-GL16 | **Players set only Quality and Preference values** (the kinds in `docs/enhanced-look.md`): step GL2.2 takes the art-direction sliders out of the Options window, together with #990 Stage 7's style choices (warmth, contrast, night darkness), so players trade them for those instead of only losing settings. Their `[EnhancedTuning]` keys are then ignored with a warning, as an unknown key is (D-GL9), and dropped at the next save, so an old reshaping of the light doesn't live on under PBR and time of day. Until then they stay, and no new value becomes a player control in the meantime (D-GL15). |
| D-GL11 | **Docs:** the file's format, layers and rules are in `docs/enhanced-look.md` ("The Enhanced values"); the mood values' meaning stays in `docs/light-mood.md`; which map has which environment stays in `docs/enhanced-look-map-environments.md`. |

## Steps

| Step | Name | Depends on | Summary |
|---|---|---|---|
| GL1 | One look file | – | D-GL1 to D-GL11, in one PR: reader and writer, the version 1 upgrade, the converted shipped file, the tests (unchanged save, every map alike), Data Checks, docs. |
| GL2 | Graphics Look tool window | GL1 | The editor of the roadmap's shared editor parts: scopes default, environment and map; for an environment, the maps that use it and a warning before a change many maps share; choosing a map's environment; Undo/Redo; problems in Data Checks; changes only through the look's edit operations, which check every change (as `ItemEdits` for items). Replaces the Dev Editor's Look panel. GL1's new possibilities (mood values on environments, shared map entries) are editable only in a file until then; the area is not done before GL2. |
| GL2.1 | Previews and live drag | GL2 | D-GL12 to D-GL15 after the design review: the Player and Authoring previews (`$look preview`, the window's Preview), the live drag, value headings grouped by prefix, and the docs. |
| GL2.2 | Player controls by kind | GL2.1, #990 Stage 7's style choices | D-GL16, with the style choices: the Options window offers only Quality and Preference values; the art-direction sliders and their help go, their keys are ignored with a warning and dropped at the next save; the docs' table becomes the rule the control list is checked against. |
| GL3+ | With #990 | GL1 | Not designed: time of day and zones as layers (zones after MP2+), the light-map split results, and the time-of-day scrubber in the tool window. |

## Agreed questions

| # | Question | Answer |
|---|---|---|
| Q-GL1 | The light mood in `Look.json`, or its own file with environments that name `Look.json`'s? *(2026-11-10)* | One file. The data rules allow names across files; what decides is that every later layer (zone, time of day) is then built once in the reader, the writer and the editor, not twice, and that the editor's undo and save work on one document. Players are not affected: the Options rows are an explicit list. |
| Q-GL2 | Environments listing their maps, or maps naming their environment? *(2026-11-10)* | Maps name their environment: users refer to a shared definition by name and the editor shows its users (roadmap D26). A map's environment is then one field, and a map cannot be in two environments. `maps` lists only for the variants of one map; maps alike in kind share an environment. |
| Q-GL3 | A version 1 import: keep a map's mood value that equals the default, or drop it? *(2026-11-13)* | Keep it: the map named it, so a later change of the default must leave it. Deduplicating the shipped file is a separate, reviewed conversion. |
| Q-GL4 | A version 1 `Look.json` whose `LightMood.json` is missing or invalid: save it? *(2026-11-13)* | No. It applies with the Classic mood, but a save would make that mood the file's and the old file would never be read again. The loader cannot tell a file that never existed from one that was lost, so both block the save. |
