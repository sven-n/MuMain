#include "stdafx.h"

#include "doctest.h"

// The items the monsters wear (monsters design, D60): where an entry names an
// item, the move's update of those places, and the check of an item the item
// list does not have.

#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/MonsterData/MonsterItemReferences.h"

#include <set>
#include <string>
#include <vector>

using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Items::MakeItemType;
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterItem;
using Data::Monsters::MonsterItemPlace;
using Data::Monsters::MonsterItemWearer;

namespace
{
MonsterItem Item(int group, int number, int level = 0)
{
    MonsterItem item;
    item.group = group;
    item.number = number;
    item.level = level;
    return item;
}

// A Chaos Castle soldier's way of wearing things: its own crossbow, a helm on
// a map, armor while the siege runs, a random look with a staff, and a body
// that is no item.
MonsterDefinition Soldier()
{
    MonsterDefinition monster;
    monster.number = 163;
    monster.setup.equipment.weapons[0] = Item(4, 8, 7);
    monster.setup.equipment.weapons[0]->excellent = 1;
    monster.setup.equipment.pants = MonsterItem{};
    monster.setup.equipment.pants->part = "MODEL_BODY_PANTS+2";
    monster.mapVariants.emplace_back();
    monster.mapVariants[0].setup.equipment.helm = Item(7, 2);
    Data::Monsters::MonsterStateVariant siege;
    siege.state = "castleSiegeRunning";
    siege.setup.equipment.armor = Item(8, 2);
    monster.setup.stateVariants.push_back(siege);
    monster.randomVariants.emplace_back();
    monster.randomVariants[0].model = "MONSTER_MODEL_CHAOS_CASTLE_WIZARD";
    monster.randomVariants[0].equipment.emplace();
    monster.randomVariants[0].equipment->weapons[0] = Item(5, 3);
    monster.randomVariants[0].equipment->weapons[1] = Item(4, 8);
    return monster;
}
} // namespace

TEST_CASE("Monster items: the places an entry names an item, with their fields [data][monsters]")
{
    const MonsterDefinition monster = Soldier();
    std::vector<std::string> fields;
    Data::Monsters::ForEachMonsterItem(monster, [&](const MonsterItem&, const MonsterItemPlace& place)
                                       { fields.push_back(Data::Monsters::GetItemPlaceField(place)); });
    // The body that is no item is left out.
    CHECK(fields == std::vector<std::string>{
                        "equipment.weapons[0]", "mapVariants[0].equipment.helm", "stateVariants[0].equipment.armor",
                        "randomVariants[0].equipment.weapons[0]", "randomVariants[0].equipment.weapons[1]"});
}

// A map variant's state variants name items too: a rename or a removal of
// the item reaches them.
TEST_CASE("Monster items: a map variant's state variants are places too [data][monsters]")
{
    MonsterDefinition monster;
    monster.number = 163;
    monster.mapVariants.emplace_back();
    Data::Monsters::MonsterStateVariant state;
    state.state = "castleSiegeRunning";
    state.setup.equipment.helm = Item(7, 2);
    monster.mapVariants[0].setup.stateVariants.push_back(state);

    std::vector<std::string> fields;
    Data::Monsters::ForEachMonsterItem(monster, [&](const MonsterItem&, const MonsterItemPlace& place)
                                       { fields.push_back(Data::Monsters::GetItemPlaceField(place)); });
    CHECK(fields == std::vector<std::string>{"mapVariants[0].stateVariants[0].equipment.helm"});

    const std::vector<MonsterDefinition> monsters = {monster};
    const std::vector<MonsterItemWearer> helm = Data::Monsters::FindItemWearers(monsters, MakeItemType(7, 2));
    REQUIRE(helm.size() == 1);
    CHECK(helm[0].place.variant == MonsterItemPlace::Variant::MapState);
    CHECK(helm[0].place.mapIndex == 0);
    CHECK(helm[0].place.state == "castleSiegeRunning");
}

TEST_CASE("Monster items: the wearers of an item, by number and place [data][monsters]")
{
    MonsterDefinition other;
    other.number = 165;
    other.setup.equipment.weapons[1] = Item(4, 8);
    const std::vector<MonsterDefinition> monsters = {Soldier(), other};

    const std::vector<MonsterItemWearer> crossbow = Data::Monsters::FindItemWearers(monsters, MakeItemType(4, 8));
    REQUIRE(crossbow.size() == 3);
    CHECK(crossbow[0].number == 163);
    CHECK(crossbow[0].place.variant == MonsterItemPlace::Variant::None);
    CHECK(crossbow[0].place.slot == "weapons[0]");
    CHECK(crossbow[1].number == 163);
    CHECK(crossbow[1].place.variant == MonsterItemPlace::Variant::Random);
    CHECK(crossbow[1].place.index == 0);
    CHECK(crossbow[1].place.slot == "weapons[1]");
    CHECK(crossbow[2].number == 165);

    const std::vector<MonsterItemWearer> armor = Data::Monsters::FindItemWearers(monsters, MakeItemType(8, 2));
    REQUIRE(armor.size() == 1);
    CHECK(armor[0].place.variant == MonsterItemPlace::Variant::State);
    CHECK(armor[0].place.state == "castleSiegeRunning");

    CHECK(Data::Monsters::FindItemWearers(monsters, MakeItemType(8, 3)).empty());
}

TEST_CASE("Monster items: a move updates every place that names the item, its level stays [data][monsters]")
{
    MonsterDefinition monster = Soldier();
    CHECK(Data::Monsters::ReplaceItemReferences(monster, MakeItemType(4, 8), MakeItemType(4, 30)) == 2);
    CHECK(monster.setup.equipment.weapons[0]->group == 4);
    CHECK(monster.setup.equipment.weapons[0]->number == 30);
    CHECK(monster.setup.equipment.weapons[0]->level == 7);
    CHECK(monster.setup.equipment.weapons[0]->excellent == 1);
    CHECK(monster.randomVariants[0].equipment->weapons[1]->number == 30);
    CHECK(monster.randomVariants[0].equipment->weapons[0]->number == 3);
    CHECK(monster.setup.equipment.pants->part == "MODEL_BODY_PANTS+2");

    // Into another group.
    CHECK(Data::Monsters::ReplaceItemReferences(monster, MakeItemType(7, 2), MakeItemType(9, 2)) == 1);
    CHECK(monster.mapVariants[0].setup.equipment.helm->group == 9);
    CHECK(Data::Monsters::ReplaceItemReferences(monster, MakeItemType(7, 2), MakeItemType(9, 2)) == 0);
}

TEST_CASE("Monster items: an item the item list does not have is an error [data][monsters]")
{
    const MonsterDefinition monster = Soldier();
    const std::vector<const MonsterDefinition*> monsters = {&monster};
    const std::set<int> items = {MakeItemType(4, 8), MakeItemType(7, 2), MakeItemType(5, 3)};
    std::vector<ItemDataIssue> issues;
    Data::Monsters::ValidateMonsterItems(
        monsters, [&](int itemType) { return items.contains(itemType); }, "Monsters.json", issues);

    REQUIRE(issues.size() == 1);
    CHECK(issues[0].severity == ItemDataIssueSeverity::Error);
    CHECK(issues[0].number == 163);
    CHECK(issues[0].group == ItemDataIssue::NoItem);
    CHECK(issues[0].source == "Monsters.json");
    CHECK(issues[0].field == "monster 163 stateVariants[0].equipment.armor");
    CHECK(issues[0].message.find("item (8, 2)") != std::string::npos);
}

// The item tools' Save writes the references a move changed into the monster
// list as last saved (Q-MN41): a number's entry that names the saved item
// there; a shared value its numbers shared stays shared.
TEST_CASE("Monster items: the changes go into the monsters of the file [data][monsters]")
{
    using Data::Monsters::MonsterGroup;
    using Data::Monsters::MonsterItemChange;
    MonsterDefinition shared;
    shared.setup.equipment.weapons[0] = Item(4, 8);
    MonsterDefinition first = shared;
    first.number = 163;
    MonsterDefinition second = shared;
    second.number = 164;
    // Changed since by the monster tools: another item there.
    MonsterDefinition third = shared;
    third.number = 165;
    third.setup.equipment.weapons[0] = Item(4, 9);
    std::vector<MonsterGroup> groups = {{shared, {first, second, third}}};

    MonsterItemPlace weapon;
    weapon.slot = "weapons[0]";
    const int crossbow = MakeItemType(4, 8);
    const int moved = MakeItemType(4, 30);
    const std::vector<MonsterItemChange> changes = {
        {163, weapon, crossbow, moved}, {164, weapon, crossbow, moved}, {165, weapon, crossbow, moved}};
    REQUIRE(Data::Monsters::ApplyMonsterItemChanges(groups, changes));

    CHECK(Data::Monsters::FindItemAt(groups[0].numbers[0], weapon) == moved);
    CHECK(Data::Monsters::FindItemAt(groups[0].numbers[1], weapon) == moved);
    CHECK(Data::Monsters::FindItemAt(groups[0].numbers[2], weapon) == MakeItemType(4, 9));
    CHECK(Data::Monsters::FindItemAt(groups[0].shared, weapon) == moved);

    // Written already: nothing changes.
    CHECK_FALSE(Data::Monsters::ApplyMonsterItemChanges(groups, changes));
    MonsterItemPlace helm;
    helm.slot = "helm";
    CHECK_FALSE(Data::Monsters::FindItemAt(groups[0].shared, helm).has_value());
}
