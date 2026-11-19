#include <doctest.h>

#include "Render/PostProcessing/EdgeSmoothing.h"

namespace
{
constexpr mu::PostProcessingLook AuthoredLook{1.0f, 0.4f, 0.8f, 0.6f, 75.0f};
} // namespace

TEST_CASE("edge smoothing alone still takes the post-processed frame [render][edge-smoothing]")
{
    using Render::PostProcessing::UsesPostProcessedFrame;
    CHECK_FALSE(UsesPostProcessedFrame(false, false));
    CHECK(UsesPostProcessedFrame(true, false));
    CHECK(UsesPostProcessedFrame(false, true));
    CHECK(UsesPostProcessedFrame(true, true));
}

TEST_CASE("with post-processing on, the composite keeps the authored look [render][edge-smoothing]")
{
    const mu::PostProcessingLook look = Render::PostProcessing::CompositedLook(true, AuthoredLook);
    CHECK(look.bloomStrength == AuthoredLook.bloomStrength);
    CHECK(look.bloomLevelFalloff == AuthoredLook.bloomLevelFalloff);
    CHECK(look.gradingStrength == AuthoredLook.gradingStrength);
    CHECK(look.ambientOcclusionStrength == AuthoredLook.ambientOcclusionStrength);
    CHECK(look.ambientOcclusionRadius == AuthoredLook.ambientOcclusionRadius);
}

TEST_CASE("with only edge smoothing on, the composite adds no bloom, grading or occlusion [render][edge-smoothing]")
{
    const mu::PostProcessingLook look = Render::PostProcessing::CompositedLook(false, AuthoredLook);
    CHECK(look.bloomStrength == 0.0f);
    CHECK(look.gradingStrength == 0.0f);
    CHECK(look.ambientOcclusionStrength == 0.0f);
}

TEST_CASE("$fxaa accepts on and off [render][edge-smoothing]")
{
    using Render::PostProcessing::ParseEdgeSmoothingCommand;
    CHECK(ParseEdgeSmoothingCommand(L"on") == true);
    CHECK(ParseEdgeSmoothingCommand(L" OFF ") == false);
    CHECK_FALSE(ParseEdgeSmoothingCommand(L"").has_value());
    CHECK_FALSE(ParseEdgeSmoothingCommand(L"2").has_value());
    CHECK_FALSE(ParseEdgeSmoothingCommand(L"onn").has_value());
}
