#include "stdafx.h"
#include "doctest.h"

#include "App/Platform/Windows/Winmain.h"
#include "Engine/AI/GOBoid.h"
#include "Character/CSParts.h"
#include "Core/Utilities/DrawSpawnCheck.h"
#include "Engine/Object/CharacterAttachments.h"
#include "Engine/Object/ObjectPose.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Physics/PhysicsManager.h"
#include "GameLogic/Pets/CSPetSystem.h"
#include "GameLogic/Social/MonkSystem.h"
#include "Render/Effects/ZzzEffect.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "Render/Items/ItemEffects.h"
#include "Render/Items/ItemRenderStyles.h"
#include "Render/Models/ModelDraw.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "World/GameMaps/GMBattleCastle.h"
#include "World/GameMaps/GMEmpireGuardian1.h"
#include "World/GameMaps/GM_Kanturu_1st.h"
#include "World/GameMaps/GM_Kanturu_2nd.h"
#include "World/GameMaps/GM_Kanturu_3rd.h"
#include "World/GameMaps/GMNewTown.h"
#include "World/GameMaps/GM_PK_Field.h"
#include "World/MapInfra/MapManager.h"
#include "World/MapInfra/w_MapProcess.h"

#include "../data/ItemModelDatabaseScope.h"
#include "RepeatedDraws.h"

#include <algorithm>
#include <cstring>
#include <functional>
#include <memory>
#include <new>
#include <vector>

namespace
{
// Runs a draw as the frame runs it and checks it spawned nothing.
template <typename Draw> void CheckDrawSpawnsNone(Draw draw)
{
    Diagnostics::DrawSpawnCheck::ResetViolations();
    {
        const Diagnostics::DrawSpawnCheck::DrawScope drawing;
        draw();
    }
    CHECK(Diagnostics::DrawSpawnCheck::Violations() == 0);
}

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

TEST_CASE("The update plans a client character's parts in draw order, each with bones of its own [effects]")
{
    namespace Attachments = Engine::Object::Attachments;
    CharacterScope scope(MODEL_PLAYER);
    CHARACTER* c = &scope.character();
    PART_t temporary;

    REQUIRE(Attachments::BeginPlan(c));
    Attachments::Slot& weapon =
        Attachments::Plan(c, &c->Weapon[0], MODEL_SWORD, 0.f, 0.f, 0.f, false, true, 33, 1.f, 0.5f);
    Attachments::Slot& copy = Attachments::Plan(c, &temporary, MODEL_SWORD, 0.f, 0.f, 15.f, true, false, 33, 1.f, 0.f);
    CHECK(&weapon != &copy);
    CHECK(weapon.part == &c->Weapon[0]);
    CHECK(copy.part == nullptr);
    CHECK(copy.copy != nullptr);
    CHECK(Attachments::Bones(weapon) != Attachments::Bones(copy));

    // The draws of the coming frame find it, a part built for one draw by its model and hand.
    CHECK(Attachments::Find(c, &c->Weapon[0], MODEL_SWORD, true) == nullptr);
    Attachments::BeginFrame();
    CHECK(Attachments::Find(c, &c->Weapon[0], MODEL_SWORD, true) == &weapon);
    PART_t other;
    CHECK(Attachments::Find(c, &other, MODEL_SWORD, false) == &copy);
    CHECK(Attachments::Find(c, &c->Weapon[0], MODEL_AXE, true) == nullptr);
    // A frame the update planned nothing for finds nothing.
    Attachments::BeginFrame();
    CHECK(Attachments::Find(c, &c->Weapon[0], MODEL_SWORD, true) == nullptr);

    CHARACTER preview;
    CHECK_FALSE(Attachments::BeginPlan(&preview));
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

TEST_CASE("A drawn caped monster's cloth is made and simulated by the update, not by the draw [effects]")
{
    CharacterScope scope(MODEL_CRUST);
    CHARACTER& c = scope.character();
    OBJECT& o = c.Object;
    for (int bone = 0; bone < MAX_BONES; ++bone)
        for (int axis = 0; axis < 3; ++axis)
            o.BoneTransform[bone][axis][axis] = 1.f;
    o.Alpha = 1.f;
    c.MonsterIndex = MONSTER_DEATH_KING;
    c.GuildMarkIndex = -1;

    UpdateCharacterEffects();

    REQUIRE(o.m_pCloth != nullptr);
    CHECK(o.m_byNumCloth == 1);
    CHECK(((CPhysicsCloth*)o.m_pCloth)[0].GetOwner() == &o);
    DeleteCloth(&c, &o);
}

TEST_CASE("A GM's transformation spawns its lights from the update, and none while cloaked [effects]")
{
    CharacterScope scope(MODEL_PLAYER);
    OBJECT& o = scope.character().Object;
    o.Kind = KIND_PLAYER;
    o.SubType = MODEL_GM_CHARACTER;
    o.Alpha = 1.f;
    o.HiddenMesh = -1;
    auto liveSprites = []
    {
        int live = 0;
        for (int i = 0; i < MAX_SPRITES; ++i)
            live += Sprites[i].Live ? 1 : 0;
        return live;
    };
    FenrirScope::ClearSpawns();

    UpdateTransformSkinEffects(&o);
    CHECK(o.HiddenMesh == 2);
    CHECK(liveSprites() == 8);

    FenrirScope::ClearSpawns();
    OBJECT* gm = &o;
    g_CharacterRegisterBuff(gm, eBuff_Cloaking);
    UpdateTransformSkinEffects(gm);
    CHECK(liveSprites() == 0);
    g_CharacterClearBuff(gm);
}

TEST_CASE("A character's mark spawns its sprite in the update, and its draw spawns none [effects]")
{
    CharacterScope scope(MODEL_PLAYER);
    CHARACTER& c = scope.character();
    c.m_pParts = new CSParts2D(BITMAP_LIGHT, 0, 0);
    auto liveSprites = []
    {
        int live = 0;
        for (int i = 0; i < MAX_SPRITES; ++i)
            live += Sprites[i].Live ? 1 : 0;
        return live;
    };
    FenrirScope::ClearSpawns();

    CheckDrawSpawnsNone([&c] { RenderParts(&c); });
    CHECK(liveSprites() == 0);
    UpdateParts(&c);
    CHECK(liveSprites() == 1);

    DeleteParts(&c);
    FenrirScope::ClearSpawns();
}

TEST_CASE("A wing's item effect spawns its lights on the bones it is given, and its draw spawns none [effects]")
{
    FenrirScope scope;
    ItemModelDatabaseScope restoreModels;
    Data::Items::ItemModelDefinition eternal;
    eternal.group = 12;
    eternal.number = 37;
    eternal.file = "Data/Item/Wing37.bmd";
    eternal.itemEffect = "wingOfEternal";
    g_ItemModelDatabase.Build(std::vector<Data::Items::ItemModelDefinition>{eternal}, Data::Effects::GlowColorList{});
    BMD* b = &Models[MODEL_WING_OF_ETERNAL];
    b->BodyScale = 1.f;
    Vector(0.f, 0.f, 0.f, b->BodyOrigin);
    OBJECT wing;
    wing.Type = MODEL_WING_OF_ETERNAL;

    int level = 0;
    float alpha = 1.f;
    DrawLook look;
    CheckDrawSpawnsNone(
        [&]
        {
            CHECK(Render::Items::ItemEffects::Apply(b, &wing, look, MODEL_WING_OF_ETERNAL, alpha, level) ==
                  Render::Items::ItemEffects::Result::Applied);
        });
    for (int i = 0; i < MAX_SPRITES; ++i)
        CHECK_FALSE(Sprites[i].Live);

    REQUIRE(Render::Items::ItemEffects::Spawns(MODEL_WING_OF_ETERNAL));
    Render::Items::ItemEffects::Spawn(b, &wing, MODEL_WING_OF_ETERNAL, scope.bones.get());
    int onPose = 0;
    for (int i = 0; i < MAX_SPRITES; ++i)
    {
        if (!Sprites[i].Live)
            continue;
        CHECK(Sprites[i].Position[0] == FenrirScope::poseAt);
        ++onPose;
    }
    CHECK(onPose == 28); // its lights
}

TEST_CASE(
    "A New Town crystal sparks from the update on its own pose, and its after-character draw spawns none [effects]")
{
    FenrirScope scope;
    const int previousWorld = gMapManager.WorldActive;
    gMapManager.WorldActive = WD_51HOME_6TH_CHAR;
    OBJECT crystal;
    crystal.Live = true;
    crystal.Type = 78;
    crystal.Scale = 1.f;
    crystal.BoneTransform = scope.bones.get();
    crystal.EnableBoneMatrix = true;
    BMD* b = &Models[crystal.Type];
    PlaceObjectModel(&crystal, b);
    Engine::Object::Pose::Record(&crystal, b, true);
    auto liveSprites = []
    {
        int live = 0;
        for (int i = 0; i < MAX_SPRITES; ++i)
            live += Sprites[i].Live ? 1 : 0;
        return live;
    };

    DrawLook look = DrawLook::Of(crystal);
    CheckDrawSpawnsNone([&] { SEASON3B::GMNewTown::RenderObjectAfterCharacter(&crystal, look, b); });
    CHECK(liveSprites() == 0);
    SEASON3B::GMNewTown::UpdateObjectEffects(&crystal, b);
    CHECK(liveSprites() == 4);
    for (int i = 0; i < MAX_SPRITES; ++i)
    {
        if (Sprites[i].Live)
            CHECK(Sprites[i].Position[0] == FenrirScope::poseAt);
    }

    crystal.BoneTransform = nullptr;
    gMapManager.WorldActive = previousWorld;
}

TEST_CASE("A dying Blood Assassin breaks apart from the update, body and head, and its draw spawns none [effects]")
{
    CharacterScope scope(MODEL_BLOOD_ASSASSIN);
    CHARACTER& c = scope.character();
    OBJECT& o = c.Object;
    o.CurrentAction = MONSTER01_DIE;
    o.LifeTime = 100;
    const int previousWorld = gMapManager.WorldActive;
    gMapManager.WorldActive = WD_63PK_FIELD;
    auto map = CGM_PK_Field::Make();
    BMD* b = &Models[o.Type];
    auto liveEffects = []
    {
        int live = 0;
        for (int i = 0; i < MAX_EFFECTS; ++i)
            live += Effects[i].Live ? 1 : 0;
        return live;
    };
    FenrirScope::ClearSpawns();

    DrawLook look = DrawLook::Of(o);
    CheckDrawSpawnsNone([&] { map->RenderObjectMesh(&o, look, b, false); });
    CHECK(liveEffects() == 0);
    map->UpdateMonsterEffects(&c, &o, b);
    CHECK(liveEffects() == 2);
    CHECK(o.LifeTime == 90);

    FenrirScope::ClearSpawns();
    gMapManager.WorldActive = previousWorld;
}

TEST_CASE("A helper's light comes from the mount update, even under a cloaked rider, as its draw spawned it [effects]")
{
    FenrirScope scope;
    scope.mount.Type = MODEL_HELPER;
    auto liveSprites = []
    {
        int live = 0;
        for (int i = 0; i < MAX_SPRITES; ++i)
            live += Sprites[i].Live ? 1 : 0;
        return live;
    };

    UpdateDrawnMountEffects(&scope.mount);
    CHECK(liveSprites() == 1);

    FenrirScope::ClearSpawns();
    OBJECT* rider = &scope.rider;
    g_CharacterRegisterBuff(rider, eBuff_Cloaking);
    UpdateDrawnMountEffects(&scope.mount);
    CHECK(liveSprites() == 1);
}

TEST_CASE("A stunned or sleeping owner holds a drawn object at its first frame, as its draw did [effects]")
{
    OBJECT rider;
    OBJECT* owner = &rider;
    OBJECT mount;
    mount.Owner = owner;
    mount.Alpha = 1.f;
    mount.AnimationFrame = 3.5f;

    UpdateDrawnObjectHold(&mount);
    CHECK(mount.AnimationFrame == 3.5f);
    CHECK(mount.Alpha == 1.f);

    g_CharacterRegisterBuff(owner, eDeBuff_Sleep);
    UpdateDrawnObjectHold(&mount);
    CHECK(mount.AnimationFrame == 0.f);
}

TEST_CASE("The stun skull is refreshed for a stunned object, and not while its stun tint is hidden by cloak [effects]")
{
    FenrirScope scope;
    OBJECT* stunned = &scope.rider;
    vec3_t light{1.f, 1.f, 1.f};
    auto skulls = []
    {
        int live = 0;
        for (int i = 0; i < MAX_EFFECTS; ++i)
            live += (Effects[i].Live && Effects[i].Type == BITMAP_SKULL && Effects[i].SubType == 5) ? 1 : 0;
        return live;
    };

    RefreshStunSkull(stunned, light);
    CHECK(skulls() == 0);

    g_CharacterRegisterBuff(stunned, eDeBuff_Stun);
    RefreshStunSkull(stunned, light);
    RefreshStunSkull(stunned, light);
    CHECK(skulls() == 1);

    FenrirScope::ClearSpawns();
    g_CharacterRegisterBuff(stunned, eBuff_Cloaking);
    RefreshStunSkull(stunned, light);
    CHECK(skulls() == 0);
}

TEST_CASE("Every spawn while drawing is a violation; outside a draw it is none [effects]")
{
    using namespace Diagnostics::DrawSpawnCheck;
    ResetViolations();
    Note("sprite");
    CHECK(Violations() == 0);
    {
        const DrawScope drawing;
        Note("sprite");
        Note("sprite");
        CHECK(Violations() == 2);
    }
    ResetViolations();
}

TEST_CASE("A preview gets slots of its own only while its owner tracks it [effects]")
{
    namespace Attachments = Engine::Object::Attachments;
    auto preview = std::make_unique<CHARACTER>();

    CHECK_FALSE(Attachments::BeginPlan(preview.get()));
    Attachments::Track(preview.get());
    CHECK(Attachments::BeginPlan(preview.get()));
    Attachments::Untrack(preview.get());
    CHECK_FALSE(Attachments::BeginPlan(preview.get()));
}

TEST_CASE("Drawing a part the update has not advanced yet, again and again, advances and spawns nothing [effects]")
{
    namespace Attachments = Engine::Object::Attachments;
    CharacterScope scope(MODEL_PLAYER);
    CHARACTER* c = &scope.character();
    PART_t& part = c->Weapon[0];
    part.Type = MODEL_SWORD;
    part.AnimationFrame = 2.5f;
    part.PriorAnimationFrame = 1.5f;
    part.CurrentAction = 1;
    part.PriorAction = 0;

    Attachments::BeginFrame();
    RepeatedDraws::CheckChangesNothing(
        [c, &part] { RenderLinkObject(0.f, 0.f, 0.f, c, &part, MODEL_SWORD, 0, 0, false, true); });

    CHECK(part.AnimationFrame == 2.5f);
    CHECK(part.PriorAnimationFrame == 1.5f);
    CHECK(part.CurrentAction == 1);
    CHECK(part.PriorAction == 0);
}

extern OBJECT g_ItemObject[];

TEST_CASE("Drawing a held part sets up its own item object and leaves the shared one as it stands [effects]")
{
    CharacterScope scope(MODEL_PLAYER);
    CHARACTER* c = &scope.character();
    c->Object.EnableShadow = true;
    PART_t& part = c->Weapon[0];
    part.Type = MODEL_SWORD;
    OBJECT& shared = g_ItemObject[MODEL_SWORD];
    shared.Type = MODEL_STAFF;
    shared.EnableShadow = false;
    Vector(1.f, 2.f, 3.f, shared.Position);
    Vector(4.f, 5.f, 6.f, shared.Angle);

    Engine::Object::Attachments::BeginFrame();
    RenderLinkObject(10.f, 20.f, 30.f, c, &part, MODEL_SWORD, 0, 0, false, true);

    CHECK(shared.Type == MODEL_STAFF);
    CHECK_FALSE(shared.EnableShadow);
    CHECK(shared.Position[0] == 1.f);
    CHECK(shared.Position[2] == 3.f);
    CHECK(shared.Angle[1] == 5.f);
}

TEST_CASE(
    "Dark Side steps in the update, once per part its draw hands over, and drawing it again steps nothing [effects]")
{
    CharacterScope scope(MODEL_PLAYER);
    // Actions without keys: the draws pose nothing, and the Rage Fighter look reads the action's key count.
    Models[MODEL_PLAYER].NumActions = 0;
    Models[MODEL_PLAYER].Actions = new Action_t[PLAYER_SKILL_DARKSIDE_ATTACK + 1]{};
    CHARACTER* c = &scope.character();
    OBJECT* o = &c->Object;
    c->Class = CLASS_RAGEFIGHTER;
    o->Kind = KIND_PLAYER;
    o->CurrentAction = PLAYER_SKILL_DARKSIDE_READY;
    o->m_sTargetIndex = 2;
    Vector(1000.f, 1000.f, 0.f, o->Position);
    c->BodyPart[2].Type = MODEL_SWORD;
    c->BodyPart[3].Type = MODEL_SWORD;
    CHARACTER& target = scope.characters[2];
    target.Object.Live = true;
    target.Key = 77;
    Vector(2000.f, 1000.f, 0.f, target.Object.Position);

    WORD targets[DARKSIDE_TARGET_MAX];
    for (WORD& index : targets)
        index = NUMOFMON;
    targets[0] = 77;
    g_CMonkSystem.InitDarksideTarget();
    g_CMonkSystem.SetDarksideTargetIndex(targets, AT_SKILL_DARKSIDE);
    const int referenceFrames = REFERENCE_FRAME_STEP;
    REFERENCE_FRAME_STEP = 1;
    const std::vector<PART_t*> parts{&c->BodyPart[3], &c->BodyPart[2]};

    g_CMonkSystem.UpdateDarkside(c, parts);
    const auto* first = g_CMonkSystem.DarksidePlacementsOf(c, parts[0]);
    const auto* second = g_CMonkSystem.DarksidePlacementsOf(c, parts[1]);
    REQUIRE(first != nullptr);
    REQUIRE(second != nullptr);
    REQUIRE_FALSE(first->empty());
    REQUIRE_FALSE(second->empty());
    // The second part's step dashed further toward the target; the character still stands where it stood.
    CHECK(second->front().position[0] > first->front().position[0]);
    CHECK(first->front().position[0] > 1000.f);
    CHECK(o->Position[0] == 1000.f);

    const CMonkSystem undrawn = g_CMonkSystem;
    RepeatedDraws::CheckChangesNothing(
        [c, &parts]
        {
            for (PART_t* part : parts)
                g_CMonkSystem.DarksideRendering(c, part);
        });
    CHECK(o->Angle[2] == 0.f);
    CHECK(o->Position[0] == 1000.f);
    CHECK(o->CurrentAction == PLAYER_SKILL_DARKSIDE_READY);

    // The next step goes on from where the update left it, as if nothing had been drawn.
    CMonkSystem stepped = undrawn;
    stepped.UpdateDarkside(c, parts);
    FenrirScope::ClearSpawns();
    g_CMonkSystem.UpdateDarkside(c, parts);
    FenrirScope::ClearSpawns();
    CHECK(g_CMonkSystem.DarksidePlacementsOf(c, parts[0])->front().position[0] ==
          stepped.DarksidePlacementsOf(c, parts[0])->front().position[0]);

    // A frame without a reference frame steps nothing.
    REFERENCE_FRAME_STEP = 0;
    const float placed = g_CMonkSystem.DarksidePlacementsOf(c, parts[0])->front().position[0];
    g_CMonkSystem.UpdateDarkside(c, parts);
    CHECK(g_CMonkSystem.DarksidePlacementsOf(c, parts[0])->front().position[0] == placed);

    g_CMonkSystem.UpdateDarkside(c, {});
    CHECK(g_CMonkSystem.DarksidePlacementsOf(c, parts[0]) == nullptr);
    g_CMonkSystem.InitDarksideTarget();
    REFERENCE_FRAME_STEP = referenceFrames;
    FenrirScope::ClearSpawns();
}

TEST_CASE("Creating a sprite while drawing is counted as a violation [effects]")
{
    FenrirScope::ClearSpawns();
    Diagnostics::DrawSpawnCheck::ResetViolations();
    vec3_t position = {0.f, 0.f, 0.f}, light = {1.f, 1.f, 1.f};
    {
        const Diagnostics::DrawSpawnCheck::DrawScope drawing;
        CreateSprite(BITMAP_LIGHT, position, 1.f, light, nullptr);
    }
    CHECK(Diagnostics::DrawSpawnCheck::Violations() == 1);
    Diagnostics::DrawSpawnCheck::ResetViolations();
    FenrirScope::ClearSpawns();
}

TEST_CASE("Drawing an item's effect or style again and again leaves the look on its object as it was [effects]")
{
    FenrirScope scope;
    ItemModelDatabaseScope restoreModels;
    Data::Items::ItemModelDefinition eye;
    eye.group = 14;
    eye.number = 17;
    eye.file = "Data/Item/EventBullCatEye.bmd";
    eye.itemEffect = "devilsEye";
    Data::Items::ItemModelDefinition bone;
    bone.group = 13;
    bone.number = 17;
    bone.file = "Data/Item/BloodBone.bmd";
    bone.renderStyle = "bloodBone";
    g_ItemModelDatabase.Build(std::vector<Data::Items::ItemModelDefinition>{eye, bone},
                              Data::Effects::GlowColorList{});
    const int eyeModel = Data::Items::ToModelSlot(Data::Items::MakeItemType(14, 17));
    const int boneModel = Data::Items::ToModelSlot(Data::Items::MakeItemType(13, 17));
    const double previousTime = WorldTime;
    WorldTime = 1000.0;

    for (const int model : {eyeModel, boneModel})
    {
        CAPTURE(model);
        Models[model].BodyScale = 1.f;
        OBJECT item;
        item.Type = model;
        item.Alpha = 0.6f;
        item.BlendMesh = -1;
        item.BlendMeshLight = 0.25f;
        item.BlendMeshTexCoordU = 0.125f;
        item.BlendMeshTexCoordV = 0.125f;
        item.HiddenMesh = -2;
        vec3_t light = {1.f, 1.f, 1.f};
        // The ground item's passes, each taking its alpha from the item.
        CheckDrawSpawnsNone(
            [&]
            {
                for (int pass = 0; pass < 3; ++pass)
                    RenderPartObjectEffect(&item, model, light, item.Alpha, 0, 0, 0, 0, RENDER_TEXTURE);
            });
        CHECK(item.Alpha == 0.6f);
        CHECK(item.BlendMesh == -1);
        CHECK(item.BlendMeshLight == 0.25f);
        CHECK(item.BlendMeshTexCoordU == 0.125f);
        CHECK(item.BlendMeshTexCoordV == 0.125f);
        CHECK(item.HiddenMesh == -2);
    }

    // The eye is still drawn bright: its effect hands the draw its alpha.
    OBJECT item;
    item.Type = eyeModel;
    float alpha = 0.6f;
    int level = 0;
    DrawLook look = DrawLook::Of(item);
    CHECK(ApplyPartObjectEffect(&Models[eyeModel], &item, look, eyeModel, alpha, level, 0) ==
          Render::Items::ItemEffects::Result::Applied);
    CHECK(alpha == 2.f);
    WorldTime = previousTime;
}

TEST_CASE("A cycling item look comes from the clock: every pass of a frame shows the same, and no draw keeps a count "
          "[effects]")
{
    struct MeshRecorder final : Render::ModelDrawing::ModelDrawer
    {
        void DrawMesh(const Render::ModelDrawing::MeshDraw& draw) override
        {
            draws.push_back(draw);
        }
        void DrawShadow(const Render::ModelDrawing::ShadowDraw&) override {}
        std::vector<Render::ModelDrawing::MeshDraw> draws;
    };

    FenrirScope scope;
    ItemModelDatabaseScope restoreModels;
    Data::Items::ItemModelDefinition storm;
    storm.group = 12;
    storm.number = 36;
    storm.file = "Data/Item/Wing36.bmd";
    storm.renderStyle = "wingOfStorm";
    Data::Items::ItemModelDefinition symbol;
    symbol.group = 14;
    symbol.number = 29;
    symbol.file = "Data/Item/Symbol.bmd";
    symbol.renderStyle = "symbolOfKundun";
    g_ItemModelDatabase.Build(std::vector<Data::Items::ItemModelDefinition>{storm, symbol},
                              Data::Effects::GlowColorList{});
    const int stormModel = Data::Items::ToModelSlot(Data::Items::MakeItemType(12, 36));
    const int symbolModel = Data::Items::ToModelSlot(Data::Items::MakeItemType(14, 29));
    const double previousTime = WorldTime;

    // The value each style gives its cycling mesh in three passes at one time.
    const auto drawnAt = [&](int model, int mesh, double time, auto value)
    {
        WorldTime = time;
        MeshRecorder recorder;
        Render::ModelDrawing::SetModelDrawer(&recorder);
        OBJECT item;
        item.Type = model;
        for (int pass = 0; pass < 3; ++pass)
        {
            DrawLook look = DrawLook::Of(item);
            Render::Items::Styles::Render(&Models[model], &item, look, model, 1.f, RENDER_TEXTURE);
        }
        Render::ModelDrawing::SetModelDrawer(nullptr);
        std::vector<float> values;
        for (const auto& draw : recorder.draws)
            if (draw.mesh == mesh)
                values.push_back(value(draw));
        return values;
    };
    const auto texCoordU = [](const Render::ModelDrawing::MeshDraw& draw) { return draw.blendMeshU; };
    const auto meshLight = [](const Render::ModelDrawing::MeshDraw& draw) { return draw.blendMeshAlpha; };

    // 1000 ms is 25 reference frames: the Storm wing's fourth-frame texture strip is on frame 2 (25 % 16 / 4), and
    // the Symbol of Kundun's light is 0.01 + 0.25 on its way up.
    CHECK(drawnAt(stormModel, 1, 1000.0, texCoordU) == std::vector<float>(3, 0.5f));
    const std::vector<float> first = drawnAt(symbolModel, 2, 1000.0, meshLight);
    REQUIRE(first.size() == 3);
    CHECK(first[0] == doctest::Approx(0.26f));
    CHECK(first[2] == first[0]);
    // 29 reference frames: strip frame 3; 0.29 later the light still climbs.
    CHECK(drawnAt(stormModel, 1, 1160.0, texCoordU) == std::vector<float>(3, 0.75f));
    const std::vector<float> later = drawnAt(symbolModel, 2, 1160.0, meshLight);
    REQUIRE(later.size() == 3);
    CHECK(later[0] == doctest::Approx(0.30f));
    CHECK(later[2] == later[0]);
    WorldTime = previousTime;
}

TEST_CASE("A Lion Gate is placed by its update, not by its draw [effects]")
{
    CharacterScope scope(MODEL_LION_GATE);
    const int previousWorld = gMapManager.WorldActive;
    gMapManager.WorldActive = WD_69EMPIREGUARDIAN1;
    OBJECT& gate = scope.character().Object;
    gate.Type = MODEL_LION_GATE;
    Vector(23410.f, 2860.f, 0.f, gate.Position); // tile 234, 28
    BMD* b = &Models[gate.Type];

    DrawLook look = DrawLook::Of(gate);
    CheckDrawSpawnsNone([&] { GMEmpireGuardian1::Make()->RenderMonster(&gate, look, b); });
    CHECK(gate.Position[0] == 23410.f);

    GMEmpireGuardian1::Make()->MoveMonsterVisual(&scope.character(), &gate, b);
    CHECK(gate.Position[0] == 23450.f);
    CHECK(gate.Position[1] == 2820.f);
    gMapManager.WorldActive = previousWorld;
}

TEST_CASE("A dying Blade Hunter loses its shadow in its update [effects]")
{
    CharacterScope scope(MODEL_BLADE_HUNTER);
    OBJECT& hunter = scope.character().Object;
    hunter.Type = MODEL_BLADE_HUNTER;
    hunter.m_bRenderShadow = true;
    hunter.CurrentAction = MONSTER01_WALK;
    M37Kanturu1st::MoveKanturu1stMonsterVisual(&scope.character(), &hunter, &Models[hunter.Type]);
    CHECK(hunter.m_bRenderShadow);
    hunter.CurrentAction = MONSTER01_DIE;
    M37Kanturu1st::MoveKanturu1stMonsterVisual(&scope.character(), &hunter, &Models[hunter.Type]);
    CHECK_FALSE(hunter.m_bRenderShadow);
}

TEST_CASE("A dying Dreadfear fades in its update, by the reference clock, not in its draw [effects]")
{
    CharacterScope scope(MODEL_DREADFEAR);
    const int previousWorld = gMapManager.WorldActive;
    const float previousFactor = FPS_ANIMATION_FACTOR;
    FPS_ANIMATION_FACTOR = 1.f;
    OBJECT& dreadfear = scope.character().Object;
    dreadfear.Type = MODEL_DREADFEAR;
    dreadfear.CurrentAction = MONSTER01_DIE;
    BMD* b = &Models[dreadfear.Type];

    for (const int world : {static_cast<int>(WD_38KANTURU_2ND), static_cast<int>(WD_39KANTURU_3RD)})
    {
        CAPTURE(world);
        gMapManager.WorldActive = world;
        dreadfear.Alpha = 0.5f;
        DrawLook look = DrawLook::Of(dreadfear);
        CheckDrawSpawnsNone(
            [&]
            {
                M38Kanturu2nd::Render_Kanturu2nd_MonsterObjectMesh(&dreadfear, look, b, 0);
                M39Kanturu3rd::RenderKanturu3rdMonsterObjectMesh(&dreadfear, look, b, false);
            });
        CHECK(dreadfear.Alpha == 0.5f);

        // The monster update runs both maps' steps; each fades only on its own map, once.
        M38Kanturu2nd::Move_Kanturu2nd_MonsterVisual(&scope.character(), &dreadfear, b);
        M39Kanturu3rd::MoveKanturu3rdMonsterVisual(&dreadfear, b);
        CHECK(dreadfear.Alpha == doctest::Approx(0.4f));
    }
    FPS_ANIMATION_FACTOR = previousFactor;
    gMapManager.WorldActive = previousWorld;
}

TEST_CASE("An afterimage draws its ghosts fainter and at earlier frames, and leaves the object as it stands [effects]")
{
    struct AlphaRecorder final : Render::ModelDrawing::ModelDrawer
    {
        void DrawMesh(const Render::ModelDrawing::MeshDraw& draw) override
        {
            meshes.emplace_back(draw.mesh, draw.alpha);
        }
        void DrawShadow(const Render::ModelDrawing::ShadowDraw&) override {}
        std::vector<std::pair<int, float>> meshes;
    };

    FenrirScope scope;
    // Fred draws mesh 3 only before frame 2.5 of its second attack, so each ghost's frame shows.
    OBJECT fred;
    fred.Live = true;
    fred.Type = MODEL_FRED;
    fred.Scale = 1.f;
    fred.Alpha = 1.f;
    fred.CurrentAction = MONSTER01_ATTACK2;
    fred.AnimationFrame = 3.f;

    AlphaRecorder recorder;
    Render::ModelDrawing::SetModelDrawer(&recorder);
    CheckDrawSpawnsNone([&] { RenderObject_AfterImage(&fred); });
    Render::ModelDrawing::SetModelDrawer(nullptr);

    CHECK(fred.Alpha == 1.f);
    CHECK(fred.AnimationFrame == 3.f);
    // The first ghost (frame 1.6, alpha 0.2) and the second (frame 2.3, alpha 0.6) are before frame 2.5, so they
    // draw mesh 3; the object itself (frame 3, alpha 1) does not.
    std::vector<float> mesh3;
    std::string drawn;
    for (const auto& [mesh, alpha] : recorder.meshes)
    {
        drawn += std::to_string(mesh) + "@" + std::to_string(alpha) + " ";
        if (mesh == 3)
            mesh3.push_back(alpha);
    }
    CAPTURE(drawn);
    CHECK(mesh3 == std::vector<float>{0.2f, 0.6f});
}

TEST_CASE("Drawing an object whose pose the update has not built leaves its pose and bones as they were [effects]")
{
    FenrirScope scope;
    OBJECT rock;
    rock.Live = true;
    rock.Type = 78;
    rock.Scale = 1.f;
    rock.Alpha = 1.f;
    rock.BoneTransform = scope.bones.get();
    rock.EnableBoneMatrix = true;
    BMD* b = &Models[rock.Type];
    REQUIRE_FALSE(Engine::Object::Pose::IsCurrent(&rock, b, true));
    const float before = rock.BoneTransform[0][0][3];

    CheckDrawSpawnsNone([&] { CHECK(Calc_RenderObject(&rock, false, 0, 0)); });
    CHECK_FALSE(Engine::Object::Pose::IsCurrent(&rock, b, true));
    CHECK(rock.BoneTransform[0][0][3] == before);
}

TEST_CASE("Cursed Castle Water lights the bone it is given from the update, and its draw spawns none [effects]")
{
    FenrirScope scope;
    ItemModelDatabaseScope restoreModels;
    Data::Items::ItemModelDefinition water;
    water.group = 14;
    water.number = 64;
    water.file = "Data/Item/Water.bmd";
    water.renderStyle = "cursedCastleWater";
    g_ItemModelDatabase.Build(std::vector<Data::Items::ItemModelDefinition>{water}, Data::Effects::GlowColorList{});
    const int model = Data::Items::ToModelSlot(Data::Items::MakeItemType(14, 64));
    BMD* b = &Models[model];
    b->BodyScale = 1.f;
    Vector(0.f, 0.f, 0.f, b->BodyOrigin);
    OBJECT item;
    item.Type = model;

    CheckDrawSpawnsNone(
        [&]
        {
            DrawLook look = DrawLook::Of(item);
            Render::Items::Styles::Render(b, &item, look, model, 1.f, RENDER_TEXTURE);
        });
    for (int i = 0; i < MAX_SPRITES; ++i)
        CHECK_FALSE(Sprites[i].Live);

    REQUIRE(Render::Items::Styles::Spawns(model));
    Render::Items::Styles::Spawn(b, &item, model, scope.bones.get());
    int onPose = 0;
    for (int i = 0; i < MAX_SPRITES; ++i)
    {
        if (!Sprites[i].Live)
            continue;
        CHECK(Sprites[i].Position[0] == FenrirScope::poseAt);
        ++onPose;
    }
    CHECK(onPose == 1);
}

TEST_CASE("A Dragon Kick afterimage leaves the fading character's alpha, place and frame as it found them [effects]")
{
    CharacterScope scope(MODEL_PLAYER);
    // Actions without keys pose nothing; the afterimage spreads its ghosts over the action's key count.
    Models[MODEL_PLAYER].NumActions = 0;
    Models[MODEL_PLAYER].Actions = new Action_t[1]{};
    Models[MODEL_PLAYER].Actions[0].NumAnimationKeys = 10;
    CHARACTER* c = &scope.character();
    OBJECT* o = &c->Object;
    o->CurrentAction = 0;
    o->AnimationFrame = 4.f;
    o->Alpha = 0.4f;
    Vector(1000.f, 1000.f, 0.f, o->Position);
    Vector(900.f, 1000.f, 0.f, o->StartPosition);
    PART_t* part = &c->BodyPart[BODYPART_ARMOR];
    part->Type = MODEL_SWORD;

    RepeatedDraws::CheckChangesNothing([c, part] { RenderCharacter_AfterImage(c, part, true, 0, 2.5f, 1.0f); });
    CHECK(o->Alpha == 0.4f);
    CHECK(o->AnimationFrame == 4.f);
    CHECK(o->Position[0] == 1000.f);

    RepeatedDraws::CheckChangesNothing([o] { RenderObject_AfterImage(o, true, 0, 0); });
    CHECK(o->Alpha == 0.4f);
    CHECK(o->AnimationFrame == 4.f);
}

TEST_CASE("Drawing an Explosion Blade leaves the item's alpha for the next pass [effects]")
{
    FenrirScope scope;
    ItemModelDatabaseScope restoreModels;
    Data::Items::ItemModelDefinition blade;
    blade.group = 0;
    blade.number = 31;
    blade.file = "Data/Item/Sword32.bmd";
    blade.renderStyle = "explosionBlade";
    g_ItemModelDatabase.Build(std::vector<Data::Items::ItemModelDefinition>{blade}, Data::Effects::GlowColorList{});
    const int model = Data::Items::ToModelSlot(Data::Items::MakeItemType(0, 31));
    BMD* b = &Models[model];
    OBJECT item;
    item.Type = model;
    item.Alpha = 1.f;

    for (int pass = 0; pass < 2; ++pass)
    {
        CheckDrawSpawnsNone(
            [&]
            {
                DrawLook look = DrawLook::Of(item);
                Render::Items::Styles::Render(b, &item, look, model, item.Alpha, RENDER_TEXTURE);
            });
        CHECK(item.Alpha == 1.f);
    }
}

TEST_CASE("Drawing a Christmas companion's three copies leaves the companion where it stands [effects]")
{
    CharacterScope scope(MODEL_PLAYER);
    CHARACTER* c = &scope.character();
    OBJECT* o = &c->Object;
    for (PART_t& part : c->BodyPart)
        part.Type = -1;
    c->Weapon[0].Type = c->Weapon[1].Type = c->Wing.Type = c->Helper.Type = -1;
    c->HideShadow = true;
    o->SubType = MODEL_XMAS_EVENT_CHA_SNOWMAN;
    o->Alpha = 1.f;
    Vector(1000.f, 1200.f, 30.f, o->Position);

    for (int pass = 0; pass < 2; ++pass)
    {
        CheckDrawSpawnsNone([c] { RenderChristmasCompanions(c, true, 0); });
        CHECK(o->Position[0] == 1000.f);
        CHECK(o->Position[1] == 1200.f);
        CHECK(o->Position[2] == 30.f);
    }
}

TEST_CASE("Character passes leave a standing siege gate's collision; the update finishes the siege census [effects]")
{
    CharacterScope scope(MODEL_PLAYER);
    const int world = gMapManager.WorldActive;
    gMapManager.WorldActive = WD_30BATTLECASTLE;
    CHARACTER* gate = &scope.character();
    gate->MonsterIndex = MONSTER_CASTLE_GATE1;
    gate->Object.Visible = false;
    // The first gate's row of tiles blocks movement.
    const int row = (114 + 1) * TERRAIN_SIZE;
    WORD saved[4];
    for (int x = 0; x < 4; ++x)
    {
        saved[x] = TerrainWall[row + 66 + x];
        TerrainWall[row + 66 + x] |= TW_NOMOVE;
    }

    battleCastle::MoveBattleCastleMonster(gate, &gate->Object);
    for (int pass = 0; pass < 2; ++pass)
    {
        CheckDrawSpawnsNone([] { RenderCharactersClient(); });
        CHECK((TerrainWall[row + 67] & TW_NOMOVE) == TW_NOMOVE);
    }

    // With no gate reported by this update, the census drops the gate's collision.
    UpdateCharacterEffects();
    UpdateCharacterEffects();
    CHECK((TerrainWall[row + 67] & TW_NOMOVE) == 0);

    for (int x = 0; x < 4; ++x)
        TerrainWall[row + 66 + x] = saved[x];
    gMapManager.WorldActive = world;
}

namespace
{
// Every field of a part an update or a draw could read.
struct PartFields
{
    short type;
    unsigned char linkBone;
    unsigned short currentAction;
    unsigned short priorAction;
    float animationFrame;
    float priorAnimationFrame;
    float playSpeed;

    explicit PartFields(const PART_t& part)
        : type(part.Type), linkBone(part.LinkBone), currentAction(part.CurrentAction), priorAction(part.PriorAction),
          animationFrame(part.AnimationFrame), priorAnimationFrame(part.PriorAnimationFrame), playSpeed(part.PlaySpeed)
    {
    }
    bool operator==(const PartFields&) const = default;
};

// Gives the model `count` looping actions of ten keys; the scope's models free them.
void LoopingActions(int type, int count = 1)
{
    BMD& model = Models[type];
    model.NumActions = count;
    model.Actions = new Action_t[count]{};
    for (int i = 0; i < count; ++i)
    {
        model.Actions[i].NumAnimationKeys = 10;
        model.Actions[i].Loop = true;
    }
}

// A preview player carrying no weapons and no body parts, tracked until the scope ends.
struct PreviewPlayer
{
    std::unique_ptr<CHARACTER> character = std::make_unique<CHARACTER>();
    float frameFactor = FPS_ANIMATION_FACTOR;

    explicit PreviewPlayer(CharacterScope& scope)
    {
        FPS_ANIMATION_FACTOR = 1.f;
        CHARACTER* c = character.get();
        OBJECT* o = &c->Object;
        o->Live = o->Visible = true;
        o->Type = MODEL_PLAYER;
        o->Scale = 1.f;
        o->Alpha = 1.f;
        o->BoneTransform = scope.bones.get();
        for (PART_t& part : c->BodyPart)
            part.Type = -1;
        for (PART_t& part : c->Weapon)
            part.Type = -1;
        c->Wing.Type = -1;
        c->Helper.Type = -1;
        Engine::Object::Attachments::Track(c);
    }

    ~PreviewPlayer()
    {
        Engine::Object::Attachments::Untrack(character.get());
        character->Object.BoneTransform = nullptr;
        FPS_ANIMATION_FACTOR = frameFactor;
    }
};

// Gives the model one bone whose x position is ten times its animation frame, over ten looping keys.
void SlidingBone(int type)
{
    LoopingActions(type);
    BMD& model = Models[type];
    model.NumBones = 1;
    model.Bones = new Bone_t[1]{};
    Bone_t& bone = model.Bones[0];
    bone.Parent = -1;
    bone.BoneMatrixes = new BoneMatrix_t[1]{};
    BoneMatrix_t& keys = bone.BoneMatrixes[0];
    keys.Position = new vec3_t[10]{};
    keys.Rotation = new vec3_t[10]{};
    keys.Quaternion = new vec4_t[10]{};
    for (int key = 0; key < 10; ++key)
    {
        keys.Position[key][0] = 10.f * static_cast<float>(key);
        keys.Quaternion[key][3] = 1.f;
    }
}

void RestAnimation(PART_t& part)
{
    part.CurrentAction = part.PriorAction = 0;
    part.AnimationFrame = part.PriorAnimationFrame = 0.f;
}
} // namespace

TEST_CASE("Which passes draw a carried part, and in what order, changes nothing the next update does [effects]")
{
    namespace Attachments = Engine::Object::Attachments;
    CharacterScope scope(MODEL_PLAYER);
    LoopingActions(MODEL_PLAYER, MAX_PLAYER_ACTION);
    LoopingActions(MODEL_WING);
    LoopingActions(MODEL_IMP);
    // The gloves the character holds, and the wing each glove carries.
    LoopingActions(MODEL_PHOENIX_SOUL_STAR);
    LoopingActions(MODEL_SWORD_35_WING);
    PreviewPlayer player(scope);
    CHARACTER* c = player.character.get();
    OBJECT* o = &c->Object;
    c->Weapon[0].Type = MODEL_PHOENIX_SOUL_STAR;
    c->Weapon[0].LinkBone = 33;
    c->Wing.Type = MODEL_WING;
    c->Helper.Type = MODEL_IMP;
    // The bone the extra pass hangs the wing from is elsewhere than the back bone.
    scope.bones[19][0][3] = 30.f;
    for (PART_t* part : {&c->Wing, &c->Helper})
    {
        part->LinkBone = 3;
        part->PlaySpeed = 0.7f;
    }

    PrepareCarriedParts(c);
    CHECK(c->Wing.LinkBone == 47);
    CHECK(c->Wing.PlaySpeed == 0.25f);
    CHECK(c->Helper.LinkBone == 34);
    CHECK(c->Helper.PlaySpeed == 0.5f);

    struct Result
    {
        PartFields wing;
        PartFields helper;
        float wingPosed;
        float impPosed;
        bool operator==(const Result&) const = default;
    };
    // From the same start: an update, the frame's passes, the next update.
    const auto run = [c](const std::function<void()>& passes)
    {
        RestAnimation(c->Wing);
        RestAnimation(c->Helper);
        UpdatePreviewCharacter(c);
        Attachments::BeginFrame();
        const Attachments::Slot* wing = Attachments::Find(c, &c->Wing, MODEL_WING, true);
        REQUIRE(wing != nullptr);
        const std::vector<float> planned(&Attachments::Bones(*wing)[0][0][0],
                                         &Attachments::Bones(*wing)[0][0][0] + MAX_BONES * 12);
        passes();
        // No pass touches what the update planned.
        CHECK(std::equal(planned.begin(), planned.end(), &Attachments::Bones(*wing)[0][0][0]));

        UpdatePreviewCharacter(c);
        Attachments::BeginFrame();
        wing = Attachments::Find(c, &c->Wing, MODEL_WING, true);
        const Attachments::Slot* imp = Attachments::Find(c, &c->Helper, MODEL_IMP, true);
        REQUIRE(wing != nullptr);
        REQUIRE(imp != nullptr);
        return Result{PartFields(c->Wing), PartFields(c->Helper), wing->posed.animationFrame,
                      imp->posed.animationFrame};
    };
    const auto normal = [c, o] { RenderCharacterBackItem(c, o, true); };
    // A pass that shows only the wing, hung from a bone of its own.
    const auto wingOnly = [c] { RenderLinkObject(0.f, 0.f, 15.f, c, &c->Wing, MODEL_WING, 0, 0, true, true, 0, true, 19); };

    const Result none = run([] {});
    // Two updates: standing, the wing at a quarter step, the Imp at half.
    CHECK(none.wing.animationFrame == doctest::Approx(0.5f));
    CHECK(none.helper.animationFrame == doctest::Approx(1.f));
    CHECK(none.wingPosed == none.wing.animationFrame);
    CHECK(none.impPosed == none.helper.animationFrame);
    CHECK(c->Weapon[0].LinkBone == PhoenixGloveBone(0));

    CHECK(run(normal) == none);
    CHECK(run([&] { normal(); wingOnly(); }) == none);
    CHECK(run([&] { wingOnly(); normal(); }) == none);
    CHECK(run([&] { normal(); normal(); }) == none);

    // The planned draw shows the update's bones; the other bone gets a pose of its own.
    const Attachments::Slot* wing = Attachments::Find(c, &c->Wing, MODEL_WING, true);
    REQUIRE(wing != nullptr);
    CHECK(RenderLinkObject(0.f, 0.f, 15.f, c, &c->Wing, MODEL_WING, 0, 0, false, true) == Attachments::Bones(*wing));
    CHECK(RenderLinkObject(0.f, 0.f, 15.f, c, &c->Wing, MODEL_WING, 0, 0, true, true, 0, true, 19) !=
          Attachments::Bones(*wing));
}

TEST_CASE("A weapon's shadow shows the weapon's own frame, not the rest pose of the same weapon on the back [effects]")
{
    namespace Attachments = Engine::Object::Attachments;
    CharacterScope scope(MODEL_PLAYER);
    LoopingActions(MODEL_PLAYER, MAX_PLAYER_ACTION);
    SlidingBone(MODEL_CRYSTAL_SWORD);
    // The character's bones are identities, so an attached part's pose shows its own frame.
    for (int i = 0; i < MAX_BONES; ++i)
    {
        std::memset(scope.bones[i], 0, sizeof(scope.bones[i]));
        scope.bones[i][0][0] = scope.bones[i][1][1] = scope.bones[i][2][2] = 1.f;
    }
    PreviewPlayer player(scope);
    CHARACTER* c = player.character.get();
    OBJECT* o = &c->Object;
    // Greeting, the weapons go on the back; outside a safe zone the weapons still cast their shadows.
    o->CurrentAction = PLAYER_GREETING1;
    for (int hand = 0; hand < 2; ++hand)
    {
        PART_t& weapon = c->Weapon[hand];
        weapon.Type = MODEL_CRYSTAL_SWORD;
        weapon.AnimationFrame = weapon.PriorAnimationFrame = 3.f;
    }

    UpdatePreviewCharacter(c);
    Attachments::BeginFrame();
    PART_t any;
    const Attachments::Slot* back = Attachments::Find(c, &any, MODEL_CRYSTAL_SWORD, true);
    REQUIRE(back != nullptr);
    // The weapon on the back is drawn at rest.
    CHECK(back->posed.animationFrame == 0.f);

    // Each hand's shadow pose, as the shadow pass draws it; then the same with nothing planned.
    const auto shadows = [c, o]
    {
        std::vector<float> roots;
        o->EnableShadow = true;
        for (int hand = 0; hand < 2; ++hand)
        {
            PART_t shadow = c->Weapon[hand];
            const vec34_t* bones = RenderLinkObject(0.f, 0.f, 0.f, c, &shadow, MODEL_CRYSTAL_SWORD, 0, 0, false, true, 0,
                                                    hand == 0);
            roots.insert(roots.end(), &bones[0][0][0], &bones[0][0][0] + 12);
        }
        o->EnableShadow = false;
        return roots;
    };
    const std::vector<float> withPlan = shadows();
    Attachments::BeginFrame();
    REQUIRE(Attachments::Find(c, &any, MODEL_CRYSTAL_SWORD, true) == nullptr);
    CHECK(shadows() == withPlan);
}

TEST_CASE("A flying Storm wing and the quest item it carries step the shared wing each at its own speed [effects]")
{
    namespace Attachments = Engine::Object::Attachments;
    CharacterScope scope(MODEL_PLAYER);
    LoopingActions(MODEL_PLAYER, MAX_PLAYER_ACTION);
    for (const int type : {static_cast<int>(MODEL_WING_OF_STORM), static_cast<int>(MODEL_DIVINE_STAFF_OF_ARCHANGEL),
                           static_cast<int>(MODEL_WING), static_cast<int>(MODEL_CAPE_OF_EMPEROR)})
        LoopingActions(type);
    PreviewPlayer player(scope);
    CHARACTER* c = player.character.get();
    OBJECT* o = &c->Object;
    const int world = gMapManager.WorldActive;
    gMapManager.WorldActive = WD_11BLOODCASTLE1;
    o->CurrentAction = PLAYER_FLY;
    c->EtcPart = 1;
    c->Wing.Type = MODEL_WING_OF_STORM;

    UpdatePreviewCharacter(c);
    Attachments::BeginFrame();
    // The quest item steps first, at its full speed; the wing then adds its half step.
    const Attachments::Slot* quest = Attachments::Find(c, &c->Wing, MODEL_DIVINE_STAFF_OF_ARCHANGEL, true);
    const Attachments::Slot* wing = Attachments::Find(c, &c->Wing, MODEL_WING_OF_STORM, true);
    REQUIRE(quest != nullptr);
    REQUIRE(wing != nullptr);
    CHECK(quest->posed.animationFrame == doctest::Approx(1.f));
    CHECK(wing->posed.animationFrame == doctest::Approx(1.5f));
    CHECK(c->Wing.AnimationFrame == doctest::Approx(1.5f));

    // Repeated passes add no step.
    RepeatedDraws::CheckChangesNothing([c, o] { RenderCharacterBackItem(c, o, true); });
    UpdatePreviewCharacter(c);
    CHECK(c->Wing.AnimationFrame == doctest::Approx(3.f));

    // Without the quest item: an ordinary wing in flight takes full steps, a standing cape quarter steps.
    c->EtcPart = 0;
    c->Wing.Type = MODEL_WING;
    RestAnimation(c->Wing);
    UpdatePreviewCharacter(c);
    CHECK(c->Wing.AnimationFrame == doctest::Approx(1.f));
    c->Wing.Type = MODEL_CAPE_OF_EMPEROR;
    o->CurrentAction = 0;
    RestAnimation(c->Wing);
    UpdatePreviewCharacter(c);
    Attachments::BeginFrame();
    CHECK(c->Wing.AnimationFrame == doctest::Approx(0.25f));
    const Attachments::Slot* cape = Attachments::Find(c, &c->Wing, MODEL_CAPE_OF_EMPEROR, true);
    REQUIRE(cape != nullptr);
    CHECK(cape->linkBone == 19);

    gMapManager.WorldActive = world;
}

TEST_CASE("The Red Dragon's and the Saint statues' carried models play on a wing the update prepares [effects]")
{
    for (const int monster : {static_cast<int>(MONSTER_RED_DRAGON), static_cast<int>(MONSTER_STATUE_OF_SAINT_1),
                              static_cast<int>(MONSTER_STATUE_OF_SAINT_2), static_cast<int>(MONSTER_STATUE_OF_SAINT_3)})
    {
        CharacterScope scope(MODEL_PLAYER);
        CHARACTER* c = &scope.character();
        OBJECT* o = &c->Object;
        o->Type = MODEL_PLAYER + 1;
        c->MonsterIndex = static_cast<EMonsterType>(monster);
        c->Wing.Type = -1;
        Vector(0.4f, 0.5f, 0.6f, c->Light);
        REQUIRE(RendersWingCarriedModels(c));

        PrepareCarriedParts(c);
        CHECK(c->Wing.CurrentAction == 1);
        CHECK(c->Wing.PriorAction == 1);
        CHECK(c->Wing.PlaySpeed == 0.2f);
        const PartFields wing(c->Wing);

        RepeatedDraws::CheckChangesNothing([c] { RenderWingCarriedModels(c, true); });
        CHECK(PartFields(c->Wing) == wing);
        CHECK(o->Scale == 1.f);
        CHECK(c->Light[0] == 0.4f);
    }
}

TEST_CASE("Starting or stopping music while drawing is a violation [effects]")
{
    Diagnostics::DrawSpawnCheck::ResetViolations();
    {
        const Diagnostics::DrawSpawnCheck::DrawScope drawing;
        PlayMp3("Music/MuTheme.mp3");
        StopMp3("Music/MuTheme.mp3");
    }
    CHECK(Diagnostics::DrawSpawnCheck::Violations() == 2);
    Diagnostics::DrawSpawnCheck::ResetViolations();
}

TEST_CASE("Every value Dark Side's fingerprint reads starts defined, whatever memory the system is built in [effects]")
{
    // Built over zeros and over a pattern, a member the constructor leaves unset would differ.
    alignas(CMonkSystem) unsigned char zeroed[sizeof(CMonkSystem)];
    alignas(CMonkSystem) unsigned char patterned[sizeof(CMonkSystem)];
    std::memset(zeroed, 0, sizeof(zeroed));
    std::memset(patterned, 0xA5, sizeof(patterned));
    CMonkSystem* first = new (zeroed) CMonkSystem;
    CMonkSystem* second = new (patterned) CMonkSystem;
    CHECK(first->DarksideStateHash() == second->DarksideStateHash());
    first->~CMonkSystem();
    second->~CMonkSystem();
}

TEST_CASE("Dark Side's fingerprint follows a dummy's step, not only how many dummies there are [effects]")
{
    CMonkSystem monk;
    vec3_t position{1000.f, 1000.f, 0.f};
    vec3_t target{2000.f, 1000.f, 0.f};
    monk.SetDummy(position, target, 1);
    const std::uint64_t before = monk.DarksideStateHash();

    vec3_t placed;
    float frame = 0.f;
    monk.IsDummyRender(placed, target, frame, 1);
    CHECK(monk.DarksideStateHash() != before);
}
