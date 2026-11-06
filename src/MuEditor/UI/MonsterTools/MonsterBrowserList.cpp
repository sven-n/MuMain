#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterBrowserList.h"

#include "MonsterEdits.h"
#include "MonsterKindField.h"
#include "MonsterOwnValues.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/KeyboardSelection.h"
#include "../MuEditor/UI/Common/MapPicker.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <cstdio>
#include <optional>
#include <string>

namespace MuEditor::Monsters
{
namespace
{
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterValueField;
using Row = MonsterTreeModel::Row;

// Pixel widths before the editor UI scale.
constexpr float NumberColumnWidth = 90.0f;
constexpr float NameColumnWidth = 240.0f;
constexpr float KindColumnWidth = 110.0f;
constexpr float NumberIndent = 14.0f;
constexpr int ColumnCount = 4;
// The ids of monster rows, apart from their first number's row.
constexpr int MonsterRowId = 1 << 16;

constexpr ImVec4 SharedTextColor{0.62f, 0.62f, 0.62f, 1.0f};
constexpr ImVec4 OwnTextColor{0.95f, 0.75f, 0.25f, 1.0f};

void RenderKindCell(const Row& row, Data::Monsters::MonsterKind current, MonsterBrowserList::Result& result)
{
    const OwnValues::State state = row.kind == Row::Kind::Number
                                       ? OwnValues::GetNumberState(row.number, MonsterValueField::Kind)
                                       : OwnValues::State::Plain;
    // No vertical padding: the combo is as tall as a line of text, so the rows
    // stay as dense as without it.
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0.0f));
    ImGui::SetNextItemWidth(-FLT_MIN);
    OwnValues::BeginField(state);
    Data::Monsters::MonsterKind kind = current;
    if (KindField::RenderCombo("##Kind", kind))
    {
        result.kindChanged = row.number;
        result.kindOfMonster = row.kind == Row::Kind::Monster;
        result.kind = kind;
    }
    OwnValues::EndField(state);
    ImGui::PopStyleVar();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", state == OwnValues::State::Shared ? I18N::Editor::SharedValueLocked
                                                                  : I18N::Editor::KindChangeInGame);
    }
}

void RenderMapsCell(const Row& row, const MonsterDefinition& monster, MonsterBrowserList::Result& result)
{
    if (row.kind == Row::Kind::Monster)
    {
        // The maps of all its numbers.
        std::vector<Data::Maps::MapRef> maps;
        for (const int number : g_MonsterEdits.ReadMonsterNumbers(row.number))
        {
            const MonsterDefinition* member = g_MonsterDatabase.Find(number);
            maps.insert(maps.end(), member->maps.begin(), member->maps.end());
        }
        std::ranges::sort(maps);
        maps.erase(std::unique(maps.begin(), maps.end()), maps.end());
        ImGui::TextDisabled("%s", MapPicker::Describe(maps, I18N::Editor::NoMaps).c_str());
        return;
    }
    // A label for the tools: the server places the monsters (Q-MN98).
    std::vector<Data::Maps::MapRef> maps = monster.maps;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0.0f));
    if (MapPicker::Render("##Maps", maps, I18N::Editor::NoMaps))
    {
        result.mapsChanged = row.number;
        result.maps = std::move(maps);
    }
    ImGui::PopStyleVar();
}

void RenderContextMenu(const Row& row, MonsterBrowserList::Result& result)
{
    if (!ImGui::BeginPopupContextItem("##RowMenu"))
    {
        return;
    }
    const auto item = [&](const char* label, MonsterBrowserList::Action action)
    {
        if (ImGui::MenuItem(label))
        {
            result.action = action;
            result.actionNumber = row.number;
        }
    };
    using Action = MonsterBrowserList::Action;
    if (row.kind == Row::Kind::Number)
    {
        OwnValues::RenderMenuItems(row.number, MonsterValueField::Names);
        OwnValues::RenderMenuItems(row.number, MonsterValueField::Kind);
        ImGui::Separator();
        item(I18N::Editor::AddNumber, Action::AddNumber);
        item(I18N::Editor::CopyNumber, Action::CopyNumber);
        item(I18N::Editor::MoveNumberTo, Action::MoveNumber);
        item(I18N::Editor::SplitNumberOut, Action::SplitNumber);
        item(I18N::Editor::RemoveNumber, Action::RemoveNumber);
    }
    else
    {
        item(I18N::Editor::AddNumber, Action::AddNumber);
        item(I18N::Editor::CopyMonster, Action::CopyMonster);
        if (row.kind == Row::Kind::Single)
        {
            item(I18N::Editor::MoveNumberTo, Action::MoveNumber);
        }
        item(I18N::Editor::RemoveMonster, Action::RemoveMonster);
    }
    ImGui::EndPopup();
}

// The number column's text: the number, or for a monster with several the
// count of its numbers.
std::string NumberLabel(const Row& row)
{
    if (row.kind == Row::Kind::Monster)
    {
        return I18N::Format(I18N::Editor::NumberCount0, {std::to_string(row.numberCount)});
    }
    return std::to_string(row.number);
}

void RenderRow(const Row& row, bool selected, MonsterBrowserList::Result& result)
{
    // Read each frame: an undo, a revert or a language switch changes it.
    std::optional<MonsterDefinition> monster;
    if (row.kind == Row::Kind::Monster)
    {
        monster = g_MonsterEdits.ReadSharedValues(row.number);
    }
    else if (const MonsterDefinition* entry = g_MonsterDatabase.Find(row.number))
    {
        monster = *entry;
    }
    ImGui::TableNextRow();
    if (!monster)
    {
        return;
    }
    ImGui::PushID(row.kind == Row::Kind::Monster ? row.number + MonsterRowId : row.number);
    ImGui::TableSetColumnIndex(0);
    const float scale = g_MuEditorCore.GetUIScale();
    if (row.kind == Row::Kind::Number)
    {
        ImGui::Indent(NumberIndent * scale);
    }
    const float start = ImGui::GetCursorPosX();
    // The arrow of a monster row is drawn over the label's start.
    std::string label = row.kind == Row::Kind::Monster ? "     " + NumberLabel(row) : NumberLabel(row);
    label += "##Row";
    // AllowOverlap: the arrow, kind combo and maps button drawn over the row
    // take their own clicks.
    if (ImGui::Selectable(label.c_str(), selected,
                          ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap |
                              KeyboardSelection::RowFlags))
    {
        result.clicked = row.number;
        result.clickedMonster = row.kind == Row::Kind::Monster;
    }
    if (row.kind == Row::Kind::Monster && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
    {
        ImGui::SetTooltip("%s", I18N::Editor::MonsterRowNote);
    }
    RenderContextMenu(row, result);
    if (row.kind == Row::Kind::Monster)
    {
        ImGui::SameLine(start);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
        if (ImGui::ArrowButton("##Open", row.open ? ImGuiDir_Down : ImGuiDir_Right))
        {
            result.toggled = row.number;
            result.open = !row.open;
        }
        ImGui::PopStyleVar();
    }
    if (row.kind == Row::Kind::Number)
    {
        ImGui::Unindent(NumberIndent * scale);
    }

    ImGui::TableSetColumnIndex(1);
    const std::string& name = monster->names.Get(I18N::GetCurrentLocale());
    const OwnValues::State nameState = row.kind == Row::Kind::Number
                                           ? OwnValues::GetNumberState(row.number, MonsterValueField::Names)
                                           : OwnValues::State::Plain;
    if (nameState == OwnValues::State::Own)
    {
        ImGui::TextColored(OwnTextColor, "%s", name.c_str());
    }
    else if (nameState != OwnValues::State::Plain)
    {
        ImGui::TextColored(SharedTextColor, "%s", name.c_str());
    }
    else
    {
        ImGui::TextUnformatted(name.c_str());
    }
    ImGui::TableSetColumnIndex(2);
    RenderKindCell(row, monster->kind, result);
    ImGui::TableSetColumnIndex(3);
    RenderMapsCell(row, *monster, result);
    ImGui::PopID();
}
} // namespace

MonsterBrowserList::Result MonsterBrowserList::Render(const std::vector<MonsterTreeModel::Row>& rows,
                                                      int selectedNumber, bool selectedWhole, bool reveal)
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
    ImGui::TableSetupColumn(I18N::Editor::Kind, ImGuiTableColumnFlags_None, KindColumnWidth * uiScale);
    ImGui::TableSetupColumn(I18N::Editor::Maps, ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();

    // A whole monster is selected on its row, a number on its own row.
    const std::vector<int> selectedMonster = g_MonsterEdits.ReadMonsterNumbers(selectedNumber);
    const int selectedFirst = selectedMonster.empty() ? -1 : selectedMonster.front();
    const auto isSelected = [&](const Row& row)
    {
        if (row.kind == Row::Kind::Monster)
        {
            return selectedWhole && row.number == selectedFirst;
        }
        return row.number == selectedNumber && (!selectedWhole || row.kind == Row::Kind::Single);
    };
    // Only the rows in view are drawn, combos included.
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(rows.size()));
    std::optional<int> selectedRow;
    if (reveal && g_MonsterDatabase.Find(selectedNumber) != nullptr)
    {
        const auto found = std::ranges::find_if(rows, isSelected);
        if (found != rows.end())
        {
            selectedRow = static_cast<int>(found - rows.begin());
            clipper.IncludeItemByIndex(*selectedRow);
        }
    }
    while (clipper.Step())
    {
        for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
        {
            const Row& row = rows[static_cast<size_t>(index)];
            RenderRow(row, isSelected(row), result);
            if (selectedRow == index)
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
