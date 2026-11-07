#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"
#include "HeroScope.h"
#include "LogCapture.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInventory.h"
#include "GameLogic/Items/ItemOptionSlots.h"
#include "GameLogic/Items/ItemOptions.h"
#include "GameLogic/Pets/GIPetManager.h"
#include "I18N/All.h"
#include "Network/Server/WSclient.h"
#include "UI/Inventory/ItemMng.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

struct OptionSlot
{
    WORD option;
    WORD value;
};
} // namespace

// An item has MAX_ITEM_SPECIAL option slots. An option past the last is dropped and leaves the item's other fields as
// they are.
TEST_CASE("An item keeps the options its slots hold and drops the rest [items]")
{
    constexpr DWORD key = 0x12345678;
    ITEM item{};
    item.Type = ITEM_BLADE;
    item.Key = key;
    for (int i = 0; i < MAX_ITEM_SPECIAL; ++i)
    {
        GameLogic::Items::AddOptionSlot(item, static_cast<std::uint16_t>(AT_IMPROVE_DAMAGE + i), i + 1);
    }
    GameLogic::Items::AddOptionSlot(item, AT_LUCK, 99);

    REQUIRE(item.SpecialNum == MAX_ITEM_SPECIAL);
    for (int i = 0; i < MAX_ITEM_SPECIAL; ++i)
    {
        CAPTURE(i);
        CHECK(item.Special[i] == AT_IMPROVE_DAMAGE + i);
        CHECK(item.SpecialValue[i] == i + 1);
    }
    CHECK(item.Key == key);
}

// The first option an item type drops is logged, with the item and the option; the type's later drops are not. The log
// remembers the types for the whole run, so this test uses types no other test drops an option of.
TEST_CASE("A dropped option is logged once per item type [items]")
{
    const auto fullItem = [](int type)
    {
        ITEM item{};
        item.Type = static_cast<short>(type);
        for (int i = 0; i < MAX_ITEM_SPECIAL; ++i)
        {
            GameLogic::Items::AddOptionSlot(item, AT_IMPROVE_DEFENSE);
        }
        return item;
    };
    const auto droppedLine = [](int type, int option)
    {
        return "Item " + std::to_string(type) + " has more than " + std::to_string(MAX_ITEM_SPECIAL) +
               " options; option " + std::to_string(option) + " and those after it are dropped";
    };

    {
        LogCapture log("gameplay");
        ITEM shield = fullItem(ITEM_SMALL_SHIELD);
        GameLogic::Items::AddOptionSlot(shield, AT_LUCK);
        CHECK_FALSE(log.LineWith(droppedLine(ITEM_SMALL_SHIELD, AT_LUCK)).empty());
    }
    {
        LogCapture log("gameplay");
        ITEM shield = fullItem(ITEM_SMALL_SHIELD);
        GameLogic::Items::AddOptionSlot(shield, AT_LUCK);
        GameLogic::Items::AddOptionSlot(shield, AT_IMPROVE_BLOCKING);
        ITEM otherShield = fullItem(ITEM_HORN_SHIELD);
        GameLogic::Items::AddOptionSlot(otherShield, AT_IMPROVE_BLOCKING);

        CHECK(log.LineWith("Item " + std::to_string(ITEM_SMALL_SHIELD) + " ").empty());
        CHECK_FALSE(log.LineWith(droppedLine(ITEM_HORN_SHIELD, AT_IMPROVE_BLOCKING)).empty());
    }
}

// A weapon with its skill, luck, an additional option and all six excellent options has nine options. With eight
// slots, the ninth was written over the first two slots' values.
TEST_CASE("A weapon keeps its skill, luck, additional option and six excellent options [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    const WORD bladeSkill = ItemAttribute[ITEM_BLADE].m_wSkillIndex;
    REQUIRE(bladeSkill != 0);

    // Blade (0/5) +0 as the server writes it: group and number, level, durability, the option flags, then the
    // additional option's byte (level 4) and the excellent options' byte (all six).
    const auto flags = static_cast<BYTE>(ItemOptionFlags::HasSkill | ItemOptionFlags::HasLuck |
                                         ItemOptionFlags::HasOption | ItemOptionFlags::HasExcellent);
    const std::array<BYTE, 7> itemData{0x00, 5, 0, 30, flags, 0x04, 0x3F};

    // The excellent damage level option is the hero's level / 20.
    HeroScope hero;
    hero.Hero().Level = 100;
    ITEM blade;
    FillItemFromParameters(blade, ParseItemData(itemData));

    const std::array<OptionSlot, 9> expected{{
        {bladeSkill, 0},
        {AT_LUCK, 0},
        {AT_IMPROVE_DAMAGE, 16},
        {AT_EXCELLENT_DAMAGE, 0},
        {AT_IMPROVE_DAMAGE_LEVEL, 5},
        {AT_IMPROVE_DAMAGE_PERCENT, 2},
        {AT_IMPROVE_ATTACK_SPEED, 7},
        {AT_IMPROVE_GAIN_LIFE, 0},
        {AT_IMPROVE_GAIN_MANA, 0},
    }};
    REQUIRE(blade.SpecialNum == expected.size());
    for (size_t i = 0; i < expected.size(); ++i)
    {
        CAPTURE(i);
        CHECK(blade.Special[i] == expected[i].option);
        CHECK(blade.SpecialValue[i] == expected[i].value);
    }
}

// The Dark Horse's defense (5 + agility / 20 + the horse's level * 2) gets its value again on every pet info update,
// for example while its tooltip is shown. It keeps one slot, also when that slot is the first: a horse without its
// skill has the defense in slot 0, which the update took for "no defense slot yet" and added another one.
TEST_CASE("A Dark Horse keeps one defense slot over pet info updates [items]")
{
    HeroScope hero;
    hero.Hero().Dexterity = 40;
    PET_INFO pet{};

    for (const bool withSkill : {false, true})
    {
        CAPTURE(withSkill);
        ITEM horse{};
        horse.Type = ITEM_DARK_HORSE_ITEM;
        if (withSkill)
        {
            GameLogic::Items::AddOptionSlot(horse, AT_SKILL_EARTHSHAKE);
        }
        const int defenseSlot = horse.SpecialNum;

        for (const int level : {1, 2, 3})
        {
            pet.m_wLevel = static_cast<WORD>(level);
            giPetManager::SetPetItemConvert(&horse, &pet);
        }
        REQUIRE(horse.SpecialNum == defenseSlot + 1);
        CHECK(horse.Special[defenseSlot] == AT_SET_OPTION_IMPROVE_DEFENCE);
        CHECK(horse.SpecialValue[defenseSlot] == 5 + 40 / 20 + 3 * 2);
    }
}

// The Dark Horse's defense counts all of the character's agility, as OpenMU does (Pets.cs, Stats.TotalAgility): its
// base agility and the agility its items, sets and sockets add.
TEST_CASE("A Dark Horse's defense counts the agility from items [items]")
{
    HeroScope hero;
    hero.Hero().Dexterity = 40;
    hero.Hero().AddDexterity = 60;
    PET_INFO pet{};
    pet.m_wLevel = 1;
    ITEM horse{};
    horse.Type = ITEM_DARK_HORSE_ITEM;
    giPetManager::SetPetItemConvert(&horse, &pet);

    REQUIRE(horse.SpecialNum == 1);
    CHECK(horse.SpecialValue[0] == 5 + (40 + 60) / 20 + 1 * 2);
}

// An option's value has more than a byte: the Dark Horse's defense passes 255 at 3020 agility with a horse at level 50,
// and OpenMU adds all of it (Pets.cs, DefenseBase + 5 + horse level * 2 + agility / 20).
TEST_CASE("An option keeps a value above 255 [items]")
{
    HeroScope hero;
    hero.Hero().Dexterity = 3020;
    PET_INFO pet{};
    pet.m_wLevel = 50;
    ITEM horse{};
    horse.Type = ITEM_DARK_HORSE_ITEM;
    giPetManager::SetPetItemConvert(&horse, &pet);

    REQUIRE(horse.SpecialNum == 1);
    CHECK(horse.SpecialValue[0] == 256);
}

// The Wings of Despair's additional option is wizardry damage (option type 2) or curse damage (type 0), as on the other
// second tier wings. A type the client has no option for takes no slot: it took one with option 0, which has no text,
// and the tooltip stops at its first empty line, so every line after it was gone.
TEST_CASE("The Wings of Despair take no option slot for an option type they have no option for [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));

    const auto wingsWithOption = [](BYTE optionType)
    {
        ItemCreationParams params{};
        params.Group = ITEM_WINGS_OF_DESPAIR / MAX_ITEM_INDEX;
        params.Number = ITEM_WINGS_OF_DESPAIR % MAX_ITEM_INDEX;
        params.WithOption = true;
        params.OptionLevel = 2;
        params.OptionType = optionType;
        ITEM wings;
        FillItemFromParameters(wings, params);
        return wings;
    };

    const ITEM wizardry = wingsWithOption(2);
    REQUIRE(wizardry.SpecialNum == 1);
    CHECK(wizardry.Special[0] == AT_IMPROVE_MAGIC);
    CHECK(wizardry.SpecialValue[0] == 8);
    const ITEM curse = wingsWithOption(0);
    REQUIRE(curse.SpecialNum == 1);
    CHECK(curse.Special[0] == AT_IMPROVE_CURSE);
    CHECK(curse.SpecialValue[0] == 8);
    for (const int otherType : {1, 3})
    {
        CAPTURE(otherType);
        CHECK(wingsWithOption(static_cast<BYTE>(otherType)).SpecialNum == 0);
    }
}

namespace
{
// The items the code before the option definitions gave luck (SetItemAttributes): groups 0 to 11 but arrows and bolts,
// the first wings (12/0 to 12/6), the Wing of Curse and the Wings of Despair, the Cape of Lord and the Cape of Fighter,
// and the third tier wings.
bool OldCodeGivesLuck(int type)
{
    if (type >= ITEM_SWORD && type < ITEM_BOOTS + MAX_ITEM_INDEX)
    {
        return !g_ItemDatabase.HasTag(type, Data::Items::ItemTag::Ammunition);
    }
    if ((type >= ITEM_WING && type <= ITEM_WINGS_OF_DARKNESS) ||
        (type >= ITEM_WING_OF_CURSE && type <= ITEM_WINGS_OF_DESPAIR))
    {
        return true;
    }
    return type == ITEM_CAPE_OF_LORD || type == ITEM_CAPE_OF_FIGHTER ||
           g_ItemDatabase.GetWingTier(type) == Data::Items::WingTier::Third;
}
} // namespace

// Luck comes from the definition the item links (Data/Items/Options/Luck.json): every item gets its slot where the code
// before the definitions put it (after the skill), with the value 0 that adds no critical damage rate, and the slot
// keeps where it came from, which gives its tooltip lines the definition's value.
TEST_CASE("Every item's luck comes from its option links as the code before the definitions gave it [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    const int luck = g_ItemDatabase.GetOptions().Find("luck");
    REQUIRE(luck != Data::Items::ItemOptionDatabase::NoDefinition);
    // The Dark Horse's defense reads the hero's agility.
    HeroScope hero;

    int luckyItems = 0;
    for (int type = 0; type < MAX_ITEM; ++type)
    {
        if (g_ItemDatabase.Find(type) == nullptr)
        {
            continue;
        }
        CAPTURE(type);
        ItemCreationParams params{};
        params.Group = type / MAX_ITEM_INDEX;
        params.Number = type % MAX_ITEM_INDEX;
        params.WithLuck = true;
        params.WithSkill = true;
        ITEM item;
        FillItemFromParameters(item, params);

        std::vector<int> luckSlots;
        for (int slot = 0; slot < item.SpecialNum; ++slot)
        {
            if (item.Special[slot] == AT_LUCK)
            {
                luckSlots.push_back(slot);
            }
        }
        REQUIRE(luckSlots.size() == (OldCodeGivesLuck(type) ? 1u : 0u));
        if (luckSlots.empty())
        {
            continue;
        }
        ++luckyItems;
        const int slot = luckSlots.front();
        CHECK(slot == (ItemAttribute[type].m_wSkillIndex != 0 ? 1 : 0));
        CHECK(item.SpecialValue[slot] == 0);
        CHECK(GameLogic::Items::GetOptionSource(item, slot) ==
              GameLogic::Items::OptionSource{static_cast<std::uint16_t>(luck), 0});
        CHECK(GameLogic::Items::GetItemOptionValue(item, slot) == 5);
    }
    CHECK(luckyItems > 400);
}

// A slot the code wrote has no source, a definition's keeps it; past the slots there is none.
TEST_CASE("An option slot keeps where its option came from [items]")
{
    ITEM item{};
    item.Type = ITEM_BLADE;
    GameLogic::Items::AddOptionSlot(item, AT_SKILL_FALLING_SLASH);
    GameLogic::Items::AddOptionSlot(item, AT_LUCK, 0, {3, 1});
    CHECK_FALSE(GameLogic::Items::GetOptionSource(item, 0).has_value());
    CHECK(GameLogic::Items::GetOptionSource(item, 1) == GameLogic::Items::OptionSource{3, 1});
    CHECK_FALSE(GameLogic::Items::GetOptionSource(item, 2).has_value());
    CHECK_FALSE(GameLogic::Items::FindSlotOption(item, 0).has_value());
}

// Both item tooltips, the item's and the Dark Lord pets', write a slot of an option definition as its texts, and leave
// a slot the code wrote (the skill) to GetSpecialOptionText.
TEST_CASE("A slot of an option definition writes its texts into the tooltip's rows [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    ItemCreationParams params{};
    params.Group = ITEM_BLADE / MAX_ITEM_INDEX;
    params.Number = ITEM_BLADE % MAX_ITEM_INDEX;
    params.WithLuck = true;
    params.WithSkill = true;
    ITEM blade;
    FillItemFromParameters(blade, params);
    REQUIRE(blade.SpecialNum == 2);

    CHECK_FALSE(WriteDefinitionOptionLines(blade, 0, 0, MaxTextListRows).has_value());
    CHECK(WriteDefinitionOptionLines(blade, 1, 3, MaxTextListRows) == 2);
    CHECK(std::wstring(TextList[3]) == L"Luck (success rate of Jewel of Soul +25%)");
    CHECK(std::wstring(TextList[4]) == L"Luck (critical damage rate +5%)");
    CHECK(TextListColor[4] == TEXT_COLOR_BLUE);
    CHECK(TextBold[4] == 0);
    // Only the rows before the limit.
    CHECK(WriteDefinitionOptionLines(blade, 1, 3, 4) == 1);
}
