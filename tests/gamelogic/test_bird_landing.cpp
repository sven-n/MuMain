// Tests of when a bird (Lorencia's birds, Blood Castle's crows) lands and when a landed one takes off.

#include "stdafx.h"

#include "doctest.h"

#include "Engine/AI/BirdLanding.h"

TEST_CASE("Every stop pose of SetPlayerStop keeps a landed bird [gamelogic][boids]")
{
    using Engine::AI::BirdLanding::IsHeroStill;

    CHECK(IsHeroStill(PLAYER_SET));
    CHECK(IsHeroStill(PLAYER_STOP_MALE));
    CHECK(IsHeroStill(PLAYER_STOP_FEMALE));
    CHECK(IsHeroStill(PLAYER_STOP_SWORD));
    CHECK(IsHeroStill(PLAYER_STOP_FLY));
    CHECK(IsHeroStill(PLAYER_STOP_RIDE_WEAPON));

    // The stop poses that come after PLAYER_WALK_MALE in the action list.
    CHECK(IsHeroStill(PLAYER_STOP_RAGEFIGHTER));
    CHECK(IsHeroStill(PLAYER_DARKLORD_STAND));
    CHECK(IsHeroStill(PLAYER_STOP_RIDE_HORSE));
    CHECK(IsHeroStill(PLAYER_RAGE_UNI_STOP_ONE_RIGHT));
    CHECK(IsHeroStill(PLAYER_STOP_TWO_HAND_SWORD_TWO));
    CHECK(IsHeroStill(PLAYER_FENRIR_STAND));
    CHECK(IsHeroStill(PLAYER_FENRIR_STAND_TWO_SWORD));
    CHECK(IsHeroStill(PLAYER_FENRIR_STAND_ONE_RIGHT));
    CHECK(IsHeroStill(PLAYER_FENRIR_STAND_ONE_LEFT));
    CHECK(IsHeroStill(PLAYER_RAGE_FENRIR_STAND));
    CHECK(IsHeroStill(PLAYER_RAGE_FENRIR_STAND_TWO_SWORD));
    CHECK(IsHeroStill(PLAYER_RAGE_FENRIR_STAND_ONE_RIGHT));
    CHECK(IsHeroStill(PLAYER_RAGE_FENRIR_STAND_ONE_LEFT));
}

TEST_CASE("Sitting, healing and posing keep a landed bird [gamelogic][boids]")
{
    using Engine::AI::BirdLanding::IsHeroStill;

    CHECK(IsHeroStill(PLAYER_SIT1));
    CHECK(IsHeroStill(PLAYER_SIT_FEMALE2));
    CHECK(IsHeroStill(PLAYER_HEALING1));
    CHECK(IsHeroStill(PLAYER_HEALING_FEMALE1));
    CHECK(IsHeroStill(PLAYER_POSE1));
    CHECK(IsHeroStill(PLAYER_POSE_FEMALE1));
}

TEST_CASE("Walking, running, attacking and skills scare a landed bird [gamelogic][boids]")
{
    using Engine::AI::BirdLanding::IsHeroStill;

    CHECK_FALSE(IsHeroStill(PLAYER_WALK_MALE));
    CHECK_FALSE(IsHeroStill(PLAYER_WALK_FEMALE));
    CHECK_FALSE(IsHeroStill(PLAYER_RUN));
    CHECK_FALSE(IsHeroStill(PLAYER_FENRIR_RUN));
    CHECK_FALSE(IsHeroStill(PLAYER_ATTACK_FIST));
    CHECK_FALSE(IsHeroStill(PLAYER_SKILL_HAND1));
    CHECK_FALSE(IsHeroStill(PLAYER_SHOCK));
    CHECK_FALSE(IsHeroStill(PLAYER_DIE1));
}

TEST_CASE("A bird lands only on free, open ground [gamelogic][boids]")
{
    using Engine::AI::BirdLanding::CanLandOn;

    CHECK(CanLandOn(0));
    CHECK(CanLandOn(TW_SAFEZONE));
    CHECK(CanLandOn(TW_SAFEZONE | TW_NOATTACKZONE));

    CHECK_FALSE(CanLandOn(TW_NOMOVE));
    CHECK_FALSE(CanLandOn(TW_NOGROUND));
    CHECK_FALSE(CanLandOn(TW_WATER));
    CHECK_FALSE(CanLandOn(TW_CHARACTER));
    CHECK_FALSE(CanLandOn(TW_SAFEZONE | TW_NOMOVE));
}
