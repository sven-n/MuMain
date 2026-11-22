#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "UI/ItemTools/ItemOptionFileChange.h"

#include "Data/GameData/ItemData/ItemType.h"

#include <set>
#include <string>
#include <vector>

using namespace MuEditor::Items;
using namespace Data::Items;

namespace
{
ItemDefinition MakeItem(int group, int number, std::vector<std::string> options)
{
    ItemDefinition item;
    item.group = group;
    item.number = number;
    item.names.Set("en", "Item " + std::to_string(number));
    item.options = std::move(options);
    return item;
}

ItemOptionDefinition MakeDefinition(std::string name, ItemOptionType type, std::vector<int> values,
                                    std::string text = "additionalDamage")
{
    ItemOptionDefinition definition{.name = std::move(name), .type = type, .file = "Option.json"};
    ItemOption option{.attribute = "physicalBaseDmg"};
    option.value.values = std::move(values);
    option.value.isList = true;
    option.texts = {std::move(text)};
    definition.options.push_back(std::move(option));
    return definition;
}

ItemOptionFiles MakeFiles()
{
    ItemOptionFiles files;
    files.definitions.push_back({.name = "luck", .type = ItemOptionType::Luck, .file = "Luck.json"});
    files.definitions.push_back(MakeDefinition("physicalDamageOption", ItemOptionType::Option, {4, 8, 12, 16}));
    files.definitions.push_back(MakeDefinition("jewelryHealthRecovery", ItemOptionType::Option, {1, 2, 3}, "heal"));
    files.texts.texts["additionalDamage"].Set("en", "Additional damage +{value}");
    files.texts.texts["heal"].Set("en", "Automatic HP recovery {value}%");
    files.rules.push_back({.options = {"luck", "physicalDamageOption"}});
    files.rules.push_back({.options = {"jewelryHealthRecovery"}});
    return files;
}

std::vector<ItemDefinition> MakeSlots()
{
    return {MakeItem(0, 0, {"luck", "physicalDamageOption"}), MakeItem(0, 1, {"luck", "physicalDamageOption"}),
            MakeItem(13, 8, {"jewelryHealthRecovery"}), MakeItem(14, 0, {})};
}
} // namespace

TEST_CASE("Item Options finds the items and rules that link a definition [data][items][editor][options]")
{
    const std::vector<ItemDefinition> slots = MakeSlots();
    const ItemOptionFiles files = MakeFiles();
    CHECK(FindOptionUsers(slots, files, "luck") == std::vector<int>{MakeItemType(0, 0), MakeItemType(0, 1)});
    CHECK(FindOptionUsers(slots, files, "jewelryHealthRecovery") == std::vector<int>{MakeItemType(13, 8)});
    CHECK(FindOptionUsers(slots, files, "noSuchDefinition").empty());
    CHECK(CountOptionRuleUses(files.rules, "luck") == 1);
    CHECK(CountOptionRuleUses(files.rules, "noSuchDefinition") == 0);
}

TEST_CASE("Item Options puts a new definition in the file of its type's definitions [data][items][editor][options]")
{
    ItemOptionFiles files = MakeFiles();
    CHECK(FindFileOfType(files, ItemOptionType::Option) == "Option.json");
    CHECK(FindFileOfType(files, ItemOptionType::Luck) == "Luck.json");
    // The file named after the type, also where another file holds more of them (Wings.json the wings' options).
    for (int wing = 0; wing < 3; ++wing)
    {
        files.definitions.push_back(MakeDefinition("wing" + std::to_string(wing), ItemOptionType::Option, {1}));
        files.definitions.back().file = "Wings.json";
    }
    CHECK(FindFileOfType(files, ItemOptionType::Option) == "Option.json");
    // Else the file most of them are in.
    files.definitions.push_back(MakeDefinition("secondTierWing", ItemOptionType::Wing, {1}));
    files.definitions.back().file = "Wings.json";
    CHECK(FindFileOfType(files, ItemOptionType::Wing) == "Wings.json");
    // No definition of the type: the type's name, or the shipped file's (Wings.json).
    CHECK(FindFileOfType(files, ItemOptionType::Excellent) == "Excellent.json");
    CHECK(FindFileOfType(MakeFiles(), ItemOptionType::Wing) == "Wings.json");
    CHECK(FindFileOfType(files, ItemOptionType::AncientOption) == "Ancient.json");
    // The two ancient types share a file: set options go where the bonuses are.
    files.definitions.push_back(MakeDefinition("ancientBonusStrength", ItemOptionType::AncientBonus, {5}));
    files.definitions.back().file = "Sets.json";
    CHECK(FindFileOfType(files, ItemOptionType::AncientOption) == "Sets.json");
}

TEST_CASE("Item Basics offers only the option links that do something [data][items][editor][options]")
{
    ItemOptionFiles files = MakeFiles();
    files.definitions.push_back(MakeDefinition("excellentWeapon", ItemOptionType::Excellent, {1}));
    files.definitions.push_back(MakeDefinition("ancientBonusStrength", ItemOptionType::AncientBonus, {5}));
    files.definitions.push_back(MakeDefinition("secondTierWing", ItemOptionType::Wing, {1}));
    const ItemOptionDatabase options(files);
    const auto definition = [&files](std::string_view name) -> const ItemOptionDefinition&
    { return *std::ranges::find(files.definitions, name, &ItemOptionDefinition::name); };

    const ItemDefinition kris = MakeItem(0, 0, {"luck", "physicalDamageOption"});
    // Linked already, or a second definition of a type the item links: the first counts.
    CHECK_FALSE(IsUsefulOptionLink(kris, definition("physicalDamageOption"), options));
    CHECK_FALSE(IsUsefulOptionLink(kris, definition("jewelryHealthRecovery"), options));
    CHECK(IsUsefulOptionLink(kris, definition("excellentWeapon"), options));
    // The item sets give the ancient types.
    CHECK_FALSE(IsUsefulOptionLink(kris, definition("ancientBonusStrength"), options));
    // The wings' excellent byte holds their wing options.
    ItemDefinition wings = MakeItem(12, 3, {});
    wings.slot = ItemSlot::Wings;
    CHECK_FALSE(IsUsefulOptionLink(wings, definition("excellentWeapon"), options));
    CHECK(IsUsefulOptionLink(wings, definition("luck"), options));
    CHECK(IsUsefulOptionLink(wings, definition("secondTierWing"), options));
    // The excellent byte holds excellent options on every other item: no wing options there, and an excellent link
    // counts even where a wing link is written already. A horn's byte holds its kind.
    CHECK_FALSE(IsUsefulOptionLink(kris, definition("secondTierWing"), options));
    const ItemDefinition wingedRing = MakeItem(13, 8, {"secondTierWing"});
    CHECK(IsUsefulOptionLink(wingedRing, definition("excellentWeapon"), options));
    ItemDefinition excellentWings = MakeItem(12, 4, {"excellentWeapon"});
    excellentWings.slot = ItemSlot::Wings;
    CHECK(IsUsefulOptionLink(excellentWings, definition("secondTierWing"), options));
    ItemDefinition horn = MakeItem(13, 37, {});
    horn.tags.Set(ItemTag::HornMount);
    CHECK_FALSE(IsUsefulOptionLink(horn, definition("secondTierWing"), options));
}

TEST_CASE("Item Options counts the items whose attackType picks an ancient bonus definition by its name "
          "[data][items][editor][options]")
{
    std::vector<ItemDefinition> slots = MakeSlots();
    slots[2].attackType = 2;
    ItemOptionFiles files = MakeFiles();
    files.definitions.push_back(MakeDefinition(std::string(AncientBonusNames[1]), ItemOptionType::AncientBonus, {5}));
    CHECK(FindAncientBonusPick(slots[2], FindAncientBonusPicks(files)) == AncientBonusNames[1]);
    CHECK(FindAncientBonusPick(slots[0], FindAncientBonusPicks(files)).empty());
    CHECK(FindOptionUsers(slots, files, AncientBonusNames[1]) == std::vector<int>{MakeItemType(13, 8)});
    // A change of the definition reaches the item, and its name cannot be changed.
    CHECK(FindItemsReached(slots, files, files, std::vector<std::string>{std::string(AncientBonusNames[1])}) ==
          std::vector<int>{MakeItemType(13, 8)});
    CHECK(IsPickedByName(files.definitions.back()));
    CHECK_FALSE(IsPickedByName(files.definitions.front()));

    // As in the game, a definition of that name but another type is not picked.
    files.definitions.back().type = ItemOptionType::Option;
    CHECK(FindAncientBonusPick(slots[2], FindAncientBonusPicks(files)).empty());
    CHECK(FindOptionUsers(slots, files, AncientBonusNames[1]).empty());
    CHECK_FALSE(IsPickedByName(files.definitions.back()));
}

TEST_CASE("Item Options lists the stored definitions, then the copy's new ones, by type and search "
          "[data][items][editor][options]")
{
    const ItemOptionFiles stored = MakeFiles();
    ItemOptionFiles edit = stored;
    edit.definitions.push_back(MakeDefinition("physicalDamageCopy", ItemOptionType::Option, {1}));
    edit.definitions.push_back(MakeDefinition("excellentNew", ItemOptionType::Excellent, {1}));
    const auto names = [&](int typeFilter, std::string_view search)
    {
        std::vector<std::string> listed;
        for (const ItemOptionDefinition* definition : ListDefinitions(stored, edit, typeFilter, search))
        {
            listed.push_back(definition->name);
        }
        return listed;
    };
    CHECK(names(-1, "") == std::vector<std::string>{"luck", "physicalDamageOption", "jewelryHealthRecovery",
                                                    "physicalDamageCopy", "excellentNew"});
    // The filters also hold for the copy's new definitions.
    CHECK(names(-1, "PHYSICAL") == std::vector<std::string>{"physicalDamageOption", "physicalDamageCopy"});
    CHECK(names(static_cast<int>(ItemOptionType::Excellent), "") == std::vector<std::string>{"excellentNew"});
    CHECK(names(static_cast<int>(ItemOptionType::Option), "copy") == std::vector<std::string>{"physicalDamageCopy"});
}

TEST_CASE("Item Options keeps only the marks of names the stored files or the copy have "
          "[data][items][editor][options]")
{
    const ItemOptionFiles stored = MakeFiles();
    ItemOptionFiles edit = stored;
    edit.definitions.push_back(MakeDefinition("physicalDamageCopy", ItemOptionType::Option, {1}));
    std::set<std::string, std::less<>> marked = {"physicalDamageOption", "physicalDamageCopy", "renamedAway"};
    DropMissingMarks(marked, stored, edit);
    CHECK(marked == std::set<std::string, std::less<>>{"physicalDamageOption", "physicalDamageCopy"});
    // A revert drops the copy's new definition, and its mark.
    DropMissingMarks(marked, stored, stored);
    CHECK(marked == std::set<std::string, std::less<>>{"physicalDamageOption"});
}

TEST_CASE("Item Options counts the items a new ancient bonus definition of the copy would reach "
          "[data][items][editor][options]")
{
    std::vector<ItemDefinition> slots = MakeSlots();
    slots[2].attackType = 2;
    const ItemOptionFiles stored = MakeFiles();
    ItemOptionFiles edit = stored;
    edit.definitions.push_back(MakeDefinition(std::string(AncientBonusNames[1]), ItemOptionType::AncientBonus, {5}));
    CHECK(FindOptionUsers(slots, stored, AncientBonusNames[1]).empty());
    CHECK(FindOptionUsers(slots, edit, AncientBonusNames[1]) == std::vector<int>{MakeItemType(13, 8)});
    CHECK(FindAncientBonusPick(slots[2], FindAncientBonusPicks(edit)) == AncientBonusNames[1]);
}

TEST_CASE("Item Options counts each item once, with the picks the copy's new ancient bonus definitions add "
          "[data][items][editor][options]")
{
    std::vector<ItemDefinition> slots = MakeSlots();
    slots[0].attackType = 1;
    slots[2].attackType = 2;
    ItemOptionFiles stored = MakeFiles();
    stored.definitions.push_back(MakeDefinition(std::string(AncientBonusNames[0]), ItemOptionType::AncientBonus, {5}));
    ItemOptionFiles edit = stored;
    edit.definitions.push_back(MakeDefinition(std::string(AncientBonusNames[1]), ItemOptionType::AncientBonus, {5}));

    const AncientBonusPicks picks = MergeCopyPicks(stored, edit);
    CHECK(picks[0] == AncientBonusNames[0]);
    CHECK(picks[1] == AncientBonusNames[1]);
    const auto counts = CountOptionUsers(slots, picks);
    CHECK(counts.at("luck") == 2);
    CHECK(counts.at("physicalDamageOption") == 2);
    CHECK(counts.at("jewelryHealthRecovery") == 1);
    CHECK(counts.at(std::string(AncientBonusNames[0])) == 1);
    CHECK(counts.at(std::string(AncientBonusNames[1])) == 1);
    CHECK(IsOnlyInCopy(stored, AncientBonusNames[1]));
    CHECK_FALSE(IsOnlyInCopy(stored, AncientBonusNames[0]));

    // The copy's definition as another type is picked no more: the picks change, so the counts are made again.
    edit.definitions.back().type = ItemOptionType::Option;
    CHECK(MergeCopyPicks(stored, edit)[1].empty());
    CHECK(MergeCopyPicks(stored, edit) != picks);
    CHECK_FALSE(CountOptionUsers(slots, MergeCopyPicks(stored, edit)).contains(AncientBonusNames[1]));

    // A stored definition the copy changes to another type keeps its stored pick until it is applied.
    ItemOptionFiles changed = stored;
    changed.definitions.back().type = ItemOptionType::Option;
    CHECK(MergeCopyPicks(stored, changed)[0] == AncientBonusNames[0]);
}

TEST_CASE("Item Options' copy follows the stored files until it is changed, then waits for a revert when they change "
          "[data][items][editor][options]")
{
    const ItemOptionFiles stored = MakeFiles();
    ItemOptionFiles undone = stored;
    undone.definitions.pop_back();

    // Without changes, an undo is taken.
    ItemOptionFiles edit = stored;
    ItemOptionFiles loaded = stored;
    CHECK(UpdateEditedCopy(edit, loaded, undone) == EditedCopyState::Follows);
    CHECK(edit == undone);
    CHECK(loaded == undone);

    // With changes: an undo makes the copy stale, the redo back to what it was taken from makes it fresh again.
    edit = stored;
    loaded = stored;
    edit.definitions[1].options[0].value.values = {9};
    const ItemOptionFiles changed = edit;
    CHECK(UpdateEditedCopy(edit, loaded, stored) == EditedCopyState::Edited);
    CHECK(UpdateEditedCopy(edit, loaded, undone) == EditedCopyState::Stale);
    CHECK(edit == changed);
    CHECK(UpdateEditedCopy(edit, loaded, stored) == EditedCopyState::Edited);

    // A revert takes the stored files; the copy follows them again.
    CHECK(UpdateEditedCopy(edit, loaded, undone) == EditedCopyState::Stale);
    edit = undone;
    loaded = undone;
    CHECK(UpdateEditedCopy(edit, loaded, stored) == EditedCopyState::Follows);
    CHECK(edit == stored);

    // A redo that makes the stored files the copy's changes: nothing is left to apply, the copy follows them.
    edit = undone;
    loaded = undone;
    edit.definitions[1].options[0].value.values = {9};
    const ItemOptionFiles redone = edit;
    CHECK(UpdateEditedCopy(edit, loaded, redone) == EditedCopyState::Follows);
    CHECK(edit == redone);
    CHECK(loaded == redone);
}

TEST_CASE("Item Options lists the items a new ancient bonus definition reaches by their attackType "
          "[data][items][editor][options]")
{
    std::vector<ItemDefinition> slots = MakeSlots();
    slots[2].attackType = 4;
    const ItemOptionFiles before = MakeFiles();
    ItemOptionFiles after = before;
    after.definitions.push_back(MakeDefinition(std::string(AncientBonusNames[3]), ItemOptionType::AncientBonus, {5}));
    const std::vector<std::string> changed = FindChangedDefinitions(before, after);
    REQUIRE(changed == std::vector<std::string>{std::string(AncientBonusNames[3])});
    // The files before the change have no such definition; the item picks the new one.
    CHECK(FindItemsReached(slots, before, after, changed) == std::vector<int>{MakeItemType(13, 8)});
    // Removing it reaches the item too.
    CHECK(FindItemsReached(slots, after, before, FindChangedDefinitions(after, before)) ==
          std::vector<int>{MakeItemType(13, 8)});
}

TEST_CASE("A change of the option files reaches the items that link what it changes "
          "[data][items][editor][options]")
{
    const ItemOptionFiles before = MakeFiles();
    const std::vector<ItemDefinition> slots = MakeSlots();

    SUBCASE("nothing changed")
    {
        CHECK(FindChangedDefinitions(before, before).empty());
    }

    SUBCASE("a value list: the items that link the definition")
    {
        ItemOptionFiles after = before;
        after.definitions[1].options[0].value.values.push_back(20);
        const std::vector<std::string> changed = FindChangedDefinitions(before, after);
        CHECK(changed == std::vector<std::string>{"physicalDamageOption"});
        CHECK(FindItemsReached(slots, before, after, changed) ==
              std::vector<int>{MakeItemType(0, 0), MakeItemType(0, 1)});
    }

    SUBCASE("a text an option names")
    {
        ItemOptionFiles after = before;
        after.texts.texts["heal"].Set("en", "Life recovery {value}%");
        CHECK(FindChangedDefinitions(before, after) == std::vector<std::string>{"jewelryHealthRecovery"});
    }

    SUBCASE("the display name alone changes no line")
    {
        ItemOptionFiles after = before;
        after.definitions[0].displayName.Set("en", "Luck");
        CHECK(FindChangedDefinitions(before, after).empty());
    }

    SUBCASE("a rename reaches the items that link the old name")
    {
        ItemOptionFiles after = before;
        RenameOptionDefinition(after, "luck", "fortune");
        CHECK(after.definitions[0].name == "fortune");
        CHECK(after.rules[0].options == std::vector<std::string>{"fortune", "physicalDamageOption"});
        const std::vector<std::string> changed = FindChangedDefinitions(before, after);
        CHECK(changed == std::vector<std::string>{"fortune", "luck"});
        CHECK(FindItemsReached(slots, before, after, {}, "luck") ==
              std::vector<int>{MakeItemType(0, 0), MakeItemType(0, 1)});

        const std::vector<ItemDefinition> relinked = FindItemListChanges(slots, before, after, "luck", "fortune");
        REQUIRE(relinked.size() == 2);
        CHECK(relinked[0].options == std::vector<std::string>{"fortune", "physicalDamageOption"});
        CHECK(relinked[1].number == 1);
    }
}

TEST_CASE("A change of the option rules changes the lists of the items that follow them "
          "[data][items][editor][options]")
{
    ItemOptionFiles before = MakeFiles();
    // The first rule gives the weapons (group 0) their list, the second the rest.
    before.rules[0].conditions.groups.set(0);
    std::vector<ItemDefinition> slots = MakeSlots();
    // A picked list: the second sword has no options of the rules.
    slots[1].options = {"luck"};
    // The ticket (14/0) follows the second rule, which gives it the jewelry option.
    slots[3].options = {"jewelryHealthRecovery"};
    ItemOptionFiles after = before;
    after.rules[0].options = {"physicalDamageOption", "luck"};
    after.rules[0].options.push_back("jewelryHealthRecovery");

    const std::vector<ItemDefinition> changed = FindItemListChanges(slots, before, after);
    REQUIRE(changed.size() == 1);
    CHECK(changed[0].number == 0);
    // In the order the game takes the links in: by type, then by name.
    CHECK(changed[0].options == std::vector<std::string>{"luck", "jewelryHealthRecovery", "physicalDamageOption"});
    CHECK(FindItemListChanges(slots, before, before).empty());
    CHECK(IsSameOptionList(std::vector<std::string>{"a", "b"}, std::vector<std::string>{"b", "a"}));
    CHECK_FALSE(IsSameOptionList(std::vector<std::string>{"a"}, std::vector<std::string>{"a", "b"}));
}

TEST_CASE("Fill levels makes a definition's value lists end at a level by their own step "
          "[data][items][editor][options]")
{
    ItemOptionFiles files = MakeFiles();
    ItemOptionDefinition& damage = files.definitions[1];
    ItemOptionDefinition& jewelry = files.definitions[2];
    CHECK(HighestOptionLevel(damage) == 4);
    CHECK(HighestOptionLevel(jewelry) == 3);
    CHECK(HighestOptionLevel(files.definitions[0]) == 0);

    SUBCASE("longer lists go on by the step between their last two entries")
    {
        CHECK(FillOptionLevels(damage, 7));
        CHECK(damage.options[0].value.values == std::vector<int>{4, 8, 12, 16, 20, 24, 28});
        CHECK(FillOptionLevels(jewelry, 7));
        CHECK(jewelry.options[0].value.values == std::vector<int>{1, 2, 3, 4, 5, 6, 7});
        CHECK(HighestOptionLevel(damage) == 7);
        CHECK_FALSE(FillOptionLevels(damage, 7));
    }

    SUBCASE("shorter lists are cut")
    {
        CHECK(FillOptionLevels(damage, 2));
        CHECK(damage.options[0].value.values == std::vector<int>{4, 8});
    }

    SUBCASE("a list of one entry goes on by that entry; a constant stays")
    {
        damage.options[0].value.values = {5};
        CHECK(FillOptionLevels(damage, 3));
        CHECK(damage.options[0].value.values == std::vector<int>{5, 10, 15});
        damage.options[0].value = {.values = {7}, .isList = false};
        CHECK_FALSE(FillOptionLevels(damage, 3));
        CHECK(damage.options[0].value.values == std::vector<int>{7});
    }

    SUBCASE("a list written from a start level keeps the levels below it")
    {
        ItemOptionValue& value = damage.options[0].value;
        FillOptionValue(value, 3, 20, 5, 6);
        CHECK(value.values == std::vector<int>{4, 8, 20, 25, 30, 35});
        CHECK(value.isList);
        ItemOptionValue constant{.values = {7}, .isList = false};
        FillOptionValue(constant, 2, 10, 10, 3);
        CHECK(constant.values == std::vector<int>{0, 10, 20});
    }

    SUBCASE("a list by the item's level is written from level 0 up to 15")
    {
        ItemOptionValue health{
            .values = {50, 55}, .isList = true, .levelType = Data::Items::ItemOptionLevelType::ItemLevel};
        FillOptionValue(health, 0, 50, 5, 15);
        REQUIRE(health.values.size() == 16);
        CHECK(health.values.front() == 50);
        CHECK(health.values.back() == 125);
        FillOptionValue(health, 2, 100, 0, 3);
        CHECK(health.values == std::vector<int>{50, 55, 100, 100});
    }
}

#endif // _EDITOR
