#include "stdafx.h"
#include "GameLogic/Automation/Attack.h"

#include "Character/CharacterManager.h"
#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/PlayerActionState.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"

#include <algorithm>

// Movement/target globals are defined in ZzzInterface.cpp.
extern int SelectedCharacter;
extern int TargetX;
extern int TargetY;

namespace GameLogic::Automation
{
bool IsSwingInProgress()
{
    const int action = Hero->Object.CurrentAction;

    // Outside the swing enum range entirely -> not a swing.
    if (!Engine::Object::IsAttackAction(action))
    {
        return false;
    }

    // Several non-swing *stance* animations (mounted idle/walk/run, two-hand-
    // sword stance, ride-horse, rage-fenrir) share the [PLAYER_ATTACK_FIST ..
    // PLAYER_RIDE_SKILL] enum range that IsAttackAction() spans. MoveHero
    // (ZzzInterface.cpp) OR-excludes exactly these four ranges when deciding
    // whether the hero may move; mirror that here. Otherwise a Fenrir-mounted
    // idle character (CurrentAction == PLAYER_FENRIR_STAND, inside the range)
    // reads as a perpetual swing and no automated action ever fires.
    if ((action >= PLAYER_STOP_TWO_HAND_SWORD_TWO && action <= PLAYER_RUN_TWO_HAND_SWORD_TWO) ||
        (action >= PLAYER_DARKLORD_STAND && action <= PLAYER_RUN_RIDE_HORSE) ||
        (action >= PLAYER_FENRIR_RUN && action <= PLAYER_FENRIR_WALK_ONE_LEFT) ||
        (action >= PLAYER_RAGE_FENRIR_WALK && action <= PLAYER_RAGE_FENRIR_STAND_ONE_LEFT))
    {
        return false;
    }

    // Genuine attack/skill swing -> Fenrir attack/skill actions sit below
    // PLAYER_FENRIR_RUN, so they stay gated and cadence still tracks
    // AttackSpeed when mounted.
    return true;
}

// Steps of the planned path that are walked per call. Re-planning every
// frame keeps a moving target reachable without committing to a long walk.
constexpr int StepsPerApproach = 2;

// Walks the first steps of `path` towards the target and sends the move.
static void ApproachAlong(const PATH_t& path)
{
    Hero->Path.Lock.lock();
    const int steps = std::min<int>(path.PathNum, StepsPerApproach);
    for (int step = 0; step < steps; ++step)
    {
        Hero->Path.PathX[step] = path.PathX[step];
        Hero->Path.PathY[step] = path.PathY[step];
    }
    Hero->Path.PathNum = static_cast<unsigned char>(steps);
    Hero->Path.CurrentPath = 0;
    Hero->Path.CurrentPathFloat = 0;
    Hero->Path.Lock.unlock();

    // A walk the engine can recognise: without this the character keeps the
    // movement mode of whatever it was doing before (MOVEMENT_ATTACK after a
    // swing), which is not what a walk towards a target is.
    Hero->MovementType = MOVEMENT_MOVE;
    SendMove(Hero, &Hero->Object);
}

// The reach of whatever is in the character's hands right now.
static float EquippedAttackRange()
{
    const int rightHand = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
    const bool spear = rightHand >= ITEM_SPEAR && rightHand < ITEM_SPEAR + MAX_ITEM_INDEX;
    const bool bow = gCharacterManager.GetEquipedBowType() != BOWTYPE_NONE;
    return BasicAttackRange(spear, bow);
}

AttackResult AttackObject(int targetKey, bool allowPlayers, int huntingDistance)
{
    if (targetKey == -1)
    {
        return AttackResult::NoTarget;
    }

    // Let the current swing finish before attacking again, so the cadence
    // tracks AttackSpeed instead of the caller's timer.
    if (IsSwingInProgress())
    {
        return AttackResult::Busy;
    }

    const int targetIndex = FindCharacterIndex(targetKey);
    if (targetIndex == MAX_CHARACTERS_CLIENT)
    {
        return AttackResult::NotInView;
    }

    CHARACTER* target = &CharactersClient[targetIndex];
    if (target->Dead > 0 || (!allowPlayers && !IsMonster(target)))
    {
        return AttackResult::NotAttackable;
    }

    const float range = EquippedAttackRange();

    // The target tile is what the path finder and the wall test read.
    TargetX = (int)(target->Object.Position[0] / TERRAIN_SCALE);
    TargetY = (int)(target->Object.Position[1] / TERRAIN_SCALE);

    // CheckWall reads the global SelectedCharacter on every step of its walk
    // (ZzzInterface.cpp) to exempt gate models from the terrain-wall test —
    // gates stand on no-move tiles, so without the exemption they can never
    // be attacked. The selection therefore has to be this target *before*
    // the checks, as the helper path this was extracted from did. A refused
    // attack still must not leave the UI pointing at something it will not
    // hit, so the previous selection is put back on the refusal paths.
    const int previousSelection = SelectedCharacter;
    SelectedCharacter = targetIndex;

    // Range first, then a path only when one is needed — the order the
    // engine's own attack loop uses (ZzzInterface.cpp:1282, 1349). The other
    // way round refuses a target that is standing in range but that no walk
    // reaches: across a fence or a stretch of water, where the point of
    // having range is that the walk is not needed.
    const bool inRange = CheckTile(Hero, &Hero->Object, range);
    if (inRange && !CheckWall(Hero->PositionX, Hero->PositionY, TargetX, TargetY))
    {
        SelectedCharacter = previousSelection;
        return AttackResult::Blocked;
    }

    PATH_t path;
    if (!inRange && !PathFinding2(Hero->PositionX, Hero->PositionY, TargetX, TargetY, &path,
                                  static_cast<float>(huntingDistance) + range))
    {
        SelectedCharacter = previousSelection;
        return AttackResult::NoPath;
    }

    if (!inRange)
    {
        ApproachAlong(path);
        return AttackResult::Approaching;
    }

    if (gCharacterManager.GetEquipedBowType() != BOWTYPE_NONE && !CheckArrow())
    {
        return AttackResult::NoArrows;
    }

    // Action() drops the hit packet silently inside a safe zone; say so
    // rather than reporting a swing that never reached the server.
    if (Hero->SafeZone)
    {
        return AttackResult::SafeZone;
    }

    Hero->MovementType = MOVEMENT_ATTACK;
    ActionTarget = targetIndex;
    Attacking = 1;
    Action(Hero, &Hero->Object, true);
    return AttackResult::Attacked;
}
} // namespace GameLogic::Automation
