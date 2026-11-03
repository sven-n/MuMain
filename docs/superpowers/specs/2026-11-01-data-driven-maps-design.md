# Data-Driven Maps and Map Objects (WIP)

> **Working document.** The maps area of the
> [data-driven content roadmap](2026-09-29-data-driven-content-roadmap-design.md)
> (area MP). It starts with one step, the sounds of map objects (MP1); the
> later steps are listed but not designed yet. Based on MuMain `main` @
> `f81fb1b2c`.

## Goal

What a map does that is not its terrain is hard-coded today: which objects
make sounds and effects, the map's ambient sounds, its names and groups, the
files it loads. It becomes data that MuEditor edits, map by map, without
code. The first step moves the sounds of map objects into data, because the
Kalima waterfall sound is broken and its fix is the first entry.

## Scope

- **MP1 (now):** map object sounds: which object types of which maps play
  which sound while they are drawn, in `Data/Maps/ObjectSounds.json`, edited
  in a Sounds tab of the Map Editor, checked in Data Checks. The first and
  only entry: the Kalima waterfalls.
- **Later (MP2 and on, not designed):** the other object sounds of the maps
  (about 170 sound calls in `World/GameMaps`, many of them monster sounds
  that belong to the monster sound sets instead); the map's own ambient
  loops and timed ambience (Kalima's ambient sound every frame, its second
  and third ambient sounds every 4 seconds); the object effects (most of
  `World/GameMaps`); maps as data (names, groups, server numbers, files
  folders, the code's map helpers, shared files; monsters design D67); and
  the changes of the Map Editor's look and behavior that these need.

## Current state

### How a map object plays a sound

Every frame, `RenderObjectVisual` (`ZzzObject.cpp`) runs for each map object
that passes the camera's view test (`o->Visible`, `TestFrustrum2D`), and the
map's code plays sounds there by object type. Kalima's waterfalls (object
types 37 and 38 of Kalima 1-7) call `PlayBuffer(SOUND_KALIMA_WATER_FALL)` in
`RenderHellasVisual` (`GMHellas.cpp`) for each waterfall drawn, every frame.
`CMapManager::Load` (`MapManager.cpp`) loads the sound with 3 channels and
without 3D, so it plays at full volume at any distance.

### What the original client did

DirectSound's `Play` on a channel that is playing changed nothing: the
calls went round the 3 channels, a playing channel ignored its call, and a
channel that had ended started again. While waterfalls were on screen up to
three copies ran, a frame or two apart; when they left the screen nothing
new started and the copies ran to their end.

### What it does since the move to miniaudio

`MiniAudioBackend::StartSound` stops, rewinds and starts the next channel on
every call. It skips a call while the sound plays only for one-channel
sounds, calls with an object and looped sounds (commit 261a498cf). The
waterfall is none of these: one of its channels restarts every frame, which
sounds like a button pressed again and again.

## Decisions

| # | Decision |
|---|---|
| D-MP1 | **Map object sounds are data** (Q-MP1): `Data/Maps/ObjectSounds.json`; the code that played them by object type goes as each moves. |
| D-MP2 | **One file for object sounds** (Q-MP4): entries with `maps` (server map numbers and stages, as the monster data, monsters design D67), `objects` (object types, 0 to 159), `sound` (`file`, `channels` 1 to 4) and `plays`. Maps that share objects share one entry (Kalima 1-7). Later kinds (map loops, timed ambience) get their own key or file when they are designed. |
| D-MP3 | **"whileInView" plays as the original** (Q-MP5): requested in `RenderObjectVisual` for each drawn object of a type the map's entries name; a channel that has ended starts again, a playing one is left alone, round the channels; full volume at any distance. A new engine call does that (`PlayBufferOnFreeChannel`), used only by the data entries; every other sound keeps today's behavior. |
| D-MP4 | **The sound loads with the map** (where `CMapManager::Load` loaded it): its number is its file's (`Audio/SoundNumbers.h`), so the entry needs no `SOUND_*` id; `SOUND_KALIMA_WATER_FALL` is no longer loaded or played. |
| D-MP5 | **The Map Editor gets a Sounds tab** (Q-MP3), its last tab, for the map the game shows: the entries of that map, add, remove, edit (objects, sound file, channels, plays, maps), Play, pick an object in the world, Save and Revert. The other tabs stay as they are. |
| D-MP6 | **Data Checks gets a "Map sounds" area**: the file's problems and those of the entries in the editor; a click opens the Map Editor. |
| D-MP7 | **Distance volume is a test** (Q-MP6): after MP1, a separate commit lowers the volume of a copy by the drawn object's distance from the hero; the owner keeps it or drops it after listening. |
| D-MP8 | **The engine-wide fix comes later**, as its own small PR: a playing channel is left alone for every sound, as DirectSound did (Q-MP7). The data entries do not need it. |

## Target design (MP1)

### File

```json
{
  "formatVersion": 1,
  "objectSounds": [
    { "maps": [24, 25, 26, 27, 28, 29, 36],
      "objects": [37, 38],
      "sound": { "file": "Data/Sound/aKalimaWaterFall.wav", "channels": 3 },
      "plays": "whileInView" }
  ]
}
```

Fatal (the game does not start, the save is refused): a value of the wrong
form, an entry without maps or objects, an object type out of range, a file
that is not a path below the client, channels out of range, a `plays` that is
not `whileInView`, and an object type of a map in two entries. A warning: a
file that is not in the client folder (the sound is silent), unknown fields.

### Loading and playing

`CMapManager::LoadWorld` loads the map; after `Load()` the object sounds of
the map the server sent (`GetServerMap`, `GetServerStage`) load: each entry
naming it loads its file under its file's number, and a table of object type
to sound is built. `UpdateObjectVisual`, which runs in the update for each
object the last frame drew (ADR 0003), asks the table and plays its sound on a
free channel. Editing in the Map Editor rebuilds the
table and loads a new file at once.

### Editor

The Sounds tab (D-MP5): a note; the map shown (name and server number);
Save, Revert and whether the file has unsaved changes; "Add a sound to an
object type"; the map's entries as a list (objects, sound, channels, plays,
maps); the selected entry's fields with Play, "Pick an object in the world"
(the next click in the world on an object adds its type) and the map picker
for its maps; Remove. Changes play at once; Save writes the file after its
checks, Revert reads it again.

### Verification

Tests: the reader, writer and checks; the shipped file loads clean and is
written as the writer writes it; the engine call leaves a playing channel
alone and starts an ended one; the table plays an entry's sound for its
object types on its maps only; the Kalima waterfall is played from data and
`GMHellas.cpp` plays it no more; the Sounds tab draws without ImGui errors,
adds, edits and removes an entry and saves the file. Each new test fails
against a mutation of the code it covers. In game: Kalima 1 on a test
server, the client log free of sound errors, and the owner listens.

## Steps

| Step | Name | Depends on | Summary |
|---|---|---|---|
| MP1 | Map object sounds | – | D-MP1 to D-MP6; the Kalima waterfalls as the first entry; then the distance volume test (D-MP7). Lands before MN9 of the monsters design. |
| MP1b | Kanturu object sounds | MP1 | The object sounds of Kanturu Ruins and Kanturu Relics, which their maps' object update played: entries for the object types, and an entry without objects for each map-wide hum (Q-MP8). |
| MP2+ | Later | MP1 | Not designed: the other object sounds, map loops and timed ambience, object effects, maps as data, Map Editor changes. |

## Agreed questions

| # | Question | Answer |
|---|---|---|
| Q-MP1 | The Kalima waterfall restarts every frame since miniaudio. Engine fix, the waterfall's object, or map object sounds as data? *(2026-11-01)* | Map object sounds as data, now, only this one feature; a new spec and a roadmap step before cleanup. The engine fix compared: both are useful, data is the long-run architecture. |
| Q-MP2 | Where does the plan live? *(2026-11-01)* | Its own maps area MP with this spec and a roadmap row; MP1 before MN9 in the monsters design's PR table. |
| Q-MP3 | Where in the editor? *(2026-11-01)* | A Sounds tab in the Map Editor. |
| Q-MP4 | Which file? *(2026-11-01)* | `Data/Maps/ObjectSounds.json`, maps shared in one entry. |
| Q-MP5 | How does "while in view" play? *(2026-11-01)* | As the original: up to N copies on free channels, full volume, copies run out after the view moves. |
| Q-MP6 | Distance? *(2026-11-01)* | A separate test commit with the volume lowered by distance; kept or dropped after listening. |
| Q-MP7 | Engine fix too? *(2026-11-01)* | Later, as its own PR; recommended because every sound path benefits. |
| Q-MP8 | Kanturu Ruins and Relics play a map-wide hum from every object's update: while any object of the map is in view. How does the data say that? *(2026-11-01)* | An entry that names **maps but no objects** plays while any object of those maps is in view; the Sounds tab offers "Any object". Not chosen: listing every object type, a new map ambience file (MP2), keeping the hums in code. |
