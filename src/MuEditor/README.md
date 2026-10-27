# MU Editor - In-Game Editor Module

## Overview
The MU Editor is a C++ ImGui-based in-game editor that provides real-time game editing capabilities:
- **Editor Window**: Its own window beside the game window, in front of it, with a menu bar (tool windows, UI scale, language, Close Editor)
- **Tool Windows**: Docked as tabs in the editor window; drag one out to float it anywhere, also over the game
- **Console**: Split view with Editor Console (left) and Game Console (right), docked along the bottom at first
- **Item Tools**: Browse, add, duplicate, move and remove items and edit their data and models while the game runs, with undo and checks before saving (docs/item-data.md, "Editing items")
- **Item Translations**: Item names in every language side by side
- **Monster Tools**: Browse, add, copy and remove monster numbers, set their kind, translate their names (with import and export of the old NpcName txt files), edit their models' files, folders, animation speeds and sounds, preview a model's animations with their sounds, choose the animation each skill plays, and check them while the game runs, with undo (docs/monster-data.md, "Editing monsters")
- **Item Sync**: Import and Export of the items and item formulas with an OpenMU server through a file; an import shows a diff whose checked rows are applied (docs/item-data.md, "Item Sync")

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
- **MuEditorUI.h/cpp** - The editor window and its menu bar, or the Open Editor button while closed
- **MuEditorLayout.h/cpp** - Where the editor window first opens, and the first layout of its tool windows
- **UndoTarget.h/cpp** - The editor bar's Undo and Redo, for the tool worked in last (the item tools, the monster tools or the Server List window)
- **ToolGroups.h/cpp** - The tool groups (the item tools, the monster tools): their browser and tools, which give each group its Windows sub-menu and dock it together
- **ToolWindowFrame.h/cpp** - What every tool window of a group does when it begins: its docking, the editor's bar while it floats, the mouse over it, and its group's Ctrl+Z and Ctrl+Y
- **MuEditorWindowIds.h** - The names the tool windows are docked by
- **MuEditorPlatformWindows.h/cpp** - The editor's own OS windows on Windows, Linux (X11) and macOS
- **MuEditorCursor.h/cpp** - The OS cursor's shape over the editor off Windows

### Console System
- **MuEditorConsole.h/cpp** - Console rendering and logging (split Editor/Game consoles)
- **MuEditorConsoleRedirect.h** - Redirects wprintf/fwprintf to ImGui console

### Item Tools (UI/ItemTools)
One selected item (**ItemSelection**) is shown by every item tool. Changes go
through **ItemEdits** (the handler plus the session's undo history,
**ItemEditHistory**); a tool edits a copy of the item (**ItemEditDraft**) and
stores it when a field is left.
- **MuItemBrowserUI** - The item list with search and filters (ItemBrowserList, ItemGroupFilter), save, import and export, and the add, duplicate, move and remove dialogs (ItemAddDialog, ItemMoveDialog, ItemRemoveDialog) with what uses an item (ItemUsage, ItemUsageView, ItemCodeReferences)
- **MuItemBasicsUI** - Name, size, slot, stats, prices, stat profile and resistances (ItemFieldWidgets, ItemHeldByCharacter, ItemStatProfileWidgets), with the formula of each value (ItemValueFormulas, ItemValueFormulaText)
- **MuItemRequirementsUI** - The stat profile, the requirements (a stat requirement as its +0 value and its weight, with the weights that give the same value: ItemRequirementTable, RequirementWeightPopup), also by item level, and the class steps
- **MuItemRulesUI** - The rule flags of many items at once (ItemRulesTable, ItemRuleFlags)
- **MuItemCategoriesUI** - The items of a tag
- **MuItemModelUI** - The model entry with a preview, file picker, shared models and looks (ItemModelPreview, ItemModelFilePicker, ItemSharedModelForm, ItemModelForms)
- **MuItemGlowColorsUI** - The glow color list: new colors, changed values and unused colors removed (GlowColorDialog, which the model tool's color fields open too)
- **MuItemFormulasUI** - The item stat file: drop level bonus, default, profiles, level bonus tables and profile rules, changed on a copy and applied after the list of the items it reaches (ItemStatTermFields, ItemStatFileChange)
- **MuItemChecksUI** - The checks of a save in the shared problem table, each leading to its item and field (ItemCheckTargets)
- **MuItemSyncUI** - Import and Export of the item exchange file with OpenMU: the import's diff of the items and formulas with a box per field (ItemSyncDiff, which also keeps the known differences), applied as one undo step after a confirmation; the export of the client's data, confirmed when it replaces a file (ItemSyncApply); and a read-only comparison of two files
- **ItemToolWindow** - What every item tool window shares: its title, the selected item, and the item edit history as the editor bar's undo target

### Item Actions (UI/ItemEditor)
- **ItemEditorActions.h/cpp** - Save, import from bmd and the exports
- **ItemListModel.h/cpp** - The items the item tools list (search over all names, missing-name filter), apart from ImGui

### Item Translations
- **MuItemTranslationsUI.h/cpp** - Item names as a grid of items and languages, with missing counts, on the shared names grid with ItemNameRows as its rows (docs/translation-system.md)

### Monster Tools (UI/MonsterTools)
One selected monster (**MonsterSelection**) is shown by every monster tool.
Changes go through **MonsterEdits** (the monster data handler plus the
session's undo history, **MonsterEditHistory**); **MonsterStepLabel** names
each step ("Budge Dragon (2): kind").
- **MuMonsterBrowserUI** - The monster list with search and filters (MonsterBrowserList, MonsterListModel), Save and Revert (MonsterFileActions), the kind combo of each row (MonsterKindField), and the add, copy and remove dialogs (MonsterAddDialog, MonsterRemoveDialog) with what uses a number: the quest data (MonsterDataUsers) and the source lines (MonsterCodeReferences, MonsterCodeLine, MonsterCodeReferenceIndex)
- **MuMonsterModelUI** - The model entry of the selected monster's model: file (MonsterModelFiles), texture folders, animation speeds and held last frames (MonsterModelValues), sounds, sound slots and sound events (MonsterSoundEventsEditor), and what the game's code adds (MonsterAnimationFacts); the model is opened for the tools when shown (MonsterShownModel)
- **MuMonsterPreviewUI** - The selected monster's model in a preview with the item preview's camera: any animation with play, pause and a frame slider, the slot sounds, the hit frame and the game's speed factors (MonsterPreviewScene, with the effect preview's pose, PreviewModelPose)
- **MuMonsterAttacksUI** - The attacks of the selected monster: the animation each monster skill and player skill plays, what any other attack plays (or a draw), when the monster skill is cleared, their sound, and the handlers that read them (MonsterAttackReaders)
- **MuMonsterTranslationsUI** - Monster names as a grid of numbers and languages (the shared names grid, with MonsterNameRows as its rows), and the import and export of the old NpcName txt files (MonsterNpcNameFiles)
- **MuMonsterChecksUI** - The checks of a save and of the attacks in the shared problem table, each leading to its monster and field (MonsterChecks, MonsterAttackChecks, MonsterCheckTargets)
- **MonsterToolWindow** - What every monster tool window shares: the monster tools' frame, and the monster edit history as the editor bar's undo target while a monster tool is open

### Shared Tool Parts (UI/Common)
Parts the item tools and the monster tools both use:
- **NamesGrid.h/cpp**, **NamesGridRows.h** - The names x languages grid of Item Translations and Monster Translations; the rows say what is listed and how a name is stored
- **IssueTable.h/cpp**, **IssueSubjects.h** - The problem table of Item Checks and Monster Checks; the subjects name what a problem is about and open its tool
- **TextSearch.h/cpp** - The search of the editor's lists (a part of a name, A to Z in either case, or a whole number)
- **CodeTokenLine.h/cpp**, **CodeRanges.h/cpp**, **CodeReference.h**, **CodeReferenceList.h/cpp** - Reading the sources for the lines that name an item or a monster, and their list in the remove dialogs

### Server List (UI/ServerList)
- **MuServerListUI.h/cpp** - The server groups of the login screen (Data/Local/ServerList.json): order, names, test server and channel types, shown on the login screen right away (docs/server-list.md)
- **ServerListEdits.h/cpp** - The groups being changed, what the file has, and their undo history (the editor bar's Undo and Redo while the window was used last)
- **ServerListStepLabel.h/cpp** - What each undo step did ("Remove Helheim", "Valhalla: channel 3"), for the tooltips of the editor bar's Undo and Redo

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
#include "../MuEditor/Core/MuEditorCore.h"

// Initialization
g_MuEditorCore.Initialize(g_sdlWindow);
if (szCmdLine && std::strstr(szCmdLine, "--editor"))
    g_MuEditorCore.SetEnabled(true);

// SDL events: what the editor takes (a click or a key on one of its windows)
// does not reach RmlUi or the game
if (g_MuEditorCore.ClaimsEvent(event))
    continue;

// Main loop: F12 from the key itself, in the game window or the editor's own
if (f12Down && !wasF12Pressed)
    g_MuEditorCore.ToggleEditor();
g_MuEditorCore.Update();                // before the game's frame
RenderProfiledFrame();                  // the scene calls g_MuEditorCore.Render()
g_MuEditorCore.RenderPlatformWindows(); // the editor's own windows, after the game's frame
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
- **Editor Window** - Menu bar to open the tool windows, docked as tabs; Close Editor
- **Console** - Dual-panel console with auto-scroll and copy/clear
- **Item Tools** - Live editing of items and their models
- **Monster Tools** - Live editing of monster names, kinds, models and attacks, and a preview of a model's animations
- **Item Sync** - Export and Import of the item data with OpenMU
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
// In CMuEditorCore::Render(), after g_MuEditorUI.Render(...);
// myWindow is a new flag in struct ToolWindows (MuEditorToolWindows.h)
if (m_windows.myWindow)
{
    MuEditor::Layout::PrepareToolWindow();  // its tab, also when floating
    // The editor's bar while it floats outside the editor window.
    if (ImGui::Begin("My Window", &m_windows.myWindow, MuEditor::Layout::ToolWindowBarFlags("My Window")))
    {
        MuEditor::Layout::RenderToolWindowBar();
        ImGui::Text("Custom content here");
    }
    ImGui::End();
}
```
To open it as a tab of the editor window like the other tool windows, add its
name to `MuEditorWindowIds.h` (with a `###` id if the title is translated),
Begin it with that name, add the name to the first layout in
`MuEditorLayout.cpp` (BuildDefaultLayout, which Reset Layout builds too) and a
`MenuItem` to the Windows menu in `MuEditorUI.cpp`.

A new tool of the item or monster tools is an entry of its group in
`ToolGroups.cpp` instead (with its id in `MuEditorWindowIds.h` and its flag in
`MuEditorToolWindows.h`): that gives it its entry in the group's Windows
sub-menu and its place in the first layout and beside the group. Draw it in
`CMuEditorCore::RenderItemTools` or `RenderMonsterTools`, and begin its window
with the group's `ToolWindow::Begin` (`ItemToolWindow.h`,
`MonsterToolWindow.h`), which goes through `ToolWindowFrame`.

### 4. Access Game State
```cpp
// Direct access to game memory:
#ifdef _EDITOR
if (g_MuEditor.IsEnabled())
{
    g_MuEditorConsole.LogGame("Player HP: " + std::to_string(PlayerHP));
    Hero->Object.Position[2] += 100.0f;  // Modify in real-time
}
#endif
```

## Architecture

### Component Hierarchy
```
CMuEditor (main editor singleton)
├── CMuEditorUI (the editor window and its menu bar, or the Open Editor button)
├── CMuEditorConsole (console rendering & logging)
│   └── ConsoleStreamBuf (C++ stream redirection for cout/cerr)
├── the item tools (UI/ItemTools) and CMuItemTranslationsUI
├── the monster tools (UI/MonsterTools)
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
