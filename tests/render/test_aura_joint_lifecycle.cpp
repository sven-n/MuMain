#include <doctest.h>

#include "Render/Effects/AuraJointLifecycle.h"

TEST_CASE("supported buffs keep character aura joints alive [render][aura]")
{
    CHECK(Render::Effects::ShouldKeepAuraJointAlive(true, false, false));
    CHECK(Render::Effects::ShouldKeepAuraJointAlive(false, true, false));
    CHECK(Render::Effects::ShouldKeepAuraJointAlive(false, false, true));
}

TEST_CASE("character aura joint expires without a supported buff [render][aura]")
{
    CHECK_FALSE(Render::Effects::ShouldKeepAuraJointAlive(false, false, false));
}
