#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

class BMD;

// A monster model read from a glTF binary: what a BMD cannot hold. The BMD it
// is attached to (BMD::GltfModel) keeps its actions and gets the file's bones
// and meshes, so code that reads bones and vertices keeps working:
//  - joints: the BMD's bones by name, in the BMD's order, then the file's extra
//    joints (parents first); the BMD bone index of a joint is its index here;
//  - clips: an action's pose comes from its clip, sampled by phase on the
//    action's MU timeline (SampleJoint);
//  - skin: every vertex of the BMD's meshes with up to four joints and weights
//    (SkinPosition, SkinNormal).
// Everything here is in MU model space: glTF (x, y, z) is MU (x, -z, y),
// 1 unit = 1 MU unit.
namespace Render::Models::Gltf
{
using Quaternion = std::array<float, 4>; // x, y, z, w, as AngleQuaternion writes it
using Vector3 = std::array<float, 3>;
using Matrix34 = std::array<std::array<float, 4>, 3>;

// A rotation and a translation: the local transform of a joint relative to its
// parent joint, in MU space for a joint without one.
struct JointPose
{
    Quaternion rotation{0.f, 0.f, 0.f, 1.f};
    Vector3 position{0.f, 0.f, 0.f};
};

struct Joint
{
    std::string name;
    // Index of the parent joint, -1 for a root.
    int parent = -1;
    // The pose without animation (the node's own transform).
    JointPose rest;
    // Root joints: the transform of the nodes above the skeleton, glTF to MU
    // space included, applied before the joint's own; identity otherwise.
    JointPose base;
    // The file's inverse bind matrix: vertex attribute space -> joint space.
    Matrix34 inverseBind{};
};

// One LINEAR animation channel of a joint: key times and values (3 or 4 per key).
struct Channel
{
    std::vector<float> times;
    std::vector<float> values;

    bool Empty() const
    {
        return times.empty();
    }
};

struct JointTrack
{
    Channel translation;
    Channel rotation;
};

struct Clip
{
    std::string name;
    // The last key's time, in seconds: one cycle of the action.
    float cycle = 0.f;
    // Per joint; a joint without a channel keeps its rest value.
    std::vector<JointTrack> joints;
};

// A vertex of a BMD mesh of the model: up to four joints with weights that sum
// to 1, its position and normal in the space of each of these joints, and as
// the file has them (bind space: the inverse bind matrices map it to joint
// space; the GPU skins from there, see Render::Models::SkinnedVertexPacking).
struct SkinnedVertex
{
    int count = 0;
    std::array<short, 4> joints{};
    std::array<float, 4> weights{};
    std::array<Vector3, 4> positions{};
    std::array<Vector3, 4> normals{};
    Vector3 bindPosition{};
    Vector3 bindNormal{};
};

// An image the file carries (a material's base colour texture), still encoded.
struct EmbeddedImage
{
    // The image's name in the file, else "image<index>".
    std::string name;
    std::string mimeType;
    std::vector<std::uint8_t> bytes;
};

class SkinnedModel
{
public:
    std::vector<Joint> joints;
    // The number of joints that are the BMD's own bones (the first ones).
    int muBoneCount = 0;
    std::vector<Clip> clips;
    // Per BMD action: index into `clips`, -1 when the file has no clip for it
    // (the action shows the rest pose).
    std::vector<int> actionClips;
    // Per BMD action: the length of the crossfade into it in ticks
    // (Data/Models/ModelFiles.json "crossfade"), nothing for the playback rule's
    // fade (AnimationSampling::SetCrossfadeLength).
    std::vector<std::optional<float>> actionCrossfades;
    // Per BMD mesh, per vertex (the mesh's vertex and normal i are vertex i here).
    std::vector<std::vector<SkinnedVertex>> meshVertices;
    // The images the meshes' materials use as base colour texture.
    std::vector<EmbeddedImage> images;
    // Per BMD mesh: index into `images`, -1 when its material has no embedded
    // image (the mesh's texture is then looked up by its name, as for a BMD).
    std::vector<int> meshImages;

    // The key intervals of MU's timeline that one cycle of an action's clip
    // spans (the clip rule): n - 1 for a locked action (MU
    // wraps at n - 1) and for a held one (Action_t::Loop: it stops on key
    // n - 1), n otherwise (a repeating action blends key n - 1 into key 0).
    // At least 1.
    [[nodiscard]] static int ClipKeyIntervals(int numKeys, bool lockPositions, bool held);

    // The clip time of MU key time `keyTime` of an action played over
    // `keyIntervals` key intervals: keyTime / keyIntervals x cycle, clamped to
    // the clip.
    [[nodiscard]] static float ClipTime(float keyTime, int keyIntervals, float cycle);

    // The local pose of a joint in action `action` at MU key time `keyTime`
    // (BMD bone conventions: root joints in MU model space, others relative to
    // their parent). Rotations are interpolated with QuaternionNLERP, positions
    // linearly.
    void SampleJoint(int joint, int action, float keyTime, int keyIntervals, JointPose& pose) const;

    // `pose` of the head bone turned by the game's head angles (radians), as
    // BMD::Animation turns a BMD key: its Euler angles x and z minus angleX and
    // angleY. With AngleQuaternion's order that is Rz(-angleY) * q * Rx(-angleX).
    static void TurnHead(Quaternion& rotation, float angleX, float angleY);

    // The rest pose of every joint in MU model space (no parent matrix).
    [[nodiscard]] std::vector<Matrix34> RestPose() const;

    // The skinned position of vertex `vertex` of BMD mesh `mesh`, with the
    // arithmetic of BMD::SkinVertex applied per joint and weighted: `scale`
    // (non-zero) scales the joint-space position first when boneScale is 1,
    // otherwise the rotated position is scaled by boneScale.
    void SkinPosition(int mesh, int vertex, const float (*boneMatrices)[3][4], float boneScale, float scale,
                      float out[3]) const;
    // The skinned normal (weighted rotation, normalised).
    void SkinNormal(int mesh, int vertex, const float (*boneMatrices)[3][4], float out[3]) const;
};

// The model's SkinnedModel when it was loaded from a glTF binary, else nullptr.
[[nodiscard]] const SkinnedModel* SkinnedModelOf(const BMD& model);
} // namespace Render::Models::Gltf
