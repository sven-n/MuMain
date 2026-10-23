#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"
#include "TemporaryMonsterFolder.h"

#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/MonsterData/LegacyMonsterKind.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterKind.h"
#include "Data/GameData/MonsterData/MonstersJson.h"

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

using Data::Items::HasErrors;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Monsters::MonsterDefinition;

namespace
{
const std::filesystem::path ClientData = MU_TEST_DATA_DIR;

std::string Messages(const std::vector<ItemDataIssue>& issues)
{
    std::string text;
    for (const ItemDataIssue& issue : issues)
    {
        text += issue.ToString() + "\n";
    }
    return text;
}

const MonsterDefinition* FindMonster(const std::vector<MonsterDefinition>& monsters, int number)
{
    const auto found = std::find_if(monsters.begin(), monsters.end(),
                                    [&](const MonsterDefinition& monster) { return monster.number == number; });
    return found == monsters.end() ? nullptr : &*found;
}
} // namespace

TEST_CASE("Shipped monster list: loads, with only the warning about 252's empty name [data][monsters]")
{
    const Data::Monsters::MonsterDataLoadResult loaded =
        Data::Monsters::LoadMonstersFile(ClientData / "Monsters" / "Monsters.json");

    REQUIRE_MESSAGE(loaded.issues.size() == 1, Messages(loaded.issues));
    CHECK(loaded.issues[0].severity == ItemDataIssueSeverity::Warning);
    CHECK(loaded.issues[0].number == 252);
    CHECK(loaded.monsters.size() == 547);
}

TEST_CASE("Shipped monster list: written in the writer's format [data][monsters]")
{
    const std::filesystem::path file = ClientData / "Monsters" / "Monsters.json";
    const Data::Monsters::MonsterDataLoadResult loaded = Data::Monsters::LoadMonstersFile(file);
    const auto text = Data::ReadTextFile(file);

    REQUIRE(text.has_value());
    CHECK(Data::Monsters::WriteMonstersJson(loaded.monsters) == *text);
}

TEST_CASE("Shipped monster list: the names the old name file test asked for [data][monsters]")
{
    const Data::Monsters::MonsterDataLoadResult loaded =
        Data::Monsters::LoadMonstersFile(ClientData / "Monsters" / "Monsters.json");

    // The monsters the game creates on its own (the cinematics, the Elf Soldier, ...).
    for (int number : {42, 297, 318, 464})
    {
        CAPTURE(number);
        const MonsterDefinition* monster = FindMonster(loaded.monsters, number);
        REQUIRE(monster != nullptr);
        CHECK_FALSE(monster->names.GetNeutral().empty());
        CHECK(monster->names.GetTranslations().contains("pt"));
        CHECK(monster->names.GetTranslations().contains("es"));
    }
    CHECK(FindMonster(loaded.monsters, 89)->names.Get("es") == "Esqueleto Mágico");
    CHECK(FindMonster(loaded.monsters, 566)->names.Get("pt") == "Administrador da Guilda Mercenária Tercia");
    CHECK(FindMonster(loaded.monsters, 566)->names.Get("es") == "Tercia de Encargados del Gremio Mercenario");
}

TEST_CASE("Shipped monster list: names are plain text, the Kalima gates too [data][monsters]")
{
    const Data::Monsters::MonsterDataLoadResult loaded =
        Data::Monsters::LoadMonstersFile(ClientData / "Monsters" / "Monsters.json");

    // The gates' owner text is the UI text MagicStoneOfOwner; a "%" in a name
    // would be a format that nothing fills in.
    for (const MonsterDefinition& monster : loaded.monsters)
    {
        CAPTURE(monster.number);
        CHECK(monster.names.GetNeutral().find('%') == std::string::npos);
        for (const auto& [locale, name] : monster.names.GetTranslations())
        {
            CAPTURE(locale);
            CHECK(name.find('%') == std::string::npos);
        }
    }
    CHECK(FindMonster(loaded.monsters, 152)->names.GetNeutral() == "Gate to Kalima 1");
    CHECK(FindMonster(loaded.monsters, 158)->names.GetNeutral() == "Gate to Kalima 7");
}

TEST_CASE("Shipped monster list: OpenMU's statues 658-668 have its English names and their old kind [data][monsters]")
{
    const Data::Monsters::MonsterDataLoadResult loaded =
        Data::Monsters::LoadMonstersFile(ClientData / "Monsters" / "Monsters.json");

    CHECK(FindMonster(loaded.monsters, 658)->names.GetNeutral() == "Cursed Statue");
    CHECK(FindMonster(loaded.monsters, 659)->names.GetNeutral() == "Captured Stone Statue (1)");
    CHECK(FindMonster(loaded.monsters, 668)->names.GetNeutral() == "Captured Stone Statue (10)");
    for (int number = 658; number <= 668; ++number)
    {
        CAPTURE(number);
        const MonsterDefinition* monster = FindMonster(loaded.monsters, number);
        REQUIRE(monster != nullptr);
        CHECK(Data::Monsters::ToObjectKind(monster->kind) == Data::Monsters::GetLegacyObjectKind(number));
    }
}

TEST_CASE("Shipped monster list: Tercia and Veina are NPCs [data][monsters]")
{
    // The old rules made them monsters (P7 of the monsters design).
    const Data::Monsters::MonsterDataLoadResult loaded =
        Data::Monsters::LoadMonstersFile(ClientData / "Monsters" / "Monsters.json");

    CHECK(FindMonster(loaded.monsters, 566)->kind == Data::Monsters::MonsterKind::Npc);
    CHECK(FindMonster(loaded.monsters, 567)->kind == Data::Monsters::MonsterKind::Npc);
}

TEST_CASE("Monster data: the game's load fills the table, errors stop it [data][monsters]")
{
    SUBCASE("the shipped file")
    {
        ClientDataScope scope(ClientData.parent_path());
        std::string error;

        CHECK(g_MonsterDataHandler.Load(error));
        CHECK(error.empty());
        CHECK(std::wstring(g_MonsterDatabase.FindDisplayName(2)) == L"Budge Dragon");
    }
    SUBCASE("a broken file")
    {
        TemporaryMonsterFolder folder;
        folder.WriteMonsters(R"({ "formatVersion": 1, "monsters": [ { "number": 7, "name": "X", "kind": "boss" } ] })");
        ClientDataScope scope(folder.Path());
        std::string error;

        CHECK_FALSE(g_MonsterDataHandler.Load(error));
        CHECK(error.find("monster data") != std::string::npos);
    }
    SUBCASE("a missing file")
    {
        TemporaryMonsterFolder folder;
        ClientDataScope scope(folder.Path());
        std::string error;

        CHECK_FALSE(g_MonsterDataHandler.Load(error));
        CHECK(error.find("could not be read") != std::string::npos);
    }
}

TEST_CASE("Monster data: saving writes what loads back, and refuses errors [data][monsters]")
{
    TemporaryMonsterFolder folder;
    const std::filesystem::path file = folder.MonstersFile();
    MonsterDefinition monster;
    monster.number = 2;
    monster.names.Set("en", "Budge Dragon");
    std::vector<MonsterDefinition> monsters = {monster};
    std::vector<ItemDataIssue> issues;

    CHECK(Data::Monsters::SaveMonstersFile(file, monsters, issues));
    CHECK(Data::Monsters::LoadMonstersFile(file).monsters == monsters);

    monsters[0].names.Set("en", "Budge||Dragon");
    issues.clear();
    CHECK_FALSE(Data::Monsters::SaveMonstersFile(file, monsters, issues));
    CHECK(HasErrors(issues));
    CHECK(Data::Monsters::LoadMonstersFile(file).monsters[0].names.GetNeutral() == "Budge Dragon");
    CHECK_FALSE(std::filesystem::exists(file.string() + ".tmp"));
}
