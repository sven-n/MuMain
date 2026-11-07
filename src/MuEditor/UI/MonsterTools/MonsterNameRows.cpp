#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterNameRows.h"

#include "MonsterEdits.h"
#include "MonsterOwnValues.h"
#include "MonsterSelection.h"
#include "MonsterShownValues.h"
#include "MonsterStepLabel.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/Common/TextSearch.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <cstdio>
#include <optional>

namespace MuEditor::Monsters
{
namespace
{
using Data::Monsters::MonsterDefinition;
using ShownValues::WholeMonsterKey;

int NumberOf(int id)
{
    return id >= WholeMonsterKey ? id - WholeMonsterKey : id;
}

// A number of a monster with several numbers has a row of its own when its
// names are its own (Q-MN50) or were made its own to be edited here (Q-MN96);
// else they are in its monster's row.
bool HasOwnRow(int number)
{
    return OwnValues::GetNumberState(number, Data::Monsters::MonsterValueField::Names) != OwnValues::State::Shared;
}
} // namespace

const std::vector<int>& MonsterNameRows::List(std::string_view search, std::string_view missingLocale)
{
    m_filter.search.assign(search);
    m_filter.missingLocale.assign(missingLocale);
    if (m_built && m_filter == m_builtFilter && m_builtVersion == GetVersion())
    {
        return m_rows;
    }
    m_rows.clear();
    for (const Row& row : CollectRows())
    {
        if (Lists(row))
        {
            m_rows.push_back(row.id);
        }
    }
    m_builtFilter = m_filter;
    m_builtVersion = GetVersion();
    m_built = true;
    return m_rows;
}

bool MonsterNameRows::Lists(const Row& row) const
{
    if (MatchesMonsterFilter(row.values, m_filter))
    {
        return true;
    }
    // A monster's row is found by any of its numbers too.
    int number = 0;
    if (!TextSearch::ReadWholeNumber(m_filter.search, number) ||
        std::ranges::find(row.numbers, number) == row.numbers.end())
    {
        return false;
    }
    MonsterListModel::Filter withoutSearch = m_filter;
    withoutSearch.search.clear();
    return MatchesMonsterFilter(row.values, withoutSearch);
}

std::vector<MonsterNameRows::Row> MonsterNameRows::CollectRows() const
{
    std::vector<Row> rows;
    for (const std::vector<int>& numbers : g_MonsterEdits.ReadMonsters())
    {
        const int number = numbers.front();
        if (numbers.size() < 2)
        {
            rows.push_back({number, *g_MonsterDatabase.Find(number), {number}});
            continue;
        }
        // The names the numbers share, then the numbers with names of their
        // own (Q-MN50).
        MonsterDefinition shared = *g_MonsterEdits.FindSharedValues(number);
        shared.number = numbers.front();
        rows.push_back({numbers.front() + WholeMonsterKey, shared, numbers});
        for (const int member : numbers)
        {
            if (HasOwnRow(member))
            {
                rows.push_back({member, *g_MonsterDatabase.Find(member), {member}});
            }
        }
    }
    return rows;
}

void MonsterNameRows::Refresh()
{
    m_built = false;
}

int MonsterNameRows::GetVersion() const
{
    return g_MonsterDatabase.GetVersion() + g_MonsterDataHandler.GetGroupsVersion() + OwnValues::GetUnlockVersion();
}

std::vector<int> MonsterNameRows::CountMissing(std::span<const char* const> locales) const
{
    std::vector<int> missing(locales.size(), 0);
    for (const Row& row : CollectRows())
    {
        for (size_t index = 0; index < locales.size(); ++index)
        {
            missing[index] += row.values.names.HasOwn(locales[index]) ? 0 : 1;
        }
    }
    return missing;
}

const Data::LocalizedString* MonsterNameRows::FindNames(int id) const
{
    if (id < WholeMonsterKey)
    {
        const MonsterDefinition* monster = g_MonsterDatabase.Find(id);
        return monster != nullptr ? &monster->names : nullptr;
    }
    // Read each frame for each row shown: no copies. A monster's row whose
    // monster has one number left shows that number's names.
    const MonsterDefinition* shared = g_MonsterEdits.FindSharedValues(NumberOf(id));
    const MonsterDefinition* values = shared != nullptr ? shared : g_MonsterDatabase.Find(NumberOf(id));
    if (values == nullptr)
    {
        return nullptr;
    }
    // The grid reads the names where they are: the shared ones are kept
    // here, in place, so a pointer given before stays valid.
    Data::LocalizedString& kept = m_sharedNames[id];
    if (kept != values->names)
    {
        kept = values->names;
    }
    return &kept;
}

void MonsterNameRows::FormatId(int id, char* buffer, std::size_t size) const
{
    if (id >= WholeMonsterKey)
    {
        // "145 (7)": the monster's first number and how many it has.
        std::snprintf(buffer, size, "%d (%d)", NumberOf(id), g_MonsterEdits.CountMonsterNumbers(NumberOf(id)));
        return;
    }
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
    const std::optional<ShownValues::ShownMonster> shown = ShownValues::Load(id);
    if (!shown)
    {
        return;
    }
    const std::string label = I18N::Format(I18N::Editor::StepMonsterName,
                                           {ShownValues::LogName(*shown), I18N::GetLanguageDisplayName(locale)});
    // Through the monster tools' history, so the name can be undone like any
    // edit. The number has an entry and the grid's languages are language
    // codes: only an emptied English name is refused. An English name that is
    // empty already (252) stays so; Data Checks warns about it.
    bool stored = false;
    if (shown->wholeMonster)
    {
        ShownValues::ShownMonster changed = *shown;
        changed.monster.names.Set(locale, text);
        const bool emptiesEnglish = std::string_view(locale) == Data::LocalizedString::NeutralLocale &&
                                    changed.monster.names.GetNeutral().empty();
        stored = !emptiesEnglish && ShownValues::Store(label, changed);
    }
    else
    {
        // A monster's row whose monster has one number left names that number.
        stored = g_MonsterEdits.SetMonsterName(label, NumberOf(id), locale, text);
    }
    if (!stored)
    {
        g_MuEditorConsoleUI.LogEditor(I18N::Editor::TheEnglishNameCannotBeEmpty);
        return;
    }
    // The rows stay until the filter changes.
    m_builtVersion = GetVersion();
}

int MonsterNameRows::GetSelected() const
{
    // Read each frame: no copies.
    const int number = g_MonsterSelection.GetNumber();
    if (g_MonsterEdits.CountMonsterNumbers(number) < 2)
    {
        return number;
    }
    // A number that shares its monster's names is shown in the monster's row.
    if (g_MonsterSelection.IsWholeMonster() || !HasOwnRow(number))
    {
        return g_MonsterEdits.FindFirstNumber(number) + WholeMonsterKey;
    }
    return number;
}

int MonsterNameRows::GetSelectionSerial() const
{
    return g_MonsterSelection.GetSerial();
}

void MonsterNameRows::Select(int id)
{
    if (id >= WholeMonsterKey)
    {
        g_MonsterSelection.SelectMonster(NumberOf(id));
    }
    else
    {
        g_MonsterSelection.Select(id);
    }
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
