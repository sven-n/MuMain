#include "stdafx.h"

#include "LightMood.h"

#include "Data/GameData/ItemData/ItemJsonCommon.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <set>
#include <sstream>

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
constexpr double MaxScale = 1.0;
constexpr double MinBackground = 0.0;
constexpr double MinSun = 0.0;

constexpr double MaxShadowKeep = 1.5;
constexpr double MaxShadeCool = 0.5;
constexpr double MinBrightScale = 0.3;

// The default needs every required value; optional ones keep their Classic value.
constexpr std::array<LightMoodValueInfo, LightMoodValueCount> MoodValues = {{
    {"base", &LightMood::base, true, MinLightMoodBase, MaxScale},
    {"figures", &LightMood::figures, true, MinLightMoodBase, MaxScale},
    {"dynamicGain", &LightMood::dynamicGain, true, MinLightMoodDynamicGain, MaxLightMoodDynamicGain},
    {"background", &LightMood::background, true, MinBackground, MaxScale},
    {"sun", &LightMood::sun, true, MinSun, MaxScale},
    {"shadowKeep", &LightMood::shadowKeep, false, 0.0, MaxShadowKeep},
    {"shadeCool", &LightMood::shadeCool, false, 0.0, MaxShadeCool},
    {"brightScale", &LightMood::brightScale, false, MinBrightScale, MaxScale},
}};

class IssueList
{
public:
    IssueList(const std::string& source, std::vector<ItemDataIssue>& issues) : m_source(source), m_issues(issues) {}

    void Error(const std::string& field, const std::string& message)
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

std::string Field(const std::string& object, const char* key)
{
    return object + "." + key;
}

// Reads the values the object has into `mood`; the default needs all of them.
bool ReadMoodValues(const OrderedJson& json, const std::string& object, bool needsAll, LightMood& mood,
                    IssueList& issues)
{
    bool valid = true;
    for (const LightMoodValueInfo& value : MoodValues)
    {
        const auto field = json.find(value.key);
        if (field == json.end())
        {
            if (needsAll && value.required)
            {
                issues.Error(Field(object, value.key), "missing");
                valid = false;
            }
            continue;
        }
        if (!field->is_number() || !std::isfinite(field->get<double>()))
        {
            issues.Error(Field(object, value.key), "must be a number");
            valid = false;
            continue;
        }
        mood.*value.member = field->get<double>();
    }
    return valid;
}

bool CheckRange(double value, double min, double max, const std::string& field, IssueList& issues)
{
    if (value >= min && value <= max)
    {
        return true;
    }
    issues.Error(field,
                 "is " + FormatNumber(value) + ", must be from " + FormatNumber(min) + " to " + FormatNumber(max));
    return false;
}

bool CheckRanges(const LightMood& mood, const std::string& object, IssueList& issues)
{
    // Non-short-circuit: every value out of range is reported.
    bool valid = true;
    for (const LightMoodValueInfo& value : MoodValues)
    {
        if (value.member == &LightMood::figures)
        {
            continue;
        }
        valid = CheckRange(mood.*value.member, value.min, value.max, Field(object, value.key), issues) && valid;
    }
    // Figures are never darker than the base: the rule names both values.
    if (!(mood.figures >= mood.base && mood.figures <= MaxScale))
    {
        issues.Error(Field(object, "figures"), "is " + FormatNumber(mood.figures) + ", must be from " +
                                                   FormatNumber(mood.base) + " to " + FormatNumber(MaxScale) +
                                                   " (not below base " + FormatNumber(mood.base) + ")");
        valid = false;
    }
    return valid;
}

bool IsMoodKey(const std::string& key)
{
    return std::any_of(MoodValues.begin(), MoodValues.end(),
                       [&key](const LightMoodValueInfo& value) { return key == value.key; });
}

// A mood object (the default, a map entry) may also have the mood values.
void WarnUnknownKeys(const OrderedJson& json, const std::string& object, bool isMood,
                     std::initializer_list<const char*> known, IssueList& issues)
{
    for (const auto& [key, value] : json.items())
    {
        const bool isKnown = (isMood && IsMoodKey(key)) ||
                             std::any_of(known.begin(), known.end(), [&key](const char* name) { return key == name; });
        if (!isKnown)
        {
            issues.Warning(object.empty() ? key : Field(object, key.c_str()), "unknown field, ignored");
        }
    }
}

bool ReadDefault(const OrderedJson& root, LightMood& mood, IssueList& issues)
{
    const auto json = root.find(DefaultKey);
    if (json == root.end() || !json->is_object())
    {
        issues.Error(DefaultKey, "missing or not an object of base, figures, dynamicGain, background and sun");
        return false;
    }
    WarnUnknownKeys(*json, DefaultKey, true, {}, issues);
    return ReadMoodValues(*json, DefaultKey, true, mood, issues) && CheckRanges(mood, DefaultKey, issues);
}

bool ReadWorld(const OrderedJson& json, const std::string& object, int& world, IssueList& issues)
{
    const auto field = json.find(WorldKey);
    long long number = 0;
    if (field == json.end() || !field->is_number_integer() || !Items::Json::ReadWholeNumber(*field, number) ||
        number < 0 || number > MaxLightMoodWorld)
    {
        issues.Error(Field(object, WorldKey),
                     "missing or not a whole number from 0 to " + std::to_string(MaxLightMoodWorld));
        return false;
    }
    world = static_cast<int>(number);
    return true;
}

bool ReadMap(const OrderedJson& json, size_t index, const LightMood& defaultMood, std::set<int>& worlds,
             LightMoodData& data, IssueList& issues)
{
    const std::string entry = std::string(MapsKey) + "[" + std::to_string(index) + "]";
    if (!json.is_object())
    {
        issues.Error(entry, "must be an object with a world and its values");
        return false;
    }
    MapLightMood map{0, defaultMood};
    if (!ReadWorld(json, entry, map.world, issues))
    {
        return false;
    }
    const std::string object = std::string(MapsKey) + "[world " + std::to_string(map.world) + "]";
    if (!worlds.insert(map.world).second)
    {
        issues.Error(object, "is in the list more than once");
        return false;
    }
    WarnUnknownKeys(json, object, true, {WorldKey, NameKey}, issues);
    if (!ReadMoodValues(json, object, false, map.mood, issues) || !CheckRanges(map.mood, object, issues))
    {
        return false;
    }
    data.maps.push_back(map);
    return true;
}

bool ReadMaps(const OrderedJson& root, LightMoodData& data, IssueList& issues)
{
    const auto list = root.find(MapsKey);
    if (list == root.end())
    {
        return true;
    }
    if (!list->is_array())
    {
        issues.Error(MapsKey, "must be a list of map entries");
        return false;
    }
    bool valid = true;
    std::set<int> worlds;
    for (size_t index = 0; index < list->size(); ++index)
    {
        valid = ReadMap((*list)[index], index, data.defaultMood, worlds, data, issues) && valid;
    }
    return valid;
}
} // namespace

const std::array<LightMoodValueInfo, LightMoodValueCount>& LightMoodValueInfos()
{
    return MoodValues;
}

const LightMoodValueInfo* FindLightMoodValue(std::string_view key)
{
    const auto value = std::find_if(MoodValues.begin(), MoodValues.end(),
                                    [key](const LightMoodValueInfo& entry) { return key == entry.key; });
    return value != MoodValues.end() ? &*value : nullptr;
}

const LightMood& LightMoodData::ForWorld(int world) const
{
    const auto map =
        std::find_if(maps.begin(), maps.end(), [world](const MapLightMood& entry) { return entry.world == world; });
    return map != maps.end() ? map->mood : defaultMood;
}

void ReadLightMoodJson(std::string_view text, const std::string& source, LightMoodData& data,
                       std::vector<ItemDataIssue>& issues)
{
    data = {};
    IssueList list(source, issues);
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, LightMoodFormatVersion, root, issues))
    {
        return;
    }
    WarnUnknownKeys(root, "", false, {Items::Json::Keys::FormatVersion, DefaultKey, MapsKey}, list);

    LightMoodData read;
    const bool valid = ReadDefault(root, read.defaultMood, list) && ReadMaps(root, read, list);
    if (valid)
    {
        data = std::move(read);
    }
}

LightMoodLoadResult LoadLightMoodFile(const std::filesystem::path& file)
{
    LightMoodLoadResult result;
    const std::string source = file.generic_string();
    const std::optional<std::string> text = Items::Json::ReadTextFile(file);
    if (!text)
    {
        IssueList(source, result.issues).Error("", "could not be read");
        return result;
    }
    ReadLightMoodJson(*text, source, result.data, result.issues);
    return result;
}
} // namespace Data::Maps
