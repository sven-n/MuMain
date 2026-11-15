#include "stdafx.h"

#ifdef _EDITOR

#include "ItemTextCheckArea.h"

#include "ItemCheckTargets.h"
#include "ItemSelection.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/Translation/ItemReferenceChecks.h"
#include "I18N/All.h"
#include "imgui.h"

namespace MuEditor::Items
{
namespace
{
using Data::Items::ItemDataIssue;

constexpr std::string_view UiTextExtension = ".resx";

bool IsUiText(const ItemDataIssue& issue)
{
    return issue.source.ends_with(UiTextExtension);
}

bool IsItemText(const ItemDataIssue& issue)
{
    return issue.source.empty() && Data::Items::IsValidItemId(issue.group, issue.number);
}

void OpenTool(ItemTool tool)
{
    const ItemToolWindowRef window = GetToolWindow(tool);
    g_MuEditorCore.GetToolWindows().*window.open = true;
    ImGui::SetWindowFocus(window.windowId);
}
} // namespace

const char* ItemTextCheckArea::GetName() const
{
    return I18N::Editor::Texts;
}

void ItemTextCheckArea::Check(std::vector<ItemDataIssue>& issues)
{
    Data::Translation::CheckUiItemTexts(g_ItemDatabase, issues);
    Data::Translation::CheckItemDataTexts(g_ItemDatabase, Data::Items::GetItemTooltipFile().generic_string(), issues);
}

void ItemTextCheckArea::RenderSubject(const ItemDataIssue& issue) const
{
    if (!IsItemText(issue))
    {
        ImGui::TextUnformatted(issue.source.c_str());
        return;
    }
    ImGui::TextUnformatted(g_ItemDatabase.GetLogName(Data::Items::MakeItemType(issue.group, issue.number)).c_str());
}

void ItemTextCheckArea::Open(const ItemDataIssue& issue, bool clicked)
{
    if (!Data::Items::IsValidItemId(issue.group, issue.number))
    {
        return;
    }
    // A UI or tooltip file text that names a number no item has ({item:14,511}): the selection stays, the tool would
    // show nothing.
    if (!IsItemText(issue) && g_ItemDatabase.Find(issue.group, issue.number) == nullptr)
    {
        return;
    }
    g_ItemSelection.Select(Data::Items::MakeItemType(issue.group, issue.number));
    if (!clicked)
    {
        return;
    }
    if (IsUiText(issue))
    {
        OpenTool(ItemTool::Translations);
        return;
    }
    OpenTool(IsItemText(issue) ? GetToolForIssue(issue) : ItemTool::Tooltip);
}

std::unique_ptr<DataChecks::DataCheckArea> CreateItemTextCheckArea()
{
    return std::make_unique<ItemTextCheckArea>();
}
} // namespace MuEditor::Items

#endif // _EDITOR
