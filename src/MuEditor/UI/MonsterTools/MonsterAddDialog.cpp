#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterAddDialog.h"

#include "MonsterEdits.h"
#include "MonsterKindField.h"
#include "MonsterListModel.h"
#include "MonsterSelection.h"
#include "MonsterStepLabel.h"

#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <cstdio>

namespace MuEditor::Monsters
{
namespace
{
constexpr char PopupId[] = "###MonsterAddDialog";
constexpr float FieldWidth = 220.0f;
constexpr ImVec4 NotFreeColor{1.0f, 0.4f, 0.4f, 1.0f};

using Data::Monsters::MonsterDefinition;
} // namespace

void MonsterAddDialog::OpenAdd()
{
    m_mode = Mode::Add;
    m_sourceNumber = -1;
    m_number = GetNextFreeMonsterNumber(0);
    m_name.clear();
    m_kindPicked = false;
    m_open = true;
    ImGui::OpenPopup(PopupId);
}

void MonsterAddDialog::OpenCopy(int sourceNumber)
{
    const MonsterDefinition* source = g_MonsterDatabase.Find(sourceNumber);
    if (source == nullptr)
    {
        return;
    }
    m_mode = Mode::Copy;
    m_sourceNumber = sourceNumber;
    m_number = GetNextFreeMonsterNumber(sourceNumber);
    m_name = source->names.GetNeutral();
    m_kind = source->kind;
    m_kindPicked = true;
    m_open = true;
    ImGui::OpenPopup(PopupId);
}

void MonsterAddDialog::Render()
{
    const char* title = m_mode == Mode::Add ? I18N::Editor::AddMonster : I18N::Editor::CopyMonster;
    char fullTitle[128];
    std::snprintf(fullTitle, sizeof(fullTitle), "%s%s", title, PopupId);
    if (!ImGui::BeginPopupModal(fullTitle, &m_open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    if (m_mode == Mode::Copy)
    {
        // The source is read each frame: an undo may have removed it.
        if (const MonsterDefinition* source = g_MonsterDatabase.Find(m_sourceNumber))
        {
            ImGui::TextDisabled("%s",
                                I18N::Format(I18N::Editor::CopyMonsterNote, {StepLabel::LogName(*source)}).c_str());
        }
    }
    RenderNumber();
    ImGui::SetNextItemWidth(FieldWidth);
    Items::InputString(I18N::Editor::EnglishName, m_name);
    RenderKind();

    ImGui::Separator();
    ImGui::BeginDisabled(!CanConfirm());
    if (ImGui::Button(I18N::Editor::Add))
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

void MonsterAddDialog::RenderNumber()
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

void MonsterAddDialog::RenderKind()
{
    // The kind the game gave the number so far, until another one is picked.
    if (!m_kindPicked)
    {
        m_kind = KindField::GetLegacyKind(m_number);
    }
    ImGui::SetNextItemWidth(FieldWidth);
    if (KindField::RenderCombo(I18N::Editor::Kind, m_kind))
    {
        m_kindPicked = true;
    }
}

bool MonsterAddDialog::CanConfirm() const
{
    return !m_name.empty() && !Data::LocalizedString::ContainsSeparator(m_name) && IsFreeMonsterNumber(m_number);
}

void MonsterAddDialog::Confirm()
{
    MonsterDefinition monster;
    if (m_mode == Mode::Copy)
    {
        if (const MonsterDefinition* source = g_MonsterDatabase.Find(m_sourceNumber))
        {
            monster = *source;
        }
    }
    monster.number = m_number;
    // Only the English name: the translations of a copied monster name that
    // monster.
    monster.names = {};
    monster.names.Set(Data::LocalizedString::NeutralLocale, m_name);
    monster.kind = m_kind;
    g_MonsterEdits.AddMonster(StepLabel::Add(monster), monster);
    g_MonsterSelection.Select(m_number);
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
