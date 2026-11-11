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

int ReadMarkedScale(std::span<const std::uint8_t> file)
{
    if (!HasMarker(file))
    {
        return NoScale;
    }

    const int scale = file[ScaleOffset];
    return IsSupportedScale(scale) ? scale : NoScale;
}

int ReadScale(std::span<const std::uint8_t> file, int textureWidth, int textureHeight)
{
    const int scale = ReadMarkedScale(file);
    if (textureWidth < scale || textureHeight < scale)
    {
        return NoScale;
    }
    return scale;
}

std::vector<std::uint8_t> ShrinkToOriginal(std::span<const std::uint8_t> pixels, int width, int height,
                                           int bytesPerPixel, int scale)
{
    const int originalWidth = width / scale;
    const int originalHeight = height / scale;
    const int blockPixels = scale * scale;
    std::vector<std::uint8_t> original(static_cast<std::size_t>(originalWidth) * originalHeight * bytesPerPixel);

    for (int y = 0; y < originalHeight; ++y)
    {
        for (int x = 0; x < originalWidth; ++x)
        {
            for (int channel = 0; channel < bytesPerPixel; ++channel)
            {
                int sum = 0;
                for (int blockY = 0; blockY < scale; ++blockY)
                {
                    const std::size_t row = static_cast<std::size_t>(y * scale + blockY) * width;
                    for (int blockX = 0; blockX < scale; ++blockX)
                    {
                        sum += pixels[(row + x * scale + blockX) * bytesPerPixel + channel];
                    }
                }
                original[(static_cast<std::size_t>(y) * originalWidth + x) * bytesPerPixel + channel] =
                    static_cast<std::uint8_t>(sum / blockPixels);
            }
        }
    }
    return original;
}
} // namespace Render::Textures::Upscale
