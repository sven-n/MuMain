// Tests of the turning of the monsters in the directions (cinematics), e.g. the intro of the crywolf event.
//
// The monsters turn towards their target tile and then walk along their heading. The heading has to point
// exactly at the target at any frame rate, otherwise a monster can pass beside its tile and the direction,
// which waits for all monsters to arrive, hangs.

#include "doctest.h"

#include "GameLogic/Events/Cinematic/DirectionTurning.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr float kTileSize = 100.0f;
constexpr double kReferenceFps = 25.0;

float TileCenter(int tile)
{
    return (static_cast<float>(tile) + 0.5f) * kTileSize;
}

// Turns like a monster does it in each frame at the frame rate, and returns the number of frames.
int TurnUntilAligned(float& heading, float targetAngle, double fps)
{
    const auto animationFactor = static_cast<float>(std::clamp(kReferenceFps / fps, 0.0, 1.0));
    const float step = GameLogic::Cinematic::kMonsterTurnStepDegrees * animationFactor;
    for (int frame = 1; frame <= 10000; ++frame)
    {
        if (GameLogic::Cinematic::TurnTowards(heading, targetAngle, step))
        {
            return frame;
        }
    }

    return -1;
}
} // namespace

TEST_CASE("The angle to a tile is the one the monster walks along [gamelogic][cinematic]")
{
    SUBCASE("to the east")
    {
        CHECK(GameLogic::Cinematic::CalculateAngleToTile(TileCenter(100), TileCenter(100), 101, 100, kTileSize) ==
              doctest::Approx(90.0f));
    }

    SUBCASE("a balram of the crywolf intro, from 114,229 to 116,219")
    {
        CHECK(GameLogic::Cinematic::CalculateAngleToTile(TileCenter(114), TileCenter(229), 116, 219, kTileSize) ==
              doctest::Approx(11.31f).epsilon(0.001));
    }
}

TEST_CASE("A turning monster ends exactly at its target angle at any frame rate [gamelogic][cinematic]")
{
    const float targetAngle =
        GameLogic::Cinematic::CalculateAngleToTile(TileCenter(114), TileCenter(229), 116, 219, kTileSize);
    for (double fps = 25.0; fps <= 1000.0; fps += 0.25)
    {
        float heading = 0.0f;
        CAPTURE(fps);
        REQUIRE(TurnUntilAligned(heading, targetAngle, fps) > 0);
        CHECK(heading == targetAngle);
    }
}

TEST_CASE("A turning monster takes the short way over 0 degrees [gamelogic][cinematic]")
{
    SUBCASE("from 350 to 10 degrees")
    {
        float heading = 350.0f;
        const int frames = TurnUntilAligned(heading, 10.0f, kReferenceFps);
        CHECK(frames > 0);
        CHECK(frames <= 7);
        CHECK(heading == 10.0f);
    }

    SUBCASE("from 1 to 359 degrees")
    {
        float heading = 1.0f;
        CHECK(TurnUntilAligned(heading, 359.0f, kReferenceFps) == 1);
        CHECK(heading == 359.0f);
    }
}
