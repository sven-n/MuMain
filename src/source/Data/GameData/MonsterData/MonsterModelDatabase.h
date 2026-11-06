#pragma once

#include "Data/GameData/MonsterData/MonsterModelDefinition.h"

#include <optional>
#include <span>
#include <vector>

namespace Data::Monsters
{
// The model list in memory: one entry per model slot of the monster and NPC
// ranges, found with one array read. Built once at startup; the monster
// editor changes it afterwards.
class MonsterModelDatabase
{
public:
    static MonsterModelDatabase& GetInstance();

    // Replaces all models. Models whose slot is not a slot of the monster
    // area are skipped (the file's reader reports them).
    void Build(std::span<const MonsterModelDefinition> models);

    // The model of the slot; nullptr for a slot without an entry.
    const MonsterModelDefinition* Find(int slot) const;

    // The models, sorted by slot.
    std::vector<const MonsterModelDefinition*> GetAll() const;

#ifdef _EDITOR
    // The monster editor's changes of one slot.
    void Set(const MonsterModelDefinition& model);
    void Remove(int slot);
#endif

    // Counts the changes (Build, and the editor's Set and Remove).
    int GetVersion() const
    {
        return m_version;
    }

private:
    static std::optional<size_t> IndexOf(int slot);

    std::vector<std::optional<MonsterModelDefinition>> m_models;
    int m_version = 0;
};
} // namespace Data::Monsters

#define g_MonsterModelDatabase Data::Monsters::MonsterModelDatabase::GetInstance()
