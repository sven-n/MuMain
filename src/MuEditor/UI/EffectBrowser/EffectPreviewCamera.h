#pragma once

#ifdef _EDITOR

#include "UI/Common/LookAt.h"

#include <array>

namespace MuEditor::Effects
{
using PreviewVector = std::array<float, 3>;

// The camera of the effect preview: it looks at the previewed type from a
// direction the mouse turns and a distance the wheel changes. MU is Z-up.
class EffectPreviewCamera
{
public:
    static constexpr float FieldOfViewDegrees = 35.0f;
    // The first direction: from the front right and above, as the map
    // editor's object thumbnails look at their objects.
    static constexpr float DefaultYawDegrees = -45.0f;
    static constexpr float DefaultPitchDegrees = 29.5f;

    // The unit vectors of the view: right and up on the screen, and forward
    // into it.
    using Basis = LookAt::Basis;

    // Looks at `center` from the distance that shows a sphere of `radius`,
    // keeping the direction.
    void Frame(const PreviewVector& center, float radius);
    // Turns around the center by mouse movement in pixels.
    void Turn(float dxPixels, float dyPixels);
    // Moves closer for positive wheel steps, further for negative ones.
    void Zoom(float wheelSteps);
    // The first direction and the framed distance again.
    void Reset();

    PreviewVector Eye() const;
    const PreviewVector& Center() const
    {
        return m_center;
    }
    float Distance() const
    {
        return m_distance;
    }
    float Near() const;
    float Far() const;
    Basis GetBasis() const;
    // The view as a column-major look-at matrix.
    std::array<float, 16> View() const;

private:
    PreviewVector Direction() const;
    float FramedDistance() const;

    PreviewVector m_center{};
    float m_radius = 100.0f;
    float m_yaw = DefaultYawDegrees;
    float m_pitch = DefaultPitchDegrees;
    float m_distance = 0.0f;
};
} // namespace MuEditor::Effects

#endif // _EDITOR
