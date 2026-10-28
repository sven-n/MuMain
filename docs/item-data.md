# Item data

All item definitions (names, size, slot, stats, requirements, tags and
rules) live in JSON files in `src/bin/Data/Items/`, one file per item
group; the item models are in `src/bin/Data/Items/Models/` (see
[Item models](#item-models)). The client loads them once at startup into an in-memory item
database; `Item_<lang>.bmd` is no longer read by the game.

The data matches OpenMU's `ItemDefinition` where both sides have the same
fields, so it can be exchanged with the server (see [Item Sync](#item-sync)).

---

## Files

| File | Group |
|---|---|
| `Group00_Sword.json` … `Group15_Etc.json` | 0–15, named after the `ITEM_GROUP_*` constants |
| `Stats/ItemStats.json` | What the + level, excellent options, ancient sets and option level add to the stats and requirements; see [Item stats by level](#item-stats-by-level) |
| `Tooltips/Tooltip.json` | The item tooltip's shared texts and layouts: which stat and requirement lines a tooltip shows; see [Tooltip layouts](#tooltip-layouts) |
| `Sync/KnownDifferences.json` | Not read by the game: the differences from the server that MuEditor's Item Sync leaves unchecked; see [Item Sync](#item-sync) |

The file name is only a convention; the group comes from the file content.

```json
{
  "formatVersion": 1,
  "group": 0,
  "items": [
    {
      "number": 5,
      "name": {
        "en": "Blade",
        "de": "Klinge",
        "es": "Espada",
        "pt": "Lâmina"
      },
      "width": 1,
      "height": 3,
      "slot": "mainHand",
      "skill": 22,
      "level": 36,
      "durability": 39,
      "damageMin": 36,
      "damageMax": 47,
      "attackSpeed": 30,
      "attackType": 1,
      "requirements": { "strength": 80, "dexterity": 50 },
      "classRequirements": { "darkWizard": 1, "darkKnight": 1, "fairyElf": 1, "magicGladiator": 1, "darkLord": 1 }
    }
  ]
}
```

- An item is identified by its group and `number` (0–511), the same as
  on the server.
- **Fields with their default value are left out.** Missing fields get the
  default: `0`/`false`, no slot (not equippable), no wing tier, no weapon
  kind, no tags,
  and the rule flags at their defaults (see [Rules](#rules)).
- Items are sorted by number and the fields always come in the same order,
  so saving unchanged data gives the same file.
- Files are UTF-8 with LF line endings.

### Names and translations

`name` holds the item's names by language: `en` (English, required) first,
then the translations sorted by language code. The codes are the same as
for the UI texts (`de`, `pt`, `es`, …). A plain text (`"name": "Kris"`) is
read as the English name only; saving writes the object form.

- The game shows the name in the current UI locale (options window
  → language). Items without a translation show the English name.
  Switching the language updates item names immediately.
- A name that is the same in a language (*Kris*, *Zen*) is written like the
  English one (`"de": "Kris"`), so the translations tool does not count it
  as missing.
- Logs always use the English name and the item id, e.g.
  `Short Sword (0,1)`, never a translation.

### MU Helper pickup filters

**Pick All** collects every eligible drop. **Selected** applies the enabled
categories and Extra Items; with neither mode selected, retained filters collect
nothing. Pick All takes precedence if an older save has both modes set, matching
the original client and OpenMU's offline leveling.

Extra Items uses case-sensitive substrings of the localized inventory-style name,
including names selected by level. For example, `Box of Luck` does not match
`Heart of love`, medals or Kundun boxes that share its item id. This is not always
the ground label: summoned creatures and spirits can use different word order or
names, ancient set prefixes are omitted, and money uses `Zen +amount`. A language
change may require updating saved keywords; option words follow the UI language
too. Ordinary equipment keeps contiguous keywords such as `+7+Skill` in English.
`+N` matches the displayed name's number, not necessarily the raw source level:
Kundun tiers use their displayed numbers, while seeds and some quest items omit
their source level. Jewel and stone bundles show their count instead (`x10` to
`x30`), so a keyword such as `Bless +2` no longer matches them; `Bless x20` does. Unmapped variant levels fall back to the base
name and `+level`.

A saved keyword holds **at most 14 encoded bytes** (14 single-byte characters,
fewer when characters need multiple bytes); longer or unencodable keywords are
discarded when saving and will be absent after relog. Use short substrings such as
`Heart`, `Kundun`, `Crest` or `+7+Skill`, rather than a long full name or combined
option string. The Zen category selects money; other enabled categories and extra
keywords remain independent.

OpenMU's offline leveling reads the same saved list but uses its own rules:
case-insensitive server item-definition names, without these localized level
variants or option words. An online keyword need not select the same items offline.

### Fields

| Field | Meaning |
|---|---|
| `tags` | Categories the game code asks for, e.g. `["mount", "flying"]`; see [Tags](#tags) |
| `levelValues` | Values of the description's own placeholders, one per item level from +0: `{"stat": ["ENG", "STA", ...]}` makes `{stat}` "ENG" at +0 and "STA" at +1. Each value is a text with its translations (written like `name`) or a shared text (`"@name"`), without placeholders. A line that shows one is shown only at levels that have a value. The Fruits' `{stat}` is one. A client-only field; the bmd import keeps it |
| `description` | Lines the tooltip shows for the item, e.g. the Jewel of Bless's "It is used to increase your item level up to 6"; see [Descriptions](#descriptions). A client-only field; the bmd import keeps it |
| `nameLines` | How the game names the item (its tooltip, the repair window, windows, messages and ground labels) and the tooltip's name color, when not the standard name and color (see [Name lines](#name-lines)). Each is written like a `description` line without `bold` and `after` (the name is always bold and first), its text is checked like one, and its `text` and `color` may be left out. Name lines are in the item names' language (English with the options' "Translate item names" off), and a text that is empty in it keeps the standard name. A client-only field; the bmd import keeps it |
| `width`, `height` | Size in inventory slots |
| `slot` | Equipment slot: `mainHand`, `offHand`, `helm`, `armor`, `pants`, `gloves`, `boots`, `wings`, `pet`, `pendant`, `ring`. Left out = not equippable |
| `wingTier` | Only for wings: `small`, `first`, `second`, `third`. The wing formulas (defense, defense per level, damage increase, absorption) depend on it; excellent second and third tier wings get no extra durability, and the chaos mix knows a wing's tier from it |
| `weaponKind` | Only for weapons and shields: `sword`, `glove`, `axe`, `mace`, `spear`, `bow`, `crossbow`, `staff`, `book`, `shield`, as OpenMU's item groups 0 to 6: swords and the Rage Fighter's gloves (group 0; gloves are the items only the Rage Fighter may use), axes (1), maces and sceptres (2), spears (3), bows and crossbows (4: bows have the slot `offHand`, crossbows `mainHand`; arrows and bolts have no weapon kind), staffs, sticks and books (5: books have the slot `offHand`) and shields (6). While an item has the weapon kind its group gives, the bmd import and the item editor (also when it moves an item to another number) keep it matching the group, the slot and, in group 0, the class requirements (a group gives its kind only on that kind's slot, and in group 4 only to two-handed items), so a new weapon gets one too; a kind written in the file that differs from the group's stays as long as it differs. Which bow or crossbow a character holds (its animations, arrows and damage), the elf's arrows-only rule, a spear's reach and Impale, the shield defense set option, the bonus of leveled arrows and bolts, a staff's damage (it counts for the left hand), which right-hand weapons a summoner may hold in the left hand (not staffs or books), the character window's wizardry line, the tooltip's magic power line, the knight skills that need a weapon without a staff or book, Death Stab, Death Cannon and Spiral Slash, the weapon in the left hand that adds its magic options, the dual wield of Dark Knights, Magic Gladiators and Rage Fighters, and the weapon that moves into an empty right hand depend on it |
| `potionPriority` | Only for potions with the `healingPotion`, `manaPotion`, `complexPotion` or `shieldPotion` tag: which one of a kind the game takes first, the higher the earlier (0 or left out = last; equal ones take the higher group and number first). The shipped potions count up with their size, so the largest is taken first. A client-only field (OpenMU has none); the bmd import keeps it |
| `statProfile` | The item's profile in the [item stat file](#item-stats-by-level): what its + level, excellent options, ancient set and option level add to its stats and requirements. `"default"` names the default. The [profile rules](#item-stats-by-level) of the item stat file say which profile an item's data gives it: in the shipped file `sceptre` for the items with the `sceptre` tag, `book` for summoner books, `shield` for shields, `weapon` for the other items with a weapon kind (not arrows and bolts), `armor` for helms, armour, pants, gloves and boots, `scroll` for the items of group 15 with requirements, `orb` for the items of group 12 with stat requirements (the orbs, crystals and the Dark Lord's scrolls), `helper` for the items of groups 13 and 14 with a base level requirement (`requirements.level`; the Transformation Ring's comes from `requirementsByLevel`, so it has the default), and a wing tier's profile for wings (`firstTierWing`, `secondTierWing`, `secondTierCape` for the second tier capes, `thirdTierWing`, `smallWing`). While an item has the profile its data gives, the bmd import and the item editor keep it matching (also when it moves an item to another number); a profile written in the file that differs from it stays as long as it differs: the Horn of Fenrir's data gives `helper`, and it names `"default"`. A left-out profile always follows the data; while the data gives one, the item has the default and loading warns about it, as for item files saved before the field. An item whose data stops giving its profile loses the profile's terms: a bow made one-handed, which is then no bow, loses the `weapon` profile's strength per option level |
| `twoHanded` | Weapon needs both hands |
| `skill` | Skill the item gives |
| `level` | Item level (the drop level); also used in price and damage calculations |
| `durability`, `magicDurability` | Durability at item level 0; an item with `magicDurability` (staffs, sticks, books) uses it as its maximum durability instead |
| `damageMin`, `damageMax`, `attackSpeed`, `walkSpeed`, `magicPower` | Weapon values. Damage and magic power go up to 65535 (later items pass 255); the [bmd exports](#import-from-bmd--export-as-bmd) take up to 255 |
| `excellentBonus` | A fixed excellent bonus of damage and magic power instead of the [item stat file's](#item-stats-by-level) excellent term; left out = the term. The Chaos Dragon Axe has 15, the Chaos Nature Bow 30 and the Chaos Lightning Staff 25. The bmd import keeps it |
| `excellentRaisesLevel` | `false`: excellent options do not raise the level requirement by the [item stat file's](#item-stats-by-level) `excellentLevelAdd`. Left out = they do. The shipped data has it on the Dragon Knight, Venom Mist, Sylphid Ray, Volcano, Sunlight and Storm Blitz armour, the Bone Blade, Explosion Blade, Soleil Scepter, Sylph Wind Bow, Grand Viper Staff and Storm Blitz Stick, the lucky items and the Horn of Fenrir. The bmd import keeps it |
| `defense`, `magicDefense`, `blockRate` | Armor and shield values |
| `attackType` | Attack type used by the client |
| `sellValue` | Value factor used by the price calculation of some items |
| `buyPrice` | Fixed price; when set, it replaces the calculated price |
| `requirements` | `level`, `strength`, `dexterity`, `energy`, `vitality`, `leadership`. The stat requirements of the orbs, crystals and scrolls (groups 12 and 15, without a slot) are the values the game asks for, as OpenMU stores them: their profiles keep them as they are. The original client's bmd files hold weights for them, which Import from bmd turns into the values and Export as bmd and Export as S6E3 turn back (the smallest weight that gives each value). Which ones convert follows the item's profile: a requirement it keeps as it is while the default grows it (an orb given the default profile holds weights like any item) |
| `requirementsByLevel` | A requirement for each item level 0 to 15 (one number for all of them, or 16), instead of the [item stat file's](#item-stats-by-level) formula; above +15 the +15 value counts. The Orb of Summoning has `{"energy": [30, 60, 90, 130, 170, 210, 300, 500, 0, ...]}` and the Transformation Ring `{"level": [20, 20, 20, 50, ...]}`. The base value of that requirement is not used. The bmd import keeps it |
| `classRequirements` | Per class (`darkWizard`, `darkKnight`, `fairyElf`, `magicGladiator`, `darkLord`, `summoner`, `rageFighter`): 0 = cannot use, otherwise the class level needed |
| `resistances` | `ice`, `poison`, `lightning`, `fire`, `earth`, `wind`, `water` |
| `tradable`, `droppable`, `storable`, `sellable`, `personalShopSellable`, `repairable` | What a player may do with the item; see [Rules](#rules). Only `false` is written |
| `droppableWhileRented`, `personalShopSellableWhileRented`, `sellableWhenRentalExpired` | What changes for a rented item; see [Rules](#rules) |

### Tags

A tag puts an item into a category that the game code asks for. An item
can have several tags; the file lists them in the order below.

| Tag | Meaning |
|---|---|
| `cape` | A cape (Cape of Lord, Cape of Fighter, …); second tier capes have their own wing formulas, and their defense set option counts |
| `mount` | Can be ridden (Uniria, Dinorant, Dark Horse, Fenrir) |
| `hornMount` | A mount summoned by a horn (Uniria, Dinorant, Fenrir) |
| `flying` | A mount that can fly (Dinorant, Dark Horse, Fenrir), needed for Icarus; see [Flight equipment](#flight-equipment) |
| `darkLordPet` | Dark Horse and Dark Raven |
| `guardianPet` | Demon and Spirit of Guardian |
| `pandaOrSkeleton` | The panda and skeleton pets and transformation rings |
| `transformationRing` | Rings that change the character's look: a second one cannot be dragged onto a ring slot, the Kanturu entry and `/move` to Icarus refuse them, and the move window offers no Icarus with one that is not the panda or skeleton ring |
| `jewel` | Jewels of Bless, Soul, Life, Chaos, Creation and Guardian |
| `refineStone` | Lower and Higher Refine Stone |
| `socketSeed`, `socketSphere`, `socketSeedSphere` | Socket system items |
| `healingPotion`, `manaPotion`, `complexPotion`, `shieldPotion` | Potions the hotkeys look for; a skill without enough mana takes a mana potion and the MU Helper a healing potion. They take the one with the highest `potionPriority` first, then the highest group and number |
| `elitePotion`, `elixir`, `buffScroll`, `battleOrStrengthScroll` | Cash shop potions and scrolls |
| `ammunition` | Arrows and bolts |
| `bloodCastleTicketPart` | Scroll of Archangel and Blood Bone |
| `secondClassQuestItem`, `thirdClassQuestItem` | Class change quest items |
| `summonerBook` | Summoner books |
| `divineArchangelWeapon` | The Divine weapons of the Archangel |
| `sceptre` | The Dark Lord's sceptres |
| `magicSword` | The Magic Gladiator's swords with magic power |
| `luckyItem` | The lucky set armour (numbers 62 to 72 of groups 7 to 11): no repair cost and no price, a maximum durability of 255, no stat requirements in the tooltip (the shipped ones also have `excellentRaisesLevel: false`); only Jerridon's mix and the lucky item window's refinery take them, and of the jewels only the Jewels of Extension and Elevation act on them |
| `cashShop` | Items from the cash shop. Only information for now (for the editors and the OpenMU exchange); no client code reads it |
| `gambleItem` | Gamble items |
| `gemJewelry` | The gem rings and necklaces from the cash shop |
| `luckyItemTicket` | Lucky item tickets |
| `valuable` | The game asks for confirmation before the item is sold or dropped |
| `noLevelRequirement` | The tooltip shows no level requirement, though the item has one (Loch's Feather) |
| `noDurabilityLine` | The tooltip shows no standard durability line, though the item has a durability: it shows none, or its description has its own (a pet's "Life", a stack's "Number of items", a ticket's "usable … times") |
| `noBlankAfterOptions` | The tooltip leaves out the blank line after the options, so the item's lines drawn there follow the options directly (the Pet Skeleton, the Invitation to Santa Village) |

Wings need no tag: an item is a wing when its `slot` is `wings`.

### Tooltip layouts

`Tooltips/Tooltip.json` says which lines an item tooltip shows besides the
item's own description: damage, defense, magic defense, defense rate, the
speeds, the resistances, the requirements with their "(lacking …)" lines, and
the wizardry, curse and pet attack increases. It has the **shared texts**, by
name and with their translations, and the **layouts**. Every item uses the
layout `default` for now.

```json
{
  "formatVersion": 1,
  "texts": {
    "defense": {"en": "Defense: {defense}", "de": "Verteidigung: {defense}"},
    "lacking": {"en": "(lacking {missing})", "de": "(fehlt {missing})"}
  },
  "layouts": [
    {
      "name": "default",
      "lines": [
        {"section": "name"},
        {"text": "@defense", "when": ["defense"], "colors": [{"when": ["defenseHarmonyBonus"], "color": "yellow"}, {"when": ["armour", "excellent"], "color": "blue"}]},
        {"section": "description"},
        {"text": "@durability", "when": ["hasDurability", "!timed", "!tag:noDurabilityLine"]},
        {"text": "@requiredLevel", "when": ["requiredLevel", "!tag:noLevelRequirement"], "colors": [{"when": ["missingLevel"], "color": "red"}]},
        {"text": "@lacking", "values": {"missing": "missingLevel"}, "when": ["requiredLevel", "!tag:noLevelRequirement", "missingLevel"], "color": "red"},
        {"section": "classes"},
        {"text": "", "when": ["tag:sceptre"]},
        {"text": "@petAttackIncrease", "when": ["tag:sceptre"], "color": "blue", "bold": true},
        {"section": "options"}
      ]
    }
  ]
}
```

A layout line has:

- **`text`**: `"@name"` for a shared text, a text written in place (with its
  translations, as in a description), or `""` for a blank line. Its
  placeholders are the [built-in values](#description-placeholders) and those
  the line's `values` names (the stat file's tables, which descriptions can
  use, aren't checked here, so a layout can't use them yet).
- **`when`**: the line shows only when every entry holds. An entry is
  - a built-in value, which holds when it is not 0 (`"defense"`,
    `"missingStrength"`), or a built-in value compared with a whole number by
    `=`, `!=`, `<`, `<=`, `>` or `>=` (`"durability=2"`, `"level!=1"`); the
    number is an `int`, from -2147483648 to 2147483647, with an optional `-`;
  - a state (below);
  - `"tag:<tag>"`, which holds when the item has that [tag](#tags);
  - `"class:<class>"`, which holds when the hero is of that class (written as in
    `classRequirements`: `darkLord`, `summoner`, …), whatever its rank. A line
    that says which class can use the item is white for it and dark red for
    the others, as the class lines the game writes.

  A `!` in front turns an entry around: `"!tag:luckyItem"`. Without `when`
  the line always shows.
- **`color`**: the line's color, white when left out. The color names are
  those of a description line.
- **`colors`**: rules `{"when": [...], "color": ...}`. The first rule whose
  `when` holds gives the color, else `color` does.
- **`bold`**: `true` for a bold line.
- **`values`**: a placeholder of the text and the built-in value it shows on
  this line. One "(lacking {missing})" serves every requirement this way.

The **states** a condition can name:

| State | Set when |
|---|---|
| `hasDamage` | The item has damage above 0 to show; for a skill book, its skill's |
| `skillBook` | A skill book or scroll (group 15): its damage line is wizardry damage |
| `twoHanded` | A two-handed weapon |
| `excellent`, `ancient` | An excellent or ancient item |
| `damageHarmonyBonus`, `defenseHarmonyBonus` | A Jewel of Harmony option raises its damage or defense |
| `strengthReduced`, `dexterityReduced` | A Jewel of Harmony option or a socket lowers that requirement |
| `strengthRequired`, `dexterityRequired` | The item asks for that stat at all (its own requirement, before the reductions): a reduction to 0 still shows the line, at 0 |
| `armour` | A helm, armor, pants, gloves or boots (groups 7 to 11) |
| `wizardryWeapon` | A staff, a book, or a Magic Gladiator's sword with magic power |
| `hasDurability` | The item keeps a durability: an item of groups 0 to 11 or 13 with a `durability` or `magicDurability`, or the wings and capes 12/0–6, 12/36–43 and 12/49–50 |
| `timed` | A timed item, one that expires |
| `hasOptions` | The item has options to show: a skill, luck, an additional or an excellent option |

A **section** is a part of the tooltip the code still writes, and the lines
of a layout go between them:

| Section | What the code writes there |
|---|---|
| `name` | Everything up to the item's name, and the name |
| `description` | The item's own texts: the code's texts for particular items, the item's [description](#fields) |
| `classes` | The classes that can use the item, and the boots' and gloves' texts from +5 |
| `options` | The excellent, set, socket, Jewel of Harmony and other options, to the end of the tooltip |

Each layout has the four sections once each, in this order: it starts with
`name` and ends with `options`.

**Errors** when the item data loads: a `@name` that is not a shared text (in a
layout or a description), an entry of `when` that is neither a value, a
comparison of a value with a whole number, a state nor a known tag, an unknown
color, a placeholder that is neither built in nor named by the line's
`values`, a shared text of a description that names a table itself (it names one only through the line's
`values`, which the table checks and the editor's table counts and rename read), a translation whose placeholders
are not those of the English text,
a text (other than a blank line) without its English text, a language code
that has more than letters, digits and `-`, a `values` entry that names no
built-in value, missing or misplaced sections, and no layout `default`. The
messages suggest the name meant for a typo. **Warnings**: a shared text that
neither a layout nor a description uses, a `values` entry the text does not
have (a layout's or a description's line), a translation in a language the game has no texts in (as for item
names), and a layout other than `default` (nothing uses it yet).

### Flight equipment

Wings (capes included) and mounts with the `flying` tag let a character fly.
An item at 0 durability does not count. Icarus requires flight equipment:
without it the character cannot warp or move there, and in Icarus the last
flight equipment cannot be taken off, neither by dragging it nor by
right-clicking it.

The Dark Horse flies, as in the original client. OpenMU does not give it
`CanFly` yet ([MUnique/OpenMU#982](https://github.com/MUnique/OpenMU/issues/982)),
so until that is fixed the server refuses a Dark Lord who only rides a Dark
Horse into Icarus. Like OpenMU, a broken item gives no power-ups. The client
check only spares the player the fall; the server has to refuse the move as
well.

A tag name that does not exist is an error, so a typo cannot silently
remove an item from a category. New tags need code that uses them; they
are added in `ItemTag` (`ItemTagSet.h`) and `ItemEnumNames.cpp`.

### Descriptions

`description` lists the lines the item's tooltip shows, for example the
Jewel of Bless's:

```json
"description": [
  {"text": {"en": "It is used to increase your item level up to 6", "de": "Wird verwendet, um das Gegenstandslevel auf bis zu 6 zu erhöhen"}}
]
```

Each line is `{"text": ..., "values": {...}, "when": [...], "color": ...,
"colors": [...], "bold": true, "levels": [first, last], "after":
"options"}`:

- **`text`** is written like `name`: a plain English text, or the texts by
  language. It is checked like a name too: the English text is required, a
  language code has only letters, digits and `-`, and a language the game
  has no texts in is a warning. It can also be `"@name"`, a shared text of
  the [tooltip file](#tooltip-layouts) (`"@increaseSpeed"`), which is how a
  text several items show is written once. A text can have
  [placeholders](#description-placeholders).
- **`values`** names the value a placeholder of the text shows on this line:
  a built-in value, a table as `table:` and its name (`"values":
  {"percent": "table:wingDamageIncrease1st"}` with the shared text "Increase
  {percent}% of Damage"), or a [game data value](#description-placeholders)
  such as `skillMana:<skill>`, the mana a skill costs, by its number as `skill`
  writes it (`"values": {"mana": "skillMana:22"}`).
- **An empty text** (`{"text": ""}`) is a blank line.
- **Long lines** wrap at about the width of the original tooltips' widest
  lines, and `\n` in a text breaks the line there (`\n\n` leaves a blank
  line). A line's text is shown up to 511 characters (with its placeholders
  filled, before it wraps), a name line's up to 255; a text written longer
  is a warning, and the tooltip cuts it there.
- **`color`** is one of the tooltip colors: `white` (left out), `blue`,
  `red`, `yellow`, `green`, `darkRed`, `purple`, `darkBlue`, `darkYellow`,
  `greenBlue`, `gray`, `redPurple`, `violet`, `orange`.
- **`levels`** shows the line only at those item levels, both included (`[1,
  8]`; left out: every level): a list of two levels from 0 to 15, the first
  not above the last. Most items that use it today are level variants (the
  Fruits, Contract Summon, Life Stone, Rena, …), which the items design makes
  items of their own later: their lines then move into those items and lose
  their `levels`.
- **`when`** shows the line only when its [conditions](#tooltip-layouts)
  hold, and **`colors`** gives it the color of its first rule that holds, as
  a layout line's (`["!timed"]`, `["durability=2"]`, `[{"when":
  ["class:darkLord"], "color": "white"}]`). **`bold`** draws it bold.
- **`"after": "options"`** draws the line after the item's options
  (excellent, luck, skill), at the end of the tooltip, instead of with the
  description: the transformation rings' effects and "Unable to Equip with a
  Different Transformation Ring", for example.

In the tooltip:

- **Place:** after the item's damage, defense and speed lines, and before
  its durability, requirements and options. A few lines that read the hero
  or other tables (the Armor of Guardsman's Chaos Castle table, the Lost
  Map, the Talisman of Mobility's saved location) are still written by the
  tooltip code, after the description, and a few before it (the reset
  fruits' reset points, the Talisman of Resurrection's saved place, the
  socket seeds and spheres, the Sign of Dimensions).
- **Language:** the UI's language, also when the item names stay English
  ("Translate item names" off).
- **Size:** at most 32 rows. The description has them to itself, so a long
  one never takes the room of the tooltip's other lines.
- **Dark Horse and Dark Raven:** they show it only in shops; elsewhere the
  pet window writes their tooltip.

### Name lines

The tooltip's first line is the item's name. Its **standard name** is the
item's `name` in the item names' language (the UI's, or English with the
options' "Translate item names" off), after its set's name for an ancient item
and "Excellent" for an excellent one, with " +level" above +0; its
**standard color** is green-blue for an ancient item, violet for a socket
item, green for an excellent item with options, yellow from +7, blue for an
item with options, else white.

`nameLines` changes them: the first line whose `levels` and `when` hold gives
the name, its `text` with its [placeholders](#description-placeholders) filled
in (left out: the standard name), and the color, its `color` (left out: the
standard color) and then its first `colors` rule that holds. Without such a
line, the standard name and color. The NPC shop's price line has the name's
color.

The repair window writes the same name in the same color. Elsewhere the name
lines give only the text: the mix and quest windows, the server's messages,
the control socket and the ground labels (which keep their own colors) write
the line's text, and where no line holds, the item's `name` with " +level"
above +0 (the ground labels add the set's name and the options themselves).
There may be no item there, only its type and level: a condition on the
item's durability or options then reads 0 (the Horn of Fenrir is plain
"Horn of Fenrir").

```json
"nameLines": [
  {"text": "{name}", "color": "white", "levels": [0, 0], "colors": [{"when": ["level>=7"], "color": "yellow"}, {"when": ["hasOptions"], "color": "blue"}]},
  {"text": "{name} +{level}", "color": "white", "colors": [{"when": ["level>=7"], "color": "yellow"}, {"when": ["hasOptions"], "color": "blue"}]}
]
```

are the wings' and capes': no "Excellent", yellow from +7, else blue with
options. `[{"color": "yellow"}]` keeps the standard name in yellow (the
jewels); `{"text": "{monster} Transformation Ring", "values": {"monster":
"monsterName:2"}, "levels": [0, 0]}` names a +0 Transformation Ring after the
Budge Dragon.

### Description placeholders

A description or name text can show values of the item: `{value}` is replaced
by that value for the hovered item, e.g.
`"Increases damage by {table:damageIncreaseWeapons} at +{level}"`. Item texts
have no braces of their own, so every brace belongs to a placeholder. A
placeholder is one of:

- a **built-in value**, by its name: `{level}`, the item's + level, or a value of the tooltip's stat
  lines, as the game computes them for the item and the hero:
  `damageMin`, `damageMax` (with the Jewel of Harmony's attack bonus; a skill
  book's wizardry damage), `defense` (with its defense bonus), `magicDefense`,
  `blockRate`, `attackSpeed`, `walkSpeed`, `magicPower`, `durability`,
  `maxDurability`, the resistances `iceResistance`, `poisonResistance`,
  `lightningResistance`, `fireResistance`, `earthResistance`, `windResistance`,
  `waterResistance` (the item's level + 1 for a resistance it has), and for
  `Level`, `Strength`, `Dexterity`, `Vitality`, `Energy` and `Leadership`
  `required…` (what the item asks, after the Jewel of Harmony's and the sockets'
  strength and agility reductions) and `missing…` (what the hero lacks of it, 0
  when nothing), and the item's add option (the potions', scrolls', seals' and
  charms' effect in the game's ItemAddOption data): `addOptionValue1`,
  `addOptionValue2` and `addOptionDuration`, a text such as "1 Hour"; the
  hero's `heroLevel`; and for the [name lines](#name-lines) the item's `name`
  (its `name` in the item names' language), `levelPlusOne` (its level + 1),
  `bundleCount` (how many jewels or stones a bundle holds, as the game counts
  them; 0 for other items) and `excellentFlags` (its excellent options, 0 to 63; the Horn of Fenrir's
  1, 2 and 4 are its Destroy, Protect and Illusion kinds);
- a **level bonus table** of [`Stats/ItemStats.json`](#item-stats-by-level),
  as `table:` and its name: the table's value at the item's + level
  (`{table:damageIncreaseWeapons}` is 3 at +1 and 66 at +15, the damage a
  weapon gains). A new table is a new placeholder.
  The wings, capes and pets show their damage increase and absorption this
  way: the shared text `"Increase {percent}% of Damage"` with
  `"values": {"percent": "table:wingDamageIncrease1st"}` on the line, over the
  table `wingDamageIncrease1st` (`{"values": 12, "perLevel": 2}`, 12 + 2 per
  level). Such tables are only for descriptions: no stat profile names them.
- a name the line's `values` binds to a built-in value, a table
  (`"table:<name>"`) or a value of other game data, so one shared text serves
  items whose numbers come from different tables; the game data values are
  `skillMana:<skill>` (the mana a skill costs), `skillName:<skill>` (its name),
  by its number as `skill` writes it, and `monsterName:<monster>` (a monster's
  name, by its number in the monster data);
- one of the item's `levelValues`, a text by item level.

The tables are written apart from the built-in values, so a table may have
any name, also one a later version gives a built-in value: `{level}` and
`{table:level}` are not the same. The names of a line's `values` and the item's
`levelValues` are the item's own: one named like a built-in value is the
item's, on its lines.

**Errors** (when the item data loads, in the save checks and in Import from
bmd):

- a `{` without its `}`, an empty `{}`, a `}` without its `{`, or a name that is
  not letters and digits (after `table:` for a table), in any language;
- a translation whose placeholders are not those of the English text (another
  order is fine);
- a name that is neither built in nor a table: `has the unknown placeholder
  {table:damgeIncreaseWeapons}; did you mean {table:damageIncreaseWeapons}?`.
  A table written without `table:` gets the same hint;
- a line's `values` entry that names neither a built-in value nor a table;
- a level value with placeholders, and a line that shows a level value at a
  level the value has none for.

A level value no line shows is a warning.

**Warnings:**

- a table of the stat file that neither a stat nor a description names;
- a wing (an item with a `wingTier`) whose description lines are not those of
  most of the other wings of its tier, `cape` tag and group: the tooltip code
  chose the wings' lines by the tier, so a new wing without them, or one moved
  to another tier that keeps its old tier's tables, is pointed out
  (`its wing lines are not those of the other third-tier wings of group 12:
  it has 1 line naming no table, they have 3 lines naming …`). A wing alone
  there is compared with the wings of its tier and `cape` tag in the other
  groups that have more than one; Cape of Fighter (12) and Cape of Lord (13),
  each alone and written apart by the old code, are not compared with each
  other. Wings of a group that split evenly get one warning that they don't
  agree, without blaming either side. Wings without lines never set the
  lines the others are compared with, so new wings imported without them are
  pointed out, also when they are more than the wings with lines.

The shipped data is checked by the tests, so a wrong placeholder fails the
build's tests.

### Rules

`tradable`, `droppable`, `storable`, `sellable` (to an NPC),
`personalShopSellable` and `repairable` say what a player may do with the
item. They are `true` unless the file says `false`.

Rented items (items with a rental time) can never be stored. For the rest:

| Field | Default | Meaning |
|---|---|---|
| `droppableWhileRented` | `true` | `false`: the item cannot be dropped while it is rented |
| `personalShopSellableWhileRented` | `true` | `false`: the item cannot be sold in a personal shop while it is rented |
| `sellableWhenRentalExpired` | `false` | `true`: the item can be sold to an NPC once its rental time ran out, even when `sellable` is `false` |

An item the client does not know (no definition for its group and number)
allows no action.

A few exceptions depend on the item level, the durability or the player
and are in the code (`GameLogic/Items/TradeRestrictions.cpp`,
`ShopRestrictions.cpp`):

- **Item level:** some items are a different item at each level. Rena +3
  (Sign of Lord) can be traded, stored and sold in a personal shop; Box
  of Luck +13 (Heart of Dark Lord) cannot. Rena +1 and Remedy of Love +1
  to +5 cannot be sold to an NPC. The Wizard's Ring has level exceptions
  too, but its flags already block trading, storing and selling at every
  level, because the server binds it to the character. A later phase makes
  these level variants items of their own, with their own flags.
- **Durability:** a Talisman of Mobility with durability 1 cannot be stored.
- **GM Gift:** only a game master can trade it.

These checks only decide what the client allows. The server has its own
checks; the design document lists how the flags map to OpenMU.

## Item stats by level

`Stats/ItemStats.json` says what an item's + level, excellent options and
ancient set add to its damage, magic power, defense rate (`blockRate`),
defense and magic defense, and how its requirements grow. It is in a folder of its own because every
`*.json` file in `Data/Items` is read as a group file. An excerpt:

```json
{
  "formatVersion": 1,
  "dropLevelBonus": {"excellent": 25, "ancient": 30},
  "tables": [
    {
      "name": "damageIncreaseWeapons",
      "caption": {"en": "Damage Increase (Weapons)"},
      "values": [0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 31, 36, 42, 49, 57, 66]
    }
  ],
  "default": {
    "damage": {"table": "damageIncreaseWeapons", "excellent": {"times": 25, "add": 5}, "ancient": {"add": 5, "dropLevelDivisor": 40}},
    "magicDefense": {"table": "magicDefenseIncrease", "excellent": "none", "ancient": "none"},
    "requirements": {
      "levelPerItemLevel": 0,
      "excellentLevelAdd": 20,
      "strength": {"add": 20, "levelWeight": 3, "percent": 3},
      "leadership": "base",
      "strengthPerOptionLevel": 0
    }
  },
  "profiles": [
    {
      "name": "sceptre",
      "magicPower": {"perLevelAfterHalving": 0}
    }
  ],
  "profileRules": [
    {"profile": "default", "tags": ["ammunition"]},
    {"profile": "sceptre", "tags": ["sceptre"]}
  ]
}
```

- **Tables** (`tables`) hold what the + level adds: `values` has one value
  for each item level 0 to 15 (or one number for all of them), and
  `perLevel` adds that much for every level. Above +15 the +15 value of
  `values` counts, while `perLevel` keeps growing. The names follow OpenMU's
  level bonus tables (`ItemLevelBonusTable`) where OpenMU has the table:
  OpenMU's first and second tier wings share one, which the client splits
  into `defenseBonus1stWings` and `defenseBonus2ndWings`, and
  `magicPowerIncrease` and `magicDefenseIncrease` are the client's own.
- **The default** (`default`) writes the terms of every stat:
  - `table`: its level bonus table;
  - `excellent`: added to excellent and ancient items, as
    `base * times / drop level + add + drop level / dropLevelDivisor`, where
    the drop level is the item's `level`. Damage takes the minimum damage
    as the base of both damage values;
  - `ancient`: added to ancient items, as
    `value * times / d + add + d / dropLevelDivisor`, where `d` is the drop
    level plus `dropLevelBonus.ancient` and the value is the base plus the
    excellent part (with `afterLevelBonus` also the level bonus);
  - `perLevelAfterHalving`, magic power only: the magic power is halved
    after everything is added, then this is added for every level.

  `"none"` means the stat has no such term. A number left out of a term is 0;
  a left-out `dropLevelDivisor` leaves its part out.
- **Requirements** (`requirements` in the default and the profiles):
  - `levelPerItemLevel`: the level requirement grows by this much per item
    level (an item without a level requirement keeps none);
  - `excellentLevelAdd`: an item with excellent options needs this much more
    level, if it has a level requirement, is not a wing and does not have
    `excellentRaisesLevel: false`;
  - `strength`, `dexterity`, `energy`, `vitality`, `leadership`: each
    `add + base * (drop level + item level * levelWeight) * percent / 100`,
    where the drop level is the item's `level` plus `dropLevelBonus.excellent`
    for an item with excellent options, else plus `dropLevelBonus.ancient`
    for an ancient item; `"base"` keeps the base value, and `"basis":
    "levelRequirement"` takes the item's level requirement (its base value,
    or the one `requirementsByLevel` gives at the item's level) instead of
    the drop level (without the item level) when it has one. A requirement the item
    does not have stays 0;
  - `strengthPerOptionLevel`: the strength requirement grows by this much per
    option level (also when the item has no strength requirement).
- **Profiles** (`profiles`) write only what differs from the default. A term
  a profile writes replaces the default's as a whole. An item names its
  profile with `statProfile`; an item without one has the default.
- **Profile rules** (`profileRules`) say which profile an item's data gives
  it, in order: the first rule the item matches names its `profile`
  (`"default"` for the default), and an item no rule matches has the default.
  A rule's conditions are lists of values: `group` (0 to 15), `slot`,
  `wingTier`, `weaponKind`, `tags`, and `requirements` (the keys of an item's
  `requirements`: its base value is above 0; a requirement given by level
  does not count). An item meets a condition when it has one of its values,
  and a rule without conditions matches every item. The writer keeps the
  rules in order, one per line. A new condition needs a new `formatVersion`:
  a reader refuses conditions and values it does not know.
- Every `/` is a whole-number division, from left to right.
- What stays code: the order of the terms; which items count as excellent
  (excellent options on wings are wing options, and ancient items get the
  excellent part too); that a stat the item does not have stays 0; that the
  excellent part needs a drop level above 0; and that defense rate stops at
  255.

The shipped profiles:

| Profile | Differs from the default |
|---|---|
| `weapon` | 5 strength per option level: the other weapons (staffs and the Magic Gladiator's swords included) |
| `armor` | 5 strength per option level: helms, armour, pants, gloves and boots |
| `book` | Energy `20 + base * (drop level + item level) * 3 / 100`; 5 strength per option level: summoner books |
| `helper` | Level requirement +4 per level: the items of groups 13 and 14 with a base level requirement (in the shipped data the rings, pendants, pets and Loch's Feather of group 13); the Horn of Fenrir names `"default"` |
| `scroll` | Level requirement +4 per level; the stat requirements as stored (`"base"`): the skill scrolls and parchments of group 15 |
| `orb` | The stat requirements as stored (`"base"`): the orbs, crystals and the Dark Lord's scrolls of group 12 |
| `sceptre` | No magic power per level after halving; 5 strength per option level: the Dark Lord's sceptres |
| `shield` | Defense +1 per level (`defenseIncreaseShields`), no excellent part, ancient part `(defense + level) * 20 / d + 2` |
| `firstTierWing` | Defense from `defenseBonus1stWings`; level requirement +4 per level |
| `smallWing` | Defense from `defenseBonus1stWings` |
| `secondTierWing` | Defense from `defenseBonus2ndWings` (2 per level up to +9, then 4 to 9 more per level); level requirement +5 per level |
| `secondTierCape` | Defense from `defenseBonus2ndWings`; level requirement +4 per level |
| `thirdTierWing` | Defense from `defenseBonus3rdWings`: 4 per level up to +9, then 5 to 10 more per level |

The shield also adds 5 strength per option level. Items without a profile
or with `"default"` (arrows and bolts, jewels, potions, the Horn of Fenrir,
…) have the default: their level requirement does not grow.

The shipped profile rules, in order, with the items each gives its profile:

| # | Profile | Conditions | Items |
|---|---|---|---|
| 1 | `default` | tags `ammunition` (arrows and bolts) | 2 |
| 2 | `sceptre` | tags `sceptre` | 10 |
| 3 | `book` | tags `summonerBook` | 3 |
| 4 | `shield` | weapon kind `shield` | 22 |
| 5 | `weapon` | every other weapon kind | 113 |
| 6 | `armor` | slot `helm`, `armor`, `pants`, `gloves` or `boots` | 327 |
| 7 | `smallWing` | slot `wings`, wing tier `small` | 6 |
| 8 | `firstTierWing` | slot `wings`, wing tier `first` | 4 |
| 9 | `secondTierCape` | slot `wings`, wing tier `second`, tags `cape` | 2 |
| 10 | `secondTierWing` | slot `wings`, wing tier `second` | 5 |
| 11 | `thirdTierWing` | slot `wings`, wing tier `third` | 7 |
| 12 | `default` | slot `wings` (wings without a tier) | 0 |
| 13 | `orb` | group 12, a base stat requirement | 12 |
| 14 | `scroll` | group 15, any base requirement | 36 |
| 15 | `helper` | groups 13 and 14, a base level requirement | 32 |

The other 330 items match no rule and have the default. Rule 15 reaches the
Horn of Fenrir, which names `"default"`.

**Errors** in the file stop the start like errors in the item files: a file
that cannot be read, invalid JSON, a top level that is not an object, a
missing or unsupported `formatVersion`; a missing `dropLevelBonus` (with
`excellent` and `ancient`), `tables` or `default`, or one that is not an
object (`tables` a list of objects); a stat section or `requirements` that
is not an object; a missing name, or one that is not letters and digits, is
defined twice or is a profile named `default`; a table without `values` or `perLevel`; `values` that are
not one number or 16 numbers; a number that is not a whole number in its
range (`dropLevelBonus` 0 to 1000, `perLevelAfterHalving` and
`strengthPerOptionLevel` 0 to 255, `dropLevelDivisor` 1 to 65535, all other
numbers 0 to 65535); a default without a stat, a term or a requirement term;
a `table` the file does not have; a term that is neither an object nor
`"none"`; an `afterLevelBonus` that is not true or false; a requirement
formula that is neither an object nor `"base"`; a `basis` other than
`"levelRequirement"`; a caption that is neither a text nor texts by
language; `profiles` that are not a list of objects; `profileRules` that are
not a list of objects; a rule without a `profile`, or one naming a profile
the file does not have (other than `"default"`); a condition the format does
not have; and a condition that is not a list of one or more values, or a
value that is not one of its names (for `group`, a whole number from 0 to
15).
**Warnings:** unknown fields, a table that neither a stat nor a
[description placeholder](#description-placeholders) uses (see there for the
wings' tables too), and a caption without English. An item whose `statProfile` the file does not have is an error, and
a profile no item names is a warning.

---

## Loading and errors

At startup all `*.json` files in `Data/Items` and `Stats/ItemStats.json` are
read and checked.

**Errors** stop the start with a message that names the file, the item and
the field (**Copy text** copies it). All problems are also written to
`MuError.log`. Errors are:

- invalid JSON, a missing or unsupported `formatVersion`, an invalid `group`
- an item without `number` or `name`, a number outside 0–511
- a value that does not fit its field (e.g. `width: 256`) or has the wrong
  type
- an item defined more than once
- an item without an English name
- a name containing `||`
- a name that is not a text, e.g. `"pt": 5`
- a language code that is empty or has other characters than letters,
  digits and `-` (e.g. `"p=t"`)
- a `slot`, `wingTier`, `weaponKind` or tag name that does not exist
- a `statProfile` that is not a name of letters and digits, or that the
  item stat file does not have
- a `requirementsByLevel` entry that is not a requirement or not one number
  or 16 numbers from 0 to 65535

**Warnings** are logged, and the game starts anyway:

- unknown fields (they are ignored)
- names longer than 49 characters (the game shows them cut)
- a `wingTier` on an item whose slot is not `wings`
- a `weaponKind` on an item that is not in the slot of its kind: bows,
  shields and books need `offHand`, the other kinds `mainHand`
- no `weaponKind` on an item whose group and slot give one, as in item files
  saved before the kind existed for that group: the game then treats the
  item as no weapon of that kind
- a `potionPriority` on an item without a `healingPotion`, `manaPotion`,
  `complexPotion` or `shieldPotion` tag (nothing reads it there)
- an `excellentBonus` on an item without damage or magic power, or whose
  stat profile has no excellent term for its damage or magic power (the
  bonus takes the term's place, so it does not count there)
- `excellentRaisesLevel: false` on an item without a level requirement
- a `requirementsByLevel` entry next to a base value of the same requirement
- a name in a language the game has no UI texts for (it is never shown)
- no `statProfile` on an item the profile rules give one, as in item files
  saved before the field: the item has the default until an edit or a bmd
  import gives it the profile (`"default"` keeps the default without a
  warning)
- a `slot` written as a number, as files saved before tags existed have
  it; the number is read, and saving writes the name

An automated test loads the shipped item data, so a pull request with
broken item data fails its checks.

## Item models

Which 3D model an item shows is set in `src/bin/Data/Items/Models/`, one
file per item group with the same file names as the item files, and
`SharedModels.json` with the models that several items use (see
[Shared models](#shared-models)). They are separate from the item files
because they only matter to the client; the item files hold what client
and server share.

```json
{
  "formatVersion": 1,
  "group": 13,
  "models": [
    {
      "number": 4,
      "file": "Data/Item/DarkHorseHorn.bmd",
      "textureFolders": ["Item", "Skill"],
      "inventory": {
        "rotation": [-90, -90, 0],
        "scale": 0.0015
      }
    }
  ]
}
```

| Field | Meaning |
|---|---|
| `number` | The item number (0–511) in the file's group. |
| `file` | The `.bmd` model, relative to the game folder, with `/` between folders. |
| `model` | The name of a shared model, instead of `file`, `textureFolders` and `noneBlendMeshes`; see [Shared models](#shared-models). |
| `textureFolders` | Folders below `Data/` with the model's textures. Each texture is taken from the **first** folder that has it. Without folders the model has no textures. |
| `noneBlendMeshes` | Mesh numbers (from 0) that are drawn without blending. Optional. |
| `inventory` | How the item is drawn in the inventory, see below. Optional. |
| `ground` | How the item lies on the ground, see below. Optional. |
| `glow` | How the item glows, see below. Optional. |
| `renderStyle` | The look of the model when it is more than a plain textured model, see below. Optional. |
| `itemEffect` | What the model does before it is drawn (sprites and particles on its bones, a pulsing glow mesh, ...), see below. Optional. |
| `cloth` | `true` for capes that are worn as cloth: when one is put on or taken off, the character's cloth is deleted, so the next cape builds its own. Optional. The flag does not make a cape cloth; which capes are drawn as cloth, and how, is still decided in code. |
| `material` | `{ "specular": 0.5 }`: the strength of the highlight under Enhanced lighting, 0 (none) to 1; see [enhanced-look.md](enhanced-look.md). Optional. |

`inventory` and `ground` hold these values; a missing value has the
default, which is the look of items without values of their own:

| Value | Meaning | Default |
|---|---|---|
| `inventory.anchor` | Where the model sits in its slot, as a share of the slot width and height from the top left corner. | `[0.5, 0.6]` |
| `inventory.offset` | Moves the model from there: `[x, y]`, or `[x, y, z]` to also move it in depth. | `[0, 0]` |
| `inventory.rotation` | Degrees around x, y and z. | `[270, -10, 0]` |
| `inventory.scale` | Size of the model. | `0.0025` |
| `inventory.bodyHeight`, `ground.bodyHeight` | For armor, which is drawn on the character skeleton: how far down it sits (e.g. `-160` for helms). | `0` |
| `ground.rotation` | Degrees around x, y and z. | `[0, 0, -45]` |
| `ground.scale` | Size of the model on the ground. Without it the item keeps the size all dropped items have. | none |

In the inventory every item turns while the mouse is on it, and gamble
items (tag `gambleItem`) turn slowly all the time.

The Rage Fighter armors (8,59), (8,60), (8,61) and (8,73) are drawn in the
inventory with models of their own (`MODEL_ARMORINVEN_*`), with the
`inventory` values of their entry.

`glow` holds how the item glows. Which glow an item gets for its level,
excellent options or ancient set is decided by the game; these values
only set its colors, its meshes and the level it glows like. Colors are
names from the glow color list, see below.

```json
"glow": { "color": "ice", "meshes": [2], "shineColor": "orange", "excellentMesh": 2 }
```

| Value | Meaning | Default |
|---|---|---|
| `level` | The level the item glows like instead of its own: one level, e.g. `8` for jewels and `0` for wings, or a list of 16, one for each item level from 0 to 15 (arrows, Devil's Square items). Levels above 15 do not glow. | its level |
| `color` | Color of the glow of items +7 and up. | `orange` |
| `meshes` / `hiddenMesh` | The glow is only on these meshes (`[0, 1]`), or on all meshes but this one (`1`). | all meshes |
| `shineColor` | The extra shine of items +11 and up tints the light of the item with this color. | `white` |
| `shineWhite` | `true`: the shine is plain white instead. | `false` |
| `shineMeshes` / `shineHiddenMesh` | Like `meshes` / `hiddenMesh`, for the shine and for the glow of ancient items. | all meshes |
| `ancientColor` | Color of the glow of ancient items. | `azure` |
| `excellent` | `false`: excellent items do not glow (wings and capes). | `true` |
| `excellentMesh` | The excellent glow is only on this mesh. | all meshes |
| `excellentMeshWithoutSkin` | The same, when the item is drawn without the character, in the inventory and on the ground. | `excellentMesh` |

The glow colors are named in `src/bin/Data/Effects/GlowColors.json`, as
red, green and blue from 0 to 1:

```json
{ "formatVersion": 1, "colors": { "orange": [1, 0.5, 0], "gold": [1, 0.7, 0.2] } }
```

A name has only letters and digits. A new color is added to the list and
can then be used by any item; a name that is not in the list stops the
start with a message, and so does a list without the defaults (`orange`,
`white`, `azure`). The item editor adds colors, changes their values and
removes the ones no item uses ([Glow Colors](#glow-colors)), and Save Items
writes the file in this form, one color a line.

The glow of monsters, and of the event models that level variants are
drawn with, is still set in code; changing the list does not change them.

`renderStyle` names the look of models that are more than a plain textured
model: chrome layers, glowing or animated meshes, a texture of their own
for the inventory, ...

```json
{ "number": 15, "file": "Data/Player/ArmorMale16.bmd", "textureFolders": ["Player"], "renderStyle": "stormCrow" }
```

Each look is code, a render style with a name; the names are listed at
the end of `src/source/Render/Items/ItemRenderStyles.cpp`. Items with the
same look share a style (the Storm Crow set, the archangel staff and
crossbow, …), and a few plain looks have names of their own: `textured`
(drawn with its texture only, whatever the drawing asks for),
`texturedAllMeshes`, `chromeMesh0` and `chromeMesh1` (a chrome shine on
one mesh), `onlyMesh0WithoutSkin`, `onlyMesh1WithoutSkin` and
`onlyMesh2WithoutSkin` (in the inventory and on the ground only the armor
mesh, without the character skin). Styles that only differ in a texture
share their code, with the texture in the list of names (the package
boxes, the tickets, the chaos cards, …). An item without `renderStyle` is
drawn plainly; a new look needs a new style in code.

Most styles fit any item. The ones that pick a mesh, a texture or a color
for each of their items only know those items and draw other items
plainly: `violentWindToEternalWingHelm`, `…Armor` and `…Pants` (items 39
to 44), `socketSeed`, `socketSeedSphere`, `characterCard` and
`divineAndSuccubusSkin`.

Some styles only apply to some drawings, and the model is drawn plainly
otherwise: `runeBlade`, `greatScepter`, `grandSoulShield` and the
`violentWindToEternalWing…` styles not for doppelgangers, `monsterBattleBow`
only in the hands of the Metal Balrog and the Orc Archer of Doom,
`helperNpcPlate` only on the helper NPCs (Luke and Leo the Helper, Helper
Ellen; in the code the flag of a PC room look, which no player gets).

A few styles also shine below +3: the model is drawn with the light of the
item scaled, then with two shine passes. `sealOfAscension`, `sealOfWealth`
and `sealOfSustenance` (the seals, light at 0.9), `illusionSorcererCovenant`
and `harmonyShine` (the Jewel of Harmony and the Moonstone Pendant, which
are drawn plainly otherwise), and `cursedCastleWater`.

`itemEffect` names what the model does before it is drawn:

```json
{ "number": 37, "file": "Data/Item/wing09.bmd", "textureFolders": ["Item"], "itemEffect": "wingOfEternal" }
```

Each item effect is code with a name; the names are listed at the end of
`src/source/Render/Items/ItemEffects.cpp`. An item effect runs every frame
before the model is drawn and can:

- place sprites, particles and lightning on bones of the model, so they
  follow its animation (the Devil's Key and Invitation, Rena, the wings of
  the third tier, the Wings of Darkness);
- change values of the drawing: a pulsing glow mesh (`wingsOfDragon`,
  `wingsOfSoul`, `redSpirit`, `staffOfKundun`, `divineSet`), a mesh hidden
  by level (`hiddenMeshByLevel` of the Siege Potion and the Contract,
  `hideMesh1`), the level potions glow like (`potion`: +7 at every level
  above 0);
- draw the model itself instead of the usual drawing (the Dark Lord's
  scrolls, `fruits`, `spirit`, `firecracker`, `gmGift`,
  `meshesPerLevel`).

Items with the same item effect share it. The socket seeds and spheres and
zen have no item effect; they glow like level 0 (`"glow": {"level": 0}`), whatever
their level.

The effects of the event models that level variants are drawn with stay in
code; they get model entries later.

- All item models are loaded at startup, on the loading screen.
- An item without a model entry is not drawn, unless code draws it at some
  levels with other models: the Wizard's Ring (event models at +0 and +1
  to +3) and the Weapon of Archangel (the archangel weapons, small, at +0
  to +2). Some items are drawn with the model of another item or with an
  effect model; that choice (`Render::Items::Display::FindInventoryModel`
  for the inventory), and the look of those effect models, is still made
  in code. So is the look of items that changes with their level (level
  variants, e.g. the Box of Luck); they become items of their own later.
- Models that are not item models (effects, monsters, the character
  bodies) are not in these files.
- Write paths with the upper and lower case of the files, so the game
  also finds them on Linux and macOS.

Model files are checked like the item files: invalid JSON, a missing
`number`, a missing `file` (without `model`), a file that is not a
`.bmd`, a path that leaves the game folder (starting with `/`, a drive
letter or `..`), `\` in a path, display or glow values of the wrong kind
(e.g. a rotation with two numbers, a scale of 0 or a color value above
1), an item with two models, `model` together with `file`,
`textureFolders` or `noneBlendMeshes`, a `model` that is not in
`SharedModels.json`, or a shared model defined twice stop the start with
a message; unknown fields and a shared model that no item uses are
warnings. The automated tests also check that every model file and
texture folder exists, that every texture of a model is in one of its
texture folders, that every texture is a `.jpg` or `.tga` texture, and
that every model file that several items use is a shared model.

When a model file or a texture cannot be loaded, one message after
loading lists the problems (up to 10; all of them are in `MuError.log`).
**Continue** keeps loading, the items are then drawn without the missing
parts; **Quit** closes the game; **Copy text** copies the message, e.g.
for a bug report. Each line says what to fix:

```
Storm Hard Glove (0,33): texture Item762_Armor.jpg (Item762_Armor.OZJ) of mesh 1 of
Data/Item/Sword34.bmd not found or not readable in Data/Item/ (Data/Items/Models/Group00_Sword.json)
```

The problems are:

| Problem | Kind |
|---|---|
| The `.bmd` file could not be opened (it is missing or not a valid model). | error |
| A texture is in none of the texture folders, or could not be read. | error |
| A texture is not a `.jpg` or `.tga` texture; the game cannot load other types. | error |
| A texture is in none of the texture folders, but another model loaded it before; that one is used. The warning names the folder to add to `textureFolders`. | warning |
| `noneBlendMeshes` has a mesh number the model does not have. | warning |
| A `glow` value names a mesh the model does not have; that glow is not drawn (a hidden mesh: the glow is on all meshes). | warning |
| `renderStyle` names a style that does not exist; the model is drawn plainly. | warning |
| `itemEffect` names an item effect that does not exist; the model is drawn without it. | warning |

A model file can be in the formats of the Season 6 client (BMD versions
10 and 12) or in the encrypted format of later seasons' models (version
14); files of other versions cannot be opened. The version is the fourth
byte of the file, after `BMD`.

The model names textures as `.jpg`/`.tga`; the game reads the encrypted
copies with the same name, `.OZJ`/`.OZT`. Meshes whose texture name starts
with `hid` are not drawn, so their texture is not loaded.

On Linux the game itself hands out the copied text, so it may only be
pastable while the game runs. The text is in `MuError.log` as well.

### Shared models

Items that show the same model file (the skill parchments, the socket
seeds, the jewels and their bundles, …) name a shared model with `model`
instead of a file of their own. The shared models are in
`SharedModels.json`, sorted by name:

```json
{
  "formatVersion": 1,
  "models": [
    { "name": "skillParchment", "file": "Data/Item/rollofpaper.bmd", "textureFolders": ["Item"] }
  ]
}
```

```json
{ "number": 19, "model": "skillParchment", "inventory": { "offset": [0.03, 0.03] } }
```

A shared model has a `name` of letters and digits and the `file`,
`textureFolders` and `noneBlendMeshes` of a model entry. Its file is
opened once, by the first of its items, and its textures are loaded once;
the other items use that data. Everything else (`inventory`, `ground`,
`glow`, `renderStyle`, `itemEffect`, `cloth`) stays each item's own.
Changing a shared model changes all its items; the model tool lists them
first (see [Item Model](#item-model)).

---

## Changing names and translations

How to change item names, translate items and add a language is described
in [translation-system.md → Item names](translation-system.md#item-names).

## Editing items (MuEditor)

The item tools (editor builds, F12, **Windows → Items**) edit the items of
the running game. They share one selected item: selecting it in one tool
shows it in the others.

- **Item Browser**: the items with a search (a name in any language,
  `group,number` or the item type) and filters: group, tag, a rule flag
  that is off, items without a model, numbers that have a model but no
  item, items without a name in a language. An item whose name another
  item has too shows in grey what tells them apart, in the list and above
  each tool: the tags it has that not all its namesakes have (the lucky
  sets: `luckyItem`), or `#1`, `#2` by number where no tag does (the Seals of
  Wealth, the tiers of the seed spheres); the one namesake without such a tag
  (the plain item of a lucky set) shows nothing. Names are compared in the
  UI language. It has **Save Items**,
  **Import from bmd**, **Export as bmd**, **Export as S6E3** and
  **Export as CSV**, and:
  - **Add item**: a group, a free number (one with neither an item nor a
    model entry), the English name, and either the selected item's values
    or a size and slot of its own (no slot to start with, for an item that
    cannot be worn; an item that can be worn starts usable by every class).
    Its model is linked in **Item Model**.
  - **Duplicate item**: a copy with its model. A model file of its own
    becomes a shared model that both items use, so it is opened once.
  - **Move item**: to a free number, with its model (the model data and the
    model the game has opened), so the item keeps its looks.
  - **Remove item**: the item and its model entry.

  Moving and removing first list what goes with the item (its tags, its
  model entry and the shared model it names) and what uses it by its
  number and stays there: the source lines that name it (read from the
  sources the editor was built from; without them that part is not known)
  and an ancient set, add option, buff or pet of the game data.
- **Item Basics**: the English name, size, slot and wing tier (*None* for
  none), two-handed, weapon kind (*By the group* takes the kind the item's
  group gives, which then follows its group, slot and classes), skill, drop
  level, potion priority, the stats, the prices,
  the stat profile (*By the rules* takes what the
  [profile rules](#item-stats-by-level) give; beside the list, the rule that
  gives the item its profile, or that it is picked for the item; **Item
  Formulas** opens the profile there) and the resistances. Below the stats, what the game
  makes of them at each + level (damage, magic power, defense, magic
  defense, block rate), also as excellent or ancient, while a value is still
  being typed, and under that the formula of each with the item's numbers
  and where each part comes from: `[default]`, the profile's name, or `[item:
  excellentBonus]` for a field of the item. Size and slot stay as they are
  while your character wears the item or a window shows it (the inventory,
  the vault, a trade, a shop): an item grid frees the cells by the size an
  item has when it is taken out. Such an item is not moved or removed
  either, an undo step that would change it is refused, and Import from bmd
  keeps its size and slot.
- **Item Requirements**: the stat profile, as in Item Basics, then the level
  and stat requirements. A stat requirement whose profile gives it a formula
  shows its value at +0 (what a normal item needs without an option level,
  as the game's `/<item name>` window shows it) and its weight, the base
  value the [item stat file](#item-stats-by-level)'s formula takes and the
  item file stores, with the weights that give the same +0 value beside it
  ("Weights 39 to 44 give 27"): the formula rounds down, so several weights
  can give one +0 value and differ at higher levels and for excellent items.
  Typing a +0 value takes the weight that gives it; when several do, or none
  does, a list shows them with what each gives at +15 and for excellent
  items (or the nearest values some weight gives), and *Pick a weight* opens
  it for the value the item has, to fine tune the weight. The weight can be
  typed too. The level, and a requirement whose formula keeps the base value
  (`"base"`), take the value itself. Beside each, its formula with the item's
  numbers and where each part comes from, as in Item Basics. Each also by
  item level (16 values that replace the formula; they start at what the
  base value gives a normal item at each level, and turning them off finds
  the base value back; excellent and ancient items get no higher stat
  requirement from them, while excellent options still raise a level given
  by level and the option level still adds the strength its stat profile
  gives per option level; a stat requirement whose formula takes the level
  requirement, follows a level given by level). Below them, what the game asks at each + level
  (the `/<item name>` window shows the normal item), also as excellent,
  ancient or with an option level, while a value is still being typed. Then
  the class steps (0: the
  class cannot use the item) and whether excellent options raise the level
  requirement.
- **Item Rules**: the rule flags as a table of items; select several rows
  (Ctrl or Shift click) to allow or forbid a flag for all of them.
- **Item Categories**: a tag and its items; add the selected item or remove
  items from it. New tags need code.
- **Item Model**: see [Item Model](#item-model).
- **Glow Colors**: the glow color list, see [Glow Colors](#glow-colors).
- **Item Formulas**: the item stat file, see [Item Formulas](#item-formulas).
- **Item Checks**: the checks of a save as a list, errors first. A click on
  a problem of an item selects it and opens the tool of its field; an arrow
  key onto it only selects the item. A click on a problem of the item stat
  file (a profile or a table nothing uses) opens Item Formulas at it.
- **Item Sync**: Export and Import of the items and item formulas with an
  OpenMU server, see [Item Sync](#item-sync).
- **Item Translations**: the names in all languages, see
  [translation-system.md → In the item editor](translation-system.md#in-the-item-editor).

In the browser, Item Rules, Item Categories, Item Checks and Item
Translations, the arrow keys move the selection (see
[dev-editor.md](dev-editor.md)). In the lists of the items that use a model
or a look they only move the cursor: selecting one there shows that item's
form instead, with its own list.

How changes work:

- **Every change goes into the item database right away**, and the game
  uses it at once. Items in the inventory keep the values computed when
  they were made until they are made again (e.g. after logging in again).
- A text or number field is stored when you leave it after a change, a
  checkbox or list right away. Each is one step of **Undo** and **Redo**
  (in the editor's bar, see [dev-editor.md](dev-editor.md); also Ctrl+Z
  and Ctrl+Y while an item tool has the focus and no field is
  being typed in): the last 256 steps, for the whole session and across
  saves. A step is not undone when the item changed in a way the history
  did not record. Import from bmd clears the history. The browser's title
  has a `*` while there are changes that are not saved.
- The weapon kind and the stat profile follow the item's data while they
  are what it gives: the weapon kind its group, slot and classes, the stat
  profile the profile rules of the item stat file; a value you pick stays.
- **Save Items** runs the checks and writes `Data/Items` next to the game,
  with the model files of the groups whose models changed,
  `SharedModels.json` when a shared model changed,
  `Data/Effects/GlowColors.json` when a glow color changed and
  `Stats/ItemStats.json` when the item formulas changed; the other model
  files are not touched. It lists each UI language that misses item names, with
  how many. With errors nothing is written, and the errors are listed in
  the editor console and in **Item Checks**. Warnings (a model file that is
  missing, a file several items name themselves instead of through a
  shared model, a render style or item effect that does not exist) do not
  stop it. If a file cannot be written (read-only folder, full disk), no
  file is replaced, the popup says so and the console names the file. A
  file open in another program can still fail when it is replaced, after
  others were. Only files whose content changed are rewritten.

The game runs from the build folder, which has a copy of `src/bin/Data`.
To keep your changes, copy the changed files from
`<build folder>/Data/Items` to `src/bin/Data/Items` (and
`<build folder>/Data/Effects/GlowColors.json` to `src/bin/Data/Effects`) and
commit them. A new
`.bmd` file or texture has to be copied to `src/bin/Data` too (textures as
`.OZJ` or `.OZT`).

### Item Model

The model tool edits the selected item's model entry, also the one of a
number that has a model but no item:

- A preview of the model as the game has it opened, at an item level,
  excellent or ancient, standing as the inventory turns it or lying as on
  the ground (over a ground plane as high below it as the game drops it) (drag to turn, drag with the middle mouse button to move,
  wheel to zoom, double click to put it back).
- The model file, picked from the `.bmd` files below `Data/Item` and
  `Data/Player`. An item without a model gets an entry, with the file's
  folder as its texture folder; a model whose only texture folder is its
  old file's folder takes the new file's. A shared model that opens the same file is
  offered instead; other items that name the file themselves are counted
  (a shared model would open it once).
- The shared model: pick one; for the one the item uses, its items and its
  name, file, texture folders and none-blend meshes. A change is applied
  only after a warning that lists every item it reaches. **Make a copy for
  this item** gives the item a file entry of its own instead (a variant).
  A shared model that items use cannot be removed; the ones no item uses
  are listed with a button that removes them.
- For a file of its own, the texture folders and none-blend meshes; then
  the inventory display as sliders, each with a number field to type the
  value (a drag is one undo step) beside the item in an inventory grid of
  its size, drawn as
  the game's inventory draws it at the preview's item level (it turns while
  the mouse is on it); the ground display, cloth, specular and the glow
  (levels, colors with their swatches, meshes, excellent). The **+** beside
  each glow color adds a new color to the list, starting as a copy of the
  field's color, and picks it, as one undo step. While a slider is
  dragged, the grid and the game's inventory show the value at once; it is
  stored when the slider is let go.
- For an item drawn by code with other models at some levels, the list of
  those models by level (the Box of Luck, the Wizard's Ring, ...); the
  preview and the inventory grid show them at the chosen level, also for
  an item without a model entry.
- The render style and the item effect, picked from their lists with how
  many models use each, and the items that use the picked one (a click
  selects one). What a look is made of is drawing code.

A changed file, texture folder, mesh list or material is opened again
before the next frame, so the game draws it at once. Models that the game
changes in code after loading (the cloth of some pants, the second models
of the Rage Fighter gloves, the inventory models of the Rage Fighter
armors) may need a restart to show a new file.

### Glow Colors

The colors of the glow color list, each with how many models use it (also
as the default they leave out). **New glow color** adds one: a name of
letters and digits no other color has, and its red, green and blue. A
picked color's value is changed with the color field and **Apply to the
glow color**; when items use it, the change lists them first, as for a
shared model. A color no item uses is removed (**Remove**, or **Remove
them** for all of them); the defaults (`orange`, `white`, `azure`) stay,
since items without colors of their own glow with them. Every change is a
step of the undo history; renaming a color is left out.

### Item Formulas

The [item stat file](#item-stats-by-level): its drop level bonus, the
default, the profiles, the level bonus tables and the profile rules, each in
the list with how
many items use it (a table: how many stats and item descriptions name it;
unused ones are grey).
The picked entry is changed on a copy and **Apply to the formulas** takes
it: the file is checked first (written and read back, as Save Items does),
and when the change reaches items (other values, or another profile when a
profile is renamed) they are listed before it is applied, as for a shared
model. Every change is one step of the undo history, and Save Items writes
`Stats/ItemStats.json`. While the picked entry has changes that are not
applied, **New stat profile**, **New level bonus table**, **Copy**,
**Rename** and **Remove them** wait until they are applied or reverted.

- **Drop level bonus**: what excellent and ancient items add to their drop
  level for their requirements and the ancient part of their stats.
- **Default**: every term of every stat and the requirement formulas.
- **A profile**: each term it writes, and the ones it inherits from the
  default (grey, with **Override** to write the term in the profile; **Reset
  to default** takes a written term out again). **New stat profile**,
  **Copy** and **Rename** take a name of letters and digits no other profile
  has (not `default`); a rename renames the profile in the profile rules and
  in every item that names it. **Remove** works while no item and no rule
  uses the profile, **Remove them** for all such profiles.
- **A table**: the bonus at each item level 0 to 15, what it adds for every
  level (also above +15), its English caption and the items whose
  descriptions name it. New, copied, renamed (in every stat and every item
  description that names it, in every language) and removed tables work like
  profiles; a table is removed while no stat and no item description names
  it.
- **Profile rules**: the rules in order, each with its profile, its
  conditions and how many items it gives their profile (the selected item's
  rule is marked). **New rule** adds a copy of the picked rule below it (or a
  rule for the default at the end), **Move up** and **Move down** change the
  order, **Remove the rule** takes it out; the picked rule's conditions are
  lists to tick values in (none ticked: any). Applying lists the items that
  follow the rules and take another profile with the change; a profile picked
  for an item stays. A rule changed by hand in the file takes no item along:
  the items keep the profile they name, and those it no longer gives count as
  picked.

The server keeps its own formulas until they are synced (see
[Item Sync](#item-sync)), so what it checks and computes can differ from
what changed formulas give. Items your character already has keep their
values until they are made again.

### Item Sync

Item Sync exchanges the items and the item formulas with an OpenMU server
through a file, the *item exchange file*. The server stays authoritative:
the client never writes to it. (OpenMU's export and import of the file, and
Update from server, which downloads it, are later steps of phase 7; see the
[design](superpowers/specs/2026-09-25-data-driven-items-design.md).)

- **Import…** opens a file and lists how it differs from the client's
  items and formulas: what the import would add to the client, change in
  it or remove from it. **Apply the checked changes** first says what it
  changes ("3 items added, 5 changed, 1 removed; 2 formula entries
  change"; what is not checked or not listed stays as it is), then takes
  the checked rows into the client as one step of the undo history; Save
  Items writes the files, as for any edit.
- **Export…** writes the client's items and formulas into the file it
  picks, without a diff: OpenMU's import page shows what the file changes
  on the server, and Import shows what one changes in the client. A new
  file is written right away ("Exported 911 items and all formulas to
  …"). A file that is there is replaced after a confirmation that says what
  changes in it ("3 items added, 5 changed, 1 removed; 2 formula entries
  change"), or that it cannot be read as an exchange file; a file that has
  the client's data already is left as it is. The written file says
  `"source": "MuMain"`.
- **Compare two files** (folded under the path field) compares two exchange
  files, e.g. a client export with a server export, without the client's
  data: pick file A and file B (**Swap** swaps them), then **Compare the
  files**. The lists say what changes from file A to file B (Added,
  Changed, Removed under "From A to B", the values under "File A" and
  "File B"; an item only file A has lists file A's values), in every
  language, also one only one file has. An item a file has twice is a
  locked row that names that file. Nothing can be applied, but a row can be
  marked as a known difference. An import or a comparison of two files
  replaces the other in the window.
- An import lists only what differs. The lists say how much of what was
  compared differs ("Items that differ (3 of 911)"), and when nothing does,
  how many are the same ("Items: all 911 are the same in the client and the
  file"). When the file has exactly the client's data, the window says so
  in green ("The file matches the client: nothing differs."), e.g. for a
  file you just exported; change an item or the file, or pick another
  file, to see differences again. "The same" means that there is nothing to
  take: a name in a language the file lacks, or one the client's UI does
  not show, is not compared (no language is removed).
- After applying, the window compares again: what was applied no longer
  differs and leaves the list. A green line above says what was applied;
  what was not checked stays listed and unchecked. An import is refused
  when an item it would add, resize, give another slot or remove is worn or
  in an item window by then (compare again).
- The path of an import can also be typed into **Exchange file**;
  **Compare again** compares with it. **Apply the checked changes** is off
  while nothing is checked, while the path field holds another file than
  the one compared, and while the client's items or formulas changed since
  the comparison (the window says which; compare again).

**The file** has the layout of the item files: items sorted by group and
number, fields with their default value left out, LF line endings.

```json
{
  "formatVersion": 1,
  "source": "OpenMU",
  "server": "…",
  "gameVersion": "season6",
  "formulas": {
    "dropLevelBonus": {"excellent": 25, "ancient": 30},
    "tables": […],
    "default": {…},
    "profiles": […]
  },
  "items": [
    {"group": 0, "number": 0, "name": {"en": "Kris", …}, "width": 1, "height": 2, …, "statProfile": "weapon"}
  ]
}
```

- `source` is `MuMain` or `OpenMU`; `server`, only in a file from OpenMU,
  names the server's installation.
- Only `season6` data is exchanged; a file of another `gameVersion` is
  refused, as is a `formatVersion` the client does not know.
- `formulas` is the [item stat file](#item-stats-by-level) without its
  `formatVersion` and its profile rules, which stay with the client.
- `items` has the fields both sides have, with the keys and meanings of the
  item files: the group and number, the names, size, slot, skill, drop
  level, durability and magic durability, damage, attack and walk speed,
  magic power, defense, block rate, resistances, the requirements (also by
  level), the class requirements, the prices, the [rule flags](#rules)
  without the rental ones, the tags `ammunition`, `secondClassQuestItem`
  and `thirdClassQuestItem`, and `statProfile` (always written: the profile
  the item uses, `default` for the default), `excellentBonus` and
  `excellentRaisesLevel`.
- What only the client has is not in it: the model and display data, the
  description, the other tags, the wing tier, weapon kind, two-handed flag,
  potion priority, attack type, magic defense and the rental flags. A file
  with such a field is read with a warning and the field is left out; an
  import keeps the client's.

**The diff of an import**:

- The **Items** list says for each item what the import does to it in the
  client (Added, Changed, Removed). The box of an added or removed item
  checks the whole item, the box of a changed one all its fields. A click on a row shows its fields (and selects the item in the
  item tools): each field (`width`, `requirements.strength`, `name.de`,
  `tags.ammunition`, …) with its value now and the file's, and a box of its
  own.
- The **Formulas** list has the drop level bonus, each table, each term of
  the default and of each profile (`default.damage.table`,
  `profiles.scroll.requirements.energy`, …) and a profile only one side
  has. A file without formulas compares none and leaves the client's as
  they are.
- Checked at first: what the file changes. Unchecked: removals (also of
  a formula entry only the client has), known differences (grey) and
  fields with a note: the size and slot of an
  item your character wears or an item window shows (they stay; such an
  item is not removed either, nor added when the client has no data for it
  yet), and a drop level above 255, which OpenMU keeps in a byte.
- **Check all** and **Check none** take every row; **A field in all items**
  checks or unchecks one field in every changed item.
- **Names**: only the client's UI languages are taken from the file; a
  language the file lacks stays as it is, and no language is removed.
- **Stat profile**: a file's profile that the item's profile rules give
  leaves the item following its rules; another one becomes a pick (see
  Item Basics).
- **Added items** get the client-only fields at their defaults; the
  two-handed flag follows the rule of the shipped items (groups 0 to 5, a
  hand slot, two cells wide or more) and the weapon kind follows the group,
  as for Add item. The row says when the item has no model entry yet (link
  one in Item Model; the browser's "without a model" filter lists it) and
  when its profile rules give another profile than the file (a wing tier
  or a tag such as `cape` may be missing).
- **Removed items** go with their model entry, as Remove item does; the row
  lists what goes with the item and what uses it at its number.
- An item a file has more than once (OpenMU does not keep the group and
  number unique) is a locked row: the import leaves it as it is.
- Before the import is applied, the data as it would be gets the checks of
  a save. With errors nothing changes; the errors are listed and the rows of
  their items are unchecked.

**Known differences** are fields and formula entries meant to differ
between the client and the server. Ticking **Known** on a field, on an
added or removed item or on a formula entry leaves it unchecked and grey in
every comparison. They are kept in `Data/Items/Sync/KnownDifferences.json`
next to the game, written when one is ticked; copy it to
`src/bin/Data/Items/Sync` with the item files to share it. `""` stands for
the whole item:

```json
{
  "formatVersion": 1,
  "items": [
    {"group": 13, "number": 4, "fields": ["", "repairable"]}
  ],
  "formulas": [
    "default.requirements.leadership"
  ]
}
```

### Import from bmd / Export as bmd

The repository does not ship `Item_<lang>.bmd` files; the JSON files are the
only item data. The import is there to bring in your own old item files.

- **Import from bmd** replaces all items with
  `Data/Local/<Eng|Por|Spn>/Item_<lang>.bmd` next to the game; copy your
  files there first. English provides the values and is required,
  Portuguese and Spanish add their names. A name equal to the English one
  stays only where the item has a name of its own in that language now (it
  is the same there); otherwise it is taken for a missing translation,
  which a bmd file has as the English name, and stays missing. Items the
  English file does not have keep
  the English name they have now, and every item keeps its tags, wing tier,
  weapon kind, stat fields (`statProfile`, `excellentBonus`,
  `excellentRaisesLevel`, `requirementsByLevel`) and rule flags (the bmd
  format has none). A weapon kind that its group gives follows the imported
  slot (in group 0 also the class requirements), so a new weapon gets one
  too; a stat profile that the profile rules give follows the same way. The values
  come from the files: the original ones have no defense for the Cape of
  Lord (the client set its 15 in code until phase 3c), so it then has none.
  The imported items are at their bmd numbers, so the models of items you
  moved since the start go back to their old numbers too.
  Save afterwards to keep the result. The console lists what the import had to fix (see below) and any
  problem the imported data still has; Save refuses it until those are
  fixed.
- **Export as bmd** writes `Item_<lang>.bmd` for English, Portuguese and
  Spanish with the names of each language; a backup of each old file is kept.
  A missing Portuguese or Spanish name is written as the English one.
  Files that already have the data stay as they are. Values that the bmd
  format does not have (tags, wing tier, weapon kind, stat fields, rule
  flags) are left out. The format holds damage and magic power in one byte:
  while an item has more than 255, nothing is written, and the console names
  every such item with its field and value ("Item (0,51) DamageMin is 300; a
  .bmd item file holds up to 255"). **Export as S6E3** refuses them the same
  way.

### How the bmd import repairs the legacy files

The original item files have 30 bytes for a name. Longer names ran on into
the next fields (two-handed flag, level, slot, …), so those fields hold
parts of the name instead of values. The game used these broken values;
for example, *Open Access Ticket to Chaos Castle* was two-handed, had
level 25964 and went into the weapon slot.

The import:

1. reads each name up to its end, so the full name is kept
   (*Open Access Ticket to Chaos Castle* instead of
   *Open Access Ticket to Chaos Ca*);
2. takes every field a name ran into from the next language whose name did
   not reach that field (English, then Portuguese, then Spanish);
3. uses the field's default when no language has an undamaged value.

Names that are not valid UTF-8 are read as Windows-1252, the encoding of
the Portuguese and Spanish files; the original client showed their
accented letters as `�`. The byte pair `A1 AF` (a right quote on Korean
systems) becomes an apostrophe (*Gaion's Order*).
