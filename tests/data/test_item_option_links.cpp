#include "stdafx.h"

#include "doctest.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"
#include "Data/GameData/ItemData/ItemOptionDatabase.h"
#include "Data/GameData/ItemData/ItemOptionEffects.h"
#include "Data/GameData/ItemData/ItemOptionValidation.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace Data::Items;

namespace
{
ItemOptionDefinition MakeLuck(const std::string& name, const std::string& attribute = "criticalDamageChance")
{
    ItemOptionDefinition luck;
    luck.name = name;
    luck.type = ItemOptionType::Luck;
    luck.displayName.Set(Data::LocalizedString::NeutralLocale, "Luck");
    luck.file = "Luck.json";
    ItemOption option;
    option.attribute = attribute;
    option.add = ItemOptionAdd::Percent;
    option.value = {{5}, false};
    option.texts = {"luckJewelOfSoul", "luckCriticalDamage"};
    luck.options.push_back(option);
    return luck;
}

ItemOptionTexts MakeTexts()
{
    ItemOptionTexts texts;
    texts.texts["luckJewelOfSoul"].Set(Data::LocalizedString::NeutralLocale,
                                       "Luck (success rate of Jewel of Soul +25%)");
    texts.texts["luckCriticalDamage"].Set(Data::LocalizedString::NeutralLocale,
                                          "Luck (critical damage rate +{value}%)");
    return texts;
}

ItemDefinition MakeItem(int group, int number, std::vector<std::string> options)
{
    ItemDefinition item;
    item.group = group;
    item.number = number;
    item.names.Set(Data::LocalizedString::NeutralLocale, "Item");
    item.options = std::move(options);
    return item;
}

bool HasIssue(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& field)
{
    return std::any_of(issues.begin(), issues.end(),
                       [&](const ItemDataIssue& issue) { return issue.severity == severity && issue.field == field; });
}
} // namespace

TEST_CASE("An item's option links are written after its tags and read back [data][items]")
{
    ItemDefinition kris = MakeItem(0, 0, {"luck"});
    kris.tags.Set(ItemTag::Valuable);
    const std::string text = WriteItemGroupJson(0, std::vector<ItemDefinition>{kris});
    CHECK(text.find(R"("tags": ["valuable"],
      "options": ["luck"])") != std::string::npos);

    std::vector<ItemDefinition> items;
    std::vector<ItemDataIssue> issues;
    ReadItemGroupJson(text, "Group00_Sword.json", items, issues);
    CHECK(issues.empty());
    REQUIRE(items.size() == 1);
    CHECK(items[0].options == std::vector<std::string>{"luck"});

    // An item without links writes none.
    CHECK(WriteItemGroupJson(0, std::vector<ItemDefinition>{MakeItem(0, 0, {})}).find("options") == std::string::npos);

    items.clear();
    ReadItemGroupJson(
        R"({"formatVersion": 1, "group": 0, "items": [{"number": 0, "name": "Kris", "options": "luck"}]})",
        "Group00_Sword.json", items, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Fatal, "options"));

    // A name twice is read once: the save writes it once, and the list compares with the rules' as one name.
    items.clear();
    issues.clear();
    ReadItemGroupJson(
        R"({"formatVersion": 1, "group": 0, "items": [{"number": 0, "name": "Kris", "options": ["luck", "luck"]}]})",
        "Group00_Sword.json", items, issues);
    REQUIRE(items.size() == 1);
    CHECK(items[0].options == std::vector<std::string>{"luck"});
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "options"));
    CHECK_FALSE(HasFatal(issues));
}

TEST_CASE(
    "The option database takes an item's links by type and name, without unknown names or a second luck [data][items]")
{
    const ItemOptionDatabase options({MakeLuck("luck"), MakeLuck("otherLuck")}, MakeTexts());
    CHECK(options.Find("otherLuck") == 1);
    CHECK(options.Find("nothing") == ItemOptionDatabase::NoDefinition);

    std::vector<std::string> names = {"nothing", "otherLuck", "luck"};
    options.OrderLinks(names);
    CHECK(names == std::vector<std::string>{"luck", "otherLuck", "nothing"});

    // Luck has one byte to turn it on: the first definition of it in that order counts.
    const std::vector<ItemOptionIndex> links = options.ResolveLinks(std::vector<std::string>{"nothing", "otherLuck"});
    CHECK(links == std::vector<ItemOptionIndex>{1});
    CHECK(options.ResolveLinks(std::vector<std::string>{"otherLuck", "luck"}) == std::vector<ItemOptionIndex>{0});
}

TEST_CASE("An option's effect is what the client computes for its attribute, or none [data][items]")
{
    const ItemOptionDatabase options({MakeLuck("luck"), MakeLuck("luckyHit", "excellentDamageChance")}, MakeTexts());
    CHECK(options.GetEffect(0, 0) == ItemOptionEffect{AT_LUCK, true});
    CHECK(options.GetEffect(1, 0).effect == NoItemOptionEffect);
    CHECK_FALSE(FindItemOptionEffect(ItemOptionType::Luck, "criticalDamageChance", ItemOptionAdd::Add).has_value());
}

TEST_CASE("The option files' checks across files [data][items]")
{
    ItemOptionDefinition otherFile = MakeLuck("luck");
    otherFile.file = "Other.json";
    ItemOptionTexts texts = MakeTexts();
    texts.texts["unused"].Set(Data::LocalizedString::NeutralLocale, "Unused");
    ItemOptionDefinition withoutText = MakeLuck("withoutText", "luckyness");
    withoutText.options[0].texts = {"missing"};

    std::vector<ItemDataIssue> issues;
    ValidateItemOptions(std::vector<ItemOptionDefinition>{MakeLuck("luck"), otherFile, withoutText}, texts,
                        "OptionTexts.json", issues);
    // The same name in two files: either could be what an item links.
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Fatal, "definitions[0].name"));
    CHECK(issues.front().source == "Options/Other.json");
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "definitions[1].options[0].attribute"));
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Error, "definitions[1].options[0].text"));
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "texts.unused"));

    issues.clear();
    OptionRule rule;
    rule.options = {"luck", "nothing"};
    ValidateOptionRules(std::vector<OptionRule>{rule}, std::vector<ItemOptionDefinition>{MakeLuck("luck")},
                        "OptionRules.json", issues);
    REQUIRE(issues.size() == 1);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Fatal, "rules[0].options"));
}

TEST_CASE("An item's links name definitions, one of each byte, and every definition is linked [data][items]")
{
    const ItemOptionDatabase options({MakeLuck("luck"), MakeLuck("otherLuck"), MakeLuck("unused")}, MakeTexts());
    const std::vector<ItemDefinition> items{MakeItem(0, 0, {"luck", "nothing"}), MakeItem(0, 1, {"luck", "otherLuck"})};
    std::vector<ItemDataIssue> issues;
    ValidateItemOptionLinks(items, options, issues);
    ValidateOptionDefinitionUse(items, options, issues);
    const auto itemIssue = [&](int number, ItemDataIssueSeverity severity)
    {
        return std::any_of(
            issues.begin(), issues.end(), [&](const ItemDataIssue& issue)
            { return issue.number == number && issue.severity == severity && issue.field == "options"; });
    };
    CHECK(itemIssue(0, ItemDataIssueSeverity::Error));
    CHECK(itemIssue(1, ItemDataIssueSeverity::Error));
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "definitions[2].name"));
    CHECK_FALSE(HasIssue(issues, ItemDataIssueSeverity::Warning, "definitions[1].name"));

    // A rule links a definition too.
    OptionRule rule;
    rule.options = {"unused"};
    issues.clear();
    ValidateOptionDefinitionUse(std::vector<ItemDefinition>{},
                                ItemOptionDatabase({MakeLuck("unused")}, MakeTexts(), {rule}), issues);
    CHECK(issues.empty());
}

// The excellent byte of wings and the Horn of Fenrir means something else, and no horn is excellent, so they can't link
// excellent options.
TEST_CASE("Wings and horn mounts link no excellent options [data][items]")
{
    ItemOptionDefinition excellent;
    excellent.name = "excellent";
    excellent.type = ItemOptionType::Excellent;
    excellent.file = "Excellent.json";
    const ItemOptionDatabase options({MakeLuck("luck"), excellent}, MakeTexts());
    ItemDefinition wing = MakeItem(12, 3, {"luck", "excellent"});
    wing.slot = ItemSlot::Wings;
    ItemDefinition horn = MakeItem(13, 37, {"excellent"});
    horn.tags.Set(ItemTag::HornMount);
    const ItemDefinition sword = MakeItem(0, 0, {"luck", "excellent"});
    ItemDefinition wingWithLuck = MakeItem(12, 4, {"luck"});
    wingWithLuck.slot = ItemSlot::Wings;
    std::vector<ItemDataIssue> issues;
    // The Horn of Uniria's byte holds nothing, but it isn't excellent either.
    ItemDefinition uniria = MakeItem(13, 2, {"excellent"});
    uniria.tags.Set(ItemTag::HornMount);
    ValidateItemOptionLinks(std::vector<ItemDefinition>{wing, horn, sword, wingWithLuck, uniria}, options, issues);
    const auto linksIssue = [&](int group, int number)
    {
        return std::any_of(issues.begin(), issues.end(), [&](const ItemDataIssue& issue)
                           { return issue.group == group && issue.number == number && issue.field == "options"; });
    };
    CHECK(linksIssue(12, 3));
    CHECK(linksIssue(13, 37));
    CHECK_FALSE(linksIssue(0, 0));
    CHECK_FALSE(linksIssue(12, 4));
    CHECK(linksIssue(13, 2));
}

TEST_CASE("An edited item's option list follows the option rules while it is the one they give [data][items]")
{
    OptionRule ammunition;
    ammunition.conditions.tags.Set(ItemTag::Ammunition);
    OptionRule weapons;
    weapons.options = {"luck"};
    weapons.conditions.groups.set(0);
    const std::vector<OptionRule> rules{ammunition, weapons};

    // A new item takes its data's list.
    const ItemDefinition kris = MakeItem(0, 0, {});
    CHECK(FollowItemOptions(rules, nullptr, kris) == std::vector<std::string>{"luck"});

    // While it has its data's list, the list follows a change of its data.
    const ItemDefinition lucky = MakeItem(0, 0, {"luck"});
    ItemDefinition arrow = lucky;
    arrow.tags.Set(ItemTag::Ammunition);
    CHECK(FollowItemOptions(rules, &lucky, arrow).empty());
    ItemDefinition followedArrow = arrow;
    followedArrow.options.clear();
    CHECK(FollowItemOptions(rules, &followedArrow, MakeItem(0, 0, {})) == std::vector<std::string>{"luck"});

    // A list the change picked stays, and so does one that differs from its data's, empty too.
    CHECK(FollowItemOptions(rules, &lucky, MakeItem(0, 0, {})).empty());
    const ItemDefinition picked = MakeItem(0, 0, {});
    ItemDefinition widened = picked;
    widened.width = 2;
    CHECK(FollowItemOptions(rules, &picked, widened).empty());
    ItemDefinition pickedArrow = arrow;
    pickedArrow.options = {"luck"};
    ItemDefinition stillArrow = pickedArrow;
    stillArrow.width = 2;
    CHECK(FollowItemOptions(rules, &pickedArrow, stillArrow) == std::vector<std::string>{"luck"});
}

TEST_CASE("The item database resolves an item's links when the item or the options are stored [data][items]")
{
    ItemDatabase database;
    database.Build(std::vector<ItemDefinition>{MakeItem(0, 0, {"luck"}), MakeItem(0, 1, {})});
    CHECK(database.GetOptionLinks(MakeItemType(0, 0)).empty());

    database.SetOptions(ItemOptionDatabase({MakeLuck("otherLuck"), MakeLuck("luck")}, MakeTexts()));
    REQUIRE(database.GetOptionLinks(MakeItemType(0, 0)).size() == 1);
    CHECK(database.GetOptionLinks(MakeItemType(0, 0))[0] == 1);
    CHECK(database.GetOptionLinks(MakeItemType(0, 1)).empty());

    database.Set(MakeItem(0, 1, {"otherLuck"}));
    REQUIRE(database.GetOptionLinks(MakeItemType(0, 1)).size() == 1);
    CHECK(database.GetOptionLinks(MakeItemType(0, 1))[0] == 0);
    CHECK(database.GetOptionLinks(-1).empty());
}

// The excellent options' effects: the slots the stat code, the price and the mixes read, with a value where the code
// before the definitions wrote one.
TEST_CASE("The excellent options' effects are the client's excellent slots [data][items]")
{
    using Type = ItemOptionType;
    CHECK(FindItemOptionEffect(Type::Excellent, "maximumHealth", ItemOptionAdd::Percent) ==
          ItemOptionEffect{AT_IMPROVE_LIFE, false});
    CHECK(FindItemOptionEffect(Type::Excellent, "moneyAmountRate", ItemOptionAdd::Percent) ==
          ItemOptionEffect{AT_IMPROVE_GAIN_GOLD, false});
    CHECK(FindItemOptionEffect(Type::Excellent, "physicalBaseDmg", ItemOptionAdd::Add) ==
          ItemOptionEffect{AT_IMPROVE_DAMAGE_LEVEL, true});
    CHECK(FindItemOptionEffect(Type::Excellent, "wizardryBaseDmgIncrease", ItemOptionAdd::Percent) ==
          ItemOptionEffect{AT_IMPROVE_MAGIC_PERCENT, true});
    CHECK(FindItemOptionEffect(Type::Excellent, "attackSpeedAny", ItemOptionAdd::Add) ==
          ItemOptionEffect{AT_IMPROVE_ATTACK_SPEED, true});
    // The same attribute of another type is another effect.
    CHECK(FindItemOptionEffect(Type::Option, "physicalBaseDmg", ItemOptionAdd::Add) ==
          ItemOptionEffect{AT_IMPROVE_DAMAGE, true});
    CHECK(FindItemOptionEffect(Type::Excellent, "maximumMana", ItemOptionAdd::Percent) ==
          ItemOptionEffect{AT_IMPROVE_MANA, false});
}

// A term on a hero attribute the client doesn't read is shown and adds nothing, with a warning.
TEST_CASE("A term on a hero attribute the client doesn't read loads with a warning [data][items]")
{
    ItemOptionDefinition luck = MakeLuck("luck");
    luck.options[0].value.per = {{"level", 0, 20}, {"horseLevel", 2, 0}};
    std::vector<ItemDataIssue> issues;
    ValidateItemOptions(std::vector<ItemOptionDefinition>{luck}, MakeTexts(), "OptionTexts.json", issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "definitions[0].options[0].per[1].attribute"));
    CHECK_FALSE(HasIssue(issues, ItemDataIssueSeverity::Warning, "definitions[0].options[0].per[0].attribute"));
}

// A text with {divisor} on an option whose terms have no divisor would show 0 there, and on one with several only the
// first one's: both load with a warning.
TEST_CASE("A {divisor} text on an option without one divisor term loads with a warning [data][items]")
{
    ItemOptionTexts texts = MakeTexts();
    texts.texts["luckCriticalDamage"].Set(Data::LocalizedString::NeutralLocale, "Damage +level/{divisor}");
    ItemOptionDefinition luck = MakeLuck("luck");
    std::vector<ItemDataIssue> issues;
    ValidateItemOptions(std::vector<ItemOptionDefinition>{luck}, texts, "OptionTexts.json", issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "definitions[0].options[0].text"));

    luck.options[0].value.per = {{"level", 0, 20}};
    issues.clear();
    ValidateItemOptions(std::vector<ItemOptionDefinition>{luck}, texts, "OptionTexts.json", issues);
    CHECK_FALSE(HasIssue(issues, ItemDataIssueSeverity::Warning, "definitions[0].options[0].text"));

    // Two terms with a divisor: the line shows only the first one's.
    luck.options[0].value.per = {{"level", 0, 20}, {"masterLevel", 0, 10}};
    issues.clear();
    ValidateItemOptions(std::vector<ItemOptionDefinition>{luck}, texts, "OptionTexts.json", issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "definitions[0].options[0].text"));
}
