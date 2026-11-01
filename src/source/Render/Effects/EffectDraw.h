#pragma once

#include <cstdint>

// The seam between game code and how an effect is drawn. The effect passes describe each draw as plain data
// (a SpriteDraw, ...) and hand it to the active EffectDrawer; the default drawer draws it as the game always has.
// Another drawer can change how effect draws are carried out without touching the code that asks for them.
namespace Render::EffectDrawing
{
// The render-state preset a draw sets first: one of the legacy Enable* wrappers, or none, which keeps the state
// the previous draw left.
enum class Blend : std::uint8_t
{
    Unchanged,
    AlphaTest,
    Glow,
    Subtract,
    Blend2,
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
    Blend blend = Blend::Unchanged;
    // Drawn inside a bloom exclusion, so post-processing leaves it alone.
    bool bloomExcluded = false;
};

class EffectDrawer
{
public:
    virtual ~EffectDrawer() = default;
    virtual void DrawSprite(const SpriteDraw& draw) = 0;
};

[[nodiscard]] EffectDrawer& ActiveEffectDrawer();
// nullptr restores the default drawer. The drawer must outlive its use.
void SetEffectDrawer(EffectDrawer* drawer);
} // namespace Render::EffectDrawing
