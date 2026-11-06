#include "stdafx.h"

#ifdef _EDITOR

#include "ItemCheckArea.h"

#include "ItemCheckTargets.h"
#include "ItemSelection.h"
#include "MuItemFormulasUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "imgui.h"

namespace MuEditor::Items
{
namespace
{
using Data::Items::ItemDataIssue;

bool IsAboutAnItem(const ItemDataIssue& issue)
{
    return Data::Items::IsValidItemId(issue.group, issue.number);
}
} // namespace

const char* ItemCheckArea::GetName() const
{
    return I18N::Editor::Items;
}

void ItemCheckArea::Check(std::vector<ItemDataIssue>& issues)
{
    g_ItemDataHandler.Validate(issues);
}

void ItemCheckArea::RenderSubject(const ItemDataIssue& issue) const
{
    if (!IsAboutAnItem(issue))
    {
        ImGui::TextDisabled("%s", issue.source.c_str());
        return;
    }
    const int itemType = Data::Items::MakeItemType(issue.group, issue.number);
    if (g_ItemDatabase.Find(itemType) == nullptr)
    {
        ImGui::Text("(%d,%d)", issue.group, issue.number);
        return;
    }
    ImGui::TextUnformatted(g_ItemDatabase.GetLogName(itemType).c_str());
}

void ItemCheckArea::Open(const ItemDataIssue& issue, bool clicked)
{
    if (!IsAboutAnItem(issue))
    {
        if (clicked && issue.source == Data::Items::GetItemStatsFile().generic_string())
        {
            OpenItemFormulas(FindFormulaEntryForField(issue.field));
        }
        return;
    }
    g_ItemSelection.Select(Data::Items::MakeItemType(issue.group, issue.number));
    if (!clicked)
    {
        return;
    }
    const ItemToolWindowRef tool = GetToolWindow(GetToolForIssue(issue));
    g_MuEditorCore.GetToolWindows().*tool.open = true;
    ImGui::SetWindowFocus(tool.windowId);
}

std::unique_ptr<DataChecks::DataCheckArea> CreateItemCheckArea()
{
    return std::make_unique<ItemCheckArea>();
}
} // namespace MuEditor::Items

#endif // _EDITOR
