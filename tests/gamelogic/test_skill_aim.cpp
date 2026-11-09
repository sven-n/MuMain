// Tests of how a held right button aims a direction skill at a ground point beyond its range.

#include "stdafx.h"

#include "doctest.h"

#include "GameLogic/Combat/SkillAim.h"

#include <cmath>

using GameLogic::Combat::SkillAim::IsAimedByDirection;
using GameLogic::Combat::SkillAim::PullIntoRange;

namespace
{
constexpr float Tile = 100.0f;

// CheckTile's test: the centre of the point's tile within the range.
bool TileCentreInRange(const float origin[2], const float point[2], float rangeTiles)
{
    const float cx = std::floor(point[0] / Tile) * Tile + Tile * 0.5f;
    const float cy = std::floor(point[1] / Tile) * Tile + Tile * 0.5f;
    const float dx = origin[0] - cx;
    const float dy = origin[1] - cy;
    return dx * dx + dy * dy <= (rangeTiles * Tile) * (rangeTiles * Tile);
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
