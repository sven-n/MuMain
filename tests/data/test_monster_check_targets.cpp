#include "stdafx.h"

#include "doctest.h"

// What Monster Checks finds in the monster table and which monster tool a
// problem opens (MN1), without ImGui.
#ifdef _EDITOR

#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/DataHandler/MonsterData/MonsterModelJsonStorage.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonstersJson.h"
#include "UI/Common/MuEditorToolWindows.h"
#include "UI/Common/MuEditorWindowIds.h"
#include "UI/MonsterTools/MonsterCheckTargets.h"
#include "UI/MonsterTools/MonsterChecks.h"

#include <algorithm>
#include <array>
#include <string>
#include <string_view>
#include <vector>

using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterKind;
using MuEditor::Monsters::GetMonsterField;
using MuEditor::Monsters::GetToolForField;
using MuEditor::Monsters::MonsterTool;

namespace
{
// Longer than the 64 UTF-16 units a monster name holds in the game.
constexpr std::size_t LongNameLength = 65;

MonsterDefinition Monster(int number, const std::string& english, MonsterKind kind = MonsterKind::Monster)
{
    MonsterDefinition monster;
    monster.number = number;
    monster.names.Set("en", english);
    monster.kind = kind;
    return monster;
}

// The problem of the monster whose message has `messagePart`; nullptr when
// there is none.
const ItemDataIssue* FindIssue(const std::vector<ItemDataIssue>& issues, int number, std::string_view messagePart)
{
    const auto found =
        std::find_if(issues.begin(), issues.end(), [&](const ItemDataIssue& issue)
                     { return issue.number == number && issue.message.find(messagePart) != std::string::npos; });
    return found != issues.end() ? &*found : nullptr;
}

std::vector<ItemDataIssue> ReadFileIssues(const std::string& text)
{
    std::vector<MonsterDefinition> monsters;
    std::vector<ItemDataIssue> issues;
    Data::Monsters::ReadMonstersJson(text, "Monsters.json", monsters, issues);
    return issues;
}

const ItemDataIssue* FindField(const std::vector<ItemDataIssue>& issues, std::string_view field)
{
    const auto found = std::ranges::find(issues, field, &ItemDataIssue::field);
    return found != issues.end() ? &*found : nullptr;
}
} // namespace

TEST_CASE("Monster checks: a name opens Monster Translations, every other field the browser [editor][monsters]")
{
    CHECK(GetToolForField("name") == MonsterTool::Translations);
    CHECK(GetToolForField("name.de") == MonsterTool::Translations);
    CHECK(GetToolForField("monster 2 name") == MonsterTool::Translations);
    CHECK(GetToolForField("monster 2 name.pt") == MonsterTool::Translations);

    CHECK(GetToolForField("monster 2 kind") == MonsterTool::Browser);
    CHECK(GetToolForField("monster 2 number") == MonsterTool::Browser);
    // An unknown key, also one that only starts like the name's.
    CHECK(GetToolForField("monster 2 scale") == MonsterTool::Browser);
    CHECK(GetToolForField("monster 2 names") == MonsterTool::Browser);
    CHECK(GetToolForField("monsters[3].number") == MonsterTool::Browser);
    CHECK(GetToolForField("") == MonsterTool::Browser);
}

TEST_CASE("Monster checks: the field column leaves out the monster in front [editor][monsters]")
{
    CHECK(GetMonsterField("monster 2 name.de") == "name.de");
    CHECK(GetMonsterField("monster 1023 kind") == "kind");
    CHECK(GetMonsterField("monsters[3].number") == "monsters[3].number");
    CHECK(GetMonsterField("monster name") == "monster name");
    CHECK(GetMonsterField("monster 2") == "monster 2");
    CHECK(GetMonsterField("name") == "name");
    CHECK(GetMonsterField("").empty());
}

TEST_CASE("Monster checks: the problems of the monster list file name their monster and tool [editor][monsters]")
{
    const std::vector<ItemDataIssue> issues = ReadFileIssues(R"({ "formatVersion": 1, "monsters": [
        { "number": 5, "name": { "en": "Spider", "de": 3 }, "kind": "monster" },
        { "number": 6, "name": "Hound", "kind": "dragon" },
        { "number": 7, "name": "Lich", "kind": "monster", "scale": 2 },
        { "number": 7, "name": "Lich", "kind": "monster" },
        { "name": "Nobody", "kind": "monster" } ] })");

    const ItemDataIssue* name = FindField(issues, "monster 5 name.de");
    REQUIRE(name != nullptr);
    CHECK(name->number == 5);
    CHECK(GetToolForField(name->field) == MonsterTool::Translations);

    const ItemDataIssue* kind = FindField(issues, "monster 6 kind");
    REQUIRE(kind != nullptr);
    CHECK(kind->number == 6);
    CHECK(GetToolForField(kind->field) == MonsterTool::Browser);

    const ItemDataIssue* unknown = FindField(issues, "monster 7 scale");
    REQUIRE(unknown != nullptr);
    CHECK(unknown->number == 7);
    CHECK(GetToolForField(unknown->field) == MonsterTool::Browser);

    const ItemDataIssue* twice = FindField(issues, "monster 7 number");
    REQUIRE(twice != nullptr);
    CHECK(twice->number == 7);
    CHECK(GetToolForField(twice->field) == MonsterTool::Browser);

    // Without a number the problem is about no monster.
    const ItemDataIssue* noNumber = FindField(issues, "monsters[4].number");
    REQUIRE(noNumber != nullptr);
    CHECK(noNumber->number == ItemDataIssue::NoItem);
}

TEST_CASE("Monster checks: the tools of the targets are the monster tools' windows [editor][monsters]")
{
    using MuEditor::Monsters::GetToolWindow;
    CHECK(std::string_view(GetToolWindow(MonsterTool::Browser).windowId) == MuEditor::WindowIds::MonsterBrowser);
    CHECK(GetToolWindow(MonsterTool::Browser).open == &MuEditor::ToolWindows::monsterBrowser);
    CHECK(std::string_view(GetToolWindow(MonsterTool::Translations).windowId) ==
          MuEditor::WindowIds::MonsterTranslations);
    CHECK(GetToolWindow(MonsterTool::Translations).open == &MuEditor::ToolWindows::monsterTranslations);
    CHECK(std::string_view(GetToolWindow(MonsterTool::Attacks).windowId) == MuEditor::WindowIds::MonsterAttacks);
    CHECK(GetToolWindow(MonsterTool::Attacks).open == &MuEditor::ToolWindows::monsterAttacks);
}

TEST_CASE("Monster checks: the attacks go to Monster Attacks [editor][monsters]")
{
    CHECK(GetToolForField("monster 459 attacks") == MonsterTool::Attacks);
    CHECK(GetToolForField("monster 459 attacks.monsterSkills") == MonsterTool::Attacks);
    CHECK(GetToolForField("attacks") == MonsterTool::Attacks);
    CHECK(GetToolForField("monster 459 attackSpeed") == MonsterTool::Browser);
}

TEST_CASE("Monster checks: Run finds the name problems of the table and sends them to Monster Translations "
          "[editor][monsters]")
{
    MonsterDefinition unknownLanguage = Monster(9, "Thunder Lich");
    unknownLanguage.names.Set("zz", "Lich");
    const std::array monsters = {Monster(2, "Budge Dragon"), Monster(7, "Budge||Dragon"), unknownLanguage,
                                 Monster(11, std::string(LongNameLength, 'x')), Monster(252, "", MonsterKind::Npc)};
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterModelDatabase.Build({});
    std::vector<ItemDataIssue> issues;

    MuEditor::Monsters::MonsterChecks::Run(issues);

    REQUIRE(issues.size() == 4);
    const ItemDataIssue* separator = FindIssue(issues, 7, "must not contain");
    REQUIRE(separator != nullptr);
    CHECK(separator->severity == ItemDataIssueSeverity::Error);
    const ItemDataIssue* language = FindIssue(issues, 9, "never shown");
    REQUIRE(language != nullptr);
    CHECK(language->severity == ItemDataIssueSeverity::Warning);
    const ItemDataIssue* longName = FindIssue(issues, 11, "is cut in the game");
    REQUIRE(longName != nullptr);
    CHECK(longName->severity == ItemDataIssueSeverity::Warning);
    const ItemDataIssue* emptyName = FindIssue(issues, 252, "English name is empty");
    REQUIRE(emptyName != nullptr);
    CHECK(emptyName->severity == ItemDataIssueSeverity::Warning);

    for (const ItemDataIssue& issue : issues)
    {
        CHECK(GetMonsterField(issue.field) == "name");
        CHECK(GetToolForField(issue.field) == MonsterTool::Translations);
        CHECK(issue.source == Data::Monsters::GetMonstersFile().generic_string());
    }
}

TEST_CASE("Monster checks: Run lists a model whose file is not in the client folder (P1) [editor][monsters]")
{
    g_MonsterDatabase.Build({}, "en");
    Data::Monsters::MonsterModelDefinition molt;
    molt.slot = "MONSTER_MODEL_MOLT";
    molt.load.file = "Data/Monster/NoSuchMonster.bmd";
    g_MonsterModelDatabase.Build(std::array{molt});
    std::vector<ItemDataIssue> issues;

    MuEditor::Monsters::MonsterChecks::Run(issues);
    g_MonsterModelDatabase.Build({});

    REQUIRE(issues.size() == 1);
    CHECK(issues[0].severity == ItemDataIssueSeverity::Warning);
    CHECK(issues[0].number == ItemDataIssue::NoItem);
    CHECK(issues[0].source == Data::Monsters::GetMonsterModelsFile().generic_string());
    CHECK(issues[0].field.find("MONSTER_MODEL_MOLT") != std::string::npos);
    CHECK(issues[0].message.find("Data/Monster/NoSuchMonster.bmd is not in the client folder") != std::string::npos);
}

#endif // _EDITOR
