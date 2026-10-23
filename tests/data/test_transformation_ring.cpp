#include "stdafx.h"

#include "doctest.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "GameLogic/Items/TransformationRing.h"

#include <string>
#include <vector>

using Data::Monsters::MonsterDefinition;

namespace
{
MonsterDefinition Monster(int number, const char* english)
{
    MonsterDefinition monster;
    monster.number = number;
    monster.names.Set("en", english);
    return monster;
}
} // namespace

TEST_CASE("Transformation ring: each level names its form from the monster list [data][monsters]")
{
    const std::vector<MonsterDefinition> monsters = {Monster(2, "Budge Dragon"),     Monster(7, "Giant"),
                                                     Monster(14, "Skeleton Warrior"), Monster(8, "Poison Bull"),
                                                     Monster(9, "Thunder Lich"),     Monster(41, "Death Cow")};
    g_MonsterDatabase.Build(monsters, "en");

    using GameLogic::Items::TransformationRing::MonsterName;
    CHECK(std::wstring(MonsterName(0)) == L"Budge Dragon");
    CHECK(std::wstring(MonsterName(1)) == L"Giant");
    CHECK(std::wstring(MonsterName(2)) == L"Skeleton Warrior");
    CHECK(std::wstring(MonsterName(3)) == L"Poison Bull");
    CHECK(std::wstring(MonsterName(4)) == L"Thunder Lich");
    CHECK(std::wstring(MonsterName(5)) == L"Death Cow");
    CHECK(MonsterName(6) == nullptr);
    CHECK(MonsterName(-1) == nullptr);
}

TEST_CASE("Transformation ring: the monster's name follows Translate item names [data][monsters]")
{
    MonsterDefinition dragon = Monster(2, "Budge Dragon");
    dragon.names.Set("pt", "Dragão Lento");
    const std::vector<MonsterDefinition> monsters = {dragon};
    g_MonsterDatabase.Build(monsters, "pt");

    using GameLogic::Items::TransformationRing::MonsterName;
    CHECK(std::wstring(MonsterName(0)) == L"Dragão Lento");
    g_ItemDataHandler.SetTranslateItemNames(false);
    CHECK(std::wstring(MonsterName(0)) == L"Budge Dragon");
    g_ItemDataHandler.SetTranslateItemNames(true);
}

TEST_CASE("Transformation ring: a form the monster list has no entry for has no name [data][monsters]")
{
    const std::vector<MonsterDefinition> monsters = {Monster(2, "Budge Dragon")};
    g_MonsterDatabase.Build(monsters, "en");

    CHECK(GameLogic::Items::TransformationRing::MonsterName(1) == nullptr);
}
