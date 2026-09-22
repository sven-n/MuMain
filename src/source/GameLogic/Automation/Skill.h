// Mouse-free skill casting.
//
// Extracted from MUHelper/MuHelper.cpp with the same structure as the plain
// attack: walk into the skill's range, then let GameLogic::Combat::ExecuteSkill
// perform the cast the manual path performs.
#pragma once

#include "Data/GameData/SkillData/SkillStructs.h"

#include <cstdint>
#include <string_view>

namespace GameLogic::Automation
{
enum class SkillResult : std::uint8_t
{
    // The cast was issued this call.
    Cast,
    // Out of range: a step towards the target was taken instead.
    Approaching,
    // A swing is still playing; try again on a later frame.
    Busy,
    // A target is required but none was given.
    NoTarget,
    // The client does not know an object with that id.
    NotInView,
    // Dead, or a player while players are not allowed.
    NotAttackable,
    // The path finder found no way into range.
    NoPath,
    // In range, but a wall stands between the two.
    Blocked,
    // The client refused the cast (unknown skill, no mana, wrong state).
    Refused,
    // Nothing happened this call; the caller may try again.
    NotReady,
};

[[nodiscard]] constexpr std::string_view ResultName(SkillResult result)
{
    switch (result)
    {
    case SkillResult::Cast:
        return "cast";
    case SkillResult::Approaching:
        return "approaching";
    case SkillResult::Busy:
        return "busy";
    case SkillResult::NoTarget:
        return "no_target";
    case SkillResult::NotInView:
        return "not_in_view";
    case SkillResult::NotAttackable:
        return "not_attackable";
    case SkillResult::NoPath:
        return "no_path";
    case SkillResult::Blocked:
        return "blocked";
    case SkillResult::Refused:
        return "skill_refused";
    case SkillResult::NotReady:
        return "not_ready";
    }
    return "unknown";
}

[[nodiscard]] constexpr bool ShouldForgetTarget(SkillResult result)
{
    return result == SkillResult::NotInView || result == SkillResult::NotAttackable || result == SkillResult::NoPath ||
           result == SkillResult::Blocked || result == SkillResult::Refused;
}

// Skills that are aimed at the caster's own tile rather than at a target
// (Nova, Hellfire, Inferno). Matches AttackWizard() in ZzzInterface.cpp.
[[nodiscard]] bool IsSelfPositionSkill(ActionSkillType skill);

// Casts the skill, walking into its range first when a target is required.
// A self-positioned skill is always aimed at the caster's own tile, but a
// `targetKey` given with one is still checked: the cast is refused when
// that object has left view or is not attackable, so a scenario that names
// a target is told rather than casting into an empty tile. `targetKey` is
// ignored entirely when `targetRequired` is false. `huntingDistance` bounds how far the path
// finder may search beyond the skill's range.
SkillResult CastSkill(ActionSkillType skill, bool targetRequired, int targetKey, bool allowPlayers,
                      int huntingDistance);
} // namespace GameLogic::Automation
