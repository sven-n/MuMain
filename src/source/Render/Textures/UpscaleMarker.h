#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

// Upscaled art: an OZJ or OZT file whose image is 2 or 4 times the size the
// game was written against. Such a file starts with 'M' 'U' 'X' and the scale,
// in the first 4 bytes, which no loader reads (an original file repeats the
// start of its JPEG or TGA data there). The game uploads the whole image but
// keeps working with the original size, so terrain UVs, sprite sizes and UI
// coordinates stay valid.
namespace Render::Textures::Upscale
{
inline constexpr std::size_t MarkerSize = 4;
inline constexpr int NoScale = 1;

// The scale of the image in a file: 2 or 4 for a marked file, 1 for any other.
// Also 1 when the texture is smaller than the scale, as it then has no
// original size.
int ReadScale(std::span<const std::uint8_t> file, int textureWidth, int textureHeight);

// An upscaled image brought back to the original size, each pixel the average
// of a scale x scale block. For a texture the game draws on through its pixels,
// which are laid out for the original size. Width and height are multiples of
// the scale.
std::vector<std::uint8_t> ShrinkToOriginal(std::span<const std::uint8_t> pixels, int width, int height,
                                           int bytesPerPixel, int scale);
} // namespace Render::Textures::Upscale
