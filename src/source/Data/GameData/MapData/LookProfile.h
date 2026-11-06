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
//   { "formatVersion": 1,
//     "default": { "lighting.rim.strength": 0.35, "bloom.strength": 0.6 },
//     "maps": [ { "world": 1, "name": "Dungeon", "grading.strength": 0.8 } ] }
//
// Every value has a built-in value (the renderer's behaviour without the file)
// and a range. The values apply in the order built-in, "default", the map's
// entry; either may name any subset of them. "world" is the map number as in
// LightMood.json, "name" is for the reader only. A file with an unknown name, a
// value of the wrong type or out of range, a missing or unsupported
// formatVersion, a duplicate world or a world outside 0 to 255 is unusable as a
// whole: the built-in values apply everywhere.
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
    BloomStrength,
    BloomLevelFalloff,
    GradingStrength,
    Count,
};

inline constexpr std::size_t LookValueCount = static_cast<std::size_t>(LookValue::Count);

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

struct MapLookOverrides
{
    int world = 0;
    LookOverrides overrides;

    bool operator==(const MapLookOverrides&) const = default;
};

struct LookProfileData
{
    LookOverrides defaultOverrides;
    std::vector<MapLookOverrides> maps;

    // Built-in, then the default, then the world's entry.
    [[nodiscard]] LookValues ForWorld(int world) const;
    bool operator==(const LookProfileData&) const = default;
};

[[nodiscard]] LookValues BuiltInLookValues();

constexpr int LookProfileFormatVersion = 1;
// The server sends the map number as one byte.
constexpr int MaxLookProfileWorld = 255;

// The file, relative to the client folder.
constexpr const char* LookProfileFile = "Data/Render/Look.json";

// Reads the profile of the file into `data`. One error makes the whole file
// unusable: `data` is then empty (every value built-in) and `issues` has
// errors (Items::HasFatal).
void ReadLookProfileJson(std::string_view text, const std::string& source, LookProfileData& data,
                         std::vector<Items::ItemDataIssue>& issues);

struct LookProfileLoadResult
{
    LookProfileData data;
    std::vector<Items::ItemDataIssue> issues;
};

// Reads the file. A missing file gives the built-in values with a warning; an
// unreadable or invalid one gives them with fatal problems.
LookProfileLoadResult LoadLookProfileFile(const std::filesystem::path& file);
} // namespace Data::Maps
