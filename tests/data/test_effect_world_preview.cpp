#include "stdafx.h"

#include "doctest.h"

// The world preview of the effect browser (FX1.7b): where it creates a type,
// how it finds and follows what the game's call created, what it refuses,
// and a run with the game's own create call.
#ifdef _EDITOR
#include "EffectTestData.h"

#include "Audio/EditorSoundMute.h"
#include "Core/Globals/_enum.h"
#include "Core/MuEditorCore.h"
#include "Core/Utilities/AssetLoadWorld.h"
#include "Core/Utilities/WorldClearing.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "UI/EffectBrowser/EffectPoolSnapshot.h"
#include "UI/EffectBrowser/EffectPreviewTracker.h"
#include "UI/EffectBrowser/EffectWorldPreview.h"
#include "World/MapInfra/MapManager.h"

#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <span>
#include <vector>

using namespace MuEditor::Effects;
using Data::Effects::EffectKind;

namespace
{
constexpr float Tolerance = 1e-3f;

// Small pools for the tests, on the heap: an OBJECT is large.
struct TestPools
{
    std::array<OBJECT, 6> effects{};
    std::array<OBJECT, 3> skillEffects{};
    std::array<OBJECT, 3> sprites{};
    std::array<PARTICLE, 6> particles{};
    std::array<JOINT, 4> joints{};

    EffectPools Spans()
    {
        return {effects, skillEffects, sprites, particles, joints};
    }
};

void Live(OBJECT& object, int type, OBJECT* owner = nullptr)
{
    object.Live = true;
    object.Type = type;
    object.Owner = owner;
}

void Live(PARTICLE& particle, int type, OBJECT* target)
{
    particle.Live = true;
    particle.Type = type;
    particle.Target = target;
}

void Live(JOINT& joint, int type, OBJECT* target)
{
    joint.Live = true;
    joint.Type = type;
    joint.Target = target;
}

int g_removedEffects = 0;

// The effect slot a listener of the map code looks at, and what it saw.
int g_watchedEffect = 0;
int g_clearingsTold = 0;
bool g_watchedLiveWhenTold = false;

void WatchClearing()
{
    ++g_clearingsTold;
    g_watchedLiveWhenTold = Effects[g_watchedEffect].Live;
}

// Removes an effect of the test pools.
void RemoveTestEffect(OBJECT& effect)
{
    effect.Live = false;
    ++g_removedEffects;
}

bool Has(const std::vector<EffectPoolSlot>& slots, EffectPool pool, int index)
{
    return std::any_of(slots.begin(), slots.end(),
                       [&](const EffectPoolSlot& slot) { return slot.pool == pool && slot.index == index; });
}

// The live slot of the game's effects that holds `type` and belongs to
// `owner`, other than `except`.
std::optional<int> FindEffect(int type, const OBJECT* owner, std::optional<int> except = std::nullopt)
{
    for (int i = 0; i < MAX_EFFECTS; ++i)
    {
        if (Effects[i].Live && Effects[i].Type == type && Effects[i].Owner == owner && std::optional<int>(i) != except)
            return i;
    }
    return std::nullopt;
}

// The game's character for a test: Hero points to it until the end.
class TestHero
{
public:
    TestHero() : m_character(std::make_unique<CHARACTER>()), m_previous(Hero)
    {
        OBJECT& o = m_character->Object;
        o.Live = true;
        Vector(1000.0f, 1000.0f, 0.0f, o.Position);
        Vector(0.0f, 0.0f, 0.0f, o.Angle);
        Hero = m_character.get();
    }

    ~TestHero()
    {
        Hero = m_previous;
    }

    TestHero(const TestHero&) = delete;
    TestHero& operator=(const TestHero&) = delete;

    OBJECT& Object()
    {
        return m_character->Object;
    }

private:
    std::unique_ptr<CHARACTER> m_character;
    CHARACTER* m_previous;
};
} // namespace

TEST_CASE("The world preview creates a type two tiles in front of the character as the game turns forward "
          "[effects][editor]")
{
    const PreviewVector north = PlaceInFrontOf({100.0f, 200.0f, 5.0f}, 0.0f, 200.0f);
    CHECK(north[0] == doctest::Approx(100.0f).epsilon(Tolerance));
    CHECK(north[1] == doctest::Approx(0.0f).epsilon(Tolerance));
    CHECK(north[2] == doctest::Approx(5.0f).epsilon(Tolerance));
    const PreviewVector east = PlaceInFrontOf({100.0f, 200.0f, 5.0f}, 90.0f, 200.0f);
    CHECK(east[0] == doctest::Approx(300.0f).epsilon(Tolerance));
    CHECK(east[1] == doctest::Approx(200.0f).epsilon(Tolerance));
    const PreviewVector south = PlaceInFrontOf({100.0f, 200.0f, 5.0f}, 180.0f, 200.0f);
    CHECK(south[0] == doctest::Approx(100.0f).epsilon(Tolerance));
    CHECK(south[1] == doctest::Approx(400.0f).epsilon(Tolerance));
}

TEST_CASE("The pool snapshot finds the slots filled since, in every pool, also when refilled with another type "
          "[effects][editor]")
{
    auto pools = std::make_unique<TestPools>();
    Live(pools->effects[1], 5);
    Live(pools->joints[0], 7, nullptr);
    EffectPoolSnapshot snapshot;
    snapshot.Take(pools->Spans());

    pools->effects[1].Type = 6;
    Live(pools->effects[2], 5);
    Live(pools->skillEffects[0], 8);
    Live(pools->sprites[2], 9);
    Live(pools->particles[3], 10, nullptr);
    const std::vector<EffectPoolSlot> filled = snapshot.NewSince(pools->Spans());

    CHECK(filled.size() == 5);
    CHECK(Has(filled, EffectPool::Effect, 1));
    CHECK(Has(filled, EffectPool::Effect, 2));
    CHECK(Has(filled, EffectPool::SkillEffect, 0));
    CHECK(Has(filled, EffectPool::Sprite, 2));
    CHECK(Has(filled, EffectPool::Particle, 3));
    CHECK_FALSE(Has(filled, EffectPool::Joint, 0));
}

TEST_CASE("The world preview keeps what its call created and what that created in turn, and removes only that "
          "[effects][editor]")
{
    auto pools = std::make_unique<TestPools>();
    OBJECT character{};
    // The game's own objects: an effect, and a particle whose stale target
    // is the slot the call will fill.
    Live(pools->effects[0], 1);
    Live(pools->particles[2], 20, &pools->effects[1]);

    EffectPreviewTracker tracker;
    CHECK(tracker.IsEmpty());
    tracker.BeginCreate(pools->Spans());
    Live(pools->effects[1], 10, &character);
    Live(pools->skillEffects[1], 11, &character);
    Live(pools->sprites[0], 12);
    CHECK(tracker.EndCreate(pools->Spans()) == 3);
    CHECK(tracker.Count().effects == 2);
    CHECK(tracker.AnyCreatedLive(pools->Spans()));

    // The next frames: a child, its particle, lightning to the skill effect,
    // and objects of the game.
    Live(pools->effects[2], 12, &pools->effects[1]);
    Live(pools->particles[0], 21, &pools->effects[2]);
    Live(pools->joints[1], 30, &pools->skillEffects[1]);
    Live(pools->particles[1], 22, &character);
    Live(pools->effects[3], 13, &pools->effects[0]);
    tracker.Update(pools->Spans());
    CHECK(tracker.Count().effects == 3);
    CHECK(tracker.Count().particles == 1);
    CHECK(tracker.Count().joints == 1);

    // The child ends and the game fills its slot with another type.
    Live(pools->effects[2], 99);
    tracker.Update(pools->Spans());
    CHECK(tracker.Count().effects == 2);

    g_removedEffects = 0;
    tracker.RemoveAll(pools->Spans(), &RemoveTestEffect);
    CHECK(g_removedEffects == 2);
    CHECK_FALSE(pools->effects[1].Live);
    CHECK_FALSE(pools->skillEffects[1].Live);
    CHECK_FALSE(pools->particles[0].Live);
    CHECK(pools->particles[0].Target == nullptr);
    CHECK_FALSE(pools->joints[1].Live);
    CHECK(pools->effects[0].Live);
    CHECK(pools->effects[2].Live);
    CHECK(pools->effects[3].Live);
    CHECK(pools->particles[1].Live);
    CHECK(pools->particles[2].Live);
    CHECK(tracker.IsEmpty());
}

TEST_CASE("The world preview tells a refilled slot by its type: a follower is kept, the game's object owns nothing "
          "[effects][editor]")
{
    auto pools = std::make_unique<TestPools>();
    EffectPreviewTracker tracker;
    tracker.BeginCreate(pools->Spans());
    Live(pools->effects[1], 10);
    tracker.EndCreate(pools->Spans());
    Live(pools->particles[0], 20, &pools->effects[1]);
    Live(pools->effects[2], 11, &pools->effects[1]);
    tracker.Update(pools->Spans());
    REQUIRE(tracker.Count().particles == 1);
    REQUIRE(tracker.Count().effects == 2);

    // Within one frame the followed particle ends and the effect's next
    // particle, of another type, takes its slot.
    Live(pools->particles[0], 21, &pools->effects[1]);
    // The followed child ends and the game puts an effect of its own there,
    // whose particle must not count as the preview's.
    Live(pools->effects[2], 50);
    Live(pools->particles[1], 22, &pools->effects[2]);
    tracker.Update(pools->Spans());
    CHECK(tracker.Count().particles == 1);
    CHECK(tracker.Count().effects == 1);

    g_removedEffects = 0;
    tracker.RemoveAll(pools->Spans(), &RemoveTestEffect);
    CHECK_FALSE(pools->particles[0].Live);
    CHECK_FALSE(pools->effects[1].Live);
    CHECK(pools->effects[2].Live);
    CHECK(pools->particles[1].Live);
}

TEST_CASE("The world preview forgets without removing, and tells when its call created nothing [effects][editor]")
{
    auto pools = std::make_unique<TestPools>();
    EffectPreviewTracker tracker;
    tracker.BeginCreate(pools->Spans());
    CHECK(tracker.EndCreate(pools->Spans()) == 0);
    CHECK_FALSE(tracker.AnyCreatedLive(pools->Spans()));

    tracker.BeginCreate(pools->Spans());
    Live(pools->effects[4], 10);
    CHECK(tracker.EndCreate(pools->Spans()) == 1);
    tracker.Forget();
    CHECK(tracker.IsEmpty());
    CHECK(pools->effects[4].Live);

    // Only a sprite: counted, not kept.
    tracker.BeginCreate(pools->Spans());
    Live(pools->sprites[1], 12);
    CHECK(tracker.EndCreate(pools->Spans()) == 1);
    CHECK(tracker.IsEmpty());
}

TEST_CASE("The world preview refuses the types whose code changes the character or tells the server "
          "[effects][editor]")
{
    CHECK(IsRefusedInWorld({EffectKind::Effect, MODEL_SUMMONER_SUMMON_LAGUL, 1}));
    CHECK_FALSE(IsRefusedInWorld({EffectKind::Effect, MODEL_SUMMONER_SUMMON_LAGUL, 0}));
    // The stones of SubTypes 0 and 1 land as 88 and 99.
    for (const int subType : {0, 1, 2, 88, 99})
    {
        CHECK(IsRefusedInWorld({EffectKind::Effect, MODEL_FLY_BIG_STONE1, subType}));
        CHECK(IsRefusedInWorld({EffectKind::Effect, MODEL_FLY_BIG_STONE2, subType}));
    }
    CHECK(IsRefusedInWorld({EffectKind::Effect, MODEL_CHANGE_UP_EFF, 0}));
    CHECK_FALSE(IsRefusedInWorld({EffectKind::Effect, MODEL_CHANGE_UP_EFF, 1}));
    CHECK_FALSE(IsRefusedInWorld({EffectKind::Particle, MODEL_FLY_BIG_STONE1, 88}));
}

TEST_CASE("The world preview creates the type with the game's call, follows it and removes only it "
          "[effects][editor]")
{
    EffectTestData::BuildShippedRegistry();
    const TestHero hero;
    OBJECT& character = const_cast<TestHero&>(hero).Object();
    const double worldTime = WorldTime;

    // The character's own effect of the same type stays.
    vec3_t position = {500.0f, 500.0f, 0.0f};
    vec3_t angle = {0.0f, 0.0f, 0.0f};
    vec3_t light = {1.0f, 1.0f, 1.0f};
    CreateEffect(MODEL_POISON, position, angle, light, 0, &character);
    const std::optional<int> own = FindEffect(MODEL_POISON, &character);
    REQUIRE(own.has_value());

    auto worldPreview = std::make_unique<EffectWorldPreview>();
    EffectWorldPreview& world = *worldPreview;
    const OBJECT* owner = &world.GetOwner();
    world.SetMute(true);
    world.Start({EffectKind::Effect, MODEL_POISON, 0});
    CHECK(Audio::EditorMute::IsMuted());
    world.AfterFrame(true, true);
    CHECK(world.IsRunning());
    CHECK(world.GetCounts().effects >= 1);
    // The owner is a copy of the character where the character stands.
    const std::optional<int> created = FindEffect(MODEL_POISON, owner);
    REQUIRE(created.has_value());
    CHECK(owner->Position[0] == doctest::Approx(character.Position[0]));
    CHECK(owner->Position[1] == doctest::Approx(character.Position[1]));

    world.Stop();
    CHECK_FALSE(Effects[*created].Live);
    CHECK(Effects[*own].Live);
    CHECK_FALSE(world.IsRunning());
    CHECK_FALSE(Audio::EditorMute::IsMuted());

    SUBCASE("Repeat creates the type again shortly after it ended")
    {
        world.SetRepeat(true);
        world.Start({EffectKind::Effect, MODEL_POISON, 0});
        world.AfterFrame(true, true);
        const std::optional<int> first = FindEffect(MODEL_POISON, owner);
        REQUIRE(first.has_value());
        Effects[*first].Live = false;
        world.AfterFrame(true, true);
        CHECK_FALSE(FindEffect(MODEL_POISON, owner).has_value());
        WorldTime += 1000.0;
        world.AfterFrame(true, true);
        CHECK(FindEffect(MODEL_POISON, owner).has_value());
        world.Stop();
        CHECK_FALSE(FindEffect(MODEL_POISON, owner).has_value());
    }
    SUBCASE("Closing the browser and clearing the world remove what the preview created")
    {
        world.Start({EffectKind::Effect, MODEL_POISON, 0});
        world.AfterFrame(true, true);
        REQUIRE(FindEffect(MODEL_POISON, owner).has_value());
        world.AfterFrame(false, true);
        CHECK_FALSE(FindEffect(MODEL_POISON, owner).has_value());
        CHECK_FALSE(world.IsRunning());

        world.Start({EffectKind::Effect, MODEL_POISON, 0});
        world.AfterFrame(true, true);
        REQUIRE(FindEffect(MODEL_POISON, owner).has_value());
        world.OnWorldClearing();
        CHECK_FALSE(FindEffect(MODEL_POISON, owner).has_value());
    }
    SUBCASE("A call that creates nothing leaves what earlier calls created running, and Stop removes it")
    {
        world.Start({EffectKind::Effect, MODEL_POISON, 0});
        world.AfterFrame(true, true);
        REQUIRE(FindEffect(MODEL_POISON, owner).has_value());
        // A full pool: the next call fills nothing.
        std::array<bool, MAX_EFFECTS> live{};
        for (int i = 0; i < MAX_EFFECTS; ++i)
        {
            live[static_cast<size_t>(i)] = Effects[i].Live;
            Effects[i].Live = true;
        }
        world.Start({EffectKind::Effect, MODEL_POISON, 0});
        world.AfterFrame(true, true);
        for (int i = 0; i < MAX_EFFECTS; ++i)
        {
            if (Effects[i].Owner != owner)
                Effects[i].Live = live[static_cast<size_t>(i)];
        }
        CHECK((world.GetNotes() & WorldNoteNothingCreated) != 0);
        CHECK(world.IsRunning());
        world.KeepOnly(EffectTypeRef{EffectKind::Effect, MODEL_FIRE});
        CHECK_FALSE(world.IsRunning());
        CHECK_FALSE(FindEffect(MODEL_POISON, owner).has_value());
    }
    SUBCASE("Showing another type stops the preview; a refused type creates nothing")
    {
        world.Start({EffectKind::Effect, MODEL_POISON, 0});
        world.AfterFrame(true, true);
        world.KeepOnly(EffectTypeRef{EffectKind::Effect, MODEL_FIRE});
        CHECK_FALSE(world.IsRunning());
        CHECK_FALSE(FindEffect(MODEL_POISON, owner).has_value());

        world.Start({EffectKind::Effect, MODEL_SUMMONER_SUMMON_LAGUL, 1});
        CHECK_FALSE(world.IsRunning());
        CHECK(world.GetNotes() == WorldNoteRefused);

        // A refused SubType chosen while the preview runs ends the run.
        world.Start({EffectKind::Effect, MODEL_SUMMONER_SUMMON_LAGUL, 0});
        CHECK(world.IsRunning());
        world.UpdateRunning({EffectKind::Effect, MODEL_SUMMONER_SUMMON_LAGUL, 1});
        CHECK_FALSE(world.IsRunning());
        CHECK(world.GetNotes() == WorldNoteRefused);
    }

    Effects[*own].Live = false;
    WorldTime = worldTime;
}
TEST_CASE("The world preview starts particles, lightning and sprites at the chest and finds the nearest monster or "
          "NPC [effects][editor]")
{
    CHECK(DefaultWorldPreviewCall(EffectKind::Effect).height == 0.0f);
    CHECK(DefaultWorldPreviewCall(EffectKind::Joint).height == WorldPreviewChestHeight);
    CHECK(DefaultWorldPreviewCall(EffectKind::Particle).height == WorldPreviewChestHeight);
    CHECK(DefaultWorldPreviewCall(EffectKind::Sprite).height == WorldPreviewChestHeight);
    CHECK(DefaultWorldPreviewCall(EffectKind::Effect).distance == WorldPreviewDistance);

    auto characters = std::make_unique<CHARACTER[]>(4);
    const auto place = [&](int index, int kind, float x, int action)
    {
        OBJECT& o = characters[static_cast<size_t>(index)].Object;
        o.Live = true;
        o.Kind = kind;
        o.CurrentAction = action;
        Vector(x, 0.0f, 0.0f, o.Position);
    };
    place(0, KIND_MONSTER, 300.0f, 0);
    place(1, KIND_NPC, 100.0f, MONSTER01_DIE);
    place(2, KIND_PLAYER, 50.0f, 0);
    place(3, KIND_NPC, 700.0f, 0);
    const std::span<CHARACTER> all(characters.get(), 4);
    CHECK(FindNearestCharacter(all, {0.0f, 0.0f, 0.0f}, NearestCharacterRange) == &characters[0]);
    CHECK(FindNearestCharacter(all, {650.0f, 0.0f, 0.0f}, NearestCharacterRange) == &characters[3]);
    CHECK(FindNearestCharacter(all, {0.0f, 0.0f, 0.0f}, 200.0f) == nullptr);
}

TEST_CASE("The world preview creates with the call's size, light, place and target [effects][editor]")
{
    EffectTestData::BuildShippedRegistry();
    const TestHero hero;
    auto worldPreview = std::make_unique<EffectWorldPreview>();
    EffectWorldPreview& world = *worldPreview;
    const OBJECT* owner = &world.GetOwner();
    const auto findJoint = [](int type, const OBJECT* target) -> JOINT*
    {
        for (int i = 0; i < MAX_JOINTS; ++i)
        {
            if (Joints[i].Live && Joints[i].Type == type && Joints[i].Target == target)
                return &Joints[i];
        }
        return nullptr;
    };

    WorldPreviewCall call = DefaultWorldPreviewCall(EffectKind::Joint);
    call.scale = 100.0f;
    call.jointColour = true;
    call.light = {0.2f, 0.4f, 1.0f};
    world.Start({EffectKind::Joint, BITMAP_JOINT_ENERGY, 0, call});
    world.AfterFrame(true, true);
    JOINT* withCharacter = findJoint(BITMAP_JOINT_ENERGY, owner);
    REQUIRE(withCharacter != nullptr);
    CHECK(withCharacter->Scale == doctest::Approx(100.0f));
    CHECK(withCharacter->Light[2] == doctest::Approx(1.0f));
    CHECK(withCharacter->Light[0] == doctest::Approx(0.2f));

    // Another target, same type: what is there stays.
    CHECK(findJoint(BITMAP_JOINT_ENERGY, nullptr) == nullptr);
    call.target = WorldPreviewTarget::None;
    world.Start({EffectKind::Joint, BITMAP_JOINT_ENERGY, 0, call});
    world.AfterFrame(true, true);
    CHECK(findJoint(BITMAP_JOINT_ENERGY, owner) != nullptr);
    CHECK(findJoint(BITMAP_JOINT_ENERGY, nullptr) != nullptr);
    CHECK(world.GetCounts().joints >= 2);
    world.Stop();
    CHECK(findJoint(BITMAP_JOINT_ENERGY, owner) == nullptr);
    CHECK(findJoint(BITMAP_JOINT_ENERGY, nullptr) == nullptr);

    // The place: this far in front of the character (yaw 0 is -y) and this
    // high above the ground.
    WorldPreviewCall placed = DefaultWorldPreviewCall(EffectKind::Effect);
    placed.distance = 100.0f;
    placed.height = 50.0f;
    world.Start({EffectKind::Effect, MODEL_POISON, 0, placed});
    world.AfterFrame(true, true);
    const std::optional<int> poison = FindEffect(MODEL_POISON, owner);
    REQUIRE(poison.has_value());
    CHECK(Effects[*poison].Position[0] == doctest::Approx(1000.0f).epsilon(Tolerance));
    CHECK(Effects[*poison].Position[1] == doctest::Approx(900.0f).epsilon(Tolerance));
    CHECK(Effects[*poison].Position[2] == doctest::Approx(RequestTerrainHeight(1000.0f, 900.0f) + 50.0f));
    world.Stop();
}
TEST_CASE("The running world preview creates with the values as they are now and tells which its code did not keep "
          "[effects][editor]")
{
    EffectTestData::BuildShippedRegistry();
    const TestHero hero;
    auto worldPreview = std::make_unique<EffectWorldPreview>();
    EffectWorldPreview& world = *worldPreview;
    const OBJECT* owner = &world.GetOwner();
    const double worldTime = WorldTime;

    // An effect whose code puts it at its owner and chooses its own size.
    // (Particles are left out: CreateParticle asks the options window, which
    // the tests do not build.)
    WorldPreviewCall distant = DefaultWorldPreviewCall(EffectKind::Effect);
    distant.distance = 300.0f;
    distant.scale = 5.0f;
    world.Start({EffectKind::Effect, MODEL_BIG_STONE1, 5, distant});
    world.AfterFrame(true, true);
    CHECK((world.GetNotes() & WorldNoteOwnPlace) != 0);
    CHECK((world.GetNotes() & WorldNoteOwnSize) != 0);
    world.Stop();

    // Lightning that keeps the call's size; Repeat takes a size set later.
    const auto findJoint = [owner]() -> JOINT*
    {
        for (int i = 0; i < MAX_JOINTS; ++i)
        {
            if (Joints[i].Live && Joints[i].Type == BITMAP_JOINT_ENERGY && Joints[i].Target == owner)
                return &Joints[i];
        }
        return nullptr;
    };
    WorldPreviewCall thin = DefaultWorldPreviewCall(EffectKind::Joint);
    thin.scale = 20.0f;
    world.SetRepeat(true);
    world.Start({EffectKind::Joint, BITMAP_JOINT_ENERGY, 0, thin});
    world.AfterFrame(true, true);
    JOINT* first = findJoint();
    REQUIRE(first != nullptr);
    CHECK(first->Scale == doctest::Approx(20.0f));
    CHECK((world.GetNotes() & WorldNoteOwnSize) == 0);
    first->Live = false;
    WorldPreviewCall wide = thin;
    wide.scale = 80.0f;
    world.UpdateRunning({EffectKind::Joint, BITMAP_JOINT_ENERGY, 0, wide});
    world.AfterFrame(true, true);
    WorldTime += 1000.0;
    world.AfterFrame(true, true);
    JOINT* again = findJoint();
    REQUIRE(again != nullptr);
    CHECK(again->Scale == doctest::Approx(80.0f));
    world.Stop();
    WorldTime = worldTime;
}

TEST_CASE("The world preview aims at a copy of the nearest monster or NPC with bones of its own, and stops following "
          "it once its slot holds another [effects][editor]")
{
    EffectTestData::BuildShippedRegistry();
    const TestHero hero;
    auto characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT);
    auto bones = std::make_unique<vec34_t[]>(MAX_BONES);
    bones[5][0][3] = 42.0f;
    CHARACTER& monster = characters[7];
    monster.Key = 77;
    monster.Object.Live = true;
    monster.Object.Kind = KIND_MONSTER;
    monster.Object.BoneTransform = bones.get();
    Vector(1100.0f, 1000.0f, 0.0f, monster.Object.Position);
    CHARACTER* const previous = CharactersClient;
    CharactersClient = characters.get();

    auto worldPreview = std::make_unique<EffectWorldPreview>();
    EffectWorldPreview& world = *worldPreview;
    const OBJECT* owner = &world.GetOwner();
    const auto findJoint = [](const OBJECT* target) -> JOINT*
    {
        for (int i = 0; i < MAX_JOINTS; ++i)
        {
            if (Joints[i].Live && Joints[i].Type == BITMAP_JOINT_ENERGY && Joints[i].Target == target)
                return &Joints[i];
        }
        return nullptr;
    };
    const auto findAimed = [&]() -> JOINT*
    {
        for (int i = 0; i < MAX_JOINTS; ++i)
        {
            if (Joints[i].Live && Joints[i].Type == BITMAP_JOINT_ENERGY && Joints[i].Target != nullptr &&
                Joints[i].Target != owner)
                return &Joints[i];
        }
        return nullptr;
    };

    WorldPreviewCall aimed = DefaultWorldPreviewCall(EffectKind::Joint);
    aimed.target = WorldPreviewTarget::NearestCharacter;
    world.Start({EffectKind::Joint, BITMAP_JOINT_ENERGY, 0, aimed});
    world.AfterFrame(true, true);
    const JOINT* joint = findAimed();
    REQUIRE(joint != nullptr);
    const OBJECT* copy = joint->Target;
    CHECK(copy != &monster.Object);
    CHECK(copy->Position[0] == doctest::Approx(1100.0f));
    REQUIRE(copy->BoneTransform != nullptr);
    CHECK(copy->BoneTransform != monster.Object.BoneTransform);
    CHECK(copy->BoneTransform[5][0][3] == doctest::Approx(42.0f));
    CHECK((world.GetNotes() & WorldNoteNoCharacterNear) == 0);

    // It follows the monster until its slot holds another character.
    monster.Object.Position[0] = 1200.0f;
    world.AfterFrame(true, true);
    CHECK(copy->Position[0] == doctest::Approx(1200.0f));
    monster.Key = 78;
    monster.Object.Position[0] = 1300.0f;
    world.AfterFrame(true, true);
    CHECK(copy->Position[0] == doctest::Approx(1200.0f));
    world.Stop();

    // With none near, the copy of the character is the target.
    monster.Object.Live = false;
    world.Start({EffectKind::Joint, BITMAP_JOINT_ENERGY, 0, aimed});
    CHECK((world.GetNotes() & WorldNoteNoCharacterNear) != 0);
    world.AfterFrame(true, true);
    CHECK(findJoint(owner) != nullptr);
    world.Stop();
    CharactersClient = previous;
}

TEST_CASE("The world preview gives lightning a colour only when one is chosen, as most of the game's calls do "
          "[effects][editor]")
{
    EffectTestData::BuildShippedRegistry();
    const TestHero hero;
    auto worldPreview = std::make_unique<EffectWorldPreview>();
    EffectWorldPreview& world = *worldPreview;
    const OBJECT* owner = &world.GetOwner();
    const auto findJoint = [owner](int type) -> JOINT*
    {
        for (int i = 0; i < MAX_JOINTS; ++i)
        {
            if (Joints[i].Live && Joints[i].Type == type && Joints[i].Target == owner)
                return &Joints[i];
        }
        return nullptr;
    };

    // Without one the type's code chooses it: SubType 0 of the energy bolt is
    // brown, and nothing is noted.
    world.Start({EffectKind::Joint, BITMAP_JOINT_ENERGY, 0, DefaultWorldPreviewCall(EffectKind::Joint)});
    world.AfterFrame(true, true);
    const JOINT* own = findJoint(BITMAP_JOINT_ENERGY);
    REQUIRE(own != nullptr);
    CHECK(own->Light[0] == doctest::Approx(0.4f));
    CHECK(own->Light[1] == doctest::Approx(0.3f));
    CHECK(own->Light[2] == doctest::Approx(0.2f));
    CHECK((world.GetNotes() & WorldNoteOwnLight) == 0);
    world.Stop();

    // A chosen colour goes to the call.
    WorldPreviewCall coloured = DefaultWorldPreviewCall(EffectKind::Joint);
    coloured.jointColour = true;
    coloured.light = {0.1f, 0.9f, 0.1f};
    world.Start({EffectKind::Joint, BITMAP_JOINT_ENERGY, 0, coloured});
    world.AfterFrame(true, true);
    const JOINT* chosen = findJoint(BITMAP_JOINT_ENERGY);
    REQUIRE(chosen != nullptr);
    CHECK(chosen->Light[1] == doctest::Approx(0.9f));
    world.Stop();

    // The SubTypes whose code copies the colour unchecked get white.
    for (const JointNeedingColour& entry : GetJointsNeedingColour())
    {
        CAPTURE(entry.type);
        CAPTURE(entry.subType);
        world.Start({EffectKind::Joint, entry.type, entry.subType, DefaultWorldPreviewCall(EffectKind::Joint)});
        world.AfterFrame(true, true);
        const JOINT* white = findJoint(entry.type);
        REQUIRE(white != nullptr);
        CHECK(white->Light[0] == doctest::Approx(1.0f));
        CHECK(white->Light[1] == doctest::Approx(1.0f));
        CHECK(white->Light[2] == doctest::Approx(1.0f));
        world.Stop();
    }
}

TEST_CASE("The map code tells the editor before it clears the effect pools, and the editor's listener stops the "
          "world preview [effects][editor]")
{
    int slot = 0;
    while (slot < MAX_EFFECTS && Effects[slot].Live)
        ++slot;
    REQUIRE(slot < MAX_EFFECTS);
    Effects[slot].Live = true;
    Effects[slot].Type = MODEL_POISON;
    g_watchedEffect = slot;
    g_clearingsTold = 0;
    g_watchedLiveWhenTold = false;
    Core::WorldClearing::SetListener(&WatchClearing);
    gMapManager.DeleteObjects();
    CHECK(g_clearingsTold == 1);
    CHECK(g_watchedLiveWhenTold);
    CHECK_FALSE(Effects[slot].Live);

    // The editor's listener (set when it starts) stops the effect browser's
    // world preview, which lets the game start sounds again.
    g_MuEditorCore.ConnectGameHooks();
    Audio::EditorMute::SetMuted(true);
    gMapManager.DeleteObjects();
    CHECK_FALSE(Audio::EditorMute::IsMuted());

    Core::WorldClearing::SetListener(nullptr);
    Core::AssetLoadWorld::SetSource(nullptr);
    Audio::EditorMute::SetMuted(false);
}
#endif // _EDITOR
