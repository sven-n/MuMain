#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

// Render scale and sharpening (FSR 1): the 3D scene is drawn at a share of the window's size, then
// upscaled to it with AMD's edge-adaptive upsampling (EASU) and sharpened (RCAS), before the UI,
// which stays at the window's size. The sharpening also works alone, at 100 %. Player settings of
// the Enhanced look: `[Render] RenderScale` and `Sharpness`.
namespace Render::PostProcessing
{

// The render scale's range in percent of the window's size; 100 draws at the window's size.
inline constexpr int MinRenderScalePercent = 50;
inline constexpr int MaxRenderScalePercent = 100;
// The sharpness's range in percent; 0 is no sharpening.
inline constexpr int MinSharpnessPercent = 0;
inline constexpr int MaxSharpnessPercent = 100;
// The sharpening's strength in stops below the strongest, at the lowest sharpness above 0 (FSR's
// own scale: each stop halves it).
inline constexpr float WeakestSharpnessStops = 2.0f;

struct FrameSize
{
    std::uint32_t width = 0u;
    std::uint32_t height = 0u;

    friend bool operator==(const FrameSize&, const FrameSize&) = default;
};

// A configured render scale within the range; a value outside it is the nearest end.
[[nodiscard]] int ClampRenderScale(int percent);
// A configured sharpness within the range; a value outside it is the nearest end.
[[nodiscard]] int ClampSharpness(int percent);

// The size the 3D scene is drawn at for a window of `window` at `percent`, rounded, at least 1x1.
[[nodiscard]] FrameSize ScaledSize(FrameSize window, int percent);

// A viewport in pixels, and a scissor in whole pixels.
struct ViewRect
{
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};
struct PixelRect
{
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

// A viewport or scissor recorded for a target of `from`, for the same place on one of `to`. A scissor
// grows to whole pixels, so it never clips what it let through before.
[[nodiscard]] ViewRect ScaleViewRect(ViewRect rect, FrameSize from, FrameSize to);
[[nodiscard]] PixelRect ScalePixelRect(PixelRect rect, FrameSize from, FrameSize to);

// The passes between the composite and the UI, in this order; the last draws into the UI pass.
struct FinalPassPlan
{
    bool edgeSmoothing = false; // FXAA, at the render size
    bool upscale = false;       // EASU, from the render size to the window's
    bool sharpen = false;       // RCAS, at the window's size

    [[nodiscard]] bool Any() const
    {
        return edgeSmoothing || upscale || sharpen;
    }
};

// The passes a frame takes. A scene drawn smaller than the window is upscaled and, if the player
// chose a sharpness, sharpened; at the window's size only a sharpness above 0 sharpens.
[[nodiscard]] FinalPassPlan PlanFinalPasses(bool edgeSmoothing, FrameSize scene, FrameSize window,
                                            int sharpnessPercent);

// RCAS's sharpness for `percent` (above 0): linear, 1 at 100 %, WeakestSharpnessStops stops below
// it near 0 %.
[[nodiscard]] float RcasSharpness(int percent);

// The constants of FSR's EASU (FsrEasuCon) for upscaling an image of `input` to `output`, as the four
// float4 of post_fsr_easu.frag.hlsl's EasuUniforms.
[[nodiscard]] std::array<float, 16> EasuConstants(FrameSize input, FrameSize output);

// The argument of `$renderscale` (50 to 100) or `$sharpness` (0 to 100): a whole number in the
// range, surrounding spaces ignored. nullopt for anything else.
[[nodiscard]] std::optional<int> ParseRenderScaleCommand(std::wstring_view argument);
[[nodiscard]] std::optional<int> ParseSharpnessCommand(std::wstring_view argument);

} // namespace Render::PostProcessing
