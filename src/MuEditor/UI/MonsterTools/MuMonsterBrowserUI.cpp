#include "stdafx.h"

#ifdef _EDITOR

#include "MuMonsterBrowserUI.h"

#include "MonsterEdits.h"
#include "MonsterFileActions.h"
#include "MonsterKindField.h"
#include "MonsterSelection.h"
#include "MonsterStepLabel.h"
#include "MonsterToolWindow.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <span>
#include <string>
#include <string_view>

namespace MuEditor::Monsters
{
namespace
{
// Pixel widths before the editor UI scale.
constexpr float SearchWidth = 180.0f;
constexpr float FilterWidth = 160.0f;

using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterKind;
} // namespace

MuMonsterBrowserUI& MuMonsterBrowserUI::GetInstance()
{
    static MuMonsterBrowserUI instance;
    return instance;
}

void MuMonsterBrowserUI::Render(bool* open)
{
    // A mark in the title while there are changes that are not saved.
    std::string title = I18N::Editor::MonsterBrowser;
    if (g_MonsterEdits.GetHistory().IsModifiedSinceSave())
    {
        title += " *";
    }
    if (ToolWindow::Begin(title.c_str(), WindowIds::MonsterBrowser, open))
    {
        const int selected = g_MonsterSelection.GetNumber();
        RenderFileActions();
        RenderFilters();
        RenderMonsterActions(selected);
        ImGui::Separator();

        const bool reveal = RevealSelection(selected);
        m_filter.search = m_search;
        const MonsterBrowserList::Result rows = m_rows.Render(m_list.GetNumbers(m_filter), selected, reveal);
        if (rows.clicked >= 0)
        {
            g_MonsterSelection.Select(rows.clicked);
            m_shownSerial = g_MonsterSelection.GetSerial();
        }
        if (rows.kindChanged >= 0)
        {
            SetKind(rows.kindChanged, rows.kind);
        }
        m_addDialog.Render();
        m_removeDialog.Render();
    }
    ToolWindow::End();
}

void MuMonsterBrowserUI::RenderFileActions()
{
    g_MonsterFileActions.RenderSaveButton();
    ImGui::SameLine();
    if (g_MonsterFileActions.RenderRevertButton())
    {
        m_list.Refresh();
    }
    if (g_MonsterEdits.GetHistory().IsModifiedSinceSave())
    {
        ImGui::SameLine();
        ImGui::TextUnformatted(I18N::Editor::UnsavedChanges);
    }
    g_MonsterFileActions.RenderStatus();
}

void MuMonsterBrowserUI::RenderFilters()
{
    ImGui::SetNextItemWidth(SearchWidth * g_MuEditorCore.GetUIScale());
    Items::InputStringWithHint("##Search", I18N::Editor::NameOrNumber, m_search);
    ImGui::SameLine();
    RenderKindFilter();
    ImGui::SameLine();
    RenderMissingFilter();
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::ClearFilters))
    {
        m_filter = {};
        m_search.clear();
    }
}

void MuMonsterBrowserUI::RenderKindFilter()
{
    ImGui::SetNextItemWidth(FilterWidth * g_MuEditorCore.GetUIScale());
    const char* shown = m_filter.kind ? KindField::GetLabel(*m_filter.kind) : I18N::Editor::AllKinds;
    if (!ImGui::BeginCombo("##KindFilter", shown))
    {
        return;
    }
    if (ImGui::Selectable(I18N::Editor::AllKinds, !m_filter.kind))
    {
        m_filter.kind.reset();
    }
    for (const MonsterKind kind : KindField::GetKinds())
    {
        if (ImGui::Selectable(KindField::GetLabel(kind), m_filter.kind == kind))
        {
            m_filter.kind = kind;
        }
    }
    ImGui::EndCombo();
}

void MuMonsterBrowserUI::RenderMissingFilter()
{
    const auto missingName = [](const char* locale)
    { return I18N::Format(I18N::Editor::MissingANameIn0, {I18N::GetLanguageDisplayName(locale)}); };
    const std::string shown = m_filter.missingLocale.empty() ? std::string(I18N::Editor::AllMonsters)
                                                             : missingName(m_filter.missingLocale.c_str());
    ImGui::SetNextItemWidth(FilterWidth * g_MuEditorCore.GetUIScale());
    if (!ImGui::BeginCombo("##Missing", shown.c_str()))
    {
        return;
    }
    if (ImGui::Selectable(I18N::Editor::AllMonsters, m_filter.missingLocale.empty()))
    {
        m_filter.missingLocale.clear();
    }
    // English is the name every monster has; the other languages may lack it.
    for (const char* locale : I18N::GetAvailableLocales())
    {
        if (std::string_view(locale) != Data::LocalizedString::NeutralLocale &&
            ImGui::Selectable(missingName(locale).c_str(), m_filter.missingLocale == locale))
        {
            m_filter.missingLocale = locale;
        }
    }
    ImGui::EndCombo();
}

void MuMonsterBrowserUI::RenderMonsterActions(int selectedNumber)
{
    const bool hasMonster = g_MonsterDatabase.Find(selectedNumber) != nullptr;
    if (ImGui::Button(I18N::Editor::AddMonster))
    {
        m_addDialog.OpenAdd();
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!hasMonster);
    if (ImGui::Button(I18N::Editor::CopyMonster))
    {
        m_addDialog.OpenCopy(selectedNumber);
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::RemoveMonster))
    {
        m_removeDialog.Open(selectedNumber);
    }
    ImGui::EndDisabled();
}

void MuMonsterBrowserUI::SetKind(int number, MonsterKind kind)
{
    const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
    if (monster == nullptr)
    {
        return;
    }
    MonsterDefinition changed = *monster;
    changed.kind = kind;
    if (g_MonsterEdits.SetMonster(StepLabel::Kind(changed), changed))
    {
        // The row stays while a kind filter would hide it now.
        m_list.KeepRowsAfterOwnEdit();
        // The monster whose kind changed is the one the tools show.
        g_MonsterSelection.Select(number);
        m_shownSerial = g_MonsterSelection.GetSerial();
    }
}

bool MuMonsterBrowserUI::RevealSelection(int selectedNumber)
{
    if (g_MonsterSelection.GetSerial() == m_shownSerial)
    {
        return false;
    }
    m_shownSerial = g_MonsterSelection.GetSerial();
    // A number without an entry (a removed monster, an undone Add) is in no
    // list: the filters stay.
    if (g_MonsterDatabase.Find(selectedNumber) == nullptr)
    {
        return false;
    }
    m_filter.search = m_search;
    const std::vector<int>& listed = m_list.GetNumbers(m_filter);
    if (std::ranges::find(listed, selectedNumber) == listed.end())
    {
        m_filter = {};
        m_search.clear();
    }
    return true;
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
