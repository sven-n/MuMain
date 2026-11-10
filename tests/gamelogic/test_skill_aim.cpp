// Tests of how a held right button aims a direction skill at a ground point beyond its range.

#include "stdafx.h"

#include "doctest.h"

#include "GameLogic/Combat/SkillAim.h"
#include "Engine/Object/ZzzInterface.h"

#include <cmath>

extern int TargetX, TargetY;

using GameLogic::Combat::SkillAim::IsAimedByDirection;
using GameLogic::Combat::SkillAim::PullIntoRange;

namespace
{
constexpr float Tile = TERRAIN_SCALE;

// The real CheckTile on the tile the point falls on, as the attack code tests it.
bool TileCentreInRange(const float origin[2], const float point[2], float rangeTiles)
{
    CHARACTER caster{};
    caster.Object.Position[0] = origin[0];
    caster.Object.Position[1] = origin[1];
    TargetX = static_cast<int>(point[0] / Tile);
    TargetY = static_cast<int>(point[1] / Tile);
    return CheckTile(&caster, &caster.Object, rangeTiles);
}
} // namespace

TEST_CASE("The skills held on the ground toward the cursor are direction skills [gamelogic][skills]")
{
    CHECK(IsAimedByDirection(AT_SKILL_EVIL_SPIRIT));
    CHECK(IsAimedByDirection(AT_SKILL_EVIL_SPIRIT_STR_MG));
    CHECK(IsAimedByDirection(AT_SKILL_TRIPLE_SHOT));
    CHECK(IsAimedByDirection(AT_SKILL_TRIPLE_SHOT_MASTERY));
    CHECK(IsAimedByDirection(AT_SKILL_POWER_SLASH));
    CHECK(IsAimedByDirection(AT_SKILL_POWER_SLASH_STR));
    CHECK(IsAimedByDirection(AT_SKILL_FIRE_SLASH));
    CHECK(IsAimedByDirection(AT_SKILL_THUNDER_STRIKE));
    CHECK(IsAimedByDirection(AT_SKILL_FLAME_STRIKE));
    CHECK(IsAimedByDirection(AT_SKILL_GIGANTIC_STORM));
    CHECK(IsAimedByDirection(AT_SKILL_LIGHTNING_SHOCK));
    CHECK(IsAimedByDirection(AT_SKILL_LIGHTNING_SHOCK_STR));

    // Skills cast on a chosen spot or creature keep their range.
    CHECK_FALSE(IsAimedByDirection(AT_SKILL_DECAY));
    CHECK_FALSE(IsAimedByDirection(AT_SKILL_ICE_STORM));
    CHECK_FALSE(IsAimedByDirection(AT_SKILL_NOVA));
    CHECK_FALSE(IsAimedByDirection(AT_SKILL_HEALING));
}

TEST_CASE("A ground point beyond the range is pulled back along its direction [gamelogic][skills]")
{
    const float origin[2] = {12050.0f, 8050.0f};
    for (const float angle : {0.0f, 0.7f, 1.9f, 3.3f, 4.4f, 5.9f})
    {
        float point[2] = {origin[0] + std::cos(angle) * 1500.0f, origin[1] + std::sin(angle) * 1500.0f};
        CHECK(PullIntoRange(origin, point, 6.0f, Tile));

        const float dx = point[0] - origin[0];
        const float dy = point[1] - origin[1];
        CHECK(std::sqrt(dx * dx + dy * dy) == doctest::Approx(500.0f));
        CHECK(std::atan2(dy, dx) == doctest::Approx(std::atan2(std::sin(angle), std::cos(angle))));
        CHECK(TileCentreInRange(origin, point, 6.0f));
    }
}

TEST_CASE("A ground point within the range is left where it is [gamelogic][skills]")
{
    const float origin[2] = {5000.0f, 5000.0f};
    float point[2] = {5300.0f, 5100.0f};
    CHECK_FALSE(PullIntoRange(origin, point, 6.0f, Tile));
    CHECK(point[0] == 5300.0f);
    CHECK(point[1] == 5100.0f);

    float tooShortRange[2] = {9000.0f, 9000.0f};
    CHECK_FALSE(PullIntoRange(origin, tooShortRange, 1.0f, Tile));
}
