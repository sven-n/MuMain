#pragma once

#ifdef _EDITOR

#include "EffectPreviewCamera.h"

#include "Render/Renderer/MuRenderer.h"

#include <array>
#include <cstdint>

// The quads the effect preview draws in world space (Z-up), for
// RenderQuad3D: four vertices per quad.
namespace MuEditor::Effects::PreviewGeometry
{
using Quad = std::array<mu::Vertex3D, 4>;

// The squares of the plane's checker along each side.
constexpr int PlaneSquares = 8;
using PlaneQuads = std::array<mu::Vertex3D, PlaneSquares * PlaneSquares * 4>;
using CubeQuads = std::array<mu::Vertex3D, 6 * 4>;

// A texture facing the camera, `width` by `height`, with the corners and
// texture coordinates of RenderSprite.
Quad Billboard(const PreviewVector& center, const EffectPreviewCamera::Basis& basis, float width, float height,
               std::uint32_t color);

// A texture lying flat at `center`, `size` across, turned by `angleDegrees`.
Quad GroundQuad(const PreviewVector& center, float size, float angleDegrees, std::uint32_t color);

// A grey checkered square at height 0, `size` across, around the origin.
PlaneQuads Plane(float size);

// A grey cube with edges of `edge`, standing on height 0 around the origin;
// its faces are shaded differently so its edges show without light.
CubeQuads Cube(float edge);

// A quad just in front of the camera that covers all it sees at the aspect
// `width / height`.
Quad ScreenCover(const PreviewVector& eye, const EffectPreviewCamera::Basis& basis, float nearDistance, float aspect,
                 std::uint32_t color);
} // namespace MuEditor::Effects::PreviewGeometry

#endif // _EDITOR
