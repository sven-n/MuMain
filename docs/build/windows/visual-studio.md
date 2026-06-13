# Windows - Visual Studio

Native Windows build with the MSVC toolchain, driven by the bundled CMake
presets. This produces a full, connectable Windows client (the `.dll` network
library is built by the native Windows .NET SDK).

See [the build guide](../README.md) for shared concepts.

## Prerequisites

- Visual Studio with the **Desktop development with C++** workload (includes
  CMake and Ninja).
- The **.NET 10 SDK** (for the network library and the build-time codegen).

## Build

1. **Open** the repository folder in Visual Studio (`File > Open > Folder`). VS
   reads `CMakePresets.json` automatically.
2. Pick a configure preset from the toolbar:

   | Preset | Arch | Editor |
   |--------|------|--------|
   | `windows-x64` | 64-bit | off |
   | `windows-x64-mueditor` | 64-bit | on |
   | `windows-x86` | 32-bit | off |
   | `windows-x86-mueditor` | 32-bit | on |

3. Build (`Build > Build All`). The output lands in
   `out/build/<preset>/src/<config>/`.

## Run

Set the debug target to `Main.exe` and its **working directory** to the build
output, e.g. `out/build/windows-x64-mueditor/src/Debug/`. The post-build step
copies the assets and `MUnique.Client.Library.dll` there.

> Visual Studio cannot open projects from the WSL filesystem
> (`\\wsl.localhost\...`). Use a repository checked out on a Windows drive.
