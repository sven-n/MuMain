#include "stdafx.h"
#include "doctest.h"

#include "ClientDataScope.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Engine/Object/ZzzInventory.h"
#include "GameLogic/Automation/HelperPickupFilter.h"
#include "MUHelper/MuHelperData.h"

#include <filesystem>
#include <string>

namespace
{
constexpr int SerpentSwordType = Data::Items::MakeItemType(0, 8);
const std::filesystem::path ShippedClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

void LoadShippedItems()
{
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
}

ITEM MakeItem(int type, int level = 0)
{
    ITEM item{};
    item.Type = static_cast<short>(type);
    item.Level = level;
    return item;
}

MUHelper::ConfigData ExtraFilter(const wchar_t* keyword)
{
    MUHelper::ConfigData config;
    config.bPickSelectItems = true;
    config.bPickExtraItems = true;
    config.aExtraItems.insert(keyword);
    return config;
}

std::wstring InventoryName(const ITEM& item)
{
    return GetItemName(item.Type, item.Level);
}
} // namespace

TEST_CASE("Helper extra filters distinguish level-aware box names [data][helper][pickup]")
{
    ClientDataScope client(ShippedClientDirectory);
    LoadShippedItems();
    const auto luckFilter = ExtraFilter(L"Box of Luc");
    const auto heartFilter = ExtraFilter(L"Heart");
    const auto kundunFilter = ExtraFilter(L"Kundun");
    for (const int level : {0, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15})
    {
        CAPTURE(level);
        ITEM item = MakeItem(ITEM_BOX_OF_LUCK, level);
        CHECK(GetItemDisplayName(&item) == InventoryName(item));
        CHECK(GameLogic::Automation::ShouldPickUpHelperItem(luckFilter, &item) == (level == 0));
        CHECK(GameLogic::Automation::ShouldPickUpHelperItem(heartFilter, &item) == (level == 3 || level == 13));
        CHECK(GameLogic::Automation::ShouldPickUpHelperItem(kundunFilter, &item) == (level >= 8 && level <= 12));
    }
}

TEST_CASE("Helper extra filters preserve contiguous equipment option keywords [data][helper][pickup]")
{
    ClientDataScope client(ShippedClientDirectory);
    LoadShippedItems();
    ITEM item = MakeItem(SerpentSwordType, 7);
    item.HasSkill = true;
    item.OptionLevel = 2;
    item.HasLuck = true;
    CHECK(GetItemDisplayName(&item) == L"Serpent Sword +7+Skill+Option+Luck");
    CHECK(GameLogic::Automation::ShouldPickUpHelperItem(ExtraFilter(L"+7+Skill"), &item));
    CHECK_FALSE(GameLogic::Automation::ShouldPickUpHelperItem(ExtraFilter(L"+8"), &item));
    CHECK_FALSE(GameLogic::Automation::ShouldPickUpHelperItem(ExtraFilter(L"serpent"), &item));
}

TEST_CASE("Helper extra filters resolve feather and ring variants [data][helper][pickup]")
{
    ClientDataScope client(ShippedClientDirectory);
    LoadShippedItems();
    for (const int level : {0, 1})
    {
        ITEM item = MakeItem(ITEM_LOCHS_FEATHER, level);
        CHECK(GetItemDisplayName(&item) == InventoryName(item));
        CHECK(GameLogic::Automation::ShouldPickUpHelperItem(ExtraFilter(L"Crest"), &item) == (level == 1));
        CHECK(GameLogic::Automation::ShouldPickUpHelperItem(ExtraFilter(L"Feather"), &item) == (level == 0));
    }
    for (const int level : {0, 1, 2, 3})
    {
        ITEM item = MakeItem(ITEM_WIZARDS_RING, level);
        CHECK(GetItemDisplayName(&item) == InventoryName(item));
        CHECK(GameLogic::Automation::ShouldPickUpHelperItem(ExtraFilter(L"Wizard"), &item) == (level == 0));
        CHECK(GameLogic::Automation::ShouldPickUpHelperItem(ExtraFilter(L"Warrior"), &item) ==
              (level == 1 || level == 2));
    }
}

TEST_CASE("Helper extra filters follow localized inventory names [data][helper][pickup]")
{
    ClientDataScope client(ShippedClientDirectory);
    LoadShippedItems();
    const ITEM heart = MakeItem(ITEM_BOX_OF_LUCK, 3);
    const auto englishHeartName = InventoryName(heart);
    I18N::SetLocale("de");
    CHECK(InventoryName(heart) != englishHeartName);
    ITEM luck = MakeItem(ITEM_BOX_OF_LUCK);
    CHECK(InventoryName(luck) == L"Glücksbox");
    CHECK_FALSE(GameLogic::Automation::ShouldPickUpHelperItem(ExtraFilter(L"Box of Luck"), &luck));
    CHECK(GameLogic::Automation::ShouldPickUpHelperItem(ExtraFilter(L"Glück"), &luck));
    for (const int level : {3, 5, 6, 8, 12})
    {
        ITEM item = MakeItem(ITEM_BOX_OF_LUCK, level);
        const auto name = InventoryName(item);
        CHECK(GetItemDisplayName(&item) == name);
        CHECK(GameLogic::Automation::ShouldPickUpHelperItem(ExtraFilter(name.c_str()), &item));
    }
}

TEST_CASE("Helper equipment option keywords follow the UI language [data][helper][pickup]")
{
    ClientDataScope client(ShippedClientDirectory);
    LoadShippedItems();
    I18N::SetLocale("de");
    ITEM item = MakeItem(SerpentSwordType, 7);
    item.HasSkill = item.HasLuck = true;
    item.OptionLevel = 2;
    CHECK(GetItemDisplayName(&item) == L"Schlangenschwert +7+Fähigkeit+Option+Glück");
    CHECK(GameLogic::Automation::ShouldPickUpHelperItem(ExtraFilter(L"+Fähigkeit"), &item));
    CHECK(GameLogic::Automation::ShouldPickUpHelperItem(ExtraFilter(L"+Glück"), &item));
    CHECK_FALSE(GameLogic::Automation::ShouldPickUpHelperItem(ExtraFilter(L"+Skill"), &item));
    CHECK_FALSE(GameLogic::Automation::ShouldPickUpHelperItem(ExtraFilter(L"+Luck"), &item));
}

TEST_CASE("Helper unmapped variant levels retain the base name [data][helper][pickup]")
{
    ClientDataScope client(ShippedClientDirectory);
    LoadShippedItems();
    // The Box of Luck +4 has a name of its own in the item data's name lines ("Olive of love", the original client's
    // name for that level), and the extra filters match it.
    ITEM olive = MakeItem(ITEM_BOX_OF_LUCK, 4);
    CHECK(GetItemDisplayName(&olive) == L"Olive of love");
    CHECK(GameLogic::Automation::ShouldPickUpHelperItem(ExtraFilter(L"Olive"), &olive));
    for (auto item : {MakeItem(ITEM_POTION + 12, 3), MakeItem(ITEM_FRUITS, 5), MakeItem(ITEM_LOCHS_FEATHER, 2)})
    {
        CAPTURE(item.Type);
        CAPTURE(item.Level);
        const auto baseName = client.Name(item.Type);
        REQUIRE_FALSE(baseName.empty());
        CHECK(GetItemDisplayName(&item) == baseName + L" +" + std::to_wstring(item.Level));
        CHECK(GameLogic::Automation::ShouldPickUpHelperItem(ExtraFilter(baseName.c_str()), &item));
    }
}

TEST_CASE("Helper saved keywords respect the fixed packet field [data][helper][pickup]")
{
    PRECEIVE_MUHELPER_DATA packet{};
    const std::size_t maximumKeywordBytes = sizeof(packet.ExtraItems[0]) - 1;
    auto config = ExtraFilter(L"+7+Skill");
    const std::wstring atLimit(maximumKeywordBytes, L'a');
    const std::wstring overLimit(maximumKeywordBytes + 1, L'b');
    config.aExtraItems.insert({L"Heart", L"Kundun", L"Crest", atLimit, overLimit, L"+7+Skill+Option+Luck"});
    MUHelper::ConfigDataSerDe::Serialize(config, packet);
    MUHelper::ConfigData reloaded;
    MUHelper::ConfigDataSerDe::Deserialize(packet, reloaded);
    CHECK(reloaded.bPickSelectItems);
    CHECK(reloaded.bPickExtraItems);
    for (const auto* keyword : {L"+7+Skill", L"Heart", L"Kundun", L"Crest"})
    {
        CHECK(reloaded.aExtraItems.contains(keyword));
    }
    CHECK(reloaded.aExtraItems.contains(atLimit));
    CHECK_FALSE(reloaded.aExtraItems.contains(overLimit));
    CHECK_FALSE(reloaded.aExtraItems.contains(L"+7+Skill+Option+Luck"));
}

TEST_CASE("Helper retained filters are inactive without a pickup mode [data][helper][pickup]")
{
    auto config = ExtraFilter(L"Serpent");
    config.bPickSelectItems = false;
    config.bPickZen = config.bPickJewel = config.bPickAncient = config.bPickExcellent = true;
    for (auto item : {MakeItem(SerpentSwordType), MakeItem(ITEM_ZEN), MakeItem(ITEM_JEWEL_OF_BLESS)})
    {
        CAPTURE(item.Type);
        CHECK_FALSE(GameLogic::Automation::ShouldPickUpHelperItem(config, &item));
    }
    ITEM excellent = MakeItem(SerpentSwordType);
    excellent.ExcellentFlags = 1;
    CHECK_FALSE(GameLogic::Automation::ShouldPickUpHelperItem(config, &excellent));
    ITEM ancient = MakeItem(SerpentSwordType);
    ancient.AncientDiscriminator = 1;
    CHECK_FALSE(GameLogic::Automation::ShouldPickUpHelperItem(config, &ancient));
}

TEST_CASE("Helper All mode retains precedence for legacy saves [data][helper][pickup]")
{
    auto config = ExtraFilter(L"Serpent");
    config.bPickAllItems = true;
    ITEM sword = MakeItem(SerpentSwordType);
    ITEM luck = MakeItem(ITEM_BOX_OF_LUCK);
    CHECK(GameLogic::Automation::ShouldPickUpHelperItem(config, &sword));
    CHECK(GameLogic::Automation::ShouldPickUpHelperItem(config, &luck));
    PRECEIVE_MUHELPER_DATA packet{};
    MUHelper::ConfigDataSerDe::Serialize(config, packet);
    MUHelper::ConfigData reloaded;
    MUHelper::ConfigDataSerDe::Deserialize(packet, reloaded);
    CHECK(reloaded.bPickAllItems);
    CHECK(reloaded.bPickSelectItems);
    CHECK(GameLogic::Automation::ShouldPickUpHelperItem(reloaded, &luck));
}

TEST_CASE("Helper All and Selected retain their category semantics [data][helper][pickup]")
{
    // Jewel classification reads the shipped item tags, even without a name lookup.
    ClientDataScope client(ShippedClientDirectory);
    LoadShippedItems();
    MUHelper::ConfigData config;
    ITEM money = MakeItem(ITEM_ZEN);
    ITEM jewel = MakeItem(ITEM_JEWEL_OF_BLESS);
    ITEM sword = MakeItem(SerpentSwordType);
    config.bPickAllItems = true;
    CHECK(GameLogic::Automation::ShouldPickUpHelperItem(config, &money));
    CHECK(GameLogic::Automation::ShouldPickUpHelperItem(config, &sword));
    config.bPickAllItems = false;
    config.bPickSelectItems = true;
    CHECK_FALSE(GameLogic::Automation::ShouldPickUpHelperItem(config, &sword));
    config.bPickZen = true;
    CHECK(GameLogic::Automation::ShouldPickUpHelperItem(config, &money));
    CHECK_FALSE(GameLogic::Automation::ShouldPickUpHelperItem(config, &jewel));
    config.bPickZen = false;
    config.bPickJewel = true;
    CHECK(GameLogic::Automation::ShouldPickUpHelperItem(config, &jewel));
    CHECK_FALSE(GameLogic::Automation::ShouldPickUpHelperItem(config, &money));
}
