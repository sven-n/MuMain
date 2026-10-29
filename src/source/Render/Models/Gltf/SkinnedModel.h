#pragma once

#include "Data/GameData/ModelData/ModelFiles.h"
#include "Render/Renderer/MuRenderer.h"

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
    // The file's TANGENT in bind space, w = handedness (+1 or -1, the sign of the
    // bitangent cross(normal, tangent) x w); all 0 when the vertex's material
    // has no normal map (only those carry tangents).
    std::array<float, 4> bindTangent{};
};

// An image the file carries (a material's base colour texture), still encoded.
struct EmbeddedImage
{
    // The image's name in the file, else "image<index>".
    std::string name;
    std::string mimeType;
    std::vector<std::uint8_t> bytes;
};

// A mesh's normal map, as SkinnedModel and LodMesh keep it: the image
// (index into SkinnedModel::images, -1: none) and the material's
// normalTexture.scale (glTF: the map's x and y are multiplied by it).
struct NormalMap
{
    int image = -1;
    float scale = 1.f;
};

// A normal map ready to draw: its texture (a bitmap index) and scale; scale 0
// means none (the unperturbed normal is lit).
struct NormalMapTexture
{
    unsigned int texture = 0;
    float scale = 0.f;
};

// A mesh of a coarser LOD (extras.lod 1 or 2), on the
// same skin as the BMD's meshes (LOD 0): drawn in place of the BMD mesh of the
// same index while that LOD is active (BMD::ActiveLod).
struct LodMesh
{
    std::vector<SkinnedVertex> vertices;
    // Per vertex: u, v.
    std::vector<std::array<float, 2>> texCoords;
    // Vertex indices of each triangle.
    std::vector<std::array<int, 3>> triangles;
    // The material's extras.texture (the BMD texture it stands for), may be empty.
    std::string texture;
    // Index into SkinnedModel::images of the base colour image, -1 for none.
    int image = -1;
    // The material's normal map (Enhanced only).
    NormalMap normalMap;
};

// A mesh packed for indexed GPU-skinned draws: each distinct corner (vertex
// and texture coordinate) once, in order of first use, and the triangles as
// indices into them; `indices` empty when the vertices are the corners
// themselves (more distinct corners than 16-bit indices reach).
struct PackedMesh
{
    std::vector<mu::SkinnedWeightedVertex3D> vertices;
    std::vector<std::uint16_t> indices;
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
    // Per BMD mesh: its material's normal map (Enhanced lighting only).
    std::vector<NormalMap> meshNormalMaps;
    // Per BMD mesh, then per lods, per mesh: the normal map's texture (a bitmap
    // index), BITMAP_UNKNOWN when the mesh has none or it cannot be used; set
    // with the other textures (OpenEmbeddedTextures).
    mutable std::vector<unsigned int> meshNormalTextures;
    mutable std::vector<std::vector<unsigned int>> lodNormalTextures;
    // The coarser LODs: lods[k] holds LOD k + 1, each with at most as many
    // meshes as the BMD has (LOD 0).
    std::vector<std::vector<LodMesh>> lods;
    // Per lods, per mesh: its texture (a bitmap index), set when the model's
    // textures are opened (OpenEmbeddedTextures), after loading.
    mutable std::vector<std::vector<unsigned int>> lodTextures;
    // When the LODs are drawn (Data/Models/ModelFiles.json "lod").
    Data::Models::LodSettings lodSettings;
    // Per LOD, per BMD mesh: its GPU vertices and triangle indices as a
    // GPU-skinned draw sends them (Render::Models::PackedWeightedMeshOf),
    // packed on the mesh's first draw at that LOD and unchanged after that.
    // The renderer caches a packed mesh under its vertices' address
    // (BMD::RenderMesh); the destructor releases those.
    mutable std::vector<std::vector<PackedMesh>> packedMeshes;

    ~SkinnedModel();

    // The number of LODs, 1 + lods.size().
    [[nodiscard]] int LodCount() const
    {
        return 1 + static_cast<int>(lods.size());
    }

    // The mesh drawn in place of BMD mesh `mesh` while LOD `lod` is active:
    // nullptr for LOD 0 or a LOD the model does not have (the BMD mesh is
    // drawn), and for a mesh beyond the LOD's meshes, which sets `absent` (that
    // LOD draws nothing for it).
    [[nodiscard]] const LodMesh* LodMeshFor(int lod, int mesh, bool& absent) const;

    // The normal map texture of BMD mesh `mesh` at LOD `lod` (the LOD's mesh
    // that stands in for it, see LodMeshFor) and its scale; scale 0 when the
    // mesh has no usable map.
    [[nodiscard]] NormalMapTexture NormalMapTextureFor(int lod, int mesh) const;

    // The box around LOD 0 in rest pose, in MU model space (unscaled): the
    // bounds whose screen height picks the LOD. Set by ComputeRestBounds.
    Vector3 restBoundsMin{};
    Vector3 restBoundsMax{};
    void ComputeRestBounds();

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
    // The same for any vertex on the skin (a coarser LOD's, see LodMesh).
    static void SkinPosition(const SkinnedVertex& vertex, const float (*boneMatrices)[3][4], float boneScale,
                             float scale, float out[3]);
    // The skinned normal (weighted rotation, normalised).
    void SkinNormal(int mesh, int vertex, const float (*boneMatrices)[3][4], float out[3]) const;
};

// The model's SkinnedModel when it was loaded from a glTF binary, else nullptr.
[[nodiscard]] const SkinnedModel* SkinnedModelOf(const BMD& model);
} // namespace Render::Models::Gltf
