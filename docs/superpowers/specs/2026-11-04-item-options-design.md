# Item Option Definitions (Items Phase 9) Design (WIP)

> **Draft, 2026-11-04.** Items phase 9 of the
> [items design](2026-09-25-data-driven-items-design.md) and its decision
> D14. Written from the code of MuMain `upstream/main` @ `88cf9d208` and
> OpenMU `master` @ `13a600f3a`. Line numbers are those of these commits.
> sven-n/MuMain#966 (open) makes the client read the guardian (380) flag
> that the server sends; where this design depends on it, it says so.
>
> **To implement:** read Goal, Decisions and Target design; section 14 of
> the target design says where to start, "Pull requests" gives the order.
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
| O6 | Option values (Q-O2, 2026-11-04) | An option's value names the attribute it raises with OpenMU's attribute names (`Stats`, in camelCase: `maximumHealth`, `physicalBaseDmg`) and gives the value in the units the tooltip shows: `{"attribute": "maximumHealth", "percent": {"values": 4}}`, `{"attribute": "defenseBase", "add": {"perLevel": 4}}`, and `addFinal` for OpenMU's `AddFinal` (the value is always an object, section 4). The loader maps each attribute to the client's effect (`AT_*`); an attribute the client does not compute loads with a warning, and its line is shown but not added to the stats. The exchange converts the units (4 % to the factor 1.04 and back). |
| O7 | Values that depend on the hero or the item (Q-O3, 2026-11-04) | A value is a constant, a value per level (O11), or either plus `per` terms: `"per": [{"attribute": "totalLevel", "divisor": 20}]`, `{"attribute": "horseLevel", "times": 2}`. A term multiplies an attribute of the hero, the pet or the item, as OpenMU's related values with `Multiply`; it covers excellent level / 20, Gold Fenrir, the Dark Horse and the socket value kinds 3 to 5. The value is computed when the tooltip is drawn and when the stats are summed, not when the item is made, so level / 20 follows the hero (P12, a behavior change the pull request names). An OpenMU option with another operator is a known difference of the exchange |
| O8 | Option texts (Q-O4, 2026-11-04) | The option lines' texts live in a file of their own, `Data/Items/Tooltips/OptionTexts.json` (`formatVersion` and `texts`, the shape of `Tooltip.json`'s `texts`: a name, then the text by language with `{value}`). An option names its text (`"text": "increaseMaxHp"`); names are looked up only in this file, so they cannot clash with `Tooltip.json`, which stays as it is. A line that several options show (the additional damage of the physical option and of the wings) is one text. The `.resx` texts only options use move into it with their translations, and the values written into them (`Increase Max HP +4%%`) become `{value}`. The harmony and socket option names (bmd today) join it in 9b. About 100 texts; in `Tooltip.json` (99 texts, 1360 lines, 95 % texts) they would have doubled the file |
| O9 | The excellent byte (Q-O5, 2026-11-04) | What the excellent byte means follows from the type of the definitions the item links to, as OpenMU reads it (`ItemSerializerHelper.cs`): a definition of type `excellent` makes it the excellent bits, `wing` the wing option bits (bit = number - 1 for both), the Fenrir types the colour (the value picks the option of that number: 1 black, 2 blue, 4 gold). An item that links two types that use the byte is a load error. "Excellent" (name prefix and colour, durability, price, requirements, the tooltip state `excellent`) then means "has an active option of type `excellent`", not "the byte is above 0", so the wing exceptions in that code go. This applies only where it gives the same result for every item the server can send; a test where it would not stays and becomes a fix of its own (section 8 lists every place, P15). The pull request shows with the tooltip snapshot and the stat tests that nothing changes |
| O10 | Wing additional options (Q-O6, 2026-11-04) | One definition per wing, as OpenMU (`Wings.cs:74-107`): 18 definitions (4 first tier, 5 second tier, 2 capes, 7 third tier) named after the wing (`wingsOfDragon`, `wingOfStorm`), of type `option` like OpenMU's (type `wing` is only the second and third tier options of O9). An option's number is the option type the server sends (0, 2 or 3): Wings of Dragon 0 = recovery +1 % per level, 2 = damage +4 per level; Wing of Storm 0 = recovery, 3 = damage, 2 = defense. This replaces the chain of `if`s in `SetItemAttributes` (`ZzzInfomation.cpp:620-826`), the item files need no field beyond their links, and the exchange is 1:1. Values repeat across the definitions; the texts do not (O8). Shared definitions with a type table per wing were weighed and dropped: one wing with values of its own would need a copy and a table edit, and the import would have to merge and split definitions |
| O11 | Values per level (Q-O7, 2026-11-04) | The form of `ItemStats.json`'s level bonus tables: `values` is a number or a list, and `perLevel` adds that much for every level (`{"perLevel": 4}` for the additional option, `{"values": 50, "perLevel": 5}` for the second tier wings' max HP). A list starts at level 1, or at `firstLevel` (harmony: 0). The level is the option's own level (option, harmony, sphere or ancient bonus level) unless `"levelType": "itemLevel"` says it is the item's + level (OpenMU's `LevelType.ItemLevel`). `"requiresItemLevel": true` is the harmony rule: an option level is active only while the item level is at least that level (OpenMU's `RequiredItemLevel` on every level). A straight line has no upper end, as today's `level * 4`, so any level the server sends keeps its value; tables only were weighed and dropped for that reason. Rules per level range ("from 6, start 10, +2") are not stored: Item Options gets a **Fill levels** action that writes them into a list. The exchange expands a straight line into OpenMU's one value per level, and its import writes an evenly spaced list back as `perLevel` |
| O12 | Differences from OpenMU (Q-O8, 2026-11-04) | Checked against Season 6 sources on the web, one by one. **P5:** the data keeps the client's five second tier wing options; Max AG +50 is in the [Murred FAQ](https://murred.com/FAQ-2-wings.html) ("not applicable to Cape of Lord") and the [InfinityMU wiki](https://wiki.infinitymu.net/index.php?title=Item_Options), attack speed +5 only in the latter. **P6:** the data keeps 200 for the pants' defensive skill and 700 for the boots' max SD, as in every guide found ([guiamuonline](https://www.guiamuonline.com/component/content/article/301-todo-sobre-las-opciones-38?showall=1), [InfinityMU forum](https://forum.infinitymu.net/threads/380-level-item-options.10/)). **P7:** summoner books keep the staff harmony options, as [MuCustoms' Season 6 guide](https://mucustoms.com/harmony-system) lists them ("Staff, Stick, Book Summoner"). **P8:** luck is critical damage rate +5 % in the data, as every source and OpenMU have it; until a client fix right after 9a1 its effect row writes no `SpecialValue` (section 4), so the move changes nothing and the fix is a pull request of its own. P5 to P7 (and P14, P16: O21, O22) are known differences of the option exchange (Q-O15), and an OpenMU issue names them with these sources |
| O13 | `ItemAddOption.bmd` and `socketitem_<lang>.bmd` (Q-O9, 2026-11-04) | Split by meaning. 9b moves the 380 part of `ItemAddOption.bmd` (guardian definitions, OpenMU's `GuardianOption`) and the whole of `socketitem_<lang>.bmd`: the seed options (`SocketOption`), the bonus options (`SocketBonusOption`) and the set bonus, which becomes a combination bonus next to the socket definitions (OpenMU's `ItemOptionCombinationBonus`). Phase 11 keeps only the buff values of `ItemAddOption.bmd` (scrolls, elixirs, seals, charms, potions; `{addOptionValue1}` and `{addOptionDuration}`), which are no options in OpenMU, and `SocketItem` leaves phase 11. The items design's D13 and phase 11 are changed to say so when this design moves into the docs |
| O14 | Ancient bonus (Q-O10, 2026-11-04) | Per item of a set, as OpenMU (`ItemOfItemSet.BonusOption`). 9a4 adds four definitions of type `ancientBonus` (`ancientBonusStrength`, `…Agility`, `…Energy`, `…Vitality`; `{"perLevel": 5}`, so level 1 = +5, level 2 = +10). Until phase 10 the item's `attackType` (1 to 4) picks the definition, so 9a4 changes nothing. Phase 10 links a bonus definition to each item of a set and removes `attackType`, whose only reader is this code. Of OpenMU's 115 set items, 113 have the stat of the client's `attackType`; the Pendant of Ability (13,28) has none on both sides, and the Ring of Fire (13,21) has energy in one set and vitality in the other on the server, energy in the client: phase 10 follows the server there and its pull request names it |
| O15 | Which items have which groups (Q-O11, 2026-11-04) | As `statProfile` and `profileRules` (phase 3c): each item writes the definitions it can have (`"options": ["luck", "physicalDamageOption", "excellentPhysical"]`), and the game reads only that list; the type ranges in `SetItemAttributes`, `CalcExcellentOptions`, `GetItemType` and `IsSocketItem` go. Rules in data (`Options/OptionRules.json`, the shape of `profileRules`: a list of definitions for the items whose `group`, `slot`, `weaponKind`, `wingTier` or `tags` match) give the list an item's data gives it; the bmd import, Item Add and the item editor keep an item's list matching them, and a list written in the file that differs (the Bone Blade with `guardianWeapon`, a wing with its own definition) stays as long as it differs. The first rules are today's code ranges, so a converted item gets what it has today |
| O16 | Server values (Q-O12, 2026-11-04) | The client files keep OpenMU's server-only values, as D14 says ("values, chances, levels"), grouped under `"server"` so it is clear the game does not read them: on a definition `{"addsRandomly": true, "addChance": 0.001, "maximumOptionsPerItem": 2}` (`excellentPhysical`), on a harmony option `{"weight": 40}`. Item Options shows them in a Server box and a weight column, the exchange carries them both ways, and Update from server shows a change made on the server (Luck's chance 0.25 to 0.30) in its diff. A new option group made in MuEditor so reaches OpenMU complete. This differs on purpose from the item exchange, which leaves server-only item fields out (phase 7): an option group without its chances is not a whole definition |
| O17 | Option lines in the tooltip (Q-O13, 2026-11-04) | Layout lines, one per option type, as D72 planned: `{"options": "guardian", "color": "redPurple", "bold": true}` expands into the item's options of that type, each with its text from `OptionTexts.json` (O8). Order, colours, bold and blank rows use what layout lines already have (`when`, `colors`, empty lines with states such as `hasGuardianOptions`, `hasHarmonyOptions` and `harmonyActive`). The `default` layout gets today's order at the `options` marker: guardian, harmony, ancient bonus, wing, the skill lines (no option, they stay code: `{"code": "skillLines"}`), luck, additional option, excellent; the Horn of Fenrir before `afterOptions` and the sockets after the expiry lines, where the code writes them today. The tooltip snapshot shows the output is the same, and its variants grow with each group (380, sockets, wing option types, ancient bonus levels, Fenrir and Dinorant). Layout variants can hide or move a type, and the tooltip editor (#815 step 6) edits these lines like the others |
| O18 | The editor (Q-O14, 2026-11-04) | One window, **Item Options**, in the Items group (Windows > Items), built like Item Formulas: the definitions as a list with search, a type filter and how many items use each; the picked definition's name (D17), type, Server box (O16) and options with attribute, value and text, edited on a copy and taken with Apply after the save checks; value editors that fit the type (a level grid for harmony, one row per element for sockets) and **Fill levels** (O11); its users (D26: a used definition cannot be removed, a rename updates the links, Copy makes a variant). Tabs **Rules** (`OptionRules.json`, O15) and **Texts** (`OptionTexts.json` in the names grid `NamesGrid`, O8). Item Basics gets the item's option links with a picker, marking a list that differs from the rules. Item Tooltip's controls grow with each group (option level and type, harmony option and level, 380, sockets, ancient bonus level). Every change is one step of the undo history; Save Items writes the files |
| O19 | The exchange (Q-O15, 2026-11-04) | A file of its own for the definitions (D11, D14): `formatVersion`, `source`, `gameVersion` and `definitions` in the shape of the client files (O5 to O7, O11, O16), from MuEditor's Export, Import and Update from server and OpenMU's `GET api/items/options/exchange` (phase C). The item exchange gets the field `options` (the item's links, O15), written by an OpenMU `ItemExchangeField`. Client-only and never exchanged: the `text` names, `OptionTexts.json` and `OptionRules.json`. Every import shows a diff per definition and field, as phase 7; `Sync/KnownDifferences.json` gets an `options` list (P5 to P7, P14, P16; O12, O21, O22). Links to a definition the client does not have are refused, so options are imported before items. Phase 10's item sets follow the same pattern |
| O20 | Pull requests (Q-O16, 2026-11-04) | The finer split of "Pull requests": one concern per pull request, 9a in six (format and luck, additional option, excellent, ancient bonus, editor, exchange) plus the luck fix, then phase 10, then 9b in seven (wings, harmony, guardian, sockets, pets and specials, the tooltip lines, the docs). Each move proves itself with the tooltip snapshot, its variants added per group, and the stat tests |
| O21 | The Zen option (Q-O17, 2026-11-04) | `excellentDefense` option 1 is `moneyAmountRate` +30 %, as the client's text and the Season 6 guides (P14); OpenMU's 40 is a known difference of the option exchange and named in the OpenMU issue. |
| O22 | Excellent level / 20 (Q-O18, 2026-11-04) | Configurable through the data: the option's `per` term names the attribute it divides, `level` (the hero's level) or `totalLevel` (level plus master level, OpenMU's `TotalLevel`), and Item Options lets the user pick it. The shipped data writes `level`, the client's behaviour today (P16), so nothing changes; OpenMU's `TotalLevel` is a known difference of the option exchange, and an import from a server that keeps `TotalLevel` shows it in the diff like any other value. |

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
| P14 | The defensive excellent Zen option is +30 % in the client's text, +40 % in OpenMU (`MoneyAmountRate` 1.4, `ExcellentOptions.cs`); Season 6 guides give +30 % ([muonlinefanz](https://muonlinefanz.com/guide/items/excellent)), +40 % appears in a later Webzen event | `ZzzInventory.cpp` (`AT_IMPROVE_GAIN_GOLD`), `ExcellentOptions.cs:67` | 9a3 (O21) |
| P15 | `CalcDurabilityPercent` gives every item whose excellent byte is above 0 the excellent +15 durability, also second and third tier wings with wing options, while `CalcMaxDurability` leaves those wings out; the durability per cent of such a wing is computed against 15 more than its maximum | `ZzzInfomation.cpp:1785`, `ZzzInventory.cpp:1059-1060` | 9a3 keeps it, a fix of its own |
| P16 | Excellent "+level / 20": the client divides the hero's level (`CharacterAttribute->Level`), OpenMU `TotalLevel`, which is level plus master level (`Stats.cs:143`) | `ZzzInfomation.cpp:419`, `ExcellentOptions.cs` | 9a3 (O22) |

## Target design

The finished design, put together from O1 to O20. It is written so that
someone who did not take part in the decisions can implement it: every
section names the files and functions it changes. Field names are this
design's working names; the pull request that first reads a field fixes its
name in `docs/item-data.md`. Points that are no decision of their own but
follow from two of them say so. File and line references are those of
`88cf9d208` (see the note at the top); look a function up by name if the
lines moved.

**How to read the examples.** JSON examples use the real values of the
client (`Engine/Object/ZzzInfomation.cpp`) and of OpenMU's Season 6 data.
Where the two differ, the client's value is written and the difference is
listed (P5 to P8, P14).

### 1. Files

```
src/bin/Data/Items/
├── Group00_Sword.json …          item files: each item gets "options" (section 6)
├── Options/                      NEW, the option definitions (sections 2-4)
│   ├── Luck.json                 9a1
│   ├── Option.json               9a2  the additional option ("+4 … +28")
│   ├── Excellent.json            9a3
│   ├── Ancient.json              9a4  the ancient bonus (the ancient set options come with phase 10)
│   ├── Wings.json                9b1
│   ├── Harmony.json              9b2
│   ├── Guardian.json             9b3
│   ├── Socket.json               9b4
│   ├── Pets.json                 9b5  Horn of Fenrir, Dark Horse, Horn of Dinorant, the Wizard's rings
│   └── OptionRules.json          9a1  which items get which definitions (section 6)
├── Tooltips/
│   ├── Tooltip.json              unchanged until 9b6 (section 9)
│   └── OptionTexts.json          NEW 9a1, the option lines' texts (section 5)
└── Sync/
    └── KnownDifferences.json     not shipped: written by Item Sync on the user's side; gets "options" in 9a6 (section 12)
```

- `Options/` is a subfolder because every `*.json` directly in
  `Data/Items` is read as an item group file (D68).
- Every file follows the phase 2 rules of `docs/item-data.md` ("Files" and "Loading and
  errors"): UTF-8, LF, `formatVersion` first, fields with their
  default value left out, a fixed key order, saved through a temporary file,
  only valid data saved, a newer `formatVersion` is an error, older ones are
  upgraded in memory.
- The file a definition is in is a convention: the loader reads every
  `*.json` in `Options/` except `OptionRules.json`, and a definition's `type`
  says what it is, not its file. A definition name used twice, also across
  files, is fatal.
- The loader goes next to the others in
  `Data/DataHandler/ItemData/ItemJsonStorage.{h,cpp}`
  (`GetItemOptionsDirectory`, `LoadItemOptionsDirectory`,
  `GetOptionTextsFile`, `LoadOptionTextsFile`, `ValidateItemOptionsAsWritten`,
  as `LoadItemStatsFile` and `ValidateItemStatsAsWritten` are built) and is
  called from `CItemDataHandler::Load`
  (`Data/DataHandler/ItemData/ItemDataHandler.cpp:217-270`) after the item
  files and before `FillItemAttributes`, because the item links are resolved
  against it.
- The in-memory model is a new database in `Data/GameData/ItemData/`
  (`ItemOptionDatabase.{h,cpp}`): definitions by name and by index, each
  item's links resolved to indices when loading (D26: names are resolved
  when loading, the game never looks up a name).

### 2. A definition

```json
{
  "formatVersion": 1,
  "definitions": [
    {
      "name": "excellentPhysical",
      "type": "excellent",
      "displayName": {"en": "Excellent Physical Attack Options", "de": "…"},
      "server": {"addsRandomly": true, "addChance": 0.001, "maximumOptionsPerItem": 2},
      "options": [
        {"number": 1, "attribute": "manaAfterMonsterKillMultiplier", "add": {"values": 8}, "text": "manaAfterKill"},
        {"number": 2, "attribute": "healthAfterMonsterKillMultiplier", "add": {"values": 8}, "text": "lifeAfterKill"},
        {"number": 3, "attribute": "attackSpeedAny", "add": {"values": 7}, "text": "increaseAttackSpeed"},
        {"number": 4, "attribute": "physicalBaseDmgIncrease", "percent": {"values": 2}, "text": "increaseDamagePercent"},
        {"number": 5, "attribute": "physicalBaseDmg", "add": {"values": 0, "per": [{"attribute": "totalLevel", "divisor": 20}]}, "text": "increaseDamageLevel20"},
        {"number": 6, "attribute": "excellentDamageChance", "percent": {"values": 10}, "text": "excellentDamageRate"}
      ]
    }
  ]
}
```

**Definition fields.**

| Field | Required | Meaning |
|---|---|---|
| `name` | yes | The definition's name (O5): camelCase, unique across all option files, what items link to. |
| `type` | yes | One of the types of section 3. |
| `displayName` | yes | Translated name (D17: an object by language, `en` required and first). Shown in the editor and sent to OpenMU as `Name`. |
| `server` | no | OpenMU's server-only values (O16): `addsRandomly` (bool), `addChance` (0 to 1), `maximumOptionsPerItem` (int). The game does not read them. |
| `options` | yes | The options, at least one. |

**Option fields.**

| Field | Required | Meaning |
|---|---|---|
| `number` | yes, except types with one option (`luck`, `ancientBonus`), where it is left out and read as 0 | OpenMU's `ItemOption.Number`; what it means depends on the type (section 3). Unique within a definition, except for `guardian` (section 3). |
| `subOptionType` | no | OpenMU's `SubOptionType`: the socket element (`fire`, `water`, `ice`, `wind`, `lightning`, `earth`). Only for `socket`. |
| `attribute` | yes | The attribute it raises, OpenMU's `Stats` name in camelCase (O6, section 4). |
| `add` / `addFinal` / `percent` | exactly one | How it adds, with the value as an object (section 4): `{"values": 7}`, `{"perLevel": 4}`, … A bare number is fatal, so there is one way to write a value. |
| `levelType` | no | `"itemLevel"`: the level of the value is the item's + level, not the option's own level (O11). |
| `requiresItemLevel` | no | `true`: an option level counts only while the item level is at least that level (the harmony rule, O11). |
| `text` | yes | The name of its line in `OptionTexts.json` (O8), or a list of names when the option shows several lines (luck: the Jewel of Soul line and the critical damage line, in that order). |
| `server` | no | `{"weight": 40}`: OpenMU's `IncreasableItemOption.Weight` (O16). |

### 3. Types

The type says which byte of the server's item data holds the option and
what `number` means (P1). The names are OpenMU's `ItemOptionTypes` in
camelCase.

| Type | OpenMU type (GUID in `ItemOptionTypes.cs`) | Read from (section 7) | `number` means | Phase |
|---|---|---|---|---|
| `luck` | Luck | flag 0x02 | – (one option) | 9a1 |
| `option` | Option | flag 0x01 and the option byte | the option type the server sends in the high nibble (0 for most items; 0, 2 or 3 for wings, O10); the option level is the low nibble | 9a2, 9b1 |
| `excellent` | Excellent | excellent byte | 1 to 6, bit `number - 1` | 9a3 |
| `wing` | Wing | excellent byte | 1 to 5, bit `number - 1` (O9) | 9b1 |
| `ancientOption` | AncientOption | ancient byte (with phase 10's sets) | 1 to n, in the order the set gives them | 10 |
| `ancientBonus` | AncientBonus | ancient byte, high nibble = level | – (one option) | 9a4 |
| `harmony` | HarmonyOption | harmony byte, high nibble = number, low nibble = level | 1 to 10 | 9b2 |
| `guardian` | GuardianOption | flag 0x40 (no byte): all options of the item's guardian definition | the item group (OpenMU: 0xF0 for weapons, 7 to 11 for armour); both options of a definition have the same number | 9b3 |
| `socket` | SocketOption | socket bytes: `(sphere level - 1) * 50 + element offset + number` | 0-based within its element | 9b4 |
| `socketBonus` | SocketBonusOption | socket header, high nibble | 0 to 5, unique across the bonus definitions | 9b4 |
| `blackFenrir`, `blueFenrir`, `goldFenrir` | BlackFenrir, BlueFenrir, GoldFenrir | excellent byte, bit test `ExcellentFlags & number` (OpenMU ORs the colours, `ItemSerializerHelper.cs:302-317`; for the single values it sends this equals today's `==`) | the colour value (1, 2, 4) | 9b5 |
| `darkHorse` | DarkHorse | none (always on) | 1 to 2 | 9b5 |

- Two definitions of an item that read the same byte, of any type
  (`excellentPhysical` with `excellentWizardry`, `excellent` with `wing` or a
  Fenrir type, two of type `option`), are a load error (O9, section 10).
- The Horn of Dinorant's options are of type `option`, but its option level
  is a set of bits, not a level (OpenMU's "fixed option": type `option`
  without levels). Its definition has `"bits": true`; each option's `number`
  is its bit value (1 absorb 5 %, 2 max AG +50, 4 speed +5), and the option
  level selects the options whose bit is set (9b5).
- The Wizard's rings have options that need no byte (fixed 10 % / 10 % /
  speed 10, Lethal 15 / 15 / 10, at levels 0 and 3 only for the Wizard's
  Ring; `CalcExcellentOptions` `ZzzInfomation.cpp:453-483`). They become a
  definition of type `option` with `"always": true` and, for the Wizard's
  Ring, `"itemLevels": [0, 3]` (9b5). OpenMU writes them as the ring's base
  power-ups, not as options; the exchange leaves this type out and it is a
  known difference.
- Skill is no option (Scope): it stays the item's `skill` field.

### 4. Values

**How a value adds (O6).** `add` adds the value as it is, `addFinal` adds it
after the other additions (OpenMU's `AddFinal`, the wings' damage), and
`percent` adds the value in per cent (OpenMU's `Multiplicate` with the
factor `1 + value / 100` for attributes that multiply, or `AddRaw` with
`value / 100` for chances such as `excellentDamageChance`; the exchange
knows which from the attribute, section 12).

**Levels (O11).** The value is always an object, with the field names of
`ItemStats.json`'s level bonus tables:

```json
{"values": 7}                                   // a constant: 7 at every level
{"perLevel": 4}                                 // 4 * L: 4, 8, 12, … no upper end
{"values": 50, "perLevel": 5}                   // 50 + 5 * L
{"values": [7, 8, 9, 10, 11]}                   // a list: levels 1 to 5
{"firstLevel": 0, "values": [3, 4, 5, …, 25]}   // a list: levels 0 to 13
```

How the value at a level `L` is computed:

- **Number form** (`values` a number, default 0, plus `perLevel`, default
  0): `values + perLevel * L`.
- **List form** (`values` a list): the entry `L - firstLevel`; a level above
  the list takes the last entry. A list together with `perLevel` is fatal
  (unlike `ItemStats.json`, where `perLevel` continues a list above +15; the
  options need neither).
- `firstLevel` defaults to 1, or to 0 with `"levelType": "itemLevel"`.

Which level `L` is:

| The option's type | `L` |
|---|---|
| `option`, `harmony`, `socket`, `ancientBonus` | the option's own level from the server (option level, harmony level, sphere level, bonus level) |
| any type, with `"levelType": "itemLevel"` | the item's + level, 0 to 15 (second tier wing HP and MP, the Cape of Lord's command) |
| the types without a level (`luck`, `excellent`, `wing`, `guardian`, `socketBonus`, the Fenrir types, `darkHorse`) | 1, so a constant is its value |

Whether an option is there at all is decided by the server's byte (section
7), not by its value: an option level of 0 means no additional option, and a
value of 0 still shows a line (the level / 20 option of a level 15 hero).

**Terms on hero or item attributes (O7).** `per` adds terms to the value:

```json
{"values": 0, "per": [{"attribute": "level", "divisor": 20}]}                           // excellent level / 20 (P16)
{"values": 5, "per": [{"attribute": "totalAgility", "divisor": 20}, {"attribute": "horseLevel", "times": 2}]}  // Dark Horse
```

A term is `attribute * times` or `attribute / divisor`, each term rounded
down on its own, as the client's integer code does today. The attributes a
term can read, with OpenMU's names where OpenMU has the attribute:

| Term attribute | Client source | OpenMU |
|---|---|---|
| `level` | `CharacterAttribute->Level` | `Level` |
| `totalLevel` | `Level + MasterLevel` | `TotalLevel` (`Stats.cs:143`) |
| `masterLevel` | `Master_Level_Data.nMLevel` | `MasterLevel` |
| `totalStrength`, `totalAgility`, `totalVitality`, `totalEnergy`, `totalLeadership` | the hero's stats with their additions (`CharacterAttribute->Strength + AddStrength`, …) | the same names |
| `maximumHealth`, `maximumMana` | `CharacterAttribute->LifeMax`, `ManaMax` | the same names |
| `horseLevel`, `ravenLevel` | the Dark Lord pets' levels (`giPetManager`) | `HorseLevel`, `RavenLevel` |
| `itemLevel` | the item's + level | none: client-only; the exchange writes it as OpenMU's item level related value where OpenMU has one, else as a known difference |

Look the client sources up by name when implementing; the table names the
meaning, the pull request picks the exact field.

**When a term is computed (P12).** The value of an option with `per` terms
is computed by one function, `GetItemOptionValue(const ITEM&, int slot)`
(section 8), every time it is asked: by the tooltip, and by the stat code
(`PlusSpecial`, `PlusSpecialPercent`, `PlusSpecialPercent2`,
`ZzzInfomation.cpp:1666-1708`, which then call it instead of reading
`SpecialValue` for these options). `SpecialValue` keeps the value at the
time the item was made for the price, mixes and the comparison, as today.

**Attribute names and the client's effects (O6).** The client keeps its
option list `ITEM::Special[]` / `SpecialValue[]` (`Core/Globals/_struct.h`)
as the result of the definitions, so the price (`ItemValue`), mixes
(`MixMgr.cpp`), the item comparison and the stat code (`PlusSpecial*`,
`ZzzInfomation.cpp:1666`) keep reading what they read today. This follows
from the goal (nothing changes) and O6. The bridge is one table in code,
`Data/GameData/ItemData/ItemOptionEffects.cpp`, from (type, attribute, how it
adds) to the client's effect id (`AT_*`, `Core/Globals/_enum.h`, e.g.
`:288-317` and `AT_IMPROVE_MAX_MANA` / `AT_IMPROVE_MAX_AG` at `:3633`).
The rows for 9a; the last column says whether the option writes its value
into `SpecialValue`, as today's code does or does not:

| Type | Attribute | Adds | Effect | Writes `SpecialValue` |
|---|---|---|---|---|
| `luck` | `criticalDamageChance` | percent | `AT_LUCK` | no (stays 0); yes from the luck fix on (P8) |
| `option` | `physicalBaseDmg` | add | `AT_IMPROVE_DAMAGE` | yes |
| `option` | `wizardryBaseDmg` | add | `AT_IMPROVE_MAGIC` | yes |
| `option` | `curseBaseDmg` | add | `AT_IMPROVE_CURSE` | yes |
| `option` | `baseDamageBonus` | add | `AT_IMPROVE_DAMAGE` (the magic swords; their second line is the option's second text) | yes |
| `option` | `defenseRatePvm` | add | `AT_IMPROVE_BLOCKING` | yes |
| `option` | `defenseBase` | add | `AT_IMPROVE_DEFENSE` | yes |
| `option` | `healthRecoveryMultiplier` | percent | `AT_LIFE_REGENERATION` | yes |
| `option` | `maximumMana` | percent | `AT_IMPROVE_MAX_MANA` | yes |
| `option` | `maximumAbility` | percent | `AT_IMPROVE_MAX_AG` | yes |
| `excellent` | `maximumHealth` | percent | `AT_IMPROVE_LIFE` | no |
| `excellent` | `maximumMana` | percent | `AT_IMPROVE_MANA` | no |
| `excellent` | `armorDamageDecrease` | percent | `AT_DECREASE_DAMAGE` | no |
| `excellent` | `damageReflection` | percent | `AT_REFLECTION_DAMAGE` | no |
| `excellent` | `defenseRatePvm` | percent | `AT_IMPROVE_BLOCKING_PERCENT` | no |
| `excellent` | `moneyAmountRate` | percent | `AT_IMPROVE_GAIN_GOLD` | no |
| `excellent` | `excellentDamageChance` | percent | `AT_EXCELLENT_DAMAGE` | no |
| `excellent` | `physicalBaseDmg` / `wizardryBaseDmg` with a `level` term | add | `AT_IMPROVE_DAMAGE_LEVEL` / `AT_IMPROVE_MAGIC_LEVEL` | yes |
| `excellent` | `physicalBaseDmgIncrease` / `wizardryBaseDmgIncrease` | percent | `AT_IMPROVE_DAMAGE_PERCENT` / `AT_IMPROVE_MAGIC_PERCENT` | yes (2) |
| `excellent` | `attackSpeedAny` | add | `AT_IMPROVE_ATTACK_SPEED` | yes (7) |
| `excellent` | `healthAfterMonsterKillMultiplier` | add | `AT_IMPROVE_GAIN_LIFE` | no |
| `excellent` | `manaAfterMonsterKillMultiplier` | add | `AT_IMPROVE_GAIN_MANA` | no |
| `ancientBonus` | `totalStrength` … `totalVitality` | add | none: read by `CSItemOption::GetDefaultOptionValue` (`CSItemOption.cpp:546-594`), which asks the definition | – |

"No" keeps today's `SpecialValue` of 0 where the stat code uses a fixed
number instead of the value (the blocking 10 % of `AT_IMPROVE_BLOCKING_PERCENT`,
`ZzzInfomation.cpp:2529-2538`); these readers are not changed in 9a, so the
snapshot and the stat tests stay the same. A later pull request that makes
such a reader use the value says so. The rows for 9b are added by their pull
requests. An attribute with no row loads with a warning, and its line is
shown but adds nothing (O6).

### 5. Option texts

`Tooltips/OptionTexts.json` (O8) has the shape of `Tooltip.json`'s `texts`:

```json
{
  "formatVersion": 1,
  "texts": {
    "luckJewelOfSoul":      {"en": "Luck (success rate of Jewel of Soul +25%)", "de": "…"},
    "luckCriticalDamage":   {"en": "Luck (critical damage rate +{value}%)", "de": "…"},
    "additionalDamage":     {"en": "Additional Dmg +{value}", "de": "…"},
    "increaseMaxHp":        {"en": "Increase Max HP +{value}%", "de": "…"},
    "increaseDamageLevel20":{"en": "Increase Damage +level/20", "de": "…"}
  }
}
```

- `{value}` is the option's value at the item's level from the definition,
  as an integer. It does not come from `SpecialValue`: luck shows 5 while its
  `SpecialValue` stays 0 until the luck fix (section 4). A text
  without `{value}` shows none (the level / 20 lines keep their wording).
- The texts come from the `.resx` texts the code uses today
  (`GetSpecialOptionText`, `Engine/Object/ZzzInventory.cpp:1074-1298`, and
  the luck line at `:2148`), with all their translations (`src/Localization/
  Game.<lang>.resx`); a value written into a text (`Increase Max HP +4%%`)
  becomes `{value}`, and `%%` becomes `%`. A `.resx` text only these lines
  use is removed in the same pull request; one that other code uses stays.
- A text name an option names but the file lacks is an error (the line is
  left out); a text no option names is a warning.
- **Languages:** every language the `.resx` texts have (12 today, with
  `zh-CN`), `en` first, the others sorted. `OptionTexts.json` gets the same
  locale check as `Tooltip.json` (`ValidateTooltipTextLocales`,
  `ItemDataHandler.cpp:224`).
- **Names in this document:** the text names the code uses today are the
  generated `I18N::Game::*` identifiers (`IncreaseMaxHP4`, `AdditionalDmgD`);
  their `.resx` key is the English text itself (`Increase Max HP +4%%`).
  Removing a `.resx` text means removing its `<data>` entry in every
  `Game.<lang>.resx`; `tools/ResxGen` regenerates the identifiers at build
  time (`docs/translation-system.md`).

### 6. Item links and rules

**On the item (O15).** Each item file entry gets `options`, the names of
the definitions the item can have:

```json
{"number": 0, "name": {"en": "Kris"}, "options": ["luck", "physicalDamageOption", "excellentPhysical"], …}
```

- The field goes into `ItemDefinition` (`Data/GameData/ItemData/ItemDefinition.h`,
  next to `statProfile`) and the item JSON format (`ItemJsonFormat.cpp`,
  next to `visit("statProfile", …)` at `:74`).
- **The order does not matter** to the game (section 8 orders by type). The
  writer sorts the list by the type order of section 8, then by name, so
  saving gives the same file.
- **Missing, empty and written**, as `statProfile` (an empty profile follows
  the rules, `ItemDataValidation.cpp:268-279`):
  - `options` left out: the item follows the rules; the loader sets the
    list the rules give. The writer leaves the field out when the item's
    list equals what the rules give.
  - `"options": []`: the item has no options even if a rule gives some (the
    writer writes `[]` only then).
  - a list: the item has exactly these.
- A link to a definition that does not exist is an error (the link is
  dropped).

**Rules (O15).** `Options/OptionRules.json` has the shape of `profileRules`
in `ItemStats.json`: each rule gives a list of definitions (`options`, which
may be empty) to the items it matches; the first rule that matches wins; an
item no rule matches gets no options.

- Conditions: `group`, `slot`, `weaponKind`, `wingTier`, `tags` and
  `requirements`, as `profileRules`, plus two new ones: `numbers` (item
  numbers; with `group`) and a `!` before a tag for "without the tag".
- `StatProfileRule` (`Data/GameData/ItemData/StatProfileRule.{h,cpp}`) has
  neither `numbers` nor `!tag`, and its target is one profile name. So the
  condition part moves into a struct of its own (`ItemRuleConditions`, same
  folder), which `StatProfileRule` keeps using unchanged and a new
  `OptionRule` uses with the two new conditions. `ItemStats.json` and its
  `formatVersion` do not change (`docs/item-data.md:799`: a new condition in
  `profileRules` would need a new version).

```json
{
  "formatVersion": 1,
  "rules": [
    {"options": ["excellentPhysical"], "tags": ["ammunition"]},
    {"options": ["luck", "physicalAndWizardryDamageOption", "excellentPhysical"], "tags": ["magicSword"]},
    {"options": ["luck", "physicalDamageOption", "excellentPhysical"], "group": [0, 1, 2, 3, 4]},
    {"options": ["luck", "curseDamageOption", "excellentWizardry"], "group": [5], "tags": ["summonerBook"]},
    {"options": ["luck", "wizardryDamageOption", "excellentWizardry"], "group": [5]},
    {"options": ["luck", "defenseRateOption", "excellentDefense"], "group": [6]},
    {"options": ["luck", "defenseOption", "excellentDefense"], "group": [7, 8, 9, 10, 11]},
    {"options": ["luck"], "group": [12], "numbers": [0, 1, 2, 3, 4, 5, 6, 41, 42, 49]},
    {"options": ["luck"], "slot": ["wings"], "wingTier": ["third"]},
    {"options": ["luck"], "group": [13], "numbers": [30]},
    {"options": ["jewelryHealthRecovery", "excellentDefense"], "group": [13], "numbers": [8, 9, 21, 22, 23]},
    {"options": ["ringOfMagicMana", "excellentDefense"], "group": [13], "numbers": [24]},
    {"options": ["jewelryHealthRecovery", "excellentWizardry"], "group": [13], "numbers": [12, 25, 27]},
    {"options": ["jewelryHealthRecovery", "excellentPhysical"], "group": [13], "numbers": [13, 26]},
    {"options": ["pendantOfAbilityAg", "excellentPhysical"], "group": [13], "numbers": [28]}
  ]
}
```

These are 9a's rules at its end (after 9a3). Each 9a pull request adds the
definitions of its group to the lists; 9a1 writes the same rules with
`luck` only, and the first rule as `{"options": [], "tags": ["ammunition"]}`
so that ammunition does not fall through to the groups 0 to 4 rule. They
reproduce today's code ranges:

- `luck`: groups 0 to 11 without `ammunition`, and the wings today's luck
  code names (`ZzzInfomation.cpp:523-551`): `ITEM_WING` to
  `ITEM_WINGS_OF_DARKNESS` (12/0 to 12/6), `ITEM_WING_OF_CURSE` and
  `ITEM_WINGS_OF_DESPAIR` (12/41, 12/42), the Cape of Fighter (12/49), the
  Cape of Lord (13/30) and every third tier wing. Small wings get none.
- Excellent: defensive for groups 6 to 11 and the rings 13/8, 13/9, 13/21 to
  13/24; offensive for groups 0 to 5 (also ammunition, as the code's range
  includes it) and the pendants 13/12, 13/13, 13/25 to 13/28, wizardry for
  group 5 and the pendants of Lightning (13/12), Ice (13/25) and Water
  (13/27) (`CalcExcellentOptions`, `ZzzInfomation.cpp:375-450`).
- The additional option: as the table in section 15. **Do not** use the slot
  `ring`: it also holds the transformation rings, the seals, the Wizard's
  rings and the gem rings, which have no options.
- Today's code also gives the life regeneration option to every other item
  from `ITEM_RING_OF_ICE` to the end of group 13 (transformation rings,
  Loch's Feather, …) when the server sends an option level. OpenMU attaches
  no option to them, so the server never does; the rules leave them out, and
  the 9a2 pull request lists them as the one place the links differ from
  the code. Its snapshot changes for them: the `option` and `full` variants
  write an (empty) line for them today (`ZzzInventory.cpp:1149`, the loop
  counts the line even when the text is empty); 9a2 re-records these hashes
  and names the items.

**Kept matching**, as `statProfile`: `FollowStatProfile` and
`CItemDataHandler::WithDerivedFields` (`ItemAttributeConversion.h:58`,
`ItemDataHandler.cpp:365-388`, called at `:703` and `:957`) are the model.
The bmd import, Item Add and every edit that changes what a rule reads
(group, number, slot, weapon kind, wing tier, tags, requirements) give the
item the list the rules give it now, unless its list differed from what the
rules gave it before the edit; then the list stays (the Bone Blade with
`guardianWeapon`). With `options` left out (above) this is automatic.

### 7. Reading an item from the server

`ParseItemData` (`UI/Inventory/ItemMng.cpp:11-66`) keeps reading the bytes
into `ItemCreationParams` and `ITEM` as today (with #966's guardian flag).
What changes is how `SetItemAttributes` (`ZzzInfomation.cpp:486-834`) turns
them into options: instead of the type ranges, it takes the item's linked
definitions in the type order of section 8 and, for each, reads its byte as
section 3 says:

- `luck`: `ITEM::HasLuck`.
- `option`: `ITEM::OptionLevel` (level) and `ITEM::OptionType` (number). The
  option whose `number` equals `OptionType` counts at level `OptionLevel`;
  level 0 means none.
- `excellent`, `wing`: `ITEM::ExcellentFlags`, each option whose bit
  `number - 1` is set.
- Fenrir types: the bit test of section 3 (9b5).
- `ancientBonus`: `ITEM::AncientBonusOption` is the level (9a4: of the
  definition the item's `attackType` picks; from phase 10 on, of its set's).
- `harmony`, `guardian`, `socket`, `socketBonus`: from 9b2 to 9b4, from the
  fields `ParseItemData` fills today.

The result is written into `Special[]` / `SpecialValue[]` through the table
of section 4, in the order of today's code (section 8). **Each slot also
records where it came from**: a new array on `ITEM`,
`SpecialSource[MAX_ITEM_SPECIAL]`, holds the definition index and option
index (two `std::uint16_t`, or one packed value) of every slot that came
from a definition, and a "none" value for slots written by code (the skill,
and until 9b the groups not moved yet). The tooltip (section 9) and
`GetItemOptionValue` (section 4) find the option through it.

### 8. In the game

**Order.** `SetItemAttributes` writes today: wing options, skill, luck,
additional option, excellent options. The new code writes the options of the
linked definitions in this order of types: `wing`, then the skill (as
today, from the item's `skill`), `luck`, `option`, `excellent`. Within a
definition, options are written in the order of today's bit tests (excellent
from bit 5 down to bit 0; the tooltip shows them in this order). Phase 9b6
moves the order into the layout (O17).

**"Excellent" (O9).** One function, `GameLogic::Items::HasExcellentOptions`,
answers "has an active option of type `excellent`": on an `ITEM` from its
`SpecialSource`, and for code that has only the type and the byte (the
editor) from the item's links and the byte. It replaces the tests of the
excellent byte that mean "excellent", **but only where it gives the same
result for every item the server can send**; where it would not, the test
stays as it is in 9a3 and becomes a problem with a fix of its own (P15).
`grep -n "ExcellentFlags" src/source` lists every place; the 9a3 pull
request goes through all of them. The known ones:

| Place | Test today | 9a3 |
|---|---|---|
| `StandardItemName` (`ZzzInventory.cpp:1604-1616`), name prefix and colour | `ExcellentFlags > 0`, no exception | `HasExcellentOptions`. Same for server items: wings and the Horn of Fenrir get their names from `nameLines` (`Group12_Wing.json`, `Group13_Helper.json`); check it per item in the snapshot. Snapshot variants that set the byte on items that cannot be excellent (`full` sets 63 on every item, `ItemTooltipSnapshot.cpp:31`) change: 9a3 re-records them and lists the items. |
| `CalcMaxDurability` (`ZzzInventory.cpp:1056-1061`) | `> 0`, not second or third tier wings, not `IsDivineArchangelWeapon` | `HasExcellentOptions` and still not `IsDivineArchangelWeapon` (those weapons link `excellentPhysical`, so that exception stays); the wing exception goes |
| `CalcDurabilityPercent` (`ZzzInfomation.cpp:1785`) | `(excellentFlags & 63) > 0`, no exception | **stays** (P15: today it gives wings with wing options the +15 that `CalcMaxDurability` leaves out; changing it changes real wings) |
| `ComputeItemStats` (`GameLogic/Items/ItemStats.cpp:138-201`; wing checks at `:151`, `:186`, `:199`) | `ItemStatInput::excellentFlags` (`ItemStats.h:35-43`) and its own wing checks | `ItemStatInput` gets `bool excellent`, set by the caller (`SetItemAttributes` from `HasExcellentOptions`, the editor from the links); the wing checks go. `excellentFlags` stays in the input for whatever else reads it. |
| Tooltip state `excellent` (`ItemTooltipValues.cpp:176`) | `ExcellentFlags > 0` | `HasExcellentOptions`; same snapshot rule as the name |
| Fenrir model and colour (`ZzzCharacter.cpp:246-250` and others) | `== 1 / 2 / 4` | stays until 9b5 |

**What stays code.** The stat code, the price, mixes and the comparison
read `Special[]` as today. The skill lines (mana per skill, the class notes)
stay code. `SpecialValue` is computed per option from section 4 when the
item is made. The values with `per` terms are computed again by
`GetItemOptionValue` each time the tooltip or the stat code asks
(section 4, P12); that is the one intended behaviour change of 9a3, named in
its pull request. Whether `level / 20` reads the hero's level or level plus
master level is the data's choice (O22).

### 9. Tooltip

**In 9a.** The tooltip's option loop (`BuildItemTooltipText`,
`ZzzInventory.cpp:2134-2172`) stays. For a slot whose `SpecialSource`
(section 7) names a definition, it writes that option's texts from
`OptionTexts.json` with `{value}` from `GetItemOptionValue` (one line per
text name, so luck's two lines and the magic swords' second line come from
the data), instead of calling `GetSpecialOptionText`
(`ZzzInventory.cpp:1074`) and the special cases after it (`:2148-2171`). Slots
without a source (the skill) go through `GetSpecialOptionText` as today. The
output is the same.

**In 9b6 (O17).** The option lines become layout lines in `Tooltip.json`'s
`default` layout, one per type. A line `{"options": "<type>", …}` expands
into the item's active options of that type, each with its text; it takes
the line fields layout lines already have (`color`, `colors`, `bold`,
`when`). 9b6 adds two new kinds of layout line to `ItemTooltipLayouts.cpp`
(`:27-35` lists today's kinds): `options` and `code` (a named code block,
for the skill lines). New states for `when`: `has<Type>Options` (e.g.
`hasGuardianOptions`, `hasHarmonyOptions`) and `harmonyActive`. The default
layout then reads, at the `options` marker, as today's order:

```json
{"section": "options"},
{"text": "", "when": ["hasGuardianOptions"]},
{"options": "guardian", "color": "redPurple", "bold": true},
{"text": "", "when": ["hasGuardianOptions"]},
{"text": "", "when": ["hasHarmonyOptions"]},
{"options": "harmony", "bold": true, "color": "yellow", "colors": [{"when": ["!harmonyActive"], "color": "gray"}]},
{"text": "", "when": ["hasHarmonyOptions"]},
{"options": "ancientBonus", "color": "blue"},
{"options": "wing", "color": "blue"},
{"code": "skillLines"},
{"options": "luck", "color": "blue"},
{"options": "option", "color": "blue"},
{"options": "excellent", "color": "blue"},
{"id": "blankAfterOptions", "text": ""},
```

The Horn of Fenrir's lines (`{"options": "blackFenrir"}` and the others)
go before `{"section": "afterOptions"}` and the sockets after the expiry
lines, where the code writes them today (`ZzzInventory.cpp:2178-2245`,
`:2380`). `Tooltip.json` gets `formatVersion` 3, because a version 2 reader
would not know these lines. The gem jewelry's stop (`ITEM_SAPPHIRE_RING` to
`ITEM_SAPPHIRE_NECKLACE`, `:2137`) becomes `"when": ["!tag:gemJewelry"]` on
the option lines, as the layout's blank line already has it.

### 10. Loading and checks

The three levels of `docs/item-data.md` ("Loading and errors") and
`ItemDataIssue` (`Data/GameData/ItemData/ItemDataIssue.h`); all show in Data
Checks (`MuEditor/UI/DataChecks/`, a new area as `CreateLookCheckArea`).

- **Fatal:** invalid JSON; a missing or unsupported `formatVersion`; a
  definition without `name`, `type` or `options`; an unknown `type`; a name
  used twice; an option without `attribute`, or with none or more than one of
  `add`, `addFinal`, `percent`; a value of the wrong kind; a `number` used
  twice in a definition (except `guardian`); `OptionRules.json` naming a
  definition that does not exist. (A rule with an empty `options` list is
  valid.)
- **Error:** an item linking a definition that does not exist (the link is
  dropped); an item linking two definitions that read the same byte or
  flag, of any type (section 3; the later one in the sorted list is
  dropped); an option naming a text `OptionTexts.json` lacks (the line is
  left out); an option text without `en`.
- **Warning:** an attribute the client has no effect for (section 4: shown,
  not applied); a definition no item and no rule links (except
  `ancientBonus` and `ancientOption`, which `attackType` and phase 10's sets
  link); a text no option names; an item whose list differs from its rules while it has no list;
  an unknown field (at game load and in the editor, as for the item files,
  `docs/item-data.md:943`).

The automated data test (`tests/data/test_item_database_files.cpp`, "Shipped
item data loads without problems") loads `Options/` and `OptionTexts.json`
with the items, so a pull request with broken option data fails its checks.

### 11. Editor

**Item Options (O18)**, a new tool in `src/MuEditor/UI/ItemTools/`
(`MuItemOptionsUI.{h,cpp}`), registered as the other item tools are: a bool
in `UI/Common/MuEditorToolWindows.h`, an id in `UI/Common/MuEditorWindowIds.h`,
an entry in `ItemToolEntries` (`UI/Common/ToolGroups.cpp:38`, the array size
grows by one), its title an `I18N::Editor` text. Built like Item Formulas
(`MuItemFormulasUI.cpp`):

```
┌ Item Options ──────────────────────────────────────────────────────────────┐
│ [Definitions] [Rules] [Texts]                                              │
│ [search……]  Type: [all ▾]                                                  │
│ ┌ Definitions ─────────┐ ┌ excellentPhysical ─────────────────────────────┐ │
│ │ luck             412 │ │ Name [Excellent Physical Attack Options] (en ▾) │ │
│ │ physicalDmgOpt.  188 │ │ Type excellent                                  │ │
│ │▸excellentPhysical156 │ │ Server ☑ random  chance [0.001]  max/item [2]   │ │
│ │ …                    │ │ # │ attribute            │ value     │ text     │ │
│ │ [New] [Copy]         │ │ 3 │ attackSpeedAny       │ +7        │ incr…    │ │
│ │ [Rename] [Remove]    │ │ 5 │ physicalBaseDmg      │ lvl / 20  │ incr…    │ │
│ └──────────────────────┘ │ Used by: 156 items ▸ Kris, Short Sword, …       │ │
│                          │ [Fill levels…]   [Apply] [Revert]               │ │
│                          └─────────────────────────────────────────────────┘ │
└────────────────────────────────────────────────────────────────────────────┘
```

- **Definitions**: the list with search, type filter and how many items link
  each; the picked definition edited on a copy and taken with **Apply** after
  the save checks (as Item Formulas); a value editor that fits the value (a
  number, a level grid for lists, `per` terms); **Fill levels** writes a
  list from a start level, a start value and a step (O11); **Used by**
  lists the items; D26: a definition that is used cannot be removed (the
  list shows why), **Rename** updates the links and the rules, **Copy**
  makes a variant.
- **Rules**: `OptionRules.json` as a list, with how many items each rule
  matches; a changed rule lists the items whose list it changes before it is
  applied.
- **Texts**: `OptionTexts.json` in the names grid (`UI/Common/NamesGrid`),
  names × languages, missing translations counted.
- **Item Basics** gets the item's `options` as a list with a picker of
  definitions; a list that differs from its rules is marked, with "Use the
  rules" to reset it.
- **Item Tooltip** (`MuItemTooltipUI.cpp:153-161`): the Option checkbox
  becomes an option level (0 to 7) and type (for wings); Excellent becomes
  one checkbox per option of the item's excellent or wing definition; with
  9a4 an ancient bonus level (0 to 2); 9b adds harmony option and level, 380,
  sockets.
- Every change is one step of the item undo history
  (`ItemEditHistory.cpp`); Save Items writes the option files with the item
  files.

### 12. Exchange

**The option exchange file (O19)**, MuEditor's Export, Import and Update
from server, next to the item exchange (`Data/GameData/ItemData/ItemExchangeFile.{h,cpp}`,
the Item Sync tool):

```json
{
  "formatVersion": 1,
  "source": "MuMain",
  "gameVersion": "season6",
  "definitions": [
    {"name": "excellentPhysical", "type": "excellent",
     "displayName": {"en": "Excellent Physical Attack Options", "de": "…"},
     "server": {"addsRandomly": true, "addChance": 0.001, "maximumOptionsPerItem": 2},
     "options": [{"number": 3, "attribute": "attackSpeedAny", "add": {"values": 7}}, …]}
  ]
}
```

- The definitions in the shape of the client files without `text`; the
  texts and `OptionRules.json` are client-only. The types without an OpenMU
  counterpart (the Wizard's rings' `always` options) are left out.
- **The item field `options`**: the item exchange gets the field `options`
  (the links), listed in its `fields` (format version 2), written on
  OpenMU's side by an `ItemExchangeField` (`ItemExchangeField.cs`).
  Importing items whose links name a definition the client does not have is
  refused with a message, so the options file is imported first.
- **Mapping to OpenMU** (phase C does OpenMU's side): a definition is
  OpenMU's `ItemOptionDefinition`; its `name` is matched to OpenMU's
  definition by the fixed GUID of `ItemOptionDefinitionNumbers` (a table in
  the exchange code, e.g. `excellentPhysical` ↔ `00000083-0013-…`), else by
  type and English name (O5, P4). An option is an `IncreasableItemOption`
  with `Number`, `SubOptionType`, `OptionType` (the type's GUID), `Weight`
  (`server.weight`) and `LevelType`; its value is a `PowerUpDefinition` on
  the attribute: `add` → `AddRaw`, `addFinal` → `AddFinal`, `percent` →
  `Multiplicate` with `1 + v/100`, or `AddRaw` with `v/100` for the
  attributes OpenMU keeps as fractions (the exchange code keeps this list;
  for 9a: `criticalDamageChance`, `excellentDamageChance`,
  `damageReflection`, `armorDamageDecrease`, `healthRecoveryMultiplier`;
  9b adds its own, e.g. `defenseIgnoreChance`). The two "after kill"
  attributes are the exception of section 15: `add` 8 ↔ `AddRaw` 1/8; `per` terms →
  `RelatedValues` with `Multiply`; a level list or `perLevel` → one
  `ItemOptionOfLevel` per level (`RequiredItemLevel` = the level with
  `requiresItemLevel`). OpenMU's level 1 in the base value (P3) is read and
  written as level 1.
- **Every import shows a diff** per definition, option and field, checked
  by hand, as phase 7; Apply is one undo step.
- **Known differences**: `Sync/KnownDifferences.json` gets
  `"options": [{"definition": "secondTierWing", "fields": ["options.4", "options.5"]}, …]`
  for P5 to P7, P14 and P16 (O12, O21, O22); they are shown unchecked.

### 13. Proof

Each move pull request shows that the game is the same:

- **Tooltip snapshot** (`tests/data/item_tooltip_snapshot.txt`,
  `tests/data/test_item_tooltips.cpp`, variants in
  `Engine/Object/ItemTooltipSnapshot.cpp:22-70`): unchanged hashes. Its
  variants grow with the groups: every option level 1 to 7 and option type
  for 9a2 and 9b1, every single excellent bit for 9a3, ancient bonus level 1
  and 2 for 9a4, the wing bits for 9b1, harmony options per kind for 9b2, 380
  for 9b3, sockets for 9b4, the Fenrir colours and Dinorant bits for 9b5. New
  variants are recorded on `main` first (in the pull request's first commit,
  with the old code), then the move must keep them. Where a pull request
  says hashes change (9a2: the group 13 items without links, section 6; 9a3:
  synthetic variants of items that cannot be excellent, section 8), it
  re-records exactly those and lists the items in its description.
- **`Special[]` test**: for every item and every variant, the old code's
  `Special[]` / `SpecialValue[]` equals the new one (a test that runs both
  while the old code still exists, removed with it), except where a pull
  request names a change (P12 in 9a3).
- **Stat tests** (`tests/data/`, the item stat and requirement tests):
  unchanged.
- **Data test**: the shipped option files load without problems (section 10).

### 14. Where to start (9a1)

For whoever picks this up: 9a1 is the first pull request and the base of all
others.

1. Read `docs/item-data.md` ("Files", "Loading and errors", "Item stats by
   level" for `values` / `perLevel` and `profileRules`), this design's
   sections 1 to 7 and 10, and `docs/CODING_RULES.md`.
2. Add `ItemOptionDatabase` (section 1), the loaders, **writers** and
   validators in `ItemJsonStorage` for `Options/*.json`, `OptionRules.json`
   and `OptionTexts.json` (Save Items writes them from 9a1 on, because the
   conversion and the bmd import write the item lists), and the call in
   `CItemDataHandler::Load`.
3. Add `options` to `ItemDefinition` and the item JSON format with the
   missing / empty / written rules of section 6; move the conditions of
   `StatProfileRule` into `ItemRuleConditions` and add `OptionRule` with
   `numbers` and `!tag`; derive the lists where `statProfile` is derived
   (section 6, "Kept matching").
4. Write `Options/Luck.json` (one definition, `luck`, type `luck`, one
   option: `criticalDamageChance`, `"percent": {"values": 5}`, texts
   `["luckJewelOfSoul", "luckCriticalDamage"]`, server: random, chance
   0.25, 1 per item), `Tooltips/OptionTexts.json` with these two texts from
   the `.resx` (`Luck (success rate of Jewel of Soul +25%%)` and
   `Luck (critical damage rate +5%%)`, `Game.en.resx:238` and `:256`, all
   languages; the second becomes `…+{value}%`), and `OptionRules.json` as in
   section 6 with `luck` only. Items then follow the rules with `options`
   left out, so the item files do not change in 9a1.
5. Add `ITEM::SpecialSource` and `GetItemOptionValue` (sections 4, 7). In
   `SetItemAttributes`, replace the luck block (`ZzzInfomation.cpp:523-551`)
   by reading the item's `luck` definition (section 7); the effect row
   writes no `SpecialValue` (P8).
6. In the tooltip loop, take the luck lines from the data (section 9) and
   remove the `AT_LUCK` special case (`ZzzInventory.cpp:2148-2151`); remove
   the two `.resx` texts if nothing else uses them.
7. Add the effect table (section 4) with its `luck` row, the Data Checks
   area and the checks of section 10.
8. Prove it (section 13): snapshot unchanged, the `Special[]` test, the
   data test. Document the new files in `docs/item-data.md`.

What each later 9a pull request removes from the code:

| PR | Removes | Keeps until |
|---|---|---|
| 9a2 | In `SetItemAttributes`, the additional option branches for groups 0-4, 5, 6, 7-11 and `ITEM_RING_OF_ICE` to the end of group 13 (`ZzzInfomation.cpp:572-616`); the magic sword special case in the tooltip loop (`ZzzInventory.cpp:2166-2171`) | the Horn of Dinorant (`:557-570`, 9b5), the wing and cape chain (`:620-826`, 9b1) |
| 9a3 | In `CalcExcellentOptions`, the defensive and offensive blocks (`:375-450`) | the Wizard's rings (`:451-483`, 9b5) |
| 9a4 | In `CSItemOption::GetDefaultOptionText` (`CSItemOption.cpp:567-590`), the switch on `attackType`: the value and text come from the bonus definition | the second line `IncreaseAttributeDamage` for rings and pendants (`RenderDefaultOptionText`, `:641-658`, depends on the item, not the bonus; phase 10 decides), and `GetDefaultOptionValue`'s result `SET_OPTION_*` for its callers (`getAllAddState`) |

### 15. The 9a data

**`Options/Luck.json` (9a1)**: `luck` (type `luck`; server: random, chance
0.25, 1 per item), one option `criticalDamageChance` `"percent": {"values": 5}`,
texts `luckJewelOfSoul`, `luckCriticalDamage`.

**`Options/Option.json` (9a2)**, all of type `option`, `number` 0, server:
random, chance 0.25, 1 per item (OpenMU's values). The text names are new;
the column after them is the `I18N::Game` text they are made from.

| Name | Attribute | Value | Text name | From | Items (rules, section 6) | OpenMU |
|---|---|---|---|---|---|---|
| `physicalDamageOption` | `physicalBaseDmg` | `"add": {"perLevel": 4}` | `additionalDamage` | `AdditionalDmgD` | groups 0-4 without `ammunition` | "Physical Base Damage (min and max) Option", 0x03 |
| `physicalAndWizardryDamageOption` | `baseDamageBonus` | `"add": {"perLevel": 4}` | `additionalDamage`, `additionalWizardryDamage` | `AdditionalDmgD`, `AdditionalWizardryDmgD` (`ZzzInventory.cpp:2166-2171`) | `magicSword` | "Base Damage Bonus (physical and wizardry …) Option", 0x07 |
| `wizardryDamageOption` | `wizardryBaseDmg` | `"add": {"perLevel": 4}` | `additionalWizardryDamage` | `AdditionalWizardryDmgD` | group 5 without `summonerBook` | 0x04 |
| `curseDamageOption` | `curseBaseDmg` | `"add": {"perLevel": 4}` | `additionalCurseDamage` | `AdditionalCurseSpellD` | group 5 with `summonerBook` | 0x05 |
| `defenseRateOption` | `defenseRatePvm` | `"add": {"perLevel": 5}` | `additionalDefenseRate` | `AdditionalDefenseRateD` | group 6 | 0x06 |
| `defenseOption` | `defenseBase` | `"add": {"perLevel": 4}` | `additionalDefense` | `AdditionalDefenseD` | groups 7-11 | 0x02 |
| `jewelryHealthRecovery` | `healthRecoveryMultiplier` | `"percent": {"perLevel": 1}` | `automaticHpRecovery` | `AutomaticHPRecoveryD` | rings 13/8, 13/9, 13/21-23, pendants 13/12, 13/13, 13/25-27 | "JeweleryHealth", 0x10 |
| `ringOfMagicMana` | `maximumMana` | `"percent": {"perLevel": 1}` | `maxManaIncrease` | `MaxManaIncreasedByD` | 13/24 | per item, GUID from the item id |
| `pendantOfAbilityAg` | `maximumAbility` | `"percent": {"perLevel": 1}` | `maxAgIncrease` | `MaxAGIncreasedByD` | 13/28 | per item |

`AutomaticHPRecoveryD` is not shown for `ITEM_LOCHS_FEATHER` to
`ITEM_INVISIBILITY_CLOAK` (13/14 to 13/18) today (`ZzzInventory.cpp`, case
`AT_LIFE_REGENERATION`); they get no link (section 6), so that exception
goes.

**`Options/Excellent.json` (9a3)**, type `excellent`, server: random, chance
0.001, 2 per item. Bit = `number - 1`, as OpenMU and as today's code. Rules
in section 6.

| Name | # | Attribute | Value | Text name | From |
|---|---|---|---|---|---|
| `excellentDefense` | 1 | `moneyAmountRate` | `"percent": {"values": 30}` (OpenMU 40, P14, O21) | `zenAfterKill` | `IncreasesAcquisitionRateOfZenAfterHuntingMonsters30` |
| | 2 | `defenseRatePvm` | `"percent": {"values": 10}` | `defenseSuccessRate` | `DefenseSuccessRate10` |
| | 3 | `damageReflection` | `"percent": {"values": 5}` | `reflectDamage` | `ReflectDamage5` |
| | 4 | `armorDamageDecrease` | `"percent": {"values": 4}` | `damageDecrease` | `DamageDecrease4` |
| | 5 | `maximumMana` | `"percent": {"values": 4}` | `increaseMaxMana` | `IncreaseMaxMana4` |
| | 6 | `maximumHealth` | `"percent": {"values": 4}` | `increaseMaxHp` | `IncreaseMaxHP4` |
| `excellentPhysical` / `excellentWizardry` | 1 | `manaAfterMonsterKillMultiplier` | `"add": {"values": 8}` (mana / 8) | `manaAfterKill` | `IncreasesAcquisitionRateOfManaAfterHuntingMonstersMana8` |
| | 2 | `healthAfterMonsterKillMultiplier` | `"add": {"values": 8}` | `lifeAfterKill` | `IncreasesAcquisitionRateOfLifeAfterHuntingMonstersLife8` |
| | 3 | `attackSpeedAny` | `"add": {"values": 7}` | `increaseAttackSpeed` | `IncreaseAttackingWizardrySpeedD` |
| | 4 | `physicalBaseDmgIncrease` / `wizardryBaseDmgIncrease` | `"percent": {"values": 2}` | `increaseDamagePercent` / `increaseWizardryPercent` | `IncreaseDamageD` / `IncreaseWizardryDmgD` |
| | 5 | `physicalBaseDmg` / `wizardryBaseDmg` | `"add": {"values": 0, "per": [{"attribute": "level", "divisor": 20}]}` (`level` or `totalLevel`, O22) | `increaseDamageLevel20` / `increaseWizardryLevel20` | `IncreaseDamageLevel20` / `IncreaseWizardryDmgLevel20` |
| | 6 | `excellentDamageChance` | `"percent": {"values": 10}` | `excellentDamageRate` | `ExcellentDamageRate10` |

OpenMU writes the "after kill" options as the factor 1/8; the client file
writes the divisor 8 as `add` and the exchange converts (`8` ↔ `1/8` for
these two attributes, section 12).

**`Options/Ancient.json` (9a4)**, type `ancientBonus`, no `server` values
(OpenMU links them through the sets, not randomly), no `number`:

| Name | Attribute | Value | Text name | From | `attackType` |
|---|---|---|---|---|---|
| `ancientBonusStrength` | `totalStrength` | `"add": {"perLevel": 5}` | `increaseStrength` | `IncreaseStrengthD` | 1 |
| `ancientBonusAgility` | `totalAgility` | `"add": {"perLevel": 5}` | `increaseAgility` | `IncreaseAgilityD` | 2 |
| `ancientBonusEnergy` | `totalEnergy` | `"add": {"perLevel": 5}` | `increaseEnergy` | `IncreaseEnergyD` | 3 |
| `ancientBonusVitality` | `totalVitality` | `"add": {"perLevel": 5}` | `increaseVitality` | `IncreaseStaminaD` | 4 |

Until phase 10 a fixed table in code maps the item's `attackType` to these
names (the last column; `CSItemOption.cpp:572-590`), and items get no
`options` link for them. 9a4 adds no `ancientOption` definitions: the type
is known to the loader so phase 10 can use it, and the set options stay
where they are until then.

### 16. The 9b groups in short

| PR | Source today | Definitions | Notes |
|---|---|---|---|
| 9b1 | `CalcWingOptions` (`ZzzInfomation.cpp:291-373`), the wing chain (`:620-826`) | 18 per-wing `option` definitions (O10, `Wings.cs:74-107` for the numbers); `secondTierWing` (5 options, P5), `secondTierCape` (Cape of Fighter: 3), `capeOfLord` (HP, MP, ignore defense, command `{"values": 10, "perLevel": 5}`), `thirdTierWing` (4); HP / MP / command with `levelType: itemLevel` | Wing types read the excellent byte (O9) |
| 9b2 | `JewelOfHarmonyOption_<lang>.bmd` (`UIJewelHarmony.cpp:15-88`) | `harmonyWeapon`, `harmonyStaff` (also books, P7), `harmonyDefense`; values `{"firstLevel": <Minlevel>, "values": [...]}` with `requiresItemLevel`; Zen costs per level stay with the definition as `"zen": [...]` (client-only); names into `OptionTexts.json`; OpenMU weights under `server` | `GetStrengthenCapability` reads the definitions; P11 is reported, not fixed |
| 9b3 | `ItemAddOption.bmd`, its 380 records (`ItemAddOptioninfo.cpp`) | one `guardian` definition per item kind (weapon, helm, armor, pants, gloves, boots), values as the bmd (P6) | needs #966; the buff records stay in the bmd (O13) |
| 9b4 | `socketitem_<lang>.bmd` (`SocketSystem.cpp:605`) | one `socket` definition per element, values per sphere level; `socketBonus` definitions; the set bonus as `"combinations"` in `Socket.json` (OpenMU's `ItemOptionCombinationBonus`); value kinds 3 to 5 as `per` terms (O7) | `IsSocketItem`'s list becomes the links; P13 |
| 9b5 | Fenrir (`ZzzInventory.cpp:2178-2245`), Dark Horse (`GIPetManager.cpp:580-605`), Dinorant (`ZzzInfomation.cpp:557-574`), Wizard's rings (`:453-483`) | the Fenrir types, `darkHorse`, `hornOfDinorant` (`bits`), `wizardsRing` (`always`) | section 3 |
| 9b6 | the option loop of `BuildItemTooltipText` | – | section 9 |

`zen` on harmony options and `combinations` in `Socket.json` are named here
so 9b2 and 9b4 do not have to invent them; their pull requests fix the
details in `docs/item-data.md`.

## Pull requests

From O20. Each move is compared with the old code before the old code goes:
the tooltip snapshot (with variants added for each group) and the stat tests
stay the same.

| PR | Contents | Waits for |
|---|---|---|
| 0 | This design, for review | – |
| 9a1 | The file format and loader: `Options/*.json`, `Tooltips/OptionTexts.json`, `Options/OptionRules.json`, the items' `options` links (bmd import, Item Add, kept matching, O15), the attribute names and their client effects (O6), Data Checks and the data test. First group: luck | 0 |
| fix | Luck adds its critical damage rate of 5 % in the client (P8, O12) | 9a1 |
| 9a2 | The additional option of the items that are no wings (weapons, shields, armour, the jewelry's regeneration, the Ring of Magic, the Pendant of Ability) | 9a1 |
| 9a3 | Excellent options, defensive and offensive; the excellent byte read by the linked type (O9) | 9a1 |
| 9a4 | The ancient bonus definitions (the type `ancientOption` known, no definitions; phase 10 adds them); `attackType` picks the bonus until phase 10 (O14) | 9a1 |
| 9a5 | Item Options (O18), the links in Item Basics, Item Tooltip's controls for the groups so far | 9a1 to 9a4 |
| 9a6 | The option exchange file and the item field `options` (O19); an OpenMU issue for P5 to P7 | 9a5 |
| C | OpenMU: the endpoint and the import page (phase C, OpenMU repository) | 9a6 |
| 10 | Item sets (a design of its own) | 9a4 |
| 9b1 | Wings: the additional options per wing (O10), the second and third tier wing options | 9a3 |
| 9b2 | Jewel of Harmony (`JewelOfHarmonyOption_<lang>.bmd`) | 9a1 |
| 9b3 | Guardian (380), the 380 part of `ItemAddOption.bmd` (O13) | 9a1, sven-n/MuMain#966 |
| 9b4 | Sockets (`socketitem_<lang>.bmd`: seed options, bonus options, set bonus; O13) | 9a1 |
| 9b5 | Pets and specials: Horn of Fenrir, Dark Horse, Horn of Dinorant, the Wizard's rings | 9a3 |
| 9b6 | The tooltip's option lines as layout lines per type (O17) | 9b1 to 9b5 |
| 9b7 | What stays true into `docs/item-data.md`; the items design's D13 and phase 11 changed (O13); this document deleted | 9b6 |

## Open questions

Answered on 2026-11-04: Q-O1 (O5), Q-O2 (O6), Q-O3 (O7), Q-O4 (O8), Q-O5
(O9), Q-O6 (O10), Q-O7 (O11), Q-O8 (O12), Q-O9 (O13), Q-O10 (O14), Q-O11
(O15), Q-O12 (O16), Q-O13 (O17), Q-O14 (O18), Q-O15 (O19), Q-O16 (O20),
Q-O17 (O21) and Q-O18 (O22). All questions are answered. New questions from
the pull requests are added here.

## Tasks

1. [x] Answer the open questions one by one (O5 to O20).
2. [x] Write the target design from O1 to O20.
3. [ ] The pull requests of "Pull requests".
4. [ ] Move what stays true into `docs/item-data.md`, and point the items
   design (phase 9) and the roadmap (row "Items") at those docs.
5. [ ] Delete this document.
