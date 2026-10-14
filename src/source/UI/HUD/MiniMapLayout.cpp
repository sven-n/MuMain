#include "UI/HUD/MiniMapLayout.h"

#include <cmath>

namespace
{
constexpr float kPi = 3.14159265358979323846f;
constexpr float kReferenceHeight = 480.f;
// RenderPointRotate() shifted every marker 25 physical pixels right after rotating it.
constexpr float kMarkerShift = 25.f;

// AngleMatrix() with only a z angle: a counter-clockwise turn in the original's y-up space.
UI::MiniMap::Point Rotate(float x, float y, float degrees)
{
    const float radians = degrees * (kPi * 2.f / 360.f);
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    return {c * x - s * y, s * x + c * y};
}

// A y-up point relative to the window centre, as a y-down window pixel.
UI::MiniMap::Point FromCentre(const UI::MiniMap::Screen& screen, UI::MiniMap::Point p)
{
    return {p.x + screen.width / 2.f, screen.height - (p.y + screen.height / 2.f)};
}
} // namespace

UI::MiniMap::CssMatrix UI::MiniMap::ElementToQuad(const Quad& quad, float elementWidth, float elementHeight)
{
    CssMatrix m;
    m.a = (quad.topRight.x - quad.topLeft.x) / elementWidth;
    m.b = (quad.topRight.y - quad.topLeft.y) / elementWidth;
    m.c = (quad.bottomLeft.x - quad.topLeft.x) / elementHeight;
    m.d = (quad.bottomLeft.y - quad.topLeft.y) / elementHeight;
    m.e = quad.topLeft.x;
    m.f = quad.topLeft.y;
    return m;
}

UI::MiniMap::Quad UI::MiniMap::MapQuad(const Screen& screen, float heroX, float heroY, float length, float rotation)
{
    // RenderBitRotate(texture, length - heroX, length - heroY, length, length, rotation).
    const float x = screen.scaleX * (length - heroX);
    const float width = screen.scaleX * length;
    const float height = screen.scaleY * length;
    const float y = height - screen.scaleY * (length - heroY);
    const float cx = (width / 2.f) - (width - x);
    const float cy = (height / 2.f) - (height - y);
    const float ax = -width * 0.5f + cx;
    const float bx = width * 0.5f + cx;
    const float ay = -height * 0.5f + cy;
    const float by = height * 0.5f + cy;

    Quad quad;
    quad.topLeft = FromCentre(screen, Rotate(ax, by, rotation));
    quad.bottomLeft = FromCentre(screen, Rotate(ax, ay, rotation));
    quad.topRight = FromCentre(screen, Rotate(bx, by, rotation));
    return quad;
}

UI::MiniMap::Marker UI::MiniMap::MarkerQuad(const Screen& screen, float heroX, float heroY, float pointX, float pointY,
                                            float size, float pointRotation, float length, float rotation, bool portal)
{
    // RenderPointRotate(texture, pointX, pointY, size, size, length - heroX, length - heroY, length, length,
    //                   rotation, pointRotation, ...).
    const float ix = screen.scaleX * pointX;
    const float width = screen.scaleX * length;
    const float height = screen.scaleY * length;
    const float x = screen.scaleX * (length - heroX);
    const float y = height - screen.scaleY * (length - heroY);
    const float iy = height - screen.scaleY * pointY;

    const Point centre = Rotate((ix - width * 0.5f) + ((width / 2.f) - (width - x)),
                                (iy - height * 0.5f) + ((height / 2.f) - (height - y)), rotation);
    const float half = size * 0.5f;
    auto corner = [&](float px, float py)
    {
        const Point turned = Rotate(px, py, pointRotation);
        return Point{turned.x + centre.x + kMarkerShift, turned.y + centre.y};
    };
    const Point first = corner(-half, half);

    Marker marker;
    marker.quad.topLeft = FromCentre(screen, first);
    marker.quad.bottomLeft = FromCentre(screen, corner(-half, -half));
    marker.quad.topRight = FromCentre(screen, corner(half, half));

    // SetBtnPos() from the quad's first corner, back in reference units.
    const float dx = (first.x + screen.width / 2.f - screen.offsetX) / screen.scaleX;
    const float dy = (first.y + screen.height / 2.f - screen.offsetY) / screen.scaleY;
    if (portal)
        marker.hitBox = {dx - half, (kReferenceHeight - dy) - half, size, size};
    else
        marker.hitBox = {dx, kReferenceHeight - dy, half, half};
    return marker;
}

UI::MiniMap::Quad UI::MiniMap::RotatedQuad(const Screen& screen, float x, float y, float width, float height,
                                           float rotation)
{
    const float cx = x * screen.scaleX + screen.offsetX;
    const float cyDown = y * screen.scaleY + screen.offsetY;
    const float w = width * screen.scaleX;
    const float h = height * screen.scaleY;
    auto corner = [&](float px, float py)
    {
        const Point turned = Rotate(px, py, rotation);
        return Point{cx + turned.x, cyDown - turned.y};
    };

    Quad quad;
    quad.topLeft = corner(-w * 0.5f, h * 0.5f);
    quad.bottomLeft = corner(-w * 0.5f, -h * 0.5f);
    quad.topRight = corner(w * 0.5f, h * 0.5f);
    return quad;
}
