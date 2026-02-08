# Technology Stack

**Analysis Date:** 2025-02-08

## Languages

**Primary:**
- C++ (C++20) - All game client logic, rendering, UI, scene management in `src/source/`
- C# (.NET 10) - Network layer in `ClientLibrary/`, build tool in `ConstantsReplacer/`

**Secondary:**
- XSLT - Code generation from packet XML definitions (`ClientLibrary/*.xslt`)
- CMake - Build configuration
- JSON - Translation files (`src/bin/Translations/`)

## Runtime

**Environment:**
- Windows native (32-bit x86 primary, x64 supported)
- .NET 10 runtime for ClientLibrary (Native AOT published as single DLL)
- Win32 API for windowing, input, audio

**Package Manager:**
- NuGet - .NET dependencies (`ClientLibrary`, `ConstantsReplacer`)
- Git submodules - ImGui (`src/ThirdParty/imgui`)

## Frameworks

**Core:**
- OpenGL / GLEW - Graphics rendering
- DirectSound / wzAudio - Audio playback
- ImGui - In-game editor UI (Debug builds only, `_EDITOR`)

**Build/Dev:**
- CMake 3.25+ - Cross-platform build system
- MSVC 2022+ or MinGW-w64 - C++ compilers
- .NET SDK 10.0+ - ClientLibrary and ConstantsReplacer

## Key Dependencies

**Critical:**
- MUnique.OpenMU.Network.Packets 0.9.8 - Packet definitions, XSLT generates C++/C# bindings
- Pipelines.Sockets.Unofficial 2.2.8 - High-performance network I/O in .NET
- turbojpeg (libjpeg-turbo) - JPEG texture loading
- wzAudio - Audio library (DLL shipped in `src/bin`)
- GLEW - OpenGL extension loading

**Third-Party:**
- imgui - Dear ImGui (git submodule) for editor UI
- nlohmann/json (json.hpp) - JSON parsing for translations

## Configuration

**Environment:**
- No environment variables required for runtime
- CLI params: `/u<host>`, `/p<port>` for connect (e.g. `Main.exe connect /u192.168.0.20 /p55902`)
- `--editor` flag for debug builds (editor enabled on start)

**Build:**
- `src/CMakeLists.txt` - Main CMake config
- `cmake/toolchains/mingw-w64-i686.cmake` - MinGW cross-compilation
- `stylecop.json` - C# StyleCop settings
- `.editorconfig` - C# and XML formatting

## Platform Requirements

**Development:**
- Windows (VS 2022+, CLion, Rider) or WSL + MinGW for cross-compilation
- .NET SDK 10.0+
- CMake 3.25+
- Git (for submodules)

**Production:**
- Windows x86 or x64
- OpenMU server (or compatible Season 6 Episode 3 server)
- Required DLLs: glew32, ogg, vorbisfile, wzAudio, MUnique.Client.Library.dll

---

*Stack analysis: 2025-02-08*
*Update after major dependency changes*
