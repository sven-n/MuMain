#include <doctest.h>

#include "Render/PostProcessing/RenderScale.h"

using Render::PostProcessing::FrameSize;

namespace
{
constexpr FrameSize Window{1920u, 1080u};
} // namespace

TEST_CASE("the render scale and sharpness stay in their ranges [render][render-scale]")
{
    using namespace Render::PostProcessing;
    CHECK(ClampRenderScale(10) == MinRenderScalePercent);
    CHECK(ClampRenderScale(75) == 75);
    CHECK(ClampRenderScale(150) == MaxRenderScalePercent);
    CHECK(ClampSharpness(-5) == MinSharpnessPercent);
    CHECK(ClampSharpness(40) == 40);
    CHECK(ClampSharpness(101) == MaxSharpnessPercent);
}

TEST_CASE("the scene is drawn at the render scale's share of the window [render][render-scale]")
{
    using Render::PostProcessing::ScaledSize;
    CHECK(ScaledSize(Window, 100) == Window);
    CHECK(ScaledSize(Window, 50) == FrameSize{960u, 540u});
    CHECK(ScaledSize(Window, 67) == FrameSize{1286u, 724u});
    // Below the range it is the range's end; never smaller than one pixel.
    CHECK(ScaledSize(Window, 10) == FrameSize{960u, 540u});
    CHECK(ScaledSize(FrameSize{1u, 1u}, 50) == FrameSize{1u, 1u});
}

TEST_CASE("only a smaller scene is upscaled, only a sharpness above 0 sharpens [render][render-scale]")
{
    using Render::PostProcessing::PlanFinalPasses;
    const auto none = PlanFinalPasses(false, Window, Window, 0);
    CHECK_FALSE(none.Any());

    const auto scaled = PlanFinalPasses(false, FrameSize{960u, 540u}, Window, 0);
    CHECK(scaled.upscale);
    CHECK_FALSE(scaled.sharpen);
    CHECK_FALSE(scaled.edgeSmoothing);

    const auto sharpened = PlanFinalPasses(true, Window, Window, 30);
    CHECK(sharpened.edgeSmoothing);
    CHECK_FALSE(sharpened.upscale);
    CHECK(sharpened.sharpen);
}

TEST_CASE("the sharpness maps onto FSR's stops [render][render-scale]")
{
    using Render::PostProcessing::RcasSharpness;
    CHECK(RcasSharpness(100) == doctest::Approx(1.0f));
    CHECK(RcasSharpness(50) == doctest::Approx(0.5f));
    CHECK(RcasSharpness(0) == doctest::Approx(0.25f));
}

TEST_CASE("the upscale constants match FSR's for a whole-image viewport [render][render-scale]")
{
    const auto constants = Render::PostProcessing::EasuConstants(FrameSize{960u, 540u}, Window);
    CHECK(constants[0] == doctest::Approx(0.5f));
    CHECK(constants[1] == doctest::Approx(0.5f));
    CHECK(constants[2] == doctest::Approx(-0.25f));
    CHECK(constants[3] == doctest::Approx(-0.25f));
    CHECK(constants[4] == doctest::Approx(1.0f / 960.0f));
    CHECK(constants[7] == doctest::Approx(-1.0f / 540.0f));
    CHECK(constants[9] == doctest::Approx(2.0f / 540.0f));
    CHECK(constants[13] == doctest::Approx(4.0f / 540.0f));
}

TEST_CASE("views recorded for the window are scaled to the scene [render][render-scale]")
{
    using namespace Render::PostProcessing;
    const FrameSize scene{960u, 540u};
    const ViewRect view = ScaleViewRect({100.0f, 50.0f, 800.0f, 600.0f}, Window, scene);
    CHECK(view.x == doctest::Approx(50.0f));
    CHECK(view.y == doctest::Approx(25.0f));
    CHECK(view.width == doctest::Approx(400.0f));
    CHECK(view.height == doctest::Approx(300.0f));

    // An odd scissor grows to whole pixels instead of clipping what it let through.
    const PixelRect scissor = ScalePixelRect({101, 51, 99, 99}, Window, scene);
    CHECK(scissor.x == 50);
    CHECK(scissor.y == 25);
    CHECK(scissor.width == 50);
    CHECK(scissor.height == 50);

    const PixelRect whole = ScalePixelRect({0, 0, 1920, 1080}, Window, Window);
    CHECK(whole.width == 1920);
    CHECK(whole.height == 1080);
}

TEST_CASE("$renderscale and $sharpness take a whole number in range [render][render-scale]")
{
    using namespace Render::PostProcessing;
    CHECK(ParseRenderScaleCommand(L"75") == 75);
    CHECK(ParseRenderScaleCommand(L" 50 ") == 50);
    CHECK_FALSE(ParseRenderScaleCommand(L"49").has_value());
    CHECK_FALSE(ParseRenderScaleCommand(L"101").has_value());
    CHECK_FALSE(ParseRenderScaleCommand(L"abc").has_value());
    CHECK(ParseSharpnessCommand(L"0") == 0);
    CHECK(ParseSharpnessCommand(L"100") == 100);
    CHECK_FALSE(ParseSharpnessCommand(L"-1").has_value());
}
