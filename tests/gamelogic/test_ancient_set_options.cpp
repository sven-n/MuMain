#include "stdafx.h"
#include "doctest.h"

#include "GameLogic/Items/CSItemOption.h"

#include "I18N/All.h"

#include <cstdint>

TEST_CASE("Set options 24 to 28 have no description, the others keep theirs [gamelogic][items][sets]")
{
    for (int option = 0; option < 24; ++option)
    {
        CAPTURE(option);
        CHECK(CSItemOption::GetSetOptionFormat(static_cast<std::uint8_t>(option)) != nullptr);
    }
    for (int option = 24; option <= 28; ++option)
    {
        CAPTURE(option);
        CHECK(CSItemOption::GetSetOptionFormat(static_cast<std::uint8_t>(option)) == nullptr);
    }

    CHECK(CSItemOption::GetSetOptionFormat(0) == I18N::Game::IncreaseStrengthD);
    CHECK(CSItemOption::GetSetOptionFormat(23) == I18N::Game::IncreaseDamageWhenUsingTwoHandedWeaponsD);
    CHECK(CSItemOption::GetSetOptionFormat(MASTERY_OPTION) == I18N::Game::IceAttributeSkillIncreaseDamageD);
    CHECK(CSItemOption::GetSetOptionFormat(29) == I18N::Game::IceAttributeSkillIncreaseDamageD);
    CHECK(CSItemOption::GetSetOptionFormat(30) == I18N::Game::PoisonAttributeSkillIncreaseDamageD);
    CHECK(CSItemOption::GetSetOptionFormat(31) == I18N::Game::LightningAttributeSkillIncreaseDamageD);
    CHECK(CSItemOption::GetSetOptionFormat(32) == I18N::Game::FireAttributeSkillIncreaseDamageD);
    CHECK(CSItemOption::GetSetOptionFormat(33) == I18N::Game::EarthAttributeSkillIncreaseDamageD);
    CHECK(CSItemOption::GetSetOptionFormat(34) == I18N::Game::WindAttributeSkillIncreaseDamageD);
    CHECK(CSItemOption::GetSetOptionFormat(35) == I18N::Game::WaterAttributeSkillIncreaseDamageD);
    CHECK(CSItemOption::GetSetOptionFormat(36) == nullptr);
    CHECK(CSItemOption::GetSetOptionFormat(0xFF) == nullptr);
}
