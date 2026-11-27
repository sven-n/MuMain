# Content access

The game reads its content through one layer, `src/source/Core/Content`, instead of opening files
itself. On desktop the content is the client folder, as before. A port serves it from somewhere
else (an Android APK, a browser's downloads) without changing any code that reads it.

## What content is

Content is everything the game ships and only reads:

- `Data/`: tables, models, textures, sounds, maps, UI themes;
- `fonts/`;
- `shaders/`;
- the window icon.

These are not content and stay on the file system:

- `config.ini`, logs, screenshots, macros: per-user files in the user folder
  ([where settings and logs live](build/README.md#where-settings-and-logs-live));
- the shop's download cache, still under `Data/`;
- what the editor and the developer tools save.

## Paths

Content is named by a logical path, `Content::Path`:

- relative to the content root, with `/` between folders, as in `Data/Items/Items.json`;
- `\` is accepted and becomes `/`; `.` parts and doubled separators are dropped;
- an absolute path, or one with `..`, is not content.

A lookup ignores case on every platform. `data/items/items.json` finds `Data/Items/Items.json` on
Windows, Linux and in an APK alike.

- If the path exists as written, that file is read.
- Otherwise each folder is listed once and the name is matched ignoring case.
- If two names in a folder differ only in case, the first in sort order is read, and the log
  says so once.

## Reading

```cpp
#include "Core/Content/ContentStore.h"

std::string error;
if (const auto bytes = Content::ReadAll(*Content::Path::From("Data/Items/Items.json"), &error))
{
    // the whole file
}
else
{
    // error is "Data/Items/Items.json: not found"
}
```

The layer has these calls:

- `ReadAll(path)`: the whole file. Reads are whole files and synchronous; a reader parses the
  bytes it gets.
- `Exists(path)`: true for a file (not a folder).
- `OpenIO(path)`: an `SDL_IOStream` over the file's bytes, for code that reads piece by piece
  (`DataFileIO::ReadExact`, `SDL_ReadIO`) or a library that takes a stream.
- `List(folder)`: what a folder holds, sorted. An empty folder name is the root.

Code that takes a path from its caller, where tests and the editor pass absolute paths, uses the
`std::filesystem::path` forms instead:

- `ReadFile`, `Probe` (a file, a folder, nothing, or a disk path it can't check), `ListFolder`;
- a relative path is content;
- an absolute path, or one that leaves the content, is read from disk.

`Data::ReadTextFile` reads this way and drops a UTF-8 byte order mark. Every JSON data table is
read through it.

The UI themes read this way too:

- RmlUi opens its documents, stylesheets, templates and fonts through `ThemeFileInterface`, which
  serves each file from its `ReadFile` bytes;
- `theme.ini` and `tokens.ini` are parsed from their bytes;
- the theme list is a `ListFolder` of `Data/Interface/RmlUi/themes`.

A theme's own files are named by relative paths, so they are content. A font the theme names by an
absolute path (a system font) is read from disk.

Reads are safe from any thread, such as the texture decode workers. `Open` and `Close` are not
thread-safe, and run only while nothing reads.

## Writing

Writes stay on the file system. After writing into the content folder, call
`Content::Refresh()` so that later lookups list that folder again and find the new file in any
case. `Data::WriteTextFile(s)` and the look files already call it.

## Where the content comes from

At startup, `StartClient` (`Winmain.cpp`) opens SDL's title storage at the client folder and hands
it to `Content::Open`. The error report then names the folder in its `> Content:` line.

- On desktop, the client folder is the executable's folder.
- On Android, SDL's title storage reads the APK's assets.
- A port with another source passes its own storage: `SDL_OpenStorage` with an
  `SDL_StorageInterface` that implements `ready`, `enumerate`, `info` and `read_file`. Nothing
  else in the game changes.

If nothing is opened (some tests and tools), content is read relative to the working directory.
Names still match in any case there, but a folder is listed again on every lookup that misses,
because a test may have changed the working directory.

## Tests

`tests/core/ContentMemoryStorage.h` is such a storage: over files held in memory, or over a
folder whose files it reads only when the game asks for one.

- `ClientDataScope` uses it when `MU_TEST_CONTENT=storage` is set: the scope then reads the shipped
  client data through it, with each file's bytes read only when asked for.
- The scope then leaves the working directory where it is, so a loader that goes around the layer
  finds nothing and fails that run.
- CTest runs each case of `monster_data_tests` and `test_item_database_files` both ways. The
  second entry of a case starts with `content storage: `.

## The check

The `content_io_goes_through_the_content_layer` test (`tests/check_content_io.py`) looks for code
in `src/source` that opens, probes or lists files itself:

- `fopen`, `ifstream`, and `.open(` on a stream;
- `std::filesystem` probes and listings;
- the Win32 file calls `CreateFile`, `FindFirstFile`, `GetFileAttributes`;
- `SDL_IOFromFile`, `SDL_LoadFile`, `SDL_LoadPNG`;
- `TTF_OpenFont`, `cgltf_parse_file`, the miniaudio file loaders.

`tests/check_content_io_baseline.txt` lists the calls that stay on disk. Each line is a file, a
kind of call, how many it may have, and why they stay:

- the platform shims, the Win32 and CRT file calls themselves;
- per-user files: settings, macros, the shop's download cache;
- system information, such as `/etc/os-release`;
- what the editor and the savers write, and the editor's imports.

A line without a reason is a call still to move. The check lists it as `still to move:` and
passes. When every call is on the layer or has its reason, the check prints nothing.

- A new call fails the check.
- A file that lost calls also fails it until the baseline is lowered with
  `python tests/check_content_io.py . --update`. The update keeps the reason of each line that
  remains; a new line gets none.

## Per-map file lists

`src/bin/Data/Maps/Files` says which content files each part of the game needs, so a port can ship,
preload or download one map's data on its own:

- `Shared.json`: what start-up, the login scene and character selection read, plus the folders
  shipped whole (`Data/Interface`, `Data/Local`, `Data/Render`, `fonts`) and the images the
  interface documents name outside their folder. Every player, item and skill model loads at
  start-up, so other players' equipment and dropped items are here, with the player bodies of both
  player-model choices (the option applies at the next start) and the dragons an event sends over
  any map.
- `Map<n>.json`, by the map number the server sends: what entering the map reads beyond the shared
  files, in each of its event states (castle siege, Crywolf, Kanturu, Raklion), with the monsters
  and NPCs the server spawns there. Lists may overlap; a file read from another map's folder at
  start-up (`Object31/flag.OZT`) is only in the shared list.
- `Monsters.json`, by monster number: what one monster or NPC reads beyond the shared files. A
  server can spawn any monster on any map (invasions, events, summons, transformation rings, GM
  commands), so a port fetches the monster's files when the spawn arrives if the map did not bring
  them. At start-up the game checks `Data/Monsters/Models.json` against the content and gives a
  model whose file is missing no body; a client that adds monster files later has to run that check
  again.

Paths are the files' stored names, as `Content::List` gives them.

### How they are made

`map_file_lists_tests` (`tests/monsters/test_map_file_lists.cpp`) runs the game's own loaders on the
shipped client folder, without a window or a server, through a `ContentMemoryStorage` that names
every file read. Each map and monster starts from the state right after start-up, as a port that
enters it first would. The test fails when the lists in the repository differ from what it recorded.

Every server map whose world folder has a terrain and a height map gets a list; the login and
character scenes' worlds are not maps and are left out. The maps' spawns come from
`tools/MapFileLists/openmu-spawns.json`, which `python tools/MapFileLists/export_openmu_spawns.py`
writes from an initialized OpenMU database (with `PGHOST`, `PGPORT`, `PGUSER`, `PGPASSWORD`).
Another server can export its own spawns in the same format.

The run has no frames, so what the game opens only while it plays a map is named in the test:
`FrameModels` (Kalima's Bahamut), and the event dragons are opened in the shared phase.

To rewrite the lists, build `map_file_lists_tests` and run it with `MU_WRITE_MAP_FILE_LISTS=1`.
Rerun it, and commit the lists, after:

- a loader reads another file, or content files are added, renamed or removed;
- a new map, a new event state that loads other files (add it to `MapStates`), or a model the
  game opens while it plays a map (add it to `FrameModels`);
- a new monster or NPC in `Data/Monsters/Monsters.json`;
- a new spawn export, when OpenMU's spawns changed. CI has no OpenMU database, so it cannot tell
  that the spawn file is old; a stale one only makes a port fetch a monster later.

The run uses the English language files; the other languages ship whole in `Data/Local`. Sounds
(`Data/Sound`) and music (`Data/Music`) are not in the repository, so no list names them: a port
that has them ships both folders with the shared files. The same holds for an installed texture
pack (`Data/TexturePacks`).

## Packs that arrive after start-up

A target that cannot hold every file at start-up (a browser's downloads, an app store's asset packs) plugs a pack source
into `Content::SetPackSource`. The game asks it by pack name (`Content::RequestPack`) and the source answers ready,
pending or failed. Where there is no source, every pack is ready.

- `map<n>`: a map's files, as `Data/Maps/Files/Map<n>.json` lists them. A map change asks for it and waits (the old
  map stays on screen) before its first load step. A failed pack drops the map change.
- `monster<n>`: one monster's or NPC's files, as `Data/Maps/Files/Monsters.json` lists them under its number. A server
  can spawn any monster on any map, so `CreateMonster` asks for the pack at each spawn; the source answers ready for a
  pack it already has, such as the monsters of the map's own pack.
  - While the pack is pending the monster stands in as a Bull Fighter's body, with its name, and the frame's pump
    (`RemakeMonstersWhoseFilesArrived`) makes it again, in place and at the tile it stands on, once the pack is ready.
    What the server sent for it in between, such as its life and buffs, is not kept: only the spawn is.
  - A failed pack is logged once per monster and the monster is made as usual, which gives the unknown monsters' look
    to a model whose file is missing.
  - A session that is torn down drops the waiting monsters.
