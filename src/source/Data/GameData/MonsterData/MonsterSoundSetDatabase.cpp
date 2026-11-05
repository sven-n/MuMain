#include "stdafx.h"

#include "MonsterSoundSetDatabase.h"

namespace Data::Monsters
{
MonsterSoundSetDatabase& MonsterSoundSetDatabase::GetInstance()
{
    static MonsterSoundSetDatabase database;
    return database;
}

void MonsterSoundSetDatabase::Build(std::span<const MonsterSoundSet> sets)
{
    m_sets.clear();
    for (const MonsterSoundSet& set : sets)
    {
        m_sets.insert_or_assign(set.name, set);
    }
    ++m_version;
}

const MonsterSoundSet* MonsterSoundSetDatabase::Find(std::string_view name) const
{
    const auto found = m_sets.find(name);
    return found != m_sets.end() ? &found->second : nullptr;
}

std::vector<const MonsterSoundSet*> MonsterSoundSetDatabase::GetAll() const
{
    std::vector<const MonsterSoundSet*> sets;
    sets.reserve(m_sets.size());
    for (const auto& [name, set] : m_sets)
    {
        sets.push_back(&set);
    }
    return sets;
}

#ifdef _EDITOR
void MonsterSoundSetDatabase::Set(const MonsterSoundSet& set)
{
    m_sets.insert_or_assign(set.name, set);
    ++m_version;
}

void MonsterSoundSetDatabase::Remove(std::string_view name)
{
    if (const auto found = m_sets.find(name); found != m_sets.end())
    {
        m_sets.erase(found);
        ++m_version;
    }
}
#endif
} // namespace Data::Monsters
