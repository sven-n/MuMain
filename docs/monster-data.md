# Monster data

The monster list gives every monster and NPC number the client knows its
names, its kind and the model it appears with. It lives in
`src/bin/Data/Monsters/Monsters.json` and is loaded once at startup into a
table by number. It replaces the original client's name files
(`Data/Local/<Language>/NpcName_<Language>.txt`), which the repository no
longer ships.

The models are a list of their own, `src/bin/Data/Monsters/Models.json`: the
file, textures, animation speeds and sounds of each monster and NPC model
(see [Models](#models)). Which model a monster gets is still chosen by code
when it appears (its creator); a monster's `model` names the model the code
chooses, and a test checks that they agree. Its setup when it appears and its
attacks are still code, chosen by its number; later phases move them into the
monster data. The number and the names are what OpenMU calls a monster's
`Number` and `Designation`, so they can later be exchanged with the server.

Design and plans: [monsters design](superpowers/specs/2026-10-23-data-driven-monsters-design.md).

---

## Files

| File | What |
|---|---|
| `Monsters.json` | The monsters and NPCs: number, names, kind, model and attacks |
| `Models.json` | The monster and NPC models, see [Models](#models) |

```json
{
  "formatVersion": 1,
  "monsters": [
    {
      "number": 2,
      "name": {"en": "Budge Dragon", "es": "Dragón Escurridizo", "pt": "Dragão Lento"},
      "kind": "monster",
      "model": "MONSTER_MODEL_BUDGE_DRAGON"
    },
    {
      "number": 251,
      "name": {"en": "Hanzo the Blacksmith", "es": "Hanzo el Herrero", "pt": "Hanzo, o ferreiro"},
      "kind": "npc",
      "model": "MODEL_SMITH"
    }
  ]
}
```

- A monster is identified by its `number`, 0–1023: the number the server
  sends when a monster or NPC appears (packet 0x13 has 10 bits for it). Each
  number has at most one entry.
- Every entry has `number`, `name` and `kind`; `model` may be left out (every
  shipped entry has it), and so may `attacks`.
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
| `model` | The model it appears with: a slot of the model list (`MONSTER_MODEL_HOUND`, `MODEL_SMITH`), `player` (a player-shaped monster: the player model with class and items), `MODEL_WARCRAFT` or `MODEL_BALL` (models of other areas: the Kalima gates, the soccer ball), or `mapObject <number>` (a map object, a trap). The code still chooses the model; the field says which one, for the tools |
| `attacks` | Which animation the monster plays for the skill it attacks with, see [Attacks](#attacks); left out: attack 1, attack 2, attack 2 |

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

## Models

`Models.json` has one entry per model slot of the monster and NPC models.

```json
{
  "formatVersion": 1,
  "models": [
    {
      "slot": "MONSTER_MODEL_BULL_FIGHTER",
      "file": "Data/Monster/Monster01.bmd",
      "textureFolders": ["Monster"],
      "headBone": 20,
      "sounds": [
        {"file": "Data/Sound/mBull1.wav", "channels": 2, "spatial": true},
        {"file": "Data/Sound/mBullAttack1.wav", "id": "SOUND_MONSTER_BULLATTACK1", "channels": 2, "spatial": true},
        {"file": "Data/Sound/mBullDie.wav", "channels": 2, "spatial": true}
      ],
      "soundSlots": {
        "idle": ["Data/Sound/mBull1.wav", "Data/Sound/mBull1.wav"],
        "attack": ["SOUND_MONSTER_BULLATTACK1", null],
        "die": "Data/Sound/mBullDie.wav"
      },
      "linkBones": [42, 33]
    }
  ]
}
```

- `slot` names the model slot by the code's symbol. The **monster range**
  has the `MONSTER_MODEL_*` types (slot `MODEL_MONSTER01` + type): its
  models are opened when a monster with them appears and released when the
  map changes. The **NPC range** has the `MODEL_*` slots from `MODEL_NPC_BEGIN`
  on (`MODEL_SMITH`, `MODEL_MERCHANT_FEMALE_HEAD+1`). The code treats the
  two ranges differently, see [What the code adds](#what-the-code-adds).
- Models are sorted by slot and written in one way, as `Monsters.json`; a
  test checks that the shipped file is written this way, that every file and
  texture folder is there with exactly that spelling, and that each model's
  textures are in its folders.
- Files are `Data/...` paths in their spelling on disk; the game finds them
  in any case on Linux too.

### Model fields

| Field | Meaning |
|---|---|
| `slot` | The model slot, see above |
| `file` | The `.bmd` file |
| `textureFolders` | The folders below `Data/` its textures are searched in, in this order; at least one |
| `worldTextureFolders` | Other folders on some maps: `[{"worlds": [18, 19], "folders": ["NPC"]}]` (the Chaos Castle knights) |
| `textureNames` | A texture name that replaces the file's, by mesh: `{"0": "xmassanta1.jpg"}` (the Little Santas share one file) |
| `animations` | The speeds that differ from the range's defaults, in frames per game frame, by animation: `stop1`, `stop2`, `walk`, `attack1`, `attack2`, `shock`, `die`, `appear`, `attack3`, `attack4`, `run`, `attack5`, then the number (`"12"`). The monster range plays stop1 at 0.25, stop2 at 0.2, walk at 0.34, attack1 and attack2 at 0.33, shock at 0.5, die at 0.55 and the others at 0; the NPC range plays all at 0.25 |
| `holdLastFrame` | The animations that stop at their last frame instead of starting again. Without it: die in the monster range, none in the NPC range |
| `headBone` | The bone the head turns with; without it as the file has it |
| `streamMesh` | The mesh drawn as a stream; without it as the file has it |
| `noneBlendMeshes` | Meshes drawn without blending |
| `sounds` | The sounds the model loads: `file`, `channels` (how many can play at once), `spatial` (3D, louder near the hero), and `id` for a sound that keeps its `SOUND_*` number, see below |
| `soundSlots` | What the game plays when a character of the model stands or walks (`idle`, two), attacks or is hit (`attack`, two) and dies (`die`); of a pair it picks one at random. A slot names a sound of the model (its `id`, or its `file` when it has none) or any `SOUND_*` number; `null` is no sound. `secondSet` has the sounds a monster with SubType 9 (the big Hellas monsters) plays instead of the first set. Without `soundSlots` no slot has a sound |
| `soundEvents` | Sounds the game plays on top of the slots, see [Sound events](#sound-events) |
| `soundEventWorlds` | The maps the sound events play on; without it every map |
| `bitmaps` | Bitmaps loaded into fixed bitmap slots that the model's code draws with: `file` below `Data/`, `slot` (`BITMAP_MONSTER_SKIN`), `filter` (`linear` or `nearest`), `wrap` (`repeat` or `clamp`) |
| `parts` | Models loaded with it: `slot` (an NPC slot) or `effect` (an effect type of the effect catalogue), `file`, `textureFolders`, `animations`, `holdLastFrame` (no range defaults), and `worlds` for parts loaded with a map instead (the Kundun parts in Kalima, the Totem Golem's in Elbeland) |
| `boundingBox` | `min` and `max` of a character's box; without one -60, -60, 0 to 50, 50, 150 |
| `linkBones` | The bones the weapons hang on, right then left; without it 33 and 42. One bone sets the right hand's only |

A sound without an `id` gets a number of its own when it is first loaded,
after the `SOUND_*` list; the same file keeps the same number for the
session. A sound keeps its `SOUND_*` number (`id`) when code plays it by that
name, or when a slot of another model names it: a slot plays the sound of its
number, whichever model loaded it (the Giant Ogre's attack plays the Dark
Skull Soldier's `SOUND_MONSTER_BLACKSKULLATTACK` in Kalima). Some slots name
numbers that no model loads, as in the original client; they play nothing (a
test lists them).

### Sound events

`soundEvents` are sounds the game plays for a character of the model while it
is in one of the event's animations, when it starts an attack, is hit or
dies, at each step of a walk and at the end of each loop of stop1 and stop2,
and only within 500 units of the hero (the Empire Guardian bosses' steps):

```json
"soundEvents": [
  {"animations": ["walk"], "frames": [7, 8], "sounds": ["SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01"]},
  {"animations": ["walk"], "chance": {"oneIn": 100, "draw": "rand"}, "sounds": ["SOUND_RAKLION_ICEGIANT_MOVE"]},
  {"animations": ["stop1"], "pick": "rand", "sounds": ["SOUND_XMAS_SANTA_IDLE_1", "SOUND_XMAS_SANTA_IDLE_2"]},
  {"animations": ["die"], "deathEffectArmed": true, "sounds": ["SOUND_XMAS_SNOWMAN_DEATH_1"]}
],
"soundEventWorlds": [69, 70, 71, 72]
```

| Field | Meaning |
|---|---|
| `animations` | The animations it plays in |
| `frames` | Only while the animation's frame is from the first number to below the second |
| `deathEffectArmed` | Only while the character's death effect is armed |
| `chance` | Plays with a chance of one in `oneIn`, drawn as `draw` says: `rand` (`rand() % oneIn == 0`) or `randFps` (the original client's `rand_fps_check`, whose chance depends on the frame rate) |
| `pick` | Plays the first of the two sounds when a draw of one in two comes up, else the second (`rand` or `randFps`) |
| `sounds` | The sounds, played together (one of the two with `pick`), each named as a slot names it |

One call can play several events, in their order; their draws come in that
order too.

A model loads the sounds its events play, like its other sounds, when the
first monster of it appears (the Empire Guardian monsters' too; the map
loads only its ambient sounds). A sound that another model or the map
already loaded from the same file, with the same channels and 3D setting,
is not loaded again, so models that share a sound list it alike (a test
checks both).

Not all monster sounds are data yet: the monsters of Aida, Crywolf, Kanturu,
the Hunting Ground, the Balgass barracks, Illusion Temple and a few others
(among them 340 to 365, 386 to 403 and 409 to 412) play their sounds from
their map's per-frame drawing code, and their maps load them; Monster Model
shows no sounds for their models. They move into sound events with the
monsters' looks (design, MN8).

### What the code adds

On top of the model list the code does (the Monster Model and Monster
Preview tools show it):

- **Monster range**: a character of the model dies with a blood pool and
  stays dead, goes back to stop1 after stop2, shock and attacks 1 to 5 (and
  after appear for Maya's hands and Selupan), and lands the hit of attacks
  1 and 2 at frame 5.
- **NPC range**: some NPCs pick their next animation in code (among them the
  Smith, the wedding NPC, Silvia and Rhea).
- **Speed factors**: a player transformed into a monster plays attacks 1 and
  2 at 1.5 times their speed; the Illusion of Kundun plays die 4 times as
  fast after frame 6; Erohim plays die at half its speed.

A model whose file is missing has no body: the monster appears without one
and the game keeps running; the file is logged and listed by Monster Checks.
Molt's `Data/Monster/Monster50.bmd` is missing, here and in the original
client (OpenMU has no monster 68).

## Attacks

A monster entry's `attacks` say which animation the monster plays for the
skill it attacks with (Selupan's):

```json
"attacks": {
  "monsterSkills": {"34": "attack2", "35": "attack1", "36": "attack3", "37": "appear", "41": "none"},
  "otherwise": "attack4",
  "clearSkill": "afterAttack"
}
```

| Field | Meaning |
|---|---|
| `monsterSkills` | The monster skills (packet 0x69) and the animation each plays |
| `skills` | The player skills (packets 0x19 and 0x1E) and the animation each plays |
| `otherwise` | What any other skill, a stale skill or a normal attack plays; `attack1` when left out |
| `otherwiseRandom` | In place of `otherwise`, a draw of the original client: `rand` (attack 1 or 2, half and half), `randFps` (attack 1 or 2 with a chance that depends on the frame rate) or `bali` (Bali's draw of attack 1 to 4) |
| `clearSkill` | When the monster skill is cleared: `never` (when left out), `onMatch` (an entry named it), `onNoMatch` (no entry named it), `always` (right after the animation was chosen) or `afterAttack` (the skill plays only the attack its packet starts and stays while that attack plays; it is cleared when the animation ends, when a hit's shock ends attack 3 to 5 or appear early, at once when the entry plays `none`, and by any other attack, which is a normal one also while the skill's attack plays) |
| `sound` | A `SOUND_*` sound the attacks play each time they play (the traps') |
| `soundAtMonster` | The sound plays at the monster (3D) instead of everywhere alike |

An animation is a name of the model's animations (`attack1` to `attack5`,
`appear`, `shock`, `stop2`, …) or `none`: the animation that plays goes on.
The monster skill's entry comes first, then the player skill's, else
`otherwise`. The player skill is never cleared.

### When the game plays an attack

What the server sends:

- **0x18** (a normal attack) plays an attack.
- **0x69** (a monster skill) stores the monster skill and plays an attack
  for the skills 1 to 77 (`ATMON_SKILL_END`); others it only stores.
- **0x19** (a player skill) stores the player skill and plays an attack only
  for some skills (200 to 202, the Lightning to Flame group, Fire Slash,
  Party Teleport, Chaotic Diseier). Heals and buffs play attack 1, attack 2,
  attack 2 without the attacks; other skills play nothing. A player skill's
  entry then plays on the next 0x18, which reads the stored skill.
- **0x1E** (an area skill) stores the player skill and plays an attack.

Playing an attack (`SetPlayerAttack`) for a monster:

1. A player-shaped monster (model `player`) plays a player attack chosen
   from its equipment; its `attacks` do nothing.
2. The traps (`mapObject 39`, `40` and `51`) play their `attacks` and their
   effects and sounds; Bali plays its `attacks` (its draw).
3. Else the map handlers are asked in a row, each for the monsters and maps
   it knows: Hellas (any map), the Battle Castle guards (by model, any map),
   the Hunting Ground, Aida, Crywolf and Devil Square, Kanturu 1st, 2nd and
   3rd, the Balgass barracks and refuge, Illusion Temple, the Swamp of Quiet
   (by model), Raklion, and Empire Guardian 1 to 4. A handler that answers
   for the monster plays its `attacks` and keeps what its code adds: the
   guards' AI, Maya's scene, Selupan's height, sounds and effects, and the
   rage of the Empire Guardian bosses (in a rage a boss plays appear).
4. If none answered, the generic pattern plays: attack 1, attack 2, attack 2
   by a count of the character.

So `attacks` work where a handler answers for the monster: the Monster
Attacks tool says which one, and Monster Checks lists the attacks no handler
reads (a monster moved to another model loses a model-keyed handler).

The game counts only attack 1 and attack 2 as attacks: a hit plays shock
over any other animation, and only they have a hit frame. After appear, only
Maya's hands and Selupan go back to stop1; other models play it again
until their next action. Monster Checks warns about both.

## Loading and errors

At startup `Data/Monsters/Monsters.json` is read and checked, and after the
effect catalogue `Data/Monsters/Models.json`.

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

In the model list, errors (they stop the start with "Monster data error" too,
naming the model list) are a
value of the wrong form (each names the model and the field), a slot that is
no model slot of the two ranges or is there twice, a sound slot that names no
sound of the model and no `SOUND_*` number, a sound `id`, bitmap slot or
part slot that does not exist, and a part `effect` that is not an effect
type. Unknown fields and a model or part file that is not in the client
folder are warnings; the shipped list has one, Molt's file.

An automated test loads the shipped monster list (it expects only the
warning about 252) and the shipped model list (only Molt's file), so a pull request with broken monster data fails its
checks. With a test build (`-DBUILD_TESTING=ON`):

```bash
ctest --test-dir <build folder> --build-config Debug -R monster --output-on-failure
```

## Editing monsters (MuEditor)

The monster tools (editor builds, F12, **Windows → Monsters**) edit the
monsters of the running game: **Monster Browser**, **Monster Model**,
**Monster Preview**, **Monster Attacks**, **Monster Translations** and
**Monster Checks**. They
share one selected monster: selecting it in one tool shows it in the others.
In the first layout (also after **Windows > Reset Layout**) the Monster
Browser is a tab beside the Item Browser and the others are tabs beside the
item tools.

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
  - **Kind**: each row has a combo with the monster's kind; picking another
    kind changes it (one undo step) and selects the monster.
- **Monster Translations**: the names in all languages, with **Save
  Monsters**, **Import names from NpcName txt** and **Export as NpcName
  txt**, see
  [translation-system.md → In the monster editor](translation-system.md#in-the-monster-editor)
  and [Import and export of NpcName txt](#import-and-export-of-npcname-txt).
- **Monster Model**: the model of the selected monster, from the model list:
  its slot and range (hover over the range for what the code does with it),
  the monsters with the model (a click selects one), and
  - **Model file**: the file, with a search over the `.bmd` files below
    `Data/Monster` and `Data/NPC` and in the other folders the model list
    uses. Picking one changes the file; a model whose texture folder was its
    file's folder takes the new file's folder.
  - **Texture folders**: remove one (not the last) or add one.
  - **Animations**: one row per animation of the model: its speed (drag it;
    **Default** gives back the range's), whether it holds its last frame, and
    what the game adds in game (a speed factor, the next animation, the hit
    frame).
  - **Sounds**: the model's sounds, each with **Play**, and the five sound
    slots, each set to a sound of the model or to no sound.
  - **Sound events**: a row per [sound event](#sound-events): its
    animations (one or more), frames, whether the death effect must be armed,
    a chance of one in n, a pick of one of two sounds, and its sounds, picked
    from the model's own; **Play** plays them, **Remove** removes the event.
    **Add a sound event** adds one for attack1 with the model's first sound
    (a model without sounds cannot have events). **Maps** lists the maps the
    events play on (empty: every map).
  - **More of the model entry**: the values the tool shows but does not edit
    (head bone, stream mesh, part models, bitmaps, box, weapon bones); they
    are edited in the file.

  A model is shown once the game has it open; the tool opens it when it
  shows it (as if a monster of it had appeared). A monster whose model is
  outside the model list (`player`, a map object) says so.
- **Monster Preview**: the selected monster's model, turning if you like,
  with the item preview's camera: drag to turn it, the middle button moves
  the picture, the wheel zooms, a double click puts it back. Pick any
  animation; **Play**, **Pause**, **From the start**, and a frame slider to go
  frame by frame. The speeds are those of Monster Model, also while one is
  being dragged there. The model's slot sounds play when their animation
  starts (an idle sound for stop and walk, an attack sound for attacks and
  shock, the die sound). The hit frame of attacks 1 and 2 is marked on the
  slider, and the line under it says what the game adds. **The game's speed
  factors** plays them (**As a transformed player** too). The preview is
  only in the editor's window.
- **Monster Attacks**: the [attacks](#attacks) of the selected monster, and
  the handlers that read them, with their maps (or that none does). **Add
  attacks** gives a monster without them attacks that play attack 1;
  **Remove the attacks** gives it the generic pattern again. Each table
  (monster skills, player skills) has a row per skill with its animation and
  **Remove**, and a number field with **Add**; a monster skill that 0x69
  plays no attack for is marked. **Any other attack** is an animation or a
  draw, and **Clear the monster skill** says when the skill is cleared.
  **Sound of the attacks** is a `SOUND_*` id they play each time (the traps'),
  **At the monster (3D)** plays it there; a name the monster data does not
  know is not stored, and the field says so. The
  animations offered are those of the open model with a speed above 0 (the
  tool opens it); while it is not open, stop1 to attack5. Each change is one
  undo step and reaches the monsters' next attack. A player-shaped monster
  shows a note instead.
- **Monster Checks**: the checks of loading and saving, run on the monsters
  and models as they are now: a name with `||` or a language code that is not one
  (errors), an empty English name, a name longer than 64 characters and a
  name in a language the game has no UI texts for (warnings), and the checks
  of the model list's load (a model or part file that is not in the client
  folder is a warning), and the checks of the attacks (warnings): attacks
  that no handler reads or of a player-shaped monster, a monster skill that
  0x69 plays no attack for, and each animation the attacks can play that the
  open model lacks, whose speed is 0, after which the model does not go back
  to stop1, or that is not attack 1 or 2 (see [Attacks](#attacks)). With the
  shipped lists it shows 50 warnings: 252's empty name, Molt's missing file,
  and 48 of the attacks (the bosses' appear and attack 3, Bali's draw, the
  Battle Castle guards' shock and the like). The checks run when
  the window is first shown and on **Check**; after a change the window says
  that the monsters changed since the last check. Errors come first, and
  each kind can be hidden with its box. A click on a problem selects the
  monster and opens the tool of its field (Monster Translations for a name,
  Monster Attacks for the attacks);
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
- A name or a speed is stored when you leave its field after a change, a
  kind, a file, a folder, a held last frame or a sound slot right away. A
  changed model is opened again before the next frame, with its textures and
  sounds, and the characters of the model keep playing (an animation it no
  longer has goes back to the first). Each is one step of **Undo** and **Redo** (in the editor's bar, see
  [dev-editor.md](dev-editor.md); also Ctrl+Z and Ctrl+Y while a monster
  tool has the focus and no field is being typed in): the last 256 steps,
  for the whole session and across saves. Undo and Redo select the monster
  they change. A step is not undone when the monster changed in a way the
  history did not record; the console says so. The Monster Browser's title
  has a `*` while there are changes that are not saved.
- **Save Monsters** (in the Monster Browser and Monster Translations) runs
  the checks and writes `Data/Monsters/Monsters.json` and
  `Data/Monsters/Models.json` next to the game. With
  errors nothing is written, and the problems are listed under the button;
  warnings do not stop it. The new file takes the old one's place only when
  it was written whole.
- **Revert** reads both files again: the changes since they were saved are
  lost, and so is the undo history. A file with errors is not taken; its
  monsters or models stay as they are and the problems are listed.

The game runs from the build folder, which has a copy of `src/bin/Data`.
To keep your changes, copy `<build folder>/Data/Monsters/Monsters.json` and
`Models.json` to `src/bin/Data/Monsters` and commit them.

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
  with their plain names. A name containing `"` or a line break cannot be
  written: it is left out with a warning. A file that already has these names is not
  written; a file that changes is backed up first. A file that cannot be
  backed up is not written (an error in the console); the other languages
  are still written.

## Control socket

The [control socket](control-socket.md) reports each character's monster
number as `monster` (in `nearby` and in the objects of the `hit`, `killed`
and `view_*` events) and the kinds of this list (`monster`, `npc`, `trap`,
`other`; summons, Kalima gates and transformed players are `player`). A
`target` can be `{"monster": <number>}`, the nearest living monster or NPC
of that number (traps, `other` objects and players are not found by it).
Since monster and NPC names follow the UI language, scripts find NPCs by
their number, not by their name.
