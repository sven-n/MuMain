#include "UpscaleMarker.h"

namespace Render::Textures::Upscale
{
namespace
{
constexpr std::size_t ScaleOffset = 3;

// A power of two keeps the padding to a power of two in proportion: the padded
// size of the upscaled image is the scale times that of the original.
bool IsSupportedScale(int scale)
{
    return scale == 2 || scale == 4;
}

bool HasMarker(std::span<const std::uint8_t> file)
{
    return file.size() >= MarkerSize && file[0] == 'M' && file[1] == 'U' && file[2] == 'X';
}
} // namespace

int ReadScale(std::span<const std::uint8_t> file, int textureWidth, int textureHeight)
{
    if (!HasMarker(file))
    {
        return NoScale;
    }

    const int scale = file[ScaleOffset];
    if (!IsSupportedScale(scale) || textureWidth < scale || textureHeight < scale)
    {
        return NoScale;
    }
    return scale;
}
} // namespace Render::Textures::Upscale
