#pragma once

#include <optional>
#include <string_view>

// Multisample anti-aliasing: what the `[Render] Antialiasing` setting and the
// `$msaa` command ask for, and which sample count the frame then renders with.
namespace Render::Antialiasing
{

// Samples per pixel: 1 is off; 2, 4 and 8 are the multisampled settings.
inline constexpr int OffSampleCount = 1;
inline constexpr int MaxSampleCount = 8;

// The sample count a configured `[Render] Antialiasing` value asks for: 0 and
// 1 are off, 2, 4 and 8 as given; anything else rounds down to the nearest of
// those (3 is 2, above 8 is 8), and a negative value is off.
[[nodiscard]] int RequestedSampleCount(int configured);

// The argument of `$msaa`: "off", "0", "1", "2", "4" or "8", in any case and
// with surrounding spaces ignored, as a sample count (off is 1). nullopt for
// anything else.
[[nodiscard]] std::optional<int> ParseCommandArgument(std::wstring_view argument);

// The multisampled counts the GPU supports for every attachment format the
// frame uses (the colour target and the depth buffer).
struct SupportedSampleCounts
{
    bool two = false;
    bool four = false;
    bool eight = false;
};

// The highest supported count not above `requested`; 1 (off) when none is.
[[nodiscard]] int SelectSampleCount(int requested, SupportedSampleCounts supported);

// The highest count the GPU supports at all; 1 when it supports none.
[[nodiscard]] int HighestSupportedSampleCount(SupportedSampleCounts supported);

// How a draw blends, as far as alpha-to-coverage is concerned.
enum class Blending : unsigned char
{
    Off,
    Alpha, // source alpha, one minus source alpha
    Other,
};

// The state of a 3D draw that decides whether it is an alpha-tested cutout
// drawn with alpha-to-coverage.
struct CutoutDraw
{
    bool multisampled = false;
    bool alphaTest = false;
    bool depthTest = false;
    bool depthWrite = false;
    Blending blending = Blending::Off;
    // Every vertex colour has alpha 255: the draw is not fading.
    bool opaqueVertices = false;
    // The texture is sampled with the Enhanced (linear, mipmapped) sampler.
    bool enhancedTexture = false;
};

// True for an alpha-tested cutout while anti-aliasing is on: alpha test,
// depth test and depth write on, and no blending or Alpha blending of opaque
// vertices (fading objects keep blending). Only Enhanced-filtered textures
// qualify: linearly filtered alpha has sub-texel edges to turn into coverage,
// while Classic (nearest) alpha is 0 or 1 per texel and keeps the alpha test.
[[nodiscard]] bool UsesAlphaToCoverage(const CutoutDraw& draw);

} // namespace Render::Antialiasing
