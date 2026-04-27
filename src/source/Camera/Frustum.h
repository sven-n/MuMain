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
     * @param terrainCullDist Distance for terrain tile culling (defaults to farDist if not specified)
     */
    void BuildFromCamera(const vec3_t position, const vec3_t forward, const vec3_t up,
                         float fovDegrees, float aspectRatio, float nearDist, float farDist,
                         float terrainCullDist = -1.0f);

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

    /**
     * @brief Cheap 2D point-in-frustum test using ground-plane projection
     *
     * Tests if a point (in tile coordinates = world/100) is inside the 2D
     * convex hull of the frustum projected to the XY ground plane.
     * Uses the same cross-product winding test as the original TestFrustrum2D.
     *
     * @param tileX X coordinate in tile space (world / 100)
     * @param tileY Y coordinate in tile space (world / 100)
     * @param range Tolerance: negative = stricter culling, 0 = exact boundary
     * @return true if point is visible (inside frustum projection)
     */
    bool TestPoint2D(float tileX, float tileY, float range) const;

    /**
     * @brief Gets the number of vertices in the 2D ground-plane convex hull
     */
    int Get2DCount() const { return m_2DCount; }

    /**
     * @brief Gets the X coordinates of the 2D convex hull (tile space)
     */
    const float* Get2DX() const { return m_2DX; }

    /**
     * @brief Gets the Y coordinates of the 2D convex hull (tile space)
     */
    const float* Get2DY() const { return m_2DY; }

    /**
     * @brief Overwrites the 2D ground-plane hull with custom points.
     *
     * Used by DevEditor overrides to inject a user-defined trapezoid. The points
     * must already be in tile coordinates (world / 100). Up to 12 points.
     */
    void SetCustom2DHull(const float* xs, const float* ys, int count);

private:
    // 6 frustum planes: Left, Right, Top, Bottom, Near, Far
    Plane m_Planes[6];

    // 8 corner vertices of the frustum box
    vec3_t m_Vertices[8];

    // Axis-aligned bounding box containing the frustum
    AABB m_BoundingBox;

    // 2D ground-plane projection (convex hull of vertices projected to XY, in tile coords)
    // 8 frustum corners + up to 4 terrain-cull far corners
    static constexpr int MAX_2D_HULL_POINTS = 12;
    float m_2DX[MAX_2D_HULL_POINTS];
    float m_2DY[MAX_2D_HULL_POINTS];
    int m_2DCount;

    // Extended terrain cull far vertices (when terrainCullDist > farDist)
    vec3_t m_TerrainFarVertices[4];
    bool m_bHasTerrainExtension = false;

    // Helper methods
    void CalculateBoundingBox();
    void Calculate2DProjection();

    // BuildFromCamera helpers. Inputs are logically const but declared non-const because
    // the project's vec3_t helpers don't accept const; these methods copy to locals internally.
    void CalculateFrustumVertices(const vec3_t position, const vec3_t forward,
                                  const vec3_t up, const vec3_t right,
                                  float tanHalfFov, float aspectRatio,
                                  float nearDist, float farDist);
    void CalculateTerrainExtension(const vec3_t position, const vec3_t forward,
                                    const vec3_t up, const vec3_t right,
                                    float tanHalfFov, float aspectRatio,
                                    float farDist, float terrainCullDist);
    void CalculatePlanes(const vec3_t position, const vec3_t forward,
                         const vec3_t nearCenter, const vec3_t farCenter);
};
