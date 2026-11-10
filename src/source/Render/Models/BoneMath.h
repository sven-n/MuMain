#pragma once

#include <algorithm>
#include <cmath>

// Rotations for a bone's transform (3x4: the rotation in the first three columns, the place in the last) and for
// directions, as the character realism steps turn bones after BMD::Animation built them: a head turned in the world,
// the ponytail and the wings trailing a turn, the legs bent to put the feet on the ground.
namespace Render::Models::BoneMath
{
// A direction's length.
inline float Length(const float v[3])
{
    return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

// a - b, as points or directions.
inline void Subtract(const float a[3], const float b[3], float out[3])
{
    for (int r = 0; r < 3; ++r)
    {
        out[r] = a[r] - b[r];
    }
}

// A direction turned by a 3x3 rotation.
inline void Rotate(const float rotation[3][3], const float v[3], float out[3])
{
    for (int r = 0; r < 3; ++r)
    {
        out[r] = rotation[r][0] * v[0] + rotation[r][1] * v[1] + rotation[r][2] * v[2];
    }
}

// a * b, as 3x3 rotations; `out` may be either of them.
inline void Multiply(const float a[3][3], const float b[3][3], float out[3][3])
{
    float result[3][3];
    for (int r = 0; r < 3; ++r)
    {
        for (int c = 0; c < 3; ++c)
        {
            result[r][c] = a[r][0] * b[0][c] + a[r][1] * b[1][c] + a[r][2] * b[2][c];
        }
    }
    for (int r = 0; r < 3; ++r)
    {
        for (int c = 0; c < 3; ++c)
        {
            out[r][c] = result[r][c];
        }
    }
}

// The rotation about `axis` (unit length) by the angle with this cosine and sine, as a 3x3 matrix.
inline void RotationAbout(const float axis[3], float cosine, float sine, float rotation[3][3])
{
    const float rest = 1.f - cosine;
    const float x = axis[0];
    const float y = axis[1];
    const float z = axis[2];
    const float m[3][3] = {{cosine + x * x * rest, x * y * rest - z * sine, x * z * rest + y * sine},
                           {y * x * rest + z * sine, cosine + y * y * rest, y * z * rest - x * sine},
                           {z * x * rest - y * sine, z * y * rest + x * sine, cosine + z * z * rest}};
    for (int r = 0; r < 3; ++r)
    {
        for (int c = 0; c < 3; ++c)
        {
            rotation[r][c] = m[r][c];
        }
    }
}

// The rotation about `axis` (unit length) by `radians`.
inline void RotationAbout(const float axis[3], float radians, float rotation[3][3])
{
    RotationAbout(axis, std::cos(radians), std::sin(radians), rotation);
}

// The rotation turning direction `from` onto direction `to` (the shortest way).
inline void RotationBetween(const float from[3], const float to[3], float rotation[3][3])
{
    const float fromLength = Length(from);
    const float toLength = Length(to);
    float axis[3] = {from[1] * to[2] - from[2] * to[1], from[2] * to[0] - from[0] * to[2],
                     from[0] * to[1] - from[1] * to[0]};
    const float lengths = std::max(fromLength * toLength, 0.0001f);
    const float sine = Length(axis) / lengths;
    const float cosine = (from[0] * to[0] + from[1] * to[1] + from[2] * to[2]) / lengths;
    const float axisLength = Length(axis);
    if (axisLength < 0.0001f)
    {
        for (int r = 0; r < 3; ++r)
        {
            for (int c = 0; c < 3; ++c)
            {
                rotation[r][c] = r == c ? 1.f : 0.f;
            }
        }
        return;
    }
    for (float& a : axis)
    {
        a /= axisLength;
    }
    RotationAbout(axis, cosine, sine, rotation);
}

// Turns a bone's rotation by `rotation` (in the space the bone is in); its place stays.
inline void TurnRotation(float bone[3][4], const float rotation[3][3])
{
    float turned[3][3];
    for (int r = 0; r < 3; ++r)
    {
        for (int c = 0; c < 3; ++c)
        {
            turned[r][c] = rotation[r][0] * bone[0][c] + rotation[r][1] * bone[1][c] + rotation[r][2] * bone[2][c];
        }
    }
    for (int r = 0; r < 3; ++r)
    {
        for (int c = 0; c < 3; ++c)
        {
            bone[r][c] = turned[r][c];
        }
    }
}

// Turns a transform by `radians` about one of its space's axes, from axis `first` toward axis `second` (0, 1: about
// z, the upright; 1, 2: about x). With `withPlace` its place turns too, about the line through (pivotFirst,
// pivotSecond); without, only its rotation turns.
inline void TurnInPlane(float matrix[3][4], int first, int second, float radians, bool withPlace,
                        float pivotFirst = 0.f, float pivotSecond = 0.f)
{
    const float cosine = std::cos(radians);
    const float sine = std::sin(radians);
    const int columns = withPlace ? 4 : 3;
    for (int column = 0; column < columns; ++column)
    {
        const bool place = column == 3;
        const float a = matrix[first][column] - (place ? pivotFirst : 0.f);
        const float b = matrix[second][column] - (place ? pivotSecond : 0.f);
        matrix[first][column] = cosine * a - sine * b + (place ? pivotFirst : 0.f);
        matrix[second][column] = sine * a + cosine * b + (place ? pivotSecond : 0.f);
    }
}
} // namespace Render::Models::BoneMath
