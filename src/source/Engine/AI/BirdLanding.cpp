#include "stdafx.h"
#include "Engine/AI/BirdLanding.h"

namespace Engine::AI::BirdLanding
{
namespace
{
bool IsPlayerModelStill(int heroAction)
{
    // The basic stop poses come before the walks in the action list.
    if (heroAction >= PLAYER_SET && heroAction < PLAYER_WALK_MALE)
    {
        return true;
    }

    if (heroAction >= PLAYER_SIT1 && heroAction <= PLAYER_POSE_FEMALE1)
    {
        return true;
    }

    // The stop poses added after the walks.
    switch (heroAction)
    {
    case PLAYER_STOP_TWO_HAND_SWORD_TWO:
    case PLAYER_STOP_RAGEFIGHTER:
    case PLAYER_DARKLORD_STAND:
    case PLAYER_STOP_RIDE_HORSE:
    case PLAYER_RAGE_UNI_STOP_ONE_RIGHT:
    case PLAYER_FENRIR_STAND:
    case PLAYER_FENRIR_STAND_TWO_SWORD:
    case PLAYER_FENRIR_STAND_ONE_RIGHT:
    case PLAYER_FENRIR_STAND_ONE_LEFT:
    case PLAYER_RAGE_FENRIR_STAND:
    case PLAYER_RAGE_FENRIR_STAND_TWO_SWORD:
    case PLAYER_RAGE_FENRIR_STAND_ONE_RIGHT:
    case PLAYER_RAGE_FENRIR_STAND_ONE_LEFT:
        return true;
    default:
        return false;
    }
}

// A monster form has the monster's actions, whose numbers overlap the player's (MONSTER01_WALK is
// PLAYER_STOP_FEMALE), so it needs its own list.
bool IsMonsterFormStill(int heroAction)
{
    return heroAction == MONSTER01_STOP1 || heroAction == MONSTER01_STOP2;
}
} // namespace

bool IsHeroStill(int heroAction, bool playerModel)
{
    return playerModel ? IsPlayerModelStill(heroAction) : IsMonsterFormStill(heroAction);
}

bool CanLandOn(unsigned int terrainWallFlags)
{
    constexpr unsigned int NoLanding = TW_CHARACTER | TW_NOMOVE | TW_NOGROUND | TW_WATER;
    return (terrainWallFlags & NoLanding) == 0;
}
} // namespace Engine::AI::BirdLanding
