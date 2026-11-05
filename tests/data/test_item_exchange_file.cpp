#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "ClientDataScope.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemExchangeFile.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"

#include <algorithm>
#include <filesystem>
#include <set>
#include <string>
#include <vector>

using namespace Data::Items;

// The item exchange file with OpenMU (phase 7): the shared fields of the items and the item
// formulas, in the layout of the item files.
namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

class ShippedItems
{
public:
    ShippedItems() : m_client(ClientDirectory)
    {
        I18N::SetLocale("en");
        std::string errorMessage;
        REQUIRE(g_ItemDataHandler.Load(errorMessage));
    }

private:
    ClientDataScope m_client;
};

ItemExchangeFile ReadExchange(const std::string& text, std::vector<ItemDataIssue>& issues)
{
    ItemExchangeFile file;
    ReadItemExchangeJson(text, "exchange.json", file, issues);
    return file;
}

bool HasIssueAbout(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& text)
{
    for (const ItemDataIssue& issue : issues)
    {
        if (issue.severity == severity && issue.ToString().find(text) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}

// A file of one item around `item` (the JSON of one item, with its group).
std::string FileWithItem(const std::string& item,
                         const std::string& header = R"("source": "OpenMU", "gameVersion": "season6")")
{
    return R"({"formatVersion": 1, )" + header + R"(, "items": [)" + item + "]}";
}
} // namespace

TEST_CASE(
    "Every shipped item and the shipped formulas go to the exchange file and back unchanged [data][items][editor]")
{
    ShippedItems items;
    const ItemExchangeFile built = BuildItemExchange(g_ItemDatabase.GetAllSlots(), g_ItemDataHandler.GetStatFile());
    REQUIRE(built.items.size() == 911);
    REQUIRE(built.formulas);
    CHECK(built.formulas->profileRules.empty());
    CHECK(built.formulas->profiles == g_ItemDataHandler.GetStatFile().profiles);

    const std::string text = WriteItemExchangeJson(built);
    std::vector<ItemDataIssue> issues;
    const ItemExchangeFile read = ReadExchange(text, issues);
    CHECK(issues.empty());
    CHECK(read.source == ItemExchangeSourceMuMain);
    CHECK(read.server.empty());
    CHECK(read.gameVersion == ItemExchangeGameVersion);
    REQUIRE(read.formulas);
    CHECK(*read.formulas == *built.formulas);
    REQUIRE(read.items.size() == built.items.size());
    for (size_t index = 0; index < built.items.size(); ++index)
    {
        CAPTURE(g_ItemDatabase.GetLogName(MakeItemType(built.items[index].group, built.items[index].number)));
        CHECK(read.items[index] == built.items[index]);
    }
    // The same data gives the same text.
    CHECK(WriteItemExchangeJson(read) == text);

    // In the layout of the item files and the item stat file.
    CHECK(text.find(R"("dropLevelBonus": {"excellent": 25, "ancient": 30})") != std::string::npos);
    CHECK(text.find(R"("tags": ["ammunition"])") != std::string::npos);
    CHECK(text.find("profileRules") == std::string::npos);
}

TEST_CASE("The exchange file has only the shared fields of an item [data][items][editor]")
{
    ShippedItems items;
    const ItemDefinition kris = *g_ItemDatabase.Find(MakeItemType(0, 0));
    REQUIRE(kris.weaponKind != WeaponKind::None);
    const ItemDefinition shared = ToExchangeItem(kris);
    CHECK(shared.names == kris.names);
    CHECK(shared.requirements == kris.requirements);
    CHECK(shared.classRequirements == kris.classRequirements);
    CHECK(shared.statProfile == "weapon");
    // Client-only: the weapon kind and the attack type.
    CHECK(shared.weaponKind == WeaponKind::None);
    CHECK(shared.attackType == 0);

    // The tags OpenMU has too; a profile the item has by default is named.
    ItemDefinition arrows = *g_ItemDatabase.Find(MakeItemType(4, 15));
    REQUIRE(arrows.tags.Has(ItemTag::Ammunition));
    arrows.tags.Set(ItemTag::Valuable);
    arrows.statProfile.clear();
    const ItemDefinition sharedArrows = ToExchangeItem(arrows);
    CHECK(sharedArrows.tags.Has(ItemTag::Ammunition));
    CHECK_FALSE(sharedArrows.tags.Has(ItemTag::Valuable));
    CHECK(sharedArrows.statProfile == "default");
}

TEST_CASE("The exchange file reader refuses what cannot be exchanged [data][items][editor]")
{
    const std::string kris = R"({"group": 0, "number": 0, "name": {"en": "Kris"}, "width": 1, "height": 2})";
    std::vector<ItemDataIssue> issues;

    SUBCASE("a file of another game version")
    {
        ReadExchange(FileWithItem(kris, R"("source": "OpenMU", "gameVersion": "season4")"), issues);
        CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Fatal, "season6"));
    }
    SUBCASE("an unknown source")
    {
        ReadExchange(FileWithItem(kris, R"("source": "Other", "gameVersion": "season6")"), issues);
        CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Fatal, "MuMain"));
    }
    SUBCASE("an empty server is as if there were none")
    {
        const ItemExchangeFile file = ReadExchange(
            R"({"formatVersion": 1, "source": "OpenMU", "server": "", "gameVersion": "season6", "items": []})", issues);
        CHECK_FALSE(HasFatal(issues));
        CHECK(file.server.empty());
    }
    SUBCASE("a newer format")
    {
        ReadExchange(R"({"formatVersion": 3, "source": "OpenMU", "gameVersion": "season6", "items": []})", issues);
        CHECK(HasFatal(issues));
    }
    SUBCASE("an item without a group")
    {
        const ItemExchangeFile file = ReadExchange(FileWithItem(R"({"number": 0, "name": "Kris"})"), issues);
        CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Fatal, "group"));
        CHECK(file.items.empty());
        CHECK_FALSE(CanImportItemExchange(file, issues));
    }
    SUBCASE("an item that is there twice")
    {
        const std::string other = R"({"group": 0, "number": 1, "name": "Short Sword"})";
        const ItemExchangeFile file = ReadExchange(FileWithItem(kris + ", " + other + ", " + kris), issues);
        CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Fatal, "2 times"));
        REQUIRE(file.items.size() == 1);
        CHECK(file.items[0].number == 1);
        CHECK(file.repeatedItems == std::vector<std::pair<int, int>>{{0, 0}});
        // The rest of the file can be imported.
        CHECK(CanImportItemExchange(file, issues));
    }
}

// OpenMU's export adds its fields one at a time: a file of version 2 names the ones it carries.
TEST_CASE("A file of version 2 carries only the fields of its list [data][items][editor]")
{
    const std::string header = R"({"formatVersion": 2, "source": "OpenMU", "gameVersion": "season6", )";
    std::vector<ItemDataIssue> issues;

    SUBCASE("its items need no name while the list leaves it out")
    {
        const ItemExchangeFile file = ReadExchange(
            header + R"("fields": ["width", "height"], "items": [{"group": 0, "number": 0, "width": 1, "height": 2}]})",
            issues);
        CHECK_FALSE(HasFatal(issues));
        REQUIRE(file.fields);
        CHECK(*file.fields == std::set<std::string, std::less<>>{"height", "width"});
        CHECK(CarriesField(file, "width"));
        CHECK_FALSE(CarriesField(file, "damageMin"));
        CHECK_FALSE(CarriesEveryField(file));
        REQUIRE(file.items.size() == 1);
        CHECK(file.items[0].names.GetNeutral().empty());
        CHECK(file.items[0].height == 2);
    }
    SUBCASE("a name it lists is required")
    {
        ReadExchange(header + R"("fields": ["name", "width"], "items": [{"group": 0, "number": 0, "width": 1}]})",
                     issues);
        CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Fatal, "name"));
    }
    SUBCASE("a field the exchange file has not got is ignored with a warning")
    {
        const ItemExchangeFile file =
            ReadExchange(header + R"("fields": ["width", "weaponKind", "group"], "items": []})", issues);
        CHECK_FALSE(HasFatal(issues));
        CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Warning, "weaponKind"));
        REQUIRE(file.fields);
        CHECK(*file.fields == std::set<std::string, std::less<>>{"width"});
    }
    SUBCASE("a list that is not a list of field names is refused")
    {
        ReadExchange(header + R"("fields": "width", "items": []})", issues);
        CHECK(HasFatal(issues));
    }
    SUBCASE("a file without the list carries every field")
    {
        const ItemExchangeFile file = ReadExchange(header + R"("items": []})", issues);
        CHECK_FALSE(HasFatal(issues));
        CHECK_FALSE(file.fields);
        CHECK(CarriesEveryField(file));
    }
}

TEST_CASE("A file with a field list is written as version 2 with only those fields [data][items][editor]")
{
    const ShippedItems shipped;
    ItemExchangeFile file = BuildItemExchange(g_ItemDatabase.GetAllSlots(), g_ItemDataHandler.GetStatFile());
    file.source = ItemExchangeSourceOpenMU;
    file.fields = std::set<std::string, std::less<>>{"height", "width"};

    const std::string text = WriteItemExchangeJson(file);
    CHECK(text.find(R"("formatVersion": 2)") != std::string::npos);
    CHECK(text.find(R"("damageMin")") == std::string::npos);
    CHECK(text.find(R"("en": "Kris")") == std::string::npos);

    std::vector<ItemDataIssue> issues;
    const ItemExchangeFile read = ReadExchange(text, issues);
    CHECK_FALSE(HasFatal(issues));
    CHECK(read.fields == file.fields);
    REQUIRE(read.items.size() == file.items.size());
    for (size_t index = 0; index < read.items.size(); ++index)
    {
        CHECK(read.items[index].width == file.items[index].width);
        CHECK(read.items[index].height == file.items[index].height);
        CHECK(read.items[index].damageMin == 0);
    }
    // The client's own export has every field, as version 1.
    file.fields.reset();
    CHECK(WriteItemExchangeJson(file).find(R"("formatVersion": 1)") != std::string::npos);
}

// The list of the fields a "fields" list may name is the list of what an exchange item has.
TEST_CASE("The exchange field keys are the fields of the exchange items [data][items][editor]")
{
    const ShippedItems shipped;
    std::set<std::string> written;
    for (const ItemDefinition& item :
         BuildItemExchange(g_ItemDatabase.GetAllSlots(), g_ItemDataHandler.GetStatFile()).items)
    {
        // A temporary's items() would outlive it in the loop.
        const Json::OrderedJson json = WriteItemJson(item);
        for (const auto& [key, value] : json.items())
        {
            written.insert(key);
        }
    }
    written.erase("number");
    std::set<std::string> listed;
    for (const char* key : GetExchangeFieldKeys())
    {
        listed.insert(key);
    }
    CHECK(written == listed);
}

TEST_CASE("The exchange file reader leaves out what stays with the client [data][items][editor]")
{
    std::vector<ItemDataIssue> issues;

    SUBCASE("a client-only field of an item")
    {
        const ItemExchangeFile file = ReadExchange(
            FileWithItem(
                R"({"group": 0, "number": 0, "name": "Kris", "weaponKind": "sword", "statProfile": "weapon"})"),
            issues);
        CHECK_FALSE(HasFatal(issues));
        CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Warning, "stay with the client"));
        REQUIRE(file.items.size() == 1);
        CHECK(file.items[0].weaponKind == WeaponKind::None);
        CHECK(file.items[0].statProfile == "weapon");
    }
    SUBCASE("an item's description")
    {
        const ItemExchangeFile file = ReadExchange(
            FileWithItem(R"({"group": 0, "number": 0, "name": "Kris", "description": [{"text": "A short dagger"}]})"),
            issues);
        CHECK_FALSE(HasFatal(issues));
        CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Warning, "stay with the client"));
        REQUIRE(file.items.size() == 1);
        CHECK(file.items[0].description.empty());
    }
    SUBCASE("profile rules in the formulas")
    {
        ShippedItems items;
        ItemExchangeFile built = BuildItemExchange({}, g_ItemDataHandler.GetStatFile());
        std::string text = WriteItemExchangeJson(built);
        const std::string profiles = R"("profiles": [)";
        const size_t at = text.find(profiles);
        REQUIRE(at != std::string::npos);
        text.insert(at, R"("profileRules": [{"profile": "default", "tags": ["ammunition"]}], )");
        const ItemExchangeFile file = ReadExchange(text, issues);
        CHECK_FALSE(HasFatal(issues));
        CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Warning, "profile rules"));
        REQUIRE(file.formulas);
        CHECK(file.formulas->profileRules.empty());
    }
    SUBCASE("the server of a file from OpenMU")
    {
        const ItemExchangeFile file =
            ReadExchange(FileWithItem(R"({"group": 0, "number": 0, "name": "Kris"})",
                                      R"("source": "OpenMU", "server": "3f2a", "gameVersion": "season6")"),
                         issues);
        CHECK(issues.empty());
        CHECK(file.source == ItemExchangeSourceOpenMU);
        CHECK(file.server == "3f2a");
        CHECK_FALSE(file.formulas);
    }
}

#endif // _EDITOR
