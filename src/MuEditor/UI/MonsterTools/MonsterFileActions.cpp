#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterFileActions.h"

#include "MonsterEdits.h"

#include "../MuEditor/UI/Common/IssueLevels.h"

#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/DataHandler/MonsterData/MonsterModelJsonStorage.h"
#include "I18N/All.h"
#include "imgui.h"

namespace MuEditor::Monsters
{
namespace
{
constexpr ImVec4 ErrorColor{1.0f, 0.4f, 0.4f, 1.0f};

// The files as the user finds them below the client folder: the monster
// list and the model list.
std::string MonsterFileNames()
{
    return Data::Monsters::GetMonstersFile().generic_string() + ", " +
           Data::Monsters::GetMonsterModelsFile().generic_string();
}

void RenderError(const char* text)
{
    ImGui::PushStyleColor(ImGuiCol_Text, ErrorColor);
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(text);
    ImGui::PopTextWrapPos();
    ImGui::PopStyleColor();
}
} // namespace

MonsterFileActions& MonsterFileActions::GetInstance()
{
    static MonsterFileActions instance;
    return instance;
}

bool MonsterFileActions::RenderSaveButton()
{
    return ImGui::Button(I18N::Editor::SaveMonsters) && Save();
}

bool MonsterFileActions::RenderRevertButton()
{
    const bool reverted = ImGui::Button(I18N::Editor::Revert) && Revert();
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s",
                          I18N::Format(I18N::Editor::Reads0AgainUnsavedChangesAreLost, {MonsterFileNames()}).c_str());
    }
    return reverted;
}

bool MonsterFileActions::Save()
{
    m_issues.clear();
    if (!g_MonsterEdits.Save(m_issues))
    {
        m_status = I18N::Editor::NotSavedSeeTheProblemsBelow;
        m_statusIsError = true;
        return false;
    }
    m_status = I18N::Format(I18N::Editor::SavedTo0, {MonsterFileNames()});
    m_statusIsError = false;
    return true;
}

bool MonsterFileActions::Revert()
{
    m_issues.clear();
    // A file with fatal problems leaves its table as it is; the problems say why.
    const bool reverted = g_MonsterEdits.Revert(m_issues);
    m_status.clear();
    m_statusIsError = false;
    return reverted;
}

void MonsterFileActions::RenderStatus() const
{
    if (!m_status.empty())
    {
        if (m_statusIsError)
        {
            RenderError(m_status.c_str());
        }
        else
        {
            ImGui::TextUnformatted(m_status.c_str());
        }
    }
    // The problems under the file they are in, in the order they came.
    const std::string* lastSource = nullptr;
    for (const Data::Items::ItemDataIssue& issue : m_issues)
    {
        if (lastSource == nullptr || *lastSource != issue.source)
        {
            ImGui::TextUnformatted(I18N::Format(I18N::Editor::ProblemsIn0, {issue.source}).c_str());
            lastSource = &issue.source;
        }
        const std::string line = issue.field.empty() ? issue.message : issue.field + ": " + issue.message;
        IssueLevels::RenderLine(issue.severity, line.c_str());
    }
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
