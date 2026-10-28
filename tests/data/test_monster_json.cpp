#include "stdafx.h"

#include "doctest.h"

#include "Core/Globals/_define.h"
#include "Data/GameData/MonsterData/MonsterDataValidation.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Core/Globals/_enum.h"
#include "Data/GameData/MonsterData/MonstersJson.h"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

using Data::Items::HasFatal;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterKind;

namespace
{
struct ReadResult
{
    std::vector<MonsterDefinition> monsters;
    std::vector<ItemDataIssue> issues;
};

ReadResult Read(const std::string& text)
{
    ReadResult result;
    Data::Monsters::ReadMonstersJson(text, "Monsters.json", result.monsters, result.issues);
    return result;
}

std::string Messages(const std::vector<ItemDataIssue>& issues)
{
    std::string text;
    for (const ItemDataIssue& issue : issues)
    {
        text += issue.ToString() + "\n";
    }
    return text;
}

bool HasIssueAbout(const std::vector<ItemDataIssue>& issues, const std::string& field, const std::string& message)
{
    return std::any_of(issues.begin(), issues.end(), [&](const ItemDataIssue& issue)
                       { return issue.field == field && issue.message.find(message) != std::string::npos; });
}

// One monster with nothing else around it.
std::string MonsterFile(const std::string& monster)
{
    return R"({ "formatVersion": 1, "monsters": [ )" + monster + " ] }";
}

MonsterDefinition Monster(int number, const char* english, MonsterKind kind = MonsterKind::Monster)
{
    MonsterDefinition monster;
    monster.number = number;
    monster.names.Set("en", english);
    monster.kind = kind;
    return monster;
}
} // namespace

TEST_CASE("Monster data: reads the monsters with their names and kinds [data][monsters]")
{
    const ReadResult read = Read(R"({ "formatVersion": 1, "monsters": [
        { "number": 2, "name": {"en": "Budge Dragon", "pt": "Dragão Lento"}, "kind": "monster" },
        { "number": 251, "name": "Hanzo the Blacksmith", "kind": "npc" },
        { "number": 100, "name": {"en": "Lance"}, "kind": "trap" },
        { "number": 451, "name": {"en": "Cursed Wizard"}, "kind": "other" } ] })");

    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.monsters.size() == 4);
    CHECK(read.monsters[0].number == 2);
    CHECK(read.monsters[0].names.GetNeutral() == "Budge Dragon");
    CHECK(read.monsters[0].names.Get("pt") == "Dragão Lento");
    CHECK(read.monsters[0].kind == MonsterKind::Monster);
    CHECK(read.monsters[1].names.GetNeutral() == "Hanzo the Blacksmith");
    CHECK(read.monsters[1].kind == MonsterKind::Npc);
    CHECK(read.monsters[2].kind == MonsterKind::Trap);
    CHECK(read.monsters[3].kind == MonsterKind::Other);
}

TEST_CASE("Monster data: a number outside 0-1023, a missing name and an unknown kind are errors [data][monsters]")
{
    CHECK(HasIssueAbout(Read(MonsterFile(R"({ "number": 1024, "name": "X", "kind": "monster" })")).issues,
                        "monsters[0].number", "0 to 1023"));
    CHECK(HasIssueAbout(Read(MonsterFile(R"({ "number": -1, "name": "X", "kind": "monster" })")).issues,
                        "monsters[0].number", "0 to 1023"));
    CHECK(HasIssueAbout(Read(MonsterFile(R"({ "number": 7, "kind": "monster" })")).issues, "monster 7 name",
                        "missing"));
    CHECK(HasIssueAbout(Read(MonsterFile(R"({ "number": 7, "name": 5, "kind": "monster" })")).issues,
                        "monster 7 name", "a text"));
    CHECK(HasIssueAbout(Read(MonsterFile(R"({ "number": 7, "name": "X", "kind": "boss" })")).issues,
                        "monster 7 kind", "monster, npc, trap or other"));
    CHECK(HasIssueAbout(Read(MonsterFile(R"({ "number": 7, "name": "X" })")).issues, "monster 7 kind", "missing"));

    const ReadResult broken = Read(MonsterFile(R"({ "number": 7, "name": "X", "kind": "boss" })"));
    CHECK(broken.monsters.empty());
    REQUIRE_FALSE(broken.issues.empty());
    CHECK(broken.issues.front().number == 7);
}

TEST_CASE("Monster data: a number twice is an error, the first one is kept [data][monsters]")
{
    const ReadResult read = Read(R"({ "formatVersion": 1, "monsters": [
        { "number": 7, "name": "First", "kind": "monster" },
        { "number": 7, "name": "Second", "kind": "npc" } ] })");

    CHECK(HasIssueAbout(read.issues, "monster 7 number", "more than once"));
    REQUIRE(read.monsters.size() == 1);
    CHECK(read.monsters[0].names.GetNeutral() == "First");
}

TEST_CASE("Monster data: a bad header or list is an error, unknown fields are warnings [data][monsters]")
{
    CHECK(HasFatal(Read("not json").issues));
    CHECK(HasFatal(Read(R"({ "formatVersion": 2, "monsters": [] })").issues));
    CHECK(HasIssueAbout(Read(R"({ "formatVersion": 1, "monsters": {} })").issues, "monsters", "a list"));
    CHECK(HasIssueAbout(Read(MonsterFile("5")).issues, "monsters[0]", "an object"));

    const ReadResult unknown = Read(R"({ "formatVersion": 1, "color": "red", "monsters": [
        { "number": 7, "name": "X", "kind": "monster", "scale": 2 } ] })");
    CHECK_FALSE(HasFatal(unknown.issues));
    CHECK(HasIssueAbout(unknown.issues, "color", "unknown field"));
    CHECK(HasIssueAbout(unknown.issues, "monster 7 scale", "unknown field"));
    CHECK(unknown.monsters.size() == 1);
}

TEST_CASE("Monster data: the writer sorts by number and puts each name on one line [data][monsters]")
{
    MonsterDefinition budge = Monster(2, "Budge Dragon");
    budge.names.Set("pt", "Dragão Lento");
    budge.names.Set("es", "Dragón Escurridizo");
    const std::array monsters = {Monster(251, "Hanzo the Blacksmith", MonsterKind::Npc), budge};

    const std::string text = Data::Monsters::WriteMonstersJson(monsters);

    CHECK(text == "{\n"
                  "  \"formatVersion\": 1,\n"
                  "  \"monsters\": [\n"
                  "    {\n"
                  "      \"number\": 2,\n"
                  "      \"name\": {\"en\": \"Budge Dragon\", \"es\": \"Dragón Escurridizo\", \"pt\": \"Dragão Lento\"},\n"
                  "      \"kind\": \"monster\"\n"
                  "    },\n"
                  "    {\n"
                  "      \"number\": 251,\n"
                  "      \"name\": {\"en\": \"Hanzo the Blacksmith\"},\n"
                  "      \"kind\": \"npc\"\n"
                  "    }\n"
                  "  ]\n"
                  "}\n");
    const ReadResult back = Read(text);
    CHECK_MESSAGE(back.issues.empty(), Messages(back.issues));
    REQUIRE(back.monsters.size() == 2);
    CHECK(back.monsters[0] == budge);
}

TEST_CASE("Monster data: an empty English name and a long name are warnings [data][monsters]")
{
    MonsterDefinition empty = Monster(252, "");
    MonsterDefinition longName = Monster(7, std::string(MAX_MONSTER_NAME + 1, 'x').c_str());
    MonsterDefinition fits = Monster(8, std::string(MAX_MONSTER_NAME, 'x').c_str());
    const std::array monsters = {empty, longName, fits};
    std::vector<ItemDataIssue> issues;

    Data::Monsters::ValidateMonsterNames(monsters, issues);

    CHECK_FALSE(HasFatal(issues));
    REQUIRE(issues.size() == 2);
    CHECK(issues[0].number == 252);
    CHECK(issues[0].message.find("English name is empty") != std::string::npos);
    CHECK(issues[0].message.find("without a translation") == std::string::npos);
    CHECK(issues[1].number == 7);
    CHECK(issues[1].message.find("cut in the game") != std::string::npos);
}

TEST_CASE("Monster data: without an English name a translation still shows in its language [data][monsters]")
{
    MonsterDefinition monster = Monster(252, "");
    monster.names.Set("pt", "Nome");
    const std::array monsters = {monster};
    std::vector<ItemDataIssue> issues;

    Data::Monsters::ValidateMonsterNames(monsters, issues);

    REQUIRE(issues.size() == 1);
    CHECK(issues[0].severity == ItemDataIssueSeverity::Warning);
    CHECK(issues[0].message.find("without a translation") != std::string::npos);
}

TEST_CASE("Monster data: an emoji counts as two characters of the name [data][monsters]")
{
    // 63 characters and one emoji: 65 UTF-16 units, one more than the game holds.
    const std::string name = std::string(MAX_MONSTER_NAME - 1, 'x') + "\xF0\x9F\x90\x89";
    const std::array monsters = {Monster(9, name.c_str())};
    std::vector<ItemDataIssue> issues;

    Data::Monsters::ValidateMonsterNames(monsters, issues);

    CHECK(HasIssueAbout(issues, "monster 9 name", "cut in the game"));
}

TEST_CASE("Monster data: the separator and bad language codes are errors [data][monsters]")
{
    MonsterDefinition separator = Monster(7, "Budge||Dragon");
    MonsterDefinition badLocale = Monster(8, "Budge Dragon");
    badLocale.names.Set("p t", "Dragão");
    const std::array monsters = {separator, badLocale};
    std::vector<ItemDataIssue> issues;

    Data::Monsters::ValidateMonsterNames(monsters, issues);

    CHECK(HasIssueAbout(issues, "monster 7 name", "must not contain"));
    CHECK(HasIssueAbout(issues, "monster 8 name", "not a language code"));
    CHECK(HasFatal(issues));
}

TEST_CASE("Monster data: a name in a language the game has no texts for is a warning [data][monsters]")
{
    MonsterDefinition monster = Monster(2, "Budge Dragon");
    monster.names.Set("pt", "Dragão Lento");
    monster.names.Set("xx", "Something");
    const std::array monsters = {monster};
    const std::array<const char*, 2> uiLocales = {"en", "pt"};
    std::vector<ItemDataIssue> issues;

    Data::Monsters::ValidateMonsterNameLocales(monsters, uiLocales, issues);

    REQUIRE(issues.size() == 1);
    CHECK(issues[0].severity == ItemDataIssueSeverity::Warning);
    CHECK(issues[0].message.find("xx name is never shown") != std::string::npos);
}

TEST_CASE("Monster data: kinds have names and object kinds [data][monsters]")
{
    using Data::Monsters::FindMonsterKind;
    using Data::Monsters::FromObjectKind;
    using Data::Monsters::GetMonsterKindName;
    using Data::Monsters::ToObjectKind;

    for (MonsterKind kind : {MonsterKind::Monster, MonsterKind::Npc, MonsterKind::Trap, MonsterKind::Other})
    {
        CHECK(FindMonsterKind(GetMonsterKindName(kind)) == kind);
        CHECK(FromObjectKind(ToObjectKind(kind)) == kind);
    }
    CHECK(ToObjectKind(MonsterKind::Monster) == KIND_MONSTER);
    CHECK(ToObjectKind(MonsterKind::Npc) == KIND_NPC);
    CHECK(ToObjectKind(MonsterKind::Trap) == KIND_TRAP);
    CHECK(ToObjectKind(MonsterKind::Other) == KIND_TMP);
    CHECK_FALSE(FromObjectKind(KIND_PLAYER).has_value());
    CHECK_FALSE(FindMonsterKind("boss").has_value());
}

TEST_CASE("Monster data: a monster names the model it appears with [data][monsters]")
{
    const ReadResult read = Read(MonsterFile(
        R"({ "number": 2, "name": "Budge Dragon", "kind": "monster", "model": "MONSTER_MODEL_BUDGE_DRAGON" },
        { "number": 151, "name": "Soldier", "kind": "monster", "model": "player" },
        { "number": 152, "name": "Gate to Kalima 1", "kind": "monster", "model": "MODEL_WARCRAFT" },
        { "number": 102, "name": "Trap", "kind": "trap", "model": "mapObject 39" },
        { "number": 251, "name": "Hanzo the Blacksmith", "kind": "npc", "model": "MODEL_SMITH" },
        { "number": 7, "name": "Giant", "kind": "monster" })"));
    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.monsters.size() == 6);
    CHECK(read.monsters[0].model == "MONSTER_MODEL_BUDGE_DRAGON");
    CHECK(read.monsters[3].model == "mapObject 39");
    // A monster without a model is read; the list does not say its model.
    CHECK(read.monsters[5].model.empty());

    MonsterDefinition budge = Monster(2, "Budge Dragon");
    budge.model = "MONSTER_MODEL_BUDGE_DRAGON";
    const std::string text = Data::Monsters::WriteMonstersJson(std::array{budge});
    CHECK(text.find("\"kind\": \"monster\",\n      \"model\": \"MONSTER_MODEL_BUDGE_DRAGON\"\n") != std::string::npos);
    const ReadResult back = Read(text);
    REQUIRE(back.monsters.size() == 1);
    CHECK(back.monsters[0] == budge);
}

TEST_CASE("Monster data: a model that names no model is an error [data][monsters]")
{
    for (const char* model : {"MODEL_HOUND", "MODEL_PLAYER", "Player", "mapObject 160", "mapObject x", "\"\""})
    {
        const std::string text = std::string(R"({ "number": 2, "name": "X", "kind": "monster", "model": )") +
                                 (model[0] == '"' ? model : "\"" + std::string(model) + "\"") + " }";
        INFO(model);
        const ReadResult read = Read(MonsterFile(text));
        CHECK(read.monsters.empty());
        CHECK(HasIssueAbout(read.issues, "monster 2 model", "must be a model"));
    }
    CHECK(HasIssueAbout(Read(MonsterFile(R"({ "number": 2, "name": "X", "kind": "monster", "model": 5 })")).issues,
                        "monster 2 model", "must be a model"));
}

TEST_CASE("Monster data: the models a monster can name, and back [data][monsters]")
{
    CHECK(Data::Monsters::FindMonsterModelReference("player") == MODEL_PLAYER);
    CHECK(Data::Monsters::FindMonsterModelReference("MODEL_BALL") == MODEL_BALL);
    CHECK(Data::Monsters::FindMonsterModelReference("mapObject 51") == 51);
    CHECK(Data::Monsters::FindMonsterModelReference("MONSTER_MODEL_HOUND") == MODEL_HOUND);
    for (const int slot : {static_cast<int>(MODEL_PLAYER), static_cast<int>(MODEL_WARCRAFT), 25,
                           static_cast<int>(MODEL_HOUND), static_cast<int>(MODEL_SMITH)})
    {
        CHECK(Data::Monsters::FindMonsterModelReference(Data::Monsters::GetMonsterModelReference(slot)) == slot);
    }
    CHECK(Data::Monsters::GetMonsterModelReference(MODEL_ARROW).empty());
}
