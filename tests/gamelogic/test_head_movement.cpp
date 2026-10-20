#include "stdafx.h"
#include "doctest.h"

#include "Engine/AI/ZzzAI.h"

#include <array>
#include <cmath>

TEST_CASE("Head smoothing preserves the reference turn and leaves roll unchanged [gamelogic][head]")
{
    vec3_t angles = {10.f, 50.f, 7.f};
    const vec3_t targets = {60.f, 0.f, 100.f};
    SmoothHeadAngles(angles, targets, 1.f);
    CHECK(angles[0] == doctest::Approx(20.f));
    CHECK(angles[1] == doctest::Approx(40.f));
    CHECK(angles[2] == 7.f);
}

TEST_CASE("Head smoothing has the same response over equal elapsed time [gamelogic][head]")
{
    constexpr double DurationSeconds = 0.2;
    const float remainingGap = std::pow(0.8f, static_cast<float>(REFERENCE_FPS * DurationSeconds));
    for (const double fps : std::array{25.0, 30.0, 60.0, 120.0, 240.0, 1000.0})
    {
        CAPTURE(fps);
        vec3_t angles = {0.f, 60.f, 7.f};
        const vec3_t targets = {60.f, 0.f, 100.f};
        const float factor = static_cast<float>(REFERENCE_FPS / fps);
        const int frames = static_cast<int>(std::lround(DurationSeconds * fps));
        for (int frame = 0; frame < frames; ++frame)
        {
            SmoothHeadAngles(angles, targets, factor);
        }
        CHECK(angles[0] == doctest::Approx(60.f * (1.f - remainingGap)).epsilon(0.0001));
        CHECK(angles[1] == doctest::Approx(60.f * remainingGap).epsilon(0.0001));
        CHECK(angles[2] == 7.f);
    }
}

TEST_CASE("Head smoothing takes the shortest turn across zero [gamelogic][head]")
{
    vec3_t angles = {350.f, 10.f, 0.f};
    const vec3_t targets = {10.f, 350.f, 0.f};
    SmoothHeadAngles(angles, targets, 1.f);
    CHECK(angles[0] == doctest::Approx(354.f));
    CHECK(angles[1] == doctest::Approx(6.f));
}

TEST_CASE("Head smoothing stops without elapsed time and caps slow frames [gamelogic][head]")
{
    const vec3_t targets = {60.f, 0.f, 0.f};
    for (const float factor : std::array{0.f, -1.f})
    {
        vec3_t angles = {10.f, 50.f, 7.f};
        SmoothHeadAngles(angles, targets, factor);
        CHECK(angles[0] == 10.f);
        CHECK(angles[1] == 50.f);
        CHECK(angles[2] == 7.f);
    }
    vec3_t angles = {10.f, 50.f, 7.f};
    SmoothHeadAngles(angles, targets, 2.f);
    CHECK(angles[0] == doctest::Approx(20.f));
    CHECK(angles[1] == doctest::Approx(40.f));
}

TEST_CASE("Head smoothing retains fractional high-FPS turns and never overshoots [gamelogic][head]")
{
    vec3_t angles = {10.f, 50.f, 7.f};
    const vec3_t targets = {11.f, 49.f, 0.f};
    const float factor = static_cast<float>(REFERENCE_FPS / 1000.0);
    SmoothHeadAngles(angles, targets, factor);
    CHECK(angles[0] > 10.f);
    CHECK(angles[0] < 11.f);
    CHECK(angles[1] < 50.f);
    CHECK(angles[1] > 49.f);
    for (int frame = 0; frame < 1000; ++frame)
    {
        SmoothHeadAngles(angles, targets, factor);
    }
    CHECK(angles[0] <= 11.f);
    CHECK(angles[1] >= 49.f);
}

TEST_CASE("Head smoothing responds to a changed target without resetting orientation [gamelogic][head]")
{
    vec3_t angles = {0.f, 0.f, 7.f};
    const vec3_t firstTarget = {60.f, 0.f, 0.f};
    SmoothHeadAngles(angles, firstTarget, 1.f);
    CHECK(angles[0] == doctest::Approx(12.f));
    const vec3_t nextTarget = {350.f, 0.f, 0.f};
    SmoothHeadAngles(angles, nextTarget, 1.f);
    CHECK(angles[0] == doctest::Approx(7.6f));
}
