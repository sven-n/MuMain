#include "stdafx.h"
#include "Frustum.h"
#include "_define.h"
#include <cmath>

Frustum::Frustum()
    : m_2DCount(0)
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

    for (int i = 0; i < 8; i++)
    {
        m_2DX[i] = 0.f;
        m_2DY[i] = 0.f;
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

    // Calculate 2D ground-plane projection for cheap culling
    Calculate2DProjection();
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
    // Project all 8 frustum vertices to XY ground plane in tile coordinates
    struct Point2D { float x, y; };
    Point2D pts[8];
    for (int i = 0; i < 8; i++)
    {
        pts[i].x = m_Vertices[i][0] * 0.01f;
        pts[i].y = m_Vertices[i][1] * 0.01f;
    }

    // Sort by X, then Y (simple insertion sort for N=8)
    for (int i = 1; i < 8; i++)
    {
        Point2D key = pts[i];
        int j = i - 1;
        while (j >= 0 && (pts[j].x > key.x || (pts[j].x == key.x && pts[j].y > key.y)))
        {
            pts[j + 1] = pts[j];
            j--;
        }
        pts[j + 1] = key;
    }

    // Andrew's monotone chain convex hull (produces CCW ordering)
    Point2D hull[16];
    int k = 0;

    // Lower hull
    for (int i = 0; i < 8; i++)
    {
        while (k >= 2)
        {
            float cross = (hull[k-1].x - hull[k-2].x) * (pts[i].y - hull[k-2].y)
                        - (hull[k-1].y - hull[k-2].y) * (pts[i].x - hull[k-2].x);
            if (cross <= 0.f) k--;
            else break;
        }
        hull[k++] = pts[i];
    }

    // Upper hull
    int lowerSize = k + 1;
    for (int i = 6; i >= 0; i--)
    {
        while (k >= lowerSize)
        {
            float cross = (hull[k-1].x - hull[k-2].x) * (pts[i].y - hull[k-2].y)
                        - (hull[k-1].y - hull[k-2].y) * (pts[i].x - hull[k-2].x);
            if (cross <= 0.f) k--;
            else break;
        }
        hull[k++] = pts[i];
    }
    k--; // Remove duplicate last point

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

