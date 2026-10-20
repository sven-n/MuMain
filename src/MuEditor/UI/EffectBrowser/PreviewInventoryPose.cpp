#include "stdafx.h"

#ifdef _EDITOR

#include "PreviewInventoryPose.h"

#include "EffectPreviewCamera.h"

#include <array>
#include <cmath>

namespace MuEditor::Effects
{
namespace
{
constexpr float DegreesToRadians = 3.14159265f / 180.0f;
} // namespace

void InventoryViewToScene(float turnDegrees, float matrix[3][4])
{
    const float yaw = (EffectPreviewCamera::DefaultYawDegrees + turnDegrees) * DegreesToRadians;
    // Level toward the camera, up, and to its right (right x up = toward).
    const std::array<float, 3> toward = {std::cos(yaw), std::sin(yaw), 0.0f};
    const std::array<float, 3> up = {0.0f, 0.0f, 1.0f};
    const std::array<float, 3> right = {-toward[1], toward[0], 0.0f};
    for (int row = 0; row < 3; ++row)
    {
        matrix[row][0] = right[row];
        matrix[row][1] = up[row];
        matrix[row][2] = toward[row];
        matrix[row][3] = 0.0f;
    }
}
} // namespace MuEditor::Effects

#endif // _EDITOR
