#include "stdafx.h"

#include "MonsterJsonCommon.h"

#include "Data/GameData/Common/DataPaths.h"
#include "Data/GameData/MonsterData/MonsterAnimations.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <set>

namespace Data::Monsters::Json
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;

EntryIssues::EntryIssues(const std::string& source, std::string entry, std::vector<ItemDataIssue>& issues)
    : m_source(source), m_entry(std::move(entry)), m_issues(issues)
{
}

void EntryIssues::Fatal(const std::string& key, const std::string& message)
{
    Add(ItemDataIssueSeverity::Fatal, key, message);
    m_valid = false;
}

void EntryIssues::Warning(const std::string& key, const std::string& message)
{
    Add(ItemDataIssueSeverity::Warning, key, message);
}

void EntryIssues::WarnAboutUnknownKeys(const OrderedJson& json, const KeySet& known, const std::string& prefix)
{
    Items::Json::WarnAboutUnknownKeys(json, known, prefix, [&](const std::string& unknown, const std::string& message)
                                      { Warning(unknown, message); });
}

void EntryIssues::Add(ItemDataIssueSeverity severity, const std::string& key, const std::string& message)
{
    m_issues.push_back(
        {severity, m_source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, m_entry + " " + key, message});
}

std::optional<long long> ReadWhole(const OrderedJson& json)
{
    long long value = 0;
    if (!json.is_number_integer() || !Items::Json::ReadWholeNumber(json, value))
    {
        return std::nullopt;
    }
    return value;
}

std::optional<int> ReadWholeIn(const OrderedJson& json, long long low, long long high)
{
    const std::optional<long long> value = ReadWhole(json);
    if (!value || *value < low || *value > high)
    {
        return std::nullopt;
    }
    return static_cast<int>(*value);
}

std::string RangeText(long long low, long long high)
{
    return "must be a whole number from " + std::to_string(low) + " to " + std::to_string(high);
}

bool ReadWholeList(const OrderedJson& json, const std::string& key, long long low, long long high,
                   std::vector<int>& values, EntryIssues& issues, bool repeats)
{
    if (!json.is_array())
    {
        issues.Fatal(key, "must be a list of whole numbers");
        return false;
    }
    std::set<int> seen;
    for (size_t i = 0; i < json.size(); ++i)
    {
        const std::optional<int> value = ReadWholeIn(json[i], low, high);
        if (!value)
        {
            issues.Fatal(key + "[" + std::to_string(i) + "]", RangeText(low, high));
            return false;
        }
        if (!seen.insert(*value).second && !repeats)
        {
            issues.Fatal(key, "has " + std::to_string(*value) + " more than once");
            return false;
        }
        values.push_back(*value);
    }
    return true;
}

bool ReadText(const OrderedJson& json, const std::string& key, std::string& text, EntryIssues& issues)
{
    if (!json.is_string() || json.get_ref<const std::string&>().empty())
    {
        issues.Fatal(key, "must be a text");
        return false;
    }
    text = json.get<std::string>();
    return true;
}

bool ReadPath(const OrderedJson& json, const std::string& key, std::string& path, EntryIssues& issues)
{
    if (!ReadText(json, key, path, issues))
    {
        return false;
    }
    const std::string problem = Paths::FindPathProblem(path);
    if (!problem.empty())
    {
        issues.Fatal(key, problem);
        return false;
    }
    return true;
}

bool ReadAnimationList(const OrderedJson& json, const std::string& key, std::vector<int>& animations,
                       EntryIssues& issues)
{
    if (!json.is_array())
    {
        issues.Fatal(key, "must be a list of animations");
        return false;
    }
    for (size_t i = 0; i < json.size(); ++i)
    {
        const std::optional<int> animation =
            json[i].is_string() ? FindMonsterAnimation(json[i].get<std::string>()) : std::nullopt;
        if (!animation)
        {
            issues.Fatal(key + "[" + std::to_string(i) + "]", "is not an animation");
            return false;
        }
        if (std::find(animations.begin(), animations.end(), *animation) != animations.end())
        {
            issues.Fatal(key, "has " + GetMonsterAnimationName(*animation) + " more than once");
            return false;
        }
        animations.push_back(*animation);
    }
    std::sort(animations.begin(), animations.end());
    return true;
}

bool ReadMaps(const OrderedJson& json, const std::string& key, std::vector<Data::Maps::MapRef>& maps,
              EntryIssues& issues)
{
    return Data::Maps::ReadMapRefs(json, key, maps, [&](const std::string& field, const std::string& message)
                                   { issues.Fatal(field, message); });
}

bool ReadFrames(const OrderedJson& json, const std::string& key, std::pair<float, float>& frames, EntryIssues& issues)
{
    const auto isFrame = [](const OrderedJson& value)
    { return value.is_number() && std::isfinite(value.get<double>()) && value.get<double>() >= 0.0; };
    if (!json.is_array() || json.size() != 2 || !isFrame(json[0]) || !isFrame(json[1]) ||
        json[0].get<double>() >= json[1].get<double>())
    {
        issues.Fatal(key, "must be two frames, the first below the second: [7, 8]");
        return false;
    }
    frames = {static_cast<float>(json[0].get<double>()), static_cast<float>(json[1].get<double>())};
    return true;
}

// Found digit by digit: std::to_chars of a float, which does it at once, is
// missing before macOS 13.3.
OrderedJson WriteFloat(float value)
{
    if (value == std::floor(value) && std::abs(value) < static_cast<float>(1 << 24))
    {
        return static_cast<long long>(value);
    }
    std::array<char, 32> buffer{};
    for (int digits = 1; digits <= std::numeric_limits<float>::max_digits10; ++digits)
    {
        std::snprintf(buffer.data(), buffer.size(), "%.*g", digits, static_cast<double>(value));
        if (std::strtof(buffer.data(), nullptr) == value)
        {
            break;
        }
    }
    return std::strtod(buffer.data(), nullptr);
}

OrderedJson WriteAnimationList(const std::vector<int>& animations)
{
    OrderedJson json = OrderedJson::array();
    for (const int animation : animations)
    {
        json.push_back(GetMonsterAnimationName(animation));
    }
    return json;
}

OrderedJson WriteFrames(const std::pair<float, float>& frames)
{
    return OrderedJson::array({WriteFloat(frames.first), WriteFloat(frames.second)});
}
} // namespace Data::Monsters::Json
