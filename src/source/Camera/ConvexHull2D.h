#pragma once

// Minimal 2D convex hull utility shared by the camera frustum (Frustum.cpp)
// and the terrain culling hull (ZzzLodTerrain.cpp). Both build a small (<= a
// few dozen points) hull each frame from frustum corners, so an O(n^2)
// insertion sort plus Andrew's monotone chain is more than fast enough.

struct Point2D { float x, y; };

// z-component of the 3D cross product (b - a) x (c - a). Sign tells you the
// turn direction at b: positive = left turn (CCW), negative = right turn (CW).
inline float Cross2D(const Point2D& a, const Point2D& b, const Point2D& c)
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

// In-place insertion sort by X then Y. Stable; intended for small `count`.
inline void SortPoints2D(Point2D* pts, int count)
{
    for (int i = 1; i < count; i++)
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
}

// Andrew's monotone chain. `sortedPts` MUST already be sorted via SortPoints2D.
// Writes a CCW-ordered hull to `outHull` (capacity `hullCapacity`) and returns
// the number of hull points written. Caller is responsible for re-ordering to
// CW if their downstream test (e.g. TestFrustrum2D) expects that winding.
inline int ConvexHullCCW(const Point2D* sortedPts, int numPts,
                         Point2D* outHull, int hullCapacity)
{
    int k = 0;

    // Lower hull
    for (int i = 0; i < numPts; i++)
    {
        while (k >= 2 && Cross2D(outHull[k - 2], outHull[k - 1], sortedPts[i]) <= 0.f)
            k--;
        if (k < hullCapacity)
            outHull[k++] = sortedPts[i];
    }

    // Upper hull
    int lowerSize = k + 1;
    for (int i = numPts - 2; i >= 0; i--)
    {
        while (k >= lowerSize && Cross2D(outHull[k - 2], outHull[k - 1], sortedPts[i]) <= 0.f)
            k--;
        if (k < hullCapacity)
            outHull[k++] = sortedPts[i];
    }
    k--;  // Remove duplicate last point
    return k;
}
