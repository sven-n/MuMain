#include "stdafx.h"
#include "doctest.h"

#include "GameLogic/Buffs/w_BuffStateSystem.h"
#include "Engine/Object/ZzzInfomation.h"

#include <chrono>
#include <filesystem>
#include <string>
#include <thread>

namespace
{
// The buff table is loaded from the client's data folder; the game does this once at startup.
struct BuffSystemScope
{
    BuffSystemScope()
    {
        g_strSelectedML = L"Eng";
        std::filesystem::current_path(MU_BIN_DIR);
        g_BuffSystem = BuffStateSystem::Make();
    }

    ~BuffSystemScope()
    {
        g_BuffSystem.reset();
        std::filesystem::current_path(m_PreviousDirectory);
        g_strSelectedML = m_PreviousLanguage;
    }

    std::wstring m_PreviousLanguage = g_strSelectedML;
    std::filesystem::path m_PreviousDirectory = std::filesystem::current_path();
};

// The skill durations are read from the hero's synced attributes.
struct AttributeScope
{
    AttributeScope()
    {
        m_Attribute.Level = 400;
        m_Attribute.Energy = 1000;
        m_Previous = CharacterAttribute;
        CharacterAttribute = &m_Attribute;
    }

    ~AttributeScope()
    {
        CharacterAttribute = m_Previous;
    }

    CHARACTER_ATTRIBUTE m_Attribute{};
    CHARACTER_ATTRIBUTE* m_Previous = nullptr;
};
} // namespace

// A timed buff's remaining share is what the strip draws: full when applied, then falling with
// real time even before the 900 ms scheduler tick updates the stored remainder.
TEST_CASE("a registered buff exposes its remaining share [gamelogic][buff-time]")
{
    BuffSystemScope system;
    TheBuffTimeControl().RegisterBuffTime(eBuff_Seal1, 60);

    float fraction = 0.f;
    REQUIRE(TheBuffTimeControl().GetBuffRemainingFraction(eBuff_Seal1, fraction));
    CHECK(fraction == doctest::Approx(1.f).epsilon(0.01));

    std::this_thread::sleep_for(std::chrono::milliseconds(1100));

    REQUIRE(TheBuffTimeControl().GetBuffRemainingFraction(eBuff_Seal1, fraction));
    CHECK(fraction < 0.995f);
    CHECK(fraction > 0.95f);

    TheBuffTimeControl().UnRegisterBuffTime(eBuff_Seal1);
}

// Most skill buffs have no server-sent duration, so the server timer has nothing for them; the
// client's catalogue estimate is what the strip's fallback uses.
TEST_CASE("a skill buff has no server timer [gamelogic][buff-time]")
{
    BuffSystemScope system;
    float fraction = 1.f;
    CHECK_FALSE(TheBuffTimeControl().GetBuffRemainingFraction(eBuff_Life, fraction));
}

// The catalogue's base + TotalEnergy / divisor and cap, for the effects the hero can carry.
TEST_CASE("skill buff durations follow the server formulas [gamelogic][buff-time]")
{
    BuffSystemScope system;
    AttributeScope attributes;

    CHECK(BuffDurationSeconds(eBuff_Attack) == 60);
    CHECK(BuffDurationSeconds(eBuff_Defense) == 60);
    CHECK(BuffDurationSeconds(eBuff_InfinityArrow) == 600);
    CHECK(BuffDurationSeconds(eBuff_Life) == 60 + 1000 / 180);
    CHECK(BuffDurationSeconds(eBuff_AddCriticalDamage) == 60 + 1000 / 10);
    CHECK(BuffDurationSeconds(eBuff_Berserker) == 30 + 1000 / 20);
    CHECK(BuffDurationSeconds(eBuff_Thorns) == 30 + 1000 / 24);
    CHECK(BuffDurationSeconds(eBuff_SwellOfMagicPower) == 1800);
    CHECK(BuffDurationSeconds(eBuff_Scroll_Healing) == 1800);
    CHECK(BuffDurationSeconds(eBuff_GMEffect) == 0);

    // The hero is a player: Sleep and Innovation use their PvP variants.
    CHECK(BuffDurationSeconds(eDeBuff_Sleep) == 4 + 1000 / 250);
    CHECK(BuffDurationSeconds(eDeBuff_DefenseDown) == 5 + 1000 / 300);

    // Caps apply before the 1 s minimum.
    CharacterAttribute->Energy = 30000;
    CHECK(BuffDurationSeconds(eBuff_AddCriticalDamage) == 180);
    CHECK(BuffDurationSeconds(eBuff_Life) == 180);
    CHECK(BuffDurationSeconds(eBuff_Berserker) == 180);
}

// The estimate counts down from the moment the hero gains the buff and restarts on refresh.
TEST_CASE("an estimated skill buff counts down [gamelogic][buff-time]")
{
    BuffSystemScope system;
    AttributeScope attributes;

    TheBuffTimeControl().RegisterEstimatedBuffTime(eBuff_Life, BuffDurationSeconds(eBuff_Life));
    float fraction = 0.f;
    REQUIRE(TheBuffTimeControl().GetEstimatedBuffRemainingFraction(eBuff_Life, fraction));
    CHECK(fraction == doctest::Approx(1.f).epsilon(0.01));
    const DWORD fullMs = TheBuffTimeControl().GetEstimatedBuffRemainingMilliseconds(eBuff_Life);
    CHECK(fullMs == 65000);

    std::this_thread::sleep_for(std::chrono::milliseconds(1100));

    REQUIRE(TheBuffTimeControl().GetEstimatedBuffRemainingFraction(eBuff_Life, fraction));
    CHECK(fraction < 0.995f);
    CHECK(fraction > 0.9f);
    CHECK(TheBuffTimeControl().GetEstimatedBuffRemainingMilliseconds(eBuff_Life) < fullMs);

    TheBuffTimeControl().RegisterEstimatedBuffTime(eBuff_Life, BuffDurationSeconds(eBuff_Life));
    REQUIRE(TheBuffTimeControl().GetEstimatedBuffRemainingFraction(eBuff_Life, fraction));
    CHECK(fraction == doctest::Approx(1.f).epsilon(0.01));

    CHECK(TheBuffTimeControl().UnRegisterEstimatedBuffTime(eBuff_Life));
    CHECK_FALSE(TheBuffTimeControl().GetEstimatedBuffRemainingFraction(eBuff_Life, fraction));
    CHECK(TheBuffTimeControl().GetEstimatedBuffRemainingMilliseconds(eBuff_Life) == 0);
}
