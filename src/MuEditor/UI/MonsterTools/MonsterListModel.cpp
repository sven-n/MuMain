#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterListModel.h"

#include "../MuEditor/UI/Common/TextSearch.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"

#include <algorithm>

namespace MuEditor::Monsters
{
namespace
{
using Data::Monsters::MonsterDefinition;

bool Lists(const MonsterDefinition& monster, const MonsterListModel::Filter& filter)
{
    if (!MatchesMonsterSearch(monster, filter.search))
    {
        return false;
    }
    if (!filter.missingLocale.empty() && monster.names.HasOwn(filter.missingLocale))
    {
        return false;
    }
    return !filter.kind || monster.kind == *filter.kind;
}
} // namespace

bool MatchesMonsterSearch(const MonsterDefinition& monster, std::string_view search)
{
    int number = 0;
    if (search.empty() || (TextSearch::ReadWholeNumber(search, number) && number == monster.number))
    {
        return true;
    }
    return TextSearch::AnyNameContains(monster.names, search);
}

bool IsFreeMonsterNumber(int number)
{
    return number >= 0 && number < Data::Monsters::MonsterNumberCount && g_MonsterDatabase.Find(number) == nullptr;
}

std::vector<int> GetFreeMonsterNumbers()
{
    std::vector<int> numbers;
    for (int number = 0; number < Data::Monsters::MonsterNumberCount; ++number)
    {
        if (IsFreeMonsterNumber(number))
        {
            numbers.push_back(number);
        }
    }
    return numbers;
}

int GetNextFreeMonsterNumber(int from)
{
    const std::vector<int> free = GetFreeMonsterNumbers();
    const auto next = std::ranges::find_if(free, [from](int number) { return number >= from; });
    return next != free.end() ? *next : (free.empty() ? 0 : free.front());
}

const std::vector<int>& MonsterListModel::GetNumbers(const Filter& filter)
{
    if (m_built && filter == m_filter && m_version == g_MonsterDatabase.GetVersion())
    {
        return m_numbers;
    }

    m_numbers.clear();
    for (int number = 0; number < Data::Monsters::MonsterNumberCount; ++number)
    {
        const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
        if (monster != nullptr && Lists(*monster, filter))
        {
            m_numbers.push_back(number);
        }
    }
    m_filter = filter;
    m_version = g_MonsterDatabase.GetVersion();
    m_built = true;
    return m_numbers;
}

void MonsterListModel::KeepRowsAfterOwnEdit()
{
    m_version = g_MonsterDatabase.GetVersion();
}

void MonsterListModel::Refresh()
{
    m_built = false;
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
