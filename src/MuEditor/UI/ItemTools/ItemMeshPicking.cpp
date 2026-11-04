#include "stdafx.h"

#ifdef _EDITOR

#include "ItemMeshPicking.h"

#include "../MuEditor/UI/EffectBrowser/EffectPreviewCamera.h"
#include "Render/Models/ZzzBMD.h"

#include <cmath>
#include <numbers>

namespace MuEditor::Items::MeshPicking
{
namespace
{
// Below this the ray runs along the triangle's plane.
constexpr float ParallelEpsilon = 1e-9f;
constexpr int QuadCorners = 4;

Vector Subtract(const float* a, const float* b)
{
    return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}

Vector Cross(const Vector& a, const Vector& b)
{
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}

float Dot(const Vector& a, const Vector& b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

Vector Normalized(const Vector& v)
{
    const float length = std::sqrt(Dot(v, v));
    return length > 0.0f ? Vector{v[0] / length, v[1] / length, v[2] / length} : v;
}

bool IsVertex(const Mesh_t& mesh, short index)
{
    return index >= 0 && index < mesh.NumVertices;
}
} // namespace

Ray RayThroughView(const MuEditor::Effects::EffectPreviewCamera& camera, float aspect, float viewX, float viewY)
{
    using MuEditor::Effects::EffectPreviewCamera;
    const float halfHeight =
        std::tan(EffectPreviewCamera::FieldOfViewDegrees * 0.5f * std::numbers::pi_v<float> / 180.0f);
    const float across = viewX * halfHeight * aspect;
    const float up = viewY * halfHeight;
    const EffectPreviewCamera::Basis basis = camera.GetBasis();
    Vector direction{};
    for (int axis = 0; axis < 3; ++axis)
    {
        direction[axis] = basis.forward[axis] + basis.right[axis] * across + basis.up[axis] * up;
    }
    return {camera.Eye(), Normalized(direction)};
}

// Möller and Trumbore's test, without culling either side.
std::optional<float> HitTriangle(const Ray& ray, const float* a, const float* b, const float* c)
{
    const Vector edge1 = Subtract(b, a);
    const Vector edge2 = Subtract(c, a);
    const Vector p = Cross(ray.direction, edge2);
    const float determinant = Dot(edge1, p);
    if (std::abs(determinant) < ParallelEpsilon)
    {
        return std::nullopt;
    }
    const float inverse = 1.0f / determinant;
    const Vector fromA = Subtract(ray.origin.data(), a);
    const float u = Dot(fromA, p) * inverse;
    if (u < 0.0f || u > 1.0f)
    {
        return std::nullopt;
    }
    const Vector q = Cross(fromA, edge1);
    const float v = Dot(ray.direction, q) * inverse;
    if (v < 0.0f || u + v > 1.0f)
    {
        return std::nullopt;
    }
    const float distance = Dot(edge2, q) * inverse;
    if (distance < 0.0f)
    {
        return std::nullopt;
    }
    return distance;
}

std::optional<float> HitMesh(const Ray& ray, const BMD& model, int mesh, float (*vertices)[3])
{
    if (mesh < 0 || mesh >= model.NumMeshs || model.Meshs == nullptr)
    {
        return std::nullopt;
    }
    const Mesh_t& meshData = model.Meshs[mesh];
    std::optional<float> nearest;
    const auto keepNearer = [&nearest](std::optional<float> hit)
    {
        if (hit && (!nearest || *hit < *nearest))
        {
            nearest = hit;
        }
    };
    for (int index = 0; index < meshData.NumTriangles; ++index)
    {
        const Triangle_t& triangle = meshData.Triangles[index];
        const short* corner = triangle.VertexIndex;
        if (!IsVertex(meshData, corner[0]) || !IsVertex(meshData, corner[1]) || !IsVertex(meshData, corner[2]))
        {
            continue;
        }
        keepNearer(HitTriangle(ray, vertices[corner[0]], vertices[corner[1]], vertices[corner[2]]));
        if (triangle.Polygon == QuadCorners && IsVertex(meshData, corner[3]))
        {
            keepNearer(HitTriangle(ray, vertices[corner[0]], vertices[corner[2]], vertices[corner[3]]));
        }
    }
    return nearest;
}
} // namespace MuEditor::Items::MeshPicking

#endif // _EDITOR
