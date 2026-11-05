#include "stdafx.h"

#include "doctest.h"

#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"
#include "Data/GameData/MonsterData/MonsterGroup.h"

#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <tuple>

// The one-time comparison of PR 3a's grouping (monsters design, D61, Q-MN52):
// the grouped monster list gives each number the entry it had in the flat list
// before (monsters_before_grouping.json, the list of main before PR 3a), in
// every field and language. The next commit removes it with that file.

namespace
{
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterGroup;

const std::filesystem::path ClientData = MU_TEST_DATA_DIR;
const std::filesystem::path Before = MU_TEST_MONSTERS_BEFORE_GROUPING;

// The numbers without attack data in a monster whose shared value has it: they
// play the generic pattern as their own value now (P26, Q-MN62, Q-MN69).
const std::set<int> GenericPatternNumbers = {331, 332, 335, 336, 337, 338, 434, 435, 436, 437, 438, 439, 531};

Data::Monsters::MonsterAttacks GenericPattern()
{
    Data::Monsters::MonsterAttacks attacks;
    attacks.otherwiseGeneric = true;
    return attacks;
}
} // namespace

TEST_CASE("Monster grouping: every number keeps its entry of the flat list (one-time, Q-MN52) [data][monsters]")
{
    const Data::Monsters::MonsterDataLoadResult before = Data::Monsters::LoadMonstersFile(Before);
    const Data::Monsters::MonsterDataLoadResult after =
        Data::Monsters::LoadMonstersFile(ClientData / "Monsters" / "Monsters.json");
    REQUIRE_FALSE(Data::Items::HasFatal(before.issues));
    REQUIRE_FALSE(Data::Items::HasFatal(after.issues));

    std::map<int, const MonsterDefinition*> afterByNumber;
    for (const MonsterDefinition& entry : after.monsters)
    {
        afterByNumber.emplace(entry.number, &entry);
    }
    CHECK(before.monsters.size() == 547);
    CHECK(afterByNumber.size() == before.monsters.size());
    int generic = 0;
    for (const MonsterDefinition& old : before.monsters)
    {
        INFO("number " << old.number);
        const auto found = afterByNumber.find(old.number);
        REQUIRE(found != afterByNumber.end());
        MonsterDefinition expected = old;
        if (GenericPatternNumbers.contains(old.number))
        {
            CHECK_FALSE(old.attacks.has_value());
            expected.attacks = GenericPattern();
            ++generic;
        }
        CHECK(*found->second == expected);
    }
    CHECK(generic == static_cast<int>(GenericPatternNumbers.size()));
}

TEST_CASE("Monster grouping: one monster for each English name, kind and model (one-time, Q-MN49) [data][monsters]")
{
    const Data::Monsters::MonsterDataLoadResult after =
        Data::Monsters::LoadMonstersFile(ClientData / "Monsters" / "Monsters.json");

    std::set<std::tuple<std::string, Data::Monsters::MonsterKind, std::string>> keys;
    int several = 0;
    int numbersInThem = 0;
    for (const MonsterGroup& monster : after.groups)
    {
        const MonsterDefinition& shared = Data::Monsters::GetSharedValues(monster);
        const auto key = std::make_tuple(shared.names.GetNeutral(), shared.kind, shared.model);
        INFO(shared.names.GetNeutral());
        CHECK(keys.insert(key).second);
        for (const MonsterDefinition& entry : monster.numbers)
        {
            CHECK(std::make_tuple(entry.names.GetNeutral(), entry.kind, entry.model) == key);
        }
        if (monster.numbers.size() > 1)
        {
            ++several;
            numbersInThem += static_cast<int>(monster.numbers.size());
        }
    }
    CHECK(after.groups.size() == 423);
    CHECK(several == 36);
    CHECK(numbersInThem == 160);
}
