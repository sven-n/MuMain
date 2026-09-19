#include "DiagnosticFrameCaptureWriter.h"

#include "Render/Renderer/FramePixelReadback.h"

#include <cstddef>
#include <fstream>
#include <limits>

namespace mu
{

bool WriteDiagnosticFrameCapturePpm(const char* path, const FramePixels& pixels)
{
    constexpr std::size_t RgbBytesPerPixel = 3;

    if (path == nullptr || path[0] == '\0' || pixels.width == 0 || pixels.height == 0)
    {
        return false;
    }

    const std::size_t pixelWidth = pixels.width;
    const std::size_t pixelHeight = pixels.height;
    if (pixelWidth > std::numeric_limits<std::size_t>::max() / RgbBytesPerPixel)
    {
        return false;
    }

    const std::size_t rgbRowBytes = pixelWidth * RgbBytesPerPixel;
    if (pixelHeight > std::numeric_limits<std::size_t>::max() / rgbRowBytes)
    {
        return false;
    }

    const std::size_t expectedBytes = rgbRowBytes * pixelHeight;
    if (pixels.rgb.size() != expectedBytes ||
        expectedBytes > static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max()))
    {
        return false;
    }

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open())
    {
        return false;
    }

    file << "P6\n" << pixels.width << ' ' << pixels.height << "\n255\n";
    file.write(reinterpret_cast<const char*>(pixels.rgb.data()), static_cast<std::streamsize>(expectedBytes));
    file.close();
    return file.good();
}

} // namespace mu
