#include <doctest.h>

#include "Render/Renderer/ShadowPassSelection.h"

using namespace Render::ShadowPass;

namespace
{
DrawTraits SolidWorldDraw()
{
    DrawTraits draw;
    draw.worldGeometry = true;
    draw.depthTest = true;
    draw.depthWrite = true;
    return draw;
}

// What EnableAlphaTest leaves: alpha blending on, the alpha test on, depth written.
DrawTraits AlphaTestedDraw()
{
    DrawTraits draw = SolidWorldDraw();
    draw.blended = true;
    draw.blendMode = mu::BlendMode::Alpha;
    draw.alphaTest = true;
    return draw;
}
} // namespace

TEST_CASE("an alpha-tested static mesh writing depth is a cutout in both passes [render][shadow_pass]")
{
    const DrawTraits fence = AlphaTestedDraw();
    CHECK(IsCutout(fence));
    CHECK(CoverageOf(fence) == SunCoverage::Solid);
    CHECK(CastsSunShadow(fence));
    CHECK(WritesSceneDepth(fence));
}

TEST_CASE("an alpha-to-coverage character cutout stays in the scene depth and the sun's [render][shadow_pass]")
{
    DrawTraits hair = SolidWorldDraw();
    hair.blended = true;
    hair.blendMode = mu::BlendMode::Alpha;
    hair.alphaToCoverage = true;
    hair.characterPart = true;
    CHECK(WritesSceneDepth(hair));
    CHECK(CastsSunShadow(hair));
    CHECK(CoverageOf(hair) == SunCoverage::Solid);

    // A pipeline that blends nothing is solid already; its coverage changes nothing.
    hair.blended = false;
    CHECK(WritesSceneDepth(hair));
    CHECK(CastsSunShadow(hair));
}

TEST_CASE("genuine translucency is no cutout [render][shadow_pass]")
{
    DrawTraits fading = AlphaTestedDraw();
    fading.translucentVertices = true;
    CHECK_FALSE(IsCutout(fading));
    CHECK_FALSE(CastsSunShadow(fading));
    CHECK_FALSE(WritesSceneDepth(fading));

    DrawTraits glass = SolidWorldDraw();
    glass.blended = true;
    glass.blendMode = mu::BlendMode::Alpha;
    CHECK_FALSE(IsCutout(glass));
    CHECK_FALSE(CastsSunShadow(glass));
    CHECK_FALSE(WritesSceneDepth(glass));

    DrawTraits additive = AlphaTestedDraw();
    additive.blendMode = mu::BlendMode::Additive;
    CHECK_FALSE(IsCutout(additive));
    CHECK_FALSE(CastsSunShadow(additive));
    CHECK_FALSE(WritesSceneDepth(additive));
}

TEST_CASE("an alpha-tested draw that does not write depth is left out [render][shadow_pass]")
{
    DrawTraits sprite = AlphaTestedDraw();
    sprite.depthWrite = false;
    CHECK_FALSE(IsCutout(sprite));
    CHECK_FALSE(CastsSunShadow(sprite));
    CHECK_FALSE(WritesSceneDepth(sprite));
}

TEST_CASE("a character's blended part keeps its dithered sun coverage, out of the depth [render][shadow_pass]")
{
    DrawTraits cape = SolidWorldDraw();
    cape.depthWrite = false;
    cape.blended = true;
    cape.blendMode = mu::BlendMode::Alpha;
    cape.characterPart = true;
    CHECK(CoverageOf(cape) == SunCoverage::Alpha);
    CHECK(CastsSunShadow(cape));
    CHECK_FALSE(WritesSceneDepth(cape));

    DrawTraits aura = cape;
    aura.blendMode = mu::BlendMode::Additive;
    CHECK(CoverageOf(aura) == SunCoverage::Brightness);
    CHECK(CastsSunShadow(aura));

    // A fading character's cutout dithers instead of casting whole.
    DrawTraits fadingHair = AlphaTestedDraw();
    fadingHair.characterPart = true;
    fadingHair.translucentVertices = true;
    CHECK(CoverageOf(fadingHair) == SunCoverage::Alpha);
    CHECK(CastsSunShadow(fadingHair));
    CHECK_FALSE(WritesSceneDepth(fadingHair));
}

TEST_CASE("shadow eligibility leaves the sun pass only, never the scene depth [render][shadow_pass]")
{
    for (DrawTraits draw : {SolidWorldDraw(), AlphaTestedDraw()})
    {
        draw.noSunCaster = true;
        CHECK_FALSE(CastsSunShadow(draw));
        CHECK(WritesSceneDepth(draw));
    }
    DrawTraits wing = SolidWorldDraw();
    wing.depthWrite = false;
    wing.blended = true;
    wing.characterPart = true;
    wing.noSunCaster = true;
    CHECK_FALSE(CastsSunShadow(wing));
}

TEST_CASE("ground shadows, models for the UI and depth-test-off draws are in neither pass [render][shadow_pass]")
{
    DrawTraits shadow = SolidWorldDraw();
    shadow.groundShadow = true;
    DrawTraits uiModel = SolidWorldDraw();
    uiModel.worldGeometry = false;
    DrawTraits noDepth = SolidWorldDraw();
    noDepth.depthTest = false;
    for (const DrawTraits& draw : {shadow, uiModel, noDepth})
    {
        CHECK_FALSE(CastsSunShadow(draw));
        CHECK_FALSE(WritesSceneDepth(draw));
    }
}
