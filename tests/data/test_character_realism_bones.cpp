#include "stdafx.h"

#include "doctest.h"

#include "TestModelSlots.h"

#include "Engine/Object/Movement/FootPlacement.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Terrain/ZzzLodTerrain.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <utility>
#include <vector>

extern EGameScene SceneFlag;

namespace
{
const std::filesystem::path PlayerFolder = std::filesystem::path(MU_TEST_DATA_DIR) / "Player";

int BoneNamed(const BMD& model, const char* name)
{
    for (int i = 0; i < model.NumBones; ++i)
    {
        if (!model.Bones[i].Dummy && std::strcmp(model.Bones[i].Name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

float Distance(const float a[3][4], const float b[3][4])
{
    const float x = a[0][3] - b[0][3];
    const float y = a[1][3] - b[1][3];
    const float z = a[2][3] - b[2][3];
    return std::sqrt(x * x + y * y + z * z);
}

// A bone's rotation with its columns made unit length (a bone may carry the model's scale).
void UnitRotation(const float bone[3][4], float rotation[3][3])
{
    for (int c = 0; c < 3; ++c)
    {
        const float length = std::sqrt(bone[0][c] * bone[0][c] + bone[1][c] * bone[1][c] + bone[2][c] * bone[2][c]);
        for (int r = 0; r < 3; ++r)
        {
            rotation[r][c] = bone[r][c] / length;
        }
    }
}

// The rotation taking bone `from`'s rotation onto bone `to`'s: to * from^T.
void RotationBetween(const float from[3][4], const float to[3][4], float rotation[3][3])
{
    float a[3][3];
    float b[3][3];
    UnitRotation(from, a);
    UnitRotation(to, b);
    for (int r = 0; r < 3; ++r)
    {
        for (int c = 0; c < 3; ++c)
        {
            rotation[r][c] = b[r][0] * a[c][0] + b[r][1] * a[c][1] + b[r][2] * a[c][2];
        }
    }
}

struct PlayerModel
{
    PlayerModel() : slots(), model(Models[MODEL_PLAYER])
    {
        const std::wstring folder = PlayerFolder.wstring() + L"/";
        REQUIRE(model.Open2(folder.c_str(), L"player.bmd"));
        REQUIRE(model.NumBones > 0);
        model.BoneHead = 20;
        model.BodyScale = 1.f;
        Vector(0.f, 0.f, 0.f, model.BodyOrigin);
    }

    // The bones of a standing player facing `facing`, in model space, as the pre-pass builds them.
    void Build(float bones[MAX_BONES][3][4], float facing, float ponytailLag = 0.f, float headTurnUp = 0.f)
    {
        vec3_t angle = {0.f, 0.f, facing};
        vec3_t head = {0.f, 0.f, 0.f};
        model.Animation(bones, 0.f, 0.f, PLAYER_STOP_MALE, angle, head, false, false, nullptr, PLAYER_STOP_MALE,
                        MAX_BONES, ponytailLag, headTurnUp);
    }

    // The slots first: the model is one of them.
    TestModelSlots slots;
    BMD& model;
};

// A terrain sloping down toward +X by `drop` per tile around tile (128, 128); the heights and the scene put back after
// the test.
class SideSlope
{
public:
    static constexpr int From = 120;
    static constexpr int To = 136;

    explicit SideSlope(float drop) : m_scene(SceneFlag)
    {
        SceneFlag = MAIN_SCENE;
        for (int y = From; y <= To; ++y)
        {
            for (int x = From; x <= To; ++x)
            {
                const int index = TERRAIN_INDEX(x, y);
                m_saved.emplace_back(BackTerrainHeight[index], TerrainWall[index]);
                BackTerrainHeight[index] = 500.f - static_cast<float>(x - From) * drop;
                TerrainWall[index] = 0;
            }
        }
        MarkTerrainHeightsChanged();
    }

    ~SideSlope()
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
    }

    SideSlope(const SideSlope&) = delete;
    SideSlope& operator=(const SideSlope&) = delete;

private:
    EGameScene m_scene;
    std::vector<std::pair<float, WORD>> m_saved;
};

CHARACTER& StandingPlayer()
{
    static CHARACTER player;
    player.Initialize();
    player.Helper.Type = 0;
    player.Object.Type = MODEL_PLAYER;
    player.Object.Kind = KIND_PLAYER;
    player.Object.CurrentAction = PLAYER_STOP_MALE;
    player.Object.Scale = 1.f;
    Vector(128.5f * TERRAIN_SCALE, 128.5f * TERRAIN_SCALE, 0.f, player.Object.Position);
    Vector(0.f, 0.f, 0.f, player.Object.Angle);
    return player;
}
} // namespace

TEST_CASE("Feet on flat ground leave the bones as animated [data][animation][feet]")
{
    PlayerModel player;
    const SideSlope flat(0.f);
    CHARACTER& c = StandingPlayer();
    static float bones[MAX_BONES][3][4];
    static float animated[MAX_BONES][3][4];
    player.Build(bones, 0.f);
    std::memcpy(animated, bones, sizeof(bones));

    Engine::Characters::PlaceFeet(&c, bones);
    CHECK(std::memcmp(bones, animated, sizeof(float) * 12 * player.model.NumBones) == 0);
}

TEST_CASE("A foot over lower ground reaches down to it, the body lowered and the legs bent, not stretched "
          "[data][animation][feet]")
{
    PlayerModel player;
    const BMD& model = player.model;
    const int legs[2][3] = {
        {BoneNamed(model, "Bip01 L Thigh"), BoneNamed(model, "Bip01 L Calf"), BoneNamed(model, "Bip01 L Foot")},
        {BoneNamed(model, "Bip01 R Thigh"), BoneNamed(model, "Bip01 R Calf"), BoneNamed(model, "Bip01 R Foot")}};
    for (const auto& leg : legs)
    {
        for (const int bone : leg)
        {
            REQUIRE(bone >= 0);
        }
    }

    // The ground drops by 60 a tile across the feet (they stand side by side along X when facing -Y).
    const SideSlope slope(60.f);
    CHARACTER& c = StandingPlayer();
    const OBJECT& o = c.Object;
    static float bones[MAX_BONES][3][4];
    static float animated[MAX_BONES][3][4];
    player.Build(bones, 0.f);
    std::memcpy(animated, bones, sizeof(bones));

    const float ground = RequestTerrainHeight(o.Position[0], o.Position[1]);
    float rise[2];
    for (int i = 0; i < 2; ++i)
    {
        const int foot = legs[i][2];
        rise[i] =
            RequestTerrainHeight(o.Position[0] + animated[foot][0][3], o.Position[1] + animated[foot][1][3]) - ground;
    }
    CAPTURE(rise[0]);
    CAPTURE(rise[1]);
    REQUIRE(std::fabs(rise[0] - rise[1]) > 5.f);
    const int lower = rise[0] < rise[1] ? 0 : 1;
    REQUIRE(rise[lower] < 0.f);

    Engine::Characters::PlaceFeet(&c, bones);

    // The body (the pelvis) goes down as far as the lower foot's ground.
    const int pelvis = model.Bones[legs[0][0]].Parent;
    CHECK(bones[pelvis][2][3] - animated[pelvis][2][3] == doctest::Approx(rise[lower]).epsilon(0.001));
    for (int i = 0; i < 2; ++i)
    {
        CAPTURE(i);
        const int thigh = legs[i][0];
        const int calf = legs[i][1];
        const int foot = legs[i][2];
        // Each foot on its own ground, within a unit or two (a leg that would go straight stays a little bent).
        CHECK(std::fabs(bones[foot][2][3] - animated[foot][2][3] - rise[i]) < 2.f);
        // Across the ground the foot stays where it was.
        CHECK(std::fabs(bones[foot][0][3] - animated[foot][0][3]) < 2.f);
        CHECK(std::fabs(bones[foot][1][3] - animated[foot][1][3]) < 2.f);
        // The leg keeps its lengths, and never goes quite straight.
        const float thighLength = Distance(animated[thigh], animated[calf]);
        const float shinLength = Distance(animated[calf], animated[foot]);
        CHECK(Distance(bones[thigh], bones[calf]) == doctest::Approx(thighLength).epsilon(0.001));
        CHECK(Distance(bones[calf], bones[foot]) == doctest::Approx(shinLength).epsilon(0.001));
        CHECK(Distance(bones[thigh], bones[foot]) <= (thighLength + shinLength) * 0.996f);
    }
    // The higher foot's leg bends more than it was animated.
    const int higher = 1 - lower;
    CHECK(Distance(bones[legs[higher][0]], bones[legs[higher][2]]) <
          Distance(animated[legs[higher][0]], animated[legs[higher][2]]));
}

TEST_CASE("The ponytail's lag swings it about the head's upright and leaves its root where it hangs [data][animation]")
{
    PlayerModel player;
    const BMD& model = player.model;
    const int ponytail = BoneNamed(model, "Bip01 Ponytail1");
    REQUIRE(ponytail >= 0);
    REQUIRE(model.PonytailBone == ponytail);
    const int head = model.Bones[ponytail].Parent;

    static float straight[MAX_BONES][3][4];
    static float lagging[MAX_BONES][3][4];
    player.Build(straight, 30.f);
    player.Build(lagging, 30.f, 10.f);

    // The head doesn't move; the ponytail's root stays at the head.
    CHECK(std::memcmp(straight[head], lagging[head], sizeof(straight[head])) == 0);
    for (int r = 0; r < 3; ++r)
    {
        CHECK(lagging[ponytail][r][3] == doctest::Approx(straight[ponytail][r][3]).epsilon(0.0001));
    }

    // Turned by 10 degrees about the head's upright (its X axis).
    float turn[3][3];
    RotationBetween(straight[ponytail], lagging[ponytail], turn);
    const float trace = turn[0][0] + turn[1][1] + turn[2][2];
    CHECK(std::acos(std::clamp((trace - 1.f) / 2.f, -1.f, 1.f)) * 180.f / static_cast<float>(Q_PI) ==
          doctest::Approx(10.f).epsilon(0.01));
    float headRotation[3][3];
    UnitRotation(straight[head], headRotation);
    const float upright[3] = {headRotation[0][0], headRotation[1][0], headRotation[2][0]};
    for (int r = 0; r < 3; ++r)
    {
        const float turned = turn[r][0] * upright[0] + turn[r][1] * upright[1] + turn[r][2] * upright[2];
        CHECK(turned == doctest::Approx(upright[r]).epsilon(0.001).scale(1.f));
    }

    // The bone below it swings with it.
    const int tip = BoneNamed(model, "Bip01 Ponytail11");
    if (tip >= 0)
    {
        CHECK(Distance(straight[tip], lagging[tip]) > 0.1f);
    }
}

TEST_CASE("A head turned up in the world turns about +Z and stays where it is [data][animation]")
{
    PlayerModel player;
    const BMD& model = player.model;
    const int head = model.BoneHead;
    REQUIRE(BoneNamed(model, "Bip01 Head") == head);

    static float straight[MAX_BONES][3][4];
    static float turned[MAX_BONES][3][4];
    player.Build(straight, 45.f);
    player.Build(turned, 45.f, 0.f, 20.f);

    // Its parent isn't touched; its place stays.
    const int neck = model.Bones[head].Parent;
    CHECK(std::memcmp(straight[neck], turned[neck], sizeof(straight[neck])) == 0);
    for (int r = 0; r < 3; ++r)
    {
        CHECK(turned[head][r][3] == doctest::Approx(straight[head][r][3]).epsilon(0.0001));
    }

    // Its rotation is the old one turned 20 degrees about the world's upright.
    const float radians = 20.f * static_cast<float>(Q_PI) / 180.f;
    const float aboutZ[3][3] = {
        {std::cos(radians), -std::sin(radians), 0.f}, {std::sin(radians), std::cos(radians), 0.f}, {0.f, 0.f, 1.f}};
    for (int r = 0; r < 3; ++r)
    {
        for (int c = 0; c < 3; ++c)
        {
            CAPTURE(r);
            CAPTURE(c);
            const float expected = aboutZ[r][0] * straight[head][0][c] + aboutZ[r][1] * straight[head][1][c] +
                                   aboutZ[r][2] * straight[head][2][c];
            CHECK(turned[head][r][c] == doctest::Approx(expected).epsilon(0.001).scale(1.f));
        }
    }
}

namespace
{
// Nanoseconds per call of `work`, the fastest of five runs of `calls` calls.
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

// Not run with the tests (ctest -E benchmark): what the realism steps add to a player's bone build, for the PR.
TEST_CASE("The realism steps' cost in a player's bone build (benchmark) [data][animation][benchmark]")
{
    PlayerModel player;
    static float bones[MAX_BONES][3][4];
    constexpr int Calls = 20000;
    float sink = 0.f;
    const auto build = [&](float headYaw, float ponytailLag)
    {
        vec3_t angle = {0.f, 0.f, 45.f};
        vec3_t head = {headYaw, 0.f, 0.f};
        player.model.Animation(bones, 2.5f, 2.f, PLAYER_WALK_SWORD, angle, head, false, false, nullptr,
                               PLAYER_WALK_SWORD, MAX_BONES, ponytailLag);
        sink += bones[20][2][3];
    };
    const double plain = NanosecondsPerCall(Calls, [&](int) { build(0.f, 0.f); });
    const double headTurned = NanosecondsPerCall(Calls, [&](int) { build(40.f, 0.f); });
    const double ponytail = NanosecondsPerCall(Calls, [&](int) { build(40.f, 8.f); });
    CHARACTER& walker = StandingPlayer();
    build(0.f, 0.f);
    const double feetFlat = NanosecondsPerCall(Calls, [&](int) { Engine::Characters::PlaceFeet(&walker, bones); });
    double feetSlope = 0.;
    {
        const SideSlope slope(30.f);
        feetSlope = NanosecondsPerCall(Calls,
                                       [&](int)
                                       {
                                           build(0.f, 0.f);
                                           Engine::Characters::PlaceFeet(&walker, bones);
                                       }) -
                    plain;
    }
    MESSAGE("bone build, ns per player: plain " << plain << ", head turned (neck and spine) " << headTurned
                                                << ", plus the ponytail's lag " << ponytail << "; feet on flat ground +"
                                                << feetFlat << ", on a slope +" << feetSlope);
    CHECK(sink == sink);
}
