# MU Online Client Sources

[![MinGW Build](https://github.com/sven-n/MuMain/actions/workflows/mingw-build.yml/badge.svg?branch=main)](https://github.com/sven-n/MuMain/actions/workflows/mingw-build.yml)

This is my special fork of the Season 5.2 client sources [uploaded by Luois](https://github.com/LouisEmulator/Main5.2).

The ultimate goal is to clean it up and make it compatible and feature complete
to Season 6 Episode 3.

What I have done so far:
  * 🔥 The framerate has been increased.
    * By default, it uses V-Sync without fps limit. If V-Sync is not
    available, it limits to 60 fps.
    * The options menu includes a checkbox to reduce effects to achieve higher frame rates.
    * Chat commands:
      * Change FPS-Limit: `$fps <value>`
      * Enable V-Sync: `$vsync on`
      * Disable V-Sync: `$vsync off`
  * 🔥 Optimized some OpenGL calls by using vertex arrays. This should result in
    a better frame rate when many players and objects are visible.
  * 🔥 Added inventory and vault extensions.
  * 🔥 The master skill tree system was upgraded to Season 6
  * 🔥 Unicode support: The client works with UTF-16LE instead of ANSI in memory.
    All strings and char arrays have been changed to use wide characters.
    Strings coming from files and the network are handled as UTF-8.
  * 🔥 Replaced the network stack with MUnique.OpenMU.Network to make it easier to
    apply changes. This repository includes a C# .NET 9 client library which is built
    with Native AOT.
  * 🔥 The network protocol has been adapted for Season 6 Episode 3 - there is probably
    still some work to do, but it connects to [OpenMU](https://github.com/MUnique/OpenMU)
    and is playable. Additionally, the protocol has been extended so it's not standard
    anymore.
    * Damage, Exp etc. can exceed 16 bit now.
    * Improved item serialization
    * Improved appearance serialization
    * Added monster health status bar after attack
  * 🔥 Significant changes from Qubit have been incorporated, such as
    * Rage Fighter class
    * Visual bug when Dark Lord walks with Raven
    * Item equipping with right mouse click
    * Glow for red, blue and black fenrir
    * Additional screen resolutions
  * 🔥 Incorporated MU Helper UI and logic - there's some work to do but core functionality is usable
  * Removed if-defs for Rage Fighter class as we are targeting Season 6, so Rage
    Fighter should always be included.
  * Some minor bug fixes, e.g.:
    * Storm Crow item labels
    * Ancient set labels
  * The code has been refactored. A lot of magic values have been replaced by
    enums and constants.

What needs to be done for Season 6:
  * Lucky Items

## How to build & run

### Requirements
  * **Visual Studio Build Tools 2022** (minimum requirement - older versions won't work, newer versions are not tested)
  * Visual Studio 2022 with the newest update, workloads for C++ and C#, Jetbrains Rider also Works
  * A compatible server: [OpenMU](https://github.com/MUnique/OpenMU)

### First Time Setup - Initialize Submodules

The project uses ImGui as a git submodule for the in-game editor (debug builds only). After cloning the repository, you must initialize the submodules:

```bash
# From the repository root
git submodule update --init --recursive
```

This will download the ImGui library into `Source Main 5.2/ThirdParty/imgui`.

**Note:** This is only required for **Debug builds** (`Global Debug` configuration). Release builds do not require ImGui.

### Updating ImGui Submodule

To update ImGui to the latest version:

```bash
# From the repository root
git submodule update --remote "Source Main 5.2/ThirdParty/imgui"
```

### Build Configurations

#### Debug Builds (`Global Debug`)
- Includes the in-game MU Editor (ImGui-based)
- Requires ImGui submodule to be initialized (see above)
- Press **F12** in-game to toggle the editor
- Start with `--editor` flag to launch with editor enabled
- Preprocessor define: `_EDITOR`

#### Release Builds (`Global Release`)
- No editor code included
- ImGui submodule not required
- Optimized for production use
- Zero editor overhead

### Building with CMake and MinGW-w64 (Linux)

The repository also contains a CMake setup to cross-compile the Windows client
from Linux using a MinGW-w64 toolchain.

**Prerequisites**

  * A working MinGW-w64 toolchain (for example `i686-w64-mingw32-g++`).
  * A MinGW-w64 build of libjpeg-turbo which provides a `libturbojpeg` library
    (static or import library) on the library search path of your toolchain.
  * Standard Windows / OpenGL libraries shipped with MinGW-w64 (e.g. `opengl32`,
    `glu32`, `winmm`, `imm32`, `ws2_32`, etc.).

**Example build commands**

From the repository root:

```sh
cmake -S . -B build-mingw \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw-w64-i686.cmake \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build-mingw -j$(nproc)
```

If the linker reports `cannot find -lturbojpeg`, install a MinGW-w64 build of
libjpeg-turbo (providing `libturbojpeg.a` / `libturbojpeg.dll.a`) or adjust the
`target_link_libraries` entry in `Source Main 5.2/CMakeLists.txt` to match the
name of the library available on your system.

---

Because of the integrated C# code, you need to publish the ManagedLibrary first
to the debug output folder of the main.exe, so that the DLL is built with Native AOT.
A simple build is not enough in this case, however the publish just needs to be done once.

It supports the common starting parameters `/u` and `/p`, example: `main.exe connect /u192.168.0.20 /p55902`.
The [OpenMU launcher](https://github.com/MUnique/OpenMU/releases/download/v0.8.17/MUnique.OpenMU.ClientLauncher_0.8.17.zip)
will work as well. By default, it connects to localhost and port `44406`.
The client identifies itself with Version `2.04d` and serial `k1Pk2jcET48mxL3b`.

## Credits

  * Webzen
  * Louis
  * Qubit (tuservermu.com.ve)
  * Community members of RaGEZONE and tuservermu.com.ve for posting fixes
  * [Nitoy](https://github.com/nitoygo) for the MU Helper