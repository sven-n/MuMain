// doctest unit tests for the mouse-free automation primitives' pure parts:
// the weapon -> range table and the result vocabulary the auto-helper and the
// control socket share. Driving a character needs the world; these do not.
//
// Run: ctest --test-dir <build directory> --build-config Release -R "\[core\]\[automation\]"

#include "doctest.h"

#include "GameLogic/Automation/Attack.h"
#include "GameLogic/Automation/Movement.h"
#include "GameLogic/Automation/Pickup.h"

// MSVC needs a complete std::basic_ostream to print a string_view comparison.
#include <ostream>

using namespace GameLogic::Automation;

TEST_CASE("Automation reach follows the equipped weapon [core][automation]")
{
    CHECK(BasicAttackRange(false, false) == doctest::Approx(1.8f));
    CHECK(BasicAttackRange(true, false) == doctest::Approx(2.2f));
    CHECK(BasicAttackRange(false, true) == doctest::Approx(6.0f));

    // A bow in hand outranges a spear even when both read as equipped, which is
    // the order the helper's original checks had.
    CHECK(BasicAttackRange(true, true) == doctest::Approx(6.0f));

    // The named constants are the single source of those numbers.
    CHECK(BasicAttackRangeDefault < BasicAttackRangeSpear);
    CHECK(BasicAttackRangeSpear < BasicAttackRangeBow);
    CHECK(BasicAttackRange(false, false) == BasicAttackRangeDefault);
}

TEST_CASE("Automation results map to the control vocabulary [core][automation]")
{
    // The two the spec names for a refused act.
    CHECK(ResultName(AttackResult::NoPath) == "no_path");
    CHECK(ResultName(AttackResult::NotInView) == "not_in_view");
    CHECK(ResultName(MoveResult::NoPath) == "no_path");
    CHECK(ResultName(PickupResult::Gone) == "gone");
    CHECK(ResultName(PickupResult::NoPath) == "no_path");

    CHECK(ResultName(AttackResult::NotAttackable) == "not_attackable");
    CHECK(ResultName(AttackResult::Attacked) == "attacked");
    CHECK(ResultName(MoveResult::Arrived) == "arrived");
    CHECK(ResultName(PickupResult::Requested) == "requested");

    // A new enumerator that falls through its switch is what this catches,
    // so every result of the three vocabularies reachable from here is
    // walked, not just the attack one. SkillResult is not among them:
    // Skill.h needs ActionSkillType from the client's global headers, which
    // a standalone test translation unit cannot include.
    const AttackResult attacks[] = {AttackResult::Attacked, AttackResult::Approaching,  AttackResult::Busy,
                                    AttackResult::NoTarget, AttackResult::NotInView,    AttackResult::NotAttackable,
                                    AttackResult::NoPath,   AttackResult::Blocked,      AttackResult::NoArrows,
                                    AttackResult::SafeZone};
    for (const AttackResult result : attacks)
    {
        CHECK_FALSE(ResultName(result).empty());
    }

    const MoveResult moves[] = {MoveResult::Walking, MoveResult::Arrived, MoveResult::NoPath};
    for (const MoveResult result : moves)
    {
        CHECK_FALSE(ResultName(result).empty());
    }

    const PickupResult pickups[] = {PickupResult::Requested, PickupResult::Approaching, PickupResult::TooFar,
                                    PickupResult::Gone,      PickupResult::NoPath,      PickupResult::Busy};
    for (const PickupResult result : pickups)
    {
        CHECK_FALSE(ResultName(result).empty());
    }

}

TEST_CASE("Automation forgets a target only when it is hopeless [core][automation]")
{
    // What the auto-helper has always dropped a target for.
    CHECK(ShouldForgetTarget(AttackResult::NotInView));
    CHECK(ShouldForgetTarget(AttackResult::NotAttackable));
    CHECK(ShouldForgetTarget(AttackResult::NoPath));
    CHECK(ShouldForgetTarget(AttackResult::Blocked));

    // And what it keeps trying: a swing in progress, a walk in progress, an
    // empty quiver.
    CHECK_FALSE(ShouldForgetTarget(AttackResult::Attacked));
    CHECK_FALSE(ShouldForgetTarget(AttackResult::Approaching));
    CHECK_FALSE(ShouldForgetTarget(AttackResult::Busy));
    CHECK_FALSE(ShouldForgetTarget(AttackResult::NoTarget));
    CHECK_FALSE(ShouldForgetTarget(AttackResult::NoArrows));
}
