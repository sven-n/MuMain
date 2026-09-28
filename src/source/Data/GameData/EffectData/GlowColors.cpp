#include "stdafx.h"

#include "GlowColors.h"

#include "Data/GameData/ItemData/ItemJsonCommon.h"

#include <cmath>
#include <set>

namespace Data::Effects
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::Json::OrderedJson;

constexpr const char* ColorsKey = "colors";
constexpr double MaxColorValue = 1.0;
// The keys of the root object, where "colors" is.
constexpr int RootKeyDepth = 1;

// The keys of an object that the parser is in; only those of "colors" are
// color names.
struct ObjectKeys
{
    bool isColorList = false;
    std::set<std::string> keys;
};

// The JSON reader keeps only the last of two equal keys, so a name that is
// in the list twice is looked for in the text.
std::vector<std::string> FindRepeatedColorNames(std::string_view text)
{
    std::vector<std::string> repeated;
    std::vector<ObjectKeys> objects;
    std::string lastRootKey;
    const OrderedJson::parser_callback_t callback =
        [&](int depth, OrderedJson::parse_event_t event, OrderedJson& parsed)
    {
        if (event == OrderedJson::parse_event_t::object_start)
        {
            objects.push_back({depth == RootKeyDepth && lastRootKey == ColorsKey, {}});
        }
        else if (event == OrderedJson::parse_event_t::object_end && !objects.empty())
        {
            objects.pop_back();
        }
        else if (event == OrderedJson::parse_event_t::key)
        {
            const std::string key = parsed.get<std::string>();
            if (depth == RootKeyDepth)
            {
                lastRootKey = key;
            }
            else if (!objects.empty() && objects.back().isColorList && !objects.back().keys.insert(key).second)
            {
                repeated.push_back(key);
            }
        }
        return true;
    };
    static_cast<void>(OrderedJson::parse(text, callback, false));
    return repeated;
}

bool ReadValue(const OrderedJson& json, GlowColorValue& value)
{
    if (!json.is_array() || json.size() != value.size())
    {
        return false;
    }
    for (size_t i = 0; i < value.size(); ++i)
    {
        if (!json[i].is_number() || !std::isfinite(json[i].get<double>()) || json[i].get<double>() < 0.0 ||
            json[i].get<double>() > MaxColorValue)
        {
            return false;
        }
        value[i] = json[i].get<double>();
    }
    return true;
}

void AddError(std::vector<ItemDataIssue>& issues, const std::string& source, const std::string& field,
              const std::string& message)
{
    Items::Json::AddFileIssue(issues, source, ItemDataIssue::NoItem, field, message);
}
} // namespace

void ReadGlowColorsJson(std::string_view text, const std::string& source, std::vector<GlowColor>& colors,
                        std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, GlowColorsFormatVersion, root, issues))
    {
        return;
    }

    const auto list = root.find(ColorsKey);
    if (list == root.end() || !list->is_object())
    {
        AddError(issues, source, ColorsKey, "missing or not an object of names and colors");
        return;
    }
    for (const std::string& name : FindRepeatedColorNames(text))
    {
        AddError(issues, source, std::string(ColorsKey) + "." + name, "is in the list more than once");
    }
    for (const auto& [name, json] : list->items())
    {
        const std::string field = std::string(ColorsKey) + "." + name;
        GlowColorValue value{};
        if (!Items::Json::IsName(name))
        {
            AddError(issues, source, field, "a name has only letters and digits");
        }
        else if (!ReadValue(json, value))
        {
            AddError(issues, source, field, "must be a list of red, green and blue from 0 to 1");
        }
        else
        {
            colors.push_back({name, value});
        }
    }
    for (const auto& [key, value] : root.items())
    {
        if (key != Items::Json::Keys::FormatVersion && key != ColorsKey)
        {
            issues.push_back({ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, key,
                              "unknown field, ignored"});
        }
    }
}

} // namespace Data::Effects
