#include "stdafx.h"

#include "GlowColors.h"

#include "Data/GameData/ItemData/ItemJsonCommon.h"

#include <algorithm>
#include <cctype>
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

bool IsValidName(const std::string& name)
{
    return !name.empty() &&
           std::all_of(name.begin(), name.end(), [](unsigned char character) { return std::isalnum(character) != 0; });
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
    for (const auto& [name, json] : list->items())
    {
        const std::string field = std::string(ColorsKey) + "." + name;
        GlowColorValue value{};
        if (!IsValidName(name))
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

GlowColorList& GlowColorList::GetInstance()
{
    static GlowColorList instance;
    return instance;
}

void GlowColorList::Build(std::span<const GlowColor> colors)
{
    m_colors.assign(colors.begin(), colors.end());
    ++m_version;
}

const GlowColorValue* GlowColorList::Find(std::string_view name) const
{
    const auto found =
        std::find_if(m_colors.begin(), m_colors.end(), [name](const GlowColor& color) { return color.name == name; });
    return found != m_colors.end() ? &found->value : nullptr;
}
} // namespace Data::Effects
