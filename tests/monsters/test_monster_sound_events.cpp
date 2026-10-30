#include "stdafx.h"

// The sound events of the model entries (monsters design, MN7): the sounds
// the game plays for a character on top of its sound slots, by animation,
// frame, death effect and map, within 500 units of the hero; and the traps'
// attack sounds.

#include "MonsterCreationScope.h"
#include "MonsterModelRecorder.h"

#include "Audio/DSPlaySound.h"
#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Core/Utilities/Random.h"
#include "GameLogic/Monsters/MonsterSoundEvents.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "World/MapInfra/MapManager.h"
#include "World/MapInfra/w_MapProcess.h"

#ifdef _EDITOR
#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "UI/MonsterTools/MuMonsterPreviewUI.h"
#endif

#include <doctest.h>

#include <array>
#include <cstdlib>
#include <vector>

using MonsterRecorder::PlayedSound;

namespace
{
class SoundScope
{
public:
    explicit SoundScope(int world) : m_creation(world), m_previousWorld(gMapManager.WorldActive)
    {
        gMapManager.WorldActive = world;
        m_creation.Load().TakePlayedSounds();
    }
    ~SoundScope()
    {
        gMapManager.WorldActive = m_previousWorld;
    }
    SoundScope(const SoundScope&) = delete;
    SoundScope& operator=(const SoundScope&) = delete;

    // Plays the sound events of a character of the model in the animation.
    std::vector<PlayedSound> Play(int slot, int animation, float frame = 0.0f, int lifeTime = 0,
                                  float distance = 100.0f)
    {
        OBJECT object;
        object.Type = static_cast<short>(slot);
        object.CurrentAction = static_cast<unsigned short>(animation);
        object.AnimationFrame = frame;
        object.LifeTime = lifeTime;
        object.Position[0] = Hero->Object.Position[0] + distance;
        object.Position[1] = Hero->Object.Position[1];
        CHECK(GameLogic::Monsters::SoundEvents::Play(object));
        return m_creation.Load().TakePlayedSounds();
    }

    MonsterRecorder::CreationScope& Creation()
    {
        return m_creation;
    }

private:
    MonsterRecorder::CreationScope m_creation;
    int m_previousWorld = 0;
};

std::vector<PlayedSound> Sounds(std::initializer_list<int> ids)
{
    std::vector<PlayedSound> sounds;
    for (const int id : ids)
    {
        sounds.push_back({id});
    }
    return sounds;
}
} // namespace

TEST_CASE("Monster sounds: an event plays in its animations on its maps, near the hero [monsters][sounds]")
{
    SoundScope scope(WD_69EMPIREGUARDIAN1);

    CHECK(scope.Play(MODEL_RAYMOND, MONSTER01_ATTACK1) == Sounds({SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01}));
    CHECK(scope.Play(MODEL_RAYMOND, MONSTER01_APEAR) == Sounds({SOUND_EMPIREGUARDIAN_RAYMOND_MONSTER_RAGE}));
    CHECK(scope.Play(MODEL_RAYMOND, MONSTER01_STOP1).empty());
    // Past 500 units from the hero, nothing.
    CHECK(scope.Play(MODEL_RAYMOND, MONSTER01_ATTACK1, 0.0f, 0, 500.5f).empty());
    // Raymond's events are those of the four Empire Guardian maps.
    gMapManager.WorldActive = WD_0LORENCIA;
    CHECK(scope.Play(MODEL_RAYMOND, MONSTER01_ATTACK1).empty());
}

TEST_CASE("Monster sounds: the events' maps are the server's, Devil Square 5-7 is 32 (D67) [monsters][sounds]")
{
    SoundScope scope(WD_69EMPIREGUARDIAN1);
    std::vector<Data::Monsters::MonsterModelDefinition> models;
    for (const Data::Monsters::MonsterModelDefinition* model : g_MonsterModelDatabase.GetAll())
    {
        models.push_back(*model);
    }
    std::vector<Data::Monsters::MonsterModelDefinition> changed = models;
    for (Data::Monsters::MonsterModelDefinition& model : changed)
    {
        if (Data::Monsters::FindMonsterModelReference(model.slot) == MODEL_RAYMOND)
        {
            model.soundEventMaps = {{32}};
        }
    }
    g_MonsterModelDatabase.Build(changed);

    gMapManager.SetServerMap(32);
    CHECK(gMapManager.WorldActive == WD_9DEVILSQUARE);
    CHECK(scope.Play(MODEL_RAYMOND, MONSTER01_ATTACK1) == Sounds({SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01}));
    gMapManager.SetServerMap(WD_9DEVILSQUARE);
    CHECK(scope.Play(MODEL_RAYMOND, MONSTER01_ATTACK1).empty());

    // A stage of Devil Square, which the server sends with the map.
    for (Data::Monsters::MonsterModelDefinition& model : changed)
    {
        if (Data::Monsters::FindMonsterModelReference(model.slot) == MODEL_RAYMOND)
        {
            model.soundEventMaps = {{WD_9DEVILSQUARE, 2}};
        }
    }
    g_MonsterModelDatabase.Build(changed);
    gMapManager.SetServerStage(2);
    gMapManager.SetServerMap(WD_9DEVILSQUARE);
    CHECK(scope.Play(MODEL_RAYMOND, MONSTER01_ATTACK1) == Sounds({SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01}));
    gMapManager.SetServerStage(1);
    CHECK(scope.Play(MODEL_RAYMOND, MONSTER01_ATTACK1).empty());
    gMapManager.SetServerStage(0);
    g_MonsterModelDatabase.Build(models);
}

TEST_CASE("Monster sounds: an event with frames plays only within them [monsters][sounds]")
{
    SoundScope scope(WD_69EMPIREGUARDIAN1);

    CHECK(scope.Play(MODEL_RAYMOND, MONSTER01_WALK, 7.5f) == Sounds({SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01}));
    CHECK(scope.Play(MODEL_RAYMOND, MONSTER01_WALK, 1.0f) == Sounds({SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE02}));
    CHECK(scope.Play(MODEL_RAYMOND, MONSTER01_WALK, 2.0f).empty());
    CHECK(scope.Play(MODEL_RAYMOND, MONSTER01_WALK, 3.0f).empty());
}

TEST_CASE("Monster sounds: an event plays its sounds together, and one may need the death effect armed "
          "[monsters][sounds]")
{
    SoundScope scope(WD_70EMPIREGUARDIAN2);
    // The Dark Ghost's first attack plays two sounds.
    CHECK(scope.Play(MODEL_DARK_GHOST, MONSTER01_ATTACK1) == Sounds({SOUND_METEORITE01, SOUND_EXPLOTION01}));

    // The Snowman's death sound plays only while its death effect is armed.
    CHECK(scope.Play(MODEL_XMAS2008_SNOWMAN, MONSTER01_DIE, 0.0f, 100) == Sounds({SOUND_XMAS_SNOWMAN_DEATH_1}));
    CHECK(scope.Play(MODEL_XMAS2008_SNOWMAN, MONSTER01_DIE, 0.0f, 0).empty());
}

// Monster Preview's events: the game's, without the map and the hero.
TEST_CASE("Monster sounds: PlayEvents plays a model's events on any map and at any distance (Q-MN74) "
          "[monsters][sounds]")
{
    SoundScope scope(WD_0LORENCIA);
    std::vector<int> played;
    const auto play = [&played](int slot, int animation, float frame, bool deathEffectArmed)
    {
        played.clear();
        const Data::Monsters::MonsterModelDefinition* model = g_MonsterModelDatabase.Find(slot);
        REQUIRE(model != nullptr);
        GameLogic::Monsters::SoundEvents::PlayEvents(*model, animation, frame, deathEffectArmed,
                                                     [&played](int sound) { played.push_back(sound); });
        return played;
    };

    // Raymond's events are those of the Empire Guardian maps; this is Lorencia.
    CHECK(play(MODEL_RAYMOND, MONSTER01_ATTACK1, 0.0f, false) ==
          std::vector<int>{SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01});
    CHECK(play(MODEL_RAYMOND, MONSTER01_WALK, 7.5f, false) ==
          std::vector<int>{SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01});
    CHECK(play(MODEL_RAYMOND, MONSTER01_WALK, 3.0f, false).empty());
    CHECK(play(MODEL_XMAS2008_SNOWMAN, MONSTER01_DIE, 0.0f, true) == std::vector<int>{SOUND_XMAS_SNOWMAN_DEATH_1});
    CHECK(play(MODEL_XMAS2008_SNOWMAN, MONSTER01_DIE, 0.0f, false).empty());
    // The sounds went to `play`, not to the game.
    CHECK(scope.Creation().Load().TakePlayedSounds().empty());
}

#ifdef _EDITOR
// Monster Attacks' Play in Monster Preview: the attack sounds of the model,
// sound events or slots, on a map the events do not name.
TEST_CASE("Monster Preview: Play plays the model's sound events or its slot sound of the attack (Q-MN74) "
          "[monsters][sounds][editor]")
{
    SoundScope scope(WD_0LORENCIA);
    MonsterRecorder::LoadScope& load = scope.Creation().Load();
    const std::array<int, 4> slots{MODEL_RAYMOND, MODEL_TOTEM_GOLEM, MODEL_XMAS2008_SNOWMAN, MODEL_BULL_FIGHTER};
    for (const int slot : slots)
    {
        Data::Monsters::ModelLoader::Open(slot);
        REQUIRE(Data::Monsters::ModelLoader::IsOpen(slot));
    }
    load.TakePlayedSounds();

    g_MuMonsterPreviewUI.PlayAttack(MODEL_RAYMOND, MONSTER01_ATTACK1, std::nullopt);
    CHECK(load.TakePlayedSounds() == Sounds({SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01}));
    g_MuMonsterPreviewUI.PlayAttack(MODEL_RAYMOND, MONSTER01_APEAR, std::nullopt);
    CHECK(load.TakePlayedSounds() == Sounds({SOUND_EMPIREGUARDIAN_RAYMOND_MONSTER_RAGE}));
    // A walk's events play frame by frame, not when it starts: the Totem
    // Golem's at every frame.
    g_MuMonsterPreviewUI.PlayAttack(MODEL_TOTEM_GOLEM, MONSTER01_WALK, std::nullopt);
    CHECK(load.TakePlayedSounds().empty());
    // The Snowman's death sound needs its death effect, which counts as armed.
    g_MuMonsterPreviewUI.PlayAttack(MODEL_XMAS2008_SNOWMAN, MONSTER01_DIE, std::nullopt);
    CHECK(load.TakePlayedSounds() == Sounds({SOUND_XMAS_SNOWMAN_DEATH_1}));

    // The Bull Fighter has sound slots: one of its two attack sounds.
    g_MuMonsterPreviewUI.PlayAttack(MODEL_BULL_FIGHTER, MONSTER01_ATTACK1, std::nullopt);
    const std::vector<PlayedSound> bull = load.TakePlayedSounds();
    REQUIRE(bull.size() == 1);
    CHECK((bull[0].id == SOUND_MONSTER + Models[MODEL_BULL_FIGHTER].Sounds[2] ||
           bull[0].id == SOUND_MONSTER + Models[MODEL_BULL_FIGHTER].Sounds[3]));
}

// An attack Play started stands the model when it ends, as SetPlayerStop
// after an attack: also one that holds its last frame, which the game ends
// there (BMD::PlayAnimation).
TEST_CASE("Monster Preview: an attack Play started ends with the model standing, also one that holds its last "
          "frame [monsters][editor]")
{
    SoundScope scope(WD_0LORENCIA);
    MonsterRecorder::LoadScope& load = scope.Creation().Load();
    Data::Monsters::ModelLoader::Open(MODEL_RAYMOND);
    REQUIRE(Data::Monsters::ModelLoader::IsOpen(MODEL_RAYMOND));
    const Data::Monsters::MonsterModelDefinition* model = g_MonsterModelDatabase.Find(MODEL_RAYMOND);
    REQUIRE(model != nullptr);
    // More frames than any attack has at its speed, each a reference frame.
    constexpr int MaxFrames = 10000;
    const float frameFactor = FPS_ANIMATION_FACTOR;
    FPS_ANIMATION_FACTOR = 1.0f;
    for (const bool held : {false, true})
    {
        INFO("holds its last frame: " << held);
        Data::Monsters::MonsterModelDefinition values = *model;
        values.load.holdLastFrame = held ? std::vector<int>{MONSTER01_ATTACK1} : std::vector<int>{};
        g_MuMonsterPreviewUI.PlayAttack(MODEL_RAYMOND, MONSTER01_ATTACK1, std::nullopt);
        REQUIRE(g_MuMonsterPreviewUI.GetAnimation() == MONSTER01_ATTACK1);
        for (int frame = 0; frame < MaxFrames && g_MuMonsterPreviewUI.GetAnimation() == MONSTER01_ATTACK1; ++frame)
        {
            g_MuMonsterPreviewUI.Advance(MODEL_RAYMOND, values);
        }
        CHECK(g_MuMonsterPreviewUI.GetAnimation() == MONSTER01_STOP1);
    }
    FPS_ANIMATION_FACTOR = frameFactor;
    load.TakePlayedSounds();
}
#endif // _EDITOR

TEST_CASE("Monster sounds: a model without sound events plays none of them [monsters][sounds]")
{
    SoundScope scope(WD_0LORENCIA);
    OBJECT object;
    object.Type = MODEL_BULL_FIGHTER;
    object.CurrentAction = MONSTER01_ATTACK1;
    CHECK_FALSE(GameLogic::Monsters::SoundEvents::Play(object));
    CHECK(scope.Creation().Load().TakePlayedSounds().empty());
}

TEST_CASE("Monster sounds: the traps' attacks play their sounds, the Iron Stick's at the trap [monsters][sounds]")
{
    SoundScope scope(WD_1DUNGEON);
    const std::vector<Data::Monsters::MonsterDefinition> monsters =
        Data::Monsters::LoadMonstersFile(MonsterRecorder::ClientFolder() / "Data" / "Monsters" / "Monsters.json")
            .monsters;
    g_MonsterDatabase.Build(monsters, "en");
    g_MapProcess = MapProcess::Make();

    CHARACTER* ironStick = scope.Creation().Create(101);
    REQUIRE(ironStick != nullptr);
    ironStick->Object.AnimationFrame = 3.0f;
    SetPlayerAttack(ironStick);
    CHECK(scope.Creation().Load().TakePlayedSounds() == std::vector<PlayedSound>{{SOUND_TRAP01, &ironStick->Object}});

    CHARACTER* fire = scope.Creation().Create(102);
    REQUIRE(fire != nullptr);
    fire->Object.AnimationFrame = 3.0f;
    SetPlayerAttack(fire);
    CHECK(scope.Creation().Load().TakePlayedSounds() == Sounds({SOUND_FLAME}));
}

// Q-MN29: the shipped events draw with rand(), as the Hellas monsters'
// attacks: a chance of one in n and a pick of one of two come out the same at
// any frame rate; with rand_fps_check they came out less often the higher the
// frame rate.
TEST_CASE("Monster sounds: the shipped chances and picks are the same at any frame rate (Q-MN29) [monsters][sounds]")
{
    SoundScope scope(WD_57ICECITY);
    const float factor = FPS_ANIMATION_FACTOR;
    constexpr int Seeds = 600;
    constexpr std::array<float, 2> Factors = {1.0f, 0.25f};
    int steps = 0;
    for (unsigned seed = 0; seed < Seeds; ++seed)
    {
        // The Ice Giant's step plays one time in a hundred.
        std::srand(seed);
        const std::vector<PlayedSound> expected =
            std::rand() % 100 == 0 ? Sounds({SOUND_RAKLION_ICEGIANT_MOVE}) : std::vector<PlayedSound>{};
        steps += expected.empty() ? 0 : 1;
        for (const float fps : Factors)
        {
            std::srand(seed);
            Random::Seed(seed);
            FPS_ANIMATION_FACTOR = fps;
            INFO("seed " << seed << ", factor " << fps);
            CHECK(scope.Play(MODEL_ICE_GIANT, MONSTER01_WALK) == expected);
        }
    }
    CHECK(steps > 0);

    // The Totem Golem's step picks one of two.
    gMapManager.WorldActive = WD_51HOME_6TH_CHAR;
    int first = 0;
    for (unsigned seed = 0; seed < Seeds; ++seed)
    {
        std::srand(seed);
        const bool firstSound = std::rand() % 2 == 0;
        first += firstSound ? 1 : 0;
        for (const float fps : Factors)
        {
            std::srand(seed);
            Random::Seed(seed);
            FPS_ANIMATION_FACTOR = fps;
            INFO("seed " << seed << ", factor " << fps);
            CHECK(scope.Play(MODEL_TOTEM_GOLEM, MONSTER01_WALK) ==
                  Sounds({firstSound ? SOUND_ELBELAND_TOTEMGOLEM_MOVE01 : SOUND_ELBELAND_TOTEMGOLEM_MOVE02}));
        }
    }
    // About half.
    CHECK(first > Seeds / 3);
    CHECK(first < Seeds * 2 / 3);
    FPS_ANIMATION_FACTOR = factor;
}
