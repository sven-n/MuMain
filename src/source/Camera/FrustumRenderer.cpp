#include "stdafx.h"

#ifdef _EDITOR

#include "FrustumRenderer.h"
#include "CameraConfig.h"
#include "Core/Globals/_types.h"
#include "Core/Globals/_define.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Renderer/RenderUtils.h"
#include <cmath>
#include <cstring>
#include <vector>

float RequestTerrainHeight(float xf, float yf);

namespace
{
// Distance threshold for degenerate/orthographic detection when computing camera apex.
constexpr float DEGENERATE_EPSILON = 0.001f;

// Vertical offset above terrain surface for the ground-projection line so it
// doesn't z-fight with the ground itself.
constexpr float GROUND_LINE_Z_OFFSET = 5.0f;

// Ground-projection edge subdivision: one segment per (SUBDIVISIONS_PER_TILE * TERRAIN_SCALE)
// units of edge length, clamped to [1, MAX_EDGE_SEGMENTS].
constexpr float SUBDIVISIONS_PER_TILE = 2.0f;
constexpr int MAX_EDGE_SEGMENTS = 32;

// Camera position marker is drawn as an axis-aligned cross with arms this long.
constexpr float CAMERA_MARKER_HALF_LENGTH = 50.0f;

mu::Vertex3D MakeVertex(const vec3_t position, std::uint32_t color)
{
    return {position[0], position[1], position[2], 0.f, 0.f, 1.f, 0.f, 0.f, color};
}

void AppendLine(std::vector<mu::Vertex3D>& vertices, float x0, float y0, float z0, float x1, float y1, float z1,
                std::uint32_t color)
{
    vertices.push_back({x0, y0, z0, 0.f, 0.f, 1.f, 0.f, 0.f, color});
    vertices.push_back({x1, y1, z1, 0.f, 0.f, 1.f, 0.f, 0.f, color});
}

// Average of 4 vec3_t points.
inline void AverageQuad(const vec3_t a, const vec3_t b, const vec3_t c, const vec3_t d, vec3_t out)
{
    out[0] = (a[0] + b[0] + c[0] + d[0]) * 0.25f;
    out[1] = (a[1] + b[1] + c[1] + d[1]) * 0.25f;
    out[2] = (a[2] + b[2] + c[2] + d[2]) * 0.25f;
}

// Distance between two vec3_t points.
inline float Distance3D(const vec3_t a, const vec3_t b)
{
    float dx = a[0] - b[0], dy = a[1] - b[1], dz = a[2] - b[2];
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

// Compute the frustum apex (camera eye) by extrapolating the near→far edge back
// to where the plane width would be zero. Falls back to nearCenter for degenerate cases.
void ComputeFrustumApex(const vec3_t v[8], vec3_t outApex, vec3_t outNearCenter, vec3_t outFarCenter)
{
    AverageQuad(v[0], v[1], v[2], v[3], outNearCenter);
    AverageQuad(v[4], v[5], v[6], v[7], outFarCenter);

    float nearHW = Distance3D(v[0], outNearCenter);
    float farHW = Distance3D(v[4], outFarCenter);

    float dx = outFarCenter[0] - outNearCenter[0];
    float dy = outFarCenter[1] - outNearCenter[1];
    float dz = outFarCenter[2] - outNearCenter[2];
    float planeDist = std::sqrt(dx * dx + dy * dy + dz * dz);

    float denom = farHW - nearHW;
    if (planeDist > DEGENERATE_EPSILON && std::fabs(denom) > DEGENERATE_EPSILON)
    {
        float nearDist = nearHW * planeDist / denom;
        float invPlaneDist = 1.0f / planeDist;
        outApex[0] = outNearCenter[0] - dx * invPlaneDist * nearDist;
        outApex[1] = outNearCenter[1] - dy * invPlaneDist * nearDist;
        outApex[2] = outNearCenter[2] - dz * invPlaneDist * nearDist;
    }
    else
    {
        // Fallback: use near-plane center (orthographic or degenerate)
        outApex[0] = outNearCenter[0];
        outApex[1] = outNearCenter[1];
        outApex[2] = outNearCenter[2];
    }
}

// Scale far-plane vertices outward from the camera by RENDER_DISTANCE_MULTIPLIER
// so the visualization matches the actual GL projection extent.
void ScaleFarVerticesFromApex(vec3_t v[8], const vec3_t apex)
{
    for (int i = 4; i < 8; ++i)
    {
        for (int axis = 0; axis < 3; ++axis)
        {
            float dir = v[i][axis] - apex[axis];
            v[i][axis] = apex[axis] + dir * RENDER_DISTANCE_MULTIPLIER;
        }
    }
}

void RenderPyramidWireframe(const vec3_t v[8], const vec3_t apex)
{
    const std::uint32_t nearColor = mu::PackABGR(0.f, 1.f, 0.f, 0.8f);
    const std::uint32_t farColor = mu::PackABGR(1.f, 0.f, 0.f, 0.8f);
    const std::uint32_t sideColor = mu::PackABGR(1.f, 1.f, 0.f, 0.8f);
    const mu::Vertex3D lines[] = {
        MakeVertex(v[0], nearColor), MakeVertex(v[1], nearColor), MakeVertex(v[1], nearColor),
        MakeVertex(v[2], nearColor), MakeVertex(v[2], nearColor), MakeVertex(v[3], nearColor),
        MakeVertex(v[3], nearColor), MakeVertex(v[0], nearColor), MakeVertex(v[4], farColor),
        MakeVertex(v[5], farColor),  MakeVertex(v[5], farColor),  MakeVertex(v[6], farColor),
        MakeVertex(v[6], farColor),  MakeVertex(v[7], farColor),  MakeVertex(v[7], farColor),
        MakeVertex(v[4], farColor),  MakeVertex(apex, sideColor), MakeVertex(v[4], sideColor),
        MakeVertex(apex, sideColor), MakeVertex(v[5], sideColor), MakeVertex(apex, sideColor),
        MakeVertex(v[6], sideColor), MakeVertex(apex, sideColor), MakeVertex(v[7], sideColor),
    };
    mu::GetRenderer().RenderLines(lines, 0);
}

void RenderPyramidFilled(const vec3_t v[8], const vec3_t apex)
{
    const std::uint32_t color = mu::PackABGR(1.f, 1.f, 0.f, 0.08f);
    const mu::Vertex3D triangles[] = {
        MakeVertex(apex, color), MakeVertex(v[4], color), MakeVertex(v[7], color), MakeVertex(apex, color),
        MakeVertex(v[5], color), MakeVertex(v[6], color), MakeVertex(apex, color), MakeVertex(v[4], color),
        MakeVertex(v[5], color), MakeVertex(apex, color), MakeVertex(v[7], color), MakeVertex(v[6], color),
        MakeVertex(v[4], color), MakeVertex(v[5], color), MakeVertex(v[6], color), MakeVertex(v[4], color),
        MakeVertex(v[6], color), MakeVertex(v[7], color),
    };
    mu::GetRenderer().RenderTriangles(triangles, 0);
}

void RenderGroundProjection(const Frustum& frustum)
{
    const int hullCount = frustum.Get2DCount();
    if (hullCount < 3)
        return;

    const float* hullX = frustum.Get2DX();
    const float* hullY = frustum.Get2DY();

    const std::uint32_t color = mu::PackABGR(0.f, 1.f, 0.f, 0.7f);
    thread_local std::vector<mu::Vertex3D> lines;
    lines.clear();
    lines.reserve(static_cast<std::size_t>(hullCount) * MAX_EDGE_SEGMENTS * 2);

    for (int i = 0; i < hullCount; ++i)
    {
        int next = (i + 1) % hullCount;

        float x0 = hullX[i] * TERRAIN_SCALE;
        float y0 = hullY[i] * TERRAIN_SCALE;
        float x1 = hullX[next] * TERRAIN_SCALE;
        float y1 = hullY[next] * TERRAIN_SCALE;

        // Subdivide edge so each segment tracks terrain elevation
        float dx = x1 - x0;
        float dy = y1 - y0;
        float edgeLen = std::sqrt(dx * dx + dy * dy);
        int segments = (int)(edgeLen / (SUBDIVISIONS_PER_TILE * TERRAIN_SCALE)) + 1;
        if (segments < 1)
            segments = 1;
        if (segments > MAX_EDGE_SEGMENTS)
            segments = MAX_EDGE_SEGMENTS;

        for (int s = 0; s < segments; ++s)
        {
            float t0 = (float)s / (float)segments;
            float t1 = (float)(s + 1) / (float)segments;

            float sx0 = x0 + dx * t0;
            float sy0 = y0 + dy * t0;
            float sx1 = x0 + dx * t1;
            float sy1 = y0 + dy * t1;

            float z0 = RequestTerrainHeight(sx0, sy0) + GROUND_LINE_Z_OFFSET;
            float z1 = RequestTerrainHeight(sx1, sy1) + GROUND_LINE_Z_OFFSET;

            AppendLine(lines, sx0, sy0, z0, sx1, sy1, z1, color);
        }
    }
    mu::GetRenderer().RenderLines(lines, 0);
}

// Draw a terrain-hugging horizontal line between two ground hit points.
void AppendGroundSegment(std::vector<mu::Vertex3D>& lines, float x0, float y0, float x1, float y1, std::uint32_t color)
{
    float dx = x1 - x0;
    float dy = y1 - y0;
    float edgeLen = std::sqrt(dx * dx + dy * dy);
    int segments = (int)(edgeLen / (SUBDIVISIONS_PER_TILE * TERRAIN_SCALE)) + 1;
    if (segments < 1)
        segments = 1;
    if (segments > MAX_EDGE_SEGMENTS)
        segments = MAX_EDGE_SEGMENTS;

    for (int s = 0; s < segments; ++s)
    {
        float t0 = (float)s / (float)segments;
        float t1 = (float)(s + 1) / (float)segments;
        float sx0 = x0 + dx * t0;
        float sy0 = y0 + dy * t0;
        float sx1 = x0 + dx * t1;
        float sy1 = y0 + dy * t1;
        float z0 = RequestTerrainHeight(sx0, sy0) + GROUND_LINE_Z_OFFSET;
        float z1 = RequestTerrainHeight(sx1, sy1) + GROUND_LINE_Z_OFFSET;
        AppendLine(lines, sx0, sy0, z0, sx1, sy1, z1, color);
    }
}

// Cast each ray (apex → far-plane corner) onto the ground plane (Z=0)
// and connect the resulting points to show where the camera's top and
// bottom FOV edges intersect the ground. Frustum vertex order from
// CalculateFrustumVertices: 4=far TL, 5=far TR, 6=far BR, 7=far BL.
void RenderFovGroundIntersect(const vec3_t apex, const vec3_t v[8])
{
    auto rayToGround = [&](const vec3_t corner, float& outX, float& outY) -> bool
    {
        float dz = corner[2] - apex[2];
        if (dz >= -DEGENERATE_EPSILON)
            return false; // ray not going down
        float t = -apex[2] / dz;
        if (t <= 0.0f)
            return false;
        outX = apex[0] + t * (corner[0] - apex[0]);
        outY = apex[1] + t * (corner[1] - apex[1]);
        return true;
    };

    float topLx, topLy, topRx, topRy, botLx, botLy, botRx, botRy;
    bool tlOk = rayToGround(v[4], topLx, topLy);
    bool trOk = rayToGround(v[5], topRx, topRy);
    bool brOk = rayToGround(v[6], botRx, botRy);
    bool blOk = rayToGround(v[7], botLx, botLy);

    thread_local std::vector<mu::Vertex3D> lines;
    lines.clear();
    lines.reserve(MAX_EDGE_SEGMENTS * 4);

    if (blOk && brOk)
    {
        const std::uint32_t color = mu::PackABGR(1.f, 0.f, 0.f, 0.9f);
        AppendGroundSegment(lines, botLx, botLy, botRx, botRy, color);
    }
    if (tlOk && trOk)
    {
        const std::uint32_t color = mu::PackABGR(1.f, 1.f, 0.f, 0.9f);
        AppendGroundSegment(lines, topLx, topLy, topRx, topRy, color);
    }
    mu::GetRenderer().RenderLines(lines, 0);
}

void RenderCameraMarker(const vec3_t apex)
{
    const std::uint32_t color = mu::PackABGR(0.f, 1.f, 1.f, 0.9f);
    const mu::Vertex3D lines[] = {
        {apex[0] - CAMERA_MARKER_HALF_LENGTH, apex[1], apex[2], 0.f, 0.f, 1.f, 0.f, 0.f, color},
        {apex[0] + CAMERA_MARKER_HALF_LENGTH, apex[1], apex[2], 0.f, 0.f, 1.f, 0.f, 0.f, color},
        {apex[0], apex[1] - CAMERA_MARKER_HALF_LENGTH, apex[2], 0.f, 0.f, 1.f, 0.f, 0.f, color},
        {apex[0], apex[1] + CAMERA_MARKER_HALF_LENGTH, apex[2], 0.f, 0.f, 1.f, 0.f, 0.f, color},
        {apex[0], apex[1], apex[2] - CAMERA_MARKER_HALF_LENGTH, 0.f, 0.f, 1.f, 0.f, 0.f, color},
        {apex[0], apex[1], apex[2] + CAMERA_MARKER_HALF_LENGTH, 0.f, 0.f, 1.f, 0.f, 0.f, color},
    };
    mu::GetRenderer().RenderLines(lines, 0);
}
} // namespace

void RenderFrustumWireframe(const Frustum& frustum)
{
    // Copy vertices so we can scale the far plane without mutating the frustum
    vec3_t v[8];
    std::memcpy(v, frustum.GetVertices(), sizeof(vec3_t) * 8);

    vec3_t apex, nearCenter, farCenter;
    ComputeFrustumApex(v, apex, nearCenter, farCenter);
    ScaleFarVerticesFromApex(v, apex);

    mu::GetRenderer().SetDepthTest(false);
    mu::GetRenderer().SetTexture2D(false);
    mu::GetRenderer().SetBlendMode(mu::BlendMode::Alpha);
    RenderPyramidWireframe(v, apex);
    RenderPyramidFilled(v, apex);
    RenderGroundProjection(frustum);
    // FOV ground-intersection lines: ray-cast through the scaled far
    // corners (= the cone's yellow apex→corner edges) so the lines end
    // exactly where the visualization cone hits the ground.
    RenderFovGroundIntersect(apex, v);
    RenderCameraMarker(apex);
    mu::GetRenderer().SetDepthTest(true);
    mu::GetRenderer().SetTexture2D(true);
}

#endif // _EDITOR
