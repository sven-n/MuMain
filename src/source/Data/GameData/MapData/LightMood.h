#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <array>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

// The light mood of the world maps (Data/Maps/LightMood.json):
//
//   { "formatVersion": 1,
//     "default": { "base": 0.7, "figures": 0.7, "dynamicGain": 1, "background": 1, "sun": 0.6 },
//     "maps": [ { "world": 10, "name": "Icarus", "background": 0.7 } ] }
//
// Each value scales the Classic light of a world map:
//   base         the static (baked) light of the terrain and the map objects, 0.2 to 1;
//   figures      the static light on characters, monsters and NPCs, from base to 1;
//   dynamicGain  the light of candles, fires, skills and effects, 0.5 to 2;
//   background   the clear and fog colour behind the scene, 0 to 1;
//   sun          the directional light on models (buildings, rocks, figures):
//                how far a face turned to it gets brighter than one turned
//                away, 0 (none, only the even fill light) to 1 (Classic).
//   shadowKeep   optional, 0 (default) to 1.5: the baked light at which a place
//                keeps its full brightness. base and figures then darken the
//                shade only, fading out towards this light (sunlit ground stays
//                day); 0 darkens everything evenly.
//   shadeCool    optional, 0 (default) to 0.5: how much bluer the darkened
//                shade gets (red and green darken a little more).
//   brightScale  optional, 0.3 to 1 (default, Classic): the scale places at the
//                shadow keep get, never below base (figures: never below the
//                figure scale); only with a shadow keep.
// "default" has all five and applies to every map without an entry. A map
// entry names its world number and overrides any of them; "name" is for the
// reader only.
namespace Data::Maps
{
struct LightMood
{
    double base = 1.0;
    double figures = 1.0;
    double dynamicGain = 1.0;
    double background = 1.0;
    double sun = 1.0;
    double shadowKeep = 0.0;
    double shadeCool = 0.0;
    double brightScale = 1.0;

    bool operator==(const LightMood&) const = default;
};

struct MapLightMood
{
    int world = 0;
    LightMood mood;

    bool operator==(const MapLightMood&) const = default;
};

struct LightMoodData
{
    LightMood defaultMood;
    std::vector<MapLightMood> maps;

    // The world's own mood, or the default.
    const LightMood& ForWorld(int world) const;
};

constexpr int LightMoodFormatVersion = 1;

// The darkest base: the figure light divides by it.
constexpr double MinLightMoodBase = 0.2;
constexpr double MinLightMoodDynamicGain = 0.5;
constexpr double MaxLightMoodDynamicGain = 2.0;
// The server sends the map number as one byte.
constexpr int MaxLightMoodWorld = 255;

// One value of a mood: its key in the file, the member it fills, whether the
// default must name it, and its range. "figures" has the effective base as its
// lower bound on top of `min`.
struct LightMoodValueInfo
{
    const char* key;
    double LightMood::* member;
    bool required;
    double min;
    double max;
};

constexpr std::size_t LightMoodValueCount = 8;
[[nodiscard]] const std::array<LightMoodValueInfo, LightMoodValueCount>& LightMoodValueInfos();
// The value with that key in the file, if there is one.
[[nodiscard]] const LightMoodValueInfo* FindLightMoodValue(std::string_view key);

// The file, relative to the client folder.
constexpr const char* LightMoodFile = "Data/Maps/LightMood.json";

// Reads the moods of the file into `data`. One invalid value makes the whole
// file unusable: `data` is then left Classic (every value 1) and `issues`
// has errors (Items::HasErrors).
void ReadLightMoodJson(std::string_view text, const std::string& source, LightMoodData& data,
                       std::vector<Items::ItemDataIssue>& issues);

struct LightMoodLoadResult
{
    LightMoodData data;
    std::vector<Items::ItemDataIssue> issues;
};

// Reads the file; one that cannot be read is an error.
LightMoodLoadResult LoadLightMoodFile(const std::filesystem::path& file);
} // namespace Data::Maps
