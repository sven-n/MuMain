# Windows - Terminal (CMake presets)

Native MSVC build from a **Developer Command Prompt / Developer PowerShell for
VS** (so `cl`, CMake, and Ninja are on `PATH`), using the bundled presets.

See [the build guide](../README.md) for shared concepts. To cross-compile the
Windows client from Linux/WSL instead, see [wsl.md](wsl.md).

## Prerequisites

- Visual Studio C++ build tools (CMake + Ninja).
- The **.NET 10 SDK**.

## Configure and build

```powershell
# Configure (pick one)
cmake --preset windows-x64                # 64-bit
cmake --preset windows-x64-mueditor       # 64-bit + editor
cmake --preset windows-x86                # 32-bit
cmake --preset windows-x86-mueditor       # 32-bit + editor

# Build (pick the matching Debug/Release build preset)
cmake --build --preset windows-x64-mueditor-debug
cmake --build --preset windows-x64-mueditor-release
```

The configure presets are listed in `CMakePresets.json`; each has
`-debug`/`-release` build presets.

## Run

```powershell
cd out/build/windows-x64-mueditor/src/Debug
.\Main.exe
```

Run from the build output's `src/<config>` directory so the client finds its
assets, `config.ini`, and `MUnique.Client.Library.dll` (all placed there by the
post-build step).
