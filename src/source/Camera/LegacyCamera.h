#pragma once

#include "DefaultCamera.h"

/**
 * @brief Legacy camera for A/B performance testing
 *
 * Uses the same position/angle logic as DefaultCamera but replaces
 * expensive 3D frustum sphere culling with cheap 2D distance checks.
 * This approximates the old main-branch culling behavior.
 */
class LegacyCamera : public DefaultCamera
{
public:
    LegacyCamera(CameraState& state) : DefaultCamera(state), m_StateRef(state) {}

    const char* GetName() const override { return "Legacy"; }

    // Override culling with cheap distance-based checks (no 6-plane frustum test)
    bool ShouldCullObject(const vec3_t position, float radius) const override
    {
        float dx = position[0] - m_StateRef.Position[0];
        float dy = position[1] - m_StateRef.Position[1];
        float distSq = dx * dx + dy * dy;
        float maxDist = m_StateRef.ViewFar + radius;
        return distSq > maxDist * maxDist;
    }

    bool ShouldCullTerrain(int tileX, int tileY) const override
    {
        float worldX = (tileX + 0.5f) * 100.0f;
        float worldY = (tileY + 0.5f) * 100.0f;
        float dx = worldX - m_StateRef.Position[0];
        float dy = worldY - m_StateRef.Position[1];
        float distSq = dx * dx + dy * dy;
        float maxDist = m_StateRef.ViewFar + 200.0f;
        return distSq > maxDist * maxDist;
    }

    bool ShouldCullObject2D(float x, float y, float radius) const override
    {
        float dx = x - m_StateRef.Position[0];
        float dy = y - m_StateRef.Position[1];
        float distSq = dx * dx + dy * dy;
        float maxDist = m_StateRef.ViewFar + radius;
        return distSq > maxDist * maxDist;
    }

private:
    CameraState& m_StateRef;
};
