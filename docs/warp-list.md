# Warp list

The warp window (M), `/move` and the `/warp <map>` chat command use the warps
of `Data/Maps/Warps.json`, which follows Webzen's Season 6. The file holds
the list, the names, what a warp costs and what a character needs to warp.
It replaced the original client's `Data/Local/<Eng|Por|Spn>/movereq_<lang>.bmd`
files, one per data language.

## The file

```json
{
  "formatVersion": 1,
  "classLevelDiscount": [
    {"class": "magicGladiator", "numerator": 2, "denominator": 3},
    {"class": "darkLord", "numerator": 2, "denominator": 3},
    {"class": "rageFighter", "numerator": 2, "denominator": 3}
  ],
  "warps": [
    {"index": 23, "name": {"en": "Icarus", "es": "Ícaro", "pt": "Icarus"}, "command": "Icarus", "level": 170, "zen": 10000, "gate": 63, "needsOneOf": [{"tag": "flying"}, {"wings": true}], "notWith": [{"item": {"group": 13, "number": 2}}, {"tag": "transformationRing", "except": {"tag": "pandaOrSkeleton"}}], "blockedMessage": "YouCanEnterIcarusOnlyWithWingsDinorantFenrirr"},
    {"index": 33, "name": {"en": "PeaceSwamp", "es": "PantanodePaz", "pt": "PântanoPaz"}, "command": "Swamp_of_Calmness", "level": 400, "zen": 15000, "gate": 273, "noClassDiscount": true},
    {"index": 42, "name": {"en": "Vulcanus", "es": "Vulcanus", "pt": "Vulcanus"}, "command": "Vulcanus", "level": 30, "zen": 15000, "gate": 294, "pvpServersOnly": true, "gensOnly": true}
  ]
}
```

The order of `warps` is the order of the M window. Each warp is on one line.
Fields with their default value are left out.

| Field | Value |
|---|---|
| `index` | The warp's number, which the client sends to the server to warp there. 0 to 65535, as the warp request sends it in two bytes. Two warps cannot have the same index. |
| `name` | The name the M window shows, per language (`en`, `es`, `pt` today). English is required. |
| `command` | The name `/move` and `/warp` take besides the shown name (the old "sub name", English). One word. |
| `level` | The character level the warp needs. Some classes need less (see below). |
| `zen` | What the warp costs. |
| `gate` | The server's landing spot. The client does not use it; it is kept as information. |
| `noClassDiscount` | `true`: the class level discount does not apply (Swamp of Calmness, 33). |
| `minMasterLevel` | The master level it needs; 0 (the default) for none. |
| `minResets` | The resets it needs; 0 (the default) for none. |
| `pvpServersOnly` | `true`: closed on non-PvP servers (Vulcanus, 42). |
| `gensOnly` | `true`: on PvP servers only for Gens members (Vulcanus, 42). |
| `needsOneOf` | At least one of these items has to be equipped, not broken. Empty or left out: no rule. |
| `notWith` | None of these items may be equipped. |
| `blockedMessage` | The game text the chat shows when the equipment rules block `/move`. Left out: the general text. |

### Names

The M window shows the name in the UI language, as the monster names do. A
language without a name of its own shows the English one. When the UI
language changes, the list is renamed at once. With a server's list (see
below), a warp the server only has keeps the server's name.

The game holds up to 31 characters of a name; a longer one is cut, with a
warning.

### Level and the class level discount

`classLevelDiscount` lists the classes that need less level, for every warp.
A class needs the warp's level times `numerator` / `denominator`, cut to a
whole number. Magic Gladiators, Dark Lords and Rage Fighters need 2/3 of the
level: 113 instead of 170 for Icarus. A warp with `noClassDiscount` asks the
full level of every class: Swamp of Calmness asks 400.

`class` is the first stage of a class, as the item files write it
(`darkWizard`, `darkKnight`, `fairyElf`, `magicGladiator`, `darkLord`,
`summoner`, `rageFighter`). Each part goes up to 1000.

When a warp sets `minMasterLevel` or `minResets`, the M window shows the
master level and resets under the level, on a second line of the warp's row,
as game texts in the UI language (*Warp master level %d*, *Warp resets %d*),
e.g. "ML 50 R 2" in English or "NM 50 R 2" in Spanish; beside the level they
would run into the zen. No warp of the file sets them.

### Equipment rules

`needsOneOf` and `notWith` are lists of entries. An entry is one of:

- `{"item": {"group": 13, "number": 2}}`: one item (here the Horn of Uniria);
- `{"tag": "flying"}`: the items with an item tag, as the item files write the
  tags ([item-data.md](item-data.md)); with
  `"except": {"tag": "pandaOrSkeleton"}`, without the items of that tag;
- `{"wings": true}`: any wings (any item of the wing slot).

An item matches in any equipment slot. A needed item counts only when it is
not broken (durability above 0). Icarus (23) needs wings or a flying mount,
and is closed with a Horn of Uniria or a transformation ring other than the
Panda and Skeleton rings. Atlans 1-3 (11, 12, 13) are closed with a Horn of
Uniria or a Horn of Dinorant.

### Blocked message

`blockedMessage` is the key of a game text. The keys a warp may name:

- `YouCanEnterIcarusOnlyWithWingsDinorantFenrirr` (Icarus);
- `YouCannotGoToAtlansWhileRidingAUnicorn` (Atlans);
- `WarpBlockedByEquipment`, the general text "You can't warp there with your
  equipment.", which is also shown when the field is left out or empty.

The texts are in every language of the game texts.

### PvP and Gens

On a non-PvP server, a warp with `pvpServersOnly` is closed. On a PvP server,
a warp with `gensOnly` is marked in the M window and only Gens members can
pick it. On a non-PvP server `gensOnly` does nothing.

## What the M window checks

A warp is open when the character has the level (after the class discount),
the zen, the master level and resets, is no murderer, the server allows it
(`pvpServersOnly`), the equipment rules allow it, and, for a Gens warp, the
character is a Gens member. Otherwise the window greys it out. The control
socket's `warp` uses the same check.

## The server's list (OpenMU)

OpenMU sends its own warp list when the character enters the game (the
*WarpListExtended* message, only to this client). The warp window and `/warp`
use that list then:

- a warp which is also in the data file keeps its translated name, its place
  in the list and its rules, with the level, zen, master level and resets of
  the server;
- a warp which only the server has (like Crywolf or Loren Market on OpenMU) is
  added after them, with the name the server sends in the language of the
  account, its master level and resets, and no other rules;
- a warp which only the data file has isn't shown, because the server can't
  warp there.

The class level discount applies in both cases, as before.

Right after the list, OpenMU sends the minimum master level and resets of the
warps which need them (the *WarpRequirementsExtended* message, C2 F5 0B), and
nothing when no warp needs any. Like level and zen, these are the server's
with its list: a warp of the list without an entry needs neither, even if the
data file sets `minMasterLevel` or `minResets` for it. The M window greys out
such a warp and shows the master level and resets under its level ("ML n",
"R n" in English), as with the data file's
values; the server checks them again when the character asks for the warp.

The list of the server applies until the client leaves it (back to the
character list, to the login screen, or to another game server); then the list
of the data file is shown again until the next server sends its own. A server
which doesn't send a warp list, like Webzen's, gets the list of the data file.

## Chat: /move and /warp

`/warp <map>` takes the shown name or the `command`, in any case, and sends
the warp to the server.

`/move` goes to the server as chat. Before it is sent, the client checks only
the equipment rules of the warp the text names, and shows the warp's blocked
message when they block it. Level, zen and the other rules are the server's
to check. The warp is the first of the list whose shown name or `command` is
the word after `/move` or the warp word, in any case (as `/warp` finds it), so
`/move Atlans2` checks Atlans2, not Atlans. Otherwise the
check keeps the quirks of the original client:

- when no warp is that word, the warp is the first of the list whose shown
  name is somewhere in the text (case as written), or whose `command` is
  somewhere in it (any case);
- the warp word is looked for in the text in capitals, so a warp word in
  small letters (English `/warp`) never triggers the check; `/MOVE` is found
  in any case.

The chat check applies `except` as the M window does: with wings and a
Panda or Skeleton ring, `/move Icarus` is allowed.

## Saving the key configuration

The server keeps the potion keys (Q, W, E, R), auto attack, whisper sound,
slide help and the chat box size and transparency with the key
configuration. The client sends it when one of them changes (in game, once
the server's configuration arrived, or 2 seconds after the character joined
the map server and its map loaded: a new character has no stored
configuration, so the server sends none; the packets after the join wait
while the map loads), and before a warp from the M window, `/warp` or the
control socket. A save asked in those 2 seconds is sent once they are
over. `/move` goes to the server as chat and saves nothing first: the
settings were already sent as they changed, except a change in those 2
seconds, which still waits. When the stored configuration arrives first, the
save is dropped (the settings are then the stored ones), unless its chat
box byte stores no size or transparency: then nothing replaced the change,
and the save goes at once. A warp to a map on another game server ends the
session, so nothing is lost. Leaving the game, the session or the game
server saves only when a save would go at once; in the 2 seconds before the
configuration is known it saves nothing, so the reset (empty) keys never
overwrite the character's stored layout.

The join restores the chat box size and transparency from the stored
configuration (the original client read them and threw them away). Entering
the game first resets everything the key configuration holds: empty skill
keys and Q, W, E and R, auto attack
on, whisper sound off, slide help on, and the box at 6 lines at
transparency 0.6, so a character without a stored configuration doesn't
keep the one before's. The
client saves the lines the box shows; older clients saved the number of
chat messages in the log / 3 there, so a character last saved by one gets
an unrelated chat box size once, until the player changes it.

## Problems

The file is read at start, after the item list. A fatal problem stops the
game with a message, as the other data files do.

Fatal: invalid JSON or `formatVersion`, a value of the wrong form or out of
range (the problem names the warp and field, e.g. `warps[3].level`), a warp
without `name`, an English name or `command`, a malformed rule entry, and an
index two warps have.

Error: an item a rule names that the item list does not have (the entry
matches no item).

Warning: unknown fields, an unknown item tag (matches no item), an unknown
blocked message key (the general text is shown), a name longer than 31
characters (cut), a name or `command` with a space (`/move` takes one
word), a name or `command` that an earlier warp has as its name or
`command` in any case, in any language the M window shows (`/move` takes
the first), a class discount with numerator 0 or above its denominator, a rule
entry twice in its list, a tag entry whose `except` is its own tag, and a
blocked message on a warp without equipment rules (never shown).

## Editing (MuEditor)

**Windows > Warp List** shows the warps in the order of the M window. The
game takes each change at once: the warp table and the M window's list. It
has two tabs, **Warps** and **Translations**.

**Warps**:

- The bar: **Save**, **Revert**, **Add warp** (named `Warp<index>`),
  **Copy** (the copy's names and `/move` name get its index added:
  `Arena7`; a long one is cut so the whole fits in 31 characters). When
  another warp already has the new text as its name or `/move` name, in any
  case, `_2`, `_3` and so on is added. **Remove**,
  **Move up** and **Move down**.
- Left: a search and the list ("index name").
- Right, the picked warp: its index, its English name (required),
  **/move name**, level, zen, gate,
  **No class discount**, **Master level** and **Resets** (0 = none).
- **Rules**: **Needs one of** and **Not with**: items (with their names),
  item tags (with the number of items they match, listed on hover; **except**
  for Not with) or **Any wings**; **PvP servers only**, **Gens members only**
  and **Blocked message** (General or a warp message, with its text).
- **Class level discount**, the collapsing section at the bottom, edits the
  discount of all warps.

**Translations**: the names of all warps in every UI language, one column
each, as in the Monster Translations window: a search (index, `/move` name or
a name of any language), **Missing in** to list the warps a language has no
name for, and a grey English name where one is missing. A click on an index
picks the warp in the Warps tab. The English name can't be emptied here.

An index another warp has stays with that warp. Every change, and Add,
Copy, Remove and the moves, is a step of the editor bar's **Undo** and
**Redo** ("Add Warp7", "Icarus: Level"), as in the Server List window.

**Save** writes `Data/Maps/Warps.json` in the folder the game runs from. It
writes the same text the file has when nothing changed, and refuses a list
with fatal problems. Copy the file to `src/bin/Data/Maps` before committing.
**Revert** reads the file again.

**Data Checks** has an area **Warp List**: the list as the game and the
window have it, read back as the file would be, and its items checked
against the item list. A click on a problem opens the Warp List.
