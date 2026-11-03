#include "stdafx.h"

#include "MonsterModelDatabase.h"

#include "Core/Globals/_enum.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"

namespace Data::Monsters
{
namespace
{
// The monster range and the NPC range follow each other without a gap.
constexpr int FirstSlot = MODEL_MONSTER01;
constexpr int SlotCount = MODEL_NPC_END - MODEL_MONSTER01;
} // namespace

MonsterModelDatabase& MonsterModelDatabase::GetInstance()
{
    static MonsterModelDatabase database;
    return database;
}

std::optional<size_t> MonsterModelDatabase::IndexOf(int slot)
{
    if (slot < FirstSlot || slot >= FirstSlot + SlotCount)
    {
        return std::nullopt;
    }
    return static_cast<size_t>(slot - FirstSlot);
}

void MonsterModelDatabase::Build(std::span<const MonsterModelDefinition> models)
{
    m_models.assign(static_cast<size_t>(SlotCount), std::nullopt);
    for (const MonsterModelDefinition& model : models)
    {
        const std::optional<int> slot = FindMonsterModelSlot(model.slot);
        const std::optional<size_t> index = slot ? IndexOf(*slot) : std::nullopt;
        if (index)
        {
            m_models[*index] = model;
        }
    }
    ++m_version;
}

const MonsterModelDefinition* MonsterModelDatabase::Find(int slot) const
{
    const std::optional<size_t> index = IndexOf(slot);
    if (!index || *index >= m_models.size() || !m_models[*index])
    {
        return nullptr;
    }
    return &*m_models[*index];
}

std::vector<const MonsterModelDefinition*> MonsterModelDatabase::GetAll() const
{
    std::vector<const MonsterModelDefinition*> models;
    for (const std::optional<MonsterModelDefinition>& model : m_models)
    {
        if (model)
        {
            models.push_back(&*model);
        }
    }
    return models;
}

#ifdef _EDITOR
void MonsterModelDatabase::Set(const MonsterModelDefinition& model)
{
    const std::optional<int> slot = FindMonsterModelSlot(model.slot);
    const std::optional<size_t> index = slot ? IndexOf(*slot) : std::nullopt;
    if (!index)
    {
        return;
    }
    if (m_models.size() < static_cast<size_t>(SlotCount))
    {
        m_models.resize(static_cast<size_t>(SlotCount));
    }
    m_models[*index] = model;
    ++m_version;
}

void MonsterModelDatabase::Remove(int slot)
{
    const std::optional<size_t> index = IndexOf(slot);
    if (!index || *index >= m_models.size())
    {
        return;
    }
    m_models[*index].reset();
    ++m_version;
}
#endif
} // namespace Data::Monsters
