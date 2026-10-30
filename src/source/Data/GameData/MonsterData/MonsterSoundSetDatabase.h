#pragma once

#include "Data/GameData/MonsterData/MonsterSoundSet.h"

#include <map>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Data::Monsters
{
// The sound sets in memory (Data/Monsters/Sounds.json, Q-MN78), found by
// name. Built once at startup; the monster editor changes it afterwards.
class MonsterSoundSetDatabase
{
public:
    static MonsterSoundSetDatabase& GetInstance();

    // Replaces all sets.
    void Build(std::span<const MonsterSoundSet> sets);

    // The set of the name; nullptr when there is none.
    const MonsterSoundSet* Find(std::string_view name) const;

    // The sets, sorted by name.
    std::vector<const MonsterSoundSet*> GetAll() const;

#ifdef _EDITOR
    // The monster editor's changes of one set, by its name.
    void Set(const MonsterSoundSet& set);
    void Remove(std::string_view name);
#endif

    // Counts the changes (Build, and the editor's Set and Remove).
    int GetVersion() const
    {
        return m_version;
    }

private:
    std::map<std::string, MonsterSoundSet, std::less<>> m_sets;
    int m_version = 0;
};
} // namespace Data::Monsters

#define g_MonsterSoundSetDatabase Data::Monsters::MonsterSoundSetDatabase::GetInstance()
