#pragma once

#include <cstdint>

// The legacy render-state calls (Render/Textures/ZzzOpenglUtil.h) as data. Each draw inherits the state the calls and
// the draws before it left, so a drawer that draws later records the calls in order with its draws.
namespace Render::Drawing
{
enum class StateCall : std::uint8_t
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
};

class StateCallRecorder
{
public:
    virtual ~StateCallRecorder() = default;
    virtual void RecordStateCall(StateCall call, int argument) = 0;
};

// While a recorder is set, the legacy state calls are recorded instead of made; nullptr makes them again.
void SetStateCallRecorder(StateCallRecorder* recorder);
[[nodiscard]] StateCallRecorder* ActiveStateCallRecorder();
// Makes a recorded call, or records it again while a recorder is set.
void MakeStateCall(StateCall call, int argument);
} // namespace Render::Drawing
