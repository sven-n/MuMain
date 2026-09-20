#include "stdafx.h"
#include "GameLogic/Automation/Skill.h"

#include "GameLogic/Automation/Attack.h"
#include "GameLogic/Combat/SkillExecution.h"
#include "GameLogic/Skills/SkillManager.h"
#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"

#include <algorithm>

// Movement/target globals are defined in ZzzInterface.cpp.
extern MovementSkill g_MovementSkill;
extern int SelectedCharacter;
extern int TargetX;
extern int TargetY;

namespace GameLogic::Automation
{
// Steps of the planned path that are walked per call, as in Attack.cpp.
constexpr int StepsPerApproach = 2;

// Result of ExecuteSkill: -1 refused, 0 nothing yet, 1 cast.
constexpr int SkillExecutionRefused = -1;
constexpr int SkillExecutionCast = 1;

bool IsSelfPositionSkill(ActionSkillType skill)
{
    return (skill == AT_SKILL_NOVA_BEGIN || skill == AT_SKILL_NOVA || skill == AT_SKILL_HELL_FIRE ||
            skill == AT_SKILL_HELL_FIRE_STR || skill == AT_SKILL_INFERNO || skill == AT_SKILL_INFERNO_STR ||
            skill == AT_SKILL_INFERNO_STR_MG);
}

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

    SendMove(Hero, &Hero->Object);
}

// Aims a self-positioned skill at the caster's own tile, checking that a
// named target, if any, is still worth casting at.
static SkillResult AimAtSelf(int targetKey, bool allowPlayers)
{
    TargetX = Hero->PositionX;
    TargetY = Hero->PositionY;
    g_MovementSkill.m_iTarget = -1;

    if (targetKey == -1)
    {
        return SkillResult::NotReady;
    }

    const int targetIndex = FindCharacterIndex(targetKey);
    if (targetIndex == MAX_CHARACTERS_CLIENT)
    {
        return SkillResult::NotInView;
    }

    CHARACTER* target = &CharactersClient[targetIndex];
    if (target->Dead > 0 || (!allowPlayers && !IsMonster(target)))
    {
        return SkillResult::NotAttackable;
    }

    return SkillResult::NotReady;
}

// Aims a targeted skill, walking into range when needed.
static SkillResult AimAtTarget(int targetKey, bool allowPlayers, int huntingDistance, float skillDistance)
{
    if (targetKey == -1)
    {
        return SkillResult::NoTarget;
    }

    const int targetIndex = FindCharacterIndex(targetKey);
    if (targetIndex == MAX_CHARACTERS_CLIENT)
    {
        return SkillResult::NotInView;
    }

    CHARACTER* target = &CharactersClient[targetIndex];
    if (target->Dead > 0 || (!allowPlayers && !IsMonster(target)))
    {
        return SkillResult::NotAttackable;
    }

    // Selected only once the target is one the client may actually cast at:
    // a refused cast must not leave the UI pointing at a corpse.
    SelectedCharacter = targetIndex;
    g_MovementSkill.m_iTarget = targetIndex;

    TargetX = (int)(target->Object.Position[0] / TERRAIN_SCALE);
    TargetY = (int)(target->Object.Position[1] / TERRAIN_SCALE);

    // Range first, then a path only when one is needed — the order the
    // engine's own attack loop uses (ZzzInterface.cpp:1282, 1349). The other
    // way round refuses a target that is standing in range but that no walk
    // reaches: across a fence or a stretch of water, where the point of
    // having range is that the walk is not needed.
    const bool inRange = CheckTile(Hero, &Hero->Object, skillDistance);
    if (inRange && !CheckWall(Hero->PositionX, Hero->PositionY, TargetX, TargetY))
    {
        return SkillResult::Blocked;
    }

    PATH_t path;
    if (!inRange && !PathFinding2(Hero->PositionX, Hero->PositionY, TargetX, TargetY, &path,
                                  static_cast<float>(huntingDistance) + skillDistance))
    {
        return SkillResult::NoPath;
    }

    if (!inRange)
    {
        ApproachAlong(path);
        return SkillResult::Approaching;
    }

    return SkillResult::NotReady;
}

SkillResult CastSkill(ActionSkillType skill, bool targetRequired, int targetKey, bool allowPlayers, int huntingDistance)
{
    // Let the current swing finish before issuing another action.
    if (IsSwingInProgress())
    {
        return SkillResult::Busy;
    }

    g_MovementSkill.m_iSkill = skill;
    g_MovementSkill.m_bMagic = true;

    const float skillDistance = gSkillManager.GetSkillDistance(skill, Hero);

    if (!targetRequired)
    {
        TargetX = Hero->PositionX;
        TargetY = Hero->PositionY;
        // Both aiming paths set this; without it here an untargeted skill
        // would be cast at whatever the previous targeted one aimed at.
        g_MovementSkill.m_iTarget = -1;
    }
    else if (IsSelfPositionSkill(skill))
    {
        const SkillResult aimed = AimAtSelf(targetKey, allowPlayers);
        if (aimed != SkillResult::NotReady)
        {
            return aimed;
        }
    }
    else
    {
        const SkillResult aimed = AimAtTarget(targetKey, allowPlayers, huntingDistance, skillDistance);
        if (aimed != SkillResult::NotReady)
        {
            return aimed;
        }
    }

    const int executed = GameLogic::Combat::ExecuteSkill(Hero, skill, skillDistance);
    if (executed == SkillExecutionRefused)
    {
        return SkillResult::Refused;
    }

    return executed == SkillExecutionCast ? SkillResult::Cast : SkillResult::NotReady;
}
} // namespace GameLogic::Automation
