#include "stdafx.h"
#include "doctest.h"

#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Effects/EffectBlurs.h"
#include "Render/Effects/ZzzEffect.h"

#include <algorithm>
#include <array>
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
