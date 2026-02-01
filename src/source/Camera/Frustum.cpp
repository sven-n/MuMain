#include "stdafx.h"
#include "Frustum.h"
#include "_define.h"
#include <cmath>

Frustum::Frustum()
    : m_TerrainTileMinX(0)
    , m_TerrainTileMinY(0)
    , m_TerrainTileMaxX(0)
    , m_TerrainTileMaxY(0)
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
        Vector(0.f, 0.f, 0.f, m_GroundProjection[i]);
    }
}

void Frustum::BuildFromCamera(const vec3_t position, const vec3_t forward, const vec3_t up,
                               float fovDegrees, float aspectRatio, float nearDist, float farDist,
                               float terrainCullDist)
{
    // If terrainCullDist not specified, use farDist for terrain culling
    if (terrainCullDist < 0.0f)
        terrainCullDist = farDist;
    // Calculate right vector (need temp copies - CrossProduct doesn't accept const)
    vec3_t right, forwardTemp, upTemp;
    VectorCopy(forward, forwardTemp);
    VectorCopy(up, upTemp);
    CrossProduct(forwardTemp, upTemp, right);
    VectorNormalize(right);

    // Calculate actual up vector (ensure orthogonal)
    vec3_t actualUp;
    VectorCopy(forward, forwardTemp);
    CrossProduct(right, forwardTemp, actualUp);
    VectorNormalize(actualUp);

    // Calculate frustum dimensions
    float fovRadians = fovDegrees * Q_PI / 180.0f;
    float tanHalfFov = tanf(fovRadians * 0.5f);

    float nearHeight = 2.0f * tanHalfFov * nearDist;
    float nearWidth = nearHeight * aspectRatio;

    float farHeight = 2.0f * tanHalfFov * farDist;
    float farWidth = farHeight * aspectRatio;

    // Calculate centers of near and far planes
    vec3_t nearCenter, farCenter;
    vec3_t posTemp2, forwardTemp2;
    VectorCopy(position, posTemp2);
    VectorCopy(forward, forwardTemp2);
    VectorMA(posTemp2, nearDist, forwardTemp2, nearCenter);
    VectorCopy(position, posTemp2);
    VectorCopy(forward, forwardTemp2);
    VectorMA(posTemp2, farDist, forwardTemp2, farCenter);

    // Calculate the 8 corner vertices
    // Near plane corners
    vec3_t temp1, temp2;

    // Near top-left
    VectorMA(nearCenter, nearHeight * 0.5f, actualUp, temp1);
    VectorMA(temp1, -nearWidth * 0.5f, right, m_Vertices[0]);

    // Near top-right
    VectorMA(nearCenter, nearHeight * 0.5f, actualUp, temp1);
    VectorMA(temp1, nearWidth * 0.5f, right, m_Vertices[1]);

    // Near bottom-right
    VectorMA(nearCenter, -nearHeight * 0.5f, actualUp, temp1);
    VectorMA(temp1, nearWidth * 0.5f, right, m_Vertices[2]);

    // Near bottom-left
    VectorMA(nearCenter, -nearHeight * 0.5f, actualUp, temp1);
    VectorMA(temp1, -nearWidth * 0.5f, right, m_Vertices[3]);

    // Far plane corners
    // Far top-left
    VectorMA(farCenter, farHeight * 0.5f, actualUp, temp1);
    VectorMA(temp1, -farWidth * 0.5f, right, m_Vertices[4]);

    // Far top-right
    VectorMA(farCenter, farHeight * 0.5f, actualUp, temp1);
    VectorMA(temp1, farWidth * 0.5f, right, m_Vertices[5]);

    // Far bottom-right
    VectorMA(farCenter, -farHeight * 0.5f, actualUp, temp1);
    VectorMA(temp1, farWidth * 0.5f, right, m_Vertices[6]);

    // Far bottom-left
    VectorMA(farCenter, -farHeight * 0.5f, actualUp, temp1);
    VectorMA(temp1, -farWidth * 0.5f, right, m_Vertices[7]);

    // Calculate the 6 frustum planes
    // Need temporary copies for FaceNormalize (doesn't accept const)
    vec3_t v1, v2, v3;

    // Phase 5: CRITICAL FIX - Match old CreateFrustrum() plane indices!
    // Old system had planes in different order due to vertex winding:
    // Old [0] = TOP, [1] = RIGHT, [2] = BOTTOM, [3] = LEFT (rotated 90° from expected!)
    // New [0] = LEFT, [1] = RIGHT, [2] = TOP, [3] = BOTTOM (standard order)
    // Must use old winding to match TestFrustrum() expectations

    // Plane [0]: Old TOP plane - apex → far-left-top → far-right-top
    VectorCopy(position, v1);
    VectorCopy(m_Vertices[4], v2);  // Far top-left
    VectorCopy(m_Vertices[5], v3);  // Far top-right
    FaceNormalize(v1, v2, v3, m_Planes[0].normal);
    m_Planes[0].distance = -DotProduct(position, m_Planes[0].normal);

    // Plane [1]: Old RIGHT plane - apex → far-right-top → far-right-bottom
    VectorCopy(position, v1);
    VectorCopy(m_Vertices[5], v2);  // Far top-right
    VectorCopy(m_Vertices[6], v3);  // Far bottom-right
    FaceNormalize(v1, v2, v3, m_Planes[1].normal);
    m_Planes[1].distance = -DotProduct(position, m_Planes[1].normal);

    // Plane [2]: Old BOTTOM plane - apex → far-right-bottom → far-left-bottom
    VectorCopy(position, v1);
    VectorCopy(m_Vertices[6], v2);  // Far bottom-right
    VectorCopy(m_Vertices[7], v3);  // Far bottom-left
    FaceNormalize(v1, v2, v3, m_Planes[2].normal);
    m_Planes[2].distance = -DotProduct(position, m_Planes[2].normal);

    // Plane [3]: Old LEFT plane - apex → far-left-bottom → far-left-top
    VectorCopy(position, v1);
    VectorCopy(m_Vertices[7], v2);  // Far bottom-left
    VectorCopy(m_Vertices[4], v3);  // Far top-left
    FaceNormalize(v1, v2, v3, m_Planes[3].normal);
    m_Planes[3].distance = -DotProduct(position, m_Planes[3].normal);

    // Near plane: forward direction
    VectorCopy(forward, m_Planes[4].normal);
    m_Planes[4].distance = -DotProduct(nearCenter, m_Planes[4].normal);

    // Far plane: negative forward direction
    VectorScale(forward, -1.0f, m_Planes[5].normal);
    m_Planes[5].distance = -DotProduct(farCenter, m_Planes[5].normal);

    // Calculate bounding box
    CalculateBoundingBox();

    // Calculate 2D ground projection for terrain culling
    // Use terrainCullDist for terrain (separate from 3D object farDist)
    float terrainFarHeight = 2.0f * tanHalfFov * terrainCullDist;
    float terrainFarWidth = terrainFarHeight * aspectRatio;
    CalculateGroundProjection(position, forward, actualUp, nearDist, terrainCullDist, nearWidth, terrainFarWidth);
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

void Frustum::CalculateGroundProjection(const vec3_t position, const vec3_t forward,
                                        const vec3_t up, float nearDist, float terrainFarDist,
                                        float nearWidth, float terrainFarWidth)
{
    // Calculate right vector (need temp copies - CrossProduct doesn't accept const)
    vec3_t right, forwardTemp, upTemp;
    VectorCopy(forward, forwardTemp);
    VectorCopy(up, upTemp);
    CrossProduct(forwardTemp, upTemp, right);
    VectorNormalize(right);

    // Project forward onto XY plane (ground)
    vec3_t groundForward;
    groundForward[0] = forward[0];
    groundForward[1] = forward[1];
    groundForward[2] = 0.0f;

    // FIX: When looking straight down, ground forward becomes (0,0,0)
    // In this case, use a much larger radius for terrain culling
    float groundLength = VectorLength(groundForward);
    bool lookingStraightDown = (groundLength < 0.1f);

    if (lookingStraightDown)
    {
        // Looking nearly straight down - use full circular radius
        // Multiply terrainFarDist by 1.5x to ensure we see enough terrain
        terrainFarDist *= 1.5f;
        terrainFarWidth *= 1.5f;

        // Use arbitrary forward direction for trapezoid calculation
        groundForward[0] = 0.0f;
        groundForward[1] = 1.0f;
        groundForward[2] = 0.0f;
    }
    else
    {
        VectorNormalize(groundForward);
    }

    // Project right onto XY plane
    vec3_t groundRight;
    groundRight[0] = right[0];
    groundRight[1] = right[1];
    groundRight[2] = 0.0f;
    VectorNormalize(groundRight);

    // Calculate ground projection of camera position
    vec3_t groundPosition;
    groundPosition[0] = position[0];
    groundPosition[1] = position[1];
    groundPosition[2] = 0.0f;

    // Calculate near and far centers on ground
    vec3_t nearCenter, farCenter;
    VectorMA(groundPosition, nearDist, groundForward, nearCenter);
    VectorMA(groundPosition, terrainFarDist, groundForward, farCenter);

    // Calculate the 4 trapezoid vertices
    vec3_t temp;

    // Far-left
    VectorMA(farCenter, -terrainFarWidth * 0.5f, groundRight, m_GroundProjection[0]);

    // Far-right
    VectorMA(farCenter, terrainFarWidth * 0.5f, groundRight, m_GroundProjection[1]);

    // Near-right
    VectorMA(nearCenter, nearWidth * 0.5f, groundRight, m_GroundProjection[2]);

    // Near-left
    VectorMA(nearCenter, -nearWidth * 0.5f, groundRight, m_GroundProjection[3]);

    // Calculate terrain tile bounds
    float minX = TERRAIN_SIZE * TERRAIN_SCALE;
    float minY = TERRAIN_SIZE * TERRAIN_SCALE;
    float maxX = 0.0f;
    float maxY = 0.0f;

    for (int i = 0; i < 4; i++)
    {
        if (m_GroundProjection[i][0] < minX) minX = m_GroundProjection[i][0];
        if (m_GroundProjection[i][1] < minY) minY = m_GroundProjection[i][1];
        if (m_GroundProjection[i][0] > maxX) maxX = m_GroundProjection[i][0];
        if (m_GroundProjection[i][1] > maxY) maxY = m_GroundProjection[i][1];
    }

    // Convert to tile coordinates with padding
    int tileWidth = 4;
    m_TerrainTileMinX = (int)(minX / TERRAIN_SCALE) / tileWidth * tileWidth - tileWidth;
    m_TerrainTileMinY = (int)(minY / TERRAIN_SCALE) / tileWidth * tileWidth - tileWidth;
    m_TerrainTileMaxX = (int)(maxX / TERRAIN_SCALE) / tileWidth * tileWidth + tileWidth;
    m_TerrainTileMaxY = (int)(maxY / TERRAIN_SCALE) / tileWidth * tileWidth + tileWidth;

    // Clamp to terrain bounds
    if (m_TerrainTileMinX < 0) m_TerrainTileMinX = 0;
    if (m_TerrainTileMinY < 0) m_TerrainTileMinY = 0;
    if (m_TerrainTileMaxX > TERRAIN_SIZE_MASK - tileWidth)
        m_TerrainTileMaxX = TERRAIN_SIZE_MASK - tileWidth;
    if (m_TerrainTileMaxY > TERRAIN_SIZE_MASK - tileWidth)
        m_TerrainTileMaxY = TERRAIN_SIZE_MASK - tileWidth;
}

bool Frustum::Test2D(float x, float y, float radius) const
{
    // Test point against the 4 edges of the ground projection trapezoid
    // This matches the TestFrustrum2D algorithm exactly
    // The trapezoid vertices are: [0]=far-left, [1]=far-right, [2]=near-right, [3]=near-left

    int j = 3;
    for (int i = 0; i < 4; j = i, i++)
    {
        // Calculate signed distance from point to edge (i,j)
        // This is the cross product in 2D: (edge_x * point_y - edge_y * point_x)
        float d = (m_GroundProjection[i][0] - x) * (m_GroundProjection[j][1] - y) -
                  (m_GroundProjection[j][0] - x) * (m_GroundProjection[i][1] - y);

        // If point is outside this edge (beyond tolerance), it's culled
        if (d <= radius)
        {
            return false;  // Outside trapezoid
        }
    }

    return true;  // Inside trapezoid
}
