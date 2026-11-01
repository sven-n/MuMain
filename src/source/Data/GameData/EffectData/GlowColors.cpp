#include "stdafx.h"

#include "GlowColors.h"

#include "Data/GameData/ColorFolders/ColorFolders.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemModelJsonValues.h"

#include <algorithm>
#include <cmath>

namespace Data::Effects
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::Json::OrderedJson;

constexpr const char* ColorsKey = "colors";
constexpr double MaxColorValue = 1.0;

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

void AddFatal(std::vector<ItemDataIssue>& issues, const std::string& source, const std::string& field,
              const std::string& message)
{
    Items::Json::AddFileIssue(issues, source, ItemDataIssue::NoItem, field, message);
}
} // namespace

void ReadGlowColorsJson(std::string_view text, const std::string& source, std::vector<GlowColor>& colors,
                        std::vector<ItemDataIssue>& issues)
{
    std::vector<Colors::ColorFolder> folders;
    ReadGlowColorsJson(text, source, colors, folders, issues);
}

void ReadGlowColorsJson(std::string_view text, const std::string& source, std::vector<GlowColor>& colors,
                        std::vector<Colors::ColorFolder>& folders, std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, GlowColorsFormatVersion, root, issues))
    {
        return;
    }

    const auto list = root.find(ColorsKey);
    if (list == root.end() || !list->is_object())
    {
        AddFatal(issues, source, ColorsKey, "missing or not an object of names and colors");
        return;
    }
    for (const std::string& name : Items::Json::FindRepeatedKeys(text, ColorsKey))
    {
        AddFatal(issues, source, std::string(ColorsKey) + "." + name, "is in the list more than once");
    }
    for (const auto& [name, json] : list->items())
    {
        const std::string field = std::string(ColorsKey) + "." + name;
        GlowColorValue value{};
        if (!Items::Json::IsName(name))
        {
            AddFatal(issues, source, field, "a name has only letters and digits");
        }
        else if (!ReadValue(json, value))
        {
            AddFatal(issues, source, field, "must be a list of red, green and blue from 0 to 1");
        }
        else
        {
            colors.push_back({name, value});
        }
    }
    folders = Colors::ReadColorFolders(
        root, source,
        [&colors](std::string_view name)
        {
            return std::any_of(colors.begin(), colors.end(),
                               [name](const GlowColor& color) { return color.name == name; });
        },
        issues);
    for (const auto& [key, value] : root.items())
    {
        if (key != Items::Json::Keys::FormatVersion && key != ColorsKey && key != Colors::ColorFoldersKey)
        {
            issues.push_back({ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, key,
                              "unknown field, ignored"});
        }
    }
}

std::string WriteGlowColorsJson(std::span<const GlowColor> colors, std::span<const Colors::ColorFolder> folders)
{
    OrderedJson list = OrderedJson::object();
    for (const GlowColor& color : colors)
    {
        list[color.name] = Items::ModelJson::WriteNumbers(color.value);
    }
    OrderedJson root;
    root[Items::Json::Keys::FormatVersion] = GlowColorsFormatVersion;
    if (!folders.empty())
    {
        root[Colors::ColorFoldersKey] = Colors::WriteColorFolders(
            folders,
            [colors](std::string_view name)
            {
                return std::any_of(colors.begin(), colors.end(),
                                   [name](const GlowColor& color) { return color.name == name; });
            });
    }
    root[ColorsKey] = std::move(list);
    std::string text = root.dump(Items::Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    text = Colors::PutColorFoldersOnLines(text);
    for (const GlowColor& color : colors)
    {
        text = Items::Json::PutListsOnOneLine(text, color.name);
    }
    return text + "\n";
}

} // namespace Data::Effects
