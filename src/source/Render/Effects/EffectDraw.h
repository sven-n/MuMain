#pragma once

#include <cstdint>

// The seam between game code and how an effect is drawn. The effect passes describe each draw as plain data
// (a SpriteDraw, ...) and hand it to the active EffectDrawer; the default drawer draws it as the game always has.
// Another drawer can change how effect draws are carried out without touching the code that asks for them.
namespace Render::EffectDrawing
{
// A render-state preset: one of the legacy Enable* wrappers, or none, which keeps the state the previous draw left.
enum class Blend : std::uint8_t
{
    Unchanged,
    AlphaTest,
    // Alpha test that leaves the depth mask as it is (EnableAlphaTest(false)).
    AlphaTestKeepDepthMask,
    Glow,
    Subtract,
    Blend2,
    Blend3,
};

enum class Toggle : std::uint8_t
{
    Unchanged,
    On,
    Off,
};

// Render state set before a draw, or on its own. Unchanged parts keep what the previous draws left, as the
// effect passes always relied on.
struct EffectState
{
    Blend blend = Blend::Unchanged;
    Toggle depthTest = Toggle::Unchanged;
    // Adds the texture to the vertex color instead of multiplying (GL_ADD instead of GL_MODULATE).
    Toggle additiveTexture = Toggle::Unchanged;
};

// One camera-facing textured quad.
struct SpriteDraw
{
    int texture = 0;
    float position[3] = {};
    float width = 0.f;
    float height = 0.f;
    float light[3] = {};
    // Degrees around the view axis.
    float rotation = 0.f;
    float u = 0.f;
    float v = 0.f;
    float uWidth = 1.f;
    float vHeight = 1.f;
    EffectState state;
    // Drawn inside a bloom exclusion, so post-processing leaves it alone.
    bool bloomExcluded = false;
};

class EffectDrawer
{
public:
    virtual ~EffectDrawer() = default;
    // State the following draws inherit, set without drawing.
    virtual void SetState(const EffectState& state) = 0;
    virtual void DrawSprite(const SpriteDraw& draw) = 0;
};

[[nodiscard]] EffectDrawer& ActiveEffectDrawer();
// nullptr restores the default drawer. The drawer must outlive its use.
void SetEffectDrawer(EffectDrawer* drawer);
} // namespace Render::EffectDrawing
