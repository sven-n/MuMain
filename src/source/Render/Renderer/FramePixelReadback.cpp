#include "FramePixelReadback.h"

#include <cstddef>
#include <limits>
#include <utility>

namespace mu
{

namespace
{

constexpr std::size_t SourceBytesPerPixel = 4;
constexpr std::size_t RgbBytesPerPixel = 3;
constexpr std::size_t GreenChannelOffset = 1;
constexpr std::size_t RgbaRedChannelOffset = 0;
constexpr std::size_t RgbaBlueChannelOffset = 2;
constexpr std::size_t BgraRedChannelOffset = 2;
constexpr std::size_t BgraBlueChannelOffset = 0;

bool TryMultiply(std::size_t left, std::size_t right, std::size_t& product)
{
    if (left != 0 && right > std::numeric_limits<std::size_t>::max() / left)
    {
        return false;
    }

    product = left * right;
    return true;
}

} // namespace

bool ConvertToTopDownRgb(std::span<const std::uint8_t> source,
                         std::uint32_t width,
                         std::uint32_t height,
                         std::uint32_t rowPitch,
                         PixelChannelOrder channelOrder,
                         bool reverseRows,
                         FramePixels& output)
{
    if (width == 0 || height == 0)
    {
        return false;
    }

    const std::size_t pixelWidth = width;
    const std::size_t pixelHeight = height;
    const std::size_t sourceRowPitch = rowPitch;

    std::size_t sourceRowBytes = 0;
    if (!TryMultiply(pixelWidth, SourceBytesPerPixel, sourceRowBytes) || sourceRowPitch < sourceRowBytes)
    {
        return false;
    }

    std::size_t sourceByteCount = 0;
    if (!TryMultiply(sourceRowPitch, pixelHeight, sourceByteCount) || source.size() < sourceByteCount)
    {
        return false;
    }

    std::size_t rgbRowBytes = 0;
    if (!TryMultiply(pixelWidth, RgbBytesPerPixel, rgbRowBytes))
    {
        return false;
    }

    std::size_t rgbByteCount = 0;
    if (!TryMultiply(rgbRowBytes, pixelHeight, rgbByteCount))
    {
        return false;
    }

    std::size_t redChannelOffset = 0;
    std::size_t blueChannelOffset = 0;
    switch (channelOrder)
    {
    case PixelChannelOrder::Rgba:
        redChannelOffset = RgbaRedChannelOffset;
        blueChannelOffset = RgbaBlueChannelOffset;
        break;
    case PixelChannelOrder::Bgra:
        redChannelOffset = BgraRedChannelOffset;
        blueChannelOffset = BgraBlueChannelOffset;
        break;
    default:
        return false;
    }

    FramePixels converted;
    converted.width = width;
    converted.height = height;
    converted.rgb.resize(rgbByteCount);

    for (std::size_t destinationRow = 0; destinationRow < pixelHeight; ++destinationRow)
    {
        const std::size_t sourceRow = reverseRows ? pixelHeight - destinationRow - 1 : destinationRow;
        const std::size_t sourceOffset = sourceRow * sourceRowPitch;
        const std::size_t destinationOffset = destinationRow * rgbRowBytes;

        for (std::size_t pixel = 0; pixel < pixelWidth; ++pixel)
        {
            const std::size_t sourcePixelOffset = sourceOffset + pixel * SourceBytesPerPixel;
            const std::size_t destinationPixelOffset = destinationOffset + pixel * RgbBytesPerPixel;

            converted.rgb[destinationPixelOffset] = source[sourcePixelOffset + redChannelOffset];
            converted.rgb[destinationPixelOffset + GreenChannelOffset] = source[sourcePixelOffset + GreenChannelOffset];
            converted.rgb[destinationPixelOffset + RgbaBlueChannelOffset] = source[sourcePixelOffset + blueChannelOffset];
        }
    }

    output = std::move(converted);
    return true;
}

bool FrameReadbackState::Request()
{
    if (state_ != State::Idle)
    {
        return false;
    }

    state_ = State::Pending;
    return true;
}

bool FrameReadbackState::IsPending() const
{
    return state_ == State::Pending;
}

void FrameReadbackState::Complete(FramePixels pixels)
{
    if (state_ != State::Pending || pixels.rgb.empty())
    {
        return;
    }

    pixels_ = std::move(pixels);
    state_ = State::Completed;
}

void FrameReadbackState::Fail()
{
    Reset();
}

FramePixels FrameReadbackState::Consume()
{
    if (state_ != State::Completed)
    {
        return {};
    }

    FramePixels pixels = std::move(pixels_);
    Reset();
    return pixels;
}

void FrameReadbackState::Reset()
{
    pixels_ = {};
    state_ = State::Idle;
}

} // namespace mu
