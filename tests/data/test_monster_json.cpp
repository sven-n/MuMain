#include "stdafx.h"

#include "doctest.h"

#include "Core/Globals/_define.h"
#include "Data/GameData/MonsterData/MonsterDataValidation.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Core/Globals/_enum.h"
#include "Data/GameData/MonsterData/MonstersJson.h"

#include <algorithm>
#include <array>
#include <span>
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
    CHECK(HasFatal(Read(R"({ "formatVersion": 3, "monsters": [] })").issues));
    CHECK(HasIssueAbout(Read(R"({ "formatVersion": 1, "monsters": {} })").issues, "monsters", "a list"));
    CHECK(HasIssueAbout(Read(MonsterFile("5")).issues, "monsters[0]", "an object"));

    const ReadResult unknown = Read(R"({ "formatVersion": 1, "color": "red", "monsters": [
        { "number": 7, "name": "X", "kind": "monster", "colour": 2 } ] })");
    CHECK_FALSE(HasFatal(unknown.issues));
    CHECK(HasIssueAbout(unknown.issues, "color", "unknown field"));
    CHECK(HasIssueAbout(unknown.issues, "monster 7 colour", "unknown field"));
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
                  "  \"formatVersion\": 2,\n"
                  "  \"monsters\": [\n"
                  "    {\n"
                  "      \"name\": {\"en\": \"Budge Dragon\", \"es\": \"Dragón Escurridizo\", \"pt\": \"Dragão Lento\"},\n"
                  "      \"kind\": \"monster\",\n"
                  "      \"numbers\": [\n"
                  "        {\"number\": 2}\n"
                  "      ]\n"
                  "    },\n"
                  "    {\n"
                  "      \"name\": {\"en\": \"Hanzo the Blacksmith\"},\n"
                  "      \"kind\": \"npc\",\n"
                  "      \"numbers\": [\n"
                  "        {\"number\": 251}\n"
                  "      ]\n"
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
    CHECK(text.find("\"kind\": \"monster\",\n      \"model\": \"MONSTER_MODEL_BUDGE_DRAGON\",\n      \"numbers\"") !=
          std::string::npos);
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

TEST_CASE("Monster data: a monster names its sound set and its random variants' (Q-MN78, Q-MN83) [data][monsters]")
{
    const ReadResult read = Read(MonsterFile(
        R"({ "number": 163, "name": "Chaos Castle 2", "kind": "monster", "model": "MONSTER_MODEL_CHAOSCASTLE_ELF",
             "soundSet": "Chaos Castle",
             "randomVariants": [{"model": "MONSTER_MODEL_CHAOSCASTLE_WIZARD", "soundSet": "Chaos Castle Wizard"}] })"));
    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.monsters.size() == 1);
    const MonsterDefinition& elf = read.monsters[0];
    CHECK(elf.soundSet == "Chaos Castle");
    REQUIRE(elf.randomVariants.size() == 1);
    CHECK(elf.randomVariants[0].model == "MONSTER_MODEL_CHAOSCASTLE_WIZARD");
    CHECK(elf.randomVariants[0].soundSet == "Chaos Castle Wizard");

    const std::string text = Data::Monsters::WriteMonstersJson(std::array{elf});
    INFO(text);
    CHECK(text.find("\"model\": \"MONSTER_MODEL_CHAOSCASTLE_ELF\",\n      \"soundSet\": \"Chaos Castle\",\n") !=
          std::string::npos);
    CHECK(text.find("\"randomVariants\": [\n        {\"model\": \"MONSTER_MODEL_CHAOSCASTLE_WIZARD\", \"soundSet\": "
                    "\"Chaos Castle Wizard\"}\n      ]") != std::string::npos);
    const ReadResult back = Read(text);
    REQUIRE(back.monsters.size() == 1);
    CHECK(back.monsters[0] == elf);

    // A variant that names no set keeps the entry's.
    MonsterDefinition plain = elf;
    plain.randomVariants[0].soundSet.reset();
    const ReadResult plainBack = Read(Data::Monsters::WriteMonstersJson(std::array{plain}));
    REQUIRE(plainBack.monsters.size() == 1);
    CHECK(plainBack.monsters[0] == plain);
}

TEST_CASE("Monster data: a wrong sound set or random variant is an error [data][monsters]")
{
    struct Case
    {
        std::string fields;
        std::string field;
        std::string message;
    };
    const std::vector<Case> cases = {
        {R"("soundSet": "")", "monster 163 soundSet", "name of a sound set"},
        {R"("soundSet": 5)", "monster 163 soundSet", "name of a sound set"},
        {R"("randomVariants": {"model": "MONSTER_MODEL_CHAOSCASTLE_WIZARD"})", "monster 163 randomVariants",
         "list of random variants"},
        {R"("randomVariants": [{"soundSet": "X"}])", "monster 163 randomVariants[0].model", "the model of the look"},
        {R"("randomVariants": [{"model": "MODEL_HOUND"}])", "monster 163 randomVariants[0].model",
         "the model of the look"},
        {R"("randomVariants": [{"model": "MONSTER_MODEL_CHAOSCASTLE_ELF"}])", "monster 163 randomVariants[0].model",
         "the entry's model or another variant's"},
        {R"("randomVariants": [{"model": "MONSTER_MODEL_CHAOSCASTLE_WIZARD"}, {"model": "MONSTER_MODEL_CHAOSCASTLE_WIZARD"}])",
         "monster 163 randomVariants[1].model", "the entry's model or another variant's"},
        {R"("randomVariants": [{"model": "MONSTER_MODEL_CHAOSCASTLE_WIZARD", "soundSet": ""}])",
         "monster 163 randomVariants[0].soundSet", "name of a sound set"},
    };
    for (const Case& test : cases)
    {
        const ReadResult read = Read(MonsterFile(
            R"({ "number": 163, "name": "X", "kind": "monster", "model": "MONSTER_MODEL_CHAOSCASTLE_ELF", )" +
            test.fields + " }"));
        INFO(test.fields);
        INFO(Messages(read.issues));
        CHECK(read.monsters.empty());
        CHECK(HasIssueAbout(read.issues, test.field, test.message));
    }
}

namespace
{
using Data::Monsters::MonsterGroup;

struct GroupsResult
{
    std::vector<MonsterGroup> monsters;
    std::vector<ItemDataIssue> issues;
};

GroupsResult ReadGroups(const std::string& text)
{
    GroupsResult result;
    Data::Monsters::ReadMonstersJson(text, "Monsters.json", result.monsters, result.issues);
    return result;
}

// One monster of version 2 with nothing else around it.
std::string GroupFile(const std::string& monster)
{
    return R"({ "formatVersion": 2, "monsters": [ )" + monster + " ] }";
}

Data::Monsters::MonsterAttacks GenericPattern()
{
    Data::Monsters::MonsterAttacks attacks;
    attacks.otherwiseGeneric = true;
    return attacks;
}

Data::Monsters::MonsterAttacks Rotation()
{
    Data::Monsters::MonsterAttacks attacks;
    attacks.otherwise = {MONSTER01_ATTACK1, MONSTER01_ATTACK2};
    return attacks;
}

// Death Centurion of Kalima 1 (145) and 7 (336), which has no attack data (P26).
MonsterGroup Centurion()
{
    MonsterDefinition shared = Monster(145, "Death Centurion");
    shared.names.Set("es", "Centurión de la Muerte");
    shared.model = "MONSTER_MODEL_DEATH_CENTURION";
    shared.attacks = Rotation();
    MonsterDefinition first = shared;
    MonsterDefinition seventh = shared;
    seventh.number = 336;
    seventh.attacks = GenericPattern();
    return {shared, {first, seventh}};
}
} // namespace

TEST_CASE("Monster data: a monster's numbers have its values, and their own ones instead (D61) [data][monsters]")
{
    const GroupsResult read = ReadGroups(GroupFile(
        R"({ "name": {"en": "Death Centurion", "es": "Centurión de la Muerte"}, "kind": "monster",
             "model": "MONSTER_MODEL_DEATH_CENTURION", "attacks": {"otherwise": ["attack1", "attack2"]},
             "numbers": [ {"number": 145},
                          {"number": 336, "attacks": {"otherwise": "generic"}},
                          {"number": 337, "name": "Kalima Centurion", "kind": "npc"} ] })"));

    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.monsters.size() == 1);
    const MonsterGroup& monster = read.monsters[0];
    REQUIRE(monster.numbers.size() == 3);
    CHECK(monster.shared.names.Get("es") == "Centurión de la Muerte");
    CHECK(monster.shared.attacks == Rotation());

    const MonsterDefinition& first = monster.numbers[0];
    CHECK(first.number == 145);
    CHECK(first.names == monster.shared.names);
    CHECK(first.model == "MONSTER_MODEL_DEATH_CENTURION");
    CHECK(first.attacks == Rotation());

    const MonsterDefinition& seventh = monster.numbers[1];
    CHECK(seventh.number == 336);
    CHECK(seventh.model == "MONSTER_MODEL_DEATH_CENTURION");
    CHECK(seventh.attacks == GenericPattern());

    // A number's own name replaces all of the monster's, not one language.
    const MonsterDefinition& renamed = monster.numbers[2];
    CHECK(renamed.names.GetNeutral() == "Kalima Centurion");
    CHECK(renamed.names.Get("es") == "Kalima Centurion");
    CHECK(renamed.kind == MonsterKind::Npc);
    CHECK(renamed.attacks == Rotation());

    // The game's list has the entry of each number.
    std::vector<MonsterDefinition> entries;
    std::vector<ItemDataIssue> issues;
    Data::Monsters::ReadMonstersJson(
        GroupFile(R"({ "name": "X", "kind": "monster", "numbers": [{"number": 9}, {"number": 3}] })"), "Monsters.json",
        entries, issues);
    REQUIRE(entries.size() == 2);
    CHECK(entries[0].number == 9);
    CHECK(entries[1].number == 3);
}

TEST_CASE("Monster data: a number's own value may be empty where its monster's is not [data][monsters]")
{
    const GroupsResult read = ReadGroups(GroupFile(
        R"({ "name": "Chaos Castle 2", "kind": "monster", "model": "MONSTER_MODEL_CHAOSCASTLE_ELF",
             "soundSet": "Chaos Castle", "attacks": {"otherwise": "attack2"},
             "mapVariants": [{"maps": [9], "attacks": {"otherwise": "attack1"}}],
             "randomVariants": [{"model": "MONSTER_MODEL_CHAOSCASTLE_WIZARD"}],
             "numbers": [ {"number": 163},
                          {"number": 164, "model": "", "soundSet": "", "attacks": null, "mapVariants": [],
                           "randomVariants": []} ] })"));

    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.monsters.size() == 1);
    REQUIRE(read.monsters[0].numbers.size() == 2);
    const MonsterDefinition& full = read.monsters[0].numbers[0];
    CHECK(full.soundSet == "Chaos Castle");
    CHECK(full.attacks.has_value());
    CHECK(full.mapVariants.size() == 1);
    CHECK(full.randomVariants.size() == 1);
    const MonsterDefinition& empty = read.monsters[0].numbers[1];
    CHECK(empty.model.empty());
    CHECK(empty.soundSet.empty());
    CHECK_FALSE(empty.attacks.has_value());
    CHECK(empty.mapVariants.empty());
    CHECK(empty.randomVariants.empty());

    // The writer writes them so, and they read back the same.
    const std::string text = Data::Monsters::WriteMonstersJson(std::span<const MonsterGroup>(read.monsters));
    INFO(text);
    CHECK(text.find(R"({"number": 164, "model": "", "soundSet": "", "attacks": null, "mapVariants": [], )"
                    R"("randomVariants": []})") != std::string::npos);
    CHECK(ReadGroups(text).monsters == read.monsters);
}

// A map variant's setup is read with its maps: its problems show at once,
// not only once the maps are right.
TEST_CASE("Monster data: a map variant's setup problems show with its maps' [data][monsters]")
{
    const GroupsResult read = ReadGroups(GroupFile(
        R"({ "name": "Chaos Castle 2", "kind": "monster", "model": "MONSTER_MODEL_CHAOSCASTLE_ELF",
             "mapVariants": [{"maps": "nine", "scale": "big"}],
             "numbers": [ {"number": 163} ] })"));

    const std::string messages = Messages(read.issues);
    INFO(messages);
    const auto names = [&](std::string_view field)
    {
        return std::ranges::any_of(read.issues, [&](const ItemDataIssue& issue)
                                   { return issue.field.find(field) != std::string::npos; });
    };
    CHECK(names(".maps"));
    CHECK(names(".scale"));
}

TEST_CASE("Monster data: the writer writes the shared values once, and each number's own ones [data][monsters]")
{
    MonsterGroup centurion = Centurion();
    MonsterGroup budge{Monster(2, "Budge Dragon"), {Monster(2, "Budge Dragon")}};
    const std::array monsters = {centurion, budge};

    const std::string text = Data::Monsters::WriteMonstersJson(std::span<const MonsterGroup>(monsters));

    CHECK(text == "{\n"
                  "  \"formatVersion\": 2,\n"
                  "  \"monsters\": [\n"
                  "    {\n"
                  "      \"name\": {\"en\": \"Budge Dragon\"},\n"
                  "      \"kind\": \"monster\",\n"
                  "      \"numbers\": [\n"
                  "        {\"number\": 2}\n"
                  "      ]\n"
                  "    },\n"
                  "    {\n"
                  "      \"name\": {\"en\": \"Death Centurion\", \"es\": \"Centurión de la Muerte\"},\n"
                  "      \"kind\": \"monster\",\n"
                  "      \"model\": \"MONSTER_MODEL_DEATH_CENTURION\",\n"
                  "      \"attacks\": {\n"
                  "        \"otherwise\": [\"attack1\", \"attack2\"],\n"
                  "        \"clearSkill\": \"never\"\n"
                  "      },\n"
                  "      \"numbers\": [\n"
                  "        {\"number\": 145},\n"
                  "        {\"number\": 336, \"attacks\": {\"otherwise\": \"generic\", \"clearSkill\": \"never\"}}\n"
                  "      ]\n"
                  "    }\n"
                  "  ]\n"
                  "}\n");
    const GroupsResult back = ReadGroups(text);
    CHECK_MESSAGE(back.issues.empty(), Messages(back.issues));
    REQUIRE(back.monsters.size() == 2);
    CHECK(back.monsters[0] == budge);
    CHECK(back.monsters[1] == centurion);
}

TEST_CASE("Monster data: the writer sorts the numbers, and the monsters by their first number [data][monsters]")
{
    MonsterGroup centurion = Centurion();
    std::swap(centurion.numbers[0], centurion.numbers[1]);
    MonsterGroup late{Monster(200, "Late"), {Monster(200, "Late")}};
    const std::array monsters = {late, centurion};

    const GroupsResult back = ReadGroups(Data::Monsters::WriteMonstersJson(std::span<const MonsterGroup>(monsters)));

    REQUIRE(back.monsters.size() == 2);
    REQUIRE(back.monsters[0].numbers.size() == 2);
    CHECK(back.monsters[0].numbers[0].number == 145);
    CHECK(back.monsters[0].numbers[1].number == 336);
    CHECK(back.monsters[1].numbers[0].number == 200);
}

TEST_CASE("Monster data: a monster with one number shares all its values with it [data][monsters]")
{
    // The shared values of a monster whose other numbers went no longer count.
    MonsterGroup monster = Centurion();
    monster.numbers.erase(monster.numbers.begin());

    const std::string text = Data::Monsters::WriteMonstersJson(std::span<const MonsterGroup>(&monster, 1));

    INFO(text);
    CHECK(text.find("\"otherwise\": \"generic\"") != std::string::npos);
    CHECK(text.find("{\"number\": 336}") != std::string::npos);
    const GroupsResult back = ReadGroups(text);
    REQUIRE(back.monsters.size() == 1);
    CHECK(back.monsters[0].numbers == monster.numbers);
    CHECK(Data::Monsters::GetSharedValues(back.monsters[0]) == monster.numbers[0]);

    // A monster without numbers is not written.
    MonsterGroup empty = Centurion();
    empty.numbers.clear();
    CHECK(ReadGroups(Data::Monsters::WriteMonstersJson(std::span<const MonsterGroup>(&empty, 1))).monsters.empty());
}

TEST_CASE("Monster data: a monster needs numbers, and a number is in one monster once [data][monsters]")
{
    CHECK(HasIssueAbout(ReadGroups(GroupFile(R"({ "name": "X", "kind": "monster" })")).issues, "monsters[0].numbers",
                        "one or more numbers"));
    CHECK(HasIssueAbout(ReadGroups(GroupFile(R"({ "name": "X", "kind": "monster", "numbers": [] })")).issues,
                        "monsters[0].numbers", "one or more numbers"));
    CHECK(HasIssueAbout(ReadGroups(GroupFile(R"({ "name": "X", "kind": "monster", "numbers": [5] })")).issues,
                        "monsters[0].numbers[0]", "an object"));
    CHECK(HasIssueAbout(ReadGroups(GroupFile(R"({ "name": "X", "kind": "monster", "numbers": [{}] })")).issues,
                        "monsters[0].numbers[0].number", "0 to 1023"));
    // A version 2 entry with a number of its own is no monster.
    CHECK(HasIssueAbout(ReadGroups(GroupFile(R"({ "number": 7, "name": "X", "kind": "monster" })")).issues,
                        "monsters[0].numbers", "one or more numbers"));

    const GroupsResult twice = ReadGroups(R"({ "formatVersion": 2, "monsters": [
        { "name": "First", "kind": "monster", "numbers": [{"number": 7}, {"number": 8}, {"number": 7}] },
        { "name": "Second", "kind": "monster", "numbers": [{"number": 8}, {"number": 9}] } ] })");
    CHECK(HasIssueAbout(twice.issues, "monster 7 number", "more than once"));
    CHECK(HasIssueAbout(twice.issues, "monster 8 number", "more than once"));
    REQUIRE(twice.monsters.size() == 2);
    REQUIRE(twice.monsters[0].numbers.size() == 2);
    CHECK(twice.monsters[0].numbers[1].number == 8);
    REQUIRE(twice.monsters[1].numbers.size() == 1);
    CHECK(twice.monsters[1].numbers[0].number == 9);
}

TEST_CASE("Monster data: an error leaves out the number it is in, or the monster for a shared value [data][monsters]")
{
    const GroupsResult own = ReadGroups(GroupFile(
        R"({ "name": "X", "kind": "monster", "numbers": [{"number": 7}, {"number": 8, "kind": "boss"}] })"));
    CHECK(HasIssueAbout(own.issues, "monster 8 kind", "monster, npc, trap or other"));
    REQUIRE(own.monsters.size() == 1);
    REQUIRE(own.monsters[0].numbers.size() == 1);
    CHECK(own.monsters[0].numbers[0].number == 7);

    // A shared value names the monster by its first number.
    const GroupsResult shared =
        ReadGroups(GroupFile(R"({ "name": "X", "kind": "boss", "numbers": [{"number": 7}, {"number": 8}] })"));
    CHECK(HasIssueAbout(shared.issues, "monster 7 kind", "monster, npc, trap or other"));
    CHECK(shared.monsters.empty());

    // An own model that is one of the looks the number shares.
    const GroupsResult look = ReadGroups(GroupFile(
        R"({ "name": "X", "kind": "monster", "model": "MONSTER_MODEL_CHAOSCASTLE_ELF",
             "randomVariants": [{"model": "MONSTER_MODEL_CHAOSCASTLE_WIZARD"}],
             "numbers": [{"number": 163}, {"number": 164, "model": "MONSTER_MODEL_CHAOSCASTLE_WIZARD"}] })"));
    CHECK(HasIssueAbout(look.issues, "monster 164 randomVariants", "the entry's model and a variant's"));
    REQUIRE(look.monsters.size() == 1);
    CHECK(look.monsters[0].numbers.size() == 1);
}

TEST_CASE("Monster data: unknown fields of a monster and of a number are warnings [data][monsters]")
{
    const GroupsResult read = ReadGroups(
        GroupFile(R"({ "name": "X", "kind": "monster", "colour": 2, "numbers": [{"number": 7, "size": 3}] })"));

    CHECK_FALSE(HasFatal(read.issues));
    CHECK(HasIssueAbout(read.issues, "monster 7 colour", "unknown field"));
    CHECK(HasIssueAbout(read.issues, "monster 7 size", "unknown field"));
    REQUIRE(read.monsters.size() == 1);
    CHECK(read.monsters[0].numbers.size() == 1);
}

TEST_CASE("Monster data: a file of version 1 has a monster for each entry [data][monsters]")
{
    const GroupsResult read = ReadGroups(R"({ "formatVersion": 1, "monsters": [
        { "number": 145, "name": "Death Centurion", "kind": "monster", "attacks": {"otherwise": "attack2"} },
        { "number": 336, "name": "Death Centurion", "kind": "monster" } ] })");

    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.monsters.size() == 2);
    for (const MonsterGroup& monster : read.monsters)
    {
        REQUIRE(monster.numbers.size() == 1);
        CHECK(monster.shared == monster.numbers[0]);
    }
    CHECK(read.monsters[0].numbers[0].attacks.has_value());
    CHECK_FALSE(read.monsters[1].numbers[0].attacks.has_value());
}

TEST_CASE("Monster data: each number names the maps it appears on (Q-MN61) [data][monsters]")
{
    const GroupsResult read = ReadGroups(GroupFile(
        R"({ "name": "Illusion of Kundun", "kind": "monster",
             "numbers": [ {"number": 161, "maps": [24]},
                          {"number": 197, "maps": [{"map": 32, "stage": 6}, 27]},
                          {"number": 338} ] })"));

    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.monsters.size() == 1);
    const std::vector<MonsterDefinition>& numbers = read.monsters[0].numbers;
    REQUIRE(numbers.size() == 3);
    CHECK(numbers[0].maps == std::vector<Data::Maps::MapRef>{{24, 0}});
    // Sorted, a stage after its map's number.
    CHECK(numbers[1].maps == std::vector<Data::Maps::MapRef>{{27, 0}, {32, 6}});
    CHECK(numbers[2].maps.empty());
    // A monster has no maps of its own; each number has its maps.
    CHECK(read.monsters[0].shared.maps.empty());

    const std::string text = Data::Monsters::WriteMonstersJson(std::span<const MonsterGroup>(read.monsters));
    INFO(text);
    CHECK(text.find("{\"number\": 161, \"maps\": [24]},\n") != std::string::npos);
    CHECK(text.find("{\"number\": 197, \"maps\": [27, {\"map\": 32, \"stage\": 6}]},\n") != std::string::npos);
    CHECK(text.find("{\"number\": 338}\n") != std::string::npos);
    CHECK(ReadGroups(text).monsters == read.monsters);

    // A monster with one number writes its maps with the number too.
    MonsterDefinition budge = Monster(2, "Budge Dragon");
    budge.maps = {{0, 0}, {2, 0}};
    const std::string single = Data::Monsters::WriteMonstersJson(std::array{budge});
    CHECK(single.find("{\"number\": 2, \"maps\": [0, 2]}") != std::string::npos);
    CHECK(Read(single).monsters == std::vector{budge});
}

TEST_CASE("Monster data: maps that are no list of maps are an error, a monster's maps a warning [data][monsters]")
{
    const GroupsResult bad = ReadGroups(GroupFile(
        R"({ "name": "X", "kind": "monster", "numbers": [{"number": 7, "maps": 24}, {"number": 8, "maps": ["x"]}] })"));
    CHECK(HasIssueAbout(bad.issues, "monster 7 maps", ""));
    CHECK(HasIssueAbout(bad.issues, "monster 8 maps[0]", ""));
    CHECK(HasFatal(bad.issues));
    CHECK(bad.monsters.empty());

    const GroupsResult shared =
        ReadGroups(GroupFile(R"({ "name": "X", "kind": "monster", "maps": [24], "numbers": [{"number": 7}] })"));
    CHECK_FALSE(HasFatal(shared.issues));
    CHECK(HasIssueAbout(shared.issues, "monster 7 maps", "unknown field"));
    REQUIRE(shared.monsters.size() == 1);
    CHECK(shared.monsters[0].numbers[0].maps.empty());
}

// A talk text without a text of its own in one of the game's languages shows
// the English one there: a warning naming them (Q-MN47).
TEST_CASE("Monster data: a talk text missing a language is a warning (Q-MN47) [data][monsters]")
{
    MonsterDefinition santa = Monster(470, "Little Santa");
    Data::Monsters::MonsterTalkBlock message;
    message.kind = Data::Monsters::MonsterTalkBlock::Kind::Message;
    message.message.Set("en", "Health has been recovered.");
    message.message.Set("de", "Leben wiederhergestellt.");
    santa.setup.onTalk = {message};
    MonsterDefinition whole = santa;
    whole.number = 471;
    whole.setup.onTalk[0].message.Set("es", "Vida recuperada.");
    const std::array<const MonsterDefinition*, 2> monsters = {&santa, &whole};
    const std::array<const char*, 3> locales = {"en", "de", "es"};
    std::vector<ItemDataIssue> issues;

    Data::Monsters::MonsterTalkTexts shared;
    Data::Monsters::MonsterAltarContract& altar = shared["crywolfAltar"];
    for (Data::LocalizedString* text : Data::Monsters::AltarContractTexts(altar))
    {
        text->Set("en", "Text");
        text->Set("de", "Text");
        text->Set("es", "Texto");
    }
    altar.ongoing = Data::LocalizedString{};
    altar.ongoing.Set("en", "Busy.");

    Data::Monsters::ValidateMonsterTalkTexts(monsters, shared, locales, "Monsters.json", issues);

    REQUIRE(issues.size() == 2);
    CHECK(issues[0].severity == ItemDataIssueSeverity::Warning);
    CHECK(issues[0].field == "talkTexts.crywolfAltar.ongoing");
    CHECK(issues[0].message.find("de, es") != std::string::npos);
    CHECK(issues[1].field == "monster 470 onTalk[0].message");
    CHECK(issues[1].message.find("es") != std::string::npos);
}

// A variant's talk text is named as Monster Translations' Talk texts rows name
// it, so a field copied from the warning finds the row.
TEST_CASE("Monster data: a variant's talk text missing a language is named as its row [data][monsters]")
{
    MonsterDefinition guard = Monster(407, "Werewolf Guardsman");
    Data::Monsters::MonsterStateVariant state;
    state.state = "crywolfElfTime";
    state.setup.onTalk.emplace_back().kind = Data::Monsters::MonsterTalkBlock::Kind::Message;
    state.setup.onTalk[0].message.Set("en", "Elf time.");
    Data::Monsters::MonsterMapVariant variant;
    variant.maps = {Data::Maps::MapRef{34, 0}};
    variant.setup.stateVariants = {state};
    guard.mapVariants.push_back(variant);
    const std::array<const MonsterDefinition*, 1> monsters = {&guard};
    const std::array<const char*, 2> locales = {"en", "de"};
    std::vector<ItemDataIssue> issues;

    Data::Monsters::ValidateMonsterTalkTexts(monsters, {}, locales, "Monsters.json", issues);

    REQUIRE(issues.size() == 1);
    CHECK(issues[0].field == "monster 407 mapVariants[0].stateVariants[0].onTalk[0].message");
}

// Old behaviours the setup keeps are warnings (Q-MN36): `live` false drops the
// character when it appears (the Wolf Status, P24), wherever it is set.
TEST_CASE("Monster data: a setup that drops the monster when it appears is a warning (P24) [data][monsters]")
{
    MonsterDefinition statue = Monster(204, "Wolf Status");
    statue.setup.live = false;
    MonsterDefinition onMap = Monster(205, "Altar");
    Data::Monsters::MonsterMapVariant variant;
    variant.maps = {Data::Maps::MapRef{34, 0}};
    variant.setup.live = false;
    onMap.mapVariants.push_back(variant);
    MonsterDefinition inState = Monster(206, "Altar 2");
    Data::Monsters::MonsterStateVariant state;
    state.state = "crywolfReady";
    state.setup.live = false;
    inState.setup.stateVariants.push_back(state);
    MonsterDefinition shown = Monster(207, "Altar 3");
    shown.setup.live = true;
    const std::array<const MonsterDefinition*, 4> monsters = {&statue, &onMap, &inState, &shown};
    std::vector<ItemDataIssue> issues;

    Data::Monsters::ValidateMonsterSetups(monsters, "Monsters.json", issues);

    REQUIRE(issues.size() == 3);
    for (const ItemDataIssue& issue : issues)
    {
        CHECK(issue.severity == ItemDataIssueSeverity::Warning);
        CHECK(issue.message.find("(P24)") != std::string::npos);
    }
    CHECK(issues[0].number == 204);
    CHECK(issues[0].field == "monster 204 live");
    CHECK(issues[1].field == "monster 205 (map variant 1) live");
    CHECK(issues[2].field == "monster 206 (state crywolfReady) live");
}

// The talk steps as the editor leaves them: an OK quest without a state, a
// warning; an altar step naming a set that is not there, an error.
TEST_CASE("Monster data: Data Checks finds an OK quest without a state and an unknown talk text set [data][monsters]")
{
    MonsterDefinition guard = Monster(407, "Werewolf Guardsman");
    Data::Monsters::MonsterTalkBlock dialog;
    dialog.kind = Data::Monsters::MonsterTalkBlock::Kind::Dialog;
    dialog.dialog.ok.emplace().quest = "thirdChangeUp2";
    Data::Monsters::MonsterMapVariant variant;
    variant.maps = {Data::Maps::MapRef{34, 0}};
    variant.setup.onTalk = {dialog};
    guard.mapVariants.push_back(variant);
    MonsterDefinition altar = Monster(205, "Wolf Altar1");
    Data::Monsters::MonsterTalkBlock contract;
    contract.kind = Data::Monsters::MonsterTalkBlock::Kind::CrywolfAltarContract;
    contract.altarTexts = "gone";
    altar.setup.onTalk = {contract};
    MonsterDefinition named = Monster(206, "Wolf Altar2");
    contract.altarTexts = "crywolfAltar";
    named.setup.onTalk = {contract};
    Data::Monsters::MonsterTalkTexts texts;
    texts["crywolfAltar"];
    const std::array<const MonsterDefinition*, 3> monsters = {&guard, &altar, &named};
    std::vector<ItemDataIssue> issues;

    Data::Monsters::ValidateMonsterTalkSteps(monsters, texts, "Monsters.json", issues);

    REQUIRE(issues.size() == 2);
    CHECK(issues[0].severity == ItemDataIssueSeverity::Warning);
    CHECK(issues[0].field == "monster 407 mapVariants[0].onTalk[0].dialog.ok.if.is");
    CHECK(issues[1].severity == ItemDataIssueSeverity::Error);
    CHECK(issues[1].field == "monster 205 onTalk[0].crywolfAltarContract");
}

// The altar contract's code counts from the altars' models: on another model
// the step only talks, a warning.
TEST_CASE("Monster data: an altar contract step on a model that is no altar is a warning [data][monsters]")
{
    Data::Monsters::MonsterTalkBlock contract;
    contract.kind = Data::Monsters::MonsterTalkBlock::Kind::CrywolfAltarContract;
    MonsterDefinition altar = Monster(205, "Altar");
    altar.model = "MODEL_CRYWOLF_ALTAR1";
    altar.setup.onTalk = {contract};
    MonsterDefinition smith = Monster(251, "Hanzo");
    smith.model = "MODEL_SMITH";
    smith.setup.onTalk = {contract};
    MonsterDefinition onMap = Monster(206, "Altar 2");
    onMap.model = "MODEL_CRYWOLF_ALTAR2";
    Data::Monsters::MonsterMapVariant variant;
    variant.maps = {Data::Maps::MapRef{34, 0}};
    variant.model = "MODEL_SMITH";
    variant.setup.onTalk = {contract};
    onMap.mapVariants.push_back(variant);
    const std::array<const MonsterDefinition*, 3> monsters = {&altar, &smith, &onMap};
    std::vector<ItemDataIssue> issues;

    Data::Monsters::ValidateMonsterSetups(monsters, "Monsters.json", issues);

    REQUIRE(issues.size() == 2);
    CHECK(issues[0].severity == ItemDataIssueSeverity::Warning);
    CHECK(issues[0].field == "monster 251 onTalk");
    CHECK(issues[0].message.find("MODEL_SMITH") != std::string::npos);
    CHECK(issues[1].field == "monster 206 mapVariants[0].onTalk");
}
