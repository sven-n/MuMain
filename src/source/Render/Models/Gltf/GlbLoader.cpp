#include "stdafx.h"

#include "Render/Models/Gltf/GlbLoader.h"

#include "Core/Math/ZzzMathLib.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Render/Models/Gltf/SkinnedModel.h"
#include "Render/Models/ZzzBMD.h"

#include "cgltf/cgltf.h"
#include "json.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace Render::Models::Gltf
{
namespace
{
using Matrix4 = std::array<std::array<double, 4>, 4>; // row-major

// The highest triangle count of a BMD mesh (Mesh_t::NumTriangles is a short).
constexpr int MaxMeshTriangles = 32767;
// LOD 0 to 2.
constexpr int MaxLod = Data::Models::MaxLodThresholds;

// How far a matrix may be from a rotation and still count as one.
constexpr double RigidTolerance = 1e-3;

struct CgltfDeleter
{
    void operator()(cgltf_data* data) const
    {
        cgltf_free(data);
    }
};

// glTF (x, y, z) -> MU (x, -z, y).
Matrix4 GltfToMu()
{
    return {{{1, 0, 0, 0}, {0, 0, -1, 0}, {0, 1, 0, 0}, {0, 0, 0, 1}}};
}

Matrix4 Identity()
{
    return {{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
}

Matrix4 FromColumnMajor(const float* m)
{
    Matrix4 out{};
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            out[r][c] = m[c * 4 + r];
        }
    }
    return out;
}

Matrix4 Multiply(const Matrix4& a, const Matrix4& b)
{
    Matrix4 out{};
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            double sum = 0;
            for (int k = 0; k < 4; ++k)
            {
                sum += a[r][k] * b[k][c];
            }
            out[r][c] = sum;
        }
    }
    return out;
}

// The rotation and translation of a matrix that is a rotation (no scale, no
// mirror) followed by a translation; nothing otherwise.
std::optional<JointPose> RigidPose(const Matrix4& m)
{
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            double dot = 0;
            for (int k = 0; k < 3; ++k)
            {
                dot += m[k][i] * m[k][j];
            }
            if (std::fabs(dot - (i == j ? 1.0 : 0.0)) > RigidTolerance)
            {
                return std::nullopt;
            }
        }
    }
    const double determinant = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
                               m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
                               m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    if (determinant <= 0)
    {
        return std::nullopt;
    }
    // The quaternion QuaternionMatrix turns into this rotation.
    double x, y, z, w;
    const double trace = m[0][0] + m[1][1] + m[2][2];
    if (trace > 0)
    {
        const double s = std::sqrt(trace + 1.0) * 2.0;
        w = 0.25 * s;
        x = (m[2][1] - m[1][2]) / s;
        y = (m[0][2] - m[2][0]) / s;
        z = (m[1][0] - m[0][1]) / s;
    }
    else if (m[0][0] > m[1][1] && m[0][0] > m[2][2])
    {
        const double s = std::sqrt(1.0 + m[0][0] - m[1][1] - m[2][2]) * 2.0;
        w = (m[2][1] - m[1][2]) / s;
        x = 0.25 * s;
        y = (m[0][1] + m[1][0]) / s;
        z = (m[0][2] + m[2][0]) / s;
    }
    else if (m[1][1] > m[2][2])
    {
        const double s = std::sqrt(1.0 + m[1][1] - m[0][0] - m[2][2]) * 2.0;
        w = (m[0][2] - m[2][0]) / s;
        x = (m[0][1] + m[1][0]) / s;
        y = 0.25 * s;
        z = (m[1][2] + m[2][1]) / s;
    }
    else
    {
        const double s = std::sqrt(1.0 + m[2][2] - m[0][0] - m[1][1]) * 2.0;
        w = (m[1][0] - m[0][1]) / s;
        x = (m[0][2] + m[2][0]) / s;
        y = (m[1][2] + m[2][1]) / s;
        z = 0.25 * s;
    }
    const double length = std::sqrt(x * x + y * y + z * z + w * w);
    JointPose pose;
    pose.rotation = {static_cast<float>(x / length), static_cast<float>(y / length), static_cast<float>(z / length),
                     static_cast<float>(w / length)};
    pose.position = {static_cast<float>(m[0][3]), static_cast<float>(m[1][3]), static_cast<float>(m[2][3])};
    return pose;
}

// The Euler angles AngleQuaternion turns into `rotation` (x, then y, then z:
// R = Rz(angles[2]) * Ry(angles[1]) * Rx(angles[0])).
void QuaternionAngles(const Quaternion& rotation, vec3_t angles)
{
    float m[3][4];
    QuaternionMatrix(rotation.data(), m);
    const float sinY = std::clamp(-m[2][0], -1.f, 1.f);
    angles[1] = std::asin(sinY);
    if (std::fabs(sinY) < 0.9999f)
    {
        angles[0] = std::atan2(m[2][1], m[2][2]);
        angles[2] = std::atan2(m[1][0], m[0][0]);
    }
    else
    {
        // Gimbal lock: only x - z (or x + z) is defined; put it all into x.
        angles[0] = std::atan2(-m[1][2], m[1][1]);
        angles[2] = 0.f;
    }
}

std::string NodeName(const cgltf_node* node)
{
    return node && node->name ? node->name : "";
}

// The joint name of a BMD bone, as the exporter writes it.
std::string MuBoneName(const BMD& model, int index)
{
    const Bone_t& bone = model.Bones[index];
    const size_t length = strnlen(bone.Name, sizeof(bone.Name));
    if (bone.Dummy || length == 0)
    {
        return "dummy" + std::to_string(index);
    }
    return std::string(bone.Name, length);
}

std::optional<nlohmann::json> Extras(const cgltf_data* data, const cgltf_extras& extras)
{
    cgltf_size size = 0;
    if (extras.end_offset <= extras.start_offset ||
        cgltf_copy_extras_json(data, &extras, nullptr, &size) != cgltf_result_success || size == 0)
    {
        return std::nullopt;
    }
    std::string text(size, '\0');
    if (cgltf_copy_extras_json(data, &extras, text.data(), &size) != cgltf_result_success)
    {
        return std::nullopt;
    }
    text.resize(strnlen(text.c_str(), text.size()));
    nlohmann::json json = nlohmann::json::parse(text, nullptr, false);
    if (json.is_discarded() || !json.is_object())
    {
        return std::nullopt;
    }
    return json;
}

int LodOf(const cgltf_data* data, const cgltf_node* node)
{
    const auto extras = Extras(data, node->extras);
    if (!extras)
    {
        return 0;
    }
    const auto lod = extras->find("lod");
    return lod != extras->end() && lod->is_number_integer() ? lod->get<int>() : 0;
}

std::string TextureNameOf(const cgltf_data* data, const cgltf_primitive& primitive)
{
    for (const cgltf_extras* extras :
         {&primitive.extras,
          primitive.material ? &primitive.material->extras : static_cast<const cgltf_extras*>(nullptr)})
    {
        if (!extras)
        {
            continue;
        }
        if (const auto json = Extras(data, *extras))
        {
            const auto texture = json->find("texture");
            if (texture != json->end() && texture->is_string())
            {
                return texture->get<std::string>();
            }
        }
    }
    return {};
}

// The image of a primitive's base colour texture when the file carries it
// (a buffer view, not an external file), else nullptr.
const cgltf_image* EmbeddedBaseColorImageOf(const cgltf_primitive& primitive)
{
    const cgltf_material* material = primitive.material;
    if (!material || !material->has_pbr_metallic_roughness)
    {
        return nullptr;
    }
    const cgltf_texture* texture = material->pbr_metallic_roughness.base_color_texture.texture;
    if (!texture || !texture->image || !texture->image->buffer_view)
    {
        return nullptr;
    }
    return texture->image;
}

// The material's embedded normal map and its scale; image nullptr when it has none.
struct EmbeddedNormalMap
{
    const cgltf_image* image = nullptr;
    float scale = 1.f;
};

EmbeddedNormalMap EmbeddedNormalMapOf(const cgltf_primitive& primitive)
{
    const cgltf_material* material = primitive.material;
    if (!material)
    {
        return {};
    }
    const cgltf_texture* texture = material->normal_texture.texture;
    if (!texture || !texture->image || !texture->image->buffer_view)
    {
        return {};
    }
    return {texture->image, material->normal_texture.scale};
}

// Every component of an accessor as floats (sparse data applied, normalised
// integers converted).
std::vector<float> ReadFloats(const cgltf_accessor* accessor)
{
    const cgltf_size count = cgltf_accessor_unpack_floats(accessor, nullptr, 0);
    std::vector<float> values(count);
    if (count > 0)
    {
        cgltf_accessor_unpack_floats(accessor, values.data(), count);
    }
    return values;
}

bool AllFinite(const std::vector<float>& values)
{
    return std::all_of(values.begin(), values.end(), [](float v) { return std::isfinite(v); });
}

const cgltf_accessor* FindAttribute(const cgltf_primitive& primitive, cgltf_attribute_type type, int index = 0)
{
    for (cgltf_size i = 0; i < primitive.attributes_count; ++i)
    {
        if (primitive.attributes[i].type == type && primitive.attributes[i].index == index)
        {
            return primitive.attributes[i].data;
        }
    }
    return nullptr;
}

// A BMD mesh to build: the vertices of one primitive it uses, and its triangles.
struct MeshPart
{
    const cgltf_primitive* primitive = nullptr;
    size_t primitiveIndex = 0;
    std::string texture;
    std::vector<int> vertices;                 // primitive vertex of each mesh vertex
    std::vector<std::array<int, 3>> triangles; // mesh vertices
};

// A primitive's vertex data, read once.
struct PrimitiveData
{
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> texCoords;
    std::vector<float> joints;
    std::vector<float> weights;
    // Only for a material with a normal map: 4 per vertex, w the handedness.
    std::vector<float> tangents;
    std::vector<float> indices;
};

class Loader
{
public:
    Loader(BMD& model, const std::filesystem::path& file, const std::map<int, Data::Models::ActionClip>& actions,
           const Data::Models::LodSettings& lod)
        : model_(model), file_(file), actions_(actions)
    {
        skinned_->lodSettings = lod;
    }

    bool Load(std::string& reason)
    {
        if (!Parse(reason) || !ReadSkeleton(reason) || !ReadClips(reason) || !ReadMeshes(reason) ||
            !ReadCoarserLods(reason))
        {
            return false;
        }
        Apply();
        return true;
    }

private:
    BMD& model_;
    const std::filesystem::path& file_;
    const std::map<int, Data::Models::ActionClip>& actions_;
    std::unique_ptr<cgltf_data, CgltfDeleter> data_;
    const cgltf_skin* skin_ = nullptr;
    std::shared_ptr<SkinnedModel> skinned_ = std::make_shared<SkinnedModel>();
    // Joint index (BMD bone order) of each node that is a joint of the skin.
    std::unordered_map<const cgltf_node*, int> jointOfNode_;
    // Joint index of each entry of the skin's joint list.
    std::vector<int> jointOfSkinJoint_;
    std::vector<MeshPart> parts_;
    // The meshes of LOD 1 and 2, while there are any.
    std::vector<std::vector<MeshPart>> lodParts_;
    std::vector<PrimitiveData> primitives_;

    bool Parse(std::string& reason)
    {
        const std::string path = file_.string();
        cgltf_options options{};
        cgltf_data* data = nullptr;
        if (cgltf_parse_file(&options, path.c_str(), &data) != cgltf_result_success)
        {
            reason = "it is no glTF file that can be read";
            return false;
        }
        data_.reset(data);
        // The file is self-contained: no buffer is read from another file or a data URI.
        for (cgltf_size i = 0; i < data->buffers_count; ++i)
        {
            if (data->buffers[i].uri)
            {
                reason = "buffer " + std::to_string(i) + " names a URI; the conventions want a self-contained file";
                return false;
            }
        }
        if (cgltf_load_buffers(&options, data, path.c_str()) != cgltf_result_success)
        {
            reason = "its buffers cannot be read";
            return false;
        }
        if (cgltf_validate(data) != cgltf_result_success)
        {
            reason = "it is no valid glTF file";
            return false;
        }
        if (data->skins_count != 1)
        {
            reason = "it has " + std::to_string(data->skins_count) + " skins, the conventions want one";
            return false;
        }
        skin_ = &data->skins[0];
        return true;
    }

    // The transform of the nodes above a root joint, glTF -> MU included.
    bool RootBase(const cgltf_node* joint, JointPose& base, std::string& reason) const
    {
        Matrix4 above = Identity();
        if (joint->parent)
        {
            float world[16];
            cgltf_node_transform_world(joint->parent, world);
            above = FromColumnMajor(world);
        }
        const std::optional<JointPose> pose = RigidPose(Multiply(GltfToMu(), above));
        if (!pose)
        {
            reason = "the nodes above joint \"" + NodeName(joint) + "\" scale or mirror the skeleton";
            return false;
        }
        base = *pose;
        return true;
    }

    // Whether a node or one of the nodes above it is a joint.
    bool HasJointAbove(const cgltf_node* node, const std::unordered_map<const cgltf_node*, size_t>& skinJoints) const
    {
        for (const cgltf_node* n = node; n; n = n->parent)
        {
            if (skinJoints.contains(n))
            {
                return true;
            }
        }
        return false;
    }

    bool ReadSkeleton(std::string& reason)
    {
        const size_t jointCount = skin_->joints_count;
        if (jointCount > MAX_BONES)
        {
            reason = "it has " + std::to_string(jointCount) + " joints, at most " + std::to_string(MAX_BONES) +
                     " are allowed";
            return false;
        }
        if (!skin_->inverse_bind_matrices || skin_->inverse_bind_matrices->count != jointCount)
        {
            reason = "its skin has no inverse bind matrix per joint";
            return false;
        }
        if (skin_->inverse_bind_matrices->type != cgltf_type_mat4 ||
            skin_->inverse_bind_matrices->component_type != cgltf_component_type_r_32f)
        {
            reason = "its inverse bind matrices are no FLOAT MAT4";
            return false;
        }
        const std::vector<float> inverseBinds = ReadFloats(skin_->inverse_bind_matrices);
        if (inverseBinds.size() != jointCount * 16)
        {
            reason = "its inverse bind matrices do not hold 16 values per joint";
            return false;
        }
        if (!AllFinite(inverseBinds))
        {
            reason = "its inverse bind matrices hold a value that is not finite";
            return false;
        }

        std::unordered_map<const cgltf_node*, size_t> skinJoints;
        std::unordered_map<std::string, size_t> skinJointOfName;
        for (size_t i = 0; i < jointCount; ++i)
        {
            const cgltf_node* node = skin_->joints[i];
            if (!skinJoints.emplace(node, i).second || !skinJointOfName.emplace(NodeName(node), i).second)
            {
                reason = "joint name \"" + NodeName(node) + "\" occurs more than once";
                return false;
            }
        }

        // The BMD's bones first, by name.
        const int muBones = model_.NumBones;
        std::vector<size_t> skinJointOfJoint;
        std::vector<bool> used(jointCount, false);
        for (int bone = 0; bone < muBones; ++bone)
        {
            const std::string name = MuBoneName(model_, bone);
            const auto found = skinJointOfName.find(name);
            if (found == skinJointOfName.end())
            {
                reason = "it lacks the bone \"" + name + "\" (" + std::to_string(bone) + ") of the BMD";
                return false;
            }
            if (used[found->second])
            {
                reason = "the BMD has the bone name \"" + name + "\" more than once";
                return false;
            }
            used[found->second] = true;
            skinJointOfJoint.push_back(found->second);
        }
        // Then the extra joints, each after its parent.
        std::vector<size_t> extras;
        for (size_t i = 0; i < jointCount; ++i)
        {
            if (!used[i])
            {
                extras.push_back(i);
            }
        }
        std::vector<bool> placed(jointCount, false);
        for (size_t i : skinJointOfJoint)
        {
            placed[i] = true;
        }
        while (!extras.empty())
        {
            const auto ready = std::find_if(extras.begin(), extras.end(),
                                            [&](size_t i)
                                            {
                                                const cgltf_node* parent = skin_->joints[i]->parent;
                                                const auto parentJoint =
                                                    parent ? skinJoints.find(parent) : skinJoints.end();
                                                return parentJoint == skinJoints.end() || placed[parentJoint->second];
                                            });
            if (ready == extras.end())
            {
                reason =
                    "joint \"" + NodeName(skin_->joints[extras.front()]) + "\" hangs below a node that is no joint";
                return false;
            }
            placed[*ready] = true;
            skinJointOfJoint.push_back(*ready);
            extras.erase(ready);
        }

        jointOfSkinJoint_.assign(jointCount, -1);
        for (size_t joint = 0; joint < skinJointOfJoint.size(); ++joint)
        {
            jointOfSkinJoint_[skinJointOfJoint[joint]] = static_cast<int>(joint);
            jointOfNode_[skin_->joints[skinJointOfJoint[joint]]] = static_cast<int>(joint);
        }

        skinned_->muBoneCount = muBones;
        skinned_->joints.resize(skinJointOfJoint.size());
        for (size_t joint = 0; joint < skinJointOfJoint.size(); ++joint)
        {
            const size_t skinJoint = skinJointOfJoint[joint];
            const cgltf_node* node = skin_->joints[skinJoint];
            Joint& out = skinned_->joints[joint];
            out.name = NodeName(node);

            const cgltf_node* parent = node->parent;
            // No joint is nullptr: a root finds nothing.
            const auto parentJoint = jointOfNode_.find(parent);
            if (parentJoint != jointOfNode_.end())
            {
                out.parent = parentJoint->second;
                if (static_cast<int>(joint) < muBones && out.parent >= muBones)
                {
                    reason =
                        "the BMD's bone \"" + out.name + "\" hangs below the extra joint \"" + NodeName(parent) + "\"";
                    return false;
                }
            }
            else
            {
                if (parent && HasJointAbove(parent, skinJoints))
                {
                    reason = "joint \"" + out.name + "\" hangs below a node that is no joint";
                    return false;
                }
                out.parent = -1;
                if (!RootBase(node, out.base, reason))
                {
                    return false;
                }
            }
            // A bone of the BMD keeps its BMD parent: BMD::Animation
            // computes the bones in index order, each after its parent.
            if (static_cast<int>(joint) < muBones && !model_.Bones[joint].Dummy &&
                out.parent != model_.Bones[joint].Parent)
            {
                const short bmdParent = model_.Bones[joint].Parent;
                reason = "the BMD's bone \"" + out.name + "\" hangs below " +
                         (out.parent >= 0 ? "\"" + NodeName(parent) + "\"" : std::string("no joint")) +
                         ", in the BMD below " +
                         (bmdParent >= 0 ? "\"" + MuBoneName(model_, bmdParent) + "\"" : std::string("no bone"));
                return false;
            }

            float local[16];
            cgltf_node_transform_local(node, local);
            const std::optional<JointPose> rest = RigidPose(FromColumnMajor(local));
            if (!rest)
            {
                reason = "joint \"" + out.name + "\" is scaled or mirrored; joints may only rotate and move";
                return false;
            }
            out.rest = *rest;

            const float* inverse = &inverseBinds[skinJoint * 16];
            for (int r = 0; r < 3; ++r)
            {
                for (int c = 0; c < 4; ++c)
                {
                    out.inverseBind[r][c] = inverse[c * 4 + r];
                }
            }
        }
        return true;
    }

    bool ReadChannel(const cgltf_animation_channel& channel, const std::string& clipName, Clip& clip,
                     std::string& reason) const
    {
        const auto joint = jointOfNode_.find(channel.target_node);
        if (joint == jointOfNode_.end() || !channel.sampler)
        {
            // Animation of nodes that are no joints does not move the model.
            return true;
        }
        const cgltf_animation_sampler& sampler = *channel.sampler;
        const std::string where = "clip \"" + clipName + "\", joint \"" + NodeName(channel.target_node) + "\"";
        if (sampler.interpolation != cgltf_interpolation_type_linear)
        {
            reason = where + ": only LINEAR interpolation is supported";
            return false;
        }
        std::vector<float> times = ReadFloats(sampler.input);
        std::vector<float> values = ReadFloats(sampler.output);
        if (times.empty() || !AllFinite(times) || !std::is_sorted(times.begin(), times.end()))
        {
            reason = where + ": key times are missing or not in order";
            return false;
        }
        if (!AllFinite(values))
        {
            reason = where + ": a key value is not finite";
            return false;
        }
        switch (channel.target_path)
        {
        case cgltf_animation_path_type_translation:
        case cgltf_animation_path_type_rotation:
        {
            const size_t components = channel.target_path == cgltf_animation_path_type_rotation ? 4 : 3;
            if (values.size() != times.size() * components)
            {
                reason = where + ": the key values do not match the key times";
                return false;
            }
            Channel& target = channel.target_path == cgltf_animation_path_type_rotation
                                  ? clip.joints[joint->second].rotation
                                  : clip.joints[joint->second].translation;
            target.times = std::move(times);
            target.values = std::move(values);
            clip.cycle = std::max(clip.cycle, target.times.back());
            return true;
        }
        case cgltf_animation_path_type_scale:
            if (std::any_of(values.begin(), values.end(), [](float v) { return std::fabs(v - 1.f) > RigidTolerance; }))
            {
                reason = where + ": joints may not be scaled";
                return false;
            }
            return true;
        default:
            return true;
        }
    }

    bool ReadClips(std::string& reason)
    {
        const size_t jointCount = skinned_->joints.size();
        for (cgltf_size a = 0; a < data_->animations_count; ++a)
        {
            const cgltf_animation& animation = data_->animations[a];
            Clip clip;
            clip.name = animation.name ? animation.name : "";
            clip.joints.resize(jointCount);
            for (cgltf_size c = 0; c < animation.channels_count; ++c)
            {
                if (!ReadChannel(animation.channels[c], clip.name, clip, reason))
                {
                    return false;
                }
            }
            skinned_->clips.push_back(std::move(clip));
        }

        const auto findClip = [this](const std::string& name)
        {
            const auto& clips = skinned_->clips;
            const auto found =
                std::find_if(clips.begin(), clips.end(), [&](const Clip& clip) { return clip.name == name; });
            return found != clips.end() ? static_cast<int>(found - clips.begin()) : -1;
        };
        skinned_->actionClips.assign(std::max<short>(model_.NumActions, 0), -1);
        skinned_->actionCrossfades.assign(std::max<short>(model_.NumActions, 0), std::nullopt);
        for (int action = 0; action < model_.NumActions; ++action)
        {
            const auto entry = actions_.find(action);
            if (entry != actions_.end())
            {
                skinned_->actionCrossfades[action] = entry->second.crossfade;
            }
            if (entry != actions_.end() && !entry->second.clip.empty())
            {
                const int clip = findClip(entry->second.clip);
                if (clip < 0)
                {
                    reason = "it has no clip \"" + entry->second.clip + "\" for action " + std::to_string(action);
                    return false;
                }
                skinned_->actionClips[action] = clip;
                continue;
            }
            char name[32];
            std::snprintf(name, sizeof(name), "action%02d", action);
            skinned_->actionClips[action] = findClip(name);
        }
        return true;
    }

    bool ReadPrimitive(const cgltf_primitive& primitive, PrimitiveData& out, std::string& reason) const
    {
        if (primitive.type != cgltf_primitive_type_triangles || !primitive.indices)
        {
            reason = "a mesh is not made of indexed triangles";
            return false;
        }
        const struct
        {
            cgltf_attribute_type type;
            const char* name;
            std::vector<float>* values;
            size_t components;
        } attributes[] = {{cgltf_attribute_type_position, "POSITION", &out.positions, 3},
                          {cgltf_attribute_type_normal, "NORMAL", &out.normals, 3},
                          {cgltf_attribute_type_texcoord, "TEXCOORD_0", &out.texCoords, 2},
                          {cgltf_attribute_type_joints, "JOINTS_0", &out.joints, 4},
                          {cgltf_attribute_type_weights, "WEIGHTS_0", &out.weights, 4}};
        size_t vertexCount = 0;
        for (const auto& attribute : attributes)
        {
            const cgltf_accessor* accessor = FindAttribute(primitive, attribute.type);
            if (!accessor || cgltf_num_components(accessor->type) != attribute.components)
            {
                reason = std::string("a mesh lacks ") + attribute.name;
                return false;
            }
            if (attribute.type == cgltf_attribute_type_position)
            {
                vertexCount = accessor->count;
            }
            else if (accessor->count != vertexCount)
            {
                reason = std::string("a mesh has another count of ") + attribute.name + " than of POSITION";
                return false;
            }
            // Joint indices are integers, never converted from floats or normalised.
            if (attribute.type == cgltf_attribute_type_joints &&
                (accessor->normalized || (accessor->component_type != cgltf_component_type_r_8u &&
                                          accessor->component_type != cgltf_component_type_r_16u)))
            {
                reason = "a mesh's JOINTS_0 is no UNSIGNED_BYTE or UNSIGNED_SHORT";
                return false;
            }
            *attribute.values = ReadFloats(accessor);
            if (!AllFinite(*attribute.values))
            {
                reason = std::string("a mesh's ") + attribute.name + " holds a value that is not finite";
                return false;
            }
        }
        if (std::any_of(out.weights.begin(), out.weights.end(), [](float w) { return w < 0.f; }))
        {
            reason = "a mesh has a negative weight";
            return false;
        }
        if (EmbeddedNormalMapOf(primitive).image && !ReadTangents(primitive, vertexCount, out, reason))
        {
            return false;
        }
        // Indices are exact as floats below 2^24, far beyond any model.
        out.indices = ReadFloats(primitive.indices);
        for (const float index : out.indices)
        {
            if (index < 0.f || index >= static_cast<float>(vertexCount))
            {
                reason = "a mesh has an index beyond its vertices";
                return false;
            }
        }
        for (const float joint : out.joints)
        {
            if (joint < 0.f || joint >= static_cast<float>(jointOfSkinJoint_.size()))
            {
                reason = "a vertex names a joint the skin does not have";
                return false;
            }
        }
        return true;
    }

    // The TANGENT of a primitive whose material has a normal map: required, used as baked.
    static bool ReadTangents(const cgltf_primitive& primitive, size_t vertexCount, PrimitiveData& out,
                             std::string& reason)
    {
        const cgltf_accessor* accessor = FindAttribute(primitive, cgltf_attribute_type_tangent);
        if (!accessor || cgltf_num_components(accessor->type) != 4 || accessor->count != vertexCount)
        {
            reason = "a mesh with a normal map lacks TANGENT (4 components per vertex)";
            return false;
        }
        out.tangents = ReadFloats(accessor);
        if (!AllFinite(out.tangents))
        {
            reason = "a mesh's TANGENT holds a value that is not finite";
            return false;
        }
        return true;
    }

    // Splits a primitive into BMD meshes of at most MAX_VERTICES vertices and
    // MaxMeshTriangles triangles.
    void SplitPrimitive(const cgltf_primitive& primitive, size_t primitiveIndex, const PrimitiveData& data,
                        std::vector<MeshPart>& parts) const
    {
        const std::string texture = TextureNameOf(data_.get(), primitive);
        std::vector<int> local(data.positions.size() / 3, -1);
        MeshPart part;
        const auto startPart = [&]
        {
            part = MeshPart{};
            part.primitive = &primitive;
            part.primitiveIndex = primitiveIndex;
            part.texture = texture;
        };
        const auto finishPart = [&]
        {
            if (part.triangles.empty())
            {
                return;
            }
            for (int vertex : part.vertices)
            {
                local[vertex] = -1;
            }
            parts.push_back(std::move(part));
        };
        startPart();
        for (size_t t = 0; t + 2 < data.indices.size(); t += 3)
        {
            const int corners[3] = {static_cast<int>(data.indices[t]), static_cast<int>(data.indices[t + 1]),
                                    static_cast<int>(data.indices[t + 2])};
            int added = 0;
            for (int i = 0; i < 3; ++i)
            {
                // A corner repeated in the triangle is counted twice: harmless, the bound only gets stricter.
                added += local[corners[i]] < 0 ? 1 : 0;
            }
            if (static_cast<int>(part.vertices.size()) + added > MAX_VERTICES ||
                static_cast<int>(part.triangles.size()) + 1 > MaxMeshTriangles)
            {
                finishPart();
                startPart();
            }
            std::array<int, 3> triangle{};
            for (int i = 0; i < 3; ++i)
            {
                int& index = local[corners[i]];
                if (index < 0)
                {
                    index = static_cast<int>(part.vertices.size());
                    part.vertices.push_back(corners[i]);
                }
                triangle[i] = index;
            }
            part.triangles.push_back(triangle);
        }
        finishPart();
    }

    // The primitives of the mesh nodes of LOD `lod`; false when a node breaks the conventions.
    bool LodPrimitives(int lod, std::vector<const cgltf_primitive*>& primitives, std::string& reason) const
    {
        for (cgltf_size n = 0; n < data_->nodes_count; ++n)
        {
            const cgltf_node& node = data_->nodes[n];
            if (!node.mesh)
            {
                continue;
            }
            const int nodeLod = LodOf(data_.get(), &node);
            if (nodeLod < 0 || nodeLod > MaxLod)
            {
                reason = "mesh node \"" + NodeName(&node) + "\" has LOD " + std::to_string(nodeLod) +
                         ", the conventions allow 0 to " + std::to_string(MaxLod);
                return false;
            }
            if (nodeLod != lod)
            {
                continue;
            }
            if (node.skin != skin_)
            {
                reason = "mesh node \"" + NodeName(&node) + "\" is not on the skin";
                return false;
            }
            for (cgltf_size p = 0; p < node.mesh->primitives_count; ++p)
            {
                primitives.push_back(&node.mesh->primitives[p]);
            }
        }
        return true;
    }

    // Reads and splits `primitives` into `parts`.
    bool ReadLodPrimitives(const std::vector<const cgltf_primitive*>& primitives, std::vector<MeshPart>& parts,
                           std::string& reason)
    {
        for (const cgltf_primitive* primitive : primitives)
        {
            const size_t index = primitives_.size();
            primitives_.emplace_back();
            if (!ReadPrimitive(*primitive, primitives_[index], reason))
            {
                return false;
            }
            SplitPrimitive(*primitive, index, primitives_[index], parts);
        }
        return true;
    }

    // LOD 1 and 2, when the file has them: each at most as many BMD meshes as LOD 0, LOD 2 only with LOD 1.
    bool ReadCoarserLods(std::string& reason)
    {
        for (int lod = 1; lod <= MaxLod; ++lod)
        {
            std::vector<const cgltf_primitive*> primitives;
            if (!LodPrimitives(lod, primitives, reason))
            {
                return false;
            }
            if (primitives.empty())
            {
                continue;
            }
            if (static_cast<int>(lodParts_.size()) != lod - 1)
            {
                reason = "it has a LOD " + std::to_string(lod) + " but no LOD " + std::to_string(lod - 1);
                return false;
            }
            std::vector<MeshPart> parts;
            if (!ReadLodPrimitives(primitives, parts, reason))
            {
                return false;
            }
            if (parts.empty() || parts.size() > parts_.size())
            {
                reason = "its LOD " + std::to_string(lod) + " needs " + std::to_string(parts.size()) +
                         " BMD meshes, from 1 to LOD 0's " + std::to_string(parts_.size()) + " are allowed";
                return false;
            }
            lodParts_.push_back(std::move(parts));
        }
        return true;
    }

    bool ReadMeshes(std::string& reason)
    {
        std::vector<const cgltf_primitive*> primitives;
        if (!LodPrimitives(0, primitives, reason))
        {
            return false;
        }
        if (primitives.empty())
        {
            reason = "it has no LOD 0 mesh";
            return false;
        }
        if (!ReadLodPrimitives(primitives, parts_, reason))
        {
            return false;
        }
        if (parts_.size() > MAX_MESH)
        {
            reason = "its LOD 0 needs " + std::to_string(parts_.size()) + " BMD meshes, at most " +
                     std::to_string(MAX_MESH) + " are allowed";
            return false;
        }
        if (parts_.empty())
        {
            reason = "its LOD 0 has no triangle";
            return false;
        }
        return true;
    }

    // The vertex of a primitive with its joints in their own spaces.
    SkinnedVertex SkinVertexOf(const PrimitiveData& data, size_t vertex) const
    {
        SkinnedVertex out;
        float sum = 0.f;
        for (int k = 0; k < 4; ++k)
        {
            const float weight = data.weights[vertex * 4 + k];
            if (weight <= 0.f)
            {
                continue;
            }
            const int joint = jointOfSkinJoint_[static_cast<size_t>(data.joints[vertex * 4 + k])];
            out.joints[out.count] = static_cast<short>(joint);
            out.weights[out.count] = weight;
            sum += weight;
            ++out.count;
        }
        if (out.count == 0)
        {
            // No weight at all: the vertex follows the first joint it names.
            out.count = 1;
            out.joints[0] = static_cast<short>(jointOfSkinJoint_[static_cast<size_t>(data.joints[vertex * 4])]);
            out.weights[0] = 1.f;
            sum = 1.f;
        }
        const float* p = &data.positions[vertex * 3];
        const float* n = &data.normals[vertex * 3];
        out.bindPosition = {p[0], p[1], p[2]};
        out.bindNormal = {n[0], n[1], n[2]};
        if (!data.tangents.empty())
        {
            const float* t = &data.tangents[vertex * 4];
            out.bindTangent = {t[0], t[1], t[2], t[3] < 0.f ? -1.f : 1.f};
        }
        for (int k = 0; k < out.count; ++k)
        {
            out.weights[k] /= sum;
            const Matrix34& inverse = skinned_->joints[out.joints[k]].inverseBind;
            float length = 0.f;
            for (int r = 0; r < 3; ++r)
            {
                out.positions[k][r] =
                    inverse[r][0] * p[0] + inverse[r][1] * p[1] + inverse[r][2] * p[2] + inverse[r][3];
                out.normals[k][r] = inverse[r][0] * n[0] + inverse[r][1] * n[1] + inverse[r][2] * n[2];
                length += out.normals[k][r] * out.normals[k][r];
            }
            length = std::sqrt(length);
            if (length > 1e-12f)
            {
                for (float& c : out.normals[k])
                {
                    c /= length;
                }
            }
        }
        return out;
    }

    // The index in SkinnedModel::images of an embedded image, copied there on first use.
    int EmbeddedImageIndex(const cgltf_image* image, std::unordered_map<const cgltf_image*, int>& indices) const
    {
        if (!image)
        {
            return -1;
        }
        const auto found = indices.find(image);
        if (found != indices.end())
        {
            return found->second;
        }
        const cgltf_buffer_view* view = image->buffer_view;
        const auto* bytes = view->data           ? static_cast<const std::uint8_t*>(view->data)
                            : view->buffer->data ? static_cast<const std::uint8_t*>(view->buffer->data) + view->offset
                                                 : nullptr;
        EmbeddedImage embedded;
        embedded.name = image->name ? image->name : "image" + std::to_string(image - data_->images);
        embedded.mimeType = image->mime_type ? image->mime_type : "";
        if (bytes)
        {
            embedded.bytes.assign(bytes, bytes + view->size);
        }
        const int index = static_cast<int>(skinned_->images.size());
        skinned_->images.push_back(std::move(embedded));
        indices.emplace(image, index);
        return index;
    }

    // A primitive's normal map, its image copied into SkinnedModel::images on first use.
    NormalMap NormalMapOf(const cgltf_primitive& primitive, std::unordered_map<const cgltf_image*, int>& indices) const
    {
        const EmbeddedNormalMap map = EmbeddedNormalMapOf(primitive);
        return {EmbeddedImageIndex(map.image, indices), map.scale};
    }

    // The coarser LODs' meshes into SkinnedModel::lods.
    void ApplyCoarserLods(std::unordered_map<const cgltf_image*, int>& imageIndices)
    {
        skinned_->lods.resize(lodParts_.size());
        skinned_->lodTextures.resize(lodParts_.size());
        skinned_->lodNormalTextures.resize(lodParts_.size());
        for (size_t lod = 0; lod < lodParts_.size(); ++lod)
        {
            std::vector<LodMesh>& meshes = skinned_->lods[lod];
            meshes.resize(lodParts_[lod].size());
            skinned_->lodTextures[lod].assign(meshes.size(), BITMAP_UNKNOWN);
            skinned_->lodNormalTextures[lod].assign(meshes.size(), BITMAP_UNKNOWN);
            for (size_t i = 0; i < meshes.size(); ++i)
            {
                const MeshPart& part = lodParts_[lod][i];
                const PrimitiveData& data = primitives_[part.primitiveIndex];
                LodMesh& mesh = meshes[i];
                mesh.vertices.reserve(part.vertices.size());
                mesh.texCoords.reserve(part.vertices.size());
                for (const int vertex : part.vertices)
                {
                    const auto source = static_cast<size_t>(vertex);
                    mesh.vertices.push_back(SkinVertexOf(data, source));
                    mesh.texCoords.push_back({data.texCoords[source * 2], data.texCoords[source * 2 + 1]});
                }
                mesh.triangles = part.triangles;
                mesh.texture = part.texture;
                mesh.image = EmbeddedImageIndex(EmbeddedBaseColorImageOf(*part.primitive), imageIndices);
                mesh.normalMap = NormalMapOf(*part.primitive, imageIndices);
            }
        }
    }

    // Replaces the BMD's bones and meshes; nothing can fail any more.
    void Apply()
    {
        const int jointCount = static_cast<int>(skinned_->joints.size());
        const int muBones = skinned_->muBoneCount;

        auto* bones = new Bone_t[jointCount]();
        for (int joint = 0; joint < jointCount; ++joint)
        {
            Bone_t& bone = bones[joint];
            const Joint& source = skinned_->joints[joint];
            if (joint < muBones)
            {
                // The BMD's bone keeps its name, dummy flag and keys (the fallback).
                bone = model_.Bones[joint];
                if (!bone.Dummy)
                {
                    bone.Parent = static_cast<short>(source.parent);
                }
                continue;
            }
            std::snprintf(bone.Name, sizeof(bone.Name), "%s", source.name.c_str());
            bone.Parent = static_cast<short>(source.parent);
            bone.Dummy = 0;
            // Keys like a BMD bone's, all the rest pose, for code that reads a
            // bone's keys (the pose itself comes from the clips).
            vec3_t restAngles;
            QuaternionAngles(source.rest.rotation, restAngles);
            bone.BoneMatrixes = new BoneMatrix_t[std::max<short>(model_.NumActions, 1)]();
            for (int action = 0; action < model_.NumActions; ++action)
            {
                const int keys = model_.Actions[action].NumAnimationKeys;
                if (keys <= 0)
                {
                    continue;
                }
                BoneMatrix_t& matrices = bone.BoneMatrixes[action];
                matrices.Position = new vec3_t[keys];
                matrices.Rotation = new vec3_t[keys];
                matrices.Quaternion = new vec4_t[keys];
                for (int key = 0; key < keys; ++key)
                {
                    std::copy(source.rest.position.begin(), source.rest.position.end(), matrices.Position[key]);
                    VectorCopy(restAngles, matrices.Rotation[key]);
                    std::copy(source.rest.rotation.begin(), source.rest.rotation.end(), matrices.Quaternion[key]);
                }
            }
        }

        const int meshCount = static_cast<int>(parts_.size());
        auto* meshes = new Mesh_t[meshCount]();
        auto* textures = new Texture_t[meshCount]();
        auto* textureIndices = new GLuint[meshCount]();
        skinned_->meshVertices.resize(meshCount);
        skinned_->meshImages.assign(meshCount, -1);
        skinned_->meshNormalMaps.assign(meshCount, NormalMap{});
        skinned_->meshNormalTextures.assign(meshCount, BITMAP_UNKNOWN);
        std::unordered_map<const cgltf_image*, int> imageIndices;
        for (int i = 0; i < meshCount; ++i)
        {
            const MeshPart& part = parts_[i];
            const PrimitiveData& data = primitives_[part.primitiveIndex];
            Mesh_t& mesh = meshes[i];
            const auto vertexCount = static_cast<short>(part.vertices.size());
            mesh.NoneBlendMesh = false;
            mesh.Texture = static_cast<short>(i);
            mesh.NumVertices = mesh.NumNormals = mesh.NumTexCoords = vertexCount;
            mesh.NumVertexColors = 0;
            mesh.NumTriangles = static_cast<short>(part.triangles.size());
            mesh.NumCommandBytes = 0;
            mesh.Vertices = new Vertex_t[vertexCount];
            mesh.Normals = new Normal_t[vertexCount];
            mesh.TexCoords = new TexCoord_t[vertexCount];
            mesh.VertexColors = nullptr;
            mesh.Triangles = new Triangle_t[mesh.NumTriangles]();
            mesh.Commands = nullptr;

            std::vector<SkinnedVertex>& skinnedVertices = skinned_->meshVertices[i];
            skinnedVertices.reserve(part.vertices.size());
            for (int v = 0; v < vertexCount; ++v)
            {
                const auto source = static_cast<size_t>(part.vertices[v]);
                const SkinnedVertex skinned = SkinVertexOf(data, source);
                const int dominant = static_cast<int>(
                    std::max_element(skinned.weights.begin(), skinned.weights.begin() + skinned.count) -
                    skinned.weights.begin());
                mesh.Vertices[v].Node = skinned.joints[dominant];
                std::copy(skinned.positions[dominant].begin(), skinned.positions[dominant].end(),
                          mesh.Vertices[v].Position);
                mesh.Normals[v].Node = skinned.joints[dominant];
                std::copy(skinned.normals[dominant].begin(), skinned.normals[dominant].end(), mesh.Normals[v].Normal);
                mesh.Normals[v].BindVertex = static_cast<short>(v);
                mesh.TexCoords[v].TexCoordU = data.texCoords[source * 2];
                mesh.TexCoords[v].TexCoordV = data.texCoords[source * 2 + 1];
                skinnedVertices.push_back(skinned);
            }
            for (int t = 0; t < mesh.NumTriangles; ++t)
            {
                Triangle_t& triangle = mesh.Triangles[t];
                triangle.Polygon = 3;
                for (int c = 0; c < 4; ++c)
                {
                    const short index = c < 3 ? static_cast<short>(part.triangles[t][c]) : 0;
                    triangle.VertexIndex[c] = triangle.NormalIndex[c] = triangle.TexCoordIndex[c] = index;
                    triangle.EdgeTriangleIndex[c] = -1;
                }
                triangle.Front = false;
            }

            std::snprintf(textures[i].FileName, sizeof(textures[i].FileName), "%s", part.texture.c_str());
            skinned_->meshImages[i] = EmbeddedImageIndex(EmbeddedBaseColorImageOf(*part.primitive), imageIndices);
            skinned_->meshNormalMaps[i] = NormalMapOf(*part.primitive, imageIndices);
            TextureScriptParsing script;
            mesh.m_csTScript = nullptr;
            if (script.parsingTScriptA(textures[i].FileName))
            {
                mesh.m_csTScript = new TextureScript;
                mesh.m_csTScript->setScript(script);
            }
        }

        // The BMD's own meshes go; its bones' arrays moved into `bones`.
        for (int i = 0; i < model_.NumMeshs; ++i)
        {
            Mesh_t& mesh = model_.Meshs[i];
            delete[] mesh.Vertices;
            delete[] mesh.Normals;
            delete[] mesh.TexCoords;
            delete[] mesh.Triangles;
            delete mesh.m_csTScript;
        }
        delete[] model_.Meshs;
        delete[] model_.Textures;
        delete[] model_.IndexTexture;
        delete[] model_.Bones;

        model_.Bones = bones;
        model_.NumBones = static_cast<short>(jointCount);
        model_.Meshs = meshes;
        model_.Textures = textures;
        model_.IndexTexture = textureIndices;
        model_.NumMeshs = static_cast<short>(meshCount);
        model_.CreateBoundingBox();
        ApplyCoarserLods(imageIndices);
        skinned_->ComputeRestBounds();
        model_.GltfModel = std::move(skinned_);
    }
};
} // namespace

bool LoadGlbIntoBmd(BMD& model, const std::filesystem::path& file,
                    const std::map<int, Data::Models::ActionClip>& actions, std::string& reason,
                    const Data::Models::LodSettings& lod)
{
    reason.clear();
    if (!model.m_bCompletedAlloc || !model.Bones || !model.Actions || model.SharesData() ||
        model.GetDataUserCount() > 1 || model.GltfModel)
    {
        reason = "the model is not a BMD of its own, opened and not yet replaced";
    }
    else
    {
        Loader loader(model, file, actions, lod);
        if (loader.Load(reason))
        {
            return true;
        }
    }
    MU_LOG_WARN(mu::log::Get("render"), "Model file {} is not used, {}; the BMD stays.", file.generic_string(), reason);
    return false;
}
} // namespace Render::Models::Gltf
