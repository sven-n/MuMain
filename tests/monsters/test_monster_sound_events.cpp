#include "stdafx.h"

// The sound events of the sound sets (monsters design, MN7, MNS, Q-MN78): the
// sounds the game plays for a monster by animation, cue of its model, frame,
// death effect and map, within 500 units of the hero, and at moments, which
// the sound slots became; and the traps' attack sounds.

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
#include "Data/GameData/MonsterData/MonsterSoundSetDatabase.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzOpenData.h"
#include "Core/Utilities/Random.h"
#include "GameLogic/Monsters/MonsterSoundEvents.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "World/MapInfra/MapManager.h"
#include "World/MapInfra/w_MapProcess.h"

#ifdef _EDITOR
#include "UI/MonsterTools/MonsterSelection.h"
#include "UI/MonsterTools/MuMonsterPreviewUI.h"
#endif

#include <doctest.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

using Data::Monsters::MonsterSoundSet;
using MonsterRecorder::PlayedSound;

namespace
{
// The monsters of the tests (Monsters.json) and their models.
constexpr int Raymond = 506;
constexpr int GrandWizard = 515;
constexpr int Snowman = 477;
constexpr int IceGiant = 456;
constexpr int TotemGolem = 423;
constexpr int LifeStone = 278;
constexpr int BullFighter = 0;
constexpr int Assassin = 21;
constexpr int Balrog = 38;
constexpr int QueenRainier = 70;
constexpr int RedDragon = 42;
constexpr int DeathCenturion = 145;
constexpr int RogueCenturion = 148;
constexpr int ChaosCastle2 = 163;
constexpr int Skeleton = 14;

// The slot of the monster's model.
int SlotOf(int monster)
{
    const Data::Monsters::MonsterDefinition* entry = g_MonsterDatabase.Find(monster);
    REQUIRE(entry != nullptr);
    const std::optional<int> slot = Data::Monsters::FindMonsterModelReference(entry->model);
    REQUIRE(slot);
    return *slot;
}

// A character of the monster with its model, or another.
std::unique_ptr<CHARACTER> MakeCharacter(int monster, std::optional<int> slot = std::nullopt)
{
    auto character = std::make_unique<CHARACTER>();
    character->MonsterIndex = static_cast<EMonsterType>(monster);
    character->Object.Type = static_cast<short>(slot.value_or(SlotOf(monster)));
    character->Object.CurrentAction = MONSTER01_STOP1;
    return character;
}

const MonsterSoundSet& SetOf(int monster)
{
    const Data::Monsters::MonsterDefinition* entry = g_MonsterDatabase.Find(monster);
    REQUIRE(entry != nullptr);
    const MonsterSoundSet* set = g_MonsterSoundSetDatabase.Find(entry->soundSet);
    REQUIRE(set != nullptr);
    return *set;
}

const Data::Monsters::MonsterModelDefinition& ModelOf(int slot)
{
    const Data::Monsters::MonsterModelDefinition* model = g_MonsterModelDatabase.Find(slot);
    REQUIRE(model != nullptr);
    return *model;
}

// The number of a sound the set's sound events name.
int SoundOf(const MonsterSoundSet& set, const char* name)
{
    const std::optional<int> number = Data::Monsters::ModelLoader::FindSoundNumber(set, name);
    REQUIRE(number);
    return *number;
}

// Changes the set of the name for the life of the scope.
class SetChange
{
public:
    SetChange(const std::string& name, const std::function<void(MonsterSoundSet&)>& change)
    {
        for (const MonsterSoundSet* set : g_MonsterSoundSetDatabase.GetAll())
        {
            m_sets.push_back(*set);
        }
        std::vector<MonsterSoundSet> changed = m_sets;
        for (MonsterSoundSet& set : changed)
        {
            if (set.name == name)
            {
                change(set);
            }
        }
        g_MonsterSoundSetDatabase.Build(changed);
    }
    ~SetChange()
    {
        g_MonsterSoundSetDatabase.Build(m_sets);
    }
    SetChange(const SetChange&) = delete;
    SetChange& operator=(const SetChange&) = delete;

private:
    std::vector<MonsterSoundSet> m_sets;
};

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

    // Plays the sound events of a character of the monster in the animation.
    std::vector<PlayedSound> Play(int monster, int animation, float frame = 0.0f, int lifeTime = 0,
                                  float distance = 100.0f)
    {
        const std::unique_ptr<CHARACTER> character = MakeCharacter(monster);
        OBJECT& object = character->Object;
        object.CurrentAction = static_cast<unsigned short>(animation);
        object.AnimationFrame = frame;
        object.LifeTime = lifeTime;
        object.Position[0] = Hero->Object.Position[0] + distance;
        object.Position[1] = Hero->Object.Position[1];
        CHECK(GameLogic::Monsters::SoundEvents::Play(*character));
        return Take();
    }

    std::vector<PlayedSound> Take()
    {
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

    CHECK(scope.Play(Raymond, MONSTER01_ATTACK1) == Sounds({SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01}));
    CHECK(scope.Play(Raymond, MONSTER01_APEAR) == Sounds({SOUND_EMPIREGUARDIAN_RAYMOND_MONSTER_RAGE}));
    CHECK(scope.Play(Raymond, MONSTER01_STOP1).empty());
    // Past 500 units from the hero, nothing.
    CHECK(scope.Play(Raymond, MONSTER01_ATTACK1, 0.0f, 0, 500.5f).empty());
    // Raymond's events are those of the four Empire Guardian maps.
    gMapManager.WorldActive = WD_0LORENCIA;
    CHECK(scope.Play(Raymond, MONSTER01_ATTACK1).empty());
}

TEST_CASE("Monster sounds: the set's maps are the server's, Devil Square 5-7 is 32 (D67) [monsters][sounds]")
{
    SoundScope scope(WD_69EMPIREGUARDIAN1);
    {
        SetChange change("Raymond", [](MonsterSoundSet& set) { set.maps = {{32}}; });
        gMapManager.SetServerMap(32);
        CHECK(gMapManager.WorldActive == WD_9DEVILSQUARE);
        CHECK(scope.Play(Raymond, MONSTER01_ATTACK1) == Sounds({SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01}));
        gMapManager.SetServerMap(WD_9DEVILSQUARE);
        CHECK(scope.Play(Raymond, MONSTER01_ATTACK1).empty());
    }
    // A stage of Devil Square, which the server sends with the map.
    SetChange change("Raymond", [](MonsterSoundSet& set) { set.maps = {{WD_9DEVILSQUARE, 2}}; });
    gMapManager.SetServerStage(2);
    gMapManager.SetServerMap(WD_9DEVILSQUARE);
    CHECK(scope.Play(Raymond, MONSTER01_ATTACK1) == Sounds({SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01}));
    gMapManager.SetServerStage(1);
    CHECK(scope.Play(Raymond, MONSTER01_ATTACK1).empty());
    gMapManager.SetServerStage(0);
}

TEST_CASE("Monster sounds: an event on cues plays only in their frames of the model's animation (Q-MN79) "
          "[monsters][sounds]")
{
    SoundScope scope(WD_69EMPIREGUARDIAN1);

    CHECK(scope.Play(Raymond, MONSTER01_WALK, 7.5f) == Sounds({SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01}));
    CHECK(scope.Play(Raymond, MONSTER01_WALK, 1.0f) == Sounds({SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE02}));
    CHECK(scope.Play(Raymond, MONSTER01_WALK, 2.0f).empty());
    CHECK(scope.Play(Raymond, MONSTER01_WALK, 3.0f).empty());
    // The cues are walk's: another animation at the frame plays nothing.
    CHECK(scope.Play(Raymond, MONSTER01_STOP1, 7.5f).empty());
}

TEST_CASE("Monster sounds: an event plays its sounds together, and one may need the death effect armed "
          "[monsters][sounds]")
{
    SoundScope scope(WD_70EMPIREGUARDIAN2);
    // The Grand Wizard's first attack plays two sounds.
    CHECK(scope.Play(GrandWizard, MONSTER01_ATTACK1) == Sounds({SOUND_METEORITE01, SOUND_EXPLOTION01}));

    // The Snowman's death sound plays only while its death effect is armed.
    CHECK(scope.Play(Snowman, MONSTER01_DIE, 0.0f, 100) == Sounds({SOUND_XMAS_SNOWMAN_DEATH_1}));
    CHECK(scope.Play(Snowman, MONSTER01_DIE, 0.0f, 0).empty());
}

// Monster Preview's events: the game's, without the map and the hero.
TEST_CASE("Monster sounds: PlayEvents plays a set's events on any map and at any distance (Q-MN74) "
          "[monsters][sounds]")
{
    SoundScope scope(WD_0LORENCIA);
    std::vector<int> played;
    const auto play = [&played](int monster, int animation, float frame, bool deathEffectArmed)
    {
        played.clear();
        GameLogic::Monsters::SoundEvents::Situation situation;
        situation.animation = animation;
        situation.frame = frame;
        situation.deathEffectArmed = deathEffectArmed;
        GameLogic::Monsters::SoundEvents::PlayEvents(SetOf(monster), ModelOf(SlotOf(monster)).cues, situation,
                                                     [&played](int sound, bool) { played.push_back(sound); });
        return played;
    };

    // Raymond's events are those of the Empire Guardian maps; this is Lorencia.
    CHECK(play(Raymond, MONSTER01_ATTACK1, 0.0f, false) ==
          std::vector<int>{SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01});
    CHECK(play(Raymond, MONSTER01_WALK, 7.5f, false) == std::vector<int>{SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01});
    CHECK(play(Raymond, MONSTER01_WALK, 3.0f, false).empty());
    CHECK(play(Snowman, MONSTER01_DIE, 0.0f, true) == std::vector<int>{SOUND_XMAS_SNOWMAN_DEATH_1});
    CHECK(play(Snowman, MONSTER01_DIE, 0.0f, false).empty());
    // The sounds went to `play`, not to the game.
    CHECK(scope.Take().empty());
}

#ifdef _EDITOR
// Monster Attacks' Play in Monster Preview: the attack sounds of the
// selected monster's set, on a map the events do not name.
TEST_CASE("Monster Preview: Play plays the attack's sound events of the selected monster's set (Q-MN74, Q-MN78) "
          "[monsters][sounds][editor]")
{
    SoundScope scope(WD_0LORENCIA);
    for (const int monster : {Raymond, TotemGolem, Snowman, BullFighter})
    {
        const int slot = SlotOf(monster);
        Data::Monsters::ModelLoader::Open(slot);
        REQUIRE(Data::Monsters::ModelLoader::IsOpen(slot));
    }
    scope.Take();

    g_MonsterSelection.Select(Raymond);
    g_MuMonsterPreviewUI.PlayAttack(MODEL_RAYMOND, MONSTER01_ATTACK1, std::nullopt);
    CHECK(scope.Take() == Sounds({SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01}));
    g_MuMonsterPreviewUI.PlayAttack(MODEL_RAYMOND, MONSTER01_APEAR, std::nullopt);
    CHECK(scope.Take() == Sounds({SOUND_EMPIREGUARDIAN_RAYMOND_MONSTER_RAGE}));
    // A walk's events play frame by frame, not when it starts: the Totem
    // Golem's at every frame.
    g_MonsterSelection.Select(TotemGolem);
    g_MuMonsterPreviewUI.PlayAttack(MODEL_TOTEM_GOLEM, MONSTER01_WALK, std::nullopt);
    CHECK(scope.Take().empty());
    // The Snowman's death sound needs its death effect, which counts as armed.
    g_MonsterSelection.Select(Snowman);
    g_MuMonsterPreviewUI.PlayAttack(MODEL_XMAS2008_SNOWMAN, MONSTER01_DIE, std::nullopt);
    CHECK(scope.Take() == Sounds({SOUND_XMAS_SNOWMAN_DEATH_1}));

    // The Bull Fighter's attack sounds are an event at the attack moment: one
    // of its two.
    g_MonsterSelection.Select(BullFighter);
    g_MuMonsterPreviewUI.PlayAttack(MODEL_BULL_FIGHTER, MONSTER01_ATTACK1, std::nullopt);
    const std::vector<PlayedSound> bull = scope.Take();
    REQUIRE(bull.size() == 1);
    const MonsterSoundSet& bullSet = SetOf(BullFighter);
    const int attack1 = SoundOf(bullSet, "SOUND_MONSTER_BULLATTACK1");
    const int attack2 = SoundOf(bullSet, "Data/Sound/mBullAttack2.wav");
    CHECK((bull[0].id == attack1 || bull[0].id == attack2));
    // The shock plays the hit moment's events, the death the death moment's.
    g_MuMonsterPreviewUI.PlayAttack(MODEL_BULL_FIGHTER, MONSTER01_SHOCK, std::nullopt);
    const std::vector<PlayedSound> hit = scope.Take();
    REQUIRE(hit.size() == 1);
    CHECK((hit[0].id == attack1 || hit[0].id == attack2));
    g_MuMonsterPreviewUI.PlayAttack(MODEL_BULL_FIGHTER, MONSTER01_DIE, std::nullopt);
    CHECK(scope.Take() == Sounds({SoundOf(bullSet, "Data/Sound/mBullDie.wav")}));
    g_MonsterSelection.Select(MuEditor::Monsters::MonsterSelection::NoMonster);
}

// An attack Play started stands the model when it ends, as SetPlayerStop
// after an attack: also one that holds its last frame, which the game ends
// there (BMD::PlayAnimation).
TEST_CASE("Monster Preview: an attack Play started ends with the model standing, also one that holds its last "
          "frame [monsters][editor]")
{
    SoundScope scope(WD_0LORENCIA);
    Data::Monsters::ModelLoader::Open(MODEL_RAYMOND);
    REQUIRE(Data::Monsters::ModelLoader::IsOpen(MODEL_RAYMOND));
    const Data::Monsters::MonsterModelDefinition& model = ModelOf(MODEL_RAYMOND);
    // More frames than any attack has at its speed, each a reference frame.
    constexpr int MaxFrames = 10000;
    const float frameFactor = FPS_ANIMATION_FACTOR;
    FPS_ANIMATION_FACTOR = 1.0f;
    for (const bool held : {false, true})
    {
        INFO("holds its last frame: " << held);
        Data::Monsters::MonsterModelDefinition values = model;
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
    scope.Take();
}
#endif // _EDITOR

TEST_CASE("Monster sounds: a monster without a sound set plays no sound event [monsters][sounds]")
{
    SoundScope scope(WD_0LORENCIA);
    // The Life Stone's sound slots were all empty.
    const std::unique_ptr<CHARACTER> stone = MakeCharacter(LifeStone);
    stone->Object.CurrentAction = MONSTER01_ATTACK1;
    CHECK(GameLogic::Monsters::SoundEvents::FindSoundSet(*stone) == nullptr);
    CHECK_FALSE(GameLogic::Monsters::SoundEvents::Play(*stone));
    CHECK_FALSE(GameLogic::Monsters::SoundEvents::PlayMoment(*stone, Data::Monsters::MonsterSoundMoment::Attack));
    CHECK(scope.Take().empty());
}

TEST_CASE("Monster sounds: only the monster's own models play its set; a player has no monster number (Q-MN78) "
          "[monsters][sounds]")
{
    SoundScope scope(WD_0LORENCIA);
    // A character starts as a Bull Fighter number (CHARACTER()); a player's
    // has the player model, CreateCharacterPointer gives it no number.
    const std::unique_ptr<CHARACTER> player = MakeCharacter(BullFighter, MODEL_PLAYER);
    CHECK(GameLogic::Monsters::SoundEvents::FindSoundSet(*player) == nullptr);
    player->MonsterIndex = MONSTER_UNDEFINED;
    CHECK(GameLogic::Monsters::SoundEvents::FindSoundSet(*player) == nullptr);
    // A monster shown with a model that is not its own (a map creator's Bull
    // Fighter): none either.
    const std::unique_ptr<CHARACTER> other = MakeCharacter(Raymond, MODEL_BULL_FIGHTER);
    CHECK(GameLogic::Monsters::SoundEvents::FindSoundSet(*other) == nullptr);
    for (const Data::Monsters::MonsterSoundMoment moment : Data::Monsters::AllMonsterSoundMoments)
    {
        CHECK_FALSE(GameLogic::Monsters::SoundEvents::PlayMoment(*player, moment));
        CHECK_FALSE(GameLogic::Monsters::SoundEvents::PlayMoment(*other, moment));
    }
    CHECK(scope.Take().empty());
}

TEST_CASE("Monster sounds: the Chaos Castle soldiers' wizard look plays its random variant's set (Q-MN83) "
          "[monsters][sounds]")
{
    SoundScope scope(WD_18CHAOS_CASTLE);
    const std::unique_ptr<CHARACTER> elf = MakeCharacter(ChaosCastle2, MODEL_CHAOS_CASTLE_ELF);
    const std::unique_ptr<CHARACTER> wizard = MakeCharacter(ChaosCastle2, MODEL_CHAOS_CASTLE_WIZARD);
    const MonsterSoundSet* elfSet = GameLogic::Monsters::SoundEvents::FindSoundSet(*elf);
    const MonsterSoundSet* wizardSet = GameLogic::Monsters::SoundEvents::FindSoundSet(*wizard);
    REQUIRE(elfSet != nullptr);
    REQUIRE(wizardSet != nullptr);
    CHECK(elfSet->name == "Chaos Castle");
    CHECK(wizardSet->name == "Chaos Castle Wizard");

    CHECK(GameLogic::Monsters::SoundEvents::PlayMoment(*elf, Data::Monsters::MonsterSoundMoment::Attack));
    CHECK(scope.Take() == std::vector<PlayedSound>{{SOUND_MONSTER_ORCCAPATTACK1, &elf->Object}});
    CHECK(GameLogic::Monsters::SoundEvents::PlayMoment(*wizard, Data::Monsters::MonsterSoundMoment::Attack));
    CHECK(scope.Take() ==
          std::vector<PlayedSound>{{SoundOf(*wizardSet, "Data/Sound/mOrcArcherAttack1.wav"), &wizard->Object}});
}

namespace
{
using Data::Monsters::MonsterSoundMoment;
using GameLogic::Monsters::SoundEvents::Situation;
using Played = std::vector<std::pair<int, bool>>;

Played PlayIn(const MonsterSoundSet& set, const Situation& situation,
              std::span<const Data::Monsters::MonsterModelCue> cues = {})
{
    Played played;
    GameLogic::Monsters::SoundEvents::PlayEvents(set, cues, situation, [&played](int sound, bool atMonster)
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

Situation In(int animation, float frame = 0.0f)
{
    Situation situation;
    situation.animation = animation;
    situation.frame = frame;
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
    MonsterSoundSet set;
    Data::Monsters::MonsterSoundEvent walk;
    walk.animations = {MONSTER01_WALK};
    walk.sounds = {"SOUND_MONSTER_DSIDLE1"};
    set.soundEvents = {Event({MonsterSoundMoment::Attack, MonsterSoundMoment::Hit}, {"SOUND_MONSTER_BULLATTACK1"}),
                       walk};

    CHECK(PlayIn(set, At(MonsterSoundMoment::Attack, MONSTER01_STOP2)) == Played{{SOUND_MONSTER_BULLATTACK1, false}});
    // At a moment, the events of the animation do not play.
    CHECK(PlayIn(set, At(MonsterSoundMoment::Hit, MONSTER01_WALK)) == Played{{SOUND_MONSTER_BULLATTACK1, false}});
    CHECK(PlayIn(set, At(MonsterSoundMoment::Death)).empty());
    // In an animation, the events of a moment do not play.
    CHECK(PlayIn(set, In(MONSTER01_ATTACK1)).empty());
    CHECK(PlayIn(set, In(MONSTER01_WALK)) == Played{{SOUND_MONSTER_DSIDLE1, false}});
}

TEST_CASE("Monster sounds: one set plays on the cues of each model that has them (Q-MN79) [monsters][sounds]")
{
    MonsterSoundSet set;
    Data::Monsters::MonsterSoundEvent step;
    step.cues = {"step 1"};
    step.sounds = {"SOUND_MONSTER_DSIDLE1"};
    set.soundEvents = {step};
    // Two models whose feet land at other frames, and one without the cue.
    const std::vector<Data::Monsters::MonsterModelCue> early = {{"step 1", MONSTER01_WALK, {1.0f, 2.0f}}};
    const std::vector<Data::Monsters::MonsterModelCue> late = {{"step 1", MONSTER01_WALK, {4.5f, 5.5f}}};
    const std::vector<Data::Monsters::MonsterModelCue> none = {{"step 2", MONSTER01_WALK, {1.0f, 2.0f}}};

    CHECK(PlayIn(set, In(MONSTER01_WALK, 1.5f), early) == Played{{SOUND_MONSTER_DSIDLE1, false}});
    CHECK(PlayIn(set, In(MONSTER01_WALK, 1.5f), late).empty());
    CHECK(PlayIn(set, In(MONSTER01_WALK, 5.0f), late) == Played{{SOUND_MONSTER_DSIDLE1, false}});
    // From the first frame to below the second.
    CHECK(PlayIn(set, In(MONSTER01_WALK, 5.5f), late).empty());
    CHECK(PlayIn(set, In(MONSTER01_WALK, 1.5f), none).empty());
    // A cue plays in its animation, not at a moment.
    CHECK(PlayIn(set, At(MonsterSoundMoment::Walk, MONSTER01_WALK), early).empty());
}

TEST_CASE("Monster sounds: an event heard anywhere and at the monster (Q-MN76) [monsters][sounds]")
{
    MonsterSoundSet set;
    Data::Monsters::MonsterSoundEvent first = Event({MonsterSoundMoment::Death}, {"SOUND_MONSTER_DSDEATH"});
    first.heardAnywhere = true;
    first.atMonster = true;
    Data::Monsters::MonsterSoundEvent second = Event({MonsterSoundMoment::Death}, {"SOUND_MONSTER_LSDEATH"});
    set.soundEvents = {first, second};

    Situation death = At(MonsterSoundMoment::Death);
    CHECK(PlayIn(set, death) == Played{{SOUND_MONSTER_DSDEATH, true}, {SOUND_MONSTER_LSDEATH, false}});
    // Far from the hero or on another map: only the event heard anywhere.
    death.heard = false;
    CHECK(PlayIn(set, death) == Played{{SOUND_MONSTER_DSDEATH, true}});
}

TEST_CASE("Monster sounds: a pick with one sound plays it in one draw of two, else none (Q-MN76) [monsters][sounds]")
{
    MonsterSoundSet set;
    Data::Monsters::MonsterSoundEvent idle = Event({MonsterSoundMoment::Walk}, {"SOUND_MONSTER_DSIDLE1"});
    idle.pick = Data::Monsters::MonsterSoundDraw::Rand;
    set.soundEvents = {idle};

    int played = 0;
    for (unsigned int seed = 1; seed <= 20; ++seed)
    {
        std::srand(seed);
        const bool first = std::rand() % 2 == 0;
        const int next = std::rand();
        std::srand(seed);
        const Played sounds = PlayIn(set, At(MonsterSoundMoment::Walk));
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
    SetChange change("Raymond",
                     [](MonsterSoundSet& set)
                     {
                         Data::Monsters::MonsterSoundEvent hit =
                             Event({MonsterSoundMoment::Hit}, {"SOUND_MONSTER_BULLATTACK1"});
                         hit.heardAnywhere = true;
                         hit.atMonster = true;
                         set.soundEvents.push_back(hit);
                     });

    const std::unique_ptr<CHARACTER> raymond = MakeCharacter(Raymond);
    OBJECT& object = raymond->Object;
    object.CurrentAction = MONSTER01_SHOCK;
    // Raymond's other events name the Empire Guardian maps; this is Lorencia,
    // and the character is far from the hero.
    object.Position[0] = Hero->Object.Position[0] + 1000.0f;
    object.Position[1] = Hero->Object.Position[1];
    CHECK(GameLogic::Monsters::SoundEvents::PlayMoment(*raymond, MonsterSoundMoment::Hit));
    CHECK(scope.Take() == std::vector<PlayedSound>{{SOUND_MONSTER_BULLATTACK1, &object, false}});
    CHECK(GameLogic::Monsters::SoundEvents::PlayMoment(*raymond, MonsterSoundMoment::Attack));
    CHECK(scope.Take().empty());
}

namespace
{
// How many of `tries` draws, each from its own seed, play a sound.
int PlayedOf(const MonsterSoundSet& set, const Situation& situation, int tries)
{
    const float frameFactor = FPS_ANIMATION_FACTOR;
    FPS_ANIMATION_FACTOR = 1.0f;
    int played = 0;
    for (int seed = 1; seed <= tries; ++seed)
    {
        std::srand(static_cast<unsigned int>(seed));
        Random::Seed(static_cast<std::uint32_t>(seed));
        played += PlayIn(set, situation).empty() ? 0 : 1;
    }
    FPS_ANIMATION_FACTOR = frameFactor;
    return played;
}
} // namespace

TEST_CASE("Monster sounds: the sound slots became events at moments, the second set the monsters' own "
          "(MNS, Q-MN78) [monsters][sounds]")
{
    SoundScope scope(WD_0LORENCIA);

    // Rogue Centurion and Death Centurion share a model and play their own
    // sets: the second set was SubType 9's, which the Hellas creator gave the
    // Death Centurion.
    Situation death = At(MonsterSoundMoment::Death);
    CHECK(PlayIn(SetOf(RogueCenturion), death) == Played{{SOUND_MONSTER_DSDEATH, true}});
    CHECK(PlayIn(SetOf(DeathCenturion), death) == Played{{SOUND_MONSTER_LSDEATH, true}});
    // Heard anywhere, as the slots were.
    death.heard = false;
    CHECK(PlayIn(SetOf(DeathCenturion), death) == Played{{SOUND_MONSTER_LSDEATH, true}});
    // Their characters find them by the monster number.
    CHECK(GameLogic::Monsters::SoundEvents::FindSoundSet(*MakeCharacter(DeathCenturion)) == &SetOf(DeathCenturion));
    CHECK(GameLogic::Monsters::SoundEvents::FindSoundSet(*MakeCharacter(RogueCenturion)) == &SetOf(RogueCenturion));

    // The Bull Fighter: one of its attack sounds when it attacks or is hit.
    const MonsterSoundSet& bull = SetOf(BullFighter);
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
    CHECK(PlayIn(SetOf(Assassin), At(MonsterSoundMoment::Hit)).empty());
    CHECK(PlayIn(SetOf(Assassin), At(MonsterSoundMoment::Attack)).size() == 1);
    CHECK(PlayedOf(SetOf(Balrog), At(MonsterSoundMoment::Walk), 64) == 0);
    CHECK(PlayedOf(SetOf(Balrog), At(MonsterSoundMoment::Stand), 64) > 0);

    // A pair whose second slot was empty: the first sound one time in two.
    const MonsterSoundSet& rainer = SetOf(QueenRainier);
    const int attacks = PlayedOf(rainer, At(MonsterSoundMoment::Attack), 64);
    CHECK(attacks == 64);
    const int idles = PlayedOf(rainer, At(MonsterSoundMoment::Stand), 640);
    CHECK(idles > 0);
    CHECK(idles < PlayedOf(bull, At(MonsterSoundMoment::Stand), 640));
}

TEST_CASE("Monster sounds: the Dragon's events at moments play the sounds its model loads (P9) [monsters][sounds]")
{
    MonsterRecorder::LoadScope scope(0);
    const MonsterRecorder::LoadChanges changes = scope.Run([] { OpenMonsterModel(MONSTER_MODEL_DRAGON); });
    const MonsterSoundSet& dragon = SetOf(RedDragon);
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
    const std::unique_ptr<CHARACTER> skeleton = MakeCharacter(Skeleton, MODEL_PLAYER);
    skeleton->Object.SubType = MODEL_SKELETON1;
    for (const MonsterSoundMoment moment : Data::Monsters::AllMonsterSoundMoments)
    {
        CHECK_FALSE(GameLogic::Monsters::SoundEvents::PlayMoment(*skeleton, moment));
    }
    CHECK(scope.Take().empty());
}

TEST_CASE("Monster sounds: the traps' attacks play their sounds, the Iron Stick's at the trap [monsters][sounds]")
{
    SoundScope scope(WD_1DUNGEON);
    g_MapProcess = MapProcess::Make();

    CHARACTER* ironStick = scope.Creation().Create(101);
    REQUIRE(ironStick != nullptr);
    ironStick->Object.AnimationFrame = 3.0f;
    SetPlayerAttack(ironStick);
    CHECK(scope.Take() == std::vector<PlayedSound>{{SOUND_TRAP01, &ironStick->Object}});

    CHARACTER* fire = scope.Creation().Create(102);
    REQUIRE(fire != nullptr);
    fire->Object.AnimationFrame = 3.0f;
    SetPlayerAttack(fire);
    CHECK(scope.Take() == Sounds({SOUND_FLAME}));
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
            CHECK(scope.Play(IceGiant, MONSTER01_WALK) == expected);
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
            CHECK(scope.Play(TotemGolem, MONSTER01_WALK) ==
                  Sounds({firstSound ? SOUND_ELBELAND_TOTEMGOLEM_MOVE01 : SOUND_ELBELAND_TOTEMGOLEM_MOVE02}));
        }
    }
    // About half.
    CHECK(first > Seeds / 3);
    CHECK(first < Seeds * 2 / 3);
    FPS_ANIMATION_FACTOR = factor;
}
