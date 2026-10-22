#include "doctest.h"

#include "Engine/AI/BoidSteering.h"
#include "Core/Utilities/Random.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace
{
constexpr double ReferenceFps = 25.0;
constexpr float BirdTurnDegrees = 13.f;

float ReferenceFrames(double fps)
{
    return static_cast<float>(std::clamp(ReferenceFps / fps, 0.0, 1.0));
}
} // namespace

TEST_CASE("Flock directions depend on geometry, not frame time [gamelogic][boids]")
{
    float x = 0.f;
    float y = 0.f;
    Engine::AI::BoidSteering::AddDirection(3.f, 4.f, x, y);
    CHECK(x == doctest::Approx(0.6f));
    CHECK(y == doctest::Approx(0.8f));
    Engine::AI::BoidSteering::AddDirection(30.f, 40.f, x, y);
    CHECK(x == doctest::Approx(1.2f));
    CHECK(y == doctest::Approx(1.6f));
}

TEST_CASE("Coincident flock targets do not create invalid headings [gamelogic][boids]")
{
    float x = 0.25f;
    float y = -0.75f;
    Engine::AI::BoidSteering::AddDirection(0.f, 0.f, x, y);
    CHECK(x == 0.25f);
    CHECK(y == -0.75f);
}

TEST_CASE("Invalid flock directions do not contaminate the accumulated target [gamelogic][boids]")
{
    constexpr std::array invalidValues{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN()};
    for (const float invalid : invalidValues)
    {
        float x = 0.25f;
        float y = -0.75f;
        Engine::AI::BoidSteering::AddDirection(invalid, 0.f, x, y);
        CHECK(x == 0.25f);
        CHECK(y == -0.75f);
    }
}

TEST_CASE("Opposing flock directions cancel without a preferred heading [gamelogic][boids]")
{
    float x = 0.f;
    float y = 0.f;
    Engine::AI::BoidSteering::AddDirection(3.f, 4.f, x, y);
    Engine::AI::BoidSteering::AddDirection(-3.f, -4.f, x, y);
    CHECK(x == 0.f);
    CHECK(y == 0.f);
}

TEST_CASE("Flock turning preserves the reference step and fractional high-FPS steps [gamelogic][boids]")
{
    using Engine::AI::BoidSteering::TurnTowards;
    CHECK(TurnTowards(10.f, 170.f, BirdTurnDegrees, ReferenceFrames(25.0)) == 23.f);
    CHECK(TurnTowards(10.125f, 170.f, BirdTurnDegrees, ReferenceFrames(1000.0)) == doctest::Approx(10.45f));
}

TEST_CASE("Flock turns cover the same angle over equal reference time [gamelogic][boids]")
{
    constexpr double durationSeconds = 0.16;
    constexpr std::array frameRates{25.0, 50.0, 75.0, 100.0, 125.0, 250.0, 1000.0};
    for (const double fps : frameRates)
    {
        CAPTURE(fps);
        float heading = 10.f;
        const int frames = static_cast<int>(std::lround(durationSeconds * fps));
        for (int frame = 0; frame < frames; ++frame)
        {
            heading = Engine::AI::BoidSteering::TurnTowards(heading, 170.f, BirdTurnDegrees, ReferenceFrames(fps));
        }
        CHECK(heading == doctest::Approx(62.f));
    }
}

TEST_CASE("Flock turns reach their target within one rendered frame at every FPS [gamelogic][boids]")
{
    constexpr std::array frameRates{25.0, 30.0, 60.0, 120.0, 144.0, 240.0, 1000.0};
    constexpr double expectedSeconds = 160.0 / (BirdTurnDegrees * ReferenceFps);
    constexpr double timeTolerance = 0.00001;
    for (const double fps : frameRates)
    {
        CAPTURE(fps);
        float heading = 10.f;
        int frames = 0;
        const int maximumFrames = static_cast<int>(std::ceil(fps));
        while (heading != 170.f && frames < maximumFrames)
        {
            heading = Engine::AI::BoidSteering::TurnTowards(heading, 170.f, BirdTurnDegrees, ReferenceFrames(fps));
            ++frames;
        }
        REQUIRE(heading == 170.f);
        const double seconds = frames / fps;
        CHECK(seconds >= expectedSeconds - timeTolerance);
        CHECK(seconds <= expectedSeconds + 1.0 / fps + timeTolerance);
    }
}

TEST_CASE("Flock turns wrap, stop at their target, and bound delayed steps [gamelogic][boids]")
{
    using Engine::AI::BoidSteering::TurnTowards;
    CHECK(TurnTowards(350.f, 10.f, BirdTurnDegrees, 0.5f) == doctest::Approx(356.5f));
    CHECK(TurnTowards(1.f, 359.f, BirdTurnDegrees, 0.5f) == 359.f);
    CHECK(TurnTowards(169.f, 170.f, BirdTurnDegrees, 1.f) == 170.f);
    CHECK(TurnTowards(10.f, 170.f, BirdTurnDegrees, 0.f) == 10.f);
    CHECK(TurnTowards(10.f, 170.f, BirdTurnDegrees, -1.f) == 10.f);
    CHECK(TurnTowards(10.f, 170.f, BirdTurnDegrees, 2.f) == 23.f);
}

TEST_CASE("Rate-gated butterflies keep their reference turning speed at every FPS [gamelogic][boids]")
{
    using Engine::AI::BoidSteering::SteerFrames;
    using Engine::AI::BoidSteering::TurnTowards;
    constexpr double durationSeconds = 400.0;
    constexpr double expectedDegreesPerSecond = BirdTurnDegrees * ReferenceFps / 4.0;
    for (const double fps : std::array{25.0, 60.0, 250.0, 1000.0})
    {
        CAPTURE(fps);
        Random::Seed(771);
        double turned = 0.0;
        bool wholeSteps = true;
        const int frames = static_cast<int>(durationSeconds * fps);
        for (int frame = 0; frame < frames; ++frame)
        {
            const float steerFrames = SteerFrames(true, ReferenceFrames(fps));
            wholeSteps = wholeSteps && (steerFrames == 0.f || steerFrames == 1.f);
            turned += TurnTowards(0.f, 90.f, BirdTurnDegrees, steerFrames);
        }
        CHECK(wholeSteps);
        CHECK(turned / durationSeconds == doctest::Approx(expectedDegreesPerSecond).epsilon(0.05));
    }
}

TEST_CASE("Ungated flock members steer every frame by its share of a reference frame [gamelogic][boids]")
{
    using Engine::AI::BoidSteering::SteerFrames;
    CHECK(SteerFrames(false, ReferenceFrames(250.0)) == ReferenceFrames(250.0));
    CHECK(SteerFrames(false, ReferenceFrames(25.0)) == 1.f);
}
