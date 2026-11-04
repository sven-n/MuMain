# Sound Mixer (editor) design

> Status: agreed with the owner on 2026-11-01 (Q-SM1 to Q-SM12), 2026-11-02 (Q-SM13 to Q-SM15), 2026-11-03 (Q-SM16, Q-SM17). The last step
> moves what lasts into `docs/dev-editor.md` and removes this document.

## Goal

Listen to one sound, or one kind of sound, while the game runs: the Kalima
waterfall alone, the Aida monsters without the music, the game without the
interface clicks. Testing a sound change by ear (map object sounds, the Aida
monster sounds) needs it: today the editor can only mute every one-shot sound
while an effect preview runs (`Audio::EditorMute`), and the game's Options have
one effect and one music volume (0 to 10).

## Decisions

| | Question *(2026-11-01)* | Answer |
|---|---|---|
| Q-SM1 | Where do the options live? | A **Sound Mixer** window of MuEditor (Windows menu), editor builds only. Players' builds and the game's volumes stay as they are. Not chosen: more sliders in the game's Options, both. |
| Q-SM2 | What can be muted? | **Five kinds** plus a **heard just now** list. The kinds are known from where a sound comes from: music, map ambience (looped sounds: wind, rain, water), map object sounds (`Data/Maps/ObjectSounds.json`), monster sounds (sound sets and attacks), everything else. The list shows the sounds of the last 10 seconds with their file and source (monster set and number, map object entry, ambience, other); any of them can be controlled. Not chosen: kinds only, a hand-sorted catalogue of all 894 `SOUND_*` ids. |
| Q-SM3 | Which controls? | **Mute, solo and volume** (0 to 100 %) for each kind and each listed sound. Not chosen: mute and solo, mute only. |
| Q-SM4 | What happens to sounds already playing? | Changes apply **at once**: a muted sound goes silent while it plays, sliders change playing copies, and a muted loop keeps running silently, so unmuting brings it back where it is. Not chosen: only new sounds, mute stops them. |
| Q-SM5 | Are the settings kept? | **Session only**: each start plays everything as normal. While anything is muted, solo or below 100 %, the menu bar says so ("Sound Mixer: 2 muted, 1 solo"); a click opens the window. Not chosen: saved with the reminder, named presets. |
| Q-SM6 | Where does the work go? | On a branch whose base has the monster sounds (sven-n/MuMain#909), the Aida sounds (MNA) and the map object sounds (MP1), so the owner can listen to them with it: `feat/sound-mixer`, the Aida branch with the map object sounds merged in. |
| Q-SM7 | What does a click on a column header do? (asked after trying the table) | It **sorts** by the column; again reverses; a third click goes back to the order of first hearing. **Filters** sit in a bar above the table, not in the columns: text (sound or source contains), kind, show (all, playing now, changed only), loops (all, loops, one-shots), at least N plays; "Clear filters"; "N of M shown". Not chosen: a filter per column behind the header, sort plus a filter row under the header. |
| Q-SM8 | What do Mute all and Unmute all act on? | **The rows shown**: every row the filters leave (no filter = every listed sound). Sounds heard later are not muted and come as new rows. The top button becomes **Reset all** (unmute, unsolo, full volume). Not chosen: every listed sound whatever the filters, the kinds (everything, also later sounds). |
| Q-SM9 | What does the in-world overlay draw? | **Both sources with a position**: a map object with a sound gets a marker, its file and two rings (3 tiles = full volume, 15 tiles = quietest); a monster with a sound set gets a marker, the set's name and a 5-tile hearing ring, solid while the hero is inside it (heard), dashed outside. A source flashes for a second when one of its sounds starts. Music, ambience loops and other sounds have no position and are not drawn. Not chosen: map object sounds only, monster sounds only, markers without rings. |
| Q-SM10 | How is the overlay switched on? | A **Show in world** checkbox in the Sound Mixer, for the session; the menu bar reminder names it while it is on. It **follows the filters**: it draws the sources of the rows shown (no filter = every source in view), and hovering a row highlights its sources. Not chosen: a checkbox that always draws all, two checkboxes (map objects, monsters), a Dev Editor toggle. |
| Q-SM11 | (After trying it: Kanturu's sounds are listed but not drawn.) How does the overlay show the sounds a map's or monster's own code plays, with no data? | **Both: markers now, data later.** While the game moves or draws a map object, and moves or draws a monster, its place is named (`Audio::ScopedSoundPlace`); a sound its code starts there gets a marker at that place and a flash, no ring (the code has no range), and one marker per sound, so a map-wide sound played from every object (Kanturu's hum) shows once. The place changes no sound's kind. Moving a map's sounds into data (object sounds, monster sound sets) follows map by map and gives it rings. Not chosen: data first, leave it. |
| Q-SM12 | (Asked with Q-SM11.) Picking up zen plays nothing, in the original too; what should it play? | The **item pickup sound** (pGetItem.wav), as other picked-up items. Zen dropping keeps the coin sound. Not chosen: the coin sound, silence. |
| Q-SM13 | *(2026-11-02, after an audit found sounds with no marker: players', skills from the server, drops, birds, the interface.)* Which types does the mixer list? | **12 types**: music, map ambience, map object sounds, monsters, NPCs, your character, other players, effects, items, creatures (birds, bats, mice, fish), interface, everything else. A sound's type comes from where it plays: the data's source, else the code's place (the monster, NPC, player, map object or effect whose code runs; the caster of a skill the server sends), else the object the game passes along, else map ambience for a loop, else everything else. An effect counts for whoever made it (your fireball: your character; Charon's sparks: NPCs); one nobody made is an effect. Not chosen: 9 types (players together; effects, items and creatures together), 7 types (the five plus NPCs and players). |
| Q-SM14 | What does a type's checkbox do? | **Hear and Show**: two boxes per type. Hear off mutes the type (it replaces the type's Mute button; the reminder counts it); Show off draws nothing of the type in the world and in the top-left list, but it is still heard. Volume and Solo stay. Not chosen: Hear only, a row of Hear boxes above the old table. |
| Q-SM15 | How do the sounds without a place show? (Interface, ambience loops, music, some server messages.) | **A log at the top left** of the game while Show in world is on: the last 10 starts without a place, newest on top, each with how long ago ("0.2 s  iButtonClick.wav (Interface)"); every start gets its line, a loop once when it starts, a music track when it starts. Lines stay until newer ones push them out and follow the Show boxes and the table's filters. (The owner first picked a live list of what plays, with times, then the log.) Not chosen: a live list while each plays, a section in the mixer window only. |
| Q-SM16 | What does Clear all do with the changed sounds? (The owner, after trying it, 2026-11-03.) | **Clear all puts them back too**: the list empties and every sound's own Mute, Solo and volume are normal again; the types' controls stay (Reset all puts everything back). Before: the changed sounds stayed listed with no plays, so that a muted sound could be found again. |
| Q-SM17 | How does music show in the table? (Self-decided, 2026-11-03: the owner saw no row while the login music played; music only had its type's controls and a log line.) | **A row per track**: each track heard gets a row of type Music: its file below `Data/Music`, its starts, "playing" while it is the music. Its Mute, Solo and volume act on the music while that track plays, times the Music type's; a track keeps its row number for the session, so its controls come back with it. Its Sound cell copies the file and selects nothing in the Sound Editor, which has no music. The log's music lines follow the table's filters like a row. |

## Design

**When a sound plays.** Every sound of the game goes through `PlayBuffer` or
`PlayBufferOnFreeChannel` (`Audio/DSPlaySound.cpp`); music goes through
`PlayMusic`. In editor builds `PlayBuffer` asks the mixer first:

1. The sound's **kind and source**: the source the caller named, if any
   (`Audio::ScopedSoundSource`, below); else a looped sound is map ambience;
   else everything else.
2. The mixer **notes** the sound in its heard list (sound number, file, kind,
   source, time).
3. The mixer gives the sound's **gain**: 0 when the sound or its kind is
   muted, or when something is solo and neither the sound nor its kind is;
   else the kind's volume times the sound's volume. Mute wins over solo; with
   no solo, everything not muted plays.
4. The sound starts with that gain. A muted sound still starts, silently, so
   that unmuting it brings it back (Q-SM4).

**Naming the place.** Most sounds are played by the maps' and monsters' own
code, with no data. `Audio::ScopedSoundPlace` names where that code runs: the
map object in `MoveObject` and `UpdateObjectVisual`, the monster in
`MoveCharacter` and `UpdateDrawnCharacterEffects`. A start under a place and no source is
kept with the place's position for the overlay (Q-SM11); a source wins over a
place, and the kind never comes from a place.

**Types (Q-SM13 to Q-SM15).** The mixer's kinds are the 12 types. A sound's
type is resolved when it starts: the living source (data) wins; else the
innermost living place (`ScopedSoundPlace`, now also for players, NPCs, the
casters of `ReceiveMagic*`, effects as their makers and the interface, which
has no position); else the object `PlayBuffer` was given (the hero, a
character by its kind, a ground item, a bird or fish, an effect); else a loop
is map ambience; else everything else. Each type has Hear (its mute) and Show
(the overlay's markers, rings, flashes and log lines of the type). A start
with no position goes to the overlay's log of place-less starts.

**Naming the source.** `Audio::ScopedSoundSource` names the kind and source of
the sounds played while it lives. The data's players set it: the monster sound
events (`GameLogic/Monsters/MonsterSoundEvents.cpp`) and the attacks' sounds
(`MonsterAttackPlay.cpp`) with the set name and monster number, the map object
sounds (`World/MapInfra/MapObjectSounds.cpp`) with the map and object type. In
players' builds it is an empty class: the game's code does the same as before.

**Gains in the backend.** `IPlatformAudio` gets a mix gain per sound number and
group (the type of the play), and one for music, 1 by default. `MiniAudioBackend`
keeps each channel's volume without the gain and the gain and group it started
with, and multiplies the gain in when a channel starts and when its group's gain
changes, so playing copies follow at once; a copy another type started (the hero's
swing while another player's swing of the same sound starts) keeps its own.
Players' builds never set a gain.

**What the mixer does not touch.** The editor's own previews (Monster Sounds'
and the Map Editor's Play buttons, Monster Preview, the effect preview) play
through `PlayBufferAtLevel` at their own volume: they are heard whatever the
mixer says. The effect preview's mute (`Audio::EditorMute`) stays as it is.

**The window.** Windows → Sound Mixer:

- a line with what is changed and **Reset all**, which puts every control
  back;
- **Kinds**: music, map ambience, map object sounds, monster sounds,
  everything else, each with a volume slider, Mute and Solo;
- **Sounds heard**: one row per sound heard since the list was cleared, in
  the order of first hearing: the file (or the sound number of a sound
  without a file), the kind and source of its last play, how often it
  started, its length, when it last started ("playing", a green row, while it
  plays), a slider, Mute and Solo. **Clear all** empties the list and counts
  again; the sounds whose controls are changed stay listed (changed
  2026-11-03, see Q-SM16: Clear all also puts their controls back). (Changed after
  trying it, 2026-11-01: in the first "heard just now" list of the last 10
  seconds, newest first, the rows came, went and moved, so a sound was hard
  to mute. The backend counts the starts, so a sound asked for each frame is
  not counted each frame.)

**The reminder.** While a kind or sound is muted, solo or below 100 %, the menu
bar shows "Sound Mixer: …" with the counts; a click opens the window.

## Steps

1. The mixer's rules and heard list (`Audio/SoundMixer`), the source scope,
   the backend gains and the `PlayBuffer` hook, with tests.
2. The sources: monster sound events and attacks, map object sounds, music.
3. The Sound Mixer window and the menu bar reminder, texts in English and
   German, with tests.
4. Docs: `docs/dev-editor.md` gets the Sound Mixer; this document goes.

## Tests

The rules (mute, solo, volume, mute over solo, music), the kinds of the
sources (scope, looped, other), the heard list (order of first hearing, starts
counted, Clear all keeping the changed sounds), the counts of the reminder, the backend's gains on playing and new
copies and on music, the sources of the monster events and map object sounds,
the window's rows and buttons. Mutation checks for each.
