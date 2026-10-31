#include "stdafx.h"

#include "MonsterSoundSetLinks.h"

#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/MonsterSoundSetDatabase.h"

#include <algorithm>
#include <array>
#include <optional>
#include <unordered_map>

namespace Data::Monsters::SoundSetLinks
{
namespace
{
// A model a monster appears with, and the set it plays with it.
struct Look
{
    int slot = -1;
    const MonsterSoundSet* set = nullptr;
};

struct Links
{
    bool built = false;
    int monstersVersion = 0;
    int setsVersion = 0;
    std::array<std::vector<Look>, MonsterNumberCount> looks;
    // The monster numbers that appear with each model, in order.
    std::unordered_map<int, std::vector<int>> numbersOfSlot;
};

const MonsterSoundSet* SetOfLook(const MonsterDefinition& monster, int slot)
{
    const std::string& name = GetSoundSetWithModel(monster, slot);
    return name.empty() ? nullptr : g_MonsterSoundSetDatabase.Find(name);
}

void AddLook(Links& links, const MonsterDefinition& monster, int number, const std::string& model)
{
    const std::optional<int> slot = FindMonsterModelReference(model);
    std::vector<Look>& looks = links.looks[static_cast<size_t>(number)];
    if (!slot || std::any_of(looks.begin(), looks.end(), [&](const Look& look) { return look.slot == *slot; }))
    {
        return;
    }
    looks.push_back({*slot, SetOfLook(monster, *slot)});
    links.numbersOfSlot[*slot].push_back(number);
}

void Build(Links& links)
{
    for (std::vector<Look>& looks : links.looks)
    {
        looks.clear();
    }
    links.numbersOfSlot.clear();
    for (int number = 0; number < MonsterNumberCount; ++number)
    {
        const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
        if (monster == nullptr)
        {
            continue;
        }
        AddLook(links, *monster, number, monster->model);
        for (const MonsterRandomVariant& variant : monster->randomVariants)
        {
            AddLook(links, *monster, number, variant.model);
        }
    }
}

// The links of the tables as they are now.
const Links& Current()
{
    static Links links;
    const int monstersVersion = g_MonsterDatabase.GetVersion();
    const int setsVersion = g_MonsterSoundSetDatabase.GetVersion();
    if (!links.built || links.monstersVersion != monstersVersion || links.setsVersion != setsVersion)
    {
        Build(links);
        links.built = true;
        links.monstersVersion = monstersVersion;
        links.setsVersion = setsVersion;
    }
    return links;
}
} // namespace

const MonsterSoundSet* FindSet(int number, int slot)
{
    if (number < 0 || number >= MonsterNumberCount)
    {
        return nullptr;
    }
    for (const Look& look : Current().looks[static_cast<size_t>(number)])
    {
        if (look.slot == slot)
        {
            return look.set;
        }
    }
    return nullptr;
}

std::vector<const MonsterSoundSet*> FindSetsOfSlot(int slot)
{
    std::vector<const MonsterSoundSet*> sets;
    const Links& links = Current();
    const auto numbers = links.numbersOfSlot.find(slot);
    if (numbers == links.numbersOfSlot.end())
    {
        return sets;
    }
    for (const int number : numbers->second)
    {
        const MonsterSoundSet* set = FindSet(number, slot);
        if (set != nullptr && std::find(sets.begin(), sets.end(), set) == sets.end())
        {
            sets.push_back(set);
        }
    }
    return sets;
}

int CountMonstersWithModel(int slot)
{
    const Links& links = Current();
    const auto numbers = links.numbersOfSlot.find(slot);
    return numbers != links.numbersOfSlot.end() ? static_cast<int>(numbers->second.size()) : 0;
}
} // namespace Data::Monsters::SoundSetLinks
