#include "stdafx.h"

#ifdef _EDITOR

#include "ItemToolWindow.h"

#include "ItemEdits.h"
#include "ItemNamesakes.h"
#include "ItemSelection.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorLayout.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <cstdint>
#include <string>

namespace MuEditor::Items::ToolWindow
{
namespace
{
constexpr int VersionBits = 32;

void LogRefusal()
{
    switch (g_ItemEdits.GetHistory().GetLastRefusal())
    {
    case ItemEditHistory::Refusal::ItemsChanged:
        g_MuEditorConsoleUI.LogEditor(I18N::Editor::TheItemsChangedSinceThisStepItIsNotUndone);
        break;
    case ItemEditHistory::Refusal::ItemHeld:
        g_MuEditorConsoleUI.LogEditor(I18N::Editor::NotDoneTheStepWouldChangeAnItemThatIsWornOrInAnItemWindow);
        break;
    case ItemEditHistory::Refusal::None:
    case ItemEditHistory::Refusal::NothingToDo:
        break;
    }
}

void Undo()
{
    if (g_ItemEdits.GetHistory().CanUndo() && !g_ItemEdits.Undo())
    {
        LogRefusal();
    }
}

void Redo()
{
    if (g_ItemEdits.GetHistory().CanRedo() && !g_ItemEdits.Redo())
    {
        LogRefusal();
    }
}

// Only while the keyboard is the editor's and this window has the focus: ImGui
// keeps the focus in the window when the game window takes the keyboard back
// (Alt+Tab with the editor inside the game window). A text field keeps its own
// Ctrl+Z.
void HandleUndoKeys()
{
    if (!g_MuEditorCore.IsKeyboardWithEditor())
    {
        return;
    }
    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z))
    {
        Undo();
    }
    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Y) || ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z))
    {
        Redo();
    }
}
} // namespace

bool Begin(const char* title, const char* windowId, bool* open)
{
    char fullTitle[MuEditor::WindowIds::TitleCapacity];
    MuEditor::Layout::DockBesideItemTools(windowId);
    MuEditor::Layout::PrepareToolWindow();
    if (!ImGui::Begin(MuEditor::WindowIds::Title(fullTitle, title, windowId), open,
                      ImGuiWindowFlags_NoCollapse | MuEditor::Layout::ToolWindowBarFlags(windowId)))
    {
        return false;
    }
    MuEditor::Layout::RenderToolWindowBar();
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByPopup |
                               ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) ||
        ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId))
    {
        g_MuEditorCore.SetHoveringUI(true);
    }
    HandleUndoKeys();
    return true;
}

void End()
{
    ImGui::End();
}

void RenderUndoRedo()
{
    // In the editor's bar: a click leaves the focus in the item tool, where
    // Ctrl+Z and Ctrl+Y undo and redo.
    ImGui::PushItemFlag(ImGuiItemFlags_NoFocus, true);
    const ItemEditHistory& history = g_ItemEdits.GetHistory();
    ImGui::BeginDisabled(!history.CanUndo());
    if (ImGui::Button(I18N::Editor::Undo))
    {
        Undo();
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && history.CanUndo())
    {
        ImGui::SetTooltip("%s (Ctrl+Z)", history.GetUndoLabel().c_str());
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!history.CanRedo());
    if (ImGui::Button(I18N::Editor::Redo))
    {
        Redo();
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && history.CanRedo())
    {
        ImGui::SetTooltip("%s (Ctrl+Y)", history.GetRedoLabel().c_str());
    }
    ImGui::EndDisabled();
    ImGui::PopItemFlag();
}

bool RenderSelectedItem(int itemType)
{
    if (!Data::Items::IsValidItemType(itemType))
    {
        ImGui::TextDisabled("%s", I18N::Editor::SelectAnItem);
        return false;
    }
    const Data::Items::ItemDefinition* item = g_ItemDatabase.Find(itemType);
    if (item == nullptr)
    {
        ImGui::TextDisabled("(%d,%d) %s", Data::Items::GetItemGroup(itemType), Data::Items::GetItemNumber(itemType),
                            I18N::Editor::NoItemAtThisNumber);
        return false;
    }
    // In the UI language, as the item browser shows it and its namesakes are
    // found.
    ImGui::Text("%s (%d,%d)", Core::Text::ToUtf8(item->name.c_str()).c_str(), Data::Items::GetItemGroup(itemType),
                Data::Items::GetItemNumber(itemType));
    RenderNamesakeQualifier(itemType);
    return true;
}

long long GetDataVersion()
{
    return (static_cast<long long>(g_ItemDatabase.GetVersion()) << VersionBits) |
           static_cast<std::uint32_t>(g_ItemModelDatabase.GetVersion());
}
} // namespace MuEditor::Items::ToolWindow

#endif // _EDITOR
