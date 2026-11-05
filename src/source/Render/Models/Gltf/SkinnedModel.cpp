#include "stdafx.h"

#include "Render/Models/Gltf/SkinnedModel.h"

#include "Core/Math/ZzzMathLib.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>
#include <cmath>

namespace Render::Models::Gltf
{
namespace
{
// q = a * b (x, y, z, w).
Quaternion Multiply(const Quaternion& a, const Quaternion& b)
{
    return {
        a[3] * b[0] + a[0] * b[3] + a[1] * b[2] - a[2] * b[1], a[3] * b[1] - a[0] * b[2] + a[1] * b[3] + a[2] * b[0],
        a[3] * b[2] + a[0] * b[1] - a[1] * b[0] + a[2] * b[3], a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2]};
}

Vector3 Rotate(const Quaternion& q, const Vector3& v)
{
    float m[3][4];
    QuaternionMatrix(q.data(), m);
    return {m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2], m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2],
            m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2]};
}

// The segment of `times` around `time` and the fraction in it; times are sorted.
void FindSegment(const std::vector<float>& times, float time, size_t& key0, size_t& key1, float& fraction)
{
    if (times.size() == 1 || time <= times.front())
    {
        key0 = key1 = 0;
        fraction = 0.f;
        return;
    }
    if (time >= times.back())
    {
        key0 = key1 = times.size() - 1;
        fraction = 0.f;
        return;
    }
    const auto next = std::upper_bound(times.begin(), times.end(), time);
    key1 = static_cast<size_t>(next - times.begin());
    key0 = key1 - 1;
    const float span = times[key1] - times[key0];
    fraction = span > 0.f ? (time - times[key0]) / span : 0.f;
}

void SampleTranslation(const Channel& channel, float time, Vector3& out)
{
    size_t key0 = 0;
    size_t key1 = 0;
    float fraction = 0.f;
    FindSegment(channel.times, time, key0, key1, fraction);
    const float* a = &channel.values[key0 * 3];
    const float* b = &channel.values[key1 * 3];
    for (int i = 0; i < 3; ++i)
    {
        out[i] = fraction == 0.f ? a[i] : a[i] * (1.f - fraction) + b[i] * fraction;
    }
}

void SampleRotation(const Channel& channel, float time, Quaternion& out)
{
    size_t key0 = 0;
    size_t key1 = 0;
    float fraction = 0.f;
    FindSegment(channel.times, time, key0, key1, fraction);
    const float* a = &channel.values[key0 * 4];
    const float* b = &channel.values[key1 * 4];
    const vec4_t negatedB = {-b[0], -b[1], -b[2], -b[3]};
    // Keys within QuaternionCompare's epsilon count as one, as BMD::Animation
    // counts them; the other hemisphere's copy of a key too (files store each
    // key in the hemisphere of the previous one).
    if (fraction == 0.f || key0 == key1 || QuaternionCompare(a, b) || QuaternionCompare(a, negatedB))
    {
        std::copy(a, a + 4, out.begin());
        return;
    }
    QuaternionNLERP(a, b, fraction, out.data());
}

void Transform(const float (*matrix)[4], const Vector3& v, float out[3])
{
    for (int r = 0; r < 3; ++r)
    {
        out[r] = matrix[r][0] * v[0] + matrix[r][1] * v[1] + matrix[r][2] * v[2] + matrix[r][3];
    }
}

void RotateBy(const float (*matrix)[4], const Vector3& v, float out[3])
{
    for (int r = 0; r < 3; ++r)
    {
        out[r] = matrix[r][0] * v[0] + matrix[r][1] * v[1] + matrix[r][2] * v[2];
    }
}
} // namespace

const SkinnedModel* SkinnedModelOf(const BMD& model)
{
    return static_cast<const SkinnedModel*>(model.GltfModel.get());
}

SkinnedModel::~SkinnedModel()
{
    for (const auto& lod : packedMeshes)
    {
        for (const PackedMesh& packed : lod)
        {
            if (!packed.vertices.empty())
            {
                mu::GetRenderer().ReleaseSkinnedMesh(packed.vertices.data());
            }
        }
    }
}

int SkinnedModel::ClipKeyIntervals(int numKeys, bool lockPositions, bool held)
{
    const int intervals = lockPositions || held ? numKeys - 1 : numKeys;
    return intervals < 1 ? 1 : intervals;
}

void SkinnedModel::TurnHead(Quaternion& rotation, float angleX, float angleY)
{
    const Quaternion turnZ{0.f, 0.f, std::sin(-angleY * 0.5f), std::cos(-angleY * 0.5f)};
    const Quaternion turnX{std::sin(-angleX * 0.5f), 0.f, 0.f, std::cos(-angleX * 0.5f)};
    rotation = Multiply(Multiply(turnZ, rotation), turnX);
}

float SkinnedModel::ClipTime(float keyTime, int keyIntervals, float cycle)
{
    if (keyIntervals < 1 || !(cycle > 0.f) || !(keyTime > 0.f))
    {
        return 0.f;
    }
    const float time = keyTime / static_cast<float>(keyIntervals) * cycle;
    return time < cycle ? time : cycle;
}

void SkinnedModel::SampleJoint(int joint, int action, float keyTime, int keyIntervals, JointPose& pose) const
{
    const Joint& j = joints[joint];
    pose = j.rest;
    const int clipIndex = action >= 0 && action < static_cast<int>(actionClips.size()) ? actionClips[action] : -1;
    if (clipIndex >= 0)
    {
        const Clip& clip = clips[clipIndex];
        const JointTrack& track = clip.joints[joint];
        const float time = ClipTime(keyTime, keyIntervals, clip.cycle);
        if (!track.translation.Empty())
        {
            SampleTranslation(track.translation, time, pose.position);
        }
        if (!track.rotation.Empty())
        {
            SampleRotation(track.rotation, time, pose.rotation);
        }
    }
    if (j.parent < 0)
    {
        const Vector3 rotated = Rotate(j.base.rotation, pose.position);
        pose.rotation = Multiply(j.base.rotation, pose.rotation);
        for (int i = 0; i < 3; ++i)
        {
            pose.position[i] = rotated[i] + j.base.position[i];
        }
    }
}

std::vector<Matrix34> SkinnedModel::RestPose() const
{
    std::vector<Matrix34> world(joints.size());
    for (size_t i = 0; i < joints.size(); ++i)
    {
        const Joint& j = joints[i];
        JointPose pose = j.rest;
        if (j.parent < 0)
        {
            const Vector3 rotated = Rotate(j.base.rotation, pose.position);
            pose.rotation = Multiply(j.base.rotation, pose.rotation);
            for (int k = 0; k < 3; ++k)
            {
                pose.position[k] = rotated[k] + j.base.position[k];
            }
        }
        float local[3][4];
        QuaternionMatrix(pose.rotation.data(), local);
        for (int r = 0; r < 3; ++r)
        {
            local[r][3] = pose.position[r];
        }
        float out[3][4];
        if (j.parent < 0)
        {
            std::copy(&local[0][0], &local[0][0] + 12, &out[0][0]);
        }
        else
        {
            float parent[3][4];
            std::copy(&world[j.parent][0][0], &world[j.parent][0][0] + 12, &parent[0][0]);
            R_ConcatTransforms(parent, local, out);
        }
        for (int r = 0; r < 3; ++r)
        {
            for (int c = 0; c < 4; ++c)
            {
                world[i][r][c] = out[r][c];
            }
        }
    }
    return world;
}

const LodMesh* SkinnedModel::LodMeshFor(int lod, int mesh, bool& absent) const
{
    absent = false;
    if (lod <= 0 || lod > static_cast<int>(lods.size()) || mesh < 0)
    {
        return nullptr;
    }
    const std::vector<LodMesh>& meshes = lods[static_cast<size_t>(lod - 1)];
    if (mesh >= static_cast<int>(meshes.size()))
    {
        absent = true;
        return nullptr;
    }
    return &meshes[static_cast<size_t>(mesh)];
}

NormalMapTexture SkinnedModel::NormalMapTextureFor(int lod, int mesh) const
{
    bool absent = false;
    const LodMesh* lodMesh = LodMeshFor(lod, mesh, absent);
    unsigned int texture = BITMAP_UNKNOWN;
    float scale = 0.f;
    if (lodMesh)
    {
        texture = lodNormalTextures[static_cast<size_t>(lod - 1)][static_cast<size_t>(mesh)];
        scale = lodMesh->normalMap.scale;
    }
    else if (!absent && mesh >= 0 && mesh < static_cast<int>(meshNormalTextures.size()))
    {
        texture = meshNormalTextures[static_cast<size_t>(mesh)];
        scale = meshNormalMaps[static_cast<size_t>(mesh)].scale;
    }
    if (texture == BITMAP_UNKNOWN)
    {
        return {};
    }
    return {texture, scale};
}

void SkinnedModel::ComputeRestBounds()
{
    const std::vector<Matrix34> rest = RestPose();
    std::vector<float> matrices(rest.size() * 12);
    for (size_t i = 0; i < rest.size(); ++i)
    {
        std::copy(&rest[i][0][0], &rest[i][0][0] + 12, &matrices[i * 12]);
    }
    const auto* boneMatrices = reinterpret_cast<const float (*)[3][4]>(matrices.data());
    bool any = false;
    for (size_t mesh = 0; mesh < meshVertices.size(); ++mesh)
    {
        for (size_t vertex = 0; vertex < meshVertices[mesh].size(); ++vertex)
        {
            float p[3];
            SkinPosition(static_cast<int>(mesh), static_cast<int>(vertex), boneMatrices, 1.f, 0.f, p);
            for (int r = 0; r < 3; ++r)
            {
                restBoundsMin[r] = any ? std::min(restBoundsMin[r], p[r]) : p[r];
                restBoundsMax[r] = any ? std::max(restBoundsMax[r], p[r]) : p[r];
            }
            any = true;
        }
    }
}

void SkinnedModel::SkinPosition(int mesh, int vertex, const float (*boneMatrices)[3][4], float boneScale, float scale,
                                float out[3]) const
{
    SkinPosition(meshVertices[mesh][vertex], boneMatrices, boneScale, scale, out);
}

void SkinnedModel::SkinPosition(const SkinnedVertex& v, const float (*boneMatrices)[3][4], float boneScale, float scale,
                                float out[3])
{
    out[0] = out[1] = out[2] = 0.f;
    for (int k = 0; k < v.count; ++k)
    {
        const float (*matrix)[4] = boneMatrices[v.joints[k]];
        float p[3];
        if (boneScale == 1.f)
        {
            Vector3 position = v.positions[k];
            if (scale != 0.f)
            {
                for (float& c : position)
                {
                    c *= scale;
                }
            }
            Transform(matrix, position, p);
        }
        else
        {
            RotateBy(matrix, v.positions[k], p);
            for (int r = 0; r < 3; ++r)
            {
                p[r] = p[r] * boneScale + matrix[r][3];
            }
        }
        for (int r = 0; r < 3; ++r)
        {
            out[r] += p[r] * v.weights[k];
        }
    }
}

void SkinnedModel::SkinNormal(int mesh, int vertex, const float (*boneMatrices)[3][4], float out[3]) const
{
    const SkinnedVertex& v = meshVertices[mesh][vertex];
    out[0] = out[1] = out[2] = 0.f;
    for (int k = 0; k < v.count; ++k)
    {
        float n[3];
        RotateBy(boneMatrices[v.joints[k]], v.normals[k], n);
        for (int r = 0; r < 3; ++r)
        {
            out[r] += n[r] * v.weights[k];
        }
    }
    if (v.count > 1)
    {
        const float length = std::sqrt(out[0] * out[0] + out[1] * out[1] + out[2] * out[2]);
        if (length > 1e-12f)
        {
            for (int r = 0; r < 3; ++r)
            {
                out[r] /= length;
            }
        }
    }
}
} // namespace Render::Models::Gltf
