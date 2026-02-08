# MuMain — MU Online Client

## What This Is

MuMain is a fork of the Season 5.2 MU Online client, evolved toward full Season 6 Episode 3 compatibility. It targets the OpenMU server, uses a hybrid C++/C# architecture (C++ game logic, .NET Native AOT network layer), and includes an optional in-game editor (ImGui) for debug builds.

## Core Value

**The client must connect to OpenMU, run stably, and support the Season 6 Episode 3 protocol.** Everything else—editor, translations, performance tuning—supports that goal.

## Requirements

### Validated

<!-- Shipped and confirmed valuable. -->

- ✓ Connects to OpenMU (Season 6 adapted protocol)
- ✓ Unicode support (UTF-16LE in memory, UTF-8 from files/network)
- ✓ Master skill tree Season 6
- ✓ FPS controls (V-Sync, $fps/$vsync chat commands)
- ✓ Translation system (game, editor, metadata domains)
- ✓ CMake + Ninja build via CMakePresets (windows-x86, windows-x64)

### Active

<!-- Current scope. Building toward these. -->

- [ ] Lucky Items (Season 6 feature gap per README)
- [ ] Project goals to be defined via roadmap

### Out of Scope

<!-- Explicit boundaries. -->

- Native Linux client — Use WSL + MinGW cross-compile instead
- Non-OpenMU servers — Protocol adapted for OpenMU; standard MU servers not targeted

## Context

**Build environment:**
- Windows 11
- CMake 3.25+, Ninja Multi-Config, CMakePresets.json
- Primary workflow:
  - Configure: `cmake --preset windows-x86` (or windows-x64)
  - Build Debug: `cmake --build --preset windows-x86-debug`
  - Build Release: `cmake --build --preset windows-x86-release`
- Ninja Multi-Config allows switching Debug/Release without reconfiguring
- Assets copied to build output during compilation

**Codebase:** See `.planning/codebase/` for STACK, ARCHITECTURE, STRUCTURE, CONVENTIONS, TESTING, INTEGRATIONS, CONCERNS.

**Known gaps:** Lucky Items; Linux build not working (WSL+MinGW is the path).

## Constraints

- **Tech stack**: C++20, .NET 10, OpenGL/GLEW, Win32 — must remain for compatibility
- **Platform**: Windows (x86 primary, x64 supported)
- **Server**: OpenMU or compatible Season 6 Episode 3 server
- **Build**: CMakePresets + Ninja preferred (faster than MSBuild)

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| .NET Native AOT for network | Easier protocol changes; single DLL | ✓ Good |
| CMakePresets + Ninja | Faster builds, multi-config, IDE-friendly | ✓ Good |
| ImGui editor (Debug only) | Dev tooling without release bloat | ✓ Good |
| XSLT from MUnique.OpenMU.Network.Packets | Single source of truth for packets | ✓ Good |

---
*Last updated: 2025-02-08 after new-project initialization*
