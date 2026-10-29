#include "stdafx.h"

// The sound events of the model entries (monsters design, MN7, MNS): the
// sounds the game plays for a character by animation, frame, death effect and
// map, within 500 units of the hero, and at moments, which the sound slots
// became; and the traps' attack sounds.

#include "MonsterCreationScope.h"
#include "MonsterModelRecorder.h"

#include "Audio/DSPlaySound.h"
#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/MonsterSoundMoments.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzOpenData.h"
#include "Core/Utilities/Random.h"
#include "GameLogic/Monsters/MonsterSoundEvents.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "World/MapInfra/MapManager.h"
#include "World/MapInfra/w_MapProcess.h"

#ifdef _EDITOR
#include "UI/MonsterTools/MuMonsterPreviewUI.h"
#endif

#include <doctest.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <string>
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
        GameLogic::Monsters::SoundEvents::Situation situation;
        situation.animation = animation;
        situation.frame = frame;
        situation.deathEffectArmed = deathEffectArmed;
        GameLogic::Monsters::SoundEvents::PlayEvents(*model, situation,
                                                     [&played](int sound, bool) { played.push_back(sound); });
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

    // The Bull Fighter's attack sounds are an event at the attack moment: one
    // of its two.
    g_MuMonsterPreviewUI.PlayAttack(MODEL_BULL_FIGHTER, MONSTER01_ATTACK1, std::nullopt);
    const std::vector<PlayedSound> bull = load.TakePlayedSounds();
    REQUIRE(bull.size() == 1);
    const Data::Monsters::MonsterModelDefinition* bullModel = g_MonsterModelDatabase.Find(MODEL_BULL_FIGHTER);
    REQUIRE(bullModel != nullptr);
    const auto number = [bullModel](const char* name)
    { return Data::Monsters::ModelLoader::FindSoundNumber(*bullModel, name).value_or(-1); };
    CHECK((bull[0].id == number("SOUND_MONSTER_BULLATTACK1") || bull[0].id == number("Data/Sound/mBullAttack2.wav")));
    // The shock plays the hit moment's events, the death the death moment's.
    g_MuMonsterPreviewUI.PlayAttack(MODEL_BULL_FIGHTER, MONSTER01_SHOCK, std::nullopt);
    const std::vector<PlayedSound> hit = load.TakePlayedSounds();
    REQUIRE(hit.size() == 1);
    CHECK((hit[0].id == number("SOUND_MONSTER_BULLATTACK1") || hit[0].id == number("Data/Sound/mBullAttack2.wav")));
    g_MuMonsterPreviewUI.PlayAttack(MODEL_BULL_FIGHTER, MONSTER01_DIE, std::nullopt);
    CHECK(load.TakePlayedSounds() == Sounds({number("Data/Sound/mBullDie.wav")}));
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
    // The Life Stone's sound slots were all empty.
    object.Type = MODEL_LIFE_STONE;
    object.CurrentAction = MONSTER01_ATTACK1;
    CHECK_FALSE(GameLogic::Monsters::SoundEvents::Play(object));
    CHECK_FALSE(GameLogic::Monsters::SoundEvents::PlayMoment(object, Data::Monsters::MonsterSoundMoment::Attack));
    CHECK(scope.Creation().Load().TakePlayedSounds().empty());
}

namespace
{
using Data::Monsters::MonsterSoundMoment;
using GameLogic::Monsters::SoundEvents::Situation;
using Played = std::vector<std::pair<int, bool>>;

Played PlayIn(const Data::Monsters::MonsterModelDefinition& model, const Situation& situation)
{
    Played played;
    GameLogic::Monsters::SoundEvents::PlayEvents(model, situation, [&played](int sound, bool atMonster)
                                                 { played.emplace_back(sound, atMonster); });
    return played;
}

Situation At(MonsterSoundMoment moment, int animation = MONSTER01_STOP1)
{
    Situation situation;
    situation.moment = moment;
    situation.animation = animation;
    return situation;
}

Situation In(int animation)
{
    Situation situation;
    situation.animation = animation;
    return situation;
}

Data::Monsters::MonsterSoundEvent Event(std::vector<MonsterSoundMoment> moments, std::vector<std::string> sounds)
{
    Data::Monsters::MonsterSoundEvent event;
    event.moments = std::move(moments);
    event.sounds = std::move(sounds);
    return event;
}
} // namespace

TEST_CASE("Monster sounds: an event at moments plays there, whatever the animation (Q-MN76) [monsters][sounds]")
{
    Data::Monsters::MonsterModelDefinition model;
    Data::Monsters::MonsterSoundEvent walk;
    walk.animations = {MONSTER01_WALK};
    walk.sounds = {"SOUND_MONSTER_DSIDLE1"};
    model.soundEvents = {Event({MonsterSoundMoment::Attack, MonsterSoundMoment::Hit}, {"SOUND_MONSTER_BULLATTACK1"}),
                         walk};

    CHECK(PlayIn(model, At(MonsterSoundMoment::Attack, MONSTER01_STOP2)) == Played{{SOUND_MONSTER_BULLATTACK1, false}});
    // At a moment, the events of the animation do not play.
    CHECK(PlayIn(model, At(MonsterSoundMoment::Hit, MONSTER01_WALK)) == Played{{SOUND_MONSTER_BULLATTACK1, false}});
    CHECK(PlayIn(model, At(MonsterSoundMoment::Death)).empty());
    // In an animation, the events of a moment do not play.
    CHECK(PlayIn(model, In(MONSTER01_ATTACK1)).empty());
    CHECK(PlayIn(model, In(MONSTER01_WALK)) == Played{{SOUND_MONSTER_DSIDLE1, false}});
}

TEST_CASE("Monster sounds: an event plays by SubType, heard anywhere and at the monster (Q-MN76) [monsters][sounds]")
{
    Data::Monsters::MonsterModelDefinition model;
    Data::Monsters::MonsterSoundEvent first = Event({MonsterSoundMoment::Death}, {"SOUND_MONSTER_DSDEATH"});
    first.notSubType = 9;
    first.heardAnywhere = true;
    first.atMonster = true;
    Data::Monsters::MonsterSoundEvent second = Event({MonsterSoundMoment::Death}, {"SOUND_MONSTER_LSDEATH"});
    second.subType = 9;
    model.soundEvents = {first, second};

    Situation death = At(MonsterSoundMoment::Death);
    CHECK(PlayIn(model, death) == Played{{SOUND_MONSTER_DSDEATH, true}});
    death.subType = 9;
    CHECK(PlayIn(model, death) == Played{{SOUND_MONSTER_LSDEATH, false}});
    // Far from the hero or on another map: only the event heard anywhere.
    death.heard = false;
    CHECK(PlayIn(model, death).empty());
    death.subType = 0;
    CHECK(PlayIn(model, death) == Played{{SOUND_MONSTER_DSDEATH, true}});
}

TEST_CASE("Monster sounds: a pick with one sound plays it in one draw of two, else none (Q-MN76) [monsters][sounds]")
{
    Data::Monsters::MonsterModelDefinition model;
    Data::Monsters::MonsterSoundEvent idle = Event({MonsterSoundMoment::Walk}, {"SOUND_MONSTER_DSIDLE1"});
    idle.pick = Data::Monsters::MonsterSoundDraw::Rand;
    model.soundEvents = {idle};

    int played = 0;
    for (unsigned int seed = 1; seed <= 20; ++seed)
    {
        std::srand(seed);
        const bool first = std::rand() % 2 == 0;
        const int next = std::rand();
        std::srand(seed);
        const Played sounds = PlayIn(model, At(MonsterSoundMoment::Walk));
        CHECK(sounds == (first ? Played{{SOUND_MONSTER_DSIDLE1, false}} : Played{}));
        // One draw either way.
        CHECK(std::rand() == next);
        played += first ? 1 : 0;
    }
    CHECK(played > 0);
    CHECK(played < 20);
}

TEST_CASE("Monster sounds: PlayMoment plays a character's events at the character (Q-MN76) [monsters][sounds]")
{
    SoundScope scope(WD_0LORENCIA);
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
            Data::Monsters::MonsterSoundEvent hit = Event({MonsterSoundMoment::Hit}, {"SOUND_MONSTER_BULLATTACK1"});
            hit.heardAnywhere = true;
            hit.atMonster = true;
            model.soundEvents.push_back(hit);
        }
    }
    g_MonsterModelDatabase.Build(changed);

    OBJECT object;
    object.Type = MODEL_RAYMOND;
    object.CurrentAction = MONSTER01_SHOCK;
    // Raymond's other events name the Empire Guardian maps; this is Lorencia,
    // and the character is far from the hero.
    object.Position[0] = Hero->Object.Position[0] + 1000.0f;
    object.Position[1] = Hero->Object.Position[1];
    CHECK(GameLogic::Monsters::SoundEvents::PlayMoment(object, MonsterSoundMoment::Hit));
    CHECK(scope.Creation().Load().TakePlayedSounds() ==
          std::vector<PlayedSound>{{SOUND_MONSTER_BULLATTACK1, &object, false}});
    CHECK(GameLogic::Monsters::SoundEvents::PlayMoment(object, MonsterSoundMoment::Attack));
    CHECK(scope.Creation().Load().TakePlayedSounds().empty());
    g_MonsterModelDatabase.Build(models);
}

namespace
{
// The number of a sound the model's sound events name.
int SoundOf(const Data::Monsters::MonsterModelDefinition& model, const char* name)
{
    const std::optional<int> number = Data::Monsters::ModelLoader::FindSoundNumber(model, name);
    REQUIRE(number);
    return *number;
}

const Data::Monsters::MonsterModelDefinition& ModelOf(int slot)
{
    const Data::Monsters::MonsterModelDefinition* model = g_MonsterModelDatabase.Find(slot);
    REQUIRE(model != nullptr);
    return *model;
}

// How many of `tries` draws, each from its own seed, play a sound.
int PlayedOf(const Data::Monsters::MonsterModelDefinition& model, const Situation& situation, int tries)
{
    const float frameFactor = FPS_ANIMATION_FACTOR;
    FPS_ANIMATION_FACTOR = 1.0f;
    int played = 0;
    for (int seed = 1; seed <= tries; ++seed)
    {
        std::srand(static_cast<unsigned int>(seed));
        Random::Seed(static_cast<std::uint32_t>(seed));
        played += PlayIn(model, situation).empty() ? 0 : 1;
    }
    FPS_ANIMATION_FACTOR = frameFactor;
    return played;
}
} // namespace

TEST_CASE("Monster sounds: the sound slots became events at moments, the second set for SubType 9 (MNS) "
          "[monsters][sounds]")
{
    SoundScope scope(WD_0LORENCIA);

    // Rogue Centurion and Death Centurion share a model; the Hellas creator
    // gives Death Centurion SubType 9, which plays the second set.
    const Data::Monsters::MonsterModelDefinition& centurion = ModelOf(MODEL_DEATH_CENTURION);
    Situation death = At(MonsterSoundMoment::Death);
    CHECK(PlayIn(centurion, death) == Played{{SOUND_MONSTER_DSDEATH, true}});
    death.subType = 9;
    CHECK(PlayIn(centurion, death) == Played{{SOUND_MONSTER_LSDEATH, true}});
    // Heard anywhere, as the slots were.
    death.heard = false;
    CHECK(PlayIn(centurion, death) == Played{{SOUND_MONSTER_LSDEATH, true}});

    // The Bull Fighter: one of its attack sounds when it attacks or is hit.
    const Data::Monsters::MonsterModelDefinition& bull = ModelOf(MODEL_BULL_FIGHTER);
    const int attack1 = SoundOf(bull, "SOUND_MONSTER_BULLATTACK1");
    const int attack2 = SoundOf(bull, "Data/Sound/mBullAttack2.wav");
    for (const MonsterSoundMoment moment : {MonsterSoundMoment::Attack, MonsterSoundMoment::Hit})
    {
        const Played played = PlayIn(bull, At(moment, MONSTER01_STOP2));
        REQUIRE(played.size() == 1);
        CHECK((played[0].first == attack1 || played[0].first == attack2));
    }
    CHECK(PlayIn(bull, At(MonsterSoundMoment::Death)) == Played{{SoundOf(bull, "Data/Sound/mBullDie.wav"), true}});
    // Its idle sounds when it is set to stand and walks, one time in 16.
    const int stood = PlayedOf(bull, At(MonsterSoundMoment::Stand), 64);
    CHECK(stood > 0);
    CHECK(stood < 32);
    CHECK(PlayedOf(bull, At(MonsterSoundMoment::Walk), 64) > 0);

    // The Assassin's slots played nothing when it was hit, the Balrog's none
    // while it walks (its bone sound comes first).
    CHECK(PlayIn(ModelOf(MODEL_ASSASSIN), At(MonsterSoundMoment::Hit)).empty());
    CHECK(PlayIn(ModelOf(MODEL_ASSASSIN), At(MonsterSoundMoment::Attack)).size() == 1);
    CHECK(PlayedOf(ModelOf(MODEL_BALROG), At(MonsterSoundMoment::Walk), 64) == 0);
    CHECK(PlayedOf(ModelOf(MODEL_BALROG), At(MonsterSoundMoment::Stand), 64) > 0);

    // A pair whose second slot was empty: the first sound one time in two.
    const Data::Monsters::MonsterModelDefinition& rainer = ModelOf(MODEL_QUEEN_RAINER);
    const int attacks = PlayedOf(rainer, At(MonsterSoundMoment::Attack), 64);
    CHECK(attacks == 64);
    const int idles = PlayedOf(rainer, At(MonsterSoundMoment::Stand), 640);
    CHECK(idles > 0);
    CHECK(idles < PlayedOf(bull, At(MonsterSoundMoment::Stand), 640));
}

TEST_CASE("Monster sounds: the Dragon's events at moments play the sounds it loads (P9) [monsters][sounds]")
{
    MonsterRecorder::LoadScope scope(0);
    const MonsterRecorder::LoadChanges changes = scope.Run([] { OpenMonsterModel(MONSTER_MODEL_DRAGON); });
    const Data::Monsters::MonsterModelDefinition& dragon = ModelOf(MODEL_DRAGON_);
    size_t named = 0;
    // The idle, attack and die sounds; the original slots named numbers that
    // nothing loads.
    for (const Data::Monsters::MonsterSoundEvent& event : dragon.soundEvents)
    {
        for (const std::string& sound : event.sounds)
        {
            INFO("sound " << sound);
            const int number = SoundOf(dragon, sound.c_str());
            CHECK(std::any_of(changes.sounds.begin(), changes.sounds.end(), [&](const MonsterRecorder::SoundEvent& load)
                              { return !load.release && load.id == number; }));
            ++named;
        }
    }
    CHECK(named == 5);
}

TEST_CASE("Monster sounds: a skeleton, of the player model, plays no sound event at a moment (P9) [monsters][sounds]")
{
    SoundScope scope(WD_0LORENCIA);
    OBJECT object;
    object.Type = MODEL_PLAYER;
    object.SubType = MODEL_SKELETON1;
    for (const MonsterSoundMoment moment : Data::Monsters::AllMonsterSoundMoments)
    {
        CHECK_FALSE(GameLogic::Monsters::SoundEvents::PlayMoment(object, moment));
    }
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
