#include "stdafx.h"
#include "doctest.h"

#include "Core/Utilities/Random.h"
#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Effects/EffectBlurs.h"
#include "Render/Effects/ZzzEffect.h"

#include <algorithm>
#include <array>
#include <memory>
#include <cmath>
#include <cstdlib>
#include <numeric>
#include <vector>

TEST_CASE("Integer frame counters step once per reference frame at every FPS [gamelogic][timing]")
{
    for (const double fps : std::array{25.0, 30.0, 60.0, 144.0, 250.0, 1000.0})
    {
        CAPTURE(fps);
        const float factor = static_cast<float>(REFERENCE_FPS / fps);
        float referenceFrames = 0.f;
        int steps = 0;
        int largestStep = 0;
        const int frames = static_cast<int>(fps * 10.0);
        for (int frame = 0; frame < frames; ++frame)
        {
            const int step = TakeReferenceFrameSteps(referenceFrames, factor);
            largestStep = std::max(largestStep, step);
            steps += step;
        }
        CHECK(steps >= 249);
        CHECK(steps <= 250);
        CHECK(largestStep == 1);
    }
}

TEST_CASE("Below the reference rate each frame is one step [gamelogic][timing]")
{
    float referenceFrames = 0.f;
    CHECK(TakeReferenceFrameSteps(referenceFrames, 1.f) == 1);
    CHECK(TakeReferenceFrameSteps(referenceFrames, 1.f) == 1);
    CHECK(referenceFrames == 0.f);
}

namespace
{
constexpr int BatchSamples = 10;

void StartFrame(int referenceStep)
{
    ++RENDERED_FRAME;
    REFERENCE_FRAME_STEP = referenceStep;
}

// One frame's interpolated swing: P1 holds (batch, pose index), like CreateWeaponBlur's ten poses.
void SubmitBatch(CHARACTER* owner, float batch)
{
    for (int i = 0; i < BatchSamples; ++i)
    {
        vec3_t p1 = {batch, static_cast<float>(i), 0.f};
        vec3_t p2 = {batch, static_cast<float>(i), 100.f};
        vec3_t light = {1.f, 1.f, 1.f};
        CreateBlur(owner, p1, p2, light, 0);
    }
}

void SubmitBatch(OBJECT* owner, float batch)
{
    for (int i = 0; i < BatchSamples; ++i)
    {
        vec3_t p1 = {batch, static_cast<float>(i), 0.f};
        vec3_t p2 = {batch, static_cast<float>(i), 100.f};
        vec3_t light = {1.f, 1.f, 1.f};
        CreateObjectBlur(owner, p1, p2, light, 0);
    }
}

template <typename BlurType, std::size_t Count, typename Owner>
const BlurType& FindBlur(const std::array<BlurType, Count>& blurs, const Owner* owner)
{
    const auto found = std::find_if(blurs.begin(), blurs.end(),
                                    [owner](const BlurType& blur) { return blur.Live && blur.Owner == owner; });
    REQUIRE(found != blurs.end());
    return *found;
}

template <typename BlurType> std::vector<float> TrailKeys(const BlurType& blur, int from)
{
    std::vector<float> keys;
    for (int i = from; i < blur.Number; ++i)
    {
        keys.push_back(blur.P1[i][0] * 1000.f + blur.P1[i][1]);
    }
    return keys;
}

// The newest samples come first, so the batch reads back in reverse pose order.
template <typename BlurType> void CheckNewestBatch(const BlurType& blur, float batch)
{
    REQUIRE(blur.Number >= BatchSamples);
    for (int k = 0; k < BatchSamples; ++k)
    {
        CAPTURE(k);
        CHECK(blur.P1[k][0] == batch);
        CHECK(blur.P1[k][1] == static_cast<float>(BatchSamples - 1 - k));
    }
}

template <typename BlurType, std::size_t Count, typename Owner>
void CheckBatchesWithinFrames(std::array<BlurType, Count>& blurs, Owner* owner)
{
    for (auto& blur : blurs)
    {
        blur.Live = false;
    }

    SUBCASE("a reference frame keeps its batch in the trail")
    {
        StartFrame(1);
        SubmitBatch(owner, 1.f);
        CHECK(FindBlur(blurs, owner).Number == BatchSamples);
        CheckNewestBatch(FindBlur(blurs, owner), 1.f);
        MoveBlurs();
        const std::vector<float> history = TrailKeys(FindBlur(blurs, owner), 0);
        REQUIRE(!history.empty());

        for (const float batch : {2.f, 3.f})
        {
            CAPTURE(batch);
            StartFrame(0);
            SubmitBatch(owner, batch);
            const BlurType& blur = FindBlur(blurs, owner);
            CHECK(blur.Number == static_cast<int>(history.size()) + BatchSamples);
            CheckNewestBatch(blur, batch);
            CHECK(TrailKeys(blur, BatchSamples) == history);
            MoveBlurs();
        }

        StartFrame(1);
        SubmitBatch(owner, 4.f);
        const BlurType& blur = FindBlur(blurs, owner);
        CHECK(blur.Number == static_cast<int>(history.size()) + BatchSamples);
        CheckNewestBatch(blur, 4.f);
        CHECK(TrailKeys(blur, BatchSamples) == history);
    }

    SUBCASE("a trail started between reference frames keeps the whole batch")
    {
        StartFrame(0);
        SubmitBatch(owner, 1.f);
        CHECK(FindBlur(blurs, owner).Number == BatchSamples);
        CheckNewestBatch(FindBlur(blurs, owner), 1.f);
        MoveBlurs();

        StartFrame(0);
        SubmitBatch(owner, 2.f);
        CHECK(FindBlur(blurs, owner).Number == BatchSamples);
        CheckNewestBatch(FindBlur(blurs, owner), 2.f);
    }

    for (auto& blur : blurs)
    {
        blur.Live = false;
    }
}
} // namespace

TEST_CASE("A weapon blur keeps every sample submitted within a frame [render][timing]")
{
    static CHARACTER owner;
    CheckBatchesWithinFrames(Render::Effects::g_blurs, &owner);
}

TEST_CASE("An object blur keeps every sample submitted within a frame [render][timing]")
{
    static OBJECT owner;
    CheckBatchesWithinFrames(Render::Effects::g_objectBlurs, &owner);
}

TEST_CASE("A trail laid once per reference frame keeps its authored length at high FPS [render][timing]")
{
    for (int i = 0; i < MAX_JOINTS; ++i)
    {
        Joints[i].Live = false;
    }
    const float savedFactor = FPS_ANIMATION_FACTOR;
    FPS_ANIMATION_FACTOR = 0.1f;
    vec3_t position = {1000.f, 1000.f, 100.f};
    vec3_t angle = {0.f, 0.f, 0.f};
    CreateJoint(BITMAP_JOINT_SPARK, position, position, angle, 0);
    JOINT* joint = std::find_if(Joints, Joints + MAX_JOINTS, [](const JOINT& j) { return j.Live; });
    REQUIRE(joint != Joints + MAX_JOINTS);
    CHECK(joint->ReferenceMaxTails == 2);
    CHECK(joint->MaxTails == 20);

    for (int frame = 0; frame < 30 && joint->Live; ++frame)
    {
        MoveJoints();
        CHECK(joint->MaxTails == 2);
        CHECK(joint->NumTails <= 1);
    }
    joint->Live = false;
    FPS_ANIMATION_FACTOR = savedFactor;
}

namespace
{
// Where a lightning bolt built in one move ends up, from the same random seed.
std::array<float, 3> BoltEnd(float frameFactor)
{
    for (int i = 0; i < MAX_JOINTS; ++i)
    {
        Joints[i].Live = false;
    }
    static OBJECT target;
    target.Live = true;
    Vector(1600.f, 1000.f, 0.f, target.Position);
    FPS_ANIMATION_FACTOR = frameFactor;
    vec3_t start = {1000.f, 1000.f, 100.f};
    vec3_t angle = {0.f, 0.f, 90.f};
    srand(1234);
    // This seed spawns no fire particle at the bolt's end; particles need the option window, which tests lack.
    Random::Seed(1);
    CreateJoint(BITMAP_JOINT_LASER + 1, start, target.Position, angle, 1, &target, 50.f);
    JOINT* joint = std::find_if(Joints, Joints + MAX_JOINTS, [](const JOINT& j) { return j.Live; });
    REQUIRE(joint != Joints + MAX_JOINTS);
    MoveJoints();
    const std::array<float, 3> end = {joint->Position[0], joint->Position[1], joint->Position[2]};
    joint->Live = false;
    return end;
}
} // namespace

TEST_CASE("A lightning bolt reaches its target within one move at every frame rate [render][timing]")
{
    const float savedFactor = FPS_ANIMATION_FACTOR;
    for (const float factor : {1.f, 0.1f})
    {
        CAPTURE(factor);
        const std::array<float, 3> end = BoltEnd(factor);
        // The bolt aims 80 above the target and stops within two steps (2 x 40) of it.
        const float dx = end[0] - 1600.f, dy = end[1] - 1000.f, dz = end[2] - 80.f;
        CHECK(std::sqrt(dx * dx + dy * dy + dz * dz) <= 80.f);
    }
    FPS_ANIMATION_FACTOR = savedFactor;
}

TEST_CASE("A lightning bolt built in one move keeps its authored segment count at high FPS [render][timing]")
{
    const float savedFactor = FPS_ANIMATION_FACTOR;
    int referenceTails = -1;
    for (const float factor : {1.f, 0.1f})
    {
        CAPTURE(factor);
        for (int i = 0; i < MAX_JOINTS; ++i)
        {
            Joints[i].Live = false;
        }
        FPS_ANIMATION_FACTOR = factor;
        vec3_t start = {1000.f, 1000.f, 300.f};
        vec3_t target = {1000.f, 1000.f, 0.f};
        vec3_t angle = {0.f, 0.f, 0.f};
        CreateJoint(BITMAP_JOINT_THUNDER, start, target, angle, 7, nullptr, 10.f);
        JOINT* joint = std::find_if(Joints, Joints + MAX_JOINTS, [](const JOINT& j) { return j.Live; });
        REQUIRE(joint != Joints + MAX_JOINTS);
        MoveJoints();
        if (referenceTails < 0)
        {
            referenceTails = joint->NumTails;
        }
        CHECK(joint->NumTails == referenceTails);
        joint->Live = false;
    }
    FPS_ANIMATION_FACTOR = savedFactor;
}

TEST_CASE("Monster lightning collects at most one target per TargetIndex entry [render]")
{
    for (int i = 0; i < MAX_JOINTS; ++i)
    {
        Joints[i].Live = false;
    }
    static OBJECT caster;
    caster.Live = true;
    const auto characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT);
    CHARACTER* const savedCharacters = CharactersClient;
    CharactersClient = characters.get();
    vec3_t position = {1000.f, 1000.f, 0.f};
    vec3_t angle = {0.f, 0.f, 0.f};
    for (int i = 0; i < 6; ++i)
    {
        CHARACTER& monster = CharactersClient[i];
        monster.Object.Live = true;
        monster.Object.Visible = true;
        monster.Object.Kind = KIND_MONSTER;
        monster.Dead = 0;
        Vector(1000.f + 10.f * i, 1000.f, 0.f, monster.Object.Position);
    }
    CreateJoint(BITMAP_JOINT_THUNDER, position, position, angle, 15, &caster, 50.f, -1, 0, 77);
    JOINT* joint = std::find_if(Joints, Joints + MAX_JOINTS, [](const JOINT& j) { return j.Live; });
    REQUIRE(joint != Joints + MAX_JOINTS);
    CHECK(joint->Weapon == 5 * 15);
    CHECK(joint->m_bySkillSerialNum == 77);
    joint->Live = false;
    CharactersClient = savedCharacters;
}
