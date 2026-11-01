# Sound Mixer (editor) design

> Status: agreed with the owner on 2026-11-01 (Q-SM1 to Q-SM6). The last step
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

**Naming the source.** `Audio::ScopedSoundSource` names the kind and source of
the sounds played while it lives. The data's players set it: the monster sound
events (`GameLogic/Monsters/MonsterSoundEvents.cpp`) and the attacks' sounds
(`MonsterAttackPlay.cpp`) with the set name and monster number, the map object
sounds (`World/MapInfra/MapObjectSounds.cpp`) with the map and object type. In
players' builds it is an empty class: the game's code does the same as before.

**Gains in the backend.** `IPlatformAudio` gets a mix gain per sound number and
one for music, 1 by default. `MiniAudioBackend` keeps each channel's volume
without the gain and multiplies it in when a channel starts and when a gain
changes, so playing copies follow at once. Players' builds never set a gain.

**What the mixer does not touch.** The editor's own previews (Monster Sounds'
and the Map Editor's Play buttons, Monster Preview, the effect preview) play
through `PlayBufferAtLevel` at their own volume: they are heard whatever the
mixer says. The effect preview's mute (`Audio::EditorMute`) stays as it is.

**The window.** Windows → Sound Mixer:

- a line with what is changed and **Unmute all**, which puts every control
  back;
- **Kinds**: music, map ambience, map object sounds, monster sounds,
  everything else, each with a volume slider, Mute and Solo;
- **Heard just now**: one row per sound heard in the last 10 seconds, newest
  first: how long ago, the file (or the sound number of a sound without a
  file), the kind and source of its last play, a slider, Mute and Solo. A
  sound whose controls are changed stays listed after the 10 seconds.

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
sources (scope, looped, other), the heard list (10 seconds, changed sounds
kept), the counts of the reminder, the backend's gains on playing and new
copies and on music, the sources of the monster events and map object sounds,
the window's rows and buttons. Mutation checks for each.
