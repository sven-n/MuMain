#pragma once

#include "Data/GameData/MapData/MapRef.h"

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
// A named point of one of the model's animations that sound events play on
// (monsters design, Q-MN79): "step 1" where a foot lands in the walk.
struct MonsterModelCue
{
    std::string name;
    int animation = 0;
    // From the first frame to below the second, as the game checks the frame
    // each game frame.
    std::pair<float, float> frames;

    bool operator==(const MonsterModelCue&) const = default;
};

// Texture folders that the model uses on some maps (server map numbers)
// instead of its own.
struct MonsterModelMapFolders
{
    std::vector<Data::Maps::MapRef> maps;
    std::vector<std::string> folders;

    bool operator==(const MonsterModelMapFolders&) const = default;
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
    // Loaded when one of these maps (server map numbers) loads, not with the
    // model.
    std::vector<Data::Maps::MapRef> maps;

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
    std::vector<MonsterModelMapFolders> mapTextureFolders;
    // nullopt: as the file has it.
    std::optional<int> headBone;
    std::optional<int> streamMesh;
    // Meshes drawn without blending.
    std::vector<int> noneBlendMeshes;
    // The points of its animations that sound events play on; the names are
    // unique. The sounds are in the sound sets that its monsters name.
    std::vector<MonsterModelCue> cues;
    std::vector<MonsterModelBitmap> bitmaps;
    std::vector<MonsterModelPart> parts;
    MonsterModelBody body;

    bool operator==(const MonsterModelDefinition&) const = default;
};
} // namespace Data::Monsters
