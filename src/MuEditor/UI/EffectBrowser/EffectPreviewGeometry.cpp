#include "stdafx.h"

#ifdef _EDITOR

#include "EffectPreviewGeometry.h"

#include "Render/Renderer/RenderUtils.h"

#include <cmath>

namespace MuEditor::Effects::PreviewGeometry
{
namespace
{
constexpr float DegreesToRadians = 3.14159265f / 180.0f;

// The two greys of the plane's checker and the shades of the cube's faces.
constexpr float PlaneDark = 0.30f;
constexpr float PlaneLight = 0.36f;
constexpr float CubeTop = 0.6f;
constexpr float CubeFront = 0.45f;
constexpr float CubeSide = 0.35f;
constexpr float CubeBottom = 0.2f;

// The cover lies a little behind the near plane and reaches past the view's
// edges.
constexpr float CoverDepth = 2.0f;
constexpr float CoverOverlap = 1.5f;

mu::Vertex3D Vertex(const PreviewVector& p, float u, float v, std::uint32_t color)
{
    return {p[0], p[1], p[2], 0.0f, 0.0f, 0.0f, u, v, color};
}

PreviewVector Offset(const PreviewVector& center, const PreviewVector& a, float da, const PreviewVector& b, float db)
{
    return {center[0] + a[0] * da + b[0] * db, center[1] + a[1] * da + b[1] * db, center[2] + a[2] * da + b[2] * db};
}

std::uint32_t Grey(float shade)
{
    return mu::PackABGR(shade, shade, shade, 1.0f);
}

// An untextured quad from its four corners.
void PutQuad(mu::Vertex3D* out, const std::array<PreviewVector, 4>& corners, std::uint32_t color)
{
    for (size_t i = 0; i < corners.size(); ++i)
    {
        out[i] = Vertex(corners[i], 0.0f, 0.0f, color);
    }
}
} // namespace

// RenderSprite puts (-w,-h) at v = 1, so the texture stands upright.
Quad Billboard(const PreviewVector& center, const EffectPreviewCamera::Basis& basis, float width, float height,
               std::uint32_t color)
{
    const float w = width * 0.5f;
    const float h = height * 0.5f;
    return {Vertex(Offset(center, basis.right, -w, basis.up, -h), 0.0f, 1.0f, color),
            Vertex(Offset(center, basis.right, w, basis.up, -h), 1.0f, 1.0f, color),
            Vertex(Offset(center, basis.right, w, basis.up, h), 1.0f, 0.0f, color),
            Vertex(Offset(center, basis.right, -w, basis.up, h), 0.0f, 0.0f, color)};
}

Quad GroundQuad(const PreviewVector& center, float size, float angleDegrees, std::uint32_t color)
{
    const float angle = angleDegrees * DegreesToRadians;
    const PreviewVector x = {std::cos(angle), std::sin(angle), 0.0f};
    const PreviewVector y = {-std::sin(angle), std::cos(angle), 0.0f};
    const float half = size * 0.5f;
    return {Vertex(Offset(center, x, -half, y, -half), 0.0f, 0.0f, color),
            Vertex(Offset(center, x, half, y, -half), 1.0f, 0.0f, color),
            Vertex(Offset(center, x, half, y, half), 1.0f, 1.0f, color),
            Vertex(Offset(center, x, -half, y, half), 0.0f, 1.0f, color)};
}

PlaneQuads Plane(float size)
{
    PlaneQuads quads{};
    const float square = size / PlaneSquares;
    const float start = -size * 0.5f;
    for (int row = 0; row < PlaneSquares; ++row)
    {
        for (int column = 0; column < PlaneSquares; ++column)
        {
            const float x = start + column * square;
            const float y = start + row * square;
            const std::uint32_t color = Grey((row + column) % 2 == 0 ? PlaneDark : PlaneLight);
            PutQuad(&quads[(row * PlaneSquares + column) * 4],
                    {{{x, y, 0.0f}, {x + square, y, 0.0f}, {x + square, y + square, 0.0f}, {x, y + square, 0.0f}}},
                    color);
        }
    }
    return quads;
}

CubeQuads Cube(float edge)
{
    const float h = edge * 0.5f;
    const float top = edge;
    CubeQuads quads{};
    PutQuad(&quads[0], {{{-h, -h, top}, {h, -h, top}, {h, h, top}, {-h, h, top}}}, Grey(CubeTop));
    PutQuad(&quads[4], {{{-h, -h, 0.0f}, {-h, h, 0.0f}, {h, h, 0.0f}, {h, -h, 0.0f}}}, Grey(CubeBottom));
    PutQuad(&quads[8], {{{-h, -h, 0.0f}, {h, -h, 0.0f}, {h, -h, top}, {-h, -h, top}}}, Grey(CubeFront));
    PutQuad(&quads[12], {{{h, h, 0.0f}, {-h, h, 0.0f}, {-h, h, top}, {h, h, top}}}, Grey(CubeFront));
    PutQuad(&quads[16], {{{h, -h, 0.0f}, {h, h, 0.0f}, {h, h, top}, {h, -h, top}}}, Grey(CubeSide));
    PutQuad(&quads[20], {{{-h, h, 0.0f}, {-h, -h, 0.0f}, {-h, -h, top}, {-h, h, top}}}, Grey(CubeSide));
    return quads;
}

Quad ScreenCover(const PreviewVector& eye, const EffectPreviewCamera::Basis& basis, float nearDistance, float aspect,
                 std::uint32_t color)
{
    const float depth = nearDistance * CoverDepth;
    const PreviewVector center = Offset(eye, basis.forward, depth, basis.up, 0.0f);
    const float halfHeight =
        std::tan(EffectPreviewCamera::FieldOfViewDegrees * 0.5f * DegreesToRadians) * depth * CoverOverlap;
    return Billboard(center, basis, halfHeight * 2.0f * aspect, halfHeight * 2.0f, color);
}
} // namespace MuEditor::Effects::PreviewGeometry

#endif // _EDITOR
