#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// The look profile of the Enhanced rendering features (Data/Render/Look.json):
//
//   { "formatVersion": 2,
//     "default": { "lighting.rim.strength": 0.35, "mood.base": 0.6 },
//     "environments": { "enclosed": { "sun.shadow": 0, "mood.base": 0.45 } },
//     "maps": [ { "maps": [11, 12, 13, 14, 15, 16, 17, 52], "name": "Blood Castle", "environment": "fixed" } ] }
//
// Every value has a built-in value (the renderer's behaviour without the file)
// and a range; the light mood's values are the "mood." ones. The values apply in
// the order built-in, "default", the map's environment, the map's entry; each
// may name any subset of them. A map entry lists the loaded worlds it is for
// (the variants of one map share an entry), "name" is for the reader only and
// "environment" names one of "environments": the kind of place a map is.
// Format 1 named one world per entry ("world").
//
// A file with an unknown name, a value of the wrong type or out of range, a
// missing or unsupported formatVersion, a map in two entries, a map outside 0
// to 255, an unknown environment or figures below base (ValidateLookProfile) is
// unusable as a whole: the built-in values apply everywhere.
namespace Data::Maps
{
enum class LookValue : std::uint8_t
{
    DiffuseScale,
    DiffuseAmbient,
    DiffuseFloor,
    HighlightKnee,
    RimStrength,
    RimPower,
    SpecularScale,
    SpecularPower,
    DynamicWrap,
    DynamicMapSmoothing,
    DynamicEffectGain,
    DynamicEffectRange,
    DynamicEffectRangeMax,
    DynamicEffectFade,
    DynamicEffectCap,
    GroundShadowOpacity,
    GroundShadowAngle,
    GroundShadowLights,
    BloomStrength,
    BloomLevelFalloff,
    GradingStrength,
    AmbientOcclusionStrength,
    AmbientOcclusionRadius,
    SunShadowStrength,
    SunShadowSoftness,
    SunShadowResolution,
    FlatSunShadow,
    CloudStrength,
    CloudScale,
    CloudSpeed,
    CloudCover,
    SkyFill,
    RimSunFollow,
    // The light mood (Render::Lighting::LightMood), last so the file lists them after the others.
    MoodBase,
    MoodFigures,
    MoodDynamicGain,
    MoodBackground,
    MoodSun,
    MoodShadowKeep,
    MoodShadeCool,
    MoodBrightScale,
    Count,
};

inline constexpr std::size_t LookValueCount = static_cast<std::size_t>(LookValue::Count);

// The light mood's values: the map's own light, not something a player tunes.
[[nodiscard]] constexpr bool IsMoodValue(LookValue value)
{
    return value >= LookValue::MoodBase && value < LookValue::Count;
}

struct LookValueInfo
{
    LookValue value;
    const char* name;
    double builtIn;
    double min;
    double max;
};

[[nodiscard]] constexpr bool IsInRange(const LookValueInfo& info, double value)
{
    return value >= info.min && value <= info.max;
}

// Every value with its name in the file, its built-in value and its range, in LookValue order.
[[nodiscard]] const std::array<LookValueInfo, LookValueCount>& LookValueInfos();
[[nodiscard]] const LookValueInfo& Info(LookValue value);
// The value with that name in the file, if there is one.
[[nodiscard]] std::optional<LookValue> FindLookValue(std::string_view name);

// A full set of values, indexed by LookValue.
struct LookValues
{
    std::array<double, LookValueCount> values{};

    [[nodiscard]] double operator[](LookValue value) const
    {
        return values[static_cast<std::size_t>(value)];
    }
    double& operator[](LookValue value)
    {
        return values[static_cast<std::size_t>(value)];
    }
    bool operator==(const LookValues&) const = default;
};

// The values an entry of the file names; the others are left to the layer below.
struct LookOverrides
{
    std::array<std::optional<double>, LookValueCount> values{};

    [[nodiscard]] const std::optional<double>& operator[](LookValue value) const
    {
        return values[static_cast<std::size_t>(value)];
    }
    std::optional<double>& operator[](LookValue value)
    {
        return values[static_cast<std::size_t>(value)];
    }
    // Replaces the values of `target` this entry names.
    void ApplyTo(LookValues& target) const;
    bool operator==(const LookOverrides&) const = default;
};

struct EnvironmentLookOverrides
{
    std::string name;
    LookOverrides overrides;

    bool operator==(const EnvironmentLookOverrides&) const = default;
};

struct MapLookOverrides
{
    // The loaded worlds (gMapManager.WorldActive) the entry is for.
    std::vector<int> maps;
    std::string name;
    // Index into LookProfileData::environments.
    std::optional<std::size_t> environment;
    LookOverrides overrides;

    [[nodiscard]] bool Has(int world) const;
    bool operator==(const MapLookOverrides&) const = default;
};

struct LookProfileData
{
    LookOverrides defaultOverrides;
    std::vector<EnvironmentLookOverrides> environments;
    std::vector<MapLookOverrides> maps;

    // Built-in, then the default, the world's environment and the world's entry.
    [[nodiscard]] LookValues ForWorld(int world) const;
    // Built-in, then the default and the environment.
    [[nodiscard]] LookValues ForEnvironment(std::size_t environment) const;
    // The entry whose maps have `world`.
    [[nodiscard]] const MapLookOverrides* FindMap(int world) const;
    [[nodiscard]] MapLookOverrides* FindMap(int world);
    bool operator==(const LookProfileData&) const = default;
};

[[nodiscard]] LookValues BuiltInLookValues();

constexpr int LookProfileFormatVersion = 2;
// The server sends the map number as one byte.
constexpr int MaxLookProfileWorld = 255;

// The file, relative to the client folder.
constexpr const char* LookProfileFile = "Data/Render/Look.json";

// Reads the profile of the file into `data`. One error makes the whole file
// unusable: `data` is then empty (every value built-in) and `issues` has
// errors (Items::HasFatal). `formatVersion`, when given, gets the file's
// version (0 when it has none): a version 1 profile still needs its light mood
// (UpgradeLightMood).
void ReadLookProfileJson(std::string_view text, const std::string& source, LookProfileData& data,
                         std::vector<Items::ItemDataIssue>& issues, int* formatVersion = nullptr);

// The rules between entries and values, on the whole profile: a map in at most
// one entry, every map entry with maps, environments that exist, and
// mood.figures not below mood.base on the default, on each environment over the
// default and on each map. False with fatal `issues` when one fails.
bool ValidateLookProfile(const LookProfileData& data, const std::string& source,
                         std::vector<Items::ItemDataIssue>& issues);

// The profile as the file has it, always the same text for the same profile:
// formatVersion, default, environments by name, map entries by their first map;
// values in LookValue order; the default one value per line, each environment
// and map entry on one line.
[[nodiscard]] std::string WriteLookProfileJson(const LookProfileData& data);

struct LookProfileLoadResult
{
    LookProfileData data;
    std::vector<Items::ItemDataIssue> issues;
};

// Reads the file. A missing file gives the built-in values with a warning; an
// unreadable or invalid one gives them with fatal problems.
LookProfileLoadResult LoadLookProfileFile(const std::filesystem::path& file);
} // namespace Data::Maps
