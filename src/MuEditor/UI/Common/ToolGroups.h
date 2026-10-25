#pragma once

#ifdef _EDITOR

// Not a forward declaration: MSVC sizes a pointer to a member of an incomplete
// class differently, so ToolWindowEntry would differ between files.
#include "MuEditorToolWindows.h"

#include <span>

namespace MuEditor
{
// A tool window of a group: its "###" id, its translated label (read when it
// is drawn, so it follows the editor language) and its flag in ToolWindows.
struct ToolWindowEntry
{
    const char* windowId;
    const char* const* label;
    bool ToolWindows::* open;
};

// Tools that share one selection (the item tools, the monster tools): they
// dock as tabs beside each other, next to their browser, and have one sub-menu
// in the Windows menu.
struct ToolGroup
{
    const char* const* menuTitle;
    ToolWindowEntry browser;
    // The other tools, in tab and menu order.
    std::span<const ToolWindowEntry> tools;
    // Where a tool of the group docks while no window of the group has a
    // place yet; nullptr for none.
    const ToolGroup* dockFallback = nullptr;

    bool IsAnyOpen(const ToolWindows& windows) const;
};

const ToolGroup& GetItemTools();
// Docks with the item tools until a monster tool has a place of its own.
const ToolGroup& GetMonsterTools();
} // namespace MuEditor

#endif // _EDITOR
