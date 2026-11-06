#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace Render::Textures
{

// One mip level of an RGBA8 image, rows top to bottom, 4 bytes per texel.
struct MipLevel
{
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::vector<std::uint8_t> rgba;
};

// Number of levels of a full chain down to 1x1, level 0 included:
// floor(log2(max(width, height))) + 1, or 0 for an empty image.
[[nodiscard]] std::uint32_t CountMipLevels(std::uint32_t width, std::uint32_t height);

// Halves an RGBA8 image with a box filter. Each axis becomes max(1, size / 2),
// the size rule the GPU uses for mip levels; when a size is odd, the last
// texel of that axis also averages the leftover row or column.
[[nodiscard]] MipLevel DownsampleBox(std::span<const std::uint8_t> rgba, std::uint32_t width, std::uint32_t height);

// Builds levels 1 to CountMipLevels - 1 of an RGBA8 image; level 0 is the
// input itself and is not copied. Returns nothing for a 1x1 image or when the
// input is smaller than width * height * 4 bytes.
[[nodiscard]] std::vector<MipLevel> BuildMipChain(std::span<const std::uint8_t> rgba, std::uint32_t width,
                                                  std::uint32_t height);

// Reference of the alpha test the game enables for world drawing
// (EnableAlphaTest: GL_GREATER 0.25); the shader discards alpha <= reference.
inline constexpr float AlphaTestReference = 0.25f;

// An alpha test `alpha / 255 > reference`, held as the smallest 8-bit alpha
// that passes it.
struct AlphaTest
{
    std::uint8_t minPassingAlpha = 0;

    [[nodiscard]] static AlphaTest FromReference(float reference);
};

// Fraction of texels that pass the alpha test; 0 for an empty or too small
// input.
[[nodiscard]] float AlphaCoverage(std::span<const std::uint8_t> rgba, std::uint32_t width, std::uint32_t height,
                                  AlphaTest alphaTest);

// Rescales the alpha of one level so that round(coverage * texel count) texels
// pass the alpha test, keeping alpha-tested surfaces as dense as level 0 at a
// distance. Alpha is multiplied by one factor; texels that share the alpha
// value at the pass boundary are split by an ordered dither, nudging their
// alpha by at most one step across the boundary. Colour is not changed and a
// level that already has that many passing texels is left as it is.
void PreserveAlphaCoverage(MipLevel& level, float coverage, AlphaTest alphaTest);

// Builds the chain like BuildMipChain and, for each level, applies
// PreserveAlphaCoverage with level 0's coverage. Every level is downsampled
// from the unscaled level above it, so rescaling does not compound.
[[nodiscard]] std::vector<MipLevel> BuildCoveragePreservingMipChain(std::span<const std::uint8_t> rgba,
                                                                    std::uint32_t width, std::uint32_t height,
                                                                    AlphaTest alphaTest);

} // namespace Render::Textures
