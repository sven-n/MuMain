#pragma once

#include "_types.h"

/**
 * @brief Axis-Aligned Bounding Box for culling tests
 */
struct AABB
{
    vec3_t min;
    vec3_t max;

    AABB()
    {
        Vector(0.f, 0.f, 0.f, min);
        Vector(0.f, 0.f, 0.f, max);
    }

    AABB(const vec3_t& minimum, const vec3_t& maximum)
    {
        VectorCopy(minimum, min);
        VectorCopy(maximum, max);
    }
};

/**
 * @brief Unified frustum representation for camera culling
 *
 * Replaces the old dual frustum system (3D pyramid + 2D trapezoid).
 * Provides both 3D object culling and 2D terrain tile culling.
 */
class Frustum
{
public:
    /**
     * @brief Frustum plane definition
     */
    struct Plane
    {
        vec3_t normal;    // Plane normal (normalized)
        float distance;   // Distance from origin (D in plane equation Ax + By + Cz + D = 0)

        Plane()
        {
            Vector(0.f, 0.f, 0.f, normal);
            distance = 0.f;
        }
    };

    Frustum();
    ~Frustum() = default;

    /**
     * @brief Builds frustum from camera parameters
     *
     * @param position Camera world position
     * @param forward Camera forward vector (normalized)
     * @param up Camera up vector (normalized)
     * @param fovDegrees Vertical field of view in degrees
     * @param aspectRatio Width / Height ratio
     * @param nearDist Near clipping plane distance
     * @param farDist Far clipping plane distance
     */
    void BuildFromCamera(const vec3_t position, const vec3_t forward, const vec3_t up,
                         float fovDegrees, float aspectRatio, float nearDist, float farDist);

    /**
     * @brief Tests if a sphere is inside or intersecting the frustum
     *
     * @param center Sphere center in world space
     * @param radius Sphere radius
     * @return true if sphere is visible, false if completely outside
     */
    bool TestSphere(const vec3_t center, float radius) const;

    /**
     * @brief Tests if an AABB is inside or intersecting the frustum
     *
     * @param box Axis-aligned bounding box
     * @return true if box is visible, false if completely outside
     */
    bool TestAABB(const AABB& box) const;

    /**
     * @brief Tests if a point is inside the frustum
     *
     * @param point Point in world space
     * @return true if point is visible, false if outside
     */
    bool TestPoint(const vec3_t point) const;

    /**
     * @brief Gets the 8 corner vertices of the frustum
     *
     * Vertices are ordered:
     * [0-3] = Near plane corners (top-left, top-right, bottom-right, bottom-left)
     * [4-7] = Far plane corners (top-left, top-right, bottom-right, bottom-left)
     */
    const vec3_t* GetVertices() const { return m_Vertices; }

    /**
     * @brief Gets the 6 frustum planes (left, right, top, bottom, near, far)
     */
    const Plane* GetPlanes() const { return m_Planes; }

    /**
     * @brief Gets the axis-aligned bounding box that contains the frustum
     */
    const AABB& GetBoundingBox() const { return m_BoundingBox; }

    // ========== Terrain-Specific 2D Projection ==========
    // For backward compatibility with terrain tile culling

    /**
     * @brief Gets the 2D ground projection of the frustum (trapezoid on XY plane)
     *
     * Returns 4 vertices in world space, projected onto ground (Z coordinate from terrain height)
     * Ordered: far-left, far-right, near-right, near-left
     */
    const vec3_t* GetGroundProjection() const { return m_GroundProjection; }

    /**
     * @brief Gets the terrain tile bounding box for efficient culling
     *
     * Terrain tiles are indexed in world space divided by TERRAIN_SCALE (100)
     */
    void GetTerrainTileBounds(int* outMinX, int* outMinY, int* outMaxX, int* outMaxY) const
    {
        if (outMinX) *outMinX = m_TerrainTileMinX;
        if (outMinY) *outMinY = m_TerrainTileMinY;
        if (outMaxX) *outMaxX = m_TerrainTileMaxX;
        if (outMaxY) *outMaxY = m_TerrainTileMaxY;
    }

    /**
     * @brief Tests if a 2D circle (XY only) is inside the ground projection trapezoid
     *
     * This matches TestFrustrum2D behavior - only tests XY position, ignores Z
     *
     * @param x World X coordinate
     * @param y World Y coordinate
     * @param radius Tolerance/radius for the test
     * @return true if visible, false if outside trapezoid
     */
    bool Test2D(float x, float y, float radius) const;

private:
    // 6 frustum planes: Left, Right, Top, Bottom, Near, Far
    Plane m_Planes[6];

    // 8 corner vertices of the frustum box
    vec3_t m_Vertices[8];

    // Axis-aligned bounding box containing the frustum
    AABB m_BoundingBox;

    // 2D ground projection (trapezoid) - for terrain culling compatibility
    vec3_t m_GroundProjection[4];

    // Terrain tile bounds (in tile coordinates, not world space)
    int m_TerrainTileMinX;
    int m_TerrainTileMinY;
    int m_TerrainTileMaxX;
    int m_TerrainTileMaxY;

    // Helper methods
    void CalculateBoundingBox();
    void CalculateGroundProjection(const vec3_t position, const vec3_t forward,
                                   const vec3_t up, float nearDist, float farDist,
                                   float nearWidth, float farWidth);
};
