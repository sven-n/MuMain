# Data-Driven Monsters and NPCs Design (WIP)

> **Draft, 2026-10-23.** Area MN of the
> [data-driven content roadmap](2026-09-29-data-driven-content-roadmap-design.md).
> Written from the code of MuMain `upstream/main` @ `077c954a4` and OpenMU
> `master` @ `d54b6b24b`. Line numbers are those of `077c954a4`. On the later
> main (`c7915f44b`: the Server List window, the shared editor undo, the item
> descriptions and tooltip texts) `ZzzOpenData.cpp` moves by +11 lines after
> its line 387, `GMHellas.cpp` by −2 after its line 27 and `ZzzInterface.cpp`
> by −4 after its line 3621; no other file the design cites by line changed.
>
> The owner agreed to the proposals of Q-MN1 to Q-MN15 on 2026-10-23 (see
> "Decisions" and "Agreed questions"). A review in two rounds (seven areas
> each, every finding checked by a second reviewer, then a completeness check)
> and a final check of this text corrected and completed the first draft;
> Q-MN8 to Q-MN15 are the questions it raised. The owner chose three PRs for
> the work that can start now (see "Pull requests and phases"). Q-MN16 to
> Q-MN19 were answered on 2026-10-25, at the start of PR 2, and Q-MN20 to
> Q-MN25 the same day, after MN3. Q-MN35 to Q-MN63 were answered on 2026-10-27
> and 2026-10-28, at the start of PR 3: the work after PR 2 now runs as five
> PRs (see "Pull requests and phases"), and D61–D66 are new. Line numbers in
> Q-MN35–Q-MN63, D61–D66 and P23–P27 are those of `938349ec4`.
>
> The counts come from reading and searching the code, not from a compiler.
> Where code is sorted into kinds, a count can be off by a few.

## Goal

Replace hardcoded monster and NPC knowledge in the client with **monster data
plus code for the behaviour that stays special**, as the items did:

- What the client knows about a monster number (name, kind, model, size,
  equipment, which animation an attack or a skill plays) lives in data files,
  is loaded once into a table indexed by monster number, and the game reads
  that table instead of `switch` statements.
- What belongs to a **model** (its file, texture folders, the speed of each
  animation, its sounds, its bounding box and weapon bones) lives in model
  entries that monsters reference, so several monsters share one model as
  today.
- Runtime lookups stay as fast as the hardcoded code: an array read per
  monster number or model slot, no names looked up while playing.
- Every field can be edited in **focused MuEditor tools that share one monster
  selection**, like the item tools.
- Monster names are **translatable** like item names.
- Data that client and server share (number, name) can be exchanged with
  OpenMU by file, like the items (D2, D4).

## Scope

In scope: monsters and NPCs as the server sends them and everything the client
sets up for them: their names, kind, models, animation speeds, sounds, setup at
spawn, and which animation an attack or a skill plays. The ways they arrive:

| Path | Where | What it does after `CreateMonster` |
|---|---|---|
| 0x13 monsters and NPCs | `ReceiveCreateMonsterViewport`, `WSclient.cpp:2898` | Facing angles for 10 model types (Ballista by tile, Kanturu 2nd entrance NPC, 6 Little Santas, Xmas Santa, Julia), which overwrite any facing a creator set (`:2957`); outline 1.0 for the Kanturu 2nd entrance NPC (`:2953`); the Nightmare teleport effect (`:3003`); no path finding on the Doppelganger maps (`:3015`); the Castle Gate state; MU Helper targets; buffs; `AppearMonster` (17 cases). The number is the low 10 bits; bits 0x70 and 0x80 of the high byte are server flags (build time, own or friendly). |
| 0x1F summons | `WSclient.cpp:3034` | Kind = player for every summon and Kalima gate (`:3072`); level 1 for murderer owners (`:3075`); the "name (owner)" format. Kalima gates (152–158) go to `CreateHellGate` (`ZzzCharacter.cpp:15142`), which sets their tier in `Level` and `SubType` 1 for gate 7 (`:15153`); a new gate restarts at frame 0 with the falling-stone sound (`WSclient.cpp:3097-3102`), overriding the creator's frame 10. |
| 0x45 transformed players | `WSclient.cpp:2680` | Giant scale 0.8 instead of its 1.6 (`:2742`), cloth deleted (`:2747-2757`), kind = player (`:2769`), the player's name. |
| 0x68 castle siege preview | `ReceivePreviewPort`, `WSclient.cpp:11690` | Random angle, deleted after 150 frames, its own Castle Gate rule. |
| Cinematics | `CDirection::SummonCreateMonster`, `CDirection.cpp:162` | Weapons removed, alpha 0, keys from `NUMOFMON` (10000), its own monster → model switch, and it writes the walk speed of the shared model for Balgass, Dark Elf and Nightmare (`CMVP1stDirection.cpp:207-211`, `CKANTURUDirection.cpp:351`). |

Monster and NPC models are also used without a monster: the ambient dragons
and the Kalima Bahamut (`GOBoid.cpp:847`, `GMHellas.cpp:639`), the Battle
Castle guard statues, the Devias donkey and flag, the Chaos Castle angel, the
Kanturu trap canon, the Blood Castle box and the Xmas snowman effect. The
Elbeland map objects 133–155 that create monsters and fighters
(`GMNewTown.cpp:300-600`) never run with the shipped maps; the Monster
Browser's source references mark these lines as unreachable. The login and
character select scenes create no monsters.

Not in scope here, but referenced:

- **What a skill is made of** (effects, models, sounds, the code that handles
  it) belongs to the skill areas SK1 and SK2 of the roadmap. The monster tools
  show which skills a monster uses and which animation each plays; the skill
  tools list the monsters that use a skill (D26).
- **Looks** (glow, render styles, per-frame effects, cloth, death debris) use
  the look format of items phase 13 (D25); MN8 moves them.
- **Effect types** are named by the effect catalogue (FX1, done). The part
  models that monsters drop when they die are effect types of it.
- **NPC dialogues and quests** (the roadmap's "later" area): `NPCDialogue.bmd`
  (keys are NPC number × 0x10000), `QuestProgress.bmd`, `Quest_<lang>.bmd`
  (quest NPCs 229, 235, 406 and kill targets 409–412) and
  `QuestWords_<lang>.bmd`. For OpenMU's `NpcWindow.NpcDialog` (257, 543, 544,
  566, 568) the server sends the NPC number (F9-01) and the client picks the
  dialogue by number; OpenMU warns that a client without that dialogue
  crashes.
- **The minimap's NPC markers** (`Data/Local/<lang>/Minimap/Minimap_<map>_<lang>.bmd`,
  67 NPC markers in 9 maps) hold their own NPC names per language, without a
  monster number. They stay until the map area; a later phase can link them to
  monster numbers.
- **NPC names as UI texts**: window titles and event texts name NPCs and
  monsters through `.resx` texts in all 11 UI languages (Trainer, Elpis, Seed
  Master, Delgado, Titus, Lugard, the Messenger of Archangel, the Werewolf
  Guardsman, the Gatekeeper; Blood Castle and Crywolf counters). They stay UI
  texts; the Monster Translations tool can offer them as suggestions.
- **Pets and mounts** are the roadmap's "later" area.

## Decisions

Numbers continue after the roadmap's D43. D44–D52 were the first draft's
proposals; the review added D53–D60. Agreed on 2026-10-23 through the
questions: the phase order (Q-MN1), D48 with its map gates and `clearSkill`
(Q-MN2, Q-MN13, Q-MN14), D46 (Q-MN3), D47 (Q-MN4), D51 with the P12
exception (Q-MN5, Q-MN11), D52 with the name language and the txt import and
export (Q-MN8, Q-MN9), D53 (Q-MN6, Q-MN12), D55 (Q-MN10), the entries for
658–668 (Q-MN7) and the bodies of 523 and 658–668 (Q-MN15). D44, D45, D49,
D50, D54 and D56–D60 were not asked as questions; they stand as written
unless the owner changes them. D61–D66 and the changes to D44, D46, D48, D49,
D55 and D60 come from Q-MN35 to Q-MN63 (2026-10-28).

| # | Topic | Decision |
|---|---|---|
| D44 | Identity | A monster is its **monster number**. Entries exist for 0–1023: 0x13 sends 10 bits. 0x1F, 0x45, 0x68 and the quest and NPC dialogue packets carry 16 bits, and players have the number -1 (`MONSTER_UNDEFINED`), so **every lookup goes through one accessor** that checks the range and keeps today's fallbacks for a number outside 0–1023 or without an entry: `getMonsterName`'s `"()"` (`Setting_Monster` leaves the name as it is), the Bull Fighter body (P2; MN4 replaces it with the bug look of D64), the kind the old number ranges give. Such a number is logged once as a warning. No string key, as items (D9). Logs show `<English name> (<number>)`. |
| D45 | Monster and model | Two kinds of entries. A **monster** (per number): names, kind, the model it uses, its setup at spawn, its attack animations, its NPC rules. A **model** (per model slot): file, texture folders, animations, sounds, head bone, bounding box, weapon link bones. Values that `BMD` has a member for are written into `Models[]` when loading; the bounding box and the link bones go into a table by model slot, read with one array read in `CreateCharacterPointer`, where only the slot is known. Slots without an entry (`MODEL_PLAYER`, the faces) keep today's defaults. |
| D46 | Files *(agreed)* | `Data/Monsters/Monsters.json` and `Data/Monsters/Models.json`. UTF-8, LF (`.gitattributes`), `formatVersion`, sorted by number or slot. The folder is re-included in `.gitignore` (`src/bin/**` is ignored) and its `.tmp` files ignored, as `Data/Items`. Unlike items (D22), a monster's shared fields (number, names) and its client-only fields share one file, so a monster's data stays in one place; the OpenMU exchange (MN6) reads and writes only the shared fields and leaves the others untouched. Problems have three levels (D65): fatal ones stop the game like the item data, errors and warnings are logged and listed in the editor. |
| D47 | Model list *(agreed)* | Monster and NPC models get a list of their own, `Data/Monsters/Models.json`. A model entry names its slot by an enum symbol, as FX1 names its types (D28), while code still names the slots: the model type in the monster range (`MONSTER_MODEL_ICE_WALKER`, slot `MODEL_MONSTER01` + type), the slot itself in the NPC range (`MODEL_SMITH`). A new model entry has no enum symbol: it has a name of its own, and the loader gives it a free slot of the monster range (400 slots, 217 used); the NPC range has no free slot. Code that behaves differently for the two ranges (die hold and blood pool, return to idle, the attack impact frame, unloading) is listed in the entry's tool, and Monster Checks warns when a monster of kind "monster" uses an NPC-range model. A monster's `model` may also name: `player` (`MODEL_PLAYER` with class and items), a model of another area by its name (the Kalima gate's `HellGate`, the soccer `Ball`), or a map object slot (traps). The monster loader never loads or frees slots of other areas. Part models are FX1 effect types, named by their effect name. |
| D48 | Attack animations *(agreed, refined)* | Each monster lists **which animation each skill plays**: monster skills (packet 0x69, `CHARACTER::MonsterSkill`) and player skills (packets 0x19 and 0x1E, `CHARACTER::Skill`), each read where the monster's handler reads it today. Values: `attack1`–`attack5`, `appear`, `shock`, and `none` (no animation, the current one goes on). `otherwise` is what any other skill, a stale skill or a normal attack plays: the code cannot tell these apart. `otherwiseRandom` (`rand`, `randFps`, Bali's draw) replaces `otherwise` with one of the old random choices; an entry has one or the other. A monster without `attacks` keeps the generic "attack 1, attack 2, attack 2" pattern. The attacks PR (Q-MN62, Q-MN64) makes the data decide on every map: any number with `attacks` plays them, so the handlers' lists of numbers and their map checks for the animation go; what differs by map is a map variant (D62, Q-MN65). `otherwise` may also be a list of animations played in turn (a rotation with its own count per monster) or `generic`, today's pattern by `SwordCount` (Q-MN69); Monster Attacks shows `generic` as the default of a number without `attacks`. Transformed players (0x45) keep the form's number and model, so its `attacks` apply to them too. `attacks` applies only to monster-shaped models: `MODEL_PLAYER` monsters choose a player attack from their equipment. Map code that is no choice of animation stays (Q-MN13: the Empire Guardian rage and Berserker skill, Selupan, Maya's scene, the Battle Castle guards' AI), and so do the generic rules that count only attack 1 and attack 2 as attacks (shock on hit, the impact frame). This replaces the positions of `MonsterSkill.bmd`, which cannot express what the map code does. |
| D49 | Code stays for logic | Data says *what* is used; code stays for what a monster *does* beyond that (rage states, the Maya scene, the Crywolf altars, spawn effects, random model choices). Such code is a named **hook** the entry names, so a reader sees in the data that a monster has special code, and a copied monster brings the hook along only through that field. Refined by Q-MN36 and Q-MN46: **data first**, a hook only where logic hands over to code, with the values it uses editable; an entry lists its hooks (`"hooks": ["RaklionBossDrop"]`). Spawn effects, random choices, map and event values, facings and talk dialogs are data (D62, D63, D66). Hooks left: `Kanturu2ndEntrance` (367), `EmpireGuardianRageReset` (504–511, 522), `RaklionBossDrop` (459), `IllusionTempleStorageReset` (383, 384), `CastleGateTerrain` (277), `CrywolfAltarContract` (205–209). The editor adds and removes known hooks and shows what they do; it cannot write hook code. |
| D50 | One phase, one comparison | Every phase that deletes code compares the old and the new code for every case before the old code goes: MN1 with a one-time test, MN2–MN5 and MN7 with a committed **monster recorder** like the effect recorder of FX1 (D40), MN8 with the look recorder of items phase 13. Section 10 says what it records and how it runs in a test binary. Known bugs are kept in the move and fixed in separate commits (D51). |
| D51 | Known bugs *(agreed)* | Found while reading (see "Problems found"). The move keeps the old behaviour; each fix is a commit of its own, after the comparison, decided one by one by the owner. A loader that writes a value the code never wrote (e.g. resets a field the code leaves alone) is such a fix too. Exceptions: P12 is fixed before MN1's comparison (Q-MN11), and 658–668 get OpenMU's English names in MN1 (Q-MN7). |
| D52 | Names *(agreed, Q-MN8, Q-MN9)* | Names by language in the monster entries (`{"en": …, "es": …, "pt": …}`, English first), like item names (D17), from `NpcName_<lang>.txt`; the txt files are removed. Names are cut at 64 UTF-16 units (`MAX_MONSTER_NAME`) as today, and the loader now logs a warning for each cut name. Names follow the **UI language**, no longer `[LOGIN] Language` (`g_strSelectedML`), with English where a name is missing; this is a visible change for players whose two settings differ. Live monsters and NPCs get the new name when the language changes; transformed players, summons and Kalima gates, whose names are composed, get it at their next spawn. The TranslateItemNames option stays for item names; the transformation ring's item name follows it. Monster Translations can import names from `NpcName_<lang>.txt` (by number and language, cut at 64) and export them as such files, as items keep their bmd import and export (D10). Names stay in per-number storage that does not move while the game runs, so pointers from `getMonsterName` stay valid. |
| D53 | Names of OpenMU *(agreed, Q-MN6, Q-MN12)* | The client's English names stay. OpenMU's English names differ for 222 of the 478 numbers both have (about 106 are numbered variants like "Chief Skeleton Warrior 1", 12 differ in case, about 104 really differ); the exchange shows them on a known-differences list, as the import of items phase 7 does, and takes none over by default. Only the client's UI languages are taken (German: 64 numbers, 57 without the Kalima gates); zh-CN is neither imported nor removed on export, and OpenMU's `@src`/`@stamp` entries are kept. The Kalima gates (152–158) stay out of the name exchange. |
| D54 | Kind | The client's kind (monster, NPC, trap, other) stays client data: it decides click to talk or attack, the auto-attack target, the outline colour, the chat and name window colour, MU Helper targets and idle chat. It is **not** exchanged with OpenMU, whose `ObjectKind` controls server behaviour and maps to several client kinds (see "Server"). Monster Checks compares the two with a table of allowed pairs. |
| D55 | Editing a running game *(agreed, Q-MN10)* | Model changes reopen the model slot with the current map's texture folders and reload its sounds, as the item tools reopen changed models; running animations are clamped to the new action count. Monster changes reach live monsters of that number at once: attacks are read when used; names and setup values are written again into the live characters of that number (not the composed names of transformed players, summons and Kalima gates; Q-MN8, Q-MN10). Refined by Q-MN39: values a living monster can take change **at once** (scale, skin, outline, render tier, meshes, light, shadow, alpha, move speed, no turn, no blood, PK colour, equipment, fixed facing, the variant values of the current map and state); the others at the **next spawn** (model, offsets, start action and frame, bounding box, named bones, spawn effects, random picks, spawn buffs and sound, shown, appear, hooks, death effect; facing step and facing by tile at the next position packet). Monster Setup badges each field, marks waiting changes, and has a **Respawn live monsters** test button (same key and slot; state noted and restored; what the last spawn attached removed; optional "Play appear"; the server is not told). |
| D56 | Editor parts | The monster tools use the editor bar's shared undo (`MuEditor::UndoTarget`, upstream since sven-n/MuMain#845) with a history of their own, and parts made generic from the item tools: the names × languages grid, the tool window `Begin` with its undo keys, the docking group and a Windows sub-menu. |
| D57 | File names | Model files are written as `Data/...` paths in their on-disk spelling, as `SharedModels.json` does. 52 of the 209 `Monster<N>.bmd` files are spelled differently on disk (`MONSTER158.bmd`, `monster101.bmd`) and the NPC folder is `NPC`; checks and the file picker resolve case on Linux (`MuResolvePath`), and a test checks that every file and texture folder in `Models.json` exists with exactly that spelling and that each shipped model's textures are found in its listed folders. |
| D58 | Documentation | `docs/monster-data.md` (files, fields, names, editing, how the attack animations are chosen) starts in MN1 and grows with each phase; the monster tools go into `src/MuEditor/README.md` (coding rule 11). The phases that make existing docs wrong update them: `docs/translation-system.md` (monster names replace `NpcName_<lang>.txt` in MN1; German names with MN6) and `docs/dev-editor.md` (the bar's undo for the monster tools). |
| D59 | Control socket and tests | The control socket's `nearby`, `hit` and object descriptions get the monster number and name the kinds trap and other (`KIND_TRAP`, `KIND_TMP`), `target` accepts a number, and the in-game test scenarios find NPCs by number instead of by their shown name (MN1). |
| D60 | Item references | Monsters wear items by item type. Once MN4c moves those lines out of the code, the item tools' "used by" and their move and remove dialogs list the monsters that wear an item; an item move updates the monster entries that name the item in the same undo step of the bar; Monster Checks reports item types that no longer exist. Refined by Q-MN41: the move **asks** ("Update them to the new number as well?") and lists the wearers (slot, random, map or state variant); yes puts the move and the updates into one step of the item history; no leaves them as errors (D65). The item tools' Save writes the item files and only those monster references, into `Monsters.json` as last saved; other unsaved monster edits stay unsaved. A monster naming a missing item is an error, and Monster Equipment marks the slot. |
| D61 | One monster, several numbers *(agreed, Q-MN43, Q-MN49–Q-MN52)* | A monster entry holds the values its numbers share; `numbers` lists each server number with only the values it changes (any field may be a number's own) and the maps it appears on (`maps`, map numbers, Q-MN61). Every monster has `numbers`, also with one number (Q-MN53). The loader merges, the writer stores differences; game code still finds a number's merged entry with `g_MonsterDatabase.Find(number)`. A one-time conversion groups today's 547 entries by English name, kind and model (36 monsters with 160 numbers, 387 with one), compared once for every field and language before the flat file goes; load → save → load and the merge rules stay tested. The Monster Browser shows monsters with their numbers and can add, copy, move, split out and remove numbers, each one undo step. |
| D62 | Variants *(agreed, Q-MN35, Q-MN36)* | A number has one setup on every map; the Bull Fighter for map-gated numbers on other maps goes (a fix commit). What changes by map or event state is a **variant inside the entry**: `mapVariants` (maps + only the fields they change, e.g. the Cursed Wizard's scale 1.0 on Devil Square, world 35's bodies of 310, 315–317, the sizes of the Castle Gate and Statues of Saint 131–134 by Blood Castle) and `stateVariants` (a state from a fixed list in code: `castleSiegeRunning`, `castleSiegeNotRunning`, `raklionBossWaiting`, `crywolfReady`, `castleGateOpen`, `playerTransformed`). Not new numbers: the server picks the number. Monster Preview has Map and State pickers. Map variants come with the attacks PR (Q-MN65), `attacks` their first field: `"mapVariants": [{"maps": [32], "attacks": {...}}]`; a variant's field replaces the entry's on its maps (Q-MN68); later PRs add the setup fields. |
| D63 | Setup as data *(agreed, Q-MN36–Q-MN38, Q-MN53)* | Beyond the setup fields of section 3: `equipment` (`weapons`, `helm`, `armor`, `pants`, `gloves`, `boots`, `wings`, `helper`; an item as `{"group", "number", "level", "excellent"}`, group and number also for armor; a body that is no item as `{"part": <slot symbol>}`; the Dark Horse helper's `mountLook`, `normal` or `doppelganger`, Q-MN57), `scale: "character"`, `spawnEffects` (rows of joint, effect, object or eyes), `randomVariants`, `randomLook`, `randomStart`, `spawnBuffs`, `spawnSound`, `live`, `facing`, `facingStep`, `facingByTile`, `appear` (with `teleport`), and the model's `namedBones` and `parts` in `Models.json`. Random choices draw the same numbers in the same order as the old code. A field an entry does not name means a fresh slot's value (P14, Q-MN44). |
| D64 | Unknown monsters *(agreed, Q-MN42, Q-MN45)* | A number without an entry, an entry without a model (76, 252, 296, 585) or a model whose file is missing (Molt 68) gets a **bug look** instead of the Bull Fighter: the Ghost model, a red pulsing glow, red X marks rising, the name "Unknown monster {number}", a chat bubble "I am a bug: {number}" when it appears and every 10 s, one log line. A settings section "Unknown monsters" in `Monsters.json` holds it, edited in Monster Setup with "Preview as N"; the texts are game texts. A missing file is tried once per map. 523 gets the reserved empty model `none`. |
| D65 | Three problem levels *(agreed, Q-MN45, Q-MN60)* | **Fatal**: the file cannot be read or contradicts itself; the game stops at start and Save refuses (today's "Error"). **Error**: readable, but something is missing (an item, a model file, a sound); shown red, the game runs with the bug look or without the item, Save allowed. **Warning**: works, worth knowing (old behaviours such as P24 and P26, a missing translation). The level list is shared by every tool (items, tooltips, server list, effects, looks, light moods, monsters, models), and one **Data Checks** window with area and level filters replaces Item Checks and Monster Checks (an editor-wide PR before PR 3). |
| D66 | NPC rules and texts *(agreed, Q-MN46–Q-MN48)* | `talks` (default on for NPCs; the "number ≥ 205" rule goes), `talkLevel` with its message, `repairShop`, and **talk blocks** in order: dialog (typed, translatable lines; buttons; OK runs a request from a fixed list with conditions and a failure message), send the normal talk request, ask for pet information, message, quest window for classes without the NPC's quest. They replace the code for the Werewolf Guardsman, the barracks Gatekeeper, Marlon and the Pet Trainer. Texts are translatable strings in `Monsters.json` like names, copied once from `Game.resx` (which ResxGen compiles into the binary, so the editor could not change them at runtime). |
| D67 | Maps by server number *(agreed, Q-MN66, Q-MN67)* | Data names maps one way: `maps` lists of the map numbers the server sends. The client keeps that number next to the world it loads, so Devil Square 5–7 (32) stays apart from 1–4 (9) although both load as world 9. `Models.json`'s `worlds`, `worldTextureFolders` and `soundEventWorlds` become `maps`, `mapTextureFolders` and `soundEventMaps` (the values stay: none names 9 or 32). One map picker for every tool groups the maps (Devil Square 1–7, Kalima 1–7, Blood Castle 1–8, Chaos Castle 1–7, Illusion Temple 1–6) and shows each map's server numbers and files folder. A list may name a stage of a map, `{"map": 9, "stage": 2}`: OpenMU sends the extended client the stage of Devil Square with the map (Q-MN70, Q-MN71); a group's levels are maps of their own or stages of one map (Q-MN72). Maps as data (names, groups, folders, shared files) are a later PR of the content roadmap. |

## Current state

### Client (MuMain)

| Area | Where | Notes |
|---|---|---|
| Monster numbers | `EMonsterType`, `_enum.h:4565` | 543 numbers from 0 to 668, with gaps. |
| Names | `Data/Local/<lang>/NpcName_<lang>.txt` → `MonsterScript[1024]` (`OpenMonsterScript`, `ZzzInfomation.cpp:1789`) | 536 names (0–585) in English, Portuguese and Spanish; the UI has 11 languages. Read by a linear search (`getMonsterName`). |
| Kind | `DetermineMonsterObjectKind`, `ZzzCharacter.cpp:13146` | Number ranges, 368–370 and a list of 49 numbers forced to NPC: 436 monsters, 99 NPCs, 7 traps, 1 other. |
| Spawn paths | See "Scope" | Five paths; each changes the monster after `CreateMonster`. |
| Setup at spawn | `CreateMonster`, `ZzzCharacter.cpp:13231-15085`; 15 creators outside it and 14 map processes (5 of them empty); `Setting_Monster` | 531 numbers have a creator: 269 case labels (185 bodies) in the core switch, 267 entries in the other creators. |
| Models | `OpenMonsterModel`, `ZzzOpenData.cpp:1317-2930`; `OpenNpc`, `:583-1055`; `CMapManager::Load` | 217 monster model types (`Data/Monster/MonsterNN.bmd`), 80 NPC model groups (`Data/NPC`), per-map part models. |
| Animation speeds | The same functions, the creators, the cinematics | Defaults, multipliers for 5 types, 519 overrides; speeds also set at spawn and in cutscenes. |
| Sounds | The same functions, `CMapManager::Load`, `PlayMonsterSound` | 387 loads in `OpenMonsterModel`, about 235 more per map, 12 in `OpenNpc`; slots for 74 models. |
| Attack animations | `SetPlayerAttack`, `ZzzCharacter.cpp:1073`, 13 handlers, `MonsterSkill.bmd` | See "Attack animations today". |
| Death, idle, movement | `SetPlayerDie`, `DeadCharacter`, `SetPlayerStop`, `EtcStopAnimationSetting`, `CharacterAnimation`, `MoveCharacterPosition` | Per-model code; see "Death, idle and movement". |
| Effects and looks | About 20 map files and `ZzzCharacter.cpp` | About 1,540 effect calls for monsters and NPCs (counted for FX1). |
| Talking to NPCs | `ZzzInterface.cpp:1525-1776` | Rules by number, model and map; the window itself is chosen by the server. |
| Editor | – | No monster tool. The legacy `ENABLE_EDIT` monster placement (`EditObjects.cpp:65`) is unreachable: nothing sets `EDIT_MONSTER`. |

#### Numbers and names

- `EMonsterType` has 543 named numbers. 13 numbers that the server can send
  have no creator and become a Bull Fighter (`default:` is grouped with it,
  `ZzzCharacter.cpp:14118`): 76, the statues 658–668, and **523, the trap that
  OpenMU spawns in every Imperial Guardian run** (all zones but the boss
  zone), which has a name but only a commented-out enum value. 252, 296 and
  585 from the name files have no creator either. `CreateMonster` never
  returns null today, and its callers rely on that: 0x13 and 0x1F leave their
  entry loop on null, so every later monster of the packet would be lost;
  0x45, the cinematics and `CreateHellGate` use the result without a check.
- `NpcName_<lang>.txt`: `<number> 1 "<name>"`, read with the old SMD
  tokenizer, the second column ignored, names cut at 64 characters after an
  unbounded conversion (`ZzzInfomation.cpp:1810-1811`; a longer name overflows
  `Name` first). All three files have the same 536 numbers. 252 has an empty
  name; 296 ("Monster7"), 523 ("Trap") and 585 ("Scarecrow") have names but no
  enum value. 96 Portuguese and 90 Spanish names equal the English ones.
  Entries 152–158 are printf formats ("Magic stone of %s ") that
  `CreateHellGate` fills with the owner's name.
- With the OpenMU-only statues 658–668, the monster list has 547 entries.
- The language of the names is `[LOGIN] Language` (`g_strSelectedML`), not the
  UI language, and a player cannot change it in the game. The minimap labels
  and the quest files use the same setting.
- Every creator ends in `Setting_Monster` (`ZzzCharacter.cpp:13198`). It copies
  the name into `CHARACTER::ID` (`wcscpy_s`, which aborts on MSVC for a name
  over 64 characters) and overwrites what the creator wrote: 140 names in the
  core switch (108 Korean) and 9 Korean names in Hellas are never shown. For a
  number without a name (658–668) the name stays empty: `CreateCharacterPointer`
  clears it (`ClearDisplayName`, `:11725`) and the Bull Fighter body writes
  none for these numbers. `QuestMng` keeps the pointer `getMonsterName`
  returns.
- `Setting_Monster` also sets the kind, and on worlds 10–17 (Heaven and Blood
  Castle 1–7) resets the render tier (`Level`) of the numbers 84–143 and adds
  0.05 to their scale for every third castle (`:13202-13210`).
- Code that finds monsters by their shown name: a dead comparison with a
  Korean guild master name (`WSclient.cpp:1950`, P16), the control socket's
  `target` (`ControlTargets.cpp:37`), and the in-game test scenarios
  (`tools/InGameTests/Scenarios/Npcs.cs`, `Quests.cs`, `Meeting.cs`). The
  control socket's descriptions carry no monster number and show traps as
  "unknown".
- Readers of `MonsterScript` by position, not by number: P8
  (`CustomMessageBox.cpp:1003`), the unreachable legacy edit mode
  (`EditObjects.cpp:73`, `ZzzInterface.cpp:4110-4113`) and the unused
  `MonsterConvert` (`ZzzInfomation.cpp:1832`).
- A test reads the txt files: `npc_name_assets` (`tests/ui/CMakeLists.txt:155`,
  `tests/ui/test_npc_name_assets.py`): unique numbers, length ≤ 64, the numbers
  42, 297, 318 and 464, and some Portuguese and Spanish names.
- `MapCharacters.bmd` (`Data/Local`) is a list of monster numbers per map (65
  maps, 465 numbers up to 578) that no code reads; MN1 deletes it as unused
  data.

#### Spawn and character slots

- `CreateCharacter` (`ZzzCharacter.cpp:12138`) reuses the slot of a live
  character with the same key (0x45 re-creates a visible player this way, and
  skips the bone, cloth and parts cleanup of the free-slot branch), else the
  first free slot; a full table returns a sentinel slot.
- `CreateCharacterPointer` (`:11722`) clears the name and sets defaults (scale
  0.9, outline 1.2, move speed 10, no blend or hidden mesh, …), a **bounding
  box by model** (37 cases including `MODEL_PLAYER` and the 3 Chaos Castle
  models, plus 2 in `SettingBattleCastleMonsterLinkBone`) and the **weapon
  link bones by model** (37 cases, plus 5 in `SettingHellasMonsterLinkBone`,
  default 33/42; Castle Gate 1 skips them and sets `SwordCount = 0`).
- It does **not** reset several fields that creators or other code write, so
  they carry over from the slot's previous character: `m_bpcroom` (the helper
  NPC plate), `HideShadow` (set by the Doppelganger rendering), `Skin` (read by
  `SetCharacterScale`), `LifeTime`, `Gravity`, `Distance`, `m_iAnimation`,
  `Appear`, `MonsterSkill`, `SwordCount` and Castle Gate 1's `LinkBone` (P14).
  A spawn therefore depends on the slot's history, not only on the number and
  the map.

#### Setup at spawn

`CreateMonster` first asks these creators in order (`ZzzCharacter.cpp:13237-13346`):
Cursed Temple, Hellas, Battle Castle, Hunting Ground, Crywolf 2nd, Crywolf
1st, Aida, Kanturu 1st, 2nd and 3rd, the Balgass barracks, the New Year event,
New Town, Swamp of Quiet, the 09 summer event, then the map process (14
processes, 9 of which create monsters; it asks them all without checking which
map is active). Some answer only on their map. A few numbers have two
creators (224; 310 and 315–317 by map).

What the core switch sets (bodies / case labels or lines):

| Field | Core switch |
|---|---|
| Model (`OpenMonsterModel` / `OpenNpc` + `CreateCharacter`) | 97 / 164 monster models (and `default:`), 63 / 70 NPC models, 18 / 22 `MODEL_PLAYER`, 7 / 13 other slots: Kalima gates on the skill model `MODEL_WARCRAFT`, the soccer ball on `MODEL_BALL`, traps on map object slots 25, 39, 40 and 51 (11 in Battle Castle) |
| `Scale` | 136 / 219; 7 bodies call `SetCharacterScale`; 44 keep 0.9 |
| Weapons (item models, any body) with level (29 lines) and excellent flags (3, the golden monsters) | 50 / 102 |
| Body parts: item sets, wings (1); NPC part models with a variant for the merchants Alex, Martin, Harold, Amy and Lumen | 12 / 14 |
| `m_fEdgeScale` (outline) | 17 |
| `BlendMesh`, `BlendMeshLight`, `HiddenMesh` | 20, 13, 12 |
| `Level` (render tier) | 17 / 24 |
| `SubType` (skin of player-shaped monsters, variants) | 14 / 17 |
| Shadow | 5 `m_bRenderShadow = false` (the flag that works), 1 `HideShadow = false`, and 12 `EnableShadow = false` that do nothing (P15) |
| No turn on magic hit | 8 lines |
| Move speed, target alpha, "no blood pool", light off | 1, 1, 4, 1 (the Ghost: alpha 0.4, move speed 15, no blood pool) |
| `LifeTime = 100` (arms a death effect hook; also about 16 lines in map creators) | 3 |
| Helper plate flag `m_bpcroom` | 2 |
| Bounding box override (Red Dragon model) | 1 |
| PK colour (Cursed Wizard) | 1 |
| Start animation or frame, tile or position offset, height offset | Kalima gates frame 10, Goblin Gate, Elphis (X+1, Y−1), Lala, Gatekeeper, Cherry Spirit, Layla, Veina, altars |
| Bones (`RegisterBone`) | 3 bodies; 175 lines in the map creators (Aida, Kanturu) |

The map creators set the same kinds of fields: about 230 scales, and in the
nine largest of them 148 weapons and 40 body parts.

Logic in the creators (stays code, D49): spawn effects (`CreateJoint`, 17 in 9
bodies), random choices (Chaos Castle model, Guardsman skin, Lunar Rabbit,
Twin Tail), values that depend on the map or event (Blood Castle weapon level,
Devil Square, Battle Castle started), extra objects (Blood Castle boxes), a
debuff at spawn (Poison Bull), `Live = false` (Wolf Statue). Twelve creator
lines write **model values** into the shared `Models[]` at spawn: 6 speeds
(the Moss, Lucky Item and Little Santa NPC models), Drakan's 5 none-blend
meshes and the Dark Phoenix shield's stream mesh. They are constants and move
into the model entries in MN3. `GM_PK_Field.cpp:24-54` already uses a table
(number, model, object model, scale, lifetime): the pattern this design
generalizes.

Player-shaped monsters and NPCs use `MODEL_PLAYER` with a class, item sets and
weapons (guards, Marlon, the Chaos Card Master, the Elf Soldier), or a skin
through `SubType` (skeletons, Jack O'Lantern, the transformation forms).

#### Models

- Slots: `MODEL_MONSTER01 + type` for 217 monster model types of 400; the NPC
  slots follow without a gap up to `MODEL_PLAYER`. The NPC range also holds
  map objects (Devias donkey and flag) and effect models (Xmas snowman).
- File: `Data/Monster/Monster<type+1>.bmd`; `OpenMonsterModel` refuses types
  from 217 up (`MONSTER_MODEL_COUNT`, `ZzzOpenData.cpp:1321`). 209 of 217
  files exist; 206 are loaded. **`MOLT` is opened (`ZzzCharacter.cpp:13617`)
  but `Monster50.bmd` is missing**, here and in the original 1.04d client. A
  missing monster model stops the game only because `AccessModel` checks the
  file name `"Monster"` (`LoadData.cpp:78`); a missing NPC model never does.
- Texture folders: `Monster\`, except types 70–72 in Chaos Castle (`Npc\`)
  and 74 in Battle Castle (`Object31\`), chosen from the map when loading.
  `OpenNpc` loads models from `Data\Npc\` (81 active calls, plus
  `Npc\LuckyItem` and `Npc\cherryblossom`), `Object35` (6) and `Item\xmas`
  (3); its `Data\Player` Angel case is compiled out (`_PVP_ATTACK_GUARD`), and
  `CMapManager::Load` loads the Angel. Textures come from the same folders,
  except the Werewolf Guardsman (`Monster\`); then a last
  `OpenTexture(…, "Npc\\")` pass runs when the model has meshes. The 8 Little
  Santas share `xmassanta.bmd` and patch one character of the texture name per
  slot. Tantallos and Crust load extra bitmaps into fixed global slots
  (`BITMAP_MONSTER_SKIN` and `+ 1`, `BITMAP_ROBE + 3` and `+ 5`).
- Part models: the Ice Giant loads 6 in `OpenMonsterModel`; 83 more are loaded
  per map in `CMapManager::Load` (Totem Golem, Shadow pieces, Condra, Kundun,
  …). All of them sit in the skill slot range, are FX1 effect types, are
  created as effects by death hooks, and are not freed on map change. The Dark
  Phoenix opens two models (shield and phoenix, `ZzzCharacter.cpp:13691`) and
  draws the second with `o->Type++` (`:8936-8970`).
- A map change frees only the monster and NPC slot ranges and the sound ids
  160–449 (NPC 160–209, monster 210–449) and the Elbeland range
  (`DeleteMonsters`, `DeleteNpcs`).
- Head bone (`BoneHead`, 43 models on 41 lines) and `StreamMesh` are set per
  model in the same functions.

#### Animations

- The `.bmd` holds the frames; **the speed of every animation is 0 unless code
  sets it**, and `SetAction` silently ignores an animation the model does not
  have (`ZzzAI.cpp:331`).
- Order in `OpenMonsterModel`: defaults (stop 0.25 and 0.2, walk 0.34, attack
  1 and 2 0.33, shock 0.5, die 0.55, and die holds its last frame), multipliers
  for 5 types, a speed switch with 495 overrides, "stop 2 holds its last frame"
  for 4 models, then 24 more overrides in the sound switch (Dragon, Bali, …):
  519 overrides; 140 of 217 models have values of their own. `Loop = true`
  means **hold the last frame**, not repeat.
- NPC models default to 0.25 for every animation, then 24 overrides.
- Speeds are also written at spawn (6 of the 12 creator lines that write model
  values, see above), in cutscenes, and scaled while playing
  (`CharacterAnimation`, `ZzzCharacter.cpp:2496`): transformed players attack
  ×1.5 (attack 1 and 2 only; `SetCharacterScale` does nothing for them),
  Illusion of Kundun dies ×4 after frame 6, Erohim dies at half speed.
- The monster animations are `#define`s (`_define.h:495-506`): stop1 0, stop2
  1, walk 2, attack1 3, attack2 4, shock 5, die 6, appear 7, attack3 8, attack4
  9, run 10, attack5 11.
- Generic rules by animation: a monster model returns to stop 1 after stop 2,
  shock and attack 1–5 (after appear only for the Maya hands and Selupan,
  `ZzzCharacter.cpp:3521-3548`); a hit interrupts any animation but attack 1
  and 2 with shock (`:1402`); only attack 1 and 2 have the impact frame that
  times the hit (`IsMonsterAttackAction`, `:2594`). Monster models and NPC
  models take different branches here (die hold and blood pool, idle).

#### Sounds

- `SetMonsterSound` fills a model's sound slots with offsets from
  `SOUND_MONSTER` (210): slots 0–1 idle, 2–3 attack and hit, 4 die. When a
  monster's `SubType` is 9 (Hellas), the played id is the slot's id **+ 5**, so
  a model with a second set (Death Centurion, Shriker) loads it at id + 5.
  Slots 5–9 are written by the 2 calls with 10 sounds but never read.
- 74 of 217 models have slots (69 calls). 40 more models load 143 sounds
  without slots, which only map hooks play by their fixed id. 141 of the 387
  loads use ids outside `SOUND_MONSTER_*`.
- Loads come with a channel count and a 3D flag: 245 with 2 channels and 3D,
  66 with 1 channel, 76 with the defaults (4 channels, no 3D).
- Quirks (P9): a slot of `-1` in slot 1 or 3 plays id 209; models without
  slots play slot 0 of the generic table; the Dragon's slots point at ids no
  code loads (333–335), so its slot sounds are silent.
- Below `SOUND_EXPAND_START` the monster range has 10 unused ids (224,
  328–332, 384, 447–449) and the NPC range has 163–209. 104 monster sound ids
  lie outside the ranges a map change frees and stay loaded.
- `PlayMonsterSound` (`ZzzCharacter.cpp:219`) plays sounds by model and
  animation **in addition** to the slot sounds: first a global hook for 6
  event and NPC models on every map (`PlayMonsterSoundGlobal`, `:15168`), which
  also cuts off all hook sounds of objects more than 500 units away, then the
  hooks of 7 map families (Raklion, PK Field, Swamp of Quiet, New Town,
  Karutan, Empire Guardian, Doppelganger).

#### Attack animations today

How the packets reach the animation choice:

- **0x18** (normal attack, animation 120) always calls `SetPlayerAttack`.
- **0x69** (monster skill) stores `MonsterSkill` and calls `SetPlayerAttack`
  when `0 < skill < ATMON_SKILL_END` (78).
- **0x19** (player skill) stores `Skill` for every skill, but calls
  `SetPlayerAttack` only for 200–202, the Lightning … Flame group, Fire Slash,
  Party Teleport and Chaotic Diseier. Heals, buffs, summons and Soul Barrier
  play `SetPlayerMagic`'s fixed pattern; Boss only sets the attack time; Decay,
  Ice Storm and Inferno play nothing.
- **0x1E** (area skill) stores `Skill` and calls `SetPlayerAttack` for every
  non-player model.
- OpenMU sends 0x18 and then 0x19 for every attack of the 141 monster numbers
  with an `AttackSkill`, so a skill-keyed mapping usually takes effect on the
  next 0x18, with the stale `Skill`.

`SetPlayerAttack` (`ZzzCharacter.cpp:1073`) then:

0. sends `MODEL_PLAYER` objects (player-shaped monsters and NPCs) to the player
   attacks, chosen from weapons, mount and class at the hero's attack speeds;
   none of the following applies to them;
1. handles traps (models 39, 40, 51: effects, sounds, action 1) and Bali by
   hand;
2. calls **all** 13 map handlers in a row (it does not stop at the first that
   answers);
3. if none answered, plays attack 1, attack 2, attack 2, … (`SwordCount`, a
   shared byte that also counts in `SetPlayerMagic` and wraps at 256).

| Handler | Gate | Keyed by | Mechanism |
|---|---|---|---|
| Hellas | none | number | player skill: Kalima monsters (Energy Ball etc. → attack 2; Necron has no default); random: Schriker and Illusion of Kundun 1–5 and 7 (`rand() % 2`); Illusion of Kundun 6 (338) is missing from the list and plays the generic pattern |
| Battle Castle | none | model | guards: shock, and the guard AI state |
| Hunting Ground | world 31 | number | Fire Golem: player skill Boss → attack 1, else attack 2 |
| Aida | Aida | number | `CheckMonsterSkill` (Hell Maine; Witch Queen, Blue Golem, Bloody Orc and Death Rider without entry); Bloody Golem and Witch Queen by hand |
| Crywolf 1st | world 34, and Devil Square (OpenMU's map 32 becomes world 9 when loading) | number | `CheckMonsterSkill` |
| Kanturu 1st | Kanturu 1st | number | random (`rand_fps_check(2)`, which depends on the frame rate, P18); Kentauros and three warriors by hand, but they return "not handled" (P3) |
| Kanturu 2nd, 3rd | their maps | number | `CheckMonsterSkill`; Maya goes to the Maya scene |
| Balgass barracks | the barracks and refuge maps | number | `CheckMonsterSkill` (412 has an entry; 409–411 have none) |
| Cursed Temple | Illusion Temple | number | `CheckMonsterSkill` (poison spirits) |
| New Town | – | – | answers nothing |
| Swamp of Quiet | its map | model | Medusa by player skill; Sapi Queen, Ice Napin, Shadow Master by monster skill |
| Map process | the active map | number | Raklion (by hand; Selupan), Empire Guardian 1–4 (by hand, rage states), Lucas on Loren Market (answers without an animation) |

Bali draws `rand() % 8`: attack 1 or 2 (5/8), attack 3 (1/4), attack 4 (1/8).

Three ways of clearing the skill after it played: only on a match (Raklion
454–458, Kanturu 1st), always (`CheckMonsterSkill`, Aida, Swamp of Quiet,
Empire Guardian, the Raklion Dark monsters), never (Selupan, Dark Ghost 51/52,
Hellas, Fire Golem). A stale skill is read again later (Selupan's height reset,
Dark Ghost's effects). Several handlers answer without an animation: Selupan's
skill 41 (teleport, which OpenMU sends), Necron's other skills, Maya, Lucas on
Loren Market, traps 39 and 51.

**`MonsterSkill.bmd`** (56 monsters): `CheckMonsterSkill` reads 20 of them (Hell
Maine 309; the Crywolf monsters 340, 341, 344, 345, 349 and 440; Kanturu 2nd's
358–360; Nightmare 361; the Maya hands 362 and 363; the six poison spirits
388–403; 412 of the Balgass barracks); the skill's position picks attack 1–5
(positions 5–9 play attack 5 and return "not handled"; no read entry uses
them), and a missing skill plays attack 1. 7 more monsters are sent there
without an entry and always play attack 1. The entries of the other 36 are
never read: some agree with the code (551, 552, 557–559, 562, 563; and 355,
553, 556, whose result P3 discards), others do not (the Raklion entries
454–458 put the skill first while the code plays attack 2; Selupan; the Empire
Guardian monsters that play `appear` for a skill, 513, 514, 517, 518; 565,
whose skill 77 belongs to 564 in the code, P5; 512 has no handler; Maya; 566's
skills are not below `ATMON_SKILL_END`, P6).

OpenMU sends 0x69 only for Crywolf, Imperial Guardian and Raklion (Selupan and
the Iron Knight's 33); the other monster skills of the file are never sent by
it. Kanturu's Nightmare gets its special attack (14) as a 0x19 player skill.

#### Effects tied to attacks

Effects follow mostly the **chosen animation and its frame or tick** ("at tick
14 of attack 1, create an inferno and play the Maya hand sound"). The skill
number also selects effects: the monster skill for Selupan and Dark Ghost, and
the player skill in `AttackEffect` (Boss for 11 monster groups and the
Meteorite trap, Energy Ball, Lightning) and in Hellas (Fire Slash, Poison,
Summon, Magic Defense of the Death Centurions). Per-frame visuals, render flags
per model and the attack effects are the looks of MN8.

#### Death, idle and movement

- `SetPlayerDie` (`ZzzCharacter.cpp:1457-1579`): skeleton skins, the Death Cow
  and the Stone Golem shatter (`Live = false`, bone or stone effects); Blade
  Hunter, Twin Tail and the Maya hands die by the Kanturu state; the
  Doppelganger keeps its appear animation; the die sound (slot 4, +5 for
  SubType 9; the Chaos Castle boom).
- `DeadCharacter` (`:3217-3323`): corpse turn speed for Bali in Atlans and for
  Illusion of Kundun (by `LifeTime`), Dreadfear never fades, falling corpses in
  Blood and Chaos Castle.
- Death effect hooks run once while `LifeTime` is 100 (Totem Golem pieces,
  Condra arms, Shadow pieces, Kundun).
- `SetPlayerStop`: Bali plays appear in the safe zone (`:425-428`).
- `EtcStopAnimationSetting` (`:3504-3692`): about 30 NPC models with weighted
  random idle animations; the Devias trader and the rabbit; the skeleton event
  NPCs with their chat texts.
- `MoveCharacterPosition`: the height is reset to the ground every step, so a
  spawn height offset holds only until the monster first walks; the Budge
  Dragon hops. `Calc_RenderObject` lifts the Castle Gate and the Statue of
  Saint.

#### Talking to NPCs

The server decides which window opens (0x30, a window code from OpenMU's
`MonsterDefinition.NpcWindow`), or sends the NPC number for a dialogue (F9-01).
Before it sends the talk request, the client checks in `MOVEMENT_TALK`
(`ZzzInterface.cpp:1525-1776`):

- by number: talk only for numbers ≥ 205, a level gate for the Chaos Goblin, 5
  repair shops, Little Santa's messages, Delgado, Lugard, Julia, David, Marlon
  (sets the quest flag), the Pet Trainer (pet information);
- by model: the Crywolf altar contracts, the Werewolf Guardsman;
- by map: the Balgass barracks Gatekeeper, the Kanturu 2nd entrance, the
  Cursed Temple statue and team storages.

Idle chat of NPCs is chosen by model and map (`OnlyNpcChatProcess`), the Elf
Soldier by number.

The outline (`ZzzObject.cpp:347-392`) is red with a size computed from the
scale for monsters and green with `m_fEdgeScale` for NPCs; for monsters, 1.2
means "compute it", any other value is used as it is.

#### Player transformations

The server sends a transformed player as a monster number (0x45). OpenMU knows
13 forms (2, 7, 8, 9, 14, 41, 372, 373, 374, 378, 477, 503, 548); 373, 374, 503
and 548 have no OpenMU monster definition. The ring → monster list in the
client (`SommonTable`, 6 entries) is only used for the ring's name, built from
`MonsterScript` names. The event forms are ordinary `CreateMonster` cases;
`ChangeRingManager` adds hair and cloak for some skins.

#### Editor and tests today

- No monster tool. The item tools offer what the monster tools need, but bound
  to items: one selection (`ItemSelection`), a browser with add, move and
  remove dialogs, an undo history with item step kinds, "used by" lists, source
  references (which know only `ITEM_` and `MODEL_` names), the Item
  Translations grid (bound to the item database), a model file picker that
  lists only `Data/Item` and `Data/Player`, and a model preview.
- Upstream main has the shared bar undo (`MuEditor::UndoTarget`) that the
  Server List window uses, and the Server List JSON as the newest small data
  file of the same kind.
- The model preview (`EffectPreviewScene`) draws an item or an effect model;
  it has no character path. The effect world preview copies the nearest live
  character's object; it never creates a character.
- The effect recorder (`tests/effects/EffectRecorder`) compares everything a
  call creates, under pinned seeds and a pinned FPS factor. Its pool snapshot
  is internal to its file, and its monster is `MODEL_PLAYER + 3`. A test binary
  has **no GPU device and no models**: `OpenTexture` of a monster model ends
  in the missing-texture dialog and `std::exit`, MOLT's missing file in a
  message box, and `SetAction` does nothing on an empty slot. Each test case
  has a 120 s timeout.
- CI's cppcheck fails today on files MN2, MN3 and MN4b must change:
  `GMBattleCastle.cpp` (two identical inner conditions), `GM_Kanturu_3rd.cpp`
  (a macro call without a semicolon at line 1346, which hides what follows)
  and `w_MapProcess.cpp` (`Smart_Ptr`).
- `src/bin/Data/Monsters/` is git-ignored like all of `src/bin` until it is
  re-included; `ServerList.json` was added by force.

### Server (OpenMU)

- `MonsterDefinition` (`src/DataModel/Configuration/MonsterDefinition.cs:230`):
  `Number` (a signed short), `Designation` (a `LocalizedString`), `ObjectKind`
  (Monster, PassiveNpc, Guard, Trap, Gate, Statue, SoccerBall, Destructible),
  `NpcWindow` (31 values), `AttackSkill` (a player skill, sent as 0x19 after
  every 0x18), attributes, drops, AI. The code comment calls `Designation`
  "not relevant for the server", but OpenMU shows it to players in invasion,
  quest kill and talk messages.
- Season 6 has 489 numbers (0–668; 485 name keys, because 132–134, 316/344 and
  317/341 share one). 478 are in both OpenMU and `NpcName_Eng.txt`; 11 only in
  OpenMU (658–668); 58 only in the client.
- Names: `MonsterNames.resx` (466; German 49, zh-CN 466) and
  `MerchantNames.resx` (19; German 15, zh-CN 19): German for 64 numbers, zh-CN
  for all. The client's UI has zh-TW, not zh-CN.
- Kinds do not map 1:1. Monster → monster (350 numbers). PassiveNpc → NPC,
  monster (152–158, 372, 378, 404, 405, 476, 477, 566) or other (451); Guard →
  NPC (220, 247, 249) or monster (285–287); Statue → monster (278, 283,
  658–668) or NPC (380); Destructible → monster (131–134) or NPC (541, 542);
  Trap → trap or monster (288, 523); SoccerBall (200) and Gate (277) → monster.
  OpenMU picks the server class of a monster by `ObjectKind`.
- 523 (Trap) has 153 spawns over the seven Imperial Guardian days: 19–33 per
  run, in every zone but the last. 658–668 are OpenMU's Illusion Temple
  statues (Cursed Statue, Captured Stone Statue 1–10); OpenMU does not spawn
  them yet (no Illusion Temple event); the client's Illusion Temple statue is
  380.
- Monster skills of events: Crywolf and Imperial Guardian are plug-in
  settings (`CrywolfEventDefinition.MonsterSkills`,
  `ImperialGuardianEventDefinition`, rage skill 59) sent as 0x69; Selupan's
  34–42 are a code enum (`SelupanSkill`, the list in
  `RaklionEventDefinition.GetSkills`) and the Iron Knight's 33 a constant
  (`RaklionEventViewPlugIn.cs:29`), both sent as 0x69; Kanturu's Nightmare
  uses a definition value (`KanturuNightmareDefinition.SpecialAttackSkillNumber`,
  14) sent as 0x19. An export cannot read all of them from the configuration.
- The admin panel edits monster definitions in the generic grid; there is no
  exchange file for monsters or items yet (items phase B).

### Problems found

Kept as they are when their phase moves the code (D51); each fix is a commit
of its own after the owner agrees.

| # | Problem | Where | Phase |
|---|---|---|---|
| P1 | `Monster50.bmd` (MOLT) is missing; spawning a Molt stops the game. Agreed fix: log it and keep running (Q-MN18). | `ZzzCharacter.cpp:13617` | MN3 |
| P2 | Numbers without a creator become a Bull Fighter: 76, 658–668 and 523, which OpenMU spawns in every Imperial Guardian run. A fix must still return a character for every number (callers rely on it). Agreed fixes (Q-MN15): 523 becomes an invisible trap with the trap kind, 658–668 get the statue 380's model and the NPC kind. Refined by Q-MN42: 523 gets the empty model `none` (no body, no sounds, never selected); 658–668 get 380's look (model, no shadow, outline 1.03) without its talk rule, 658 the monster kind (the renewed Illusion Temple's attackable Cursed Statue), 659–668 the NPC kind; 76, 252, 296 and 585 name no model and get the bug look (D64). | `ZzzCharacter.cpp:14118` | MN4 |
| P3 | Kentauros, Berserker Warrior, Kentauros Warrior and Genocider Warrior (355, 553, 554, 556): the handler picks the skill animation and then returns "not handled", so the generic pattern overwrites it. Agreed: fixed, the skill plays attack 2 (Q-MN21). | `GM_Kanturu_1st.cpp:604-657` | MN2 |
| P4 | Selupan never clears its skill, so normal attacks repeat the last skill's animation, sound and effects. Dark Ghost's skills 51 and 52 are the same. Agreed: fixed, the skill is cleared (Q-MN22): when the attack ends, also when a hit's shock ends it early, at once for Selupan's skill 41, which plays no attack, and by any other attack, which plays as a normal one also while the skill's attack plays (review of PR 2). | `GM_Raklion.cpp:2464`, `GMEmpireGuardian2.cpp:1099` | MN2 |
| P5 | Dark Iron Knight (565) plays attack 2 for skill 32, Dark Coolutin (564) for 77: the two constants look swapped. Agreed: swapped (Q-MN23). | `GM_Raklion.cpp:2429-2458` | MN2 |
| P6 | Skills 78–81 of monster 566 (`MERCENARY_GUILD_FELICIA`) are not below `ATMON_SKILL_END` (78) and never animate. Agreed first: they animate (Q-MN24). Then dropped, not a bug (Q-MN26): 566 is only ever an NPC (Tercia's model, whose file has stop1 and stop2 only; kind `npc` since MN1; one 566 in the client's name file and in OpenMU, a PassiveNpc), no handler answers for it, and only `MonsterSkill.bmd` has a row for it, which no client code could reach. | `WSclient.cpp:4104` | MN2 |
| P7 | 566 (`MERCENARY_GUILD_FELICIA`) and 567 (`PRIESTESS_VEINA`) use NPC models but count as monsters: a click attacks them instead of talking, they get the red outline and are MU Helper targets, while OpenMU makes 566 a PassiveNpc with a dialogue. | `ZzzCharacter.cpp:13086` | MN1 |
| P8 | `MonsterScript` is read by position, not by number: `MonsterScript[450]` is monster 494; the legacy edit mode does the same; `MonsterConvert` is unused. | `UI/Dialogs/CustomMessageBox.cpp:1003`, `EditObjects.cpp:73`, `ZzzInterface.cpp:4110`, `ZzzInfomation.cpp:1832` | MN1 |
| P9 | A `-1` in sound slot 1 or 3 plays sound 209; models without slots play slot 0 of the generic table; the Dragon's slot sounds are silent. Agreed: all three fixed (Q-MN19). The other slots that name numbers nothing loads (Devil, Balrog, Bahamut, Valkyrie, Lizard King, Soldier, Giant Ogre, Dark Skull Soldier, Dark Phoenix Shield) are fixed the same way (Q-MN20). | `ZzzCharacter.cpp:436-1556`, `ZzzOpenData.cpp:2396` | MN3 |
| P10 | A condition that can never be true (`==` && `==`). | `ZzzCharacter.cpp:10054` | MN8 |
| P11 | `MonsterSkill.bmd`'s loader clears only entry 0 and does not check the monster number. | `ZzzInfomation.cpp:182` | MN2 |
| P12 | `CreateHellGate` uses the name as a printf format and copies 200 bytes into the 130-byte name, overwriting the gate's key, level, monster number and more with stack bytes. OpenMU sends these gates (0x1F). | `ZzzCharacter.cpp:15146-15156` | MN1 |
| P13 | The Blood Castle step of `Setting_Monster` covers worlds 10–17 (with Heaven, without Blood Castle 8); the Blood Castle 8 monsters (428–433) are also missing from the Magic Skeleton cloth and from four attack and effect lists. Agreed (Q-MN43): a size bonus per castle, Blood Castle 1 +0, 2–5 +0.05, 6–8 +0.10 (428–433 included), as each castle number's own scale; the Red Skeleton Knights' axe level per number (88, 94 +8; the others +0); 131–134 by map variants. The cloth and the lists are looks (MN8). | `ZzzCharacter.cpp:13202`, `:8550`, `:1654`, `:2074`, `:2258`, `:5114` | MN4 |
| P14 | Fields not reset when a character slot is reused (see "Spawn and character slots"), e.g. a slot used by a Doppelganger draws no player shadow afterwards. Agreed (Q-MN44): every spawn starts from a fresh slot's `m_bpcroom`, `HideShadow`, `Skin`, `LifeTime`, `Gravity`, `Distance`, `m_iAnimation`, `Appear`, `MonsterSkill`, `SwordCount`, `PKKey`, `EyeLeft3`/`EyeRight3` and Castle Gate 1's link bones; old named bones are unregistered also on a same-key re-create (0x45). One fix commit after the move. | `ZzzCharacter.cpp:11722` | MN4 |
| P15 | 16 `EnableShadow = false` lines in the creators do nothing: `CreateCharacterPointer` sets the flag false, and only `RenderCharacter` changes it, true for the shadow copy and back. The 16 monsters have no shadow for other reasons (`m_bRenderShadow = false`, the model skip in `RenderCharacter`, invisible). Agreed (Q-MN54): dead code, deleted with the creators, no field. | `ZzzCharacter.cpp:13057-14327`, `GM_Raklion.cpp:162-184`, `w_CursedTemple.cpp:164` (lines of `938349ec4`) | MN4 |
| P16 | A comparison with a Korean guild master name that no name file has; the branch is dead. | `WSclient.cpp:1950` | MN1 |
| P17 | `Setting_Monster` uses the character before its null check. Never reached (every caller passes a character), and the lines go with the Blood Castle step into per-number data (Q-MN55). | `ZzzCharacter.cpp:13207`, `:13212` | MN4 |
| P18 | The random Kanturu 1st monsters (Berserker, Gigantis, Genocider, Splinter Wolf, Iron Rider) pick attack 1 with a chance that depends on the frame rate (`rand_fps_check(2)` in a one-time choice: about 21 % at 60 FPS, 9 % at 144 FPS). Agreed: 50/50 with `rand() % 2`, also for the Gigantis Warrior (Q-MN25). | `GM_Kanturu_1st.cpp:595`, `:640` | MN2 |
| P19 | `RenderCharacter` skips the shadow of Kalima gate 1 only, not gates 2–7. | `ZzzCharacter.cpp:8715` | MN8 |
| P20 | Destler, Kato, Galia and the Cavalry Captain (508, 510, 511, 517) play appear at speed 0, and the Cavalry Captain also attack3: their models give these animations no speed (here and in the original client), so they stand still in them until another animation starts. Destler, Kato and Galia play appear for skill 59 and in their rage, the Cavalry Captain for skills 55 (appear) and 56 (attack3). Found by Monster Checks. Agreed: fixed with each model's attack 3 speed, 0.33 for the Cavalry Captain (Q-MN27). | `Models.json`, `ZzzOpenData.cpp` before MN3 | MN3 |
| P21 | The Dark Ghost's second attack plays the fire pillar sound, which only the Balgass maps (41, 42) load: silent on Empire Guardian 2 unless the hero was there earlier. Agreed: its model loads it (Q-MN31). | `GMEmpireGuardian2.cpp:1230` before MN7 | MN7 |
| P22 | 13 models play sounds that only another model loads: Raklion's dark twins, the PK field's Burning Lava Giant and Cruel Blood Assassin, the Swamp's Sapi Queen, Ice Napin and Shadow Master, and Crypos play their normal versions' sounds (silent unless those appeared); the Coolutin's death (the Hell Spider's sound), the Forsaker's second attack (Selupan's) and the Cavalry Captain's death (the Giant Mammoth's) are silent on their maps. Agreed: each model loads the sounds its events play (Q-MN32). | the sound hooks before MN7 | MN7 |
| P23 | The Little Santa messages are formatted into `wchar_t temp[32]`; on Windows `mu_swprintf` writes past it in de (health 42 / mana 41 characters), en (34 / 32), es, pl and tl. Agreed (Q-MN48): fixed first in MN5, before its comparison. | `ZzzInterface.cpp:1706` | MN5 |
| P24 | The Wolf Status (204) is dropped at spawn: `Live = false` frees its slot, so it is not drawn, not clickable, and later packets for its key find nothing; the visible Crywolf statue is map object 81. Moves as it is with a Warning (Q-MN36); decided after a Crywolf event test. | `ZzzCharacter.cpp:14184` | MN4 |
| P25 | The Blood Castle red light skips castle 3: `== MONSTER_GIANT_OGRE_3 && == MONSTER_RED_SKELETON_KNIGHT_3 && …` is never true, and it counts castles like P13. Fixed to P13's table in MN8 (Q-MN40, Q-MN43). | `ZzzCharacter.cpp:9982` | MN8 |
| P26 | The later copies have no attack data and play the generic pattern: Kalima 7 (331, 332, 335–338) and Devil Square 7 (434–439), as in the original source. Moves as it is: the grouping writes their rotation (attack 1, 2, 2) as their own value with a Warning; changed per number later (Q-MN62). | `GMHellas.cpp:930` | PR 3a |
| P27 | The cutscenes' walk speed (Balgass 0.18, Dark Elf 0.227, Nightmare 0.25) is written into the shared model and never restored, so every monster on that model walks slowly until the map changes. Agreed (Q-MN58): the cutscene restores it when it removes its monsters; a fix commit after MN4's move. | `CDirection.cpp:214` | MN4 |

## Target design

### 1. Monster table in memory

- One flat array of entries indexed by monster number (1024), filled once at
  loading; the editor rebuilds it after a change. One accessor checks the range
  and returns an "unknown" entry with today's fallbacks (D44); it is used by
  `Setting_Monster`, `getMonsterName`, the kind lookup and the attack lookup.
- `MonsterScript`, its linear searches and its readers by position go;
  `getMonsterName` returns stable per-number storage.
- Models are resolved to slots when loading; the game keeps using
  `Models[o->Type]`. The bounding box and the link bones by model live in a
  table by model slot (D45).

### 2. Files

```json
// Data/Monsters/Monsters.json
{ "formatVersion": 1,
  "monsters": [
    { "number": 2, "name": {"en": "Budge Dragon", "es": "Dragón Escurridizo", "pt": "Dragão Lento"},
      "kind": "monster", "model": "MONSTER_MODEL_BUDGE_DRAGON", "scale": 0.5 },
    { "number": 454, "name": {"en": "Ice Walker", "es": "Caminante del Hielo", "pt": "Caminhante do gelo"},
      "kind": "monster", "model": "MONSTER_MODEL_ICE_WALKER", "scale": 1.2,
      "attacks": {"monsterSkills": {"29": "attack2"}, "otherwise": "attack1", "clearSkill": "onMatch"} },
    { "number": 459, "name": {"en": "Selupan", "es": "Selupan", "pt": "Selupan"},
      "kind": "monster", "model": "MONSTER_MODEL_SELUPAN", "scale": 2.0,
      "attacks": {"monsterSkills": {"34": "attack2", "35": "attack1", "36": "attack3",
                                    "37": "appear", "38": "attack4", "39": "attack4",
                                    "40": "attack4", "41": "none", "42": "attack4"},
                  "otherwise": "attack4", "clearSkill": "never"},
      "hook": "Selupan" } ] }
```

```json
// Data/Monsters/Models.json
{ "formatVersion": 1,
  "models": [
    { "slot": "MONSTER_MODEL_BUDGE_DRAGON", "file": "Data/Monster/Monster03.bmd", "textureFolders": ["Monster"],
      "animations": {"walk": 0.7}, "headBone": 7,
      "sounds": [ {"id": "SOUND_MONSTER_BUDGE1", "file": "Data/Sound/mBudge1.wav"},
                  {"id": "SOUND_MONSTER_BUDGEATTACK1", "file": "Data/Sound/mBudgeAttack1.wav"},
                  {"id": "SOUND_MONSTER_BUDGEDIE", "file": "Data/Sound/mBudgeDie.wav"} ],
      "soundSlots": {"idle": ["SOUND_MONSTER_BUDGE1", "SOUND_MONSTER_BUDGEATTACK1"],
                     "attack": ["SOUND_MONSTER_BUDGEATTACK1", "SOUND_MONSTER_BUDGEATTACK1"],
                     "die": "SOUND_MONSTER_BUDGEDIE"} } ] }
```

The values are today's (Budge Dragon: `ZzzCharacter.cpp:14191`,
`ZzzOpenData.cpp:1387` and `:2174`, whose slots are 10, 11, 11, 11, 12, the
sounds loaded with 2 channels and 3D; Ice Walker: `GM_Raklion.cpp:91` and
`:2317`; Selupan: `GM_Raklion.cpp:140` and `:2464`). A model entry always
names its file and texture folders and otherwise lists only what differs from
the defaults of its range. The examples show the shape; field names are
decided in each phase.

From PR 3a on, a monster entry lists its numbers (D61); each number holds only
what differs from the monster and the maps it appears on (Q-MN61):

```json
{ "name": {"en": "Red Skeleton Knight", …}, "kind": "monster",
  "model": "MONSTER_MODEL_RED_SKELETON_KNIGHT", "scale": 1.19,
  "equipment": {"weapons": [{"group": 2, "number": 6}]},
  "numbers": [ {"number": 88, "maps": [11], "equipment": {"weapons": [{"level": 8}]}},
               {"number": 94, "maps": [12], "scale": 1.24, "equipment": {"weapons": [{"level": 8}]}},
               {"number": 111, "maps": [13], "scale": 1.24}, … ] }
```

Sounds keep their ids while code names them: a model's sounds are entries of
(file, channels, 3D, and the sound id by its `SOUND_*` name where code plays
the sound by that id), its slots point at sounds, and loads without a slot are
"extra sounds" that code and the hooks play (MN7). The +5 shift for `SubType`
9 stays as a named second set. A sound without a named id gets a number when
it is loaded, from a part of the sound table that grows as sounds are loaded
(Q-MN16): no fixed range limits new sounds, a file loaded by two models gets
one number, and a map change frees them with the monster models. The audio
backend's tables, sized by the `SOUND_*` list today, grow for them.

### 3. Data fields

| Group | Fields | Shared with OpenMU? |
|---|---|---|
| Identity | numbers, each with its own values and `maps` (D61) | yes (number) |
| Name | names by language | yes, by D53 |
| Kind | monster, NPC, trap, other (D54) | no; compared with `ObjectKind` |
| Model | a model entry, `player`, a model of another area, a map object slot (D47) | no |
| Setup | scale (or "by character", `SetCharacterScale`); outline (optional: absent means "computed", today's 1.2); weapons and item sets as items (type, level, excellent flags) on any model, wings; body parts that are part models of the monster's own NPC model, with a variant (the merchants); skin (`SubType`); blend mesh and its light, hidden mesh; render tier (`Level`, which also holds the Kalima gate tier from `CreateHellGate` and the murderer flag 0x1F sets; the selection tint reads it every frame); shadow (`m_bRenderShadow` only; the no-shadow lists by number and model in `RenderCharacter`, `ZzzCharacter.cpp:8712-8716` and `:8735-8737`, move with the looks in MN8); no turn on magic hit; move speed; target alpha; no blood pool; light off; death effect armed (`LifeTime` 100; absent means "left as it is"); helper plate flag; bounding box override; PK colour; start animation and frame; tile and position offset, height offset; named bones (`RegisterBone`; in the model entries, Q-MN37); and from Q-MN36–Q-MN38 and Q-MN53 the fields of D63 (equipment, spawn effects, random choices, spawn buffs and sound, shown, facing, appear) | no |
| Variants | `mapVariants`, `stateVariants` (D62) | no |
| Attacks | D48: `monsterSkills`, `skills`, `otherwise` (one animation, a rotation or `generic`, Q-MN69) or `otherwiseRandom` (`rand`, `randFps`, Bali's weighted draw), `clearSkill` (`never`, `onMatch`, `always`) | no |
| NPC rules | `talks`, `repairShop`, `talkLevel` with its message, talk blocks with translatable texts (D66) | no (OpenMU has the window) |
| Hooks | the names of the monster's special code (D49) | no |
| Model entry | slot, file, texture folders (ordered; map exceptions; the last `Npc\` pass of NPC models), per-slot texture name (Little Santa), extra bitmaps into named slots, part models (FX1 effect names; the Dark Phoenix's phoenix model), animation speeds and "hold last frame", sounds (ids, files, channels, 3D, slots, second set), head bone, stream mesh, none-blend meshes, bounding box, weapon link bones | no |

### 4. Code on top of data

- The creators' values, spawn effects, random choices and map and event values
  move into data (D62, D63); what is left as logic becomes a named hook (D49).
  The `Setting_Monster` Blood Castle step becomes each castle number's own
  values (Q-MN43).
- The map handlers keep their gates and their logic (rage, the Maya scene,
  sounds, effects) and read the animation from data (D48). The 0x19 forward
  list stays code.
- The post-creation changes of the packet handlers: per-monster values become
  data (Q-MN38: `facing`, `facingStep`, `facingByTile`, `appear`, the Giant's
  `playerTransformed` scale); the rules of summons, player transformations, the
  Doppelganger path finding, the 0x68 preview and the cutscenes stay code.
- `CDirection` takes the model from the monster table instead of its own
  switch (MN4); its walk speed writes stay cinematic code and are restored when
  the cutscene removes its monsters (P27). Cutscenes as data are a roadmap item.
- Death, idle and movement code by model stays code; MN3 can move the plain
  values (idle animations with weights, a body origin offset, "no fade") into
  the model entries.

### 5. Names and translations

- Names by language in the monster entries (D52); missing languages fall back
  to English. Monster Translations counts the missing names, with the `.resx`
  NPC window titles as suggestions.
- 252 (empty) keeps its entry with an empty-name warning, not an error. 296,
  523 and 585 are entries without an enum value. 658–668 get English names
  once from OpenMU's `MonsterNames.resx` in MN1 (Q-MN7); this changes what the
  game shows for them (today an empty name).
- The Kalima gate formats (152–158) become a UI text with one `%s` and a
  bounded result; their entries get plain names. P12 is fixed first (Q-MN11).
- Names follow the UI language, live names are refreshed on a language
  change, and Monster Translations imports and exports the old txt files
  (D52, Q-MN8, Q-MN9).

### 6. Attack animations (D48)

- `attacks.monsterSkills`: monster skill number → animation (packet 0x69).
- `attacks.skills`: player skill number → animation (packets 0x19 and 0x1E).
  When the skills are data (SK1), the numbers stay and the editor shows names.
- `attacks.otherwise`: what any other skill, a stale skill or a normal attack
  plays; attack 1 when `attacks` exists. Without `attacks`, the generic
  pattern stays. Since the attacks PR `otherwise` may be a list played in
  turn, counted per monster from its creation, or `generic`: today's pattern,
  `SwordCount % 3` with attack 1, 2, 2, where `SetPlayerAttack` counts each
  attack and the pattern one more (Q-MN69). Monster Attacks shows `generic` as
  the default of a number without `attacks`.
- `attacks.otherwiseRandom`, in place of `otherwise`: `rand` (C `rand() % 2`,
  Hellas), `randFps` (`rand_fps_check(2)`, Kanturu 1st), Bali's weighted draw
  (or Bali stays a hook). Data that replaces a random choice draws the same
  random numbers in the same order as the old code.
- `attacks.clearSkill`: whether the skill is cleared on a match, always or
  never, filled per monster by the MN2 converter (Q-MN14).
- The lookup is read only inside `SetPlayerAttack`'s monster branch, where
  today's handler answers (Q-MN13); `SetPlayerMagic` and `SetPlayerTeleport`
  stay generic. Today a handler answers only for the numbers it lists in code
  (e.g. Kalima 1–6, `GMHellas.cpp:930`), so attack data of another number is
  never read; the attacks PR lets every number with `attacks` play them on
  every map (Q-MN64). Where a number attacks differently on a map, a map
  variant says so: Fire Golem 291 plays `generic` in Devil Square 5–7 and
  Erkanne 507 `none` in the United Market Place, as before. A variant may
  name one stage of Devil Square (Q-MN71). Model-keyed handlers (Swamp of Quiet, Battle Castle, traps)
  become entries for every number created with that model; a later model
  change in Monster Setup does not carry them along (Monster Checks shows it).
- Monster Attacks offers only animations the model has, with a speed above 0.
  Monster Checks warns for `appear` on a model other than the Maya hands and
  Selupan (no return to idle) and for animations other than attack 1–2
  (interrupted by hits, no impact frame), as the Empire Guardian monsters and
  Selupan show today.

### 7. OpenMU sync

File-based, like items (D2, D4), in MN6 on the exchange format of items phase
7, with an OpenMU phase of its own (MN-B):

- The file: `formatVersion`, monsters keyed by number, the designation as
  `LocalizedString` (with `@src`/`@stamp`), `objectKind` and `npcWindow`
  (server → client, read-only, for the checks), `attackSkill`, and the event
  skills per monster, collected by the OpenMU export from the plug-in settings
  (Crywolf, Imperial Guardian, Kanturu's Nightmare) and the code constants
  (Selupan, Iron Knight).
- Names follow D53; kinds are not exchanged (D54). Numbers the server does not
  define but sends (the transformation skins 373, 374, 503, 548) are known
  client-only numbers and are never proposed for removal. OpenMU numbers above
  1023 are reported, not imported (0x13 sends only 10 bits).
- The editor compares the skills the server sends (0x69 event skills, 0x19
  `AttackSkill` and Nightmare's special attack) with `attacks` and lists those
  without an animation.
- Spawn maps from the export can feed the Monster Browser's map filter.

### 8. Adding monsters

- A new monster needs a number the server sends, an entry and a model. The
  Monster Browser offers free numbers (a number with an entry is not free,
  even without an enum value: 252, 296, 523, 585) and copying an existing
  monster. A copy shares the code keyed by its model, not the code keyed by
  its number; its hook comes along through the copied `hook` field.
- A new model gets a free slot of the monster range (D47).
- An NPC that the server opens with `NpcDialog` also needs dialogue data (the
  later dialogue area).
- Custom monsters on original Season 6 clients are the server's question, as
  for items (Q1 of the items design).

### 9. MuEditor monster tools

Focused tools that share one monster selection (`MonsterSelection`, like
`ItemSelection`), docked together, with the bar's undo (D56):

| Tool | Purpose |
|---|---|
| **Monster Browser** | All monster numbers with name, kind and model; search and filters (kind, has a hook, missing model file; map once a source exists, MN6); add (numbers 0–1023 only), copy and remove. Remove warns when the number is named in code, used by data (quest NPCs and kill targets, `NPCDialogue` keys) or in the last OpenMU import. Source references find `MONSTER_` names (not `MONSTER_MODEL_*` or `MONSTER01_*`), raw numbers in `MonsterIndex` comparisons, ranges, and the code keyed by the monster's model. From PR 3a a tree: monsters with several numbers open to their numbers, each labelled with its maps' names in the UI language; New monster, Add number, Copy (monster or number), Move number to…, Split number out, Remove number, Remove monster, one undo step each (D61). |
| **Monster Setup** | Model (from the model list), scale, outline (with "computed"), skin, meshes, render tier, the merchants' part models, the other setup fields, NPC rules. The equipment (weapons, armor, wings and helpers as items, with item pickers) gets a tab of its own, **Monster Equipment** (Q-MN33). Says when a model change switches the attack path (`player` and monster models attack differently). From PR 3: sections for map and state variants, spawn effects (▶ per row), facing, appear, the number's "Appears on" maps, NPC rules as talk blocks, hooks (Show code); each field grey when shared by the monster's numbers, yellow when the number's own ("make it this number's own" / "back to shared"); "at once" / "next spawn" badges and the Respawn button (D55); the "Unknown monsters" settings (D64). |
| **Monster Attacks** | The skill → animation tables, `otherwise` (one animation, a rotation or `generic`) or `otherwiseRandom`, `clearSkill`; a Map row with the entry's map variants and the grouped map picker (Q-MN65, D67); **Play** on each skill row and **Play next attack** for any other attack, in Monster Preview with the attack sounds (Q-MN73); whether each skill reaches the lookup directly or only on the next 0x18; the skills OpenMU sends (after MN6). Read-only with a note for player-shaped monsters. A player skill opens the skill tools once SK1 exists. |
| **Monster Model** | The model entry: file picker over every folder the monster and NPC models use (`Data/Monster`, `Data/NPC` with subfolders, `Object35`, `Item/xmas`, the part model folders, in their on-disk spelling); texture folders; animation speeds and "hold last frame" with an animation picker, and the speed factors code applies on top (transformed ×1.5, Illusion of Kundun die ×4, Erohim die ½); sounds with play buttons; the slot range and what depends on it; users of the model: monsters, ambient boids, map objects, effects (D26). From PR 3: named bones (names the code reads are locked, bones editable; Q-MN37) and parts such as Gayion's five flying swords (Q-MN45). |
| **Monster Preview** | The selected monster in a preview, with the item model preview's camera (orbit, pan, zoom) and a character path in the preview scene (weapons, body parts, skin). Animation preview: pick any animation of the model; play, pause and a frame slider; speed edits in Monster Model show at once; the model's sounds play at their moment (idle, attack, die), its sound events too, on any map (Q-MN74); the return to idle after an attack and the impact frame of attack 1 and 2 are marked; the speed factors the game adds on top (transformed ×1.5, Illusion of Kundun die ×4, Erohim die ½) are shown as "in game". Player-shaped monsters show their equipment once MN4c moved it into data; a **Show equipment** checkbox turns the weapons, gear and skin on and off (Q-MN33). Only in the editor's window: there is no view of the previewed monster in the world (Q-MN17). From PR 3 (Q-MN40): the game's part drawing for equipment (no shadow, no mount; a line says when the monster rides a mount), the spawn and appear effects created by the game's own code (Replay spawn, Replay appear), Map and State pickers for the variants, a "Pointed at" tick box and a light setting. |
| **Monster Translations** | Names × languages (the generic grid, D56); from PR 3 also the talk texts (D66). |
| **Monster Checks** | Replaced by the editor-wide **Data Checks** window (D65), where the monster area keeps these checks: missing model files (case-resolved), animations a model does not have or with speed 0, the animation warnings of section 6, a kind against OpenMU's `ObjectKind` (allowed pairs; 0x1F numbers and summons are players in game), a monster of kind "monster" on an NPC-range model, unused models (counting non-monster users), item types that no longer exist, names over 64 units, no free model slots. |
| **Look editor** (MN8) | The roadmap's shared look editor for the looks of the selected monster. |

Common features: validation before saving, Save and Revert, live changes as
D55 says.

### 10. Verification

The monster recorder (D50) compares old and new code **case by case in one
build**: a monster entry without the phase's fields runs the old code, and the
test removes those fields to record it.

- **Creation** (MN4): for every monster number, every map whose creator or
  step can take it (including Heaven, Blood Castle 1–8, Devil Square, Chaos
  Castle) and the event states creators read (Battle Castle started, Crywolf,
  Kanturu): a **full dump of the `CHARACTER` and `OBJECT`** after creation, not
  a chosen list, so a value without a data home fails the comparison. Each case
  runs on a clean slot, on a slot left dirty by another monster, and through
  the same-key path; through the packet handlers (made-up 0x13, 0x1F, 0x45 and
  0x68 packets) and `CDirection::SummonCreateMonster`, not `CreateMonster`
  alone; and with the cases "number without entry", "number ≥ 1024", "-1" and
  "character table full".
- **Models** (MN3): every model slot after loading on every map, the global
  bitmap slots a load fills, every sound load in order (id, file, channels, 3D)
  and the file each id holds afterwards, what a map change frees, and the loads
  that are not monsters (map objects, boids, effects). The cinematic walk-speed
  writes happen after loading and are outside this part.
- **Attacks** (MN2): for each monster a handler answers for, its maps, the
  skill numbers some handler or `MonsterSkill.bmd` names plus one unnamed
  number, and the inputs (stale `MonsterSkill`, `Skill` and `SwordCount`, rage
  state, 0x18 after 0x19, 0x1E for a monster source; player-shaped monsters
  with the hero's attack and magic speed pinned): the resulting
  `CurrentAction`, `MonsterSkill`, `Skill` and `SwordCount` after the call, the
  handlers' return, `o->AI`, Selupan's height and `m_bBossHeightMove`, the rage
  flags, the Maya globals, and the sounds and effects created. A sanity case
  (Ice Walker, skill 29 → attack 2) fails a recorder whose `SetAction` does
  nothing.
- **Talk** (MN5): per number, model and map, whether the client talks, the
  message, the repair shop and the level gate.
- **Grouping** (PR 3a, Q-MN52): once, in the conversion commit, every number
  of the flat `Monsters.json` (kept as a test file only there) is resolved
  from the grouped file and compared field by field in every language;
  missing or new numbers fail. Then the test file and the comparison go;
  load → save → load, the merge rules and the Browser's number actions stay
  tested.
- **Attacks PR**: the attack part runs before and after the handlers' number
  lists go.
- **Sound events** (MN7): the sounds `PlayMonsterSound` plays per model,
  animation, frame and map, with the 500-unit cut-off.
- **Harness**: `g_MapProcess` and the client folder are set up
  (`ClientDataScope`); textures are recorded by file name through a fake
  upload, as `test_bitmap_loader` does, because the test binary has no GPU;
  every model slot gets its real action count; MOLT (P1) is an expected
  failure; the pool snapshot of the effect recorder moves into a shared part;
  the monster part loads are separated from the map objects of
  `CMapManager::Load` before they are recorded; `srand`, `Random::Seed` and
  `FPS_ANIMATION_FACTOR` are pinned, and Kanturu 1st runs at two FPS factors;
  cases are split per phase and map so each stays under the 120 s timeout.
- If a baseline is committed after the old code goes, it keeps values derived
  from `rand()` out (draw counts only, as FX1) and gets an LF rule.

### 11. Editing a running game

D55 applies. Saving the model of a slot that live characters use reopens it
and clamps their animations; their bones are per character and are kept.
Undo of such a step does the same.

### 12. Documentation

`docs/monster-data.md` from MN1 on (D58); MN2 documents how attack animations
are chosen, as coding rule 11 asks for game logic.

## Pull requests and phases

The owner asked for few PRs (2026-10-23): the phases below are grouped into
three PRs now and three later ones. Each phase stays a group of commits of its
own inside its PR, with its comparison before the old code goes (D50).

| PR | Phases | Name | Can start | Size (lines added, estimate) |
|---|---|---|---|---|
| 1 | MN.P, MN1 | Monster list and names | merged (sven-n/MuMain#860) | about 5,000, plus `Monsters.json` |
| 2 | MN3, MN2, MN7 | Models and attacks | merged (sven-n/MuMain#866) | about 12,000–15,000, plus `Models.json` |
| Data Checks | – | Three problem levels and one checks window for all editor tools (D65); this document's answers to Q-MN35–Q-MN63 first | after PR 2 | – |
| Attacks | MN2b | Attack data decides on every map, rotations, map variants of attacks, maps by server number and Devil Square's stages, Play in Monster Attacks, Monster Preview plays the sound events (Q-MN62, Q-MN64–Q-MN74; OpenMU: Mosch0512/OpenMU#7) | after Data Checks | – |
| Sounds | MNS | One sound list: the sound slots become sound events (Q-MN75) | after Attacks | – |
| 3a | MNG | One monster, several numbers (D61) | after Sounds | – |
| 3b | MN4 | Setup at spawn | after 3a | – |
| 3c | MN5 | NPC rules | after 3b | – |
| later | MN6 (and MN-B in OpenMU) | OpenMU sync | when items phase 7 has landed | – |
| later | MN8 | Looks | when items phase 13 exists | – |
| later | MN9 | Cleanup | last | – |

PR 3 was planned as one PR for MN4 and MN5. With the answers of Q-MN35 to
Q-MN63 it grew by the grouping, the variants, the new editor sections and the
checks, so it runs as five PRs in a row (Q-MN63): each goes into `main` after
the one before is merged.

For comparison: the Server List window (sven-n/MuMain#845) added about 3,100
lines, items 3c (sven-n/MuMain#801) 4,300, the item tools (sven-n/MuMain#831)
23,400. In PR 2 the models come first, because the attack animations are
checked against the animations a model has, and the sound events belong to the
model entries.

| Phase | Name | Depends on | Summary |
|---|---|---|---|
| MN0 | Design document | – | This document; the roadmap links it. |
| MN.P | cppcheck fixes | MN0 | The findings of CI's cppcheck in `GMBattleCastle.cpp`, `GM_Kanturu_3rd.cpp` (fix the macro call first, then run again; the added `;` changes how the following render case parses, so it is checked like any code change) and `w_MapProcess.cpp`, so MN2, MN3 and MN4b can change them. |
| MN1 | Monster list, names and kind | MN0 | `Data/Monsters/Monsters.json` with number, names and kind; the accessor and table of section 1; `MonsterScript` and its name searches go, and `getMonsterName`, `Setting_Monster` and the kind lookup read the table; the txt files and `MapCharacters.bmd` go; the Kalima gate format becomes a UI text after the P12 fix. Editor: generic parts (D56), `MonsterSelection`, Monster Browser, Monster Translations, Monster Checks (first checks). Control socket numbers (D59), `docs/monster-data.md`, `.gitignore` and `.gitattributes`. P7, P8, P12, P16. |
| MN2 | Attack animations | MN1, MN.P | `attacks` in the monster entries (D48); `MonsterSkill.bmd` goes; the animation choices of the handlers and the generic pattern read data; their gates and logic stay. Monster recorder, attack part. Editor: Monster Attacks. P3–P6, P11, P18. |
| MN2b | Attacks decide by data | MN2 | Every number with `attacks` plays them on every map; the handlers' number lists and map checks for the animation go; `otherwise` may be a rotation or `generic`; map variants of attacks; the client keeps the server map and the stage OpenMU sends, and every map list in data names server maps and stages (`maps`), with one grouped map picker; Play buttons in Monster Attacks; Monster Preview plays the sound events too (Q-MN62, Q-MN64–Q-MN74). Attack recorder before and after. |
| MNS | One sound list | MN7, MN2b | The sound slots of 74 models become sound events (Q-MN75): an idle, an attack and a die event each, with what slots do and events did not (heard at any distance and on every map, the monster as `PlayBuffer`'s object, the second set for `SubType` 9); `Models.json` format 2; `PlaySlotSound`, `HasSlotSound` and the monsters' `BMD::Sounds` go; Monster Model's slot rows go. Monster recorder, sound part, before and after. |
| MNG | One monster, several numbers | MN2b | D61: the grouped format (merge and difference), the one-time conversion (36 monsters with 160 numbers) with its comparison, `maps` per number from OpenMU's Season 6 placements (410 numbers), the Browser tree and its number actions, shared and own values in every monster tool. The 12 copies of P26 keep their rotation as their own value. |
| MN3 | Models | MN1, MN.P | `Data/Monsters/Models.json` with what `OpenMonsterModel`, `OpenNpc`, the creators' model constants and the slot-dependent code in `CreateCharacterPointer` (with the Hellas and Battle Castle hooks) set; part models by FX1 name; sounds with ids. `model` is filled for every monster for the tools; the creators still choose the model until MN4, and the recorder checks both agree. Monster recorder, model part. Editor: Monster Model, Monster Preview. P1, P9. |
| MN4 | Setup at spawn | MN3, MN.P, MNG | The values of `CreateMonster` and the creators into the monster entries (D62, D63); the creators keep only logic, as hooks (D49); the post-creation changes of the packet handlers as Q-MN38 decided; `CDirection::SummonCreateMonster` takes the model from the monster table. Three parts in PR 3b: **MN4a** the core switch, **MN4b** the map creators, **MN4c** player-shaped monsters and NPCs and the merchants' part models, with the item tools learning monster references (D60). Monster recorder, creation part. Then the removal commits and one fix commit each: the bug look instead of the Bull Fighter (D64), P2, P13, P14, P27, Gayion's swords as parts, Maya without the Bull Fighter model, Molt once per map (Q-MN45). Editor: Monster Setup, Monster Equipment, the Preview additions. P2, P13–P15, P17, P24, P27. |
| MN5 | NPC rules | MN4 | P23 fixed first; the talk rules by number into the monster entries as `talks`, `talkLevel`, `repairShop` and talk blocks (D66), replacing the code of Marlon, the Pet Trainer, the Werewolf Guardsman and the barracks Gatekeeper; the Crywolf altar contract stays a hook; texts as translatable strings. Monster recorder, talk part. Editor: NPC rules in Monster Setup, texts in Monster Translations. P23. |
| MN6 | OpenMU sync, client side | MN1, MN2, items 7 | Exchange of number and names (D53); the read-only kind, window and skill data for the checks; the skill check against `attacks`. |
| MN-B | OpenMU sync, server side | MN6 (file format) | OpenMU: monsters in the exchange pages of items phase B, with the event monster skills collected from settings and code. |
| MN7 | Sounds per animation | MN3 | The global and the 7 map `PlayMonsterSound` hooks (with the 500-unit cut-off), the trap sounds of `SetPlayerAttack` and the per-map sound loads become sound events of the models. Monster recorder, sound part. |
| MN8 | Looks | MN4, items 13, FX1 | Glow, render styles, per-frame visuals, attack effects (with a skill-number condition), the trap effects of `SetPlayerAttack`, death debris, the no-shadow lists of `RenderCharacter`, cloth (Magic Skeleton, the Crust family, the Death King robe, Dark Phoenix hair; items kept cloth in code, so the look format needs a cloth block or cloth stays a hook), `RenderLinkObject`'s monster check, and the item render styles only monsters use (`helperNpcPlate`, `monsterBattleBow`) move to the monsters' looks. The sounds those per-frame visuals play (about 200 `PlayBuffer` calls in 12 files: Aida, Crywolf, Kanturu 1st to 3rd, the Hunting Ground, the Balgass barracks, Illusion Temple, the events, Empire Guardian, Hellas, Raklion) and their maps' loads of them move into the models' sound events then, with two more kinds of event: once when an animation starts (the `SubType` flag they use) and a chance every frame while it plays (Q-MN34). Look recorder of items phase 13. P10, P19. |
| MN9 | Cleanup | all MN phases | What later areas still need moves to the roadmap; then remove this document. |

MN1 is first because every later phase adds fields to its monster entries, and
the tools need its list and selection. MN2 and MN3 do not depend on each other;
PR 2 does MN3 first so that Monster Attacks can check animations against the
model entries. Monster Checks starts in MN1 and gets the checks of each later
phase.

### Phase details

**MN1 Monster list, names and kind.**
- Entries for the 536 named numbers (every number with a creator has a name)
  and for 658–668 (Q-MN7): 547 entries, sorted by number. Names of the three
  txt files as names by language; Portuguese and Spanish names that equal the
  English one are kept as they are, as the items' bmd import keeps them;
  658–668 with OpenMU's English names.
- `kind` from `DetermineMonsterObjectKind`, compared for -1, 0–1023, 1024 and
  65535; its number ranges stay only as the accessor's fallback for numbers
  without an entry (D44). The names compared one to one with the txt files,
  except 152–158, whose formats move to the UI text.
- `test_npc_name_assets.py` becomes a test of `Monsters.json` with the same
  rules (numbers 42, 297, 318, 464; unique numbers; length; the Portuguese and
  Spanish names). Shipped file in the writer's format.
- P12 is fixed first (a bounded copy), so the gates' records are
  deterministic; the format moves to a UI text.
- `.gitignore` re-includes `src/bin/Data/Monsters/` and ignores its `.tmp`
  files; `.gitattributes` gets `src/bin/Data/Monsters/**/*.json text eol=lf`.

**MN2 Attack animations.**
- Convert: the 20 monsters `CheckMonsterSkill` really reads (positions → the
  animation each position plays) and the 7 it handles without an entry, the
  hand-written mappings (Raklion, Empire Guardian, Aida, Swamp of Quiet,
  Kanturu 1st), the model-keyed ones (Battle Castle guards → shock; traps 39,
  40 and 51) for every number created with that model, the player-skill
  mappings (Hellas, Medusa, Fire Golem), the random choices, and the cases
  without an animation (`none`). The 36 unused entries of `MonsterSkill.bmd`
  are listed in the PR with whether they agree with the code.
- `CheckMonsterSkill` and the handlers read `attacks`; the handlers keep
  their gates, rage states, the Maya scene, sounds and effects. The traps'
  effects and sounds in `SetPlayerAttack` and the Battle Castle guards' AI
  state stay hooks (`Trap`, `BattleCastleGuard`); MN7 and MN8 move the trap
  sounds and effects.
- What the converter must keep of `CheckMonsterSkill`: positions 5–9 play
  attack 5 and return "not handled" (no read entry uses them); `-1` entries
  never match (the skill field is a `WORD`); Maya gets the skill as a `BYTE`
  and is not cleared.
- No import or export for `MonsterSkill.bmd`: its positions cannot express the
  mappings.

**MN3 Models.**
- One model entry per model type and NPC model, named by its slot symbol,
  with the values the code sets today, in today's order (a later value wins).
- `OpenMonsterModel`'s `MONSTER_MODEL_COUNT` guard and its `Monster<type+1>`
  file name give way to the entry's slot and `file`, so new entries can use
  the free slots 217–399.
- Map-dependent texture folders as exceptions per map; the per-map part models
  as FX1 effect names, loaded when they are today (per map) and never freed,
  as today. The Dark Phoenix's phoenix model is a part model of the shield's
  model entry, loaded with it.
- The `"Monster"` file-name check that stops the game for a missing monster
  model stays (P1 decides), with a test for a missing monster file and a
  missing NPC file. Castle Gate 1's `SwordCount` reset in the Battle Castle
  hook stays code.

**MNS One sound list.**
- The sound slots of 74 models become sound events, in today's order: idle
  (stop1, stop2 and walk, with the draws `SetPlayerStop` and `SetPlayerWalk`
  make), attack (attack 1 to 5 and shock; the Assassin's shock plays none)
  and die (Chaos Castle's death sound stays code).
- Events get what the slots do and events did not: no 500-unit cut-off and
  no map limit for the converted ones, the monster as `PlayBuffer`'s object,
  and a `SubType` condition for the second set (SubType 9).
- One conversion; `Models.json` format 2, whose reader refuses
  `soundSlots`. `PlaySlotSound`, `HasSlotSound` and the monsters'
  `BMD::Sounds` go.
- The monster recorder's sound part runs on every model, animation, frame,
  `SubType` and map before and after: the same sounds in the same order.
- Monster Model: the slot rows go, Sound events shows the converted
  events.

**MN4 Setup at spawn.**
- The values of each creator body into the monster entry; bodies that only set
  values are deleted, bodies with logic keep it as a hook.
- The data writes only the fields the code writes today (D51, P14).
- Lines that change nothing get no field and go with the creators; the
  removal commit lists them (Q-MN54–Q-MN56): the 16 `EnableShadow = false`
  (P15); names (`Setting_Monster` writes them over); the kind of 234;
  `Angle[2]` of 348, 376, 377 (0x13 turns them; Pamela's and Angela's angle
  stays as their supply box's angle); `Angle[0] = 0` and `Weapon[i].Type = -1`
  (already so after `CreateCharacterPointer`); the Canon Tower's if/else hidden
  mesh (overwritten on the next line); the never compiled 227, 228 and 276;
  Necron's weapon level 7 on an empty slot.
- The comparison runs on clean, dirty and same-key slots, through 0x13, 0x1F,
  0x45, 0x68 and `CDirection` (section 10, Q-MN59).
- MN4c: the item tools list monsters in "used by" and in the move and remove
  dialogs (D60).

## Monster-specific code that is left

After MN1–MN5 the code still names monsters and models in the places below.
The Monster Browser's source references are the full list; these are the
groups:

- the map handlers' gates and logic (rage, scenes, altars, events) and the
  hooks of D49;
- `AttackEffect` (87 monster cases), the per-frame visuals of about 20 map
  files, `RenderCharacter` (68 monster comparisons), `RenderLinkObject`: looks,
  MN8;
- `PlayMonsterSound`'s global and 7 map hooks and the per-map sound loads in
  `CMapManager::Load` (MN7);
- `AppearMonster` (17 cases of spawn animations and effects, `WSclient.cpp`);
- the post-creation changes of the 0x13, 0x1F, 0x45 and 0x68 handlers that MN4
  keeps as code;
- death, idle and movement code by model (`SetPlayerDie`, `DeadCharacter`,
  `SetPlayerStop` (Bali appears in the safe zone), `EtcStopAnimationSetting`,
  `CharacterAnimation`, the Budge Dragon hop, `Calc_RenderObject`'s origin
  offsets) where MN3 does not move plain values;
- game logic by number or model: the siege target tiles (`SkillCast.cpp:422`,
  `:792`), the Thunder Lich's bow range (`ZzzInterface.cpp:3247`), the pick
  boxes by model (`Selection.cpp:162`), the no-shock exceptions
  (`WSclient.cpp:3285`, `:3367`), the crown buff by raw 216 (`:11437`), the
  castle gate switch (`npcGateSwitch.cpp:66`), and the code that behaves by
  model slot range;
- the hooks left after D49 (the Crywolf altar contract among them), idle chat;
- about 2,040 `case MODEL_` labels across the code, most of them effects and
  map objects, not monsters (FX2 and the map objects area of the roadmap).

## Agreed questions

| # | Question | Answer (2026-10-23) |
|---|---|---|
| Q-MN1 | Which phase first? | MN1 (list, names, kind), then attack animations. |
| Q-MN2 | Attack animations by skill number with any animation, instead of `MonsterSkill.bmd`'s positions? | Yes (D48). |
| Q-MN3 | One file of monsters and one of models? | Yes (D46). |
| Q-MN4 | A model list of the monster area, or one list for all areas? | A list of the monster area (D47). |
| Q-MN5 | Fix the problems found? | One by one when the phase reaches them (D51). |
| Q-MN6 | OpenMU's German names now or with the sync? | With the sync (D53). |
| Q-MN7 | Entries for OpenMU's numbers 658–668? | Yes, with names; the Bull Fighter body until they get a model (MN1, P2). |
| Q-MN8 | Monster names today follow `[LOGIN] Language`. Follow the UI language instead, like item names (D17), and does the TranslateItemNames option also cover them? | Follow the UI language, a visible change for players whose two settings differ. TranslateItemNames covers item names only; the transformation ring's item name follows it. Live monsters get the new name on a language change, except transformed players, summons and Kalima gates, whose names are composed (D52). |
| Q-MN9 | Import and export of `NpcName_<lang>.txt`, as items keep for their bmd (D10)? | Yes: "Import names from NpcName txt" (by number and language, cut at 64) and "Export as NpcName txt" in Monster Translations (D52). |
| Q-MN10 | Do setup changes reach live monsters at once, or at their next spawn? | At once for the monsters of that number; a model change reopens its slot (D55). |
| Q-MN11 | Fix P12 (the Kalima gate overflow) as the first commit of MN1, before the comparison? | Yes: without it the gates' records are random (D51). |
| Q-MN12 | OpenMU's English names: keep the client's and list the differences? | Yes (D53). |
| Q-MN13 | Keep the handlers' map gates in code, or give `attacks` a map field? | Keep them in code; `attacks` is read where a handler answers today (D48). Changed by the attacks PR: the data decides on every map, map variants say where it differs (Q-MN64, Q-MN65). |
| Q-MN14 | How the skill is cleared: a `clearSkill` field per monster, or hooks? | The field, filled by the converter (D48). |
| Q-MN15 | 523 (Trap) and 658–668 (statues): give them proper bodies as fixes in MN4? | Yes, as P2 fixes after the comparison: 523 an invisible trap with the trap kind, 658–668 the statue 380's model and the NPC kind. For 658–668 OpenMU's Illusion Temple still decides whether it sends 380 or these numbers. |
| Q-MN16 | New model sounds need ids, and only about 10 are free below 450. A pool of ids, or no new ids in MN3? *(2026-10-25)* | Neither: the fixed ids are a legacy of the `SOUND_*` list and the audio backend's fixed tables, not a need of speed. Sounds are named by file and get a number when loaded, from a table that grows without limit; sounds that code plays by name keep their named ids (section 2). |
| Q-MN17 | The previewed monster in the world: a drawn copy, or a real character? *(2026-10-25)* | Neither: the preview stays in the editor's window. A view in the world is hard to use and does not look good. |
| Q-MN18 | P1: Molt (68) has no model file; OpenMU has no monster 68, so no one notices. Keep stopping the game when a server sends it? *(2026-10-25)* | No: a missing monster model file is logged and listed by Monster Checks, the monster appears without a body, and the game keeps running. A fix of its own after MN3's comparison. |
| Q-MN19 | P9: fix the sound slot quirks (a -1 in the second slot of a pair plays the unloaded id 209; models without slots play the Bull Fighter's sounds where those are loaded; the Dragon's slots point at ids nobody loads)? *(2026-10-25)* | All three, one commit each, after MN3's comparison. |
| Q-MN20 | Other models' sound slots also name numbers that nothing loads, as the Dragon's did (Devil, Balrog, Bahamut, Valkyrie, Lizard King, Soldier, Giant Ogre, Dark Skull Soldier, Dark Phoenix Shield): fix them too? *(2026-10-25)* | Yes, like the Dragon: each slot names the sound it was meant to play. |
| Q-MN21 | P3: the Kanturu 1st handler's skill animation is overwritten by the generic pattern. Fix? *(2026-10-25)* | Yes: the skill plays attack 2. |
| Q-MN22 | P4: Selupan and Dark Ghost never clear their skill, so normal attacks repeat it. Fix? *(2026-10-25)* | Yes: the skill is cleared after its attack. |
| Q-MN23 | P5: swap the skill numbers of the Dark Iron Knight and the Dark Coolutin? *(2026-10-25)* | Yes. |
| Q-MN24 | P6: animate Felicia's skills 78–81? *(2026-10-25)* | Yes, with the animations her table entries name. Replaced by Q-MN26: the question called 566 a monster, but it is an NPC (P6). |
| Q-MN25 | P18: the random Kanturu 1st monsters choose by frame rate. How should they choose? *(2026-10-25)* | 50/50 with `rand() % 2`, as the Hellas monsters. |
| Q-MN26 | P6 again, with what the original code does: 566 is only ever an NPC whose model has no attack animations. What happens with P6? *(2026-10-25)* | Dropped: not a bug. |
| Q-MN27 | P20: fix the bosses' appear (and the Cavalry Captain's attack 3) at speed 0? *(2026-10-25)* | Yes: appear gets each model's attack 3 speed (Destler 0.4, Kato 0.4, Galia 0.45); the Cavalry Captain's appear and attack 3 get 0.33. |
| Q-MN28 | MN7: where do the trap sounds of `SetPlayerAttack` go, as traps have no model entry? *(2026-10-25)* | Into the attacks: `sound` (and `soundAtMonster` for a sound played at the monster) in the monster entry's `attacks`, played whenever the attacks play. |
| Q-MN29 | MN7: the sound hooks draw with `rand_fps_check` once per step, attack, hit or idle loop, so the chances depend on the frame rate. Fix after the move? *(2026-10-25)* | Yes, like P18: picks become 50/50 with `rand() % 2`, chances 1 in n with `rand() % n`, in a commit of their own after the comparison. |
| Q-MN30 | MN7: the Empire Guardian maps load 31 sounds their monsters' events play; every other monster loads its sounds with its model at its first appearance. Move them into the models? *(2026-10-25)* | Yes, into the models, as for all other monsters; a model does not load a sound again that is already loaded from the same file. The map keeps its ambient sounds and DEFENDER_ATTACK02, which a player weapon plays too. A preload per map can come later. |
| Q-MN31 | P21: the Dark Ghost's fire pillar sound is loaded only on the Balgass maps. Fix? *(2026-10-25)* | Yes: its model loads it. |
| Q-MN32 | P22: 13 more models play sounds only another model loads. Fix the same way? *(2026-10-25)* | Yes: each model lists the sounds its events play. |
| Q-MN33 | Monsters miss their weapons and player-shaped monsters their whole look in Monster Preview, because the equipment is still code (MN4). When? *(2026-10-25)* | In PR 3 (MN4), not before. Monster Preview gets a **Show equipment** checkbox, and the equipment gets a tab of its own, **Monster Equipment**. |
| Q-MN34 | Monsters 340 to 417 and others show no sounds in Monster Model: their maps' per-frame visual code plays them (about 200 calls in 12 files), not `PlayMonsterSound`. Move them in PR 2 (an MN7b step) or later? *(2026-10-26)* | Later, in MN8 with the per-frame visuals. |
| Q-MN35 | Six creators answer only on some maps; their 55 numbers become Bull Fighters elsewhere, and world 35 gives five numbers other bodies. One setup, variants or hooks? *(2026-10-27)* | Variants inside the entry (D62). OpenMU: one number is one monster in all its versions (0.75, 0.95d, Season 6), no version has map 35, each Blood Castle has its own numbers; only the Cursed Wizard (34) appears on two maps with two looks. The Bull Fighter elsewhere goes in a fix commit; today's map looks stay as map variants. |
| Q-MN36 | What stays code at spawn: one hook per monster? *(2026-10-27)* | Data first (D49): spawn effects, random choices, spawn buffs and sound, shown, event states as state variants (D62, D63); a list of small hooks for what hands over to code. Old behaviours move as they are with a note and a Warning (P24). |
| Q-MN37 | Named bones in the monster or the model entry? *(2026-10-27)* | The model entry (`namedBones`, 39 models), shown in Monster Model and Monster Preview. |
| Q-MN38 | The packet handlers' changes after `CreateMonster`? *(2026-10-27)* | Data for every per-monster value: `facing`, `facingStep`, `facingByTile` (Ballista), `appear` with state variants and `teleport`, the Giant's `playerTransformed` scale; the rules of summons, transformations, the Doppelganger maps, the 0x68 preview and the cutscenes stay code. |
| Q-MN39 | Setup edits on live monsters? *(2026-10-27)* | Split into "at once" and "next spawn", plus a Respawn button for testing (D55). |
| Q-MN40 | How does Monster Preview draw the equipment? *(2026-10-27)* | The game's part drawing, and spawn and appear effects created by the game's own code for the preview (no shadow, no mount; an info line for a mount). Also a "Pointed at" tick box and a light setting. Drawing mounts is a later roadmap item. P25 found. |
| Q-MN41 | An item move and the monster entries that name it? *(2026-10-27)* | The move asks and updates them in the same step; the item tools' Save writes only those references (D60). A missing item is an error. |
| Q-MN42 | P2 in detail: 523, 658–668 and 76? *(2026-10-27)* | 523 the empty model `none`; 658–668 380's look, 658 attackable (the renewed Illusion Temple's Cursed Statue), no talk rules; 76, 252, 296, 585 no model; the bug look with the Ghost model for unknown monsters (D64). |
| Q-MN43 | P13, which castle is which? *(2026-10-28)* | Blood Castle 1 +0, 2–5 +0.05, 6–8 +0.10 (BC8 added), as each castle number's own values, like OpenMU's one definition per castle; this led to D61. |
| Q-MN44 | P14, what a new spawn resets? *(2026-10-28)* | All the listed fields, one fix commit after the move (P14). |
| Q-MN45 | Small changes the move brings? *(2026-10-28)* | Gayion's swords become parts of his model; Maya (364) stops opening the Bull Fighter model; Molt tries its file once per map and shows the bug look; Necron's empty level 7 is left out. Problems get three levels (D65). |
| Q-MN46 | The talk fields? *(2026-10-28)* | `talks`, `talkLevel` with its message, `repairShop`, and talk blocks that replace four NPCs' code (D66); hooks keep editable values. |
| Q-MN47 | Where do the talk texts live? *(2026-10-28)* | In `Monsters.json` as translatable strings, like names (D66). |
| Q-MN48 | P23, the Little Santa message overflows its buffer: when? *(2026-10-28)* | Fixed first in MN5, in its own commit, before the comparison. |
| Q-MN49 | Which numbers become one monster? *(2026-10-28)* | Same English name, kind and model: 36 monsters with 160 numbers (D61). |
| Q-MN50 | What may a number change? *(2026-10-28)* | Any field; Monster Setup shows shared and own values. |
| Q-MN51 | Which number actions in the Monster Browser? *(2026-10-28)* | All: new monster, add, copy, move, split out, remove number, remove monster. |
| Q-MN52 | How is "nothing lost" proven? *(2026-10-28)* | A one-time comparison of every field and language in the conversion commit, then permanent round-trip and merge tests (section 10). |
| Q-MN53 | The data shape of the setup? *(2026-10-28)* | As D63: equipment by item with group and number also for armor, bodies that are no items by part; every monster keeps a `numbers` list, also with one number (the Browser still shows it as one row); a number's own values use the monster's shape. |
| Q-MN54 | P15: the 16 `EnableShadow = false` lines? *(2026-10-28)* | Dead code, deleted with the creators, no field; `m_bRenderShadow` becomes `shadow`. |
| Q-MN55 | P17: own commit? *(2026-10-28)* | No: never reached, and the lines go into per-number data. |
| Q-MN56 | Lines that change nothing (names, kinds, angles, empty weapons, the Canon Tower's dead branch, 227, 228, 276, Necron's level)? *(2026-10-28)* | Deleted with the creators, listed in the removal commit (MN4 details). |
| Q-MN57 | The Doppelganger Dark Lord's horse? *(2026-10-28)* | The Dark Horse helper with `mountLook` (`doppelganger` = the mount's `SubType` 1: see-through on Doppelganger 1, dark elsewhere, no gold trim, no shadow; `ZzzObject.cpp:827`); a "Horse look" row under the helper. What a look does stays code; mount looks as data with an editor go on the roadmap's pets and mounts row. |
| Q-MN58 | `CDirection`'s own model switch? *(2026-10-28)* | The model from the entry, and the cutscene restores the walk speed it changed (P27). Cutscenes as data with an editor go on the roadmap. |
| Q-MN59 | How thorough is the creation recorder? *(2026-10-28)* | Full, as section 10 says. |
| Q-MN60 | The shared problem levels get Fatal: for monsters only or for all tools? *(2026-10-28)* | All tools, and one Data Checks window for every area, in an editor-wide PR before PR 3 (D65). |
| Q-MN61 | How are a monster's numbers told apart in the Browser? *(2026-10-28)* | `maps` per number, filled from OpenMU's Season 6 placements; translated map names; Preview opens on the first map; a check for map variants on other maps. |
| Q-MN62 | P26: the later copies without attack data? *(2026-10-28)* | Their rotation becomes their own value with a Warning, changed per number later; first the attacks PR lets the data decide and allows rotations (D48). |
| Q-MN63 | One PR 3 or several, and the commit order? *(2026-10-28)* | Five PRs in a row (Data Checks, attacks, 3a, 3b, 3c); in each, the moves with their comparisons, the removals, one commit per fix, then the editor windows; this document's update first in the Data Checks PR. |
| Q-MN64 | Where does attack data play once the handlers' number lists go? *(2026-10-28)* | On every map. Of the 115 numbers with `attacks`, OpenMU's Season 6 places only Fire Golem 291 outside its handler's map: it is the boss wave of Devil Square 5, where it plays the generic pattern. The owner takes that as intended, so a map variant keeps it (Q-MN65). Dark Elf 440 (Devil Square 7's boss) already plays its data there: the client loads map 32 as world 9, which the Crywolf handler's Devil Square check names. No web source describes either monster's animations. |
| Q-MN65 | How does a monster attack differently by map? *(2026-10-28)* | With a map variant (D62) of its `attacks`, in the attacks PR; the setup fields follow in their PR. |
| Q-MN66 | Can the client tell the Devil Square rounds and Kalima levels apart? *(2026-10-28)* | Kalima 1–7 are worlds of their own (24–29, 36). Devil Square 1–4 and 5–7 arrive as maps 9 and 32, and `LoadWorld` turns 32 into 9; the client keeps the server's number (D67). The round (1–7) is known only to the entry window. Changed by Q-MN70: OpenMU sends it. |
| Q-MN67 | One way to name maps in data? *(2026-10-28)* | `maps` with server numbers everywhere, `Models.json`'s world lists renamed in the attacks PR; one grouped map picker with server numbers and files folder (D67). The grouped maps already share one folder each; the 122 MB of identical copies in the world data are between different maps and wait for the maps-as-data PR. |
| Q-MN68 | Does a map variant's `attacks` replace the entry's or merge with it? *(2026-10-28)* | It replaces it: the Fire Golem's skill 50 then plays the generic pattern in Devil Square 5–7, as before. |
| Q-MN69 | How does a rotation count? *(2026-10-28)* | In turn, with its own count per monster. `SetPlayerAttack` adds 1 to `SwordCount` on every attack and the generic pattern 1 more, so `SwordCount % n` would play other lists out of order; `generic` keeps today's pattern exactly. |
| Q-MN70 | Can data pick one stage of Devil Square? *(2026-10-29)* | Yes. OpenMU's seven Devil Square maps are numbers 9 (stages 1–4) and 32 (5–7) with discriminators 1–7; it sends the discriminator to the extended client 106.3 (Mosch0512/OpenMU#7) at the end of two packets one byte longer than the originals, which other clients still get: `MapChangedExtended` (C3 1C 0F, 16 bytes) with a map change or teleport, and `CharacterInformationExtended` (C3 F3 03, 93 bytes) when it enters the world, so a character which logs in again in Devil Square keeps its stage. The client keeps it next to the server map; a shorter packet and a revival set it to 0. Only OpenMU matters here: another server sends 0, and its data names whole maps. |
| Q-MN71 | How does a map list name a stage? *(2026-10-29)* | `{"map": 9, "stage": 2}` next to plain numbers in every `maps` list; a number names all its stages. The picker shows Devil Square's seven stages one by one, any of them pickable, and writes a map whose every stage is picked as its number. |
| Q-MN72 | Separate maps or stages for a new event with levels? *(2026-10-29)* | Both: a group's levels are maps of their own (Blood Castle 11–17, 52) or stages of one map (Devil Square), as the server sends them. Groups as data come with the maps-as-data PR. |
| Q-MN73 | Can Monster Attacks play what it edits? *(2026-10-29)* | In the attacks PR: **Play** on each skill row plays its animation once in Monster Preview (beside Monster Attacks, not a tab over it) with the model's attack sound and the attacks' sound; **Play next attack** plays what the next other attack plays: the rotation in turn, the generic pattern and the draws as the game counts them, with **From the start**. Skill effects come with the skill tools. |
| Q-MN74 | Monster Preview plays the sound slots but not the sound events, so Play is silent for the 29 monsters with attacks whose attack sound is an event (Elbeland, Kanturu, Empire Guardian and others). Play them? *(2026-10-29)* | Yes, in the attacks PR: the sound events code gets one function that the game and the preview call. The preview plays a model's events as `PlayMonsterSound` does: when an animation starts, at each loop of stop1 and stop2, and frame by frame in a walk; on any map and at any distance, with the death effect counted as armed. The data and the game stay as they are. |
| Q-MN75 | Sound slots and sound events do the same job in two shapes (no model has both). One shape? *(2026-10-29)* | Yes: the slots become sound events, in a PR of its own right after the attacks PR (MNS, before 3a). Not chosen: keeping both, and sounds under each animation (the attack sounds copied six times and the speed format changed, with the same comparison). |

## Open questions

None at the moment. New questions from the phases are added here.
