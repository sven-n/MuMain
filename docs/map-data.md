# Map data

What a map does besides its terrain is moving into data, step by step
([maps design](superpowers/specs/2026-11-01-data-driven-maps-design.md)).
The first part is the sounds of map objects; the light mood of the maps has
its own page ([light-mood.md](light-mood.md)).

## Object sounds

`Data/Maps/ObjectSounds.json` says which object types of which maps play a
sound while they are drawn: the Kalima waterfalls play their water sound;
Kanturu Ruins' objects play their wheel, waterfall, electric and plant
sounds, Kanturu Relics' their gear and incubator sounds, and both maps their
hum.

```json
{
  "formatVersion": 1,
  "objectSounds": [
    {"maps": [24, 25, 26, 27, 28, 29, 36], "objects": [37, 38], "sound": {"file": "Data/Sound/aKalimaWaterFall.wav", "channels": 3}, "plays": "whileInView"}
  ]
}
```

| Field | Value |
|---|---|
| `maps` | The maps, by the number the server sends; `{"map": 9, "stage": 2}` names one stage of a map (Devil Square). Maps with the same objects share one entry, as Kalima 1-7 do. |
| `objects` | The object types of those maps, 0 to 159: the numbers of the models in the map's `Data/Object<n>` folder, as the Map Editor's Objects tab lists them. Left out: **any object** of the maps (a map-wide sound, see below). |
| `sound.file` | The sound file below the client folder. It gets a number when the map loads it; no `SOUND_*` id is needed. |
| `sound.channels` | 1 to 4, default 1: how many copies of the sound can play at once. |
| `plays` | `whileInView` (the default and the only one so far). |

**While in view** plays a sound the way the original client did: every
frame an object of the type is drawn (it is on screen), a channel of the
sound that has ended starts again and a playing one is left alone. With 3
channels, up to three copies run, a frame or two apart. When the objects
leave the screen, nothing new starts and the copies play to their end. A
copy starts at full volume within 3 tiles of the hero and quieter the
farther the nearest object of its sound drawn that frame is, down to a
quarter at 15 tiles (a test of how it feels; the original played at full
volume at any distance).

An entry without `objects` plays while any object of its maps is drawn, at
full volume and asked for once a frame: the hum of a whole map, which Kanturu
Ruins and Kanturu Relics played from every object they moved (maps design,
Q-MP8):

```json
{"maps": [37], "sound": {"file": "Data/Sound/w37/kan_ruin_global.wav"}, "plays": "whileInView"}
```

The map loads the sounds of its entries when it loads (after its own
sounds). The file is read at start; a fatal problem stops the game with a
message, as the other data files do.

### Problems

Fatal: a value of the wrong form (the problem names the entry and field,
e.g. `objectSounds[0].sound.channels`), an entry without maps, an empty
list of object types, an object type out of range or twice, a file that is not a path below
the client folder, channels out of range, a `plays` other than
`whileInView`, and an object type that two entries name on one map (which
would play?). A warning: a file that is not in the client folder (the sound
stays silent) and unknown fields.

## Editing (MuEditor)

The Map Editor's **Sounds** tab (its last tab) edits the object sounds of the
map the game shows. It lists the entries that name the map (objects, file,
channels, plays, maps); a click selects one. **Add a sound to an object
type** adds an entry for the map. The selected entry's fields:

- **Objects**: **Any object** (a map-wide sound) or **Types**: the types,
  each with a button that removes it; **Add a type**
  lists the map's object models; **Pick an object in the world** takes the
  type of the next object you click in the world (the hovered object is
  outlined, as in the Objects tab).
- **File** with **Play**, **Channels**, **Plays** and **Maps** (the map
  picker of the monster tools).
- **Remove** removes the entry.

Changes play at once. **Save** writes the file after its checks (a fatal
problem keeps the file as it was and says why); **Revert** reads it again.
"Not saved yet" shows while the tab has changes the file lacks. Data Checks
lists the problems under **Map sounds**; a click opens the Map Editor.
