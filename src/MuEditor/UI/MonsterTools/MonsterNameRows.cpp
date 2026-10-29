#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterNameRows.h"

#include "MonsterEdits.h"
#include "MonsterSelection.h"
#include "MonsterStepLabel.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <cstdio>

namespace MuEditor::Monsters
{
const std::vector<int>& MonsterNameRows::List(std::string_view search, std::string_view missingLocale)
{
    m_filter.search.assign(search);
    m_filter.missingLocale.assign(missingLocale);
    return m_list.GetNumbers(m_filter);
}

void MonsterNameRows::Refresh()
{
    m_list.Refresh();
}

int MonsterNameRows::GetVersion() const
{
    return g_MonsterDatabase.GetVersion();
}

std::vector<int> MonsterNameRows::CountMissing(std::span<const char* const> locales) const
{
    std::vector<int> missing(locales.size(), 0);
    for (int number = 0; number < Data::Monsters::MonsterNumberCount; ++number)
    {
        const Data::Monsters::MonsterDefinition* monster = g_MonsterDatabase.Find(number);
        if (monster == nullptr)
        {
            continue;
        }
        for (size_t index = 0; index < locales.size(); ++index)
        {
            missing[index] += monster->names.HasOwn(locales[index]) ? 0 : 1;
        }
    }
    return missing;
}

const Data::LocalizedString* MonsterNameRows::FindNames(int id) const
{
    const Data::Monsters::MonsterDefinition* monster = g_MonsterDatabase.Find(id);
    return monster != nullptr ? &monster->names : nullptr;
}

void MonsterNameRows::FormatId(int id, char* buffer, std::size_t size) const
{
    std::snprintf(buffer, size, "%d", id);
}

void MonsterNameRows::CommitName(int id, const char* locale, const char* text)
{
    // A name with the separator would be stored, then refuse every save.
    if (Data::LocalizedString::ContainsSeparator(text))
    {
        g_MuEditorConsoleUI.LogEditor(I18N::Editor::MonsterNamesCannotContainTheSeparator);
        return;
    }
    const Data::Monsters::MonsterDefinition* monster = g_MonsterDatabase.Find(id);
    if (monster == nullptr)
    {
        return;
    }
    // Through the monster tools' history, so the name can be undone like any
    // edit. The number has an entry and the grid's languages are language
    // codes: only an emptied English name is refused. An English name that is
    // empty already (252) stays so; Data Checks warns about it.
    if (!g_MonsterEdits.SetMonsterName(StepLabel::Name(*monster, locale), id, locale, text))
    {
        g_MuEditorConsoleUI.LogEditor(I18N::Editor::TheEnglishNameCannotBeEmpty);
        return;
    }
    m_list.KeepRowsAfterOwnEdit();
}

int MonsterNameRows::GetSelected() const
{
    return g_MonsterSelection.GetNumber();
}

int MonsterNameRows::GetSelectionSerial() const
{
    return g_MonsterSelection.GetSerial();
}

void MonsterNameRows::Select(int id)
{
    g_MonsterSelection.Select(id);
}

void MonsterNameRows::ShowBrowser()
{
    g_MuEditorCore.GetToolWindows().monsterBrowser = true;
    ImGui::SetWindowFocus(WindowIds::MonsterBrowser);
}

const char* MonsterNameRows::GetSelectTooltip() const
{
    return I18N::Editor::SelectTheMonsterAndShowTheMonsterBrowser;
}

const char* MonsterNameRows::GetAllRowsLabel() const
{
    return I18N::Editor::AllMonsters;
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
