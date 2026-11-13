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

- `config.ini`, logs, screenshots, macros, the shop's download cache: per-user files;
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
