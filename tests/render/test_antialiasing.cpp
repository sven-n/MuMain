#include <doctest.h>

#include "Render/Renderer/Antialiasing.h"

namespace
{
constexpr Render::Antialiasing::SupportedSampleCounts AllCounts{true, true, true};
constexpr Render::Antialiasing::SupportedSampleCounts UpToFour{true, true, false};
constexpr Render::Antialiasing::SupportedSampleCounts NoMultisampling{};
} // namespace

TEST_CASE("configured anti-aliasing values map to sample counts [render][antialiasing]")
{
    using Render::Antialiasing::RequestedSampleCount;
    CHECK(RequestedSampleCount(0) == 1);
    CHECK(RequestedSampleCount(1) == 1);
    CHECK(RequestedSampleCount(2) == 2);
    CHECK(RequestedSampleCount(4) == 4);
    CHECK(RequestedSampleCount(8) == 8);
}

TEST_CASE("configured anti-aliasing values between the steps round down [render][antialiasing]")
{
    using Render::Antialiasing::RequestedSampleCount;
    CHECK(RequestedSampleCount(-4) == 1);
    CHECK(RequestedSampleCount(3) == 2);
    CHECK(RequestedSampleCount(6) == 4);
    CHECK(RequestedSampleCount(16) == 8);
}

TEST_CASE("$msaa accepts off, 2, 4 and 8 [render][antialiasing]")
{
    using Render::Antialiasing::ParseCommandArgument;
    CHECK(ParseCommandArgument(L"off") == 1);
    CHECK(ParseCommandArgument(L"OFF") == 1);
    CHECK(ParseCommandArgument(L"0") == 1);
    CHECK(ParseCommandArgument(L"1") == 1);
    CHECK(ParseCommandArgument(L"2") == 2);
    CHECK(ParseCommandArgument(L"4") == 4);
    CHECK(ParseCommandArgument(L" 8 ") == 8);
}

TEST_CASE("$msaa rejects anything else [render][antialiasing]")
{
    using Render::Antialiasing::ParseCommandArgument;
    CHECK_FALSE(ParseCommandArgument(L"").has_value());
    CHECK_FALSE(ParseCommandArgument(L"  ").has_value());
    CHECK_FALSE(ParseCommandArgument(L"3").has_value());
    CHECK_FALSE(ParseCommandArgument(L"16").has_value());
    CHECK_FALSE(ParseCommandArgument(L"on").has_value());
    CHECK_FALSE(ParseCommandArgument(L"4x").has_value());
}

TEST_CASE("a supported request is used as asked [render][antialiasing]")
{
    using Render::Antialiasing::SelectSampleCount;
    CHECK(SelectSampleCount(1, AllCounts) == 1);
    CHECK(SelectSampleCount(2, AllCounts) == 2);
    CHECK(SelectSampleCount(4, AllCounts) == 4);
    CHECK(SelectSampleCount(8, AllCounts) == 8);
}

TEST_CASE("an unsupported request drops to the highest supported count below it [render][antialiasing]")
{
    using Render::Antialiasing::SelectSampleCount;
    CHECK(SelectSampleCount(8, UpToFour) == 4);
    CHECK(SelectSampleCount(4, UpToFour) == 4);
    CHECK(SelectSampleCount(8, {true, false, false}) == 2);
    CHECK(SelectSampleCount(4, {false, false, true}) == 1);
    CHECK(SelectSampleCount(8, NoMultisampling) == 1);
}

TEST_CASE("the highest supported count is reported [render][antialiasing]")
{
    using Render::Antialiasing::HighestSupportedSampleCount;
    CHECK(HighestSupportedSampleCount(AllCounts) == 8);
    CHECK(HighestSupportedSampleCount(UpToFour) == 4);
    CHECK(HighestSupportedSampleCount(NoMultisampling) == 1);
}

namespace
{
// A cutout of the kind EnableAlphaTest draws: alpha test, Alpha blending,
// depth test and write, opaque vertices, Enhanced texture, anti-aliasing on.
Render::Antialiasing::CutoutDraw Cutout()
{
    Render::Antialiasing::CutoutDraw draw{};
    draw.multisampled = true;
    draw.alphaTest = true;
    draw.depthTest = true;
    draw.depthWrite = true;
    draw.blending = Render::Antialiasing::Blending::Alpha;
    draw.opaqueVertices = true;
    draw.enhancedTexture = true;
    return draw;
}
} // namespace

TEST_CASE("an alpha-tested cutout uses alpha-to-coverage [render][antialiasing][alpha-to-coverage]")
{
    using Render::Antialiasing::UsesAlphaToCoverage;
    CHECK(UsesAlphaToCoverage(Cutout()));

    auto unblended = Cutout();
    unblended.blending = Render::Antialiasing::Blending::Off;
    unblended.opaqueVertices = false;
    CHECK(UsesAlphaToCoverage(unblended));
}

TEST_CASE(
    "alpha-to-coverage needs anti-aliasing, the alpha test and depth writes [render][antialiasing][alpha-to-coverage]")
{
    using Render::Antialiasing::UsesAlphaToCoverage;
    auto draw = Cutout();
    draw.multisampled = false;
    CHECK_FALSE(UsesAlphaToCoverage(draw));

    draw = Cutout();
    draw.alphaTest = false;
    CHECK_FALSE(UsesAlphaToCoverage(draw));

    draw = Cutout();
    draw.depthTest = false;
    CHECK_FALSE(UsesAlphaToCoverage(draw));

    draw = Cutout();
    draw.depthWrite = false;
    CHECK_FALSE(UsesAlphaToCoverage(draw));
}

TEST_CASE("fading and other blending keep their blending [render][antialiasing][alpha-to-coverage]")
{
    using Render::Antialiasing::UsesAlphaToCoverage;
    auto fading = Cutout();
    fading.opaqueVertices = false;
    CHECK_FALSE(UsesAlphaToCoverage(fading));

    auto additive = Cutout();
    additive.blending = Render::Antialiasing::Blending::Other;
    CHECK_FALSE(UsesAlphaToCoverage(additive));
}

TEST_CASE("Classic-filtered textures keep the Classic alpha test [render][antialiasing][alpha-to-coverage]")
{
    auto classic = Cutout();
    classic.enhancedTexture = false;
    CHECK_FALSE(Render::Antialiasing::UsesAlphaToCoverage(classic));
}
