#include "stdafx.h"

#include "doctest.h"

#include "ClientFolderScope.h"
#include "TestModelSlots.h"
#include "PlayerTextureScope.h"
#include "Engine/Object/PlayerBodyModels.h"
#include "Data/GameData/ItemData/ItemTextureFiles.h"

#include "Core/Globals/_enum.h"
#include "Core/Text/Utf8.h"
#include "Data/DataHandler/LoadData.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Sprites/TextureScript.h"
#include "Render/Textures/ZzzTexture.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <set>
#include <string>
#include <vector>

// Every improved body found on disk is checked against its original, through the real loaders.
// GPU uploads alone are skipped: CPU compatibility checks, not rendering evidence.
namespace
{
using Engine::Object::PlayerBodyPart;
using Engine::Object::PlayerBodyParts;
const std::filesystem::path ClientFolder = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();
const std::filesystem::path PlayerFolder = std::filesystem::path(MU_TEST_DATA_DIR) / "Player";
constexpr int BodyClass = CLASS_KNIGHT;
constexpr const char* RoleFixtureSource = "skin_wizard_01.OZJ";
constexpr int PlayerHeadBone = 20;
constexpr float MinimumPartExtent = 1.f;
constexpr float MaximumCoordinate = 10000.f;
constexpr float MaximumTexCoord = 16.f;
constexpr int OriginalSlot = MODEL_BODY_HELM;
constexpr int ImprovedSlot = MODEL_BODY_ARMOR;

struct BodyFile
{
    std::wstring stem;
    int baseClass;
};

std::vector<BodyFile> FindImprovedBodies(const std::filesystem::path& playerFolder)
{
    std::vector<BodyFile> bodies;
    for (const auto& entry : std::filesystem::directory_iterator(playerFolder / "Updated"))
    {
        if (!entry.is_regular_file() || !Data::Items::EqualsIgnoringCase(entry.path().extension().string(), ".bmd"))
            continue;
        const std::wstring stem = entry.path().stem().wstring();
        bool recognized = false;
        for (int stage = 0; stage < MAX_CLASS_STAGES; ++stage)
        {
            for (int baseClass = 0; baseClass < MAX_CLASS; ++baseClass)
            {
                if (!Engine::Object::HasPlayerBodyForm(stage, baseClass))
                    continue;
                for (const PlayerBodyPart& part : PlayerBodyParts)
                {
                    if (stem == Engine::Object::PlayerBodyFileName(stage, part, baseClass))
                    {
                        REQUIRE(std::filesystem::is_regular_file(playerFolder / entry.path().filename()));
                        bodies.push_back({stem, baseClass});
                        recognized = true;
                    }
                }
            }
        }
        INFO(entry.path().string());
        CHECK(recognized);
    }
    std::ranges::sort(bodies, {}, &BodyFile::stem);
    REQUIRE_FALSE(bodies.empty());
    return bodies;
}

void OpenBodyPair(const BodyFile& body)
{
    Models[OriginalSlot].Release();
    Models[ImprovedSlot].Release();
    REQUIRE(gLoadData.AccessModel(OriginalSlot, Engine::Object::OriginalPlayerModelFolder, body.stem.c_str()));
    REQUIRE(gLoadData.AccessModel(ImprovedSlot, Engine::Object::ImprovedPlayerModelFolder, body.stem.c_str()));
}

BMD& OpenPlayer()
{
    REQUIRE(gLoadData.AccessModel(MODEL_PLAYER, L"Data\\Player\\", L"Player"));
    BMD& player = Models[MODEL_PLAYER];
    player.BoneHead = PlayerHeadBone;
    return player;
}

std::string MeshTextureName(const BMD& model, int mesh)
{
    const char* name = model.Textures[mesh].FileName;
    return std::string(name, strnlen(name, sizeof(model.Textures[mesh].FileName)));
}

bool IsSkin(std::string_view name)
{
    return name.starts_with("ski") || (name.size() >= 5 && Data::Items::EqualsIgnoringCase(name.substr(0, 5), "level"));
}

bool IsHair(std::string_view name)
{
    return name.starts_with("hair");
}

bool IsFinite(const float* values, int count)
{
    for (int i = 0; i < count; ++i)
    {
        if (!std::isfinite(values[i]))
            return false;
    }
    return true;
}

// A vertex follows a bone of the player's rig that the animation moves.
void CheckRigBone(const BMD& player, int node)
{
    REQUIRE(node >= 0);
    REQUIRE(node < player.NumBones);
    CHECK_FALSE(player.Bones[node].Dummy);
}

// The bones the part's vertices and normals follow, with their ancestors in the player's rig.
std::set<int> SkinnedBonesWithAncestors(const BMD& part, const BMD& player)
{
    std::set<int> bones;
    const auto addWithAncestors = [&](int bone)
    {
        for (int step = 0; bone >= 0 && bone < player.NumBones && step < player.NumBones; ++step)
        {
            bones.insert(bone);
            bone = player.Bones[bone].Parent;
        }
    };
    for (int m = 0; m < part.NumMeshs; ++m)
    {
        const Mesh_t& mesh = part.Meshs[m];
        for (int v = 0; v < mesh.NumVertices; ++v)
            addWithAncestors(mesh.Vertices[v].Node);
        for (int n = 0; n < mesh.NumNormals; ++n)
            addWithAncestors(mesh.Normals[n].Node);
    }
    return bones;
}

// The part was modelled on the player's skeleton: its copy of every bone it is
// skinned to, and of their ancestors, has the player's name and parent. Bones
// the part does not use may differ (the shipped parts carry extra ones).
void CheckSkinnedBonesMatchPlayer(const BMD& part, const BMD& player)
{
    const std::set<int> bones = SkinnedBonesWithAncestors(part, player);
    REQUIRE_FALSE(bones.empty());
    for (const int bone : bones)
    {
        INFO("bone ", bone);
        const Bone_t& rig = player.Bones[bone];
        REQUIRE_FALSE(rig.Dummy);
        REQUIRE(bone < part.NumBones);
        const Bone_t& own = part.Bones[bone];
        CHECK_FALSE(own.Dummy);
        CHECK(std::strncmp(own.Name, rig.Name, sizeof(own.Name)) == 0);
        CHECK(own.Parent == rig.Parent);
    }
}

void CheckMeshVertices(const Mesh_t& mesh, const BMD& player)
{
    REQUIRE(mesh.NumVertices > 0);
    REQUIRE(mesh.NumVertices <= MAX_VERTICES);
    vec3_t low{MaximumCoordinate, MaximumCoordinate, MaximumCoordinate};
    vec3_t high{-MaximumCoordinate, -MaximumCoordinate, -MaximumCoordinate};
    for (int v = 0; v < mesh.NumVertices; ++v)
    {
        const Vertex_t& vertex = mesh.Vertices[v];
        CheckRigBone(player, vertex.Node);
        REQUIRE(IsFinite(vertex.Position, 3));
        for (int axis = 0; axis < 3; ++axis)
        {
            CHECK(std::abs(vertex.Position[axis]) < MaximumCoordinate);
            low[axis] = std::min(low[axis], vertex.Position[axis]);
            high[axis] = std::max(high[axis], vertex.Position[axis]);
        }
    }
    CHECK(high[0] - low[0] + high[1] - low[1] + high[2] - low[2] > MinimumPartExtent);
}

void CheckMeshNormalsAndTexCoords(const Mesh_t& mesh, const BMD& player)
{
    for (int n = 0; n < mesh.NumNormals; ++n)
    {
        const Normal_t& normal = mesh.Normals[n];
        CheckRigBone(player, normal.Node);
        CHECK(IsFinite(normal.Normal, 3));
        CHECK(normal.BindVertex >= 0);
        CHECK(normal.BindVertex < mesh.NumVertices);
    }
    for (int t = 0; t < mesh.NumTexCoords; ++t)
    {
        const TexCoord_t& uv = mesh.TexCoords[t];
        CHECK(std::isfinite(uv.TexCoordU));
        CHECK(std::isfinite(uv.TexCoordV));
        CHECK(std::abs(uv.TexCoordU) <= MaximumTexCoord);
        CHECK(std::abs(uv.TexCoordV) <= MaximumTexCoord);
    }
}

void CheckMeshTriangles(const Mesh_t& mesh)
{
    REQUIRE(mesh.NumTriangles > 0);
    for (int t = 0; t < mesh.NumTriangles; ++t)
    {
        const Triangle_t& triangle = mesh.Triangles[t];
        INFO("triangle ", t);
        REQUIRE((triangle.Polygon == 3 || triangle.Polygon == 4));
        for (int corner = 0; corner < triangle.Polygon; ++corner)
        {
            CHECK(triangle.VertexIndex[corner] >= 0);
            CHECK(triangle.VertexIndex[corner] < mesh.NumVertices);
            CHECK(triangle.NormalIndex[corner] >= 0);
            CHECK(triangle.NormalIndex[corner] < mesh.NumNormals);
            CHECK(triangle.TexCoordIndex[corner] >= 0);
            CHECK(triangle.TexCoordIndex[corner] < mesh.NumTexCoords);
        }
    }
}

struct Pose
{
    const char* name;
    unsigned short action;
    vec3_t headAngle;
};

// Idle, walking, a sword swing, and idle with the head turned.
const std::array<Pose, 4> Poses{{
    {"idle", PLAYER_STOP_MALE, {0.f, 0.f, 0.f}},
    {"walk", PLAYER_WALK_MALE, {0.f, 0.f, 0.f}},
    {"attack", PLAYER_ATTACK_SWORD_RIGHT1, {0.f, 0.f, 0.f}},
    {"head turn", PLAYER_STOP_MALE, {20.f, 0.f, 40.f}},
}};

void CheckPartSkinsInPose(const BMD& part, float (*bones)[3][4])
{
    for (int m = 0; m < part.NumMeshs; ++m)
    {
        for (int v = 0; v < part.Meshs[m].NumVertices; ++v)
        {
            vec3_t skinned;
            part.SkinVertex(m, v, bones, false, 0.f, skinned);
            REQUIRE(IsFinite(skinned, 3));
            CHECK(std::abs(skinned[0]) + std::abs(skinned[1]) + std::abs(skinned[2]) < 3 * MaximumCoordinate);
        }
    }
}

const BITMAP_t* MeshBitmap(const BMD& model, int mesh)
{
    const GLuint index = model.IndexTexture[mesh];
    REQUIRE(index != BITMAP_UNKNOWN);
    const BITMAP_t* bitmap = Bitmaps.FindTexture(index);
    REQUIRE(bitmap != nullptr);
    return bitmap;
}

void CheckBodyCompatibility(const std::filesystem::path& playerFolder)
{
    const auto bodies = FindImprovedBodies(playerFolder);
    TestModelSlots slots;
    ClientFolderScope folder(playerFolder.parent_path().parent_path());
    const BMD& player = OpenPlayer();
    REQUIRE(player.NumBones > 0);
    for (const BodyFile& body : bodies)
    {
        INFO(Core::Text::ToUtf8(body.stem.c_str()));
        OpenBodyPair(body);
        const BMD& original = Models[OriginalSlot];
        const BMD& improved = Models[ImprovedSlot];
        REQUIRE(improved.NumMeshs == original.NumMeshs);
        for (const BMD* model : {&original, &improved})
        {
            REQUIRE(model->NumMeshs > 0);
            REQUIRE(model->NumMeshs <= MAX_MESH);
            for (int m = 0; m < model->NumMeshs; ++m)
            {
                INFO("mesh ", m);
                CHECK(model->Meshs[m].Texture >= 0);
                CHECK(model->Meshs[m].Texture < model->NumMeshs);
                CheckMeshVertices(model->Meshs[m], player);
                CheckMeshNormalsAndTexCoords(model->Meshs[m], player);
                CheckMeshTriangles(model->Meshs[m]);
            }
            CheckSkinnedBonesMatchPlayer(*model, player);
        }
        for (int m = 0; m < original.NumMeshs; ++m)
        {
            INFO("mesh ", m);
            CHECK(IsSkin(MeshTextureName(improved, m)) == IsSkin(MeshTextureName(original, m)));
            CHECK(IsHair(MeshTextureName(improved, m)) == IsHair(MeshTextureName(original, m)));
        }
        MESSAGE("rig pair=", Core::Text::ToUtf8(body.stem.c_str()), " meshes=", improved.NumMeshs);
    }
    MESSAGE("rig body pairs=", bodies.size());
}

void CheckBodySkinning()
{
    TestModelSlots slots;
    ClientFolderScope folder(ClientFolder);
    BMD& player = OpenPlayer();
    static float bones[MAX_BONES][3][4];
    for (const BodyFile& body : FindImprovedBodies(PlayerFolder))
    {
        INFO(Core::Text::ToUtf8(body.stem.c_str()));
        OpenBodyPair(body);
        auto poses = Poses;
        if (body.baseClass == CLASS_ELF)
        {
            poses[0].action = PLAYER_STOP_FEMALE;
            poses[1].action = PLAYER_WALK_FEMALE;
            poses[3].action = PLAYER_STOP_FEMALE;
        }
        for (const Pose& pose : poses)
        {
            INFO("pose ", std::string(pose.name));
            REQUIRE(pose.action < player.NumActions);
            const int keys = player.Actions[pose.action].NumAnimationKeys;
            REQUIRE(keys > 0);
            vec3_t angle{0.f, 0.f, 45.f};
            vec3_t headAngle;
            VectorCopy(pose.headAngle, headAngle);
            Vector(0.f, 0.f, 0.f, player.BodyOrigin);
            player.BodyScale = 1.f;
            const float frame = keys > 1 ? 0.5f * static_cast<float>(keys - 1) : 0.f;
            player.Animation(bones, frame, frame, pose.action, angle, headAngle, false, false, nullptr, pose.action);
            REQUIRE(IsFinite(&bones[0][0][0], player.NumBones * 12));
            CheckPartSkinsInPose(Models[OriginalSlot], bones);
            CheckPartSkinsInPose(Models[ImprovedSlot], bones);
        }
    }
}

void CheckBodyTextures()
{
    PlayerTextureScope textures;
    TestModelSlots slots;
    ClientFolderScope folder(ClientFolder);
    // OpenPlayerTextures preloads this bitmap before visiting the body slots.
    REQUIRE(LoadBitmap(L"Player\\hair_r.jpg", BITMAP_HAIR));
    const BITMAP_t* originalHair = Bitmaps.FindTexture(BITMAP_HAIR);
    REQUIRE(originalHair != nullptr);
    for (const BodyFile& body : FindImprovedBodies(PlayerFolder))
    {
        INFO(Core::Text::ToUtf8(body.stem.c_str()));
        OpenBodyPair(body);
        for (const int slot : {OriginalSlot, ImprovedSlot})
        {
            gLoadData.OpenTexture(slot, L"Player\\");
            const BMD& model = Models[slot];
            for (int m = 0; m < model.NumMeshs; ++m)
            {
                const std::string name = MeshTextureName(model, m);
                INFO("mesh ", m, " texture ", name);
                const BITMAP_t* bitmap = MeshBitmap(model, m);
                REQUIRE(bitmap->Width > 0.f);
                REQUIRE(bitmap->Height > 0.f);
                CHECK(Data::Items::EqualsIgnoringCase(Core::Text::ToUtf8(bitmap->FileName), "Data\\Player\\" + name));
                CHECK(bitmap->IsSkin == IsSkin(name));
                CHECK(bitmap->IsHair == IsHair(name));
                // Native cache reuses the same bitmap, with one additional reference.
                const int before = bitmap->Ref;
                const GLuint again = Bitmaps.LoadImage(bitmap->FileName, GL_NEAREST, GL_REPEAT);
                CHECK(again == bitmap->BitmapIndex);
                CHECK(bitmap->Ref == before + 1);
                Bitmaps.UnloadImage(again);
                if (IsHair(name) && body.baseClass == CLASS_DARK_LORD)
                    CHECK((bitmap == originalHair) == (slot == OriginalSlot));
            }
        }
    }
}

// The drawing flags the client reads from a mesh's texture name (TextureScriptParsing):
// glowing, hidden, stream, unblended and shadow.
struct MeshFlags
{
    bool bright = false;
    bool hidden = false;
    bool stream = false;
    bool noneBlend = false;
    BYTE shadow = SHADOW_NONE;

    bool operator==(const MeshFlags&) const = default;
};

MeshFlags ReadMeshFlags(const BMD& model, int mesh)
{
    TextureScript* script = model.Meshs[mesh].m_csTScript;
    if (script == nullptr)
        return {};
    return {script->getBright(), script->getHiddenMesh(), script->getStreamMesh(), script->getNoneBlendMesh(),
            script->getShadowMesh()};
}

} // namespace

TEST_CASE("Every discovered improved body keeps its original mesh drawing flags [data][player]")
{
    TestModelSlots slots;
    ClientFolderScope folder(ClientFolder);
    for (const BodyFile& body : FindImprovedBodies(PlayerFolder))
    {
        INFO(Core::Text::ToUtf8(body.stem.c_str()));
        OpenBodyPair(body);
        const BMD& original = Models[OriginalSlot];
        const BMD& improved = Models[ImprovedSlot];
        REQUIRE(improved.NumMeshs == original.NumMeshs);
        for (int m = 0; m < original.NumMeshs; ++m)
        {
            INFO("mesh ", m, " original ", MeshTextureName(original, m), " improved ", MeshTextureName(improved, m));
            CHECK(ReadMeshFlags(improved, m) == ReadMeshFlags(original, m));
        }
    }
}

TEST_CASE("Every discovered body pair fits the player rig and keeps mesh roles [data][player]")
{
    CheckBodyCompatibility(PlayerFolder);
}

TEST_CASE("Every discovered body pair skins with the retained player palette [data][player]")
{
    CheckBodySkinning();
}

TEST_CASE("Every discovered body texture decodes with native roles and caching [data][player]")
{
    CheckBodyTextures();
}

namespace
{
// A copy of the shipped body texture under each of `names`, in a fixture
// folder below Data; returns that folder relative to Data.
struct RoleFixture
{
    RoleFixture() : client(std::filesystem::current_path() / "player_body_roles")
    {
        std::filesystem::remove_all(client);
        std::filesystem::create_directories(client / "Data" / SubFolder);
    }
    ~RoleFixture()
    {
        std::error_code error;
        std::filesystem::remove_all(client, error);
    }
    RoleFixture(const RoleFixture&) = delete;
    RoleFixture& operator=(const RoleFixture&) = delete;

    void Add(const std::string& stem) const
    {
        std::filesystem::copy_file(PlayerFolder / RoleFixtureSource, client / "Data" / SubFolder / (stem + ".OZJ"));
    }

    static constexpr const char* SubFolder = "PlayerBodyRoles";
    std::filesystem::path client;
};

// Gives the first mesh of the slot the texture name and loads it the way
// the game loads model textures.
const BITMAP_t* LoadMeshTexture(BMD& model, int slot, const char* name)
{
    std::strncpy(model.Textures[0].FileName, name, sizeof(model.Textures[0].FileName) - 1);
    model.IndexTexture[0] = BITMAP_UNKNOWN;
    gLoadData.OpenTexture(slot, L"PlayerBodyRoles\\");
    return MeshBitmap(model, 0);
}
} // namespace

TEST_CASE("Model texture names decide the skin and hair roles of their bitmaps [data][player]")
{
    TestModelSlots slots;
    RoleFixture fixture;
    for (const char* stem : {"skin_role", "Skin_fresh", "hair_role", "LeVeL_role", "skin_cached"})
    {
        fixture.Add(stem);
    }
    ClientFolderScope folder(fixture.client);
    PlayerTextureScope textures;
    const int slot = PlayerBodyParts[0].firstSlot + BodyClass;
    {
        ClientFolderScope source(ClientFolder);
        REQUIRE(gLoadData.AccessModel(slot, L"Data\\Player\\", PlayerBodyParts[0].filePrefix, BodyClass + 1));
    }
    BMD& model = Models[slot];

    const BITMAP_t* skin = LoadMeshTexture(model, slot, "skin_role.jpg");
    CHECK(skin->IsSkin);
    CHECK_FALSE(skin->IsHair);

    const BITMAP_t* fresh = LoadMeshTexture(model, slot, "Skin_fresh.jpg");
    CHECK_FALSE(fresh->IsSkin);
    CHECK_FALSE(fresh->IsHair);

    const BITMAP_t* hair = LoadMeshTexture(model, slot, "hair_role.jpg");
    CHECK_FALSE(hair->IsSkin);
    CHECK(hair->IsHair);

    const BITMAP_t* level = LoadMeshTexture(model, slot, "LeVeL_role.jpg");
    CHECK(level->IsSkin);
    CHECK_FALSE(level->IsHair);

    // A name without a role leaves the flags of the bitmap it names alone:
    // the same file loaded first as skin stays skin.
    const BITMAP_t* cached = LoadMeshTexture(model, slot, "skin_cached.jpg");
    REQUIRE(cached->IsSkin);
    const BITMAP_t* recased = LoadMeshTexture(model, slot, "Skin_Cached.jpg");
    CHECK(recased == cached);
    CHECK(recased->IsSkin);
    CHECK_FALSE(recased->IsHair);
}

TEST_CASE("An additional Summoner body is discovered and checked without a family-table edit [data][player]")
{
    RoleFixture fixture;
    const auto player = fixture.client / "Data" / "Player";
    std::filesystem::create_directories(player / "Updated");
    const auto file = Engine::Object::PlayerBodyFileName(0, PlayerBodyParts[0], CLASS_SUMMONER) + L".bmd";
    std::filesystem::copy_file(PlayerFolder / "player.bmd", player / "player.bmd");
    std::filesystem::copy_file(PlayerFolder / file, player / file);
    std::filesystem::copy_file(PlayerFolder / file, player / "Updated" / file);
    const auto bodies = FindImprovedBodies(player);
    REQUIRE(bodies.size() == 1);
    CHECK(bodies[0].baseClass == CLASS_SUMMONER);
    CheckBodyCompatibility(player);
}
