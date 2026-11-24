// Tests of when an object counts as mid-teleport, the time the hero may not walk (issue #1152).

#include "doctest.h"

#include "Engine/Object/TeleportState.h"

namespace
{
constexpr float Visible = 1.f;
constexpr float Hidden = 0.f;
} // namespace

TEST_CASE("An object is mid-teleport while it vanishes and while it is hidden [gamelogic][teleport]")
{
    using Engine::Object::IsMidTeleport;

    CHECK(IsMidTeleport(TELEPORT_BEGIN, Visible));
    CHECK(IsMidTeleport(TELEPORT, Hidden));
}

TEST_CASE("A landed object is mid-teleport until it has faded back in [gamelogic][teleport]")
{
    using Engine::Object::IsMidTeleport;
    using Engine::Object::TeleportVisibleAlpha;

    CHECK(IsMidTeleport(TELEPORT_END, Hidden));
    CHECK_FALSE(IsMidTeleport(TELEPORT_END, TeleportVisibleAlpha));
    CHECK_FALSE(IsMidTeleport(TELEPORT_END, Visible));
}

TEST_CASE("An object that is not teleporting is not mid-teleport [gamelogic][teleport]")
{
    CHECK_FALSE(Engine::Object::IsMidTeleport(TELEPORT_NONE, Visible));
}
