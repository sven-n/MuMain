#include "stdafx.h"
#include "doctest.h"

#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/EnhancedMovement.h"
#include "Engine/Object/Movement/FlightFeel.h"
#include "Engine/Object/Movement/Greetings.h"
#include "Engine/Object/Movement/LookAt.h"
#include "Engine/Object/Movement/Momentum.h"
#include "Engine/Object/Movement/SlopeLean.h"
#include "Engine/Object/Movement/SnapTurn.h"
#include "Engine/Object/Movement/TurnLag.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Models/HeadTurn.h"
#include "Render/Terrain/ZzzLodTerrain.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <memory>
#include <vector>

extern EGameScene SceneFlag;

using namespace Engine::Characters;

namespace
{
// The frame rate and the world clock for a test, put back after it.
class Clock
{
public:
    explicit Clock(double fps)
        : m_factor(FPS_ANIMATION_FACTOR), m_worldTime(WorldTime), m_seconds(static_cast<float>(1.0 / fps))
    {
        FPS_ANIMATION_FACTOR = static_cast<float>(REFERENCE_FPS / fps);
        WorldTime = 0.0;
    }

    ~Clock()
    {
        FPS_ANIMATION_FACTOR = m_factor;
        WorldTime = m_worldTime;
    }

    Clock(const Clock&) = delete;
    Clock& operator=(const Clock&) = delete;

    float Seconds() const
    {
        return m_seconds;
    }

    // Frames in `seconds`.
    int Frames(float seconds) const
    {
        return static_cast<int>(std::lround(seconds / m_seconds));
    }

    void Tick() const
    {
        WorldTime += m_seconds * 1000.0;
    }

private:
    float m_factor;
    double m_worldTime;
    float m_seconds;
};

// Where the model's point `up` units above its feet is in the world.
void PointAbove(const OBJECT& o, float up, vec3_t out)
{
    float matrix[3][4];
    AngleMatrix(o.Angle, matrix);
    const vec3_t local = {0.f, 0.f, up};
    vec3_t rotated;
    VectorRotate(local, matrix, rotated);
    VectorAdd(o.Position, rotated, out);
}

CHARACTER& FreshPlayer()
{
    static CHARACTER player;
    player.Initialize();
    player.Helper.Type = 0;
    player.Object.Type = MODEL_PLAYER;
    player.Object.Kind = KIND_PLAYER;
    player.Object.CurrentAction = PLAYER_STOP_MALE;
    Vector(5000.f, 5000.f, 100.f, player.Object.Position);
    Vector(0.f, 0.f, 0.f, player.Object.Angle);
    return player;
}

// A spring let go from `start` at rest, frame by frame for `seconds`: where it ends and how far past 0 it went.
struct SpringRun
{
    float value = 0.f;
    float lowest = 0.f;
};

SpringRun RunSpring(float start, double fps, float seconds, float frequency, float damping)
{
    SpringRun run;
    float value = start;
    float velocity = 0.f;
    run.lowest = value;
    const auto step = static_cast<float>(1.0 / fps);
    for (int frame = 0; frame < static_cast<int>(std::lround(seconds * fps)); ++frame)
    {
        StepSpring(value, velocity, step, frequency, damping);
        run.lowest = std::min(run.lowest, value);
    }
    run.value = value;
    return run;
}
} // namespace

TEST_CASE("A spring settles at 0, overshooting a little only when damped below 1 [gamelogic][spring]")
{
    const SpringRun settled = RunSpring(10.f, 60.0, 3.f, 2.f, 0.6f);
    CHECK(std::fabs(settled.value) < 0.01f);

    const SpringRun underdamped = RunSpring(10.f, 60.0, 3.f, 2.f, 0.5f);
    CHECK(underdamped.lowest < -0.5f);
    CHECK(underdamped.lowest > -3.f);

    const SpringRun critical = RunSpring(10.f, 60.0, 3.f, 2.f, 1.f);
    CHECK(critical.lowest >= -0.001f);
    CHECK(std::fabs(critical.value) < 0.01f);
}

TEST_CASE("A spring swings about the same at 25 and at 240 frames a second [gamelogic][spring]")
{
    for (const float seconds : {0.12f, 0.2f, 0.6f})
    {
        CAPTURE(seconds);
        const SpringRun slow = RunSpring(10.f, 25.0, seconds, 2.5f, 0.6f);
        const SpringRun fast = RunSpring(10.f, 240.0, seconds, 2.5f, 0.6f);
        CHECK(slow.value == doctest::Approx(fast.value).epsilon(0.02).scale(10.f));
    }
}

TEST_CASE("A flying player banks into a turn by up to 8 degrees, pivoting about its waist [gamelogic][flight]")
{
    const Clock clock(25.0);
    CHARACTER& flyer = FreshPlayer();
    OBJECT& o = flyer.Object;
    o.CurrentAction = PLAYER_FLY;
    o.Angle[2] = 30.f;
    flyer.TurnSpeed = 300.f;
    vec3_t waist;
    PointAbove(o, 65.f, waist);

    for (int frame = 0; frame < clock.Frames(2.f); ++frame)
    {
        clock.Tick();
        UpdateFlightFeel(&flyer);
        CHECK(o.Angle[1] >= 0.f);
        CHECK(o.Angle[1] <= 8.001f);
        vec3_t now;
        PointAbove(o, 65.f, now);
        CHECK(now[0] == doctest::Approx(waist[0]).epsilon(0.00001));
        CHECK(now[1] == doctest::Approx(waist[1]).epsilon(0.00001));
    }
    CHECK(o.Angle[1] == doctest::Approx(8.f).epsilon(0.01));

    // Half the turning speed the other way: half the bank, the other way.
    flyer.TurnSpeed = -150.f;
    for (int frame = 0; frame < clock.Frames(2.f); ++frame)
    {
        clock.Tick();
        UpdateFlightFeel(&flyer);
    }
    CHECK(o.Angle[1] == doctest::Approx(-4.f).epsilon(0.01));
}

TEST_CASE("A flight bank fades in over the same time at 25 and at 250 frames a second [gamelogic][flight]")
{
    std::array<float, 2> banks{};
    const std::array fpses{25.0, 250.0};
    for (std::size_t i = 0; i < fpses.size(); ++i)
    {
        const Clock clock(fpses[i]);
        CHARACTER& flyer = FreshPlayer();
        flyer.Object.CurrentAction = PLAYER_FLY;
        flyer.TurnSpeed = 300.f;
        for (int frame = 0; frame < clock.Frames(0.2f); ++frame)
        {
            clock.Tick();
            UpdateFlightFeel(&flyer);
        }
        banks[i] = flyer.Object.Angle[1];
    }
    CHECK(banks[0] > 1.f);
    CHECK(banks[0] < 7.f);
    CHECK(banks[0] == doctest::Approx(banks[1]).epsilon(0.02));
}

TEST_CASE("A hovering player drifts up to 30 units above its height and never below it [gamelogic][flight]")
{
    const Clock clock(25.0);
    CHARACTER& flyer = FreshPlayer();
    OBJECT& o = flyer.Object;
    o.CurrentAction = PLAYER_STOP_FLY;
    const float ground = o.Position[2];

    float lowest = 0.f;
    float highest = 0.f;
    // Longer than one up-and-down cycle.
    for (int frame = 0; frame < clock.Frames(8.f); ++frame)
    {
        clock.Tick();
        UpdateFlightFeel(&flyer);
        lowest = std::min(lowest, o.Position[2] - ground);
        highest = std::max(highest, o.Position[2] - ground);
    }
    CHECK(lowest >= -0.001f);
    CHECK(highest <= 30.001f);
    CHECK(highest > 25.f);
}

TEST_CASE("A flying player that lands is back where it started, upright [gamelogic][flight]")
{
    const Clock clock(25.0);
    CHARACTER& flyer = FreshPlayer();
    OBJECT& o = flyer.Object;
    o.CurrentAction = PLAYER_FLY;
    o.Angle[2] = 120.f;
    flyer.TurnSpeed = 250.f;
    vec3_t start;
    VectorCopy(o.Position, start);

    for (int frame = 0; frame < clock.Frames(3.f); ++frame)
    {
        clock.Tick();
        UpdateFlightFeel(&flyer);
    }
    REQUIRE(o.Angle[1] > 5.f);
    REQUIRE(o.Position[2] > start[2]);

    o.CurrentAction = PLAYER_STOP_MALE;
    for (int frame = 0; frame < clock.Frames(3.f); ++frame)
    {
        clock.Tick();
        UpdateFlightFeel(&flyer);
    }
    CHECK(o.Position[0] == doctest::Approx(start[0]).epsilon(0.00001));
    CHECK(o.Position[1] == doctest::Approx(start[1]).epsilon(0.00001));
    CHECK(o.Position[2] == doctest::Approx(start[2]).epsilon(0.00001));
    CHECK(o.Angle[1] == doctest::Approx(0.f).epsilon(0.0001));
    CHECK(flyer.Motion.FlightBank == 0.f);
    CHECK(flyer.Motion.FlightHover == 0.f);
    CHECK(flyer.Motion.FlightHoverApplied == 0.f);
    CHECK(flyer.Motion.FlightPivotShift[0] == 0.f);
    CHECK(flyer.Motion.FlightPivotShift[1] == 0.f);
}

TEST_CASE(
    "The wings trail behind a sudden turn by at most 12 degrees and settle within about a second [gamelogic][turnlag]")
{
    for (const double fps : std::array{25.0, 60.0, 240.0})
    {
        CAPTURE(fps);
        const Clock clock(fps);
        CHARACTER& player = FreshPlayer();
        UpdateTurnLag(&player);
        CHECK(player.Motion.TurnLag == 0.f);

        // Turned clockwise at once: the wings stay behind, counterclockwise.
        player.Object.Angle[2] = 90.f;
        UpdateTurnLag(&player);
        CHECK(player.Motion.TurnLag < 0.f);
        CHECK(player.Motion.TurnLag >= -12.f);

        float lowest = player.Motion.TurnLag;
        float highest = player.Motion.TurnLag;
        float settledAt = -1.f;
        for (int frame = 2; frame <= clock.Frames(2.f); ++frame)
        {
            UpdateTurnLag(&player);
            lowest = std::min(lowest, player.Motion.TurnLag);
            highest = std::max(highest, player.Motion.TurnLag);
            if (player.Motion.TurnLag == 0.f && player.Motion.TurnLagSpeed == 0.f)
            {
                settledAt = static_cast<float>(frame) * clock.Seconds();
                break;
            }
        }
        CHECK(lowest >= -12.f);
        // They swing a little past before they settle.
        CHECK(highest > 0.f);
        CHECK(highest < 3.f);
        CHECK(settledAt > 0.3f);
        CHECK(settledAt < 1.2f);
    }
}

TEST_CASE("Without a turn the wings don't trail, and a non-player has no lag [gamelogic][turnlag]")
{
    const Clock clock(60.0);
    CHARACTER& player = FreshPlayer();
    player.Object.Angle[2] = 200.f;
    for (int frame = 0; frame < 30; ++frame)
    {
        UpdateTurnLag(&player);
        CHECK(player.Motion.TurnLag == 0.f);
        CHECK(player.Motion.TurnLagSpeed == 0.f);
    }

    player.Object.Angle[2] = 300.f;
    UpdateTurnLag(&player);
    REQUIRE(player.Motion.TurnLag != 0.f);
    player.Object.Type = MODEL_HORN_OF_UNIRIA;
    UpdateTurnLag(&player);
    CHECK(player.Motion.TurnLag == 0.f);
    CHECK(player.Motion.TurnLagSpeed == 0.f);
}

TEST_CASE("A sudden turn is drawn catching up from where the body was, at any frame rate [gamelogic][snapturn]")
{
    for (const double fps : std::array{25.0, 60.0, 144.0, 240.0, 1000.0})
    {
        CAPTURE(fps);
        const Clock clock(fps);
        CHARACTER& attacker = FreshPlayer();
        attacker.Object.Angle[2] = 10.f;
        UpdateSnapTurn(&attacker);
        CHECK(attacker.Motion.SnapTurnOffset == 0.f);

        // A jump of 170 degrees: the drawn body starts nearly where it was and eases onto the facing.
        attacker.Object.Angle[2] = 180.f;
        UpdateSnapTurn(&attacker);
        CHECK(attacker.Motion.SnapTurnOffset < -100.f);

        float landedAt = -1.f;
        float previous = attacker.Motion.SnapTurnOffset;
        for (int frame = 2; frame <= clock.Frames(1.f); ++frame)
        {
            UpdateSnapTurn(&attacker);
            CHECK(attacker.Motion.SnapTurnOffset <= 0.f);
            CHECK(attacker.Motion.SnapTurnOffset >= previous);
            previous = attacker.Motion.SnapTurnOffset;
            if (attacker.Motion.SnapTurnOffset == 0.f && attacker.Motion.SnapTurnSpeed == 0.f)
            {
                landedAt = static_cast<float>(frame) * clock.Seconds();
                break;
            }
        }
        // A half turn takes about 0.15 seconds; 170 degrees a little less.
        CHECK(landedAt > 0.1f);
        CHECK(landedAt < 0.2f);
    }
}

TEST_CASE("A turn of less than 30 degrees in a frame is not a sudden turn [gamelogic][snapturn]")
{
    const Clock clock(60.0);
    CHARACTER& walker = FreshPlayer();
    UpdateSnapTurn(&walker);
    for (const float facing : {20.f, 45.f, 74.f, 50.f})
    {
        walker.Object.Angle[2] = facing;
        UpdateSnapTurn(&walker);
        CHECK(walker.Motion.SnapTurnOffset == 0.f);
        CHECK(walker.Motion.SnapTurnSpeed == 0.f);
    }
    // The other way across 0.
    walker.Object.Angle[2] = 350.f;
    UpdateSnapTurn(&walker);
    CHECK(walker.Motion.SnapTurnOffset > 0.f);
}

TEST_CASE("A walker speeds up from rest over about 0.15 seconds [gamelogic][momentum]")
{
    for (const double fps : std::array{25.0, 60.0, 240.0})
    {
        CAPTURE(fps);
        const Clock clock(fps);
        CHARACTER& walker = FreshPlayer();
        float fullAt = -1.f;
        float previous = 0.f;
        for (int frame = 1; frame <= clock.Frames(1.f); ++frame)
        {
            clock.Tick();
            float heading = 45.f;
            const float pace = StepMomentum(&walker, heading);
            CHECK(heading == 45.f);
            CHECK(pace >= previous);
            previous = pace;
            if (frame == 1)
            {
                CHECK(pace < 0.5f);
            }
            if (pace == 1.f)
            {
                fullAt = static_cast<float>(frame) * clock.Seconds();
                break;
            }
        }
        CHECK(fullAt >= 0.14f);
        CHECK(fullAt <= 0.2f);
    }
}

TEST_CASE("A walker turning back slows to a stop the old way before it takes the new one [gamelogic][momentum]")
{
    for (const double fps : std::array{25.0, 60.0, 240.0})
    {
        CAPTURE(fps);
        const Clock clock(fps);
        CHARACTER& walker = FreshPlayer();
        for (int frame = 0; frame < clock.Frames(0.5f); ++frame)
        {
            clock.Tick();
            float heading = 0.f;
            StepMomentum(&walker, heading);
        }
        REQUIRE(walker.Motion.MomentumPace == 1.f);

        float turnedAt = -1.f;
        float previous = 1.f;
        for (int frame = 1; frame <= clock.Frames(1.f); ++frame)
        {
            clock.Tick();
            float heading = 180.f;
            const float pace = StepMomentum(&walker, heading);
            if (heading == 180.f)
            {
                turnedAt = static_cast<float>(frame) * clock.Seconds();
                break;
            }
            CHECK(heading == 0.f);
            CHECK(pace <= previous);
            previous = pace;
        }
        CHECK(previous == 0.f);
        CHECK(turnedAt >= 0.11f);
        CHECK(turnedAt <= 0.21f);
    }
}

TEST_CASE("A walker follows a gentler turn at once, and starts over after standing still [gamelogic][momentum]")
{
    const Clock clock(60.0);
    CHARACTER& walker = FreshPlayer();
    for (int frame = 0; frame < 30; ++frame)
    {
        clock.Tick();
        float heading = 0.f;
        StepMomentum(&walker, heading);
    }

    clock.Tick();
    float heading = 80.f;
    CHECK(StepMomentum(&walker, heading) == 1.f);
    CHECK(heading == 80.f);

    // Stood still for a moment: from rest again, the new way at once.
    WorldTime += 200.0;
    heading = 260.f;
    CHECK(StepMomentum(&walker, heading) < 0.5f);
    CHECK(heading == 260.f);
}

TEST_CASE("The back takes the spine's share of a head turn [gamelogic][turning]")
{
    namespace HeadTurn = Render::Models::HeadTurn;
    CHECK(HeadTurn::BackDegrees(0.f) == 0.f);
    for (const float yaw : {60.f, -60.f, 25.f})
    {
        CAPTURE(yaw);
        const HeadTurn::BoneTurns biped = HeadTurn::SplitTurn(yaw, true, true, true);
        CHECK(HeadTurn::BackDegrees(yaw) == doctest::Approx(biped.spine1 + biped.spine));
        CHECK(HeadTurn::BackDegrees(yaw) * yaw > 0.f);
        CHECK(std::fabs(HeadTurn::BackDegrees(yaw)) < std::fabs(HeadTurn::ShouldersDegrees(yaw)));
    }
}

TEST_CASE("A head turns toward its target within its limit, and up or down toward it [gamelogic][lookat]")
{
    OBJECT looker;
    Vector(1000.f, 1000.f, 0.f, looker.Position);
    looker.Angle[2] = 0.f;
    static CHARACTER target;
    target.Initialize();

    // 30 degrees clockwise of the facing (it faces -Y): a yaw of -30, kept as 330.
    const float radians = 30.f * static_cast<float>(Q_PI) / 180.f;
    Vector(1000.f + 200.f * std::sin(radians), 1000.f - 200.f * std::cos(radians), 50.f, target.Object.Position);
    LookTowards(&looker, &target, 60.f);
    CHECK(looker.HeadTargetAngle[0] == doctest::Approx(330.f));
    CHECK(looker.HeadTargetAngle[1] == doctest::Approx(0.f));

    // Off to the side (+X, 90 degrees) and up: held at the limit.
    Vector(1300.f, 1000.f, 150.f, target.Object.Position);
    LookTowards(&looker, &target, 60.f);
    CHECK(looker.HeadTargetAngle[0] == doctest::Approx(300.f));
    CHECK(looker.HeadTargetAngle[1] == doctest::Approx(20.f));

    // The other side, below: a positive yaw, and a pitch kept as 0 to 360.
    Vector(700.f, 1000.f, 0.f, target.Object.Position);
    LookTowards(&looker, &target, 60.f);
    CHECK(looker.HeadTargetAngle[0] == doctest::Approx(60.f));
    CHECK(looker.HeadTargetAngle[1] == doctest::Approx(350.f));
}

namespace
{
// The client's character slots, the hero and the wall cells a look-at test uses, put back after it.
class LookWorld
{
public:
    LookWorld()
        : m_characters(new CHARACTER[MAX_CHARACTERS_CLIENT + 1]), m_previous(CharactersClient), m_hero(Hero),
          m_factor(FPS_ANIMATION_FACTOR)
    {
        CharactersClient = m_characters.get();
        Hero = nullptr;
        FPS_ANIMATION_FACTOR = 1.f;
    }

    ~LookWorld()
    {
        for (const auto& [index, wall] : m_walls)
        {
            TerrainWall[index] = wall;
        }
        CharactersClient = m_previous;
        Hero = m_hero;
        FPS_ANIMATION_FACTOR = m_factor;
    }

    LookWorld(const LookWorld&) = delete;
    LookWorld& operator=(const LookWorld&) = delete;

    // Slot `index` alive at tile (x, y), facing -Y.
    CHARACTER& Place(int index, BYTE kind, int x, int y)
    {
        CHARACTER& c = CharactersClient[index];
        c.Object.Live = true;
        c.Object.Kind = kind;
        c.Object.Type = kind == KIND_PLAYER ? MODEL_PLAYER : 0;
        c.Key = static_cast<short>(index);
        c.TargetCharacter = -1;
        c.PositionX = x;
        c.PositionY = y;
        Vector((static_cast<float>(x) + 0.5f) * TERRAIN_SCALE, (static_cast<float>(y) + 0.5f) * TERRAIN_SCALE, 0.f,
               c.Object.Position);
        c.Object.Angle[2] = 0.f;
        return c;
    }

    void Wall(int x, int y)
    {
        const int index = TERRAIN_INDEX(x, y);
        m_walls.emplace_back(index, TerrainWall[index]);
        TerrainWall[index] |= TW_NOMOVE;
    }

private:
    std::unique_ptr<CHARACTER[]> m_characters;
    CHARACTER* m_previous;
    CHARACTER* m_hero;
    float m_factor;
    std::vector<std::pair<int, WORD>> m_walls;
};

// A player's look, picked again now rather than at its next look around.
const CHARACTER* LookNow(CHARACTER& c)
{
    c.Motion.LookScanSeconds = 0.f;
    return UpdateLookAt(&c);
}
} // namespace

TEST_CASE("A player in a combat zone looks at the nearest monster in front of it [gamelogic][lookat]")
{
    LookWorld world;
    CHARACTER& player = world.Place(0, KIND_PLAYER, 100, 100);
    world.Place(1, KIND_MONSTER, 100, 96);
    const CHARACTER& nearest = world.Place(2, KIND_MONSTER, 101, 97);
    // Nearer, but behind it.
    world.Place(3, KIND_MONSTER, 100, 102);
    // An NPC and a player near it count for less than an enemy.
    world.Place(4, KIND_NPC, 99, 99);
    world.Place(5, KIND_PLAYER, 101, 99);

    CHECK(LookNow(player) == &nearest);
    CHECK(player.Motion.LookTarget == 2);
}

TEST_CASE("A player doesn't look at a monster behind it or behind a wall [gamelogic][lookat]")
{
    LookWorld world;
    CHARACTER& player = world.Place(0, KIND_PLAYER, 100, 100);
    world.Place(1, KIND_MONSTER, 100, 103);
    world.Place(2, KIND_MONSTER, 97, 101);
    CHECK(LookNow(player) == nullptr);
    CHECK(player.Motion.LookTarget == -1);

    // In front, but a wall tile lies on the line between them.
    world.Place(3, KIND_MONSTER, 100, 97);
    REQUIRE(LookNow(player) == &CharactersClient[3]);
    world.Wall(100, 98);
    CHECK(LookNow(player) == nullptr);
}

TEST_CASE("A player in a safe zone looks at an NPC rather than a monster [gamelogic][lookat]")
{
    LookWorld world;
    CHARACTER& player = world.Place(0, KIND_PLAYER, 100, 100);
    player.SafeZone = true;
    world.Place(1, KIND_MONSTER, 100, 98);
    const CHARACTER& npc = world.Place(2, KIND_NPC, 101, 97);
    world.Place(3, KIND_PLAYER, 99, 99);
    CHECK(LookNow(player) == &npc);
}

TEST_CASE("A player with nothing else around looks at another player passing by [gamelogic][lookat]")
{
    LookWorld world;
    CHARACTER& player = world.Place(0, KIND_PLAYER, 100, 100);
    // Too far for a passer-by.
    world.Place(1, KIND_PLAYER, 100, 94);
    CHECK(LookNow(player) == nullptr);

    const CHARACTER& passing = world.Place(2, KIND_PLAYER, 102, 97);
    CHECK(LookNow(player) == &passing);
}

TEST_CASE("A player keeps looking at its pick while it stays in sight [gamelogic][lookat]")
{
    LookWorld world;
    CHARACTER& player = world.Place(0, KIND_PLAYER, 100, 100);
    CHARACTER& first = world.Place(1, KIND_MONSTER, 100, 96);
    REQUIRE(LookNow(player) == &first);

    // A nearer one comes along: it keeps the first, between looks around and at the next one.
    CHARACTER& second = world.Place(2, KIND_MONSTER, 99, 98);
    CHECK(UpdateLookAt(&player) == &first);
    CHECK(LookNow(player) == &first);

    // The first goes behind a wall: it lets go at once, and picks the other at its next look around.
    world.Wall(100, 97);
    CHECK(UpdateLookAt(&player) == nullptr);
    CHECK(LookNow(player) == &second);
}

TEST_CASE("A monster looks at its target in sight, and an NPC at the hero passing by [gamelogic][lookat]")
{
    LookWorld world;
    CHARACTER& hero = world.Place(0, KIND_PLAYER, 100, 100);
    Hero = &hero;

    CHARACTER& monster = world.Place(1, KIND_MONSTER, 100, 106);
    monster.TargetCharacter = 0;
    CHECK(UpdateLookAt(&monster) == &hero);
    CHECK(monster.Motion.LookTarget == 0);
    // Facing away from its target.
    monster.Object.Angle[2] = 180.f;
    CHECK(UpdateLookAt(&monster) == nullptr);

    CHARACTER& npc = world.Place(2, KIND_NPC, 101, 103);
    CHECK(UpdateLookAt(&npc) == &hero);
    hero.PositionY = 90;
    hero.Object.Position[1] = 90.5f * TERRAIN_SCALE;
    CHECK(UpdateLookAt(&npc) == nullptr);
    CHECK(npc.Motion.LookTarget == -1);
}

namespace
{
// A slope of the terrain around tile (128, 128), rising toward -Y by `rise` per tile; the heights and the scene put
// back after the test.
class Slope
{
public:
    static constexpr int From = 100;
    static constexpr int To = 156;

    explicit Slope(float rise) : m_scene(SceneFlag), m_factor(FPS_ANIMATION_FACTOR)
    {
        SceneFlag = MAIN_SCENE;
        FPS_ANIMATION_FACTOR = 1.f;
        for (int y = From; y <= To; ++y)
        {
            for (int x = From; x <= To; ++x)
            {
                const int index = TERRAIN_INDEX(x, y);
                m_saved.emplace_back(BackTerrainHeight[index], TerrainWall[index]);
                BackTerrainHeight[index] = static_cast<float>(To - y) * rise;
                TerrainWall[index] = 0;
            }
        }
        MarkTerrainHeightsChanged();
    }

    ~Slope()
    {
        std::size_t i = 0;
        for (int y = From; y <= To; ++y)
        {
            for (int x = From; x <= To; ++x)
            {
                const int index = TERRAIN_INDEX(x, y);
                BackTerrainHeight[index] = m_saved[i].first;
                TerrainWall[index] = m_saved[i].second;
                ++i;
            }
        }
        MarkTerrainHeightsChanged();
        SceneFlag = m_scene;
        FPS_ANIMATION_FACTOR = m_factor;
    }

    Slope(const Slope&) = delete;
    Slope& operator=(const Slope&) = delete;

private:
    EGameScene m_scene;
    float m_factor;
    std::vector<std::pair<float, WORD>> m_saved;
};

// The slope's angle in degrees for a rise of `rise` per tile.
float SlopeDegrees(float rise)
{
    return std::atan(rise / TERRAIN_SCALE) * 180.f / static_cast<float>(Q_PI);
}

CHARACTER& PlayerOnSlope()
{
    CHARACTER& c = FreshPlayer();
    Vector(128.5f * TERRAIN_SCALE, 128.5f * TERRAIN_SCALE, 0.f, c.Object.Position);
    c.Motion.SlopePitch = 0.f;
    return c;
}

void LeanFor(CHARACTER& c, float seconds)
{
    for (int frame = 0; frame < static_cast<int>(seconds * REFERENCE_FPS); ++frame)
    {
        UpdateSlopeLean(&c);
    }
}
} // namespace

TEST_CASE("A player walking uphill leans forward by half the slope, at most 10 degrees, and standing doesn't "
          "[gamelogic][slope]")
{
    {
        const Slope slope(10.f);
        CHARACTER& walker = PlayerOnSlope();
        LeanFor(walker, 1.f);
        CHECK(walker.Object.Angle[0] == 0.f);

        walker.Movement = true;
        walker.Object.CurrentAction = PLAYER_WALK_MALE;
        LeanFor(walker, 2.f);
        CHECK(walker.Object.Angle[0] == doctest::Approx(SlopeDegrees(10.f) * 0.5f).epsilon(0.01));
        CHECK(walker.Motion.SlopePitch == walker.Object.Angle[0]);

        // Downhill: leaning back.
        walker.Object.Angle[2] = 180.f;
        LeanFor(walker, 2.f);
        CHECK(walker.Object.Angle[0] == doctest::Approx(-SlopeDegrees(10.f) * 0.5f).epsilon(0.01));

        // Stopped: upright again.
        walker.Movement = false;
        walker.Object.CurrentAction = PLAYER_STOP_MALE;
        LeanFor(walker, 2.f);
        CHECK(walker.Object.Angle[0] == doctest::Approx(0.f).epsilon(0.0001));
        CHECK(walker.Motion.SlopePitch == 0.f);
    }
    {
        const Slope steep(100.f);
        CHARACTER& walker = PlayerOnSlope();
        walker.Movement = true;
        walker.Object.CurrentAction = PLAYER_WALK_MALE;
        LeanFor(walker, 2.f);
        CHECK(walker.Object.Angle[0] == doctest::Approx(10.f));
    }
}

TEST_CASE(
    "A mount follows the slope nose up by 1.5 times it, at most 25 degrees, but not in a safe zone [gamelogic][slope]")
{
    {
        const Slope slope(10.f);
        CHARACTER& rider = PlayerOnSlope();
        rider.Helper.Type = MODEL_HORN_OF_UNIRIA;
        LeanFor(rider, 2.f);
        CHECK(rider.Object.Angle[0] == doctest::Approx(-SlopeDegrees(10.f) * 1.5f).epsilon(0.01));

        // In a safe zone the mount is put away: standing, upright.
        rider.SafeZone = true;
        LeanFor(rider, 2.f);
        CHECK(rider.Object.Angle[0] == doctest::Approx(0.f).epsilon(0.0001));
    }
    {
        const Slope steep(100.f);
        CHARACTER& rider = PlayerOnSlope();
        rider.Helper.Type = MODEL_HORN_OF_UNIRIA;
        LeanFor(rider, 2.f);
        CHECK(rider.Object.Angle[0] == doctest::Approx(-25.f));
    }
}

namespace
{
template <class Work> double NanosecondsPerCall(int calls, Work work)
{
    double best = 1e30;
    for (int run = 0; run < 5; ++run)
    {
        const auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < calls; ++i)
        {
            work(i);
        }
        const auto elapsed = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - start).count();
        best = std::min(best, elapsed / calls);
    }
    return best;
}
} // namespace

// Not run with the tests (ctest -E benchmark): what the realism steps' per-frame updates cost a character, for the PR.
namespace
{
// Enhanced Movement switched off for a test, put back after it.
class EnhancedMovementOff
{
public:
    EnhancedMovementOff() : m_was(Engine::Characters::EnhancedMovementEnabled())
    {
        Engine::Characters::SetEnhancedMovementEnabled(false);
    }

    ~EnhancedMovementOff()
    {
        Engine::Characters::SetEnhancedMovementEnabled(m_was);
    }

    EnhancedMovementOff(const EnhancedMovementOff&) = delete;
    EnhancedMovementOff& operator=(const EnhancedMovementOff&) = delete;

private:
    bool m_was;
};
} // namespace

TEST_CASE("Without Enhanced Movement a flying player neither banks nor drifts [gamelogic][flight][option]")
{
    const Clock clock(60.0);
    const EnhancedMovementOff off;
    CHARACTER& flyer = FreshPlayer();
    OBJECT& o = flyer.Object;
    o.CurrentAction = PLAYER_FLY;
    flyer.TurnSpeed = 300.f;
    for (int frame = 0; frame < clock.Frames(2.f); ++frame)
    {
        clock.Tick();
        UpdateFlightFeel(&flyer);
    }
    CHECK(o.Angle[1] == 0.f);
    CHECK(o.Position[0] == 5000.f);
    CHECK(o.Position[1] == 5000.f);
    CHECK(o.Position[2] == 100.f);
    CHECK(flyer.Motion.FlightHoverApplied == 0.f);
}

TEST_CASE("Switching Enhanced Movement off mid-flight eases the bank and the drift out [gamelogic][flight][option]")
{
    const Clock clock(60.0);
    CHARACTER& flyer = FreshPlayer();
    OBJECT& o = flyer.Object;
    o.CurrentAction = PLAYER_FLY;
    flyer.TurnSpeed = 300.f;
    for (int frame = 0; frame < clock.Frames(2.f); ++frame)
    {
        clock.Tick();
        UpdateFlightFeel(&flyer);
    }
    REQUIRE(o.Angle[1] > 1.f);

    const EnhancedMovementOff off;
    for (int frame = 0; frame < clock.Frames(3.f); ++frame)
    {
        clock.Tick();
        UpdateFlightFeel(&flyer);
    }
    CHECK(o.Angle[1] == doctest::Approx(0.f).scale(1.f));
    CHECK(flyer.Motion.FlightBank == 0.f);
    CHECK(flyer.Motion.FlightHoverApplied == 0.f);
    CHECK(flyer.Motion.FlightPivotShift[0] == 0.f);
    CHECK(flyer.Motion.FlightPivotShift[1] == 0.f);
    CHECK(o.Position[2] == doctest::Approx(100.f));
}

TEST_CASE(
    "Without Enhanced Movement a walker keeps its full pace and the way it's headed [gamelogic][momentum][option]")
{
    const Clock clock(60.0);
    const EnhancedMovementOff off;
    CHARACTER& walker = FreshPlayer();
    for (const float wanted : {0.f, 0.f, 180.f, 90.f})
    {
        clock.Tick();
        float heading = wanted;
        CHECK(StepMomentum(&walker, heading) == 1.f);
        CHECK(heading == wanted);
    }
}

TEST_CASE("Without Enhanced Movement a sudden turn is drawn at once, and the wings don't trail "
          "[gamelogic][snapturn][turnlag][option]")
{
    const Clock clock(60.0);
    const EnhancedMovementOff off;
    CHARACTER& attacker = FreshPlayer();
    UpdateSnapTurn(&attacker);
    UpdateTurnLag(&attacker);
    attacker.Object.Angle[2] = 170.f;
    for (int frame = 0; frame < 3; ++frame)
    {
        UpdateSnapTurn(&attacker);
        UpdateTurnLag(&attacker);
        CHECK(attacker.Motion.SnapTurnOffset == 0.f);
        CHECK(attacker.Motion.SnapTurnSpeed == 0.f);
        CHECK(attacker.Motion.TurnLag == 0.f);
    }
}

TEST_CASE("The realism steps' per-frame cost for a character (benchmark) [gamelogic][benchmark]")
{
    constexpr int Calls = 200000;
    const Clock clock(60.0);
    CHARACTER& c = FreshPlayer();
    c.Object.CurrentAction = PLAYER_FLY;
    c.TurnSpeed = 200.f;
    const double flight = NanosecondsPerCall(Calls,
                                             [&](int)
                                             {
                                                 clock.Tick();
                                                 UpdateFlightFeel(&c);
                                             });
    c.Object.CurrentAction = PLAYER_STOP_MALE;
    const double turnLag = NanosecondsPerCall(Calls,
                                              [&](int i)
                                              {
                                                  c.Object.Angle[2] = static_cast<float>(i % 360);
                                                  UpdateTurnLag(&c);
                                              });
    const double snapTurn = NanosecondsPerCall(Calls,
                                               [&](int i)
                                               {
                                                   c.Object.Angle[2] = static_cast<float>((i * 7) % 360);
                                                   UpdateSnapTurn(&c);
                                               });
    const double momentum = NanosecondsPerCall(Calls,
                                               [&](int i)
                                               {
                                                   clock.Tick();
                                                   float heading = static_cast<float>((i / 50) % 2 * 180);
                                                   StepMomentum(&c, heading);
                                               });
    double slopeLean = 0.;
    {
        const Slope slope(10.f);
        CHARACTER& walker = PlayerOnSlope();
        walker.Movement = true;
        slopeLean = NanosecondsPerCall(Calls, [&](int) { UpdateSlopeLean(&walker); });
    }
    // Looking around in a full scene: 400 characters, every one a monster in front.
    double lookAround = 0.;
    {
        LookWorld world;
        CHARACTER& looker = world.Place(0, KIND_PLAYER, 100, 100);
        for (int i = 1; i < MAX_CHARACTERS_CLIENT; ++i)
        {
            world.Place(i, KIND_MONSTER, 90 + i % 20, 80 + i / 20 % 18);
        }
        // A full look around each time: no pick kept from the last one.
        lookAround = NanosecondsPerCall(2000,
                                        [&](int)
                                        {
                                            looker.Motion.LookTarget = -1;
                                            LookNow(looker);
                                        });
        CHECK(looker.Motion.LookTarget >= 0);
    }
    MESSAGE("per frame, ns per character: flight " << flight << ", turn lag " << turnLag << ", sudden turn " << snapTurn
                                                   << ", momentum " << momentum << ", slope lean " << slopeLean
                                                   << "; a look around in a 400-character scene " << lookAround
                                                   << " (each player does one every 0.25 s or so)");
    CHECK(flight >= 0.);
}

TEST_CASE("A character that greeted may greet again once its cooldown has run out [gamelogic][nod]")
{
    const Clock clock(60.0);
    CHARACTER& c = FreshPlayer();
    c.Object.Kind = KIND_NPC;
    c.Motion.NodCooldown = 10.f;
    for (int frame = 0; frame < clock.Frames(5.f); ++frame)
    {
        UpdateNod(&c);
    }
    CHECK(c.Motion.NodCooldown == doctest::Approx(5.f).epsilon(0.01));
    for (int frame = 0; frame < clock.Frames(6.f); ++frame)
    {
        UpdateNod(&c);
    }
    CHECK(c.Motion.NodCooldown == 0.f);
}
