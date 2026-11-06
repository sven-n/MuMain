#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
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
    CHARACTER_ATTRIBUTE hero{};
    hero.Level = 100;
    CHARACTER_ATTRIBUTE* const previousHero = CharacterAttribute;
    CharacterAttribute = &hero;
    ITEM blade;
    FillItemFromParameters(blade, ParseItemData(itemData));
    CharacterAttribute = previousHero;

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
