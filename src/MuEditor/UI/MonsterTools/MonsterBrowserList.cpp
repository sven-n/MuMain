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

void RenderKindCell(int number, Data::Monsters::MonsterKind current, MonsterBrowserList::Result& result)
{
    // No vertical padding: the combo is as tall as a line of text, so the rows
    // stay as dense as without it.
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0.0f));
    ImGui::SetNextItemWidth(-FLT_MIN);
    Data::Monsters::MonsterKind kind = current;
    if (KindField::RenderCombo("##Kind", kind))
    {
        result.kindChanged = number;
        result.kind = kind;
    }
    ImGui::PopStyleVar();
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::KindChangeInGame);
    }
}

void RenderRow(int number, bool selected, MonsterBrowserList::Result& result)
{
    // The number is the row's label and id: a name may hold "##" or "%".
    char label[16];
    std::snprintf(label, sizeof(label), "%d", number);
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    // AllowOverlap: the kind combo drawn over the row takes its own clicks.
    if (ImGui::Selectable(label, selected,
                          ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap |
                              KeyboardSelection::RowFlags))
    {
        result.clicked = number;
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
    ImGui::PushID(number);
    RenderKindCell(number, monster->kind, result);
    ImGui::PopID();
}
} // namespace

MonsterBrowserList::Result MonsterBrowserList::Render(const std::vector<int>& numbers, int selectedNumber, bool reveal)
{
    Result result;
    constexpr ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
                                      ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable("##Monsters", ColumnCount, flags))
    {
        return result;
    }
    const float uiScale = g_MuEditorCore.GetUIScale();
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_None, NumberColumnWidth * uiScale);
    ImGui::TableSetupColumn(I18N::Metadata::Name, ImGuiTableColumnFlags_None, NameColumnWidth * uiScale);
    ImGui::TableSetupColumn(I18N::Editor::Kind, ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();

    // Only the rows in view are drawn, combos included.
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
            RenderRow(numbers[row], selected, result);
            if (selected && scrollToSelected)
            {
                ImGui::SetScrollHereY();
            }
        }
    }
    ImGui::EndTable();
    return result;
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
