#include "stdafx.h"

#include "ItemGroundLabels.h"

#include "Data/GameData/ColorFolders/ColorFolders.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemTooltipConditionsJson.h"

#include <algorithm>
#include <cstdio>

namespace Data::Items
{
namespace
{
using Json::OrderedJson;

constexpr const char* StylesKey = "styles";
constexpr const char* StandardKey = "standard";
constexpr const char* ColorKey = "color";
constexpr const char* BackgroundKey = "background";
constexpr const char* BoldKey = "bold";
constexpr const char* WhenKey = "when";
constexpr const char* StyleKey = "style";
constexpr GroundLabelColor Black{0, 0, 0};

void AddFatal(std::vector<ItemDataIssue>& issues, const std::string& source, const std::string& field,
              const std::string& message)
{
    Json::AddFileIssue(issues, source, ItemDataIssue::NoItem, field, message);
}

void AddWarning(std::vector<ItemDataIssue>& issues, const std::string& source, const std::string& field,
                const std::string& message)
{
    issues.push_back(
        {ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
}

int HexDigit(char character)
{
    if (character >= '0' && character <= '9')
    {
        return character - '0';
    }
    if (character >= 'a' && character <= 'f')
    {
        return character - 'a' + 10;
    }
    if (character >= 'A' && character <= 'F')
    {
        return character - 'A' + 10;
    }
    return -1;
}

void ReadStyle(const std::string& name, const OrderedJson& json, const std::string& source, GroundLabelFile& file,
               std::vector<ItemDataIssue>& issues)
{
    const std::string field = std::string(StylesKey) + "." + name;
    if (!Json::IsName(name))
    {
        AddFatal(issues, source, field, "a name has only letters and digits");
        return;
    }
    if (!json.is_object() || !json.contains(ColorKey))
    {
        AddFatal(issues, source, field,
                 R"(must be an object with a color: {"color": "#rrggbb"}, and "background" and "bold" when needed)");
        return;
    }
    GroundLabelStyle style;
    style.name = name;
    bool read = true;
    const auto readColor = [&](const char* key, GroundLabelColor& color)
    {
        const auto found = json.find(key);
        if (found == json.end())
        {
            return;
        }
        if (!found->is_string() || !ReadGroundLabelColor(found->get<std::string>(), color))
        {
            AddFatal(issues, source, field + "." + key, "must be a color \"#rrggbb\" (red, green, blue from 00 to ff)");
            read = false;
        }
    };
    readColor(ColorKey, style.color);
    readColor(BackgroundKey, style.background);
    if (const auto bold = json.find(BoldKey); bold != json.end())
    {
        if (bold->is_boolean())
        {
            style.bold = bold->get<bool>();
        }
        else
        {
            AddFatal(issues, source, field + "." + BoldKey, "must be true or false");
            read = false;
        }
    }
    Json::WarnAboutUnknownKeys(json, std::set<std::string, std::less<>>{ColorKey, BackgroundKey, BoldKey}, field + ".",
                               [&](const std::string& unknown, const std::string& message)
                               { AddWarning(issues, source, unknown, message); });
    if (read)
    {
        file.styles.push_back(std::move(style));
    }
}

void ReadRules(const OrderedJson& json, const std::string& source, GroundLabelFile& file,
               std::vector<ItemDataIssue>& issues)
{
    for (size_t index = 0; index < json.size(); ++index)
    {
        const OrderedJson& entry = json[index];
        const std::string field = std::string(StandardKey) + "[" + std::to_string(index) + "]";
        const auto style = entry.is_object() ? entry.find(StyleKey) : entry.end();
        if (style == entry.end() || !style->is_string() || !Json::IsName(style->get<std::string>()))
        {
            AddFatal(issues, source, field,
                     R"(must be {"when": [...], "style": "<name>"}, the style a name of letters and digits)");
            continue;
        }
        GroundLabelRule rule;
        rule.style = style->get<std::string>();
        bool read = true;
        if (const auto when = entry.find(WhenKey); when != entry.end())
        {
            ReadTooltipConditions(*when, field + "." + WhenKey, R"(["excellentFlags", "!upperTierWings"])", rule.when,
                                  [&](const std::string& errorField, const std::string& message)
                                  {
                                      AddFatal(issues, source, errorField, message);
                                      read = false;
                                  });
        }
        Json::WarnAboutUnknownKeys(entry, std::set<std::string, std::less<>>{WhenKey, StyleKey}, field + ".",
                                   [&](const std::string& unknown, const std::string& message)
                                   { AddWarning(issues, source, unknown, message); });
        // A rule whose conditions can't be read has its error and is left out: it holds nowhere.
        if (!read)
        {
            continue;
        }
        if (rule.when.empty() && index + 1 < json.size())
        {
            AddWarning(issues, source, field,
                       "has no conditions, so it always holds and the rules after it are never used");
        }
        file.standard.push_back(std::move(rule));
    }
}
} // namespace

const GroundLabelStyle* GroundLabelFile::FindStyle(std::string_view name) const
{
    const auto found =
        std::find_if(styles.begin(), styles.end(), [&](const GroundLabelStyle& style) { return style.name == name; });
    return found != styles.end() ? &*found : nullptr;
}

bool GroundLabelFile::IsStandardRuleStyle(std::string_view name) const
{
    return std::any_of(standard.begin(), standard.end(),
                       [&](const GroundLabelRule& rule) { return rule.style == name; });
}

std::string WriteGroundLabelColor(const GroundLabelColor& color)
{
    char text[8];
    std::snprintf(text, sizeof(text), "#%02x%02x%02x", color[0], color[1], color[2]);
    return text;
}

bool ReadGroundLabelColor(std::string_view text, GroundLabelColor& color)
{
    if (text.size() != 7 || text[0] != '#')
    {
        return false;
    }
    GroundLabelColor read{};
    for (size_t channel = 0; channel < read.size(); ++channel)
    {
        const int high = HexDigit(text[1 + channel * 2]);
        const int low = HexDigit(text[2 + channel * 2]);
        if (high < 0 || low < 0)
        {
            return false;
        }
        read[channel] = static_cast<BYTE>(high * 16 + low);
    }
    color = read;
    return true;
}

void ReadGroundLabelsJson(std::string_view text, const std::string& source, GroundLabelFile& file,
                          std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Json::ReadFileVersion(text, source, GroundLabelsFormatVersion, root, issues))
    {
        return;
    }

    const auto styles = root.find(StylesKey);
    if (styles == root.end() || !styles->is_object())
    {
        AddFatal(issues, source, StylesKey, "missing or not an object of style names and their looks");
    }
    else
    {
        for (const std::string& name : Json::FindRepeatedKeys(text, StylesKey))
        {
            AddFatal(issues, source, std::string(StylesKey) + "." + name, "is in the list more than once");
        }
        for (const auto& [name, json] : styles->items())
        {
            ReadStyle(name, json, source, file, issues);
        }
    }

    const auto standard = root.find(StandardKey);
    if (standard == root.end() || !standard->is_array())
    {
        AddFatal(
            issues, source, StandardKey,
            R"(missing or not a list of rules {"when": [...], "style": "<name>"}, the first that holds gives the style)");
    }
    else
    {
        ReadRules(*standard, source, file, issues);
    }

    file.folders = Colors::ReadColorFolders(
        root, source, [&file](std::string_view name) { return file.FindStyle(name) != nullptr; }, issues);

    for (const auto& [key, value] : root.items())
    {
        if (key != Json::Keys::FormatVersion && key != StylesKey && key != StandardKey &&
            key != Colors::ColorFoldersKey)
        {
            AddWarning(issues, source, key, "unknown field, ignored");
        }
    }
}

std::string WriteGroundLabelsJson(const GroundLabelFile& file)
{
    OrderedJson styles = OrderedJson::object();
    for (const GroundLabelStyle& style : file.styles)
    {
        OrderedJson entry;
        entry[ColorKey] = WriteGroundLabelColor(style.color);
        if (style.background != Black)
        {
            entry[BackgroundKey] = WriteGroundLabelColor(style.background);
        }
        if (style.bold)
        {
            entry[BoldKey] = true;
        }
        styles[style.name] = std::move(entry);
    }
    OrderedJson standard = OrderedJson::array();
    for (const GroundLabelRule& rule : file.standard)
    {
        OrderedJson entry;
        if (!rule.when.empty())
        {
            OrderedJson when = OrderedJson::array();
            for (const TooltipCondition& condition : rule.when)
            {
                when.push_back(WriteTooltipCondition(condition));
            }
            entry[WhenKey] = std::move(when);
        }
        entry[StyleKey] = rule.style;
        standard.push_back(std::move(entry));
    }
    OrderedJson root;
    root[Json::Keys::FormatVersion] = GroundLabelsFormatVersion;
    if (!file.folders.empty())
    {
        root[Colors::ColorFoldersKey] = Colors::WriteColorFolders(file.folders, [&file](std::string_view name)
                                                                  { return file.FindStyle(name) != nullptr; });
    }
    root[StylesKey] = std::move(styles);
    root[StandardKey] = std::move(standard);
    std::string text = root.dump(Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    text = Colors::PutColorFoldersOnLines(text);
    for (const GroundLabelStyle& style : file.styles)
    {
        text = Json::PutObjectsOnOneLine(text, style.name);
    }
    return Json::PutListEntriesOnOneLine(text, StandardKey) + "\n";
}
} // namespace Data::Items
