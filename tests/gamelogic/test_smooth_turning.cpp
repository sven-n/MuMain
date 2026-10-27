#include "stdafx.h"
#include "doctest.h"

#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Models/HeadTurn.h"

#include <array>
#include <cmath>

namespace
{
// The signed gap from `angle` to `target`, -180 to 180.
float Gap(float angle, float target)
{
    return FarAngle(angle, target, false);
}

struct TurnRun
{
    float seconds = -1.f; // how long it took to land; -1: it didn't within 5 seconds
    float peakSpeed = 0.f;
    float largestSpeedChange = 0.f;
    bool passedTarget = false;
};

// Turns from `from` to `to` frame by frame at `fps` with the walk turn's speeds.
TurnRun Turn(float from, float to, double fps)
{
    TurnRun run;
    const float seconds = static_cast<float>(1.0 / fps);
    float angle = from;
    float speed = 0.f;
    const float side = Gap(from, to) > 0.f ? 1.f : -1.f;
    for (int frame = 1; frame <= static_cast<int>(5.0 * fps); ++frame)
    {
        const float before = speed;
        angle = StepEasedTurn(angle, to, speed, seconds, WalkTurnDegreesPerSecond, WalkTurnAcceleration);
        run.peakSpeed = std::max(run.peakSpeed, std::fabs(speed));
        if (speed != 0.f)
        {
            run.largestSpeedChange = std::max(run.largestSpeedChange, std::fabs(speed - before));
        }
        if (Gap(angle, to) * side < 0.f)
        {
            run.passedTarget = true;
        }
        if (angle == to && speed == 0.f)
        {
            run.seconds = frame * seconds;
            break;
        }
    }
    return run;
}
} // namespace

TEST_CASE("An eased turn lands on its target, without passing it, in the same time at any frame rate [gamelogic][turning]")
{
    // From rest to rest over 180 degrees it speeds up for half the way and slows down for the other half (it never
    // reaches the top speed): 2 * sqrt(180 / acceleration) seconds.
    const float expected = 2.f * std::sqrt(180.f / WalkTurnAcceleration);
    for (const double fps : std::array{25.0, 30.0, 60.0, 144.0, 240.0, 1000.0})
    {
        CAPTURE(fps);
        const TurnRun run = Turn(0.f, 180.f, fps);
        CHECK_FALSE(run.passedTarget);
        CHECK(run.seconds == doctest::Approx(expected).epsilon(0.08));
    }
}

TEST_CASE("An eased turn speeds up gradually and never goes past its top speed [gamelogic][turning]")
{
    constexpr double Fps = 60.0;
    // A turn long enough to reach the top speed.
    const TurnRun run = Turn(0.f, 179.f, Fps);
    CHECK(run.peakSpeed <= WalkTurnDegreesPerSecond);
    CHECK(run.largestSpeedChange <= WalkTurnAcceleration / Fps + 0.01f);

    // The first frame from rest moves only a little: the speed starts at one frame's acceleration.
    float speed = 0.f;
    const float first =
        StepEasedTurn(0.f, 179.f, speed, 1.f / 60.f, WalkTurnDegreesPerSecond, WalkTurnAcceleration);
    CHECK(speed == doctest::Approx(WalkTurnAcceleration / 60.f));
    CHECK(first == doctest::Approx(WalkTurnAcceleration / 3600.f));
}

TEST_CASE("An eased turn takes the shortest way across zero [gamelogic][turning]")
{
    float angle = 350.f;
    float speed = 0.f;
    for (int frame = 0; frame < 300 && !(angle == 10.f && speed == 0.f); ++frame)
    {
        angle = StepEasedTurn(angle, 10.f, speed, 1.f / 60.f, WalkTurnDegreesPerSecond, WalkTurnAcceleration);
        CAPTURE(angle);
        CHECK((angle >= 350.f || angle <= 10.f));
    }
    CHECK(angle == 10.f);
}

TEST_CASE("A target that moves behind the turn slows the turn down and reverses it, without a jump [gamelogic][turning]")
{
    constexpr float Seconds = 1.f / 60.f;
    float angle = 0.f;
    float speed = 0.f;
    for (int frame = 0; frame < 10; ++frame)
    {
        angle = StepEasedTurn(angle, 90.f, speed, Seconds, WalkTurnDegreesPerSecond, WalkTurnAcceleration);
    }
    REQUIRE(speed > 0.f);
    // Now the other way: the speed changes by at most one frame's acceleration, and the turn still lands.
    for (int frame = 0; frame < 300 && !(angle == 270.f && speed == 0.f); ++frame)
    {
        const float before = speed;
        angle = StepEasedTurn(angle, 270.f, speed, Seconds, WalkTurnDegreesPerSecond, WalkTurnAcceleration);
        if (speed != 0.f)
        {
            CHECK(std::fabs(speed - before) <= WalkTurnAcceleration * Seconds + 0.01f);
        }
    }
    CHECK(angle == 270.f);
}

TEST_CASE("The head's turn reaches the shoulders, and a skeleton without them leaves it to the head [gamelogic][turning]")
{
    namespace HeadTurn = Render::Models::HeadTurn;
    CHECK(HeadTurn::SignedYaw(300.f) == doctest::Approx(-60.f));
    CHECK(HeadTurn::SignedYaw(60.f) == doctest::Approx(60.f));
    CHECK(HeadTurn::SignedYaw(0.f) == 0.f);

    for (const float yaw : {60.f, -60.f, 25.f})
    {
        CAPTURE(yaw);
        const HeadTurn::BoneTurns biped = HeadTurn::SplitTurn(yaw, true, true, true);
        // The head ends up turned as far as before; the neck and spine take the shoulders' share, on the same side.
        CHECK(biped.head + biped.neck + biped.spine1 + biped.spine == doctest::Approx(yaw));
        CHECK(biped.neck + biped.spine1 + biped.spine == doctest::Approx(HeadTurn::ShouldersDegrees(yaw)));
        CHECK(biped.neck * yaw > 0.f);

        // A chain that stops at the neck gives the spine's parts back to the head; no neck: the head turns alone.
        const HeadTurn::BoneTurns neckOnly = HeadTurn::SplitTurn(yaw, true, false, false);
        CHECK(neckOnly.spine1 == 0.f);
        CHECK(neckOnly.spine == 0.f);
        CHECK(neckOnly.head + neckOnly.neck == doctest::Approx(yaw));
        const HeadTurn::BoneTurns headOnly = HeadTurn::SplitTurn(yaw, false, true, true);
        CHECK(headOnly.head == doctest::Approx(yaw));
        CHECK(headOnly.neck == 0.f);
        CHECK(headOnly.spine1 == 0.f);
    }
}

TEST_CASE("A part turned about an upright keeps its height and circles the pivot [gamelogic][turning]")
{
    float matrix[3][4] = {{1.f, 0.f, 0.f, 0.f}, {0.f, 1.f, 0.f, 0.f}, {0.f, 0.f, 1.f, 5.f}};
    Render::Models::HeadTurn::TurnAboutUpright(matrix, static_cast<float>(Q_PI) / 2.f, 10.f, 0.f);
    // The origin, 10 from the pivot, goes a quarter turn around it; the up axis and the height don't change.
    CHECK(matrix[0][3] == doctest::Approx(10.f));
    CHECK(matrix[1][3] == doctest::Approx(-10.f));
    CHECK(matrix[2][3] == 5.f);
    CHECK(matrix[0][0] == doctest::Approx(0.f).epsilon(0.0001));
    CHECK(matrix[1][0] == doctest::Approx(1.f));
    CHECK(matrix[2][2] == 1.f);
}

TEST_CASE("A walking character travels toward its path while its facing turns gradually [gamelogic][turning]")
{
    static CHARACTER walker;
    walker.Initialize();
    // A walking speed of its own (a monster's, not the player's equipment-based one).
    walker.MoveSpeed = 10;
    const float savedFactor = FPS_ANIMATION_FACTOR;
    FPS_ANIMATION_FACTOR = 1.f;

    // A straight path north along x = 10, the character facing the other way.
    PATH_t& path = walker.Path;
    path.PathNum = 4;
    for (int i = 0; i < 4; ++i)
    {
        path.PathX[i] = 10;
        path.PathY[i] = static_cast<unsigned char>(10 + i);
    }
    path.CurrentPath = 0;
    path.CurrentPathFloat = 0;
    walker.Object.Position[0] = 10.5f * TERRAIN_SCALE;
    walker.Object.Position[1] = 10.5f * TERRAIN_SCALE;
    const float toPath = CreateAngle(0.f, 0.f, 0.f, 1.f);
    walker.Object.Angle[2] = toPath + 180.f;

    CHECK_FALSE(MovePath(&walker));
    CHECK(walker.MoveHeading == doctest::Approx(toPath).epsilon(0.001));
    // One frame from rest turns a little, not the whole way (it used to snap on a turn of 45 degrees or more).
    const float turned = FarAngle(toPath + 180.f, walker.Object.Angle[2]);
    CHECK(turned > 0.f);
    CHECK(turned < 10.f);

    // It moves along its heading, not its facing: toward the path, though it still faces mostly away.
    const float x = walker.Object.Position[0];
    const float y = walker.Object.Position[1];
    MoveCharacterPosition(&walker, walker.MoveHeading);
    REQUIRE((walker.Object.Position[0] != x || walker.Object.Position[1] != y));
    CHECK(walker.Object.Position[1] > y);
    CHECK(walker.Object.Position[0] == doctest::Approx(x).epsilon(0.001));

    FPS_ANIMATION_FACTOR = savedFactor;
}

TEST_CASE("A standing turn starts only for a real change and gives way to anything else that turns the character [gamelogic][turning]")
{
    static CHARACTER standing;
    standing.Initialize();
    standing.Object.Angle[2] = 90.f;

    StartStandingTurn(&standing, 90.2f);
    CHECK_FALSE(standing.TurnPending);

    StartStandingTurn(&standing, 270.f);
    CHECK(standing.TurnPending);
    CHECK(standing.MoveHeading == 270.f);

    // Something else set the facing (an attack toward its target): the turn ends where it is.
    standing.Object.Angle[2] = 45.f;
    FinishPendingTurn(&standing);
    CHECK_FALSE(standing.TurnPending);
    CHECK(standing.Object.Angle[2] == 45.f);

    // Walking ends it too: the walk turns the character itself.
    StartStandingTurn(&standing, 270.f);
    standing.Movement = true;
    FinishPendingTurn(&standing);
    CHECK_FALSE(standing.TurnPending);
    standing.Movement = false;
}
