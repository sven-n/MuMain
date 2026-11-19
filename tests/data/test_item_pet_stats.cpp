#include "doctest.h"

#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace Data::Items;

namespace
{
const std::string Source = "Group13_Helper.json";

std::string GroupFile(const std::string& items)
{
    return R"({"formatVersion": 1, "group": 13, "items": [)" + items + "]}";
}

bool HasIssue(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& field)
{
    return std::any_of(issues.begin(), issues.end(),
                       [&](const ItemDataIssue& issue) { return issue.severity == severity && issue.field == field; });
}

ItemDefinition MakeHorse()
{
    ItemDefinition horse;
    horse.group = 13;
    horse.number = 4;
    horse.names = Data::LocalizedString::Parse("Dark Horse");
    horse.width = 1;
    horse.height = 1;
    horse.slot = ItemSlot::Pet;
    horse.tags.Set(ItemTag::DarkLordPet);
    horse.petStats.minimumDamage = {0, 5, 1, 10, 10, 0};
    horse.petStats.maximumDamage = {0, 0, 1, 0, 0, 150};
    horse.petStats.attackSpeed = {20, 4, 5, 0, 50, 0};
    return horse;
}
} // namespace

TEST_CASE("An item's pet stats are kept through write and read, without their default terms [data][items]")
{
    const ItemDefinition horse = MakeHorse();
    const std::string text = WriteItemGroupJson(13, std::vector<ItemDefinition>{horse});
    CHECK(text.find(R"("maximumDamage": {)") != std::string::npos);
    CHECK(text.find(R"("minimumPercent": 150)") != std::string::npos);
    // A level divisor of 1 and a term of 0 are the defaults, so they are left out.
    CHECK(text.find(R"("petLevelDivisor": 1)") == std::string::npos);
    CHECK(text.find(R"("add": 0)") == std::string::npos);

    std::vector<ItemDefinition> items;
    std::vector<ItemDataIssue> issues;
    ReadItemGroupJson(text, Source, items, issues);
    REQUIRE(issues.empty());
    REQUIRE(items.size() == 1);
    CHECK(items.front().petStats == horse.petStats);

    // Without pet stats there is no field.
    ItemDefinition other = MakeHorse();
    other.petStats = {};
    CHECK(WriteItemGroupJson(13, std::vector<ItemDefinition>{other}).find("petStats") == std::string::npos);
}

TEST_CASE("Pet stats that are not numbers, or name an unknown term, are reported when read [data][items]")
{
    const auto read = [](const std::string& petStats)
    {
        std::vector<ItemDefinition> items;
        std::vector<ItemDataIssue> issues;
        ReadItemGroupJson(GroupFile(R"({"number": 4, "name": "Dark Horse", "petStats": )" + petStats + "}"), Source,
                          items, issues);
        return issues;
    };
    CHECK(HasIssue(read(R"([])"), ItemDataIssueSeverity::Fatal, "petStats"));
    CHECK(HasIssue(read(R"({"maximumDamage": 3})"), ItemDataIssueSeverity::Fatal, "petStats.maximumDamage"));
    CHECK(
        HasIssue(read(R"({"attackSpeed": {"add": "20"}})"), ItemDataIssueSeverity::Fatal, "petStats.attackSpeed.add"));
    CHECK(HasIssue(read(R"({"defense": {}})"), ItemDataIssueSeverity::Warning, "petStats.defense"));
    CHECK(HasIssue(read(R"({"attackSpeed": {"agilityDivisor": 5}})"), ItemDataIssueSeverity::Warning,
                   "petStats.attackSpeed.agilityDivisor"));
}

TEST_CASE("Pet stats are checked: divisors, the minimum's share and the pet tag [data][items]")
{
    std::vector<ItemDataIssue> issues;
    ValidateItems(std::vector<ItemDefinition>{MakeHorse()}, issues);
    CHECK_FALSE(HasIssue(issues, ItemDataIssueSeverity::Fatal, "petStats.minimumDamage"));
    CHECK_FALSE(HasIssue(issues, ItemDataIssueSeverity::Warning, "petStats"));

    ItemDefinition untagged = MakeHorse();
    untagged.tags.Set(ItemTag::DarkLordPet, false);
    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{untagged}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "petStats"));

    ItemDefinition missing = MakeHorse();
    missing.petStats = {};
    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{missing}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "petStats"));
    CHECK_FALSE(HasIssue(issues, ItemDataIssueSeverity::Fatal, "petStats"));

    ItemDefinition selfShare = MakeHorse();
    selfShare.petStats.minimumDamage.minimumPercent = 50;
    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{selfShare}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "petStats.minimumDamage"));
    CHECK_FALSE(HasIssue(issues, ItemDataIssueSeverity::Fatal, "petStats.minimumDamage"));

    ItemDefinition negative = MakeHorse();
    negative.petStats.attackSpeed.commandDivisor = -50;
    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{negative}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "petStats.attackSpeed"));
    CHECK_FALSE(HasIssue(issues, ItemDataIssueSeverity::Fatal, "petStats.attackSpeed"));

    ItemDefinition noLevelDivisor = MakeHorse();
    noLevelDivisor.petStats.attackSpeed.petLevelDivisor = 0;
    issues.clear();
    ValidateItems(std::vector<ItemDefinition>{noLevelDivisor}, issues);
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "petStats.attackSpeed"));
    CHECK_FALSE(HasIssue(issues, ItemDataIssueSeverity::Fatal, "petStats.attackSpeed"));
}

TEST_CASE("A pet stat adds its terms, each division rounded down, and a share of the minimum [data][items]")
{
    const ItemDefinition horse = MakeHorse();
    // Minimum: 100 / 10 + 310 / 10 + 1 * 5 = 46; maximum 150 % of it, 69; attack speed 20 + 4 / 5 + 310 / 50 = 26.
    CHECK(EvaluatePetStats(horse.petStats, {1, 100, 310}) == PetStatValues{46, 69, 26});
    CHECK(EvaluatePetStats(ItemPetStats{}, {10, 100, 310}) == PetStatValues{});
}
