# Architecture

**Analysis Date:** 2025-02-08

## Pattern Overview

**Overall:** Desktop game client with hybrid C++/C# architecture, scene-based rendering loop, optional in-game editor.

**Key Characteristics:**
- Native C++ game loop; .NET DLL for network (Native AOT, loaded at runtime)
- Scene-based UI flow (Login → Character Select → Main game)
- Win32 window + OpenGL rendering
- Conditional editor (ImGui) in Debug builds

## Layers

**Scene Layer:**
- Purpose: High-level game states and UI flow
- Contains: `SceneManager`, `SceneCore`, `LoginScene`, `CharacterScene`, `MainScene`, `LoadingScene`, `WebzenScene`
- Location: `src/source/Scenes/`
- Depends on: Data handlers, network (Connection), rendering
- Used by: Main loop (SceneManager drives current scene)

**Game Logic Layer:**
- Purpose: Game mechanics, maps, combat, items, guilds, events
- Contains: `GM*` (map) classes, `CS*` (chaos castle, pet, quest), `ItemManager`, `CharacterManager`, `Guild/*`, `DuelMgr`, etc.
- Location: `src/source/`
- Depends on: Data handlers, utilities
- Used by: Scenes, UI

**Data Handler Layer:**
- Purpose: Load and provide game data (items, monsters, maps)
- Contains: `DataHandler/*` (16+ handlers)
- Location: `src/source/DataHandler/`
- Depends on: File I/O, `_struct.h`, `_enum.h`
- Used by: Game logic, UI

**Network Bridge Layer:**
- Purpose: Connect C++ to .NET network stack
- Contains: `Connection` class, `Dotnet/*` (packet bindings, functions)
- Location: `src/source/Dotnet/`
- Depends on: `MUnique.Client.Library.dll` (loaded via LoadLibrary)
- Used by: Scenes, game logic for server communication

**Rendering Layer:**
- Purpose: OpenGL drawing, textures, 3D models
- Contains: `ZzzTexture`, `GlobalBitmap`, `NewUI3DRenderMng`, camera, etc.
- Location: `src/source/`
- Depends on: GLEW, OpenGL, Windows
- Used by: Scenes, UI components

**Editor Layer (Debug only):**
- Purpose: In-game item/skill editor (ImGui)
- Contains: `MuEditor/*` (Core, Config, UI, ItemEditor, SkillEditor)
- Location: `src/MuEditor/`
- Depends on: ImGui, game data
- Used by: F12 toggle in Debug builds

## Data Flow

**Application Startup:**
1. WinMain initializes Win32 window
2. OpenGL context created
3. Scenes registered; `SceneManager` starts with Login/Connect
4. `Connection` loads `MUnique.Client.Library.dll`, connects to server
5. Packet callbacks route to scene/game logic

**Game Loop:**
1. Input processed
2. Current scene updated (SceneManager)
3. Rendering (OpenGL)
4. Swap buffers
5. Repeat

**State Management:**
- Scene-based: current scene holds UI and logic state
- Connection handles: static map in .NET `ConnectionManager`
- No persistent client-side DB; server is source of truth

## Key Abstractions

**Connection:**
- Purpose: Wrap .NET ConnectionManager for C++
- Pattern: Load DLL, call `ConnectionManager_Connect`, `ConnectionManager_Send` via function pointers
- Usage: One Connection per server (Connect, Chat, Game)

**Scene:**
- Purpose: Encapsulate a game screen/state
- Examples: `LoginScene`, `MainScene`, `CharacterScene`
- Pattern: Scene base class, SceneManager switches active scene

**PacketFunctions / PacketBindings:**
- Purpose: Typed packet read/write generated from XML
- Location: `src/source/Dotnet/PacketFunctions_*.cpp`, `PacketBindings_*.h`
- Pattern: XSLT generates from `MUnique.OpenMU.Network.Packets` XML

## Entry Points

**Main:**
- Location: `Main.exe` (Win32 GUI subsystem)
- Triggers: User launches executable
- Responsibilities: Window creation, game loop, scene management

**ConstantsReplacer:**
- Location: `ConstantsReplacer/Program.cs` → `MainForm`
- Triggers: Separate Windows Forms tool (build-time)
- Responsibilities: Replace constants in SQL/OpenMU models

## Error Handling

**Strategy:** Return codes, callbacks for disconnection; no global exception handler in C++ game loop.

**Patterns:**
- Network: `ConnectionManager_Connect` returns negative on failure
- Callbacks: `OnDisconnected`, `ReportDotNetError` for .NET issues
- C#: try/catch in ConnectionManager, returns -1 on exception

## Cross-Cutting Concerns

**Logging:**
- C++: `ReportDotNetError` for .NET bridge errors; limited structured logging
- C#: `Debug.WriteLine` in ConnectionManager

**Validation:**
- Packet structure validated by MUnique.OpenMU.Network
- Client-side validation in UI (e.g. `CheckSpecialText` for character names)

**Localization:**
- `i18n::Translator` singleton, JSON files in `Translations/`
- Domains: Game (always), Editor, Metadata (Debug only)

---

*Architecture analysis: 2025-02-08*
*Update when major patterns change*
