#include <doctest.h>

#include "Core/Platform/WinCompat.h"
#include "Core/Globals/_define.h"
#include "Render/Lighting/DynamicLightHistory.h"
#include "Render/Lighting/DynamicLightList.h"
#include "Render/Terrain/TerrainLightGrid.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
#include <span>

using doctest::Approx;
using Render::Lighting::DynamicLightHistory;
using Render::Lighting::LightCall;
using Render::Lighting::LightHistorySettings;
using Render::Lighting::LightSource;

namespace
{
constexpr float Frame = 1.f / 60.f;

LightCall Call(float x, float y, float brightness, int range, LightSource source)
{
    LightCall call;
    call.x = x;
    call.y = y;
    call.color[0] = brightness;
    call.color[1] = brightness * 0.6f;
    call.color[2] = brightness * 0.2f;
    call.range = range;
    call.source = source;
    return call;
}

Render::Terrain::LightGrid::FrameLightTarget Target(float (*grid)[3], Render::Lighting::DynamicLightList& list,
                                                    DynamicLightHistory& history, int world = 0)
{
    Render::Terrain::LightGrid::FrameLightTarget target;
    target.grid = grid;
    target.frameLights = &list;
    target.history = &history;
    target.world = world;
    target.height = +[](float, float) { return 0.f; };
    return target;
}

// One game tick that records `calls`, then the frame's lights.
std::span<const LightCall> Tick(DynamicLightHistory& history, std::initializer_list<LightCall> calls,
                                const LightHistorySettings& settings, float seconds = Frame)
{
    history.BeginTick();
    for (const LightCall& call : calls)
    {
        history.Record(call);
    }
    return history.Take(seconds, settings);
}
} // namespace

TEST_CASE("without smoothing or fade every frame shows exactly its own calls [render][lighthistory]")
{
    DynamicLightHistory history;
    const LightHistorySettings classic;
    auto lights = Tick(history, {Call(1000.f, 1000.f, 0.3f, 3, LightSource::Map)}, classic);
    REQUIRE(lights.size() == 1);
    CHECK(lights[0].color[0] == Approx(0.3f));
    CHECK(lights[0].range == 3);

    lights = Tick(history, {Call(1004.f, 997.f, 0.6f, 3, LightSource::Map)}, classic);
    REQUIRE(lights.size() == 1);
    CHECK(lights[0].color[0] == Approx(0.6f));

    CHECK(Tick(history, {}, classic).empty());
    CHECK(history.FollowedCount(LightSource::Map) == 0);
}

TEST_CASE("a smoothed map light breathes around its mean instead of jumping [render][lighthistory]")
{
    DynamicLightHistory history;
    LightHistorySettings settings;
    settings.mapSmoothingSeconds = 0.4f;
    // A candle alternating between its extremes every frame, its flame jittering by a few units.
    float lowest = 1.f;
    float highest = 0.f;
    for (int frame = 0; frame < 240; ++frame)
    {
        const float brightness = frame % 2 == 0 ? 0.3f : 0.6f;
        const float jitter = frame % 3 == 0 ? 8.f : -8.f;
        const auto lights =
            Tick(history, {Call(500.f + jitter, 500.f - jitter, brightness, 3, LightSource::Map)}, settings);
        REQUIRE(lights.size() == 1);
        if (frame >= 120)
        {
            lowest = (std::min)(lowest, lights[0].color[0]);
            highest = (std::max)(highest, lights[0].color[0]);
        }
    }
    CHECK(lowest > 0.43f);
    CHECK(highest < 0.47f);
    CHECK(history.FollowedCount(LightSource::Map) == 1);
}

TEST_CASE("an effect light is stronger, reaches further and fades out after its effect [render][lighthistory]")
{
    DynamicLightHistory history;
    LightHistorySettings settings;
    settings.effectGain = 1.5f;
    settings.effectRangeScale = 2.f;
    settings.effectFadeSeconds = 0.3f;
    auto lights = Tick(history, {Call(200.f, 200.f, 0.8f, 2, LightSource::Effect)}, settings);
    REQUIRE(lights.size() == 1);
    CHECK(lights[0].color[0] == Approx(1.2f));
    CHECK(lights[0].range == 4);

    lights = Tick(history, {}, settings, 0.3f);
    REQUIRE(lights.size() == 1);
    CHECK(lights[0].color[0] == Approx(1.2f * 0.3679f).epsilon(0.01));

    for (int frame = 0; frame < 120 && !lights.empty(); ++frame)
    {
        lights = Tick(history, {}, settings);
    }
    CHECK(lights.empty());
    CHECK(history.FollowedCount(LightSource::Effect) == 0);
}

TEST_CASE("the range ceiling limits how far the range factor widens effect lights [render][lighthistory]")
{
    LightHistorySettings settings;
    settings.effectRangeScale = 3.f;
    settings.effectRangeMax = 8.f;
    const auto shownRange = [&](int range)
    {
        DynamicLightHistory history;
        auto lights = Tick(history, {Call(200.f, 200.f, 0.5f, range, LightSource::Effect)}, settings);
        REQUIRE(lights.size() == 1);
        return lights[0].range;
    };
    CHECK(shownRange(1) == 3);
    CHECK(shownRange(2) == 6);   // Aqua Beam's lights
    CHECK(shownRange(4) == 8);   // Inferno's explosions: 12 without the ceiling
    CHECK(shownRange(10) == 10); // already wider than the ceiling: its own range, not narrower

    settings.effectRangeMax = 0.f; // no ceiling
    CHECK(shownRange(4) == 12);

    settings.effectRangeScale = 0.5f; // a narrowing factor is never widened back
    settings.effectRangeMax = 2.f;
    CHECK(shownRange(6) == 3);
    CHECK(shownRange(2) == 1);

    settings.effectRangeScale = 3.f; // map and character lights keep their range
    settings.effectRangeMax = 8.f;
    DynamicLightHistory history;
    auto lights = Tick(history, {Call(200.f, 200.f, 0.5f, 4, LightSource::Map)}, settings);
    REQUIRE(lights.size() == 1);
    CHECK(lights[0].range == 4);
}

TEST_CASE("a fading effect light rises again at once when its effect adds it again [render][lighthistory]")
{
    DynamicLightHistory history;
    LightHistorySettings settings;
    settings.effectFadeSeconds = 0.5f;
    (void)Tick(history, {Call(200.f, 200.f, 0.4f, 2, LightSource::Effect)}, settings);
    (void)Tick(history, {}, settings, 0.5f);
    const auto lights = Tick(history, {Call(210.f, 200.f, 0.9f, 2, LightSource::Effect)}, settings);
    REQUIRE(lights.size() == 1);
    CHECK(lights[0].color[0] == Approx(0.9f));
}

TEST_CASE("lights recorded while a frame is drawn belong to the next frame [render][lighthistory]")
{
    DynamicLightHistory history;
    const LightHistorySettings classic;
    (void)Tick(history, {}, classic);
    // Drawing the frame: the beam adds its light after the frame's lights were taken.
    history.Record(Call(300.f, 300.f, 0.5f, 2, LightSource::Effect));
    // A tick that is never drawn, then the drawn one.
    history.BeginTick();
    history.Record(Call(900.f, 900.f, 0.7f, 3, LightSource::Map));
    history.BeginTick();
    history.Record(Call(600.f, 600.f, 0.2f, 3, LightSource::Map));
    const auto lights = history.Take(Frame, classic);
    REQUIRE(lights.size() == 2);
    CHECK(history.FollowedCount(LightSource::Effect) == 1);
    CHECK(history.FollowedCount(LightSource::Map) == 1);
    for (const LightCall& light : lights)
    {
        CHECK(light.x != Approx(900.f));
    }
}

namespace
{
bool Same(const LightCall& a, const LightCall& b)
{
    return a.x == Approx(b.x) && a.y == Approx(b.y) && a.range == b.range && a.source == b.source &&
           a.color[0] == Approx(b.color[0]) && a.color[1] == Approx(b.color[1]) && a.color[2] == Approx(b.color[2]);
}

// The lights equal the calls one to one, in call order.
void CheckSameLights(std::span<const LightCall> lights, std::initializer_list<LightCall> calls)
{
    REQUIRE(lights.size() == calls.size());
    std::size_t i = 0;
    for (const LightCall& call : calls)
    {
        CHECK(Same(lights[i++], call));
    }
}
} // namespace

TEST_CASE("at built-in values every call is its own light, frame after frame [render][lighthistory]")
{
    DynamicLightHistory history;
    const LightHistorySettings classic;
    // Two flames of one fire at almost the same place with different ranges, plus an effect.
    const LightCall a = Call(100.f, 100.f, 0.2f, 2, LightSource::Map);
    const LightCall b = Call(110.f, 100.f, 0.3f, 4, LightSource::Map);
    const LightCall c = Call(100.f, 100.f, 0.4f, 2, LightSource::Effect);
    CheckSameLights(Tick(history, {a, b, c}, classic), {a, b, c});
    const LightCall a2 = Call(104.f, 98.f, 0.5f, 2, LightSource::Map);
    const LightCall b2 = Call(106.f, 103.f, 0.1f, 4, LightSource::Map);
    CheckSameLights(Tick(history, {b2, a2}, classic), {b2, a2});
    // Signed calls keep their order too: the terrain clamps after every addition.
    LightCall dark = Call(105.f, 100.f, 0.f, 3, LightSource::Character);
    dark.color[0] = dark.color[1] = dark.color[2] = -0.3f;
    const LightCall a3 = Call(100.f, 101.f, 0.4f, 2, LightSource::Map);
    CheckSameLights(Tick(history, {a3, dark, b2}, classic), {a3, dark, b2});
    CheckSameLights(Tick(history, {dark, a3}, classic), {dark, a3});
}

TEST_CASE("co-located smoothed lights keep their own ranges [render][lighthistory]")
{
    DynamicLightHistory history;
    LightHistorySettings settings;
    settings.mapSmoothingSeconds = 0.5f;
    for (int frame = 0; frame < 30; ++frame)
    {
        const float jitter = frame % 2 == 0 ? 6.f : -6.f;
        const auto lights = Tick(history,
                                 {Call(100.f + jitter, 100.f, 0.2f, 2, LightSource::Map),
                                  Call(100.f - jitter, 100.f, 0.6f, 4, LightSource::Map)},
                                 settings);
        REQUIRE(lights.size() == 2);
        for (const LightCall& light : lights)
        {
            CHECK(light.color[0] == Approx(light.range == 2 ? 0.2f : 0.6f));
        }
    }
}

TEST_CASE("a darkening call bypasses smoothing, fade and boost [render][lighthistory]")
{
    DynamicLightHistory history;
    LightHistorySettings settings;
    settings.mapSmoothingSeconds = 0.5f;
    settings.effectGain = 2.f;
    settings.effectRangeScale = 2.f;
    settings.effectFadeSeconds = 0.5f;
    LightCall dark = Call(100.f, 100.f, 0.f, 3, LightSource::Character);
    dark.color[0] = dark.color[1] = dark.color[2] = -1.f;
    LightCall mixed = Call(400.f, 100.f, 0.8f, 2, LightSource::Effect);
    mixed.color[2] = -0.3f;
    CheckSameLights(Tick(history, {dark, mixed}, settings), {dark, mixed});
    CHECK(Tick(history, {}, settings).empty());
}

TEST_CASE("a new world starts without the old world's lights [render][lighthistory]")
{
    DynamicLightHistory history;
    LightHistorySettings settings;
    settings.mapSmoothingSeconds = 0.5f;
    settings.effectFadeSeconds = 1.f;
    (void)Tick(history,
               {Call(100.f, 100.f, 0.9f, 3, LightSource::Map), Call(500.f, 500.f, 0.9f, 2, LightSource::Effect)},
               settings);
    history.Record(Call(700.f, 700.f, 0.5f, 2, LightSource::Effect)); // drawn in the old world
    history.BeginTick();
    history.SyncWorld(2);
    const LightCall fresh = Call(100.f, 100.f, 0.2f, 3, LightSource::Map);
    history.Record(fresh);
    CheckSameLights(history.Take(Frame, settings), {fresh});
}

TEST_CASE("a new call never takes the place of a light continued later in the frame [render][lighthistory]")
{
    DynamicLightHistory history;
    LightHistorySettings settings;
    settings.mapSmoothingSeconds = 0.5f;
    settings.effectFadeSeconds = 1.f;
    history.BeginTick();
    // A weak map light (index 0) and strong effects fill the history.
    history.Record(Call(0.f, 0.f, 0.05f, 2, LightSource::Map));
    for (std::size_t i = 1; i < DynamicLightHistory::kCapacity; ++i)
    {
        history.Record(Call(100.f * static_cast<float>(i), 0.f, 0.5f, 2, LightSource::Effect));
    }
    (void)history.Take(Frame, settings);
    // Next frame: a new effect call first, then the weak map light continues; the effects fade.
    const auto lights =
        Tick(history, {Call(0.f, 9000.f, 0.8f, 2, LightSource::Effect), Call(4.f, 0.f, 0.6f, 2, LightSource::Map)},
             settings);
    REQUIRE(lights.size() >= 2);
    CHECK(lights[0].y == Approx(9000.f));
    CHECK(lights[1].x == Approx(4.f));
    // Eased, not restarted: the map light moved only part of the way from 0.05 towards 0.6.
    CHECK(lights[1].color[0] < 0.1f);
    CHECK(history.Overflow() == 0);
}

TEST_CASE("fading lights leave room in the light list for darkening lights added while drawing [render][lighthistory]")
{
    DynamicLightHistory history;
    LightHistorySettings settings;
    settings.effectFadeSeconds = 1.f;
    history.BeginTick();
    for (std::size_t i = 0; i < DynamicLightHistory::kCapacity; ++i)
    {
        history.Record(Call(100.f * static_cast<float>(i), 0.f, i == 3 ? 0.9f : 0.2f, 2, LightSource::Effect));
    }
    (void)history.Take(Frame, settings);
    // All fading, plus one darkening call and one new strong light near the player.
    LightCall dark = Call(0.f, 5000.f, 0.f, 3, LightSource::Character);
    dark.color[0] = dark.color[1] = dark.color[2] = -0.5f;
    const auto lights = Tick(history, {dark, Call(0.f, 6000.f, 1.f, 2, LightSource::Effect)}, settings);
    CHECK(lights.size() == DynamicLightHistory::kListBudget);
    CHECK(lights[0].color[0] < 0.f);
    CHECK(lights[1].y == Approx(6000.f));
    // The strongest fading light is kept.
    CHECK(std::any_of(lights.begin(), lights.end(), [](const LightCall& l) { return l.x == Approx(300.f); }));
}

TEST_CASE("the history knows when the frame is being drawn [render][lighthistory]")
{
    DynamicLightHistory history;
    const LightHistorySettings classic;
    CHECK_FALSE(history.IsDrawing());
    history.BeginTick();
    CHECK_FALSE(history.IsDrawing());
    (void)history.Take(Frame, classic);
    CHECK(history.IsDrawing());
    history.BeginTick();
    CHECK_FALSE(history.IsDrawing());
    (void)history.Take(Frame, classic);
    history.Clear();
    CHECK_FALSE(history.IsDrawing());
}

TEST_CASE("dropped calls are reported at most once a minute, also after a reset [render][lighthistory]")
{
    Render::Lighting::DropReporter drops;
    CHECK(drops.Due({0, 0}, 0.f) == 0);
    CHECK(drops.Due({5, 0}, 1.f) == 5);
    CHECK(drops.Due({9, 0}, 30.f) == 0);
    CHECK(drops.Due({9, 0}, 61.5f) == 4);
    CHECK(drops.Due({9, 0}, 200.f) == 0);
    // The history was cleared (Classic and back) without the reporter seeing the count drop: an
    // equal, a higher and a lower total of the new generation are all new drops.
    CHECK(drops.Due({9, 1}, 300.f) == 9);
    CHECK(drops.Due({5, 2}, 400.f) == 5);
    CHECK(drops.Due({12, 3}, 500.f) == 12);
    // Still at most once a minute across generations.
    CHECK(drops.Due({1, 4}, 530.f) == 0);
    CHECK(drops.Due({1, 4}, 561.f) == 1);
}

TEST_CASE("clearing the history starts a new drop generation [render][lighthistory]")
{
    DynamicLightHistory history;
    const std::uint32_t before = history.Generation();
    history.Clear();
    CHECK(history.Generation() != before);
    CHECK(history.Overflow() == 0);
}

TEST_CASE("a full history makes room by replacing its weakest fading light [render][lighthistory]")
{
    DynamicLightHistory history;
    LightHistorySettings settings;
    settings.effectFadeSeconds = 1.f;
    history.BeginTick();
    for (std::size_t i = 0; i < DynamicLightHistory::kCapacity; ++i)
    {
        history.Record(Call(100.f * static_cast<float>(i), 0.f, i == 7 ? 0.1f : 0.5f, 2, LightSource::Effect));
    }
    (void)history.Take(Frame, settings);
    // Next frame: one new light, all old ones fading; the weakest (index 7) gives way.
    auto lights = Tick(history, {Call(0.f, 9000.f, 0.8f, 2, LightSource::Effect)}, settings);
    CHECK(history.Overflow() == 0);
    CHECK(lights.size() == DynamicLightHistory::kListBudget); // 1 call + the strongest fading lights
    CHECK(history.FollowedCount(LightSource::Effect) == DynamicLightHistory::kCapacity); // the weakest one was replaced
    CHECK(std::none_of(lights.begin(), lights.end(), [](const LightCall& l) { return l.x == Approx(700.f); }));

    // A frame that calls more lights than the history holds drops and counts the rest.
    history.BeginTick();
    for (std::size_t i = 0; i <= DynamicLightHistory::kCapacity; ++i)
    {
        history.Record(Call(100.f * static_cast<float>(i), 50000.f, 0.5f, 2, LightSource::Effect));
    }
    lights = history.Take(Frame, settings);
    CHECK(history.Overflow() == 1);
}

TEST_CASE("the effect boost brightens only: a darkening light keeps its strength and range [render][lighthistory]")
{
    DynamicLightHistory history;
    LightHistorySettings settings;
    settings.effectGain = 2.f;
    settings.effectRangeScale = 2.f;
    LightCall dark = Call(100.f, 100.f, 0.f, 3, LightSource::Effect);
    dark.color[0] = dark.color[1] = dark.color[2] = -1.3f;
    const auto lights = Tick(history, {dark, Call(900.f, 900.f, 0.5f, 3, LightSource::Character)}, settings);
    REQUIRE(lights.size() == 2);
    for (const LightCall& light : lights)
    {
        CHECK(light.range == 3);
        CHECK(std::fabs(light.color[0]) == Approx(light.source == LightSource::Effect ? 1.3f : 0.5f));
    }
}

TEST_CASE("character lights are smoothed like map lights [render][lighthistory]")
{
    DynamicLightHistory history;
    LightHistorySettings settings;
    settings.mapSmoothingSeconds = 0.5f;
    (void)Tick(history, {Call(100.f, 100.f, 0.2f, 3, LightSource::Character)}, settings);
    const auto lights = Tick(history, {Call(100.f, 100.f, 0.8f, 3, LightSource::Character)}, settings);
    REQUIRE(lights.size() == 1);
    CHECK(lights[0].color[0] < 0.25f);
}

TEST_CASE(
    "darkening lights added while drawing darken models even when fading lights fill the list [render][lighthistory]")
{
    namespace LightGrid = Render::Terrain::LightGrid;
    using Grid = std::vector<std::array<float, 3>>;
    const Grid staticGrid(static_cast<std::size_t>(TERRAIN_SIZE) * TERRAIN_SIZE, {0.6f, 0.6f, 0.6f});
    Grid grid = staticGrid;
    auto data = [](Grid& g) { return reinterpret_cast<float (*)[3]>(g.data()); };
    auto cdata = [](const Grid& g) { return reinterpret_cast<const float (*)[3]>(g.data()); };

    DynamicLightHistory history;
    LightHistorySettings settings;
    settings.effectFadeSeconds = 2.f;
    history.BeginTick();
    for (std::size_t i = 0; i < DynamicLightHistory::kCapacity; ++i)
    {
        history.Record(Call(100.f * static_cast<float>(i % 200), 100.f * static_cast<float>(i / 200), 0.3f, 2,
                            LightSource::Effect));
    }
    (void)history.Take(Frame, settings);

    // The next frame: every light fades; the frame's lights go into the grid and the list as in
    // AddFrameHistoryLights, then monsters add 65 darkening lights while the frame is drawn.
    history.BeginTick();
    Render::Lighting::DynamicLightList frameLights;
    const auto lights = history.Take(Frame, settings);
    for (std::size_t i = 0; i < lights.size(); ++i)
    {
        if (i == history.CalledOut())
        {
            frameLights.MarkDisplaceable();
        }
        LightGrid::AddDynamicLight(data(grid), frameLights, lights[i].x, lights[i].y, lights[i].color, lights[i].range,
                                   0.f);
    }
    frameLights.EndDisplaceable();
    CHECK(history.CalledOut() == 0);
    CHECK(frameLights.Lights().size() == Render::Lighting::DynamicLightList::kCapacity);
    REQUIRE(history.IsDrawing());
    const float shade[3] = {-0.5f, -0.5f, -0.5f};
    for (int i = 0; i < 64; ++i)
    {
        LightGrid::AddDynamicLight(data(grid), frameLights, 12000.f, 12000.f, shade, 3, 0.f);
    }
    LightGrid::AddDynamicLight(data(grid), frameLights, 15000.f, 15000.f, shade, 3, 0.f);
    CHECK(frameLights.Overflow() == 0);
    CHECK(frameLights.Lights().size() == Render::Lighting::DynamicLightList::kCapacity);

    float classicStatic[3];
    float withShade[3];
    LightGrid::Sample(cdata(staticGrid), 15000.f, 15000.f, classicStatic);
    LightGrid::SampleStatic(cdata(staticGrid), frameLights.Lights(), 15000.f, 15000.f, withShade);
    CHECK(withShade[0] < classicStatic[0] - 0.4f);
}

TEST_CASE(
    "a full light list drops its weakest lingering light for a later one, never a called one [render][lighthistory]")
{
    using Render::Lighting::DynamicLight;
    using Render::Lighting::DynamicLightList;
    auto light = [](float x)
    {
        DynamicLight l;
        l.position[0] = x;
        l.color[0] = 1.f;
        l.range = 1;
        return l;
    };
    DynamicLightList list;
    for (std::size_t i = 0; i < DynamicLightList::kCapacity - 2; ++i)
    {
        list.Add(light(static_cast<float>(i)));
    }
    list.MarkDisplaceable();
    list.Add(light(5000.f)); // lingering, stronger
    list.Add(light(6000.f)); // lingering, weakest
    list.EndDisplaceable();
    list.Add(light(7000.f)); // added while drawing
    CHECK(list.Overflow() == 0);
    REQUIRE(list.Lights().size() == DynamicLightList::kCapacity);
    CHECK(list.Lights()[DynamicLightList::kCapacity - 2].position[0] == Approx(5000.f));
    CHECK(list.Lights()[DynamicLightList::kCapacity - 1].position[0] == Approx(7000.f));
    list.Add(light(8000.f));
    CHECK(list.Lights()[DynamicLightList::kCapacity - 2].position[0] == Approx(7000.f));
    // Only called lights left: a further one overflows as in Classic.
    list.Add(light(9000.f));
    CHECK(list.Overflow() == 1);
    CHECK(list.Lights()[0].position[0] == Approx(0.f));
    list.Clear();
    CHECK(list.Lights().empty());
}

TEST_CASE("a displaceable light that finds the list full is the one that does not fit [render][lighthistory]")
{
    using Render::Lighting::DynamicLight;
    using Render::Lighting::DynamicLightList;
    const auto light = [](float x)
    {
        DynamicLight l;
        l.position[0] = x;
        l.color[0] = 1.f;
        l.range = 1;
        return l;
    };
    DynamicLightList list;
    for (std::size_t i = 0; i < DynamicLightList::kCapacity - 2; ++i)
    {
        list.Add(light(static_cast<float>(i)));
    }
    list.MarkDisplaceable();
    list.Add(light(5000.f)); // lingering, strongest
    list.Add(light(6000.f)); // lingering, fills the list
    list.Add(light(6500.f)); // lingering, weakest: does not fit
    list.EndDisplaceable();
    CHECK(list.Overflow() == 1);
    REQUIRE(list.Lights().size() == DynamicLightList::kCapacity);
    CHECK(list.Lights()[DynamicLightList::kCapacity - 2].position[0] == Approx(5000.f));
    CHECK(list.Lights()[DynamicLightList::kCapacity - 1].position[0] == Approx(6000.f));
    // Both lingering lights stay displaceable for a light added while drawing.
    list.Add(light(7000.f));
    CHECK(list.Overflow() == 1);
    CHECK(list.Lights()[DynamicLightList::kCapacity - 2].position[0] == Approx(5000.f));
    CHECK(list.Lights()[DynamicLightList::kCapacity - 1].position[0] == Approx(7000.f));
    list.Add(light(8000.f));
    CHECK(list.Overflow() == 1);
    CHECK(list.Lights()[DynamicLightList::kCapacity - 2].position[0] == Approx(7000.f));
    CHECK(list.Lights()[DynamicLightList::kCapacity - 1].position[0] == Approx(8000.f));
}

TEST_CASE("calls beyond the history's capacity are refused and counted, for the Classic path [render][lighthistory]")
{
    DynamicLightHistory history;
    history.BeginTick();
    for (std::size_t i = 0; i < DynamicLightHistory::kCapacity; ++i)
    {
        CHECK(history.Record(Call(static_cast<float>(i), 0.f, 0.2f, 1, LightSource::Map)));
    }
    CHECK_FALSE(history.Record(Call(-5.f, 0.f, 0.2f, 1, LightSource::Map)));
    CHECK(history.Overflow() == 1);
    CHECK(history.Drops().total == 1);
}

TEST_CASE("the effect cap limits overlapping effect lights, so their fade shows at once [render][lighthistory]")
{
    DynamicLightHistory history;
    LightHistorySettings settings;
    settings.effectGain = 2.5f;
    settings.effectRangeScale = 2.f;
    settings.effectFadeSeconds = 0.35f;
    settings.effectCap = 1.f;
    // A beam: twenty lights 50 units apart, as Aqua Beam adds them.
    history.BeginTick();
    for (int i = 0; i < 20; ++i)
    {
        history.Record(Call(1000.f, 1000.f + 50.f * static_cast<float>(i), 0.8f, 2, LightSource::Effect));
    }
    auto lights = history.Take(Frame, settings);
    REQUIRE(lights.size() == 20);
    auto total = [](std::span<const LightCall> all, float x, float y)
    {
        float sum = 0.f;
        for (const LightCall& light : all)
        {
            const float r = static_cast<float>(light.range) * 100.f;
            const float d = std::hypot(light.x - x, light.y - y);
            sum += d < r ? light.color[0] * (r - d) / r : 0.f;
        }
        return sum;
    };
    // Every participating source centre is bounded; no independently normalized overshoot.
    for (int i = 0; i < 20; ++i)
    {
        CHECK(total(lights, 1000.f, 1000.f + 50.f * static_cast<float>(i)) <= 1.00001f);
    }
    CHECK(total(lights, 1000.f, 1475.f) > 0.9f);

    // The beam ends: the light at its middle falls below the cap at once and keeps falling.
    float previous = total(lights, 1000.f, 1475.f);
    for (int frame = 0; frame < 12; ++frame)
    {
        lights = Tick(history, {}, settings, 0.05f);
        const float now = total(lights, 1000.f, 1475.f);
        CHECK(now < previous);
        CHECK(now < 1.f);
        previous = now;
    }

    // A single weak effect light is below the cap and keeps its boosted strength; without a cap
    // nothing is limited.
    DynamicLightHistory single;
    const auto one = Tick(single, {Call(0.f, 0.f, 0.2f, 2, LightSource::Effect)}, settings);
    REQUIRE(one.size() == 1);
    CHECK(one[0].color[0] == Approx(0.5f));
    settings.effectCap = 0.f;
    DynamicLightHistory uncapped;
    uncapped.BeginTick();
    for (int i = 0; i < 20; ++i)
    {
        uncapped.Record(Call(1000.f, 1000.f + 50.f * static_cast<float>(i), 0.8f, 2, LightSource::Effect));
    }
    CHECK(total(uncapped.Take(Frame, settings), 1000.f, 1475.f) > 10.f);
}

namespace
{
float EffectTotal(std::span<const LightCall> lights, float x, float y)
{
    float sum = 0.f;
    for (const LightCall& light : lights)
    {
        if (light.source != LightSource::Effect)
            continue;
        const float radius = static_cast<float>(light.range) * 100.f;
        const float distance = std::hypot(light.x - x, light.y - y);
        sum += light.color[0] * (std::max)(0.f, 1.f - distance / radius);
    }
    return sum;
}
LightHistorySettings CappedSettings()
{
    LightHistorySettings settings;
    settings.effectCap = 1.f;
    settings.effectFadeSeconds = 0.35f;
    settings.effectGain = 2.5f;
    return settings;
}
} // namespace

TEST_CASE("geometric overlap stays under cap [render][lighthistory]")
{
    DynamicLightHistory history;
    auto settings = CappedSettings();
    settings.effectGain = 1.f;
    const float radius = 400.f;
    auto lights =
        Tick(history,
             {Call(1000.f, 1000.f, 1.f, 4, LightSource::Effect),
              Call(1000.f + radius / 2.f, 1000.f, 1.f, 4, LightSource::Effect),
              Call(1000.f - radius / 4.f, 1000.f + radius * std::sqrt(3.f) / 4.f, 1.f, 4, LightSource::Effect),
              Call(1000.f - radius / 4.f, 1000.f - radius * std::sqrt(3.f) / 4.f, 1.f, 4, LightSource::Effect)},
             settings);
    for (const auto& light : lights)
        CHECK(EffectTotal(lights, light.x, light.y) <= 1.00001f);
}

TEST_CASE("growing overlap caps retained envelopes [render][lighthistory]")
{
    DynamicLightHistory history;
    const auto settings = CappedSettings();
    Tick(history, {Call(1000.f, 1000.f, 0.8f, 4, LightSource::Effect)}, settings);
    const auto lights =
        Tick(history,
             {Call(1000.f, 1000.f, 0.8f, 4, LightSource::Effect), Call(1000.f, 1000.f, 0.8f, 4, LightSource::Effect)},
             settings);
    CHECK(EffectTotal(lights, 1000.f, 1000.f) <= 1.00001f);
}

TEST_CASE("cap includes track 513 live or behind fading tracks [render][lighthistory]")
{
    DynamicLightHistory history;
    const auto settings = CappedSettings();
    history.BeginTick();
    for (int i = 0; i < 512; ++i)
        history.Record(Call(1000.f * static_cast<float>(i), 0.f, 0.8f, 2, LightSource::Effect));
    SUBCASE("513 live tracks")
    {
        history.Record(Call(1000.f, 5000.f, 0.8f, 2, LightSource::Effect));
        CHECK(EffectTotal(history.Take(Frame, settings), 1000.f, 5000.f) <= 1.00001f);
    }
    SUBCASE("512 fading tracks plus new call")
    {
        (void)history.Take(Frame, settings);
        const auto lights = Tick(history, {Call(1000.f, 5000.f, 0.8f, 2, LightSource::Effect)}, settings);
        CHECK(EffectTotal(lights, 1000.f, 5000.f) <= 1.00001f);
    }
}

TEST_CASE("every beam position fades immediately at 60Hz [render][lighthistory]")
{
    DynamicLightHistory history;
    auto settings = CappedSettings();
    settings.effectRangeScale = 2.f;
    history.BeginTick();
    for (int i = 0; i < 20; ++i)
        history.Record(Call(1000.f, 1000.f + 50.f * static_cast<float>(i), 0.8f, 2, LightSource::Effect));
    auto lights = history.Take(Frame, settings);
    std::array<float, 20> previous{};
    for (int i = 0; i < 20; ++i)
    {
        previous[i] = EffectTotal(lights, 1000.f, 1000.f + 50.f * static_cast<float>(i));
        CHECK(previous[i] <= 1.00001f);
    }
    for (int frame = 0; frame < 30; ++frame)
    {
        lights = Tick(history, {}, settings);
        for (int i = 0; i < 20; ++i)
        {
            const float next = EffectTotal(lights, 1000.f, 1000.f + 50.f * static_cast<float>(i));
            CHECK(next < previous[i]);
            CHECK(next < 1.f);
            previous[i] = next;
        }
    }
}

TEST_CASE("partially filled lingering interval protects late calls [render][lighthistory]")
{
    using Render::Lighting::DynamicLightList;
    DynamicLightList list;
    const float shade[3] = {-0.5f, -0.5f, -0.5f};
    const float bright[3] = {0.3f, 0.3f, 0.3f};
    for (std::size_t i = 0; i < DynamicLightList::kCapacity - 3; ++i)
        list.Add(Render::Terrain::LightGrid::MakeDynamicLight(0.f, 0.f, bright, 2, 0.f));
    list.MarkDisplaceable();
    list.Add(Render::Terrain::LightGrid::MakeDynamicLight(1000.f, 1000.f, bright, 2, 0.f));
    list.Add(Render::Terrain::LightGrid::MakeDynamicLight(2000.f, 1000.f, bright, 2, 0.f));
    list.EndDisplaceable();
    list.Add(Render::Terrain::LightGrid::MakeDynamicLight(10000.f, 10000.f, shade, 2, 0.f));
    list.Add(Render::Terrain::LightGrid::MakeDynamicLight(15000.f, 15000.f, shade, 2, 0.f));
    CHECK(list.Overflow() == 0);
    CHECK(std::any_of(list.Lights().begin(), list.Lights().end(),
                      [](const auto& l) { return l.position[0] == 10000.f && l.color[0] < 0.f; }));
}

TEST_CASE("untruncated lingering output sorts weakest last [render][lighthistory]")
{
    DynamicLightHistory history;
    LightHistorySettings settings;
    settings.effectFadeSeconds = 1.f;
    history.BeginTick();
    for (std::size_t i = 0; i < DynamicLightHistory::kCapacity; ++i)
        history.Record(Call(100.f * static_cast<float>(i), 0.f,
                            i == DynamicLightHistory::kCapacity - 1 ? 0.9f : (i == 0 ? 0.1f : 0.5f), 2,
                            LightSource::Effect));
    (void)history.Take(Frame, settings);
    const auto lights = Tick(history, {}, settings);
    REQUIRE(lights.size() == DynamicLightHistory::kCapacity);
    CHECK(lights.front().color[0] >= lights.back().color[0]);
    CHECK(lights.back().x == 0.f);
}

TEST_CASE("overflow preserves terrain and retained point order [render][lighthistory]")
{
    namespace Grid = Render::Terrain::LightGrid;
    using Buffer = std::vector<std::array<float, 3>>;
    Buffer classic(static_cast<std::size_t>(TERRAIN_SIZE) * TERRAIN_SIZE, {0.1f, 0.1f, 0.1f});
    Buffer enhanced = classic;
    auto data = [](Buffer& b) { return reinterpret_cast<float (*)[3]>(b.data()); };
    Render::Lighting::DynamicLightList classicList;
    Render::Lighting::DynamicLightList enhancedList;
    DynamicLightHistory history;
    history.BeginTick();
    std::size_t tailCalls = 1;
    SUBCASE("first refusal") {}
    SUBCASE("subsequent signed tail calls")
    {
        tailCalls = 3;
    }
    for (std::size_t i = 0; i < DynamicLightHistory::kCapacity + tailCalls; ++i)
    {
        LightCall call = i == 0 ? Call(10000.f, 10000.f, 0.4f, 1, LightSource::Map)
                                : Call(500.f + static_cast<float>(i), 500.f, 0.f, 1, LightSource::Map);
        if (i >= DynamicLightHistory::kCapacity)
            call = Call(10000.f, 10000.f, i == DynamicLightHistory::kCapacity ? -0.3f : 0.05f, 1, LightSource::Map);
        Grid::AddDynamicLight(data(classic), classicList, call.x, call.y, call.color, call.range, 0.f);
        Grid::RecordFrameLight(Target(data(enhanced), enhancedList, history), call);
    }
    Grid::ReplayFrameLights(Target(data(enhanced), enhancedList, history), Frame, {});
    CHECK(enhanced == classic);
    float sample[3];
    Grid::Sample(data(enhanced), 10000.f, 10000.f, sample);
    CHECK(sample[0] == Approx(0.2f + 0.05f * static_cast<float>(tailCalls - 1)));
    REQUIRE(enhancedList.Lights().size() == classicList.Lights().size());
    for (std::size_t i = 0; i < classicList.Lights().size(); ++i)
    {
        CHECK(enhancedList.Lights()[i].position[0] == classicList.Lights()[i].position[0]);
        CHECK(enhancedList.Lights()[i].color[0] == classicList.Lights()[i].color[0]);
    }
    CHECK(enhancedList.Overflow() == classicList.Overflow());
}

TEST_CASE("production replay protects two late negatives in a partial list [render][lighthistory]")
{
    namespace Grid = Render::Terrain::LightGrid;
    using Buffer = std::vector<std::array<float, 3>>;
    Buffer ambient(static_cast<std::size_t>(TERRAIN_SIZE) * TERRAIN_SIZE, {0.6f, 0.6f, 0.6f});
    Buffer frame = ambient;
    auto data = [](Buffer& b) { return reinterpret_cast<float (*)[3]>(b.data()); };
    Render::Lighting::DynamicLightList list;
    DynamicLightHistory history;
    LightHistorySettings settings;
    settings.effectFadeSeconds = 1.f;
    history.BeginTick();
    Grid::RecordFrameLight(Target(data(frame), list, history), Call(1000.f, 1000.f, 0.3f, 2, LightSource::Effect));
    Grid::RecordFrameLight(Target(data(frame), list, history), Call(2000.f, 1000.f, 0.1f, 2, LightSource::Effect));
    Grid::ReplayFrameLights(Target(data(frame), list, history), Frame, settings);
    frame = ambient;
    list.Clear();
    history.BeginTick();
    for (int i = 0; i < 1021; ++i)
        Grid::RecordFrameLight(Target(data(frame), list, history), Call(3000.f, 3000.f, 0.01f, 1, LightSource::Map));
    Grid::ReplayFrameLights(Target(data(frame), list, history), Frame, settings);
    REQUIRE(list.Lights().size() == 1023);
    for (float x : {10000.f, 15000.f})
        Grid::RecordFrameLight(Target(data(frame), list, history), Call(x, 15000.f, -0.5f, 2, LightSource::Character));
    CHECK(list.Overflow() == 0);
    for (float x : {10000.f, 15000.f})
    {
        float sample[3];
        Grid::SampleStatic(data(ambient), list.Lights(), x, 15000.f, sample);
        CHECK(sample[0] == Approx(0.1f));
    }
    CHECK(
        std::any_of(list.Lights().begin(), list.Lights().end(), [](const auto& l) { return l.position[0] == 1000.f; }));
    CHECK(std::none_of(list.Lights().begin(), list.Lights().end(),
                       [](const auto& l) { return l.position[0] == 2000.f; }));
}

TEST_CASE("cap includes full capacity and preserves independent weak lights [render][lighthistory]")
{
    DynamicLightHistory history;
    const auto settings = CappedSettings();
    history.BeginTick();
    for (std::size_t i = 0; i < DynamicLightHistory::kCapacity; ++i)
        history.Record(Call(1000.f * static_cast<float>(i), 0.f, i == 0 ? 0.1f : 0.8f, 2, LightSource::Effect));
    const auto lights = history.Take(Frame, settings);
    REQUIRE(lights.size() == DynamicLightHistory::kCapacity);
    CHECK(lights.front().color[0] == Approx(0.25f));
    for (const auto& light : lights)
        CHECK(light.color[0] <= 1.00001f);
}

TEST_CASE("overload is counted and next tick resumes history [render][lighthistory]")
{
    namespace Grid = Render::Terrain::LightGrid;
    std::vector<std::array<float, 3>> frame(static_cast<std::size_t>(TERRAIN_SIZE) * TERRAIN_SIZE, {0.1f, 0.1f, 0.1f});
    auto* data = reinterpret_cast<float (*)[3]>(frame.data());
    Render::Lighting::DynamicLightList list;
    DynamicLightHistory history;
    const auto settings = CappedSettings();
    history.BeginTick();
    for (std::size_t i = 0; i < DynamicLightHistory::kCapacity + 3; ++i)
        Grid::RecordFrameLight(Target(data, list, history), Call(0.f, 0.f, 0.1f, 1, LightSource::Effect));
    CHECK(history.Overflow() == 3);
    Grid::ReplayFrameLights(Target(data, list, history), Frame, settings);
    CHECK(history.IsDrawing());
    CHECK(history.CalledOut() == 0);
    list.Clear();
    history.BeginTick();
    Grid::RecordFrameLight(Target(data, list, history), Call(2000.f, 2000.f, 0.2f, 1, LightSource::Effect));
    Grid::ReplayFrameLights(Target(data, list, history), Frame, settings);
    REQUIRE(list.Lights().size() == 1);
    CHECK(list.Lights().front().color[0] == Approx(0.5f));
}

TEST_CASE("overflow while drawing keeps queued calls for the next frame, in order [render][lighthistory]")
{
    namespace Grid = Render::Terrain::LightGrid;
    using Buffer = std::vector<std::array<float, 3>>;
    const Buffer ambient(static_cast<std::size_t>(TERRAIN_SIZE) * TERRAIN_SIZE, {0.1f, 0.1f, 0.1f});
    Buffer frame = ambient;
    Buffer classic = ambient;
    auto data = [](Buffer& b) { return reinterpret_cast<float (*)[3]>(b.data()); };
    Render::Lighting::DynamicLightList list;
    Render::Lighting::DynamicLightList classicList;
    DynamicLightHistory history;
    history.BeginTick();
    Grid::ReplayFrameLights(Target(data(frame), list, history), Frame, {});
    REQUIRE(history.IsDrawing());
    // While drawing: +0.4 (queued), -0.3 (now), 1023 harmless calls (queued, history full), then
    // +0.05 refused. Only the calls applied now join this frame, in arrival order.
    const LightCall up = Call(10000.f, 10000.f, 0.4f, 1, LightSource::Effect);
    const LightCall down = Call(10000.f, 10000.f, -0.3f, 1, LightSource::Effect);
    const LightCall refused = Call(10000.f, 10000.f, 0.05f, 1, LightSource::Effect);
    Grid::RecordFrameLight(Target(data(frame), list, history), up);
    Grid::RecordFrameLight(Target(data(frame), list, history), down);
    for (std::size_t i = 1; i < DynamicLightHistory::kCapacity; ++i)
        Grid::RecordFrameLight(Target(data(frame), list, history),
                               Call(500.f + static_cast<float>(i), 500.f, 0.f, 1, LightSource::Map));
    Grid::RecordFrameLight(Target(data(frame), list, history), refused);
    for (const LightCall& call : {down, refused})
        Grid::AddDynamicLight(data(classic), classicList, call.x, call.y, call.color, call.range, 0.f);
    CHECK(history.Overflow() == 1);
    CHECK(frame == classic);
    REQUIRE(list.Lights().size() == 2);
    CHECK(list.Lights()[0].color[0] < 0.f);
    CHECK(list.Lights()[1].color[0] > 0.f);

    // The next frame gets the queued calls, first one first, never applied twice.
    frame = ambient;
    list.Clear();
    history.BeginTick();
    Grid::ReplayFrameLights(Target(data(frame), list, history), Frame, {});
    float sample[3];
    Grid::Sample(data(frame), 10000.f, 10000.f, sample);
    CHECK(sample[0] == Approx(0.5f));
    REQUIRE(list.Lights().size() == DynamicLightHistory::kCapacity);
    CHECK(list.Lights().front().color[0] == Approx(0.4f));

    // A new tick that overflows at once drains nothing twice: the queue was taken.
    frame = ambient;
    list.Clear();
    history.BeginTick();
    for (std::size_t i = 0; i < DynamicLightHistory::kCapacity + 1; ++i)
        Grid::RecordFrameLight(Target(data(frame), list, history),
                               Call(500.f + static_cast<float>(i), 500.f, 0.f, 1, LightSource::Map));
    Grid::ReplayFrameLights(Target(data(frame), list, history), Frame, {});
    Grid::Sample(data(frame), 10000.f, 10000.f, sample);
    CHECK(sample[0] == Approx(0.1f));
    CHECK(list.Lights().size() == DynamicLightHistory::kCapacity);
}

TEST_CASE("a world change drops the old world's full queue before any new call [render][lighthistory]")
{
    namespace Grid = Render::Terrain::LightGrid;
    using Buffer = std::vector<std::array<float, 3>>;
    const Buffer ambient(static_cast<std::size_t>(TERRAIN_SIZE) * TERRAIN_SIZE, {0.1f, 0.1f, 0.1f});
    Buffer frame = ambient;
    auto data = [](Buffer& b) { return reinterpret_cast<float (*)[3]>(b.data()); };
    Render::Lighting::DynamicLightList list;
    DynamicLightHistory history;
    LightHistorySettings settings;
    settings.effectFadeSeconds = 1.f;
    history.BeginTick();
    Grid::RecordFrameLight(Target(data(frame), list, history, 1), Call(9000.f, 9000.f, 0.6f, 2, LightSource::Effect));
    Grid::ReplayFrameLights(Target(data(frame), list, history, 1), Frame, settings);
    // The old world's frame is drawn and fills the queue for its next frame.
    for (std::size_t i = 0; i < DynamicLightHistory::kCapacity; ++i)
        Grid::RecordFrameLight(Target(data(frame), list, history, 1),
                               Call(10000.f, 10000.f, 0.01f, 1, LightSource::Map));
    CHECK(history.Overflow() == 0);

    frame = ambient;
    list.Clear();
    history.BeginTick();
    const LightCall fresh = Call(2000.f, 2000.f, 0.3f, 1, LightSource::Map);
    Grid::RecordFrameLight(Target(data(frame), list, history, 2), fresh);
    CHECK(frame == ambient); // nothing of the old world was drained into the new one
    Grid::ReplayFrameLights(Target(data(frame), list, history, 2), Frame, settings);
    CHECK(history.Overflow() == 0);
    REQUIRE(list.Lights().size() == 1);
    CHECK(list.Lights().front().position[0] == Approx(2000.f));
    float sample[3];
    for (float at : {9000.f, 10000.f})
    {
        Grid::Sample(data(frame), at, at, sample);
        CHECK(sample[0] == Approx(0.1f));
    }
    Grid::Sample(data(frame), 2000.f, 2000.f, sample);
    CHECK(sample[0] == Approx(0.4f));
}

TEST_CASE(
    "lingering lights of equal strength keep their history order, not their last call order [render][lighthistory]")
{
    DynamicLightHistory history;
    LightHistorySettings settings;
    settings.effectFadeSeconds = 1.f;
    const LightCall first = Call(100.f, 100.f, 0.5f, 2, LightSource::Effect);
    const LightCall second = Call(5000.f, 100.f, 0.5f, 2, LightSource::Effect);
    // Followed in the order first, second (history indices 0 and 1).
    (void)Tick(history, {first, second}, settings);
    // Continued in the other order: called lights keep call order.
    const auto called = Tick(history, {second, first}, settings);
    REQUIRE(called.size() == 2);
    CHECK(called[0].x == Approx(5000.f));
    CHECK(called[1].x == Approx(100.f));
    // Both fade by the same amount: the tie goes to the lower history index.
    const auto lingering = Tick(history, {}, settings);
    REQUIRE(lingering.size() == 2);
    CHECK(lingering[0].color[0] == Approx(lingering[1].color[0]));
    CHECK(lingering[0].x == Approx(100.f));
    CHECK(lingering[1].x == Approx(5000.f));
}
