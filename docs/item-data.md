# Item data

All item definitions (names, size, slot, stats, requirements) live in JSON
files in `src/bin/Data/Items/`, one file per item group. The client loads
them once at startup into an in-memory item database; `Item_<lang>.bmd` is
no longer read by the game.

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
      "slot": 0,
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
  default: `0`/`false`, except `slot`, whose default is `255` (not
  equippable).
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
| `width`, `height` | Size in inventory slots |
| `slot` | Equipment slot; `255` = not equippable |
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

---

## Loading and errors

At startup all `*.json` files in `Data/Items` are read and checked.

**Errors** stop the start with a message that names the file, the item and
the field. All problems are also written to `MuError.log`. Errors are:

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

**Warnings** are logged, and the game starts anyway:

- unknown fields (they are ignored)
- names longer than 49 characters (the game shows them cut)

An automated test loads the shipped item data, so a pull request with
broken item data fails its checks.

---

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

- **Import from bmd** replaces all items with
  `Data/Local/<Eng|Por|Spn>/Item_<lang>.bmd`: English provides the values,
  Portuguese and Spanish add their names (a translation equal to the
  English name is not stored). Items the English file does not have keep
  the English name they have now, so importing over the shipped data gives
  the shipped data again. Save afterwards to keep the result. The console
  lists what the import had to fix (see below) and any problem the
  imported data still has; Save refuses it until those are fixed.
- **Export as bmd** writes `Item_<lang>.bmd` for English, Portuguese and
  Spanish with the names of each language; a backup of each old file is kept.
  Files that already have the data stay as they are. Values that the bmd
  format does not have are left out.

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
