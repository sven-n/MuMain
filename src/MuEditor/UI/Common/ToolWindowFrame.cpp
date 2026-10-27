#include "stdafx.h"

#ifdef _EDITOR

#include "ToolWindowFrame.h"

#include "MuEditorLayout.h"
#include "MuEditorWindowIds.h"
#include "ToolGroups.h"
#include "UndoTarget.h"
#include "../MuEditor/Core/MuEditorCore.h"
#include "imgui.h"

namespace MuEditor::ToolWindowFrame
{
namespace
{
// Only while the keyboard is the editor's and this window has the focus: ImGui
// keeps the focus in the window when the game window takes the keyboard back
// (Alt+Tab with the editor inside the game window). A text field keeps its own
// Ctrl+Z.
void HandleUndoKeys(UndoTarget& undo)
{
    if (!g_MuEditorCore.IsKeyboardWithEditor())
    {
        return;
    }
    // The shortcut first: it claims the key also when there is nothing to undo.
    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z) && undo.CanUndo())
    {
        undo.Undo();
    }
    if ((ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Y) || ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z)) &&
        undo.CanRedo())
    {
        undo.Redo();
    }
}
} // namespace

bool Begin(const char* title, const char* windowId, bool* open, const ToolGroup& group, UndoTarget& undo)
{
    char fullTitle[WindowIds::TitleCapacity];
    Layout::DockInGroup(windowId, group);
    Layout::PrepareToolWindow();
    if (!ImGui::Begin(WindowIds::Title(fullTitle, title, windowId), open,
                      ImGuiWindowFlags_NoCollapse | Layout::ToolWindowBarFlags(windowId)))
    {
        return false;
    }
    // Before the bar: in this window's own bar it shows this group's undo then.
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    {
        UndoTargets::Activate(undo);
    }
    Layout::RenderToolWindowBar();
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByPopup |
                               ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) ||
        ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId))
    {
        g_MuEditorCore.SetHoveringUI(true);
    }
    HandleUndoKeys(undo);
    return true;
}

void End()
{
    ImGui::End();
}
} // namespace MuEditor::ToolWindowFrame

#endif // _EDITOR
