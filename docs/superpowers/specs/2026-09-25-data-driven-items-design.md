# Data-Driven Items Design (WIP)

> **Temporary working document.** It is used to refine the plan on the
> `refactor/data-driven-items` branch and will be removed once the work
> has landed.
>
> Based on MuMain `upstream/main` @ `7187ec2a` and OpenMU `master` @
> `1e50df3bd`.

## Goal

Replace hardcoded item knowledge in the client with **item data plus rule
code**:

- All item data (stats, flags, categories, names, models) lives in data
  files, is loaded into an **in-memory item database** at startup, and rule
  code reads it instead of hardcoded item lists.
- Runtime lookups are **as fast as hardcoded checks** (see section 1).
- The data model is **equivalent to OpenMU's `ItemDefinition`** where both
  sides overlap, so data can be moved between client and server with
  file-based import and export on both sides.
- Items can be **added and removed** on client and server, and linked to
  their model data with MuEditor tools.
- Every data field can be edited in **focused MuEditor editors** (several
  small tools, not one huge table).
- Item names are **translatable**; logs always use a stable, language-neutral
  identifier.

Scope: **items only.** Skills, monsters, maps, etc. may follow the same
pattern later but are not part of this work.

## Decisions so far

| # | Topic | Decision |
|---|---|---|
| D1 | Data format | JSON files as the source of truth, loaded once at startup into flat tables. Runtime speed must be as close to hardcoded as possible; a binary cache is added only if startup parsing is measurably too slow. |
| D2 | File layout | One file per item group. |
| D3 | Server rules | Check what OpenMU already enforces (see "Server-side checks today"), then extend OpenMU's tables or add new ones following OpenMU's usual workflow. |
| D4 | Target version | Season 6, because MuMain is a Season 6 Episode 3 client (see Q5 note). |
| D5 | Sync | File-based import and export on **both** sides. No live connection or pull button. |
| D6 | Editors | Several focused editors instead of one big table. |
| D7 | Add/remove | Items can be created and removed on client and server; MuEditor tools link a new item to its model data. |

## Current state

### Client (MuMain)

| Area | Where | Notes |
|---|---|---|
| Item stats | `Data/Local/<lang>/Item_<lang>.bmd` → `CItemDataHandler` (`src/source/Data/DataHandler/ItemData/`) → `ITEM_ATTRIBUTE[MAX_ITEM]` | Fields defined once as X-macros in `Data/GameData/ItemData/ItemFieldDefs.h`. The file is per language, so stats are duplicated for every language. |
| Item names | `ITEM_ATTRIBUTE::Name` (from the same per-language bmd) | No shared key between languages except the array index. |
| Categories and rules | `GameLogic/Items/ItemCategories`, `TradeRestrictions`, `ShopRestrictions` (from PR sven-n/MuMain#625) | Now one source of truth, but still hardcoded item lists in C++. |
| Models and textures | `OpenItems()` / `OpenItemTextures()` in `Engine/Object/ZzzOpenData.cpp` | Around 600 hardcoded lines mapping `MODEL_*` to `Data\Item\<File><n>.bmd`. The model id is always `MODEL_ITEM + itemType`, so every item slot already has a model slot. |
| Item id space | `MAX_ITEM = MAX_ITEM_TYPE * MAX_ITEM_INDEX` (16 groups × 512) | Same limits as the network protocol, so new items must fit into free slots. |
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
- OpenMU ships separate data sets per game version (`Version075`,
  `Version095d`, `VersionSeasonSix`); a server database is initialized from
  one of them.
- Data changes ship as `UpdatePlugInBase` plug-ins. The admin panel's
  generic config grid (`EditConfigGrid`) can already **add and delete**
  item definitions; deleting fails while something still references the
  item. There is no file import or export for config yet.

### Server-side checks today

What OpenMU enforces for the rules the client has in `TradeRestrictions` and
`ShopRestrictions`:

| Client rule | OpenMU today | Where |
|---|---|---|
| Trade ban | Only `IsBoundToCharacter` (blocks moving between storages, so also into the trade window) | `MoveItemAction` |
| Vault/storage ban | Same `IsBoundToCharacter` check | `MoveItemAction` |
| Personal shop ban | `IsBoundToCharacter` (item cannot be moved there) and items with a harmony option cannot open a store | `MoveItemAction`, `OpenStoreAction` |
| Sell to NPC ban | `IsBoundToCharacter` (with a durability exception) | `SellItemToNpcAction` |
| Drop ban | None; bound items can only be picked up by their owner | `DropItemAction` |
| Repair ban | None per item type; only durability and money | `ItemRepairAction` |
| Equip | Data-driven: `ItemSlot`, requirements, two-hand conflicts | `MoveItemAction` |
| Use/consume | Consume plug-ins keyed by hardcoded `ItemConstants`, or `ConsumeEffect` data | `ItemConsumeActions/*` |
| Categories (wing, jewel, …) | Hardcoded group/number checks, about 113 in `src/GameLogic` | `ItemExtensions`, crafting, events, … |

So the client is **stricter** than the server for trade, drop, store, sell
and repair. Those client rules are only UI checks today; a modified client
can skip them.

**Approach (D3):**

1. Build the full mapping of every client rule and category to an OpenMU
   field or check (phase 3 prerequisite).
2. Where OpenMU has no equivalent, extend it the OpenMU way: new
   `ItemDefinition` properties (or a separate table if it grows large),
   an EF migration, Season 6 initialization values, an `UpdatePlugIn` that
   sets the values on existing databases, and enforcement in the matching
   `PlayerAction`. The admin panel picks up new properties automatically.
3. These are separate OpenMU PRs. The client uses the same field names so
   the data maps 1:1.

## Target design

### 1. In-memory item database (performance)

The data is loaded once at startup and then works like a hardcoded table:

- **Flat arrays indexed by item type** (`group * MAX_ITEM_INDEX + number`,
  8192 entries). No hash maps or string lookups at runtime.
- **Flags and tags as bitmasks** per item. `IsTradeBan(item)` becomes one
  array read plus a bit test, which is as cheap as or cheaper than the
  current `switch` and range checks.
- **Hot data separate from cold data**: small, contiguous records for data
  used every frame (flags, tags, size, model id); names, requirements and
  editor-only data in separate arrays.
- **Names**: for the selected language, one pointer table is built at load
  time; the English name is kept for logging.
- **Immutable after load.** Only the editor rebuilds the tables after saving.
- `ITEM_ATTRIBUTE` / `ItemAttribute[]` stay as a compatibility view at
  first and are removed step by step.
- Startup: measure the JSON load time. If it is noticeably slow, generate a
  binary cache from the JSON (rebuilt when the JSON changes).

### 2. Data fields

Each field belongs to one of two groups:

- **Shared** fields map 1:1 to OpenMU and take part in import/export.
- **Client-only** fields (rendering and UI) never go to the server.

| Group | Fields | Shared? |
|---|---|---|
| Identity | group, number | yes |
| Name | `LocalizedString`, same format as OpenMU | yes |
| Size and slot | width, height, item slot, two-handed | yes |
| Stats | damage, defense, speed, durability, requirements, resistances, class requirements, value, skill | yes (mapped to OpenMU requirements and power-ups) |
| Server flags | ammunition, bound to character, quest item, drops from monsters, storage limit | yes |
| Rule flags | tradable, droppable, storable, sellable to NPC, personal-shop sellable, repairable, usable/consumable, equippable | yes, after OpenMU is extended (D3) |
| Categories/tags | wing, flying, mount, pet, potion, jewel, ticket, cash shop, event, … | client-only at first; shared where OpenMU gets matching data |
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

### 5. OpenMU sync (file-based, both sides)

- **Server is authoritative.** The client never writes to the server, not
  even in editor mode.
- One **exchange file format** (JSON, with a format version) keyed by
  `(Group, Number)`, containing only the shared fields.
- **Client → server:** MuEditor exports the file; the admin imports it in a
  new OpenMU admin panel import page.
- **Server → client:** a new OpenMU admin panel export page writes the same
  file; MuEditor imports it and updates the shared fields.
- Import on both sides shows a **diff** (new, changed, removed items) and
  applies only what the user confirms.
- Removals in an import are only applied when confirmed; on the server they
  fail if characters still own the item (existing OpenMU behavior).
- The OpenMU import/export pages are a separate OpenMU PR.

### 6. Adding and removing items

**Client (MuEditor):**

- **New item wizard**: choose the group, pick a free number (the editor
  shows used and free slots), optionally copy an existing item as a
  template, enter the English name, size and slot, then link the model with
  the model tool.
- The model id is `MODEL_ITEM + itemType`, so every new item already has a
  model slot; only the file link is needed.
- **Remove item**: warns when the item is still in use: named in code (enum
  constant with special behavior), member of a tag, used by other data, or
  present in the last OpenMU import.
- Limit: new items must fit into free slots of the 16 × 512 id space, the
  same as the network protocol.

**Server (OpenMU):** the admin panel can already add and delete item
definitions. New items also arrive through the import (section 5).

### 7. Build gating

- Editors, add/remove, import and export exist only in `_EDITOR` builds,
  like MuEditor today. Player release builds only contain the loader and the
  read-only database.
- Editing client data can never affect the server; server-side validation
  stays independent.

### 8. MuEditor item tools

Several focused editors that share one item selection, instead of one big
table:

| Editor | Purpose |
|---|---|
| **Item browser** | List with search and filters (group, tag, flag, missing data); add, duplicate and remove items. Selecting an item opens it in the other editors. |
| **Basics and stats** | Name, size, slot, durability, value, damage/defense/speed, resistances. |
| **Requirements** | Level and stat requirements, allowed classes. |
| **Rules** | Flags as a matrix (items × tradable, droppable, storable, sellable, …) for bulk editing. |
| **Categories** | Tag-centered view: pick a tag, see and change its items. |
| **Model and visuals** | Model file picker for `Data\Item\*.bmd` with 3D preview, texture folder, later inventory render offsets. |
| **Translations** | Items × languages grid, filter for missing translations. |
| **OpenMU sync** | Import, diff and export of the exchange file. |

Common features: validation before saving (duplicate ids, missing English
name, missing model file), and undo for the current session.

## Phases (one PR each)

0. **This document**: agree on the design.
1. **Item database**: flat tables loaded from the existing `Item_<lang>.bmd`;
   `ItemAttribute[]` filled from them. No behavior change. Add the log-name
   helper. Measure lookup and load performance.
2. **Data file format**: JSON files per group, plus a converter from the
   bmd. Decide whether bmd stays as a fallback.
3. **Rules and categories into data**: flags and tags replace the hardcoded
   lists in `ItemCategories`, `TradeRestrictions` and `ShopRestrictions`.
   Item sets are verified to be identical. Includes the client ↔ OpenMU
   rule mapping.
4. **Models into data**: `OpenItems()` / `OpenItemTextures()` are driven by
   the model fields.
5. **Translations**: `LocalizedString` names and fallback; one stat data set
   for all languages.
6. **Editors**: the MuEditor tools from section 8, including add/remove.
7. **Sync**: client import/export and diff.

OpenMU PRs (in parallel, separate repo):

- A. New rule fields/tables with migration, initialization, update plug-in
  and server enforcement (from the phase 3 mapping).
- B. Admin panel import and export pages for the exchange file.

## Open questions

- **Q1: bmd fallback.** Keep reading `Item_<lang>.bmd` as a fallback after
  phase 2, or remove it once the JSON files exist?
- **Q2: Exchange file scope.** Only item definitions, or already include
  the linked data OpenMU needs for a complete item (item options, set
  groups, drop settings)?
- **Q3: Custom items beyond Season 6.** Should custom items get their own
  number range (e.g. high numbers per group) so they do not collide with
  future official items?
- **Q4: Other item files.** When do `ItemAddOption`, `SocketItem`, `Mix`,
  `pet` and the set options follow, and in which order?
