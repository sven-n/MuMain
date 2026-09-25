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
| D1 | Data format | JSON files as the source of truth, loaded once at startup into flat tables. Runtime speed must be as close to hardcoded as possible; a binary cache is added only if startup parsing is measurably too slow. Chosen over a database or SQL text files; see section 2. |
| D2 | Exchange format with OpenMU | JSON keyed by `(Group, Number)`, not SQL `UPDATE` statements (SQL would break whenever OpenMU's database schema changes). |
| D3 | Server rules | Check what OpenMU already enforces (see "Server-side checks today"), then extend OpenMU's tables or add new ones following OpenMU's usual workflow. |
| D4 | Sync | File-based import and export on **both** sides. No live connection or pull button. |
| D5 | Editors | Several focused editors instead of one big table. |
| D6 | Add/remove | Items can be created and removed on client and server; MuEditor tools link a new item to its model data. |
| D7 | File layout | One JSON file per item group. |
| D8 | Target version | Season 6 only for now. Other versions are easier to add once the data-driven setup exists. |
| D9 | Identity | `(group, number)` only, no readable string key. Logs always show the English name next to it. Once rules read data instead of hardcoded ids, a readable key has no extra benefit. |
| D10 | bmd files | The JSON files are the **only** source of truth for items in the game. The game no longer reads `Item_<lang>.bmd`. MuEditor gets "Import from bmd" and "Export as bmd"; fields the bmd format does not have are left out on export and keep their current or default values on import. |
| D11 | Exchange scope | Item sets and drop settings are separate exchange files, not part of the item file. |
| D12 | Id ranges | No reserved number ranges for custom items, and no range checks (`type >= X && type <= Y`) in new code; everything an item "is" comes from its data. OpenMU must keep supporting the original Season 6 client **and** this client (see section 7). |
| D13 | Other item files | `ItemAddOption`, `SocketItem`, `Mix`, `pet` and set options are decided later, when we know more. |
| D14 | Item options | The item file only holds **links** to the option groups an item can have (Luck, additional option, excellent, wing, harmony, guardian, socket, …). The option definitions (values, chances, levels) live in their own files. Related data stays together in one file, e.g. an option definition with all its levels and values. |
| D15 | Tooltips | Tooltips become data-driven with JSON data, converted from `ItemTooltip*` / `ItemLevelTooltip` (see "Unused item files in the repo"). Phase 8, after the item phases; moved earlier if the item changes turn out to need it. The tooltip bmd files stay in the repo until then. |
| D16 | Unused item bmd copies | Removed on this branch; the game only loads `Item_<lang>.bmd`. |

## Current state

### Client (MuMain)

| Area | Where | Notes |
|---|---|---|
| Item stats | `Data/Local/<lang>/Item_<lang>.bmd` → `CItemDataHandler` (`src/source/Data/DataHandler/ItemData/`) → `ITEM_ATTRIBUTE[MAX_ITEM]` | Fields defined once as X-macros in `Data/GameData/ItemData/ItemFieldDefs.h`. The file is per language, so stats are duplicated for every language. |
| Item names | `ITEM_ATTRIBUTE::Name` (from the same per-language bmd) | No shared key between languages except the array index. |
| Categories and rules | `GameLogic/Items/ItemCategories`, `TradeRestrictions`, `ShopRestrictions` (from PR sven-n/MuMain#625) | Now one source of truth, but still hardcoded item lists in C++. |
| Models and textures | `OpenItems()` / `OpenItemTextures()` in `Engine/Object/ZzzOpenData.cpp` | Around 600 hardcoded lines mapping `MODEL_*` to `Data\Item\<File><n>.bmd`. The model id is always `MODEL_ITEM + itemType`, so every item slot already has a model slot. |
| Item id space | `MAX_ITEM = MAX_ITEM_TYPE * MAX_ITEM_INDEX` (16 groups × 512) | Same limits as the network protocol, so new items must fit into free slots. |
| Data files in the repo | `src/bin/Data/` (e.g. `src/bin/Data/Local/Eng/item_eng.bmd`), copied beside `Main` at build time | Game data is versioned with the code, so data changes already go through PRs. |
| Other item data | `ItemAddOption.bmd`, `SocketItem_<lang>.bmd`, `Mix.bmd`, `pet.bmd`, set options (`ItemSetType.bmd`, `ItemSetOption_<lang>.bmd`, loaded by `CSItemOption`) | Out of scope for the first phases; listed so they are not forgotten. **Item sets may have to move together with items** if items cannot be made fully data-driven without them; we decide that when we get there. |
| Tooltips | `RenderItemInfo()` in `Engine/Object/ZzzInventory.cpp` | Which lines an item's tooltip shows is hardcoded. |
| Unused item files | See "Unused item files in the repo" below | Loaded by no code. |
| Editor | `src/MuEditor/UI/ItemEditor/` (only in `_EDITOR` builds) | Table editor for the bmd fields; save to bmd, export to S6E3 bmd and CSV. |
| UI translations | `.resx` → generated `I18N::*` (`docs/translation-system.md`) | Good for UI strings; item names are not part of it yet. |

### Unused item files in the repo

No code in the repo loads these files, and `git log -S` finds no loader
in the history either. They use the usual bmd encoding (`BuxConvert` XOR
`FC CF AB`, restarted for every record), followed by a 4-byte trailer
(probably a checksum). They exist for `Eng`, `Por` and `Spn`.

| File | Layout | Content |
|---|---|---|
| `ItemTooltip_<lang>.bmd` | 8192 records × 124 bytes; 906 used | Per item: `u16 group`, `u16 number`, `char name[64]`, then a list of up to about 12 tooltip lines as `(u16 textId, u16 style)` pairs (`0xFFFF` = no line). Example: *Kris* (0,0) shows lines 6, 8, 11, 12, 13, 14. Names are translated in the `Por`/`Spn` files ("Espada curta", "Espada corta"). |
| `ItemTooltipText_<lang>.bmd` | 512 records × 260 bytes; 351 used | Tooltip line texts: `u16 id`, `char text[256]`, `u16 valueKind`. Examples: "One handed attack power: %d~%d", "Armor: %d", "Available HP: %d", "Increases damage by %d%%". `valueKind` looks like the value that fills the `%d` (0 = damage, 8 = HP, 2xx = options). |
| `ItemLevelTooltip_<lang>.bmd` | 128 records × 102 bytes; 105 used | Items whose name and tooltip change with the item level: `u16 id`, `char name[64]`, then header words and `(textId, style)` pairs. Examples: "Box of Kundun +1" … "+5", "Scroll of Emperor", "Box of Heaven". |

Our reading of the layout is taken from the bytes, not from a loader, so
field meanings (especially the style and header words) are guesses.

This is exactly the data a data-driven tooltip needs (which lines, which
texts, which values, per item and item level), and it comes with names in
three languages. The files are kept as the input for data-driven tooltips
(D15).

Extra item bmd copies that no code loaded (`Data/Local/Item.bmd`,
`Eng/item.bmd`, `Eng/item_eng_old.bmd`, `Eng/item_eng_orig.bmd`,
`Por/item.bmd`, `Por/item_eng_old.bmd`, `Spn/item.bmd`) were removed on
this branch (D16).

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
- The rule code only talks to these tables, never to the storage format, so
  the storage can change later without touching rules.

### 2. Storage, loading and validation

#### Why JSON (D1)

At runtime every option ends up as the same in-memory tables (section 1);
the choice is only about how the data is stored, reviewed and exchanged.

| Option | Why not chosen |
|---|---|
| Binary database file (e.g. SQLite `.db`) | A blob in git: changes cannot be reviewed, and parallel branches cannot be merged. |
| SQL text files (schema + `INSERT`s) loaded into SQLite | Diffable, and the database checks links and uniqueness itself. But one item is spread over several tables, a changed value shows up as a long changed `INSERT` line, the player build needs SQLite as a new dependency, and the SQL cannot be run on OpenMU anyway (PostgreSQL via Entity Framework, GUID keys, different tables), so a second format for the exchange would still be needed. |
| Editing in a database and exporting JSON for git | Two sources of truth that drift apart. |

JSON gives readable diffs with related data kept together (D14), the same
format as the OpenMU exchange (D2), and no new dependency: the client
already bundles nlohmann JSON (`src/ThirdParty/json.hpp`). The cost is that
the checks a database would do (unique ids, valid links) must be done by
our own loader; see "Validation" below.

#### Files

- Location: `src/bin/Data/Items/`, next to the other game data in the repo.
  It is copied beside `Main` like the rest of `Data` (`MU_COPY_RUNTIME_ASSETS`).
- One file per item group (D7), e.g. `Group14_Potions.json`. Files for other
  data (option definitions, sets, …) follow the same rules when they come.
- Every file starts with a `formatVersion` and the `group` it contains.
- Encoding UTF-8, LF line endings.

#### Writing (editor, bmd import, OpenMU import)

- **Deterministic output** so diffs only show real changes: items sorted by
  number, fixed key order, fixed indentation, one field per line.
- **Safe save**: write to a temporary file, then replace the old file, so a
  crash never leaves a half-written file.
- Only valid data is saved: the editor runs the validation below first and
  refuses to save while there are errors.

#### Loading (game and editor)

1. Read and parse all item files once at startup.
2. Check `formatVersion`: a newer version than the client understands is an
   error; older versions are upgraded in memory.
3. Missing optional fields get their default value, so adding a field never
   breaks older files. Unknown fields are ignored by the game and shown as a
   warning in the editor.
4. Run the validation below.
5. Build the flat tables from section 1, then release the parsed JSON.
6. Log the load time and item count (used for the performance check in
   phase 1).

#### Validation

The same validation code runs in the game loader, in the editor before
saving, and in an automated test.

Errors (data cannot be used):

- invalid JSON, or a file whose `group` does not match its items
- `group` outside 0–15 or `number` outside 0–511
- duplicate `(group, number)`
- missing English name
- a value outside its type's range (e.g. width 0, a byte field above 255)
- a link to something that does not exist: unknown tag or flag, unknown
  option group, unknown skill, and later unknown set or option definition

Warnings (data works, but is probably wrong):

- the model file or texture folder does not exist
- a translation is missing for a supported language
- an item has no model link
- an equippable item without an item slot, or similar combinations that
  do not make sense

Behavior:

- **Game**: errors stop the start with a clear message naming the file, the
  item `(group, number)` and English name, and the field. The game never
  runs with half-loaded item data. Warnings are logged.
- **Editor**: errors and warnings are listed and clickable (they jump to the
  item and field); saving is blocked while errors exist.
- **Automated test**: a doctest test in `tests/` loads all item files in
  `src/bin/Data/Items/` and runs the validation, so broken data fails the
  PR checks before it is merged.

### 3. Data fields

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

### 4. Rule code on top of data

- Keep behavior in code and move item lists into data. Example:
  `IsTradeBan(item)` stays, but reads the `tradable` flag and the item's
  tags instead of `switch` lists.
- Every move must keep the exact item set, checked as in #625 by expanding
  the data back to item ids and comparing with the hardcoded version.
- Named enum constants remain for items that code must refer to directly
  (special behavior); lists of items become data.

### 5. Names, translations and logging

- Item names are stored as OpenMU-style `LocalizedString`, so names from the
  server can be taken over directly.
- The name in the selected language is shown; missing translations fall back
  to English.
- Logs always use `<English name> (<group>,<number>)`, never the translated
  name. One helper (e.g. `ItemLogName(item)`) is used everywhere.
- Stats are no longer duplicated per language: one data set, only names are
  translated.

### 6. OpenMU sync (file-based, both sides)

- **Server is authoritative.** The client never writes to the server, not
  even in editor mode.
- One **exchange file format** (JSON, with a format version) keyed by
  `(Group, Number)`, containing only the shared fields.
- Item sets, drop settings and option definitions get their own exchange
  files later (D11, D14). The item file only links to the option groups an
  item can have.
- **Client → server:** MuEditor exports the file; the admin imports it in a
  new OpenMU admin panel import page.
- **Server → client:** a new OpenMU admin panel export page writes the same
  file; MuEditor imports it and updates the shared fields.
- Import on both sides shows a **diff** (new, changed, removed items) and
  applies only what the user confirms.
- Removals in an import are only applied when confirmed; on the server they
  fail if characters still own the item (existing OpenMU behavior).
- The OpenMU import/export pages are a separate OpenMU PR.

### 7. Adding and removing items

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

**Custom items and the original client (D12):**

- Official Season 6 items keep their official `(group, number)`, so the
  original client and this client agree on them.
- Custom items take any free slot; there are no reserved ranges. The data
  marks them as custom, so nothing depends on where their number lies.
- OpenMU must keep working with the original Season 6 client. That client
  does not know custom items, so the server has to handle them for it
  (see Q1). OpenMU already knows which client a server is for
  (`GameClientDefinition`: season, episode, language, version), which is a
  possible hook.

**Server (OpenMU):** the admin panel can already add and delete item
definitions. New items also arrive through the import (section 6).

### 8. Build gating

- Editors, add/remove, import and export exist only in `_EDITOR` builds,
  like MuEditor today. Player release builds only contain the loader and the
  read-only database.
- Editing client data can never affect the server; server-side validation
  stays independent.

### 9. MuEditor item tools

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

**Side:** *Client* = MuMain only, *Server* = OpenMU only, *Both* = changes
in both repos (as separate PRs, one per repo).

| Phase | Name | Side | Repo / PR | Depends on | Summary |
|---|---|---|---|---|---|
| 0 | Design document | Both | MuMain | – | Agree on the design (this document). |
| 1 | Item database | Client | MuMain | 0 | `ItemDefinition` model and flat in-memory table, built from the loaded `Item_<lang>.bmd` data; English names for logs; log-name helper; load-time log. No behavior change. |
| 2 | Data file format | Client | MuMain | 1 | JSON per group becomes the only item source and the database becomes the source for `ItemAttribute[]`; loading, writing and validation rules; automated data test; bmd import/export in MuEditor. |
| 3 | Rules and categories into data | Client | MuMain | 2 | Flags and tags replace the hardcoded lists; client ↔ OpenMU rule mapping (input for A). |
| A | Server rule fields and checks | Server | OpenMU | 3 (mapping) | New `ItemDefinition` fields or tables, migration, Season 6 values, update plug-in, enforcement in player actions. |
| 4 | Models into data | Client | MuMain | 2 | `OpenItems()` / `OpenItemTextures()` driven by the model fields. |
| 5 | Translations | Client | MuMain | 2 | `LocalizedString` names with fallback; one stat data set for all languages. |
| 6 | Editors | Client | MuMain | 2–5 | Focused MuEditor tools (section 9), including add/remove items. |
| 7 | Item sync, client side | Client | MuMain | 2, 6 | MuEditor import/export of the item exchange file, with diff. |
| B | Item sync, server side | Server | OpenMU | 7 (file format) | Admin panel import/export pages for the item exchange file, with diff. |
| 8 | Data-driven tooltips | Client | MuMain | 2, 5 | Tooltip JSON converted from the `ItemTooltip*` files; `RenderItemInfo()` reads data; tooltip editor. Moved earlier if needed. |
| 9 | Item option definitions | Both | MuMain + C | 2 | Option groups in their own JSON files matching OpenMU's `ItemOptionDefinition`; exchange file and editor. |
| 10 | Item sets | Both | MuMain + C | 2 | Item sets in JSON matching OpenMU's `ItemSetGroup`; exchange file and editor; based on the `item-set-editor` branch. Moved earlier if needed. |
| C | Option and set sync, server side | Server | OpenMU | 9, 10 | Admin panel import/export for the option definition and item set exchange files. |
| 11 | Remaining item files | Both (per file) | MuMain, OpenMU as needed | 2 | `ItemAddOption`, `SocketItem`, `Mix`, `pet`, drop settings; one phase each, order decided later. |
| 12 | Cleanup | Client | MuMain | all | Remove this document. |

The deferred question Q1 (custom items on the original client) is a
**Server** topic and must be decided before custom items are used on a
server with original clients (after phases 6 and B).

### Phase details

0. **This document**: agree on the design.
1. **Item database**: an `ItemDefinition` model (clean field names, lossless
   conversion to and from `ITEM_ATTRIBUTE`) and a flat table indexed by item
   type (`Data::Items::ItemDatabase`). In this phase the database is built
   **from** the loaded `ItemAttribute[]`, because the item editor still
   edits `ItemAttribute[]` directly; saving in the editor rebuilds it.
   English names are loaded from the `Eng` item file when another language
   is selected, and `GetLogName()` returns `<English name> (<group>,<number>)`.
   The load and build times are logged. No behavior change.
   Hot/cold splitting of the table comes with the first hot data (flags and
   tags, phase 3).
2. **Data file format**: JSON files per group become the only item source
   for the game, with the loading, writing and validation rules from
   section 2 and the automated data test. MuEditor gets "Import from bmd"
   and "Export as bmd"; the game stops reading `Item_<lang>.bmd`. The data
   flow turns around: JSON → database → `ItemAttribute[]` (compatibility
   view), and the item editor edits the database instead of
   `ItemAttribute[]`.
3. **Rules and categories into data**: flags and tags replace the hardcoded
   lists in `ItemCategories`, `TradeRestrictions` and `ShopRestrictions`.
   The resulting item lists are verified to be identical. Includes the
   client ↔ OpenMU rule mapping.

   **A (OpenMU):** new rule fields/tables with migration, initialization,
   update plug-in and server enforcement, based on the phase 3 mapping.
4. **Models into data**: `OpenItems()` / `OpenItemTextures()` are driven by
   the model fields.
5. **Translations**: `LocalizedString` names and fallback; one stat data set
   for all languages.
6. **Editors**: the MuEditor tools from section 9, including add/remove.
7. **Item sync, client side**: MuEditor import/export of the item exchange
   file and diff.

   **B (OpenMU):** admin panel import and export pages for the item
   exchange file, with diff.
8. **Data-driven tooltips** (D15): convert `ItemTooltip_<lang>`,
   `ItemTooltipText_<lang>` and `ItemLevelTooltip_<lang>` into JSON (line
   texts as `LocalizedString`, like item names), and let `RenderItemInfo()`
   build tooltips from that data instead of hardcoded lines. Adds a tooltip
   editor to MuEditor. The tooltip bmd files are deleted afterwards.
   Moved earlier if the item phases need it.
9. **Item option definitions** (D14): the option groups items link to
   (Luck, additional option, excellent, wing, harmony, guardian, socket, …)
   get their own JSON files, matching OpenMU's `ItemOptionDefinition`, with
   their own exchange file and editor.
10. **Item sets** (D11): `ItemSetType.bmd` / `ItemSetOption_<lang>.bmd` into
    JSON, matching OpenMU's `ItemSetGroup`, with their own exchange file and
    editor. Moved earlier if items cannot be made fully data-driven without
    them.

    **Starting point:** branch
    [`Mosch0512/MuMain:item-set-editor`](https://github.com/Mosch0512/MuMain/tree/item-set-editor)
    (one commit, `e8194a1c`, April 2026, still bmd-based). When we reach
    this phase, rebase it onto the current branch and use it as the base,
    or as a reference if the rebase is not worth it. It contains:

    - an ImGui item set editor modeled on the item editor (search,
      hide-empty toggle, option combobox per row) that saves back to
      `ItemSetOption_<lang>.bmd`
    - `CSItemOption` access for editor tools, and option labels built
      from the text template without `printf`, so options whose texts
      expect more arguments cannot crash the editor
    - a fallback so tooltips also show set options for **unequipped**
      ancient items (a behavior fix)
    - a CMake change to copy runtime assets with
      `copy_directory_if_different` (faster incremental builds)

    Expected rework: the branch is about 1060 commits behind `main`.
    `CSItemOption` moved to `GameLogic/Items/`, `GlobalText` and
    `Translations/*.json` were replaced by the generated `I18N::*` texts,
    and loading and saving bmd files has to become JSON (phase 2 rules).
    The tooltip fix and the CMake change are unrelated to the data
    format and could be split into their own small PRs.

    **C (OpenMU):** import and export for the option definition (phase 9)
    and item set (phase 10) exchange files.
11. **Remaining item files** (D13): `ItemAddOption`, `SocketItem`, `Mix`,
    `pet` and drop settings, one phase each. Order decided when we get
    there. Each one gets OpenMU work where the server has matching data.
12. **Cleanup**: remove this document once the work has landed.

## Open questions

- **Q1: Custom items on the original client** *(deferred; must be
  discussed before the related PR lands).* What should OpenMU do with a
  custom item when the player uses the original Season 6 client? For
  example: never send it to that client (hide it in shops, drops and
  views), block such items on servers set up for the original client, or
  show a placeholder item.
