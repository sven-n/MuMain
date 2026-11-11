#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterRemoveDialog.h"

#include "MonsterCodeReferenceIndex.h"
#include "MonsterEdits.h"
#include "MonsterKindField.h"
#include "MonsterShownValues.h"
#include "MonsterStepLabel.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/CodeReferenceList.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <cstdio>
#include <span>
#include <string>
#include <vector>

namespace MuEditor::Monsters
{
namespace
{
constexpr char PopupId[] = "###MonsterRemoveDialog";
constexpr ImVec4 RemoveColor{0.7f, 0.2f, 0.2f, 1.0f};
// Where the longer texts wrap, in pixels before the editor UI scale.
constexpr float TextWrapWidth = 520.0f;

void RenderWrapped(const char* text, bool disabled)
{
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + TextWrapWidth * g_MuEditorCore.GetUIScale());
    if (disabled)
    {
        ImGui::TextDisabled("%s", text);
    }
    else
    {
        ImGui::TextUnformatted(text);
    }
    ImGui::PopTextWrapPos();
}

// Changes with the entries and the monsters' numbers.
int ReadDataVersion()
{
    return g_MonsterDatabase.GetVersion() + g_MonsterDataHandler.GetGroupsVersion();
}

std::string Joined(const std::vector<std::string>& names)
{
    std::string joined;
    for (const std::string& name : names)
    {
        joined += (joined.empty() ? "" : ", ") + name;
    }
    return joined;
}
} // namespace

void MonsterRemoveDialog::Open(int number)
{
    Open(number, false);
}

void MonsterRemoveDialog::OpenWholeMonster(int number)
{
    Open(number, true);
}

void MonsterRemoveDialog::Open(int number, bool wholeMonster)
{
    if (g_MonsterDatabase.Find(number) == nullptr)
    {
        return;
    }
    m_number = number;
    m_wholeRequested = wholeMonster;
    ReadNumbers();
    // Reads the sources on the first remove, not when the editor starts.
    GetMonsterCodeReferences();
    m_open = true;
    ImGui::OpenPopup(PopupId);
}

void MonsterRemoveDialog::ReadNumbers()
{
    const std::vector<int> numbers = g_MonsterEdits.ReadMonsterNumbers(m_number);
    m_wholeMonster = m_wholeRequested && numbers.size() > 1;
    m_numbers = m_wholeMonster ? numbers : std::vector{m_number};
    m_dataUsers.clear();
    for (const int member : m_numbers)
    {
        m_dataUsers.push_back(FindMonsterDataUsers(member));
    }
    m_numbersVersion = ReadDataVersion();
}

void MonsterRemoveDialog::Render()
{
    char title[128];
    std::snprintf(title, sizeof(title), "%s%s", I18N::Editor::RemoveMonster, PopupId);
    if (!ImGui::BeginPopupModal(title, &m_open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    // Read each frame: an undo may have removed the entry meanwhile.
    const Data::Monsters::MonsterDefinition* monster = g_MonsterDatabase.Find(m_number);
    if (monster == nullptr)
    {
        ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
        return;
    }
    // The numbers listed are the ones Remove removes, also after an undo.
    if (m_numbersVersion != ReadDataVersion())
    {
        ReadNumbers();
    }
    const std::string numbers = ShownValues::ListNumbers(m_numbers);
    // A whole monster by the name its numbers share, not its first number's.
    const std::string name = m_wholeMonster ? ShownValues::MonsterName(m_number) : monster->names.GetNeutral();
    if (m_wholeMonster)
    {
        RenderWrapped(I18N::Format(I18N::Editor::RemoveMonsterWithNumbers01, {name, numbers}).c_str(), false);
    }
    else
    {
        ImGui::TextUnformatted(StepLabel::LogName(*monster).c_str());
        ImGui::TextUnformatted(I18N::Editor::TheMonsterEntryIsRemoved);
        const char* legacyKind = KindField::GetLabel(KindField::GetLegacyKind(m_number));
        RenderWrapped(I18N::Format(I18N::Editor::MonsterWithoutEntryInGame, {legacyKind}).c_str(), false);
    }
    RenderWrapped(I18N::Editor::RemovedMonsterInGame, true);
    ImGui::Separator();
    for (size_t index = 0; index < m_numbers.size(); ++index)
    {
        if (m_wholeMonster)
        {
            ImGui::TextUnformatted(std::to_string(m_numbers[index]).c_str());
            ImGui::Indent();
        }
        RenderUses(m_numbers[index], m_dataUsers[index]);
        if (m_wholeMonster)
        {
            ImGui::Unindent();
        }
    }
    ImGui::Separator();

    ImGui::PushStyleColor(ImGuiCol_Button, RemoveColor);
    if (ImGui::Button(I18N::Editor::Remove))
    {
        if (m_wholeMonster)
        {
            g_MonsterEdits.RemoveMonsterWithNumbers(
                StepLabel::RemoveMonsterWithNumbers(name, static_cast<int>(m_numbers.size())), m_number);
        }
        else
        {
            g_MonsterEdits.RemoveMonster(StepLabel::Remove(*monster), m_number);
        }
        ImGui::CloseCurrentPopup();
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void MonsterRemoveDialog::RenderUses(int number, const MonsterDataUsers& users) const
{
    const MonsterCodeReferences& references = GetMonsterCodeReferences();
    const std::span<const CodeReference> code = references.Find(number);
    // Without the sources, code may still name the monster.
    if (references.IsLoaded() && code.empty() && users.IsEmpty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::NothingUsesTheMonster);
    }
    RenderDataUsers(users);
    CodeReferenceList::Render(references.IsLoaded(), code,
                              I18N::Editor::TheSourcesWereNotFoundCodeThatNamesTheMonsterIsNotKnown);
    if (references.IsLoaded())
    {
        RenderWrapped(I18N::Editor::MonsterCodeReferencesLimit, true);
    }
}

void MonsterRemoveDialog::RenderDataUsers(const MonsterDataUsers& users) const
{
    // Quest names are data: arguments, never the format.
    if (!users.npcOfQuests.empty())
    {
        ImGui::BulletText("%s", I18N::Format(I18N::Editor::NPCOfTheQuests0, {Joined(users.npcOfQuests)}).c_str());
    }
    if (!users.killedInQuests.empty())
    {
        ImGui::BulletText("%s", I18N::Format(I18N::Editor::KilledInTheQuests0, {Joined(users.killedInQuests)}).c_str());
    }
    if (users.dialogueStates > 0)
    {
        ImGui::BulletText(
            "%s",
            I18N::Format(I18N::Editor::DialoguesOfTheQuestSystem0, {std::to_string(users.dialogueStates)}).c_str());
    }
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
