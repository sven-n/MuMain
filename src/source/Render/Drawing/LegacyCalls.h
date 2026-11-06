#pragma once

#include <cstdint>

// The legacy render calls that are not draw descriptions, as data: the render-state wrappers
// (Render/Textures/ZzzOpenglUtil.h), the sprite matrix, the reorderable ranges and the whole terrain passes, which
// read data fixed for the frame. Each draw inherits what the calls and the draws before it left, so a drawer that
// draws later records the calls in order with its draws.
namespace Render::Drawing
{
enum class LegacyCall : std::uint8_t
{
    // Argument: the texture.
    BindTexture,
    EnableDepthTest,
    DisableDepthTest,
    EnableDepthMask,
    DisableDepthMask,
    EnableCullFace,
    DisableCullFace,
    // Argument: the alpha test.
    DisableTexture,
    DisableAlphaBlend,
    // Argument: the depth mask.
    EnableAlphaTest,
    EnableAlphaBlend,
    EnableAlphaBlendMinus,
    EnableAlphaBlend2,
    EnableAlphaBlend3,
    EnableAlphaBlend4,
    EnableLightMap,
    // Argument: the threshold's bits.
    SetAlphaFuncRef,
    BeginSprite,
    EndSprite,
    BeginReorderableDraws,
    EndReorderableDraws,
    // Argument: the edit flag.
    RenderTerrain,
    RenderWaterTerrain,
};

class LegacyCallRecorder
{
public:
    virtual ~LegacyCallRecorder() = default;
    virtual void RecordLegacyCall(LegacyCall call, int argument) = 0;
};

// While a recorder is set, the legacy calls are recorded instead of made; nullptr makes them again.
void SetLegacyCallRecorder(LegacyCallRecorder* recorder);
[[nodiscard]] LegacyCallRecorder* ActiveLegacyCallRecorder();
// Records the call when a recorder is set; false: the caller makes it now.
[[nodiscard]] bool RecordsLegacyCall(LegacyCall call, int argument = 0);
// Makes a recorded call, or records it again while a recorder is set.
void MakeLegacyCall(LegacyCall call, int argument);

// The draws in its lifetime may be reordered (mu::ScopedReorderableDraws), recorded with them when drawn later.
class ScopedReorderableDraws
{
public:
    ScopedReorderableDraws()
    {
        MakeLegacyCall(LegacyCall::BeginReorderableDraws, 0);
    }
    ~ScopedReorderableDraws()
    {
        MakeLegacyCall(LegacyCall::EndReorderableDraws, 0);
    }
    ScopedReorderableDraws(const ScopedReorderableDraws&) = delete;
    ScopedReorderableDraws& operator=(const ScopedReorderableDraws&) = delete;
};
} // namespace Render::Drawing
