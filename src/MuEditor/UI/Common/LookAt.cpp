#include "stdafx.h"

#ifdef _EDITOR

#include "LookAt.h"

#include <cmath>

namespace MuEditor::LookAt
{
namespace
{
Vector Cross(const Vector& a, const Vector& b)
{
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}

float Dot(const Vector& a, const Vector& b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

Vector Normalized(const Vector& v)
{
    const float length = std::sqrt(Dot(v, v));
    return length > 0.0f ? Vector{v[0] / length, v[1] / length, v[2] / length} : v;
}
} // namespace

Basis BasisAlong(const Vector& forward, const Vector& worldUp)
{
    const Vector right = Normalized(Cross(forward, worldUp));
    return {right, Cross(right, forward), forward};
}

Matrix ViewMatrix(const Basis& basis, const Vector& eye)
{
    return {basis.right[0],         basis.up[0],         -basis.forward[0],       0.0f,
            basis.right[1],         basis.up[1],         -basis.forward[1],       0.0f,
            basis.right[2],         basis.up[2],         -basis.forward[2],       0.0f,
            -Dot(basis.right, eye), -Dot(basis.up, eye), Dot(basis.forward, eye), 1.0f};
}

Matrix ViewMatrix(const Vector& eye, const Vector& center, const Vector& worldUp)
{
    const Vector toCenter = {center[0] - eye[0], center[1] - eye[1], center[2] - eye[2]};
    return ViewMatrix(BasisAlong(Normalized(toCenter), worldUp), eye);
}
} // namespace MuEditor::LookAt

#endif // _EDITOR
