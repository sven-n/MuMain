#include "stdafx.h"

#include "Render/Models/SkinnedVertexPacking.h"

#include "Render/Models/Gltf/SkinnedModel.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numeric>
#include <unordered_map>

namespace Render::Models
{
namespace
{
static_assert(MAX_BONES <= 256, "skinned_textured.vert reads joints as bytes");

constexpr int MaxWeightsPerVertex = 4;
constexpr float WeightUnits = 255.f;
// SDL_GPU_VERTEXELEMENTFORMAT_SHORT4_NORM: -32767 to 32767 is -1 to 1.
constexpr std::int16_t SnormUnits = 32767;

// The weight indices of a vertex, the largest weight first (equal weights keep
// their order). Insertion sort: at most four entries, per vertex and draw.
std::array<int, MaxWeightsPerVertex> ByWeight(const Gltf::SkinnedVertex& vertex)
{
    std::array<int, MaxWeightsPerVertex> order{};
    std::iota(order.begin(), order.end(), 0);
    for (int i = 1; i < vertex.count; ++i)
    {
        const int index = order[i];
        int j = i;
        for (; j > 0 && vertex.weights[order[j - 1]] < vertex.weights[index]; --j)
        {
            order[j] = order[j - 1];
        }
        order[j] = index;
    }
    return order;
}
} // namespace

mu::SkinnedVertex3D PackBmdSkinnedVertex(const float position[3], int positionBone, const float normal[3],
                                         int normalBone, float u, float v)
{
    return {position[0], position[1], position[2], normal[0], normal[1], normal[2], u, v, positionBone, normalBone};
}

mu::SkinnedWeightedVertex3D PackWeightedSkinnedVertex(const Gltf::SkinnedVertex& vertex, float u, float v)
{
    const std::array<int, MaxWeightsPerVertex> order = ByWeight(vertex);
    const int dominantJoint = vertex.joints[order[0]];
    mu::SkinnedWeightedVertex3D packed{vertex.bindPosition[0],
                                       vertex.bindPosition[1],
                                       vertex.bindPosition[2],
                                       vertex.bindNormal[0],
                                       vertex.bindNormal[1],
                                       vertex.bindNormal[2],
                                       u,
                                       v,
                                       dominantJoint,
                                       dominantJoint};
    // The other weights are rounded; the largest takes the rest, so they sum to 255 exactly.
    int rest = static_cast<int>(WeightUnits);
    for (int k = 1; k < vertex.count; ++k)
    {
        const int units = static_cast<int>(std::lround(vertex.weights[order[k]] * WeightUnits));
        packed.joints[k] = static_cast<std::uint8_t>(vertex.joints[order[k]]);
        packed.weights[k] = static_cast<std::uint8_t>(units);
        rest -= units;
    }
    packed.joints[0] = static_cast<std::uint8_t>(dominantJoint);
    packed.weights[0] = static_cast<std::uint8_t>(std::max(rest, 1));
    const std::array<std::int16_t, 4> tangent = PackTangent(vertex.bindTangent);
    std::copy(tangent.begin(), tangent.end(), packed.tangent);
    return packed;
}

std::array<std::int16_t, 4> PackTangent(const std::array<float, 4>& tangent)
{
    if (tangent[3] == 0.f)
    {
        return {};
    }
    std::array<std::int16_t, 4> packed{};
    for (int c = 0; c < 3; ++c)
    {
        packed[c] = static_cast<std::int16_t>(std::lround(std::clamp(tangent[c], -1.f, 1.f) * SnormUnits));
    }
    packed[3] = static_cast<std::int16_t>(tangent[3] < 0.f ? -SnormUnits : SnormUnits);
    return packed;
}

void BuildWeightedSkinPalette(const Gltf::SkinnedModel& model, const float (*boneMatrices)[3][4], float boneScale,
                              float restPoseScale, std::vector<float>& palette)
{
    const float restScale = restPoseScale != 0.f ? restPoseScale : 1.f;
    const float scale = boneScale == 1.f ? restScale : boneScale;
    const size_t jointCount = model.joints.size();
    palette.resize(jointCount * 12);
    for (size_t joint = 0; joint < jointCount; ++joint)
    {
        const float (*bone)[4] = boneMatrices[joint];
        const Gltf::Matrix34& inverseBind = model.joints[joint].inverseBind;
        float* out = &palette[joint * 12];
        for (int r = 0; r < 3; ++r)
        {
            for (int c = 0; c < 4; ++c)
            {
                const float rotated =
                    bone[r][0] * inverseBind[0][c] + bone[r][1] * inverseBind[1][c] + bone[r][2] * inverseBind[2][c];
                out[r * 4 + c] = scale * rotated + (c == 3 ? bone[r][3] : 0.f);
            }
        }
    }
}

Gltf::PackedMesh& PackedWeightedMeshOf(const Gltf::SkinnedModel& model, const Mesh_t& mesh, int meshIndex, int lod,
                                       const Gltf::LodMesh* lodMesh)
{
    auto& meshes = model.packedMeshes;
    if (meshes.size() <= static_cast<std::size_t>(lod))
    {
        meshes.resize(static_cast<std::size_t>(lod) + 1);
    }
    if (meshes[lod].size() <= static_cast<std::size_t>(meshIndex))
    {
        meshes[lod].resize(static_cast<std::size_t>(meshIndex) + 1);
    }
    Gltf::PackedMesh& packed = meshes[lod][meshIndex];
    if (!packed.vertices.empty())
    {
        return packed;
    }

    // Corner c of the mesh: its vertex and texture coordinate (which identify it) and its packed vertex.
    struct Corner
    {
        std::uint64_t key;
        const Gltf::SkinnedVertex* vertex;
        float u;
        float v;
    };
    std::vector<Corner> corners;
    if (lodMesh)
    {
        corners.reserve(lodMesh->triangles.size() * 3);
        for (const auto& triangle : lodMesh->triangles)
        {
            for (const int vertexIndex : triangle)
            {
                const auto& texCoord = lodMesh->texCoords[vertexIndex];
                corners.push_back({static_cast<std::uint64_t>(vertexIndex), &lodMesh->vertices[vertexIndex],
                                   texCoord[0], texCoord[1]});
            }
        }
    }
    else
    {
        corners.reserve(static_cast<std::size_t>(mesh.NumTriangles) * 3);
        for (int j = 0; j < mesh.NumTriangles; ++j)
        {
            const Triangle_t& triangle = mesh.Triangles[j];
            for (int k = 0; k < triangle.Polygon; ++k)
            {
                const auto& texCoord = mesh.TexCoords[triangle.TexCoordIndex[k]];
                const auto key =
                    (static_cast<std::uint64_t>(static_cast<std::uint32_t>(triangle.VertexIndex[k])) << 32u) |
                    static_cast<std::uint32_t>(triangle.TexCoordIndex[k]);
                corners.push_back({key, &model.meshVertices[meshIndex][triangle.VertexIndex[k]], texCoord.TexCoordU,
                                   texCoord.TexCoordV});
            }
        }
    }

    const auto pack = [](const Corner& corner)
    { return PackWeightedSkinnedVertex(*corner.vertex, corner.u + 0.0f, corner.v + 0.0f); };
    std::unordered_map<std::uint64_t, std::uint16_t> indexOf;
    packed.indices.reserve(corners.size());
    for (const Corner& corner : corners)
    {
        const auto [found, added] = indexOf.try_emplace(corner.key, static_cast<std::uint16_t>(packed.vertices.size()));
        if (added)
        {
            if (packed.vertices.size() > std::numeric_limits<std::uint16_t>::max())
            {
                break;
            }
            packed.vertices.push_back(pack(corner));
        }
        packed.indices.push_back(found->second);
    }
    if (packed.indices.size() != corners.size())
    {
        // More distinct corners than 16-bit indices reach: the corners themselves.
        packed.indices.clear();
        packed.vertices.clear();
        packed.vertices.reserve(corners.size());
        for (const Corner& corner : corners)
        {
            packed.vertices.push_back(pack(corner));
        }
    }
    return packed;
}
} // namespace Render::Models
