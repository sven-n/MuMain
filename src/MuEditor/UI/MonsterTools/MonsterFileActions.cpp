#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterFileActions.h"

#include "MonsterEdits.h"

#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "I18N/All.h"
#include "imgui.h"

namespace MuEditor::Monsters
{
namespace
{
constexpr ImVec4 ErrorColor{1.0f, 0.4f, 0.4f, 1.0f};

// The file as the user finds it below the client folder.
std::string MonstersFileName()
{
    return Data::Monsters::GetMonstersFile().generic_string();
}

void RenderNote(const char* text)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", text);
    ImGui::PopTextWrapPos();
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
                          I18N::Format(I18N::Editor::Reads0AgainUnsavedChangesAreLost, {MonstersFileName()}).c_str());
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
    m_status = I18N::Format(I18N::Editor::SavedTo0, {MonstersFileName()});
    m_statusIsError = false;
    return true;
}

bool MonsterFileActions::Revert()
{
    m_issues.clear();
    // On errors the monsters stay as they are; the problems say why.
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
    if (m_issues.empty())
    {
        return;
    }
    ImGui::TextUnformatted(I18N::Format(I18N::Editor::ProblemsIn0, {MonstersFileName()}).c_str());
    for (const Data::Items::ItemDataIssue& issue : m_issues)
    {
        const std::string line = issue.field.empty() ? issue.message : issue.field + ": " + issue.message;
        if (issue.severity == Data::Items::ItemDataIssueSeverity::Error)
        {
            RenderError(line.c_str());
        }
        else
        {
            RenderNote(line.c_str());
        }
    }
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
