#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterNumberDialog.h"

#include "MonsterEdits.h"
#include "MonsterListModel.h"
#include "MonsterSelection.h"
#include "MonsterShownValues.h"
#include "MonsterStepLabel.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <cstdio>
#include <vector>

namespace MuEditor::Monsters
{
namespace
{
constexpr char PopupId[] = "###MonsterNumberDialog";
// Pixel sizes before the editor UI scale.
constexpr float FieldWidth = 220.0f;
constexpr float TargetListWidth = 360.0f;
constexpr float TargetListHeight = 240.0f;
constexpr ImVec4 NotFreeColor{1.0f, 0.4f, 0.4f, 1.0f};

using Data::Monsters::MonsterDefinition;

// "Death Centurion (145, 175, 336)": a monster with its numbers.
std::string MonsterWithNumbers(int number)
{
    return ShownValues::MonsterName(number) + " (" +
           ShownValues::ListNumbers(g_MonsterEdits.ReadMonsterNumbers(number)) + ")";
}
} // namespace

void MonsterNumberDialog::OpenAddNumber(int memberNumber)
{
    Open(Mode::AddNumber, memberNumber);
}

void MonsterNumberDialog::OpenCopyNumber(int number)
{
    Open(Mode::CopyNumber, number);
}

void MonsterNumberDialog::OpenMoveNumber(int number)
{
    Open(Mode::MoveNumber, number);
}

void MonsterNumberDialog::Open(Mode mode, int source)
{
    const std::vector<int> numbers = g_MonsterEdits.ReadMonsterNumbers(source);
    if (numbers.empty())
    {
        return;
    }
    m_mode = mode;
    m_source = source;
    // The next free number after the monster's last one.
    m_number = GetNextFreeMonsterNumber(numbers.back());
    m_target = -1;
    m_search.clear();
    m_targetsVersion = -1;
    m_open = true;
    ImGui::OpenPopup(PopupId);
}

void MonsterNumberDialog::Render()
{
    const char* title = m_mode == Mode::AddNumber    ? I18N::Editor::AddNumber
                        : m_mode == Mode::CopyNumber ? I18N::Editor::CopyNumber
                                                     : I18N::Editor::MoveNumberTo;
    char fullTitle[128];
    std::snprintf(fullTitle, sizeof(fullTitle), "%s%s", title, PopupId);
    if (!ImGui::BeginPopupModal(fullTitle, &m_open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    // Read each frame: an undo may have removed the source.
    const MonsterDefinition* source = g_MonsterDatabase.Find(m_source);
    if (source == nullptr)
    {
        ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
        return;
    }
    switch (m_mode)
    {
    case Mode::AddNumber:
        ImGui::TextUnformatted(I18N::Format(I18N::Editor::AddANumberTo0, {MonsterWithNumbers(m_source)}).c_str());
        ImGui::TextDisabled("%s", I18N::Editor::AddNumberNote);
        RenderNumber();
        break;
    case Mode::CopyNumber:
        ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::CopyNumberNote0, {StepLabel::LogName(*source)}).c_str());
        RenderNumber();
        break;
    case Mode::MoveNumber:
        ImGui::TextUnformatted(I18N::Format(I18N::Editor::Move0Into, {StepLabel::LogName(*source)}).c_str());
        RenderTargets();
        ImGui::TextDisabled("%s", I18N::Editor::MoveNumberNote);
        break;
    }

    ImGui::Separator();
    ImGui::BeginDisabled(!CanConfirm());
    if (ImGui::Button(m_mode == Mode::MoveNumber ? I18N::Editor::Move : I18N::Editor::Add))
    {
        Confirm();
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void MonsterNumberDialog::RenderNumber()
{
    ImGui::SetNextItemWidth(FieldWidth);
    ImGui::InputInt(I18N::Editor::Number, &m_number);
    m_number = std::clamp(m_number, 0, Data::Monsters::MonsterNumberCount - 1);
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::NextFree))
    {
        m_number = GetNextFreeMonsterNumber(m_number + 1);
    }
    if (!IsFreeMonsterNumber(m_number))
    {
        ImGui::TextColored(NotFreeColor, "%s", I18N::Editor::ThisNumberIsNotFree);
    }
}

void MonsterNumberDialog::RenderTargets()
{
    const float scale = g_MuEditorCore.GetUIScale();
    ImGui::SetNextItemWidth(TargetListWidth * scale);
    Items::InputStringWithHint("##Search", I18N::Editor::NameOrNumber, m_search);
    if (!ImGui::BeginChild("##Targets", ImVec2(TargetListWidth * scale, TargetListHeight * scale),
                           ImGuiChildFlags_Borders))
    {
        ImGui::EndChild();
        return;
    }
    const int version = g_MonsterDatabase.GetVersion() + g_MonsterDataHandler.GetGroupsVersion();
    if (version != m_targetsVersion || m_search != m_targetsSearch)
    {
        BuildTargets();
        m_targetsVersion = version;
        m_targetsSearch = m_search;
    }
    // Only the rows in view are drawn.
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(m_targets.size()));
    while (clipper.Step())
    {
        for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
        {
            const Target& target = m_targets[static_cast<size_t>(index)];
            if (ImGui::Selectable(target.label.c_str(), m_target == target.number))
            {
                m_target = target.number;
            }
        }
    }
    ImGui::EndChild();
}

void MonsterNumberDialog::BuildTargets()
{
    m_targets.clear();
    const std::vector<int> own = g_MonsterEdits.ReadMonsterNumbers(m_source);
    for (const std::vector<int>& numbers : g_MonsterEdits.ReadMonsters())
    {
        const bool matches = std::ranges::any_of(
            numbers, [&](int member) { return MatchesMonsterSearch(*g_MonsterDatabase.Find(member), m_search); });
        // Not the monster the number is in.
        if (matches && numbers != own)
        {
            const int first = numbers.front();
            m_targets.push_back({first, MonsterWithNumbers(first) + "##" + std::to_string(first)});
        }
    }
    // A monster the list no longer shows is no longer picked: Move would
    // move the number into a monster out of sight.
    if (std::ranges::none_of(m_targets, [this](const Target& target) { return target.number == m_target; }))
    {
        m_target = -1;
    }
}

bool MonsterNumberDialog::CanConfirm() const
{
    if (m_mode == Mode::MoveNumber)
    {
        return m_target >= 0 && g_MonsterDatabase.Find(m_target) != nullptr;
    }
    return IsFreeMonsterNumber(m_number);
}

void MonsterNumberDialog::Confirm()
{
    const MonsterDefinition* source = g_MonsterDatabase.Find(m_source);
    if (source == nullptr)
    {
        return;
    }
    switch (m_mode)
    {
    case Mode::AddNumber:
        if (g_MonsterEdits.AddNumber(StepLabel::AddNumber(m_number, ShownValues::MonsterName(m_source)), m_source,
                                     m_number))
        {
            g_MonsterSelection.Select(m_number);
        }
        break;
    case Mode::CopyNumber:
        if (g_MonsterEdits.CopyNumber(StepLabel::CopyNumber(*source, m_number), m_source, m_number))
        {
            g_MonsterSelection.Select(m_number);
        }
        break;
    case Mode::MoveNumber:
        if (g_MonsterEdits.MoveNumber(StepLabel::MoveNumber(*source, ShownValues::MonsterName(m_target)), m_source,
                                      m_target))
        {
            g_MonsterSelection.Select(m_source);
        }
        break;
    }
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
