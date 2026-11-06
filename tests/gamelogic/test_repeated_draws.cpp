#include "stdafx.h"
#include "doctest.h"

#include "Camera/CameraProjection.h"
#include "App/Control/DrawPurityCheck.h"
#include "Camera/CameraState.h"
#include "Core/Utilities/Random.h"
#include "Engine/Object/CharacterAttachments.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Physics/PhysicsManager.h"
#include "GameLogic/Social/MonkSystem.h"
#include "Render/Lighting/DynamicLightHistory.h"
#include "Scenes/SceneCore.h"
#include "Render/Effects/EffectDraw.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Drawing/DrawCallLog.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "World/GameMaps/GMHellas.h"
#include "World/MapInfra/MapManager.h"

#include "RepeatedDraws.h"

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

extern unsigned int WindowWidth;
extern unsigned int WindowHeight;
extern double WorldTime;
extern float FPS_ANIMATION_FACTOR;
extern unsigned char TerrainMappingLayer1[];
extern float TerrainMappingAlpha[];

namespace
{
// A frame's view over the middle of the map, built as the update builds it; the camera is given back afterwards.
struct ViewScope
{
    const CameraState savedCamera = g_Camera;
    const unsigned int savedWidth = WindowWidth;
    const unsigned int savedHeight = WindowHeight;

    ViewScope()
    {
        WindowWidth = 1280;
        WindowHeight = 720;
        g_Camera = CameraState();
        Vector(-48.5f, 0.f, 33.3f, g_Camera.Angle);
        Vector(12800.f, 12800.f, 900.f, g_Camera.Position);
        CameraProjection::UpdateView(g_Camera, 0, 0, 1280, 720);
        CreateFrustrum(1.f, 1.f);
    }

    ~ViewScope()
    {
        g_Camera = savedCamera;
        WindowWidth = savedWidth;
        WindowHeight = savedHeight;
    }

    // A point `distance` along the view, behind the camera when negative.
    void Ahead(float distance, vec3_t out) const
    {
        for (int i = 0; i < 3; i++)
            out[i] = g_Camera.Position[i] - g_Camera.Matrix[2][i] * distance;
    }
};

void ClearEffects()
{
    for (int i = 0; i < MAX_EFFECTS; ++i)
        Effects[i].Live = false;
}

int CreateEffectAt(const vec3_t position)
{
    vec3_t at;
    VectorCopy(position, at);
    vec3_t angle{0.f, 0.f, 0.f};
    vec3_t light{1.f, 1.f, 1.f};
    CreateEffect(BITMAP_MAGIC, at, angle, light, 8);
    for (int i = MAX_EFFECTS - 1; i >= 0; --i)
    {
        if (Effects[i].Live && VectorCompare(Effects[i].Position, position))
            return i;
    }
    return -1;
}
} // namespace

TEST_CASE(
    "The update culls the effects against the frame's view, and the effect pass only draws what it marked [effects]")
{
    ViewScope view;
    ClearEffects();
    vec3_t ahead;
    vec3_t behind;
    view.Ahead(1000.f, ahead);
    view.Ahead(-1000.f, behind);
    const int shown = CreateEffectAt(ahead);
    const int hidden = CreateEffectAt(behind);
    REQUIRE(shown >= 0);
    REQUIRE(hidden >= 0);

    CullEffects();
    CHECK(Effects[shown].Visible);
    CHECK_FALSE(Effects[hidden].Visible);
    const unsigned int marked = Effects[shown].DrawnFrame;

    CullEffects();
    CHECK(Effects[shown].DrawnFrame == marked + 1);

    ClearEffects();
}

// The draw parity check restores this snapshot after each run, so it must hold every sprite's flag.
TEST_CASE("The snapshot of drawn sprites keeps the flag of every sprite [effects]")
{
    SpriteDrawFlags drawn{};
    for (size_t sprite = 0; sprite < drawn.size(); ++sprite)
    {
        drawn[sprite] = sprite % 3 == 0;
    }
    SetSpritesDrawnThisFrame(drawn);
    const SpriteDrawFlags snapshot = SpritesDrawnThisFrame();
    BeginSpriteDraws();

    CHECK(snapshot == drawn);
    CHECK(SpritesDrawnThisFrame() == SpriteDrawFlags{});
}

TEST_CASE("Drawing the effect passes again and again changes nothing the next update reads [effects]")
{
    ViewScope view;
    ClearEffects();
    vec3_t ahead;
    view.Ahead(1000.f, ahead);
    REQUIRE(CreateEffectAt(ahead) >= 0);
    CullEffects();

    RepeatedDraws::CheckChangesNothing(
        []
        {
            RenderEffects();
            RenderEffects(true);
        },
        3);

    ClearEffects();
}

TEST_CASE("A draw sends an effect to the after-character pass for its own frame, without marking the effect "
          "[effects]")
{
    ViewScope view;
    ClearEffects();
    OBJECT& storm = Effects[0];
    storm.Live = true;
    storm.Type = MODEL_STORM3;
    storm.Visible = true;

    BeginObjectDraws();
    RepeatedDraws::CheckChangesNothing([] { RenderEffects(); }, 3);
    CHECK(IsSentAfterCharacter(&storm));
    CHECK_FALSE(storm.m_bRenderAfterCharacter);

    // The next frame's draw decides again.
    BeginObjectDraws();
    CHECK_FALSE(IsSentAfterCharacter(&storm));
    ClearEffects();
}

namespace
{
// Keeps the lights of the ground marks it is handed.
class DecalLights final : public Render::EffectDrawing::EffectDrawer
{
public:
    std::vector<float> lights;

    void SetState(const Render::EffectDrawing::EffectState&) override {}
    void DrawSprite(const Render::EffectDrawing::SpriteDraw&) override {}
    void DrawQuads(const Render::EffectDrawing::QuadsDraw&) override {}
    void DrawDecal(const Render::EffectDrawing::DecalDraw& draw) override
    {
        lights.insert(lights.end(), draw.light, draw.light + 3);
    }
    void DrawCircle(const Render::EffectDrawing::CircleDraw&) override {}
    void DrawPlane(const Render::EffectDrawing::PlaneDraw&) override {}
};
} // namespace

TEST_CASE("A draw's random value is the same in every pass of a frame and takes nothing from the update's sequences "
          "[effects]")
{
    int key = 0;
    Random::SeedRand(914);
    const int next = Random::Rand();
    Random::SeedRand(914);
    const std::mt19937 engine = Random::Snapshot();

    const int first = Random::DrawRand(&key, 1);
    CHECK(Random::DrawRand(&key, 1) == first);
    CHECK(Random::Rand() == next);
    CHECK(Random::Snapshot() == engine);

    // Over frames it spreads like Random::Rand() does.
    int changed = 0;
    long long sum = 0;
    for (int frame = 0; frame < 1000; ++frame)
    {
        Random::NextDrawFrame();
        const int value = Random::DrawRand(&key, 1);
        CHECK(value >= 0);
        CHECK(value <= 32767);
        changed += value != first ? 1 : 0;
        sum += value;
    }
    CHECK(changed > 990);
    CHECK(sum / 1000 > 15000);
    CHECK(sum / 1000 < 17800);
}

TEST_CASE("A draw's frame check passes as often as rand_fps_check does [effects]")
{
    int key = 0;
    int passed = 0;
    for (int frame = 0; frame < 20000; ++frame)
    {
        Random::NextDrawFrame();
        passed += Random::DrawFpsCheck(20, 1.0, &key, 1) ? 1 : 0;
    }
    CHECK(passed > 850);
    CHECK(passed < 1150);
}

TEST_CASE(
    "An effect's flickering ground mark is the same in every pass of a frame, and draws from no sequence [effects]")
{
    OBJECT flame{};
    flame.Live = true;
    flame.Type = BITMAP_FLAME;
    flame.SubType = 0;
    flame.Scale = 1.f;

    DecalLights first;
    DecalLights second;
    Render::EffectDrawing::SetEffectDrawer(&first);
    RepeatedDraws::CheckChangesNothing(
        [&flame] { DescribeEffectShadow(flame, Render::EffectDrawing::ActiveEffectDrawer()); }, 1);
    Render::EffectDrawing::SetEffectDrawer(&second);
    RepeatedDraws::CheckChangesNothing(
        [&flame] { DescribeEffectShadow(flame, Render::EffectDrawing::ActiveEffectDrawer()); }, 1);
    Render::EffectDrawing::SetEffectDrawer(nullptr);

    REQUIRE_FALSE(first.lights.empty());
    CHECK(first.lights == second.lights);
}

TEST_CASE("The update marks the map objects the frame's view shows, and only those [effects]")
{
    ViewScope view;
    // The 2D cull lets everything through outside the world scenes.
    const EGameScene scene = SceneFlag;
    SceneFlag = MAIN_SCENE;
    OBJECT shown{};
    shown.Live = true;
    shown.CollisionRange = 0.f;
    view.Ahead(1000.f, shown.Position);
    OBJECT hidden{};
    hidden.Live = true;
    Vector(200.f, 200.f, 0.f, hidden.Position); // across the map from the camera
    shown.Next = &hidden;
    // The block that covers the shown object: 16 tiles of 100 a side.
    const int column = static_cast<int>(shown.Position[0] / 100.f) / 16;
    const int row = static_cast<int>(shown.Position[1] / 100.f) / 16;
    OBJECT_BLOCK& block = ObjectBlock[column * 16 + row];
    OBJECT* const previousHead = block.Head;
    block.Head = &shown;

    CullObjects();
    const unsigned int marked = shown.DrawnFrame;
    CHECK(shown.Visible);
    CHECK_FALSE(hidden.Visible);
    CHECK(hidden.DrawnFrame != marked);

    CullObjects();
    CHECK(shown.DrawnFrame == marked + 1);

    block.Head = previousHead;
    SceneFlag = scene;
}

#ifdef MU_ENABLE_CONTROL_SOCKET
TEST_CASE("The draw purity check counts a draw pass that draws from the update's random sequences [effects]")
{
    namespace Purity = Diagnostics::DrawPurityCheck;
    const auto counted = [](const char* what)
    {
        int draws = 0;
        for (const Purity::Change& change : Purity::Report(draws))
        {
            if (change.what == what)
                return change.count;
        }
        return 0;
    };

    Purity::Start();
    Purity::BeforeDraw();
    Purity::AfterDraw();
    Purity::BeforeDraw();
    (void)Random::Rand();
    Purity::AfterDraw();
    Purity::BeforeDraw();
    (void)Random::Unit();
    Purity::AfterDraw();

    CHECK(counted("random legacy sequence") == 1);
    CHECK(counted("random engine") == 1);
}

TEST_CASE("The draw purity check reports a draw that changes parts, previews, sprites, buffs, the terrain, the light "
          "history, cloth or Dark Side [effects]")
{
    namespace Purity = Diagnostics::DrawPurityCheck;
    const auto reported = [](const std::string& kind)
    {
        int draws = 0;
        for (const Purity::Change& change : Purity::Report(draws))
        {
            if (change.what.rfind(kind, 0) == 0)
                return true;
        }
        return false;
    };
    // Runs `change` as a draw pass would, and tells whether the check reported `kind`.
    const auto caught = [&reported](const std::string& kind, const auto& change)
    {
        Purity::Start();
        Purity::BeforeDraw();
        change();
        Purity::AfterDraw();
        return reported(kind);
    };

    CHARACTER* const savedCharacters = CharactersClient;
    auto characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT + 1);
    CharactersClient = characters.get();
    CHECK(caught("character part", [] { CharactersClient[2].Helper.LinkBone = 34; }));
    // A part's cloth whose wind changes, with no vertex moved.
    CPhysicsCloth cape;
    CharactersClient[2].Wing.m_byNumCloth = 1;
    CharactersClient[2].Wing.m_pCloth[0] = &cape;
    CHECK(caught("cloth", [&cape] { cape.SetWindMinMax(3, 9); }));
    // A collider the next step pushes the cloth out of.
    CHECK(caught("cloth", [&cape] { cape.AddCollisionSphere(0.f, 0.f, 20.f, 25.f, 17); }));
    cape.Destroy();
    CharactersClient[2].Wing.m_pCloth[0] = nullptr;
    CharactersClient[2].Wing.m_byNumCloth = 0;
    CharactersClient = savedCharacters;

    auto preview = std::make_unique<CHARACTER>();
    Engine::Object::Attachments::Track(preview.get());
    CHECK(caught("preview", [&preview] { preview->Object.Alpha = 0.5f; }));
    CHECK(caught("character part", [&preview] { preview->Wing.PlaySpeed = 0.25f; }));
    Engine::Object::Attachments::Untrack(preview.get());

    CHECK(caught("sprite", [] { Sprites[0].Live = !Sprites[0].Live; }));
    CHECK(caught("item", [] { Items[0].Object.m_BuffMap.RegisterBuff(eBuff_Cloaking); }));
    Items[0].Object.m_BuffMap.ClearBuff();
    CHECK(caught("terrain walls", [] { TerrainWall[TERRAIN_INDEX(10, 10)] ^= TW_NOMOVE; }));
    CHECK(caught("terrain light", [] { PrimaryTerrainLight[TERRAIN_INDEX(10, 10)][0] += 1.f; }));
    CHECK(caught("light history", [] { Render::Lighting::FrameLightHistory().BeginTick(); }));
    CHECK(caught("dark side", [] { g_CMonkSystem.SetRepeatedly(100, 0, false, false); }));
}

TEST_CASE("A cloth vertex's fingerprint follows the move it has pending for the next step [effects]")
{
    const auto fingerprint = [](const CPhysicsVertex& vertex)
    {
        Fnv1aHash hash;
        vertex.AddStateTo(hash);
        return hash.Value();
    };
    CPhysicsVertex vertex;
    const std::uint64_t before = fingerprint(vertex);
    vec3_t move = {4.f, 0.f, 0.f};
    vertex.AddOneTimeMove(move);
    CHECK(fingerprint(vertex) != before);

    // The next step moves the vertex by it.
    vertex.DoOneTimeMove();
    vec3_t position;
    vertex.GetPosition(&position);
    CHECK(position[0] == 4.f);
}

TEST_CASE("The draw purity check draws nothing from the update's random sequences itself [effects]")
{
    namespace Purity = Diagnostics::DrawPurityCheck;
    Random::SeedRand(914);
    const int next = Random::Rand();
    const std::mt19937 engine = Random::Snapshot();

    Random::SeedRand(914);
    Purity::Start();
    for (int pass = 0; pass < 2; ++pass)
    {
        Purity::BeforeDraw();
        Purity::AfterDraw();
    }
    CHECK(Random::Rand() == next);
    CHECK(Random::Snapshot() == engine);
}
#endif

TEST_CASE("The game's legacy sequence gives what the Windows CRT's rand() gives, on every platform [effects]")
{
    Random::SeedRand(1);
    CHECK(Random::Rand() == 41);
    CHECK(Random::Rand() == 18467);
    Random::SeedRand(914);
    for (const int expected : {3023, 1362, 30627, 8977, 31960})
        CHECK(Random::Rand() == expected);
#ifdef _WIN32
    Random::SeedRand(77);
    std::srand(77);
    for (int i = 0; i < 1000; ++i)
        CHECK(Random::Rand() == std::rand());
#endif
    // Reading the state draws nothing.
    const std::uint32_t state = Random::RandState();
    CHECK(Random::RandState() == state);
}

TEST_CASE("A copy drawn in place of an object draws as that object while its scope lasts [effects]")
{
    OBJECT source{};
    OBJECT copy{};
    const int alone = Random::DrawRand(&copy, 1);
    {
        const Random::DrawnAs drawnAs(&copy, &source);
        CHECK(Random::DrawKey(&copy) == &source);
        CHECK(Random::DrawRand(&copy, 1) == Random::DrawRand(&source, 1));
        // A copy of the copy is the source too.
        OBJECT inner{};
        const Random::DrawnAs innerDrawnAs(&inner, &copy);
        CHECK(Random::DrawKey(&inner) == &source);

        BeginObjectDraws();
        SendAfterCharacter(&copy);
        CHECK(IsSentAfterCharacter(&source));
    }
    CHECK(Random::DrawKey(&copy) == &copy);
    CHECK(Random::DrawRand(&copy, 1) == alone);
    BeginObjectDraws();
}

TEST_CASE("A sprite is visible from its creation, its draw sets nothing on it [effects]")
{
    for (int i = 0; i < MAX_SPRITES; ++i)
        Sprites[i].Live = false;
    vec3_t position{0.f, 0.f, 0.f};
    vec3_t light{1.f, 1.f, 1.f};
    Sprites[0].Visible = false;
    CreateSprite(BITMAP_LIGHT, position, 1.f, light, nullptr);
    REQUIRE(Sprites[0].Live);
    CHECK(Sprites[0].Visible);
    Sprites[0].Live = false;
}

TEST_CASE("An effect drawn as its owner's weapon draws a copy, the effect stays as the update left it [effects]")
{
    ClearEffects();
    const std::unique_ptr<BMD[]> models(new BMD[MAX_MODELS]);
    BMD* previousModels = Models;
    Models = models.get();
    const auto owner = std::make_unique<CHARACTER>();
    owner->Object.Weapon = 0;
    CHARACTER* previousHero = Hero;
    Hero = owner.get();
    for (int type : {MODEL_SKILL_WHEEL1, MODEL_SKILL_FURY_STRIKE})
    {
        OBJECT& effect = Effects[0];
        effect.Live = true;
        effect.Visible = true;
        effect.Type = type;
        effect.Kind = 0;
        effect.LifeTime = 20;
        effect.Alpha = 1.f;
        effect.Owner = &owner->Object;
        Vector(100.f, 200.f, 0.f, effect.Position);
        Vector(0.f, 0.f, 10.f, effect.Angle);
        Vector(0.f, 0.f, 5.f, effect.Direction);

        RepeatedDraws::CheckChangesNothing([] { RenderEffects(); }, 3);
        CHECK(effect.Type == type);
        CHECK(effect.Position[2] == 0.f);
        CHECK(effect.Angle[2] == 10.f);
    }
    Hero = previousHero;
    Models = previousModels;
    ClearEffects();
}

TEST_CASE("A Skill Spear's blend mesh light is set where it is created, not by its draw [effects]")
{
    ClearEffects();
    vec3_t position{100.f, 200.f, 0.f};
    vec3_t angle{0.f, 0.f, 0.f};
    vec3_t light{1.f, 1.f, 1.f};
    CreateEffect(MODEL_SPEARSKILL, position, angle, light);
    int spear = -1;
    for (int i = 0; i < MAX_EFFECTS; ++i)
        if (Effects[i].Live && Effects[i].Type == MODEL_SPEARSKILL)
            spear = i;
    REQUIRE(spear >= 0);
    CHECK(Effects[spear].BlendMeshLight == 1.f);
    ClearEffects();
}

TEST_CASE("Drawing Kalima's water again and again leaves its waves as the update stepped them [effects]")
{
    auto hero = std::make_unique<CHARACTER>();
    hero->PositionX = 100;
    hero->PositionY = 100;
    CHARACTER* const savedHero = Hero;
    Hero = hero.get();
    const double savedTime = WorldTime;
    const float savedFactor = FPS_ANIMATION_FACTOR;
    WorldTime = 0.0;
    FPS_ANIMATION_FACTOR = 1.f;
    REQUIRE(CreateWaterTerrain(WD_24HELLAS));
    AddWaterWave(100, 100, 8, 2000);
    MoveWaterTerrain();

    const auto heights = []
    {
        std::vector<float> h;
        for (int y = 90; y <= 110; ++y)
            for (int x = 90; x <= 110; ++x)
                h.push_back(GetWaterTerrain(x * TERRAIN_SCALE, y * TERRAIN_SCALE));
        return h;
    };
    const std::vector<float> stepped = heights();

    Render::Drawing::DrawCallLog log;
    mu::SetRendererOverride(&log);
    RepeatedDraws::CheckChangesNothing([] { RenderWaterTerrain(); }, 3);
    mu::SetRendererOverride(nullptr);
    CHECK_FALSE(log.calls.empty());
    CHECK(heights() == stepped);

    MoveWaterTerrain();
    CHECK(heights() != stepped);

    DeleteWaterTerrain();
    FPS_ANIMATION_FACTOR = savedFactor;
    WorldTime = savedTime;
    Hero = savedHero;
}

TEST_CASE("Crywolf's water spots are rolled in the update, once for the water the last terrain pass drew [effects]")
{
    ViewScope view;
    auto hero = std::make_unique<CHARACTER>();
    CHARACTER* const savedHero = Hero;
    Hero = hero.get();
    const int savedWorld = gMapManager.WorldActive;
    const float savedFactor = FPS_ANIMATION_FACTOR;
    gMapManager.WorldActive = WD_34CRYWOLF_1ST;
    FPS_ANIMATION_FACTOR = 1.f;
    constexpr int tiles = TERRAIN_SIZE * TERRAIN_SIZE;
    const std::vector<unsigned char> savedLayer(TerrainMappingLayer1, TerrainMappingLayer1 + tiles);
    const std::vector<float> savedAlpha(TerrainMappingAlpha, TerrainMappingAlpha + tiles);
    // Water everywhere: a first layer of texture 5, half covered.
    std::fill(TerrainMappingLayer1, TerrainMappingLayer1 + tiles, static_cast<unsigned char>(5));
    std::fill(TerrainMappingAlpha, TerrainMappingAlpha + tiles, 0.5f);
    const auto clearParticles = []
    {
        for (int i = 0; i < MAX_PARTICLES; ++i)
            Particles[i].Live = false;
    };
    clearParticles();

    Render::Drawing::DrawCallLog log;
    // The random state after `passes` terrain passes and the update after them.
    const auto afterUpdate = [&log](int passes)
    {
        mu::SetRendererOverride(&log);
        RepeatedDraws::CheckChangesNothing([] { RenderTerrain(false); }, passes);
        mu::SetRendererOverride(nullptr);
        Random::Seed(914);
        UpdateTerrainWaterSpots();
        return Random::Snapshot();
    };
    Random::Seed(914);
    const std::mt19937 seeded = Random::Snapshot();
    const std::mt19937 once = afterUpdate(1);
    const std::mt19937 thrice = afterUpdate(3);
    CHECK(once != seeded);
    CHECK(thrice == once);
    CHECK(std::any_of(Particles, Particles + MAX_PARTICLES, [](const PARTICLE& p) { return p.Live; }));

    // The update took the notes: the next one rolls nothing.
    Random::Seed(914);
    UpdateTerrainWaterSpots();
    CHECK(Random::Snapshot() == seeded);

    clearParticles();
    std::copy(savedLayer.begin(), savedLayer.end(), TerrainMappingLayer1);
    std::copy(savedAlpha.begin(), savedAlpha.end(), TerrainMappingAlpha);
    FPS_ANIMATION_FACTOR = savedFactor;
    gMapManager.WorldActive = savedWorld;
    Hero = savedHero;
}

TEST_CASE("A frame with no update since the last draws the sprites again, a pass in the same frame doesn't [effects]")
{
    ViewScope view;
    ClearSprites();
    vec3_t ahead;
    view.Ahead(1000.f, ahead);
    vec3_t light{1.f, 1.f, 1.f};
    REQUIRE(CreateSprite(BITMAP_LIGHT, ahead, 1.f, light, nullptr) >= 0);

    Render::Drawing::DrawCallLog log;
    mu::SetRendererOverride(&log);
    BeginSpriteDraws();
    RenderSprites();
    const std::size_t oneFrame = log.calls.size();
    RenderSprites();
    const std::size_t samePass = log.calls.size();
    BeginSpriteDraws();
    RenderSprites();
    const std::size_t nextFrame = log.calls.size();
    mu::SetRendererOverride(nullptr);

    CHECK(oneFrame > 0);
    CHECK(samePass == oneFrame);
    CHECK(nextFrame - samePass == oneFrame);
    ClearSprites();
}
