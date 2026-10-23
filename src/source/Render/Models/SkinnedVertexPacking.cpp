#include "stdafx.h"

#include "Render/Models/SkinnedVertexPacking.h"

#include "Render/Models/Gltf/SkinnedModel.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>

namespace Render::Models
{
namespace
{
static_assert(MAX_BONES <= 256, "skinned_textured.vert reads joints as bytes");

constexpr int MaxWeightsPerVertex = 4;
constexpr float WeightUnits = 255.f;

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

mu::SkinnedVertex3D PackWeightedSkinnedVertex(const Gltf::SkinnedVertex& vertex, float u, float v)
{
    const std::array<int, MaxWeightsPerVertex> order = ByWeight(vertex);
    const int dominantJoint = vertex.joints[order[0]];
    mu::SkinnedVertex3D packed{vertex.bindPosition[0],
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
} // namespace Render::Models
