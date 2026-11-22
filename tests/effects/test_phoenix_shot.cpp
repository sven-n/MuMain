#include "stdafx.h"
#include "doctest.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Models/ZzzBMD.h"
#include "World/MapInfra/w_MapProcess.h"

#include "EffectTestData.h"

#include <cmath>
#include <map>
#include <memory>
#include <vector>

extern float FPS_ANIMATION_FACTOR;

// Phoenix Shot as its move handlers play it (Render/Effects/Behaviors/PhoenixShotMoves.cpp): the rig cast at the
// caster's feet lets go three blades, which fly at the target and strike it.
namespace
{
constexpr int CasterIndex = 1;
constexpr int TargetIndex = 5;
constexpr int TargetModel = MODEL_MONSTER01;
constexpr int TargetBones = 4;
constexpr float StrikeLife = 30.f;

// A caster with its feet at `feet` and a target `away` along x from it, at the frame factor `frameFactor`, with the
// shipped effect rows. The rig's model has one action and no bones; the target's model has TargetBones bones, kept in
// a pose of its own.
struct PhoenixScope
{
    std::unique_ptr<BMD[]> models{new BMD[MAX_MODELS]};
    std::unique_ptr<CHARACTER[]> characters{new CHARACTER[MAX_CHARACTERS_CLIENT]{}};
    std::unique_ptr<vec34_t[]> targetPose{new vec34_t[MAX_BONES]{}};
    Action_t rigAction{};
    BMD* previousModels = Models;
    CHARACTER* previousCharacters = CharactersClient;
    CHARACTER* previousHero = Hero;
    MapProcessPtr previousMapProcess = g_MapProcess;
    float previousFrameFactor = FPS_ANIMATION_FACTOR;
    vec3_t feet{12800.f, 12800.f, 100.f};

    PhoenixScope(float away, float frameFactor)
    {
        EffectTestData::BuildShippedRegistry();
        Models = models.get();
        CharactersClient = characters.get();
        Hero = nullptr;
        g_MapProcess = MapProcess::Make();
        FPS_ANIMATION_FACTOR = frameFactor;
        ClearPools();

        rigAction.NumAnimationKeys = 30;
        Models[MODEL_PHOENIX_SHOT].NumActions = 1;
        Models[MODEL_PHOENIX_SHOT].Actions = &rigAction;
        Models[TargetModel].NumBones = TargetBones;

        OBJECT& caster = Caster();
        caster.Live = true;
        caster.Visible = true;
        caster.Scale = 1.f;
        VectorCopy(feet, caster.Position);

        OBJECT& target = Target();
        target.Live = true;
        target.Visible = true;
        target.Type = TargetModel;
        target.Scale = 1.f;
        Vector(feet[0] + away, feet[1], feet[2], target.Position);
        target.BoneTransform = targetPose.get();
        for (int bone = 0; bone < TargetBones; ++bone)
        {
            targetPose[bone][0][3] = 20.f * bone;
            targetPose[bone][1][3] = -10.f * bone;
            targetPose[bone][2][3] = 60.f + 40.f * bone;
        }
    }

    ~PhoenixScope()
    {
        ClearPools();
        Target().BoneTransform = nullptr;
        Models[MODEL_PHOENIX_SHOT].Actions = nullptr;
        Models[MODEL_PHOENIX_SHOT].NumActions = 0;
        Models[TargetModel].NumBones = 0;
        FPS_ANIMATION_FACTOR = previousFrameFactor;
        g_MapProcess = previousMapProcess;
        Hero = previousHero;
        CharactersClient = previousCharacters;
        Models = previousModels;
    }

    static void ClearPools()
    {
        for (int i = 0; i < MAX_EFFECTS; ++i)
            Effects[i].Live = false;
        for (int i = 0; i < MAX_PARTICLES; ++i)
            Particles[i].Live = false;
    }

    OBJECT& Caster()
    {
        return characters[CasterIndex].Object;
    }
    OBJECT& Target()
    {
        return characters[TargetIndex].Object;
    }

    // As MonkSystem's RageCreateEffect casts it.
    void Cast()
    {
        vec3_t angle{0.f, 0.f, 0.f};
        vec3_t light{1.f, 1.f, 1.f};
        CreateEffect(MODEL_PHOENIX_SHOT, feet, angle, light, 0, &Caster(), -1, 0, 0, 0, 1.f, TargetIndex);
    }
};

std::vector<const OBJECT*> LiveEffects(int type, int subType)
{
    std::vector<const OBJECT*> found;
    for (int i = 0; i < MAX_EFFECTS; ++i)
        if (Effects[i].Live && Effects[i].Type == type && Effects[i].SubType == subType)
            found.push_back(&Effects[i]);
    return found;
}

std::vector<const OBJECT*> Blades()
{
    std::vector<const OBJECT*> found;
    for (int i = 0; i < MAX_EFFECTS; ++i)
        if (Effects[i].Live && Effects[i].Type == MODEL_WINDSPIN03)
            found.push_back(&Effects[i]);
    return found;
}

const OBJECT* LeadBlade()
{
    for (const OBJECT* blade : Blades())
        if (blade->SubType == 0)
            return blade;
    return nullptr;
}

std::vector<const PARTICLE*> LiveParticles(int type, int subType)
{
    std::vector<const PARTICLE*> found;
    for (int i = 0; i < MAX_PARTICLES; ++i)
        if (Particles[i].Live && Particles[i].Type == type && Particles[i].SubType == subType)
            found.push_back(&Particles[i]);
    return found;
}

bool Struck()
{
    return !LiveEffects(BITMAP_TWLIGHT, 4).empty();
}

// Moves the effects until the strike, at most `ticks` reference ticks; the reference time at the end of the move
// that struck, or -1.
float MoveUntilStruck(float ticks, float* elapsed)
{
    while (*elapsed < ticks)
    {
        MoveEffects();
        *elapsed += FPS_ANIMATION_FACTOR;
        if (Struck())
            return *elapsed;
    }
    return -1.f;
}
} // namespace

TEST_CASE("Phoenix Shot's rig lets go its three blades on its eighth tick, 100 over the caster's feet [effects]")
{
    for (const float frameFactor : {1.f, 0.5f})
    {
        CAPTURE(frameFactor);
        PhoenixScope scope(600.f, frameFactor);
        scope.Cast();

        // The reference time at the start of the move that lets them go.
        float letGo = -1.f;
        for (float elapsed = 0.f; elapsed < 20.f; elapsed += frameFactor)
        {
            MoveEffects();
            if (!Blades().empty())
            {
                letGo = elapsed;
                break;
            }
        }
        CHECK(letGo == doctest::Approx(7.f));

        const std::vector<const OBJECT*> blades = Blades();
        REQUIRE(blades.size() == 3);
        int subTypes = 0;
        for (const OBJECT* blade : blades)
        {
            CHECK(blade->StartPosition[0] == doctest::Approx(scope.feet[0]));
            CHECK(blade->StartPosition[1] == doctest::Approx(scope.feet[1]));
            CHECK(blade->StartPosition[2] == doctest::Approx(scope.feet[2] + 100.f));
            CHECK(blade->m_sTargetIndex == TargetIndex);
            subTypes |= 1 << blade->SubType;
        }
        CHECK(subTypes == 0b111);
    }
}

TEST_CASE("Phoenix Shot's lead blade strikes its target once: sparks over it, spirals on the ground under it "
          "[effects]")
{
    PhoenixScope scope(600.f, 1.f);
    scope.Cast();
    float elapsed = 0.f;
    REQUIRE(MoveUntilStruck(60.f, &elapsed) > 0.f);
    const OBJECT& target = scope.Target();

    const std::vector<const OBJECT*> spirals = LiveEffects(BITMAP_TWLIGHT, 4);
    CHECK(spirals.size() == 8);
    for (const OBJECT* spiral : spirals)
    {
        CHECK(spiral->Position[0] == doctest::Approx(target.Position[0]));
        CHECK(spiral->Position[1] == doctest::Approx(target.Position[1]));
        CHECK(spiral->Position[2] == doctest::Approx(target.Position[2]));
    }
    const std::vector<const PARTICLE*> sparks = LiveParticles(BITMAP_SPARK, 14);
    CHECK(sparks.size() == 7);
    for (const PARTICLE* spark : sparks)
    {
        CHECK(std::fabs(spark->Position[0] - target.Position[0]) <= 75.f);
        CHECK(std::fabs(spark->Position[1] - target.Position[1]) <= 75.f);
        CHECK(spark->Position[2] >= target.Position[2] + 230.f);
    }
    // Every blade ends its flight at the target and fades there.
    const std::vector<const OBJECT*> blades = Blades();
    CHECK(blades.size() == 3);
    for (const OBJECT* blade : blades)
    {
        CHECK(blade->Position[0] == doctest::Approx(target.Position[0]));
        CHECK(blade->Position[1] == doctest::Approx(target.Position[1]));
        CHECK(blade->LifeTime < StrikeLife);
    }

    // Nothing strikes again while they fade.
    for (int move = 0; move < 10; ++move)
        MoveEffects();
    CHECK(LiveEffects(BITMAP_TWLIGHT, 4).size() == 8);
    CHECK(LiveParticles(BITMAP_SPARK, 14).size() == 7);
}

TEST_CASE("A Phoenix Shot blade still short of its target when its flight runs out strikes it then [effects]")
{
    // 20 ticks of flight take a blade 880 along.
    PhoenixScope scope(5000.f, 1.f);
    scope.Cast();
    float elapsed = 0.f;
    const float struck = MoveUntilStruck(60.f, &elapsed);
    // Let go on the eighth tick, first moved on the ninth, 20 ticks of flight.
    CHECK(struck == doctest::Approx(28.f));
    const std::vector<const OBJECT*> spirals = LiveEffects(BITMAP_TWLIGHT, 4);
    CHECK(spirals.size() == 8);
    for (const OBJECT* spiral : spirals)
        CHECK(spiral->Position[0] == doctest::Approx(scope.Target().Position[0]));
    for (const OBJECT* blade : Blades())
        CHECK(blade->Position[0] == doctest::Approx(scope.Target().Position[0]));
}

TEST_CASE("Phoenix Shot's blades are where they were at each reference tick and strike in the same tick, at 25 FPS "
          "and at 50 [effects]")
{
    // Where the lead blade is at the end of each whole reference tick before the strike, and when it struck.
    struct Flight
    {
        std::map<int, float> along;
        float struck = -1.f;
    };
    const auto fly = [](float frameFactor)
    {
        Flight flight;
        PhoenixScope scope(600.f, frameFactor);
        scope.Cast();
        float elapsed = 0.f;
        while (elapsed < 60.f && flight.struck < 0.f)
        {
            MoveEffects();
            elapsed += frameFactor;
            if (Struck())
            {
                flight.struck = elapsed;
            }
            else if (const OBJECT* lead = LeadBlade();
                     lead != nullptr && std::fabs(elapsed - std::round(elapsed)) < 1e-4f)
            {
                flight.along[static_cast<int>(std::lround(elapsed))] = lead->Position[0] - scope.feet[0];
            }
        }
        return flight;
    };
    const Flight reference = fly(1.f);
    const Flight fast = fly(0.5f);

    REQUIRE(reference.struck > 0.f);
    REQUIRE(fast.struck > 0.f);
    // The faster frame rate looks at the flight more often, so it may see the blade get past the target up to a frame
    // of its own sooner, never later.
    CHECK(fast.struck <= reference.struck);
    CHECK(reference.struck - fast.struck < 1.f);

    REQUIRE(reference.along.size() > 5);
    for (const auto& [tick, along] : reference.along)
    {
        CAPTURE(tick);
        const auto same = fast.along.find(tick);
        if (same == fast.along.end())
            continue; // struck by then at the faster rate
        CHECK(same->second == doctest::Approx(along));
    }
}

TEST_CASE("Phoenix Shot's lead blade lands a blow a tick on its target's bones for 14 ticks after the strike "
          "[effects]")
{
    PhoenixScope scope(600.f, 1.f);
    scope.Cast();
    float elapsed = 0.f;
    REQUIRE(MoveUntilStruck(60.f, &elapsed) > 0.f);
    REQUIRE(LiveParticles(BITMAP_SBUMB, 1).empty());

    // The blows live 4 ticks, but particles move apart from the effects, so every one stays to count.
    for (int move = 0; move < 20; ++move)
        MoveEffects();
    const std::vector<const PARTICLE*> blows = LiveParticles(BITMAP_SBUMB, 1);
    CHECK(blows.size() == 14);

    const OBJECT& target = scope.Target();
    for (const PARTICLE* blow : blows)
    {
        // On one of the bones, up to 20 off it along the ground.
        bool onABone = false;
        for (int bone = 0; bone < TargetBones; ++bone)
        {
            const float x = target.Position[0] + scope.targetPose[bone][0][3];
            const float y = target.Position[1] + scope.targetPose[bone][1][3];
            const float z = target.Position[2] + scope.targetPose[bone][2][3];
            onABone = onABone || (std::fabs(blow->Position[0] - x) <= 20.f &&
                                  std::fabs(blow->Position[1] - y) <= 20.f && blow->Position[2] == doctest::Approx(z));
        }
        CHECK(onABone);
    }
}
