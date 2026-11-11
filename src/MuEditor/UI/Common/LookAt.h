#pragma once

#ifdef _EDITOR

#include <array>

// The view of a camera that looks from an eye toward a point, as gluLookAt
// makes it. The map editor's thumbnails and the effect preview load it.
namespace MuEditor::LookAt
{
using Vector = std::array<float, 3>;
// Column-major, as IMuRenderer::LoadMatrix takes it.
using Matrix = std::array<float, 16>;

// The unit vectors of a view: right and up on the screen, and forward into
// it.
struct Basis
{
    Vector right{};
    Vector up{};
    Vector forward{};
};

// The view along the unit vector `forward`, turned so `worldUp` points up on
// the screen. `forward` must not point along `worldUp`.
Basis BasisAlong(const Vector& forward, const Vector& worldUp);

// The view matrix of `basis` seen from `eye`: it puts the eye at the origin,
// looking down -z.
Matrix ViewMatrix(const Basis& basis, const Vector& eye);

// The gluLookAt matrix from `eye` toward `center`, with `worldUp` up on the
// screen.
Matrix ViewMatrix(const Vector& eye, const Vector& center, const Vector& worldUp);
} // namespace MuEditor::LookAt

#endif // _EDITOR
