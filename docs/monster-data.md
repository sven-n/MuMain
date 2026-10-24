# Monster data

The monster list gives every monster and NPC number the client knows its
names and its kind. It lives in `src/bin/Data/Monsters/Monsters.json` and is
loaded once at startup into a table by number. It replaces the original
client's name files (`Data/Local/<Language>/NpcName_<Language>.txt`), which
the repository no longer ships.

How a monster looks, its setup when it appears and its attacks are still
code, chosen by its number; later phases move them into the monster data. The
number and the names are what OpenMU calls a monster's `Number` and
`Designation`, so they can later be exchanged with the server.

Design and plans: [monsters design](superpowers/specs/2026-10-23-data-driven-monsters-design.md).

---

## Files

| File | What |
|---|---|
| `Monsters.json` | The monsters and NPCs: number, names and kind |

```json
{
  "formatVersion": 1,
  "monsters": [
    {
      "number": 2,
      "name": {"en": "Budge Dragon", "es": "Dragón Escurridizo", "pt": "Dragão Lento"},
      "kind": "monster"
    },
    {
      "number": 251,
      "name": {"en": "Hanzo the Blacksmith", "es": "Hanzo el Herrero", "pt": "Hanzo, o ferreiro"},
      "kind": "npc"
    }
  ]
}
```

- A monster is identified by its `number`, 0–1023: the number the server
  sends when a monster or NPC appears (packet 0x13 has 10 bits for it). Each
  number has at most one entry.
- Every entry has all three fields.
- Monsters are sorted by number and the fields always come in the same
  order, so saving unchanged data gives the same file (a test checks that
  the shipped file is written this way).
- The file is UTF-8 with LF line endings. `formatVersion` is 1; a file with
  a newer version stops the start.

### Fields

| Field | Meaning |
|---|---|
| `number` | The monster number, 0–1023 |
| `name` | The names by language, see [Names and translations](#names-and-translations) |
| `kind` | `monster`, `npc`, `trap` or `other`: what the game does with it, see [Kinds](#kinds) |

### Names and translations

`name` holds the monster's names by language: `en` (English) first, then the
translations sorted by language code. The codes are the same as for the UI
texts (`de`, `pt`, `es`, …). A plain text (`"name": "Hound"`) is read as the
English name only; saving writes the object form. The shipped list has
English names and, for most monsters, Portuguese and Spanish ones from the
original client's name files.

- The game shows the name in the UI language (options window → language).
  A monster without a name in that language shows the English one; an empty
  translation counts as none. The original client took the names from the
  `[LOGIN] Language` folder instead, so a player whose two settings differ
  now sees the names in the UI language.
- Switching the language renames the monsters and NPCs on screen at once.
  Summons ("Budge Dragon of <owner>") and the Kalima gates keep the name they
  appeared with until they appear again. Transformed players show the
  player's name.
- **Translate item names** does not change monster names. Only the monster
  name inside a transformation ring's item name follows it, as the rest of
  the item name does.
- The game holds up to 64 characters of a name (UTF-16 units: an emoji
  counts as two). A longer name is cut, and loading warns about it.
- 252 has an empty English name, so the game shows no name for it; loading
  warns about it. It keeps its entry.
- The Kalima gates (152–158) have plain names (`Gate to Kalima 1` …). In the
  game a gate shows "Magic stone of <owner>", which is the UI text
  `MagicStoneOfOwner` of the `Game` group, not a monster name.
- 658–668 (Cursed Statue and the Captured Stone Statues) have OpenMU's
  English names. The original client had no names for them.
- The editor names a monster as `<English name> (<number>)`, e.g.
  `Budge Dragon (2)`, never by a translation.

How to change and translate names is described in
[translation-system.md → Monster and NPC names](translation-system.md#monster-and-npc-names).

## Kinds

The kind decides what the game does with a monster or NPC. It becomes the
character's kind when the character appears:

| Kind | In the game |
|---|---|
| `monster` | A click attacks it (where the game allows an attack). While the mouse is over it, it has a red outline, and its name and health bar show at the top of the screen. F8 turns health bars above the monsters on and off. **Auto attack** (options window) and the MU Helper attack only monsters, and skills that pick their targets in the client (the Fenrir's Plasma Storm) take only monsters |
| `npc` | A click walks to it and talks to it (with the talk cursor). While the mouse is over it, it has a green outline and its name shows above it. Its speech bubbles never get a player killer's colour, and some NPCs say a line from time to time |
| `trap` | Cannot be clicked: no outline, no name. It casts no shadow, and the client's path finding walks over its tile (other characters block theirs) |
| `other` | Cannot be clicked: it is neither attacked nor talked to. In the shipped list only the Cherry Blossom Tree (451) has it |

Summons, the Kalima gates (152–158) and transformed players are players in
the game, whatever their entry says: the server's packets that create them
make them players. They keep their monster number (the control socket reports
it).

The original client decided the kind by the number alone. The shipped list
has those kinds, except for Mercenary Guild Manager Tercia (566) and
Priestess Veina (567), which are NPCs (the old rules made them monsters).

### Numbers without an entry

The server may send a number the list has no entry for (a monster the
server has and the list has not, or one you removed). The game then does
what the original client did, and writes a warning for each such number once
to `MuError.log`:

- The kind comes from the old number rules:

  | Numbers | Kind |
  |---|---|
  | 0–99 | monster |
  | 100–110 | trap |
  | 111–200 | monster |
  | 201–259 | NPC |
  | 260 and up | monster |

  Except: 451 is `other`, and 368–370 and a list of 49 numbers from 367 to
  579 are NPCs (e.g. Apostle Devin, 406, and Wandering Merchant Zyro, 568).
  The list is in `src/source/Data/GameData/MonsterData/LegacyMonsterKind.cpp`.
- No name comes from the list: the windows that name a monster by its number
  (the quest windows) show `()`, and a character of that number that appears
  gets the name its code gives it (often an old Korean name) or none. A
  character already on screen whose entry is removed in the editor keeps its
  name until it appears again.

## Loading and errors

At startup `Data/Monsters/Monsters.json` is read and checked.

**Errors** stop the start with a message ("Monster data error") that names
the file, the monster and the field. All problems are also written to
`MuError.log`. Errors are:

- a file that cannot be read, invalid JSON, a missing or unsupported
  `formatVersion`, no `monsters` list
- an entry that is not an object, or whose `number` is missing or not a
  whole number from 0 to 1023
- a number that is in the list more than once
- a missing `name`, a name that is not a text or an object of texts, or a
  translation that is not a text (e.g. `"pt": 5`)
- a missing `kind`, or one that is not `monster`, `npc`, `trap` or `other`
- a name containing `||`
- a language code that is empty or has other characters than letters,
  digits and `-` (e.g. `"p=t"`)

**Warnings** are logged, and the game starts anyway:

- unknown fields (they are ignored)
- an empty English name (also a name object without `en`): the game shows no
  name in a language without a translation
- a name longer than 64 characters: the game shows it cut
- a name in a language the game has no UI texts for (it is never shown)

An automated test loads the shipped monster list (it expects only the
warning about 252), so a pull request with broken monster data fails its
checks. With a test build (`-DBUILD_TESTING=ON`):

```bash
ctest --test-dir <build folder> --build-config Debug -R monster --output-on-failure
```

## Editing monsters (MuEditor)

The monster tools (editor builds, F12, **Windows → Monsters**) edit the
monsters of the running game: **Monster Browser**, **Monster Translations**
and **Monster Checks**. They share one selected monster: selecting it in one
tool shows it in the others. In the first layout (also after **Windows >
Reset Layout**) the Monster Browser is a tab beside the Item Browser and the
other two are tabs beside the item tools.

- **Monster Browser**: the monsters with their number, their name in the UI
  language and their kind, with a search (the whole number, or a part of a
  name in any language; A to Z in either case) and filters: a kind, and the
  monsters without a name in a language. **Clear filters** clears them. A
  monster selected in another tool is scrolled to; the search and filters
  are cleared when they hide it. It has **Save Monsters** and **Revert**,
  and:
  - **Add monster**: a number 0–1023 (the first free one to start with;
    **Next free** finds the next), the English name and the kind. A number
    with an entry is never free, also 252 with its empty name. The kind
    follows the old rules of the number (see
    [Numbers without an entry](#numbers-without-an-entry)) until you pick
    one. Its look still comes from the code by number: a number the code
    does not know appears with the Bull Fighter's body.
  - **Copy monster**: the selected monster's English name and kind at the
    next free number after it, without the translations.
  - **Remove monster**: removes the entry. The dialog says what the game
    then does with the number (the kind of the old rules, no name from the
    list) and lists what still uses it and stays there: the quests of the
    quest window whose NPC it is or that ask to kill it, how many dialogue
    states the quest system's NPC dialogues have for it, and the source
    lines that name it (read from the sources the editor was built from;
    without them that part is not known). A source line names it by its `MONSTER_` constant of the
    monster number list, by its number compared with a variable whose name
    ends in `MonsterIndex` (`c->MonsterIndex == 495`), or by a range check
    around it (`78 <= c->MonsterIndex && c->MonsterIndex <= MONSTER_GOLDEN_WHEEL`).
    Code that uses the number in other ways is not found: a `case` label, a
    function argument (`getMonsterName(236)`), a variable with another name.
  - **Kind**: the selected monster's kind.
- **Monster Translations**: the names in all languages, with **Save
  Monsters**, **Import names from NpcName txt** and **Export as NpcName
  txt**, see
  [translation-system.md → In the monster editor](translation-system.md#in-the-monster-editor)
  and [Import and export of NpcName txt](#import-and-export-of-npcname-txt).
- **Monster Checks**: the checks of loading and saving, run on the monsters
  as they are now: a name with `||` or a language code that is not one
  (errors), an empty English name, a name longer than 64 characters and a
  name in a language the game has no UI texts for (warnings). With the
  shipped list it shows one warning, 252's empty name. The checks run when
  the window is first shown and on **Check**; after a change the window says
  that the monsters changed since the last check. Errors come first, and
  each kind can be hidden with its box. A click on a problem selects the
  monster and opens the tool of its field (Monster Translations for a name);
  an arrow key onto it only selects the monster. There is no check of the
  kind: a kind that differs from the old rules (566, 567) is not a problem.

In the browser, Monster Translations and Monster Checks, the arrow keys move
the selection (see [dev-editor.md](dev-editor.md)).

How changes work:

- **Every change goes into the monster table right away**, and the game
  uses it at once. Monsters and NPCs of that number on screen get a changed
  name at once, also after Add and Undo (not summons and Kalima gates, see
  [Names and translations](#names-and-translations)). Their kind stays until
  they appear again: a kind change, Add and Remove reach the monsters and
  NPCs that appear after it. After Remove, the ones on screen keep their name
  and kind until they appear again.
- A name is stored when you leave its field after a change, a kind right
  away. Each is one step of **Undo** and **Redo** (in the editor's bar, see
  [dev-editor.md](dev-editor.md); also Ctrl+Z and Ctrl+Y while a monster
  tool has the focus and no field is being typed in): the last 256 steps,
  for the whole session and across saves. Undo and Redo select the monster
  they change. A step is not undone when the monster changed in a way the
  history did not record; the console says so. The Monster Browser's title
  has a `*` while there are changes that are not saved.
- **Save Monsters** (in the Monster Browser and Monster Translations) runs
  the checks and writes `Data/Monsters/Monsters.json` next to the game. With
  errors nothing is written, and the problems are listed under the button;
  warnings do not stop it. The new file takes the old one's place only when
  it was written whole.
- **Revert** reads the file again: the changes since it was saved are lost,
  and so is the undo history. A file with errors is not taken; the monsters
  stay as they are and the problems are listed.

The game runs from the build folder, which has a copy of `src/bin/Data`.
To keep your changes, copy `<build folder>/Data/Monsters/Monsters.json` to
`src/bin/Data/Monsters` and commit it.

### Import and export of NpcName txt

The repository does not ship the original client's name files; the monster
list is the only monster name data. The import is there to bring in names
from your own old files. Both buttons are in Monster Translations, and both
work on `Data/Local/<Eng|Por|Spn>/NpcName_<Eng|Por|Spn>.txt` next to the game
(English, Portuguese and Spanish). The console names each file with its
whole path.

- **Import names from NpcName txt** sets the names of each file's language
  on the monsters that have an entry, by number. The whole import is one
  undo step.
  - Numbers without an entry are skipped with a warning; the import adds no
    monsters (Add monster in the Monster Browser does).
  - The Kalima gates (152–158) are skipped: their names in the old files
    were formats for the owner's name, which the game now builds from a UI
    text.
  - An empty name removes that language's translation. An empty English name
    and a name with `||` are skipped with a warning.
  - A name equal to the English one stays missing where the monster has no
    name of its own in that language, since the export writes the English
    name there. So an export followed by an import changes nothing.
  - A missing file is reported and skipped. A line that is not a name, a
    number outside 0–1023, a name that is not UTF-8 and a number that is in
    the file twice (the first name counts, as in the original client) are
    skipped with a warning. Names longer than 64 characters are cut, with a
    warning.
  - The console lists per language how many names were read, changed,
    skipped and cut.
- **Export as NpcName txt** writes the three files with the names of every
  monster with an entry, sorted by number, in English where a language has
  no name of its own (the console counts them). The Kalima gates are written
  with their plain names. A name containing `"` cannot be written: it is
  left out with a warning. A file that already has these names is not
  written; a file that changes is backed up first. A file that cannot be
  backed up is not written (an error in the console); the other languages
  are still written.

## Control socket

The [control socket](control-socket.md) reports each character's monster
number as `monster` (in `nearby` and in the objects of the `hit`, `killed`
and `view_*` events) and the kinds of this list (`monster`, `npc`, `trap`,
`other`; summons, Kalima gates and transformed players are `player`). A
`target` can be `{"monster": <number>}`. Since monster and NPC names follow the UI language,
scripts find NPCs by their number, not by their name.
