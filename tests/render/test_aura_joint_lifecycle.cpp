#include <doctest.h>

#include "Render/Effects/AuraJointLifecycle.h"

TEST_CASE("defense buffs keep character aura joints alive [render][aura]")
{
    CHECK(Render::Effects::ShouldKeepAuraJointAlive(true, false));
    CHECK(Render::Effects::ShouldKeepAuraJointAlive(false, true));
}

TEST_CASE("character aura joint expires without a defense buff [render][aura]")
{
    // Greater Damage alone has no aura, so the aura of an expired Greater Defense ends.
    CHECK_FALSE(Render::Effects::ShouldKeepAuraJointAlive(false, false));
}
