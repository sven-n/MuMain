#include "stdafx.h"
#include "doctest.h"

#include "Engine/AI/GOBoid.h"
#include "Engine/Object/CharacterAttachments.h"
#include "Engine/Object/ObjectPose.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "GameLogic/Pets/CSPetSystem.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Models/ZzzBMD.h"
#include "World/MapInfra/w_MapProcess.h"

#include <cstring>
#include <memory>

namespace
{
// One drawn character of the given model, alone in the client's character list.
struct CharacterScope
{
    std::unique_ptr<BMD[]> models{new BMD[MAX_MODELS]};
    std::unique_ptr<CHARACTER[]> characters{new CHARACTER[MAX_CHARACTERS_CLIENT]{}};
    std::unique_ptr<vec34_t[]> bones{new vec34_t[MAX_BONES]{}};
    BMD* previousModels = Models;
    CHARACTER* previousCharacters = CharactersClient;
    CHARACTER* previousHero = Hero;
    MapProcessPtr previousMapProcess = g_MapProcess;
    float previousEarthQuake = EarthQuake;

    explicit CharacterScope(int type)
    {
        Models = models.get();
        CharactersClient = characters.get();
        Hero = nullptr;
        g_MapProcess = MapProcess::Make();
        Models[type].NumActions = 1;

        OBJECT& o = character().Object;
        o.Live = true;
        o.Visible = true;
        o.Type = type;
        o.Scale = 1.f;
        o.BoneTransform = bones.get();
    }

    ~CharacterScope()
    {
        character().Object.BoneTransform = nullptr;
        EarthQuake = previousEarthQuake;
        g_MapProcess = previousMapProcess;
        Hero = previousHero;
        CharactersClient = previousCharacters;
        Models = previousModels;
    }

    CHARACTER& character()
    {
        return characters[1];
    }
};

// A Cursed Temple statue that dies this frame: its effects shake the camera, on any map.
// A red Fenrir under a rider, its own world pose already built: every bone sits at `poseAt`, every bone of the
// shared array at `sharedAt`.
struct FenrirScope
{
    static constexpr float poseAt = 500.f;
    static constexpr float sharedAt = -90000.f;
    std::unique_ptr<BMD[]> models{new BMD[MAX_MODELS]};
    std::unique_ptr<vec34_t[]> bones{new vec34_t[MAX_BONES]{}};
    std::unique_ptr<vec34_t[]> savedShared{new vec34_t[MAX_BONES]};
    BMD* previousModels = Models;
    float previousFrameFactor = FPS_ANIMATION_FACTOR;
    OBJECT rider;
    OBJECT mount;

    FenrirScope()
    {
        Models = models.get();
        Models[MODEL_FENRIR_RED].NumActions = 1;
        FPS_ANIMATION_FACTOR = 1.f; // one reference frame per update: every frame-checked spawn runs
        std::memcpy(savedShared.get(), BoneTransform, sizeof(vec34_t) * MAX_BONES);
        for (int i = 0; i < MAX_BONES; ++i)
        {
            for (int axis = 0; axis < 3; ++axis)
            {
                bones[i][axis][axis] = 1.f;
                bones[i][axis][3] = poseAt;
                BoneTransform[i][axis][3] = sharedAt;
            }
        }

        rider.Live = true;
        rider.Type = MODEL_PLAYER;
        mount.Live = true;
        mount.Type = MODEL_FENRIR_RED;
        mount.Owner = &rider;
        mount.Alpha = 1.f;
        mount.Scale = 1.f;
        mount.HiddenMesh = -1;
        mount.BoneTransform = bones.get();
        mount.EnableBoneMatrix = true;
        PlaceObjectModel(&mount, &Models[mount.Type]);
        Engine::Object::Pose::Record(&mount, &Models[mount.Type], true);
        ClearSpawns();
    }

    ~FenrirScope()
    {
        ClearSpawns();
        mount.BoneTransform = nullptr;
        std::memcpy(BoneTransform, savedShared.get(), sizeof(vec34_t) * MAX_BONES);
        FPS_ANIMATION_FACTOR = previousFrameFactor;
        Models = previousModels;
    }

    static void ClearSpawns()
    {
        for (int i = 0; i < MAX_SPRITES; ++i)
            Sprites[i].Live = false;
        for (int i = 0; i < MAX_EFFECTS; ++i)
            Effects[i].Live = false;
    }
};

struct DyingStatueScope : CharacterScope
{
    DyingStatueScope() : CharacterScope(MODEL_CURSEDTEMPLE_STATUE)
    {
        character().Object.CurrentAction = MONSTER01_DIE;
        character().Object.PKKey = 0;
    }
};
} // namespace

TEST_CASE("A camera shake the monster effects request reaches the camera after the next damping [gamelogic]")
{
    DyingStatueScope scope;
    constexpr float sampled = 5.f; // no statue shake is this strong
    EarthQuake = sampled;

    UpdateCharacterEffects();

    CHECK(scope.character().Object.PKKey == 1); // its death effects ran
    CHECK(EarthQuake == sampled);            // the camera of this frame still sees last frame's shake

    ApplyCharacterEffectsEarthQuake();
    CHECK(EarthQuake != sampled);
    CHECK(EarthQuake >= -0.8f);
    CHECK(EarthQuake <= 0.7f);

    EarthQuake = 0.25f;
    ApplyCharacterEffectsEarthQuake();
    CHECK(EarthQuake == 0.25f); // a request is handed over once
}

TEST_CASE("No camera shake is handed over when no monster effect requests one [gamelogic]")
{
    DyingStatueScope scope;
    scope.character().Object.Live = false;
    EarthQuake = -0.4f;

    UpdateCharacterEffects();
    ApplyCharacterEffectsEarthQuake();

    CHECK(EarthQuake == -0.4f);
}

TEST_CASE("An effect does not keep the visibility of the effect that had its slot [effects]")
{
    for (int i = 0; i < MAX_EFFECTS; ++i)
        Effects[i].Live = false;
    Effects[0].Visible = true;

    vec3_t position{1000.f, 1000.f, 0.f};
    vec3_t angle{0.f, 0.f, 0.f};
    vec3_t light{1.f, 1.f, 1.f};
    CreateEffect(BITMAP_MAGIC, position, angle, light, 8);

    CHECK(Effects[0].Live);
    CHECK(Effects[0].Type == BITMAP_MAGIC);
    CHECK_FALSE(Effects[0].Visible);

    Effects[0].Live = false;
}

TEST_CASE("The sprites of the monster effects stay out of the bloom, as when they were drawn [effects]")
{
    for (int i = 0; i < MAX_SPRITES; ++i)
        Sprites[i].Live = false;

    vec3_t position{1000.f, 1000.f, 0.f};
    vec3_t light{1.f, 1.f, 1.f};
    const int outside = CreateSprite(BITMAP_LIGHT, position, 1.f, light, nullptr);
    REQUIRE(Sprites[outside].Live);
    CHECK_FALSE(IsSpriteBloomExcluded(outside));
    Sprites[outside].Live = false;

    CharacterScope scope(MODEL_CRUST); // its eyes are two sprites
    UpdateCharacterEffects();

    int created = 0;
    for (int i = 0; i < MAX_SPRITES; ++i)
    {
        if (!Sprites[i].Live)
            continue;
        ++created;
        CHECK(IsSpriteBloomExcluded(i));
        Sprites[i].Live = false;
    }
    CHECK(created == 2);
}

TEST_CASE("A client character's drawn parts get a slot each, in draw order, with their own bones [effects]")
{
    namespace Attachments = Engine::Object::Attachments;
    CharacterScope scope(MODEL_PLAYER);
    CHARACTER* c = &scope.character();
    PART_t temporary;

    Attachments::BeginFrame();
    Attachments::BeginCharacter(c);
    Attachments::Slot* weapon =
        Attachments::Record(c, &c->Weapon[0], MODEL_SWORD, 0.f, 0.f, 0.f, 3, 0, false, true, 0, false, 33, 1.f);
    Attachments::Slot* copy =
        Attachments::Record(c, &temporary, MODEL_SWORD, 0.f, 0.f, 15.f, 3, 0, true, true, 0, true, 33, 1.f);
    REQUIRE(weapon != nullptr);
    REQUIRE(copy != nullptr);
    CHECK(weapon != copy);
    CHECK(weapon->part == &c->Weapon[0]);
    CHECK(copy->part == nullptr);
    CHECK(copy->z == 15.f);
    CHECK(Attachments::Bones(*weapon) != Attachments::Bones(*copy));

    // The next frame's draw of the same character reuses the slots in the same order.
    float (*bones)[3][4] = Attachments::Bones(*weapon);
    Attachments::BeginFrame();
    Attachments::BeginCharacter(c);
    Attachments::Slot* again =
        Attachments::Record(c, &c->Weapon[0], MODEL_SWORD, 0.f, 0.f, 0.f, 3, 0, false, true, 0, false, 33, 1.f);
    CHECK(Attachments::Bones(*again) == bones);

    CHARACTER preview;
    CHECK(Attachments::Record(&preview, &preview.Weapon[0], MODEL_SWORD, 0.f, 0.f, 0.f, 0, 0, false, true, 0, false, 33,
                              1.f) == nullptr);
}

TEST_CASE("The update advances each part of the last draw once, and the next draw reuses it [effects]")
{
    namespace Attachments = Engine::Object::Attachments;
    CharacterScope scope(MODEL_PLAYER);
    CHARACTER* c = &scope.character();
    PART_t temporary;
    const auto record = [c](PART_t* part, float z)
    { return Attachments::Record(c, part, MODEL_SWORD, 0.f, 0.f, z, 0, 0, false, true, 0, false, 33, 1.f); };
    const auto advance = [c]
    {
        int advanced = 0;
        Attachments::ForEachToAdvance(c, [&advanced](Attachments::Slot&) { ++advanced; });
        return advanced;
    };

    Attachments::BeginFrame();
    Attachments::BeginCharacter(c);
    record(&c->Weapon[0], 0.f);
    record(&temporary, 0.f);
    CHECK(advance() == 1); // a part built for one call has no animation state to advance
    CHECK(advance() == 0); // once per draw

    Attachments::BeginFrame();
    Attachments::BeginCharacter(c);
    CHECK(Attachments::WasAdvanced(*record(&c->Weapon[0], 0.f)));

    // The next draw attaches something else in that slot: it advances the part itself.
    CHECK(advance() == 1);
    Attachments::BeginFrame();
    Attachments::BeginCharacter(c);
    CHECK_FALSE(Attachments::WasAdvanced(*record(&c->Weapon[0], 15.f)));
}

TEST_CASE("A cloaked rider's Fenrir spawns none of its effects, as its outline draw spawned none [effects]")
{
    FenrirScope scope;
    OBJECT* rider = &scope.rider;
    g_CharacterRegisterBuff(rider, eBuff_Cloaking);

    UpdateDrawnMountEffects(&scope.mount);

    for (int i = 0; i < MAX_SPRITES; ++i)
        CHECK_FALSE(Sprites[i].Live);
    for (int i = 0; i < MAX_EFFECTS; ++i)
        CHECK_FALSE(Effects[i].Live);
}

TEST_CASE("A Fenrir's sprites stay out of the bloom, as when its draw spawned them [effects]")
{
    FenrirScope scope;

    UpdateDrawnMountEffects(&scope.mount);

    int created = 0;
    for (int i = 0; i < MAX_SPRITES; ++i)
    {
        if (!Sprites[i].Live)
            continue;
        ++created;
        CHECK(IsSpriteBloomExcluded(i));
    }
    CHECK(created >= 6); // its eye and chest lights
}

TEST_CASE("A Fenrir's body thunder starts on the Fenrir's own pose, not on the shared bones [effects]")
{
    FenrirScope scope;

    int onPose = 0;
    for (int frame = 0; frame < 40; ++frame)
    {
        UpdateDrawnMountEffects(&scope.mount);
        for (int i = 0; i < MAX_EFFECTS; ++i)
        {
            const OBJECT& thunder = Effects[i];
            if (!thunder.Live || thunder.Type != MODEL_FENRIR_THUNDER || thunder.SubType != 0)
                continue;
            CHECK(thunder.Position[0] > FenrirScope::sharedAt / 2.f);
            if (thunder.Position[0] == FenrirScope::poseAt && thunder.Position[2] == FenrirScope::poseAt)
                ++onPose;
        }
        FenrirScope::ClearSpawns();
    }
    CHECK(onPose > 0);
}

TEST_CASE("A dying castle gate crumbles from its own posed mesh in the update [effects]")
{
    CharacterScope scope(MODEL_CASTLE_GATE);
    const float previousFrameFactor = FPS_ANIMATION_FACTOR;
    FPS_ANIMATION_FACTOR = 1.f; // every frame-checked spawn gets its chance
    Vertex_t vertices[3]{};
    Triangle_t triangle{};
    triangle.Polygon = 3;
    for (short k = 0; k < 3; ++k)
        triangle.VertexIndex[k] = k;
    Mesh_t mesh;
    mesh.NumVertices = 3;
    mesh.Vertices = vertices;
    mesh.NumTriangles = 1;
    mesh.Triangles = &triangle;
    BMD& model = Models[MODEL_CASTLE_GATE];
    model.NumMeshs = 1;
    model.Meshs = &mesh;

    OBJECT& gate = scope.character().Object;
    for (int axis = 0; axis < 3; ++axis)
        gate.BoneTransform[0][axis][axis] = 1.f;
    gate.BoneTransform[0][0][3] = 100.f; // every vertex sits 100 along x in model space
    Vector(1000.f, 2000.f, 0.f, gate.Position);
    gate.Alpha = 1.f;
    gate.HiddenMesh = -1;
    gate.EnableBoneMatrix = true;
    gate.CurrentAction = MONSTER01_DIE;
    FenrirScope::ClearSpawns();

    int debris = 0;
    for (int frame = 0; frame < 60; ++frame)
    {
        gate.Live = true;
        UpdateRenderObjectEffects(&gate);
        CHECK_FALSE(gate.Live);
        for (int i = 0; i < MAX_EFFECTS; ++i)
        {
            const OBJECT& piece = Effects[i];
            if (!piece.Live || (piece.Type != MODEL_GATE && piece.Type != MODEL_GATE + 1))
                continue;
            // The vertex, placed where the gate's draw puts it (60 behind its position).
            CHECK(piece.Position[0] == doctest::Approx(1100.f));
            CHECK(piece.Position[1] == doctest::Approx(2060.f));
            ++debris;
        }
        FenrirScope::ClearSpawns();
    }
    CHECK(debris > 0);

    model.Meshs = nullptr;
    model.NumMeshs = 0;
    FPS_ANIMATION_FACTOR = previousFrameFactor;
}

TEST_CASE("A wall's debris effect bursts before its two ticks of life run out, drawn or not [effects]")
{
    CharacterScope scope(BATTLE_CASTLE_WALL1);
    const float previousFrameFactor = FPS_ANIMATION_FACTOR;
    FPS_ANIMATION_FACTOR = 1.f; // 25 FPS: the effect lives two moves
    Vertex_t vertices[3]{};
    Triangle_t triangles[10]{};
    for (Triangle_t& triangle : triangles)
    {
        triangle.Polygon = 3;
        for (short k = 0; k < 3; ++k)
            triangle.VertexIndex[k] = k;
    }
    Mesh_t mesh;
    mesh.NumVertices = 3;
    mesh.Vertices = vertices;
    mesh.NumTriangles = 10; // 30 one-in-three chances
    mesh.Triangles = triangles;
    BMD& model = Models[BATTLE_CASTLE_WALL1];
    model.NumActions = 0; // no keys to pose: the scratch bones stay as they are
    model.NumMeshs = 1;
    model.Meshs = &mesh;
    FenrirScope::ClearSpawns();

    vec3_t position{1000.f, 2000.f, 0.f};
    vec3_t angle{0.f, 0.f, 0.f};
    vec3_t light{1.f, 1.f, 1.f};
    CreateEffect(BATTLE_CASTLE_WALL1, position, angle, light, 1);
    OBJECT& wall = Effects[0];
    REQUIRE(wall.Type == BATTLE_CASTLE_WALL1);
    wall.LifeTime = 2.f;

    int debris = 0;
    for (int move = 0; move < 2; ++move)
    {
        MoveEffects();
        for (int i = 1; i < MAX_EFFECTS; ++i)
        {
            if (Effects[i].Live && (Effects[i].Type == MODEL_WALL_PART1 || Effects[i].Type == MODEL_WALL_PART1 + 1))
                ++debris;
            Effects[i].Live = false;
        }
    }
    CHECK_FALSE(wall.Live);
    CHECK(debris > 0);

    model.Meshs = nullptr;
    model.NumMeshs = 0;
    FPS_ANIMATION_FACTOR = previousFrameFactor;
}

TEST_CASE("A Dark Spirit the last frame left off screen flies back from the update [effects]")
{
    struct Raven : CSPetDarkSpirit
    {
        using CSPetDarkSpirit::CSPetDarkSpirit;
        OBJECT& object()
        {
            return m_PetCharacter.Object;
        }
    };
    CharacterScope scope(MODEL_PLAYER);
    Raven raven(&scope.character());
    OBJECT& o = raven.object();
    o.Visible = false;
    o.AI = PET_ATTACK;
    o.Velocity = 0.f;

    raven.UpdatePetEffects();

    CHECK(o.AI == PET_FLYING);
    CHECK(o.Velocity == 3.f);
}
