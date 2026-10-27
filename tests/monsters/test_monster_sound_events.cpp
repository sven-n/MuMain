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
#include "Engine/Object/ZzzCharacter.h"
#include "Core/Utilities/Random.h"
#include "GameLogic/Monsters/MonsterSoundEvents.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "World/MapInfra/MapManager.h"
#include "World/MapInfra/w_MapProcess.h"

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
