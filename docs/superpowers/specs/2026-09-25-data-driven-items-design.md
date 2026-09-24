# Data-Driven Items Design (WIP)

> **Temporary working document.** It is used to refine the plan on the
> `refactor/data-driven-items` branch and will be removed once the work
> has landed.

## Goal

Replace hardcoded item knowledge in the client with **item data plus rule
code**:

- All item data (stats, flags, categories, names, models) lives in data
  files, is loaded into an **in-memory item database** at startup, and rule
  code reads it instead of hardcoded item lists.
- The data model is **equivalent to OpenMU's `ItemDefinition`** where both
  sides overlap, so data can be moved between client and server.
- Every data field can be edited in a proper **MuEditor item editor**.
- Item names are **translatable**; logs always use a stable, language-neutral
  identifier.

Scope: **items only.** Skills, monsters, maps, etc. may follow the same
pattern later but are not part of this work.

## Current state

### Client (MuMain)

| Area | Where | Notes |
|---|---|---|
| Item stats | `Data/Local/<lang>/Item_<lang>.bmd` → `CItemDataHandler` (`src/source/Data/DataHandler/ItemData/`) → `ITEM_ATTRIBUTE[MAX_ITEM]` | Fields defined once as X-macros in `Data/GameData/ItemData/ItemFieldDefs.h`. The file is per language, so stats are duplicated for every language. |
| Item names | `ITEM_ATTRIBUTE::Name` (from the same per-language bmd) | No shared key between languages except the array index. |
| Categories and rules | `GameLogic/Items/ItemCategories`, `TradeRestrictions`, `ShopRestrictions` (from PR sven-n/MuMain#625) | Now one source of truth, but still hardcoded item lists in C++. |
| Models and textures | `OpenItems()` / `OpenItemTextures()` in `Engine/Object/ZzzOpenData.cpp` | Around 600 hardcoded lines mapping `MODEL_*` to `Data\Item\<File><n>.bmd`. |
| Other item data | `ItemAddOption.bmd`, `SocketItem_<lang>.bmd`, `Mix.bmd`, `pet.bmd`, set options (`CSItemOption`) | Out of scope for the first phases; listed so they are not forgotten. |
| Editor | `src/MuEditor/UI/ItemEditor/` (only in `_EDITOR` builds) | Table editor for the bmd fields; save to bmd, export to S6E3 bmd and CSV. |
| UI translations | `.resx` → generated `I18N::*` (`docs/translation-system.md`) | Good for UI strings; item names are not part of it yet. |

### Server (OpenMU)

- `ItemDefinition` (`src/DataModel/Configuration/Items/ItemDefinition.cs`):
  `Group`, `Number`, `Width`, `Height`, `Durability`, `Value`, `DropLevel`,
  `MaximumItemLevel`, `ItemSlot`, `Skill`, `QualifiedCharacters`,
  `Requirements`, `BasePowerUpAttributes`, `PossibleItemOptions`,
  `PossibleItemSetGroups`, `MaximumSockets`, `ConsumeEffect`, and the flags
  `IsAmmunition`, `IsBoundToCharacter`, `IsQuestItem`, `DropsFromMonsters`,
  `StorageLimitPerCharacter`.
- Names are `LocalizedString`: `"English||de=Deutsch||fr=Français"`, English
  (neutral) first.
- Identity: `(Group, Number)`. Initialization creates deterministic GUIDs
  (`GuidHelper`, type id `0x80` for `ItemDefinition`).
- Trade, vault and NPC sell rules only use `IsBoundToCharacter`. The client's
  rules are more detailed (trade, drop, store, personal shop, sell, repair
  bans).
- Data changes ship as `UpdatePlugInBase` plug-ins; the admin panel offers
  backup and snapshot downloads but no config import.

## Target design (proposal)

### 1. In-memory item database

- A client `ItemDefinition` record, keyed by `(group, number)`, with lookups
  by `(group, number)`, raw item type (`group * MAX_ITEM_INDEX + number`) and
  model id.
- `ITEM_ATTRIBUTE` and `ItemAttribute[]` stay as a compatibility view at
  first and are filled from the new database, so existing callers keep
  working. They are removed step by step.
- Loaded once at startup; read-only at runtime (the editor excepted).

### 2. Data fields

Each field belongs to one of two groups:

- **Shared** fields map 1:1 to OpenMU and take part in import/export.
- **Client-only** fields (rendering and UI) never go to the server.

| Group | Fields | Shared? |
|---|---|---|
| Identity | group, number, stable key (e.g. `sword.kris`, optional) | yes |
| Name | `LocalizedString`, same format as OpenMU | yes |
| Size and slot | width, height, item slot, two-handed | yes |
| Stats | damage, defense, speed, durability, requirements, resistances, class requirements, value, skill | yes (mapped to OpenMU requirements and power-ups) |
| Server flags | ammunition, bound to character, quest item, drops from monsters, storage limit | yes |
| Client rules | tradable, droppable, storable, sellable to NPC, personal-shop sellable, repairable, usable/consumable, equippable | see open question Q4 |
| Categories/tags | wing, flying (fly), mount, pet, potion, jewel, ticket, cash shop, event, … | client-only at first (replaces `ItemCategories` lists) |
| Model | model folder, file name, file index, texture folder | client-only |
| Rendering | inventory scale, rotation, offsets, glow/effects hooks | client-only (later phase) |

### 3. Rule code on top of data

- Keep behavior in code and move item lists into data. Example:
  `IsTradeBan(item)` stays, but reads the `tradable` flag and the item's
  tags instead of `switch` lists.
- Every move must keep the exact item set, checked as in #625 by expanding
  the data back to item ids and comparing with the hardcoded version.
- Named enum constants remain for items that code must refer to directly
  (special behavior); lists of items become data.

### 4. Names, translations and logging

- Item names are stored as OpenMU-style `LocalizedString`, so names from the
  server can be taken over directly.
- The name in the selected language is shown; missing translations fall back
  to English.
- Logs always use `<English name> (<group>,<number>)`, never the translated
  name. One helper (e.g. `ItemLogName(item)`) is used everywhere.
- Stats are no longer duplicated per language: one data set, only names are
  translated.

### 5. OpenMU sync (dump and import)

- **Server is authoritative.** The client never writes to the server, not
  even in editor mode.
- **Import (server → client):** read an OpenMU item export (JSON) and update
  the shared fields of the client data. Later maybe a button that pulls it
  from a configured admin endpoint.
- **Export (client → server):** create a dump of the shared fields as a file
  or on the clipboard, which the admin loads in the OpenMU admin panel
  (import page or update plug-in). This needs a small OpenMU-side change,
  proposed as a separate OpenMU PR.
- A **diff view** in the editor shows which items differ from the last
  import before anything is exported.

### 6. Build gating

- Editor, import, export and dump exist only in `_EDITOR` builds, like
  MuEditor today. Player release builds only contain the loader and the
  read-only database.
- Editing client data can never affect the server; server-side validation
  stays independent.

### 7. MuEditor item editor

Extend `src/MuEditor/UI/ItemEditor/` with:

- item list with search and filters by group, tag and flag
- detail view grouped like the field table above: flags as checkboxes, tags
  as a multi-select, stats, requirements
- name editor with one row per language, English required
- model picker with a 3D preview of the item
- OpenMU import, diff view and export dump
- validation (duplicate ids, missing English names, missing model files)
  before saving

## Phases (one PR each)

0. **This document**: agree on the design and answer the open questions.
1. **Item database**: `ItemDefinition` and lookups loaded from the existing
   `Item_<lang>.bmd`; `ItemAttribute[]` filled from it. No behavior change.
   Add the log-name helper.
2. **Data file format**: new data file(s) (see Q1), plus a converter from the
   bmd. The loader reads the new format; decide whether bmd stays as a
   fallback.
3. **Rules and categories into data**: flags and tags replace the hardcoded
   lists in `ItemCategories`, `TradeRestrictions` and `ShopRestrictions`.
   Item sets are verified to be identical.
4. **Models into data**: `OpenItems()` / `OpenItemTextures()` are driven by
   the model fields.
5. **Translations**: `LocalizedString` names and fallback; one stat data set
   for all languages.
6. **Editor**: the MuEditor features from section 7.
7. **OpenMU sync**: import, diff, export; the OpenMU-side import as a
   separate PR over there.

## Open questions

- **Q1: Data file format.** JSON (readable, diffable, close to OpenMU's JSON
  serializer), keep bmd, or SQLite? *Proposal:* JSON as the source of truth,
  optionally packed to a binary for release builds.
- **Q2: One file or several?** A single `items.json`, or one file per item
  group (smaller diffs)? *Proposal:* one file per group.
- **Q3: Dump format for OpenMU.** JSON keyed by `(Group, Number)` or SQL
  `UPDATE` statements? SQL is tied to OpenMU's database schema and breaks
  when it changes. *Proposal:* JSON plus a small import on the OpenMU side;
  SQL only if an OpenMU change is not wanted.
- **Q4: Client-only rules vs. OpenMU.** OpenMU only has
  `IsBoundToCharacter`. Should the detailed flags (tradable, sellable,
  storable, repairable, personal shop, …) be proposed as new `ItemDefinition`
  fields for OpenMU, or stay client-only?
- **Q5: Target version.** Season 6 (`VersionSeasonSix` initialization) only,
  or support other OpenMU versions?
- **Q6: Stable string key.** Is `(group, number)` enough as identity, or do
  we also want a readable key (e.g. `potion.apple`) for data files and
  logs?
- **Q7: Server pull button.** Is a file-based import/export enough for now,
  or is pulling directly from the admin panel wanted early? How does the
  editor authenticate against the admin panel?
- **Q8: Other item files.** When do `ItemAddOption`, `SocketItem`, `Mix`,
  `pet` and the set options follow, and in which order?
