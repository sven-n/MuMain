#include <doctest.h>

#include "Render/PostProcessing/BloomSource.h"
#include "Render/PostProcessing/SceneBoundary.h"

using Render::PostProcessing::FrameSplit;
using Render::PostProcessing::SceneBoundary;

TEST_CASE("a frame splits into scene and UI at its boundary [render][post_processing]")
{
    SceneBoundary boundary;
    boundary.Mark(120);

    const FrameSplit split = boundary.Split(200, true);

    CHECK(split.postProcessed);
    CHECK(split.sceneEnd == 120);
}

TEST_CASE("a frame without a boundary is not post-processed [render][post_processing]")
{
    SceneBoundary boundary;

    const FrameSplit split = boundary.Split(200, true);

    CHECK_FALSE(boundary.IsMarked());
    CHECK_FALSE(split.postProcessed);
    CHECK(split.sceneEnd == 200);
}

TEST_CASE("post-processing off replays the whole frame unprocessed [render][post_processing]")
{
    SceneBoundary boundary;
    boundary.Mark(120);

    const FrameSplit split = boundary.Split(200, false);

    CHECK_FALSE(split.postProcessed);
    CHECK(split.sceneEnd == 200);
}

TEST_CASE("the first boundary of a frame wins [render][post_processing]")
{
    SceneBoundary boundary;
    boundary.Mark(80);
    boundary.Mark(150);

    CHECK(boundary.Split(200, true).sceneEnd == 80);
}

TEST_CASE("a boundary with nothing after it leaves an empty UI [render][post_processing]")
{
    SceneBoundary boundary;
    boundary.Mark(200);

    const FrameSplit split = boundary.Split(200, true);

    CHECK(split.postProcessed);
    CHECK(split.sceneEnd == 200);
}

TEST_CASE("a boundary before any command leaves an empty scene [render][post_processing]")
{
    SceneBoundary boundary;
    boundary.Mark(0);

    const FrameSplit split = boundary.Split(50, true);

    CHECK(split.postProcessed);
    CHECK(split.sceneEnd == 0);
}

TEST_CASE("a boundary past the recorded commands ends at the last command [render][post_processing]")
{
    SceneBoundary boundary;
    boundary.Mark(300);

    CHECK(boundary.Split(200, true).sceneEnd == 200);
}

TEST_CASE("reset forgets the previous frame's boundary [render][post_processing]")
{
    SceneBoundary boundary;
    boundary.Mark(120);
    boundary.Reset();

    CHECK_FALSE(boundary.IsMarked());
    CHECK_FALSE(boundary.Split(200, true).postProcessed);

    boundary.Mark(40);
    CHECK(boundary.Split(200, true).sceneEnd == 40);
}

TEST_CASE("additive glow blend modes feed the bloom [render][post_processing]")
{
    using Render::PostProcessing::IsBloomSource;
    CHECK(IsBloomSource(true, mu::BlendMode::Additive, false));
    CHECK(IsBloomSource(true, mu::BlendMode::Glow, false));
    CHECK(IsBloomSource(true, mu::BlendMode::Luminance, false));
}

TEST_CASE("opaque, alpha-blended and darkening draws do not feed the bloom [render][post_processing]")
{
    using Render::PostProcessing::IsBloomSource;
    CHECK_FALSE(IsBloomSource(false, mu::BlendMode::Glow, false));
    CHECK_FALSE(IsBloomSource(true, mu::BlendMode::Alpha, false));
    CHECK_FALSE(IsBloomSource(true, mu::BlendMode::Mixed, false));
    CHECK_FALSE(IsBloomSource(true, mu::BlendMode::Subtract, false));
    CHECK_FALSE(IsBloomSource(true, mu::BlendMode::InverseColor, false));
    CHECK_FALSE(IsBloomSource(true, mu::BlendMode::LightMap, false));
}

TEST_CASE("a draw inside a bloom exclusion (chrome pass, character) never feeds the bloom [render][post_processing]")
{
    using Render::PostProcessing::IsBloomSource;
    CHECK_FALSE(IsBloomSource(true, mu::BlendMode::Glow, true));
    CHECK_FALSE(IsBloomSource(true, mu::BlendMode::Additive, true));
    CHECK_FALSE(IsBloomSource(true, mu::BlendMode::Luminance, true));
}
