# Windows - VS Code

Build the native Windows client with the CMake Tools extension and MSVC. See
[the build guide](../README.md) for shared concepts; the presets are the same
as [windows/visual-studio.md](visual-studio.md).

## Setup

1. Install the **CMake Tools** and **C/C++** extensions, plus the Visual Studio
   C++ build tools and the **.NET 10 SDK**.
2. Open the repository folder. CMake Tools picks up `CMakePresets.json`.
3. Select a configure preset from the status bar (e.g. `windows-x64-mueditor`),
   then **CMake: Build**.

## Run

Set the launch `cwd` to the build output's `src/<config>` directory (e.g.
`out/build/windows-x64-mueditor/src/Debug`) so the client finds its assets and
`MUnique.Client.Library.dll`, with `program` pointing at `Main.exe` there. Or
just run it from an integrated Developer terminal:

```powershell
cd out/build/windows-x64-mueditor/src/Debug
.\Main.exe
```

## Tests

Add `"BUILD_TESTING": "ON"` to the preset's cache variables (or a settings
override); the CMake Tools test explorer runs the CTest cases.
