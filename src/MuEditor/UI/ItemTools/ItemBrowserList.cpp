#include "stdafx.h"

#ifdef _EDITOR

#include "ItemBrowserList.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "imgui.h"
#include "../MuEditor/UI/Common/KeyboardSelection.h"
#include "ItemLevelModels.h"
#include "ItemNamesakes.h"
#include "ItemUsage.h"

#include <algorithm>
#include <cstdio>
#include <string>

namespace MuEditor::Items
{
namespace
{
// Pixel widths before the editor UI scale.
constexpr float NumberColumnWidth = 70.0f;
constexpr float NameColumnWidth = 220.0f;
constexpr int ColumnCount = 3;

void RenderRow(int itemType, bool selected, int& clicked)
{
    char number[16];
    std::snprintf(number, sizeof(number), "%d,%d", Data::Items::GetItemGroup(itemType),
                  Data::Items::GetItemNumber(itemType));
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    if (ImGui::Selectable(number, selected,
                          ImGuiSelectableFlags_SpanAllColumns | MuEditor::KeyboardSelection::RowFlags))
    {
        clicked = itemType;
    }
    ImGui::TableSetColumnIndex(1);
    if (const Data::Items::ItemDefinition* item = g_ItemDatabase.Find(itemType))
    {
        ImGui::TextUnformatted(Core::Text::ToUtf8(item->name.c_str()).c_str());
        RenderNamesakeQualifier(itemType);
    }
    else
    {
        ImGui::TextDisabled("%s", GetNumberWithoutItemLabel(itemType));
    }
    ImGui::TableSetColumnIndex(2);
    if (const Data::Items::ItemModelDefinition* model = g_ItemModelDatabase.Find(itemType))
    {
        ImGui::TextUnformatted(model->model.empty() ? model->file.c_str() : model->model.c_str());
    }
    else
    {
        // Some items are drawn by code with other models (the Wizard's Ring).
        ImGui::TextDisabled("%s", g_ItemDatabase.Find(itemType) != nullptr && IsDrawnByCodeAtSomeLevel(itemType)
                                      ? I18N::Editor::DrawnByCode
                                      : I18N::Editor::WithoutAModel);
    }
}
} // namespace

int CItemBrowserList::Render(const std::vector<int>& itemTypes, int selectedItemType, bool reveal)
{
    int clicked = -1;
    constexpr ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
                                      ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable("##Items", ColumnCount, flags))
    {
        return clicked;
    }
    const float uiScale = g_MuEditorCore.GetUIScale();
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_None, NumberColumnWidth * uiScale);
    ImGui::TableSetupColumn(I18N::Metadata::Name, ImGuiTableColumnFlags_None, NameColumnWidth * uiScale);
    ImGui::TableSetupColumn(I18N::Editor::ModelFile, ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(itemTypes.size()));
    const auto selectedRow = std::ranges::find(itemTypes, selectedItemType);
    const bool scrollToSelected = reveal && selectedRow != itemTypes.end();
    if (scrollToSelected)
    {
        clipper.IncludeItemByIndex(static_cast<int>(selectedRow - itemTypes.begin()));
    }
    while (clipper.Step())
    {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
        {
            const bool selected = itemTypes[row] == selectedItemType;
            RenderRow(itemTypes[row], selected, clicked);
            if (selected && scrollToSelected)
            {
                ImGui::SetScrollHereY();
            }
        }
    }
    ImGui::EndTable();
    return clicked;
}
} // namespace MuEditor::Items

#endif // _EDITOR
