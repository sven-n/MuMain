#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"
#include "HeroScope.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Engine/Object/ZzzInfomation.h"
#include "GameLogic/Items/ItemOptionSlots.h"
#include "GameLogic/Pets/GIPetManager.h"
#include "I18N/All.h"
#include "Network/Server/WSclient.h"
#include "UI/Inventory/ItemMng.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>

namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

struct OptionSlot
{
    WORD option;
    BYTE value;
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
