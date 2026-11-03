#include "doctest.h"

#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"
#include "Data/GameData/ItemData/ItemStatFile.h"

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

TEST_CASE("An item file entry without a name is kept through read and write [data][items]")
{
    const std::string text = WriteItemGroupJson(0, std::vector<ItemDefinition>{MakeKris()});
    const ReadResult kris = Read(text);
    REQUIRE(kris.items.size() == 1);
    CHECK_FALSE(kris.items.front().entryWithoutName);

    const ReadResult result = Read(GroupFile(0, R"({"number": 3})"));
    REQUIRE(result.issues.empty());
    REQUIRE(result.items.size() == 1);
    const ItemDefinition& entry = result.items.front();
    CHECK(entry.entryWithoutName);
    CHECK(entry.HasEntry());
    CHECK_FALSE(entry.Exists());
    CHECK(entry.number == 3);

    const std::string written = WriteItemGroupJson(0, result.items);
    CHECK(written.find("\"name\"") == std::string::npos);
    CHECK(Read(written).items == result.items);

    // Only its id is checked; an item without a name is still an error.
    std::vector<ItemDataIssue> issues;
    ValidateItems(result.items, issues);
    CHECK(issues.empty());
    ItemDefinition twice = entry;
    ValidateItems(std::vector<ItemDefinition>{entry, twice}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Fatal, "number"));
}

TEST_CASE("An empty item name is not an entry without a name [data][items]")
{
    for (const std::string name : {R"("")", "{}", R"({"en": ""})"})
    {
        INFO(name);
        const ReadResult result = Read(GroupFile(0, R"({"number": 3, "name": )" + name + "}"));
        REQUIRE(result.items.size() == 1);
        CHECK_FALSE(result.items.front().entryWithoutName);
        CHECK_FALSE(result.items.front().HasEntry());

        // The English name is missing, as for any item.
        std::vector<ItemDataIssue> issues;
        ValidateItems(result.items, issues);
        CHECK(HasIssue(issues, ItemDataIssueSeverity::Fatal, "name"));
    }
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
    CHECK(result.issues.front().severity == ItemDataIssueSeverity::Fatal);
    CHECK(result.issues.front().source == Source);
}

TEST_CASE("A newer item file format is rejected [data][items]")
{
    const ReadResult result = Read(R"({"formatVersion": 99, "group": 0, "items": []})");

    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Fatal, "formatVersion"));
}

TEST_CASE("Item groups and numbers must be in range [data][items]")
{
    CHECK(HasIssue(Read(GroupFile(16, "")).issues, ItemDataIssueSeverity::Fatal, "group"));
    CHECK(
        HasIssue(Read(GroupFile(0, R"({"number": 512, "name": "X"})")).issues, ItemDataIssueSeverity::Fatal, "number"));
}

TEST_CASE("Item values must fit their type [data][items]")
{
    const ReadResult result = Read(GroupFile(0, R"({"number": 0, "name": "Kris", "width": 256, "twoHanded": 1})"));

    CHECK(result.items.empty());
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Fatal, "width"));
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Fatal, "twoHanded"));
}

TEST_CASE("Damage and magic power past 255 are kept through write and read [data][items]")
{
    // A later-season weapon and staff.
    ItemDefinition sword = MakeKris();
    sword.damageMin = 345;
    sword.damageMax = 380;
    sword.magicPower = 400;

    const std::string text = WriteItemGroupJson(0, std::vector<ItemDefinition>{sword});
    CHECK(text.find(R"("damageMin": 345)") != std::string::npos);
    const ReadResult result = Read(text);
    REQUIRE(result.items.size() == 1);
    CHECK(result.issues.empty());
    CHECK(result.items.front().damageMin == 345);
    CHECK(result.items.front().damageMax == 380);
    CHECK(result.items.front().magicPower == 400);

    // Up to what the field holds.
    const ReadResult tooHigh = Read(GroupFile(0, R"({"number": 0, "name": "Kris", "damageMax": 65536})"));
    CHECK(HasIssue(tooHigh.issues, ItemDataIssueSeverity::Fatal, "damageMax"));
}

TEST_CASE("Unknown item fields are warnings [data][items]")
{
    const ReadResult result =
        Read(GroupFile(0, R"({"number": 0, "name": "Kris", "sparkle": 1, "requirements": {"luck": 1}})"));

    REQUIRE(result.items.size() == 1);
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "sparkle"));
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "requirements.luck"));
    CHECK_FALSE(HasFatal(result.issues));
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

    CHECK(HasIssue(issues, ItemDataIssueSeverity::Fatal, "number"));
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Fatal, "name"));
}

TEST_CASE("Item issues describe where the problem is [data][items]")
{
    const ItemDataIssue issue{ItemDataIssueSeverity::Fatal, Source, 0, 5, "width", "must be a whole number"};

    CHECK(issue.ToString() == "fatal: Group00_Sword.json: item (0,5) width: must be a whole number");
}

TEST_CASE("Item issues name their level: fatal, error or warning [data][items]")
{
    ItemDataIssue issue{ItemDataIssueSeverity::Error, Source, ItemDataIssue::NoItem,
                        ItemDataIssue::NoItem,        "",     "names a model file that is missing"};
    CHECK(issue.ToString() == "error: Group00_Sword.json: names a model file that is missing");
    issue.severity = ItemDataIssueSeverity::Warning;
    CHECK(issue.ToString() == "warning: Group00_Sword.json: names a model file that is missing");
}

TEST_CASE("Only fatal issues make data unusable [data][items]")
{
    std::vector<ItemDataIssue> issues{{ItemDataIssueSeverity::Error, Source, 0, 5, "model", "missing"},
                                      {ItemDataIssueSeverity::Warning, Source, 0, 5, "name.pt", "missing"}};
    CHECK_FALSE(Data::Items::HasFatal(issues));
    issues.push_back({ItemDataIssueSeverity::Fatal, Source, 0, 5, "width", "must be a whole number"});
    CHECK(Data::Items::HasFatal(issues));
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
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Fatal, "name.pt"));
}

TEST_CASE("Numbers too large for their field do not wrap around [data][items]")
{
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 4294967297, "name": "X"})")).issues, ItemDataIssueSeverity::Fatal,
                   "number"));
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "buyPrice": 18446744073709551615})")).issues,
                   ItemDataIssueSeverity::Fatal, "buyPrice"));
    CHECK(HasIssue(Read(R"({"formatVersion": 4294967297, "group": 0, "items": []})").issues,
                   ItemDataIssueSeverity::Fatal, "formatVersion"));
    CHECK(HasIssue(Read(R"({"formatVersion": 1, "group": 4294967296, "items": []})").issues,
                   ItemDataIssueSeverity::Fatal, "group"));
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
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Fatal, "name"));

    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{empty}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Fatal, "name"));

    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{valid}, issues);
    CHECK_FALSE(HasFatal(issues));
}

TEST_CASE("An item of a kind without class lines warns when it lacks noClassLines [data][items]")
{
    ItemDefinition elixir = MakeKris();
    elixir.tags.Set(ItemTag::Elixir);
    std::vector<ItemDataIssue> issues;
    ValidateItems(std::vector<ItemDefinition>{elixir}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "tags"));
    CHECK_FALSE(HasFatal(issues));

    elixir.tags.Set(ItemTag::NoClassLines);
    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{elixir}, issues);
    CHECK_FALSE(HasIssue(issues, ItemDataIssueSeverity::Warning, "tags"));

    // Any other item may show its class lines.
    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{MakeKris()}, issues);
    CHECK_FALSE(HasIssue(issues, ItemDataIssueSeverity::Warning, "tags"));
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

TEST_CASE("An item's description is kept through write and read [data][items]")
{
    ItemDefinition jewel = MakeKris();
    jewel.group = 14;
    jewel.number = 13;
    jewel.slot = ItemSlot::None;
    jewel.weaponKind = WeaponKind::None;
    jewel.statProfile.clear();
    // The Jewel of Bless's own tooltip line, as the client's translations have it.
    ItemDescriptionLine first;
    first.text.Set("en", "It is used to increase your item level up to 6");
    first.text.Set("de", "Wird verwendet, um das Gegenstandslevel auf bis zu 6 zu erhöhen");
    first.text.Set("es", "Se utiliza para aumentar el nivel de tu objeto hasta 6.");
    first.text.Set("pt", "É usado para aumentar o nível do seu item até 6");
    ItemDescriptionLine blank;
    ItemDescriptionLine last = first;
    last.color = ItemTextColor::Blue;
    jewel.description = {first, blank, last};

    const std::string text = WriteItemGroupJson(14, std::vector<ItemDefinition>{jewel});
    CHECK(text.find("\"color\": \"blue\"") != std::string::npos);
    const ReadResult result = Read(text);
    REQUIRE(result.items.size() == 1);
    CHECK(result.items.front().description == jewel.description);
    CHECK_FALSE(HasFatal(result.issues));
    CHECK(WriteItemGroupJson(14, result.items) == text);

    // A line break in a text is kept; the tooltip starts a new line there.
    ItemDefinition twoLines = jewel;
    twoLines.description = {first};
    twoLines.description[0].text.Set("en", "It is used to increase\nyour item level up to 6");
    const ReadResult broken = Read(WriteItemGroupJson(14, std::vector<ItemDefinition>{twoLines}));
    REQUIRE(broken.items.size() == 1);
    CHECK(broken.items.front().description == twoLines.description);

    const ReadResult badColor = Read(R"({"formatVersion": 1, "group": 14, "items": [
        {"number": 13, "name": "Jewel of Bless", "description": [{"text": "x", "color": "pink"}]}]})");
    CHECK(HasFatal(badColor.issues));
}

TEST_CASE("A description line's item levels are kept through write and read, and checked [data][items]")
{
    ItemDefinition scroll = MakeKris();
    scroll.group = 14;
    scroll.number = 10;
    scroll.slot = ItemSlot::None;
    scroll.weaponKind = WeaponKind::None;
    scroll.statProfile.clear();
    ItemDescriptionLine warp;
    warp.text.Set("en", "Warp to the corresponding area after 3 seconds");
    warp.minLevel = 1;
    warp.maxLevel = 8;
    ItemDescriptionLine always;
    always.text.Set("en", "Shown at every level");
    scroll.description = {warp, always};

    CHECK_FALSE(warp.IsShownAt(0));
    CHECK(warp.IsShownAt(1));
    CHECK(warp.IsShownAt(8));
    CHECK_FALSE(warp.IsShownAt(9));
    CHECK(always.IsShownAt(0));
    CHECK(always.IsShownAt(ItemLevelCount - 1));

    // Written on one line, and only for a line that is not shown at every level.
    const std::string text = WriteItemGroupJson(14, std::vector<ItemDefinition>{scroll});
    CHECK(text.find("\"levels\": [1, 8]") != std::string::npos);
    CHECK(text.find("\"levels\"") == text.rfind("\"levels\""));
    const ReadResult result = Read(text);
    REQUIRE(result.items.size() == 1);
    CHECK(result.items.front().description == scroll.description);
    CHECK_FALSE(HasFatal(result.issues));
    CHECK(WriteItemGroupJson(14, result.items) == text);

    for (const char* levels : {R"("levels": [8, 1])", R"("levels": [0, 16])", R"("levels": [-1, 2])",
                               R"("levels": [3])", R"("levels": 3)", R"("levels": [1.5, 2])",
                               // Outside int's range: they'd wrap into a level (2^32 + 1 into 1) if read as an int.
                               R"("levels": [4294967297, 4294967298])", R"("levels": [-4294967295, 2])",
                               R"("levels": [0, 18446744073709551615])", R"("levels": [-9223372036854775808, 2])"})
    {
        INFO(levels);
        const ReadResult bad = Read(std::string(R"({"formatVersion": 1, "group": 14, "items": [
            {"number": 10, "name": "Town Portal Scroll", "description": [{"text": "x", )") +
                                    levels + "}]}]}");
        CHECK(HasIssue(bad.issues, ItemDataIssueSeverity::Fatal, "description[0].levels"));
    }
}

TEST_CASE("An item's name lines are kept through write and read, with or without a text and a color [data][items]")
{
    ItemDefinition horn = MakeKris();
    horn.group = 13;
    horn.number = 37;
    horn.slot = ItemSlot::None;
    horn.weaponKind = WeaponKind::None;
    horn.statProfile.clear();
    // The Horn of Fenrir's Destroy kind, then its plain name; a jewel keeps the standard name in yellow.
    ItemNameLine destroy;
    destroy.hasText = destroy.hasColor = true;
    destroy.text.Set("en", "{name} +Destroy");
    destroy.color = ItemTextColor::Blue;
    TooltipCondition kind;
    kind.value = TooltipValue::ExcellentFlags;
    kind.comparison = TooltipCondition::Comparison::Equal;
    kind.number = 1;
    destroy.when = {kind};
    ItemNameLine plain;
    plain.hasText = plain.hasColor = true;
    plain.text.Set("en", "{name}");
    plain.color = ItemTextColor::White;
    ItemNameLine colorOnly;
    colorOnly.hasColor = true;
    colorOnly.color = ItemTextColor::Yellow;
    colorOnly.minLevel = 1;
    colorOnly.maxLevel = 3;
    horn.nameLines = {destroy, plain, colorOnly};

    const std::string text = WriteItemGroupJson(13, std::vector<ItemDefinition>{horn});
    // A white name line writes its color, which keeps a standard color off it.
    CHECK(text.find("\"color\": \"white\"") != std::string::npos);
    const ReadResult result = Read(text);
    REQUIRE(result.items.size() == 1);
    CHECK(result.items.front().nameLines == horn.nameLines);
    CHECK_FALSE(HasFatal(result.issues));
    CHECK(WriteItemGroupJson(13, result.items) == text);

    const ReadResult notObject = Read(R"({"formatVersion": 1, "group": 13, "items": [
        {"number": 37, "name": "Horn of Fenrir", "nameLines": ["{name}"]}]})");
    CHECK(HasIssue(notObject.issues, ItemDataIssueSeverity::Fatal, "nameLines[0]"));

    // The name is always bold and the first row: a name line's "bold" and "after" are errors.
    const ReadResult boldAndAfter = Read(R"({"formatVersion": 1, "group": 13, "items": [
        {"number": 37, "name": "Horn of Fenrir", "nameLines": [{"text": "{name}", "bold": true, "after": "options"}]}]})");
    CHECK(HasIssue(boldAndAfter.issues, ItemDataIssueSeverity::Fatal, "nameLines[0].bold"));
    CHECK(HasIssue(boldAndAfter.issues, ItemDataIssueSeverity::Fatal, "nameLines[0].after"));
}

TEST_CASE("A description or name line longer than the tooltip shows is a warning [data][items]")
{
    const auto read = [](const std::string& field, const std::string& text)
    {
        return Read(
            GroupFile(13, R"({"number": 13, "name": "Jewel of Bless", ")" + field + R"(": [{"text": )" + text + "}]}"));
    };
    const auto quoted = [](size_t length, const std::string& character = "x")
    {
        std::string text;
        for (size_t index = 0; index < length; ++index)
        {
            text += character;
        }
        return "\"" + text + "\"";
    };

    // Up to the limit, nothing; one character more, a warning naming the language, and the line is still read.
    CHECK(read("description", quoted(MaxDescriptionLineLength)).issues.empty());
    const ReadResult longDescription = read("description", quoted(MaxDescriptionLineLength + 1));
    CHECK(HasIssue(longDescription.issues, ItemDataIssueSeverity::Warning, "description[0].text"));
    CHECK_FALSE(HasFatal(longDescription.issues));
    REQUIRE(longDescription.items.size() == 1);
    CHECK(longDescription.items.front().description.size() == 1);
    CHECK(read("nameLines", quoted(MaxNameLineLength)).issues.empty());
    CHECK(HasIssue(read("nameLines", quoted(MaxNameLineLength + 1)).issues, ItemDataIssueSeverity::Warning,
                   "nameLines[0].text"));

    // Counted in UTF-16 characters, in every language: "ä" is one, an emoji two.
    CHECK(read("nameLines", R"({"en": "x", "de": )" + quoted(MaxNameLineLength, "ä") + "}").issues.empty());
    CHECK(HasIssue(read("nameLines", R"({"en": "x", "de": )" + quoted(MaxNameLineLength / 2 + 1, "😀") + "}").issues,
                   ItemDataIssueSeverity::Warning, "nameLines[0].text"));
}

TEST_CASE("A description line's \"@\" names a shared text [data][items]")
{
    // "@" alone names none: an error, as in a layout, not a silent blank line.
    const ReadResult empty = Read(R"({"formatVersion": 1, "group": 14, "items": [
        {"number": 10, "name": "Town Portal Scroll", "description": [{"text": "@"}]}]})");
    CHECK(HasIssue(empty.issues, ItemDataIssueSeverity::Fatal, "description[0].text"));
    const ReadResult named = Read(R"({"formatVersion": 1, "group": 14, "items": [
        {"number": 10, "name": "Town Portal Scroll", "description": [{"text": "@durability"}]}]})");
    CHECK_FALSE(HasFatal(named.issues));
    REQUIRE(named.items.size() == 1);
    CHECK(named.items.front().description.front().textName == "durability");
    // So does a level value's.
    const ReadResult levelValue = Read(R"({"formatVersion": 1, "group": 14, "items": [
        {"number": 10, "name": "Town Portal Scroll", "levelValues": {"stat": ["ENG", "@"]}}]})");
    CHECK(HasIssue(levelValue.issues, ItemDataIssueSeverity::Fatal, "levelValues.stat[1]"));
}

TEST_CASE("An item's stat profile is kept through write and read [data][items]")
{
    ItemDefinition kris = MakeKris();
    kris.statProfile = "weapon";

    const std::string text = WriteItemGroupJson(0, std::vector<ItemDefinition>{kris});
    CHECK(text.find(R"("statProfile": "weapon")") != std::string::npos);
    const ReadResult result = Read(text);
    REQUIRE(result.issues.empty());
    REQUIRE(result.items.size() == 1);
    CHECK(result.items.front().statProfile == "weapon");

    // The default profile is not written.
    CHECK(WriteItemGroupJson(0, std::vector<ItemDefinition>{MakeKris()}).find("statProfile") == std::string::npos);

    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "statProfile": "two words"})")).issues,
                   ItemDataIssueSeverity::Fatal, "statProfile"));
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "statProfile": 5})")).issues,
                   ItemDataIssueSeverity::Fatal, "statProfile"));
}

TEST_CASE("An item's excellent bonus is kept through write and read, and warns without damage [data][items]")
{
    ItemDefinition axe = MakeKris();
    axe.excellentBonus = 15;
    const ReadResult result = Read(WriteItemGroupJson(0, std::vector<ItemDefinition>{axe}));
    REQUIRE(result.issues.empty());
    REQUIRE(result.items.size() == 1);
    CHECK(result.items.front().excellentBonus == 15);

    ItemDefinition helm = MakeKris();
    helm.damageMin = 0;
    helm.damageMax = 0;
    helm.excellentBonus = 15;
    std::vector<ItemDataIssue> issues;
    ValidateItems(std::vector<ItemDefinition>{helm}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "excellentBonus"));
    CHECK_FALSE(HasFatal(issues));

    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{axe}, issues);
    CHECK_FALSE(HasIssue(issues, ItemDataIssueSeverity::Warning, "excellentBonus"));
}

TEST_CASE("An item's excellentRaisesLevel is kept through write and read, and warns without a level requirement "
          "[data][items]")
{
    ItemDefinition helm = MakeKris();
    helm.requirements.level = 380;
    helm.excellentRaisesLevel = false;
    const std::string text = WriteItemGroupJson(0, std::vector<ItemDefinition>{helm});
    CHECK(text.find(R"("excellentRaisesLevel": false)") != std::string::npos);
    const ReadResult result = Read(text);
    REQUIRE(result.items.size() == 1);
    CHECK_FALSE(result.items.front().excellentRaisesLevel);
    // The default is not written.
    CHECK(WriteItemGroupJson(0, std::vector<ItemDefinition>{MakeKris()}).find("excellentRaisesLevel") ==
          std::string::npos);

    std::vector<ItemDataIssue> issues;
    ValidateItems(std::vector<ItemDefinition>{helm}, issues);
    CHECK_FALSE(HasIssue(issues, ItemDataIssueSeverity::Warning, "excellentRaisesLevel"));
    helm.requirements.level = 0;
    ValidateItems(std::vector<ItemDefinition>{helm}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "excellentRaisesLevel"));
}

TEST_CASE("An item's requirements by level are kept through write and read [data][items]")
{
    ItemDefinition orb = MakeKris();
    orb.requirements = {};
    RequirementByLevel same{};
    same.fill(7);
    // Set in any order; they are written in the order of the requirements.
    orb.requirementsByLevel.Set(ItemRequirement::Leadership, same);
    orb.requirementsByLevel.Set(ItemRequirement::Energy,
                                RequirementByLevel{30, 60, 90, 130, 170, 210, 300, 500, 0, 0, 0, 0, 0, 0, 0, 0});

    const std::string text = WriteItemGroupJson(0, std::vector<ItemDefinition>{orb});
    CHECK(
        text.find(
            R"("requirementsByLevel": {"energy": [30, 60, 90, 130, 170, 210, 300, 500, 0, 0, 0, 0, 0, 0, 0, 0], "leadership": 7})") !=
        std::string::npos);
    const ReadResult result = Read(text);
    REQUIRE(result.issues.empty());
    REQUIRE(result.items.size() == 1);
    CHECK(result.items.front().requirementsByLevel == orb.requirementsByLevel);

    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "requirementsByLevel": {"luck": 1}})")).issues,
                   ItemDataIssueSeverity::Fatal, "requirementsByLevel.luck"));
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "requirementsByLevel": {"level": [1, 2]}})")).issues,
                   ItemDataIssueSeverity::Fatal, "requirementsByLevel.level"));
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "requirementsByLevel": {"level": 70000}})")).issues,
                   ItemDataIssueSeverity::Fatal, "requirementsByLevel.level"));

    // A base value next to the list is not used.
    ItemDefinition both = orb;
    both.requirements.energy = 10;
    std::vector<ItemDataIssue> issues;
    ValidateItems(std::vector<ItemDefinition>{both}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "requirementsByLevel"));
}

TEST_CASE("Item validation checks the stat profiles against the stat file [data][items]")
{
    ItemStatFile file;
    file.profiles.push_back({"shield", {}});
    file.profiles.push_back({"sceptre", {}});
    const ItemStatProfiles profiles(file);

    ItemDefinition shield = MakeKris();
    shield.statProfile = "shield";
    ItemDefinition unknown = MakeKris();
    unknown.number = 1;
    unknown.statProfile = "wing";
    ItemDefinition withoutProfile = MakeKris();
    withoutProfile.number = 2;
    // Deleted items (no name) count for nothing.
    ItemDefinition deletedUnknown = MakeKris();
    deletedUnknown.number = 3;
    deletedUnknown.names = {};
    deletedUnknown.statProfile = "cape";
    ItemDefinition deletedSceptre = deletedUnknown;
    deletedSceptre.number = 4;
    deletedSceptre.statProfile = "sceptre";

    std::vector<ItemDataIssue> issues;
    ValidateStatProfiles(std::vector<ItemDefinition>{shield, unknown, withoutProfile, deletedUnknown, deletedSceptre},
                         profiles, "ItemStats.json", issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Fatal, "statProfile"));
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "profiles.sceptre"));
    CHECK_FALSE(HasIssue(issues, ItemDataIssueSeverity::Warning, "profiles.shield"));
    CHECK(issues.size() == 2);
}

TEST_CASE("Item validation warns about an excellent bonus the stat profile has no term for [data][items]")
{
    // Without an excellent damage term the Chaos Dragon Axe's fixed bonus would not count.
    ItemStatFile file;
    file.profiles.push_back({"plain", {}});
    WrittenStatProfile withTerm{"withTerm", {}};
    withTerm.stats[static_cast<size_t>(ItemStat::Damage)].excellent = {WrittenStatTerm::State::Values, {25, 5}};
    file.profiles.push_back(withTerm);
    const ItemStatProfiles profiles(file);

    ItemDefinition axe = MakeKris();
    axe.excellentBonus = 15;
    axe.statProfile = "plain";
    std::vector<ItemDataIssue> issues;
    ValidateStatProfiles(std::vector<ItemDefinition>{axe}, profiles, "ItemStats.json", issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "excellentBonus"));

    axe.statProfile = "withTerm";
    issues.clear();
    ValidateStatProfiles(std::vector<ItemDefinition>{axe}, profiles, "ItemStats.json", issues);
    CHECK_FALSE(HasIssue(issues, ItemDataIssueSeverity::Warning, "excellentBonus"));
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
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "slot": 11})")).issues, ItemDataIssueSeverity::Fatal,
                   "slot"));
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "slot": "tail"})")).issues,
                   ItemDataIssueSeverity::Fatal, "slot"));
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "wingTier": "fifth"})")).issues,
                   ItemDataIssueSeverity::Fatal, "wingTier"));
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "weaponKind": "wand"})")).issues,
                   ItemDataIssueSeverity::Fatal, "weaponKind"));
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "tags": ["jewl"]})")).issues,
                   ItemDataIssueSeverity::Fatal, "tags"));
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "tags": "jewel"})")).issues,
                   ItemDataIssueSeverity::Fatal, "tags"));
    CHECK(HasIssue(Read(GroupFile(0, R"({"number": 0, "name": "X", "tradable": 0})")).issues,
                   ItemDataIssueSeverity::Fatal, "tradable"));
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
    spear.statProfile = "weapon";
    ItemDefinition shield = MakeKris();
    shield.slot = ItemSlot::OffHand;
    shield.weaponKind = WeaponKind::Shield;
    shield.statProfile = "shield";
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
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Fatal, "slot"));

    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{wingTierWithoutWingSlot}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "wingTier"));
    CHECK_FALSE(HasFatal(issues));

    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{weaponKindWithoutHandSlot}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "weaponKind"));
    CHECK_FALSE(HasFatal(issues));

    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{bowInRightHand}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "weaponKind"));
    CHECK_FALSE(HasFatal(issues));

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
    CHECK_FALSE(HasFatal(issues));

    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{swordWithPotionPriority}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "potionPriority"));
    CHECK_FALSE(HasFatal(issues));

    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{potion}, issues);
    CHECK(issues.empty());
}

TEST_CASE("Item validation warns about an item without the stat profile the profile rules give [data][items]")
{
    ItemStatFile file;
    file.profiles.push_back({"shield", {}});
    StatProfileRule shields{.profile = "shield"};
    shields.weaponKinds.set(static_cast<size_t>(WeaponKind::Shield));
    file.profileRules.push_back(shields);
    const ItemStatProfiles profiles(file);

    // A shield in an item file saved before the stat profiles.
    ItemDefinition shield = MakeKris();
    shield.slot = ItemSlot::OffHand;
    shield.weaponKind = WeaponKind::Shield;

    std::vector<ItemDataIssue> issues;
    ValidateStatProfiles(std::vector<ItemDefinition>{shield}, profiles, "ItemStats.json", issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "statProfile"));
    CHECK_FALSE(HasFatal(issues));
    // The item checks run before the stat file is read; they leave it to the stat profile checks.
    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{shield}, issues);
    CHECK(issues.empty());

    shield.statProfile = "default";
    issues.clear();
    ValidateStatProfiles(std::vector<ItemDefinition>{shield}, profiles, "ItemStats.json", issues);
    CHECK_FALSE(HasIssue(issues, ItemDataIssueSeverity::Warning, "statProfile"));
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
    CHECK_FALSE(HasFatal(result.issues));
    CHECK(WriteItemGroupJson(0, result.items).find(R"("slot": "wings")") != std::string::npos);
}
