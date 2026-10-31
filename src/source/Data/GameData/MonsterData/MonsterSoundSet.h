#pragma once

#include "Data/GameData/MapData/MapRef.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

// The sound sets of Data/Monsters/Sounds.json (monsters design, Q-MN78): the
// sounds a monster plays, which every monster names (MonsterDefinition::
// soundSet), see docs/monster-data.md "Sounds".
namespace Data::Monsters
{
// LoadWaveFile's defaults.
constexpr int DefaultSoundChannels = 4;
// The limits of the sound sets, which the reader refuses past and the editor
// keeps to: the channels of a sound (1 to this), a sound event's chance (one
// in 1 to this), and the sounds of a pick (one of two, or one or none).
constexpr int MaxSoundChannels = 4;
constexpr int MaxSoundChanceOneIn = 100000;
constexpr std::size_t PickSoundCount = 2;

// One sound file a set loads.
struct MonsterSound
{
    // "Data/Sound/mBull1.wav".
    std::string file;
    // The SOUND_* id that code plays the sound by ("SOUND_MONSTER_BULL1").
    // Empty: the sound gets a number when it is loaded (Q-MN16).
    std::string id;
    int channels = DefaultSoundChannels;
    // Played in 3D, louder near the hero.
    bool spatial = false;

    bool operator==(const MonsterSound&) const = default;
};

// The name a sound event names the set's sound by: its id, or its file when
// it has none.
inline const std::string& SoundNameOf(const MonsterSound& sound)
{
    return sound.id.empty() ? sound.file : sound.id;
}

// How a sound event draws: rand_fps_check as the original client did, whose
// chance depends on the frame rate when it is drawn once per step, attack,
// hit or idle loop, or rand().
enum class MonsterSoundDraw
{
    RandFps,
    Rand,
};

// A chance of one in `oneIn`: rand_fps_check(oneIn), or rand() % oneIn == 0.
struct MonsterSoundChance
{
    int oneIn = 1;
    MonsterSoundDraw draw = MonsterSoundDraw::RandFps;

    bool operator==(const MonsterSoundChance&) const = default;
};

// The moments of the game that play a sound event whatever the character's
// animation (monsters design, Q-MN76): where the sound slots played.
enum class MonsterSoundMoment
{
    // SetPlayerAttack, at frame 0.
    Attack,
    // SetPlayerShock, at frame 0.
    Hit,
    // SetPlayerDie, at frame 0, Chaos Castle included (Q-MN77).
    Death,
    // SetPlayerStop: the character is set to stand (a walk ends, the server
    // stops it).
    Stand,
    // SetPlayerWalk: each step of a walk.
    Walk,
};

// Sounds that the game plays for a monster of the set (monsters design, MN7,
// Q-MN76, Q-MN79): while it is in one of `animations` (PlayMonsterSound, at
// the start of an attack, a hit and the death, at each step of a walk and at
// the end of each loop of stop1 and stop2), in one of the frames its model's
// `cues` name, or at one of `moments`. An event names one of the three.
struct MonsterSoundEvent
{
    // Sorted.
    std::vector<int> animations;
    // Sorted.
    std::vector<MonsterSoundMoment> moments;
    // Cues of the monster's model (MonsterModelCue::name), in their order.
    std::vector<std::string> cues;
    // Only while the character's death effect is armed (OBJECT::LifeTime 100).
    bool deathEffectArmed = false;
    // Heard at any distance from the hero and on every map; else within 500
    // units and on the set's maps.
    bool heardAnywhere = false;
    // Played with the character as PlayBuffer's object, which does not start
    // the sound again while it plays.
    bool atMonster = false;
    std::optional<MonsterSoundChance> chance;
    // Plays the first sound when a draw of one in two comes up
    // (rand_fps_check(2), or rand() % 2 == 0), else the second, or none
    // without a second.
    std::optional<MonsterSoundDraw> pick;
    // The sounds, played together (one of them with `pick`), each named by
    // its id, or by the file of a sound of the set without one.
    std::vector<std::string> sounds;

    bool operator==(const MonsterSoundEvent&) const = default;
};

// A sound set: what monsters that name it play, and the files a model loads
// for them.
struct MonsterSoundSet
{
    // Its name, which monsters name it by ("Bull Fighter"); unique.
    std::string name;
    std::vector<MonsterSound> sounds;
    // In order: one call can play several, and their draws come in this
    // order.
    std::vector<MonsterSoundEvent> soundEvents;
    // The maps (server map numbers) the sound events play on; empty: every
    // map.
    std::vector<Data::Maps::MapRef> maps;

    bool operator==(const MonsterSoundSet&) const = default;
};
} // namespace Data::Monsters
