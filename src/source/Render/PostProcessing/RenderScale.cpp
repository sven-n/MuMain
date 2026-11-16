#include "Render/PostProcessing/RenderScale.h"

#include "Core/Text/NumberParse.h"

#include <algorithm>
#include <cmath>

namespace Render::PostProcessing
{
namespace
{
constexpr float PercentScale = 100.0f;

std::uint32_t ScaledLength(std::uint32_t length, int percent)
{
    const float scaled = std::round(static_cast<float>(length) * static_cast<float>(percent) / PercentScale);
    return std::max(static_cast<std::uint32_t>(scaled), 1u);
}

std::optional<int> ParseInRange(std::wstring_view argument, int minimum, int maximum)
{
    const std::optional<int> value = Core::Text::TryParseInt(Core::Text::TrimWhitespace(argument));
    if (!value || *value < minimum || *value > maximum)
    {
        return std::nullopt;
    }
    return value;
}
} // namespace

int ClampRenderScale(int percent)
{
    return std::clamp(percent, MinRenderScalePercent, MaxRenderScalePercent);
}

int ClampSharpness(int percent)
{
    return std::clamp(percent, MinSharpnessPercent, MaxSharpnessPercent);
}

FrameSize ScaledSize(FrameSize window, int percent)
{
    const int clamped = ClampRenderScale(percent);
    if (clamped == MaxRenderScalePercent)
    {
        return window;
    }
    return {ScaledLength(window.width, clamped), ScaledLength(window.height, clamped)};
}

namespace
{
float Ratio(std::uint32_t to, std::uint32_t from)
{
    return from == 0u ? 1.0f : static_cast<float>(to) / static_cast<float>(from);
}
} // namespace

ViewRect ScaleViewRect(ViewRect rect, FrameSize from, FrameSize to)
{
    const float scaleX = Ratio(to.width, from.width);
    const float scaleY = Ratio(to.height, from.height);
    return {rect.x * scaleX, rect.y * scaleY, rect.width * scaleX, rect.height * scaleY};
}

PixelRect ScalePixelRect(PixelRect rect, FrameSize from, FrameSize to)
{
    const float scaleX = Ratio(to.width, from.width);
    const float scaleY = Ratio(to.height, from.height);
    const int left = static_cast<int>(std::floor(static_cast<float>(rect.x) * scaleX));
    const int top = static_cast<int>(std::floor(static_cast<float>(rect.y) * scaleY));
    const int right = static_cast<int>(std::ceil(static_cast<float>(rect.x + rect.width) * scaleX));
    const int bottom = static_cast<int>(std::ceil(static_cast<float>(rect.y + rect.height) * scaleY));
    return {left, top, right - left, bottom - top};
}

FinalPassPlan PlanFinalPasses(bool edgeSmoothing, FrameSize scene, FrameSize window, int sharpnessPercent)
{
    FinalPassPlan plan;
    plan.edgeSmoothing = edgeSmoothing;
    plan.upscale = scene != window;
    plan.sharpen = ClampSharpness(sharpnessPercent) > MinSharpnessPercent;
    return plan;
}

float RcasSharpness(int percent)
{
    const float share = static_cast<float>(ClampSharpness(percent)) / PercentScale;
    return std::exp2(-WeakestSharpnessStops * (1.0f - share));
}

std::array<float, 16> EasuConstants(FrameSize input, FrameSize output)
{
    const float inputWidth = static_cast<float>(input.width);
    const float inputHeight = static_cast<float>(input.height);
    const float outputWidth = static_cast<float>(output.width);
    const float outputHeight = static_cast<float>(output.height);
    const float texelWidth = 1.0f / inputWidth;
    const float texelHeight = 1.0f / inputHeight;
    // FsrEasuCon with the whole input as its viewport: con0 maps an output pixel to an input
    // position, con1 to con3 are the gather centres, from the upper left of the 'F' tap.
    return {inputWidth / outputWidth,
            inputHeight / outputHeight,
            0.5f * inputWidth / outputWidth - 0.5f,
            0.5f * inputHeight / outputHeight - 0.5f,
            texelWidth,
            texelHeight,
            texelWidth,
            -texelHeight,
            -texelWidth,
            2.0f * texelHeight,
            texelWidth,
            2.0f * texelHeight,
            0.0f,
            4.0f * texelHeight,
            0.0f,
            0.0f};
}

std::optional<int> ParseRenderScaleCommand(std::wstring_view argument)
{
    return ParseInRange(argument, MinRenderScalePercent, MaxRenderScalePercent);
}

std::optional<int> ParseSharpnessCommand(std::wstring_view argument)
{
    return ParseInRange(argument, MinSharpnessPercent, MaxSharpnessPercent);
}

} // namespace Render::PostProcessing
