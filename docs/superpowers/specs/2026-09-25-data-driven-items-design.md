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
  sides overlap, so data can be moved between client and server: by file on
  both sides, and by an update from the server in MuEditor (phase 7).
- Items can be **added and removed** on client and server, and linked to
  their model data with MuEditor tools.
- Every data field can be edited in **focused MuEditor editors** (several
  small tools, not one huge table).
- Item names are **translatable**; logs always use a stable, language-neutral
  identifier.

Scope: **items only.** What goes beyond items (effects and particles,
skills, monsters and NPCs, and the look format and editing rules they
share with items) is planned in the
[roadmap](2026-09-29-data-driven-content-roadmap-design.md).

## Decisions so far

| # | Topic | Decision |
|---|---|---|
| D1 | Data format | JSON files as the source of truth, loaded once at startup into flat tables. Runtime speed must be as close to hardcoded as possible; a binary cache is added only if startup parsing is measurably too slow. Chosen over a database or SQL text files; see section 2. |
| D2 | Exchange format with OpenMU | JSON keyed by `(Group, Number)`, not SQL `UPDATE` statements (SQL would break whenever OpenMU's database schema changes). |
| D3 | Server rules | Check what OpenMU already enforces (see "Server-side checks today"), then extend OpenMU's tables or add new ones following OpenMU's usual workflow. |
| D4 | Sync | File-based import and export on **both** sides, each import with a diff the user checks by hand (an export writes its side's data), plus a read-only Update from server in MuEditor (phase 7, section 6). The client never writes to the server. |
| D5 | Editors | Several focused editors instead of one big table. |
| D6 | Add/remove | Items can be created and removed on client and server; MuEditor tools link a new item to its model data. |
| D7 | File layout | One JSON file per item group. Splitting the mixed groups 12–15 into category files was dropped: with new groups (D21) the group *is* the category, so the file layout is decided together with phase 12. |
| D8 | Target version | Season 6 only for now. Other versions are easier to add once the data-driven setup exists. |
| D9 | Identity | `(group, number)` only, no readable string key. Logs always show the English name next to it. Once rules read data instead of hardcoded ids, a readable key has no extra benefit. |
| D10 | bmd files | The JSON files are the **only** source of truth for items in the game. The game no longer reads `Item_<lang>.bmd`. MuEditor gets "Import from bmd" and "Export as bmd"; fields the bmd format does not have are left out on export and keep their current or default values on import. **Update (2026-09-26):** the repo no longer ships `Item_<lang>.bmd`. The editor keeps "Import from bmd" for your own old files and "Export as bmd". |
| D11 | Exchange scope | Item sets and drop settings are separate exchange files, not part of the item file. |
| D12 | Id ranges | No reserved number ranges for custom items, and no range checks (`type >= X && type <= Y`) in new code; everything an item "is" comes from its data. OpenMU must keep supporting the original Season 6 client **and** this client (see section 7). |
| D13 | Other item files | `ItemAddOption`, `SocketItem`, `Mix`, `pet` and set options are decided later, when we know more. |
| D14 | Item options | The item file only holds **links** to the option groups an item can have (Luck, additional option, excellent, wing, harmony, guardian, socket, …). The option definitions (values, chances, levels) live in their own files. Related data stays together in one file, e.g. an option definition with all its levels and values. |
| D15 | Tooltips | Tooltips become data-driven with JSON data, converted from `ItemTooltip*` / `ItemLevelTooltip` (see "Unused item files in the repo"). Phase 8, after the item phases; moved earlier if the item changes turn out to need it. The tooltip bmd files stayed in the repo until then; they were removed once the item data held the tooltips and the names by level (#815). **Update (2026-10-22, #815):** an item's own tooltip texts (its `description` lines) live in its item file, with their translations, as client-only fields that the OpenMU exchange leaves out (D22). Texts and layouts that many items share get a tooltip file of their own. **Status (2026-10-29):** steps 1–5 of #815 are merged (#813–#836, and #863 resolves the names when the data loads); phase 8 lists what is left. |
| D16 | Unused item bmd copies | Removed on this branch; the game only loads `Item_<lang>.bmd`. |
| D17 | Translations in phase 2 | Item names moved into phase 2, because the per-language bmd files were the only place the Portuguese and Spanish names existed. Names are `LocalizedString`s imported from all three files; the UI locale (the same setting as the `.resx` texts) picks the shown name, also when it changes at runtime. On disk, each item's `name` is an object of names by language (`{"en": "Blade", "es": "Espada", "pt": "Lâmina"}`, English first) in the item files; separate translation files were tried and dropped as extra maintenance. Phase 5 keeps only the translation tooling. |
| D18 | Repairing the legacy data | The bmd import recovers names that ran past the 30-byte name field and takes the fields they overwrote from a language whose name did not reach them (defaults when none has them). Names that are not UTF-8 are read as Windows-1252. The changes are listed in the phase 2 PR and in `docs/item-data.md`. |
| D19 | Editor sync | Every item editor change goes into the database right away (phase 2), so the editor and the database never differ. Moving the editor fully onto the database stays in phase 6 (done: the item tools edit the database, the item table is gone). |
| D22 | Model data files | Model and display data (model file, textures, inventory and ground display, cloth, effects) lives in separate files, `Data/Items/Models/GroupNN_*.json`, one per item group, items by `number`. It is client-only: the item files keep what client and server share, and only those take part in the OpenMU exchange. The exception is an item's own tooltip texts (D15): they stay in the item file next to its name, as client-only fields that the exchange leaves out. Separate files also keep the item files small and let the model editor and the stats editor change different files. |
| D23 | Model slots | Two steps. **Own model slots (phase 4a–4c):** every item keeps its own model slot, `MODEL_ITEM + item type`, as today; the data only says which file is loaded into it, so the ~100 places that compute `+ MODEL_ITEM` / `- MODEL_ITEM` stay unchanged. **Shared models (phase 4d, right after them):** models become entries of their own, and items reference them by name, so several items can share one model (loaded once; 4d1, done). Every item keeps a slot of its own, and the conversions between item types and model slots go through one place, `ToModelSlot` / `ToItemType` (4d2), so a moved item can keep its slot (phase 12) and the named slots of items (`MODEL_KRIS`, …) stay valid. One slot per shared model, with objects keeping item types, is not planned: about 240 places identify items by their model slot (see "Item-specific code that is left"), and none of them gains anything while each item has its own slot. Phase 12 (level variants as items of their own) shares models without loading them twice. |
| D24 | Render effects | The effect code stays code, but which item uses which effect moves into the model data: the glow as values (color names from a glow color list, `Data/Effects/GlowColors.json`, which has the colors of the old `PartObjectColor*` palettes; the meshes it is drawn on; the level it glows like), a render style (`RenderPartObjectBody` recipes; identical recipes share one name) and an item effect (the item branches of `RenderPartObjectEffect`). Effects that depend on the item level (+7 glow, excellent, ancient) stay generic code, and the entries for monsters and NPCs stay in code. The named looks are code for now; D25 moves what they are made of into data. |
| D25 | Looks in data | Items keep naming their looks; what a named look is made of moves from code into data (phase 13). Skills, monsters and NPCs use the same look format, so the decision lives in the [roadmap](2026-09-29-data-driven-content-roadmap-design.md). |
| D26 | Shared definitions | Defined once with a name and referenced by it; editors show where a definition is used and warn with that list before a shared definition changes. The decision lives in the [roadmap](2026-09-29-data-driven-content-roadmap-design.md). |
| D68 | Ground labels | The name above an item on the ground takes its colour, background and bold from a **label style**, a named look in `Data/Items/GroundLabels.json` (D26). An item picks its styles with `groundLabels` in its item file, next to `nameLines`: the first entry whose `levels` hold. It is a client-only field that the exchange leaves out (D22). Items without one use the file's **standard rules**, today's level ladder as data: the first rule whose `when` holds gives the style, with the tooltip's conditions. The first styles are named by role (`ticket`, `seal`, `jewel`, `giftBoxRed`, …) and keep today's looks. The text stays code: the name line, Zen's amount, the set name and " +Skill +Option +Luck". Italic waits until the fonts have it. |
| D69 | Colors window | MuEditor has one **Colors** window with a tab per use: glow colours (D24), ground label styles (D68) and tooltip colour groups. Each tab keeps its own list, because the values differ: a glow tints a model, a label style is a text look, and a tooltip colour group names one of the 14 theme colours the RmlUi themes draw. One panel in code serves every tab, with D26's "used by" and the list before a change. Windows ▸ Colors opens it, and so do the group entries (Items ▸ Glow Colors, Items ▸ Ground Label Styles) on their own tab. |
| D70 | Conditions in the editor | Conditions are picked, not typed. Each one is a chip from a menu by kind (level, state, value, tag, class), with a description for each entry. The lists come from the loader's own, so the editor can't write a name the loader refuses. A **saved condition** is a combination under a name, in `Data/Items/Conditions.json`, used as `"@name"` in any `when`. It can't use another saved condition. New states, values and tags still need code. The same picker serves label rules and tooltip lines (#815 step 6). |
| D71 | Item Tooltip | One item tool, **Item Tooltip**, shows how the selected item is presented: its ground label with D68's rules, and from #815 step 6a on its full tooltip. The preview has item controls (level, options, excellent, ancient, sockets, expired) and hero controls (class, level, stats). It draws the built lines in the editor, each with where it comes from (name line, description line, layout line, code). Next to them, the game's own tooltip box shows the previewed item in the legacy or modern theme. |
| D21 | New item groups | *To discuss again when we reach phase 12.* Items may move into new groups (e.g. 16 = jewels, 17 = orbs) for the new client, while original Season 6 clients keep the old ids. Moved items keep their original id as a legacy id; OpenMU's Season 6 item serializer sends the legacy id, a serializer for the new client sends the new id. Planned after phases 3, 4 and OpenMU PR A, when little code depends on group numbers any more. |

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

No code in the repo loaded these files, and `git log -S` finds no loader
in the history either. They were removed once the item data held what
they describe (#815, step 5); the git history keeps them. They use the usual bmd encoding (`BuxConvert` XOR
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
three languages. They were the input for data-driven tooltips (D15): #815
moved their texts and names into the items' `description` and `nameLines`
and into `Data/Items/Tooltips/Tooltip.json`.

Extra item bmd copies that no code loaded (`Data/Local/Item.bmd`,
`Eng/item.bmd`, `Eng/item_eng_old.bmd`, `Eng/item_eng_orig.bmd`,
`Por/item.bmd`, `Por/item_eng_old.bmd`, `Spn/item.bmd`) were removed on
this branch (D16).

### Server (OpenMU)

- `ItemDefinition` (`src/DataModel/Configuration/Items/ItemDefinition.cs`):
  `Group`, `Number`, `Width`, `Height`, `Durability`, `Value`, `DropLevel`,
  `MaximumItemLevel`, `ItemSlot`, `Skill`, `QualifiedCharacters`,
  `Requirements`, `BasePowerUpAttributes`, `PossibleItemOptions`,
  `PossibleItemSetGroups`, `MaximumSockets`, `ConsumeEffect`,
  `MaximumDropLevel`, `PetExperienceFormula`, `DropItems`, the flags
  `IsAmmunition`, `IsBoundToCharacter`, `IsQuestItem`, `DropsFromMonsters`,
  `StorageLimitPerCharacter`, and since PR A the six rule flags
  `IsTradable`, `IsDroppable`, `IsStorable`, `IsSellableToNpc`,
  `IsPersonalStoreSellable` and `IsRepairable`.
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
  item. The admin panel can download the whole configuration as JSON and
  back up the database; there is no export or import of single items yet
  (phase 7, section 6).

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
- One file per item group (D7): `Group00_Sword.json` … `Group15_Etc.json`,
  named after the `ITEM_GROUP_*` constants; each item's names in all
  languages are part of the item (D17). Files for other
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

- invalid JSON, a missing or newer `formatVersion`, an invalid `group`
- `group` outside 0–15 or `number` outside 0–511
- duplicate `(group, number)`
- missing English name
- a value outside its type's range (e.g. a byte field above 255); width
  and height 0 are allowed, 12 Season 6 items have them
- a name containing `||`
- later phases: a link to something that does not exist (unknown tag or
  flag, option group, skill, set or option definition)

Warnings (data works, but is probably wrong):

- unknown fields (ignored by the game)
- a name longer than 49 characters (cut in the game)
- a name in a language the game has no UI texts for (phase 5: it is never
  shown)
- later phases: combinations that do not make sense

Missing translations are not warnings at load: the translations tool of the
editor counts them, and Save Items lists them per language (phase 5).

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
| Server flags | ammunition, quest item | yes; bound to character, drops from monsters and storage limit are server-only |
| Rule flags | tradable, droppable, storable, sellable to NPC, personal-shop sellable, repairable | yes (OpenMU has them since PR A) |
| Categories/tags | wing, flying, mount, pet, potion, jewel, ticket, cash shop, event, … | client-only at first; shared where OpenMU gets matching data |
| Model | model file, texture folders, cloth flag (in the model files, D22) | client-only |
| Rendering | inventory offset, rotation and scale; ground rotation and scale; glow color, render style, effects (D24) | client-only |
| Tooltip texts | `description` (D15) | client-only, in the item files; the exchange file leaves them out |

### 4. Rule code on top of data

- Keep behavior in code and move item lists into data. Example:
  `IsTradeBan(item)` stays, but reads the `tradable` flag and the item's
  tags instead of `switch` lists.
- Every move must keep the exact item set, checked as in #625 by expanding
  the data back to item ids and comparing with the hardcoded version.
- Named enum constants remain for items that code must refer to directly
  (special behavior); lists of items become data.

### 5. Names, translations and logging

- In memory, item names are OpenMU-style `LocalizedString`s, so names from
  the server can be taken over directly. On disk, `name` is an object of
  names by language in the item files (D17); the OpenMU exchange writes it
  in the `LocalizedString` format.
- The name in the selected language is shown; missing translations fall back
  to English.
- Logs always use `<English name> (<group>,<number>)`, never the translated
  name. One helper (e.g. `ItemLogName(item)`) is used everywhere.
- Stats are no longer duplicated per language: one data set, only names are
  translated.

### 6. OpenMU sync (phase 7)

- **The server is authoritative.** The client never writes to the server,
  not even in editor mode. Item data moves by file in both directions, and
  MuEditor can also download it from the server (Update from server).
- **What moves:** the item fields both sides have, and the item formulas:
  everything in `Stats/ItemStats.json` except the profile rules, so a
  formula OpenMU changes reaches the client too.
- **Every import shows a diff** (added, changed and removed items and
  formula entries, field by field) that the user checks by hand; only the
  checked rows are applied, after a confirmation that says what changes.
  The same holds for OpenMU's import page. An export shows no diff: it
  writes the client's data, and the side that imports the file shows what
  it changes (a diff there and one at export only confused).
- Item sets, drop settings and option definitions get exchange files of
  their own later (D11, D14).
- Phase 7 has five steps, one PR each: **1** the exchange file with Export
  and Import in MuEditor (MuMain); **2** the item formulas as data in OpenMU;
  **3** OpenMU's read-only export endpoint; **4** Update from server in
  MuEditor (MuMain); **5** OpenMU's import page. Steps 2, 3 and 5 are phase
  B's part.

**The exchange file.**

- One UTF-8 JSON file in the layout of the item files: fixed key order,
  items sorted by (group, number), LF line endings.
- Header: `formatVersion` (1), `source` (`"MuMain"` or `"OpenMU"`), `server`
  (only in a file from OpenMU: an id its export creates once per
  installation and keeps, step 3; the id of OpenMU's game configuration
  cannot serve, as its initialization gives every installation the same
  one) and `gameVersion` (`"season6"`; a file of another version is
  refused, D8).
- `formulas`: the item stat file without its `formatVersion` and
  `profileRules`: `dropLevelBonus`, `tables`, `default` and `profiles`, with
  the keys and meanings of `Stats/ItemStats.json`.
- `items`: each item with the shared fields, in the keys and meanings of the
  item files (section 3, `docs/item-data.md`):
  - `group`, `number`, `name`, `width`, `height`, `slot`, `level`,
    `durability`, `magicDurability`, `skill`;
  - `damageMin`, `damageMax`, `attackSpeed`, `walkSpeed`, `magicPower`,
    `defense`, `blockRate`, `resistances`;
  - `requirements`, `requirementsByLevel`, `classRequirements`;
  - `sellValue`, `buyPrice`;
  - `tradable`, `droppable`, `storable`, `sellable`, `personalShopSellable`,
    `repairable`;
  - the tags `ammunition`, `secondClassQuestItem`, `thirdClassQuestItem`;
  - the formula fields `statProfile` (the profile the item uses: the one its
    rules give or the picked one; `"default"` for the default; always
    written), `excellentBonus` and `excellentRaisesLevel`.
- Not in the file: what is client-only (the model and display data, D22;
  `description`; the other tags, `wingTier`, `weaponKind`, `twoHanded`,
  `potionPriority`, `attackType`, `magicDefense`, the rental flags) and what
  is server-only (`DropsFromMonsters`, `MaximumDropLevel`, `MaximumItemLevel`,
  `IsBoundToCharacter`, `StorageLimitPerCharacter`, `PetExperienceFormula`,
  `ConsumeEffect`, `MaximumSockets`, the item options and sets, the drop
  items and the power-ups the client has no field for, e.g. `CanFly`).
- The file uses the client's names and meanings; OpenMU converts to and from
  its model (steps 3 and 5). Its export and import report the cases where
  that is not exact:
  - hand slots: OpenMU gives one-handed weapons a Dark Knight, Magic
    Gladiator or Rage Fighter may use both hands ({0, 1}); the file has
    `mainHand`, and OpenMU's import gives {0, 1} by the same rule;
  - OpenMU's one `Value` is the client's `buyPrice` in groups 12 and 15 and
    its `sellValue` elsewhere (a byte in the client);
  - OpenMU's `Durability` of an item without a slot is its stack size, and
    its `Skill` of a scroll or orb is the skill it teaches; neither is
    exchanged;
  - OpenMU's drop level is a byte (the Horn of Fenrir has 300 in the client);
  - OpenMU has one `IsQuestItem` for the client's two tags
    `secondClassQuestItem` and `thirdClassQuestItem` (the same eight items);
  - a class set with a gap, a requirement by level that goes down, magic
    power's rounding (OpenMU halves each part, the client the sum), the
    `@src` and `@stamp` parts of OpenMU's names and its `;` level variant
    names (three items), and a (group, number) that exists twice on the
    server (a locked row of the import, which leaves it as it is; an
    export replaces such a file after its confirmation).
- A field left out has its default, as in the item files. A reader warns
  about keys it does not know and refuses a `formatVersion` it does not
  know; the version only goes up for a change that older readers would
  misread.

**The orbs, crystals and scrolls** (the 44 items of groups 12 and 15 that
have stat requirements and no slot) keep their requirements as the values
the game asks for, as OpenMU stores them, instead of a weight that the drop
level formula grows. Their stat profiles write `"base"` for the stat
requirements: the `scroll` profile of group 15, and a new `orb` profile with
a rule of its own for group 12. The values are converted once; a test pins
them and checks that the game asks for the same at +0 (above +0 the value
stays as it is, as on OpenMU; the game shows these items only at +0). The
Orb of Summoning keeps its energy by level. The bmd files of the original
client hold weights for these items, so Import from bmd turns them into the
values with the original client's formulas, and Export as bmd and Export as
S6E3 turn the values back into the smallest weight that gives them (the
nearest one, with a note, when none gives a value exactly). Which
requirements convert follows the item's stat profile, as in the game: one
the profile keeps as it is while the default grows it.

**The Item Sync window** (MuEditor, `_EDITOR` builds only; the Item Sync
tool of section 9):

- **Export…**: a save dialog picks the file, and the client's items and
  formulas are written into it, without a diff. A file that is there is
  replaced after a confirmation with what changes in it (items added,
  changed and removed, formula entries that change; or that it cannot be
  read as an exchange file); a file that has the client's data already is
  left as it is.
- **Import…**: an open dialog picks the file; the diff compares it with the
  client's data, and the checked rows are applied to the client after a
  confirmation with what they change (items added, changed and removed,
  formula entries; what is not checked or not listed stays as it is).
- **Update from server** (step 4) downloads the server's file and goes on as
  Import.
- **Compare two files**: a read-only diff of two exchange files (e.g. a
  client export and a server export), what changes from file A to file B,
  in every language; its rows can be marked as known differences. Only the
  client has it: OpenMU's import page compares a file with its live data,
  the client's Import a file with the client's data, and a comparison of
  two files is a tool for the one who edits the items.
- The file dialogs are SDL3's (`SDL_ShowOpenFileDialog`,
  `SDL_ShowSaveFileDialog`), on every platform. Their callback can run on
  another thread, so it only hands the path to the window, which reads it
  in its next frame. On Linux they need the desktop portal or zenity.
- **The diff** lists, for items, the added ones, the changed ones with their
  fields (client value, file value) and the removed ones; for the formulas,
  the drop level bonus, each term of the default and of each profile, and
  each table. A checkbox per field, with a toggle per item and per column.
  Checked by default: the differences of the file. Unchecked by default:
  removals, inexact fields and known differences. (Local edits and
  conflicts come with the last server state in step 4.)
- **Known differences** are the items' fields and formula entries meant to
  differ between the client and a server (e.g. the Dark Horse is repairable
  on OpenMU on purpose). They are kept in the repo, in
  `Data/Items/Sync/KnownDifferences.json` (the game does not read it), so
  they are shared and survive a new build folder; the diff shows them
  greyed and unchecked, and a row can be added to the list from the diff.
- **Names:** only the client's UI languages are taken from a file; a
  language the file lacks stays as it is, and no language is removed.
- **Stat profile:** a file's profile that the item's rules give leaves the
  item following its rules (also when the rules give the default; only a
  pick is stored as `"default"`); another one becomes a pick (section 9).
  Export writes the profile the item uses.
- **Added items:** `twoHanded` follows the rule all shipped items follow
  (groups 0 to 5, a hand slot, width 2 or more), the weapon kind follows the
  group as for Add item, then the profile is compared with the rules. The
  row flags what the rules need and the file cannot give: a wing tier, or
  the tags `cape`, `sceptre` and `summonerBook`. The other client-only
  fields take their defaults. A number that has a model entry but no item
  keeps that model; otherwise the row says the item has no model, and the
  Item Browser's "without a model" filter lists it (Item Checks gets no
  check for it: 33 shipped items have no model entry, several drawn by
  code).
- **Removed items** remove the item and its model entry, as Remove item
  does, with its "used by" list. An item the game holds (worn or in an
  item window) is not removed, and its size and slot stay; a new item of a
  type the game holds (the client had no data for it) is not added. The
  import checks this again when it is applied.
- **Apply** (after its confirmation): the changed items and the changed item
  stat file are checked first, with the checks of a save: a row with an error is unchecked and
  shows the message. Then everything checked is one step of the undo
  history: the item stat file first, so the items' profiles follow the new
  rules, then the items. The size and slot of a held item are left out of
  the change (the item tools refuse a whole change that would move them).
  Save Items writes the files, as for any edit.

**Update from server** (step 4):

- `GET <admin panel URL>/api/items/exchange` (step 3), with the API key in
  the `X-Api-Key` header. The key comes from the environment variable
  `MU_OPENMU_API_KEY` and is never written to a file or a log; the user
  creates it on OpenMU's API keys page (the Viewer role is enough).
- The URL is a setting of its own in `MuEditor/MuEditor.ini` (`[OpenMU]
  AdminPanelUrl`, read and written by the editor's settings like its other
  sections), never in `config.ini`, which ships to players. https is
  required except for `localhost`, `127.0.0.1` and `[::1]`; plain http on a
  local network is an opt-in (OpenMU's all-in-one setup serves plain http
  on port 80).
- The download runs on a worker thread with libcurl (already linked into
  every build; its global init runs once before the thread starts), with
  timeouts and a cancel; the key stays out of curl's verbose output and the
  editor's log. The window shows its state and the errors (no URL, no
  connection, 401 key, 403 role or setup mode, 404 no endpoint, 429 too many
  requests, an unknown format).
- The last file of each server is kept as its last server state,
  `MuEditor/ItemSync/<server>.json`, on the developer's machine. It makes
  the diff three-way: a field only the server changed is a server change
  (checked), a field only the client changed is a local edit (unchecked,
  "to export"), a field both changed is a conflict (unchecked). The request
  sends its ETag, so an unchanged server answers with nothing to download.
- The remove warning names the items that were in the last server state
  (section 7).

**OpenMU** (phase B, separate OpenMU PRs):

- **Step 2, formulas as data:** the named stat profiles, the level bonus
  tables (data already), the default terms and the drop level bonus become
  part of OpenMU's game configuration, and each item definition names its
  profile and gets `excellentBonus` and `excellentRaisesLevel`.
  `ItemExtensions.GetRequirement` and `ItemPowerUpFactory` read them, and so
  does the price calculation, which has a copy of the drop level bonus of
  its own (`ItemPriceCalculator`). Their defaults give exactly what today's code computes (a test for
  every item, item level, excellent and ancient), with a migration and an
  update plug-in. The two sides compute differently today in a few places
  (see the phase 3 mapping: the order of the ancient and excellent drop
  level bonus, leadership, the option level's strength, the level
  requirement per item level, magic power's rounding); the first sync shows
  them as formula differences, and each is decided then.
- **Step 3, the export endpoint:** `GET /api/items/exchange` in the admin
  panel, read only. It takes the API key scheme alone, with a policy of its
  own (the existing ones also accept the admin panel's login cookie), and
  refuses while the server has no admin user yet: OpenMU lets every request
  through then, its API keys page included, so anyone could make a key. Requests are
  limited per key and per address, and one export is built at a time (429
  otherwise). The export is built once and kept in memory with an ETag, and
  rebuilt when the configuration changes, so a repeated request costs
  nothing worth noting; it reads a fresh configuration context, not the
  admin panel's shared one. A page of the admin panel offers the same file
  as a download, for Import without a connection. In the all-in-one setup
  the admin panel runs in the game server's process: it should not be
  reachable from the internet (localhost, a VPN, a firewall), or it runs as
  a host of its own (the distributed setup).
- **Step 5, the import page:** an admin panel page (Administrator role)
  takes the client's export and shows the same diff. The checked rows are
  applied in one configuration context. A new item takes the server-only
  values (maximum item level, options, sets) from a template item picked
  on the page. Names keep OpenMU's `@src` and `;` parts. A removal fails
  while something still references the item (existing behavior).

**Later:** the server's item data reaching every player at game start or
in the launcher is a follow-up idea for a service of its own, hosted apart
from the game servers
([follow-up ideas](2026-10-06-possible-follow-up-ideas.md)).

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
| **Model and visuals** | Model file picker for `Data\Item\*.bmd` with 3D preview, texture folder, inventory and ground display values, cloth flag, and glow, render style and effects chosen from lists (D24), each with the list of the other items that use the same look. |
| **Colors** (D69) | One window for the named colours, a tab each: glow colours, ground label styles and their standard rules, tooltip colour groups. Each entry shows the items that use it. |
| **Item Tooltip** (D71) | The selected item's ground label and its label rules; from #815 step 6 on, its full tooltip preview and then its tooltip lines. |
| **Look editor** (phase 13) | The shared look editor of the [roadmap](2026-09-29-data-driven-content-roadmap-design.md), opened for the looks of the selected item: its building blocks with a live preview, bone picking, the items that use a look and the warning before a shared look changes. |
| **Item Translations** | Items × languages grid (every UI language, English first), the missing names counted per language, a filter for them, "same as English" for names that do not change. Built in phase 5, before the other tools. |
| **Item Sync** (phase 7) | Export, Import and Update from server of the exchange file; Import and Update from server with a diff checked by hand (section 6). |

Common features: validation before saving (duplicate ids, missing English
name, missing model file), and undo for the current session.

## Phases (one PR each)

**Side:** *Client* = MuMain only, *Server* = OpenMU only, *Both* = changes
in both repos (as separate PRs, one per repo).

| Phase | Name | Side | Repo / PR | Depends on | Summary |
|---|---|---|---|---|---|
| 0 | Design document | Both | MuMain | – | Agree on the design (this document). |
| 1 | Item database | Client | MuMain | 0 | `ItemDefinition` model and flat in-memory table, built from the loaded `Item_<lang>.bmd` data; English names for logs; log-name helper; load-time log. No behavior change. |
| 2 | Data file format and names | Client | MuMain | 1 | JSON per group becomes the only item source and the database becomes the source for `ItemAttribute[]`; translated names in the UI locale; loading, writing and validation rules; automated data test; bmd import (with repair) and export in MuEditor; editor edits go into the database right away. |
| 3 | Rules and categories into data | Client | MuMain | 2 | Flags and tags replace the hardcoded lists; client ↔ OpenMU rule mapping (input for A). |
| 3b | Remaining item lists | Client | MuMain | 3 | The lists of item ids outside the phase 3 files that repeat data that exists or need new data (see "Item-specific code that is left"), checked like phase 3. |
| 3c | Item stats by level | Both | MuMain (+ mapping for A) | 3 | How damage, defense, magic power, blocking and requirements change with the item level, and the bonus of excellent and ancient items, become named level bonus tables and formula parameters in data (D43), matching OpenMU's `ItemLevelBonusTable`; checked against the old formulas for every item and level. |
| A | Server rule fields and checks | Server | OpenMU | 3 (mapping) | New `ItemDefinition` fields or tables, migration, Season 6 values, update plug-in, enforcement in player actions. |
| 4 | Models into data | Client | MuMain | 2 | Model files per item group (D22), model slots stay `MODEL_ITEM + type` (D23, own model slots). One PR per part: **4a** model files and textures (`OpenItems()` / `OpenItemTextures()`), **4b** inventory and ground display, **4c** render effects (D24) in three parts: **4c1** glow, **4c2** render styles, **4c3** item effects. |
| 4d | Shared models | Client | MuMain | 4 | Models as entries of their own that items reference by name, so items can share a model (D23, shared models). Two PRs: **4d1** shared model entries, each file opened once; **4d2** the conversions between item types and model slots in one place, and shared model data freed by its last user. |
| 4e | Clear model loading errors | Client | MuMain | 4 | One message for missing model files and textures of item models that names the item, the model entry, the texture and the searched folders. |
| 5 | Translation tooling | Client | MuMain | 2 | Translations editor (items × languages), missing names counted in the editor. The names themselves moved to phase 2 (D17). Done before phase 6. |
| 6 | Editors | Client | MuMain | 2–5 | Focused MuEditor tools (section 9) on the rules of D26, including add/remove items and picking the looks of an item from lists. Takes over phase 5's item list. Done. |
| 7 | Item sync, client side | Client | MuMain | 2, 6 | The exchange file of items and item formulas; MuEditor's Export, Import (step 1) and Update from server (step 4), the imports with a diff (section 6). The client-only fields of the item files (tags at first, `description`) are left out. |
| B | Item sync, server side | Server | OpenMU | 7 (file format) | The item formulas as data (step 2), the read-only export endpoint (step 3) and the import page (step 5), as steps of phase 7 (section 6). |
| 8 | Data-driven tooltips | Client | MuMain | 2, 5 | Tooltip JSON converted from the `ItemTooltip*` files; `RenderItemInfo()` reads data; tooltip editor. Moved earlier if needed. In steps (#815): an item's own texts go into its item file (`description`, D15), shared texts and layouts into a tooltip file. Steps 1–5 done (#813–#836, #863). Left: the item cases and the class line list still in code, the ground label colours (D68–D71, six PRs), and the tooltip editor (#815 step 6); see the phase details. |
| 9 | Item option definitions | Both | MuMain + C | 2 | Option groups in their own JSON files matching OpenMU's `ItemOptionDefinition`; exchange file and editor. |
| 10 | Item sets | Both | MuMain + C | 2 | Item sets in JSON matching OpenMU's `ItemSetGroup`; exchange file and editor; based on the `item-set-editor` branch. Moved earlier if needed. |
| C | Option and set sync, server side | Server | OpenMU | 9, 10 | Admin panel import/export for the option definition and item set exchange files. |
| 11 | Remaining item files | Both (per file) | MuMain, OpenMU as needed | 2 | `ItemAddOption`, `SocketItem`, `Mix`, `pet`, drop settings; one phase each, order decided later. |
| 12 | New item groups *(to discuss again)* | Both | MuMain + D | 3, 4, 4d, A | Move items into new groups for the new client; level variants become items of their own; legacy ids for the original client (D21). |
| D | Legacy item ids, server side *(to discuss again)* | Server | OpenMU | 12 | Legacy id on item definitions, mapping tool, Season 6 serializer sends legacy ids, serializer for the new client. |
| FX1 | Effect catalogue *([roadmap](2026-09-29-data-driven-content-roadmap-design.md))* | Client | MuMain | 4c | Every effect, particle, lightning and sprite type gets a name and its creation values in `Data/Effects/`; behavior stays code. |
| 13 | Looks in data | Client | MuMain | 4c, 6, FX1 | What the named render styles and effects are made of moves from code into `Data/Effects/` as building blocks (D25), with the look editor in MuEditor (preview, bones, the items that use a look). |
| 14 | UI text translations | Client | MuMain | 5 | A MuEditor window for the `.resx` UI texts (Game, Dialog, Editor, Metadata): keys × languages with the missing texts counted per language and file, written back to `src/Localization`, with placeholder checks. Builds on the Item Translations window. |
| 15 | Adding a language | Client | MuMain | 14 | Add a UI language from MuEditor: its code and its name in its own language. Creates its `.resx` files; the game's language list and the language names come from the texts instead of code, so after the next build the language is in the game, the editor and the translation windows. |
| 16 | Cleanup | Client | MuMain | all item phases | The glow of the Phoenix Soul inventory model; what later areas still need moves to the roadmap; then remove this document. |

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
   view).

   As built: names are `LocalizedString`s from all three bmd files and
   follow the UI locale (D17); the import repairs the legacy data (D18);
   every editor change is copied into the database at once (D19), which
   closes the phase 1 sync gap. Save writes only valid data and only
   changed files. Loading the 16 files takes about 11 ms in a Release
   build (bmd: about 2 ms), so no binary cache is needed. Usage is
   documented in `docs/item-data.md`.

   Still true: `Build()` and the editor rewrite definitions in place, so an
   `ItemDefinition*` kept across an editor change sees changed content.
   Code that keeps pointers must not rely on them staying unchanged while
   the editor is used.
3. **Rules and categories into data**: flags and tags replace the hardcoded
   lists in `ItemCategories`, `TradeRestrictions` and `ShopRestrictions`.
   The resulting item lists are verified to be identical. Includes the
   client ↔ OpenMU rule mapping.

   As built:
   - Item fields: `tags` (a list of names, a bitmask in memory),
     `wingTier` (`small`/`first`/`second`/`third`), `weaponKind`
     (`sword`/`glove`/`axe`/`mace`/`spear`/`bow`/`crossbow`/`staff`/`book`/`shield`,
     since 3b-2 and 3b-3), the rule flags
     `tradable`, `droppable`, `storable`, `sellable`,
     `personalShopSellable`, `repairable` (default `true`, only `false`
     is written), the rental rules `droppableWhileRented`,
     `personalShopSellableWhileRented`, `sellableWhenRentalExpired`, and
     `slot` written by name (`mainHand`, …, `wings`, …; numbers from
     phase 2 files are still read, with a warning). Wings need no tag: a
     wing is an item with the slot `wings`.
   - Items without a definition allow no action.
   - The rule functions stay and read the data through a small table next
     to the definitions (`ItemDatabase::HasTag`, `IsAllowed`, `GetSlot`,
     `GetWingTier`, `GetWeaponKind`): one array read plus a bit test.
   - What stays in code: exceptions that depend on the item level, the
     durability or the player, with named constants, see
     `docs/item-data.md` → Rules; and the lists that are only used for
     drawing and tooltips (`ItemDisplayCategories.cpp`), which phases 4
     and 8 replace with model and tooltip data.
   - `IsPartChargeItem` became the tag `cashShop`, flattened. Only the
     old rules used it, so no client code reads the tag now; it stays as
     information for the editors and the OpenMU exchange.
     `IsRareItemTicket` was only part of it and is gone.
   - `Check_ItemAction` and the separate repair lists in `RepairAllGold`
     and `RenderRepairInfo` became the rule flags too.
   - Verified with a one-time test (in the PR history, then removed) that
     compares the old and the new code for all 949 items, every category
     function (also with out-of-range and model ids), and every rule for
     item levels 0–15, bought/rented/expired, durability 0–2, ancient and
     excellent, and GM or not. The only differences are the two
     intended behavior changes in their own commits: the personal shop
     rule for rented pets (the old condition only applied to the Demon),
     and "repair all" now counting the same items as single repairs (it
     added a price for damaged items that cannot be repaired one by one,
     e.g. transformation rings and the Little Warrior's Cloak).
   - Level variants (Box of Luck +13 = Heart of Dark Lord, Rena +3 = Sign
     of Lord, Wizard's Ring levels) stay code exceptions; phase 12 makes
     them items of their own.
   - Not moved in this phase (other hardcoded lists found on the way):
     the right-click use list (`TryConsumeItem`), price exceptions
     (`ItemValue`), the transformation ring lists (`ChangeRingManager`),
     the packed jewel list (`COMGEM`) and the tooltip class requirement
     list (`IsRequireClassRenderItem`, phase 8).

   Client ↔ OpenMU mapping (input for A):

   | Client data | OpenMU today | For PR A |
   |---|---|---|
   | `slot` | `ItemDefinition.ItemSlot` (an `ItemSlotType` with its slot numbers) | Exchange maps the names to slot types: `mainHand` ↔ types containing 0, `offHand` ↔ 1, …, `ring` ↔ 10/11 |
   | `wingTier` | None; wing values are power-ups and options per wing | Client-only for now; phase 9 (options) may replace it |
   | `weaponKind` | Item groups: swords 0 (the Rage Fighter's gloves get `Stats.IsGloveWeaponEquipped` instead of the sword ones), axes 1, maces 2 (`ItemGroups.Scepters`), spears 3, bows and crossbows 4 (split by slot into `Stats.IsBowEquipped` and `IsCrossBowEquipped`), staffs 5 (`StaffItemGroup`; the books get `Stats.IsBookEquipped`, the summoner's sticks `IsStickEquipped`), shields 6 (`ShieldItemGroup`, `Stats.IsShieldEquipped`) | Derived from group, slot and class (ammunition has none); no new OpenMU field needed. OpenMU's sticks have no kind of their own in the client (they are staffs) |
   | `sceptre`, `magicSword` | `ScepterRise` and `IsScepterEquipped` for group 2 items with Force Wave; `StaffRise` for group 0 items with Power Slash and magic power | Derived from skill and group; no new field |
   | `transformationRing` | `IsTransformationRing`: a base power-up on `Stats.TransformationSkin`, on the same eight rings; read for durability and Chaos Castle only | Derived from the power-up; no new field. The client's ring rules (one dragged onto a ring slot, no Kanturu entry, no Icarus except with the panda and skeleton rings) have no server side; OpenMU's Icarus asks only for `Stats.CanFly` |
   | `shieldPotion` | `ItemConstants.SmallShieldPotion`, `MediumShieldPotion`, `LargeShieldPotion` | Client-only, like the other potion tags |
   | `potionPriority` | None; OpenMU keeps a potion's strength in its consume handler plug-ins, and its offline leveling (`HealingHandler`) lists the healing and mana potions largest first in code | Client-only for now |
   | `luckyItem` | None; OpenMU's Season 6 data has no lucky set armour (numbers 62 to 72 of groups 7 to 11) | Client-only until OpenMU has them |
   | Level bonus tables of `Stats/ItemStats.json` | `ItemLevelBonusTable`, attached per item through `BasePowerUpAttributes`: `damageIncreaseWeapons` = Damage Increase (Weapons), `defenseIncreaseArmors` = Defense Increase (Armors), `defenseIncreaseShields` = Defense Increase (Shields), `defenseRateIncreaseShields` = Defense Rate Increase (Shields), `defenseBonus1stWings` = Defense Bonus (1st and 2nd Wings), `defenseBonus3rdWings` = Defense Bonus (3rd Wings) | Same values. `magicPowerIncrease` (T) is what OpenMU's rise tables are made of: Staff Rise even = ⌊T/2⌋ + 2L and odd = ⌈T/2⌉ + 2L, Scepter Rise the same without 2L. The client halves the sum of base, excellent and ancient part, so its table cannot be chosen by the base's parity per item as OpenMU does; the exchange expands it. `defenseBonus2ndWings` (2 per level up to +9) has no OpenMU table: OpenMU gives second tier wings and the capes the 3 per level table. `magicDefenseIncrease` is client-only (no shipped item has magic defense) |
   | Excellent and ancient terms, `dropLevelBonus` | Hard-coded in `ItemPowerUpFactory` (with a TODO to make them configurable) | Client-only data; the exchange compares them with OpenMU's numbers. Same formulas except magic power's rounding (the client halves the sum, OpenMU each part) |
   | `statProfile` | None; OpenMU picks the tables per item | The exchange expands the profile into the item's per-stat tables |
   | `excellentBonus` (Chaos Dragon Axe 15, Chaos Nature Bow 30, Chaos Lightning Staff 25) | None; OpenMU uses the formula | Client-only; a difference to report |
   | Requirement terms of `Stats/ItemStats.json` (`levelPerItemLevel`, `excellentLevelAdd`, the stat terms, `strengthPerOptionLevel`) | The base values are the item's `Requirements`; `ItemExtensions.GetRequirement` and `CalculateDropLevel` compute the rest in code | Same stat formula `20 + base * (drop level + 3 * level) * 3 / 100` (energy `* 4`) and the same book formula. Differences to report: OpenMU adds the ancient +30 before the excellent +25 (the client the other way round), computes command like the other stats (the client keeps the base), adds the option level's strength only when there is a strength requirement and 5 per level on armour, 4 on other items (the client 5 on all of groups 0 to 11 except ammunition), and neither grows the level requirement with the item level nor adds 20 for excellent items. Scrolls and orbs are not wearable in OpenMU; their requirement is computed once into the item (`OrbRequirements`), for scrolls from the level requirement like the client's `levelRequirement` basis |
   | `excellentRaisesLevel` = false | None; OpenMU has no excellent +20 | Client-only |
   | `requirementsByLevel` (Orb of Summoning energy, Transformation Ring level) | The ring: a second level requirement of 50 with `MinimumItemLevel` 3 (`TransformationRingConfiguration`). The Orb of Summoning: no energy requirement, maximum item level 6 | The exchange maps the ring's list to the two requirements; the orb's energy list is a difference to report |
   | `tradable`, `storable`, `personalShopSellable`, `sellable` = false | Only `IsBoundToCharacter`, which blocks all four at once (sell with a durability exception) | Separate rules, e.g. one `[Flags] ItemRestrictions` column with the six names; enforce in `MoveItemAction` (trade, vault, personal shop) and `SellItemToNpcAction`; `IsBoundToCharacter` stays for "only the owner can pick it up" |
   | `droppable` = false | None (`DropItemAction` drops everything) | Enforce in `DropItemAction` |
   | `repairable` = false | None (`ItemRepairAction` repairs any item below its maximum durability) | Enforce in `ItemRepairAction` |
   | `droppableWhileRented`, `personalShopSellableWhileRented`, `sellableWhenRentalExpired` | None; OpenMU has no rental items | Client-only until OpenMU has rentals |
   | Code exceptions (level, durability, GM) | Levels: none | Level variants move with phase 12/D |
   | `ammunition` | `IsAmmunition` | 1:1 |
   | `secondClassQuestItem`, `thirdClassQuestItem` | `IsQuestItem` (+ `StorageLimitPerCharacter` 1) | Both map to `IsQuestItem` |
   | `flying` | `Stats.CanFly` power-up on wings, Dinorant and Fenrir | Compare: the client also counts the Dark Horse |
   | `jewel`, `mount`, `darkLordPet`, `guardianPet`, potions, socket items, … | Hardcoded `ItemConstants` checks | Client-only; OpenMU can take them over item by item |
   | `cashShop`, `valuable`, `gemJewelry`, `luckyItemTicket`, … | None | Client-only (UI) |

   **A (OpenMU):** new rule fields/tables with migration, initialization,
   update plug-in and server enforcement, based on the mapping above.
   (Done: OpenMU has had the six rule flags since MUnique/OpenMU#983, with
   these values, and gives the Dark Horse `CanFly`. The exchange of phase 7
   uses the table above with that.)

   **3b Remaining item lists (done).** The lists of item ids outside
   the phase 3 files that repeat data the items already have (tags, `slot`,
   `wingTier`, `storable`, the model and display files) or need new data.
   As in phase 3, the code asks the item data instead of naming items, and a
   one-time test compares the old and the new code for every item type and
   item model slot before the lists go; intended behavior changes, if any,
   get commits of their own. New tags that OpenMU checks in `ItemConstants`
   go into the mapping for A. An inventory (2026-10-17) found 52 places
   (more than the estimated 35: the lucky items, SD potions and
   transformation rings, and conditions over several weapon groups), done
   in four parts:
   - **3b-1** (done): 14 places that repeat data the items have: the mount
     and pet checks of Power Slash, Chaos Castle, the helper HP frame and
     the chat animation, the mana and healing potion searches
     (`ItemDatabase::GetItemTypesWithTag`), the ammunition's durability
     icon, the capes' defense option, the excellent durability bonus, the
     mix's wing flags and the staffs' magic durability; also the capes'
     defense per level in `CalcDefense` (phase 3c's code, but the same cape
     list as the defense option, which an edited `cape` tag would otherwise
     split). MoveHero's rider
     check for operating objects was always true and is gone; `CalcPartType`
     and `ITEM::Part` were written but never read and are gone. The Crywolf
     contract's horn check stays until the unformatted code around it is
     touched anyway.
   - **3b-2** (done): bows, crossbows, spears and shields. One
     `weaponKind` field per item (`spear`, `bow`, `crossbow`, `shield`;
     like `wingTier`, the kinds are exclusive and match OpenMU's item
     groups 3, 4 and 6, group 4 split by slot) instead of a tag per kind.
     13 places ask it: `CCharacterManager`'s bow and crossbow lists (four
     functions), the elf's arrows-only rule, the shield defense set
     option, a spear's reach (`Action`, the MU Helper) and Impale (skill
     icon and skill use), and the checks of a bow or crossbow with its
     ammunition in both hands (`CalculateDamage`, `GetAttackDamage`, the
     hit sound in `MoveCharacter`). Silver Bow-J (4/27) joined the bow
     lists in a commit of its own. The group ranges also counted the
     numbers of their group that have no item data, which the client does
     not know; the weapon kind does not. The bmd import and the item editor
     keep a weapon kind that its group gives matching the group and slot, so
     new weapons get one; the game reads only the field.
   - **3b-3** (done): swords, gloves, axes, maces, staffs and books. The
     weapon kinds `sword`, `glove` (the Rage Fighter's gloves, OpenMU's
     `IsGloveWeaponEquipped`), `axe`, `mace`, `staff` and `book` (the
     summoner's books, OpenMU's `IsBookEquipped`), plus the tags `sceptre`
     and `magicSword`. 21 places ask them: the sceptre list
     (`IsCepterItem`, its callers ask the tag), the magic sword checks
     (wizardry damage for the right-hand weapon, the left-hand weapon and
     the right one beside it, the character window's wizardry line, the
     tooltip's magic power and wizardry damage lines), the staff checks
     (`CalculateDamage`, `GetAttackDamage`, the character window's
     wizardry line, the tooltip's magic power line, the summoner's left
     hand, three knight skill checks in the skill list and one in
     `AttackKnight`, Death Stab, Death Cannon), Spiral Slash, the
     left-hand weapon of `CalculateMagicDamage`, both dual wield checks
     and the weapon `MoveHero` moves into an empty right hand; a staff
     branch of `CalculateDamage` whose two sides were the same is gone.
     In commits of their own: Master Scepter-J (2/21) joined the sceptre
     list; two staff ranges that ended with `<=` (and so took in the Small
     Shield 6/0) end before it; the two-handed magic swords count their
     damage option in `CalculateMagicDamage`'s checks with something in
     the other hand too (a combination the hands never allow); and
     `MoveHero` moves only a staff, so Book of Neil-J (5/40, a book by its
     kind that lacks the `summonerBook` tag the book-only code asks) stays
     in the left hand. With their own kinds, the gloves and books mean
     "sword or glove" for group 0 and "staff or book" for group 5 at the
     converted places.
   - **3b-4** (done): the lucky set armour, the SD potions and the
     transformation rings. The tags `luckyItem` (numbers 62 to 72 of groups
     7 to 11), `shieldPotion` (14/35 to 14/37; the Elite SD Potion 14/133
     stays a hotkey item of its own) and `transformationRing` (the eight
     rings of `ChangeRingManager`), and the field `potionPriority`, which
     says which potion of a kind the game takes first: the higher, the
     earlier, ties to the higher item type. The potion searches took the
     highest item type first (the largest potion), so a higher numbered
     potion tagged in the editor (Siege Potion, Elite Healing Potion)
     would have come before the Large Healing Potion; the shipped potions
     count up with their size, so the order stays. The places that ask
     them: the nine callers of `Check_LuckyItem` (repair costs, the
     maximum durability, the price, the excellent level requirement, the
     tooltip's stat requirements, the mix sources and two jewel checks)
     and the lucky item window's refinery; the transformation ring checks
     (`CheckChangeRing`, `CheckMoveMap`, `CheckBanMoveIcarusMap`, where a
     ring that is not the panda or skeleton ring bans Icarus); the mana and
     healing potion searches; and the items the hotkeys take
     (`GetHotKeyItemTypes`, which replaces `GetHotKeyCommonItem` and the
     ranges per key). `Check_LuckyItem`, the unused `IsLuckySetItem` and
     `CheckRepair` and the lucky set constants are gone. The lucky set
     range also counted 7/71 and 10/72, which have no item data.
   The 38 items named "…-J" (copies of existing items with no model, unknown
   to OpenMU; what "J" stands for is not known) followed their data; phase 6
   removed them, since no player could get one (server data marks them not
   droppable, and no cash shop or lucky ticket list has them). Two lists
   the inventory found wrong are fixed on their own, not in 3b: the
   Illusion Temple team skins (#669) and the trade warning (#671).

   **3c Item stats by level (done).** What the item level adds to damage,
   magic power, defense, blocking and the requirements, and the bonus of
   excellent and ancient items, was computed by `SetItemAttributes` and its
   `Calc*` functions in `ZzzInfomation.cpp`; the formulas are listed under
   "Item-specific code that is left" → Item stats by level. They became
   named level bonus tables and formula parameters in data (D43 of the
   roadmap), shared in the item exchange and matching OpenMU's
   `ItemLevelBonusTable` per stat. Checked like phase 3: a one-time test
   compared the old and the new values for every item at levels 0–15,
   normal, excellent and ancient. The differences 815b8828 and
   3d5b1adc6 brought into these formulas (blocking by the drop level, the
   excellent bonus for wings with a wing option and not for ancient items,
   the empty level requirement range from the Wing of Dimension on, the
   ancient defense's second base, the maximum damage bonus from the maximum
   damage) were fixed first, with #683. The level bonus above +9, added once
   from 2021 where the Season 6 client and OpenMU add it cumulatively, was
   fixed before this phase too (`GameLogic::Items::GetItemLevelBonus`), so
   the tables can match OpenMU's. The fork
   branch `fix-some-item-calculations`
   (January 2026, still in the old `Source Main 5.2` layout) holds fixes of
   these formulas after the Season 6 client and is compared when this phase
   starts. **A (OpenMU):** the mapping of the tables to
   `ItemLevelBonusTable`, and the formulas OpenMU does not have.

   Decided 2026-10-18 after comparing three shapes (rules over existing
   data, per-stat table references as in OpenMU, and stat profiles): **stat
   profiles**. `Data/Items/Stats/ItemStats.json` holds the level bonus tables
   (16 values or a step per level, named after OpenMU's tables where OpenMU
   has one), a `default` that writes every stat's
   table, excellent and ancient terms and the requirement terms once, and
   named profiles that write only what differs. Each item names its profile
   (`statProfile`); like `weaponKind` it is written on the items and kept
   matching their data by the bmd import and the item editor. What belongs
   to one item stays on the item: `excellentBonus`, `excellentRaisesLevel`
   and `requirementsByLevel`. Wing options move with the other options to
   phase 9. The old fork branch had nothing to take over except the Cape of
   Lord's defense of 15 as data; its other changes are in main already,
   fixed differently, or wrong (first tier wing steps from 25, Wings of
   Despair as third tier, a second +25 for excellent requirements).
   - Nine tables, the default and twelve profiles on 566 items (604 before
     the "…-J" items were removed): `weapon` 113, `armor` 327, `shield` 22,
     `sceptre` 10, `book` 3, `helper` 31, `scroll` 36, and `firstTierWing`,
     `secondTierWing`, `secondTierCape`,
     `thirdTierWing` and `smallWing` on 24 wings. The Horn of Fenrir names
     `"default"`: its data gives `helper`, but its level requirement does not
     grow. The other items name none and have the default; a left-out profile
     follows the data at the next bmd import or edit, and loading warns about
     an item that leaves out the profile its data gives (as item files saved
     before the field do). Of the items with the default, the orbs, crystals
     and scrolls of group 12, the Horn of Fenrir and the Transformation Ring
     have requirements:
     their level requirement does not grow with the item level, their stat
     requirements follow the default formula, and the Orb of Summoning's
     energy and the ring's level come from `requirementsByLevel`.
   - On the items: `excellentBonus` on the three chaos weapons,
     `excellentRaisesLevel: false` on the 89 items the excellent +20 left
     out (the Dragon Knight, Venom Mist, Sylphid Ray, Volcano, Sunlight and
     Storm Blitz armour, six weapons, the lucky items and the Horn of
     Fenrir), and `requirementsByLevel` for the Orb of Summoning's energy
     (30 to 500) and the Transformation Ring's level (20, then 50 from +3).
   - `GameLogic::Items::ComputeItemStats` computes the stats and
     requirements for `SetItemAttributes` and the item help table, which had
     its own copy of the formulas. Gone from the code: `CalcDamageMin`,
     `CalcDamageMax`, `CalcMagicPower`, `CalcSuccessfulBlocking`,
     `CalcDefense`, `CalcRequirements`, `GetExcellentAddValue` with the three
     chaos weapon ids, the shield group range, `GetItemLevelBonus`,
     `GetDropLevel`, the
     strength the option level added in four group ranges (now
     `strengthPerOptionLevel` in the profiles), and the dead `PrintItem` and
     `ItemConvert`.
   - In commits of their own: the Cape of Lord's defense of 15 became item
     data (the code wrote it into the item table, which the editor could
     then save; a bmd import of the original files gives it their 0); the
     bmd import and the editor's save check the stat profiles against the
     stat file; values stop at 65535 instead of overflowing; the help table stops the defense rate at 255 and shows the
     command requirement the item has; and the Dark Raven branch of the
     command requirement went, which compared the item type with a model
     number (15/115, no item).
   - The one-time comparison covered every item at the levels 0 to 15 and
     higher bytes, each excellent option, A and B ancient, option levels 0,
     1, 4 and 15, item types without data and values the editor can give;
     the help table too. The one difference: item types without data in
     groups 0 to 11 no longer get the option level's strength.
   - Done in phase 6: which item gets which profile was code
     (`DeriveStatProfile`) that named the profiles; it is now the ordered
     profile rules of the stat file (`profileRules`: conditions on group,
     slot, wing tier, weapon kind, tags and base requirements, the first
     match giving the profile). A one-time comparison checked the 14 shipped
     rules against the code for all 911 items, 844,800 combinations of the
     fields it read, the follow logic and the load warning. The writer is
     used by Item Formulas and Save Items.
4. **Models into data**: model and display data in their own files,
   `Data/Items/Models/GroupNN_*.json` (D22):

   ```json
   { "number": 5, "file": "Data/Item/Sword06.bmd", "textureFolders": ["Item"],
     "inventory": { "offset": [-0.02, 0.03], "rotation": [180, 270, 15], "scale": 0.0039 },
     "ground": { "rotation": [60, 0, -45], "scale": 1.0 },
     "glow": { "color": "gold", "meshes": [0] },
     "renderStyle": "chromeMesh0", "itemEffect": "flameSparks" }
   ```

   Every item keeps its model slot `MODEL_ITEM + item type` (D23, own model slots). Three
   PRs:
   - **4a Model files and textures (done):** `OpenItems()` / `OpenItemTextures()`
     become one loop over the item data. Models that aren't items (e.g.
     `MODEL_EVENT + …`, body parts) and the extra effect bitmaps stay in
     code. `textureFolders` is a search list: each texture comes from the
     first folder that has it (the old code loaded folder after folder and
     sometimes relied on textures another model had loaded before).
   - **4b Display (done):** the inventory transform (`RenderItem3D()` slot
     anchor, `RenderObjectScreen()` offset, rotation, scale and body
     height) and the ground transform (`ItemAngle()`, `ItemHeight()`) come
     from the data; `"cloth": true` marks capes worn as cloth (their cloth
     is deleted when they are changed; which capes are drawn as cloth stays
     in code). The display-only lists of phase 3 (`ItemDisplayCategories.cpp`)
     go away where they only chose position, angle or scale. The inventory
     models of the Rage Fighter armor take the look of their item. Display
     cases that depend on the item level stay in code until phase 12, and
     so does the look of the event models drawn for level variants, a small
     table in `Render/Items/ItemDisplay.cpp`.
   - **4c Render effects (D24) (done)**, three PRs because the drawing code is
     large (about 3,500 lines):
     - **4c1 Glow:** `"glow"` holds the level an item glows like (jewels +8,
       wings +0, or one per item level for arrows, bolts and Devil's Square
       items), the colors of the level glow, of the shine of items +11 and
       up and of ancient items (names from `Data/Effects/GlowColors.json`),
       the meshes they are drawn on, and the excellent glow (off for wings
       and capes, or on one mesh). It replaces `PartObjectColor`,
       `PartObjectColor2`, `PartObjectColor3`, the mesh choices of
       `RenderPartObjectBodyColor(2)` and the item cases of the glow level
       switch. Still in code: the glow of monsters (area MN of the roadmap:
       monsters have their own models, data and code), of the event models
       of level variants (with 4d and phase 12, when they get model entries
       and the drawing code knows the item), of the inventory model of the
       Phoenix Soul Armor (see the last step below), and the Deadly Staff's
       second glow step (a one-off that changes the object, moved in 4c2).
     - **4c2 Render styles:** `renderStyle` names the `RenderPartObjectBody`
       recipe of an item. The 174 item branches become 137 styles in
       `Render/Items/ItemRenderStyles.cpp` (the recipes unchanged; equal
       recipes share one style, three that drew like the default are gone,
       styles that only differ in a texture share one recipe with the
       texture in the list of names), and 346 models name theirs. The
       display-only item lists of phase 3 that chose a recipe go away.
       Styles that only apply to some drawings (not for doppelgangers, only
       held by some monsters, only on the helper NPCs) fall back to the plain
       drawing, and so do styles that pick a mesh, texture or color per item
       for other items. A style can also have its own glow pass, which takes
       the Deadly Staff's glow step out of the glow code. Player
       transformations, monsters and the other non-item models stay in
       `RenderPartObjectBody`; the transformations come before the styles,
       as they came before the branches of the Rage Fighter gloves, which a
       transformed Rage Fighter draws on its own object. The green, red and
       purple chaos boxes (14,55–57) have no model entry (the old code never
       loaded their models); their recipe (textured, then
       `RENDER_BRIGHT | RENDER_CHROME2`) comes back as a style with their
       model entries.
     - **4c3 Item effects:** `itemEffect` names what the model does before it
       is drawn every frame, from the type chain of `RenderPartObjectEffect`:
       29 item branches for 81 items; 28 of them become 27 effects for 42
       items in `Render/Items/ItemEffects.cpp` (moved unchanged; the Siege
       Potion and the Contract share one). An effect places sprites, particles
       and lightning on bones, changes values of the drawing (a pulsing glow
       mesh, a mesh hidden by level, the level potions glow like), or draws
       the model itself. The socket seeds and spheres (the 39 items of the
       last branch) and zen need no effect: they glow like level 0 (`"glow":
       {"level": 0}`), which is what their level checks did. The shine of 10
       items below +3 (two more shine passes) becomes values of their render
       style, which the drawing code applies: the seals, the Illusion Sorcerer
       Covenant and the Cursed Castle water have a style already, the Jewel of
       Harmony and the Moonstone Pendant get `harmonyShine` (138 styles now).
       Fixes found while testing, which change the look: the Devil's Eye, Key
       and Invitation glow like their level up to +6, so +1 and +2, +3 and +4,
       +5 and +6 look alike, and like +13 from +7 (the original client doubled
       the level, a frame-rate change of 2023 turned that into `Level *=
       pow(2, …)` and its fix into `Level /= 2`, which gave +1 to +5 one
       look); the Blood Bone, the Invisibility Cloak and the Illusion Sorcerer
       Covenant glow by their level like the Scroll of Archangel and the Old
       Scroll (the own drawing of the Blood Bone and of the cloak ended the
       drawing before the level glow; they are the render styles `bloodBone`
       and `invisibilityCloak` now, which leaves 25 effects for 40 items and
       140 styles; the Covenant glowed like level 0). The effects of the event
       models of level variants stay in `RenderPartObjectEffect` (no model
       entries yet). Which values the effects take (bones, colors, sizes)
       comes with phase 13, when what the named looks are made of moves into
       data (D25). MuEditor gets a read-only view of the looks above the item
       table: for the selected item its model file, glow, render style and
       effect, and for a look all items that use it (D26). It only reads the
       model data, so it needs no change to the drawing code.

   Verified like phase 3: one-time comparisons of the old and the new code
   (which files and texture folders are loaded for each model; the
   transforms of every item at every level; which effect code runs with
   which values).

   Loading stays as it is: all item models and textures are read at
   startup (`OpenBasicData()` on the loading screen). Textures are already
   shared by file name (`Bitmaps.LoadImage` counts references); meshes are
   not, each model slot opens its own copy of the `.bmd` file (until 4d1).
   Loading models only when an item is first drawn would be a separate step
   later.

   **4d Shared models (D23) (done)**, in two parts:

   - **4d1 Shared model entries (done):** the model files that several
     items use (34 files, 145 items: the skill parchments, the socket
     seeds and spheres, the jewels and their bundles, …) become named
     models in `Data/Items/Models/SharedModels.json` with the file, texture
     folders and none-blend meshes. The items name them (`"model":
     "skillParchment"`) and keep their own inventory, ground, glow, render
     style and item effect. The first item of a shared model opens the file
     and loads its textures; the slots of the others use that data
     (`BMD::ShareFrom`), so each file is read once. Every item keeps its
     own model slot, so the code that computes `MODEL_ITEM + type` stays as
     it is. Verified by recording what every item slot has loaded (meshes,
     bones, actions, vertices, textures, none-blend flags) with the old and
     the new loading: the same, apart from the numbers of the texture
     slots (the users of the seed spheres no longer try the `Item` folder
     first, which used up numbers). A test checks that every file several
     items use is a shared model. The Looks view of MuEditor shows the
     shared model and lists its items.
   - **4d2 Model slots in one place (done):** every conversion between an item
     type and its model slot goes through `Data::Items::ToModelSlot`,
     `ToItemType` and `IsItemModelSlot` (`ItemModelSlots.h`) instead of
     `+ MODEL_ITEM` / `- MODEL_ITEM` in about 120 lines. They return what
     the code computed before, so nothing changes now (checked by comparing
     the compiled code of every changed file); phase 12 can give a moved
     item its old slot there. Two conversions in `ZzzEffect.cpp`
     (`RenderWheelWeapon`, `RenderFuryStrike`) and the spear check in
     `MoveHandlers.cpp` followed with the effect catalogue design (FX1.0),
     which fixed the findings of cppcheck in those files. Objects keep their model slots, and
     the places that compare them with named slots stay as they are (one
     slot per shared model was dropped, see D23). The data of a shared
     model is freed by the last slot that uses it, whichever slot that is,
     so releasing or reopening the slot that opened it (e.g. reloading
     models from the editor) no longer leaves the others with freed
     memory. Writing to the shared data through one slot would change all
     items of the model; that stays a documented rule (`BMD::ShareFrom`),
     because making the data read-only would change every user of the
     model data. A test opens the items of a shared model into the model
     slots: the first opens the file, the others share it, and a missing
     file is reported for each item. Follow-ups from the review of 4d2:
     - `OpenPlayers` clears the model slots with `ZeroMemory` after they are
       constructed, which also clears the use count (a `std::shared_ptr`).
       No count exists at that point, so it does no harm, but it is
       undefined behavior: the constructor should set what the clearing
       sets, and the clearing go.
     - The loaded data (meshes, bones, actions, textures, texture indices
       and their counts) in one struct held by a `std::shared_ptr`, whose
       destructor frees it and unloads the textures, would make sharing a
       pointer copy and `Release` a `reset()` (CODING_RULES rule 8).
     - Some callers convert slots that are no item's: `DeletePet` with an
       empty helper (-1), the helper switch of the photo viewer,
       `RenderCharacter` and `ModifyTypeCommonItemMonk` with class models,
       `MoveCharacter` without a weapon. When `ToItemType` becomes a lookup
       (phase 12), they need `IsItemModelSlot` or a defined result for
       other slots.
     - No test covers the texture references a sharing slot gives back
       when it lets go last (it needs loaded textures).

   **4e Clear model loading errors (done):** a missing model file or texture of an
   item model shows one message after loading instead of one popup per
   texture. Each line names the item (name, group, number), its entry in
   the model files, the `.bmd` file and mesh, the texture (with the
   `.OZJ`/`.OZT` file the game reads) and the folders that were searched.
   A texture of another type than `.jpg`/`.tga` is an error as well. A
   texture that is only found because another model loaded it before is
   a warning in `MuError.log` that names the folder to add. Each problem
   is logged when it is found. Errors of the models that stay in code name
   the `.bmd` file instead of the path in the model. The error dialogs
   have a **Copy text** button and show the mouse cursor, which the
   loading screen hides.
5. **Translation tooling (done)**: the names themselves are part of phase 2
   (D17); this phase added the tooling. It came before phase 6, which builds
   on its item list:
   - The **Item Translations** window of MuEditor: the item names as a grid of
     items and the UI languages (English first), edited in place for any
     language and stored whole (also beyond the 49 characters the game
     shows). Each language's header counts the items without a name of
     their own in it, also languages without any item names yet; **Missing
     in** lists them, and a filled name keeps its row until refreshed.
     "Same as English" and "Remove translation" in the cell menu; clicking
     an item number shows it in the item table (the item browser since
     phase 6).
   - A name that is the same in a language is written like the English one
     (`"de": "Kris"`), so it is not counted as missing; the bmd import keeps
     such a name where the item has a name in that language, and leaves a
     missing one (which a bmd file has as the English name) missing.
   - Missing names are counted in the editor and listed per language on
     Save Items, not logged at load. Loading warns about a name in a
     language the game has no UI texts for (it is never shown).
   - The shipped data: the names that are the same in a language are
     written explicitly, seven missing Portuguese and Spanish names come
     from the original client's `ItemTooltip_<lang>.bmd`, and the Spanish
     names of the Capes of Fighter and Overrule (Korean read as Latin-1)
     became *Capa del Guerrero* and *Capa Reinante*.
   - For phase 6: the item list (`CItemListModel`: search over all names,
     `group,number` and the item type, the missing-name filter) is kept
     apart from ImGui, so the item browser builds on it, and
     `CItemDataHandler::SetItemName` edits a name in the item database for
     any language. Importing names from OpenMU belongs to phase 7.
6. **Editors (done)**: the MuEditor tools from section 9, including
   add/remove, on the rules of D26 (one selection, "used by", a warning with
   the list of users before a shared definition changes, copies for
   variants). The model tool builds on the read-only look view of 4c3 and
   picks the looks of an item from lists; the looks themselves are edited
   from phase 13 on. As built (docs/item-data.md, "Editing items"):
   - **Item Browser**, **Item Basics**, **Item Requirements**, **Item
     Rules**, **Item Categories**, **Item Model** and **Item Checks** share
     one selected item; Item Translations (phase 5) joins them. The item
     table is gone (D5, D19): the browser took its window and its Save,
     Import and Export buttons, and every column has a tool. The item tools
     change the database through the handler (`SetItem`, `AddItem`,
     `RemoveItem`, `MoveItem`, `SetItemModel`, `SetSharedModel`), which
     keeps `ItemAttribute` and the model data in step.
   - Add (a free number has neither an item nor a model entry; the selected
     item or a size and slot as the start), duplicate (a model file of its
     own becomes a shared model of both), move (to a free number, with the
     model data and the opened model) and remove (with its model entry).
     Moving and removing list first what goes with the item (tags, model
     entry, shared model) and what stays at its number: the source lines
     naming it (from the sources the editor was built from) and the sets,
     add options, buffs and pets of the game data.
   - Undo and redo for the session (256 steps, across saves, cleared by the
     bmd import); a step whose item changed in a way the history did not
     record is refused. Fields are edited on a copy and stored when left.
   - Save Items runs the checks first (also shown in Item Checks, each
     leading to its item and field), then writes the item files, the model
     files of the changed groups, `SharedModels.json` and
     `Data/Effects/GlowColors.json` when they changed. A
     missing model file, a `.bmd` several items name directly and an unknown
     render style or item effect are warnings, so data that loads stays
     saveable.
   - The model tool: a preview from the effect preview, a file picker over
     `Data/Item` and `Data/Player`, shared models edited with the D26
     warning and "Make a copy for this item", a shared model that is used
     cannot be removed, glow colours, render styles and item effects picked
     from lists with their users. A changed file, texture folder, mesh list
     or material is opened again between frames.
   - The glow colour tool (D26 for the colours): new colours (also from a
     colour field of the model tool, which then picks it), a changed value
     listing the items it reaches first, and the colours no item uses
     removed; the three defaults stay. Renaming a colour is left out.
   - Size and slot are refused while the hero wears the item or a window
     shows it (an item grid frees its cells by the size an item has when it
     is taken out); such an item is not moved or removed, an undo step that
     would change it is refused, and the bmd import keeps its size and
     slot.
   - **Item Formulas** edits the item stat file: the drop level bonus, the
     default, the profiles (each term inherited or written), the level bonus
     tables and the profile rules, with the D26 list of the items a change
     reaches, as one undo step; Save Items writes `Stats/ItemStats.json`.
     New, copied, renamed (taking the rules and the items that name a
     profile along) and removed (only unused) profiles and tables; no
     profile name is fixed by code, only `default` as the base layer.
   - Item Basics and Item Requirements show where an item's profile comes
     from (its rule, or a pick) and the formula of each value with the
     item's numbers and the source of each part. A stat requirement shows
     its +0 value and its weight (the stored base value) with the weights
     that give the same +0 value; a typed +0 value takes the weight that
     gives it, or lists the candidates with their +15 and excellent values
     (the formula rounds down, so low drop levels have several).
   - Left out: the custom marker of section 7 (nothing reads it yet; phase
     7 and Q1), mixes and quests in "used by" (their loaded data has no way
     to walk it), the last OpenMU import in the remove warning (phase 7),
     the looks themselves (phase 13), and whether an item can be excellent,
     ancient or how high its + level goes (OpenMU has `PossibleItemOptions`,
     item sets and `MaximumItemLevel`; maybe a phase after 12, once level
     variants are items of their own).
   - Formula sync with OpenMU is part of phase 7 (section 6): only the
     level bonus tables are data there today (`ItemLevelBonusTable`); the
     requirement formula, the excellent and ancient bonuses and the drop
     level bonus are C# code (`ItemExtensions.GetRequirement`,
     `ItemPowerUpFactory`), which phase 7's step 2 turns into data. The
     client's profile rules stay client-side.
7. **Item sync** (section 6), in five steps, one PR each:
   1. **The exchange file, Export and Import (MuMain).** The file of items
      and item formulas, its reader and writer, the Item Sync window with
      the diff, the known differences, and the orbs, crystals and scrolls
      keeping their requirements as values. A test takes every shipped item and the
      shipped formulas to the file and back unchanged.
   2. **The item formulas as data (OpenMU).** Stat profiles, default terms
      and the drop level bonus in the game configuration, each item naming
      its profile; defaults equal to today's code, tested for every item and
      level; migration and update plug-in.
   3. **The export endpoint (OpenMU).** `GET /api/items/exchange`, always
      behind an API key, rate limited, cached with an ETag, and a download
      button.
   4. **Update from server (MuMain).** The download in MuEditor, the last
      server state and the three-way diff. The differences the first real
      diff shows are fixed on the side that is wrong, each in a change of
      its own.
   5. **The import page (OpenMU).** The client's export with the same diff;
      new items take their server-only values from a template item.

   **B (OpenMU):** steps 2, 3 and 5.
8. **Data-driven tooltips** (D15), in steps (#815). Done in steps 1–5
   (#813–#836):
   - an item's own lines are its `description`, its names by level its
     `nameLines`, with their translations;
   - the stat, durability and requirement lines are the `default` layout of
     `Data/Items/Tooltips/Tooltip.json`, with shared texts, built-in values
     and conditions;
   - `BuildItemTooltipText()` builds the tooltip from that data, and a
     snapshot test checks every item's tooltip;
   - the tooltip bmd files are deleted (#836), and #863 resolves the
     placeholder and condition names when the data loads.

   `docs/item-data.md` describes the data (Tooltip layouts, Descriptions,
   Name lines). Left:
   - **Item cases still in code** whose lines the data can already hold:
     the ribbon and chocolate boxes, the Pumpkin of Luck and the
     Jack O'Lanterns (`{addOptionValue1}`), the Box of Luck's texts by level,
     the jewel bundles' "… is combined" lines (`{bundleCount}`), and the
     arrows' and bolts' damage line.
   - **The class line list** (`IsRequireClassRenderItem`: the items whose
     tooltip has no class lines), as an item tag.
   - **Ground label colours** (`BuildGroundItemLabelDescriptor`): the names
     come from `nameLines` (#834), the colours are still chosen per item, from
     four model lists and about 20 single-item rules. With D68–D71, in six
     PRs:
     1. **The data move**, with no visible change. `GroundLabels.json` holds
        the styles and the standard rules. `groundLabels` goes on the about
        200 items with a style of their own, with `levels` for the level
        variants. Four new states: `socketItem`, `upperTierWings` (from
        `wingTier`: the second- and third-tier wings and capes, the wings
        that never turn green), `divineArchangel` and `skillLuckOrOption`.
        The label then reads these instead of the lists. It is compared with
        main for every item at +0 to +15, in six option variants and 11
        languages: text, colours, background and bold. Two looks stay as
        they are although they don't match their kind: the Chaos Castle
        ticket is yellow beside the orange tickets, and the Summoner
        Character Card is orange beside the yellow character cards.
     2. **The Colors window** (D69), with the glow colour tool as its Glow
        tab on the shared panel.
     3. **Ground labels in MuEditor**: the Ground labels tab, and Item Tooltip
        (D71) with the item controls, the label preview and an item's label
        rules.
     4. **The full tooltip preview**, #815 step 6a (D71).
     5. **The standard rules and the condition picker** (D70), with saved
        conditions.
     6. **Tooltip colour groups**: `"colorGroups"` in `Tooltip.json`, each a
        name for one of the 14 theme colours, usable by any tooltip line;
        the Colors window's third tab.

     Italic label styles follow once the fonts have italic: italic fonts on
     Windows, and a slant in the Linux and macOS text code.
   - **The tooltip editor** in MuEditor (#815 step 6, designed as four PRs).
     Its read-only preview (6a) comes with the ground labels (D71), and its
     conditions are picked (D70).

   Lines that need the game stay code: the reset fruits' points, the saved
   revive and portal places, the Sign of Dimensions' count, the socket
   seeds, the Armor of Guardsman's Chaos Castle table, the Lost Map and
   Symbol of Kundun, and the Horn of Fenrir. The option sections come with
   phases 9 and 10.
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
12. **New item groups** (D21) — *to discuss again when we get here.*
    Groups 12–15 mix unrelated items. The new client may get new groups
    (e.g. 16 = jewels, 17 = orbs, 18 = scrolls) so the group means
    something again, and room beyond 16 × 512 items; original Season 6
    clients keep today's ids.

    - Moved items keep their original id as a **legacy id** in the item
      data. Items that are not moved need none: their id is the same for
      both clients. Items without a legacy id are new; they are not sent
      to original clients (answers Q1 for those items).
    - Client: more groups (`MAX_ITEM_TYPE`), the item editor shows and
      edits the legacy id, a new item encoding in the protocol for groups
      above 15, and the new client identifies itself to OpenMU with its
      own client version.
    - Cost: every hardcoded item id has to follow a move (client `ITEM_*`
      constants and ranges, about 113 group/number checks in OpenMU), and
      the model ids (`MODEL_ITEM + type`) must not depend on the item type.
      That is why this comes after phases 3 and 4 and OpenMU PR A. Phases 3
      and 4 moved the rule lists and the looks, and 4d2 put the model slots
      in one place (`ToModelSlot`), but about 330 places in code still name
      items (see "Item-specific code that is left").

    **D (OpenMU):**
    - legacy group and number on `ItemDefinition` (optional), filled by the
      exchange file and editable in the admin panel;
    - a **mapping tool**: checks that every moved item has a legacy id, that
      legacy ids are unique and fit the Season 6 range (group 0–15, number
      0–511), and builds the old ↔ new lookup at startup and after a config
      reload;
    - the Season 6 item serializer (`ItemSerializer`) writes the legacy id;
      a new serializer for the new client version (`[MinimumClient]`) writes
      the new id. The serializer is chosen once per connection, so there is
      no extra layer: the new client pays nothing, and for original
      clients the serializer reads a different field of the definition it
      already reads;
    - the few requests from original clients that name an item by id look
      up old → new once;
    - game logic only uses the new ids.

    With the new groups, the mixed groups 12–15 are split up, so the item
    files (one per group) become category files by themselves (D7).

    **Level variants** become items of their own at the same time: items
    that are a different item at each item level (Box of Luck +13 = Heart
    of Dark Lord, Rena +1–3, Wizard's Ring +1–3, …; their names by level
    are their `nameLines`) get their own entries with their own names, flags
    and tooltips. Their legacy id is then (group, number, level): the
    Season 6 serializer sends the old item with that level, and requests
    from original clients are mapped back. Until then the rule code checks
    them by level (`GameLogic/Items/ItemLevelVariants.h`).

    Questions for then: the exact new groups and which items move; whether
    the legacy id lives only in OpenMU or also in the client data; the
    protocol change for the new item encoding; and which level variants
    become items of their own. They are of three kinds:
    - **Different items** under one id: Box of Luck +1–7 and +13–15 (Star
      of Sacred Birth, Firecracker, …, Heart of Dark Lord, the Lucky
      Pouches), Rena +1–3, Loch's Feather +1, Ale +1, Wizard's Ring +1–3,
      the chocolate boxes +1. Own names, models and uses: split.
    - **Tiers of one item**: Devil's Eye, Key and Invitation +1–7 (the
      square), Box of Kundun +1–5 (Box of Luck +8–12), the Blood Castle
      items +1–8. The level is a real attribute that the server uses (event
      entry, mixes such as Eye +N and Key +N into Invitation +N); splitting
      them would multiply items and mix rules. Proposal: keep them as one
      item each, with per-level values where their look changes (like the
      glow `level` list). Box of Kundun becomes an item of its own whose
      levels are the tiers; its legacy id maps with an offset (Box of Luck
      +8–12 ↔ Box of Kundun +1–5).
    - **Kinds of one item**: the fruits (+0–4 = energy, stamina, agility,
      strength, command), the Orb of Summoning (which summon) and the
      Transformation Ring (which monster). The level chooses a kind, not a
      tier; to decide then.
13. **Looks in data** (D25): what the named render styles and effects are
    made of moves from code into data files in `Data/Effects/`, as lists of
    building blocks, one look at a time; the item data keeps naming them.
    Particle and effect types are referenced by their names from the effect
    catalogue (FX1, roadmap) and stay code until FX2. Effects place sprites
    and particles on bones of the model, so they follow its animation. Bones
    are values by number, as in the code (see "Bones" in the roadmap for
    why); the look editor shows numbers and names together, and the model
    loader checks that the bones exist. The lists are resolved when loading
    (textures, bones, particle types), so drawing is not slower. Verified
    like phase 4c: every look draws and spawns the same things with the same
    values as its code did, per item and level. The look editor of section 9
    comes with it.
14. **UI text translations**: the UI texts are not item data, but they
    are translated the same way and the tool builds on phase 5's Item
    Translations window, so it comes before the cleanup. Today they are
    edited by hand in `src/Localization/<Group>.<locale>.resx` (Game,
    Dialog, Editor, Metadata), and `tools/ResxGen` compiles them into the
    game (`docs/translation-system.md`). Measured on 2026-10-20 against
    English, by key: Game misses 92 German texts, 116 Polish, 123 in each
    of es, id, pt, ru, tl, uk and zh-TW, and 606 Japanese; Dialog has no
    file for id, ja, ru, tl, uk and zh-TW (233 texts each) and misses one
    Spanish and one Portuguese text; Editor misses 7 German texts and 170
    in each other language, and has no Japanese file (313 texts); Metadata
    is complete but has no Japanese file (59 texts). `Game.en.resx` has the
    key `Connecting to the server` twice.
    - A MuEditor window (editor builds) with the texts as a grid of keys and
      languages, English first, grouped by file: the missing texts counted
      per language and file, a filter for them and a search over keys and
      texts, like the Item Translations window.
    - Edits are written to the `.resx` files of the source tree, not to the
      game folder, so the editor has to find `src/Localization`. The game
      reads the texts compiled in, so an edit shows after the next build,
      unless the window also changes the loaded texts for a preview (to
      decide).
    - Checks: a translation keeps the placeholders of the English text
      (`{0}`, `%s`, `%ls`, `%%`), and a file has each key once; the German
      terms of `translation-system.md` can be shown as hints.
    - Planned like phase 5: an inventory of the files and the generator,
      then the decisions (preview, which files, adding a missing file such
      as Dialog for a language), then the window.
15. **Adding a language**: today a language is added by hand
    (`docs/translation-system.md`, "Adding a new locale"): its
    `<Group>.<locale>.resx` files, its name in `KnownLanguageDisplayNames`
    of `tools/ResxGen`, and an entry in the game's language list
    (`s_Languages` in `src/source/UI/Options/OptionWindow.cpp`), then a
    build. Item names in it can then be filled in the Item Translations
    window; until it has texts, the item data warns about names in it.
    - An **Add language** action in the translation windows (phase 14 and
      Item Translations): the code (checked like the item data's language
      codes, and not one there already) and the language's name in its own
      language. It creates the language's `.resx` files in
      `src/Localization` for every group.
    - One place for the language list and the names: the game's language
      list shows `GetAvailableLocales` instead of a list of its own, and
      the name comes from the language's texts (for example a key of the
      Metadata group, which Japanese has no file for yet) instead of
      `ResxGen`'s code. To decide in the phase.
    - The game reads the texts compiled in, so the new language shows in
      the game and as a column of the windows after the next build, like
      the text edits of phase 14.
    - A language whose letters the fonts lack (the editor loads Latin,
      Cyrillic, Greek and the CJK ranges; the game's font row) is pointed
      out when it is added.
    - Removing a language stays by hand.
16. **Cleanup**, once the work has landed:
    - The glow of the inventory model of the Phoenix Soul Armor
      (`MODEL_ARMORINVEN_74`) moves into data. It is the only model drawn
      for an item that glows differently from its item: only on its first
      mesh (its item's armor model glows on mesh 2), and the metal pass of
      +9 and up draws that mesh three times, so it shines brighter than the
      other inventory models (probably a copy-paste leftover; to keep or to
      drop then). Left for last because it only shows in the inventory and
      is easiest to check once everything else is in data.
    - Whatever later areas still need from this document moves to the
      roadmap, so it is not lost; "Item-specific code that is left" is the
      list of it until then.
    - Remove this document.

## Item-specific code that is left

An inventory of the code that still names specific items, made after 4d1
(2026-10-01): about 3,000 source lines (`ITEM_*` and item `MODEL_*` names,
raw ids such as `ITEM_POTION + 100`, ranges of ids) in about 500 places. Of
the 492 places with item logic, 294 hold lists of items (data, by section
4), 152 are one item's own behavior or value, and 46 convert between item
types and model slots (4d2). 239 identify an item by its model slot
(`o->Type == MODEL_…`); they stay valid because every item keeps its slot
(D23).

| Kind | Places | Moved by | Not moved yet |
|---|---:|---|---|
| Character animation by item | 67 | SK2 (cast animations) | Stances by weapon, normal attacks, conditions on the mount |
| UI and tooltips | 65 | 8, 12 | Ground label colours (D68), the tooltip's class line list and item cases (phase 8), bonuses shown in the character window |
| Item use | 58 | 12 (level variants), 11 (pet) | `TryConsumeItem`, `ItemValue`, stack sizes, hotkeys, requirement exceptions |
| Item lists | 42 | 3 (the data exists), 8, 9 | Call sites outside the phase 3 files (3b) |
| Mounts, pets, helpers | 41 | 11 (`pet.bmd`) | The spawned model, ride height, camera, HUD |
| Item ↔ model slot | 41 | 4d2, 12 | Class models (`MODEL_HELM2`, `MODEL_*_MONK`; kept in code by 4a) |
| Drawing held and worn items | 39 | 13, 12, MN | Effects of held items in `RenderLinkObject`, swing trails, full-set auras, `ItemObjectAttribute` |
| Map and event rules | 33 | MN, 12 | Entry items, mount bans per map, Chaos Castle weapons |
| Skills | 29 | SK1 (items that give a skill), SK2, FX1 | Projectile per bow, skill bonuses of mounts |
| Wings, cloth, capes | 13 | 9 (wing options); 4b kept the cloth in code | Cloth setups, flap speeds |
| Position on the character | 12 | – | Shield angles, poses on the back, link bones |
| Mixes | 11 | 11 (Mix, SocketItem) | Mix values, packed jewels (`COMGEM`) |
| Sounds | 7 | roadmap, "later: sounds" | All |
| Monster and NPC setup | 1 switch, 272 cases | MN | – |

**Not planned yet**, with what could hold it:

- **Mounts** (about 97 places): animations and skills chosen by the mount,
  the model a horn spawns, ride height, camera. A `mountKind` on mount items
  would turn about 50 chains into one value; the roadmap's "later: pets"
  should cover mounts.
- **Phase 3b** (about 35 places): lists that repeat data that exists
  (tags, `slot`, `wingTier`, `storable`, the files below) or need new
  data (the `weaponKind` field, tags such as `sceptre`).
- **Held and worn items** (about 34 places): their position on the
  character (per hand, on the back, link bone) and their looks while held
  (an item effect that runs while the item is held, swing trails). An
  extension of 4b and 4c3/13.
- **Item use** (about 23 places): `maxStack`, a `useAction` that names a
  code handler, `hotkeyGroup`. Section 3 already names usable/consumable,
  ticket and event as target fields.
- **Bonuses computed in code** (about 20 places): the Demon's +40 %, the
  Panda's +50 defense, charms. A `powerUps` field mapped to OpenMU (phase
  7) and the pets (phase 11).
- **Map and event rules** (about 19 places): items required or banned per
  map, for a maps area.
- **Prices, requirements, durability** (about 16 places): fixed prices
  into `buyPrice`, price per durability, how requirements grow.
- **Item stats by level** (phase 3c, D43): `SetItemAttributes` and its
  `Calc*` functions (`ZzzInfomation.cpp`) computed what the item level adds;
  since 3c the stats and requirements below come from the item stat file
  (`docs/item-data.md`), which writes the same formulas as data, and from
  the items' own fields (`excellentBonus`, `excellentRaisesLevel`,
  `requirementsByLevel`); the functions named below are the ones the code
  had before, except `CalcWingOptions`, which computes the wing options
  until phase 9. Here level is the item level
  (`ip->Level`, 0 to 15), drop level the kind's `Level` (`p->Level`,
  OpenMU's `DropLevel`) and base the kind's value. The level bonus
  (`GameLogic::Items::GetItemLevelBonus`) is `3 * min(9, level)` and,
  above +9, steps of 4, 5, 6, 7, 8 and 9 at +10 to +15, each on top of the
  ones below: 27 at +9, then 31, 36, 42, 49, 57 and 66, as in the Season 6
  client and OpenMU's level bonus tables. From 2021 (d301eb1fb, a366bc05b)
  to its fix the client added only the last step to damage, magic power,
  defense and blocking (`level - 6`, so 31 to 36 at +10 to +15); magic
  defense kept adding them up, without the +14 step (87f3f72e0), until
  815b8828.
  - Damage gets the level bonus. Magic power gets it too, then the sum with
    the excellent and ancient bonuses is halved and items that are not
    sceptres add `2 * level`.
  - Defense gets the level bonus with 2, 3 or 4 per level up to +9 (2 for
    second tier wings and the Capes of Lord and Fighter, 4 for third tier
    wings, 3 for armor and the other wings) and steps above +9 that start
    at 5 instead of 4 for third tier wings (the parameters of
    `GetItemLevelBonus`); shields get only `+ level` and their ancient
    bonus.
  - Blocking and magic defense get the level bonus like damage. 815b8828
    had made them use the drop level, so they never grew with the item
    level; #683 brought the item level back, as in the client before and in
    OpenMU.
  - Excellent items add `base * 25 / drop level + 5` to damage (from the
    minimum damage, to minimum and maximum damage alike), magic power and
    blocking (three chaos weapons have fixed values for damage and magic
    power, `GetExcellentAddValue`) and
    `base * 12 / drop level + 4 + drop level / 5` to defense, not to
    shields. Excellent here means an excellent item that is not a wing, or
    an ancient item, as in OpenMU: wings keep their wing options in the
    excellent flags. The client before 815b8828 had the same rule but also
    left out the four archangel weapons; the client follows OpenMU there,
    which gives them the bonus with excellent options. 815b8828 had counted
    any excellent flag, so wings with a wing option got the defense bonus
    and ancient items lost the bonus, and it took the maximum damage bonus
    from the maximum damage; #683 fixed both. The + 25 drop level of the
    requirements still follows any excellent flag (wings have no stat
    requirements).
  - Ancient items use `d = drop level + 30` (`GetDropLevel`): damage
    `+ 5 + d / 40`, magic power `+ 2 + d / 60`, defense
    `+ defense * 3 / d + 2 + d / 30` and shields `+ defense * 20 / d + 2`,
    with defense the value so far (3d5b1adc6 had added the base a second
    time to the ancient defense; #683 removed it).
  - Strength, agility and vitality requirements are
    `20 + base * (drop level + 3 * level) * 3 / 100` and energy the same
    with `* 4 / 100` (summoner books `+ level` instead of `+ 3 * level` and
    `* 3 / 100`; group 15 kinds with a level requirement use
    `20 + base * required level * 4 / 100` and the Orb of Summoning a fixed
    value per level, 30 to 500), where drop level is + 25 for excellent and
    otherwise + 30 for ancient items; command stays the base value.
  - The level requirement grows by `4 * level` (5 for second tier wings)
    only for first and second tier wings, the Cape of Fighter and items
    from group 13 on except the Horn of Fenrir. Group 12 kinds from the
    Wing of Dimension (12/43) on, except the Cape of Fighter, keep their
    base value (815b8828 had made that range empty, so the Wing of Dimension
    and the Cape of Overrule lost their level 400; #683 restored it). Most
    excellent items that are not wings need 20 levels more.
  - Wing options grow too (`CalcWingOptions`): max HP and max mana
    `50 + 5 * level` for second tier wings and the capes, command
    `10 + 5 * level` for the Cape of Lord.

  OpenMU keeps the same as data (`ItemLevelBonusTable` per stat), so these
  become named tables that a designer can change, shared in the item
  exchange.
- **Smaller kinds:** weapon kind for stances (10), ground height and light
  (8, extends 4b), names and colours outside `RenderItemInfo` (8, widens
  phase 8; the names moved with #834), ground label colours (7),
  transformation ring rules (6; their
  looks go to MN), mix values and packed jewels (6, phase 11), ids that
  encode a value such as the socket element (5), effects whose mesh is an
  item's model slot (4, FX1).

**Data that exists but is not used:**

- `Data/Local/<lang>/JewelOfHarmonySmelt_<lang>.bmd` lists the 85 items
  that cannot be smelted below +4. The client never loads it; `MixMgr.cpp`
  has the same list in code.
- `BuffEffect_<lang>.bmd` links buffs to items and is loaded; the elixir
  code lists the same link again.
- The item field `skill` is empty for all scrolls and parchments;
  `GetSkillByBook` maps them in code (SK1).

**Dead code** to delete instead of moving: `CChangeRingManager::CheckRepair`,
the macro `NOTSMELTING_DATA_FILE` (`PrintItem` went in 3c).

**Bugs found on the way**, filed upstream: the durability warning of
rings at +2 (sven-n/MuMain#667), the walk of the Dragon Lance (#668),
weapons hidden on the Illusion Temple team skins (#669), the Dark Raven's
height over a Dark Horse (#670) and the trade warning for every item from
(13,30) up (#671).

## Beyond items

Effects and particles, skills, monsters and NPCs are areas of the
[roadmap](2026-09-29-data-driven-content-roadmap-design.md), each with
its own design document when its work starts. They share the look format
(D25), the rules for shared definitions (D26) and the editor parts (look
editor, effect editor, bones) with the items. Item phase 13 depends on the
effect catalogue (FX1) there.

## Open questions

- **Q1: Custom items on the original client** *(deferred; must be
  discussed before the related PR lands).* What should OpenMU do with a
  custom item when the player uses the original Season 6 client? For
  example: never send it to that client (hide it in shops, drops and
  views), block such items on servers set up for the original client, or
  show a placeholder item. Phase 12 (D21) would settle it for items
  without a legacy id: they are not sent to original clients.
