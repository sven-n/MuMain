#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "ClientDataScope.h"
#include "TemporaryMonsterFolder.h"

#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"

#include <algorithm>
#include <array>
#include <optional>
#include <span>
#include <string>
#include <vector>

using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Monsters::MonsterDataHandler;
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterKind;
using Data::Monsters::MonsterSnapshot;

namespace
{
MonsterDefinition Monster(int number, const char* english, const char* portuguese = nullptr,
                          MonsterKind kind = MonsterKind::Monster)
{
    MonsterDefinition monster;
    monster.number = number;
    monster.names.Set("en", english);
    if (portuguese != nullptr)
    {
        monster.names.Set("pt", portuguese);
    }
    monster.kind = kind;
    return monster;
}

MonsterDefinition BudgeDragon()
{
    return Monster(2, "Budge Dragon", "Dragão Lento");
}

// The numbers the handler told about a name change, while a test listens.
std::vector<int> g_toldNumbers;

void RememberNamesChanged(int number)
{
    g_toldNumbers.push_back(number);
}

class NamesChangedListener
{
public:
    NamesChangedListener()
    {
        g_toldNumbers.clear();
        g_MonsterDataHandler.SetNamesChangedCallback(&RememberNamesChanged);
    }

    ~NamesChangedListener()
    {
        g_MonsterDataHandler.SetNamesChangedCallback(nullptr);
    }

    NamesChangedListener(const NamesChangedListener&) = delete;
    NamesChangedListener& operator=(const NamesChangedListener&) = delete;
};

const ItemDataIssue* FindIssue(const std::vector<ItemDataIssue>& issues, int number)
{
    const auto found = std::ranges::find(issues, number, &ItemDataIssue::number);
    return found != issues.end() ? &*found : nullptr;
}
} // namespace

TEST_CASE("Monster table: the editor sets and removes one number in place [data][monsters][editor]")
{
    const std::array monsters = {BudgeDragon()};
    g_MonsterDatabase.Build(monsters, "pt");
    const wchar_t* name = g_MonsterDatabase.FindDisplayName(2);
    int version = g_MonsterDatabase.GetVersion();

    g_MonsterDatabase.Set(Monster(2, "Budge Dragon", "Dragão Ligeiro"));
    CHECK(g_MonsterDatabase.FindDisplayName(2) == name);
    CHECK(std::wstring(name) == L"Dragão Ligeiro");
    CHECK(g_MonsterDatabase.GetVersion() != version);

    g_MonsterDatabase.Set(Monster(3, "Larva"));
    REQUIRE(g_MonsterDatabase.Find(3) != nullptr);
    CHECK(std::wstring(g_MonsterDatabase.FindDisplayName(3)) == L"Larva");
    CHECK(std::wstring(g_MonsterDatabase.FindEnglishName(3)) == L"Larva");

    version = g_MonsterDatabase.GetVersion();
    g_MonsterDatabase.Remove(2);
    CHECK(g_MonsterDatabase.Find(2) == nullptr);
    CHECK(g_MonsterDatabase.FindDisplayName(2) == nullptr);
    CHECK(std::wstring(Data::Monsters::GetMonsterName(2)) == L"()");
    CHECK(g_MonsterDatabase.GetVersion() != version);

    // Outside 0-1023 nothing changes.
    version = g_MonsterDatabase.GetVersion();
    g_MonsterDatabase.Set(Monster(1024, "Nobody"));
    g_MonsterDatabase.Set(Monster(-1, "Nobody"));
    g_MonsterDatabase.Remove(1024);
    CHECK(g_MonsterDatabase.GetVersion() == version);
}

TEST_CASE("Monster data: the editor reads monsters as snapshots and restores them exactly [data][monsters][editor]")
{
    const std::array monsters = {BudgeDragon(), Monster(252, "", nullptr, MonsterKind::Npc)};
    g_MonsterDatabase.Build(monsters, "en");
    NamesChangedListener listener;

    CHECK(g_MonsterDataHandler.ReadMonster(2) == BudgeDragon());
    CHECK(g_MonsterDataHandler.ReadMonster(252)->names.GetNeutral().empty());
    CHECK_FALSE(g_MonsterDataHandler.ReadMonster(3).has_value());
    CHECK_FALSE(g_MonsterDataHandler.ReadMonster(1024).has_value());

    // An entry with another number is stored at the snapshot's number.
    const std::array snapshots = {MonsterSnapshot{2, std::nullopt}, MonsterSnapshot{3, Monster(99, "Larva")},
                                  MonsterSnapshot{1024, Monster(1024, "Nobody")}};
    g_MonsterDataHandler.RestoreMonsters(snapshots);

    CHECK(g_MonsterDatabase.Find(2) == nullptr);
    REQUIRE(g_MonsterDatabase.Find(3) != nullptr);
    CHECK(g_MonsterDatabase.Find(3)->number == 3);
    CHECK(g_MonsterDatabase.Find(3)->names.GetNeutral() == "Larva");
    CHECK(g_MonsterDatabase.Find(99) == nullptr);
    // Living monsters of these numbers get their names at once (D55), told
    // once for both.
    CHECK(g_toldNumbers == std::vector<int>{MonsterDataHandler::AllNumbers});

    // One number, even restored twice, is told as itself; a number outside
    // 0-1023 is not told.
    g_toldNumbers.clear();
    const std::array oneNumber = {MonsterSnapshot{2, BudgeDragon()}, MonsterSnapshot{1024, std::nullopt},
                                  MonsterSnapshot{2, std::nullopt}};
    g_MonsterDataHandler.RestoreMonsters(oneNumber);
    CHECK(g_MonsterDatabase.Find(2) == nullptr);
    CHECK(g_toldNumbers == std::vector<int>{2});

    // Nothing restored, nothing told.
    g_toldNumbers.clear();
    g_MonsterDataHandler.RestoreMonsters(std::span<const MonsterSnapshot>());
    CHECK(g_toldNumbers.empty());
}

TEST_CASE("Monster data: the editor's checks are those of loading and saving [data][monsters][editor]")
{
    MonsterDefinition separator = Monster(7, "Budge||Dragon");
    MonsterDefinition unknownLanguage = Monster(9, "Thunder Lich");
    unknownLanguage.names.Set("zz", "Lich");
    const std::array monsters = {BudgeDragon(), separator, unknownLanguage,
                                 Monster(252, "", nullptr, MonsterKind::Npc)};
    g_MonsterDatabase.Build(monsters, "en");
    std::vector<ItemDataIssue> issues;

    g_MonsterDataHandler.Validate(issues);

    REQUIRE(issues.size() == 3);
    CHECK(Data::Items::HasErrors(issues));
    CHECK(FindIssue(issues, 2) == nullptr);
    REQUIRE(FindIssue(issues, 7) != nullptr);
    CHECK(FindIssue(issues, 7)->severity == ItemDataIssueSeverity::Error);
    REQUIRE(FindIssue(issues, 9) != nullptr);
    CHECK(FindIssue(issues, 9)->severity == ItemDataIssueSeverity::Warning);
    CHECK(FindIssue(issues, 9)->message.find("never shown") != std::string::npos);
    REQUIRE(FindIssue(issues, 252) != nullptr);
    CHECK(FindIssue(issues, 252)->severity == ItemDataIssueSeverity::Warning);
    for (const ItemDataIssue& issue : issues)
    {
        CHECK(issue.source == Data::Monsters::GetMonstersFile().generic_string());
    }
}

TEST_CASE("Monster data: the editor saves the table and reverts to the file [data][monsters][editor]")
{
    TemporaryMonsterFolder folder;
    ClientDataScope scope(folder.Path());
    const std::array monsters = {BudgeDragon()};
    g_MonsterDatabase.Build(monsters, "pt");
    std::vector<ItemDataIssue> issues;

    REQUIRE(g_MonsterDataHandler.Save(issues));
    CHECK(issues.empty());
    CHECK(Data::Monsters::LoadMonstersFile(folder.MonstersFile()).monsters == std::vector{BudgeDragon()});

    SUBCASE("a revert drops the changes, in place")
    {
        const MonsterSnapshot added{3, Monster(3, "Larva")};
        g_MonsterDataHandler.RestoreMonsters(std::span<const MonsterSnapshot>(&added, 1));
        g_MonsterDatabase.Set(Monster(2, "Budge Dragon", "Dragão"));
        const wchar_t* name = g_MonsterDatabase.FindDisplayName(2);
        NamesChangedListener listener;

        REQUIRE(g_MonsterDataHandler.Revert(issues));

        CHECK(issues.empty());
        CHECK(g_MonsterDatabase.Find(3) == nullptr);
        CHECK(g_MonsterDatabase.FindDisplayName(2) == name);
        // Still in the language it was shown in.
        CHECK(std::wstring(name) == L"Dragão Lento");
        CHECK(g_toldNumbers == std::vector<int>{MonsterDataHandler::AllNumbers});
    }
    SUBCASE("a save with errors keeps the file")
    {
        g_MonsterDatabase.Set(Monster(7, "Budge||Dragon"));

        CHECK_FALSE(g_MonsterDataHandler.Save(issues));

        CHECK(Data::Items::HasErrors(issues));
        CHECK(Data::Monsters::LoadMonstersFile(folder.MonstersFile()).monsters == std::vector{BudgeDragon()});
    }
    SUBCASE("a revert of a broken file keeps the table")
    {
        folder.WriteMonsters(R"({ "formatVersion": 1, "monsters": [ { "number": 7, "name": "X", "kind": "boss" } ] })");

        CHECK_FALSE(g_MonsterDataHandler.Revert(issues));

        CHECK(Data::Items::HasErrors(issues));
        REQUIRE(g_MonsterDatabase.Find(2) != nullptr);
        CHECK(g_MonsterDatabase.Find(7) == nullptr);
    }
}

#endif // _EDITOR
