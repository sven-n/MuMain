# Codebase Structure

**Analysis Date:** 2025-02-08

## Directory Layout

```
MuMain/
├── ClientLibrary/          # .NET network stack (Native AOT → DLL)
├── ConstantsReplacer/      # WinForms tool for SQL constant replacement
├── cmake/                  # CMake toolchains (MinGW)
├── docs/                   # Build guide
├── src/                    # Main client source
│   ├── bin/               # Game assets, translations, required DLLs
│   ├── dependencies/      # Third-party headers/libs (GL, turbojpeg, netcore)
│   ├── MuEditor/          # In-game editor (ImGui, Debug only)
│   ├── source/            # Core C++ game logic
│   │   ├── DataHandler/   # Data loading
│   │   ├── Dotnet/        # .NET bridge, packet bindings
│   │   ├── Scenes/        # Scene management
│   │   ├── Translation/   # i18n system
│   │   └── ...            # Game systems (GM*, CS*, NewUI*, etc.)
│   └── ThirdParty/        # imgui submodule, json.hpp
├── .github/workflows/      # CI (MinGW builds)
├── stylecop.json          # C# StyleCop
├── .editorconfig          # C# formatting
└── CMakeLists.txt         # Root CMake (if present) or src/CMakeLists.txt
```

## Directory Purposes

**ClientLibrary/:**
- Purpose: C# network layer, published as `MUnique.Client.Library.dll`
- Contains: `ConnectionManager*.cs`, XSLT transforms, packet generation
- Key files: `ConnectionManager.cs`, `MUnique.Client.Library.csproj`
- Build: Native AOT via CMake custom command; XSLT generates C++ headers from NuGet packet XML

**ConstantsReplacer/:**
- Purpose: Windows Forms tool for replacing constants in OpenMU SQL models
- Contains: `Replacer.cs`, `MainForm.cs`, SQL const files
- Key files: `ConstantsReplacer.csproj`, `OpenMU-Item-Model-Consts.sql`

**src/source/:**
- Purpose: Core C++ game client
- Contains: Scenes, game logic (maps, items, combat), data handlers, UI, Dotnet bridge
- Key files: `Local.cpp`, `SceneManager.cpp`, `Connection.cpp`, `_define.h`, `_struct.h`

**src/source/Scenes/:**
- Purpose: Scene-based game flow
- Contains: `SceneManager`, `SceneCore`, `LoginScene`, `CharacterScene`, `MainScene`, `LoadingScene`

**src/source/DataHandler/:**
- Purpose: Load game data from files
- Contains: Multiple handler classes for items, monsters, maps, etc.

**src/source/Dotnet/:**
- Purpose: C++/.NET interop for network
- Contains: `Connection.h/cpp`, `PacketFunctions_*.cpp/h`, `PacketBindings_*.h` (generated)

**src/MuEditor/:**
- Purpose: In-game editor (Debug builds, `_EDITOR`)
- Contains: Core, Config, UI (ItemEditor, SkillEditor, Console)

**src/bin/:**
- Purpose: Runtime assets
- Contains: Translations (JSON), required DLLs (glew32, ogg, vorbisfile, wzAudio)

**src/dependencies/:**
- Purpose: Third-party includes and libs
- Contains: OpenGL headers, turbojpeg, wzAudio, netcore delegates

**src/ThirdParty/:**
- Purpose: Git submodules and vendored libs
- Contains: imgui (submodule), json.hpp

## Key File Locations

**Entry Points:**
- `Main.exe` - Game client (CMake target `Main`, sources from `src/source/*.cpp`)
- `ConstantsReplacer.exe` - Build-time tool

**Configuration:**
- `src/CMakeLists.txt` - Build config, editor option, .NET integration
- `stylecop.json` - C# StyleCop
- `.editorconfig` - C#/XML formatting
- `cmake/toolchains/mingw-w64-i686.cmake` - MinGW toolchain

**Core Logic:**
- `src/source/Scenes/` - Scene flow
- `src/source/Dotnet/Connection.cpp` - Network bridge
- `ClientLibrary/ConnectionManager.cs` - .NET network
- `src/source/DataHandler/` - Data loading

**Generated Files:**
- `src/source/Dotnet/PacketFunctions_*.cpp`, `PacketBindings_*.h` - XSLT from MUnique.OpenMU.Network.Packets
- `ClientLibrary/ConnectionManager.*Functions.cs` - XSLT packet extensions

**Documentation:**
- `README.md` - Build, run, credits
- `docs/build-guide.md` - Platform-specific build setup
- `TRANSLATION_SYSTEM_INTEGRATION.md` - i18n usage

## Naming Conventions

**Files:**
- PascalCase for C++ classes: `CharacterManager.cpp`, `SceneManager.h`
- Prefix GM for map/game mode: `GMAida.cpp`, `GMKanturu_1st.cpp`
- Prefix CS for chaos/combat systems: `CSChaosCastle.cpp`, `CSPetSystem.cpp`
- Prefix NewUI for UI: `NewUIBloodCastleEnter.cpp`
- Underscore prefix for global/defines: `_define.h`, `_struct.h`, `_enum.h`

**Directories:**
- PascalCase: `DataHandler`, `GameShop`, `MuEditor`
- No strict convention for subdirs

**Special Patterns:**
- `stdafx.h` - Precompiled header (if used)
- `*Functions*.cs`, `*Bindings*.h` - Generated from XSLT

## Where to Add New Code

**New Game Feature:**
- Logic: `src/source/` (new or existing module)
- Data: `src/source/DataHandler/` if new data type
- UI: `src/source/NewUI*` or scene-specific

**New Scene:**
- Implementation: `src/source/Scenes/`
- Register in `SceneManager`

**New Packet Type:**
- Add to MUnique.OpenMU.Network.Packets (or extend custom)
- Regenerate via XSLT (ClientLibrary PreBuild)
- Use in `src/source/Dotnet/` or game logic

**New Translation:**
- Add keys to `src/bin/Translations/{locale}/game.json` (or editor.json)

**Editor Feature:**
- Implementation: `src/MuEditor/`
- Guard with `#ifdef _EDITOR`

## Special Directories

**src/ThirdParty/imgui:**
- Purpose: ImGui library (git submodule)
- Source: https://github.com/ocornut/imgui
- Committed: Yes (submodule ref)

**src/bin:**
- Purpose: Runtime assets, copied to build output
- Source: Manually maintained, translations, DLLs
- Committed: Yes

**Generated Dotnet files:**
- Purpose: Packet bindings and C# extensions
- Source: XSLT from NuGet MUnique.OpenMU.Network.Packets
- Committed: Yes (generated in PreBuild, checked in or CI)

---

*Structure analysis: 2025-02-08*
*Update when directory structure changes*
