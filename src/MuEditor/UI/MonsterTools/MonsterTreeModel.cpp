#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterTreeModel.h"

#include "MonsterEdits.h"

#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"

#include <bitset>
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
    std::bitset<Data::Monsters::MonsterNumberCount> listed;
    for (int number = 0; number < Data::Monsters::MonsterNumberCount; ++number)
    {
        if (listed[static_cast<std::size_t>(number)] || g_MonsterDatabase.Find(number) == nullptr)
        {
            continue;
        }
        // A monster comes at its first number.
        const std::vector<int> numbers = g_MonsterEdits.ReadMonsterNumbers(number);
        std::vector<int> passing;
        for (const int member : numbers)
        {
            listed.set(static_cast<std::size_t>(member));
            if (MatchesMonsterFilter(*g_MonsterDatabase.Find(member), filter))
            {
                passing.push_back(member);
            }
        }
        if (passing.empty())
        {
            continue;
        }
        const int count = static_cast<int>(numbers.size());
        if (count == 1)
        {
            m_rows.push_back({number, Row::Kind::Single, 1, false});
            continue;
        }
        const bool open = filtered || IsOpen(number);
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
