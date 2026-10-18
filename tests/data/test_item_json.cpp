#include "doctest.h"

#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace Data::Items;

namespace
{
const std::string Source = "Group00_Sword.json";

struct ReadResult
{
    std::vector<ItemDefinition> items;
    std::vector<ItemDataIssue> issues;
};

ReadResult Read(const std::string& text)
{
    ReadResult result;
    ReadItemGroupJson(text, Source, result.items, result.issues);
    return result;
}

std::string GroupFile(int group, const std::string& items)
{
    return R"({"formatVersion": 1, "group": )" + std::to_string(group) + R"(, "items": [)" + items + "]}";
}

bool HasIssue(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& field)
{
    return std::any_of(issues.begin(), issues.end(), [&](const ItemDataIssue& issue) {
        return issue.severity == severity && issue.field == field;
    });
}

ItemDefinition MakeKris()
{
    ItemDefinition kris;
    kris.group = 0;
    kris.number = 0;
    kris.names = Data::LocalizedString::Parse("Kris||pt=Cris");
    kris.width = 1;
    kris.height = 2;
    kris.slot = ItemSlot::MainHand;
    kris.level = 6;
    kris.damageMin = 6;
    kris.damageMax = 11;
    kris.attackSpeed = 50;
    kris.durability = 20;
    kris.requirements.strength = 10;
    kris.requirements.dexterity = 8;
    kris.classRequirements = {1, 1, 1, 1, 1, 0, 1};
    kris.resistances[3] = 2;
    kris.buyPrice = 1500;
    return kris;
}
} // namespace

TEST_CASE("Item JSON keeps every value through write and read [data][items]")
{
    const ItemDefinition kris = MakeKris();
    const std::string text = WriteItemGroupJson(0, std::vector<ItemDefinition>{kris});

    const ReadResult result = Read(text);
    REQUIRE(result.issues.empty());
    REQUIRE(result.items.size() == 1);

    const ItemDefinition& read = result.items.front();
    CHECK(read.names == kris.names);
    CHECK(read.width == kris.width);
    CHECK(read.height == kris.height);
    CHECK(read.slot == kris.slot);
    CHECK(read.level == kris.level);
    CHECK(read.damageMin == kris.damageMin);
    CHECK(read.damageMax == kris.damageMax);
    CHECK(read.attackSpeed == kris.attackSpeed);
    CHECK(read.durability == kris.durability);
    CHECK(read.requirements == kris.requirements);
    CHECK(read.classRequirements == kris.classRequirements);
    CHECK(read.resistances == kris.resistances);
    CHECK(read.buyPrice == kris.buyPrice);
}

TEST_CASE("Item JSON leaves out default values [data][items]")
{
    ItemDefinition apple;
    apple.group = 14;
    apple.number = 0;
    apple.names = Data::LocalizedString::Parse("Apple");

    const std::string text = WriteItemGroupJson(14, std::vector<ItemDefinition>{apple});

    CHECK(text.find("\"slot\"") == std::string::npos);
    CHECK(text.find("\"weaponKind\"") == std::string::npos);
    CHECK(text.find("\"width\"") == std::string::npos);
    CHECK(text.find("\"requirements\"") == std::string::npos);

    const ReadResult result = Read(text);
    REQUIRE(result.items.size() == 1);
    CHECK(result.items.front().slot == ItemSlot::None);
    CHECK(result.items.front().weaponKind == WeaponKind::None);
}

TEST_CASE("Item JSON output is sorted and stable [data][items]")
{
    ItemDefinition second = MakeKris();
    second.number = 7;
    const std::vector<ItemDefinition> items = {second, MakeKris()};

    const std::string text = WriteItemGroupJson(0, items);
    CHECK(text.find("\"number\": 0") < text.find("\"number\": 7"));

    const ReadResult result = Read(text);
    CHECK(WriteItemGroupJson(0, result.items) == text);
}

TEST_CASE("Item JSON only writes items of the requested group [data][items]")
{
    ItemDefinition otherGroup = MakeKris();
    otherGroup.group = 1;

    const std::string text = WriteItemGroupJson(0, std::vector<ItemDefinition>{otherGroup});

    CHECK(Read(text).items.empty());
}

TEST_CASE("Invalid item JSON is reported [data][items]")
{
    const ReadResult result = Read("{ not json");

    CHECK(result.items.empty());
    REQUIRE(result.issues.size() == 1);
    CHECK(result.issues.front().severity == ItemDataIssueSeverity::Error);
    CHECK(result.issues.front().source == Source);
}

TEST_CASE("A newer item file format is rejected [data][items]")
{
    const ReadResult result = Read(R"({"formatVersion": 99, "group": 0, "items": []})");

    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Error, "formatVersion"));
}

TEST_CASE("Item groups and numbers must be in range [data][items]")
{
    CHECK(HasIssue(Read(GroupFile(16, "")).issues, ItemDataIssueSeverity::Error, "group"));
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 512, "name": "X"})")).issues, ItemDataIssueSeverity::Error, "number"));
}

TEST_CASE("Item values must fit their type [data][items]")
{
    const ReadResult result = Read(GroupFile(0, R"({"number": 0, "name": "Kris", "width": 256, "twoHanded": 1})"));

    CHECK(result.items.empty());
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Error, "width"));
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Error, "twoHanded"));
}

TEST_CASE("Unknown item fields are warnings [data][items]")
{
    const ReadResult result =
        Read(GroupFile(0, R"({"number": 0, "name": "Kris", "sparkle": 1, "requirements": {"luck": 1}})"));

    REQUIRE(result.items.size() == 1);
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "sparkle"));
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "requirements.luck"));
    CHECK_FALSE(HasErrors(result.issues));
}

TEST_CASE("Item validation finds duplicates and missing English names [data][items]")
{
    ItemDefinition kris = MakeKris();
    ItemDefinition duplicate = MakeKris();
    ItemDefinition noEnglishName = MakeKris();
    noEnglishName.number = 1;
    noEnglishName.names = Data::LocalizedString::Parse("||pt=Espada curta");

    std::vector<ItemDataIssue> issues;
    ValidateItems(std::vector<ItemDefinition>{kris, duplicate, noEnglishName}, issues);

    CHECK(HasIssue(issues, ItemDataIssueSeverity::Error, "number"));
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Error, "name"));
}

TEST_CASE("Item issues describe where the problem is [data][items]")
{
    const ItemDataIssue issue{ItemDataIssueSeverity::Error, Source, 0, 5, "width", "must be a whole number"};

    CHECK(issue.ToString() == "error: Group00_Sword.json: item (0,5) width: must be a whole number");
}

TEST_CASE("Item names are written as an object with English first [data][items]")
{
    const std::string text = WriteItemGroupJson(0, std::vector<ItemDefinition>{MakeKris()});

    const std::string expectedName = R"("name": {
        "en": "Kris",
        "pt": "Cris"
      })";
    CHECK(text.find(expectedName) != std::string::npos);
}

TEST_CASE("A plain item name is the English name [data][items]")
{
    const ReadResult result = Read(GroupFile(0, R"({"number": 0, "name": "Kris"})"));

    REQUIRE(result.items.size() == 1);
    CHECK(result.items.front().names.GetNeutral() == "Kris");
    CHECK(result.items.front().names.GetTranslations().empty());
}

TEST_CASE("Item names must be texts [data][items]")
{
    const ReadResult result = Read(GroupFile(0, R"({"number": 0, "name": {"en": "Kris", "pt": 5}})"));

    CHECK(result.items.empty());
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Error, "name.pt"));
}

TEST_CASE("Numbers too large for their field do not wrap around [data][items]")
{
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 4294967297, "name": "X"})")).issues, ItemDataIssueSeverity::Error,
                   "number"));
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "buyPrice": 18446744073709551615})")).issues,
                   ItemDataIssueSeverity::Error, "buyPrice"));
    CHECK(HasIssue(Read(R"({"formatVersion": 4294967297, "group": 0, "items": []})").issues,
                   ItemDataIssueSeverity::Error, "formatVersion"));
    CHECK(HasIssue(Read(R"({"formatVersion": 1, "group": 4294967296, "items": []})").issues,
                   ItemDataIssueSeverity::Error, "group"));
}

TEST_CASE("Item name language codes must be codes [data][items]")
{
    ItemDefinition kris = MakeKris();
    kris.names.Set("p=t", "x");
    ItemDefinition empty = MakeKris();
    empty.number = 1;
    empty.names.Set("", "x");
    ItemDefinition valid = MakeKris();
    valid.number = 2;
    valid.names.Set("zh-TW", "x");

    std::vector<ItemDataIssue> issues;
    ValidateItems(std::vector<ItemDefinition>{kris}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Error, "name"));

    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{empty}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Error, "name"));

    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{valid}, issues);
    CHECK_FALSE(HasErrors(issues));
}

TEST_CASE("Item tags, wing tier, potion priority and rule flags are kept through write and read [data][items]")
{
    ItemDefinition wing = MakeKris();
    wing.slot = ItemSlot::Wings;
    wing.wingTier = WingTier::Third;
    wing.potionPriority = 7;
    wing.tags.Set(ItemTag::Valuable);
    wing.tags.Set(ItemTag::CashShop);
    wing.tradable = false;
    wing.repairable = false;

    const ReadResult result = Read(WriteItemGroupJson(0, std::vector<ItemDefinition>{wing}));
    REQUIRE(result.issues.empty());
    REQUIRE(result.items.size() == 1);

    const ItemDefinition& read = result.items.front();
    CHECK(read.slot == ItemSlot::Wings);
    CHECK(read.wingTier == WingTier::Third);
    CHECK(read.potionPriority == 7);
    CHECK(read.tags == wing.tags);
    CHECK_FALSE(read.tradable);
    CHECK_FALSE(read.repairable);
    CHECK(read.droppable);
    CHECK(read.sellable);
}

TEST_CASE("Item slots, wing tiers and tags are written by name [data][items]")
{
    ItemDefinition wing = MakeKris();
    wing.slot = ItemSlot::Wings;
    wing.wingTier = WingTier::Second;
    wing.tags.Set(ItemTag::Valuable);
    wing.tags.Set(ItemTag::Jewel);
    wing.sellable = false;

    const std::string text = WriteItemGroupJson(0, std::vector<ItemDefinition>{wing});

    CHECK(text.find(R"("slot": "wings")") != std::string::npos);
    CHECK(text.find(R"("wingTier": "second")") != std::string::npos);
    // In the order of ItemTag, on one line.
    CHECK(text.find(R"("tags": ["jewel", "valuable"],)") != std::string::npos);
    CHECK(text.find(R"("sellable": false)") != std::string::npos);
    // Allowed actions are the default and left out.
    CHECK(text.find("\"tradable\"") == std::string::npos);
}

TEST_CASE("Item weapon kinds are written by name after the slot and read back [data][items]")
{
    ItemDefinition bow = MakeKris();
    bow.slot = ItemSlot::OffHand;
    bow.weaponKind = WeaponKind::Bow;

    const std::string text = WriteItemGroupJson(0, std::vector<ItemDefinition>{bow});
    const size_t slot = text.find(R"("slot": "offHand",)");
    const size_t weaponKind = text.find(R"("weaponKind": "bow",)");
    REQUIRE(slot != std::string::npos);
    REQUIRE(weaponKind != std::string::npos);
    CHECK(slot < weaponKind);

    const ReadResult result = Read(text);
    REQUIRE(result.issues.empty());
    REQUIRE(result.items.size() == 1);
    CHECK(result.items.front().weaponKind == WeaponKind::Bow);
}

TEST_CASE("Unknown slots, wing tiers, weapon kinds and tags are errors [data][items]")
{
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "slot": 11})")).issues,
                   ItemDataIssueSeverity::Error, "slot"));
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "slot": "tail"})")).issues,
                   ItemDataIssueSeverity::Error, "slot"));
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "wingTier": "fifth"})")).issues,
                   ItemDataIssueSeverity::Error, "wingTier"));
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "weaponKind": "wand"})")).issues,
                   ItemDataIssueSeverity::Error, "weaponKind"));
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "tags": ["jewl"]})")).issues,
                   ItemDataIssueSeverity::Error, "tags"));
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "tags": "jewel"})")).issues,
                   ItemDataIssueSeverity::Error, "tags"));
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "tradable": 0})")).issues,
                   ItemDataIssueSeverity::Error, "tradable"));
}

TEST_CASE("Item validation checks slots, wing tiers and weapon kinds [data][items]")
{
    ItemDefinition unnamedSlot = MakeKris();
    unnamedSlot.slot = static_cast<ItemSlot>(11);
    ItemDefinition wingTierWithoutWingSlot = MakeKris();
    wingTierWithoutWingSlot.number = 1;
    wingTierWithoutWingSlot.wingTier = WingTier::First;
    ItemDefinition weaponKindWithoutHandSlot = MakeKris();
    weaponKindWithoutHandSlot.slot = ItemSlot::Helm;
    weaponKindWithoutHandSlot.weaponKind = WeaponKind::Shield;
    // The code looks for a bow in the left hand only.
    ItemDefinition bowInRightHand = MakeKris();
    bowInRightHand.weaponKind = WeaponKind::Bow;
    ItemDefinition spear = MakeKris();
    spear.weaponKind = WeaponKind::Spear;
    ItemDefinition shield = MakeKris();
    shield.slot = ItemSlot::OffHand;
    shield.weaponKind = WeaponKind::Shield;
    // A book goes in the left hand.
    ItemDefinition bookInRightHand = MakeKris();
    bookInRightHand.weaponKind = WeaponKind::Book;
    // Group 0 gives a sword on mainHand.
    ItemDefinition swordWithoutKind = MakeKris();
    ItemDefinition swordWithPotionPriority = MakeKris();
    swordWithPotionPriority.weaponKind = WeaponKind::Sword;
    swordWithPotionPriority.potionPriority = 1;
    ItemDefinition potion = MakeKris();
    potion.slot = ItemSlot::None;
    potion.tags.Set(ItemTag::ShieldPotion);
    potion.potionPriority = 3;

    std::vector<ItemDataIssue> issues;
    ValidateItems(std::vector<ItemDefinition>{unnamedSlot}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Error, "slot"));

    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{wingTierWithoutWingSlot}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "wingTier"));
    CHECK_FALSE(HasErrors(issues));

    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{weaponKindWithoutHandSlot}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "weaponKind"));
    CHECK_FALSE(HasErrors(issues));

    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{bowInRightHand}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "weaponKind"));
    CHECK_FALSE(HasErrors(issues));

    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{spear}, issues);
    CHECK(issues.empty());

    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{shield}, issues);
    CHECK(issues.empty());

    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{bookInRightHand}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "weaponKind"));

    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{swordWithoutKind}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "weaponKind"));
    CHECK_FALSE(HasErrors(issues));

    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{swordWithPotionPriority}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "potionPriority"));
    CHECK_FALSE(HasErrors(issues));

    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{potion}, issues);
    CHECK(issues.empty());
}

TEST_CASE("Rental rules are kept through write and read [data][items]")
{
    ItemDefinition demon = MakeKris();
    demon.droppableWhileRented = false;
    demon.personalShopSellableWhileRented = false;
    demon.sellableWhenRentalExpired = true;

    const std::string text = WriteItemGroupJson(0, std::vector<ItemDefinition>{demon});
    CHECK(text.find(R"("sellableWhenRentalExpired": true)") != std::string::npos);

    const ReadResult result = Read(text);
    REQUIRE(result.items.size() == 1);
    CHECK_FALSE(result.items.front().droppableWhileRented);
    CHECK_FALSE(result.items.front().personalShopSellableWhileRented);
    CHECK(result.items.front().sellableWhenRentalExpired);
    CHECK_FALSE(result.items.front().IsAllowed(ItemAction::DropWhileRented));
    CHECK(result.items.front().IsAllowed(ItemAction::SellWhenRentalExpired));
}

TEST_CASE("A slot saved as a number before phase 3 still loads [data][items]")
{
    const ReadResult result = Read(GroupFile(0, R"({"number": 0, "name": "Kris", "slot": 7})"));

    REQUIRE(result.items.size() == 1);
    CHECK(result.items.front().slot == ItemSlot::Wings);
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "slot"));
    CHECK_FALSE(HasErrors(result.issues));
    CHECK(WriteItemGroupJson(0, result.items).find(R"("slot": "wings")") != std::string::npos);
}
