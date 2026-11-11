#include "stdafx.h"

#ifdef _EDITOR

#include "MuMonsterBrowserUI.h"

#include "MonsterEdits.h"
#include "MonsterFileActions.h"
#include "MonsterKindField.h"
#include "MonsterOwnValues.h"
#include "MonsterSelection.h"
#include "MonsterShownValues.h"
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
#include <optional>
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
        const MonsterBrowserList::Result rows =
            m_rows.Render(m_list.GetRows(m_filter), selected, g_MonsterSelection.IsWholeMonster(), reveal);
        if (rows.clicked >= 0)
        {
            SelectFromList(rows.clicked, rows.clickedMonster);
        }
        if (rows.toggled >= 0)
        {
            m_list.SetOpen(rows.toggled, rows.open);
        }
        if (rows.kindChanged >= 0)
        {
            SetKind(rows.kindChanged, rows.kindOfMonster, rows.kind);
        }
        if (rows.mapsChanged >= 0)
        {
            SetMaps(rows.mapsChanged, rows.maps);
        }
        if (rows.action != MonsterBrowserList::Action::None)
        {
            StartAction(rows.action, rows.actionNumber);
        }
        m_addDialog.Render();
        m_numberDialog.Render();
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
        OwnValues::LockAll();
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
    using Action = MonsterBrowserList::Action;
    const bool hasMonster = g_MonsterDatabase.Find(selectedNumber) != nullptr;
    // A number of a monster with several numbers (which has shared values),
    // or a monster as a whole.
    const bool number = hasMonster && !g_MonsterSelection.IsWholeMonster() &&
                        g_MonsterEdits.FindSharedValues(selectedNumber) != nullptr;
    const auto button = [&](const char* label, Action action)
    {
        ImGui::SameLine();
        if (ImGui::Button(label))
        {
            StartAction(action, selectedNumber);
        }
    };
    if (ImGui::Button(I18N::Editor::AddMonster))
    {
        m_addDialog.OpenAdd();
    }
    ImGui::BeginDisabled(!hasMonster);
    button(I18N::Editor::AddNumber, Action::AddNumber);
    if (number)
    {
        button(I18N::Editor::CopyNumber, Action::CopyNumber);
        button(I18N::Editor::MoveNumberTo, Action::MoveNumber);
        button(I18N::Editor::SplitNumberOut, Action::SplitNumber);
        button(I18N::Editor::RemoveNumber, Action::RemoveNumber);
    }
    else
    {
        button(I18N::Editor::CopyMonster, Action::CopyMonster);
        button(I18N::Editor::RemoveMonster, Action::RemoveMonster);
    }
    ImGui::EndDisabled();
}

void MuMonsterBrowserUI::StartAction(MonsterBrowserList::Action action, int number)
{
    using Action = MonsterBrowserList::Action;
    const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
    if (monster == nullptr)
    {
        return;
    }
    switch (action)
    {
    case Action::None:
        break;
    case Action::AddNumber:
        m_numberDialog.OpenAddNumber(number);
        break;
    case Action::CopyMonster:
        m_addDialog.OpenCopy(number);
        break;
    case Action::CopyNumber:
        m_numberDialog.OpenCopyNumber(number);
        break;
    case Action::MoveNumber:
        m_numberDialog.OpenMoveNumber(number);
        break;
    case Action::SplitNumber:
    {
        const std::string from = ShownValues::MonsterName(number);
        if (g_MonsterEdits.SplitNumber(StepLabel::SplitNumber(*monster, from), number))
        {
            g_MonsterSelection.Select(number);
        }
        break;
    }
    case Action::RemoveNumber:
        m_removeDialog.Open(number);
        break;
    case Action::RemoveMonster:
        m_removeDialog.OpenWholeMonster(number);
        break;
    }
}

void MuMonsterBrowserUI::SetKind(int number, bool wholeMonster, MonsterKind kind)
{
    const std::optional<ShownValues::ShownMonster> shown =
        ShownValues::Load(wholeMonster ? number + ShownValues::WholeMonsterKey : number);
    if (!shown)
    {
        return;
    }
    ShownValues::ShownMonster changed = *shown;
    changed.monster.kind = kind;
    // "Death Centurion (3 numbers): kind" for a monster as a whole.
    const std::string label = StepLabel::Kind(changed);
    if (ShownValues::Store(label, changed))
    {
        SelectFromList(number, wholeMonster);
    }
}

void MuMonsterBrowserUI::SetMaps(int number, const std::vector<Data::Maps::MapRef>& maps)
{
    const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
    if (monster == nullptr)
    {
        return;
    }
    MonsterDefinition changed = *monster;
    changed.maps = maps;
    if (g_MonsterEdits.SetMonster(StepLabel::Maps(changed), changed))
    {
        SelectFromList(number, false);
    }
}

void MuMonsterBrowserUI::SelectFromList(int number, bool wholeMonster)
{
    if (wholeMonster)
    {
        g_MonsterSelection.SelectMonster(number);
    }
    else
    {
        g_MonsterSelection.Select(number);
    }
    // The row stays while a filter would hide it now.
    m_list.KeepRowsAfterOwnEdit();
    m_shownSerial = g_MonsterSelection.GetSerial();
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
    // A number shows in its monster's rows.
    if (!g_MonsterSelection.IsWholeMonster())
    {
        m_list.SetOpen(selectedNumber, true);
    }
    m_filter.search = m_search;
    const std::vector<MonsterTreeModel::Row>& listed = m_list.GetRows(m_filter);
    const bool whole = g_MonsterSelection.IsWholeMonster();
    const bool shown = std::ranges::any_of(listed, [&](const MonsterTreeModel::Row& row)
                                           { return MonsterTreeModel::ShowsSelection(row, selectedNumber, whole); });
    if (!shown)
    {
        m_filter = {};
        m_search.clear();
    }
    return true;
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
