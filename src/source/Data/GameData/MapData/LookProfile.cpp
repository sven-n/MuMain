#include "stdafx.h"

#include "LookProfile.h"

#include "Core/Content/ContentStore.h"

#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/MapData/LightMood.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <map>
#include <sstream>
#include <system_error>
#include <utility>

namespace Data::Maps
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::Json::OrderedJson;

constexpr const char* DefaultKey = "default";
constexpr const char* MapsKey = "maps";
constexpr const char* WorldKey = "world";
constexpr const char* NameKey = "name";
constexpr const char* EnvironmentsKey = "environments";
constexpr const char* EnvironmentKey = "environment";

// The built-in values are the renderer's behaviour before the profile existed; a doctest checks
// each against the constant it replaces.
constexpr std::array<LookValueInfo, LookValueCount> Infos = {{
    {LookValue::DiffuseScale, "lighting.diffuse.scale", 0.8, 0.0, 2.0},
    {LookValue::DiffuseAmbient, "lighting.diffuse.ambient", 0.4, 0.0, 1.0},
    {LookValue::DiffuseFloor, "lighting.diffuse.floor", 0.2, 0.0, 1.0},
    {LookValue::HighlightKnee, "lighting.highlightKnee", 0.8, 0.5, 0.9},
    {LookValue::RimStrength, "lighting.rim.strength", 0.35, 0.0, 1.0},
    {LookValue::RimPower, "lighting.rim.power", 3.0, 1.0, 8.0},
    {LookValue::SpecularScale, "lighting.specular.scale", 1.0, 0.0, 2.0},
    {LookValue::SpecularPower, "lighting.specular.power", 32.0, 4.0, 128.0},
    {LookValue::DynamicWrap, "dynamic.wrap", 0.5, 0.0, 0.5},
    {LookValue::DynamicMapSmoothing, "dynamic.mapSmoothing", 0.0, 0.0, 2.0},
    {LookValue::DynamicEffectGain, "dynamic.effectGain", 1.0, 0.0, 4.0},
    {LookValue::DynamicEffectRange, "dynamic.effectRange", 1.0, 0.5, 4.0},
    {LookValue::DynamicEffectRangeMax, "dynamic.effectRangeMax", 0.0, 0.0, 16.0},
    {LookValue::DynamicEffectFade, "dynamic.effectFade", 0.0, 0.0, 2.0},
    {LookValue::DynamicEffectCap, "dynamic.effectCap", 0.0, 0.0, 8.0},
    {LookValue::GroundShadowOpacity, "dynamic.shadowOpacity", 0.6, 0.0, 1.0},
    {LookValue::GroundShadowAngle, "dynamic.shadowAngle", 180.0, 0.0, 360.0},
    {LookValue::GroundShadowLights, "dynamic.shadowLights", 0.0, 0.0, 4.0},
    {LookValue::BloomStrength, "bloom.strength", 0.6, 0.0, 2.0},
    {LookValue::BloomLevelFalloff, "bloom.levelFalloff", 0.5, 0.25, 1.0},
    {LookValue::GradingStrength, "grading.strength", 1.0, 0.0, 1.0},
    {LookValue::AmbientOcclusionStrength, "ao.strength", 0.0, 0.0, 2.0},
    {LookValue::AmbientOcclusionRadius, "ao.radius", 60.0, 10.0, 300.0},
    {LookValue::SunShadowStrength, "sun.shadow", 0.0, 0.0, 1.0},
    {LookValue::SunShadowSoftness, "sun.softness", 1.0, 0.0, 2.0},
    {LookValue::SunShadowResolution, "sun.resolution", 2048.0, 1024.0, 4096.0},
    {LookValue::FlatSunShadow, "dynamic.flatSunShadow", 1.0, 0.0, 1.0},
    {LookValue::CloudStrength, "clouds.strength", 0.0, 0.0, 1.0},
    {LookValue::CloudScale, "clouds.scale", 3000.0, 500.0, 10000.0},
    {LookValue::CloudSpeed, "clouds.speed", 40.0, 0.0, 300.0},
    {LookValue::CloudCover, "clouds.cover", 0.45, 0.0, 1.0},
    {LookValue::SkyFill, "sky.fill", 0.0, 0.0, 1.0},
    {LookValue::RimSunFollow, "lighting.rim.sunFollow", 0.0, 0.0, 1.0},
    // The light mood's built-in values are Classic's light; their ranges are the mood file's (a doctest
    // compares them with LightMoodValueInfos).
    {LookValue::MoodBase, "mood.base", 1.0, MinLightMoodBase, 1.0},
    {LookValue::MoodFigures, "mood.figures", 1.0, MinLightMoodBase, 1.0},
    {LookValue::MoodDynamicGain, "mood.dynamicGain", 1.0, MinLightMoodDynamicGain, MaxLightMoodDynamicGain},
    {LookValue::MoodBackground, "mood.background", 1.0, 0.0, 1.0},
    {LookValue::MoodSun, "mood.sun", 1.0, 0.0, 1.0},
    {LookValue::MoodShadowKeep, "mood.shadowKeep", 0.0, 0.0, 1.5},
    {LookValue::MoodShadeCool, "mood.shadeCool", 0.0, 0.0, 0.5},
    {LookValue::MoodBrightScale, "mood.brightScale", 1.0, 0.3, 1.0},
}};

constexpr bool InfosFollowLookValueOrder()
{
    for (std::size_t index = 0; index < Infos.size(); ++index)
    {
        if (static_cast<std::size_t>(Infos[index].value) != index)
        {
            return false;
        }
    }
    return true;
}
static_assert(InfosFollowLookValueOrder());

class IssueList
{
public:
    IssueList(const std::string& source, std::vector<ItemDataIssue>& issues) : m_source(source), m_issues(issues) {}

    void Fatal(const std::string& field, const std::string& message)
    {
        Items::Json::AddFileIssue(m_issues, m_source, ItemDataIssue::NoItem, field, message);
    }

    void Warning(const std::string& field, const std::string& message)
    {
        m_issues.push_back(
            {ItemDataIssueSeverity::Warning, m_source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
    }

private:
    const std::string& m_source;
    std::vector<ItemDataIssue>& m_issues;
};

std::string FormatNumber(double value)
{
    std::ostringstream text;
    text << value;
    return text.str();
}

std::string Field(const std::string& object, const std::string& key)
{
    return object.empty() ? key : object + "." + key;
}

bool ReadValue(const LookValueInfo& info, const OrderedJson& json, const std::string& field, LookOverrides& overrides,
               IssueList& issues)
{
    if (!json.is_number() || !std::isfinite(json.get<double>()))
    {
        issues.Fatal(field, "must be a number");
        return false;
    }
    const double value = json.get<double>();
    if (!IsInRange(info, value))
    {
        issues.Fatal(field, "is " + FormatNumber(value) + ", must be from " + FormatNumber(info.min) + " to " +
                                FormatNumber(info.max));
        return false;
    }
    overrides[info.value] = value;
    return true;
}

// Reads every key of an entry: a look value, or one of `allowed` (read by the caller). Every
// problem is reported, not only the first.
bool ReadOverrides(const OrderedJson& json, const std::string& object, std::initializer_list<const char*> allowed,
                   LookOverrides& overrides, IssueList& issues)
{
    bool valid = true;
    for (const auto& [key, value] : json.items())
    {
        if (std::any_of(allowed.begin(), allowed.end(), [&key](const char* name) { return key == name; }))
        {
            continue;
        }
        const std::optional<LookValue> lookValue = FindLookValue(key);
        if (!lookValue)
        {
            issues.Fatal(Field(object, key), "is not a look value");
            valid = false;
            continue;
        }
        valid = ReadValue(Info(*lookValue), value, Field(object, key), overrides, issues) && valid;
    }
    return valid;
}

std::string MapObject(int firstMap)
{
    return std::string(MapsKey) + "[map " + std::to_string(firstMap) + "]";
}

bool CheckRootKeys(const OrderedJson& root, IssueList& issues)
{
    bool valid = true;
    for (const auto& [key, value] : root.items())
    {
        if (key != Items::Json::Keys::FormatVersion && key != DefaultKey && key != MapsKey && key != EnvironmentsKey)
        {
            issues.Fatal(key, "unknown field");
            valid = false;
        }
    }
    return valid;
}

bool ReadDefault(const OrderedJson& root, LookOverrides& overrides, IssueList& issues)
{
    const auto json = root.find(DefaultKey);
    if (json == root.end())
    {
        return true;
    }
    if (!json->is_object())
    {
        issues.Fatal(DefaultKey, "must be an object of look values");
        return false;
    }
    return ReadOverrides(*json, DefaultKey, {}, overrides, issues);
}

bool ReadEnvironments(const OrderedJson& root, LookProfileData& data, IssueList& issues)
{
    const auto json = root.find(EnvironmentsKey);
    if (json == root.end())
    {
        return true;
    }
    if (!json->is_object())
    {
        issues.Fatal(EnvironmentsKey, "must be an object of named environments");
        return false;
    }
    bool valid = true;
    for (const auto& [name, values] : json->items())
    {
        const std::string object = Field(EnvironmentsKey, name);
        if (!values.is_object())
        {
            issues.Fatal(object, "must be an object of look values");
            valid = false;
            continue;
        }
        EnvironmentLookOverrides environment{name, {}};
        valid = ReadOverrides(values, object, {}, environment.overrides, issues) && valid;
        data.environments.push_back(std::move(environment));
    }
    return valid;
}

bool ReadMapNumber(const OrderedJson& json, const std::string& field, int& map, IssueList& issues)
{
    long long number = 0;
    if (!json.is_number_integer() || !Items::Json::ReadWholeNumber(json, number) || number < 0 ||
        number > MaxLookProfileWorld)
    {
        issues.Fatal(field, "missing or not a whole number from 0 to " + std::to_string(MaxLookProfileWorld));
        return false;
    }
    map = static_cast<int>(number);
    return true;
}

// Format 1: "world", one map.
bool ReadWorld(const OrderedJson& json, const std::string& object, std::vector<int>& maps, IssueList& issues)
{
    const auto field = json.find(WorldKey);
    int world = 0;
    if (!ReadMapNumber(field != json.end() ? *field : OrderedJson(), Field(object, WorldKey), world, issues))
    {
        return false;
    }
    maps.push_back(world);
    return true;
}

// Format 2: "maps", a list of maps.
bool ReadMapList(const OrderedJson& json, const std::string& object, std::vector<int>& maps, IssueList& issues)
{
    const auto field = json.find(MapsKey);
    if (field == json.end() || !field->is_array() || field->empty())
    {
        issues.Fatal(Field(object, MapsKey), "missing or not a list of map numbers");
        return false;
    }
    bool valid = true;
    for (size_t index = 0; index < field->size(); ++index)
    {
        int map = 0;
        if (ReadMapNumber((*field)[index], Field(object, MapsKey) + "[" + std::to_string(index) + "]", map, issues))
        {
            maps.push_back(map);
        }
        else
        {
            valid = false;
        }
    }
    return valid;
}

bool ReadMap(const OrderedJson& json, size_t index, int formatVersion, LookProfileData& data, IssueList& issues)
{
    const std::string entry = std::string(MapsKey) + "[" + std::to_string(index) + "]";
    if (!json.is_object())
    {
        issues.Fatal(entry, "must be an object with its maps and values");
        return false;
    }
    MapLookOverrides map;
    const char* mapsKey = formatVersion == 1 ? WorldKey : MapsKey;
    if (!(formatVersion == 1 ? ReadWorld(json, entry, map.maps, issues) : ReadMapList(json, entry, map.maps, issues)))
    {
        return false;
    }
    const std::string object = MapObject(map.maps.front());
    const auto name = json.find(NameKey);
    if (name != json.end() && !name->is_string())
    {
        issues.Fatal(Field(object, NameKey), "must be text");
        return false;
    }
    if (name != json.end())
    {
        map.name = name->get<std::string>();
    }
    if (const auto environment = json.find(EnvironmentKey); environment != json.end())
    {
        const auto known = environment->is_string()
                               ? std::find_if(data.environments.begin(), data.environments.end(),
                                              [&environment](const EnvironmentLookOverrides& entry)
                                              { return entry.name == environment->get<std::string>(); })
                               : data.environments.end();
        if (known == data.environments.end())
        {
            issues.Fatal(Field(object, EnvironmentKey), "must name one of \"environments\"");
            return false;
        }
        map.environment = static_cast<std::size_t>(known - data.environments.begin());
    }
    if (!ReadOverrides(json, object, {mapsKey, NameKey, EnvironmentKey}, map.overrides, issues))
    {
        return false;
    }
    data.maps.push_back(std::move(map));
    return true;
}

bool ReadMaps(const OrderedJson& root, int formatVersion, LookProfileData& data, IssueList& issues)
{
    const auto list = root.find(MapsKey);
    if (list == root.end())
    {
        return true;
    }
    if (!list->is_array())
    {
        issues.Fatal(MapsKey, "must be a list of map entries");
        return false;
    }
    bool valid = true;
    for (size_t index = 0; index < list->size(); ++index)
    {
        valid = ReadMap((*list)[index], index, formatVersion, data, issues) && valid;
    }
    return valid;
}

// Format 1 has its light mood in Data/Maps/LightMood.json; a mood value beside it would override that file's.
bool HasNoMoodValues(const LookProfileData& data, IssueList& list)
{
    bool none = true;
    const auto check = [&none, &list](const LookOverrides& overrides, const std::string& object)
    {
        for (const LookValueInfo& info : Infos)
        {
            if (IsMoodValue(info.value) && overrides[info.value])
            {
                list.Fatal(Field(object, info.name),
                           std::string("is a format 2 value; a format 1 file has its light mood in ") + LightMoodFile);
                none = false;
            }
        }
    };
    check(data.defaultOverrides, DefaultKey);
    for (const EnvironmentLookOverrides& environment : data.environments)
    {
        check(environment.overrides, Field(EnvironmentsKey, environment.name));
    }
    for (const MapLookOverrides& entry : data.maps)
    {
        check(entry.overrides, entry.maps.empty() ? std::string(MapsKey) : MapObject(entry.maps.front()));
    }
    return none;
}

bool CheckFiguresNotBelowBase(const LookValues& values, const std::string& object, IssueList& issues)
{
    const double base = values[LookValue::MoodBase];
    const double figures = values[LookValue::MoodFigures];
    if (figures >= base)
    {
        return true;
    }
    issues.Fatal(Field(object, Info(LookValue::MoodFigures).name),
                 "is " + FormatNumber(figures) + ", must not be below mood.base " + FormatNumber(base));
    return false;
}

std::string FormatValue(double value)
{
    // The shortest text that reads back as the same number: 1, 0.45, 2048.
    std::array<char, 32> buffer{};
    const auto [end, error] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
    return error == std::errc() ? std::string(buffer.data(), end) : FormatNumber(value);
}

std::string Quoted(const std::string& text)
{
    return OrderedJson(text).dump();
}

// `"name": value` for each value the overrides name, in LookValue order.
std::vector<std::string> ValueFields(const LookOverrides& overrides)
{
    std::vector<std::string> fields;
    for (const LookValueInfo& info : Infos)
    {
        if (const std::optional<double>& value = overrides[info.value])
        {
            fields.push_back(Quoted(info.name) + ": " + FormatValue(*value));
        }
    }
    return fields;
}

std::string InlineObject(const std::vector<std::string>& fields)
{
    std::string text = "{";
    for (size_t index = 0; index < fields.size(); ++index)
    {
        text += (index == 0 ? "" : ", ") + fields[index];
    }
    return text + "}";
}

// Lines of a block (`{` or `[` written by the caller), with commas between them.
void AppendLines(std::string& text, const std::vector<std::string>& lines, const char* indent)
{
    for (size_t index = 0; index < lines.size(); ++index)
    {
        text += indent + lines[index] + (index + 1 < lines.size() ? ",\n" : "\n");
    }
}
} // namespace

const std::array<LookValueInfo, LookValueCount>& LookValueInfos()
{
    return Infos;
}

const LookValueInfo& Info(LookValue value)
{
    return Infos[static_cast<std::size_t>(value)];
}

std::optional<LookValue> FindLookValue(std::string_view name)
{
    const auto info =
        std::find_if(Infos.begin(), Infos.end(), [name](const LookValueInfo& entry) { return name == entry.name; });
    if (info == Infos.end())
    {
        return std::nullopt;
    }
    return info->value;
}

void LookOverrides::ApplyTo(LookValues& target) const
{
    for (std::size_t index = 0; index < LookValueCount; ++index)
    {
        if (values[index])
        {
            target.values[index] = *values[index];
        }
    }
}

LookValues BuiltInLookValues()
{
    LookValues builtIn;
    for (const LookValueInfo& info : Infos)
    {
        builtIn[info.value] = info.builtIn;
    }
    return builtIn;
}

bool MapLookOverrides::Has(int world) const
{
    return std::find(maps.begin(), maps.end(), world) != maps.end();
}

const MapLookOverrides* LookProfileData::FindMap(int world) const
{
    const auto map =
        std::find_if(maps.begin(), maps.end(), [world](const MapLookOverrides& entry) { return entry.Has(world); });
    return map != maps.end() ? &*map : nullptr;
}

MapLookOverrides* LookProfileData::FindMap(int world)
{
    return const_cast<MapLookOverrides*>(std::as_const(*this).FindMap(world));
}

LookValues LookProfileData::ForEnvironment(std::size_t environment) const
{
    LookValues values = BuiltInLookValues();
    defaultOverrides.ApplyTo(values);
    if (environment < environments.size())
    {
        environments[environment].overrides.ApplyTo(values);
    }
    return values;
}

LookValues LookProfileData::ForWorld(int world) const
{
    LookValues values = BuiltInLookValues();
    defaultOverrides.ApplyTo(values);
    if (const MapLookOverrides* map = FindMap(world))
    {
        if (map->environment && *map->environment < environments.size())
        {
            environments[*map->environment].overrides.ApplyTo(values);
        }
        map->overrides.ApplyTo(values);
    }
    return values;
}

void ReadLookProfileJson(std::string_view text, const std::string& source, LookProfileData& data,
                         std::vector<ItemDataIssue>& issues, int* formatVersion)
{
    data = {};
    if (formatVersion)
    {
        *formatVersion = 0;
    }
    IssueList list(source, issues);
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, LookProfileFormatVersion, root, issues))
    {
        return;
    }
    const int version = root[Items::Json::Keys::FormatVersion].get<int>();
    if (formatVersion)
    {
        *formatVersion = version;
    }

    LookProfileData read;
    // Non-short-circuit: every problem of the file is reported.
    const bool keys = CheckRootKeys(root, list);
    const bool defaults = ReadDefault(root, read.defaultOverrides, list);
    // Before the maps, which name them.
    const bool environments = ReadEnvironments(root, read, list);
    const bool maps = ReadMaps(root, version, read, list);
    const bool moods = version >= LookProfileFormatVersion || HasNoMoodValues(read, list);
    if (keys && defaults && environments && maps && moods && ValidateLookProfile(read, source, issues))
    {
        data = std::move(read);
    }
}

bool ValidateLookProfile(const LookProfileData& data, const std::string& source, std::vector<ItemDataIssue>& issues)
{
    IssueList list(source, issues);
    // Non-short-circuit: every problem is reported.
    LookValues defaults = BuiltInLookValues();
    data.defaultOverrides.ApplyTo(defaults);
    bool valid = CheckFiguresNotBelowBase(defaults, DefaultKey, list);
    for (std::size_t index = 0; index < data.environments.size(); ++index)
    {
        valid = CheckFiguresNotBelowBase(data.ForEnvironment(index),
                                         Field(EnvironmentsKey, data.environments[index].name), list) &&
                valid;
    }
    // The entry each map was first seen in.
    std::map<int, std::size_t> entryOfMap;
    for (std::size_t index = 0; index < data.maps.size(); ++index)
    {
        const MapLookOverrides& entry = data.maps[index];
        if (entry.maps.empty())
        {
            list.Fatal(std::string(MapsKey) + "[" + std::to_string(index) + "]", "has no maps");
            valid = false;
            continue;
        }
        const std::string object = MapObject(entry.maps.front());
        if (entry.environment && *entry.environment >= data.environments.size())
        {
            list.Fatal(Field(object, EnvironmentKey), "must name one of \"environments\"");
            valid = false;
            continue;
        }
        for (const int map : entry.maps)
        {
            if (map < 0 || map > MaxLookProfileWorld)
            {
                list.Fatal(object, "has map " + std::to_string(map) + ", which is not from 0 to " +
                                       std::to_string(MaxLookProfileWorld));
                valid = false;
                continue;
            }
            const auto [other, added] = entryOfMap.emplace(map, index);
            if (!added)
            {
                list.Fatal(object, other->second == index
                                       ? "lists map " + std::to_string(map) + " twice"
                                       : "has map " + std::to_string(map) + ", which another entry has too");
                valid = false;
            }
        }
        valid = CheckFiguresNotBelowBase(data.ForWorld(entry.maps.front()), object, list) && valid;
    }
    return valid;
}

std::string WriteLookProfileJson(const LookProfileData& data)
{
    std::string text = "{\n  \"" + std::string(Items::Json::Keys::FormatVersion) +
                       "\": " + std::to_string(LookProfileFormatVersion) + ",\n";

    const std::vector<std::string> defaults = ValueFields(data.defaultOverrides);
    text += "  \"" + std::string(DefaultKey) + "\": {" + (defaults.empty() ? "" : "\n");
    AppendLines(text, defaults, "    ");
    text += std::string(defaults.empty() ? "" : "  ") + "},\n";

    std::vector<const EnvironmentLookOverrides*> environments;
    for (const EnvironmentLookOverrides& environment : data.environments)
    {
        environments.push_back(&environment);
    }
    std::sort(environments.begin(), environments.end(),
              [](const auto* left, const auto* right) { return left->name < right->name; });
    std::vector<std::string> environmentLines;
    for (const EnvironmentLookOverrides* environment : environments)
    {
        environmentLines.push_back(Quoted(environment->name) + ": " +
                                   InlineObject(ValueFields(environment->overrides)));
    }
    text += "  \"" + std::string(EnvironmentsKey) + "\": {" + (environmentLines.empty() ? "" : "\n");
    AppendLines(text, environmentLines, "    ");
    text += std::string(environmentLines.empty() ? "" : "  ") + "},\n";

    std::vector<MapLookOverrides> maps = data.maps;
    for (MapLookOverrides& entry : maps)
    {
        std::sort(entry.maps.begin(), entry.maps.end());
    }
    std::sort(maps.begin(), maps.end(),
              [](const MapLookOverrides& left, const MapLookOverrides& right)
              {
                  const int leftFirst = left.maps.empty() ? -1 : left.maps.front();
                  const int rightFirst = right.maps.empty() ? -1 : right.maps.front();
                  return leftFirst < rightFirst;
              });
    std::vector<std::string> mapLines;
    for (const MapLookOverrides& entry : maps)
    {
        std::string list;
        for (const int map : entry.maps)
        {
            list += (list.empty() ? "" : ", ") + std::to_string(map);
        }
        std::vector<std::string> fields{Quoted(MapsKey) + ": [" + list + "]"};
        if (!entry.name.empty())
        {
            fields.push_back(Quoted(NameKey) + ": " + Quoted(entry.name));
        }
        if (entry.environment && *entry.environment < data.environments.size())
        {
            fields.push_back(Quoted(EnvironmentKey) + ": " + Quoted(data.environments[*entry.environment].name));
        }
        for (std::string& field : ValueFields(entry.overrides))
        {
            fields.push_back(std::move(field));
        }
        mapLines.push_back(InlineObject(fields));
    }
    text += "  \"" + std::string(MapsKey) + "\": [" + (mapLines.empty() ? "" : "\n");
    AppendLines(text, mapLines, "    ");
    text += std::string(mapLines.empty() ? "" : "  ") + "]\n}\n";
    return text;
}

LookProfileLoadResult LoadLookProfileFile(const std::filesystem::path& file)
{
    LookProfileLoadResult result;
    const std::string source = file.generic_string();
    // Only a file that is certainly absent selects the built-in values; one whose existence cannot be
    // checked (an inaccessible folder) is read, and the read reports the failure.
    if (Content::Probe(file) == Content::Found::Nothing)
    {
        IssueList(source, result.issues).Warning("", "not found, the built-in values apply");
        return result;
    }
    const std::optional<std::string> text = Items::Json::ReadTextFile(file);
    if (!text)
    {
        IssueList(source, result.issues).Fatal("", "could not be read");
        return result;
    }
    ReadLookProfileJson(*text, source, result.data, result.issues);
    return result;
}
} // namespace Data::Maps
