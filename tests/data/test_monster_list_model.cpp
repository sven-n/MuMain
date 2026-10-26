#include "stdafx.h"

#include "doctest.h"

// The monsters the monster tools list, their search and filters, and the
// numbers a new monster can take (MN1).
#ifdef _EDITOR
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "UI/MonsterTools/MonsterListModel.h"

#include <vector>

using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterKind;
using MuEditor::Monsters::MonsterListModel;

namespace
{
MonsterDefinition Monster(int number, const char* names, MonsterKind kind = MonsterKind::Monster)
{
    MonsterDefinition monster;
    monster.number = number;
    monster.names = Data::LocalizedString::Parse(names);
    monster.kind = kind;
    return monster;
}

const std::vector<MonsterDefinition> Monsters = {
    Monster(2, "Budge Dragon||pt=Drag\xC3\xA3o Lento||de=Lindwurm"),
    Monster(9, "Lich||de=Lich"),
    Monster(229, "Marlon||pt=Marlon", MonsterKind::Npc),
    // The empty name of the original name files.
    Monster(252, ""),
    Monster(523, "Trap", MonsterKind::Trap),
};

void BuildMonsters()
{
    g_MonsterDatabase.Build(Monsters, "en");
}

std::vector<int> List(MonsterListModel& list, const MonsterListModel::Filter& filter)
{
    return list.GetNumbers(filter);
}
} // namespace

TEST_CASE("The monster list finds monsters by their number and any of their names [editor][monsters]")
{
    using MuEditor::Monsters::MatchesMonsterSearch;
    const MonsterDefinition& budgeDragon = Monsters[0];
    CHECK(MatchesMonsterSearch(budgeDragon, ""));
    CHECK(MatchesMonsterSearch(budgeDragon, "2"));
    // The whole text is the number: 22 or 2a are not monster 2.
    CHECK_FALSE(MatchesMonsterSearch(budgeDragon, "22"));
    CHECK_FALSE(MatchesMonsterSearch(budgeDragon, "2a"));
    CHECK(MatchesMonsterSearch(budgeDragon, "lindwurm"));
    CHECK(MatchesMonsterSearch(budgeDragon, "BUDGE"));
    CHECK(MatchesMonsterSearch(budgeDragon, "Drag\xC3\xA3o"));
    // Only A to Z ignore case: an upper-case letter with an accent is another letter.
    CHECK_FALSE(MatchesMonsterSearch(budgeDragon, "DRAG\xC3\x83O"));
    CHECK_FALSE(MatchesMonsterSearch(budgeDragon, "Lich"));
}

TEST_CASE("The monster list lists the monsters in number order, by search, kind and missing name "
          "[editor][monsters]")
{
    BuildMonsters();
    MonsterListModel list;

    CHECK(List(list, {}) == std::vector<int>{2, 9, 229, 252, 523});
    CHECK(List(list, {"9", "", std::nullopt}) == std::vector<int>{9});
    CHECK(List(list, {"marlon", "", std::nullopt}) == std::vector<int>{229});
    CHECK(List(list, {"", "", MonsterKind::Npc}) == std::vector<int>{229});
    CHECK(List(list, {"", "", MonsterKind::Trap}) == std::vector<int>{523});
    CHECK(List(list, {"", "", MonsterKind::Other}).empty());
    // A name of its own in the language: English stands in for the others.
    CHECK(List(list, {"", "pt", std::nullopt}) == std::vector<int>{9, 252, 523});
    CHECK(List(list, {"", "de", MonsterKind::Monster}) == std::vector<int>{252});
    CHECK(List(list, {"", "en", std::nullopt}) == std::vector<int>{252});
}

TEST_CASE("The monster list follows the monster table, and keeps its rows after an own edit [editor][monsters]")
{
    BuildMonsters();
    MonsterListModel list;
    const MonsterListModel::Filter monsters{"", "", MonsterKind::Monster};
    REQUIRE(List(list, monsters) == std::vector<int>{2, 9, 252});

    // A change of the table shows at once.
    MonsterDefinition lich = Monsters[1];
    lich.kind = MonsterKind::Npc;
    g_MonsterDatabase.Set(lich);
    CHECK(List(list, monsters) == std::vector<int>{2, 252});

    // An edit made through the list keeps the rows until the filter changes or
    // Refresh.
    MonsterDefinition budgeDragon = Monsters[0];
    budgeDragon.kind = MonsterKind::Npc;
    g_MonsterDatabase.Set(budgeDragon);
    list.KeepRowsAfterOwnEdit();
    CHECK(List(list, monsters) == std::vector<int>{2, 252});
    list.Refresh();
    CHECK(List(list, monsters) == std::vector<int>{252});

    g_MonsterDatabase.Remove(252);
    CHECK(List(list, monsters).empty());
    CHECK(List(list, {}) == std::vector<int>{2, 9, 229, 523});
}

TEST_CASE("A new monster takes a number 0-1023 without an entry [editor][monsters]")
{
    using namespace MuEditor::Monsters;
    BuildMonsters();

    CHECK(IsFreeMonsterNumber(0));
    CHECK(IsFreeMonsterNumber(1023));
    CHECK_FALSE(IsFreeMonsterNumber(2));
    // An entry with an empty name is an entry.
    CHECK_FALSE(IsFreeMonsterNumber(252));
    CHECK_FALSE(IsFreeMonsterNumber(-1));
    CHECK_FALSE(IsFreeMonsterNumber(1024));

    const std::vector<int> free = GetFreeMonsterNumbers();
    CHECK(free.size() == static_cast<size_t>(Data::Monsters::MonsterNumberCount) - Monsters.size());
    CHECK(free.front() == 0);
    CHECK(free.back() == 1023);

    CHECK(GetNextFreeMonsterNumber(0) == 0);
    CHECK(GetNextFreeMonsterNumber(2) == 3);
    CHECK(GetNextFreeMonsterNumber(252) == 253);
    // Past the last number it starts again at the first free one.
    CHECK(GetNextFreeMonsterNumber(1024) == 0);
}

#endif // _EDITOR
