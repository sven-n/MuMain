#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterBrowserList.h"

#include "MonsterKindField.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/KeyboardSelection.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <cstdio>

namespace MuEditor::Monsters
{
namespace
{
// Pixel widths before the editor UI scale.
constexpr float NumberColumnWidth = 50.0f;
constexpr float NameColumnWidth = 240.0f;
constexpr int ColumnCount = 3;

void RenderRow(int number, bool selected, int& clicked)
{
    // The number is the row's label and id: a name may hold "##" or "%".
    char label[16];
    std::snprintf(label, sizeof(label), "%d", number);
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    if (ImGui::Selectable(label, selected, ImGuiSelectableFlags_SpanAllColumns | KeyboardSelection::RowFlags))
    {
        clicked = number;
    }
    // Read each frame: an undo, a revert or a language switch changes it.
    const Data::Monsters::MonsterDefinition* monster = g_MonsterDatabase.Find(number);
    if (monster == nullptr)
    {
        return;
    }
    ImGui::TableSetColumnIndex(1);
    ImGui::TextUnformatted(monster->names.Get(I18N::GetCurrentLocale()).c_str());
    ImGui::TableSetColumnIndex(2);
    ImGui::TextUnformatted(KindField::GetLabel(monster->kind));
}
} // namespace

int MonsterBrowserList::Render(const std::vector<int>& numbers, int selectedNumber, bool reveal)
{
    int clicked = -1;
    constexpr ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
                                      ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable("##Monsters", ColumnCount, flags))
    {
        return clicked;
    }
    const float uiScale = g_MuEditorCore.GetUIScale();
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_None, NumberColumnWidth * uiScale);
    ImGui::TableSetupColumn(I18N::Metadata::Name, ImGuiTableColumnFlags_None, NameColumnWidth * uiScale);
    ImGui::TableSetupColumn(I18N::Editor::Kind, ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(numbers.size()));
    const auto selectedRow = std::ranges::find(numbers, selectedNumber);
    const bool scrollToSelected = reveal && selectedRow != numbers.end();
    if (scrollToSelected)
    {
        clipper.IncludeItemByIndex(static_cast<int>(selectedRow - numbers.begin()));
    }
    while (clipper.Step())
    {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
        {
            const bool selected = numbers[row] == selectedNumber;
            RenderRow(numbers[row], selected, clicked);
            if (selected && scrollToSelected)
            {
                ImGui::SetScrollHereY();
            }
        }
    }
    ImGui::EndTable();
    return clicked;
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
