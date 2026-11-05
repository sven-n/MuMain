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
#include "GameLogic/Monsters/MonsterModelTimers.h"
#include "GameLogic/Monsters/MonsterSoundEvents.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "World/MapInfra/MapManager.h"
#include "World/MapInfra/w_MapProcess.h"

#ifdef _EDITOR
#include "Audio/SoundMixer.h"
#include "Audio/SoundOverlay.h"
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

TEST_CASE("Monster sounds: only the monster's own models play its set, a player has no monster number (Q-MN78) "
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

TEST_CASE("Monster sounds: an event during cues plays each drawn frame in their frames, and leaves the once flag "
          "alone (Q-MN90) [monsters][sounds]")
{
    MonsterSoundSet set;
    Data::Monsters::MonsterSoundEvent attack2;
    attack2.duringCues = {"attack2 sound"};
    attack2.sounds = {"SOUND_MONSTER_DSIDLE1"};
    Data::Monsters::MonsterSoundEvent once;
    once.once = {MONSTER01_ATTACK1};
    once.sounds = {"SOUND_MONSTER_BULLATTACK1"};
    set.soundEvents = {attack2, once};
    CHECK(GameLogic::Monsters::SoundEvents::HasDrawnEvents(set));
    // An event of cues alone, or of timers alone (Q-MN91), makes a set one of
    // drawn frames.
    MonsterSoundSet onlyCues;
    onlyCues.soundEvents = {attack2};
    CHECK(GameLogic::Monsters::SoundEvents::HasDrawnEvents(onlyCues));
    MonsterSoundSet onlyTimers;
    Data::Monsters::MonsterSoundEvent swing;
    swing.timers = {"hair swing"};
    swing.sounds = {"SOUND_MONSTER_DSIDLE1"};
    onlyTimers.soundEvents = {swing};
    CHECK(GameLogic::Monsters::SoundEvents::HasDrawnEvents(onlyTimers));
    // Dreadfear's: attack 2 from frame 0 through frame 2.
    const std::vector<Data::Monsters::MonsterModelCue> cues = {{"attack2 sound", MONSTER01_ATTACK2, {0.0f, 2.001f}}};
    const auto drawn = [](int animation, float frame)
    {
        Situation situation = In(animation, frame);
        situation.drawn = true;
        return situation;
    };
    for (const float frame : {0.0f, 1.3f, 2.0f})
    {
        CAPTURE(frame);
        CHECK(PlayIn(set, drawn(MONSTER01_ATTACK2, frame), cues) == Played{{SOUND_MONSTER_DSIDLE1, false}});
    }
    CHECK(PlayIn(set, drawn(MONSTER01_ATTACK2, 2.1f), cues).empty());
    CHECK(PlayIn(set, drawn(MONSTER01_ATTACK1, 1.0f), cues) == Played{{SOUND_MONSTER_BULLATTACK1, false}});
    // Once played, it still plays (the flag is the once events').
    Situation flagged = drawn(MONSTER01_ATTACK2, 1.0f);
    flagged.oncePlayed = true;
    CHECK(PlayIn(set, flagged, cues) == Played{{SOUND_MONSTER_DSIDLE1, false}});
    // It does not set the flag: PlayEvents says no once event played.
    CHECK_FALSE(
        GameLogic::Monsters::SoundEvents::PlayEvents(set, cues, drawn(MONSTER01_ATTACK2, 1.0f), [](int, bool) {}));
    // Not at an animation's fixed points: only in drawn frames.
    CHECK(PlayIn(set, In(MONSTER01_ATTACK2, 1.0f), cues).empty());
    // A model without the cue plays nothing.
    CHECK(PlayIn(set, drawn(MONSTER01_ATTACK2, 1.0f)).empty());
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
        Random::SeedRand(seed);
        const bool first = Random::Rand() % 2 == 0;
        const int next = Random::Rand();
        Random::SeedRand(seed);
        const Played sounds = PlayIn(set, At(MonsterSoundMoment::Walk));
        CHECK(sounds == (first ? Played{{SOUND_MONSTER_DSIDLE1, false}} : Played{}));
        // One draw either way.
        CHECK(Random::Rand() == next);
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
        Random::SeedRand(static_cast<unsigned int>(seed));
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

// Q-MN29: the shipped events draw with Random::Rand(), as the Hellas monsters'
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
        Random::SeedRand(seed);
        const std::vector<PlayedSound> expected =
            Random::Rand() % 100 == 0 ? Sounds({SOUND_RAKLION_ICEGIANT_MOVE}) : std::vector<PlayedSound>{};
        steps += expected.empty() ? 0 : 1;
        for (const float fps : Factors)
        {
            Random::SeedRand(seed);
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
        Random::SeedRand(seed);
        const bool firstSound = Random::Rand() % 2 == 0;
        first += firstSound ? 1 : 0;
        for (const float fps : Factors)
        {
            Random::SeedRand(seed);
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

namespace
{
constexpr int WitchQueen = 304;
constexpr int DeathRider = 306;

// Draws the character `frames` frames in the animation, as
// UpdateDrawnCharacterEffects does (FinishDrawnFrame after the maps' monster
// effects), and returns the sounds.
std::vector<PlayedSound> Draw(SoundScope& scope, CHARACTER& character, int animation, int frames = 1)
{
    character.Object.CurrentAction = static_cast<unsigned short>(animation);
    for (int frame = 0; frame < frames; ++frame)
    {
        GameLogic::Monsters::SoundEvents::FinishDrawnFrame(character);
    }
    return scope.Take();
}

// The Death Rider's walk event without its chance and pick: its first sound
// each frame.
void WalkEveryFrame(MonsterSoundSet& set)
{
    set.soundEvents[0].chance.reset();
    set.soundEvents[0].pick.reset();
    set.soundEvents[0].sounds = {"SOUND_AIDA_DEATHRAIDER_MOVE1"};
}

// The Death Rider's events heard within 500 units of the hero, as a set
// without heardAnywhere (Aida's are heard anywhere, Q-MN94).
void HeardNear(MonsterSoundSet& set)
{
    for (Data::Monsters::MonsterSoundEvent& event : set.soundEvents)
    {
        event.heardAnywhere = false;
    }
}

// The Death Rider's attacks and death as once events, as the Aida draw code
// played them before the test of Q-MN86.
void AttacksOnce(MonsterSoundSet& set)
{
    for (size_t index = 1; index < set.soundEvents.size(); ++index)
    {
        Data::Monsters::MonsterSoundEvent& event = set.soundEvents[index];
        event.once = event.animations;
        event.animations.clear();
        event.heardAnywhere = true;
    }
}

// The Witch Queen's, the same.
void QueenAttacksOnce(MonsterSoundSet& set)
{
    AttacksOnce(set);
}
} // namespace

TEST_CASE("Monster sounds: an event during animations plays each frame the monster is drawn in them (Q-MN84) "
          "[monsters][sounds]")
{
    SoundScope scope(WD_33AIDA);
    SetChange change("Death Rider", WalkEveryFrame);
    const int move1 = SoundOf(SetOf(DeathRider), "SOUND_AIDA_DEATHRAIDER_MOVE1");
    const std::unique_ptr<CHARACTER> rider = MakeCharacter(DeathRider);
    CHECK(Draw(scope, *rider, MONSTER01_WALK, 3) == Sounds({move1, move1, move1}));
    CHECK(Draw(scope, *rider, MONSTER01_RUN, 2) == Sounds({move1, move1}));
    CHECK(Draw(scope, *rider, MONSTER01_STOP1, 2).empty());
    // Not at the fixed points of animation events.
    rider->Object.CurrentAction = MONSTER01_WALK;
    CHECK(GameLogic::Monsters::SoundEvents::Play(*rider));
    CHECK(scope.Take().empty());
}

TEST_CASE("Monster sounds: the Aida monsters' walk sounds come by a frame-rate chance each drawn frame, one of two "
          "(Q-MN84) [monsters][sounds]")
{
    SoundScope scope(WD_33AIDA);
    const MonsterSoundSet& set = SetOf(DeathRider);
    const int move1 = SoundOf(set, "SOUND_AIDA_DEATHRAIDER_MOVE1");
    const int move2 = SoundOf(set, "SOUND_AIDA_DEATHRAIDER_MOVE2");
    const float factor = FPS_ANIMATION_FACTOR;
    FPS_ANIMATION_FACTOR = 1.0f;
    Random::SeedRand(7);
    Random::Seed(7);
    const std::unique_ptr<CHARACTER> rider = MakeCharacter(DeathRider);
    const std::vector<PlayedSound> sounds = Draw(scope, *rider, MONSTER01_WALK, 300);
    FPS_ANIMATION_FACTOR = factor;
    // One in 15 frames: about 20 in 300.
    CHECK(sounds.size() > 5);
    CHECK(sounds.size() < 60);
    CHECK(std::all_of(sounds.begin(), sounds.end(),
                      [&](const PlayedSound& sound) { return sound.id == move1 || sound.id == move2; }));
    CHECK(std::any_of(sounds.begin(), sounds.end(), [&](const PlayedSound& sound) { return sound.id == move1; }));
    CHECK(std::any_of(sounds.begin(), sounds.end(), [&](const PlayedSound& sound) { return sound.id == move2; }));
}

TEST_CASE("Monster sounds: a once event plays the first drawn frame in its animation, then none of the set's once "
          "events until the monster stands or walks (Q-MN85, Q-MN87, Q-MN89) [monsters][sounds]")
{
    SoundScope scope(WD_33AIDA);
    const MonsterSoundSet& set = SetOf(DeathRider);
    const int attack1 = SoundOf(set, "SOUND_AIDA_DEATHRAIDER_ATTACK1");
    const int attack2 = SoundOf(set, "SOUND_AIDA_DEATHRAIDER_ATTACK2");
    const int die = SoundOf(set, "SOUND_AIDA_DEATHRAIDER_DIE");
    SetChange once("Death Rider", AttacksOnce);
    const std::unique_ptr<CHARACTER> rider = MakeCharacter(DeathRider);
    // A during event leaves the flag alone.
    {
        SetChange change("Death Rider", WalkEveryFrame);
        CHECK(Draw(scope, *rider, MONSTER01_WALK, 2).size() == 2);
        CHECK_FALSE(rider->SoundOncePlayed);
    }
    CHECK(Draw(scope, *rider, MONSTER01_ATTACK1, 3) == Sounds({attack1}));
    CHECK(rider->SoundOncePlayed);
    // As the Aida draw code played them: without a stand, none.
    CHECK(Draw(scope, *rider, MONSTER01_ATTACK2, 2).empty());
    CHECK(Draw(scope, *rider, MONSTER01_STOP1).empty());
    CHECK_FALSE(rider->SoundOncePlayed);
    CHECK(Draw(scope, *rider, MONSTER01_ATTACK2) == Sounds({attack2}));
    // A death right after an attack is silent, as in the original client.
    CHECK(Draw(scope, *rider, MONSTER01_DIE, 2).empty());
    CHECK(Draw(scope, *rider, MONSTER01_STOP2).empty());
    CHECK(Draw(scope, *rider, MONSTER01_DIE) == Sounds({die}));
    // A walk clears it too (Q-MN89).
    CHECK(Draw(scope, *rider, MONSTER01_STOP1).empty());
    CHECK(Draw(scope, *rider, MONSTER01_ATTACK1, 2) == Sounds({attack1}));
    CHECK(rider->SoundOncePlayed);
    // (Its walk sounds come by their chance; they do not matter here.)
    Draw(scope, *rider, MONSTER01_WALK);
    CHECK_FALSE(rider->SoundOncePlayed);
    CHECK(Draw(scope, *rider, MONSTER01_ATTACK1) == Sounds({attack1}));
    // The flag is the character's own; SubType keeps its value.
    CHECK(rider->Object.SubType == 0);
}

TEST_CASE("Monster sounds: a once event whose chance does not come up leaves the flag clear (Q-MN85) "
          "[monsters][sounds]")
{
    SoundScope scope(WD_33AIDA);
    SetChange change("Death Rider",
                     [](MonsterSoundSet& set)
                     {
                         AttacksOnce(set);
                         set.soundEvents[1].chance =
                             Data::Monsters::MonsterSoundChance{1000, Data::Monsters::MonsterSoundDraw::Rand};
                     });
    const std::unique_ptr<CHARACTER> rider = MakeCharacter(DeathRider);
    Random::SeedRand(7);
    const bool comesUp = Random::Rand() % 1000 == 0;
    Random::SeedRand(7);
    CHECK(Draw(scope, *rider, MONSTER01_ATTACK1).size() == (comesUp ? 1u : 0u));
    CHECK(rider->SoundOncePlayed == comesUp);
}

TEST_CASE("Monster sounds: once events of one animation play together: the Witch Queen's thunder and attack "
          "(Q-MN85) [monsters][sounds]")
{
    SoundScope scope(WD_33AIDA);
    SetChange once("Witch Queen", QueenAttacksOnce);
    const MonsterSoundSet& set = SetOf(WitchQueen);
    const std::unique_ptr<CHARACTER> queen = MakeCharacter(WitchQueen);
    const std::vector<PlayedSound> sounds = Draw(scope, *queen, MONSTER01_ATTACK2, 2);
    REQUIRE(sounds.size() == 2);
    CHECK((sounds[0].id == SOUND_CHAOS_THUNDER01 || sounds[0].id == SOUND_CHAOS_THUNDER02));
    CHECK(sounds[1].id == SoundOf(set, "SOUND_AIDA_WITCHQUEEN_ATTACK1"));
}

TEST_CASE("Monster sounds: FinishDrawnFrame plays a drawn frame's events unless the map's code did, once a frame "
          "[monsters][sounds]")
{
    using namespace GameLogic::Monsters::SoundEvents;
    SoundScope scope(WD_33AIDA);
    SetChange change("Death Rider", WalkEveryFrame);
    const int move1 = SoundOf(SetOf(DeathRider), "SOUND_AIDA_DEATHRAIDER_MOVE1");
    const std::unique_ptr<CHARACTER> rider = MakeCharacter(DeathRider);
    rider->Object.CurrentAction = MONSTER01_WALK;
    // The Aida effects' call, then FinishDrawnFrame: one frame, one sound.
    CHECK(PlayDrawn(*rider));
    FinishDrawnFrame(*rider);
    CHECK(scope.Take() == Sounds({move1}));
    // A frame without the map's call: FinishDrawnFrame plays it.
    FinishDrawnFrame(*rider);
    CHECK(scope.Take() == Sounds({move1}));
    // A set without events of drawn frames plays nothing.
    const std::unique_ptr<CHARACTER> bull = MakeCharacter(BullFighter);
    bull->Object.CurrentAction = MONSTER01_WALK;
    CHECK_FALSE(HasDrawnEvents(SetOf(BullFighter)));
    CHECK_FALSE(PlayDrawn(*bull));
    FinishDrawnFrame(*bull);
    CHECK(scope.Take().empty());
}

TEST_CASE("Monster sounds: the Aida monsters' attacks and deaths play when they start, each time, at any distance "
          "(Q-MN86, Q-MN94) [monsters][sounds]")
{
    SoundScope scope(WD_33AIDA);
    const MonsterSoundSet& rider = SetOf(DeathRider);
    const int attack1 = SoundOf(rider, "SOUND_AIDA_DEATHRAIDER_ATTACK1");
    const int attack2 = SoundOf(rider, "SOUND_AIDA_DEATHRAIDER_ATTACK2");
    const int die = SoundOf(rider, "SOUND_AIDA_DEATHRAIDER_DIE");
    CHECK(scope.Play(DeathRider, MONSTER01_ATTACK1) == Sounds({attack1}));
    // Right after another attack or an attack, with no stand between.
    CHECK(scope.Play(DeathRider, MONSTER01_ATTACK2) == Sounds({attack2}));
    CHECK(scope.Play(DeathRider, MONSTER01_DIE) == Sounds({die}));
    // Far from the hero too (Q-MN94).
    CHECK(scope.Play(DeathRider, MONSTER01_ATTACK1, 0.0f, 0, 2000.0f) == Sounds({attack1}));
    CHECK(scope.Play(DeathRider, MONSTER01_ATTACK2, 0.0f, 0, 2000.0f) == Sounds({attack2}));
    CHECK(scope.Play(DeathRider, MONSTER01_DIE, 0.0f, 0, 2000.0f) == Sounds({die}));
    {
        // A set without heardAnywhere still hears its monsters within 500 units.
        SetChange heardNear("Death Rider", HeardNear);
        CHECK(scope.Play(DeathRider, MONSTER01_ATTACK1, 0.0f, 0, 500.0f) == Sounds({attack1}));
        CHECK(scope.Play(DeathRider, MONSTER01_ATTACK1, 0.0f, 0, 600.0f).empty());
    }
    // The Witch Queen's second attack, far from the hero: a thunder and her
    // attack.
    const MonsterSoundSet& queen = SetOf(WitchQueen);
    const std::vector<PlayedSound> sounds = scope.Play(WitchQueen, MONSTER01_ATTACK2, 0.0f, 0, 2000.0f);
    REQUIRE(sounds.size() == 2);
    CHECK((sounds[0].id == SOUND_CHAOS_THUNDER01 || sounds[0].id == SOUND_CHAOS_THUNDER02));
    CHECK(sounds[1].id == SoundOf(queen, "SOUND_AIDA_WITCHQUEEN_ATTACK1"));
    // The drawn frames play only the walk.
    const std::unique_ptr<CHARACTER> character = MakeCharacter(DeathRider);
    CHECK(Draw(scope, *character, MONSTER01_ATTACK1, 3).empty());
}

namespace
{
std::uint32_t g_timerClock = 0;

std::uint32_t TimerClock()
{
    return g_timerClock;
}

// The model list with the slot's model changed, and the timers' clock, for
// the life of the scope.
class ModelChange
{
public:
    ModelChange(int slot, const std::function<void(Data::Monsters::MonsterModelDefinition&)>& change)
    {
        for (const Data::Monsters::MonsterModelDefinition* model : g_MonsterModelDatabase.GetAll())
        {
            m_models.push_back(*model);
        }
        std::vector<Data::Monsters::MonsterModelDefinition> changed = m_models;
        const Data::Monsters::MonsterModelDefinition* own = g_MonsterModelDatabase.Find(slot);
        REQUIRE(own != nullptr);
        for (Data::Monsters::MonsterModelDefinition& model : changed)
        {
            if (model.slot == own->slot)
            {
                change(model);
            }
        }
        g_MonsterModelDatabase.Build(changed);
        GameLogic::Monsters::ModelTimers::SetClock(&TimerClock);
    }
    ~ModelChange()
    {
        GameLogic::Monsters::ModelTimers::SetClock(nullptr);
        g_MonsterModelDatabase.Build(m_models);
    }
    ModelChange(const ModelChange&) = delete;
    ModelChange& operator=(const ModelChange&) = delete;

private:
    std::vector<Data::Monsters::MonsterModelDefinition> m_models;
};
} // namespace

TEST_CASE("Monster sounds: a model timer ticks in its animation each time more than its interval passed, unsigned "
          "as the draw code counted (Q-MN91) [monsters][sounds]")
{
    using GameLogic::Monsters::ModelTimers::Advance;
    using GameLogic::Monsters::ModelTimers::NamesOf;
    const std::vector<Data::Monsters::MonsterModelTimer> timers = {{"hair swing", MONSTER01_WALK, 500},
                                                                   {"blink", MONSTER01_STOP1, 100}};
    GameLogic::Monsters::ModelTimers::LastTicks last{};
    CHECK(Advance(timers, MONSTER01_WALK, 500, last) == 0u);
    CHECK(Advance(timers, MONSTER01_WALK, 501, last) == 1u);
    CHECK(last[0] == 501u);
    CHECK(Advance(timers, MONSTER01_WALK, 1001, last) == 0u);
    CHECK(Advance(timers, MONSTER01_WALK, 1002, last) == 1u);
    // Another animation's timer only; the walk's waits.
    CHECK(Advance(timers, MONSTER01_STOP1, 1600, last) == 2u);
    CHECK(last[0] == 1002u);
    CHECK(NamesOf(timers, 3u) == std::vector<std::string>{"hair swing", "blink"});
    CHECK(NamesOf(timers, 2u) == std::vector<std::string>{"blink"});
    // A clock that wraps.
    last[0] = 0xFFFFFF00u;
    CHECK(Advance(timers, MONSTER01_WALK, 0x100u, last) == 1u);
}

TEST_CASE("Monster sounds: a drawn frame asks the model's timers once, the map's code and the sound events get the "
          "same tick, and a timer event plays on it (Q-MN91) [monsters][sounds]")
{
    using namespace GameLogic::Monsters;
    SoundScope scope(WD_33AIDA);
    const std::unique_ptr<CHARACTER> rider = MakeCharacter(DeathRider);
    const int move1 = SoundOf(SetOf(DeathRider), "SOUND_AIDA_DEATHRAIDER_MOVE1");
    ModelChange model(rider->Object.Type, [](Data::Monsters::MonsterModelDefinition& definition)
                      { definition.timers = {{"hair swing", MONSTER01_WALK, 500}}; });
    SetChange set("Death Rider",
                  [](MonsterSoundSet& definition)
                  {
                      Data::Monsters::MonsterSoundEvent swing;
                      swing.timers = {"hair swing"};
                      swing.heardAnywhere = true;
                      swing.sounds = {"SOUND_AIDA_DEATHRAIDER_MOVE1"};
                      definition.soundEvents = {swing};
                  });
    rider->Object.CurrentAction = MONSTER01_WALK;
    rider->ModelTimerLastTicks.fill(0);
    g_timerClock = 1000;
    // The map's code asks first; the frame's sound events get the same tick.
    CHECK(ModelTimers::Ticked(*rider, "hair swing"));
    CHECK(ModelTimers::Ticked(*rider, "hair swing"));
    CHECK_FALSE(ModelTimers::Ticked(*rider, "tail"));
    SoundEvents::FinishDrawnFrame(*rider);
    CHECK(scope.Take() == Sounds({move1}));
    // The next frames: no tick until more than 500 ms passed.
    g_timerClock = 1400;
    SoundEvents::FinishDrawnFrame(*rider);
    CHECK(scope.Take().empty());
    CHECK_FALSE(ModelTimers::Ticked(*rider, "hair swing"));
    SoundEvents::FinishDrawnFrame(*rider);
    g_timerClock = 1501;
    SoundEvents::FinishDrawnFrame(*rider);
    CHECK(scope.Take() == Sounds({move1}));
    // Standing, it waits.
    rider->Object.CurrentAction = MONSTER01_STOP1;
    g_timerClock = 5000;
    SoundEvents::FinishDrawnFrame(*rider);
    CHECK(scope.Take().empty());
    // A character made again keeps its last ticks, as OBJECT::m_dwTime did.
    CHARACTER* again = scope.Creation().Create(DeathRider);
    REQUIRE(again != nullptr);
    again->ModelTimerLastTicks[0] = 4800;
    // A frame it was asked in does not carry over.
    again->ModelTimersAsked = true;
    again->ModelTimerTicks = 1;
    CHARACTER* made = scope.Creation().Create(DeathRider);
    REQUIRE(made == again);
    CHECK(made->ModelTimerLastTicks[0] == 4800u);
    CHECK_FALSE(made->ModelTimersAsked);
    CHECK(made->ModelTimerTicks == 0u);
}

TEST_CASE("Monster sounds: a monster is created with the once flag clear, as with SubType 0 (Q-MN87) "
          "[monsters][sounds]")
{
    SoundScope scope(WD_33AIDA);
    CHARACTER* first = scope.Creation().Create(DeathRider);
    REQUIRE(first != nullptr);
    first->SoundOncePlayed = true;
    first->SoundsDrawnThisFrame = true;
    CHARACTER* again = scope.Creation().Create(DeathRider);
    REQUIRE(again != nullptr);
    CHECK_FALSE(again->SoundOncePlayed);
    CHECK_FALSE(again->SoundsDrawnThisFrame);
}

#ifdef _EDITOR
TEST_CASE("Monster Preview: during events play each frame, once events after each start of the animation (Q-MN88) "
          "[monsters][sounds][editor]")
{
    SoundScope scope(WD_33AIDA);
    Data::Monsters::ModelLoader::Open(MODEL_DEATH_RIDER);
    REQUIRE(Data::Monsters::ModelLoader::IsOpen(MODEL_DEATH_RIDER));
    scope.Take();
    SetChange once("Death Rider", AttacksOnce);
    const MonsterSoundSet& set = SetOf(DeathRider);
    const int attack1 = SoundOf(set, "SOUND_AIDA_DEATHRAIDER_ATTACK1");
    const Data::Monsters::MonsterModelDefinition& values = ModelOf(MODEL_DEATH_RIDER);
    const float factor = FPS_ANIMATION_FACTOR;
    FPS_ANIMATION_FACTOR = 1.0f;
    g_MonsterSelection.Select(DeathRider);
    g_MuMonsterPreviewUI.PlayAttack(MODEL_DEATH_RIDER, MONSTER01_ATTACK1, std::nullopt);
    g_MuMonsterPreviewUI.Advance(MODEL_DEATH_RIDER, values);
    g_MuMonsterPreviewUI.Advance(MODEL_DEATH_RIDER, values);
    CHECK(scope.Take() == Sounds({attack1}));
    // Started again, it plays again.
    g_MuMonsterPreviewUI.PlayAttack(MODEL_DEATH_RIDER, MONSTER01_ATTACK1, std::nullopt);
    g_MuMonsterPreviewUI.Advance(MODEL_DEATH_RIDER, values);
    CHECK(scope.Take() == Sounds({attack1}));
    {
        SetChange change("Death Rider", WalkEveryFrame);
        const int move1 = SoundOf(SetOf(DeathRider), "SOUND_AIDA_DEATHRAIDER_MOVE1");
        g_MuMonsterPreviewUI.PlayAttack(MODEL_DEATH_RIDER, MONSTER01_WALK, std::nullopt);
        scope.Take();
        for (int frame = 0; frame < 3; ++frame)
        {
            g_MuMonsterPreviewUI.Advance(MODEL_DEATH_RIDER, values);
        }
        CHECK(scope.Take() == Sounds({move1, move1, move1}));
    }
    FPS_ANIMATION_FACTOR = factor;
    g_MonsterSelection.Select(MuEditor::Monsters::MonsterSelection::NoMonster);
    scope.Take();
}
#endif // _EDITOR

#ifdef _EDITOR
TEST_CASE("Monster sounds: the Sound Mixer hears a set's events as monster sounds of the set and the monster "
          "(Q-SM2) [monsters][sounds][editor]")
{
    SoundScope scope(WD_33AIDA);
    Data::Monsters::ModelLoader::Open(MODEL_DEATH_RIDER);
    REQUIRE(Data::Monsters::ModelLoader::IsOpen(MODEL_DEATH_RIDER));
    scope.Take();
    Audio::Mixer::Clear();
    const int attack1 = SoundOf(SetOf(DeathRider), "SOUND_AIDA_DEATHRAIDER_ATTACK1");
    CHECK(scope.Play(DeathRider, MONSTER01_ATTACK1) == Sounds({attack1}));
    const std::vector<Audio::Mixer::HeardSound> heard = Audio::Mixer::Heard();
    Audio::Mixer::Clear();
    REQUIRE(heard.size() == 1);
    CHECK(heard[0].number == attack1);
    CHECK(heard[0].kind == Audio::SoundKind::Monster);
    CHECK(heard[0].name == "Death Rider");
    CHECK(heard[0].sourceNumber == DeathRider);
}

TEST_CASE("Monster sounds: the Sound Mixer's overlay marks a drawn monster of a set with its hearing ring while it "
          "is on, and a start with the monster's place (Q-SM9) [monsters][sounds][editor]")
{
    using namespace GameLogic::Monsters::SoundEvents;
    SoundScope scope(WD_33AIDA);
    // A set heard within 500 units: its ring.
    SetChange heardNear("Death Rider", HeardNear);
    Data::Monsters::ModelLoader::Open(MODEL_DEATH_RIDER);
    REQUIRE(Data::Monsters::ModelLoader::IsOpen(MODEL_DEATH_RIDER));
    scope.Take();
    Audio::Mixer::Clear();
    const std::unique_ptr<CHARACTER> rider = MakeCharacter(DeathRider);
    rider->Object.Position[0] = Hero->Object.Position[0] + 100.0f;
    rider->Object.Position[1] = Hero->Object.Position[1];
    rider->Object.Position[2] = 50.0f;
    Audio::Overlay::SetOn(false);
    FinishDrawnFrame(*rider);
    const bool notedWhileOff = !Audio::Overlay::TakeDrawn().empty();
    Audio::Overlay::SetOn(true);
    FinishDrawnFrame(*rider);
    const std::vector<Audio::Overlay::DrawnSource> drawn = Audio::Overlay::TakeDrawn();
    Audio::Overlay::SetOn(false);
    // An attack's start: the set's event and the attack table's sound.
    rider->Object.CurrentAction = MONSTER01_ATTACK1;
    CHECK(Play(*rider));
    const std::vector<Audio::Mixer::PlacedStart> starts = Audio::Mixer::RecentStarts(Audio::Mixer::Now(), 60.0);
    scope.Take();
    Audio::Mixer::Clear();
    CHECK_FALSE(notedWhileOff);
    REQUIRE(drawn.size() == 1);
    CHECK(drawn[0].kind == Audio::SoundKind::Monster);
    CHECK(drawn[0].name == "Death Rider");
    CHECK(drawn[0].sourceNumber == DeathRider);
    CHECK(drawn[0].position == std::array<float, 3>{rider->Object.Position[0], rider->Object.Position[1], 50.0f});
    CHECK(drawn[0].inner == 500.0f);
    CHECK(drawn[0].outer == 0.0f);
    // A set heard anywhere (Kanturu's Berserker) has no hearing ring.
    {
        SoundScope kanturu(WD_37KANTURU_1ST);
        const std::unique_ptr<CHARACTER> berserker = MakeCharacter(350);
        Audio::Overlay::SetOn(true);
        FinishDrawnFrame(*berserker);
        const std::vector<Audio::Overlay::DrawnSource> anywhere = Audio::Overlay::TakeDrawn();
        Audio::Overlay::SetOn(false);
        kanturu.Take();
        REQUIRE(anywhere.size() == 1);
        CHECK(anywhere[0].name == "Berserker");
        CHECK(anywhere[0].inner == 0.0f);
    }
    REQUIRE_FALSE(starts.empty());
    CHECK(starts.back().kind == Audio::SoundKind::Monster);
    CHECK(starts.back().name == "Death Rider");
    CHECK(starts.back().position == std::array<float, 3>{rider->Object.Position[0], rider->Object.Position[1], 50.0f});
}
#endif // _EDITOR

TEST_CASE("Monster sounds: PlayDrawn says whether an event of the frame played, for the map's effects that "
          "come with a sound: Blade Hunter's smoke with its step (Q-MN92) [monsters][sounds]")
{
    using GameLogic::Monsters::SoundEvents::PlayDrawn;
    SoundScope scope(WD_37KANTURU_1ST);
    constexpr int BladeHunter = 354;
    const std::unique_ptr<CHARACTER> hunter = MakeCharacter(BladeHunter);
    hunter->Object.CurrentAction = MONSTER01_WALK;
    const float factor = FPS_ANIMATION_FACTOR;
    FPS_ANIMATION_FACTOR = 1.0f;
    Random::SeedRand(7);
    int steps = 0;
    for (int frame = 0; frame < 300; ++frame)
    {
        bool played = true;
        CHECK(PlayDrawn(*hunter, &played));
        const std::vector<PlayedSound> sounds = scope.Take();
        CHECK(played == !sounds.empty());
        steps += played ? 1 : 0;
        GameLogic::Monsters::SoundEvents::FinishDrawnFrame(*hunter);
        CHECK(scope.Take().empty());
    }
    FPS_ANIMATION_FACTOR = factor;
    // One in ten frames: about 30 in 300.
    CHECK(steps > 10);
    CHECK(steps < 60);
    // A once event that played tells too; one held back by the flag does not.
    hunter->Object.CurrentAction = MONSTER01_ATTACK1;
    bool played = false;
    PlayDrawn(*hunter, &played);
    CHECK(played);
    PlayDrawn(*hunter, &played);
    CHECK_FALSE(played);
    scope.Take();
    // A character without a set: nothing played.
    const std::unique_ptr<CHARACTER> bull = MakeCharacter(BullFighter);
    played = true;
    CHECK_FALSE(PlayDrawn(*bull, &played));
    CHECK_FALSE(played);
}

TEST_CASE("Monster sounds: Kanturu's monsters sound each death when it starts, also right after an attack, at any "
          "distance (Q-MN93) [monsters][sounds]")
{
    SoundScope scope(WD_37KANTURU_1ST);
    struct Death
    {
        int monster = 0;
        int sound = 0;
    };
    const Death deaths[] = {
        {350, SOUND_KANTURU_1ST_BER_DIE},   {351, SOUND_KANTURU_1ST_SWOLF_DIE}, {352, SOUND_KANTURU_1ST_IR_DIE},
        {353, SOUND_KANTURU_1ST_SATI_DIE},  {354, SOUND_KANTURU_1ST_BLADE_DIE}, {355, SOUND_KANTURU_1ST_KENTA_DIE},
        {356, SOUND_KANTURU_1ST_GIGAN_DIE}, {357, SOUND_KANTURU_1ST_GENO_DIE},  {358, SOUND_KANTURU_2ND_PERSO_DIE},
        {359, SOUND_KANTURU_2ND_TWIN_DIE},  {360, SOUND_KANTURU_2ND_DRED_DIE},
    };
    for (const Death& death : deaths)
    {
        INFO("monster " << death.monster);
        // Far from the hero too.
        CHECK(scope.Play(death.monster, MONSTER01_DIE, 0.0f, 0, 2000.0f) == Sounds({death.sound}));
    }
    // Right after an attack: the once flag does not hold it back.
    const std::unique_ptr<CHARACTER> berserker = MakeCharacter(350);
    Draw(scope, *berserker, MONSTER01_ATTACK1);
    CHECK(berserker->SoundOncePlayed);
    berserker->Object.CurrentAction = MONSTER01_DIE;
    CHECK(GameLogic::Monsters::SoundEvents::Play(*berserker));
    CHECK(scope.Take() == Sounds({SOUND_KANTURU_1ST_BER_DIE}));
    // The drawn frames of the death play nothing more.
    CHECK(Draw(scope, *berserker, MONSTER01_DIE, 3).empty());
}
