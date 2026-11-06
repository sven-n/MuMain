#include "stdafx.h"

#include "doctest.h"

// The monster tools' changes of monsters with their numbers (D61, PR 3a):
// shared values, the Browser's number actions and their undo steps.
#ifdef _EDITOR

#include "ClientDataScope.h"
#include "TemporaryMonsterFolder.h"

#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterSoundSetDatabase.h"
#include "UI/MonsterTools/MonsterEdits.h"

#include <optional>
#include <string>
#include <vector>

using Data::Items::ItemDataIssue;
using Data::Maps::MapRef;
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterKind;

namespace
{
// Budge Dragon (2) is a monster of its own; Death Centurion has 145, 175 and
// 336, which attacks with the generic pattern as its own value.
constexpr char GroupedMonstersJson[] = R"({ "formatVersion": 2, "monsters": [
    { "name": "Budge Dragon", "kind": "monster", "numbers": [{"number": 2, "maps": [0]}] },
    { "name": {"en": "Death Centurion", "pt": "Centurião da morte"}, "kind": "monster",
      "model": "MONSTER_MODEL_DEATH_CENTURION", "soundSet": "Centurion",
      "numbers": [{"number": 145, "maps": [24]}, {"number": 175, "maps": [25]},
                  {"number": 336, "maps": [36], "attacks": {"otherwise": "generic", "clearSkill": "never"}}] } ] })";

// The grouped monsters read by the handler, with an empty history.
class GroupedMonsters
{
public:
    GroupedMonsters() : m_scope(m_folder.Path())
    {
        m_folder.WriteMonsters(GroupedMonstersText());
        std::vector<ItemDataIssue> issues;
        REQUIRE(g_MonsterDataHandler.Revert(issues));
        g_MonsterEdits.OnDataReplaced();
        g_MonsterEdits.GetHistory().MarkSaved();
    }

    const TemporaryMonsterFolder& Folder() const
    {
        return m_folder;
    }

    static std::string GroupedMonstersText()
    {
        return GroupedMonstersJson;
    }

private:
    TemporaryMonsterFolder m_folder;
    ClientDataScope m_scope;
};

MonsterDefinition Entry(int number)
{
    const MonsterDefinition* entry = g_MonsterDatabase.Find(number);
    REQUIRE(entry != nullptr);
    return *entry;
}

// The monsters of the saved file, by their numbers.
std::vector<std::vector<int>> SavedNumbers(const TemporaryMonsterFolder& folder)
{
    std::vector<ItemDataIssue> issues;
    REQUIRE(g_MonsterEdits.Save(issues));
    const Data::Monsters::MonsterDataLoadResult saved = Data::Monsters::LoadMonstersFile(folder.MonstersFile());
    std::vector<std::vector<int>> numbers;
    for (const Data::Monsters::MonsterGroup& group : saved.groups)
    {
        std::vector<int>& monster = numbers.emplace_back();
        for (const MonsterDefinition& entry : group.numbers)
        {
            monster.push_back(entry.number);
        }
    }
    return numbers;
}
} // namespace

TEST_CASE("Monster edits: a number's monster and the values its numbers share [data][monsters][editor]")
{
    GroupedMonsters data;
    CHECK(g_MonsterEdits.ReadMonsterNumbers(175) == std::vector{145, 175, 336});
    CHECK(g_MonsterEdits.ReadMonsterNumbers(2) == std::vector{2});
    CHECK(g_MonsterEdits.ReadMonsterNumbers(3).empty());
    CHECK(g_MonsterEdits.ReadMonsters() == std::vector<std::vector<int>>{{2}, {145, 175, 336}});
    CHECK(g_MonsterEdits.ReadMaps(std::vector{336, 145, 3}) == std::vector<MapRef>{{24, 0}, {36, 0}});
    // How many and the first of them, without a copy.
    CHECK(g_MonsterEdits.CountMonsterNumbers(336) == 3);
    CHECK(g_MonsterEdits.FindFirstNumber(336) == 145);
    CHECK(g_MonsterEdits.CountMonsterNumbers(2) == 1);
    CHECK(g_MonsterEdits.FindFirstNumber(2) == 2);
    CHECK(g_MonsterEdits.CountMonsterNumbers(3) == 0);
    CHECK(g_MonsterEdits.FindFirstNumber(3) == -1);

    const std::optional<MonsterDefinition> shared = g_MonsterEdits.ReadSharedValues(336);
    REQUIRE(shared);
    CHECK(shared->names.Get("pt") == "Centurião da morte");
    CHECK_FALSE(shared->attacks);
    CHECK(shared->maps.empty());
    CHECK(shared->number == 0);
    CHECK(Entry(336).attacks);

    // A monster of its own shares its entry's values.
    MonsterDefinition budge = Entry(2);
    budge.number = 0;
    budge.maps.clear();
    CHECK(g_MonsterEdits.ReadSharedValues(2) == budge);
    CHECK_FALSE(g_MonsterEdits.ReadSharedValues(3));
}

TEST_CASE("Monster edits: a shared value reaches the numbers that share it, own values stay (Q-MN96) "
          "[data][monsters][editor]")
{
    GroupedMonsters data;
    const std::optional<Data::Monsters::MonsterAttacks> generic = Entry(336).attacks;
    MonsterDefinition own = Entry(175);
    own.names.Set("en", "Rogue Centurion");
    REQUIRE(g_MonsterEdits.SetMonster("own name", own));

    MonsterDefinition values = *g_MonsterEdits.ReadSharedValues(145);
    values.kind = MonsterKind::Npc;
    values.names.Set("en", "Death Centurion II");
    values.attacks = generic;
    // Its number and maps are not shared values.
    values.number = 999;
    values.maps = {{7, 0}};
    REQUIRE(g_MonsterEdits.SetSharedValues("shared", 175, values));

    CHECK(Entry(145).kind == MonsterKind::Npc);
    CHECK(Entry(175).kind == MonsterKind::Npc);
    CHECK(Entry(145).names.GetNeutral() == "Death Centurion II");
    CHECK(Entry(175).names.GetNeutral() == "Rogue Centurion");
    CHECK(Entry(336).names.GetNeutral() == "Death Centurion II");
    CHECK(Entry(145).attacks == generic);
    CHECK(Entry(336).attacks == generic);
    CHECK(Entry(145).maps == std::vector<MapRef>{{24, 0}});
    CHECK(Entry(145).number == 145);
    CHECK(g_MonsterDatabase.Find(999) == nullptr);
    CHECK(g_MonsterEdits.ReadSharedValues(336)->kind == MonsterKind::Npc);
    CHECK(g_MonsterEdits.ReadSharedValues(336)->maps.empty());

    // One step, undone and redone as a whole.
    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() == "shared");
    REQUIRE(g_MonsterEdits.Undo());
    CHECK(Entry(145).kind == MonsterKind::Monster);
    CHECK(Entry(336).names.GetNeutral() == "Death Centurion");
    CHECK_FALSE(Entry(145).attacks);
    CHECK(Entry(336).attacks == generic);
    CHECK(g_MonsterEdits.ReadSharedValues(145)->kind == MonsterKind::Monster);
    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() == "own name");
    REQUIRE(g_MonsterEdits.Redo());
    CHECK(Entry(336).kind == MonsterKind::Npc);

    // 336's attacks equal the shared ones now: the file writes them once.
    const std::vector<std::vector<int>> saved = SavedNumbers(data.Folder());
    const Data::Monsters::MonsterDataLoadResult file = Data::Monsters::LoadMonstersFile(data.Folder().MonstersFile());
    REQUIRE(file.groups.size() == 2);
    CHECK(file.groups[1].shared.attacks == generic);
    CHECK(file.groups[1].numbers[2] == Entry(336));
    CHECK(file.groups[1].numbers[1].names.GetNeutral() == "Rogue Centurion");
}

TEST_CASE("Monster edits: a monster with one number takes shared values as its entry [data][monsters][editor]")
{
    GroupedMonsters data;
    MonsterDefinition values = *g_MonsterEdits.ReadSharedValues(2);
    values.kind = MonsterKind::Trap;
    REQUIRE(g_MonsterEdits.SetSharedValues("kind", 2, values));
    CHECK(Entry(2).kind == MonsterKind::Trap);
    CHECK(Entry(2).maps == std::vector<MapRef>{{0, 0}});
    CHECK_FALSE(g_MonsterEdits.SetSharedValues("none", 3, values));

    values.names.Set("en", "Budge||Dragon");
    CHECK_FALSE(g_MonsterEdits.SetSharedValues("separator", 145, values));
    CHECK(Entry(145).names.GetNeutral() == "Death Centurion");
}

TEST_CASE("Monster edits: Add number and Copy number put a new number into the monster (Q-MN99) "
          "[data][monsters][editor]")
{
    GroupedMonsters data;
    REQUIRE(g_MonsterEdits.AddNumber("add", 175, 600));
    CHECK(g_MonsterEdits.ReadMonsterNumbers(600) == std::vector{145, 175, 336, 600});
    MonsterDefinition added = *g_MonsterEdits.ReadSharedValues(145);
    added.number = 600;
    CHECK(Entry(600) == added);
    CHECK(Entry(600).maps.empty());

    // A copy keeps the number's own values, not its maps.
    REQUIRE(g_MonsterEdits.CopyNumber("copy", 336, 601));
    MonsterDefinition copied = Entry(336);
    copied.number = 601;
    copied.maps.clear();
    CHECK(Entry(601) == copied);
    CHECK(g_MonsterEdits.ReadMonsterNumbers(145).size() == 5);

    CHECK_FALSE(g_MonsterEdits.AddNumber("taken", 145, 2));
    CHECK_FALSE(g_MonsterEdits.CopyNumber("no entry", 3, 602));
    CHECK_FALSE(g_MonsterEdits.AddNumber("outside", 145, 1024));

    // Each is one step: the number and its place in the monster.
    REQUIRE(g_MonsterEdits.Undo());
    CHECK(g_MonsterDatabase.Find(601) == nullptr);
    CHECK(g_MonsterEdits.ReadMonsterNumbers(145) == std::vector{145, 175, 336, 600});
    REQUIRE(g_MonsterEdits.Undo());
    CHECK(g_MonsterDatabase.Find(600) == nullptr);
    CHECK(g_MonsterEdits.ReadMonsterNumbers(145) == std::vector{145, 175, 336});
    CHECK_FALSE(g_MonsterEdits.Undo());

    // A number added to a monster of its own makes a monster of two, which
    // shares the first one's values.
    REQUIRE(g_MonsterEdits.AddNumber("second", 2, 3));
    CHECK(g_MonsterEdits.ReadMonsterNumbers(3) == std::vector{2, 3});
    CHECK(Entry(3).names.GetNeutral() == "Budge Dragon");
    CHECK(SavedNumbers(data.Folder()) == std::vector<std::vector<int>>{{2, 3}, {145, 175, 336}});
}

TEST_CASE("Monster edits: Move number and Split number out keep the number's values (Q-MN97) "
          "[data][monsters][editor]")
{
    GroupedMonsters data;
    const MonsterDefinition centurion = Entry(336);
    REQUIRE(g_MonsterEdits.MoveNumber("move", 336, 2));
    CHECK(Entry(336) == centurion);
    CHECK(g_MonsterEdits.ReadMonsterNumbers(2) == std::vector{2, 336});
    CHECK(g_MonsterEdits.ReadMonsterNumbers(145) == std::vector{145, 175});
    // Budge Dragon's values are the shared ones of the monster it joined.
    CHECK(g_MonsterEdits.ReadSharedValues(336)->names.GetNeutral() == "Budge Dragon");
    CHECK_FALSE(g_MonsterEdits.MoveNumber("same monster", 336, 2));
    CHECK_FALSE(g_MonsterEdits.MoveNumber("no entry", 336, 3));
    CHECK(SavedNumbers(data.Folder()) == std::vector<std::vector<int>>{{2, 336}, {145, 175}});
    CHECK(Entry(336) == centurion);

    REQUIRE(g_MonsterEdits.Undo());
    CHECK(g_MonsterEdits.ReadMonsterNumbers(336) == std::vector{145, 175, 336});
    CHECK(g_MonsterEdits.ReadMonsterNumbers(2) == std::vector{2});

    REQUIRE(g_MonsterEdits.SplitNumber("split", 175));
    CHECK(g_MonsterEdits.ReadMonsterNumbers(175) == std::vector{175});
    CHECK(g_MonsterEdits.ReadMonsterNumbers(145) == std::vector{145, 336});
    CHECK_FALSE(g_MonsterEdits.SplitNumber("alone", 175));
    CHECK_FALSE(g_MonsterEdits.SplitNumber("alone", 2));
    // A number of no monster is in the same monster as itself.
    CHECK_FALSE(g_MonsterEdits.MoveNumber("onto itself", 175, 175));
    CHECK(g_MonsterEdits.ReadMonsterNumbers(175) == std::vector{175});
    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() == "split");
    CHECK(SavedNumbers(data.Folder()) == std::vector<std::vector<int>>{{2}, {145, 336}, {175}});

    REQUIRE(g_MonsterEdits.Undo());
    CHECK(g_MonsterEdits.ReadMonsterNumbers(175) == std::vector{145, 175, 336});

    // The monster's last number moves out: the monster is gone.
    REQUIRE(g_MonsterEdits.MoveNumber("one", 2, 145));
    CHECK(g_MonsterEdits.ReadMonsterNumbers(2) == std::vector{2, 145, 175, 336});
    CHECK(SavedNumbers(data.Folder()) == std::vector<std::vector<int>>{{2, 145, 175, 336}});
}

TEST_CASE("Monster edits: removing a number leaves its monster, Remove monster removes all its numbers "
          "[data][monsters][editor]")
{
    GroupedMonsters data;
    // Only monsters with several numbers are listed: Budge Dragon has no id.
    CHECK(g_MonsterDataHandler.FindGroupOf(145));
    CHECK_FALSE(g_MonsterDataHandler.FindGroupOf(2));
    REQUIRE(g_MonsterEdits.RemoveMonster("number", 175));
    CHECK(g_MonsterEdits.ReadMonsterNumbers(145) == std::vector{145, 336});
    // A monster left with one number is not listed any more.
    REQUIRE(g_MonsterEdits.RemoveMonster("other number", 336));
    CHECK_FALSE(g_MonsterDataHandler.FindGroupOf(145));
    CHECK(g_MonsterEdits.ReadSharedValues(145)->names == Entry(145).names);
    REQUIRE(g_MonsterEdits.Undo());
    CHECK(g_MonsterDataHandler.FindGroupOf(145));
    // A new number on the free number is a monster of its own.
    MonsterDefinition larva;
    larva.number = 175;
    larva.names.Set("en", "Larva");
    REQUIRE(g_MonsterEdits.AddMonster("larva", larva));
    CHECK(g_MonsterEdits.ReadMonsterNumbers(175) == std::vector{175});
    REQUIRE(g_MonsterEdits.Undo());
    REQUIRE(g_MonsterEdits.Undo());
    CHECK(g_MonsterEdits.ReadMonsterNumbers(175) == std::vector{145, 175, 336});

    REQUIRE(g_MonsterEdits.RemoveMonsterWithNumbers("monster", 336));
    CHECK(g_MonsterDatabase.Find(145) == nullptr);
    CHECK(g_MonsterDatabase.Find(175) == nullptr);
    CHECK(g_MonsterDatabase.Find(336) == nullptr);
    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() == "monster");
    CHECK_FALSE(g_MonsterEdits.RemoveMonsterWithNumbers("again", 336));
    REQUIRE(g_MonsterEdits.Undo());
    CHECK(g_MonsterEdits.ReadMonsterNumbers(336) == std::vector{145, 175, 336});
    CHECK(Entry(145).maps == std::vector<MapRef>{{24, 0}});

    // Its last number removed, a monster is gone.
    REQUIRE(g_MonsterEdits.RemoveMonster("budge", 2));
    CHECK(SavedNumbers(data.Folder()) == std::vector<std::vector<int>>{{145, 175, 336}});
}

TEST_CASE("Monster edits: the actions work on a table built without the handler [data][monsters][editor]")
{
    GroupedMonsters data;
    MonsterDefinition first;
    first.number = 10;
    first.names.Set("en", "Hound");
    MonsterDefinition second = first;
    second.number = 11;
    g_MonsterDatabase.Build(std::vector{first, second}, "en");
    g_MonsterEdits.OnDataReplaced();

    // The monsters read before are not this table's: each number is a monster
    // of its own.
    CHECK(g_MonsterEdits.ReadMonsterNumbers(10) == std::vector{10});
    CHECK_FALSE(g_MonsterDataHandler.FindGroupOf(145));
    REQUIRE(g_MonsterEdits.MoveNumber("move", 11, 10));
    CHECK(g_MonsterEdits.ReadMonsterNumbers(10) == std::vector{10, 11});
    REQUIRE(g_MonsterEdits.Undo());
    CHECK(g_MonsterEdits.ReadMonsterNumbers(10) == std::vector{10});
    CHECK(g_MonsterEdits.ReadMonsterNumbers(11) == std::vector{11});
}

TEST_CASE("Monster edits: a value every number has alike becomes the monster's again [data][monsters][editor]")
{
    GroupedMonsters data;
    g_MonsterEdits.BeginGroup("import");
    for (const int number : {145, 175, 336})
    {
        REQUIRE(g_MonsterEdits.SetMonsterName("name", number, "es", "Centurión"));
    }
    CHECK_FALSE(g_MonsterEdits.ReadSharedValues(145)->names.HasOwn("es"));
    REQUIRE(g_MonsterEdits.ShareEqualValues("share", 145, Data::Monsters::MonsterValueField::Names));
    g_MonsterEdits.EndGroup();
    CHECK(g_MonsterEdits.ReadSharedValues(336)->names.Get("es") == "Centurión");
    CHECK(Entry(175).names == g_MonsterEdits.ReadSharedValues(175)->names);
    CHECK_FALSE(g_MonsterEdits.ShareEqualValues("again", 145, Data::Monsters::MonsterValueField::Names));
    // 336's attacks are its own: the values differ.
    CHECK_FALSE(g_MonsterEdits.ShareEqualValues("attacks", 145, Data::Monsters::MonsterValueField::Attacks));
    CHECK_FALSE(g_MonsterEdits.ShareEqualValues("one number", 2, Data::Monsters::MonsterValueField::Names));

    // One step with the names.
    REQUIRE(g_MonsterEdits.Undo());
    CHECK_FALSE(Entry(175).names.HasOwn("es"));
    CHECK_FALSE(g_MonsterEdits.ReadSharedValues(175)->names.HasOwn("es"));
}

TEST_CASE("Monster edits: renaming a sound set renames it in the shared values too [data][monsters][editor]")
{
    GroupedMonsters data;
    Data::Monsters::MonsterSoundSet set;
    set.name = "Centurion";
    g_MonsterSoundSetDatabase.Build(std::vector{set});
    REQUIRE(g_MonsterEdits.RenameSoundSet("rename", "Centurion", "Death Centurion", 145));
    CHECK(Entry(145).soundSet == "Death Centurion");
    CHECK(g_MonsterEdits.ReadSharedValues(336)->soundSet == "Death Centurion");
    REQUIRE(g_MonsterEdits.Undo());
    CHECK(g_MonsterEdits.ReadSharedValues(336)->soundSet == "Centurion");
    CHECK(Entry(336).soundSet == "Centurion");
    g_MonsterSoundSetDatabase.Build({});
}

#endif // _EDITOR
