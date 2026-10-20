#include <cstdint>
#include <vector>

#include <doctest.h>

#include "Render/Textures/UpscaleMarker.h"

namespace
{
namespace Upscale = Render::Textures::Upscale;

constexpr int TextureSize = 256;

std::vector<std::uint8_t> MarkedFile(std::uint8_t scale)
{
    return {'M', 'U', 'X', scale, 0, 0, 0, 0};
}

int ReadScale(const std::vector<std::uint8_t>& file, int textureSize = TextureSize)
{
    return Upscale::ReadScale(file, textureSize, textureSize);
}
} // namespace

TEST_CASE("a marked file has the scale of its marker [render][upscale_marker]")
{
    CHECK(ReadScale(MarkedFile(2)) == 2);
    CHECK(ReadScale(MarkedFile(4)) == 4);
}

TEST_CASE("an original file has no scale [render][upscale_marker]")
{
    // An original OZJ file starts with the start of its JPEG data.
    CHECK(ReadScale({0xFF, 0xD8, 0xFF, 0xE0, 0, 0, 0, 0}) == Upscale::NoScale);
    // An original OZT file starts with the start of its TGA header.
    CHECK(ReadScale({0, 0, 2, 0, 0, 0, 0, 0}) == Upscale::NoScale);
}

TEST_CASE("a marker with another scale is ignored [render][upscale_marker]")
{
    CHECK(ReadScale(MarkedFile(0)) == Upscale::NoScale);
    CHECK(ReadScale(MarkedFile(1)) == Upscale::NoScale);
    CHECK(ReadScale(MarkedFile(3)) == Upscale::NoScale);
    CHECK(ReadScale(MarkedFile(8)) == Upscale::NoScale);
}

TEST_CASE("a file shorter than the marker has no scale [render][upscale_marker]")
{
    CHECK(ReadScale({}) == Upscale::NoScale);
    CHECK(ReadScale({'M', 'U', 'X'}) == Upscale::NoScale);
}

TEST_CASE("a texture smaller than the scale has no scale [render][upscale_marker]")
{
    CHECK(ReadScale(MarkedFile(4), 2) == Upscale::NoScale);
    CHECK(ReadScale(MarkedFile(4), 4) == 4);
}

TEST_CASE("an upscaled image shrinks to the average of each block [render][upscale_marker]")
{
    // 4x2 RGB at scale 2: two blocks of 2x2 pixels.
    const std::vector<std::uint8_t> pixels{
        0, 10, 20, 4,  10, 20, /**/ 100, 0, 255, 100, 0, 255, //
        8, 10, 20, 12, 10, 20, /**/ 100, 0, 255, 104, 0, 255, //
    };

    const std::vector<std::uint8_t> original = Upscale::ShrinkToOriginal(pixels, 4, 2, 3, 2);

    CHECK(original == std::vector<std::uint8_t>{6, 10, 20, 101, 0, 255});
}

TEST_CASE("the marked scale needs no image size [render][upscale_marker]")
{
    CHECK(Upscale::ReadMarkedScale(MarkedFile(4)) == 4);
    CHECK(Upscale::ReadMarkedScale(MarkedFile(3)) == Upscale::NoScale);
    CHECK(Upscale::ReadMarkedScale(std::vector<std::uint8_t>{0xFF, 0xD8, 0xFF, 0xE0}) == Upscale::NoScale);
}
