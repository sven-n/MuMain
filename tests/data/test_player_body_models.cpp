#include "stdafx.h"

#include "doctest.h"

#include "TestModelSlots.h"

#include "Core/Globals/_enum.h"
#include "Core/Text/Utf8.h"
#include "Data/DataHandler/LoadData.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Sprites/GpuUploadSkip.h"
#include "Render/Textures/ZzzTexture.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <cwctype>
#include <filesystem>
#include <set>
#include <string>

// The 13 selected S6 body families, read through the real model/texture loaders.
// GPU uploads alone are skipped: these are CPU compatibility checks, not rendering evidence.
namespace
{
const std::filesystem::path ClientFolder = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();
const std::filesystem::path PlayerFolder = std::filesystem::path(MU_TEST_DATA_DIR) / "Player";

// The role fixture below retains the original Dark Knight model.
constexpr int BodyClass = CLASS_KNIGHT;
struct BodyFamily
{
    const char* suffix;
    int baseClass;
    int stage;
    const char* stockTexture;
    const char* preparedTexture;
};

// OpenPlayers: base slots, MAX_CLASS + class for second forms (not MG/DL),
// 2 * MAX_CLASS + class for third forms. OpenPlayerTextures visits stage first.
constexpr std::array<BodyFamily, 13> Families{{
    {"01", CLASS_WIZARD, 0, "skin_barbarian_01.jpg", "skin_hq_class01.jpg"},
    {"02", CLASS_KNIGHT, 0, "skin_wizard_01.jpg", "skin_hq_class02.jpg"},
    {"03", CLASS_ELF, 0, "skin_warrior_01.jpg", "skin_hq_class03.jpg"},
    {"04", CLASS_DARK, 0, "skin_special_01.jpg", "skin_hq_class04.jpg"},
    {"05", CLASS_DARK_LORD, 0, "level_man02.jpg", "skin_hq_class05.jpg"},
    {"201", CLASS_WIZARD, 1, "level_man022.jpg", "skin_hq_class201.jpg"},
    {"202", CLASS_KNIGHT, 1, "level_man01.jpg", "skin_hq_class202.jpg"},
    {"203", CLASS_ELF, 1, "level_man033.jpg", "skin_hq_class203.jpg"},
    {"301", CLASS_WIZARD, 2, "SkinClass302.jpg", "body_hq_class301.jpg"},
    {"302", CLASS_KNIGHT, 2, "SkinClass301.jpg", "body_hq_class302.jpg"},
    {"303", CLASS_ELF, 2, "SkinClass303.jpg", "body_hq_class303.jpg"},
    {"304", CLASS_DARK, 2, "SkinClass304.jpg", "body_hq_class304.jpg"},
    {"305", CLASS_DARK_LORD, 2, "SkinClass305.jpg", "body_hq_class305.jpg"},
}};
// A shipped texture the role fixture copies; only its name is under test.
constexpr const char* RoleFixtureSource = "skin_wizard_01.OZJ";
// The player's head bone, as OpenPlayers sets it (ZzzOpenData.cpp).
constexpr int PlayerHeadBone = 20;
// Model units; a body part spans tens of them.
constexpr float MinimumPartExtent = 1.f;
constexpr float MaximumCoordinate = 10000.f;
constexpr float MaximumTexCoord = 16.f;

struct BodyPart
{
    int slot;
    const wchar_t* file;
};

// The slots and file names OpenPlayers gives the first-form body parts.
const std::array<BodyPart, 5> BodyParts{{
    {MODEL_BODY_HELM, L"HelmClass"},
    {MODEL_BODY_ARMOR, L"ArmorClass"},
    {MODEL_BODY_PANTS, L"PantClass"},
    {MODEL_BODY_GLOVES, L"GloveClass"},
    {MODEL_BODY_BOOTS, L"BootClass"},
}};

// Runs the test from the client folder, where model and texture paths start with Data.
class ClientFolderScope
{
public:
    explicit ClientFolderScope(const std::filesystem::path& folder) : m_previous(std::filesystem::current_path())
    {
        std::filesystem::current_path(folder);
    }
    ~ClientFolderScope()
    {
        std::filesystem::current_path(m_previous);
    }
    ClientFolderScope(const ClientFolderScope&) = delete;
    ClientFolderScope& operator=(const ClientFolderScope&) = delete;

private:
    std::filesystem::path m_previous;
};

// Textures load without a GPU device and are unloaded afterwards.
class TextureScope
{
public:
    TextureScope()
    {
        Bitmaps.UnloadAllImages();
        Render::Sprites::SetGpuUploadsSkipped(true);
    }
    ~TextureScope()
    {
        Bitmaps.UnloadAllImages();
        Render::Sprites::SetGpuUploadsSkipped(false);
    }
    TextureScope(const TextureScope&) = delete;
    TextureScope& operator=(const TextureScope&) = delete;
};

BMD& OpenPlayer()
{
    REQUIRE(gLoadData.AccessModel(MODEL_PLAYER, L"Data\\Player\\", L"Player"));
    BMD& player = Models[MODEL_PLAYER];
    player.BoneHead = PlayerHeadBone;
    return player;
}

int ModelSlot(const BodyFamily& family, const BodyPart& part)
{
    return part.slot + family.stage * MAX_CLASS + family.baseClass;
}

std::wstring ModelPrefix(const BodyFamily& family, const BodyPart& part)
{
    return std::wstring(part.file) + (family.stage == 0 ? L"" : std::to_wstring(family.stage + 1));
}

void OpenBodyParts(const BodyFamily& family)
{
    for (const BodyPart& part : BodyParts)
    {
        INFO("family ", std::string(family.suffix), " part ", Core::Text::ToUtf8(part.file));
        const std::wstring prefix = ModelPrefix(family, part);
        REQUIRE(gLoadData.AccessModel(ModelSlot(family, part), L"Data\\Player\\", prefix.c_str(), family.baseClass + 1));
    }
}

std::string MeshTextureName(const BMD& model, int mesh)
{
    const char* name = model.Textures[mesh].FileName;
    return std::string(name, strnlen(name, sizeof(model.Textures[mesh].FileName)));
}

bool IsPrepared(const BodyFamily& family)
{
    const std::string name = MeshTextureName(Models[ModelSlot(family, BodyParts[0])], 0);
    REQUIRE((name == family.stockTexture || name == family.preparedTexture));
    return name == family.preparedTexture;
}

bool IsHairMesh(const BodyFamily& family, const BodyPart& part, int mesh)
{
    return family.baseClass == CLASS_DARK_LORD && part.slot == MODEL_BODY_HELM && mesh == 1;
}

std::string ExpectedTexture(const BodyFamily& family, const BodyPart& part, int mesh, bool prepared)
{
    if (IsHairMesh(family, part, mesh))
        return prepared ? "hair_hq_player.jpg" : "hair_R.jpg";
    return prepared ? family.preparedTexture : family.stockTexture;
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
    const auto addWithAncestors = [&](int bone) {
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
} // namespace

TEST_CASE("All 13 body families load 65 parts through native slots and fit the player rig [data][player]")
{
    TestModelSlots slots;
    ClientFolderScope folder(ClientFolder);
    const BMD& player = OpenPlayer();
    REQUIRE(player.NumBones > 0);
    int parts = 0;
    int meshes = 0;
    for (const BodyFamily& family : Families)
    {
        INFO("family ", std::string(family.suffix));
        OpenBodyParts(family);
        const bool prepared = IsPrepared(family);
        for (const BodyPart& part : BodyParts)
        {
            INFO("part ", Core::Text::ToUtf8(part.file));
            const BMD& model = Models[ModelSlot(family, part)];
            REQUIRE(model.NumMeshs == (IsHairMesh(family, part, 1) ? 2 : 1));
            REQUIRE(model.NumMeshs <= MAX_MESH);
            for (int m = 0; m < model.NumMeshs; ++m)
            {
                INFO("mesh ", m);
                CHECK(MeshTextureName(model, m) == ExpectedTexture(family, part, m, prepared));
                CHECK(model.Meshs[m].Texture >= 0);
                CHECK(model.Meshs[m].Texture < model.NumMeshs);
                CheckMeshVertices(model.Meshs[m], player);
                CheckMeshNormalsAndTexCoords(model.Meshs[m], player);
                CheckMeshTriangles(model.Meshs[m]);
                ++meshes;
            }
            CheckSkinnedBonesMatchPlayer(model, player);
            ++parts;
            MESSAGE("rig family=", std::string(family.suffix), " part=", Core::Text::ToUtf8(part.file), " meshes=", model.NumMeshs);
        }
    }
    CHECK(parts == 65);
    CHECK(meshes == 67);
    MESSAGE("rig totals families=", Families.size(), " parts=", parts, " meshes=", meshes);
}

TEST_CASE("All 65 body parts skin to finite positions with the retained player palette [data][player]")
{
    TestModelSlots slots;
    ClientFolderScope folder(ClientFolder);
    BMD& player = OpenPlayer();
    static float bones[MAX_BONES][3][4];
    int skinnedParts = 0;
    for (const BodyFamily& family : Families)
    {
        INFO("family ", std::string(family.suffix));
        OpenBodyParts(family);
        auto poses = Poses;
        if (family.baseClass == CLASS_ELF)
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
            for (const BodyPart& part : BodyParts)
            {
                INFO("part ", Core::Text::ToUtf8(part.file));
                CheckPartSkinsInPose(Models[ModelSlot(family, part)], bones);
                ++skinnedParts;
                MESSAGE("skin family=", std::string(family.suffix), " part=", Core::Text::ToUtf8(part.file), " pose=", std::string(pose.name));
            }
        }
    }
    CHECK(skinnedParts == 65 * Poses.size());
    MESSAGE("skin totals families=", Families.size(), " parts=65 poses=", Poses.size(), " part_poses=", skinnedParts);
}

TEST_CASE("All body mesh textures decode with native roles and shared Dark Lord hair caching [data][player]")
{
    TestModelSlots slots;
    ClientFolderScope folder(ClientFolder);
    TextureScope textures;
    // OpenPlayerTextures preloads this bitmap before visiting the body slots.
    REQUIRE(LoadBitmap(L"Player\\hair_r.jpg", BITMAP_HAIR));
    const BITMAP_t* originalHair = Bitmaps.FindTexture(BITMAP_HAIR);
    REQUIRE(originalHair != nullptr);
    const BITMAP_t* sharedHair = nullptr;
    std::set<std::string> dependencies;
    int parts = 0;
    int references = 0;
    for (const BodyFamily& family : Families)
    {
        INFO("family ", std::string(family.suffix));
        OpenBodyParts(family);
        const bool prepared = IsPrepared(family);
        for (const BodyPart& part : BodyParts)
        {
            const int slot = ModelSlot(family, part);
            INFO("part ", Core::Text::ToUtf8(part.file));
            gLoadData.OpenTexture(slot, L"Player\\");
            const BMD& model = Models[slot];
            for (int m = 0; m < model.NumMeshs; ++m)
            {
                const std::string name = ExpectedTexture(family, part, m, prepared);
                CHECK(MeshTextureName(model, m) == name);
                const BITMAP_t* bitmap = MeshBitmap(model, m);
                REQUIRE(bitmap->Width > 0.f);
                REQUIRE(bitmap->Height > 0.f);
                std::wstring path = bitmap->FileName;
                std::wstring expected = L"Data\\Player\\" + Core::Text::FromUtf8(name);
                const auto lower = [](wchar_t c) { return std::towlower(c); };
                std::transform(path.begin(), path.end(), path.begin(), lower);
                std::transform(expected.begin(), expected.end(), expected.begin(), lower);
                CHECK(path == expected);
                const bool hair = IsHairMesh(family, part, m);
                CHECK(bitmap->IsSkin == (!hair && family.stage < 2));
                CHECK(bitmap->IsHair == hair);
                if (hair)
                {
                    CHECK((bitmap == originalHair) == !prepared);
                    if (sharedHair != nullptr)
                    {
                        CHECK(bitmap == sharedHair);
                        MESSAGE("shared hair cache reused by family=", std::string(family.suffix));
                    }
                    sharedHair = bitmap;
                    CHECK_FALSE(sharedHair->IsSkin);
                    CHECK(sharedHair->IsHair);
                }
                dependencies.insert(name);
                ++references;
                MESSAGE("texture family=", std::string(family.suffix), " part=", Core::Text::ToUtf8(part.file), " mesh=", m,
                        " name=", name, " decoded=", bitmap->Width, "x", bitmap->Height,
                        " skin=", bitmap->IsSkin, " hair=", bitmap->IsHair);
            }
            ++parts;
        }
    }
    REQUIRE(sharedHair != nullptr);
    CHECK(sharedHair->IsHair);
    CHECK_FALSE(sharedHair->IsSkin);
    CHECK(parts == 65);
    CHECK(references == 67);
    CHECK(dependencies.size() == 14);
    MESSAGE("texture totals families=", Families.size(), " parts=", parts, " references=", references,
            " dependencies=", dependencies.size(), " GPU uploads skipped");
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
    TextureScope textures;
    const int slot = BodyParts[0].slot + BodyClass;
    {
        ClientFolderScope source(ClientFolder);
        REQUIRE(gLoadData.AccessModel(slot, L"Data\\Player\\", BodyParts[0].file, BodyClass + 1));
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
