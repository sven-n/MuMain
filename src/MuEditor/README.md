# MU Editor - In-Game Editor Module

## Overview
The MU Editor is a C++ ImGui-based in-game editor that provides real-time game editing capabilities:
- **Top Toolbar**: Editor controls and buttons
- **Center Viewport**: Game renders here
- **Bottom Console**: Split view with Editor Console (left) and Game Console (right)
- **Item Editor**: Edit item attributes in real-time

## Key Features
- ✅ Completely wrapped in `#ifdef _EDITOR` - **excluded from release builds**
- ✅ F12 key to toggle editor on/off
- ✅ `--editor` command line flag to start in editor mode
- ✅ Direct access to game internals (same process!)
- ✅ Console output redirection - NO separate Windows console
- ✅ Modular architecture with separate UI components

## Module Files

### Core Editor
- **MuEditor.h/cpp** - Main editor class, initialization, lifecycle management
- **MuEditorUI.h/cpp** - Toolbar rendering (open/close, item editor buttons)

### Console System
- **MuEditorConsole.h/cpp** - Console rendering and logging (split Editor/Game consoles)
- **MuEditorConsoleRedirect.h** - Redirects wprintf/fwprintf to ImGui console

### Item Editor
- **MuItemEditor.h/cpp** - Item attribute editor with live editing capabilities

### Input Management
- **MuInputBlocker.h/cpp** - Blocks game input when hovering over ImGui UI

## Console System Architecture

### How Console Redirection Works
When `_EDITOR` is defined:
1. **Windows console is disabled** - No separate console window opens
2. **wprintf/fwprintf are redirected** - Macros replace these with `editor_wprintf`/`editor_fwprintf`
3. **Output goes to ImGui** - All console output appears in the Game Console panel
4. **g_ConsoleDebug is hooked** - Game debug messages also go to ImGui console

### Console Output Sources Captured
- ✅ `wprintf()` calls (redirected via macros to `editor_wprintf`)
- ✅ `fwprintf(stderr, ...)` calls (redirected via macros to `editor_fwprintf`)
- ✅ `g_ConsoleDebug->Write()` calls (hooked directly)
- ✅ C++ streams (`std::cout`, `std::cerr`) via `ConsoleStreamBuf` redirection

### Console Features
- **Dual panels** - Editor Console (left) and Game Console (right)
- **Auto-scroll** - Automatically scrolls to newest messages
- **Log file** - All output saved to timestamped log file

## Build Configurations

### Debug Builds (Editor Enabled)
- Preprocessor define: `_EDITOR`
- Include paths: `ThirdParty\imgui` and `ThirdParty\imgui\backends`
- All editor code is compiled in
- Console output redirected to ImGui

### Release Builds (Editor Excluded)
- No `_EDITOR` define
- All editor code is **completely excluded** via `#ifdef _EDITOR`
- Normal Windows console behavior (if enabled)
- Zero overhead - as if the editor never existed

## Integration Points

### In stdafx.h
```cpp
#ifdef _EDITOR
#include "MuEditor/MuEditorConsoleRedirect.h"  // Redirect console output
#endif
```

### In Winmain.cpp
```cpp
#ifdef _EDITOR
#include "MuEditor/MuEditor.h"

// Initialization
g_MuEditor.Initialize(g_hWnd, g_hDC);
if (szCmdLine && wcsstr(GetCommandLineW(), L"--editor"))
    g_MuEditor.SetEnabled(true);

// Main loop - Update and Render
g_MuEditor.Update();
RenderScene(g_hDC);
g_MuEditor.Render();

// F12 toggle
if (GetAsyncKeyState(VK_F12) & 0x8000)
    g_MuEditor.ToggleEditor();
#endif
```

### In ZzzScene.cpp
```cpp
#ifdef _EDITOR
#include "MuEditor/MuEditor.h"
#include "MuEditor/MuEditorConsole.h"

// Log to editor console
g_MuEditorConsole.LogEditor("Scene changed");
#endif
```

### In muConsoleDebug.cpp
```cpp
#ifdef _EDITOR
#include "MuEditor/MuEditorConsole.h"

// Disable Windows console creation
// All output goes to ImGui console instead
#endif
```

## How to Use

### Starting the Editor
```bash
# Option 1: Command line flag
main.exe --editor

# Option 2: Press F12 at runtime to toggle
```

### Current Features
- **Toolbar** - Open/close editor, toggle item editor
- **Console** - Dual-panel console with auto-scroll and copy/clear
- **Item Editor** - Live editing of item attributes
- **Input Blocking** - Game input blocked when hovering UI
- **No Separate Console** - All output in ImGui, no Windows console window

## Adding New Editor Features

### 1. Log to Editor Console
```cpp
#ifdef _EDITOR
#include "MuEditor/MuEditorConsole.h"
g_MuEditorConsole.LogEditor("Custom editor message");
#endif
```

### 2. Log to Game Console
Game console captures all standard output automatically via console redirection.

### 3. Add New Editor Window
```cpp
// In MuEditor::Render()
if (m_bShowMyWindow)
{
    if (ImGui::Begin("My Window", &m_bShowMyWindow))
    {
        ImGui::Text("Custom content here");
    }
    ImGui::End();
}
```

### 4. Access Game State
```cpp
// Direct access to game memory:
#ifdef _EDITOR
if (g_MuEditor.IsEnabled())
{
    g_MuEditorConsole.LogGame("Player HP: " + std::to_string(PlayerHP));
    ItemAttribute[index].Durability = 255;  // Modify in real-time
}
#endif
```

## Architecture

### Component Hierarchy
```
CMuEditor (main editor singleton)
├── CMuEditorUI (toolbar rendering)
├── CMuEditorConsole (console rendering & logging)
│   └── ConsoleStreamBuf (C++ stream redirection for cout/cerr)
├── CMuItemEditor (item editor window)
└── CMuInputBlocker (input blocking when hovering UI)
```

### Console Redirection Flow
```
Application calls wprintf/fwprintf
    ↓
Macro redirects to editor_wprintf/editor_fwprintf
    ↓
Function formats message and calls g_MuEditorConsole.LogGame()
    ↓
Message added to m_strGameConsole
    ↓
Rendered in ImGui Game Console panel
```

## Important Notes
- All editor code is wrapped in `#ifdef _EDITOR`
- Release builds have **NO** editor code at all
- F12 key toggles editor visibility
- Editor has full access to game internals
- Console output is redirected when editor is enabled
- No Windows console window opens with editor
- All output captured in ImGui console panels
