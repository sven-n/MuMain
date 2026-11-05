// ShadowPassSelection.h: which recorded draws the sun shadow map and the scene's depth replay, and how they cover.
#pragma once

#include "Render/Renderer/MuRenderer.h"

#include <cstdint>

namespace Render::ShadowPass
{

// What the passes read of a recorded draw.
struct DrawTraits
{
    // A 3D draw of the scene's world: not 2D, not a model drawn for the UI.
    bool worldGeometry = false;
    bool groundShadow = false;
    bool depthTest = false;
    bool depthWrite = false;
    bool blended = false;
    mu::BlendMode blendMode = mu::BlendMode::Alpha;
    bool alphaTest = false;
    bool alphaToCoverage = false;
    // Below full alpha before the texture: a fading object, not a cutout.
    bool translucentVertices = false;
    // A skinned draw recorded inside a character scope.
    bool characterPart = false;
    // Recorded inside a ScopedNoSunCasters.
    bool noSunCaster = false;
};

// How a draw covers the sun shadow map (sun_depth.frag's coverageMode is this less one): solid with its alpha test,
// or a blended character part dithered by its alpha or, glowing, its brightness. Darkening blends cover nothing.
enum class SunCoverage : std::uint32_t
{
    None,
    Solid,
    Alpha,
    Brightness,
};

// A draw with hard-edged alpha that writes depth: solid where its texels pass, though the pipeline state blends.
[[nodiscard]] constexpr bool IsCutout(const DrawTraits& draw)
{
    return draw.blended && draw.blendMode == mu::BlendMode::Alpha && draw.depthWrite && !draw.translucentVertices &&
           (draw.alphaTest || draw.alphaToCoverage);
}

[[nodiscard]] constexpr SunCoverage CoverageOf(const DrawTraits& draw)
{
    if (!draw.blended || IsCutout(draw))
    {
        return SunCoverage::Solid;
    }
    switch (draw.blendMode)
    {
    case mu::BlendMode::Alpha:
    case mu::BlendMode::Mixed:
        return SunCoverage::Alpha;
    case mu::BlendMode::Additive:
    case mu::BlendMode::Glow:
    case mu::BlendMode::Luminance:
        return SunCoverage::Brightness;
    default:
        return SunCoverage::None;
    }
}

[[nodiscard]] constexpr bool IsSolidWorldDraw(const DrawTraits& draw)
{
    return draw.worldGeometry && !draw.groundShadow && draw.depthTest && draw.depthWrite &&
           CoverageOf(draw) == SunCoverage::Solid;
}

[[nodiscard]] constexpr bool CastsSunShadow(const DrawTraits& draw)
{
    if (draw.noSunCaster)
    {
        return false;
    }
    const bool blendedPart = draw.worldGeometry && !draw.groundShadow && draw.depthTest && draw.characterPart &&
                             draw.blended && CoverageOf(draw) != SunCoverage::None;
    return IsSolidWorldDraw(draw) || blendedPart;
}

// What a multisampled frame replays for ambient occlusion: every solid draw, whichever casts a shadow.
[[nodiscard]] constexpr bool WritesSceneDepth(const DrawTraits& draw)
{
    return IsSolidWorldDraw(draw);
}

} // namespace Render::ShadowPass
