#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MapData/LookProfile.h"

#include <array>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

// The light mood of the world maps: the "mood." values of the look profile
// (LookProfile.h), resolved for one map (MoodOf). Before format 2 of
// Data/Render/Look.json they had a file of their own, Data/Maps/LightMood.json:
//
//   { "formatVersion": 1,
//     "default": { "base": 0.7, "figures": 0.7, "dynamicGain": 1, "background": 1, "sun": 0.6 },
//     "maps": [ { "world": 10, "name": "Icarus", "background": 0.7 } ] }
//
// which is read only to upgrade a format 1 look profile (UpgradeLightMood).
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
// In the old file, "default" has the first five and applies to every map
// without an entry; a map entry names its world number and overrides any of
// them; "name" is for the reader only.
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

// The mood the look values give: their mood.* values.
[[nodiscard]] LightMood MoodOf(const LookValues& values);

constexpr std::size_t LightMoodValueCount = 8;

struct MapLightMood
{
    int world = 0;
    LightMood mood;
    // Which values the entry names, by LightMoodValueInfos order: those it keeps when the default changes.
    std::array<bool, LightMoodValueCount> named{};

    bool operator==(const MapLightMood&) const = default;
};

struct LightMoodData
{
    LightMood defaultMood;
    std::vector<MapLightMood> maps;

    // The world's own mood, or the default.
    const LightMood& ForWorld(int world) const;
    bool operator==(const LightMoodData&) const = default;
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

[[nodiscard]] const std::array<LightMoodValueInfo, LightMoodValueCount>& LightMoodValueInfos();
// The value with that key in the file, if there is one.
[[nodiscard]] const LightMoodValueInfo* FindLightMoodValue(std::string_view key);
// The look value of the mood value at `index` of LightMoodValueInfos: "base" is mood.base.
[[nodiscard]] LookValue MoodLookValue(std::size_t index);

// The old file, relative to the client folder.
constexpr const char* LightMoodFile = "Data/Maps/LightMood.json";

// Gives a format 1 look profile the moods of the old file: the default's values
// on the profile's default, and each map entry's named values on the entry of
// its world (a new entry when the profile has none). A map keeps every value it
// named, also one equal to the default.
void UpgradeLightMood(LookProfileData& profile, const LightMoodData& moods);

// The mood of every world 0 to 255 the profile gives: the default's, and an
// entry for each world whose mood differs from it, naming the values that do.
[[nodiscard]] LightMoodData MoodsOf(const LookProfileData& profile);

// Reads the moods of the file into `data`. One invalid value makes the whole
// file unusable: `data` is then left Classic (every value 1) and `issues`
// has fatal problems (Items::HasFatal).
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
