#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterTreeModel.h"

#include "MonsterEdits.h"

#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"

#include <algorithm>
#include <iterator>
#include <optional>

namespace MuEditor::Monsters
{
namespace
{
bool IsFiltered(const MonsterListModel::Filter& filter)
{
    return !filter.search.empty() || !filter.missingLocale.empty() || filter.kind.has_value();
}
} // namespace

const std::vector<MonsterTreeModel::Row>& MonsterTreeModel::GetRows(const MonsterListModel::Filter& filter)
{
    if (m_built && filter == m_filter && m_version == g_MonsterDatabase.GetVersion() &&
        m_groupsVersion == g_MonsterDataHandler.GetGroupsVersion())
    {
        return m_rows;
    }

    m_rows.clear();
    const bool filtered = IsFiltered(filter);
    for (const std::vector<int>& numbers : g_MonsterEdits.ReadMonsters())
    {
        std::vector<int> passing;
        std::ranges::copy_if(numbers, std::back_inserter(passing),
                             [&](int member) { return MatchesMonsterFilter(*g_MonsterDatabase.Find(member), filter); });
        if (passing.empty())
        {
            continue;
        }
        const int count = static_cast<int>(numbers.size());
        if (count == 1)
        {
            m_rows.push_back({numbers.front(), Row::Kind::Single, 1, false});
            continue;
        }
        const bool open = filtered || IsOpen(numbers.front());
        m_rows.push_back({numbers.front(), Row::Kind::Monster, count, open});
        if (open)
        {
            for (const int member : filtered ? passing : numbers)
            {
                m_rows.push_back({member, Row::Kind::Number, count, false});
            }
        }
    }
    m_filter = filter;
    m_version = g_MonsterDatabase.GetVersion();
    m_groupsVersion = g_MonsterDataHandler.GetGroupsVersion();
    m_built = true;
    return m_rows;
}

bool MonsterTreeModel::ShowsSelection(const Row& row, int number, bool wholeMonster)
{
    switch (row.kind)
    {
    case Row::Kind::Single:
        return row.number == number;
    case Row::Kind::Number:
        return !wholeMonster && row.number == number;
    case Row::Kind::Monster:
    {
        const std::optional<int> monster = g_MonsterDataHandler.FindGroupOf(number);
        return wholeMonster && monster && monster == g_MonsterDataHandler.FindGroupOf(row.number);
    }
    }
    return false;
}

void MonsterTreeModel::SetOpen(int number, bool open)
{
    // Only a monster with several numbers opens.
    const std::optional<int> monster = g_MonsterDataHandler.FindGroupOf(number);
    if (!monster)
    {
        return;
    }
    if (open)
    {
        m_open.insert(*monster);
    }
    else
    {
        m_open.erase(*monster);
    }
    m_built = false;
}

bool MonsterTreeModel::IsOpen(int number) const
{
    const std::optional<int> monster = g_MonsterDataHandler.FindGroupOf(number);
    return monster && m_open.contains(*monster);
}

void MonsterTreeModel::KeepRowsAfterOwnEdit()
{
    m_version = g_MonsterDatabase.GetVersion();
    m_groupsVersion = g_MonsterDataHandler.GetGroupsVersion();
}

void MonsterTreeModel::Refresh()
{
    m_built = false;
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
