# Item Option Definitions (Items Phase 9) Design (WIP)

> **Draft, 2026-11-04.** Items phase 9 of the
> [items design](2026-09-25-data-driven-items-design.md) and its decision
> D14. Written from the code of MuMain `upstream/main` @ `88cf9d208` and
> OpenMU `master` @ `13a600f3a`. Line numbers are those of these commits.
> sven-n/MuMain#966 (open) makes the client read the guardian (380) flag
> that the server sends; where this design depends on it, it says so.
>
> **Reviewed 2026-11-06.** The design was checked against both code bases
> and Season 6 sources; the review's questions R1 to R9 changed or added
> decisions O23 to O31 and problems P17 to P31, and the decisions they
> change say so. The Season 6 sources are listed under "Season 6 sources".
>
> **To implement:** read Goal, Decisions and Target design; section 14 of
> the target design says where to start, "Pull requests" gives the order.
>
> This document is temporary. Its last task deletes it, after what stays true
> has moved into `docs/item-data.md` and the code (see "Tasks").
>
> **Changed 2026-11-06.** sven-n/MuMain#1007 made the Dark Horse's defense count
> the hero's total agility, as OpenMU's `TotalAgility`; P30, section 4, section
> 12 and 9b5 say so.
>
> **9a1 as built (2026-11-07, sven-n/MuMain#1013).** The format reads what luck
> uses: the type `luck`, and a value's `values`. Each group's pull request adds
> its type and the fields it uses (`per` with 9a3; `firstLevel`, `levelType`,
> `requiresItemLevel`, an option's `type`, `bits`, `itemLevels`,
> `subOptionType` and `server.weight` with 9b). Save Items writes the option
> files from 9a5 on, when Item Options changes them; 9a1 has their writers and
> a test that the shipped files are written back as they are read. An item
> file saved before 9a1 keeps its items without lists in the editor too, since
> an empty list differs from the rules' (O33): Item Basics' "Use the rules"
> (9a5) writes them.

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

Made on 2026-11-04 while planning phases 9 and 10. Struck-through text was
replaced by the review: the **Changed** note at the end of that decision is
the rule that holds.

| D | Topic | Decision |
|---|---|---|
| O1 | Order | Phase 9a, then phase 10 (item sets), then phase 9b. In OpenMU an item set's options are an `ItemOptionDefinition` (type `AncientOption`) and the per-item ancient bonus is one too (`ItemOfItemSet.BonusOption`), so phase 10 builds on the format of 9a instead of a format of its own that 9a would replace. |
| O2 | Starting point | Written from scratch. No branch or pull request started phase 9 (sven-n/MuMain and Mosch0512/MuMain searched on 2026-11-04). |
| O3 | Guardian flag | sven-n/MuMain#966 is merged before the guardian options move, so that the 380 lines the move compares are the ones a server item shows. |
| O4 | This document | Deleted by its last task, after its lasting content has moved into the docs. |
| O5 | Definition names (Q-O1, 2026-11-04) | A definition has a name of our own in camelCase (`luck`, ~~`excellentPhysical`~~, `harmonyDefense`, `wingsOfSatan`), unique across all option files. Item files link to it by that name, and the exchange file carries it. OpenMU's import finds the definition by its fixed GUID where the Season 6 data gives one (`ItemOptionDefinitionNumbers`), else by its ~~type and~~ English name, because harmony and guardian definitions get random GUIDs (P4). The display name is a translated text of its own (D17). **Changed (2026-11-06, O26, O27):** the excellent and harmony names follow OpenMU's English names (`excellentPhysicalAttack`, `excellentWizardryAttack`, `excellentDefense`, `harmonyPhysicalAttack`, `harmonyWizardryAttack`, `harmonyDefense`), and the exchange matches through OpenMU's labels noted on each definition, not through a type (OpenMU's definitions have none, P23). |
| O6 | Option values (Q-O2, 2026-11-04) | An option's value names the attribute it raises with OpenMU's attribute names (`Stats`, in camelCase: `maximumHealth`, `physicalBaseDmg`) and gives the value in the units the tooltip shows: `{"attribute": "maximumHealth", "percent": {"values": 4}}`, ~~`{"attribute": "defenseBase", "add": {"perLevel": 4}}`~~, and `addFinal` for OpenMU's `AddFinal` (the value is always an object, section 4). The loader maps each attribute to the client's effect (`AT_*`); an attribute the client does not compute loads with a warning, and its line is shown but not added to the stats. The exchange converts the units (4 % to the factor 1.04 and back). **Changed (2026-11-06, O25):** a value with levels is a list, `{"attribute": "defenseBase", "add": {"values": [4, 8, 12, 16]}}`. |
| O7 | Values that depend on the hero or the item (Q-O3, 2026-11-04) | A value is a constant, a value per level (O11), or either plus `per` terms: `"per": [{"attribute": "totalLevel", "divisor": 20}]`, `{"attribute": "horseLevel", "times": 2}`. A term multiplies an attribute of the hero, the pet or the item, as OpenMU's related values with `Multiply`; it covers excellent level / 20, Gold Fenrir, the Dark Horse and the socket value kinds 3 to 5. The value is computed when the tooltip is drawn and when the stats are summed, not when the item is made, so level / 20 follows the hero (P12, a behavior change the pull request names). An OpenMU option with another operator is a known difference of the exchange |
| O8 | Option texts (Q-O4, 2026-11-04) | The option lines' texts live in a file of their own, `Data/Items/Tooltips/OptionTexts.json` (`formatVersion` and `texts`, the shape of `Tooltip.json`'s `texts`: a name, then the text by language with `{value}`). An option names its text (`"text": "increaseMaxHp"`); names are looked up only in this file, so they cannot clash with `Tooltip.json`, which stays as it is. A line that several options show (the additional damage of the physical option and of the wings) is one text. The `.resx` texts only options use move into it with their translations, and the values written into them (`Increase Max HP +4%%`) become `{value}`. The harmony and socket option names (bmd today) join it in 9b. About 100 texts; in `Tooltip.json` (99 texts, 1360 lines, 95 % texts) they would have doubled the file |
| O9 | The excellent byte (Q-O5, 2026-11-04) | What the excellent byte means follows from the type of the definitions the item links to, as OpenMU reads it (`ItemSerializerHelper.cs`): a definition of type `excellent` makes it the excellent bits, `wing` the wing option bits (bit = number - 1 for both), the Fenrir types the colour (the value picks the option of that number: 1 black, 2 blue, 4 gold). An item that links two types that use the byte is a load error. "Excellent" (name prefix and colour, durability, requirements, the tooltip state `excellent`) then means "has an active option of type `excellent`", not "the byte is above 0", so the wing exceptions in that code go. This applies only where it gives the same result for every item the server can send; a test where it would not stays and becomes a fix of its own (section 8 lists every place, P15). The pull request shows with the tooltip snapshot and the stat tests that nothing changes. **Changed (2026-11-06, P19):** the price is not in this list; it keeps reading `Special[]` (section 8), which counts the wings' and Wizard's rings' options as today. The mixes' excellent flag, the MU Helper's excellent pick-up, the Horn of Fenrir's durability and the ground label rule would change for wings or Fenrir and stay as they are (section 8); from 9b5 on the durability test reads "excellent, or an active Fenrir option". The Horn of Fenrir's definition is of type `fenrir` (O28). The reading is as OpenMU writes the byte; OpenMU's extended reader decides by `IsWing()` instead and never reads the Fenrir byte, which gives the same for Season 6 data |
| O10 | Wing additional options (Q-O6, 2026-11-04) | One definition per wing, as OpenMU (`Wings.cs:74-107`): 18 definitions (4 first tier, 5 second tier, 2 capes, 7 third tier) named after the wing (`wingsOfDragon`, `wingOfStorm`), of type `option` like OpenMU's (type `wing` is only the second and third tier options of O9). An option's number is the option type the server sends (0, 2 or 3): Wings of Dragon 0 = recovery +1 % per level, 2 = damage +4 per level; Wing of Storm 0 = recovery, 3 = damage, 2 = defense. This replaces the chain of `if`s in `SetItemAttributes` (`ZzzInfomation.cpp:620-826`), the item files need no field beyond their links, and the exchange is 1:1. Values repeat across the definitions; the texts do not (O8). Shared definitions with a type table per wing were weighed and dropped: one wing with values of its own would need a copy and a table edit, and the import would have to merge and split definitions. **Changed (2026-11-06, O26):** the second tier wings' options of type `wing` are OpenMU's three, in one definition `secondTierWing` that the second tier wings and the Cape of Fighter share, as OpenMU's "2nd Wing Options"; there is no `secondTierCape` |
| O11 | Values per level (Q-O7, 2026-11-04) | The form of `ItemStats.json`'s level bonus tables: `values` is a number or a list~~, and `perLevel` adds that much for every level (`{"perLevel": 4}` for the additional option, `{"values": 50, "perLevel": 5}` for the second tier wings' max HP)~~. A list starts at level 1, or at `firstLevel` ~~(harmony: 0)~~. The level is the option's own level (option, harmony, sphere or ancient bonus level) unless `"levelType": "itemLevel"` says it is the item's + level (OpenMU's `LevelType.ItemLevel`). `"requiresItemLevel": true` is the harmony rule: an option level is active only while the item level is at least that level (OpenMU's `RequiredItemLevel` on every level). ~~A straight line has no upper end, as today's `level * 4`, so any level the server sends keeps its value; tables only were weighed and dropped for that reason.~~ Rules per level range ("from 6, start 10, +2") are not stored: Item Options gets a **Fill levels** action that writes them into a list. ~~The exchange expands a straight line into OpenMU's one value per level, and its import writes an evenly spaced list back as `perLevel`.~~ **Changed (2026-11-06, O25):** values with levels are lists only; `perLevel` and the open-ended line are gone, and harmony's lists start at each option's own first level (0, 3, 6, 9 or 13), not at 0 |
| O12 | Differences from OpenMU (Q-O8, 2026-11-04) | Checked against Season 6 sources on the web, one by one. **P5:** ~~the data keeps the client's five second tier wing options;~~ Max AG +50 is in the [Murred FAQ](https://murred.com/FAQ-2-wings.html) ("not applicable to Cape of Lord") and the [InfinityMU wiki](https://wiki.infinitymu.net/index.php?title=Item_Options), attack speed +5 only in the latter. **P6:** the data keeps 200 for the pants' defensive skill and 700 for the boots' max SD, as in every guide found ([guiamuonline](https://www.guiamuonline.com/component/content/article/301-todo-sobre-las-opciones-38?showall=1), [InfinityMU forum](https://forum.infinitymu.net/threads/380-level-item-options.10/)). **P7:** summoner books keep the staff harmony options, as [MuCustoms' Season 6 guide](https://mucustoms.com/harmony-system) lists them ("Staff, Stick, Book Summoner"). **P8:** luck is critical damage rate +5 % in the data, as every source and OpenMU have it; until a client fix right after 9a1 its effect row writes no `SpecialValue` (section 4), so the move changes nothing and the fix is a pull request of its own. ~~P5 to P7 (and P14, P16: O21, O22) are known differences of the option exchange (Q-O15), and an OpenMU issue names them with these sources.~~ **Changed (2026-11-06, O26):** the review's Season 6 research decided these again: the second tier wings ship OpenMU's three options (P5), ~~the pants and boots keep 200 and 700 with OpenMU's pants moving to 200~~ the boots keep 700 and the pants are 100 on both sides (P6, O32), P7 is no difference (OpenMU's curse harmony is commented out, books use the wizardry set on both sides), and the OpenMU issue lists only OpenMU's own errors (O26) |
| O13 | `ItemAddOption.bmd` and `socketitem_<lang>.bmd` (Q-O9, 2026-11-04) | Split by meaning. 9b moves the 380 part of `ItemAddOption.bmd` (guardian definitions, OpenMU's `GuardianOption`) and the whole of `socketitem_<lang>.bmd`: the seed options (`SocketOption`), the bonus options (`SocketBonusOption`) and the set bonus, which becomes a combination bonus next to the socket definitions (OpenMU's `ItemOptionCombinationBonus`). Phase 11 keeps only the buff values of `ItemAddOption.bmd` (scrolls, elixirs, seals, charms, potions; `{addOptionValue1}` and `{addOptionDuration}`), which are no options in OpenMU, and `SocketItem` leaves phase 11. The items design's D13 and phase 11 are changed to say so when this design moves into the docs |
| O14 | Ancient bonus (Q-O10, 2026-11-04) | Per item of a set, as OpenMU (`ItemOfItemSet.BonusOption`). 9a4 adds four definitions of type `ancientBonus` (`ancientBonusStrength`, `…Agility`, `…Energy`, `…Vitality`; ~~`{"perLevel": 5}`~~, so level 1 = +5, level 2 = +10). Until phase 10 the item's `attackType` (1 to 4) picks the definition, so 9a4 changes nothing. Phase 10 links a bonus definition to each item of a set and removes `attackType`, whose only reader is this code. Of OpenMU's 115 set items, 113 have the stat of the client's `attackType`; the Pendant of Ability (13,28) has none on both sides, and the Ring of Fire (13,21) has energy in one set and vitality in the other on the server, energy in the client: phase 10 follows the server there and its pull request names it. **Changed (2026-11-06, O25):** the bonus is the list `{"values": [5, 10]}` |
| O15 | Which items have which groups (Q-O11, 2026-11-04) | As `statProfile` and `profileRules` (phase 3c): each item writes the definitions it can have (`"options": ["luck", "physicalDamageOption", "excellentPhysicalAttack"]`), and the game reads only that list; the type ranges in `SetItemAttributes`, `CalcExcellentOptions`, `GetItemType` and `IsSocketItem` go. Rules in data (`Options/OptionRules.json`, the shape of `profileRules`: a list of definitions for the items whose `group`, `slot`, `weaponKind`, `wingTier` or `tags` match) give the list an item's data gives it; the bmd import, Item Add and the item editor keep an item's list matching them, and a list written in the file that differs (the Bone Blade with `guardianWeapon`, a wing with its own definition) stays as long as it differs. The first rules are today's code ranges, so a converted item gets what it has today. **Changed (2026-11-06, O24):** confirmed as written here, against section 6 of the first draft: every item that has options carries its list in its file, the game build reads only the lists, and the rules are read by the editor only |
| O16 | Server values (Q-O12, 2026-11-04) | The client files keep OpenMU's server-only values, as D14 says ("values, chances, levels"), grouped under `"server"` so it is clear the game does not read them: on a definition `{"addsRandomly": true, "addChance": 0.001, "maximumOptionsPerItem": 2}` (`excellentPhysicalAttack`), on a harmony option `{"weight": 40}`. Item Options shows them in a Server box and a weight column, the exchange carries them both ways, and Update from server shows a change made on the server (Luck's chance 0.25 to 0.30) in its diff. A new option group made in MuEditor so reaches OpenMU complete. This differs on purpose from the item exchange, which leaves server-only item fields out (phase 7): an option group without its chances is not a whole definition |
| O17 | Option lines in the tooltip (Q-O13, 2026-11-04) | Layout lines, one per option type, as D72 planned: `{"options": "guardian", "color": "redPurple", "bold": true}` expands into the item's options of that type, each with its text from `OptionTexts.json` (O8). Order, colours, bold and blank rows use what layout lines already have (`when`, `colors`, empty lines with states such as `hasGuardianOptions`, `hasHarmonyOptions` and `harmonyActive`). The `default` layout gets today's order at the `options` marker: guardian, harmony, ancient bonus, wing, the skill lines (no option, they stay code: `{"code": "skillLines"}`), luck, additional option, excellent; the Horn of Fenrir before `afterOptions` and the sockets after the expiry lines, where the code writes them today. The tooltip snapshot shows the output is the same, and its variants grow with each group (380, sockets, wing option types, ancient bonus levels, Fenrir and Dinorant). Layout variants can hide or move a type, and the tooltip editor (#815 step 6) edits these lines like the others. **Changed (2026-11-06, O29):** `fixed` options follow excellent, where the Wizard's rings are today (section 9) |
| O18 | The editor (Q-O14, 2026-11-04) | One window, **Item Options**, in the Items group (Windows > Items), built like Item Formulas: the definitions as a list with search, a type filter and how many items use each; the picked definition's name (D17), type, Server box (O16) and options with attribute, value and text, edited on a copy and taken with Apply after the save checks; value editors that fit the type (a level grid for harmony, one row per element for sockets) and **Fill levels** (O11); its users (D26: a used definition cannot be removed, a rename updates the links, Copy makes a variant). Tabs **Rules** (`OptionRules.json`, O15) and **Texts** (`OptionTexts.json` in the names grid `NamesGrid`, O8). Item Basics gets the item's option links with a picker, marking a list that differs from the rules. Item Tooltip's controls grow with each group (option level and type, harmony option and level, 380, sockets, ancient bonus level). Every change is one step of the undo history; Save Items writes the files |
| O19 | The exchange (Q-O15, 2026-11-04) | A file of its own for the definitions (D11, D14): `formatVersion`, `source`, `gameVersion` and `definitions` in the shape of the client files (O5 to O7, O11, O16), from MuEditor's Export, Import and Update from server and OpenMU's `GET api/items/options/exchange` (phase C). The item exchange gets the field `options` (the item's links, O15), written by an OpenMU `ItemExchangeField`. Client-only and never exchanged: the `text` names, `OptionTexts.json` and `OptionRules.json`. Every import shows a diff per definition and field, as phase 7; `Sync/KnownDifferences.json` gets an `options` list ~~(P5 to P7, P14, P16; O12, O21, O22)~~. Links to a definition the client does not have are refused, so options are imported before items. Phase 10's item sets follow the same pattern. **Changed (2026-11-06, O27 to O30):** definitions are matched through OpenMU's labels (O27); the Horn of Fenrir is one definition as on the server (O28); OpenMU's definitions whose options have no type are the client's type `fixed` (O29); no known differences are shipped (O30); OpenMU's import updates options in place (P24) |
| O20 | Pull requests (Q-O16, 2026-11-04) | The finer split of "Pull requests": one concern per pull request, 9a in six (format and luck, additional option, excellent, ancient bonus, editor, exchange) plus the luck fix, then phase 10, then 9b in seven (wings, harmony, guardian, sockets, pets and specials, the tooltip lines, the docs). Each move proves itself with the tooltip snapshot, its variants added per group, and the stat tests. **Changed (2026-11-06):** four fixes of their own join them: the option slots before 9a1 (O23), excellent level / 20 after 9a3 (O26), the stale bold before 9a4 (O31), and the Wizard's Ring right after 9b5 (O26) |
| O21 | The Zen option (Q-O17, 2026-11-04) | `excellentDefense` option 1 is `moneyAmountRate` +30 %, as the client's text and the Season 6 guides (P14); OpenMU's 40 is a known difference of the option exchange and named in the OpenMU issue. Confirmed by the review: the decompiled Season 6 server returns 30 ("season4 changed"), an older one 40, so 40 is the value from before Season 4 (review, 2026-11-06). |
| O22 | Excellent level / 20 (Q-O18, 2026-11-04) | Configurable through the data: the option's `per` term names the attribute it divides, `level` (the hero's level) or `totalLevel` (level plus master level, OpenMU's `TotalLevel`), and Item Options lets the user pick it. The shipped data writes `level`, the client's behaviour today (P16), so nothing changes; OpenMU's `TotalLevel` is a known difference of the option exchange, and an import from a server that keeps `TotalLevel` shows it in the diff like any other value. **Changed (2026-11-06, O26):** the move ships `level` (nothing changes), and a fix right after 9a3 switches the data to `totalLevel`, rounded down, as the Season 6 servers compute it; the character window then shows +30 instead of +20 for a hero of level 400 with 200 master levels. |
| O23 | Option slots (R1, 2026-11-05) | An item keeps up to 16 option slots: a fix of its own before 9a1 raises `MAX_ITEM_SPECIAL` (8, `_define.h:342`) to 16 and writes every slot through one checked helper (P17), with a test of a weapon with skill, luck, an additional option and all six excellent options. A slot past the 16th is dropped and logged once per item type. It is no part of #966, which is about the guardian flag. |
| O24 | Option lists in the item files (R2, 2026-11-05) | O15 as written: every item that has options carries its `options` list (about 515 items), the game build reads only these lists, and `OptionRules.json` is read only by the editor, which keeps the lists matching as `WithDerivedFields` does for `statProfile` (P18). An item without a list, or with an empty one, has no options and is never filled in at game load. Item Basics shows an item whose list differs from its rules; that is no data issue. 9a1 writes the lists into the shipped item files once with the editor's rules (its data commit), so they change by one line per item; 9a2 and 9a3 add their definitions the same way (the jewellery gets its first line in 9a2). A data test checks that every shipped item's list is what its rules give, apart from the exceptions the test names. |
| O25 | Values with levels (R3, 2026-11-05) | A value with levels is always a list, one entry per level; its length is the last level on both sides (P20). In the game a level above the last shows the last entry; the export writes exactly the entries as OpenMU's levels, and the import reads them back as a list. Custom values per level work on both sides (OpenMU stores one value per level, `ItemPowerUpFactory.cs:286-302`). The shipped lists end where Season 6 does: the additional option at +16 (shields +20), the jewellery's recovery at +3 %, item-level options at +15 (16 entries from level 0), sockets at sphere level 5, the ancient bonus at level 2, harmony at level 13. **Fill levels** writes longer or custom lists. A constant (no levels) stays a number. |
| O26 | Season 6 values (R4, 2026-11-06) | Decided one by one after two rounds of Season 6 research (sources under "Season 6 sources"). **Second tier wings:** OpenMU's three options, shared with the Cape of Fighter; max AG +50 and attack speed +5 were never applied (Webzen's own client marks them "code that is not applied") and can be added in Item Options. **Ammunition:** no options (Season 3 to 6 item files mark arrows and bolts "no options"). **Wizard's Ring (13/20):** its bonus at +0 only, a client fix right after 9b5 (P25). **Lethal Wizard's Ring:** kept, +15 % / +15 % / speed 10. **380 pants:** ~~200 on both sides, exchanged 1:1 and editable on both (Season 6 shows +200 and applies +100 PvP defense because its servers halve all defense; OpenMU moves from 200 / 2 to 200).~~ **380 boots +700, zen +30 % (O21), socket Lightning critical damage 30 / 32 / 35 / 40 / 50:** kept. **Excellent level / 20:** level plus master level, the fix after 9a3 (O22). **Books' excellent options:** stay on curse damage, as Webzen's client moved them in 2008 (P28); a known difference. **Names:** `excellentPhysicalAttack`, `excellentWizardryAttack`, `excellentDefense` and the same for harmony. The OpenMU issue lists OpenMU's own errors: zen, boots, ~~pants,~~ the Lightning row, the Wizard's Ring at +1 and +2 (OpenMU has no +3), the missing Lethal Wizard's Ring, the Fenrir reader (P21) and OpenMU's socket and guardian numbering (P31). **Changed (2026-11-06, O32):** the 380 pants are 100 on both sides, OpenMU's value, and leave the OpenMU issue. |
| O27 | Labels in the exchange (R5, 2026-11-06) | Definitions keep short names of our own; each notes OpenMU's labels, `"openMu": {"id": "<GUID>", "name": "<English name>"}` (no `id` where OpenMU's GUID is random: harmony, guardian). The exchange file carries them; both sides match a definition from the other side by its id, else by its English name (MuEditor looks them up in its definitions' `openMu` labels). OpenMU needs no new field (P23). OpenMU's code finds definitions by their English name, so Item Options warns before the English display name changes, and a renamed harmony or guardian definition (no fixed id) is shown unmatched in Item Sync for the user to pair. Webzen's data has no ids for most groups (excellent, luck, the additional option, wings and pets are decided in code), only for harmony (weapon 1, staff 2, defense 3), sockets (table and element) and sets; options are numbered (bits, indices), which `number` holds. |
| O28 | The Horn of Fenrir (R6, 2026-11-06) | One definition `fenrir` of type `fenrir` (it reads the excellent byte), as OpenMU's "Fenrir Options": its options carry their own type (`blackFenrir` #1, `blueFenrir` #2, four `goldFenrir` #4), so a definition's options may differ in type, and options of one type that apply together may share a number, as guardian's (P21). The exchange is 1:1. |
| O29 | Options without a byte (R7, 2026-11-06) | A client type `fixed`: always on, read from no byte. It holds OpenMU's definitions whose options have no type, the Elite Skeleton ring (13/39) and the Skeleton ring (13/122), and the Wizard's rings (the first draft's `always`), in 9b5 (P22). The rings' effects move from description text into options identical to today (the client's "vitality + hero level" stays; the options take no `Special[]` slot, so the tooltip, price and name colour stay, section 7), and Item Sync then shows OpenMU's "max HP + level" as a difference. The Wizard's rings are base power-ups in OpenMU, so their definitions are left out of the exchange. |
| O30 | Known differences (R8, 2026-11-06) | No `Sync/KnownDifferences.json` is shipped: Item Sync shows the option differences, the user ticks **Known** or imports the client's values into the server, and `docs/item-data.md` lists the differences with their reasons. |
| O31 | Stale bold (R9, 2026-11-06) | A fix of its own before the first snapshot variants that show rows setting no bold (9a4's ancient bonus levels, 9b5's Horn of Fenrir; the option loop sets bold itself, so 9a2's and 9a3's variants don't need it): every tooltip starts with no bold rows and no colours, for all `MaxTextListRows` rows (98: the original 50, the description's 32 and the added lines' 16, `ZzzInventory.h:91-95`; P29), with a test that draws the same Horn of Fenrir tooltip after a bold one and alone. |
| O32 | 380 pants' PvP defense (R10, 2026-11-06) | 100 on both sides, exchanged 1:1, so OpenMU's data stays as it is and the pants leave the OpenMU issue. OpenMU halves all defense (`DefenseFinal` = 0.5 × `DefenseBase`, `CharacterClassInitialization.cs:105-107`) and adds the pants' `DefensePvp` after that (`200 / 2`, `GuardianOptions.cs:62`), so its 100 already gives Season 6's effect; O26's move to 200 would have doubled it. The client's `guardian` pants definition holds 100 where the bmd has 200, so the tooltip line (`DefensiveSkillD`, `ItemAddOptioninfo.cpp:85`) shows +100; no other client code reads that value (the character window reads only option types 1 and 3, `CharacterInfoWindow.cpp:739-757`). 9b3 names it as the one value it changes (P6). |
| O33 | Item files saved before 9a1 (R11, 2026-11-06) | No warning and no new `formatVersion`: O24 stays as written. The shipped item files get their lists in 9a1's data commit; an item file someone saved before 9a1 has no lists, so its items have no options in the client (OpenMU still applies its own) until the editor writes them. Unlike a missing `statProfile` (`ValidateStatProfileGiven`, `ItemDataValidation.cpp:266-279`), the game gives no warning, because it never reads the rules (O34). |
| O34 | Where the option rules build (R12, 2026-11-06) | In every build, as `StatProfileRule` and `DeriveStatProfile` (`ItemAttributeConversion.h:51`): `ItemRuleConditions`, `OptionRule` and the `OptionRules.json` reader. Only the editor loads the file (`CItemDataHandler::Load`, `#ifdef _EDITOR`) and writes lists from it (`WithDerivedFields`, P18); the game never reads it. The data test that checks every shipped item's list against its rules is outside `#ifdef _EDITOR` and reads the file itself, so upstream's pull request checks, which build with the editor off, run it. |

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
| Dark Horse | Code (`GIPetManager.cpp:580-605`): defense `5 + agility / 20 + pet level * 2` (the base agility; the total agility since sven-n/MuMain#1007) | One item | none | Blue line |
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
| P3 | Level 1 is stored several ways in OpenMU: the additional options keep it in the base value and levels 2-4 in `LevelDependentOptions`; wing options keep levels 1-4 there with a base of 0; the jewellery's recovery a base of 0 and levels 1-3; sockets a base equal to level 1 plus levels 1-5; the ancient bonus a base without a boost plus levels 1-2; item-level options the base as level 0 plus levels 1-15; harmony starts at each option's own first level (0, 3, 6, 9 or 13), which is also its `RequiredItemLevel`. OpenMU's failed Jewel of Life drops to the lowest stored level (`ItemUpgradeConsumeHandlerPlugIn.cs:123`), so the shape matters | `GameConfigurationInitializerBase.cs:112-151`, `InitializerBase.cs:180-200`, `HarmonyOptions.cs:139-143`, `SocketSystem.cs:479-494`, `AncientSets.cs:826-844`, `Wings.cs:385-396` | 9a (format) |
| P4 | Harmony and guardian definitions get a new random GUID on every OpenMU installation; only their options have fixed GUIDs | `HarmonyOptions.cs`, `GuardianOptions.cs` | 9a (exchange) |
| P5 | Second tier wings have five options in the client and three in OpenMU (no AG, no speed). Season 6 never applied the two: Webzen's own client marks them `// 적용되지 않는 코드.` ("code that is not applied", repository commit `ee2c7a35e`, `source/ZzzInfomation.cpp`), their values are the Horn of Dinorant's, and no Season 3 to 6 server rolls them (O26) | `CalcWingOptions`, `Wings.cs:342-400` | 9b |
| P6 | Guardian values differ: pants defense 200 in the client and 100 in OpenMU, boots SD 700 and 200. Season 6: the pants show +200 and the server applies +100 PvP defense, because Season 6 servers keep all defense at half the client's number; the boots are +700 in every data file from Season 3 to 18 (O26); the data holds the pants at 100, OpenMU's value (O32) | `ItemAddOption.bmd`, `GuardianOptions.cs:62`, `:106` | 9b |
| P7 | *(No difference.)* OpenMU's curse harmony group is commented out ("no curse attack harmony options until Season 16", `HarmonyOptions.cs:57-62`), and books link the wizardry set as the client's staff set; the Season 6 client's harmony file has no curse table either | `UIJewelHarmony.cpp:90-111`, `Weapons.cs:429-434` | – |
| P8 | Luck adds no critical damage rate in the client (value 0), OpenMU 5 % | `ZzzInfomation.cpp:2139-2150` | 9a |
| P9 | The ancient bonus stat comes from the item's `attackType` field; OpenMU puts it on the item's place in a set | `CSItemOption.cpp:546-594`, `AncientSets.cs:797-840` | 9a, 10 |
| P10 | `ItemAddOption.bmd` holds the 380 options and the buff values of consumables in one table | `ItemAddOptioninfo.cpp` | 9b, 11 |
| P11 | Most harmony effects (critical damage, SD, regeneration) are shown but never added to the client's stats | `UIJewelHarmony.cpp:145-273` | 9b (report) |
| P12 | Values computed when the item is made go stale: excellent level / 20 from the hero's level at that time | `CalcExcellentOptions` | 9a |
| P13 | The socket "no bonus" value arrives as 0xF, not 0xFF; it works because bonus 15 is empty | `ParseItemData` | 9b |
| P14 | The defensive excellent Zen option is +30 % in the client's text, +40 % in OpenMU (`MoneyAmountRate` 1.4, `ExcellentOptions.cs`); Season 6 guides give +30 % ([muonlinefanz](https://muonlinefanz.com/guide/items/excellent)); the decompiled Season 6 server returns 30 ("season4 changed"), an older one 40, so +40 % is the value from before Season 4 | `ZzzInventory.cpp` (`AT_IMPROVE_GAIN_GOLD`), `ExcellentOptions.cs:126` | 9a3 (O21) |
| P15 | `CalcDurabilityPercent` gives every item whose excellent byte is above 0 the excellent +15 durability, also second and third tier wings with wing options, while `CalcMaxDurability` leaves those wings out; the durability per cent of such a wing is computed against 15 more than its maximum | `ZzzInfomation.cpp:1785`, `ZzzInventory.cpp:1059-1060` | 9a3 keeps it, a fix of its own |
| P16 | Excellent "+level / 20": the client divides the hero's level (`CharacterAttribute->Level`), OpenMU `TotalLevel`, which is level plus master level (`Stats.cs:143`). Webzen's own client divides the level too; the decompiled Season 6 servers divide level plus master level, rounded down (O26) | `ZzzInfomation.cpp:417`, `:430`, `ExcellentOptions.cs` | 9a3 (O22) |
| P17 | An item has 8 option slots (`MAX_ITEM_SPECIAL`, `_define.h:342`), and nothing checks the bound. Skill, luck, an additional option and six excellent options are 9; the 9th is written to `Special[8]`, which lands on `SpecialValue[0..1]` because `ITEM` is `#pragma pack(1)` (`_struct.h:160`). The tooltip's 9th line and the price's 9th step work through this; `SpecialSource` and the luck fix's value would not | `SetItemAttributes` | before 9a1 (O23) |
| P18 | `statProfile` is derived only in the editor (`WithDerivedFields` is `#ifdef _EDITOR`, `ItemDataHandler.cpp:367`); at game load an empty one keeps the default with a warning. Deriving left-out option lists at game load would be new | `ItemDataHandler.cpp`, `ItemDataValidation.cpp:268-279` | 9a1 (O24) |
| P19 | Readers of the excellent byte that the first draft's list missed: `StandardItemNameColor` (`ZzzInventory.cpp:1619-1630`), the mixes' `RCP_SP_EXCELLENT` (`MixMgr.cpp:171`, used at `:602-603` and `:911`), `IsExcellentItem` (`ZzzInventory.cpp:3068`, the MU Helper's pick-up filter), the ground label rule `{"when": ["excellentFlags", "!upperTierWings"], "style": "excellent"}` (`GroundLabels.json:46`), `IsHighValueItem` (`ItemCategories.cpp:464-468`), `Trade.cpp:765`, the drop level in `ComputeItemStats` (`ItemStats.cpp:160`), the Horn of Fenrir in `CalcMaxDurability` (bytes 1, 2, 4 give it the excellent +15 today), the excellent glow (`ZzzObject.cpp:7848`), the trade window's item comparison (`ZzzInventory.cpp:2964-2965`), the Horn of Fenrir's run speed (`ZzzCharacter.cpp:6244`) and the character window's `CalcDurabilityPercent` calls (`CharacterInfoWindow.cpp:1368`, `:1465`) | section 8 | 9a3 |
| P20 | OpenMU's number of stored levels is game logic: the Jewel of Life raises only to a stored level (`ItemUpgradeConsumeHandlerPlugIn.cs:97`), drops pick from them (`DefaultDropGenerator.cs:412-418`), a missing level does nothing (`ItemPowerUpFactory.cs:288-295`). An open-ended value can't say how many to write | `ItemPowerUpFactory.cs` | 9a (format, O25) |
| P21 | OpenMU's Horn of Fenrir is one definition, "Fenrir Options", with Black #1, Blue #2 and four Gold options all numbered 4 (`Pets.cs:310-325`); its reader takes `PossibleOptions.First()` for any colour, so every colour becomes Black there (`ItemSerializerHelper.cs:465-468`) | `Pets.cs` | 9b5 (O28) |
| P22 | OpenMU links two items to definitions whose options have no type: the Elite Skeleton ring (13/39, max HP + level) and the Skeleton ring (13/122, +30 % experience with the pet skeleton) (`Jewelery.cs:168-172`, `:199-204`); the client shows their effects as description text, and for 13/39 says "vitality + hero level". The first draft refused an item import that links them. The client's English name for 13/39 is "Eilte Transfer Skeleton Ring" (OpenMU: "Elite Skeleton Transformation Ring") | `Group13_Helper.json`, `Jewelery.cs` | 9b5 (O29) |
| P23 | OpenMU's definitions have no type and no name for the client, only a GUID and a translated `Name`, and OpenMU's own code finds them by the English name (`Weapons.cs:403-404`, `:429-430`, `OptionExtensions.cs:103`, update plug-ins); not every fixed GUID comes from `ItemOptionDefinitionNumbers` (per-wing `SetGuid(itemId)`, jewellery and ancient bonus by attribute) | `ItemOptionDefinition.cs`, `GuidHelper.cs` | 9a6 (O27) |
| P24 | OpenMU's import has to change options in place: players' items point at `IncreasableItemOption` directly (`ItemOptionLink.cs:21`), and guardian definitions have two options with the same number, so the key is number plus attribute | phase C | C |
| P25 | The Wizard's Ring (13/20) is three items by + level: +0 Wizard's Ring, +1 and +2 Ring of Warrior (tutorial items dropped at level 40 / 80 for a box), +3 Ring of glory. Season 6 servers give the bonus at +0 only; the client shows it at +0 and +3, OpenMU gives it at every level (base power-ups) and has no Lethal Wizard's Ring; the client gives the Lethal Wizard's Ring its bonus at every level (`ZzzInfomation.cpp:475-483`) | `ZzzInfomation.cpp:453-483`, OpenMU `Jewelery.cs:274-287` (`CreateWizardsRing`: `MaximumItemLevel = 2` at `:277`, the base power-ups at `:282-286`) | 9b5 (O26) |
| P26 | The socket Lightning "critical damage increase" is 30 / 32 / 35 / 40 / 50 in the client and Season 6, 15 / 20 / 25 / 30 / 40 in OpenMU, which copies the row above it | `socketitem_<lang>.bmd` (id 31), `SocketSystem.cs:366` | 9b4 (O26) |
| P27 | The client's excellent range "groups 0 to 5" includes arrows and bolts; Season 3 to 6 item files mark them "no options", and OpenMU links none (`Weapons.cs:364-377`) | `CalcExcellentOptions` | 9a3 (O26) |
| P28 | Books' excellent level / 20 and +2 % raise curse damage in the client (`ZzzInfomation.cpp:2383-2386`, Webzen's own 2008 fix `KJH_FIX_WOPS_K27580_EQUIPED_SUMMONING_ITEM`), wizardry in OpenMU and in the decompiled Season 6 server | `ZzzInfomation.cpp` | 9a3 (O26, known difference) |
| P29 | Tooltips never reset `TextBold[]`, and of the `MaxTextListRows` rows (98, `ZzzInventory.h:91-95`) clear only the first 30 texts and 20 colours (`ZzzInventory.cpp:1768-1772`), so a long tooltip (a 32-row description with set or socket lines) also keeps the colours of the tooltip before from row 20 on; the ancient bonus lines (`RenderDefaultOptionText`, `CSItemOption.cpp:641-658`) and the Horn of Fenrir's colour lines (`ZzzInventory.cpp:2183-2200`) set no bold, so they are bold when the tooltip before had a bold row there | `BuildItemTooltipText` | before 9a4 (O31) |
| P30 | Smaller exchange details: the Cape of Fighter links OpenMU's shared "2nd Wing Options" (`Wings.cs:88`); Dinorant's absorb and Blue Fenrir's are factors below 1 (`DamageReceiveDecrement` 0.95, 0.90) where the tooltip shows "absorb 5 %"; the Dark Horse is −0.15 plus `HorseLevel × −0.005` in OpenMU (its defense counts the total agility on both sides since sven-n/MuMain#1007, `Pets.cs:298`); socket Fire #0 divides `TotalLevel` by 20, 19, 18, 17, 14 by sphere level; a `divisor` read back from a float (1/0.05f = 19.9999997) needs rounding; Gold Fenrir uses `TotalLevel` | OpenMU `Pets.cs`, `SocketSystem.cs:331` | 9a6, 9b1, 9b4, 9b5 |
| P31 | OpenMU's numbering differs from Webzen's in two places: its Earth socket offset is 36 where Webzen's ids start at 34, and its guardian option 1 is Webzen's slot 2 (the slots are swapped); the guardian `number` is the item group, which Webzen doesn't have | `ItemSerializerHelper.cs:68`, `GuardianOptions.cs` | 9b3, 9b4 (OpenMU issue) |

## Target design

The finished design, put together from O1 to O31. It is written so that
someone who did not take part in the decisions can implement it: every
section names the files and functions it changes. Field names are this
design's working names; the pull request that first reads a field fixes its
name in `docs/item-data.md`. Points that are no decision of their own but
follow from two of them say so. File and line references are those of
`88cf9d208` (see the note at the top); look a function up by name if the
lines moved.

**How to read the examples.** JSON examples use the real values of the
client (`Engine/Object/ZzzInfomation.cpp`) and of OpenMU's Season 6 data.
Where the two differ, the value O26 decided is written (mostly the client's,
OpenMU's for the second tier wings and the ammunition) and the difference
is listed in the problems.

### 1. Files

```
src/bin/Data/Items/
├── Group00_Sword.json …          item files: each item gets "options" (section 6)
├── Options/                      NEW, the option definitions (sections 2-4)
│   ├── Luck.json                 9a1
│   ├── Option.json               9a2  the additional option (+4 … +16 in Season 6)
│   ├── Excellent.json            9a3
│   ├── Ancient.json              9a4  the ancient bonus (the ancient set options come with phase 10)
│   ├── Wings.json                9b1
│   ├── Harmony.json              9b2
│   ├── Guardian.json             9b3
│   ├── Socket.json               9b4
│   ├── Pets.json                 9b5  Horn of Fenrir, Dark Horse, Horn of Dinorant, the Wizard's and skeleton rings (type fixed)
│   └── OptionRules.json          9a1  which items get which definitions (section 6)
├── Tooltips/
│   ├── Tooltip.json              unchanged until 9b6 (section 9)
│   └── OptionTexts.json          NEW 9a1, the option lines' texts (section 5)
└── Sync/
    └── KnownDifferences.json     not shipped (O30): written by Item Sync when a user ticks Known; gets "options" in 9a6 (section 12)
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
      "name": "excellentPhysicalAttack",
      "type": "excellent",
      "displayName": {"en": "Excellent Physical Attack Options", "de": "…"},
      "openMu": {"id": "00000083-0013-0000-0000-000000000000", "name": "Excellent Physical Attack Options"},
      "server": {"addsRandomly": true, "addChance": 0.001, "maximumOptionsPerItem": 2},
      "options": [
        {"number": 1, "attribute": "manaAfterMonsterKillMultiplier", "add": {"values": 8}, "text": "manaAfterKill"},
        {"number": 2, "attribute": "healthAfterMonsterKillMultiplier", "add": {"values": 8}, "text": "lifeAfterKill"},
        {"number": 3, "attribute": "attackSpeedAny", "add": {"values": 7}, "text": "increaseAttackSpeed"},
        {"number": 4, "attribute": "physicalBaseDmgIncrease", "percent": {"values": 2}, "text": "increaseDamagePercent"},
        {"number": 5, "attribute": "physicalBaseDmg", "add": {"values": 0, "per": [{"attribute": "level", "divisor": 20}]}, "text": "increaseDamageLevel20"},
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
| `openMu` | no | OpenMU's labels of the same definition (O27): `id`, its GUID where OpenMU's is fixed (left out for harmony and guardian, whose GUIDs are random, P4), and `name`, its English name, which OpenMU's code looks definitions up by. The exchange matches through them; a definition without them is new to OpenMU. The game does not read them. |
| `server` | no | OpenMU's server-only values (O16): `addsRandomly` (bool), `addChance` (0 to 1), `maximumOptionsPerItem` (int). The game does not read them. |
| `bits` | no | `true`: the option level is a set of bits, each option's `number` its bit (the Horn of Dinorant, section 3). |
| `itemLevels` | no | For `fixed`: the item + levels at which the options count (the Wizard's Ring); left out, every level. |
| `options` | yes | The options, at least one. |

**Option fields.**

| Field | Required | Meaning |
|---|---|---|
| `type` | no | The option's own type, for a definition whose options differ in type (the Horn of Fenrir, O28); else the definition's `type`. |
| `number` | yes, except types with one option (`luck`, `ancientBonus`) and `fixed`, where it is left out and read as 0 | OpenMU's `ItemOption.Number`; what it means depends on the type (section 3). Unique within a definition, except for options of one type that apply together (`guardian`, the four `goldFenrir` options, `fixed`, section 3). |
| `subOptionType` | no | OpenMU's `SubOptionType`: the socket element (`fire`, `water`, `ice`, `wind`, `lightning`, `earth`). Only for `socket`. |
| `attribute` | yes | The attribute it raises, OpenMU's `Stats` name in camelCase (O6, section 4). |
| `add` / `addFinal` / `percent` | exactly one | How it adds, with the value as an object (section 4): `{"values": 7}`, `{"values": [4, 8, 12, 16]}`, … A bare number is fatal, so there is one way to write a value. |
| `levelType` | no | `"itemLevel"`: the level of the value is the item's + level, not the option's own level (O11, O25). |
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
| `wing` | Wing | excellent byte | 1 to 4, bit `number - 1` (O9; the second tier wings ship 1 to 3, O26) | 9b1 |
| `ancientOption` | AncientOption | ancient byte (with phase 10's sets) | 1 to n, in the order the set gives them | 10 |
| `ancientBonus` | AncientBonus | ancient byte, high nibble = level | – (one option) | 9a4 |
| `harmony` | HarmonyOption | harmony byte, high nibble = number, low nibble = level | 1 to 10 | 9b2 |
| `guardian` | GuardianOption | flag 0x40 (no byte): all options of the item's guardian definition | the item group (OpenMU: 0xF0 for weapons, 7 to 11 for armour); both options of a definition have the same number | 9b3 |
| `socket` | SocketOption | socket bytes: `(sphere level - 1) * 50 + element offset + number` | 0-based within its element | 9b4 |
| `socketBonus` | SocketBonusOption | socket header, high nibble | 0 to 5, unique across the bonus definitions | 9b4 |
| `fenrir` (the definition), its options `blackFenrir`, `blueFenrir`, `goldFenrir` | BlackFenrir, BlueFenrir, GoldFenrir (on the options) | excellent byte, bit test `ExcellentFlags & number` (OpenMU ORs the colours, `ItemSerializerHelper.cs:302-317`; for the single values it sends this equals today's `==`); options of the one `fenrir` definition (O28) | the colour value (1, 2, 4); the four gold options share 4 | 9b5 |
| `darkHorse` | DarkHorse | none (always on) | 1 to 2 | 9b5 |
| `fixed` | none (OpenMU's definitions whose options have no type, O29) | none (always on) | – (0 for every option, as OpenMU's typeless options, whose `Number` is never set, `Jewelery.cs:240-261`; the options apply together, and the exchange tells them apart by number plus attribute, P24) | 9b5 |

- Two definitions of an item that read the same byte, of any type
  (`excellentPhysicalAttack` with `excellentWizardryAttack`, `excellent` with
  `wing` or a Fenrir type, two of type `option`), are a load error (O9,
  section 10).
- The Horn of Dinorant's options are of type `option`, but its option level
  is a set of bits, not a level (OpenMU's "fixed option": type `option`
  without levels). Its definition has `"bits": true`; each option's `number`
  is its bit value (1 absorb 5 %, 2 max AG +50, 4 speed +5), and the option
  level selects the options whose bit is set (9b5).
- The Wizard's rings have options that need no byte (fixed 10 % / 10 % /
  speed 10, Lethal 15 / 15 / 10, at levels 0 and 3 only for the Wizard's
  Ring; `CalcExcellentOptions` `ZzzInfomation.cpp:453-483`). They become
  definitions of type `fixed` (O29); the Wizard's Ring's has
  `"itemLevels": [0, 3]` in the move and `[0]` from 9b5's fix on (Season 6,
  P25). OpenMU writes them as the ring's base power-ups, not as options, so
  the exchange leaves these two out.
- The skeleton rings' effects (13/39, 13/122) are definitions of type
  `fixed` too, made from their description lines (O29); unlike the Wizard's
  rings they are exchanged with OpenMU's definitions whose options have
  no type.
- Skill is no option (Scope): it stays the item's `skill` field.

### 4. Values

**How a value adds (O6).** `add` adds the value as it is, `addFinal` adds it
after the other additions (OpenMU's `AddFinal`, the wings' damage), and
`percent` adds the value in per cent (OpenMU's `Multiplicate` with the
factor `1 + value / 100` for attributes that multiply, or `AddRaw` with
`value / 100` for chances such as `excellentDamageChance`; the exchange
knows which from the attribute, section 12).

**Levels (O11, O25).** The value is always an object, with the field names
of `ItemStats.json`'s level bonus tables; a value with levels is always a
list:

```json
{"values": 7}                                     // a constant: 7 at every level
{"values": [4, 8, 12, 16]}                        // levels 1 to 4 (the additional option)
{"values": [4, 5, 6, 7]}                          // custom values per level
{"levelType": "itemLevel", "values": [50, 55, …, 125]}  // item levels 0 to 15 (second tier wing HP)
{"firstLevel": 3, "values": [6, 7, …, 25]}        // a harmony option from its own first level
```

How the value at a level `L` is computed:

- **A constant** (`values` a number): that number at every level.
- **A list** (`values` a list): the entry `L - firstLevel`. Its length is
  the option's last level on both sides (O25, P20): a level above the list
  takes the last entry, and the exchange writes exactly these levels to
  OpenMU. `perLevel` is not allowed (fatal); **Fill levels** in Item
  Options writes a list from a start value and a step.
- **A level below `firstLevel`** gives 0, as today: the harmony bmd holds 0
  at every level below an option's first level, so the line shows "+0"
  (`ZzzInventory.cpp:2094-2098`), and an ancient bonus of level 0 adds
  nothing and shows no line (`CSItemOption.cpp:567`).
- `firstLevel` defaults to 1, or to 0 with `"levelType": "itemLevel"`.

Which level `L` is:

| The option's type | `L` |
|---|---|
| `option`, `harmony`, `socket`, `ancientBonus` | the option's own level from the server (option level, harmony level, sphere level, bonus level) |
| any type, with `"levelType": "itemLevel"` | the item's + level, 0 to 15 (second tier wing HP and MP, the Cape of Lord's command) |
| the types without a level (`luck`, `excellent`, `wing`, `guardian`, `socketBonus`, the Fenrir types, `darkHorse`, `fixed`) | 1, so a constant is its value |

Whether an option is there at all is decided by the server's byte (section
7), not by its value: an option level of 0 means no additional option, and a
value of 0 still shows a line (the level / 20 option of a level 15 hero).

**Terms on hero or item attributes (O7).** `per` adds terms to the value:

```json
{"values": 0, "per": [{"attribute": "level", "divisor": 20}]}                           // excellent level / 20 (P16; totalLevel after the fix, O26)
{"values": 5, "per": [{"attribute": "totalAgility", "divisor": 20}, {"attribute": "horseLevel", "times": 2}]}  // Dark Horse (total agility, as OpenMU)
{"values": 0, "per": [{"attribute": "totalLevel", "divisor": [20, 19, 18, 17, 14]}]}    // socket Fire #0, a divisor per sphere level (P30)
```

A term is `attribute * times` or `attribute / divisor`, each term rounded
down on its own, as the client's integer code does today. `times` and
`divisor` can be a list by level like a value (socket Fire #0). The exchange
writes `divisor` as OpenMU's operand `1 / divisor` and reads it back
rounded to the nearest whole divisor (OpenMU stores 1/20 as a float,
P30); an operand whose inverse is no whole number (0.3, −0.005) comes back
as `times`. The attributes a term can read, with OpenMU's names where
OpenMU has the attribute:

| Term attribute | Client source | OpenMU |
|---|---|---|
| `level` | `CharacterAttribute->Level` | `Level` |
| `totalLevel` | `Level + MasterLevel` | `TotalLevel` (`Stats.cs:143`) |
| `masterLevel` | `Master_Level_Data.nMLevel` | `MasterLevel` |
| `totalStrength`, `totalAgility`, `totalVitality`, `totalEnergy`, `totalLeadership` | the hero's stats with their additions (`CharacterAttribute->Strength + AddStrength`, …; the Dark Horse's agility, `SetPetItemConvert`) | the same names |
| `strength`, `dexterity`, `vitality`, `energy`, `leadership` | the hero's base stats (`CharacterAttribute->Dexterity`, …) | `BaseStrength`, `BaseAgility`, … |
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
| `excellent` | `physicalBaseDmg` / `wizardryBaseDmg` with a `level` or `totalLevel` term | add | `AT_IMPROVE_DAMAGE_LEVEL` / `AT_IMPROVE_MAGIC_LEVEL` | yes |
| `excellent` | `physicalBaseDmgIncrease` / `wizardryBaseDmgIncrease` | percent | `AT_IMPROVE_DAMAGE_PERCENT` / `AT_IMPROVE_MAGIC_PERCENT` | yes (2) |
| `excellent` | `attackSpeedAny` | add | `AT_IMPROVE_ATTACK_SPEED` | yes (7) |
| `excellent` | `healthAfterMonsterKillMultiplier` | add | `AT_IMPROVE_GAIN_LIFE` | no |
| `excellent` | `manaAfterMonsterKillMultiplier` | add | `AT_IMPROVE_GAIN_MANA` | no |
| `ancientBonus` | `totalStrength` … `totalVitality` | add | none: `CSItemOption::GetDefaultOptionValue` (`CSItemOption.cpp:546-558`) returns the definition's value at the bonus level instead of the level, and its two callers in `getAllAddState` (`:675-687`, `:765-777`) add it without today's `* 5`, so `[5, 10]` adds what it adds today and an edited list is added as written | – |

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
- **Languages:** every language the `.resx` texts have, `en` first, the
  others sorted. The option texts exist in 11 languages: `Game.zh-CN.resx`
  has 197 entries and none of them. `OptionTexts.json` gets the same
  locale check as `Tooltip.json` (`ValidateTooltipTextLocales`,
  `ItemDataHandler.cpp:224`).
- **Names in this document:** the text names the code uses today are the
  generated `I18N::Game::*` identifiers (`IncreaseMaxHP4`, `AdditionalDmgD`);
  their `.resx` key is the English text itself (`Increase Max HP +4%%`).
  Removing a `.resx` text means removing its `<data>` entry in every
  `Game.<lang>.resx`; `tools/ResxGen` regenerates the identifiers at build
  time (`docs/translation-system.md`).

### 6. Item links and rules

**On the item (O15, O24).** Each item file entry that has options gets
`options`, the names of the definitions the item can have:

```json
{"number": 0, "name": {"en": "Kris"}, "options": ["luck", "physicalDamageOption", "excellentPhysicalAttack"], …}
```

- The field goes into `ItemDefinition` (`Data/GameData/ItemData/ItemDefinition.h`,
  next to `statProfile`) and the item JSON format (`ItemJsonFormat.cpp`,
  next to `visit("statProfile", …)` at `:74`).
- **The order does not matter** to the game (section 8 orders by type). The
  writer sorts the list by the type order of section 8, then by name, so
  saving gives the same file.
- **Written in the file (O24):** the game reads only this list. An item
  without `options` has no options; the writer leaves the field out only for
  an empty list. An item file saved before 9a1 gives no warning (O33). The rules (below) are read by the editor only (and the data test; their
  code builds in every build, O34), as `statProfile`'s are (`WithDerivedFields`, `#ifdef _EDITOR`, P18), with
  one difference: an empty list stays empty, it is never "follow the
  rules". The bmd import, Item Add and an edit that changes what a rule
  reads write the list the rules give into the item. 9a1 writes the lists
  into the shipped item files once with the editor's rules (its data
  commit), one line per item with options; 9a2 and 9a3 add their
  definitions the same way (the jewellery gets its first line in 9a2).
- A link to a definition that does not exist is an error (the link is
  dropped).

**Rules (O15, O24, O34).** `Options/OptionRules.json`, read by the editor and
the data test only (the code builds in every build, O34),
has the shape of `profileRules` in `ItemStats.json`: each rule gives a list
of definitions (`options`, which may be empty) to the items it matches; the
first rule that matches wins; an item no rule matches gets no options.

- Conditions: `group`, `slot`, `weaponKind`, `wingTier`, `tags` and
  `requirements`, as `profileRules`, plus two new ones: `numbers` (item
  numbers; with `group`) and a `!` before a tag for "without the tag".
- `StatProfileRule` (`Data/GameData/ItemData/StatProfileRule.{h,cpp}`) has
  neither `numbers` nor `!tag`, and its target is one profile name. So the
  condition part moves into a struct of its own (`ItemRuleConditions`, same
  folder), which `StatProfileRule` keeps using unchanged and a new
  `OptionRule` uses with the two new conditions; all three build in every
  build (O34). `ItemStats.json` and its
  `formatVersion` do not change (`docs/item-data.md:799`: a new condition in
  `profileRules` would need a new version).

```json
{
  "formatVersion": 1,
  "rules": [
    {"options": [], "tags": ["ammunition"]},
    {"options": ["luck", "physicalAndWizardryDamageOption", "excellentPhysicalAttack"], "tags": ["magicSword"]},
    {"options": ["luck", "physicalDamageOption", "excellentPhysicalAttack"], "group": [0, 1, 2, 3, 4]},
    {"options": ["luck", "curseDamageOption", "excellentWizardryAttack"], "group": [5], "tags": ["summonerBook"]},
    {"options": ["luck", "wizardryDamageOption", "excellentWizardryAttack"], "group": [5]},
    {"options": ["luck", "defenseRateOption", "excellentDefense"], "group": [6]},
    {"options": ["luck", "defenseOption", "excellentDefense"], "group": [7, 8, 9, 10, 11]},
    {"options": ["luck"], "group": [12], "numbers": [0, 1, 2, 3, 4, 5, 6, 41, 42, 49]},
    {"options": ["luck"], "slot": ["wings"], "wingTier": ["third"]},
    {"options": ["luck"], "group": [13], "numbers": [30]},
    {"options": ["jewelryHealthRecovery", "excellentDefense"], "group": [13], "numbers": [8, 9, 21, 22, 23]},
    {"options": ["ringOfMagicMana", "excellentDefense"], "group": [13], "numbers": [24]},
    {"options": ["jewelryHealthRecovery", "excellentWizardryAttack"], "group": [13], "numbers": [12, 25, 27]},
    {"options": ["jewelryHealthRecovery", "excellentPhysicalAttack"], "group": [13], "numbers": [13, 26]},
    {"options": ["pendantOfAbilityAg", "excellentPhysicalAttack"], "group": [13], "numbers": [28]}
  ]
}
```

These are 9a's rules at its end (after 9a3). Each 9a pull request adds the
definitions of its group to the lists; 9a1 writes the same rules with
`luck` only. The first rule keeps ammunition from falling through to the
groups 0 to 4 rule; it stays empty (O26). Apart from that the rules
reproduce today's code ranges:

- `luck`: groups 0 to 11 without `ammunition`, and the wings today's luck
  code names (`ZzzInfomation.cpp:523-551`): `ITEM_WING` to
  `ITEM_WINGS_OF_DARKNESS` (12/0 to 12/6), `ITEM_WING_OF_CURSE` and
  `ITEM_WINGS_OF_DESPAIR` (12/41, 12/42), the Cape of Fighter (12/49), the
  Cape of Lord (13/30) and every third tier wing. Small wings get none.
- Excellent: defensive for groups 6 to 11 and the rings 13/8, 13/9, 13/21 to
  13/24; offensive for groups 0 to 5 and the pendants 13/12, 13/13, 13/25
  to 13/28, but not for arrows and bolts, which the code's range includes
  (P27: Season 6 and OpenMU give them none; 9a3 names the change and
  re-records the synthetic variants of the two items), wizardry for
  group 5 and the pendants of Lightning (13/12), Ice (13/25) and Water
  (13/27) (`CalcExcellentOptions`, `ZzzInfomation.cpp:375-450`).
- The additional option: as the table in section 15. **Do not** use the slot
  `ring`: it also holds the transformation rings, the seals, the Wizard's
  rings and the gem rings, which have no additional option.
- Today's code also gives the life regeneration option to every other item
  from `ITEM_RING_OF_ICE` to the end of group 13 (transformation rings,
  Loch's Feather, …) when the server sends an option level. OpenMU attaches
  no option to them, so the server never does; the rules leave them out, and
  the 9a2 pull request lists them as the one place its links differ from
  the code (9a3's is the ammunition, P27). Its snapshot changes for them in the
  `option` and `full` variants: 13/14 to 13/18 write an empty line today
  (`ZzzInventory.cpp:1149`), the others (13/10, 13/20, the Horn of Fenrir
  13/37, the transformation rings, …) a visible "Automatic HP recovery N%",
  and the gem jewellery (13/109-115) none, though its `SpecialNum` is above
  0. Dropping the link also changes, for these synthetic variants only,
  `hasOptions` (the blank row before the options, the name colour from blue
  to white), the price (group 13 `Gold += Gold * SpecialValue`,
  `ZzzInfomation.cpp:1253-1260`), the mixes' `m_iOption`, `CompareItem`'s
  `SpecialNum` and the Wizard's rings' slot positions (section 7). 9a2
  re-records these hashes and names the items.

**Kept matching**, as `statProfile`: `FollowStatProfile` and
`CItemDataHandler::WithDerivedFields` (`ItemAttributeConversion.h:58`,
`ItemDataHandler.cpp:365-388`, called at `:703` and `:957`) are the model.
The bmd import, Item Add and every edit that changes what a rule reads
(group, number, slot, weapon kind, wing tier, tags, requirements) give the
item the list the rules give it now, unless its list differed from what the
rules gave it before the edit; then the list stays (the Bone Blade with
`guardianWeapon`). Item Basics marks a list that differs from its rules and
offers "Use the rules"; that is no data issue, so the shipped data, whose
Bone Blade differs on purpose, still loads without problems.

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
  level 0 means none. Today's code ignores the type for every item that is
  no wing and for 12/0 to 12/2, 12/41 and 13/30, so a definition with one
  option (`number` 0) takes it whatever type the server sends, and such an
  item keeps its option as today.
- `fixed`: always, without a byte (O29; the Wizard's Ring only at its
  `itemLevels`).
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
`GetItemOptionValue` (section 4) find the option through it. Both arrays
have O23's 16 slots, and every slot is written through its checked helper
(P17), which drops a slot past the 16th and logs it once per item type.
The skeleton rings' `fixed` options (O29) take no slot: their effects have
no client stat today and show as description lines, so `SpecialNum`, the
price and the name colour stay as they are. The stat code reads the Wizard's rings' slots by position
(`RRing/LRing->SpecialValue[1]`, `ZzzInfomation.cpp:2047-2057`, and
`SpecialValue[0]`, `:2292-2298`), so 9b5 keeps their slot order.

### 8. In the game

**Order.** `SetItemAttributes` writes today: wing options, skill, luck,
additional option, the Dark Horse, excellent options, the Wizard's rings.
The new code writes the options of the linked definitions in this order of
types: `wing`, then the skill (as today, from the item's `skill`), `luck`,
`option`, `darkHorse`, `excellent`, `fixed`. Within a
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
request goes through all of them. `SetItemAttributes` calls
`ComputeItemStats` (`:494`) before `Special[]` and `SpecialSource` are
built, so there it uses the links-and-byte form. The known ones:

| Place | Test today | 9a3 |
|---|---|---|
| `StandardItemName` (`ZzzInventory.cpp:1604-1616`), name prefix, and `StandardItemNameColor` (`:1619-1630`, green while `SpecialNum > 0`) | `ExcellentFlags > 0`, no exception | `HasExcellentOptions`. Same for server items: wings and the Horn of Fenrir get their names from `nameLines` (`Group12_Wing.json`, `Group13_Helper.json`); check it per item in the snapshot. Snapshot variants that set the byte on items that cannot be excellent (`full` sets 63 on every item, `ItemTooltipSnapshot.cpp:31`) change: 9a3 re-records them and lists the items. |
| `CalcMaxDurability` (`ZzzInventory.cpp:1056-1061`) | `> 0`, not second or third tier wings, not `IsDivineArchangelWeapon` | `HasExcellentOptions` and still not `IsDivineArchangelWeapon` (those weapons link `excellentPhysicalAttack`, so that exception stays); the wing exception goes. The Horn of Fenrir gets the +15 today (bytes 1, 2, 4; no Dark Lord pet), so until 9b5 it keeps the byte test: `HasExcellentOptions` or the Horn of Fenrir with a colour |
| `CalcDurabilityPercent` (`ZzzInfomation.cpp:1785`) | `(excellentFlags & 63) > 0`, no exception | **stays** (P15: today it gives wings with wing options the +15 that `CalcMaxDurability` leaves out; changing it changes real wings) |
| `ComputeItemStats` (`GameLogic/Items/ItemStats.cpp:138-201`; wing checks at `:151`, `:186`, `:199`; the drop level at `:160` has none) | `ItemStatInput::excellentFlags` (`ItemStats.h:35-43`) and its own wing checks | `ItemStatInput` gets `bool excellent`, set by the caller (`SetItemAttributes` from the links and the byte, the editor from the links); the wing checks go. The drop level (`:159-160`) **stays** on the byte (`excellentOptions`, `:138`): wings with wing options and the Horn of Fenrir get the excellent drop-level bonus that feeds `ComputeStatRequirement` today, so `excellent` there would change their stat requirements. `excellentFlags` stays in the input for it and for whatever else reads it. The tests that fill `ItemStatInput` by position (`test_item_stats.cpp:460`, `:744`, `test_item_database_files.cpp:4192`) name their fields. |
| Tooltip state `excellent` (`ItemTooltipValues.cpp:176`) | `ExcellentFlags > 0` | `HasExcellentOptions`; same snapshot rule as the name |
| The mixes' `RCP_SP_EXCELLENT` (`MixMgr.cpp:171`; recipe matching `:602-603`, success value `:911`) | `ExcellentFlags > 0` | **stays**: wings with wing options and the Horn of Fenrir count as excellent for mixes today |
| `IsExcellentItem` (`ZzzInventory.cpp:3068`), the MU Helper's "pick up excellent" filter (`HelperPickupFilter.cpp:23`) | `ExcellentFlags > 0` | **stays**: it picks up wings with options and Fenrir today |
| Ground label rule `{"when": ["excellentFlags", "!upperTierWings"], "style": "excellent"}` (`GroundLabels.json:46`) | data, with its own wing exception | stays as data; the Horn of Fenrir with a colour keeps the excellent style |
| `IsHighValueItem` (`ItemCategories.cpp:464-468`), `Trade.cpp:765` | the byte | stay: the same for every item the server can send (wings and Fenrir are `valuable` or on the lists) |
| The price (`ItemValue`, `ZzzInfomation.cpp:863-888`, `:1372-1388`) | the `Special` ids, which include `AT_IMPROVE_ATTACK_SPEED` and the `*_PERCENT` ids (wings, Dinorant, Wizard's rings) | **stays** on `Special[]` (O9 changed) |
| Fenrir model and colour (`ZzzCharacter.cpp:246-250` and others) | `== 1 / 2 / 4` | stays until 9b5 |
| The Horn of Fenrir's run speed (`ZzzCharacter.cpp:6244`: 19 with a colour, 17 without) | `Helper.ExcellentFlags > 0` | stays until 9b5, as the model and colour |
| The excellent glow (`ZzzObject.cpp:7848`) | `(ExcellentFlags & 63) > 0` and the model's `HasExcellentGlow` (`ItemGlow.cpp:144`) | **stays**: the glow data already picks the models that glow |
| `CompareItem` (`ZzzInventory.cpp:2964-2965`), the trade window's check that the other side's item changed (`Trade.cpp:775`) | compares the bytes | **stays**: it compares what the server sent |
| The character window's durability (`CharacterInfoWindow.cpp:1368`, `:1465`) | passes the byte to `CalcDurabilityPercent` | **stays**, as `CalcDurabilityPercent` (P15) |

**What stays code.** The stat code, the price, mixes and the comparison
read `Special[]` as today. The skill lines (mana per skill, the class notes)
stay code. `SpecialValue` is computed per option from section 4 when the
item is made. The values with `per` terms are computed again by
`GetItemOptionValue` each time the tooltip or the stat code asks
(section 4, P12); that is the one intended behaviour change of 9a3, named in
its pull request. Whether `level / 20` reads the hero's level or level plus
master level is the data's choice (O22): `level` in the move, `totalLevel`
from the fix right after 9a3 on (O26). The readers that stay on the byte
or the option bytes, not the links: the "+Skill / +Option / +Luck" ground
label suffixes (`ZzzInventory.cpp:2829-2848`), `AppendItemOptionKeywords`
(`:3087-3100`, the MU Helper's name filter), the state `skillLuckOrOption`
and `strengthPerOptionLevel`; `hasOptions` follows `SpecialNum`, so a
change of an item's links shows there (9a2, section 6).

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
{"options": "fixed", "color": "blue"},
{"id": "blankAfterOptions", "text": ""},
```

What 9b6 has to keep that the first draft did not list (found in the
review):

- The ancient bonus has a second line, "Increase Attribute Damage", for the
  elemental rings and pendants (`CSItemOption.cpp:649-654`); it depends on
  the item, not the bonus, and needs a line of its own (`when` on the
  item's tag).
- The gem jewellery's stop (`ZzzInventory.cpp:2137`) ends the whole loop,
  the skill lines too, so `{"code": "skillLines"}` gets
  `"when": ["!tag:gemJewelry"]` as well.
- Harmony is skipped for socket items (`:2078`) and has an error line for
  an option number its kind doesn't have (`:2108-2128`); `hasHarmonyOptions`
  covers neither, so 9b6 adds states for both.
- The 380 blank rows are written whenever `option_380` is set and the
  ItemAddOption data exists, even when the item has no record.
- The item description's after-options lines (`WriteItemDescription(…, true)`,
  `:2176`) run between the options section and the Horn of Fenrir, so the
  Fenrir lines go after them; the set options (`RenderSetOptionListInItem`,
  `:2378`) sit between the expiry lines and the sockets, and both run only
  while `!bItemTextListBoxUse`, which the socket lines have to keep.
- The skeleton rings' effects (O29) are description lines today; 9b5 keeps
  their place and colour, either by their description lines reading the
  values of their `fixed` definition or by the definition's texts standing
  at the same place, so the snapshot stays the same.
- The Dark Lord pets have a tooltip of their own with its own option loop
  (`GIPetManager.cpp:783-797`), which the snapshot doesn't cover;
  `tests/data/test_dark_lord_pet_tooltip.cpp` does.
- Variants with ancient bonus or Horn of Fenrir rows are recorded after
  O31's fix, so rows that set no bold don't record a previous tooltip's
  bold.

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
  `add`, `addFinal`, `percent`; a value of the wrong kind, `perLevel` in
  an option's value (O25); a `number` used twice in a definition (except
  options of one type that apply together: `guardian`, `goldFenrir`);
  `OptionRules.json` naming a definition that does not exist. (A rule with
  an empty `options` list is valid.)
- **Error:** an item linking a definition that does not exist (the link is
  dropped); an item linking two definitions that read the same byte or
  flag, of any type (section 3; the later one in the sorted list is
  dropped); an option naming a text `OptionTexts.json` lacks (the line is
  left out); an option text without `en`.
- **Warning:** an attribute the client has no effect for (section 4: shown,
  not applied); a definition no item and no rule links (except
  `ancientBonus` and `ancientOption`, which `attackType` and phase 10's sets
  link); a text no option names; an unknown field (at game load and in the
  editor, as for the item files, `docs/item-data.md:943`). A list that
  differs from its rules is no issue (O24): Item Basics marks it.

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
│ ┌ Definitions ─────────┐ ┌ excellentPhysicalAttack ───────────────────────┐ │
│ │ luck             412 │ │ Name [Excellent Physical Attack Options] (en ▾) │ │
│ │ physicalDmgOpt.  188 │ │ Type excellent   OpenMU 00000083-0013-… ✓       │ │
│ │▸excellentPhys…   156 │ │ Server ☑ random  chance [0.001]  max/item [2]   │ │
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
  list from a start level, a start value, a step and a last level (O11,
  O25); OpenMU's labels (O27) are shown and set by Update from server, and
  a change of the English display name warns first, because OpenMU's code
  looks definitions up by it; **Used by**
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
    {"name": "excellentPhysicalAttack", "type": "excellent",
     "displayName": {"en": "Excellent Physical Attack Options", "de": "…"},
     "openMu": {"id": "00000083-0013-0000-0000-000000000000", "name": "Excellent Physical Attack Options"},
     "server": {"addsRandomly": true, "addChance": 0.001, "maximumOptionsPerItem": 2},
     "options": [{"number": 3, "attribute": "attackSpeedAny", "add": {"values": 7}}, …]}
  ]
}
```

- The definitions in the shape of the client files without `text`; the
  texts and `OptionRules.json` are client-only. The Wizard's rings'
  `fixed` definitions are left out (OpenMU writes them as base power-ups);
  the skeleton rings' `fixed` definitions are exchanged as OpenMU's
  definitions whose options have no type (O29).
- **Labels (O27):** OpenMU's file carries each definition's GUID and its
  English name; MuEditor finds its definition by `openMu.id`, else by
  `openMu.name`, and Update from server fills these labels in. MuEditor's
  file carries them back, and OpenMU's import finds its definition the same
  way. A definition that matches neither is new on that side; a renamed
  harmony or guardian definition (no fixed id) is shown unmatched and the
  user pairs it.
- **The item field `options`**: the item exchange gets the field `options`
  (the links), listed in its `fields` (format version 2), written on
  OpenMU's side by an `ItemExchangeField` (`ItemExchangeField.cs`).
  Importing items whose links name a definition the client does not have is
  refused with a message, so the options file is imported first; the links
  travel as OpenMU's labels too. Links to a definition the exchange leaves
  out (the Wizard's rings' `fixed` definitions, O29) are left out of the
  field as well: the export doesn't write them, and an import or Update
  from server keeps the client's links to them, so the rings neither fail
  on OpenMU nor show OpenMU's empty list as a difference.
- **Mapping to OpenMU** (phase C does OpenMU's side): a definition is
  OpenMU's `ItemOptionDefinition`, matched through its labels (O27, P23).
  The Horn of Fenrir is one definition on both sides (O28). An option is an
  `IncreasableItemOption`
  with `Number`, `SubOptionType`, `OptionType` (the type's GUID), `Weight`
  (`server.weight`) and `LevelType`; its value is a `PowerUpDefinition` on
  the attribute: `add` → `AddRaw`, `addFinal` → `AddFinal`, `percent` →
  `Multiplicate` with `1 + v/100`, or `AddRaw` with `v/100` for the
  attributes OpenMU keeps as fractions (the exchange code keeps this list;
  for 9a: `criticalDamageChance`, `excellentDamageChance`,
  `damageReflection`, `armorDamageDecrease`, `healthRecoveryMultiplier`;
  9b adds its own, e.g. `defenseIgnoreChance`). The two "after kill"
  attributes are the exception of section 15: `add` 8 ↔ `AddRaw` 1/8; `per` terms →
  `RelatedValues` with `Multiply` (a `divisor` read back rounded, section
  4); a level list → one OpenMU value per entry, so the list's length is
  OpenMU's last level (O25, P20), stored in the shape OpenMU uses for that
  option (P3: level 1 in the base value for the additional option and the
  sockets, levels in `LevelDependentOptions` for the others;
  `RequiredItemLevel` = the level with `requiresItemLevel`). The import
  reads every shape into the list, so a round trip changes nothing on the
  server (its failed Jewel of Life falls to the lowest stored level). The 380 pants are exchanged 1:1, 100 on both sides (O32). A decrease
  that OpenMU writes as a factor below 1 (Dinorant's and Blue Fenrir's
  `DamageReceiveDecrement`) is the client's positive "absorb" per cent and
  9b5 names the conversion (P30).
- **OpenMU's side (phase C, P24):** the import changes options in place,
  found by number and attribute (guardian's two options share a number),
  because players' items point at them; it never replaces a definition's
  options.
- **Every import shows a diff** per definition, option and field, checked
  by hand, as phase 7; Apply is one undo step.
- **Known differences (O30)**: `Sync/KnownDifferences.json` gets an
  `options` list, `[{"definition": "excellentDefense", "fields": ["options.1"]}, …]`,
  written when a user ticks **Known**; nothing is shipped. `docs/item-data.md`
  lists the option differences with their reasons: zen, boots, the socket Lightning row, the Wizard's Ring at +1 and +2, the Lethal Wizard's
  Ring, the books' excellent options, the rounding of level / 20, the
  Elite Skeleton ring's vitality against max HP, Gold
  Fenrir's hero level (`ZzzInventory.cpp:2203`) against `TotalLevel` (P30),
  and OpenMU's Fenrir reader and its socket and guardian numbering (P21,
  P31).

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
  synthetic variants of items that cannot be excellent, arrows and bolts
  among them, section 8), it re-records exactly those and lists the items
  in its description. Variants with ancient bonus or Horn of Fenrir rows are
  recorded after O31's fix (9a4, 9b5). The `full`
  variant needs O23's 16 slots (skill, luck, option and six excellent
  options are nine). The lists of O25 end where Season 6 does, so variants
  above a list's end change: `full` sets option level 4 on every item, and
  the jewellery's recovery shows 3 % instead of 4 % (13/8, 13/9, 13/12,
  13/13, 13/21 to 13/28, with their price); 9a2's option levels 5 to 7 show
  +16 instead of +20 to +28, and 9b1's the same for wings. Servers that
  follow Season 6 never send these levels; 9a2 and 9b1 re-record exactly
  these and name them.
- **`Special[]` test**: for every item and every variant, the old code's
  `Special[]` / `SpecialValue[]` equals the new one (a test that runs both
  while the old code still exists, removed with it), except where a pull
  request names a change (P12 in 9a3).
- **Stat tests** (`tests/data/`, the item stat and requirement tests):
  unchanged.
- **Data test**: the shipped option files load without problems (section 10).

### 14. Where to start (9a1)

For whoever picks this up: 9a1 is the first pull request and the base of all
others. The option slots fix (O23) comes first; 9a1 builds on its 16 slots.

1. Read `docs/item-data.md` ("Files", "Loading and errors", "Item stats by
   level" for `values` and `profileRules`), this design's sections 1 to 7
   and 10, and `docs/CODING_RULES.md`.
2. Add `ItemOptionDatabase` (section 1), the loaders, **writers** and
   validators in `ItemJsonStorage` for `Options/*.json`, `OptionRules.json`
   and `OptionTexts.json` (Save Items writes them from 9a5 on, when Item
   Options changes them; the item lists that the conversion and the bmd import
   write are in the item files), and the calls in `CItemDataHandler::Load`
   (`OptionRules.json` in the editor build only, O34).
3. Add `options` to `ItemDefinition` and the item JSON format (section 6,
   O24: the game reads only the list); move the conditions of
   `StatProfileRule` into `ItemRuleConditions` and add `OptionRule` with
   `numbers` and `!tag`, in every build as `StatProfileRule` (O34); derive the
   lists where `statProfile` is derived, in the editor build only (section 6,
   "Kept matching").
4. Write `Options/Luck.json` (one definition, `luck`, type `luck`, one
   option: `criticalDamageChance`, `"percent": {"values": 5}`, texts
   `["luckJewelOfSoul", "luckCriticalDamage"]`, server: random, chance
   0.25, 1 per item), `Tooltips/OptionTexts.json` with these two texts from
   the `.resx` (`Luck (success rate of Jewel of Soul +25%%)` and
   `Luck (critical damage rate +5%%)`, `Game.en.resx:238` and `:256`, all
   languages; the second becomes `…+{value}%`), and `OptionRules.json` as in
   section 6 with `luck` only. The pull request's data commit runs the
   editor's rules over the shipped items once and writes `"options":
   ["luck"]` into every item they give luck (O24), so the item files change
   by one line per such item; the data test (outside `#ifdef _EDITOR`, so upstream's
   pull request checks run it, O34) checks the lists against the rules.
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
| 9a4 | In `CSItemOption::GetDefaultOptionText` (`CSItemOption.cpp:567-590`), the switch on `attackType`: the value and text come from the bonus definition; `GetDefaultOptionValue` (`:553`) gives the definition's value, not `AncientBonusOption`, and the `Result * 5` of both callers (`:675-687`, `:765-777`) goes | the second line `IncreaseAttributeDamage` for rings and pendants (`RenderDefaultOptionText`, `:641-658`, depends on the item, not the bonus; phase 10 decides), and `GetDefaultOptionValue`'s result `SET_OPTION_*` for its callers (`getAllAddState`) |

### 15. The 9a data

**`Options/Luck.json` (9a1)**: `luck` (type `luck`; server: random, chance
0.25, 1 per item), one option `criticalDamageChance` `"percent": {"values": 5}`,
texts `luckJewelOfSoul`, `luckCriticalDamage`.

**`Options/Option.json` (9a2)**, all of type `option`, `number` 0, server:
random, chance 0.25, 1 per item (OpenMU's values). The text names are new;
the column after them is the `I18N::Game` text they are made from. The
lists end where Season 6 does (O25); today's code computes any level the
server sends, so a server that sends more shows the last entry, and
**Fill levels** writes a longer list for such a server.

| Name | Attribute | Value | Text name | From | Items (rules, section 6) | OpenMU |
|---|---|---|---|---|---|---|
| `physicalDamageOption` | `physicalBaseDmg` | `"add": {"values": [4, 8, 12, 16]}` | `additionalDamage` | `AdditionalDmgD` | groups 0-4 without `ammunition` | "Physical Base Damage (min and max) Option", 0x03 |
| `physicalAndWizardryDamageOption` | `baseDamageBonus` | `"add": {"values": [4, 8, 12, 16]}` | `additionalDamage`, `additionalWizardryDamage` | `AdditionalDmgD`, `AdditionalWizardryDmgD` (`ZzzInventory.cpp:2166-2171`) | `magicSword` | "Base Damage Bonus (physical and wizardry …) Option", 0x07 |
| `wizardryDamageOption` | `wizardryBaseDmg` | `"add": {"values": [4, 8, 12, 16]}` | `additionalWizardryDamage` | `AdditionalWizardryDmgD` | group 5 without `summonerBook` | 0x04 |
| `curseDamageOption` | `curseBaseDmg` | `"add": {"values": [4, 8, 12, 16]}` | `additionalCurseDamage` | `AdditionalCurseSpellD` | group 5 with `summonerBook` | 0x05 |
| `defenseRateOption` | `defenseRatePvm` | `"add": {"values": [5, 10, 15, 20]}` | `additionalDefenseRate` | `AdditionalDefenseRateD` | group 6 | 0x06 |
| `defenseOption` | `defenseBase` | `"add": {"values": [4, 8, 12, 16]}` | `additionalDefense` | `AdditionalDefenseD` | groups 7-11 | 0x02 |
| `jewelryHealthRecovery` | `healthRecoveryMultiplier` | `"percent": {"values": [1, 2, 3]}` | `automaticHpRecovery` | `AutomaticHPRecoveryD` | rings 13/8, 13/9, 13/21-23, pendants 13/12, 13/13, 13/25-27 | "Health recover for jewelery", 0x10 + attribute |
| `ringOfMagicMana` | `maximumMana` | `"percent": {"values": [1, 2, 3]}` | `maxManaIncrease` | `MaxManaIncreasedByD` | 13/24 | "Jewelery option Maximum Mana", GUID from the item id |
| `pendantOfAbilityAg` | `maximumAbility` | `"percent": {"values": [1, 2, 3]}` | `maxAgIncrease` | `MaxAGIncreasedByD` | 13/28 | "Jewelery option Maximum Ability", per item |

`AutomaticHPRecoveryD` is not shown for `ITEM_LOCHS_FEATHER` to
`ITEM_INVISIBILITY_CLOAK` (13/14 to 13/18) today (`ZzzInventory.cpp`, case
`AT_LIFE_REGENERATION`); they get no link (section 6), so that exception
goes.

**`Options/Excellent.json` (9a3)**, type `excellent`, server: random, chance
0.001, 2 per item. Bit = `number - 1`, as OpenMU and as today's code. Rules
in section 6.

| Name | # | Attribute | Value | Text name | From |
|---|---|---|---|---|---|
| `excellentDefense` | 1 | `moneyAmountRate` | `"percent": {"values": 30}` (Season 6; OpenMU 40, P14, O21) | `zenAfterKill` | `IncreasesAcquisitionRateOfZenAfterHuntingMonsters30` |
| | 2 | `defenseRatePvm` | `"percent": {"values": 10}` | `defenseSuccessRate` | `DefenseSuccessRate10` |
| | 3 | `damageReflection` | `"percent": {"values": 5}` | `reflectDamage` | `ReflectDamage5` |
| | 4 | `armorDamageDecrease` | `"percent": {"values": 4}` | `damageDecrease` | `DamageDecrease4` |
| | 5 | `maximumMana` | `"percent": {"values": 4}` | `increaseMaxMana` | `IncreaseMaxMana4` |
| | 6 | `maximumHealth` | `"percent": {"values": 4}` | `increaseMaxHp` | `IncreaseMaxHP4` |
| `excellentPhysicalAttack` / `excellentWizardryAttack` | 1 | `manaAfterMonsterKillMultiplier` | `"add": {"values": 8}` (mana / 8) | `manaAfterKill` | `IncreasesAcquisitionRateOfManaAfterHuntingMonstersMana8` |
| | 2 | `healthAfterMonsterKillMultiplier` | `"add": {"values": 8}` | `lifeAfterKill` | `IncreasesAcquisitionRateOfLifeAfterHuntingMonstersLife8` |
| | 3 | `attackSpeedAny` | `"add": {"values": 7}` | `increaseAttackSpeed` | `IncreaseAttackingWizardrySpeedD` |
| | 4 | `physicalBaseDmgIncrease` / `wizardryBaseDmgIncrease` | `"percent": {"values": 2}` | `increaseDamagePercent` / `increaseWizardryPercent` | `IncreaseDamageD` / `IncreaseWizardryDmgD` |
| | 5 | `physicalBaseDmg` / `wizardryBaseDmg` | `"add": {"values": 0, "per": [{"attribute": "level", "divisor": 20}]}` (`level` in 9a3, `totalLevel` from the fix after it, O22, O26) | `increaseDamageLevel20` / `increaseWizardryLevel20` | `IncreaseDamageLevel20` / `IncreaseWizardryDmgLevel20` |
| | 6 | `excellentDamageChance` | `"percent": {"values": 10}` | `excellentDamageRate` | `ExcellentDamageRate10` |

OpenMU writes the "after kill" options as the factor 1/8; the client file
writes the divisor 8 as `add` and the exchange converts (`8` ↔ `1/8` for
these two attributes, section 12). Books link `excellentWizardryAttack`,
and the stat code keeps adding its options to their curse damage
(`ZzzInfomation.cpp:2383-2386`, P28); that stays a known difference.

**`Options/Ancient.json` (9a4)**, type `ancientBonus`, no `server` values
(OpenMU links them through the sets, not randomly), no `number`:

| Name | Attribute | Value | Text name | From | `attackType` |
|---|---|---|---|---|---|
| `ancientBonusStrength` | `totalStrength` | `"add": {"values": [5, 10]}` | `increaseStrength` | `IncreaseStrengthD` | 1 |
| `ancientBonusAgility` | `totalAgility` | `"add": {"values": [5, 10]}` | `increaseAgility` | `IncreaseAgilityD` | 2 |
| `ancientBonusEnergy` | `totalEnergy` | `"add": {"values": [5, 10]}` | `increaseEnergy` | `IncreaseEnergyD` | 3 |
| `ancientBonusVitality` | `totalVitality` | `"add": {"values": [5, 10]}` | `increaseVitality` | `IncreaseStaminaD` | 4 |

Until phase 10 a fixed table in code maps the item's `attackType` to these
names (the last column; `CSItemOption.cpp:572-590`), and items get no
`options` link for them. 9a4 adds no `ancientOption` definitions: the type
is known to the loader so phase 10 can use it, and the set options stay
where they are until then.

### 16. The 9b groups in short

| PR | Source today | Definitions | Notes |
|---|---|---|---|
| 9b1 | `CalcWingOptions` (`ZzzInfomation.cpp:291-373`), the wing chain (`:620-826`) | 18 per-wing `option` definitions (O10, `Wings.cs:74-107` for the numbers); `secondTierWing` (3 options, shared by the second tier wings and the Cape of Fighter, O26, P5), `capeOfLord` (HP, MP, ignore defense, command `{"levelType": "itemLevel", "values": [10, 15, …, 85]}`), `thirdTierWing` (4); HP / MP / command with `levelType: itemLevel` | Wing types read the excellent byte (O9); the snapshot variants with bits 3 and 4 on second tier wings change and are listed |
| 9b2 | `JewelOfHarmonyOption_<lang>.bmd` (`UIJewelHarmony.cpp:15-88`) | `harmonyPhysicalAttack`, `harmonyWizardryAttack` (also books, P7), `harmonyDefense` (Webzen's types 1, 2, 3, O27); values `{"firstLevel": <Minlevel>, "values": [...]}` with `requiresItemLevel`; Zen costs per level stay with the definition as `"zen": [...]` (client-only); names into `OptionTexts.json`; OpenMU weights under `server` | `GetStrengthenCapability` reads the definitions; P11 is reported, not fixed |
| 9b3 | `ItemAddOption.bmd`, its 380 records (`ItemAddOptioninfo.cpp`) | one `guardian` definition per item kind (weapon, helm, armor, pants, gloves, boots), values as the bmd (boots 700, O26) except the pants' defensive skill, 100 where the bmd has 200, the one tooltip line 9b3 changes (O32) | needs #966; the buff records stay in the bmd (O13); OpenMU's slot order differs (P31) |
| 9b4 | `socketitem_<lang>.bmd` (`SocketSystem.cpp:605`) | one `socket` definition per element, values per sphere level (Lightning critical damage 30 / 32 / 35 / 40 / 50, P26); `socketBonus` definitions; the set bonus as `"combinations"` in `Socket.json` (OpenMU's `ItemOptionCombinationBonus`); value kinds 3 to 5 as `per` terms (O7), with a divisor per sphere level (P30) | `IsSocketItem`'s list becomes the links; P13; OpenMU's Earth offset (P31) |
| 9b5 | Fenrir (`ZzzInventory.cpp:2178-2245`), Dark Horse (`GIPetManager.cpp:580-605`), Dinorant (`ZzzInfomation.cpp:557-574`), Wizard's rings (`:453-483`), the skeleton rings' descriptions | one `fenrir` definition (O28), `darkHorse` (total agility, as OpenMU), `hornOfDinorant` (`bits`), `wizardsRing` and `lethalWizardsRing` and the two skeleton rings (`fixed`, O29) | section 3; the Wizard's Ring fix (+0 only, P25) is a pull request right after it; the skeleton rings' `fixed` options take no `Special[]` slot (section 7) |
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
| 0b | This design after its review (R1 to R9: O23 to O31, P17 to P31) | 0 |
| fix | An item keeps up to 16 option slots, written through one checked helper (O23, P17) | – |
| 9a1 | The file format and loader: `Options/*.json`, `Tooltips/OptionTexts.json`, `Options/OptionRules.json` (read by the editor and the data test, its code in every build, O34), the items' `options` lists written into the item files (bmd import, Item Add, kept matching, O15, O24), the attribute names and their client effects (O6), Data Checks and the data test. First group: luck | 0b, the slots fix |
| fix | Luck adds its critical damage rate of 5 % in the client (P8, O12) | 9a1 |
| 9a2 | The additional option of the items that are no wings (weapons, shields, armour, the jewelry's regeneration, the Ring of Magic, the Pendant of Ability) | 9a1 |
| 9a3 | Excellent options, defensive and offensive; the excellent byte read by the linked type (O9); arrows and bolts without (O26) | 9a1 |
| fix | Excellent level / 20 counts level plus master level, rounded down (O26, P16) | 9a3 |
| fix | A tooltip starts with no bold rows and no colours, all 98 rows (`MaxTextListRows`, O31, P29) | – (before 9a4) |
| 9a4 | The ancient bonus definitions (the type `ancientOption` known, no definitions; phase 10 adds them); `attackType` picks the bonus until phase 10 (O14) | 9a1, the bold fix |
| 9a5 | Item Options (O18), the links in Item Basics, Item Tooltip's controls for the groups so far, the control API's tooltip item builder (`ControlCommandsQuery.cpp:715-742`) | 9a1 to 9a4 |
| 9a6 | The option exchange file with OpenMU's labels (O27) and the item field `options` (O19); an OpenMU issue for OpenMU's own errors (O26) | 9a5 |
| C | OpenMU: the endpoint and the import page (phase C, OpenMU repository); options changed in place (P24) | 9a6 |
| 10 | Item sets (a design of its own) | 9a4 |
| 9b1 | Wings: the additional options per wing (O10), the second and third tier wing options | 9a3 |
| 9b2 | Jewel of Harmony (`JewelOfHarmonyOption_<lang>.bmd`) | 9a1 |
| 9b3 | Guardian (380), the 380 part of `ItemAddOption.bmd` (O13) | 9a1, sven-n/MuMain#966 |
| 9b4 | Sockets (`socketitem_<lang>.bmd`: seed options, bonus options, set bonus; O13) | 9a1 |
| 9b5 | Pets and specials: Horn of Fenrir (one definition, O28), Dark Horse, Horn of Dinorant, the Wizard's and skeleton rings (`fixed`, O29) | 9a3, the bold fix |
| fix | The Wizard's Ring has its bonus at +0 only (O26, P25) | 9b5 |
| 9b6 | The tooltip's option lines as layout lines per type (O17) | 9b1 to 9b5 |
| 9b7 | What stays true into `docs/item-data.md`; the items design's D13 and phase 11 changed (O13); this document deleted | 9b6 |

## Open questions

Answered on 2026-11-04: Q-O1 (O5), Q-O2 (O6), Q-O3 (O7), Q-O4 (O8), Q-O5
(O9), Q-O6 (O10), Q-O7 (O11), Q-O8 (O12), Q-O9 (O13), Q-O10 (O14), Q-O11
(O15), Q-O12 (O16), Q-O13 (O17), Q-O14 (O18), Q-O15 (O19), Q-O16 (O20),
Q-O17 (O21) and Q-O18 (O22). The review's questions, answered on
2026-11-05 (R1 to R3) and 2026-11-06 (R4 to R9): R1 (O23), R2 (O24), R3 (O25), R4 (O26), R5 (O27),
R6 (O28), R7 (O29), R8 (O30) and R9 (O31). The pull request's code review
(2026-11-06) raised three more: R10 (O32), R11 (O33) and R12 (O34). All
questions are answered. New
questions from the pull requests are added here.

Outside this design, found by the review:

- The client's English name of 13/39 is "Eilte Transfer Skeleton Ring"
  (OpenMU: "Elite Skeleton Transformation Ring"), a fix of the item data of
  its own (P22).

## Season 6 sources

The review checked every value difference against Season 6 (Episode 3, the
1.04d client) in two rounds, the second with sources independent of the
first round's main one:

- **Webzen's own client**: the 5.2 source this repository started from
  (commit `ee2c7a35e`, `source/ZzzInfomation.cpp`: the second tier wings'
  "code that is not applied", level / 20 by the hero's level, the 2008
  books fix `KJH_FIX_WOPS_K27580_EQUIPED_SUMMONING_ITEM`), and the Season 6
  Episode 3 client's data (`ItemAddOption.bmd`, `JewelOfHarmonyOption_eng.bmd`
  with no curse table).
- **Decompiled Season 6 servers** (read for values and behaviour only, no
  code taken): zTeam S6.3 (https://github.com/MuSeason6FixTeam/zTeamS6.3,
  "GameServer 1.00.90 JPN"), TitansTech (https://github.com/TitansTech/GameServer,
  Season 4.6 to 6, with Season 6 Episode 1 data "Generated from *.bmd"),
  Krono (Season 3), and MuEMU EX603 (https://github.com/ProjectRebirthMu/Server,
  a separate rewrite).
- **Official pages**: the Global MU guides archived in 2012 and 2013 (Season 6
  Episode 3: the Wizard's Ring, the quests), Global notice 3192
  (2012-11-30, the Lethal Wizard's Ring sold as "Critical Ring"), the Season
  14 renewal guide (https://muonline.webzen.com/en/gameinfo/guide/detail/91:
  the books' excellent options moved to curse in Season 14), the Season 16
  guide (https://muonline.webzen.com/en/gameinfo/guide/detail/117: curse
  harmony), and the current item guide
  (https://muonline.webzen.com/en/gameinfo/guide/detail/32).
- **Players and fan guides of the time**: the Webzen Global forum (2009:
  the 380 options per item kind; 2009-2010: 886 wing trade posts without
  max AG), guiamuonline's second tier wings guide (2010-05-25: max AG and
  attack speed "don't work and never appear"; +20 to +28 "only on unofficial
  servers").

Confidence and conflicts per value are in the decisions (O26) and problems
(P5, P6, P14, P16, P25 to P28).

## Tasks

1. [x] Answer the open questions one by one (O5 to O20).
2. [x] Write the target design from O1 to O20.
3. [x] Review the design against both code bases and Season 6 (R1 to R9,
   O23 to O31).
4. [ ] The pull requests of "Pull requests".
5. [ ] Move what stays true into `docs/item-data.md`, and point the items
   design (phase 9) and the roadmap (row "Items") at those docs.
6. [ ] Delete this document.
