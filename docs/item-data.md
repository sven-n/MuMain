# Item data

All item definitions (names, size, slot, stats, requirements, tags and
rules) live in JSON files in `src/bin/Data/Items/`, one file per item
group; the item models are in `src/bin/Data/Items/Models/` (see
[Item models](#item-models)). The client loads them once at startup into an in-memory item
database; `Item_<lang>.bmd` is no longer read by the game.

The data matches OpenMU's `ItemDefinition` where both sides have the same
fields, so it can later be exchanged with the server.

---

## Files

| File | Group |
|---|---|
| `Group00_Sword.json` … `Group15_Etc.json` | 0–15, named after the `ITEM_GROUP_*` constants |

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
  default: `0`/`false`, no slot (not equippable), no wing tier, no tags,
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
- Logs always use the English name and the item id, e.g.
  `Short Sword (0,1)`, never a translation.

### Fields

| Field | Meaning |
|---|---|
| `tags` | Categories the game code asks for, e.g. `["mount", "flying"]`; see [Tags](#tags) |
| `width`, `height` | Size in inventory slots |
| `slot` | Equipment slot: `mainHand`, `offHand`, `helm`, `armor`, `pants`, `gloves`, `boots`, `wings`, `pet`, `pendant`, `ring`. Left out = not equippable |
| `wingTier` | Only for wings: `small`, `first`, `second`, `third`. The wing formulas (defense, damage increase, absorption) depend on it |
| `twoHanded` | Weapon needs both hands |
| `skill` | Skill the item gives |
| `level` | Item level (the drop level); also used in price and damage calculations |
| `durability`, `magicDurability` | Durability at item level 0 |
| `damageMin`, `damageMax`, `attackSpeed`, `walkSpeed`, `magicPower` | Weapon values |
| `defense`, `magicDefense`, `blockRate` | Armor and shield values |
| `attackType` | Attack type used by the client |
| `sellValue` | Value factor used by the price calculation of some items |
| `buyPrice` | Fixed price; when set, it replaces the calculated price |
| `requirements` | `level`, `strength`, `dexterity`, `energy`, `vitality`, `leadership` |
| `classRequirements` | Per class (`darkWizard`, `darkKnight`, `fairyElf`, `magicGladiator`, `darkLord`, `summoner`, `rageFighter`): 0 = cannot use, otherwise the class level needed |
| `resistances` | `ice`, `poison`, `lightning`, `fire`, `earth`, `wind`, `water` |
| `tradable`, `droppable`, `storable`, `sellable`, `personalShopSellable`, `repairable` | What a player may do with the item; see [Rules](#rules). Only `false` is written |
| `droppableWhileRented`, `personalShopSellableWhileRented`, `sellableWhenRentalExpired` | What changes for a rented item; see [Rules](#rules) |

### Tags

A tag puts an item into a category that the game code asks for. An item
can have several tags; the file lists them in the order below.

| Tag | Meaning |
|---|---|
| `cape` | A cape (Cape of Lord, Cape of Fighter, …); second tier capes have their own wing formulas |
| `mount` | Can be ridden (Uniria, Dinorant, Dark Horse, Fenrir) |
| `hornMount` | A mount summoned by a horn (Uniria, Dinorant, Fenrir) |
| `flying` | A mount that can fly (Dinorant, Dark Horse, Fenrir), needed for Icarus; see [Flight equipment](#flight-equipment) |
| `darkLordPet` | Dark Horse and Dark Raven |
| `guardianPet` | Demon and Spirit of Guardian |
| `pandaOrSkeleton` | The panda and skeleton pets and transformation rings |
| `jewel` | Jewels of Bless, Soul, Life, Chaos, Creation and Guardian |
| `refineStone` | Lower and Higher Refine Stone |
| `socketSeed`, `socketSphere`, `socketSeedSphere` | Socket system items |
| `healingPotion`, `manaPotion`, `complexPotion` | Potions the hotkeys look for |
| `elitePotion`, `elixir`, `buffScroll`, `battleOrStrengthScroll` | Cash shop potions and scrolls |
| `ammunition` | Arrows and bolts |
| `bloodCastleTicketPart` | Scroll of Archangel and Blood Bone |
| `secondClassQuestItem`, `thirdClassQuestItem` | Class change quest items |
| `summonerBook` | Summoner books |
| `divineArchangelWeapon` | The Divine weapons of the Archangel |
| `cashShop` | Items from the cash shop. Only information for now (for the editors and the OpenMU exchange); no client code reads it |
| `gambleItem` | Gamble items |
| `gemJewelry` | The gem rings and necklaces from the cash shop |
| `luckyItemTicket` | Lucky item tickets |
| `valuable` | The game asks for confirmation before the item is sold or dropped |

Wings need no tag: an item is a wing when its `slot` is `wings`.

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

---

## Loading and errors

At startup all `*.json` files in `Data/Items` are read and checked.

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
- a `slot`, `wingTier` or tag name that does not exist

**Warnings** are logged, and the game starts anyway:

- unknown fields (they are ignored)
- names longer than 49 characters (the game shows them cut)
- a `wingTier` on an item whose slot is not `wings`
- a `slot` written as a number, as files saved before tags existed have
  it; the number is read, and saving writes the name

An automated test loads the shipped item data, so a pull request with
broken item data fails its checks.

## Item models

Which 3D model an item shows is set in `src/bin/Data/Items/Models/`, one
file per item group with the same file names as the item files. They are
separate from the item files because they only matter to the client; the
item files hold what client and server share.

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
| `textureFolders` | Folders below `Data/` with the model's textures. Each texture is taken from the **first** folder that has it. Without folders the model has no textures. |
| `noneBlendMeshes` | Mesh numbers (from 0) that are drawn without blending. Optional. |
| `inventory` | How the item is drawn in the inventory, see below. Optional. |
| `ground` | How the item lies on the ground, see below. Optional. |
| `glow` | How the item glows, see below. Optional. |
| `cloth` | `true` for capes that are worn as cloth: when one is put on or taken off, the character's cloth is deleted, so the next cape builds its own. Optional. The flag does not make a cape cloth; which capes are drawn as cloth, and how, is still decided in code. |

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
`white`, `azure`).

The glow of monsters, and of the event models that level variants are
drawn with, is still set in code; changing the list does not change them.

- All item models are loaded at startup, on the loading screen.
- An item without a model entry is not drawn. Some items are drawn with
  the model of another item or with an effect model; that choice, and
  the look of those effect models, is still made in code. So is the look
  of items that changes with their level (level variants, e.g. the Box
  of Luck); they become items of their own later.
- Models that are not item models (effects, monsters, the character
  bodies) are not in these files.
- Write paths with the upper and lower case of the files, so the game
  also finds them on Linux and macOS.

Model files are checked like the item files: invalid JSON, a missing
`number` or `file`, a file that is not a `.bmd`, a path that leaves the
game folder (starting with `/`, a drive letter or `..`), `\` in a path,
display or glow values of the wrong kind (e.g. a rotation with two
numbers, a scale of 0 or a color value above 1), or an item with two
models stop the start with a message;
unknown fields are warnings. The automated tests also check that every
model file and texture folder exists, that every texture of a model is in
one of its texture folders, and that every texture is a `.jpg` or `.tga`
texture.

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

The model names textures as `.jpg`/`.tga`; the game reads the encrypted
copies with the same name, `.OZJ`/`.OZT`. Meshes whose texture name starts
with `hid` are not drawn, so their texture is not loaded.

On Linux the game itself hands out the copied text, so it may only be
pastable while the game runs. The text is in `MuError.log` as well.

---

## Changing names and translations

How to change item names, translate items and add a language is described
in [translation-system.md → Item names](translation-system.md#item-names).

## Editing items (MuEditor)

The item editor (editor builds, F12) edits the items of the running game:

- **Every change goes into the item database right away**, so the game
  uses it immediately.
- **Name changes are stored for the current UI locale.** To edit a
  translation, switch the language first; to edit the English name,
  switch to English.
- **Save Items** checks the data and writes `Data/Items` next to the game.
  With errors nothing is written, and the errors are listed in the
  editor console. If a file cannot be written (read-only folder, file
  open in another program), the popup says so and the console names the
  file. Only files whose content changed are rewritten.

The game runs from the build folder, which has a copy of `src/bin/Data`.
To keep your changes, copy the changed files from
`<build folder>/Data/Items` to `src/bin/Data/Items` and commit them.

### Import from bmd / Export as bmd

The repository does not ship `Item_<lang>.bmd` files; the JSON files are the
only item data. The import is there to bring in your own old item files.

- **Import from bmd** replaces all items with
  `Data/Local/<Eng|Por|Spn>/Item_<lang>.bmd` next to the game; copy your
  files there first. English provides the values and is required,
  Portuguese and Spanish add their names (a translation equal to the
  English name is not stored). Items the English file does not have keep
  the English name they have now, and every item keeps its tags, wing tier
  and rule flags (the bmd format has none). Save afterwards to keep the
  result. The console lists what the import had to fix (see below) and any
  problem the imported data still has; Save refuses it until those are
  fixed.
- **Export as bmd** writes `Item_<lang>.bmd` for English, Portuguese and
  Spanish with the names of each language; a backup of each old file is kept.
  Files that already have the data stay as they are. Values that the bmd
  format does not have (tags, wing tier, rule flags) are left out.

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
