#include "stdafx.h"

#include "doctest.h"

#include "Core/Globals/_enum.h"
#include "Render/Effects/Behaviors/EffectBehaviors.h"
#include "Render/Effects/EffectRegistry.h"

// A test binary of its own, so that no other test has built the registry
// before this case, however the tests are run. The handlers are code and are
// there before the effect catalogue is loaded; only its creation values are
// missing until then.
TEST_CASE("The effect registry has its handlers before the catalogue is loaded [data][effects]")
{
    const Render::Effects::EffectDescriptor* desair = Render::Effects::Lookup(MODEL_DESAIR);
    REQUIRE(desair != nullptr);
    CHECK(desair->move == &Render::Effects::Behaviors::MoveDesair);
    // The catalogue gives MODEL_DESAIR creation values; they are not there yet.
    CHECK_FALSE(desair->create.has_value());
}
