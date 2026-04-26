#include "stdafx.h"
#include "Frustum.h"
#include "ConvexHull2D.h"
#include "_define.h"
#include <cmath>

namespace
{
    // Epsilon when comparing cull distances to decide whether the terrain 2D-hull
    // extension is needed. Anything closer than this to farDist is treated as equal.
    constexpr float TERRAIN_EXTENSION_EPSILON = 1.0f;

    // Compute the 4 corners of a rectangular plane (top-left, top-right, bottom-right, bottom-left)
    // given its center, half-dimensions, up, and right axes.
    // NOTE: vec3_t params are non-const because the project's vector helpers don't take const.
    inline void ComputePlaneCorners(vec3_t center, float halfHeight, float halfWidth,
                                    vec3_t up, vec3_t right,
                                    vec3_t outTL, vec3_t outTR, vec3_t outBR, vec3_t outBL)
    {
        vec3_t temp;
        VectorMA(center,  halfHeight, up, temp);  VectorMA(temp, -halfWidth, right, outTL);
        VectorMA(center,  halfHeight, up, temp);  VectorMA(temp,  halfWidth, right, outTR);
        VectorMA(center, -halfHeight, up, temp);  VectorMA(temp,  halfWidth, right, outBR);
        VectorMA(center, -halfHeight, up, temp);  VectorMA(temp, -halfWidth, right, outBL);
    }

}

Frustum::Frustum()
    : m_2DCount(0)
    , m_bHasTerrainExtension(false)
{
    for (int i = 0; i < 6; i++)
    {
        Vector(0.f, 0.f, 0.f, m_Planes[i].normal);
        m_Planes[i].distance = 0.f;
    }

    for (int i = 0; i < 8; i++)
    {
        Vector(0.f, 0.f, 0.f, m_Vertices[i]);
    }

    for (int i = 0; i < 4; i++)
    {
        Vector(0.f, 0.f, 0.f, m_TerrainFarVertices[i]);
    }

    for (int i = 0; i < MAX_2D_HULL_POINTS; i++)
    {
        m_2DX[i] = 0.f;
        m_2DY[i] = 0.f;
    }
}

void Frustum::BuildFromCamera(const vec3_t position, const vec3_t forward, const vec3_t up,
                               float fovDegrees, float aspectRatio, float nearDist, float farDist,
                               float terrainCullDist)
{
    if (terrainCullDist < 0.0f)
        terrainCullDist = farDist;

    // Build orthonormal basis from camera vectors (CrossProduct needs non-const args)
    vec3_t right, forwardTemp, upTemp;
    VectorCopy(forward, forwardTemp);
    VectorCopy(up, upTemp);
    CrossProduct(forwardTemp, upTemp, right);
    VectorNormalize(right);

    vec3_t actualUp;
    CrossProduct(right, forwardTemp, actualUp);
    VectorNormalize(actualUp);

    float tanHalfFov = tanf((fovDegrees * Q_PI / 180.0f) * 0.5f);

    // Near/far plane centers, needed by CalculatePlanes
    vec3_t nearCenter, farCenter, posTemp;
    VectorCopy(position, posTemp);
    VectorMA(posTemp, nearDist, forwardTemp, nearCenter);
    VectorCopy(position, posTemp);
    VectorMA(posTemp, farDist, forwardTemp, farCenter);

    CalculateFrustumVertices(position, forward, actualUp, right, tanHalfFov, aspectRatio, nearDist, farDist);

    m_bHasTerrainExtension = (terrainCullDist > farDist + TERRAIN_EXTENSION_EPSILON);
    if (m_bHasTerrainExtension)
        CalculateTerrainExtension(position, forward, actualUp, right, tanHalfFov, aspectRatio, farDist, terrainCullDist);

    CalculatePlanes(position, forward, nearCenter, farCenter);
    CalculateBoundingBox();
    Calculate2DProjection();
}

void Frustum::SetCustom2DHull(const float* xs, const float* ys, int count)
{
    if (!xs || !ys || count <= 0) return;
    if (count > MAX_2D_HULL_POINTS) count = MAX_2D_HULL_POINTS;

    // Run the points through SortPoints2D + ConvexHullCCW to produce a proper
    // convex hull in CCW order. Callers (e.g. OrbitalCamera) pass frustum corners
    // in view-space walking order, which is NOT the hull outline — rendering
    // would connect edges across the hull interior and look broken. The hull
    // computation fixes the winding for any input order.
    Point2D input[MAX_2D_HULL_POINTS];
    for (int i = 0; i < count; i++) { input[i].x = xs[i]; input[i].y = ys[i]; }

    SortPoints2D(input, count);

    // Andrew's monotone chain produces at most `count` hull points for `count`
    // inputs; bounding the output buffer to MAX_2D_HULL_POINTS makes the
    // capacity match m_2DX/m_2DY storage so no truncation can happen.
    Point2D hull[MAX_2D_HULL_POINTS];
    int k = ConvexHullCCW(input, count, hull, MAX_2D_HULL_POINTS);

    // Reverse to CW order — TestPoint2D's cross-product winding test expects
    // CW. Calculate2DProjection does the same flip; without it the cull test
    // inverts and tiles inside the custom hull get culled instead of those
    // outside (visible when DevEditor's orbital trapezoid override is active).
    m_2DCount = k;
    for (int i = 0; i < k; i++)
    {
        m_2DX[i] = hull[k - 1 - i].x;
        m_2DY[i] = hull[k - 1 - i].y;
    }
}

void Frustum::CalculateFrustumVertices(const vec3_t position, const vec3_t forward,
                                        const vec3_t up, const vec3_t right,
                                        float tanHalfFov, float aspectRatio,
                                        float nearDist, float farDist)
{
    float nearHeight = 2.0f * tanHalfFov * nearDist;
    float nearWidth  = nearHeight * aspectRatio;
    float farHeight  = 2.0f * tanHalfFov * farDist;
    float farWidth   = farHeight * aspectRatio;

    vec3_t posMut, forwardMut, upMut, rightMut;
    VectorCopy(position, posMut);
    VectorCopy(forward,  forwardMut);
    VectorCopy(up,       upMut);
    VectorCopy(right,    rightMut);

    vec3_t nearCenter, farCenter, posTemp;
    VectorCopy(posMut, posTemp);
    VectorMA(posTemp, nearDist, forwardMut, nearCenter);
    VectorCopy(posMut, posTemp);
    VectorMA(posTemp, farDist, forwardMut, farCenter);

    // Near plane: vertices 0..3 = TL, TR, BR, BL
    ComputePlaneCorners(nearCenter, nearHeight * 0.5f, nearWidth * 0.5f, upMut, rightMut,
                        m_Vertices[0], m_Vertices[1], m_Vertices[2], m_Vertices[3]);
    // Far plane: vertices 4..7 = TL, TR, BR, BL
    ComputePlaneCorners(farCenter, farHeight * 0.5f, farWidth * 0.5f, upMut, rightMut,
                        m_Vertices[4], m_Vertices[5], m_Vertices[6], m_Vertices[7]);
}

void Frustum::CalculateTerrainExtension(const vec3_t position, const vec3_t forward,
                                         const vec3_t up, const vec3_t right,
                                         float tanHalfFov, float aspectRatio,
                                         float /*farDist*/, float terrainCullDist)
{
    // When terrainCullDist > farDist, the 2D terrain tile culling range extends
    // beyond the 3D frustum far plane to match the rendering extent.
    float tcHeight = 2.0f * tanHalfFov * terrainCullDist;
    float tcWidth  = tcHeight * aspectRatio;

    vec3_t posMut, forwardMut, upMut, rightMut;
    VectorCopy(position, posMut);
    VectorCopy(forward,  forwardMut);
    VectorCopy(up,       upMut);
    VectorCopy(right,    rightMut);

    vec3_t tcCenter, posTemp;
    VectorCopy(posMut, posTemp);
    VectorMA(posTemp, terrainCullDist, forwardMut, tcCenter);

    ComputePlaneCorners(tcCenter, tcHeight * 0.5f, tcWidth * 0.5f, upMut, rightMut,
                        m_TerrainFarVertices[0], m_TerrainFarVertices[1],
                        m_TerrainFarVertices[2], m_TerrainFarVertices[3]);
}

void Frustum::CalculatePlanes(const vec3_t position, const vec3_t forward,
                               const vec3_t nearCenter, const vec3_t farCenter)
{
    // Phase 5: CRITICAL -- match old CreateFrustrum() plane indices!
    // Old winding: [0]=TOP, [1]=RIGHT, [2]=BOTTOM, [3]=LEFT
    // TestFrustrum() relies on this ordering.
    struct PlaneDef { int v2, v3; };
    const PlaneDef planeDefs[4] = {
        { 4, 5 },  // TOP:    apex → far-TL → far-TR
        { 5, 6 },  // RIGHT:  apex → far-TR → far-BR
        { 6, 7 },  // BOTTOM: apex → far-BR → far-BL
        { 7, 4 },  // LEFT:   apex → far-BL → far-TL
    };

    vec3_t v1, v2, v3;
    for (int i = 0; i < 4; i++)
    {
        VectorCopy(position, v1);
        VectorCopy(m_Vertices[planeDefs[i].v2], v2);
        VectorCopy(m_Vertices[planeDefs[i].v3], v3);
        FaceNormalize(v1, v2, v3, m_Planes[i].normal);
        m_Planes[i].distance = -DotProduct(position, m_Planes[i].normal);
    }

    // Near plane: forward direction
    VectorCopy(forward, m_Planes[4].normal);
    m_Planes[4].distance = -DotProduct(nearCenter, m_Planes[4].normal);

    // Far plane: negative forward direction (VectorScale needs non-const input)
    vec3_t forwardMut;
    VectorCopy(forward, forwardMut);
    VectorScale(forwardMut, -1.0f, m_Planes[5].normal);
    m_Planes[5].distance = -DotProduct(farCenter, m_Planes[5].normal);
}

bool Frustum::TestSphere(const vec3_t center, float radius) const
{
    // Test sphere against all 6 planes
    for (int i = 0; i < 6; i++)
    {
        float distance = DotProduct(center, m_Planes[i].normal) + m_Planes[i].distance;

        // If sphere is completely outside this plane, it's culled
        if (distance < -radius)
            return false;
    }

    return true;
}

bool Frustum::TestAABB(const AABB& box) const
{
    // Test AABB against all 6 planes using p-vertex method
    for (int i = 0; i < 6; i++)
    {
        // Find the most positive vertex along plane normal
        vec3_t pVertex;
        pVertex[0] = (m_Planes[i].normal[0] >= 0.0f) ? box.max[0] : box.min[0];
        pVertex[1] = (m_Planes[i].normal[1] >= 0.0f) ? box.max[1] : box.min[1];
        pVertex[2] = (m_Planes[i].normal[2] >= 0.0f) ? box.max[2] : box.min[2];

        // If p-vertex is outside, entire box is outside
        if (DotProduct(pVertex, m_Planes[i].normal) + m_Planes[i].distance < 0.0f)
            return false;
    }

    return true;
}

bool Frustum::TestPoint(const vec3_t point) const
{
    // Test point against all 6 planes
    for (int i = 0; i < 6; i++)
    {
        float distance = DotProduct(point, m_Planes[i].normal) + m_Planes[i].distance;

        // If point is outside this plane, it's culled
        if (distance < 0.0f)
            return false;
    }

    return true;
}

void Frustum::CalculateBoundingBox()
{
    // Initialize with first vertex
    VectorCopy(m_Vertices[0], m_BoundingBox.min);
    VectorCopy(m_Vertices[0], m_BoundingBox.max);

    // Expand to include all vertices
    for (int i = 1; i < 8; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (m_Vertices[i][j] < m_BoundingBox.min[j])
                m_BoundingBox.min[j] = m_Vertices[i][j];
            if (m_Vertices[i][j] > m_BoundingBox.max[j])
                m_BoundingBox.max[j] = m_Vertices[i][j];
        }
    }
}

void Frustum::Calculate2DProjection()
{
    // Project frustum vertices to XY ground plane in tile coordinates (world / TERRAIN_SCALE).
    // Include terrain-cull far vertices if present (extends 2D hull beyond 3D far plane).
    constexpr float WORLD_TO_TILE = 1.0f / TERRAIN_SCALE;

    int numPts = 8 + (m_bHasTerrainExtension ? 4 : 0);
    Point2D pts[MAX_2D_HULL_POINTS];

    for (int i = 0; i < 8; i++)
    {
        pts[i].x = m_Vertices[i][0] * WORLD_TO_TILE;
        pts[i].y = m_Vertices[i][1] * WORLD_TO_TILE;
    }
    if (m_bHasTerrainExtension)
    {
        for (int i = 0; i < 4; i++)
        {
            pts[8 + i].x = m_TerrainFarVertices[i][0] * WORLD_TO_TILE;
            pts[8 + i].y = m_TerrainFarVertices[i][1] * WORLD_TO_TILE;
        }
    }

    SortPoints2D(pts, numPts);

    Point2D hull[MAX_2D_HULL_POINTS];
    int k = ConvexHullCCW(pts, numPts, hull, MAX_2D_HULL_POINTS);

    // Reverse to CW order (the original TestFrustrum2D cross-product test expects CW winding)
    m_2DCount = k;
    for (int i = 0; i < k; i++)
    {
        m_2DX[i] = hull[k - 1 - i].x;
        m_2DY[i] = hull[k - 1 - i].y;
    }
}

bool Frustum::TestPoint2D(float tileX, float tileY, float range) const
{
    if (m_2DCount < 3)
        return true; // No valid 2D projection, treat as visible

    // Same cross-product winding test as original TestFrustrum2D
    // For CW-ordered polygon: d > range means inside, d <= range means outside
    int j = m_2DCount - 1;
    for (int i = 0; i < m_2DCount; j = i, i++)
    {
        float d = (m_2DX[i] - tileX) * (m_2DY[j] - tileY) -
                  (m_2DX[j] - tileX) * (m_2DY[i] - tileY);
        if (d <= range)
            return false;
    }
    return true;
}

