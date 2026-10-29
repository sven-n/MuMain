#include "App/stdafx.h"

#include <doctest.h>

#include <algorithm>
#include <climits>
#include <cstdint>
#include <iterator>
#include <string_view>

#include "I18N/All.h"
#include "GameLogic/Items/CSItemOption.h"
#include "GameLogic/Items/MixMgr.h"
#include "GameLogic/Pets/GIPetManager.h"
#include "UI/Inventory/MixInventory.h"
#include "UI/Inventory/Trade.h"
#include "LegacyTextComparison.h"

using mu::ui::window::CMixInventory;
using mu::ui::window::CTrade;

namespace
{
// Every text number the shipped Data/Local/mix.bmd stores in its recipes' names, descriptions and advice.
constexpr int kShippedMixTextIds[] = {
    588,  589,  590,  600,  603,  604,  605,  606,  607,  608,  610,  611,  612,  613,  614,
    615,  665,  666,  736,  1172, 1187, 1212, 1214, 1412, 1413, 1414, 1415, 1550, 1640, 1666,
    1667, 1863, 1864, 1921, 1922, 1923, 2038, 2061, 2062, 2194, 2203, 2222, 2261, 2263, 2265,
    2267, 2336, 2396, 2560, 2664, 2665, 2669, 2670, 2791, 2881, 2882, 3086, 3087,
};

// Above every legacy Game text number.
constexpr int kBeyondLegacyTextIds = 4000;

bool IsShippedMixTextId(int textId)
{
    return std::find(std::begin(kShippedMixTextIds), std::end(kShippedMixTextIds), textId) !=
           std::end(kShippedMixTextIds);
}

bool IsEmptyText(const wchar_t* text)
{
    return text != nullptr && std::wstring_view(text).empty();
}

// CSItemOption::getExplainText()'s former switch: the format it printed `option` with, nullptr where
// it returned false.
const wchar_t* LegacySetOptionFormat(std::uint8_t option)
{
    constexpr std::uint8_t EMPTY_OPTION = 0xFF;
    if (option == EMPTY_OPTION)
    {
        return nullptr;
    }

    switch (option + AT_SET_OPTION_IMPROVE_STRENGTH)
    {
    case AT_SET_OPTION_IMPROVE_MAGIC_POWER:
        return I18N::Game::IncreaseWizardryDmgD;

    case AT_SET_OPTION_IMPROVE_STRENGTH:
    case AT_SET_OPTION_IMPROVE_DEXTERITY:
    case AT_SET_OPTION_IMPROVE_ENERGY:
    case AT_SET_OPTION_IMPROVE_VITALITY:
    case AT_SET_OPTION_IMPROVE_CHARISMA:
    case AT_SET_OPTION_IMPROVE_ATTACK_MIN:
    case AT_SET_OPTION_IMPROVE_ATTACK_MAX:
        return I18N::Game::Lookup(950 + option);

    case AT_SET_OPTION_IMPROVE_DAMAGE:
    case AT_SET_OPTION_IMPROVE_ATTACKING_PERCENT:
    case AT_SET_OPTION_IMPROVE_DEFENCE:
    case AT_SET_OPTION_IMPROVE_MAX_LIFE:
    case AT_SET_OPTION_IMPROVE_MAX_MANA:
    case AT_SET_OPTION_IMPROVE_MAX_AG:
    case AT_SET_OPTION_IMPROVE_ADD_AG:
    case AT_SET_OPTION_IMPROVE_CRITICAL_DAMAGE_PERCENT:
    case AT_SET_OPTION_IMPROVE_CRITICAL_DAMAGE:
    case AT_SET_OPTION_IMPROVE_EXCELLENT_DAMAGE_PERCENT:
    case AT_SET_OPTION_IMPROVE_EXCELLENT_DAMAGE:
    case AT_SET_OPTION_IMPROVE_SKILL_ATTACK:
    case AT_SET_OPTION_DOUBLE_DAMAGE:
        return I18N::Game::Lookup(949 + option);

    case AT_SET_OPTION_DISABLE_DEFENCE:
        return I18N::Game::IgnoreEnemiesDefensiveSkillD;

    case AT_SET_OPTION_TWO_HAND_SWORD_IMPROVE_DAMAGE:
        return I18N::Game::IncreaseDamageWhenUsingTwoHandedWeaponsD;

    case AT_SET_OPTION_IMPROVE_SHIELD_DEFENCE:
        return I18N::Game::IncreaseDefensiveSkillWhenUsingShieldWeaponsD;

    case AT_SET_OPTION_IMPROVE_ATTACK_1:
    case AT_SET_OPTION_IMPROVE_ATTACK_2:
    case AT_SET_OPTION_IMPROVE_MAGIC:
    case AT_SET_OPTION_IMPROVE_DEFENCE_3:
    case AT_SET_OPTION_IMPROVE_DEFENCE_4:
    case AT_SET_OPTION_FIRE_MASTERY:
    case AT_SET_OPTION_ICE_MASTERY:
    case AT_SET_OPTION_THUNDER_MASTERY:
    case AT_SET_OPTION_POSION_MASTERY:
    case AT_SET_OPTION_WATER_MASTERY:
    case AT_SET_OPTION_WIND_MASTERY:
    case AT_SET_OPTION_EARTH_MASTERY:
        return I18N::Game::Lookup(971 + (option + AT_SET_OPTION_IMPROVE_STRENGTH - AT_SET_OPTION_IMPROVE_ATTACK_2));
    default:
        return nullptr;
    }
}
} // namespace

TEST_CASE("mix recipe texts match the legacy ids in every locale [i18n][legacy]")
{
    for (int textId : kShippedMixTextIds)
        CHECK_MESSAGE(I18N::Game::LookupSlot(textId) != nullptr, textId);

    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int textId : kShippedMixTextIds)
        {
            CHECK_MESSAGE(LegacyTextComparison::SameText(SEASON3A::GetMixRecipeText(textId),
                                                         I18N::Game::Lookup(textId)),
                          locale << " " << textId);
        }
    });
}

TEST_CASE("mix recipe numbers mix.bmd does not use have an empty text [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int textId : {0, -1, 99999, INT_MIN, INT_MAX})
            CHECK_MESSAGE(IsEmptyText(SEASON3A::GetMixRecipeText(textId)), locale << " " << textId);

        for (int textId = -1; textId < kBeyondLegacyTextIds; ++textId)
        {
            if (IsShippedMixTextId(textId))
                continue;
            CHECK_MESSAGE(IsEmptyText(SEASON3A::GetMixRecipeText(textId)), locale << " " << textId);
        }
    });
}

TEST_CASE("set option formats match the legacy ids in every locale [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        // Set option numbers come from ItemSetOption.bmd as bytes.
        for (int value = 0; value <= UINT8_MAX; ++value)
        {
            const auto option = static_cast<std::uint8_t>(value);
            const wchar_t* expected = LegacySetOptionFormat(option);
            const wchar_t* actual = CSItemOption::GetSetOptionFormat(option);
            CHECK_MESSAGE((actual == nullptr) == (expected == nullptr), locale << " " << value);
            CHECK_MESSAGE(LegacyTextComparison::SameText(actual, expected), locale << " " << value);
        }
    });
}

TEST_CASE("castle senior mix description matches the legacy ids in every locale [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int i = 0; i < CMixInventory::CASTLE_SENIOR_DESCRIPTION_ROWS; ++i)
        {
            CHECK_MESSAGE(LegacyTextComparison::SameText(CMixInventory::GetCastleSeniorDescription(i),
                                                         I18N::Game::Lookup(1644 + i)),
                          locale << " " << i);
        }
        CHECK(IsEmptyText(CMixInventory::GetCastleSeniorDescription(-1)));
        CHECK(IsEmptyText(CMixInventory::GetCastleSeniorDescription(CMixInventory::CASTLE_SENIOR_DESCRIPTION_ROWS)));
    });
}

TEST_CASE("trade alert lines match the legacy ids in every locale [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int i = 0; i < CTrade::TRADE_ALERT_LINES; ++i)
        {
            CHECK_MESSAGE(LegacyTextComparison::SameText(CTrade::GetTradeAlertLine(i), I18N::Game::Lookup(371 + i)),
                          locale << " " << i);
        }
        CHECK(IsEmptyText(CTrade::GetTradeAlertLine(-1)));
        CHECK(IsEmptyText(CTrade::GetTradeAlertLine(CTrade::TRADE_ALERT_LINES)));
    });
}

TEST_CASE("pet command names match the legacy ids in every locale [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        // BuildPetCmdTooltipModel() names the commands AT_PET_COMMAND_DEFAULT..AT_PET_COMMAND_END.
        for (int Type = AT_PET_COMMAND_DEFAULT; Type < AT_PET_COMMAND_END; ++Type)
        {
            const int cmdType = Type - AT_PET_COMMAND_DEFAULT;
            CHECK_MESSAGE(LegacyTextComparison::SameText(giPetManager::GetPetCommandName(cmdType),
                                                         I18N::Game::Lookup(1219 + cmdType)),
                          locale << " " << cmdType);
        }
        CHECK(IsEmptyText(giPetManager::GetPetCommandName(PET_CMD_DEFAULT - 1)));
        CHECK(IsEmptyText(giPetManager::GetPetCommandName(PET_CMD_END)));
    });
}
