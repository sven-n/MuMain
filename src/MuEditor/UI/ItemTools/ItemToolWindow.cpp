#include "stdafx.h"

#ifdef _EDITOR

#include "ItemToolWindow.h"

#include "ItemEdits.h"
#include "ItemNamesakes.h"
#include "ItemSelection.h"

#include "../MuEditor/UI/Common/ToolGroups.h"
#include "../MuEditor/UI/Common/ToolWindowFrame.h"
#include "../MuEditor/UI/Common/UndoTarget.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "imgui.h"

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

void UndoItemEdit()
{
    if (g_ItemEdits.GetHistory().CanUndo() && !g_ItemEdits.Undo())
    {
        LogRefusal();
    }
}

void RedoItemEdit()
{
    if (g_ItemEdits.GetHistory().CanRedo() && !g_ItemEdits.Redo())
    {
        LogRefusal();
    }
}

// The item edit history, for the editor bar's Undo and Redo.
class ItemUndoTarget final : public UndoTarget
{
public:
    // The fallback of the bar: item edits stay undoable with the item tools
    // closed, as they always were.
    bool IsOpen() const override
    {
        return true;
    }
    bool CanUndo() const override
    {
        return g_ItemEdits.GetHistory().CanUndo();
    }
    bool CanRedo() const override
    {
        return g_ItemEdits.GetHistory().CanRedo();
    }
    std::string GetUndoLabel() const override
    {
        return g_ItemEdits.GetHistory().GetUndoLabel();
    }
    std::string GetRedoLabel() const override
    {
        return g_ItemEdits.GetHistory().GetRedoLabel();
    }
    void Undo() override
    {
        UndoItemEdit();
    }
    void Redo() override
    {
        RedoItemEdit();
    }
};
} // namespace

bool Begin(const char* title, const char* windowId, bool* open)
{
    return ToolWindowFrame::Begin(title, windowId, open, GetItemTools(), GetUndoTarget());
}

void End()
{
    ToolWindowFrame::End();
}

UndoTarget& GetUndoTarget()
{
    static ItemUndoTarget target;
    return target;
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
    // In the item name language, as the item browser shows it and its
    // namesakes are found.
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
