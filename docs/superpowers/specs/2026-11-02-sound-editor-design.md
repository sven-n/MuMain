# Sound Editor design

> Status: agreed with the owner on 2026-11-02 (Q-SE1 to Q-SE4), 2026-11-03 (Q-SE5 and on). The last step
> moves what lasts into `docs/dev-editor.md` and removes this document.

## Goal

See every sound the game loads in one place, give each a name, swap its
file, and find where a sound belongs, without reading C++. Today the code
names 388 sounds and their files in 409 `LoadWaveFile` calls
(`Engine/Object/ZzzOpenData.cpp` at startup, `World/MapInfra/MapManager.cpp`
when a map is entered, `GameLogic/Events/Event.cpp`); the monster sound sets
(`Data/Monsters/Sounds.json`, 157 sets, 586 sounds) and the map object sounds
(`Data/Maps/ObjectSounds.json`, 9) are data already. Which sound plays when
is decided in C++ for the code's sounds.

Found while looking (2026-11-02): `SOUND_FENRIR_DAMAGE_1` is loaded twice
(`pWpain1.wav`, then `pWpain2.wav`) and `SOUND_FENRIR_DAMAGE_2` never, so
every other Fenrir hit is silent; Nightmare's skills 2 to 4 name
`w39/nightmare_skill-01` to `-03` without `.wav` (the files exist);
`BattleCastle/sDSwllMana.wav` is a typo of `sDSwellMana.wav`; four Battle
Castle files the code names are not in the client (`oProtectionDis.wav`,
`mGMercAttack.wav`, `sDStunCancel.wav` twice).

## Decisions

| | Question *(2026-11-02)* | Answer |
|---|---|---|
| Q-SE1 | Where do the code's file names live, what can be edited, and what is a sound's name? | **All in data, the engine way.** `Data/Audio/SoundFiles.json` holds the code's 388 sounds (id, file, channels, 3D); the code still loads each where it does today but takes them from the data (checked identical). The monster sets and map object sounds stay in their files. In the editor a sound's file is picked from the files in `Data/Sound` (search, play first) and its channels and 3D changed; the sound reloads at once; the files on disk are never touched (they are not in git and every build folder links one shared client folder). Like Unity's GUIDs or Godot's `uid://`, the `SOUND_` id stays the hidden identity that C++ and the data (751 references) point at; every sound gets a free **name** in data, shown in every editor window instead of the id, so a rename never breaks anything. **Sound events now:** which sound plays when moves into data (`Data/Audio/SoundEvents.json`, an Events tab), starting with the hero's footsteps and the weapon swings, checked identical; other systems follow one at a time. Not chosen: only changed files in data, files and load moments in data, a rename on disk, names as data keys, keys that generate the C++ ids, events later. |
| Q-SE2 | What is a sound's type in the editor? | **A stored fallback type**: a type per code sound in data, used only when no source or place names one (level up, quest prize, trade, whisper alerts get a type), next to the live "heard as" of this session. Not chosen: heard as only, a stored type that always wins. |
| Q-SE3 | How do the Sound Mixer and the Sound Editor follow each other? | **Quietly**: a click on a sound in either window selects it in both; a closed or hidden window is not opened or raised, it shows the sound selected when it is looked at. The mixer's copy on click stays. Not chosen: open and raise the other window, a menu item per row. |
| Q-SE4 | How are sounds grouped? | **Folders**, as in the Colors window: nested folders, each sound in exactly one (else "Not sorted"), new, rename, remove (contents move up), move, drag and drop; seeded from what is known (the monster sets, the maps' loads, the files' prefixes `i` interface, `p` player, `s` skill, `m` monster, `e` effect, `a` ambience, `n` NPC). Editor only: `Data/Audio/SoundFolders.json`, which the game never reads. Not chosen: tags, groups from data only. |
| Q-SE5 | Can a sound be heard from its row? (The owner, after trying it, 2026-11-03.) | **A Play button on every row**, before the file: the file once at full volume, past the Sound Mixer, as the picker's play button does; greyed out without a file or when the file is not in the client folder. |
| Q-UI1 | Do the server, channel and character screen buttons and a click on a character make a sound, and where is it chosen? (The owner, after trying it, 2026-11-03: only the login's OK and Cancel sounded; the original code played nothing for the others either. Asked twice; the owner wanted both main options compared with code, data and mockups first.) | **The mix**: a button's theme style gives it a click kind (`click-sound: confirm`, also cancel, select, menu; a custom RmlUi property), one listener hears every click before the callbacks, and the `interfaceClick` event of `SoundEvents.json` picks each kind's sound in the Events tab, where the tab shows a Click kinds field. A click on a character is the `characterSelected` event. The login's OK and Cancel move their click there (same sound); Enter and Escape on the login and Enter on the character screen play the button they stand for. Every kind plays the click OK played, until changed. Not chosen: a sound event per button named in code, the sound itself in the style, no sounds. |
| Q-SE6 | Where does a skill several classes learn go? (The owner, 2026-11-03, after moving sKnightSkill4.wav to Twisting Slash.) | **Skills / Shared / <the classes> / <skill>**, by who learns the skill in the game's skill file (`Data/Local/Eng/skill_eng.bmd`, RequireClass): Meteorite and Ice under "Dark Wizard + Magic Gladiator + Summoner", Twisting Slash and Impale under "Dark Knight + Magic Gladiator", the weapon skills Lunge, Uppercut and Defense under "Dark Knight + Magic Gladiator + Dark Lord". A skill one class learns stays in its folder; two classes' skills of one name ("Explosion") count apart. Not chosen: by the classes whose skills play the sound (a file reused by another skill would pull it into Shared), one class folder per skill. |
| Q-SE7 | Where is a row's Play button? (The owner, 2026-11-03, with mockups of five places.) | **A column of its own at the far left**: a play arrow per sound, empty for folders, pinned with the names when the table scrolls sideways. With it every column got a width of its own and the table scrolls sideways: the File column stretched to fill the window, so a drag on its border took the room from the next column and stopped after a few pixels (found then too). Not chosen: before the file (the word or an arrow, where it was first), after the file next to its picker, no button and a play of the selected row. |

## Design

**Where the files are.** `Data/Audio`, not `Data/Sound`: a build folder's
`Data/Sound` links the shared client folder, which the asset copy would write
into (decided while building, 2026-11-03).

**SoundFiles.json.** `{"formatVersion": 1, "sounds": [{"id": "SOUND_CLICK01",
"name": "Button click", "file": "Data/Sound/iButtonClick.wav", "channels": 1,
"spatial": false, "type": "interface"}]}`; `name`, `spatial` and `type` are
optional. Each former `LoadWaveFile(SOUND_X, L"...", ...)` becomes
`LoadGameSound(SOUND_X)` at the same place; an id the data lacks is an error in
`MuError.log` and stays silent. The first file is generated from the 409
calls, and a check proves that every load gets the file, channels and 3D it
had. The four fixes above are data edits in commits of their own.

**Names.** `name` in `SoundFiles.json`, in a monster set's sound
(`Sounds.json`) and in a map object sound (`ObjectSounds.json`); none: the
id, or the set and file. The game reads and ignores them. The Sound Editor,
the Sound Mixer (Sound column; the tooltip keeps file and id), the overlay's
labels and the Monster Sounds window's event picker show them.

**The Sound Editor** (Windows menu): one tree table of every sound (983 rows
today) in its folders: name (edited in the cell, the id in grey), file (the
picker), channels, 3D, when it loads (as seen this session: startup, the map
being entered, with its monster; "not loaded yet" before), the
type if no place, heard as, the data file it lives in. Search finds names,
ids and files; "Only problems" leaves the files that are missing on disk, the
ids loaded twice and the ids played but never loaded. Edits of code sounds
have their own undo and Save (`SoundFiles.json`); a set's sound goes through
the Monster Sounds window's history and Save, a map object sound through the
Map Editor's, so two windows never write one file differently.

**Types.** The mixer resolves a start's type as in Q-SM13, and before
"everything else" takes the code sound's stored `type`.

**Selection.** One selected sound for both windows; each draws it highlighted
and scrolls to it when it changed while the window was visible or the next
time it is drawn. A sound not heard yet has no mixer row.

**Folders.** `Data/Audio/SoundFolders.json`: `{"formatVersion": 1, "folders":
[{"name": "Monsters", "folders": [...], "sounds": ["SOUND_CLICK01",
"set:Death Tree:Data/Sound/w34/dt_idle1.wav", "objects:<entry>"]}]}`; a
sound named twice keeps its first place; stale entries are warned and
dropped on save.

**Sound events.** `Data/Audio/SoundEvents.json`: per event an ordered list of
rules, the first that matches plays; each event has the conditions its code
can tell (footsteps: map, ground tile, safe zone, not flying; swings: what
the character holds: bow, crossbow, magic weapon, light saber or spear, any
weapon). The Events tab edits the rules and picks the sounds by name.

## Steps

1. This document.
2. `SoundFiles.json` and `LoadGameSound`, checked identical; then the
   Fenrir, Nightmare and Battle Castle fixes, each its own commit.
3. The Sound Editor's table: names, the file picker with play, channels and
   3D, reload, Save, undo, problems; names in the mixer, overlay and
   Monster Sounds.
4. The stored type; the shared selection.
5. Folders, seeded.
6. Sound events: footsteps and swings, checked identical; the Events tab.
7. Docs: `docs/dev-editor.md` gets the Sound Editor; this document goes.

## Tests

The data's reading and writing (names, types, optional fields), every load
unchanged against the old calls, the editor's edits, undo and save per file,
reload, problems, the fallback type in the mixer, the shared selection, the
folders (seed, moves, stale entries), each event's rules against the old
code on every map and condition. Mutation checks for each.
