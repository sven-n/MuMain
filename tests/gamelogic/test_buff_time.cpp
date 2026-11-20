#include "stdafx.h"
#include "doctest.h"

#include "GameLogic/Buffs/w_BuffStateSystem.h"

#include <filesystem>
#include <string>

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
} // namespace

// A server-timed buff starts full, which is what the strip draws.
TEST_CASE("a server-timed buff exposes its remaining share [gamelogic][buff-time]")
{
    BuffSystemScope system;
    TheBuffTimeControl().RegisterBuffTime(eBuff_Seal1, 60);

    float fraction = 0.f;
    REQUIRE(TheBuffTimeControl().GetBuffRemainingFraction(eBuff_Seal1, fraction));
    CHECK(fraction == doctest::Approx(1.f).epsilon(0.01));

    TheBuffTimeControl().UnRegisterBuffTime(eBuff_Seal1);
}

// A buff the server gives no duration has nothing to draw.
TEST_CASE("a buff without a server timer has no remaining share [gamelogic][buff-time]")
{
    BuffSystemScope system;
    float fraction = 1.f;
    CHECK_FALSE(TheBuffTimeControl().GetBuffRemainingFraction(eBuff_Life, fraction));
}
