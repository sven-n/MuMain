#include "stdafx.h"

#include "doctest.h"

#include "Core/Globals/_define.h"
#include "Data/GameData/MonsterData/LegacyMonsterKind.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"

#include <array>
#include <string>
#include <vector>

using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterKind;

namespace
{
MonsterDefinition Monster(int number, const char* english, MonsterKind kind = MonsterKind::Monster)
{
    MonsterDefinition monster;
    monster.number = number;
    monster.names.Set("en", english);
    monster.kind = kind;
    return monster;
}

MonsterDefinition BudgeDragon()
{
    MonsterDefinition monster = Monster(2, "Budge Dragon");
    monster.names.Set("pt", "Dragão Lento");
    return monster;
}
} // namespace

TEST_CASE("Monster table: finds the monsters it has, nothing else [data][monsters]")
{
    const std::array monsters = {BudgeDragon(), Monster(251, "Hanzo", MonsterKind::Npc)};
    g_MonsterDatabase.Build(monsters, "en");

    REQUIRE(g_MonsterDatabase.Find(2) != nullptr);
    CHECK(g_MonsterDatabase.Find(2)->names.GetNeutral() == "Budge Dragon");
    CHECK(g_MonsterDatabase.Find(251)->kind == MonsterKind::Npc);
    CHECK(g_MonsterDatabase.Find(3) == nullptr);
    CHECK(g_MonsterDatabase.Find(-1) == nullptr);
    CHECK(g_MonsterDatabase.Find(1024) == nullptr);
    CHECK(g_MonsterDatabase.Find(65535) == nullptr);
    CHECK(g_MonsterDatabase.FindDisplayName(3) == nullptr);
}

TEST_CASE("Monster table: numbers without an entry get the answers of the old client [data][monsters]")
{
    const std::array monsters = {BudgeDragon()};
    g_MonsterDatabase.Build(monsters, "en");

    CHECK(std::wstring(Data::Monsters::GetMonsterName(2)) == L"Budge Dragon");
    for (int number : {-1, 3, 451, 1024, 65535})
    {
        CAPTURE(number);
        CHECK(std::wstring(Data::Monsters::GetMonsterName(number)) == L"()");
        CHECK(Data::Monsters::GetMonsterObjectKind(number) == Data::Monsters::GetLegacyObjectKind(number));
    }
    CHECK(Data::Monsters::GetMonsterObjectKind(2) == KIND_MONSTER);
}

TEST_CASE("Monster table: the kind of an entry wins over the old rules [data][monsters]")
{
    // 566 is a monster by the old rules (P7 of the monsters design).
    const std::array monsters = {Monster(566, "Tercia", MonsterKind::Npc)};
    g_MonsterDatabase.Build(monsters, "en");

    CHECK(Data::Monsters::GetMonsterObjectKind(566) == KIND_NPC);
}

TEST_CASE("Monster table: names in the display language, English where one is missing [data][monsters]")
{
    const std::array monsters = {BudgeDragon(), Monster(251, "Hanzo")};
    g_MonsterDatabase.Build(monsters, "pt");

    CHECK(std::wstring(g_MonsterDatabase.FindDisplayName(2)) == L"Dragão Lento");
    CHECK(std::wstring(g_MonsterDatabase.FindDisplayName(251)) == L"Hanzo");
    CHECK(std::wstring(g_MonsterDatabase.FindEnglishName(2)) == L"Budge Dragon");
    CHECK(g_MonsterDatabase.GetDisplayLocale() == "pt");
}

TEST_CASE("Monster table: a language switch keeps the names where they are [data][monsters]")
{
    const std::array monsters = {BudgeDragon()};
    g_MonsterDatabase.Build(monsters, "en");
    const wchar_t* name = g_MonsterDatabase.FindDisplayName(2);
    const int version = g_MonsterDatabase.GetVersion();

    g_MonsterDatabase.SetDisplayLocale("pt");

    CHECK(g_MonsterDatabase.FindDisplayName(2) == name);
    CHECK(std::wstring(name) == L"Dragão Lento");
    // The monsters did not change, so the editor's checks stay current.
    CHECK(g_MonsterDatabase.GetVersion() == version);
}

TEST_CASE("Monster table: a rebuild drops the monsters that are gone [data][monsters]")
{
    const std::array before = {BudgeDragon(), Monster(251, "Hanzo")};
    g_MonsterDatabase.Build(before, "en");
    const std::array after = {Monster(251, "Hanzo the Blacksmith")};

    g_MonsterDatabase.Build(after, "en");

    CHECK(g_MonsterDatabase.Find(2) == nullptr);
    CHECK(std::wstring(g_MonsterDatabase.FindDisplayName(251)) == L"Hanzo the Blacksmith");
}

TEST_CASE("Monster table: a long name is cut to what the game holds, an emoji counting as two [data][monsters]")
{
    // 63 characters and one emoji: 65 UTF-16 units; the emoji does not fit.
    const std::string name = std::string(MAX_MONSTER_NAME - 1, 'x') + "\xF0\x9F\x90\x89";
    const std::array monsters = {Monster(9, name.c_str()), Monster(10, std::string(80, 'y').c_str())};
    g_MonsterDatabase.Build(monsters, "en");

    CHECK(std::wstring(g_MonsterDatabase.FindDisplayName(9)) == std::wstring(MAX_MONSTER_NAME - 1, L'x'));
    CHECK(std::wstring(g_MonsterDatabase.FindDisplayName(10)) == std::wstring(MAX_MONSTER_NAME, L'y'));
}

TEST_CASE("Monster table: an empty name is an entry with an empty name [data][monsters]")
{
    const std::array monsters = {Monster(252, "", MonsterKind::Npc)};
    g_MonsterDatabase.Build(monsters, "en");

    REQUIRE(g_MonsterDatabase.Find(252) != nullptr);
    CHECK(std::wstring(Data::Monsters::GetMonsterName(252)).empty());
}
