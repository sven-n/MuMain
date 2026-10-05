#include "stdafx.h"

#ifdef _EDITOR

#include "EffectPreviewCamera.h"

#include <algorithm>
#include <cmath>

namespace MuEditor::Effects
{
namespace
{
constexpr float DegreesToRadians = 3.14159265f / 180.0f;
constexpr float TurnDegreesPerPixel = 0.5f;
// At 90 degrees the view and up direction would be the same.
constexpr float PitchLimitDegrees = 89.0f;
// One wheel step changes the distance by this factor.
constexpr float ZoomFactorPerStep = 0.85f;
// How far the wheel can move from the framed distance.
constexpr float MinZoom = 0.2f;
constexpr float MaxZoom = 5.0f;
// The framed distance leaves room around the sphere, as the thumbnails do.
constexpr float FrameMargin = 1.8f;
// The vertex shader clips what is nearer than 1.
constexpr float MinNear = 1.0f;
constexpr float NearShare = 0.02f;
constexpr float FarMargin = 4000.0f;
constexpr float FarRadii = 8.0f;
constexpr float MinRadius = 1.0f;
} // namespace

void EffectPreviewCamera::Frame(const PreviewVector& center, float radius)
{
    m_center = center;
    m_radius = std::max(radius, MinRadius);
    m_distance = FramedDistance();
}

void EffectPreviewCamera::Turn(float dxPixels, float dyPixels)
{
    m_yaw = std::fmod(m_yaw - dxPixels * TurnDegreesPerPixel, 360.0f);
    m_pitch = std::clamp(m_pitch + dyPixels * TurnDegreesPerPixel, -PitchLimitDegrees, PitchLimitDegrees);
}

void EffectPreviewCamera::Zoom(float wheelSteps)
{
    const float framed = FramedDistance();
    m_distance = std::clamp(m_distance * std::pow(ZoomFactorPerStep, wheelSteps), framed * MinZoom, framed * MaxZoom);
}

void EffectPreviewCamera::Reset()
{
    m_yaw = DefaultYawDegrees;
    m_pitch = DefaultPitchDegrees;
    m_distance = FramedDistance();
}

PreviewVector EffectPreviewCamera::Eye() const
{
    const PreviewVector direction = Direction();
    return {m_center[0] + direction[0] * m_distance, m_center[1] + direction[1] * m_distance,
            m_center[2] + direction[2] * m_distance};
}

float EffectPreviewCamera::Near() const
{
    return std::max(MinNear, m_distance * NearShare);
}

float EffectPreviewCamera::Far() const
{
    return m_distance + m_radius * FarRadii + FarMargin;
}

EffectPreviewCamera::Basis EffectPreviewCamera::GetBasis() const
{
    const PreviewVector direction = Direction();
    const PreviewVector forward = {-direction[0], -direction[1], -direction[2]};
    return LookAt::BasisAlong(forward, {0.0f, 0.0f, 1.0f});
}

// The gluLookAt matrix, as the map editor's thumbnails load it.
std::array<float, 16> EffectPreviewCamera::View() const
{
    return LookAt::ViewMatrix(GetBasis(), Eye());
}

PreviewVector EffectPreviewCamera::Direction() const
{
    const float yaw = m_yaw * DegreesToRadians;
    const float pitch = m_pitch * DegreesToRadians;
    return {std::cos(pitch) * std::cos(yaw), std::cos(pitch) * std::sin(yaw), std::sin(pitch)};
}

float EffectPreviewCamera::FramedDistance() const
{
    return m_radius / std::tan(FieldOfViewDegrees * 0.5f * DegreesToRadians) * FrameMargin;
}
} // namespace MuEditor::Effects

#endif // _EDITOR
