#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "UI/ItemTools/ItemStatFileChange.h"

#include "Data/GameData/ItemData/ItemType.h"

#include <string>
#include <vector>

using namespace MuEditor::Items;
using namespace Data::Items;

namespace
{
ItemDefinition MakeShield(int number, const std::string& profile)
{
    ItemDefinition shield;
    shield.group = 6;
    shield.number = number;
    shield.names.Set("en", "Shield " + std::to_string(number));
    shield.weaponKind = WeaponKind::Shield;
    shield.statProfile = profile;
    return shield;
}

StatProfileRule ShieldRule(const std::string& profile)
{
    StatProfileRule rule{.profile = profile};
    rule.weaponKinds.set(static_cast<size_t>(WeaponKind::Shield));
    return rule;
}

ItemStatFile MakeFile(std::vector<StatProfileRule> rules)
{
    ItemStatFile file;
    for (const char* name : {"guard", "shield", "wall"})
    {
        WrittenStatProfile profile;
        profile.name = name;
        file.profiles.push_back(profile);
    }
    file.profileRules = std::move(rules);
    return file;
}
} // namespace

TEST_CASE("A change of the profile rules gives the items that follow them the new profile [editor][items]")
{
    const ItemStatProfiles before(MakeFile({ShieldRule("shield")}));
    const ItemStatProfiles after(MakeFile({ShieldRule("guard")}));
    // Follows the rules; picked; picked "default"; an item file saved before the field.
    const std::vector<ItemDefinition> items = {MakeShield(0, "shield"), MakeShield(1, "wall"), MakeShield(2, "default"),
                                               MakeShield(3, "")};

    const std::vector<StatProfileChange> changes = FindStatProfileChanges(items, before, after);
    REQUIRE(changes.size() == 2);
    CHECK(changes[0].itemType == MakeItemType(6, 0));
    CHECK(changes[0].from == "shield");
    CHECK(changes[0].to == "guard");
    CHECK(changes[1].itemType == MakeItemType(6, 3));
    CHECK(changes[1].from.empty());
    CHECK(changes[1].to == "guard");

    // Nothing changes where the rules give the same.
    CHECK(FindStatProfileChanges(items, before, before).empty());

    // A rule that no longer matches gives the default.
    const ItemStatProfiles noRules(MakeFile({}));
    const std::vector<StatProfileChange> dropped = FindStatProfileChanges(items, before, noRules);
    REQUIRE(dropped.size() == 2);
    CHECK(dropped[0].to.empty());

    // An item that named none while no rule matched takes a new rule.
    const std::vector<StatProfileChange> added = FindStatProfileChanges(items, noRules, after);
    REQUIRE(added.size() == 1);
    CHECK(added[0].itemType == MakeItemType(6, 3));

    const std::vector<ItemDefinition> applied = ApplyStatProfileChanges(items, changes);
    REQUIRE(applied.size() == 2);
    CHECK(applied[0].statProfile == "guard");
    CHECK(applied[0].names.GetNeutral() == "Shield 0");
}

TEST_CASE("Renaming a stat profile renames the rules and every item that names it [editor][items]")
{
    ItemStatFile file = MakeFile({ShieldRule("shield")});
    const ItemStatProfiles before(file);
    RenameStatProfile(file, "shield", "bulwark");
    CHECK(file.profiles[1].name == "bulwark");
    CHECK(file.profileRules[0].profile == "bulwark");
    const ItemStatProfiles after(file);

    const std::vector<ItemDefinition> items = {MakeShield(0, "shield"), MakeShield(1, "wall")};
    ItemDefinition pickedShield = MakeShield(2, "shield");
    pickedShield.weaponKind = WeaponKind::None;
    std::vector<ItemDefinition> all = items;
    all.push_back(pickedShield);

    const std::vector<StatProfileChange> changes = FindStatProfileChanges(all, before, after, "shield", "bulwark");
    REQUIRE(changes.size() == 2);
    CHECK(changes[0].to == "bulwark");
    // A pick of the profile goes with the name too.
    CHECK(changes[1].itemType == MakeItemType(6, 2));
    CHECK(changes[1].to == "bulwark");
}

TEST_CASE("A change of the item stat file reaches the users of the profiles whose terms change [editor][items]")
{
    ItemStatFile file = MakeFile({});
    LevelBonusTable steps;
    steps.name = "steps";
    steps.values = {0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 31, 36, 42, 49, 57, 66};
    file.tables.push_back(steps);
    file.defaults[static_cast<size_t>(ItemStat::Defense)].table = "steps";
    file.defaults[static_cast<size_t>(ItemStat::Defense)].excellent = {WrittenStatTerm::State::Values, {12, 4, 5}};
    // "wall" writes its own excellent defense term.
    file.profiles[2].stats[static_cast<size_t>(ItemStat::Defense)].excellent = {WrittenStatTerm::State::Values,
                                                                                {1, 1, 0}};
    const ItemStatProfiles before(file);
    CHECK(FindProfilesWithChangedTerms(before, before).empty());

    // The default's excellent defense reaches every profile but the one that writes its own.
    ItemStatFile excellent = file;
    excellent.defaults[static_cast<size_t>(ItemStat::Defense)].excellent.values.add = 5;
    CHECK(FindProfilesWithChangedTerms(before, ItemStatProfiles(excellent)) ==
          std::vector<std::string>{"", "guard", "shield"});

    // A table reaches the profiles that use it, also above +15.
    ItemStatFile table = file;
    table.tables[0].perLevel = 1;
    CHECK(FindProfilesWithChangedTerms(before, ItemStatProfiles(table)).size() == 4);

    // The drop level bonuses reach every profile.
    ItemStatFile bonus = file;
    bonus.ancientDropLevelBonus = 30;
    CHECK(FindProfilesWithChangedTerms(before, ItemStatProfiles(bonus)).size() == 4);

    // A new profile counts.
    ItemStatFile added = file;
    WrittenStatProfile tower;
    tower.name = "tower";
    added.profiles.push_back(tower);
    CHECK(FindProfilesWithChangedTerms(before, ItemStatProfiles(added)) == std::vector<std::string>{"tower"});

    const std::vector<ItemDefinition> items = {MakeShield(0, "shield"), MakeShield(1, "wall"), MakeShield(2, "default"),
                                               MakeShield(3, "")};
    const std::vector<std::string> changedProfiles = {"", "wall"};
    const std::vector<StatProfileChange> changes = {{MakeItemType(6, 0), "shield", "guard"}};
    CHECK(FindItemsReached(items, changes, changedProfiles) ==
          std::vector<int>{MakeItemType(6, 0), MakeItemType(6, 1), MakeItemType(6, 2), MakeItemType(6, 3)});
    CHECK(FindItemsReached(items, {}, std::vector<std::string>{"shield"}) == std::vector<int>{MakeItemType(6, 0)});
}

TEST_CASE("Renaming a table renames it in the descriptions that name it [editor][items]")
{
    const auto describe = [](ItemDefinition item, const char* english, const char* german)
    {
        ItemDescriptionLine line;
        line.text.Set("en", english);
        line.text.Set("de", german);
        item.description = {line};
        return item;
    };
    // The bonus in English and German; another table; a deleted item that still has a description.
    std::vector<ItemDefinition> items = {
        describe(MakeShield(0, ""), "Block +{table:bonus}", "Abwehr +{table:bonus}"),
        describe(MakeShield(1, ""), "Block +{table:bonus2}", "Abwehr +{table:bonus2}"),
        describe(MakeShield(2, ""), "Block +{table:bonus}", "Abwehr +{table:bonus}"),
    };
    items[2].names = {};

    CHECK(FindTableDescriptionUsers(items, "bonus") == std::vector<int>{MakeItemType(6, 0)});
    CHECK(FindTableDescriptionUsers(items, "bonus2") == std::vector<int>{MakeItemType(6, 1)});
    CHECK(FindTableDescriptionUsers(items, "other").empty());

    // The shield of another change (here a new profile) keeps that change.
    ItemDefinition repointed = items[0];
    repointed.statProfile = "guard";
    std::vector<ItemDefinition> changed = {repointed};
    RenameTableInDescriptions(items, changed, "bonus2", "blockBonus");
    RenameTableInDescriptions(items, changed, "bonus", "shieldBonus");
    REQUIRE(changed.size() == 2);
    CHECK(changed[0].number == 0);
    CHECK(changed[0].statProfile == "guard");
    CHECK(changed[0].description[0].text.GetNeutral() == "Block +{table:shieldBonus}");
    CHECK(changed[0].description[0].text.Get("de") == "Abwehr +{table:shieldBonus}");
    CHECK(changed[1].number == 1);
    CHECK(changed[1].description[0].text.GetNeutral() == "Block +{table:blockBonus}");
    CHECK(changed[1].description[0].text.Get("de") == "Abwehr +{table:blockBonus}");
}

TEST_CASE("A table a line's values name is used and renamed there [editor][items]")
{
    ItemDefinition shield = MakeShield(0, "");
    ItemDescriptionLine line;
    line.textName = "increaseBlock";
    line.values = {{"percent", "table:bonus"}, {"level", "level"}};
    shield.description = {line};
    const std::vector<ItemDefinition> items = {shield};

    CHECK(FindTableDescriptionUsers(items, "bonus") == std::vector<int>{MakeItemType(6, 0)});
    std::vector<ItemDefinition> changed;
    RenameTableInDescriptions(items, changed, "bonus", "blockBonus");
    REQUIRE(changed.size() == 1);
    CHECK(changed[0].description[0].values ==
          std::vector<std::pair<std::string, std::string>>{{"percent", "table:blockBonus"}, {"level", "level"}});

    // A name line too.
    ItemDefinition named = MakeShield(1, "");
    ItemNameLine nameLine;
    nameLine.text.Set("en", "{name} +{table:bonus}");
    nameLine.hasText = true;
    named.nameLines = {nameLine};
    const std::vector<ItemDefinition> namedItems = {named};
    CHECK(FindTableDescriptionUsers(namedItems, "bonus") == std::vector<int>{MakeItemType(6, 1)});
    changed.clear();
    RenameTableInDescriptions(namedItems, changed, "bonus", "blockBonus");
    REQUIRE(changed.size() == 1);
    CHECK(changed[0].nameLines[0].text.GetNeutral() == "{name} +{table:blockBonus}");
}

#endif // _EDITOR
