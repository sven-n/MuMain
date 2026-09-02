#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace mu
{

struct FramePixels
{
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::vector<std::uint8_t> rgb;
};

enum class PixelChannelOrder
{
    Rgba,
    Bgra,
};

[[nodiscard]] bool ConvertToTopDownRgb(std::span<const std::uint8_t> source,
                                       std::uint32_t width,
                                       std::uint32_t height,
                                       std::uint32_t rowPitch,
                                       PixelChannelOrder channelOrder,
                                       bool reverseRows,
                                       FramePixels& output);

class FrameReadbackState
{
public:
    [[nodiscard]] bool Request();
    [[nodiscard]] bool IsPending() const;
    void Complete(FramePixels pixels);
    void Fail();
    [[nodiscard]] FramePixels Consume();
    void Reset();

private:
    enum class State : std::uint8_t
    {
        Idle,
        Pending,
        Completed,
    };

    State state_ = State::Idle;
    FramePixels pixels_;
};

} // namespace mu
