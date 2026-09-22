// Mouse-free plain attacks.
//
// Extracted from MUHelper/MuHelper.cpp. The auto-helper calls it with players
// excluded (it is a grinding aid); the control socket allows them, because a
// PvP scenario is exactly what it is for. Everything else — range by weapon,
// walking into range two steps at a time, the action that swings — is the
// helper's original behaviour, unchanged.
#pragma once

#include <cstdint>
#include <string_view>

namespace GameLogic::Automation
{
enum class AttackResult : std::uint8_t
{
    // The swing was issued this call.
    Attacked,
    // Out of range: a step towards the target was taken instead.
    Approaching,
    // A swing is still playing; try again on a later frame.
    Busy,
    // No target was given.
    NoTarget,
    // The client does not know an object with that id.
    NotInView,
    // Dead, or a player while players are not allowed.
    NotAttackable,
    // The path finder found no way into range.
    NoPath,
    // In range, but a wall stands between the two.
    Blocked,
    // A bow or crossbow is equipped and the quiver is empty.
    NoArrows,
    // The character stands in a safe zone, where the game refuses attacks.
    SafeZone,
};

// Plain-attack reach in tiles. Named once here; the helper used to carry
// its own copies.
inline constexpr float BasicAttackRangeDefault = 1.8f;
inline constexpr float BasicAttackRangeSpear = 2.2f;
inline constexpr float BasicAttackRangeBow = 6.0f;

// A bow outranges a spear, which outreaches everything else.
[[nodiscard]] constexpr float BasicAttackRange(bool spearEquipped, bool bowEquipped)
{
    if (bowEquipped)
    {
        return BasicAttackRangeBow;
    }
    return spearEquipped ? BasicAttackRangeSpear : BasicAttackRangeDefault;
}

[[nodiscard]] constexpr std::string_view ResultName(AttackResult result)
{
    switch (result)
    {
    case AttackResult::Attacked:
        return "attacked";
    case AttackResult::Approaching:
        return "approaching";
    case AttackResult::Busy:
        return "busy";
    case AttackResult::NoTarget:
        return "no_target";
    case AttackResult::NotInView:
        return "not_in_view";
    case AttackResult::NotAttackable:
        return "not_attackable";
    case AttackResult::NoPath:
        return "no_path";
    case AttackResult::Blocked:
        return "blocked";
    case AttackResult::NoArrows:
        return "no_arrows";
    case AttackResult::SafeZone:
        return "safe_zone";
    }
    return "unknown";
}

// Whether a caller that keeps a target list should drop this target: the
// outcomes the auto-helper has always treated as "forget it and pick
// another one".
[[nodiscard]] constexpr bool ShouldForgetTarget(AttackResult result)
{
    return result == AttackResult::NotInView || result == AttackResult::NotAttackable ||
           result == AttackResult::NoPath || result == AttackResult::Blocked;
}

// True while a swing animation is still playing, so callers keep the
// cadence of AttackSpeed instead of their own timer.
[[nodiscard]] bool IsSwingInProgress();

// Attacks the object with that server-assigned id, walking into range
// first when needed. `huntingDistance` bounds how far the path finder may
// search beyond the weapon's reach.
AttackResult AttackObject(int targetKey, bool allowPlayers, int huntingDistance);
} // namespace GameLogic::Automation
