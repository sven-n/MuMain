#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterRemoveDialog.h"

#include "MonsterCodeReferenceIndex.h"
#include "MonsterEdits.h"
#include "MonsterKindField.h"
#include "MonsterStepLabel.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/CodeReferenceList.h"
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
    if (g_MonsterDatabase.Find(number) == nullptr)
    {
        return;
    }
    m_number = number;
    m_dataUsers = FindMonsterDataUsers(number);
    // Reads the sources on the first remove, not when the editor starts.
    GetMonsterCodeReferences();
    m_open = true;
    ImGui::OpenPopup(PopupId);
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
    ImGui::TextUnformatted(StepLabel::LogName(*monster).c_str());
    ImGui::TextUnformatted(I18N::Editor::TheMonsterEntryIsRemoved);
    const char* legacyKind = KindField::GetLabel(KindField::GetLegacyKind(m_number));
    RenderWrapped(I18N::Format(I18N::Editor::MonsterWithoutEntryInGame, {legacyKind}).c_str(), false);
    RenderWrapped(I18N::Editor::RemovedMonsterInGame, true);
    ImGui::Separator();
    RenderUses();
    ImGui::Separator();

    ImGui::PushStyleColor(ImGuiCol_Button, RemoveColor);
    if (ImGui::Button(I18N::Editor::Remove))
    {
        g_MonsterEdits.RemoveMonster(StepLabel::Remove(*monster), m_number);
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

void MonsterRemoveDialog::RenderUses() const
{
    const MonsterCodeReferences& references = GetMonsterCodeReferences();
    const std::span<const CodeReference> code = references.Find(m_number);
    // Without the sources, code may still name the monster.
    if (references.IsLoaded() && code.empty() && m_dataUsers.IsEmpty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::NothingUsesTheMonster);
    }
    RenderDataUsers();
    CodeReferenceList::Render(references.IsLoaded(), code,
                              I18N::Editor::TheSourcesWereNotFoundCodeThatNamesTheMonsterIsNotKnown);
    if (references.IsLoaded())
    {
        RenderWrapped(I18N::Editor::MonsterCodeReferencesLimit, true);
    }
}

void MonsterRemoveDialog::RenderDataUsers() const
{
    // Quest names are data: arguments, never the format.
    if (!m_dataUsers.npcOfQuests.empty())
    {
        ImGui::BulletText("%s", I18N::Format(I18N::Editor::NPCOfTheQuests0, {Joined(m_dataUsers.npcOfQuests)}).c_str());
    }
    if (!m_dataUsers.killedInQuests.empty())
    {
        ImGui::BulletText("%s",
                          I18N::Format(I18N::Editor::KilledInTheQuests0, {Joined(m_dataUsers.killedInQuests)}).c_str());
    }
    if (m_dataUsers.dialogueStates > 0)
    {
        ImGui::BulletText(
            "%s", I18N::Format(I18N::Editor::DialoguesOfTheQuestSystem0, {std::to_string(m_dataUsers.dialogueStates)})
                      .c_str());
    }
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
