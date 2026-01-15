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
    apply changes. This repository includes a C# .NET 10 client library which is built
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
* **CMake** 3.16 or newer (bundled with Visual Studio and CLion)
* **.NET SDK 10.0** or newer (for building the Client Library)
* **Visual Studio 2022+** with C++ and C# workloads, **CLion**, or **Rider** (see IDE-specific instructions below)
* A compatible server: [OpenMU](https://github.com/MUnique/OpenMU)

### First-time Setup

After cloning the repository, initialize the git submodules:

In Root dir execute:
```bash
git submodule update --init --recursive
```

This will download the required **ImGui** library into `Source Main 5.2/ThirdParty/imgui`.

### Building with CMake and MinGW-w64 (Linux)

The repository also contains a CMake setup to cross-compile the Windows client
from Linux using a MinGW-w64 toolchain.

**Prerequisites**

  * A working MinGW-w64 toolchain (for example `i686-w64-mingw32-g++`).
  * A MinGW-w64 build of libjpeg-turbo which provides a `libturbojpeg` library
    (static or import library) on the library search path of your toolchain.
  * Standard Windows / OpenGL libraries shipped with MinGW-w64 (e.g. `opengl32`,
    `glu32`, `winmm`, `imm32`, `ws2_32`, etc.).

**Example build commands on Linux**

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





### Building the Project

The project uses **CMake** as its build system. The `.NET Client Library` is automatically built by CMake when you build the main project - no manual publishing required!

#### Option 1: Visual Studio 2022+ (Recommended)

1. **Open the project:**
   - File → Open → Folder
   - Select the root `MuMain` folder (not `Source Main 5.2`)

2. **Wait for CMake to configure** (automatically happens, check Output window)

3. **Select build configuration:**
   - Use the dropdown to select `x86-Debug` or `x86-Release`

4. **Build:**
   - Build → Build All
   - Or press `Ctrl+Shift+B`

5. **Run/Debug:**
   - Select `Main.exe` as startup item
   - Press `F5` to debug or `Ctrl+F5` to run
   - Working directory is automatically set to `Source Main 5.2/bin`

**Note:** The working directory is pre-configured in `.vs/launch.vs.json`. If it's not working, ensure you opened the root `MuMain` folder, not a subfolder.

#### Option 2: CLion

1. **Open the project:**
   - File → Open
   - Select the root `MuMain` folder

2. **Wait for CMake to configure** (automatically happens)

3. **Configure working directory:**
   - Run → Edit Configurations
   - Select `Main`
   - Set "Working directory" to: `$ProjectFileDir$/Source Main 5.2/bin`

4. **Build and Run:**
   - Click the hammer icon to build
   - Click the play icon to run

#### Option 3: Rider (CMake via Command Line + Rider for Development)

Rider doesn't have full CMake support for C++ projects, so you need to generate a Visual Studio solution first:

1. **Generate the solution** (one-time setup):
   ```bash
   cmake -B build -G "Visual Studio 17 2022" -A Win32
   ```
   *(Adjust the generator version based on your installed Visual Studio)*

2. **Open in Rider:**
   - File → Open
   - Select `build/MuMain.sln`

3. **Build and Run:**
   - Build → Build Solution
   - Run → Run 'Main'

**Important:** When you modify `CMakeLists.txt`, you must manually regenerate the solution by running the cmake command again.

#### Option 4: Command Line Build (Windows)

```powershell
# Configure (first time only, or when CMakeLists.txt changes)
cmake -B build -G "Visual Studio 17 2022" -A Win32

# Build and run Debug
cmake --build build --config Debug --target run

# Or build and run Release
cmake --build build --config Release --target run
```

**Note:** You can switch between Debug and Release without reconfiguring. The `run` target automatically builds the project and runs it from the correct working directory (`Source Main 5.2/bin`).

**To start fresh (clean build):**
```powershell
Remove-Item -Recurse -Force build
cmake -B build -G "Visual Studio 17 2022" -A Win32
```

**Manual execution (if needed):**
```powershell
# Build only
cmake --build build --config Debug

# Run manually
cd "Source Main 5.2/bin"
../../build/"Source Main 5.2"/Debug/Main.exe
```

#### Option 5: Command Line Build (Linux)

For native Linux builds or when using Unix Makefiles/Ninja generators:

```bash
# Configure for Debug (first time or when switching build types)
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Build and run
cmake --build build --target run

# To switch to Release, reconfigure:
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target run
```

**To start fresh (clean build):**
```bash
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
```

**Manual execution (if needed):**
```bash
# Build only
cmake --build build

# Run manually
cd "Source Main 5.2/bin"
../../build/Main
```

---

### Running the Client

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