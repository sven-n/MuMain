#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterBrowserList.h"

#include "MonsterEdits.h"
#include "MonsterKindField.h"
#include "MonsterOwnValues.h"
#include "MonsterShownValues.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/KeyboardSelection.h"
#include "../MuEditor/UI/Common/MapPicker.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
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

// `monsterMaps`: the maps of all numbers of a monster row, which shows them
// only.
void RenderMapsCell(const Row& row, const MonsterDefinition& monster, const char* monsterMaps,
                    MonsterBrowserList::Result& result)
{
    if (row.kind == Row::Kind::Monster)
    {
        ImGui::TextDisabled("%s", monsterMaps);
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

// The number column: the number, or for a monster row `monsterLabel`, the
// count of its numbers, with the arrow that opens it.
void RenderNumberCell(const Row& row, bool selected, const char* monsterLabel, MonsterBrowserList::Result& result)
{
    const float scale = g_MuEditorCore.GetUIScale();
    if (row.kind == Row::Kind::Number)
    {
        ImGui::Indent(NumberIndent * scale);
    }
    const float start = ImGui::GetCursorPosX();
    char numberLabel[24];
    std::snprintf(numberLabel, sizeof(numberLabel), "%d##Row", row.number);
    const char* label = row.kind == Row::Kind::Monster ? monsterLabel : numberLabel;
    // AllowOverlap: the arrow, kind combo and maps button drawn over the row
    // take their own clicks.
    if (ImGui::Selectable(label, selected,
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
}

// `monsterLabel` and `monsterMaps`: the texts of a monster row
// (MonsterRowTexts), nullptr for the others.
void RenderRow(const Row& row, bool selected, const char* monsterLabel, const char* monsterMaps,
               MonsterBrowserList::Result& result)
{
    // Read each frame, without copies: an undo, a revert or a language switch
    // changes it.
    const MonsterDefinition* monster = row.kind == Row::Kind::Monster ? g_MonsterEdits.FindSharedValues(row.number)
                                                                      : g_MonsterDatabase.Find(row.number);
    ImGui::TableNextRow();
    if (monster == nullptr)
    {
        return;
    }
    // A monster row's id is apart from its first number's row, as the tools
    // key a monster as a whole.
    ImGui::PushID(row.kind == Row::Kind::Monster ? row.number + ShownValues::WholeMonsterKey : row.number);
    ImGui::TableSetColumnIndex(0);
    RenderNumberCell(row, selected, monsterLabel, result);
    ImGui::TableSetColumnIndex(1);
    const std::string& name = monster->names.Get(I18N::GetCurrentLocale());
    const OwnValues::State nameState = row.kind == Row::Kind::Number
                                           ? OwnValues::GetNumberState(row.number, MonsterValueField::Names)
                                           : OwnValues::State::Plain;
    OwnValues::RenderValueText(nameState, name);
    ImGui::TableSetColumnIndex(2);
    RenderKindCell(row, monster->kind, result);
    ImGui::TableSetColumnIndex(3);
    RenderMapsCell(row, *monster, monsterMaps, result);
    ImGui::PopID();
}
} // namespace

void MonsterBrowserList::ForgetChangedTexts()
{
    const int version = g_MonsterDatabase.GetVersion() + g_MonsterDataHandler.GetGroupsVersion();
    const char* locale = I18N::GetCurrentLocale();
    if (version == m_textsVersion && m_textsLocale == locale)
    {
        return;
    }
    m_monsterRowTexts.clear();
    m_textsVersion = version;
    m_textsLocale = locale;
}

const MonsterBrowserList::MonsterRowTexts& MonsterBrowserList::GetMonsterRowTexts(const Row& row)
{
    const auto [texts, added] = m_monsterRowTexts.try_emplace(row.number);
    if (added)
    {
        // The arrow is drawn over the label's start.
        texts->second.label =
            "     " + I18N::Format(I18N::Editor::NumberCount0, {std::to_string(row.numberCount)}) + "##Row";
        texts->second.maps = MapPicker::Describe(g_MonsterEdits.ReadMaps(g_MonsterEdits.ReadMonsterNumbers(row.number)),
                                                 I18N::Editor::NoMaps);
    }
    return texts->second;
}

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
    ForgetChangedTexts();
    const float uiScale = g_MuEditorCore.GetUIScale();
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_None, NumberColumnWidth * uiScale);
    ImGui::TableSetupColumn(I18N::Metadata::Name, ImGuiTableColumnFlags_None, NameColumnWidth * uiScale);
    ImGui::TableSetupColumn(I18N::Editor::Kind, ImGuiTableColumnFlags_None, KindColumnWidth * uiScale);
    ImGui::TableSetupColumn(I18N::Editor::Maps, ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();

    const auto isSelected = [&](const Row& row)
    { return MonsterTreeModel::ShowsSelection(row, selectedNumber, selectedWhole); };
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
            const MonsterRowTexts* texts = row.kind == Row::Kind::Monster ? &GetMonsterRowTexts(row) : nullptr;
            RenderRow(row, isSelected(row), texts != nullptr ? texts->label.c_str() : nullptr,
                      texts != nullptr ? texts->maps.c_str() : nullptr, result);
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
