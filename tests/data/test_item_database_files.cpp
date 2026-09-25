#include "doctest.h"

#include "Data/DataHandler/ItemData/ItemDataLoader.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"

#include <string>
#include <vector>

using namespace Data::Items;

// These tests read the item files shipped in src/bin/Data, so they also catch
// a broken or missing item file in the repo.
namespace
{
const std::wstring DataDirectory = L"" MU_TEST_DATA_DIR;

std::vector<ITEM_ATTRIBUTE> LoadItemFile(const std::wstring& relativePath)
{
    std::vector<ITEM_ATTRIBUTE> attributes(MAX_ITEM, ITEM_ATTRIBUTE{});
    const std::wstring path = DataDirectory + L"/" + relativePath;
    REQUIRE(ItemDataLoader::Load(path.c_str(), attributes.data()));
    return attributes;
}

std::vector<std::string> ToEnglishNames(const std::vector<ITEM_ATTRIBUTE>& attributes)
{
    std::vector<std::string> names;
    names.reserve(attributes.size());
    for (const ITEM_ATTRIBUTE& attribute : attributes)
    {
        names.push_back(mu_wchar_to_utf8(attribute.Name));
    }
    return names;
}
} // namespace

TEST_CASE("Shipped English item file builds the item database [data][items]")
{
    const auto attributes = LoadItemFile(L"Local/Eng/item_eng.bmd");

    ItemDatabase database;
    database.Build(attributes);

    CHECK(database.GetExistingItemCount() > 0);
    CHECK(database.GetLogName(MakeItemType(0, 0)) == "Kris (0,0)");
}

TEST_CASE("Translated item file keeps English log names [data][items]")
{
    const auto englishNames = ToEnglishNames(LoadItemFile(L"Local/Eng/item_eng.bmd"));
    const auto portugueseAttributes = LoadItemFile(L"Local/Por/item_por.bmd");

    ItemDatabase database;
    database.Build(portugueseAttributes, englishNames);

    const ItemDefinition* shortSword = database.Find(0, 1);
    REQUIRE(shortSword != nullptr);
    CHECK(shortSword->englishName == englishNames[MakeItemType(0, 1)]);
    CHECK(database.GetLogName(MakeItemType(0, 1)) == englishNames[MakeItemType(0, 1)] + " (0,1)");
}
