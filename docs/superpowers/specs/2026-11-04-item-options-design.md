# Item Option Definitions (Items Phase 9) Design (WIP)

> **Draft, 2026-11-04.** Items phase 9 of the
> [items design](2026-09-25-data-driven-items-design.md) and its decision
> D14. Written from the code of MuMain `upstream/main` @ `88cf9d208` and
> OpenMU `master` @ `13a600f3a`. Line numbers are those of these commits.
> sven-n/MuMain#966 (open) makes the client read the guardian (380) flag
> that the server sends; where this design depends on it, it says so.
>
> This document is temporary. Its last task deletes it, after what stays true
> has moved into `docs/item-data.md` and the code (see "Tasks").

## Goal

The option groups an item can have (luck, the additional option, excellent,
wing, Jewel of Harmony, guardian, socket, the ancient bonus and the pet
options) move from code and bmd files into data files of their own, matching
OpenMU's `ItemOptionDefinition` (D14). An item file only links to the groups
the item can have. The tooltip's option lines then read these definitions
and become layout lines between the `options` and `afterOptions` markers
(D72), and MuEditor gets an editor and an exchange file for them.

As before for the items: nothing changes in game when an option group moves
into data, and each move is compared with the old code before the old code
goes. The tooltip snapshot and the item stat tests are the proof.

## Scope

- **In:** every option group of `SetItemAttributes`, `CalcWingOptions` and
  `CalcExcellentOptions` (`Engine/Object/ZzzInfomation.cpp:291-834`), the
  Jewel of Harmony options (`JewelOfHarmonyOption_<lang>.bmd`), the guardian
  (380) options of `ItemAddOption.bmd`, the socket options
  (`socketitem_<lang>.bmd`), the ancient bonus (+5 / +10 stat), the Horn of
  Fenrir, Dark Horse and Horn of Dinorant options; which items can have
  which group (links in the item files); the option exchange file; the
  option editor; the tooltip's option lines.
- **Split (O1):** phase 9 is done in two parts with phase 10 between them.
  **9a** is the file format, the loader, the item links, the attribute
  names and the groups phase 10 needs (ancient option, ancient bonus) plus
  the plain ones (luck, additional option, excellent). **9b** is the rest
  (wings, harmony, guardian, sockets, pets) and the tooltip's option lines.
- **Not an option:** the skill. OpenMU has no option type for it
  (`Item.HasSkill` and `ItemDefinition.Skill`); it stays the item's `skill`
  field.
- **Later:** the buff and consumable values of `ItemAddOption.bmd`
  (scrolls, elixirs, seals, charms; `{addOptionValue1}` in descriptions)
  stay in the bmd until phase 11; only its 380 part moves here (Q-O9).

## Decisions

Made on 2026-11-04 while planning phases 9 and 10.

| D | Topic | Decision |
|---|---|---|
| O1 | Order | Phase 9a, then phase 10 (item sets), then phase 9b. In OpenMU an item set's options are an `ItemOptionDefinition` (type `AncientOption`) and the per-item ancient bonus is one too (`ItemOfItemSet.BonusOption`), so phase 10 builds on the format of 9a instead of a format of its own that 9a would replace. |
| O2 | Starting point | Written from scratch. No branch or pull request started phase 9 (sven-n/MuMain and Mosch0512/MuMain searched on 2026-11-04). |
| O3 | Guardian flag | sven-n/MuMain#966 is merged before the guardian options move, so that the 380 lines the move compares are the ones a server item shows. |
| O4 | This document | Deleted by its last task, after its lasting content has moved into the docs. |
| O5 | Definition names (Q-O1, 2026-11-04) | A definition has a name of our own in camelCase (`luck`, `excellentPhysical`, `harmonyDefense`, `wingsOfSatan`), unique across all option files. Item files link to it by that name, and the exchange file carries it. OpenMU's import finds the definition by its fixed GUID where the Season 6 data gives one (`ItemOptionDefinitionNumbers`), else by its type and English name, because harmony and guardian definitions get random GUIDs (P4). The display name is a translated text of its own (D17). |
| O6 | Option values (Q-O2, 2026-11-04) | An option's value names the attribute it raises with OpenMU's attribute names (`Stats`, in camelCase: `maximumHealth`, `physicalBaseDmg`) and gives the value in the units the tooltip shows: `{"attribute": "maximumHealth", "percent": 4}`, `{"attribute": "defenseBase", "add": 4}`, and `addFinal` for OpenMU's `AddFinal`. The loader maps each attribute to the client's effect (`AT_*`); an attribute the client does not compute loads with a warning, and its line is shown but not added to the stats. The exchange converts the units (4 % to the factor 1.04 and back). |
| O7 | Values that depend on the hero or the item (Q-O3, 2026-11-04) | A value is a constant, a value per level (Q-O7), or either plus `per` terms: `"per": [{"attribute": "totalLevel", "divisor": 20}]`, `{"attribute": "horseLevel", "times": 2}`. A term multiplies an attribute of the hero, the pet or the item, as OpenMU's related values with `Multiply`; it covers excellent level / 20, Gold Fenrir, the Dark Horse and the socket value kinds 3 to 5. The value is computed when the tooltip is drawn and when the stats are summed, not when the item is made, so level / 20 follows the hero (P12, a behavior change the pull request names). An OpenMU option with another operator is a known difference of the exchange |
| O8 | Option texts (Q-O4, 2026-11-04) | The option lines' texts live in a file of their own, `Data/Items/Tooltips/OptionTexts.json` (`formatVersion` and `texts`, the shape of `Tooltip.json`'s `texts`: a name, then the text by language with `{value}`). An option names its text (`"text": "increaseMaxHp"`); names are looked up only in this file, so they cannot clash with `Tooltip.json`, which stays as it is. A line that several options show (the additional damage of the physical option and of the wings) is one text. The `.resx` texts only options use move into it with their translations, and the values written into them (`Increase Max HP +4%%`) become `{value}`. The harmony and socket option names (bmd today) join it in 9b. About 100 texts; in `Tooltip.json` (99 texts, 1360 lines, 95 % texts) they would have doubled the file |
| O9 | The excellent byte (Q-O5, 2026-11-04) | What the excellent byte means follows from the type of the definitions the item links to, as OpenMU reads it (`ItemSerializerHelper.cs`): a definition of type `excellent` makes it the excellent bits, `wing` the wing option bits (bit = number - 1 for both), the Fenrir types the colour (the value picks the option of that number: 1 black, 2 blue, 4 gold). An item that links two types that use the byte is a load error. "Excellent" (name prefix and colour, durability, price, requirements, the tooltip state `excellent`) then means "has an active option of type `excellent`", not "the byte is above 0", so the wing and Fenrir exceptions in that code go; the pull request shows with the tooltip snapshot and the stat tests that nothing changes |

## Current state

### Client (MuMain)

**Common to all groups.**

- The server sends OpenMU's extended item format (`ItemSerializerExtended`,
  minimum client 106.3). `ParseItemData` (`UI/Inventory/ItemMng.cpp:11-66`)
  reads a flags byte (`ItemOptionFlags`, `_enum.h:62-73`) and then, only for
  the flags that are set, the option byte (low nibble level, high nibble
  type), the excellent byte, the ancient byte (low nibble discriminator,
  high nibble bonus level), the harmony byte and the socket bytes. The
  guardian flag has no byte, and `ParseItemData` does not read it (#966).
- `SetItemAttributes` (`ZzzInfomation.cpp:486-834`) turns these bytes into
  the item's option list `Special[]` / `SpecialValue[]` (`_struct.h:161-229`),
  in this order: wing options, skill, luck, additional option, the Dark
  Horse, excellent options. Stats (`PlusSpecial*`, `ZzzInfomation.cpp:1666`),
  the price (`ItemValue`, `:836`), mixes (`MixMgr.cpp:150-175`), the item
  comparison and the tooltip read this list, so its order is the tooltip's
  order.
- Which items can have which group is code: type ranges in
  `SetItemAttributes`, `CalcExcellentOptions`, `GetItemType`
  (`UIJewelHarmony.cpp:90`) and `IsSocketItem` (`SocketSystem.cpp:64-95`).
  The item files have no option field. Some tags gate parts of it
  (`ammunition`, `summonerBook`, `magicSword`, `cape`, `gemJewelry`), and
  `wingTier` picks the wing options.
- Many values sit inside the translated texts, not in code or data:
  `IncreaseMaxHP4`, `DamageDecrease4`, `ReflectDamage5`,
  `DefenseSuccessRate10`, `LuckCriticalDamageRate5`.
- The bmd loaders report problems with a message box, not through the data
  issue system (`ItemDataIssue.h`).

**Per group.**

| Group | Values today | Which items | Byte | Tooltip |
|---|---|---|---|---|
| Luck | None. `AT_LUCK` with value 0, so the client adds no critical damage rate (`ZzzInfomation.cpp:523-551`) | Groups 0-11 without `ammunition`, the listed wings and capes, third tier wings | flag 0x02 | Two blue lines, values in the texts |
| Additional option | Code: level × 4 (damage, wizardry, curse, defense), × 5 (shield blocking), × 1 (life regeneration); for wings the option type picks the stat in a chain of `if`s per wing (`:553-826`) | Type ranges; wings by number | flag 0x01, option byte | Blue `…D` lines |
| Horn of Dinorant | Code: the option level is a set of bits (absorb 5, AG +50, speed +5) | One item | option byte | Blue lines |
| Excellent | Code, `CalcExcellentOptions` (`:375-484`): defensive six (shields to boots, rings) and offensive six (weapons, pendants); most values only in the texts; the % damage is a fixed 2 in `PlusSpecialPercent` (`:2006`); level/20 is computed when the item is made | Type ranges | excellent byte, bit = number - 1 | Blue lines; name prefix and green name colour |
| Wizard's rings | Code: fixed 10 % / 10 % / speed 10 (Lethal 15 / 15 / 10) at levels 0 and 3, without any byte | Two items | none | As excellent |
| Wing options | Code, `CalcWingOptions` (`:291-373`), in the **excellent byte**: second tier HP and MP `50 + 5 * level`, ignore defense 3, AG +50, speed +5; capes the first three and the Cape of Lord's command `10 + 5 * level`; third tier ignore defense, reflect, full life and full mana recovery 5 each | `wingTier`, `cape` | excellent byte | Blue lines, first in the list |
| Horn of Fenrir | Code (`ZzzInventory.cpp:2178-2245`): the excellent byte is 1, 2 or 4 (black, blue, gold) | One item | excellent byte | Code lines after the options; name lines already data (`Group13_Helper.json`) |
| Dark Horse | Code (`GIPetManager.cpp:580-605`): defense `5 + dex / 20 + pet level * 2` | One item | none | Blue line |
| Jewel of Harmony | `JewelOfHarmonyOption_<lang>.bmd` (`UIJewelHarmony.cpp:15-88`): 3 kinds (weapon 10, staff 8, defense 8 options), a name, a first level, a value and a Zen cost per harmony level 0-13 | Groups 0-4, 5, 6-11 by code | harmony byte | Yellow bold when the item level reaches the harmony level, else gray |
| Guardian (380) | `ItemAddOption.bmd` (`ItemAddOptioninfo.cpp`): two options and values per item, in the same file as the buff values | A record in the file | flag 0x40, no byte | Red-purple bold, both slots always written |
| Sockets | `socketitem_<lang>.bmd` (`SocketSystem.cpp:605`): seed options with a value per sphere level 1-5 and a value kind (flat, %, per level, per max HP, per max MP), the bonus options, the set bonus | `IsSocketItem` list | socket bytes, `sphere level * 50 + element offset + number` | Purple header, blue per socket, after all layout markers |
| Ancient bonus | Code: `AncientBonusOption * 5` of the stat in the item's `attackType` field (`CSItemOption.cpp:546-594`) | Ancient items | ancient byte, high nibble | Blue `Increase…D` line (`RenderDefaultOptionText`, `:641-658`) |

**Tooltip.** `BuildItemTooltipText` (`ZzzInventory.cpp:1756-2384`) writes
the option area in this order: the layout up to the `options` marker, the
380 lines, the harmony lines, the ancient bonus, the `Special[]` loop, the
layout from `options` to `afterOptions`, the Horn of Fenrir lines, the
layout after `afterOptions`, the set options (phase 10) and the socket
options. The tooltip snapshot (`tests/data/item_tooltip_snapshot.txt`)
covers luck, skill, option level 1 and 4, excellent 1 and 63, ancient and
harmony 1 and 13. It does not cover 380, sockets, the option type of wings,
the ancient bonus level or the Fenrir and Dinorant values.

**MuEditor.** Item Tooltip has Skill, Luck, Option (level 1) and Excellent
(all six) checkboxes for its name preview (`MuItemTooltipUI.cpp:153-161`).
Item Requirements has an option level selector for
`strengthPerOptionLevel`. No tool shows or edits option values.

### Server (OpenMU)

- `ItemOptionDefinition` (`DataModel/Configuration/Items/`): `Name`
  (`LocalizedString`), `AddsRandomly`, `AddChance`, `MaximumOptionsPerItem`
  and `PossibleOptions`. It has no number or key of its own, only its
  persistence `Guid`.
- `IncreasableItemOption` (an `ItemOption`): `Number`, `SubOptionType` (the
  socket element), `OptionType` (`ItemOptionType`), `PowerUpDefinition` (the
  value without levels or at level 1), `LevelType` (`OptionLevel` or
  `ItemLevel`), `Weight` (the Jewel of Harmony roll) and
  `LevelDependentOptions` (`Level`, `RequiredItemLevel`,
  `PowerUpDefinition`).
- A value is a `PowerUpDefinition`: a target attribute (`AttributeDefinition`,
  fixed ids in `Stats`) and a boost: a constant with an aggregate type
  (`AddRaw`, `Multiplicate` with factors such as 1.04 for +4 %, `AddFinal`,
  `Maximum`) plus values related to other attributes (excellent's
  level / 20 is `TotalLevel * 1/20`).
- The types are fixed GUIDs in `ItemOptionTypes.cs`: Excellent, Wing, Luck,
  Option, HarmonyOption, AncientOption, AncientBonus, GuardianOption,
  SocketOption, SocketBonusOption, BlueFenrir, BlackFenrir, GoldFenrir,
  DarkHorse. New types can be added as data, but the game logic and the
  serializer compare with these GUIDs, so a new type is never sent to the
  client.
- An item links to definitions through `ItemDefinition.PossibleItemOptions`;
  many items share one definition. Item sets hang off
  `ItemSetGroup.Options` and `ItemOfItemSet.BonusOption` instead.
- Season 6 data (`Persistence/Initialization/VersionSeasonSix/Items/`):
  Luck (chance 0.25), six additional option definitions (defense, physical,
  wizardry, curse, defense rate, physical and wizardry), three excellent
  definitions (defense, physical, wizardry; chance 0.001, two options per
  item), one definition per wing for its additional options plus shared
  "2nd Wing Options", "Cape of Lord Options" and "3rd Wing Options", three
  harmony definitions, six guardian definitions (by item kind), six socket
  element definitions and three socket bonus definitions, one ancient
  option definition per set and one ancient bonus definition per stat.
- The item exchange endpoint (`GET api/items/exchange`,
  `ItemExchangeController.cs`) writes per-item fields only and has no
  option data. Its extension point `ItemExchangeField` is per item.

### Problems found

| # | Problem | Where | Phase |
|---|---|---|---|
| P1 | `Number` means something different per type in OpenMU: 1-based bits for excellent and wing, 0, 2 or 3 for a wing's additional option, bit values 1, 2, 4 for the Horn of Dinorant, 0-based for sockets, the item group for guardian options (twice in one definition) | `ItemSerializerHelper.cs` | 9a (format) |
| P2 | The excellent byte is excellent options, wing options or the Fenrir colour, by item kind; code everywhere tests `ExcellentFlags > 0` (name prefix, durability, price, requirements) | `ZzzInfomation.cpp`, `ZzzInventory.cpp` | 9a, 9b |
| P3 | Level 1 is stored two ways in OpenMU: the additional options keep it in the base value and levels 2-4 in `LevelDependentOptions`; wing options keep levels 1-4 there with a base of 0; harmony starts at level 0, which is also its `RequiredItemLevel` | `GameConfigurationInitializerBase.cs:112-151`, `InitializerBase.cs:180-200`, `HarmonyOptions.cs` | 9a (format) |
| P4 | Harmony and guardian definitions get a new random GUID on every OpenMU installation; only their options have fixed GUIDs | `HarmonyOptions.cs`, `GuardianOptions.cs` | 9a (exchange) |
| P5 | Second tier wings have five options in the client and three in OpenMU (no AG, no speed) | `CalcWingOptions`, `Wings.cs:342-400` | 9b |
| P6 | Guardian values differ: pants defense 200 in the client and 100 in OpenMU, boots SD 700 and 200 | `ItemAddOption.bmd`, `GuardianOptions.cs` | 9b |
| P7 | OpenMU has a harmony group for curse weapons; the client gives books the staff options | `UIJewelHarmony.cpp:90-111` | 9b |
| P8 | Luck adds no critical damage rate in the client (value 0), OpenMU 5 % | `ZzzInfomation.cpp:2139-2150` | 9a |
| P9 | The ancient bonus stat comes from the item's `attackType` field; OpenMU puts it on the item's place in a set | `CSItemOption.cpp:546-594`, `AncientSets.cs:797-840` | 9a, 10 |
| P10 | `ItemAddOption.bmd` holds the 380 options and the buff values of consumables in one table | `ItemAddOptioninfo.cpp` | 9b, 11 |
| P11 | Most harmony effects (critical damage, SD, regeneration) are shown but never added to the client's stats | `UIJewelHarmony.cpp:145-273` | 9b (report) |
| P12 | Values computed when the item is made go stale: excellent level / 20 from the hero's level at that time | `CalcExcellentOptions` | 9a |
| P13 | The socket "no bonus" value arrives as 0xF, not 0xFF; it works because bonus 15 is empty | `ParseItemData` | 9b |

## Target design

Written from the answers to the open questions. Until then this section
only says what follows from D14 and O1:

- The option definitions are JSON files in a subfolder of `Data/Items`
  (every `*.json` directly in `Data/Items` is an item group file, D68),
  with `formatVersion`, the phase 2 rules for writing and loading, and the
  three problem levels in Data Checks.
- A definition has a name of its own that items link to (D26), its
  translated display name (D17), its options with their values and levels,
  and what belongs to it on the server (chances, options per item, weights;
  D14 says "values, chances, levels").
- An item file links to the definitions the item can have. The bmd import
  sets the links by today's code rules, so a converted item gets what it
  has today.
- The option exchange file is a file of its own (D11, D14), compared field
  by field like the item exchange (phase 7); OpenMU's side is phase C.

## Open questions

Answered one by one, each with the current state, every option with an
example, and a recommendation. The recommendation is listed here so the
questions can be read in one go.

| Q | Question | Recommendation |
|---|---|---|
| Q-O1 | The key of a definition. | Answered (O5). |
| Q-O2 | How an option's value is written. | Answered (O6). |
| Q-O3 | Values that depend on the hero or the item: excellent level / 20, the Dark Horse, Gold Fenrir, socket values per level or per max HP (P12). | Answered (O7). |
| Q-O4 | Where the option lines' texts live. Today they are `.resx` texts with the value inside the text (`IncreaseMaxHP4`). | Answered (O8). |
| Q-O5 | How the excellent byte is read (P2). | Answered (O9). |
| Q-O6 | Wing additional options: one definition per wing (OpenMU) or shared definitions with a selector table. | One per wing, as OpenMU: the option's number is the type the server sends (0, 2, 3), which replaces the chain of `if`s in `SetItemAttributes`. |
| Q-O7 | How levels are written (P3). | Always a full table from the first level to the last (`"levels": [4, 8, 12, 16]`, or `{"from": 0, "values": [...]}` for harmony), whatever OpenMU's storage; the exchange converts. A value that grows linearly may be written as `{"perLevel": 4}`. |
| Q-O8 | Client values that differ from OpenMU (P5, P6, P7, P8). | The shipped data keeps today's client values, so nothing changes in game; each difference is a known difference of the exchange, and an issue in OpenMU names them. P8 (luck) is the exception to decide on its own: the client shows 5 % and adds 0. |
| Q-O9 | `ItemAddOption.bmd` and `socketitem_<lang>.bmd`: phase 9 or phase 11 (D13 names both under phase 11). | The 380 part of `ItemAddOption.bmd` and the whole of `socketitem_<lang>.bmd` are option groups, so they move in 9b; phase 11 keeps the buff values of `ItemAddOption.bmd` and loses `SocketItem`. |
| Q-O10 | The ancient bonus (P9). | 9a adds the four ancient bonus definitions (strength, agility, energy, vitality; level 1 = +5, level 2 = +10). Phase 10 links them per item of a set, as OpenMU; until then the item's `attackType` picks the definition, and phase 10 removes `attackType`. |
| Q-O11 | Which items can have which group. Today type ranges in code, some tags. | Only the item's links (`"options": ["luck", "excellentPhysical", "physicalDamageOption"]`). The code's ranges go; the bmd import and Item Add set the links from the item's group and kind as today's code does. |
| Q-O12 | What of OpenMU's server-only values the client file keeps (`AddsRandomly`, `AddChance`, `MaximumOptionsPerItem`, `Weight`). | All of them, as D14 says, so the file round-trips with the server. The game does not read them; the editor shows them as server values. |
| Q-O13 | The order, colour and spacing of the option lines in the tooltip (380 red-purple bold, harmony yellow or gray bold, the rest blue, sockets after all markers). | A tooltip style per option type in `Tooltip.json`, and the layout lists the option types in today's order between `options` and `afterOptions`. Sockets and the Horn of Fenrir move before `afterOptions` only if the snapshot stays the same; else they get markers of their own. |
| Q-O14 | The editor. | One window, **Item Options**, in the Items group: the definitions as a list, the selected definition's options with their values and levels, and its users (D26). Item Basics gets the item's links. Item Tooltip's controls grow with each group (option level and type, harmony, 380, sockets, ancient bonus). |
| Q-O15 | The exchange file. | `Data/Items/Sync/` style, a file of its own (`formatVersion`, `source`, `gameVersion`, `definitions`), plus the items' `options` links as an item field of the item exchange (OpenMU's `ItemExchangeField`). |
| Q-O16 | The pull requests of 9a and 9b. | 9a: **9a1** format, loader, attribute names, item links, luck, additional option (not wings) and excellent, the game reading them; **9a2** ancient option and ancient bonus definitions; **9a3** Item Options editor; **9a4** exchange file. 9b: one pull request per group (wings, harmony, guardian, sockets, pets), then the tooltip lines. |

## Tasks

1. [ ] Write this design: answer the open questions one by one (in progress).
2. [ ] The pull requests from Q-O16 (filled in once it is answered).
3. [ ] Move what stays true into `docs/item-data.md`, and point the items
   design (phase 9) and the roadmap (row "Items") at those docs.
4. [ ] Delete this document.
