#include "stdafx.h"

#include "LookProfile.h"

#include "Data/GameData/ItemData/ItemJsonCommon.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <sstream>
#include <system_error>

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
    {LookValue::BloomStrength, "bloom.strength", 0.6, 0.0, 2.0},
    {LookValue::BloomLevelFalloff, "bloom.levelFalloff", 0.5, 0.25, 1.0},
    {LookValue::GradingStrength, "grading.strength", 1.0, 0.0, 1.0},
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

bool CheckRootKeys(const OrderedJson& root, IssueList& issues)
{
    bool valid = true;
    for (const auto& [key, value] : root.items())
    {
        if (key != Items::Json::Keys::FormatVersion && key != DefaultKey && key != MapsKey)
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

bool ReadWorld(const OrderedJson& json, const std::string& object, int& world, IssueList& issues)
{
    const auto field = json.find(WorldKey);
    long long number = 0;
    if (field == json.end() || !field->is_number_integer() || !Items::Json::ReadWholeNumber(*field, number) ||
        number < 0 || number > MaxLookProfileWorld)
    {
        issues.Fatal(Field(object, WorldKey),
                     "missing or not a whole number from 0 to " + std::to_string(MaxLookProfileWorld));
        return false;
    }
    world = static_cast<int>(number);
    return true;
}

bool ReadMap(const OrderedJson& json, size_t index, std::set<int>& worlds, LookProfileData& data, IssueList& issues)
{
    const std::string entry = std::string(MapsKey) + "[" + std::to_string(index) + "]";
    if (!json.is_object())
    {
        issues.Fatal(entry, "must be an object with a world and its values");
        return false;
    }
    MapLookOverrides map;
    if (!ReadWorld(json, entry, map.world, issues))
    {
        return false;
    }
    const std::string object = std::string(MapsKey) + "[world " + std::to_string(map.world) + "]";
    if (!worlds.insert(map.world).second)
    {
        issues.Fatal(object, "is in the list more than once");
        return false;
    }
    const auto name = json.find(NameKey);
    if (name != json.end() && !name->is_string())
    {
        issues.Fatal(Field(object, NameKey), "must be text");
        return false;
    }
    if (!ReadOverrides(json, object, {WorldKey, NameKey}, map.overrides, issues))
    {
        return false;
    }
    data.maps.push_back(map);
    return true;
}

bool ReadMaps(const OrderedJson& root, LookProfileData& data, IssueList& issues)
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
    std::set<int> worlds;
    for (size_t index = 0; index < list->size(); ++index)
    {
        valid = ReadMap((*list)[index], index, worlds, data, issues) && valid;
    }
    return valid;
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

LookValues LookProfileData::ForWorld(int world) const
{
    LookValues values = BuiltInLookValues();
    defaultOverrides.ApplyTo(values);
    const auto map =
        std::find_if(maps.begin(), maps.end(), [world](const MapLookOverrides& entry) { return entry.world == world; });
    if (map != maps.end())
    {
        map->overrides.ApplyTo(values);
    }
    return values;
}

void ReadLookProfileJson(std::string_view text, const std::string& source, LookProfileData& data,
                         std::vector<ItemDataIssue>& issues)
{
    data = {};
    IssueList list(source, issues);
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, LookProfileFormatVersion, root, issues))
    {
        return;
    }

    LookProfileData read;
    // Non-short-circuit: every problem of the file is reported.
    const bool keys = CheckRootKeys(root, list);
    const bool defaults = ReadDefault(root, read.defaultOverrides, list);
    const bool maps = ReadMaps(root, read, list);
    if (keys && defaults && maps)
    {
        data = std::move(read);
    }
}

LookProfileLoadResult LoadLookProfileFile(const std::filesystem::path& file)
{
    LookProfileLoadResult result;
    const std::string source = file.generic_string();
    // Only a file that is certainly absent selects the built-in values; one whose existence cannot be
    // checked (an inaccessible folder) is read, and the read reports the failure.
    std::error_code error;
    if (!std::filesystem::exists(file, error) && !error)
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
