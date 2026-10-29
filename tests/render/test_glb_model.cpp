// doctest unit tests for monster models read from glTF binaries
// (Render/Models/Gltf/): the Elite Yeti exported from its BMD
// (tests/render/data/Monster14.glb, see the README there) loaded into the BMD
// gives the BMD's bones, rest pose and geometry, and the exported Bull Fighter
// and Bali (Monster01.glb, Monster33.glb) its poses; files that break the
// conventions are refused with a reason and leave the BMD as it was. Also the
// wiring around it: the model file table with the developer overlay, the
// images a file carries, and the GPU vertex packing and palette of
// four-weight skinning (Render/Models/SkinnedVertexPacking).
//
// Run: ctest --test-dir <build directory> -R "glb_model"

#include "App/stdafx.h"

#include <doctest.h>

#include "Core/Globals/_enum.h"
#include "Core/Math/ZzzMathLib.h"
#include "Data/DataHandler/ModelData/ModelFileDataHandler.h"
#include "Render/Models/AnimationSampling.h"
#include "Render/Models/Gltf/EmbeddedTextures.h"
#include "Render/Models/Gltf/GlbLoader.h"
#include "Render/Models/Gltf/SkinnedModel.h"
#include "Render/Models/ModelLod.h"
#include "Render/Models/SkinnedVertexPacking.h"
#include "Render/Models/ZzzBMD.h"

#include "Core/Utilities/Log/MuLogger.h"

#include "json.hpp"

#include <spdlog/sinks/ringbuffer_sink.h>
#include <SDL3/SDL_surface.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <map>
#include <utility>
#include <memory>
#include <random>
#include <string>
#include <vector>

namespace
{
using Render::Models::Gltf::LoadGlbIntoBmd;
using Render::Models::Gltf::Matrix34;
using Render::Models::Gltf::SkinnedModelOf;

constexpr int YetiBones = 51;
constexpr int YetiActions = 7;

const std::filesystem::path MonsterDirectory = std::filesystem::path(MU_TEST_DATA_DIR) / "Monster";
const std::filesystem::path YetiGlb = std::filesystem::path(MU_TEST_GLB_DIR) / "Monster14.glb";
const std::map<int, Data::Models::ActionClip> NoActionClips;

std::unique_ptr<BMD> OpenYetiBmd()
{
    auto model = std::make_unique<BMD>();
    const std::wstring folder = MonsterDirectory.wstring() + L"/";
    REQUIRE(model->Open2(folder.c_str(), L"Monster14.bmd"));
    REQUIRE(model->NumBones == YetiBones);
    REQUIRE(model->NumActions == YetiActions);
    return model;
}

std::unique_ptr<BMD> OpenYetiGlb(const std::filesystem::path& file = YetiGlb)
{
    auto model = OpenYetiBmd();
    std::string reason;
    const bool loaded = LoadGlbIntoBmd(*model, file, NoActionClips, reason);
    INFO(reason);
    REQUIRE(loaded);
    REQUIRE(model->GltfModel != nullptr);
    return model;
}

// A glTF binary as JSON and binary chunk, to change a valid file into one
// that breaks a rule.
struct Glb
{
    nlohmann::json json;
    std::vector<std::uint8_t> bin;

    static Glb Read(const std::filesystem::path& file)
    {
        std::ifstream in(file, std::ios::binary);
        const std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        REQUIRE(bytes.size() > 20);
        const auto u32 = [&](size_t offset) {
            std::uint32_t value = 0;
            std::memcpy(&value, &bytes[offset], 4);
            return value;
        };
        Glb glb;
        const std::uint32_t jsonLength = u32(12);
        glb.json = nlohmann::json::parse(bytes.begin() + 20, bytes.begin() + 20 + jsonLength);
        const size_t binHeader = 20 + jsonLength;
        const std::uint32_t binLength = u32(binHeader);
        glb.bin.assign(bytes.begin() + binHeader + 8, bytes.begin() + binHeader + 8 + binLength);
        return glb;
    }

    void Write(const std::filesystem::path& file)
    {
        json["buffers"][0]["byteLength"] = bin.size();
        std::string text = json.dump();
        while (text.size() % 4 != 0)
            text.push_back(' ');
        while (bin.size() % 4 != 0)
            bin.push_back(0);
        std::ofstream out(file, std::ios::binary);
        const auto u32 = [&](std::uint32_t value) { out.write(reinterpret_cast<const char*>(&value), 4); };
        u32(0x46546C67);
        u32(2);
        u32(static_cast<std::uint32_t>(12 + 8 + text.size() + 8 + bin.size()));
        u32(static_cast<std::uint32_t>(text.size()));
        u32(0x4E4F534A);
        out.write(text.data(), static_cast<std::streamsize>(text.size()));
        u32(static_cast<std::uint32_t>(bin.size()));
        u32(0x004E4942);
        out.write(reinterpret_cast<const char*>(bin.data()), static_cast<std::streamsize>(bin.size()));
    }

    // Appends float data as a new accessor; returns its index.
    int AddAccessor(const std::vector<float>& values, const char* type, size_t count)
    {
        while (bin.size() % 4 != 0)
            bin.push_back(0);
        const size_t offset = bin.size();
        bin.resize(offset + values.size() * sizeof(float));
        std::memcpy(&bin[offset], values.data(), values.size() * sizeof(float));
        json["bufferViews"].push_back({{"buffer", 0}, {"byteOffset", offset}, {"byteLength", values.size() * 4}});
        json["accessors"].push_back({{"bufferView", json["bufferViews"].size() - 1},
                                     {"componentType", 5126},
                                     {"count", count},
                                     {"type", type}});
        return static_cast<int>(json["accessors"].size() - 1);
    }

    int NodeNamed(const std::string& name) const
    {
        const auto& nodes = json["nodes"];
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            if (nodes[i].value("name", "") == name)
                return static_cast<int>(i);
        }
        FAIL("no node " << name);
        return -1;
    }

    // Adds a node below `parent` (a node index); returns its index.
    int AddNode(const std::string& name, int parent)
    {
        json["nodes"].push_back({{"name", name}, {"translation", {0.0, 0.0, 10.0}}});
        const int node = static_cast<int>(json["nodes"].size() - 1);
        json["nodes"][parent]["children"].push_back(node);
        return node;
    }

    // Makes a node a joint of the skin with an identity inverse bind matrix;
    // the skin's inverse bind matrices move to a new accessor.
    void AddSkinJoint(int node)
    {
        auto& skin = json["skins"][0];
        const auto& accessor = json["accessors"][skin["inverseBindMatrices"].get<int>()];
        const auto& view = json["bufferViews"][accessor["bufferView"].get<int>()];
        const size_t offset = view.value("byteOffset", 0) + accessor.value("byteOffset", 0);
        const size_t count = accessor["count"];
        std::vector<float> matrices(count * 16);
        std::memcpy(matrices.data(), &bin[offset], matrices.size() * sizeof(float));
        const float identity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        matrices.insert(matrices.end(), std::begin(identity), std::end(identity));
        skin["joints"].push_back(node);
        skin["inverseBindMatrices"] = AddAccessor(matrices, "MAT4", count + 1);
    }
};

class TemporaryFile
{
public:
    TemporaryFile()
    {
        static std::atomic<int> counter{0};
        path_ = std::filesystem::temp_directory_path() /
                ("glb_model_test_" + std::to_string(std::random_device{}()) + "_" + std::to_string(counter++) + ".glb");
    }
    ~TemporaryFile()
    {
        std::error_code error;
        std::filesystem::remove(path_, error);
    }
    TemporaryFile(const TemporaryFile&) = delete;
    TemporaryFile& operator=(const TemporaryFile&) = delete;
    const std::filesystem::path& Path() const { return path_; }

private:
    std::filesystem::path path_;
};

// BMD::Animation without blending: every bone's model-space matrix at one key
// (no parent matrix), dummy bones identity.
std::vector<Matrix34> BmdKeyPose(const BMD& model, int action, int key)
{
    std::vector<Matrix34> world(model.NumBones);
    for (int i = 0; i < model.NumBones; ++i)
    {
        const Bone_t& bone = model.Bones[i];
        float local[3][4] = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}};
        if (!bone.Dummy)
        {
            const BoneMatrix_t& keys = bone.BoneMatrixes[action];
            QuaternionMatrix(keys.Quaternion[key], local);
            for (int r = 0; r < 3; ++r)
                local[r][3] = keys.Position[key][r];
        }
        float out[3][4];
        if (!bone.Dummy && bone.Parent >= 0)
        {
            float parent[3][4];
            std::memcpy(parent, world[bone.Parent].data(), sizeof(parent));
            R_ConcatTransforms(parent, local, out);
        }
        else
        {
            std::memcpy(out, local, sizeof(out));
        }
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 4; ++c)
                world[i][r][c] = out[r][c];
    }
    return world;
}

float MaxDifference(const Matrix34& a, const Matrix34& b)
{
    float difference = 0.f;
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 4; ++c)
            difference = std::max(difference, std::fabs(a[r][c] - b[r][c]));
    return difference;
}

void Apply(const Matrix34& m, const float in[3], float out[3])
{
    for (int r = 0; r < 3; ++r)
        out[r] = m[r][0] * in[0] + m[r][1] * in[1] + m[r][2] * in[2] + m[r][3];
}

void RequireRefused(const std::filesystem::path& file, const std::string& expected,
                    const std::map<int, Data::Models::ActionClip>& actions = NoActionClips)
{
    auto model = OpenYetiBmd();
    const Mesh_t* meshes = model->Meshs;
    const Bone_t* bones = model->Bones;
    std::string reason;
    CHECK_FALSE(LoadGlbIntoBmd(*model, file, actions, reason));
    INFO(reason);
    CHECK(reason.find(expected) != std::string::npos);
    CHECK(model->GltfModel == nullptr);
    CHECK(model->Meshs == meshes);
    CHECK(model->Bones == bones);
    CHECK(model->NumBones == YetiBones);
}
} // namespace

TEST_CASE("glb_model: the exported Elite Yeti keeps the BMD's bones, parents and actions")
{
    const auto bmd = OpenYetiBmd();
    const auto glb = OpenYetiGlb();
    REQUIRE(glb->NumBones == YetiBones);
    CHECK(SkinnedModelOf(*glb)->muBoneCount == YetiBones);
    for (int i = 0; i < YetiBones; ++i)
    {
        CAPTURE(i);
        CHECK(std::string(glb->Bones[i].Name) == std::string(bmd->Bones[i].Name));
        CHECK(glb->Bones[i].Dummy == bmd->Bones[i].Dummy);
        if (!bmd->Bones[i].Dummy)
            CHECK(glb->Bones[i].Parent == bmd->Bones[i].Parent);
    }
    REQUIRE(glb->NumActions == YetiActions);
    for (int a = 0; a < YetiActions; ++a)
    {
        CAPTURE(a);
        CHECK(glb->Actions[a].NumAnimationKeys == bmd->Actions[a].NumAnimationKeys);
        CHECK(glb->Actions[a].LockPositions == bmd->Actions[a].LockPositions);
        CHECK(SkinnedModelOf(*glb)->actionClips[a] >= 0);
        CHECK(SkinnedModelOf(*glb)->clips[SkinnedModelOf(*glb)->actionClips[a]].name == "action0" + std::to_string(a));
    }
}

TEST_CASE("glb_model: the exported Elite Yeti's rest pose is the BMD's action 0 key 0 within 1e-4")
{
    const auto bmd = OpenYetiBmd();
    const auto glb = OpenYetiGlb();
    const std::vector<Matrix34> expected = BmdKeyPose(*bmd, 0, 0);
    const std::vector<Matrix34> rest = SkinnedModelOf(*glb)->RestPose();
    REQUIRE(rest.size() == expected.size());
    for (size_t i = 0; i < rest.size(); ++i)
    {
        CAPTURE(i);
        CHECK(MaxDifference(rest[i], expected[i]) < 1e-4f);
    }
}

TEST_CASE("glb_model: the exported Elite Yeti's mesh is the BMD's, corner by corner, in rest pose")
{
    const auto bmd = OpenYetiBmd();
    const auto glb = OpenYetiGlb();
    REQUIRE(bmd->NumMeshs == 1);
    REQUIRE(glb->NumMeshs == 1);
    CHECK(std::string(glb->Textures[0].FileName) == std::string(bmd->Textures[0].FileName));
    const Mesh_t& original = bmd->Meshs[0];
    const Mesh_t& mesh = glb->Meshs[0];
    // The exporter writes every triangle corner as a vertex of its own.
    REQUIRE(mesh.NumTriangles == original.NumTriangles);
    CHECK(mesh.NumVertices == 3 * original.NumTriangles);
    const std::vector<Matrix34> bmdPose = BmdKeyPose(*bmd, 0, 0);
    const std::vector<Matrix34> glbPose = SkinnedModelOf(*glb)->RestPose();
    float largest = 0.f;
    for (int t = 0; t < mesh.NumTriangles; ++t)
    {
        for (int c = 0; c < 3; ++c)
        {
            const Vertex_t& a = original.Vertices[original.Triangles[t].VertexIndex[c]];
            const Vertex_t& b = mesh.Vertices[mesh.Triangles[t].VertexIndex[c]];
            CHECK(b.Node == a.Node);
            float pa[3], pb[3];
            Apply(bmdPose[a.Node], a.Position, pa);
            Apply(glbPose[b.Node], b.Position, pb);
            for (int k = 0; k < 3; ++k)
                largest = std::max(largest, std::fabs(pa[k] - pb[k]));
            const TexCoord_t& ua = original.TexCoords[original.Triangles[t].TexCoordIndex[c]];
            const TexCoord_t& ub = mesh.TexCoords[mesh.Triangles[t].TexCoordIndex[c]];
            CHECK(ub.TexCoordU == doctest::Approx(ua.TexCoordU));
            CHECK(ub.TexCoordV == doctest::Approx(ua.TexCoordV));
        }
    }
    CHECK(largest < 1e-3f);
}

TEST_CASE("glb_model: extra joints are appended after the BMD's bones, parents first")
{
    Glb glb = Glb::Read(YetiGlb);
    const int spine = glb.NodeNamed("Bip01 Spine");
    // The child is listed before its parent in the skin.
    const int tail1 = glb.AddNode("Tail1", spine);
    const int tail2 = glb.AddNode("Tail2", tail1);
    glb.AddSkinJoint(tail2);
    glb.AddSkinJoint(tail1);
    TemporaryFile file;
    glb.Write(file.Path());

    const auto bmd = OpenYetiBmd();
    const auto model = OpenYetiGlb(file.Path());
    REQUIRE(model->NumBones == YetiBones + 2);
    for (int i = 0; i < YetiBones; ++i)
    {
        CAPTURE(i);
        CHECK(std::string(model->Bones[i].Name) == std::string(bmd->Bones[i].Name));
    }
    CHECK(std::string(model->Bones[YetiBones].Name) == "Tail1");
    CHECK(model->Bones[YetiBones].Parent == 17); // Bip01 Spine
    CHECK(std::string(model->Bones[YetiBones + 1].Name) == "Tail2");
    CHECK(model->Bones[YetiBones + 1].Parent == YetiBones);
    CHECK_FALSE(model->Bones[YetiBones].Dummy);
}

TEST_CASE("glb_model: swapping model slots moves the skin and refreshes each bone lookup")
{
    Glb fileModel = Glb::Read(YetiGlb);
    fileModel.AddSkinJoint(fileModel.AddNode("Extra", fileModel.NodeNamed("Bip01 Spine")));
    TemporaryFile file;
    fileModel.Write(file.Path());

    const auto bmd = OpenYetiBmd();
    const auto glb = OpenYetiGlb(file.Path());
    const auto skin = glb->GltfModel;
    REQUIRE(skin != nullptr);
    REQUIRE(bmd->FindBone("Extra") == -1);
    REQUIRE(glb->FindBone("Extra") == YetiBones);

    bmd->SwapData(*glb);
    CHECK(bmd->GltfModel == skin);
    CHECK(glb->GltfModel == nullptr);
    CHECK(bmd->NumBones == YetiBones + 1);
    CHECK(glb->NumBones == YetiBones);
    CHECK(bmd->FindBone("Extra") == YetiBones);
    CHECK(glb->FindBone("Extra") == -1);

    bmd->SwapData(*bmd);
    CHECK(bmd->GltfModel == skin);
    CHECK(bmd->FindBone("Extra") == YetiBones);
}

TEST_CASE("glb_model: a file without a bone of the BMD is refused and the BMD stays")
{
    Glb glb = Glb::Read(YetiGlb);
    glb.json["nodes"][glb.NodeNamed("Box03")]["name"] = "Box03 renamed";
    TemporaryFile file;
    glb.Write(file.Path());
    RequireRefused(file.Path(), "lacks the bone \"Box03\"");
}

TEST_CASE("glb_model: a file with more than MAX_BONES joints is refused")
{
    Glb glb = Glb::Read(YetiGlb);
    const int spine = glb.NodeNamed("Bip01 Spine");
    for (int i = YetiBones; i <= MAX_BONES; ++i)
        glb.AddSkinJoint(glb.AddNode("Extra" + std::to_string(i), spine));
    REQUIRE(glb.json["skins"][0]["joints"].size() == MAX_BONES + 1);
    TemporaryFile file;
    glb.Write(file.Path());
    RequireRefused(file.Path(), "201 joints, at most 200");
}

TEST_CASE("glb_model: refusals name their reason")
{
    TemporaryFile file;
    SUBCASE("no file")
    {
        RequireRefused(file.Path(), "no glTF file");
    }
    SUBCASE("no inverse bind matrices")
    {
        Glb glb = Glb::Read(YetiGlb);
        glb.json["skins"][0].erase("inverseBindMatrices");
        glb.Write(file.Path());
        RequireRefused(file.Path(), "no inverse bind matrix");
    }
    SUBCASE("a scaled joint")
    {
        Glb glb = Glb::Read(YetiGlb);
        glb.json["nodes"][glb.NodeNamed("Bip01 Head")]["scale"] = {2.0, 2.0, 2.0};
        glb.Write(file.Path());
        RequireRefused(file.Path(), "\"Bip01 Head\" is scaled");
    }
    SUBCASE("a mesh without weights")
    {
        Glb glb = Glb::Read(YetiGlb);
        glb.json["meshes"][0]["primitives"][0]["attributes"].erase("WEIGHTS_0");
        glb.Write(file.Path());
        RequireRefused(file.Path(), "lacks WEIGHTS_0");
    }
    SUBCASE("a clip the table names that the file does not have")
    {
        RequireRefused(YetiGlb, "no clip \"run\" for action 2", {{2, {"run", std::nullopt}}});
    }
}

// ---------------------------------------------------------------- animation

namespace
{
constexpr int YetiHeadBone = 20;
constexpr unsigned short YetiDie = 6;
constexpr unsigned short YetiWalk = 2;
constexpr unsigned short YetiAttack1 = 3;
constexpr unsigned short YetiAttack2 = 4;
constexpr unsigned short YetiStop1 = 0;

// The game's state of a monster model that BMD::Animation reads.
void PrepareLikeTheGame(BMD& model)
{
    model.BoneHead = YetiHeadBone;
    model.BodyScale = 1.4f;
    Vector(1200.f, -800.f, 160.f, model.BodyOrigin);
    model.BodyHeight = 0.f;
    for (int a = 0; a < model.NumActions; ++a)
    {
        model.Actions[a].PlaySpeed = 0.25f;
        model.Actions[a].Loop = false;
    }
    model.Actions[YetiWalk].PlaySpeed = 0.28f;
    // OpenMonsterModel holds MONSTER01_DIE on its last key.
    model.Actions[YetiDie].Loop = true;
}

struct PoseCase
{
    unsigned short action;
    float frame;
    unsigned short priorAction;
    float priorFrame;
};

struct BoneMatrices
{
    float bones[MAX_BONES][3][4];
};

// The largest difference of a GLB model's bone matrices from the BMD's, over
// the BMD's bones, rotation and translation parts apart.
struct Difference
{
    float rotation = 0.f;
    float translation = 0.f;
};

Difference Compare(BMD& bmd, BMD& glb, const PoseCase& pose, const vec3_t headAngle)
{
    vec3_t angle = {0.f, 0.f, 35.f};
    vec3_t head = {headAngle[0], headAngle[1], headAngle[2]};
    auto expected = std::make_unique<BoneMatrices>();
    auto actual = std::make_unique<BoneMatrices>();
    bmd.Animation(expected->bones, pose.frame, pose.priorFrame, pose.priorAction, angle, head, false, true, nullptr,
                  static_cast<short>(pose.action));
    glb.Animation(actual->bones, pose.frame, pose.priorFrame, pose.priorAction, angle, head, false, true, nullptr,
                  static_cast<short>(pose.action));
    Difference difference;
    for (int i = 0; i < bmd.NumBones; ++i)
    {
        for (int r = 0; r < 3; ++r)
        {
            for (int c = 0; c < 3; ++c)
                difference.rotation =
                    std::max(difference.rotation, std::fabs(expected->bones[i][r][c] - actual->bones[i][r][c]));
            difference.translation =
                std::max(difference.translation, std::fabs(expected->bones[i][r][3] - actual->bones[i][r][3]));
        }
    }
    return difference;
}

// Tolerances of the GLB pose against the BMD's, in the units of the bone
// matrices (rotation part unitless and scaled by BodyScale 1.4, translation in
// MU units, bones placed ~1400 units from the origin). The exporter writes
// MU's Euler keys as the BMD's own quaternions (AngleQuaternion in float) and
// key k at the clip time the client computes for MU key k, but a fraction
// between keys comes from the clip time here and from the frame in the BMD,
// and the root's glTF-to-MU turn is one more float product: both differ by a
// few 1e-7. Measured: rotation <= 1e-6, translation <= 2.5e-4 (two float
// steps near 1400, where one step is 1.2e-4). The tolerances keep about two orders of magnitude for the
// rotation and eight float steps for the translation (1.4e-6 relative to the
// positions); a wrong axis, key, fraction or crossfade source moves bones by
// 1e-3 of the rotation and by whole units.
constexpr float RotationTolerance = 1e-4f;
constexpr float TranslationTolerance = 1e-3f;

void CheckPose(BMD& bmd, BMD& glb, const PoseCase& pose, const vec3_t headAngle, Difference& largest,
               float translationTolerance = TranslationTolerance)
{
    CAPTURE(pose.action);
    CAPTURE(pose.frame);
    CAPTURE(pose.priorAction);
    CAPTURE(pose.priorFrame);
    const Difference difference = Compare(bmd, glb, pose, headAngle);
    CHECK(difference.rotation < RotationTolerance);
    CHECK(difference.translation < translationTolerance);
    largest.rotation = std::max(largest.rotation, difference.rotation);
    largest.translation = std::max(largest.translation, difference.translation);
}

// The last frame MU reaches in an action (PlayAnimation): a locked action
// wraps at n - 1, a repeating one at n, a held one stops just below n.
float FrameLimit(const Action_t& action)
{
    if (action.Loop)
        return static_cast<float>(action.NumAnimationKeys) - 0.01f;
    return static_cast<float>(action.LockPositions ? action.NumAnimationKeys - 1 : action.NumAnimationKeys);
}
} // namespace

TEST_CASE("glb_model: clip helpers")
{
    using Render::Models::Gltf::SkinnedModel;
    CHECK(SkinnedModel::ClipKeyIntervals(20, false, false) == 20);
    CHECK(SkinnedModel::ClipKeyIntervals(7, true, false) == 6);
    CHECK(SkinnedModel::ClipKeyIntervals(16, false, true) == 15);
    CHECK(SkinnedModel::ClipKeyIntervals(1, false, false) == 1);
    CHECK(SkinnedModel::ClipKeyIntervals(1, true, true) == 1);
    CHECK(SkinnedModel::ClipTime(3.f, 6, 0.24f) == doctest::Approx(0.12f));
    CHECK(SkinnedModel::ClipTime(-1.f, 6, 0.24f) == 0.f);
    CHECK(SkinnedModel::ClipTime(7.f, 6, 0.24f) == doctest::Approx(0.24f));
    // A 30-key clip plays in the action's time: half the action is half the clip.
    CHECK(SkinnedModel::ClipTime(4.5f, 9, 1.2f) == doctest::Approx(0.6f));

    // TurnHead equals subtracting the head angles from a key's Euler angles.
    const float cases[][3] = {{0.3f, -0.7f, 1.1f}, {-2.f, 0.4f, -0.2f}, {0.f, 1.5f, 3.f}};
    for (const auto& angles : cases)
    {
        const float turnX = 0.21f;
        const float turnY = -0.43f;
        vec4_t q;
        AngleQuaternion(angles, q);
        Render::Models::Gltf::Quaternion turned{q[0], q[1], q[2], q[3]};
        SkinnedModel::TurnHead(turned, turnX, turnY);
        vec3_t subtracted = {angles[0] - turnX, angles[1], angles[2] - turnY};
        vec4_t expected;
        AngleQuaternion(subtracted, expected);
        const float sign = turned[0] * expected[0] + turned[1] * expected[1] + turned[2] * expected[2] +
                                       turned[3] * expected[3] < 0.f
                               ? -1.f
                               : 1.f;
        for (int k = 0; k < 4; ++k)
            CHECK(turned[k] * sign == doctest::Approx(expected[k]).epsilon(1e-5));
    }
}

TEST_CASE("glb_model: an extra joint's keys are its rest pose, Euler angles included")
{
    Glb glb = Glb::Read(YetiGlb);
    const int spine = glb.NodeNamed("Bip01 Spine");
    const int tail = glb.AddNode("Tail", spine);
    glb.json["nodes"][tail]["rotation"] = {0.3, -0.2, 0.5, std::sqrt(1.0 - 0.09 - 0.04 - 0.25)};
    glb.AddSkinJoint(tail);
    TemporaryFile file;
    glb.Write(file.Path());
    const auto model = OpenYetiGlb(file.Path());
    const Bone_t& bone = model->Bones[YetiBones];
    for (int a = 0; a < model->NumActions; ++a)
    {
        CAPTURE(a);
        const BoneMatrix_t& keys = bone.BoneMatrixes[a];
        REQUIRE(keys.Position != nullptr);
        CHECK(keys.Position[0][2] == doctest::Approx(10.f));
        vec4_t fromAngles;
        AngleQuaternion(keys.Rotation[0], fromAngles);
        const float* rest = keys.Quaternion[model->Actions[a].NumAnimationKeys - 1];
        const float dot = fromAngles[0] * rest[0] + fromAngles[1] * rest[1] + fromAngles[2] * rest[2] +
                          fromAngles[3] * rest[3];
        CHECK(std::fabs(dot) == doctest::Approx(1.f).epsilon(1e-5));
    }
}

TEST_CASE("glb_model: the exported Elite Yeti's bone matrices equal the BMD's at fractional frames of every action")
{
    auto bmd = OpenYetiBmd();
    auto glb = OpenYetiGlb();
    PrepareLikeTheGame(*bmd);
    PrepareLikeTheGame(*glb);
    const vec3_t still = {0.f, 0.f, 0.f};
    const vec3_t turned = {14.f, -27.f, 0.f};
    Difference largest;
    for (unsigned short action = 0; action < YetiActions; ++action)
    {
        const Action_t& timeline = bmd->Actions[action];
        const float limit = FrameLimit(timeline);
        const float period = static_cast<float>(timeline.LockPositions ? timeline.NumAnimationKeys - 1
                                                                       : timeline.NumAnimationKeys);
        // Ordinary playback, one step at the action's speed, the wrap included.
        for (float frame = 0.f; frame < limit; frame += 0.13f)
        {
            float prior = frame - timeline.PlaySpeed;
            if (prior < 0.f)
                prior = timeline.Loop ? 0.f : prior + period;
            CheckPose(*bmd, *glb, {action, frame, action, prior}, still, largest);
            CheckPose(*bmd, *glb, {action, frame, action, prior}, turned, largest);
        }
        // A held pose (prior key = current key).
        CheckPose(*bmd, *glb, {action, 1.5f, action, 1.2f}, turned, largest);
    }
    MESSAGE("largest difference: rotation " << largest.rotation << ", translation " << largest.translation);
}

TEST_CASE("glb_model: the exported Elite Yeti follows the BMD when keys are skipped and on an action change")
{
    auto bmd = OpenYetiBmd();
    auto glb = OpenYetiGlb();
    PrepareLikeTheGame(*bmd);
    PrepareLikeTheGame(*glb);
    const vec3_t turned = {-9.f, 31.f, 0.f};
    Difference largest;
    // Skip: two to five keys in one step, also across a wrap and in a locked action.
    for (const PoseCase& pose : {PoseCase{YetiStop1, 4.35f, YetiStop1, 2.f}, PoseCase{YetiStop1, 1.7f, YetiStop1, 17.4f},
                                 PoseCase{YetiWalk, 4.6f, YetiWalk, 1.1f}, PoseCase{YetiWalk, 0.8f, YetiWalk, 4.2f},
                                 PoseCase{YetiAttack1, 6.5f, YetiAttack1, 1.9f}, PoseCase{YetiDie, 12.3f, YetiDie, 8.f}})
    {
        CheckPose(*bmd, *glb, pose, turned, largest);
    }
    // Crossfade: from the pose on screen (prior frame >= 1 and playing) or from
    // MU's key (prior frame below 1, or a prior action that does not play).
    for (const PoseCase& pose :
         {PoseCase{YetiAttack1, 0.4f, YetiWalk, 3.6f}, PoseCase{YetiWalk, 0.f, YetiStop1, 13.25f},
          PoseCase{YetiDie, 0.7f, YetiAttack1, 5.9f}, PoseCase{YetiStop1, 0.2f, YetiWalk, 0.5f},
          PoseCase{YetiAttack1, 0.9f, YetiDie, 15.99f}})
    {
        CheckPose(*bmd, *glb, pose, turned, largest);
    }
    bmd->Actions[YetiWalk].PlaySpeed = 0.f;
    glb->Actions[YetiWalk].PlaySpeed = 0.f;
    CheckPose(*bmd, *glb, {YetiAttack1, 0.4f, YetiWalk, 3.6f}, turned, largest);
    MESSAGE("largest difference: rotation " << largest.rotation << ", translation " << largest.translation);
}

namespace
{
// Two more exported originals (tests/render/data, see the README there) whose
// BMDs hold neighbouring keys within QuaternionCompare's epsilon at chain
// ends, with the game's head bone (OpenMonsterModel) and MONSTER01_DIE held.
struct ExportedMonster
{
    const wchar_t* bmd;
    const char* glb;
    int headBone;
};

constexpr ExportedMonster BullFighter{L"Monster01.bmd", "Monster01.glb", 20};
constexpr ExportedMonster Bali{L"Monster33.bmd", "Monster33.glb", 6};
constexpr float SkippedKeysTranslationTolerance = 2e-3f;

std::unique_ptr<BMD> OpenMonsterBmd(const ExportedMonster& monster)
{
    auto model = std::make_unique<BMD>();
    const std::wstring folder = MonsterDirectory.wstring() + L"/";
    REQUIRE(model->Open2(folder.c_str(), monster.bmd));
    REQUIRE(model->NumActions > YetiDie);
    return model;
}

std::unique_ptr<BMD> OpenMonsterGlb(const ExportedMonster& monster)
{
    auto model = OpenMonsterBmd(monster);
    std::string reason;
    const bool loaded =
        LoadGlbIntoBmd(*model, std::filesystem::path(MU_TEST_GLB_DIR) / monster.glb, NoActionClips, reason);
    INFO(reason);
    REQUIRE(loaded);
    REQUIRE(model->GltfModel != nullptr);
    return model;
}

void PrepareMonsterLikeTheGame(BMD& model, const ExportedMonster& monster)
{
    PrepareLikeTheGame(model);
    model.BoneHead = monster.headBone;
}
} // namespace

TEST_CASE("glb_model: an exported original's key k lies where the client samples MU key k")
{
    // The exporter writes key k at k / K x cycle in float, as ClipTime maps it;
    // k / 25 rounded on its own lies one float step off for some keys (20
    // intervals: keys 9 and 18), and the client then sampled key k - 1 blended
    // into key k by a weight of nearly 1.
    using Render::Models::Gltf::SkinnedModel;
    for (const ExportedMonster& monster : {BullFighter, Bali})
    {
        CAPTURE(monster.glb);
        auto glb = OpenMonsterGlb(monster);
        const SkinnedModel* gltf = SkinnedModelOf(*glb);
        REQUIRE(gltf != nullptr);
        for (const auto& clip : gltf->clips)
        {
            CAPTURE(clip.name);
            for (const auto& track : clip.joints)
            {
                for (const auto* channel : {&track.translation, &track.rotation})
                {
                    const int intervals = static_cast<int>(channel->times.size()) - 1;
                    for (int k = 0; k <= intervals && intervals >= 1; ++k)
                        CHECK(channel->times[static_cast<size_t>(k)] ==
                              SkinnedModel::ClipTime(static_cast<float>(k), intervals, clip.cycle));
                }
            }
        }
    }
}

TEST_CASE("glb_model: an exported original's rotation keys are the BMD's quaternions to float rounding")
{
    // The exporter computes AngleQuaternion with the client's formula (float,
    // sinf, cosf), so BMD::Animation's near-equal test of two keys decides alike
    // for both. sinf and cosf may differ by a few ULP between C libraries
    // (glibc, Apple's libm, MSVC), so a component may differ by float rounding;
    // a key may be stored negated (same hemisphere as the key before it).
    constexpr float QuaternionRounding = 1e-6f;
    using Render::Models::Gltf::SkinnedModel;
    for (const ExportedMonster& monster : {BullFighter, Bali})
    {
        CAPTURE(monster.glb);
        auto bmd = OpenMonsterBmd(monster);
        auto glb = OpenMonsterGlb(monster);
        const SkinnedModel* gltf = SkinnedModelOf(*glb);
        REQUIRE(gltf != nullptr);
        int compared = 0;
        int differing = 0;
        for (int action = 0; action < bmd->NumActions; ++action)
        {
            const int clipIndex = gltf->actionClips[static_cast<size_t>(action)];
            REQUIRE(clipIndex >= 0);
            const auto& clip = gltf->clips[static_cast<size_t>(clipIndex)];
            for (int bone = 0; bone < bmd->NumBones; ++bone)
            {
                const Bone_t& source = bmd->Bones[bone];
                const auto& rotation = clip.joints[static_cast<size_t>(bone)].rotation;
                if (source.Dummy || rotation.Empty())
                    continue;
                // Clip key k < K is BMD key k; the closing key repeats key 0 or
                // holds key n - 1.
                const size_t keys = std::min(rotation.times.size() - 1,
                                             static_cast<size_t>(bmd->Actions[action].NumAnimationKeys));
                for (size_t k = 0; k < keys; ++k)
                {
                    const float* expected = source.BoneMatrixes[action].Quaternion[k];
                    const float* actual = &rotation.values[k * 4];
                    const float dot = actual[0] * expected[0] + actual[1] * expected[1] +
                                      actual[2] * expected[2] + actual[3] * expected[3];
                    const float sign = dot >= 0.f ? 1.f : -1.f;
                    ++compared;
                    for (int c = 0; c < 4; ++c)
                    {
                        if (std::fabs(actual[c] - sign * expected[c]) > QuaternionRounding)
                        {
                            ++differing;
                            break;
                        }
                    }
                }
            }
        }
        CHECK(compared > 1000);
        CHECK(differing == 0);
    }
}

TEST_CASE("glb_model: the exported Bull Fighter and Bali equal their BMDs at every key and between keys")
{
    const vec3_t still = {0.f, 0.f, 0.f};
    const vec3_t turned = {14.f, -27.f, 0.f};
    for (const ExportedMonster& monster : {BullFighter, Bali})
    {
        CAPTURE(monster.glb);
        auto bmd = OpenMonsterBmd(monster);
        auto glb = OpenMonsterGlb(monster);
        PrepareMonsterLikeTheGame(*bmd, monster);
        PrepareMonsterLikeTheGame(*glb, monster);
        Difference largest;
        for (unsigned short action = 0; action < bmd->NumActions; ++action)
        {
            const Action_t& timeline = bmd->Actions[action];
            const float limit = FrameLimit(timeline);
            // Quarter frames hit every integer key exactly; the prior frame is
            // the previous quarter (MU's blend of the same or the next key) or
            // the frame itself (a held pose).
            for (float frame = 0.f; frame < limit; frame += 0.25f)
            {
                const float prior = frame >= 0.25f ? frame - 0.25f : frame;
                CheckPose(*bmd, *glb, {action, frame, action, prior}, still, largest);
                CheckPose(*bmd, *glb, {action, frame, action, prior}, turned, largest);
                CheckPose(*bmd, *glb, {action, frame, action, frame}, still, largest);
            }
            // Skipped keys sample between keys (frame - 1), at a fraction the GLB
            // takes from the clip time and the BMD from the frame: the two
            // round apart by a few 1e-7, which Bali's die action carries to
            // 1.2e-3 (measured; ten float steps).
            for (float frame = 2.6f; frame < limit; frame += 1.3f)
                CheckPose(*bmd, *glb, {action, frame, action, frame - 2.3f}, still, largest,
                          SkippedKeysTranslationTolerance);
        }
        MESSAGE(std::string(monster.glb) << ": largest difference: rotation " << largest.rotation << ", translation "
                            << largest.translation);
    }
}

// ---------------------------------------------------------------- CPU skinning

extern float BoneScale;

namespace
{
// Skins the model posed at `frame` of `action` into VertexTransform /
// NormalTransform, as a consumer gets them: Transform (deferred) and
// EnsureCpuVertices / EnsureCpuNormals.
void SkinOnCpu(BMD& model, BoneMatrices& bones, unsigned short action, float frame, bool translate)
{
    vec3_t angle = {0.f, 0.f, 35.f};
    vec3_t head = {0.f, 0.f, 0.f};
    model.Animation(bones.bones, frame, frame - 0.25f, action, angle, head, false, true, nullptr,
                    static_cast<short>(action));
    vec3_t boxMin = {-50.f, -50.f, 0.f};
    vec3_t boxMax = {50.f, 50.f, 150.f};
    OBB_t obb{};
    model.LightEnable = false;
    model.Transform(bones.bones, boxMin, boxMax, &obb, translate);
    model.EnsureCpuVertices();
    model.EnsureCpuNormals();
}

// A rigid transform's inverse applied to a point: R^T (p - t).
void InverseApply(const Matrix34& m, const float in[3], float out[3])
{
    const float d[3] = {in[0] - m[0][3], in[1] - m[1][3], in[2] - m[2][3]};
    for (int c = 0; c < 3; ++c)
        out[c] = m[0][c] * d[0] + m[1][c] * d[1] + m[2][c] * d[2];
}

void RotateBy(const float (*m)[4], const float in[3], float out[3])
{
    for (int r = 0; r < 3; ++r)
        out[r] = m[r][0] * in[0] + m[r][1] * in[1] + m[r][2] * in[2];
}

// Rewrites the first primitive's JOINTS_0 and WEIGHTS_0: every vertex on its
// own joint (weight 0.6) and on that joint's parent bone, or bone 0 for a
// root (weight 0.4).
void MakeTwoWeightSkin(Glb& glb, const BMD& bmd)
{
    auto& attributes = glb.json["meshes"][0]["primitives"][0]["attributes"];
    const auto& jointsAccessor = glb.json["accessors"][attributes["JOINTS_0"].get<int>()];
    REQUIRE(jointsAccessor["componentType"] == 5123);
    const auto& view = glb.json["bufferViews"][jointsAccessor["bufferView"].get<int>()];
    const size_t offset = view.value("byteOffset", 0) + jointsAccessor.value("byteOffset", 0);
    const size_t stride = view.value("byteStride", 8);
    const size_t count = jointsAccessor["count"];
    std::vector<std::uint16_t> joints(count * 4, 0);
    std::vector<float> weights(count * 4, 0.f);
    for (size_t v = 0; v < count; ++v)
    {
        std::uint16_t joint = 0;
        std::memcpy(&joint, &glb.bin[offset + v * stride], 2);
        const Bone_t& bone = bmd.Bones[joint];
        const std::uint16_t parent = !bone.Dummy && bone.Parent >= 0 ? static_cast<std::uint16_t>(bone.Parent) : 0;
        joints[v * 4] = joint;
        joints[v * 4 + 1] = parent == joint ? 2 : parent;
        weights[v * 4] = 0.6f;
        weights[v * 4 + 1] = 0.4f;
    }
    while (glb.bin.size() % 4 != 0)
        glb.bin.push_back(0);
    const size_t jointsOffset = glb.bin.size();
    glb.bin.resize(jointsOffset + joints.size() * 2);
    std::memcpy(&glb.bin[jointsOffset], joints.data(), joints.size() * 2);
    glb.json["bufferViews"].push_back({{"buffer", 0}, {"byteOffset", jointsOffset}, {"byteLength", joints.size() * 2}});
    glb.json["accessors"].push_back({{"bufferView", glb.json["bufferViews"].size() - 1},
                                     {"componentType", 5123},
                                     {"count", count},
                                     {"type", "VEC4"}});
    attributes["JOINTS_0"] = glb.json["accessors"].size() - 1;
    attributes["WEIGHTS_0"] = glb.AddAccessor(weights, "VEC4", count);
}
} // namespace

TEST_CASE("glb_model: CPU skinning of the exported Elite Yeti gives the BMD's vertices and normals")
{
    auto bmd = OpenYetiBmd();
    auto glb = OpenYetiGlb();
    PrepareLikeTheGame(*bmd);
    PrepareLikeTheGame(*glb);
    const Mesh_t& original = bmd->Meshs[0];
    const Mesh_t& mesh = glb->Meshs[0];
    auto bmdBones = std::make_unique<BoneMatrices>();
    auto glbBones = std::make_unique<BoneMatrices>();
    std::vector<float> expected(static_cast<size_t>(original.NumTriangles) * 3 * 6);

    for (const float boneScale : {1.f, 1.25f})
    {
        for (const bool translate : {false, true})
        {
            CAPTURE(boneScale);
            CAPTURE(translate);
            float largestPosition = 0.f;
            float largestNormal = 0.f;
            for (unsigned short action = 0; action < YetiActions; ++action)
            {
                BoneScale = boneScale;
                SkinOnCpu(*bmd, *bmdBones, action, 2.4f, translate);
                // The global scratch belongs to the last model skinned: copy the BMD's first.
                for (int t = 0; t < original.NumTriangles; ++t)
                {
                    for (int c = 0; c < 3; ++c)
                    {
                        float* out = &expected[(static_cast<size_t>(t) * 3 + c) * 6];
                        VectorCopy(VertexTransform[0][original.Triangles[t].VertexIndex[c]], out);
                        VectorCopy(NormalTransform[0][original.Triangles[t].NormalIndex[c]], out + 3);
                    }
                }
                SkinOnCpu(*glb, *glbBones, action, 2.4f, translate);
                BoneScale = 1.f;
                for (int t = 0; t < mesh.NumTriangles; ++t)
                {
                    for (int c = 0; c < 3; ++c)
                    {
                        const float* want = &expected[(static_cast<size_t>(t) * 3 + c) * 6];
                        const float* position = VertexTransform[0][mesh.Triangles[t].VertexIndex[c]];
                        const float* normal = NormalTransform[0][mesh.Triangles[t].NormalIndex[c]];
                        for (int k = 0; k < 3; ++k)
                        {
                            largestPosition = std::max(largestPosition, std::fabs(position[k] - want[k]));
                            largestNormal = std::max(largestNormal, std::fabs(normal[k] - want[3 + k]));
                        }
                    }
                }
            }
            MESSAGE("largest difference: position " << largestPosition << ", normal " << largestNormal);
            // Same tolerance as the bone matrices (vertices sit on them).
            CHECK(largestPosition < TranslationTolerance);
            CHECK(largestNormal < 1e-4f);
        }
    }
}

TEST_CASE("glb_model: CPU skinning blends up to four joints by their weights")
{
    auto bmd = OpenYetiBmd();
    Glb glb = Glb::Read(YetiGlb);
    MakeTwoWeightSkin(glb, *bmd);
    TemporaryFile file;
    glb.Write(file.Path());
    auto model = OpenYetiGlb(file.Path());
    PrepareLikeTheGame(*model);
    const auto* skinned = SkinnedModelOf(*model);
    const std::vector<Matrix34> rest = skinned->RestPose();
    const Mesh_t& mesh = model->Meshs[0];
    auto bones = std::make_unique<BoneMatrices>();

    for (const float boneScale : {1.f, 1.25f})
    {
        CAPTURE(boneScale);
        BoneScale = boneScale;
        SkinOnCpu(*model, *bones, YetiAttack1, 3.7f, false);
        BoneScale = 1.f;
        float largestPosition = 0.f;
        float largestNormal = 0.f;
        for (int v = 0; v < mesh.NumVertices; ++v)
        {
            const auto& vertex = skinned->meshVertices[0][v];
            REQUIRE(vertex.count == 2);
            CHECK(vertex.weights[0] == doctest::Approx(0.6f));
            // The BMD vertex follows the dominant joint.
            CHECK(mesh.Vertices[v].Node == vertex.joints[0]);
            float modelPosition[3];
            float modelNormal[3];
            Apply(rest[vertex.joints[0]], mesh.Vertices[v].Position, modelPosition);
            RotateBy(reinterpret_cast<const float (*)[4]>(rest[vertex.joints[0]].data()), mesh.Normals[v].Normal,
                     modelNormal);
            float wantPosition[3] = {0.f, 0.f, 0.f};
            float wantNormal[3] = {0.f, 0.f, 0.f};
            for (int k = 0; k < 2; ++k)
            {
                const float (*matrix)[4] = bones->bones[vertex.joints[k]];
                float local[3];
                InverseApply(rest[vertex.joints[k]], modelPosition, local);
                float turned[3];
                RotateBy(matrix, local, turned);
                float localNormal[3];
                const Matrix34& r = rest[vertex.joints[k]];
                for (int c = 0; c < 3; ++c)
                    localNormal[c] = r[0][c] * modelNormal[0] + r[1][c] * modelNormal[1] + r[2][c] * modelNormal[2];
                float turnedNormal[3];
                RotateBy(matrix, localNormal, turnedNormal);
                for (int c = 0; c < 3; ++c)
                {
                    wantPosition[c] += vertex.weights[k] * (turned[c] * boneScale + matrix[c][3]);
                    wantNormal[c] += vertex.weights[k] * turnedNormal[c];
                }
            }
            const float length =
                std::sqrt(wantNormal[0] * wantNormal[0] + wantNormal[1] * wantNormal[1] + wantNormal[2] * wantNormal[2]);
            for (int c = 0; c < 3; ++c)
            {
                largestPosition = std::max(largestPosition, std::fabs(VertexTransform[0][v][c] - wantPosition[c]));
                largestNormal = std::max(largestNormal, std::fabs(NormalTransform[0][v][c] - wantNormal[c] / length));
            }
        }
        MESSAGE("largest difference: position " << largestPosition << ", normal " << largestNormal);
        CHECK(largestPosition < TranslationTolerance);
        CHECK(largestNormal < 1e-4f);
    }
}

namespace
{
// skinned_textured.vert's weighted branch on the CPU: the palette rows of each
// joint applied to the bind position, weighted by the unorm weights.
void ShadeWeightedPosition(const mu::SkinnedWeightedVertex3D& vertex, const std::vector<float>& palette, float out[3])
{
    out[0] = out[1] = out[2] = 0.f;
    for (int k = 0; k < 4; ++k)
    {
        const float weight = static_cast<float>(vertex.weights[k]) / 255.f;
        if (weight <= 0.f)
            continue;
        const float* rows = &palette[static_cast<size_t>(vertex.joints[k]) * 12];
        for (size_t r = 0; r < 3; ++r)
            out[r] += weight * (rows[r * 4] * vertex.x + rows[r * 4 + 1] * vertex.y + rows[r * 4 + 2] * vertex.z +
                                rows[r * 4 + 3]);
    }
}

void ShadeWeightedNormal(const mu::SkinnedWeightedVertex3D& vertex, const std::vector<float>& palette, float boneScale,
                         float restPoseScale, float out[3])
{
    const float restScale = restPoseScale != 0.f ? restPoseScale : 1.f;
    const float skinScale = boneScale == 1.f ? restScale : boneScale;
    out[0] = out[1] = out[2] = 0.f;
    for (int k = 0; k < 4; ++k)
    {
        const float weight = static_cast<float>(vertex.weights[k]) / 255.f;
        if (weight <= 0.f)
            continue;
        const float* rows = &palette[static_cast<size_t>(vertex.joints[k]) * 12];
        for (size_t r = 0; r < 3; ++r)
            out[r] += weight * (rows[r * 4] * vertex.nx + rows[r * 4 + 1] * vertex.ny + rows[r * 4 + 2] * vertex.nz);
    }
    for (int r = 0; r < 3; ++r)
        out[r] /= skinScale;
    if (vertex.weights[1] == 0)
        return;
    const float length = std::sqrt(out[0] * out[0] + out[1] * out[1] + out[2] * out[2]);
    for (int r = 0; r < 3; ++r)
        out[r] /= length;
}

// The largest difference between the GPU arithmetic on packed vertices and the
// CPU skin (SkinnedModel::SkinPosition / SkinNormal) of mesh 0.
Difference GpuAgainstCpuSkin(const BMD& model, const BoneMatrices& bones, float boneScale, float restPoseScale)
{
    const auto* skinned = SkinnedModelOf(model);
    std::vector<float> palette;
    Render::Models::BuildWeightedSkinPalette(*skinned, bones.bones, boneScale, restPoseScale, palette);
    REQUIRE(palette.size() == skinned->joints.size() * 12);
    Difference largest;
    for (int v = 0; v < model.Meshs[0].NumVertices; ++v)
    {
        const auto& source = skinned->meshVertices[0][v];
        const mu::SkinnedWeightedVertex3D packed = Render::Models::PackWeightedSkinnedVertex(source, 0.f, 0.f);
        float gpu[3];
        float cpu[3];
        ShadeWeightedPosition(packed, palette, gpu);
        skinned->SkinPosition(0, v, bones.bones, boneScale, restPoseScale, cpu);
        float gpuNormal[3];
        float cpuNormal[3];
        ShadeWeightedNormal(packed, palette, boneScale, restPoseScale, gpuNormal);
        skinned->SkinNormal(0, v, bones.bones, cpuNormal);
        for (int c = 0; c < 3; ++c)
        {
            largest.translation = std::max(largest.translation, std::fabs(gpu[c] - cpu[c]));
            largest.rotation = std::max(largest.rotation, std::fabs(gpuNormal[c] - cpuNormal[c]));
        }
    }
    return largest;
}

std::vector<std::uint8_t> ReadBytes(const std::filesystem::path& file)
{
    std::ifstream in(file, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

// A developer overlay (MU_DATA_OVERLAY) whose Data/ has a model file table
// naming `glb` for the Elite Yeti, and an empty game Data/.
class YetiOverlay
{
public:
    explicit YetiOverlay(const std::vector<std::uint8_t>& glb)
    {
        static std::atomic<int> counter{0};
        root_ = std::filesystem::temp_directory_path() /
                ("glb_model_overlay_" + std::to_string(std::random_device{}()) + "_" + std::to_string(counter++));
        std::filesystem::create_directories(root_ / "overlay" / "Data" / "Models");
        std::filesystem::create_directories(root_ / "overlay" / "Data" / "Monster");
        std::filesystem::create_directories(root_ / "game");
        std::ofstream(root_ / "overlay" / "Data" / "Models" / "ModelFiles.json")
            << R"({ "formatVersion": 1, "models": [ { "model": "MODEL_ELITE_YETI", "file": "Monster/Monster14.glb" } ] })";
        std::ofstream(root_ / "overlay" / "Data" / "Monster" / "Monster14.glb", std::ios::binary)
            .write(reinterpret_cast<const char*>(glb.data()), static_cast<std::streamsize>(glb.size()));
    }
    ~YetiOverlay()
    {
        std::error_code error;
        std::filesystem::remove_all(root_, error);
    }
    YetiOverlay(const YetiOverlay&) = delete;
    YetiOverlay& operator=(const YetiOverlay&) = delete;

    Data::Models::ModelFiles Table() const
    {
        const std::string overlay = (root_ / "overlay").string();
        return Data::Models::ReadModelFileTable(overlay.c_str(), root_ / "game");
    }

private:
    std::filesystem::path root_;
};
} // namespace

TEST_CASE("glb_model: a BMD vertex packs as before, one bone and no weights")
{
    const float position[3] = {1.f, 2.f, 3.f};
    const float normal[3] = {0.f, 0.f, 1.f};
    const mu::SkinnedVertex3D packed = Render::Models::PackBmdSkinnedVertex(position, 7, normal, 9, 0.25f, 0.75f);
    CHECK(packed.x == 1.f);
    CHECK(packed.y == 2.f);
    CHECK(packed.z == 3.f);
    CHECK(packed.nz == 1.f);
    CHECK(packed.u == 0.25f);
    CHECK(packed.v == 0.75f);
    CHECK(packed.positionBoneIndex == 7);
    CHECK(packed.normalBoneIndex == 9);
}

TEST_CASE("glb_model: a weighted vertex packs its bind position and four joints with weights summing to 255")
{
    Render::Models::Gltf::SkinnedVertex vertex;
    vertex.count = 3;
    vertex.joints = {4, 11, 2, 0};
    vertex.weights = {0.2f, 0.5f, 0.3f, 0.f};
    vertex.bindPosition = {5.f, 6.f, 7.f};
    vertex.bindNormal = {0.f, 1.f, 0.f};
    const mu::SkinnedWeightedVertex3D packed = Render::Models::PackWeightedSkinnedVertex(vertex, 0.5f, 0.125f);
    CHECK(packed.x == 5.f);
    CHECK(packed.z == 7.f);
    CHECK(packed.ny == 1.f);
    CHECK(packed.u == 0.5f);
    // The largest weight first, its joint also in the single-bone fields.
    CHECK(packed.joints[0] == 11);
    CHECK(packed.joints[1] == 2);
    CHECK(packed.joints[2] == 4);
    CHECK(packed.positionBoneIndex == 11);
    CHECK(packed.normalBoneIndex == 11);
    CHECK(packed.weights[1] == 77);
    CHECK(packed.weights[2] == 51);
    CHECK(packed.weights[3] == 0);
    CHECK(packed.weights[0] + packed.weights[1] + packed.weights[2] + packed.weights[3] == 255);
}

TEST_CASE("glb_model: GPU arithmetic on packed vertices gives the CPU skin of the exported Elite Yeti")
{
    auto model = OpenYetiGlb();
    PrepareLikeTheGame(*model);
    auto bones = std::make_unique<BoneMatrices>();
    for (const float boneScale : {1.f, 1.25f})
    {
        for (const float restPoseScale : {0.f, 1.4f})
        {
            CAPTURE(boneScale);
            CAPTURE(restPoseScale);
            SkinOnCpu(*model, *bones, YetiAttack1, 2.4f, false);
            const Difference difference = GpuAgainstCpuSkin(*model, *bones, boneScale, restPoseScale);
            CHECK(difference.translation < TranslationTolerance);
            CHECK(difference.rotation < 1e-4f);
        }
    }
}

TEST_CASE("glb_model: GPU arithmetic on packed vertices blends four-weight skins like the CPU")
{
    auto bmd = OpenYetiBmd();
    Glb glb = Glb::Read(YetiGlb);
    MakeTwoWeightSkin(glb, *bmd);
    TemporaryFile file;
    glb.Write(file.Path());
    auto model = OpenYetiGlb(file.Path());
    PrepareLikeTheGame(*model);
    auto bones = std::make_unique<BoneMatrices>();
    SkinOnCpu(*model, *bones, YetiWalk, 3.7f, false);
    for (const float boneScale : {1.f, 1.25f})
    {
        CAPTURE(boneScale);
        const Difference difference = GpuAgainstCpuSkin(*model, *bones, boneScale, 0.f);
        CHECK(difference.translation < TranslationTolerance);
        CHECK(difference.rotation < 1e-4f);
    }
}

TEST_CASE("glb_model: the overlay's model file table puts its GLB into the opened BMD")
{
    const YetiOverlay overlay(ReadBytes(YetiGlb));
    const Data::Models::ModelFiles table = overlay.Table();
    INFO(table.issues.size());
    CHECK_FALSE(Data::Items::HasFatal(table.issues));
    REQUIRE(table.models.size() == 1);
    CHECK(Data::Models::ModelFileTextureName(table.models[0]) == L"Data\\Monster\\Monster14.glb");

    auto yeti = OpenYetiBmd();
    CHECK(Data::Models::ApplyModelFile(*yeti, MODEL_ELITE_YETI, table));
    CHECK(yeti->GltfModel != nullptr);

    auto other = OpenYetiBmd();
    CHECK_FALSE(Data::Models::ApplyModelFile(*other, MODEL_YETI, table));
    CHECK(other->GltfModel == nullptr);
}

TEST_CASE("glb_model: a model file the loader refuses leaves the BMD in place")
{
    std::vector<std::uint8_t> broken = ReadBytes(YetiGlb);
    broken.resize(broken.size() / 2);
    const YetiOverlay overlay(broken);
    const Data::Models::ModelFiles table = overlay.Table();
    REQUIRE(table.models.size() == 1);

    auto yeti = OpenYetiBmd();
    const short meshes = yeti->NumMeshs;
    const short vertices = yeti->Meshs[0].NumVertices;
    CHECK_FALSE(Data::Models::ApplyModelFile(*yeti, MODEL_ELITE_YETI, table));
    CHECK(yeti->GltfModel == nullptr);
    CHECK(yeti->NumBones == YetiBones);
    CHECK(yeti->NumMeshs == meshes);
    CHECK(yeti->Meshs[0].NumVertices == vertices);
}

TEST_CASE("glb_model: the image a model file carries is the mesh's texture and decodes to RGBA")
{
    auto model = OpenYetiGlb();
    const auto* skinned = SkinnedModelOf(*model);
    REQUIRE(skinned->images.size() == 1);
    REQUIRE(skinned->meshImages.size() == static_cast<size_t>(model->NumMeshs));
    CHECK(skinned->meshImages[0] == 0);
    CHECK(skinned->images[0].name == "snowman02.jpg");
    CHECK(std::string(model->Textures[0].FileName) == "snowman02.jpg");

    Render::Models::Gltf::DecodedImage decoded;
    std::string reason;
    const bool decodedOk = Render::Models::Gltf::DecodeEmbeddedImage(skinned->images[0], decoded, reason);
    INFO(reason);
    REQUIRE(decodedOk);
    CHECK(decoded.width == 4);
    CHECK(decoded.height == 4);
    CHECK_FALSE(decoded.hasAlpha);
    CHECK(decoded.rgba.size() == 4u * 4u * 4u);

    Render::Models::Gltf::EmbeddedImage jpeg = skinned->images[0];
    jpeg.mimeType = "image/jpeg";
    CHECK_FALSE(Render::Models::Gltf::DecodeEmbeddedImage(jpeg, decoded, reason));
    CHECK(reason.find("image/jpeg") != std::string::npos);

    Render::Models::Gltf::EmbeddedImage garbage = skinned->images[0];
    garbage.bytes.resize(8);
    CHECK_FALSE(Render::Models::Gltf::DecodeEmbeddedImage(garbage, decoded, reason));
    CHECK(reason.find("PNG") != std::string::npos);
}

namespace
{
// An embedded PNG image of `width` x `height` opaque texels.
Render::Models::Gltf::EmbeddedImage EmbeddedPng(int width, int height)
{
    SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGB24);
    REQUIRE(surface != nullptr);
    const auto path = std::filesystem::temp_directory_path() /
                      ("glb_model_png_" + std::to_string(width) + "x" + std::to_string(height) + ".png");
    const bool saved = SDL_SavePNG(surface, path.string().c_str());
    SDL_DestroySurface(surface);
    REQUIRE(saved);
    Render::Models::Gltf::EmbeddedImage image;
    image.name = path.filename().string();
    image.mimeType = "image/png";
    image.bytes = ReadBytes(path);
    std::filesystem::remove(path);
    return image;
}
} // namespace

TEST_CASE("glb_model: an embedded image up to 2048x2048 decodes, one texel above is refused with its size")
{
    Render::Models::Gltf::DecodedImage decoded;
    std::string reason;
    const bool atLimit = Render::Models::Gltf::DecodeEmbeddedImage(EmbeddedPng(2048, 2048), decoded, reason);
    INFO(reason);
    REQUIRE(atLimit);
    CHECK(decoded.width == 2048);
    CHECK(decoded.height == 2048);
    CHECK(decoded.rgba.size() == std::size_t{2048} * 2048u * 4u);

    for (const auto [width, height] : {std::pair{2049, 2048}, std::pair{2048, 2049}})
    {
        const std::string size = std::to_string(width) + "x" + std::to_string(height);
        CAPTURE(size);
        reason.clear();
        CHECK_FALSE(Render::Models::Gltf::DecodeEmbeddedImage(EmbeddedPng(width, height), decoded, reason));
        CHECK(reason == "it is " + size + ", above the 2048x2048 texture limit");
    }
}

// ---------------------------------------------------------------- sockets by name

namespace
{
// The lines the "render" logger writes while it lives.
class RenderLogCapture
{
public:
    RenderLogCapture() : logger_(mu::log::Get("render")), sink_(std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(64))
    {
        sink_->set_pattern("%v");
        logger_->sinks().push_back(sink_);
    }
    ~RenderLogCapture()
    {
        auto& sinks = logger_->sinks();
        sinks.erase(std::remove(sinks.begin(), sinks.end(), sink_), sinks.end());
    }
    RenderLogCapture(const RenderLogCapture&) = delete;
    RenderLogCapture& operator=(const RenderLogCapture&) = delete;

    // The captured lines that contain `text`.
    int Count(const std::string& text) const
    {
        int count = 0;
        for (const std::string& line : sink_->last_formatted())
            count += line.find(text) != std::string::npos ? 1 : 0;
        return count;
    }

private:
    std::shared_ptr<spdlog::logger> logger_;
    std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> sink_;
};
} // namespace

TEST_CASE("glb_model: FindBone resolves the Elite Yeti's head and breath bones of the BMD")
{
    const auto bmd = OpenYetiBmd();
    // The fixed indices OpenMonsterModel and the breath smoke used before (head 20, Box03 22).
    CHECK(bmd->FindBone("Bip01 Head") == 20);
    CHECK(bmd->FindBone("Box03") == 22);
    CHECK(bmd->FindBone("Bip01") == 0);
    CHECK(bmd->FindBone("no such bone") == -1);
    // Cached: the same answer again.
    CHECK(bmd->FindBone("Box03") == 22);
}

TEST_CASE("glb_model: FindBone gives the exported Elite Yeti the BMD's indices, extra joints included")
{
    const auto bmd = OpenYetiBmd();
    const auto glb = OpenYetiGlb();
    for (const char* name : {"Bip01 Head", "Box03", "Bip01", "Bip01 R Hand", "ewr"})
    {
        CAPTURE(name);
        CHECK(glb->FindBone(name) == bmd->FindBone(name));
        CHECK(glb->FindBone(name) >= 0);
    }

    // A file with extra joints appended keeps the BMD's indices and finds the extra ones after them.
    Glb file = Glb::Read(YetiGlb);
    const int tail = file.AddNode("Tail1", file.NodeNamed("Bip01 Spine"));
    file.AddSkinJoint(tail);
    TemporaryFile extra;
    file.Write(extra.Path());
    const auto withTail = OpenYetiGlb(extra.Path());
    CHECK(withTail->FindBone("Bip01 Head") == 20);
    CHECK(withTail->FindBone("Box03") == 22);
    CHECK(withTail->FindBone("Tail1") == YetiBones);
}

TEST_CASE("glb_model: a model without Box03 falls back to the root bone and logs it once")
{
    // A GLB without one of the BMD's bones is refused, so the bone table that lacks
    // Box03 is made in memory.
    const auto model = OpenYetiGlb();
    std::snprintf(model->Bones[22].Name, sizeof(model->Bones[22].Name), "%s", "Box03 renamed");
    RenderLogCapture log;
    CHECK(model->FindBone("Box03") == -1);
    CHECK(model->FindBoneOrRoot("Box03") == 0);
    CHECK(model->FindBoneOrRoot("Box03") == 0);
    CHECK(model->FindBoneOrRoot("Bip01 Head") == 20);
    CHECK(log.Count("has no bone \"Box03\"") == 1);
    CHECK(log.Count("Bip01 Head") == 0);
}

TEST_CASE("glb_model: FindBone forgets its answers when the bones change")
{
    auto model = OpenYetiBmd();
    REQUIRE(model->FindBone("Box03") == 22);
    // Loading a file replaces the bones; the extra joint is then found.
    Glb file = Glb::Read(YetiGlb);
    file.AddSkinJoint(file.AddNode("Tail1", file.NodeNamed("Bip01 Spine")));
    TemporaryFile extra;
    file.Write(extra.Path());
    REQUIRE(model->FindBone("Tail1") == -1);
    std::string reason;
    REQUIRE(LoadGlbIntoBmd(*model, extra.Path(), NoActionClips, reason));
    CHECK(model->FindBone("Tail1") == YetiBones);
    CHECK(model->FindBone("Box03") == 22);
}

// ---------------------------------------------------------------- action table and crossfade

extern float FPS_ANIMATION_FACTOR;

namespace
{
using Render::Models::AnimationSampling::ActionTimeline;

// Adds a LINEAR clip `name` with one translation channel on `node`: key i at
// time i / (keys - 1) x cycle, translation (i, 0, 0).
void AddCountingClip(Glb& glb, const std::string& name, int node, int keys, float cycle)
{
    std::vector<float> times(keys);
    std::vector<float> values(static_cast<size_t>(keys) * 3, 0.f);
    for (int i = 0; i < keys; ++i)
    {
        times[i] = cycle * static_cast<float>(i) / static_cast<float>(keys - 1);
        values[static_cast<size_t>(i) * 3] = static_cast<float>(i);
    }
    const int input = glb.AddAccessor(times, "SCALAR", keys);
    glb.json["accessors"][input]["min"] = {0.0};
    glb.json["accessors"][input]["max"] = {cycle};
    const int output = glb.AddAccessor(values, "VEC3", keys);
    glb.json["animations"].push_back(
        {{"name", name},
         {"samplers", {{{"input", input}, {"output", output}, {"interpolation", "LINEAR"}}}},
         {"channels", {{{"sampler", 0}, {"target", {{"node", node}, {"path", "translation"}}}}}}});
}

int ClipNamed(const Render::Models::Gltf::SkinnedModel& model, const std::string& name)
{
    for (size_t i = 0; i < model.clips.size(); ++i)
    {
        if (model.clips[i].name == name)
            return static_cast<int>(i);
    }
    return -1;
}

std::unique_ptr<BMD> OpenYetiGlbWith(const std::filesystem::path& file,
                                     const std::map<int, Data::Models::ActionClip>& actions)
{
    auto model = OpenYetiBmd();
    std::string reason;
    const bool loaded = LoadGlbIntoBmd(*model, file, actions, reason);
    INFO(reason);
    REQUIRE(loaded);
    return model;
}

// One tick of the game's animation step (PlayAnimation at FPS_ANIMATION_FACTOR 1).
struct Playback
{
    float frame = 0.f;
    float priorFrame = 0.f;
    unsigned short priorAction = 0;

    void Tick(BMD& model, unsigned short action)
    {
        vec3_t origin = {0.f, 0.f, 0.f};
        vec3_t angle = {0.f, 0.f, 0.f};
        model.CurrentAction = action;
        model.PlayAnimation(&frame, &priorFrame, &priorAction, model.Actions[action].PlaySpeed, origin, angle);
    }
};
} // namespace

TEST_CASE("glb_model: crossfade length helpers")
{
    namespace Sampling = Render::Models::AnimationSampling;
    using Sampling::CrossfadeKeyTime;
    using Sampling::SetCrossfadeLength;
    using Sampling::CrossfadeContinues;
    using Sampling::PosePlan;
    const ActionTimeline walk{7, false, 0.28f, false};
    CHECK(CrossfadeKeyTime(walk, 4.f) == doctest::Approx(1.12f));
    CHECK(CrossfadeKeyTime(walk, 0.f) == 0.f);
    CHECK(CrossfadeKeyTime(walk, -2.f) == 0.f);
    // At most one step of the play speed before the frame the action repeats on, or the key a held one stops on.
    CHECK(CrossfadeKeyTime(walk, 1000.f) == doctest::Approx(7.f - 0.28f));
    CHECK(CrossfadeKeyTime({7, true, 0.28f, false}, 1000.f) == doctest::Approx(6.f - 0.28f));
    CHECK(CrossfadeKeyTime({16, false, 0.25f, true}, 1000.f) == 15.f);

    PosePlan plan;
    plan.kind = Sampling::PoseCase::Crossfade;
    SetCrossfadeLength(plan, 0.56f, 1.12f);
    CHECK(plan.weight == doctest::Approx(0.5f));
    SetCrossfadeLength(plan, 3.f, 1.12f);
    CHECK(plan.weight == 1.f);
    SetCrossfadeLength(plan, 0.f, 0.f);
    CHECK(plan.weight == 1.f);
    PosePlan mu;
    mu.weight = 0.3f;
    SetCrossfadeLength(mu, 0.56f, 1.12f);
    CHECK(mu.weight == 0.3f);
    CHECK(CrossfadeContinues(1.f, 1.12f));
    CHECK_FALSE(CrossfadeContinues(1.12f, 1.12f));
}

TEST_CASE("glb_model: the action table maps an action to a named clip with its crossfade")
{
    Glb glb = Glb::Read(YetiGlb);
    AddCountingClip(glb, "walk30", glb.NodeNamed("Box03"), 30, 2.f);
    TemporaryFile file;
    glb.Write(file.Path());
    const auto model = OpenYetiGlbWith(file.Path(), {{YetiWalk, {"walk30", 4.f}}, {YetiStop1, {"", 2.5f}}});
    const auto* skinned = SkinnedModelOf(*model);
    REQUIRE(skinned != nullptr);
    CHECK(skinned->actionClips[YetiWalk] == ClipNamed(*skinned, "walk30"));
    CHECK(skinned->actionCrossfades[YetiWalk] == 4.f);
    // An entry without a clip keeps the default clip and sets only the crossfade.
    CHECK(skinned->actionClips[YetiStop1] == ClipNamed(*skinned, "action00"));
    CHECK(skinned->actionCrossfades[YetiStop1] == 2.5f);
    // Actions without an entry: default clip, the playback rule's fade.
    CHECK(skinned->actionClips[YetiAttack1] == ClipNamed(*skinned, "action03"));
    CHECK_FALSE(skinned->actionCrossfades[YetiAttack1].has_value());
}

TEST_CASE("glb_model: a set crossfade runs over its ticks from the pose on screen")
{
    FPS_ANIMATION_FACTOR = 1.f;
    constexpr float FadeTicks = 4.f;
    auto bmd = OpenYetiBmd();
    auto glb = OpenYetiGlbWith(YetiGlb, {{YetiWalk, {"", FadeTicks}}});
    PrepareLikeTheGame(*bmd);
    PrepareLikeTheGame(*glb);
    const vec3_t still = {0.f, 0.f, 0.f};
    Difference largest;

    // On screen: attack 1 at frame 3.3 (prior key 3). SetAction switches to walk.
    const PoseCase onScreen{YetiAttack1, 3.3f, YetiAttack1, 3.f};
    Playback playback{0.f, onScreen.frame, YetiAttack1};
    const float walkSpeed = glb->Actions[YetiWalk].PlaySpeed;
    // Tick 0: the pose on screen.
    CheckPose(*bmd, *glb, {YetiWalk, 0.f, YetiAttack1, onScreen.frame}, still, largest);
    for (int tick = 1; tick <= 3; ++tick)
    {
        CAPTURE(tick);
        playback.Tick(*glb, YetiWalk);
        CHECK(playback.frame == doctest::Approx(walkSpeed * static_cast<float>(tick)));
        // The fade keeps the prior action and its frame until it has run, also past frame 1.
        CHECK(playback.priorAction == YetiAttack1);
        CHECK(playback.priorFrame == onScreen.frame);
        // Weight tick / 4. Below frame 1 the playback rule's fade has weight `frame` towards the
        // same target (walk at frame 0), so the BMD at frame tick / 4 is the expected pose.
        const float weight = static_cast<float>(tick) / FadeTicks;
        const auto expected = std::make_unique<BoneMatrices>();
        const auto actual = std::make_unique<BoneMatrices>();
        vec3_t angle = {0.f, 0.f, 35.f};
        vec3_t head = {0.f, 0.f, 0.f};
        bmd->Animation(expected->bones, weight, playback.priorFrame, playback.priorAction, angle, head, false, true,
                       nullptr, YetiWalk);
        glb->Animation(actual->bones, playback.frame, playback.priorFrame, playback.priorAction, angle, head, false,
                       true, nullptr, YetiWalk);
        float difference = 0.f;
        for (int i = 0; i < YetiBones; ++i)
            for (int r = 0; r < 3; ++r)
                for (int c = 0; c < 4; ++c)
                    difference = std::max(difference, std::fabs(expected->bones[i][r][c] - actual->bones[i][r][c]));
        CHECK(difference < TranslationTolerance);
    }
    // Tick 4: frame 1.12 reaches the fade's 4 x 0.28 key time; the walk plays on its own.
    playback.Tick(*glb, YetiWalk);
    CHECK(playback.priorAction == YetiWalk);
    CHECK(playback.priorFrame == 0.f);
    CheckPose(*bmd, *glb, {YetiWalk, playback.frame, playback.priorAction, playback.priorFrame}, still, largest);

    // Without a set length the playback rule's fade ends at frame 1, as before.
    auto plain = OpenYetiGlb();
    PrepareLikeTheGame(*plain);
    Playback rule{0.f, onScreen.frame, YetiAttack1};
    for (int tick = 1; tick <= 3; ++tick)
        rule.Tick(*plain, YetiWalk);
    CHECK(rule.priorAction == YetiAttack1);
    rule.Tick(*plain, YetiWalk);
    CHECK(rule.priorAction == YetiWalk);
}

TEST_CASE("glb_model: a set crossfade of at least the action's length ends before the wrap, without a jump")
{
    FPS_ANIMATION_FACTOR = 1.f;
    // From attack 1 into attack 2 (7 keys, repeating) or into the walk (7 keys, locked).
    unsigned short action = YetiWalk;
    SUBCASE("a repeating action")
    {
        action = YetiAttack2;
    }
    SUBCASE("a locked action")
    {
        action = YetiWalk;
    }
    auto bmd = OpenYetiBmd();
    auto glb = OpenYetiGlbWith(YetiGlb, {{action, {"", 1000.f}}});
    PrepareLikeTheGame(*bmd);
    PrepareLikeTheGame(*glb);
    const Action_t& played = glb->Actions[action];
    REQUIRE(played.LockPositions == (action == YetiWalk));
    const ActionTimeline timeline{played.NumAnimationKeys, played.LockPositions, played.PlaySpeed, played.Loop};
    const float wrap = static_cast<float>(Render::Models::AnimationSampling::KeyIntervals(timeline));
    const float fadeEnd = Render::Models::AnimationSampling::CrossfadeKeyTime(timeline, 1000.f);
    CAPTURE(wrap);
    REQUIRE(fadeEnd < wrap);
    REQUIRE(1000.f * played.PlaySpeed >= wrap);

    const vec3_t still = {0.f, 0.f, 0.f};
    Difference largest;
    Playback playback{0.f, 3.3f, YetiAttack1};
    bool wrapped = false;
    bool fadeRan = false;
    for (int tick = 1; tick < 100 && !wrapped; ++tick)
    {
        CAPTURE(tick);
        const float before = playback.frame;
        playback.Tick(*glb, action);
        wrapped = playback.frame < before;
        CAPTURE(playback.frame);
        if (!wrapped && playback.frame < fadeEnd)
        {
            // The fade runs and keeps the prior action.
            CHECK(playback.priorAction == YetiAttack1);
            continue;
        }
        if (!wrapped)
        {
            // The fade has run: the action alone, as MU shows it on this frame (key floor(frame) - 1 into
            // key floor(frame)), whatever the prior action still says.
            fadeRan = true;
            CheckPose(*bmd, *glb,
                      {action, playback.frame, action, static_cast<float>(static_cast<int>(playback.frame) - 1)},
                      still, largest);
            const auto onScreen = std::make_unique<BoneMatrices>();
            const auto actionAlone = std::make_unique<BoneMatrices>();
            vec3_t angle = {0.f, 0.f, 35.f};
            vec3_t head = {0.f, 0.f, 0.f};
            glb->Animation(onScreen->bones, playback.frame, playback.priorFrame, playback.priorAction, angle, head,
                           false, true, nullptr, action);
            glb->Animation(actionAlone->bones, playback.frame,
                           static_cast<float>(static_cast<int>(playback.frame) - 1), action, angle, head, false,
                           true, nullptr, action);
            float difference = 0.f;
            for (int i = 0; i < YetiBones; ++i)
                for (int r = 0; r < 3; ++r)
                    for (int c = 0; c < 4; ++c)
                        difference = std::max(difference, std::fabs(onScreen->bones[i][r][c] - actionAlone->bones[i][r][c]));
            CHECK(difference < TranslationTolerance);
            continue;
        }
        // The wrap: the fade is over, the prior action is the current one and the pose MU's.
        CHECK(playback.priorAction == action);
        CHECK(playback.priorFrame == static_cast<float>(static_cast<int>(wrap) - 1));
        CheckPose(*bmd, *glb, {action, playback.frame, playback.priorAction, playback.priorFrame}, still, largest);
    }
    CHECK(wrapped);
    CHECK(fadeRan);
}

TEST_CASE("glb_model: die holds its last key")
{
    FPS_ANIMATION_FACTOR = 1.f;
    auto bmd = OpenYetiBmd();
    auto glb = OpenYetiGlbWith(YetiGlb, {{YetiDie, {"", 3.f}}});
    PrepareLikeTheGame(*bmd);
    PrepareLikeTheGame(*glb);
    const int lastKey = glb->Actions[YetiDie].NumAnimationKeys - 1;
    Playback playback{0.f, 2.4f, YetiWalk};
    for (int tick = 0; tick < 200; ++tick)
        playback.Tick(*glb, YetiDie);
    CHECK(playback.frame == doctest::Approx(static_cast<float>(lastKey + 1) - 0.01f));
    const vec3_t still = {0.f, 0.f, 0.f};
    Difference largest;
    CheckPose(*bmd, *glb, {YetiDie, playback.frame, playback.priorAction, playback.priorFrame}, still, largest);
    // The pose is the last key itself and stays it.
    const auto first = std::make_unique<BoneMatrices>();
    const auto later = std::make_unique<BoneMatrices>();
    vec3_t angle = {0.f, 0.f, 0.f};
    vec3_t head = {0.f, 0.f, 0.f};
    glb->BodyScale = 1.f;
    Vector(0.f, 0.f, 0.f, glb->BodyOrigin);
    glb->Animation(first->bones, playback.frame, playback.priorFrame, playback.priorAction, angle, head, false, false,
                   nullptr, YetiDie);
    for (int tick = 0; tick < 50; ++tick)
        playback.Tick(*glb, YetiDie);
    glb->Animation(later->bones, playback.frame, playback.priorFrame, playback.priorAction, angle, head, false, false,
                   nullptr, YetiDie);
    const std::vector<Matrix34> keyPose = BmdKeyPose(*bmd, YetiDie, lastKey);
    for (int i = 0; i < YetiBones; ++i)
    {
        CAPTURE(i);
        Matrix34 m{};
        std::memcpy(m.data(), first->bones[i], sizeof(first->bones[i]));
        CHECK(MaxDifference(m, keyPose[i]) < TranslationTolerance);
        CHECK(std::memcmp(first->bones[i], later->bones[i], sizeof(first->bones[i])) == 0);
    }
}

TEST_CASE("glb_model: die started over is MU's blend into key 0, never beyond its last key")
{
    FPS_ANIMATION_FACTOR = 1.f;
    auto bmd = OpenYetiBmd();
    auto glb = OpenYetiGlb();
    PrepareLikeTheGame(*bmd);
    PrepareLikeTheGame(*glb);
    const int lastKey = glb->Actions[YetiDie].NumAnimationKeys - 1;
    const vec3_t still = {0.f, 0.f, 0.f};
    Difference largest;
    // The frame set back to 0 by a second SetAction(die) while the prior key is still a late one.
    for (const float prior : {static_cast<float>(lastKey - 1), static_cast<float>(lastKey), 5.f})
    {
        for (const float frame : {0.f, 0.25f, 0.5f, 0.99f})
        {
            CAPTURE(prior);
            CAPTURE(frame);
            const Render::Models::AnimationSampling::PosePlan plan = Render::Models::AnimationSampling::PlanPose(
                {lastKey + 1, false, 0.25f, true}, YetiDie, prior, {lastKey + 1, false, 0.25f, true}, YetiDie, frame);
            CHECK(plan.kind == Render::Models::AnimationSampling::PoseCase::Mu);
            CHECK(plan.from.key0 == static_cast<int>(prior));
            CHECK(plan.to.key0 == 0);
            CheckPose(*bmd, *glb, {YetiDie, frame, YetiDie, prior}, still, largest);
        }
    }
    // Played on from there, the action reaches its last key and holds it, as before.
    Playback playback{0.f, static_cast<float>(lastKey - 1), YetiDie};
    for (int tick = 0; tick < 200; ++tick)
        playback.Tick(*glb, YetiDie);
    CHECK(playback.frame == doctest::Approx(static_cast<float>(lastKey + 1) - 0.01f));
    CheckPose(*bmd, *glb, {YetiDie, playback.frame, playback.priorAction, playback.priorFrame}, still, largest);
}

TEST_CASE("glb_model: a 30-key clip played for a 7-key action keeps the action's duration")
{
    FPS_ANIMATION_FACTOR = 1.f;
    Glb glb = Glb::Read(YetiGlb);
    const int box03 = glb.NodeNamed("Box03");
    AddCountingClip(glb, "walk30", box03, 30, 2.f);
    TemporaryFile file;
    glb.Write(file.Path());
    auto bmd = OpenYetiBmd();
    auto model = OpenYetiGlbWith(file.Path(), {{YetiWalk, {"walk30", std::nullopt}}});
    PrepareLikeTheGame(*bmd);
    PrepareLikeTheGame(*model);
    const auto* skinned = SkinnedModelOf(*model);
    const Action_t& walk = model->Actions[YetiWalk];
    REQUIRE(walk.NumAnimationKeys == 7);
    const int intervals = Render::Models::Gltf::SkinnedModel::ClipKeyIntervals(walk.NumAnimationKeys, walk.LockPositions, walk.Loop);
    const int boxJoint = model->FindBone("Box03");

    // The same ticks per cycle as the BMD: the action's keys and play speed set the time.
    const auto ticksPerCycle = [](BMD& m) {
        Playback playback{0.f, 0.f, YetiWalk};
        int ticks = 0;
        float previous = 0.f;
        do
        {
            previous = playback.frame;
            playback.Tick(m, YetiWalk);
            ++ticks;
        } while (playback.frame >= previous && ticks < 1000);
        return ticks;
    };
    const int bmdTicks = ticksPerCycle(*bmd);
    CHECK(ticksPerCycle(*model) == bmdTicks);
    CHECK(bmdTicks == static_cast<int>(std::ceil(static_cast<float>(intervals) / walk.PlaySpeed)));

    // Over that time the clip runs once: key time k of 7 is clip key 29 x k / 7.
    Playback playback{0.f, 0.f, YetiWalk};
    for (int tick = 0; tick < bmdTicks - 1; ++tick)
    {
        playback.Tick(*model, YetiWalk);
        Render::Models::Gltf::JointPose pose;
        skinned->SampleJoint(boxJoint, YetiWalk, playback.frame, intervals, pose);
        CAPTURE(tick);
        CHECK(pose.position[0] == doctest::Approx(29.f * playback.frame / static_cast<float>(intervals)).epsilon(1e-4));
    }
    Render::Models::Gltf::JointPose end;
    skinned->SampleJoint(boxJoint, YetiWalk, static_cast<float>(intervals), intervals, end);
    CHECK(end.position[0] == doctest::Approx(29.f));
}

TEST_CASE("glb_model: one key per step samples the clip at the continuous key time, detail between keys included")
{
    FPS_ANIMATION_FACTOR = 1.f;
    // Attack 1 (7 keys, repeating: 7 key intervals) gets a dense clip that moves Box03 along x: 0 at every MU
    // key, a spike of 50 halfway between keys.
    Glb glb = Glb::Read(YetiGlb);
    const int box03 = glb.NodeNamed("Box03");
    constexpr int Intervals = 7;
    constexpr float Cycle = 2.f;
    constexpr float Spike = 50.f;
    std::vector<float> times;
    std::vector<float> values;
    for (int half = 0; half <= 2 * Intervals; ++half)
    {
        times.push_back(Cycle * static_cast<float>(half) / static_cast<float>(2 * Intervals));
        values.insert(values.end(), {half % 2 == 1 ? Spike : 0.f, 0.f, 0.f});
    }
    const int input = glb.AddAccessor(times, "SCALAR", times.size());
    glb.json["accessors"][input]["min"] = {0.0};
    glb.json["accessors"][input]["max"] = {Cycle};
    const int output = glb.AddAccessor(values, "VEC3", times.size());
    glb.json["animations"].push_back(
        {{"name", "spike"},
         {"samplers", {{{"input", input}, {"output", output}, {"interpolation", "LINEAR"}}}},
         {"channels", {{{"sampler", 0}, {"target", {{"node", box03}, {"path", "translation"}}}}}}});
    TemporaryFile file;
    glb.Write(file.Path());
    auto model = OpenYetiGlbWith(file.Path(), {{YetiAttack1, {"spike", std::nullopt}}});
    PrepareLikeTheGame(*model);
    model->BodyScale = 1.f;
    REQUIRE(Render::Models::AnimationSampling::KeyIntervals({model->Actions[YetiAttack1].NumAnimationKeys,
                                                              model->Actions[YetiAttack1].LockPositions, 0.25f,
                                                              model->Actions[YetiAttack1].Loop}) == Intervals);
    const int box = model->FindBone("Box03");
    REQUIRE(box >= 0);
    const int parent = model->Bones[box].Parent;
    REQUIRE(parent >= 0);

    // Box03's translation relative to its parent, from BMD::Animation's bone matrices.
    const auto localX = [&](float frame, float priorFrame) {
        const auto bones = std::make_unique<BoneMatrices>();
        vec3_t angle = {0.f, 0.f, 0.f};
        vec3_t head = {0.f, 0.f, 0.f};
        model->Animation(bones->bones, frame, priorFrame, YetiAttack1, angle, head, false, false, nullptr,
                         YetiAttack1);
        const auto& p = bones->bones[parent];
        const auto& b = bones->bones[box];
        const float d[3] = {b[0][3] - p[0][3], b[1][3] - p[1][3], b[2][3] - p[2][3]};
        float local[3];
        for (int c = 0; c < 3; ++c)
            local[c] = p[0][c] * d[0] + p[1][c] * d[1] + p[2][c] * d[2];
        CHECK(std::fabs(local[1]) < 1e-3f);
        CHECK(std::fabs(local[2]) < 1e-3f);
        return local[0];
    };
    // MU blends key 0 into key 1 at frame 1.5 (prior key 0): key time 0.5, the spike.
    CHECK(localX(1.5f, 0.75f) == doctest::Approx(Spike).epsilon(1e-4));
    CHECK(localX(1.25f, 0.75f) == doctest::Approx(Spike / 2.f).epsilon(1e-4));
    // On the keys themselves: 0.
    CHECK(localX(1.f, 0.75f) == doctest::Approx(0.f).epsilon(1e-4));
    CHECK(localX(4.f, 3.f) == doctest::Approx(0.f).epsilon(1e-4));
    // Halfway in the next intervals, and across the wrap (key 6 into the closing key, then key 0).
    CHECK(localX(4.5f, 3.f) == doctest::Approx(Spike).epsilon(1e-4));
    CHECK(localX(0.5f, 6.75f) == doctest::Approx(Spike).epsilon(1e-4));
    // A held pose (prior key = current key) stays the key itself.
    CHECK(localX(3.5f, 3.f) == doctest::Approx(0.f).epsilon(1e-4));
}

// ---------------------------------------------------------------- LOD by screen size

namespace
{
using Data::Models::LodSettings;
using Render::Models::SelectLod;

// The LOD 0 primitive's triangles as vertex indices.
std::vector<std::uint32_t> Lod0Indices(const Glb& glb)
{
    const auto& accessor = glb.json["accessors"][glb.json["meshes"][0]["primitives"][0]["indices"].get<int>()];
    const auto& view = glb.json["bufferViews"][accessor["bufferView"].get<int>()];
    const size_t offset = view.value("byteOffset", 0) + accessor.value("byteOffset", 0);
    const size_t count = accessor["count"];
    std::vector<std::uint32_t> indices(count);
    for (size_t i = 0; i < count; ++i)
    {
        std::uint16_t index = 0;
        std::memcpy(&index, &glb.bin[offset + i * 2], 2);
        indices[i] = index;
    }
    return indices;
}

// Adds a mesh node with extras.lod = `lod` that draws every `stride`-th
// triangle of LOD 0 (same vertices, skin and material), with 32-bit indices.
void AddLodNode(Glb& glb, int lod, int stride, bool onSkin = true)
{
    const std::vector<std::uint32_t> all = Lod0Indices(glb);
    std::vector<std::uint32_t> indices;
    for (size_t t = 0; t + 2 < all.size(); t += 3)
    {
        if ((t / 3) % static_cast<size_t>(stride) == 0)
            indices.insert(indices.end(), all.begin() + static_cast<std::ptrdiff_t>(t),
                           all.begin() + static_cast<std::ptrdiff_t>(t + 3));
    }
    while (glb.bin.size() % 4 != 0)
        glb.bin.push_back(0);
    const size_t offset = glb.bin.size();
    glb.bin.resize(offset + indices.size() * 4);
    std::memcpy(&glb.bin[offset], indices.data(), indices.size() * 4);
    glb.json["bufferViews"].push_back({{"buffer", 0}, {"byteOffset", offset}, {"byteLength", indices.size() * 4}});
    glb.json["accessors"].push_back({{"bufferView", glb.json["bufferViews"].size() - 1},
                                     {"componentType", 5125},
                                     {"count", indices.size()},
                                     {"type", "SCALAR"}});
    nlohmann::json primitive = glb.json["meshes"][0]["primitives"][0];
    primitive["indices"] = glb.json["accessors"].size() - 1;
    glb.json["meshes"].push_back({{"name", "LOD" + std::to_string(lod)}, {"primitives", {primitive}}});
    nlohmann::json node = {{"name", "LOD" + std::to_string(lod)},
                           {"mesh", glb.json["meshes"].size() - 1},
                           {"extras", {{"lod", lod}}}};
    if (onSkin)
        node["skin"] = 0;
    glb.json["nodes"].push_back(node);
    glb.json["scenes"][0]["nodes"].push_back(glb.json["nodes"].size() - 1);
}

// The exported Elite Yeti with LOD 1 (every 2nd triangle) and LOD 2 (every 4th).
void WriteThreeLodYeti(const std::filesystem::path& file)
{
    Glb glb = Glb::Read(YetiGlb);
    AddLodNode(glb, 1, 2);
    AddLodNode(glb, 2, 4);
    glb.Write(file);
}

size_t TriangleCount(const std::vector<Render::Models::Gltf::LodMesh>& meshes)
{
    size_t count = 0;
    for (const auto& mesh : meshes)
        count += mesh.triangles.size();
    return count;
}
} // namespace

TEST_CASE("glb_model: LOD selection switches with screen height and holds near the thresholds")
{
    const LodSettings settings; // 0.2 and 0.1, 10 % hysteresis
    // From afar to near and back, starting at LOD 0.
    CHECK(SelectLod(0.5f, 0, settings, 3) == 0);
    CHECK(SelectLod(0.19f, 0, settings, 3) == 1);
    CHECK(SelectLod(0.09f, 1, settings, 3) == 2);
    CHECK(SelectLod(0.05f, 0, settings, 3) == 2); // a jump goes straight to the coarsest
    // Hysteresis: finer only above threshold x 1.1.
    CHECK(SelectLod(0.105f, 2, settings, 3) == 2);
    CHECK(SelectLod(0.11f, 2, settings, 3) == 2);
    CHECK(SelectLod(0.111f, 2, settings, 3) == 1);
    CHECK(SelectLod(0.215f, 1, settings, 3) == 1);
    CHECK(SelectLod(0.221f, 1, settings, 3) == 0);
    CHECK(SelectLod(0.9f, 2, settings, 3) == 0);
    // Coarser at once below the threshold, whatever came before.
    CHECK(SelectLod(0.1999f, 0, settings, 3) == 1);
    CHECK(SelectLod(0.2f, 0, settings, 3) == 0);
    // A model with fewer levels ignores the thresholds beyond them.
    CHECK(SelectLod(0.01f, 0, settings, 2) == 1);
    CHECK(SelectLod(0.01f, 2, settings, 2) == 1);
    CHECK(SelectLod(0.01f, 0, settings, 1) == 0);
    CHECK(SelectLod(0.01f, 0, LodSettings{{}, 0.1f}, 3) == 0);
    // A stale previous LOD counts as 0.
    CHECK(SelectLod(0.5f, 7, settings, 3) == 0);
    CHECK(SelectLod(0.5f, -1, settings, 3) == 0);
    // No hysteresis: the thresholds both ways.
    CHECK(SelectLod(0.2001f, 1, LodSettings{{0.2f, 0.1f}, 0.f}, 3) == 0);
}

TEST_CASE("glb_model: LOD follows the camera distance with hysteresis on the way back")
{
    // A model 200 units high seen by a camera whose window shows 1000 units at distance 1000:
    // screen height = 200 / distance.
    const LodSettings settings;
    const auto heightAt = [](float distance) { return 200.f / distance; };
    int lod = 0;
    std::vector<std::pair<float, int>> switches;
    for (float distance = 500.f; distance <= 3000.f; distance += 10.f)
    {
        const int next = SelectLod(heightAt(distance), lod, settings, 3);
        if (next != lod)
            switches.emplace_back(distance, next);
        lod = next;
    }
    REQUIRE(switches.size() == 2);
    CHECK(switches[0] == std::pair<float, int>{1010.f, 1}); // 200 / 1010 < 0.2
    CHECK(switches[1] == std::pair<float, int>{2010.f, 2}); // 200 / 2010 < 0.1
    switches.clear();
    for (float distance = 3000.f; distance >= 500.f; distance -= 10.f)
    {
        const int next = SelectLod(heightAt(distance), lod, settings, 3);
        if (next != lod)
            switches.emplace_back(distance, next);
        lod = next;
    }
    REQUIRE(switches.size() == 2);
    CHECK(switches[0] == std::pair<float, int>{1810.f, 1}); // 200 / 1810 > 0.11
    CHECK(switches[1] == std::pair<float, int>{900.f, 0});  // 200 / 900 > 0.22
    // Jitter around a threshold does not flicker.
    lod = 1;
    for (const float distance : {1000.f, 1005.f, 995.f, 1003.f, 990.f, 1008.f})
    {
        lod = SelectLod(heightAt(distance), lod, settings, 3);
        CHECK(lod == 1);
    }
}

TEST_CASE("glb_model: screen height of projected bounds")
{
    const float rows[] = {420.f, 100.f, 380.f, 160.f};
    CHECK(Render::Models::ScreenHeightFraction(rows, true, 1000.f) == doctest::Approx(0.32f));
    CHECK(Render::Models::ScreenHeightFraction(rows, false, 1000.f) == 1.f);
    CHECK(Render::Models::ScreenHeightFraction({}, true, 1000.f) == 1.f);
}

TEST_CASE("glb_model: the loader takes LOD 1 and 2 on the same skin")
{
    TemporaryFile file;
    WriteThreeLodYeti(file.Path());
    const auto model = OpenYetiGlb(file.Path());
    const auto* skinned = SkinnedModelOf(*model);
    REQUIRE(skinned->LodCount() == 3);
    const size_t lod0Triangles = Lod0Indices(Glb::Read(YetiGlb)).size() / 3;
    size_t bmdTriangles = 0;
    for (int i = 0; i < model->NumMeshs; ++i)
        bmdTriangles += static_cast<size_t>(model->Meshs[i].NumTriangles);
    CHECK(bmdTriangles == lod0Triangles);
    CHECK(TriangleCount(skinned->lods[0]) == (lod0Triangles + 1) / 2);
    CHECK(TriangleCount(skinned->lods[1]) == (lod0Triangles + 3) / 4);
    // The LOD meshes skin to the same joints as LOD 0: the skeleton is posed once per character.
    for (const auto& meshes : skinned->lods)
    {
        REQUIRE(!meshes.empty());
        CHECK(meshes.size() <= static_cast<size_t>(model->NumMeshs));
        for (const auto& mesh : meshes)
        {
            CHECK(mesh.texture == "snowman02.jpg");
            CHECK(mesh.vertices.size() == mesh.texCoords.size());
            for (const auto& vertex : mesh.vertices)
                for (int k = 0; k < vertex.count; ++k)
                    CHECK(vertex.joints[k] < model->NumBones);
        }
    }
    // A LOD vertex is the same skinned vertex as the LOD 0 vertex it comes from.
    const auto& lod0 = skinned->meshVertices[0];
    const auto& lod1 = skinned->lods[0][0].vertices;
    const auto& first = skinned->lods[0][0].triangles[0];
    const Triangle_t& bmdFirst = model->Meshs[0].Triangles[0];
    for (int c = 0; c < 3; ++c)
    {
        const auto& a = lod1[static_cast<size_t>(first[c])];
        const auto& b = lod0[static_cast<size_t>(bmdFirst.VertexIndex[c])];
        CHECK(a.count == b.count);
        CHECK(a.joints == b.joints);
        CHECK(a.bindPosition == b.bindPosition);
    }
    // LodMeshFor: LOD 0 draws the BMD mesh; a LOD's mesh stands in for BMD mesh i.
    bool absent = true;
    CHECK(skinned->LodMeshFor(0, 0, absent) == nullptr);
    CHECK_FALSE(absent);
    CHECK(skinned->LodMeshFor(1, 0, absent) == &skinned->lods[0][0]);
    CHECK(skinned->LodMeshFor(2, 0, absent) == &skinned->lods[1][0]);
    CHECK(skinned->LodMeshFor(3, 0, absent) == nullptr);
    CHECK_FALSE(absent);
    CHECK(skinned->LodMeshFor(1, static_cast<int>(skinned->lods[0].size()), absent) == nullptr);
    CHECK(absent);
    // The rest bounds enclose the Yeti standing (z up, MU units).
    CHECK(skinned->restBoundsMax[2] - skinned->restBoundsMin[2] > 50.f);
    CHECK(skinned->restBoundsMin[2] < skinned->restBoundsMax[2]);
}

TEST_CASE("glb_model: a mesh is packed once per LOD, indexed, and expands to the corners a draw packed")
{
    TemporaryFile file;
    WriteThreeLodYeti(file.Path());
    const auto model = OpenYetiGlb(file.Path());
    const auto* skinned = SkinnedModelOf(*model);
    REQUIRE(skinned->LodCount() == 3);
    const Mesh_t& mesh = model->Meshs[0];
    const auto same = [](const mu::SkinnedWeightedVertex3D& a, const mu::SkinnedWeightedVertex3D& b)
    { return std::memcmp(&a, &b, sizeof(a)) == 0; };
    std::vector<mu::SkinnedWeightedVertex3D> corners;

    // LOD 0: the BMD mesh's corners on the file's skin, as the per-corner draw packed them.
    auto& lod0 = Render::Models::PackedWeightedMeshOf(*skinned, mesh, 0, 0, nullptr);
    REQUIRE(lod0.indices.size() == static_cast<size_t>(mesh.NumTriangles) * 3);
    CHECK(lod0.vertices.size() <= lod0.indices.size());
    mu::ExpandIndexedCorners<mu::SkinnedWeightedVertex3D>(lod0.vertices, lod0.indices, corners);
    size_t corner = 0;
    bool lod0Same = true;
    for (int t = 0; t < mesh.NumTriangles; ++t)
    {
        const Triangle_t& triangle = mesh.Triangles[t];
        for (int k = 0; k < triangle.Polygon; ++k)
        {
            const TexCoord_t& texCoord = mesh.TexCoords[triangle.TexCoordIndex[k]];
            lod0Same = lod0Same && same(corners[corner++], Render::Models::PackWeightedSkinnedVertex(
                                                               skinned->meshVertices[0][triangle.VertexIndex[k]],
                                                               texCoord.TexCoordU + 0.0f, texCoord.TexCoordV + 0.0f));
        }
    }
    CHECK(lod0Same);

    // LOD 1: its own mesh.
    bool absent = false;
    const auto* lodMesh = skinned->LodMeshFor(1, 0, absent);
    REQUIRE(lodMesh != nullptr);
    auto& lod1 = Render::Models::PackedWeightedMeshOf(*skinned, mesh, 0, 1, lodMesh);
    REQUIRE(lod1.indices.size() == lodMesh->triangles.size() * 3);
    CHECK(lod1.vertices.size() <= lodMesh->vertices.size());
    mu::ExpandIndexedCorners<mu::SkinnedWeightedVertex3D>(lod1.vertices, lod1.indices, corners);
    corner = 0;
    bool lod1Same = true;
    for (const auto& triangle : lodMesh->triangles)
    {
        for (const int vertex : triangle)
        {
            lod1Same = lod1Same && same(corners[corner++], Render::Models::PackWeightedSkinnedVertex(
                                                               lodMesh->vertices[static_cast<size_t>(vertex)],
                                                               lodMesh->texCoords[static_cast<size_t>(vertex)][0] + 0.0f,
                                                               lodMesh->texCoords[static_cast<size_t>(vertex)][1] + 0.0f));
        }
    }
    CHECK(lod1Same);

    // Kept on the model: a later draw gets the same vertices at the same address (the GPU cache key).
    const mu::SkinnedWeightedVertex3D* lod1Vertices = lod1.vertices.data();
    auto& again = Render::Models::PackedWeightedMeshOf(*skinned, mesh, 0, 1, lodMesh);
    CHECK(&again == &lod1);
    CHECK(again.vertices.data() == lod1Vertices);
    CHECK(&Render::Models::PackedWeightedMeshOf(*skinned, mesh, 0, 0, nullptr) == &lod0);
}

namespace
{
// A stand-in for the planar shadow's projection (BMD.cpp CalcShadowPosition):
// the same arithmetic with a made-up ground height, a pure function of the position.
struct TestGround
{
    float origin[3];
    float sx;
    float sy;
};

void ProjectOntoTestGround(float position[3], const void* context)
{
    const auto* ground = static_cast<const TestGround*>(context);
    float relative[3] = {position[0] - ground->origin[0], position[1] - ground->origin[1],
                         position[2] - ground->origin[2]};
    relative[0] += relative[2] * (relative[0] + ground->sx) / (relative[2] - ground->sy);
    position[0] = relative[0] + ground->origin[0];
    position[1] = relative[1] + ground->origin[1];
    position[2] = std::sin(position[0] * 0.01f) * 30.f + std::cos(position[1] * 0.013f) * 20.f + 5.f;
}

bool SameBits(const float a[3], const float b[3])
{
    return std::memcmp(a, b, sizeof(float) * 3) == 0;
}

// The corners of mesh `mesh`'s planar shadow: its projected vertices
// (BMD::ProjectWeightedShadowMesh) expanded by its indices, as an indexed draw
// rasterises them. Returns the number of corners.
int ShadowCorners(const BMD& model, int mesh, const TestGround& ground, std::vector<std::array<float, 3>>& corners)
{
    std::vector<std::array<float, 3>> vertices;
    std::vector<std::uint32_t> indices;
    model.ProjectWeightedShadowMesh(mesh, ProjectOntoTestGround, &ground, vertices, indices);
    REQUIRE(std::all_of(indices.begin(), indices.end(), [&](std::uint32_t index) { return index < vertices.size(); }));
    corners.resize(indices.size());
    for (size_t c = 0; c < indices.size(); ++c)
    {
        corners[c] = vertices[indices[c]];
    }
    return static_cast<int>(indices.size());
}
} // namespace

TEST_CASE("glb_model: a single-LOD model's indexed planar shadow expands to the per-corner projection, bit for bit")
{
    auto model = OpenYetiGlb();
    PrepareLikeTheGame(*model);
    auto bones = std::make_unique<BoneMatrices>();
    const TestGround ground{{1200.f, -800.f, 160.f}, 2000.f, 4000.f};
    const Mesh_t& mesh = model->Meshs[0];
    std::vector<std::array<float, 3>> corners(static_cast<size_t>(mesh.NumTriangles) * 3);

    for (const bool translate : {false, true})
    {
        for (unsigned short action = 0; action < YetiActions; ++action)
        {
            CAPTURE(translate);
            CAPTURE(action);
            SkinOnCpu(*model, *bones, action, 2.4f, translate);
            const int count = ShadowCorners(*model, 0, ground, corners);
            REQUIRE(count == mesh.NumTriangles * 3);
            // What AddMeshShadowTriangles does for a BMD: each corner copied from VertexTransform, then projected.
            bool same = true;
            int corner = 0;
            for (int t = 0; t < mesh.NumTriangles; ++t)
            {
                for (int k = 0; k < mesh.Triangles[t].Polygon; ++k)
                {
                    vec3_t expected;
                    VectorCopy(VertexTransform[0][mesh.Triangles[t].VertexIndex[k]], expected);
                    ProjectOntoTestGround(expected, &ground);
                    same = same && SameBits(corners[static_cast<size_t>(corner++)].data(), expected);
                }
            }
            CHECK(same);
        }
    }
}

TEST_CASE("glb_model: the planar shadow follows the LOD being drawn")
{
    TemporaryFile file;
    WriteThreeLodYeti(file.Path());
    auto model = OpenYetiGlb(file.Path());
    PrepareLikeTheGame(*model);
    const auto* skinned = SkinnedModelOf(*model);
    REQUIRE(skinned->LodCount() == 3);
    auto bones = std::make_unique<BoneMatrices>();
    const TestGround ground{{1200.f, -800.f, 160.f}, 2000.f, 4000.f};
    const Mesh_t& mesh = model->Meshs[0];
    std::vector<std::array<float, 3>> lod0(static_cast<size_t>(mesh.NumTriangles) * 3);
    std::vector<std::array<float, 3>> lod(lod0.size());

    for (const bool translate : {false, true})
    {
        CAPTURE(translate);
        SkinOnCpu(*model, *bones, YetiAttack1, 3.7f, translate);
        model->ActiveLod = 0;
        REQUIRE(ShadowCorners(*model, 0, ground, lod0) == mesh.NumTriangles * 3);
        for (int level = 1; level <= 2; ++level)
        {
            CAPTURE(level);
            model->ActiveLod = level;
            const auto& triangles = skinned->lods[static_cast<size_t>(level - 1)][0].triangles;
            const int count = ShadowCorners(*model, 0, ground, lod);
            // The LOD's own triangles: every 2nd (LOD 1) or 4th (LOD 2) triangle of LOD 0, on the same skin,
            // so each corner is where LOD 0's shadow puts it.
            REQUIRE(count == static_cast<int>(triangles.size()) * 3);
            const size_t stride = level == 1 ? 2 : 4;
            bool same = true;
            for (size_t corner = 0; corner < static_cast<size_t>(count); ++corner)
            {
                const size_t lod0Corner = (corner / 3) * stride * 3 + corner % 3;
                same = same && SameBits(lod[corner].data(), lod0[lod0Corner].data());
            }
            CHECK(same);
        }
        // A mesh the LOD does not have casts no shadow; a BMD mesh index out of range neither.
        model->ActiveLod = 1;
        CHECK(ShadowCorners(*model, model->NumMeshs, ground, lod) == 0);
        model->ActiveLod = 0;
    }
}

TEST_CASE("glb_model: the exported Elite Yeti alone has one LOD")
{
    const auto model = OpenYetiGlb();
    CHECK(SkinnedModelOf(*model)->LodCount() == 1);
    CHECK(SkinnedModelOf(*model)->lods.empty());
}

TEST_CASE("glb_model: LOD meshes that break the conventions are refused")
{
    SUBCASE("LOD 2 without LOD 1")
    {
        Glb glb = Glb::Read(YetiGlb);
        AddLodNode(glb, 2, 4);
        TemporaryFile file;
        glb.Write(file.Path());
        RequireRefused(file.Path(), "LOD 2 but no LOD 1");
    }
    SUBCASE("LOD 3")
    {
        Glb glb = Glb::Read(YetiGlb);
        AddLodNode(glb, 3, 4);
        TemporaryFile file;
        glb.Write(file.Path());
        RequireRefused(file.Path(), "has LOD 3");
    }
    SUBCASE("a LOD off the skin")
    {
        Glb glb = Glb::Read(YetiGlb);
        AddLodNode(glb, 1, 2, false);
        TemporaryFile file;
        glb.Write(file.Path());
        RequireRefused(file.Path(), "is not on the skin");
    }
}

TEST_CASE("glb_model: the LOD settings of the table reach the model")
{
    TemporaryFile file;
    WriteThreeLodYeti(file.Path());
    auto model = OpenYetiBmd();
    std::string reason;
    const LodSettings settings{{0.35f, 0.15f}, 0.2f};
    REQUIRE(LoadGlbIntoBmd(*model, file.Path(), NoActionClips, reason, settings));
    CHECK(SkinnedModelOf(*model)->lodSettings == settings);
}

// ---------------------------------------------------------------- normal maps

namespace
{
// Gives the LOD 0 primitive a TANGENT accessor (`tangentOf(vertex)`) and a
// material with a normalTexture of scale `scale` (an embedded "PNG" whose
// bytes the loader only copies).
template <typename TangentOf>
void AddNormalMap(Glb& glb, float scale, TangentOf tangentOf, bool withTangents = true)
{
    auto& primitive = glb.json["meshes"][0]["primitives"][0];
    const size_t count = glb.json["accessors"][primitive["attributes"]["POSITION"].get<int>()]["count"];
    if (withTangents)
    {
        std::vector<float> tangents;
        for (size_t v = 0; v < count; ++v)
        {
            const std::array<float, 4> t = tangentOf(v);
            tangents.insert(tangents.end(), t.begin(), t.end());
        }
        primitive["attributes"]["TANGENT"] = glb.AddAccessor(tangents, "VEC4", count);
    }
    while (glb.bin.size() % 4 != 0)
        glb.bin.push_back(0);
    const std::vector<std::uint8_t> png = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    const size_t offset = glb.bin.size();
    glb.bin.insert(glb.bin.end(), png.begin(), png.end());
    glb.json["bufferViews"].push_back({{"buffer", 0}, {"byteOffset", offset}, {"byteLength", png.size()}});
    glb.json["images"].push_back(
        {{"name", "bump_normal"}, {"mimeType", "image/png"}, {"bufferView", glb.json["bufferViews"].size() - 1}});
    glb.json["textures"].push_back({{"source", glb.json["images"].size() - 1}});
    auto& material = glb.json["materials"][primitive.value("material", 0)];
    material["normalTexture"] = {{"index", glb.json["textures"].size() - 1}, {"scale", scale}};
}

// A unit tangent per vertex, the handedness alternating: an "unmirrored" and a "mirrored" UV island.
std::array<float, 4> AlternatingTangent(size_t vertex)
{
    const float angle = 0.37f * static_cast<float>(vertex);
    return {std::cos(angle), std::sin(angle), 0.f, vertex % 2 == 0 ? 1.f : -1.f};
}

// skinned_textured.vert's tangent: the packed tangent (snorm16) through skinWeightedNormal
// (the shader skins it like the normal), then normalised as skinned_lit.frag does.
std::array<float, 4> ShadeWeightedTangent(const mu::SkinnedWeightedVertex3D& packed, const std::vector<float>& palette,
                                          float boneScale, float restPoseScale)
{
    mu::SkinnedWeightedVertex3D asNormal = packed;
    asNormal.nx = static_cast<float>(packed.tangent[0]) / 32767.f;
    asNormal.ny = static_cast<float>(packed.tangent[1]) / 32767.f;
    asNormal.nz = static_cast<float>(packed.tangent[2]) / 32767.f;
    float t[3];
    ShadeWeightedNormal(asNormal, palette, boneScale, restPoseScale, t);
    const float length = std::sqrt(t[0] * t[0] + t[1] * t[1] + t[2] * t[2]);
    return {t[0] / length, t[1] / length, t[2] / length, packed.tangent[3] > 0 ? 1.f : -1.f};
}

std::array<float, 3> Cross(const std::array<float, 3>& a, const std::array<float, 3>& b)
{
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}

// skinned_lit.frag's PerturbNormal for unit n and t.
std::array<float, 3> PerturbNormal(const std::array<float, 3>& n, const std::array<float, 3>& t, float handedness,
                                   const std::array<float, 3>& mapped)
{
    const std::array<float, 3> b = Cross(n, t);
    std::array<float, 3> out{};
    for (int c = 0; c < 3; ++c)
        out[c] = t[c] * mapped[0] + b[c] * handedness * mapped[1] + n[c] * mapped[2];
    const float length = std::sqrt(out[0] * out[0] + out[1] * out[1] + out[2] * out[2]);
    for (float& c : out)
        c /= length;
    return out;
}
} // namespace

TEST_CASE("glb_model: a mesh with a normal map keeps the file's tangents and the map's scale")
{
    Glb glb = Glb::Read(YetiGlb);
    AddNormalMap(glb, 0.7f, AlternatingTangent);
    TemporaryFile file;
    glb.Write(file.Path());
    const auto model = OpenYetiGlb(file.Path());
    const auto* skinned = SkinnedModelOf(*model);
    REQUIRE(skinned->meshNormalMaps.size() == static_cast<size_t>(model->NumMeshs));
    for (int mesh = 0; mesh < model->NumMeshs; ++mesh)
    {
        const auto& map = skinned->meshNormalMaps[static_cast<size_t>(mesh)];
        REQUIRE(map.image >= 0);
        CHECK(skinned->images[static_cast<size_t>(map.image)].name == "bump_normal");
        CHECK(map.scale == doctest::Approx(0.7f));
    }
    // The base colour image and the normal map are two images.
    CHECK(skinned->meshImages[0] != skinned->meshNormalMaps[0].image);
    // Tangents as baked, w exactly +-1; the BMD mesh vertex is the primitive's vertex, so compare by value.
    int mirrored = 0;
    int unmirrored = 0;
    for (const auto& vertex : skinned->meshVertices[0])
    {
        const float length = std::sqrt(vertex.bindTangent[0] * vertex.bindTangent[0] +
                                       vertex.bindTangent[1] * vertex.bindTangent[1] +
                                       vertex.bindTangent[2] * vertex.bindTangent[2]);
        CHECK(length == doctest::Approx(1.f).epsilon(1e-5));
        CHECK(std::fabs(vertex.bindTangent[3]) == 1.f);
        (vertex.bindTangent[3] < 0.f ? mirrored : unmirrored)++;
    }
    CHECK(mirrored > 0);
    CHECK(unmirrored > 0);
    // Not drawn before the textures are opened: no map texture yet.
    CHECK(skinned->NormalMapTextureFor(0, 0).scale == 0.f);
}

TEST_CASE("glb_model: a mesh without a normal map has no tangents and no map")
{
    const auto model = OpenYetiGlb();
    const auto* skinned = SkinnedModelOf(*model);
    for (const auto& map : skinned->meshNormalMaps)
        CHECK(map.image == -1);
    for (const auto& vertex : skinned->meshVertices[0])
        CHECK(vertex.bindTangent == std::array<float, 4>{});
    for (const auto& vertex : skinned->meshVertices[0])
    {
        const mu::SkinnedWeightedVertex3D packed = Render::Models::PackWeightedSkinnedVertex(vertex, 0.f, 0.f);
        for (int c = 0; c < 4; ++c)
            CHECK(packed.tangent[c] == 0);
    }
}

TEST_CASE("glb_model: a normal map without TANGENT is refused")
{
    Glb glb = Glb::Read(YetiGlb);
    AddNormalMap(glb, 1.f, AlternatingTangent, false);
    TemporaryFile file;
    glb.Write(file.Path());
    RequireRefused(file.Path(), "lacks TANGENT");
}

TEST_CASE("glb_model: tangents pack as snorm16 with the handedness in w; BMD vertices carry none")
{
    using Render::Models::PackTangent;
    CHECK(PackTangent({1.f, 0.f, 0.f, 1.f}) == std::array<std::int16_t, 4>{32767, 0, 0, 32767});
    CHECK(PackTangent({0.f, -1.f, 0.f, -1.f}) == std::array<std::int16_t, 4>{0, -32767, 0, -32767});
    CHECK(PackTangent({0.6f, 0.f, 0.8f, -1.f}) == std::array<std::int16_t, 4>{19660, 0, 26214, -32767});
    // No normal map: w 0 packs as no tangent at all.
    CHECK(PackTangent({0.f, 0.f, 0.f, 0.f}) == std::array<std::int16_t, 4>{});
    CHECK(PackTangent({0.3f, 0.4f, 0.f, 0.f}) == std::array<std::int16_t, 4>{});
    // Round trip within half a unit of 1/32767.
    for (size_t v = 0; v < 64; ++v)
    {
        const std::array<float, 4> tangent = AlternatingTangent(v);
        const std::array<std::int16_t, 4> packed = PackTangent(tangent);
        for (int c = 0; c < 3; ++c)
            CHECK(std::fabs(static_cast<float>(packed[c]) / 32767.f - tangent[c]) <= 0.5f / 32767.f + 1e-7f);
        CHECK(packed[3] == (tangent[3] > 0.f ? 32767 : -32767));
    }
}

TEST_CASE("glb_model: GPU tangent skinning follows the normal and keeps the handedness of both UV islands")
{
    auto bmd = OpenYetiBmd();
    Glb glb = Glb::Read(YetiGlb);
    MakeTwoWeightSkin(glb, *bmd);
    // Tangents perpendicular to the file's normals, so that (T, B, N) is a frame.
    const auto& primitive = glb.json["meshes"][0]["primitives"][0];
    const auto& normals = glb.json["accessors"][primitive["attributes"]["NORMAL"].get<int>()];
    const auto& view = glb.json["bufferViews"][normals["bufferView"].get<int>()];
    const size_t normalOffset = view.value("byteOffset", 0) + normals.value("byteOffset", 0);
    const Glb& source = glb;
    AddNormalMap(glb, 1.f, [&](size_t v) {
        std::array<float, 3> n{};
        std::memcpy(n.data(), &source.bin[normalOffset + v * 12], 12);
        const std::array<float, 3> helper = std::fabs(n[2]) < 0.9f ? std::array<float, 3>{0.f, 0.f, 1.f}
                                                                    : std::array<float, 3>{1.f, 0.f, 0.f};
        std::array<float, 3> t = Cross(helper, n);
        const float length = std::sqrt(t[0] * t[0] + t[1] * t[1] + t[2] * t[2]);
        return std::array<float, 4>{t[0] / length, t[1] / length, t[2] / length, v % 2 == 0 ? 1.f : -1.f};
    });
    TemporaryFile file;
    glb.Write(file.Path());
    auto model = OpenYetiGlb(file.Path());
    PrepareLikeTheGame(*model);
    auto bones = std::make_unique<BoneMatrices>();
    SkinOnCpu(*model, *bones, YetiWalk, 3.7f, false);
    const auto* skinned = SkinnedModelOf(*model);
    std::vector<float> palette;
    Render::Models::BuildWeightedSkinPalette(*skinned, bones->bones, 1.f, 0.f, palette);
    float largestDot = 0.f;
    float largestAngle = 0.f;
    int checked[2] = {0, 0};
    for (size_t v = 0; v < skinned->meshVertices[0].size(); ++v)
    {
        const auto& vertex = skinned->meshVertices[0][v];
        const mu::SkinnedWeightedVertex3D packed = Render::Models::PackWeightedSkinnedVertex(vertex, 0.f, 0.f);
        const std::array<float, 4> t = ShadeWeightedTangent(packed, palette, 1.f, 0.f);
        float nRaw[3];
        ShadeWeightedNormal(packed, palette, 1.f, 0.f, nRaw);
        const float nLength = std::sqrt(nRaw[0] * nRaw[0] + nRaw[1] * nRaw[1] + nRaw[2] * nRaw[2]);
        const std::array<float, 3> n{nRaw[0] / nLength, nRaw[1] / nLength, nRaw[2] / nLength};
        // The expected tangent: the bind tangent through the joints' palette rotations, like the normal.
        std::array<float, 3> expected{};
        for (int k = 0; k < vertex.count; ++k)
        {
            const float* rows = &palette[static_cast<size_t>(vertex.joints[k]) * 12];
            for (int r = 0; r < 3; ++r)
                expected[r] += vertex.weights[k] * (rows[r * 4] * vertex.bindTangent[0] +
                                                    rows[r * 4 + 1] * vertex.bindTangent[1] +
                                                    rows[r * 4 + 2] * vertex.bindTangent[2]);
        }
        const float eLength = std::sqrt(expected[0] * expected[0] + expected[1] * expected[1] + expected[2] * expected[2]);
        const float alignment = (t[0] * expected[0] + t[1] * expected[1] + t[2] * expected[2]) / eLength;
        largestAngle = std::max(largestAngle, std::acos(std::min(1.f, alignment)));
        // A rigid blend keeps T perpendicular to N closely enough for the frame.
        largestDot = std::max(largestDot, std::fabs(t[0] * n[0] + t[1] * n[1] + t[2] * n[2]));
        CHECK(t[3] == vertex.bindTangent[3]);
        // The bitangent cross(N, T) x w: a map normal tilted along +y moves towards it on both islands.
        const std::array<float, 3> b = Cross(n, {t[0], t[1], t[2]});
        const std::array<float, 3> tilted = PerturbNormal(n, {t[0], t[1], t[2]}, t[3], {0.f, 0.5f, 0.8660254f});
        const float towardsB = (tilted[0] * b[0] + tilted[1] * b[1] + tilted[2] * b[2]) * t[3];
        // 0.5 for a frame with T exactly perpendicular to N; a blend of joints leaves them slightly off
        // (tangents are used as baked, not orthogonalised), which only shortens cross(N, T).
        CHECK(towardsB > 0.45f);
        CHECK(towardsB <= 0.5f + 1e-4f);
        // The other handedness mirrors it.
        const std::array<float, 3> mirroredTilt = PerturbNormal(n, {t[0], t[1], t[2]}, -t[3], {0.f, 0.5f, 0.8660254f});
        CHECK((mirroredTilt[0] * b[0] + mirroredTilt[1] * b[1] + mirroredTilt[2] * b[2]) * t[3] < -0.45f);
        ++checked[t[3] > 0.f ? 0 : 1];
    }
    CHECK(checked[0] > 0);
    CHECK(checked[1] > 0);
    MESSAGE("largest tangent angle " << largestAngle << ", largest |T.N| " << largestDot);
    CHECK(largestAngle < 1e-3f);
    CHECK(largestDot < 0.2f);
}

TEST_CASE("glb_model: the flat map and scale 0 leave the normal as it is")
{
    const std::array<float, 3> n{0.f, 0.6f, 0.8f};
    const std::array<float, 3> t{1.f, 0.f, 0.f};
    for (const float handedness : {1.f, -1.f})
    {
        const std::array<float, 3> flat = PerturbNormal(n, t, handedness, {0.f, 0.f, 1.f});
        for (int c = 0; c < 3; ++c)
            CHECK(flat[c] == doctest::Approx(n[c]).epsilon(1e-6));
    }
    // A bump's slope along +x (tangent space) tilts the normal towards T on both islands; along +y towards
    // +B on the unmirrored one and its mirror image on the mirrored one.
    const std::array<float, 3> alongX = PerturbNormal(n, t, -1.f, {0.6f, 0.f, 0.8f});
    CHECK(alongX[0] == doctest::Approx(0.6f));
    const std::array<float, 3> b = Cross(n, t);
    const std::array<float, 3> up = PerturbNormal(n, t, 1.f, {0.f, 0.6f, 0.8f});
    const std::array<float, 3> mirroredUp = PerturbNormal(n, t, -1.f, {0.f, 0.6f, 0.8f});
    const auto dot = [](const std::array<float, 3>& a, const std::array<float, 3>& c) {
        return a[0] * c[0] + a[1] * c[1] + a[2] * c[2];
    };
    CHECK(dot(up, b) == doctest::Approx(0.6f));
    CHECK(dot(mirroredUp, b) == doctest::Approx(-0.6f));
}

TEST_CASE("glb_model: a coarser LOD keeps its own normal map and tangents")
{
    Glb glb = Glb::Read(YetiGlb);
    AddNormalMap(glb, 0.5f, AlternatingTangent);
    AddLodNode(glb, 1, 2);
    TemporaryFile file;
    glb.Write(file.Path());
    const auto model = OpenYetiGlb(file.Path());
    const auto* skinned = SkinnedModelOf(*model);
    REQUIRE(skinned->LodCount() == 2);
    const auto& mesh = skinned->lods[0][0];
    CHECK(mesh.normalMap.image == skinned->meshNormalMaps[0].image);
    CHECK(mesh.normalMap.scale == doctest::Approx(0.5f));
    for (const auto& vertex : mesh.vertices)
        CHECK(std::fabs(vertex.bindTangent[3]) == 1.f);
}

// ---------------------------------------------------------------- loader validation

namespace
{
// Overwrites component `index` of a tightly packed FLOAT accessor.
void SetFloat(Glb& glb, int accessor, size_t index, float value)
{
    const auto& a = glb.json["accessors"][accessor];
    REQUIRE(a["componentType"].get<int>() == 5126);
    const auto& view = glb.json["bufferViews"][a["bufferView"].get<int>()];
    const size_t offset = view.value("byteOffset", 0) + a.value("byteOffset", 0) + index * sizeof(float);
    REQUIRE(offset + sizeof(float) <= glb.bin.size());
    std::memcpy(&glb.bin[offset], &value, sizeof(float));
}

int AttributeAccessor(const Glb& glb, const char* attribute)
{
    return glb.json["meshes"][0]["primitives"][0]["attributes"][attribute].get<int>();
}

// Moves node `node` from its parent's children to `parent`'s.
void Reparent(Glb& glb, int node, int parent)
{
    for (auto& n : glb.json["nodes"])
    {
        if (!n.contains("children"))
            continue;
        auto& children = n["children"];
        for (auto it = children.begin(); it != children.end(); ++it)
        {
            if (it->get<int>() == node)
            {
                children.erase(it);
                break;
            }
        }
    }
    glb.json["nodes"][parent]["children"].push_back(node);
}
} // namespace

TEST_CASE("glb_model: inverse bind matrices that are no FLOAT MAT4 are refused")
{
    Glb glb = Glb::Read(YetiGlb);
    const int ibm = glb.json["skins"][0]["inverseBindMatrices"].get<int>();
    TemporaryFile file;
    SUBCASE("SCALAR")
    {
        glb.json["accessors"][ibm]["type"] = "SCALAR";
    }
    SUBCASE("VEC3")
    {
        glb.json["accessors"][ibm]["type"] = "VEC3";
    }
    SUBCASE("UNSIGNED_SHORT MAT4")
    {
        glb.json["accessors"][ibm]["componentType"] = 5123;
    }
    glb.Write(file.Path());
    RequireRefused(file.Path(), "inverse bind matrices are no FLOAT MAT4");
}

TEST_CASE("glb_model: JOINTS_0 that is no unsigned integer is refused")
{
    Glb glb = Glb::Read(YetiGlb);
    const int joints = AttributeAccessor(glb, "JOINTS_0");
    const size_t count = glb.json["accessors"][joints]["count"];
    TemporaryFile file;
    SUBCASE("FLOAT")
    {
        glb.json["meshes"][0]["primitives"][0]["attributes"]["JOINTS_0"] =
            glb.AddAccessor(std::vector<float>(count * 4, 0.f), "VEC4", count);
    }
    SUBCASE("normalized")
    {
        glb.json["accessors"][joints]["normalized"] = true;
    }
    glb.Write(file.Path());
    RequireRefused(file.Path(), "JOINTS_0 is no UNSIGNED_BYTE or UNSIGNED_SHORT");
}

TEST_CASE("glb_model: a value that is not finite is refused")
{
    Glb glb = Glb::Read(YetiGlb);
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float infinity = std::numeric_limits<float>::infinity();
    TemporaryFile file;
    std::string expected;
    SUBCASE("POSITION")
    {
        SetFloat(glb, AttributeAccessor(glb, "POSITION"), 7, nan);
        expected = "POSITION holds a value that is not finite";
    }
    SUBCASE("NORMAL")
    {
        SetFloat(glb, AttributeAccessor(glb, "NORMAL"), 4, -infinity);
        expected = "NORMAL holds a value that is not finite";
    }
    SUBCASE("TANGENT")
    {
        AddNormalMap(glb, 1.f, [nan](size_t vertex) {
            return vertex == 5 ? std::array<float, 4>{nan, 0.f, 0.f, 1.f} : AlternatingTangent(vertex);
        });
        expected = "TANGENT holds a value that is not finite";
    }
    SUBCASE("WEIGHTS_0")
    {
        SetFloat(glb, AttributeAccessor(glb, "WEIGHTS_0"), 1, nan);
        expected = "WEIGHTS_0 holds a value that is not finite";
    }
    SUBCASE("inverse bind matrix")
    {
        SetFloat(glb, glb.json["skins"][0]["inverseBindMatrices"].get<int>(), 16 * 3 + 5, infinity);
        expected = "inverse bind matrices hold a value that is not finite";
    }
    SUBCASE("animation output")
    {
        SetFloat(glb, glb.json["animations"][0]["samplers"][0]["output"].get<int>(), 2, nan);
        expected = "a key value is not finite";
    }
    glb.Write(file.Path());
    RequireRefused(file.Path(), expected);
}

TEST_CASE("glb_model: a negative weight is refused")
{
    Glb glb = Glb::Read(YetiGlb);
    const int weights = AttributeAccessor(glb, "WEIGHTS_0");
    SetFloat(glb, weights, 0, 1.25f);
    SetFloat(glb, weights, 1, -0.25f);
    TemporaryFile file;
    glb.Write(file.Path());
    RequireRefused(file.Path(), "negative weight");
}

TEST_CASE("glb_model: a bone of the BMD below another parent than the BMD's is refused")
{
    Glb glb = Glb::Read(YetiGlb);
    const int head = glb.NodeNamed("Bip01 Head");
    TemporaryFile file;
    std::string expected;
    SUBCASE("below a later bone: its parent would be computed after it")
    {
        Reparent(glb, head, glb.NodeNamed("Bip01 R Hand"));
        expected = "\"Bip01 Head\" hangs below \"Bip01 R Hand\", in the BMD below \"Bip01 Neck\"";
    }
    SUBCASE("below an earlier bone")
    {
        Reparent(glb, head, glb.NodeNamed("Bip01 Pelvis"));
        expected = "\"Bip01 Head\" hangs below \"Bip01 Pelvis\", in the BMD below \"Bip01 Neck\"";
    }
    glb.Write(file.Path());
    RequireRefused(file.Path(), expected);
}

TEST_CASE("glb_model: a buffer with a URI is refused before it is read")
{
    Glb glb = Glb::Read(YetiGlb);
    TemporaryFile file;
    SUBCASE("an external file")
    {
        glb.json["buffers"][0]["uri"] = "Monster14.bin";
    }
    SUBCASE("a data URI")
    {
        glb.json["buffers"][0]["uri"] = "data:application/octet-stream;base64,AAAAAA==";
    }
    glb.Write(file.Path());
    RequireRefused(file.Path(), "buffer 0 names a URI");
}
