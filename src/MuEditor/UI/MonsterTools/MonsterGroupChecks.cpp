#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterGroupChecks.h"

#include "MonsterEdits.h"

#include "../MuEditor/UI/Common/MapPicker.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/GameData/MapData/MapRef.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"

#include <algorithm>
#include <optional>
#include <string>

namespace MuEditor::Monsters::GroupChecks
{
namespace
{
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Maps::MapRef;
using Data::Monsters::MonsterAttacks;
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterMapVariant;

// Only the generic pattern: what the P26 numbers have as their own attacks.
bool IsGenericOnly(const std::optional<MonsterAttacks>& attacks)
{
    return attacks && attacks->otherwiseGeneric && attacks->skills.empty() && attacks->monsterSkills.empty();
}

// The maps of the variant that none of `maps` names.
std::vector<MapRef> MapsElsewhere(const MonsterMapVariant& variant, const std::vector<MapRef>& maps)
{
    std::vector<MapRef> elsewhere;
    for (const MapRef& map : variant.maps)
    {
        if (std::ranges::none_of(maps, [&](const MapRef& on) { return Data::Maps::Overlaps(map, on); }))
        {
            elsewhere.push_back(map);
        }
    }
    return elsewhere;
}

void CheckVariants(int number, const std::vector<MonsterMapVariant>& variants, const std::vector<MapRef>& maps,
                   const std::string& where, const std::string& source, std::vector<ItemDataIssue>& issues)
{
    if (maps.empty())
    {
        return;
    }
    for (size_t index = 0; index < variants.size(); ++index)
    {
        const std::vector<MapRef> elsewhere = MapsElsewhere(variants[index], maps);
        if (elsewhere.empty())
        {
            continue;
        }
        issues.push_back({ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, number,
                          "monster " + std::to_string(number) + " mapVariants[" + std::to_string(index) + "].attacks",
                          "the map variant plays on " + MapPicker::Describe(elsewhere, "") + ", where " + where +
                              " does not appear (" + MapPicker::Describe(maps, "") + ")"});
    }
}
} // namespace

void Validate(std::vector<ItemDataIssue>& issues)
{
    const std::string source = Data::Monsters::GetMonstersFile().generic_string();
    for (const std::vector<int>& numbers : g_MonsterEdits.ReadMonsters())
    {
        const MonsterDefinition shared = *g_MonsterEdits.ReadSharedValues(numbers.front());
        const std::string name = shared.names.GetNeutral();
        // The shared variants, once for the monster: on the maps of the
        // numbers that play them, named by the first of those, so that Data
        // Checks opens the monster as a whole there.
        std::vector<int> sharing = numbers;
        std::erase_if(sharing,
                      [&](int member) { return g_MonsterDatabase.Find(member)->mapVariants != shared.mapVariants; });
        if (!sharing.empty())
        {
            CheckVariants(sharing.front(), shared.mapVariants, g_MonsterEdits.ReadMaps(sharing),
                          numbers.size() > 1 ? "no number of " + name : "the monster", source, issues);
        }
        if (numbers.size() < 2)
        {
            continue;
        }
        for (const int member : numbers)
        {
            const MonsterDefinition& entry = *g_MonsterDatabase.Find(member);
            if (entry.mapVariants != shared.mapVariants)
            {
                CheckVariants(member, entry.mapVariants, entry.maps, "the number", source, issues);
            }
            if (entry.attacks != shared.attacks && shared.attacks && IsGenericOnly(entry.attacks))
            {
                issues.push_back({ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, member,
                                  "monster " + std::to_string(member) + " attacks",
                                  "has the generic pattern as its own attacks, while " + name +
                                      " has attacks: it attacks as in the original game (P26)"});
            }
        }
    }
}
} // namespace MuEditor::Monsters::GroupChecks

#endif // _EDITOR
