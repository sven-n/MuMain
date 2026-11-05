#include "AlphaTestHoles.h"

#include <cstddef>

namespace Render::Textures
{
namespace
{
constexpr std::size_t BytesPerPixel = 4;
constexpr std::size_t AlphaOffset = 3;
} // namespace

bool HasAlphaTestHoles(std::span<const std::uint8_t> rgba, int width, int height, int rowPixels)
{
    if (width <= 0 || height <= 0 || rowPixels < width)
    {
        return false;
    }
    const std::size_t rowBytes = static_cast<std::size_t>(rowPixels) * BytesPerPixel;
    const std::size_t imageBytes = static_cast<std::size_t>(width) * BytesPerPixel;
    if (rgba.size() < rowBytes * static_cast<std::size_t>(height - 1) + imageBytes)
    {
        return false;
    }
    for (int y = 0; y < height; ++y)
    {
        const std::span<const std::uint8_t> row = rgba.subspan(static_cast<std::size_t>(y) * rowBytes, imageBytes);
        for (std::size_t alpha = AlphaOffset; alpha < row.size(); alpha += BytesPerPixel)
        {
            if (row[alpha] <= AlphaTestDropsAtOrBelow)
            {
                return true;
            }
        }
    }
    return false;
}
} // namespace Render::Textures
