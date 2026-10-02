#include "GuardGuildLists.h"

#include <algorithm>
#include <utility>

namespace UI::Combat
{
void GuardGuildLists::AddDeclaration(Declaration entry)
{
    if (entry.name.empty())
        return;
    if (m_Declarations.size() == kMaxGuilds)
        m_Declarations.erase(m_Declarations.begin());
    m_Declarations.push_back(std::move(entry));
    m_SelectedDeclaration = m_Declarations.front().name;
    ++m_Revision;
}

void GuardGuildLists::SortDeclarations()
{
    std::stable_sort(m_Declarations.begin(), m_Declarations.end(),
                    [](const Declaration& a, const Declaration& b) { return a.markCount > b.markCount; });
    m_SelectedDeclaration = m_Declarations.empty() ? L"" : m_Declarations.front().name;
    ++m_Revision;
}

void GuardGuildLists::ClearDeclarations()
{
    m_Declarations.clear();
    m_SelectedDeclaration.clear();
    ++m_Revision;
}

void GuardGuildLists::AddSiegeGuild(SiegeGuild entry)
{
    if (entry.name.empty())
        return;
    if (m_SiegeGuilds.size() == kMaxGuilds)
        m_SiegeGuilds.erase(m_SiegeGuilds.begin());
    m_SiegeGuilds.push_back(std::move(entry));
    m_SelectedSiegeGuild = m_SiegeGuilds.front().name;
    m_ShowScore = false;
    ++m_Revision;
}

void GuardGuildLists::ClearSiegeGuilds()
{
    m_SiegeGuilds.clear();
    m_SelectedSiegeGuild.clear();
    m_ShowScore = false;
    ++m_Revision;
}

bool GuardGuildLists::SelectDeclaration(std::wstring_view name)
{
    const auto it = std::find_if(m_Declarations.begin(), m_Declarations.end(),
                                 [name](const Declaration& entry) { return entry.name == name; });
    if (it == m_Declarations.end())
        return false;
    m_SelectedDeclaration = name;
    ++m_Revision;
    return true;
}

bool GuardGuildLists::SelectSiegeGuild(std::wstring_view name)
{
    const auto it = std::find_if(m_SiegeGuilds.begin(), m_SiegeGuilds.end(),
                                 [name](const SiegeGuild& entry) { return entry.name == name; });
    if (it == m_SiegeGuilds.end())
        return false;
    m_SelectedSiegeGuild = name;
    m_ShowScore = true;
    ++m_Revision;
    return true;
}

const GuardGuildLists::SiegeGuild* GuardGuildLists::ScoreGuild() const
{
    if (!m_ShowScore)
        return nullptr;
    const auto it = std::find_if(m_SiegeGuilds.begin(), m_SiegeGuilds.end(),
        [this](const SiegeGuild& entry) { return entry.name == m_SelectedSiegeGuild; });
    return it == m_SiegeGuilds.end() ? nullptr : &*it;
}

bool GuardGuildLists::IsOwnDeclaration(const Declaration& entry, std::wstring_view guild, std::wstring_view alliance)
{
    return entry.name == guild || entry.name == alliance;
}
}
