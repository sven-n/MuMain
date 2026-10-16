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
