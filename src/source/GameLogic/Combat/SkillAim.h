#pragma once

// Skills that fire toward the cursor (or around the caster) instead of at a
// clicked target. When the player holds the right button on the ground, or on a
// character that can't be attacked, beyond the skill's range, the aim point is
// pulled back along the same direction so the skill still fires. See
// docs/skill-aiming.md.

#include <cmath>

#include "Core/Globals/_enum.h"

namespace GameLogic::Combat::SkillAim
{
[[nodiscard]] constexpr bool IsAimedByDirection(int skill)
{
    switch (skill)
    {
    case AT_SKILL_STORM:
    case AT_SKILL_EVIL_SPIRIT:
    case AT_SKILL_EVIL_SPIRIT_STR:
    case AT_SKILL_EVIL_SPIRIT_STR_MG:
    case AT_SKILL_HELL_FIRE:
    case AT_SKILL_HELL_FIRE_STR:
    case AT_SKILL_FLASH:
    case AT_SKILL_INFERNO:
    case AT_SKILL_INFERNO_STR:
    case AT_SKILL_INFERNO_STR_MG:
    case AT_SKILL_FLAME_STRIKE:
    case AT_SKILL_GIGANTIC_STORM:
    case AT_SKILL_LIGHTNING_SHOCK:
    case AT_SKILL_LIGHTNING_SHOCK_STR:
    case AT_SKILL_TRIPLE_SHOT:
    case AT_SKILL_TRIPLE_SHOT_STR:
    case AT_SKILL_TRIPLE_SHOT_MASTERY:
    case AT_SKILL_PENETRATION:
    case AT_SKILL_PENETRATION_STR:
    case AT_SKILL_BLAST_CROSSBOW4:
    case AT_SKILL_MULTI_SHOT:
    case AT_SKILL_FIRE_SLASH:
    case AT_SKILL_FIRE_SLASH_STR:
    case AT_SKILL_POWER_SLASH:
    case AT_SKILL_POWER_SLASH_STR:
    case AT_SKILL_TWISTING_SLASH:
    case AT_SKILL_TWISTING_SLASH_STR:
    case AT_SKILL_TWISTING_SLASH_STR_MG:
    case AT_SKILL_TWISTING_SLASH_MASTERY:
    case AT_SKILL_RAGEFUL_BLOW:
    case AT_SKILL_RAGEFUL_BLOW_STR:
    case AT_SKILL_RAGEFUL_BLOW_MASTERY:
    case AT_SKILL_CHAOTIC_DISEIER:
    case AT_SKILL_FIRE_SCREAM:
    case AT_SKILL_FIRE_SCREAM_STR:
    case AT_SKILL_EARTHSHAKE:
    case AT_SKILL_EARTHSHAKE_STR:
    case AT_SKILL_EARTHSHAKE_MASTERY:
    case AT_SKILL_THUNDER_STRIKE:
        return true;
    default:
        return false;
    }
}

// Moves `point` (x, y in world units) toward `origin` until it lies within
// `rangeTiles` - 1 tiles, keeping its direction, so the centre of the tile it falls
// on stays inside the range CheckTile tests. Returns whether it moved.
inline bool PullIntoRange(const float origin[2], float point[2], float rangeTiles, float tileScale)
{
    const float reach = (rangeTiles - 1.0f) * tileScale;
    if (reach <= 0.0f)
    {
        return false;
    }
    const float dx = point[0] - origin[0];
    const float dy = point[1] - origin[1];
    const float length = std::sqrt(dx * dx + dy * dy);
    if (length <= reach)
    {
        return false;
    }
    const float scale = reach / length;
    point[0] = origin[0] + dx * scale;
    point[1] = origin[1] + dy * scale;
    return true;
}
// What auto-attack looks at before it casts the last skill again without a button.
struct AutoAttackState
{
    bool optionOn = false;            // the auto-attack option
    bool mapAllowsIt = false;         // not the stadium or Chaos Castle
    bool afterSkillCast = false;      // a skill was cast (Attacking == 2)
    bool hasSelection = false;        // a character is under the cursor
    bool selectionAttackable = false; // CheckAttack() for that character
    int skill = 0;                    // the skill to cast again
};

// Whether auto-attack casts the last skill again. It has to agree with CheckTarget's pull: a character that can't be
// attacked gives a direction skill an aim at any distance, so it must not keep a direction skill going. Other skills
// (Soul Barrier on a party member, for one) keep repeating on any character under the cursor, as before.
[[nodiscard]] constexpr bool RepeatsAutoAttack(const AutoAttackState& state)
{
    if (!state.optionOn || !state.mapAllowsIt || !state.afterSkillCast || !state.hasSelection)
    {
        return false;
    }
    return state.selectionAttackable || !IsAimedByDirection(state.skill);
}
} // namespace GameLogic::Combat::SkillAim
