#pragma once

#include <array>

// Pure geometry of the full-screen mini map (CMiniMap). The original drew the map texture and its
// markers with RenderBitRotate()/RenderPointRotate() and the side border lines with
// RenderBitmapRotate(): quads rotated in physical window pixels, around the window centre, after
// the Hud layout's non-uniform W/640 x H/480 stretch. An RmlUi element reproduces such a quad
// exactly with a CSS matrix() transform; these functions build the quads the way the original
// did and turn them into that matrix.
namespace UI::MiniMap
{
// A physical window pixel, y down (as in a screenshot).
struct Point
{
    float x = 0.f;
    float y = 0.f;
};

// CSS matrix(a, b, c, d, e, f): X = a*x + c*y + e, Y = b*x + d*y + f, for an element placed at
// (0, 0) with transform-origin 0 0.
struct CssMatrix
{
    float a = 1.f, b = 0.f, c = 0.f, d = 1.f, e = 0.f, f = 0.f;
};

// A textured quad: where its texture's top-left, top-right and bottom-left corners land.
struct Quad
{
    Point topLeft;
    Point topRight;
    Point bottomLeft;
};

// The map's side length in map pixels (CMiniMap::m_Lenth[0], the only zoom level the original
// ever used), and the rotation of the map and its markers.
constexpr float MapLength = 800.f;
constexpr float MapRotation = 45.f;

// The window size, the Hud layout's stretch and its offset.
struct Screen
{
    float width = 640.f;
    float height = 480.f;
    float scaleX = 1.f;
    float scaleY = 1.f;
    float offsetX = 0.f;
    float offsetY = 0.f;

    bool operator==(const Screen&) const = default;
};

// The matrix that maps an element of elementWidth x elementHeight onto the quad.
CssMatrix ElementToQuad(const Quad& quad, float elementWidth, float elementHeight);

// RenderBitRotate(): the map texture, the hero's map position (heroX, heroY in map pixels, the
// original's Tx/Ty) at the window centre, rotated by `rotation` degrees.
Quad MapQuad(const Screen& screen, float heroX, float heroY, float length, float rotation);

// RenderPointRotate(): a marker at map position (pointX, pointY), `size` physical pixels square,
// turned by its own `pointRotation`. hitBox (x, y, width, height, reference units) is the rectangle
// the original stored with SetBtnPos() for the name hint: `portal` markers used their full size
// around the quad's first corner, NPCs half their size from it.
struct Marker
{
    Quad quad;
    std::array<float, 4> hitBox{};
};
Marker MarkerQuad(const Screen& screen, float heroX, float heroY, float pointX, float pointY, float size,
                  float pointRotation, float length, float rotation, bool portal);

// RenderBitmapRotate(): a width x height (reference units) quad centred on (x, y) reference,
// turned by `rotation` degrees (the side border lines).
Quad RotatedQuad(const Screen& screen, float x, float y, float width, float height, float rotation);

} // namespace UI::MiniMap
