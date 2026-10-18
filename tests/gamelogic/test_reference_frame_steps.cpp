#include "stdafx.h"
#include "doctest.h"

#include "Engine/AI/ZzzAI.h"

#include <algorithm>
#include <array>

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
