// Tests of when a bird (Lorencia's birds, Blood Castle's crows) lands and when a landed one takes off.

#include "stdafx.h"

#include "doctest.h"

#include "Engine/AI/BirdLanding.h"

namespace
{
constexpr bool PlayerModel = true;
constexpr bool MonsterForm = false;
} // namespace

TEST_CASE("Every stop pose of SetPlayerStop keeps a landed bird [gamelogic][boids]")
{
    using Engine::AI::BirdLanding::IsHeroStill;

    CHECK(IsHeroStill(PLAYER_SET, PlayerModel));
    CHECK(IsHeroStill(PLAYER_STOP_MALE, PlayerModel));
    CHECK(IsHeroStill(PLAYER_STOP_FEMALE, PlayerModel));
    CHECK(IsHeroStill(PLAYER_STOP_SWORD, PlayerModel));
    CHECK(IsHeroStill(PLAYER_STOP_FLY, PlayerModel));
    CHECK(IsHeroStill(PLAYER_STOP_RIDE_WEAPON, PlayerModel));

    // The stop poses that come after PLAYER_WALK_MALE in the action list.
    CHECK(IsHeroStill(PLAYER_STOP_RAGEFIGHTER, PlayerModel));
    CHECK(IsHeroStill(PLAYER_DARKLORD_STAND, PlayerModel));
    CHECK(IsHeroStill(PLAYER_STOP_RIDE_HORSE, PlayerModel));
    CHECK(IsHeroStill(PLAYER_RAGE_UNI_STOP_ONE_RIGHT, PlayerModel));
    CHECK(IsHeroStill(PLAYER_STOP_TWO_HAND_SWORD_TWO, PlayerModel));
    CHECK(IsHeroStill(PLAYER_FENRIR_STAND, PlayerModel));
    CHECK(IsHeroStill(PLAYER_FENRIR_STAND_TWO_SWORD, PlayerModel));
    CHECK(IsHeroStill(PLAYER_FENRIR_STAND_ONE_RIGHT, PlayerModel));
    CHECK(IsHeroStill(PLAYER_FENRIR_STAND_ONE_LEFT, PlayerModel));
    CHECK(IsHeroStill(PLAYER_RAGE_FENRIR_STAND, PlayerModel));
    CHECK(IsHeroStill(PLAYER_RAGE_FENRIR_STAND_TWO_SWORD, PlayerModel));
    CHECK(IsHeroStill(PLAYER_RAGE_FENRIR_STAND_ONE_RIGHT, PlayerModel));
    CHECK(IsHeroStill(PLAYER_RAGE_FENRIR_STAND_ONE_LEFT, PlayerModel));
}

TEST_CASE("Sitting, healing and posing keep a landed bird [gamelogic][boids]")
{
    using Engine::AI::BirdLanding::IsHeroStill;

    CHECK(IsHeroStill(PLAYER_SIT1, PlayerModel));
    CHECK(IsHeroStill(PLAYER_SIT_FEMALE2, PlayerModel));
    CHECK(IsHeroStill(PLAYER_HEALING1, PlayerModel));
    CHECK(IsHeroStill(PLAYER_HEALING_FEMALE1, PlayerModel));
    CHECK(IsHeroStill(PLAYER_POSE1, PlayerModel));
    CHECK(IsHeroStill(PLAYER_POSE_FEMALE1, PlayerModel));
}

TEST_CASE("Walking, running, attacking and skills scare a landed bird [gamelogic][boids]")
{
    using Engine::AI::BirdLanding::IsHeroStill;

    CHECK_FALSE(IsHeroStill(PLAYER_WALK_MALE, PlayerModel));
    CHECK_FALSE(IsHeroStill(PLAYER_WALK_FEMALE, PlayerModel));
    CHECK_FALSE(IsHeroStill(PLAYER_RUN, PlayerModel));
    CHECK_FALSE(IsHeroStill(PLAYER_FENRIR_RUN, PlayerModel));
    CHECK_FALSE(IsHeroStill(PLAYER_ATTACK_FIST, PlayerModel));
    CHECK_FALSE(IsHeroStill(PLAYER_SKILL_HAND1, PlayerModel));
    CHECK_FALSE(IsHeroStill(PLAYER_SHOCK, PlayerModel));
    CHECK_FALSE(IsHeroStill(PLAYER_DIE1, PlayerModel));
}

TEST_CASE("A hero in a monster form stands still only in its stop actions [gamelogic][boids]")
{
    using Engine::AI::BirdLanding::IsHeroStill;

    CHECK(IsHeroStill(MONSTER01_STOP1, MonsterForm));
    CHECK(IsHeroStill(MONSTER01_STOP2, MonsterForm));

    // All of these are below PLAYER_WALK_MALE, so the player model's list would call them still.
    CHECK_FALSE(IsHeroStill(MONSTER01_WALK, MonsterForm));
    CHECK_FALSE(IsHeroStill(MONSTER01_RUN, MonsterForm));
    CHECK_FALSE(IsHeroStill(MONSTER01_ATTACK1, MonsterForm));
    CHECK_FALSE(IsHeroStill(MONSTER01_ATTACK5, MonsterForm));
    CHECK_FALSE(IsHeroStill(MONSTER01_SHOCK, MonsterForm));
    CHECK_FALSE(IsHeroStill(MONSTER01_DIE, MonsterForm));
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
