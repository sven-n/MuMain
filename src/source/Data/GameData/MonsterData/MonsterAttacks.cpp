#include "stdafx.h"

#include "MonsterAttacks.h"

#include <array>

namespace Data::Monsters
{
namespace
{
const MonsterAttackEntry* FindEntry(const std::vector<MonsterAttackEntry>& entries, int skill)
{
    for (const MonsterAttackEntry& entry : entries)
    {
        if (entry.first == skill)
        {
            return &entry;
        }
    }
    return nullptr;
}

constexpr std::array<std::pair<MonsterAttackRandom, std::string_view>, 3> RandomNames = {{
    {MonsterAttackRandom::Rand, "rand"},
    {MonsterAttackRandom::RandFps, "randFps"},
    {MonsterAttackRandom::Bali, "bali"},
}};

constexpr std::array<std::pair<MonsterSkillClear, std::string_view>, 5> ClearNames = {{
    {MonsterSkillClear::Never, "never"},
    {MonsterSkillClear::OnMatch, "onMatch"},
    {MonsterSkillClear::OnNoMatch, "onNoMatch"},
    {MonsterSkillClear::Always, "always"},
    {MonsterSkillClear::AfterAttack, "afterAttack"},
}};
} // namespace

MonsterAttackChoice ChooseMonsterAttack(const MonsterAttacks& attacks, int monsterSkill, int skill)
{
    const MonsterAttackEntry* entry = FindEntry(attacks.monsterSkills, monsterSkill);
    if (entry == nullptr)
    {
        entry = FindEntry(attacks.skills, skill);
    }
    MonsterAttackChoice choice;
    if (entry != nullptr)
    {
        choice.animation = entry->second;
    }
    else if (attacks.otherwiseRandom)
    {
        choice.random = attacks.otherwiseRandom;
    }
    else
    {
        choice.animation = attacks.otherwise;
    }
    switch (attacks.clearSkill)
    {
    case MonsterSkillClear::Never:
    case MonsterSkillClear::AfterAttack:
        choice.clearMonsterSkill = false;
        break;
    case MonsterSkillClear::OnMatch:
        choice.clearMonsterSkill = entry != nullptr;
        break;
    case MonsterSkillClear::OnNoMatch:
        choice.clearMonsterSkill = entry == nullptr;
        break;
    case MonsterSkillClear::Always:
        choice.clearMonsterSkill = true;
        break;
    }
    return choice;
}

bool ClearsMonsterSkillAfterAttack(const MonsterAttacks& attacks)
{
    return attacks.clearSkill == MonsterSkillClear::AfterAttack;
}

std::string_view GetMonsterAttackRandomName(MonsterAttackRandom random)
{
    for (const auto& [value, name] : RandomNames)
    {
        if (value == random)
        {
            return name;
        }
    }
    return {};
}

std::optional<MonsterAttackRandom> FindMonsterAttackRandom(std::string_view name)
{
    for (const auto& [value, known] : RandomNames)
    {
        if (known == name)
        {
            return value;
        }
    }
    return std::nullopt;
}

std::string_view GetMonsterSkillClearName(MonsterSkillClear clear)
{
    for (const auto& [value, name] : ClearNames)
    {
        if (value == clear)
        {
            return name;
        }
    }
    return {};
}

std::optional<MonsterSkillClear> FindMonsterSkillClear(std::string_view name)
{
    for (const auto& [value, known] : ClearNames)
    {
        if (known == name)
        {
            return value;
        }
    }
    return std::nullopt;
}
} // namespace Data::Monsters
