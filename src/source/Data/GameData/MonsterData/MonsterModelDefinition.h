#pragma once

#include <array>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// The entries of Data/Monsters/Models.json: what the client loads into a
// monster or NPC model slot (monsters design, D45-D47), see
// docs/monster-data.md "Models".
namespace Data::Monsters
{
// LoadWaveFile's defaults.
constexpr int DefaultSoundChannels = 4;

// One sound the model loads.
struct MonsterModelSound
{
    // "Data/Sound/mBull1.wav".
    std::string file;
    // The SOUND_* id that code plays the sound by ("SOUND_MONSTER_BULL1").
    // Empty: the sound gets a number when it is loaded (Q-MN16).
    std::string id;
    int channels = DefaultSoundChannels;
    // Played in 3D, louder near the hero.
    bool spatial = false;

    bool operator==(const MonsterModelSound&) const = default;
};

// The name a sound slot or a sound event names the model's sound by: its id,
// or its file when it has none.
inline const std::string& SoundNameOf(const MonsterModelSound& sound)
{
    return sound.id.empty() ? sound.file : sound.id;
}

// The sound slots of the model: the game plays one of the two idle sounds
// while it stands or walks, one of the two attack sounds when it attacks or
// is hit, and the die sound. A slot names a sound by its id, or by its file
// when the sound has no id; an empty slot plays nothing.
constexpr int SoundSlotSetSize = 5;
using MonsterSoundSlotSet = std::array<std::string, SoundSlotSetSize>;
struct MonsterSoundSlots
{
    MonsterSoundSlotSet first;
    // What a monster with SubType 9 plays (the Hellas monsters): the game
    // plays the first set's sound numbers + 5, which the second set names.
    std::optional<MonsterSoundSlotSet> second;

    bool operator==(const MonsterSoundSlots&) const = default;
};

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

// Sounds that the game plays for a character of the model while it is in one
// of `animations` (monsters design, MN7): PlayMonsterSound, at the start of an
// attack, a hit and the death, at each step of a walk and at the end of each
// loop of stop1 and stop2, on top of the sound slots.
struct MonsterSoundEvent
{
    // Sorted.
    std::vector<int> animations;
    // Only while the animation's frame is from `first` to below `second`.
    std::optional<std::pair<float, float>> frames;
    // Only while the character's death effect is armed (OBJECT::LifeTime 100).
    bool deathEffectArmed = false;
    std::optional<MonsterSoundChance> chance;
    // Plays the first of two sounds when a draw of one in two comes up
    // (rand_fps_check(2), or rand() % 2 == 0), else the second.
    std::optional<MonsterSoundDraw> pick;
    // The sounds, played together (one of the two with `pick`), each named
    // as a sound slot names it: by its id, or by the file of a sound of the
    // model without one.
    std::vector<std::string> sounds;

    bool operator==(const MonsterSoundEvent&) const = default;
};

// Texture folders that the model uses on some maps instead of its own.
struct MonsterModelWorldFolders
{
    std::vector<int> worlds;
    std::vector<std::string> folders;

    bool operator==(const MonsterModelWorldFolders&) const = default;
};

// A bitmap the model loads into a fixed bitmap slot, which its drawing code
// uses (Tantallos' skins, Crust's robe).
struct MonsterModelBitmap
{
    // Below Data/: "Monster/bv01_2.jpg".
    std::string file;
    // "BITMAP_MONSTER_SKIN", "BITMAP_ROBE+3".
    std::string slot;
    bool linearFilter = false;
    bool repeat = false;

    bool operator==(const MonsterModelBitmap&) const = default;
};

// An animation speed: the animation's number and its frames per game frame.
using MonsterAnimationSpeed = std::pair<int, float>;

// What a model slot loads with its file, apart from the file itself; part
// models have these too.
struct MonsterModelLoad
{
    // "Data/Monster/Monster01.bmd", in its spelling on disk.
    std::string file;
    // Below Data/, searched in this order: "Monster", "NPC/LuckyItem".
    std::vector<std::string> textureFolders;
    // Animation speeds that differ from the defaults of the slot's range,
    // sorted by animation.
    std::vector<MonsterAnimationSpeed> speeds;
    // The animations that hold their last frame instead of starting again;
    // nullopt: the range's default (monsters: die).
    std::optional<std::vector<int>> holdLastFrame;

    bool operator==(const MonsterModelLoad&) const = default;
};

// A model that another model (or a map) loads with it: the Ice Giant's
// pieces, the merchants' clothes, the Kundun parts of Hellas. It has no
// defaults of a range: only what is listed is set.
struct MonsterModelPart
{
    // A code symbol of an NPC slot ("MODEL_MERCHANT_FEMALE_HEAD+1"), or the
    // name of an effect type of the effect catalogue ("iceGiantPart1").
    std::string slot;
    std::string effect;
    MonsterModelLoad load;
    // Loaded when one of these maps loads, not with the model.
    std::vector<int> worlds;

    bool operator==(const MonsterModelPart&) const = default;
};

// What CreateCharacterPointer gives a character of the model.
struct MonsterModelBody
{
    // nullopt: the default box.
    std::optional<std::array<float, 3>> boxMin;
    std::optional<std::array<float, 3>> boxMax;
    // The bones its weapons hang on (right, left); nullopt: 33 and 42. One
    // bone sets only the right hand's, none leaves both as the character
    // slot had them.
    std::optional<std::vector<int>> linkBones;

    bool operator==(const MonsterModelBody&) const = default;
};

struct MonsterModelDefinition
{
    // The slot: the code's symbol ("MONSTER_MODEL_BULL_FIGHTER" in the
    // monster range, "MODEL_SMITH" in the NPC range).
    std::string slot;
    MonsterModelLoad load;
    // A texture name that replaces the file's, by mesh (the Little Santas
    // share one file and have one texture each).
    std::vector<std::pair<int, std::string>> textureNames;
    std::vector<MonsterModelWorldFolders> worldTextureFolders;
    // nullopt: as the file has it.
    std::optional<int> headBone;
    std::optional<int> streamMesh;
    // Meshes drawn without blending.
    std::vector<int> noneBlendMeshes;
    std::vector<MonsterModelSound> sounds;
    // nullopt: no slot has a sound (the loader sets them all to -1).
    std::optional<MonsterSoundSlots> soundSlots;
    // In order: one call can play several, and their draws come in this
    // order.
    std::vector<MonsterSoundEvent> soundEvents;
    // The maps the sound events play on; empty: every map.
    std::vector<int> soundEventWorlds;
    std::vector<MonsterModelBitmap> bitmaps;
    std::vector<MonsterModelPart> parts;
    MonsterModelBody body;

    bool operator==(const MonsterModelDefinition&) const = default;
};
} // namespace Data::Monsters
