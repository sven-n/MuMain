#include "stdafx.h"

#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"

#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemStatProfiles.h"

#include <algorithm>
#include <array>

namespace Data::Items
{
namespace
{
constexpr std::array<std::string_view, 1> BuiltIns = {LevelPlaceholder};

// The placeholder of `text` that starts at the "{" at `open`: its name and the position after its "}", or the problem.
struct Placeholder
{
    std::string_view name;
    size_t end = std::string_view::npos;
    std::string error;
};

Placeholder ReadPlaceholder(std::string_view text, size_t open)
{
    const size_t close = text.find_first_of("{}", open + 1);
    if (close == std::string_view::npos || text[close] == '{')
    {
        return {{}, std::string_view::npos, "has a \"{\" without its \"}\""};
    }
    const std::string_view name = text.substr(open + 1, close - open - 1);
    if (name.empty())
    {
        return {{}, std::string_view::npos, "has an empty placeholder \"{}\""};
    }
    const std::string_view table = GetPlaceholderTable(name);
    if (!Json::IsName(table.empty() ? name : table))
    {
        return {{},
                std::string_view::npos,
                "has \"{" + std::string(name) +
                    "}\": a placeholder is a name of letters and digits, or \"table:\" and a table's name"};
    }
    return {name, close + 1, {}};
}

size_t EditDistance(std::string_view first, std::string_view second)
{
    std::vector<size_t> previous(second.size() + 1);
    std::vector<size_t> current(second.size() + 1);
    for (size_t column = 0; column <= second.size(); ++column)
    {
        previous[column] = column;
    }
    for (size_t row = 1; row <= first.size(); ++row)
    {
        current[0] = row;
        for (size_t column = 1; column <= second.size(); ++column)
        {
            const size_t replace = previous[column - 1] + (first[row - 1] == second[column - 1] ? 0 : 1);
            current[column] = std::min({previous[column] + 1, current[column - 1] + 1, replace});
        }
        std::swap(previous, current);
    }
    return previous[second.size()];
}
} // namespace

std::span<const std::string_view> GetBuiltInPlaceholders()
{
    return BuiltIns;
}

std::string MakeTablePlaceholder(std::string_view table)
{
    return std::string(TablePlaceholderPrefix) + std::string(table);
}

std::string_view GetPlaceholderTable(std::string_view name)
{
    return name.starts_with(TablePlaceholderPrefix) ? name.substr(TablePlaceholderPrefix.size()) : std::string_view{};
}

PlaceholderScan ScanPlaceholders(std::string_view text)
{
    PlaceholderScan scan;
    size_t position = 0;
    while (position < text.size())
    {
        const size_t brace = text.find_first_of("{}", position);
        if (brace == std::string_view::npos)
        {
            break;
        }
        if (text[brace] == '}')
        {
            scan.error = "has a \"}\" without its \"{\"";
            return scan;
        }
        const Placeholder placeholder = ReadPlaceholder(text, brace);
        if (!placeholder.error.empty())
        {
            scan.error = placeholder.error;
            return scan;
        }
        scan.names.emplace_back(placeholder.name);
        position = placeholder.end;
    }
    return scan;
}

std::optional<int> GetPlaceholderValue(std::string_view name, int level, const ItemStatProfiles& profiles)
{
    if (name == LevelPlaceholder)
    {
        return level;
    }
    const std::string_view tableName = GetPlaceholderTable(name);
    if (const LevelBonusTable* table = tableName.empty() ? nullptr : profiles.FindTable(tableName))
    {
        return table->GetBonus(level);
    }
    return std::nullopt;
}

std::string FillPlaceholders(std::string_view text, int level, const ItemStatProfiles& profiles)
{
    std::string filled;
    filled.reserve(text.size());
    size_t position = 0;
    while (position < text.size())
    {
        const size_t brace = text.find_first_of("{}", position);
        if (brace == std::string_view::npos)
        {
            break;
        }
        filled.append(text.substr(position, brace - position));
        const Placeholder placeholder = text[brace] == '{' ? ReadPlaceholder(text, brace) : Placeholder{};
        const std::optional<int> value = placeholder.error.empty() && placeholder.end != std::string_view::npos
                                             ? GetPlaceholderValue(placeholder.name, level, profiles)
                                             : std::nullopt;
        if (!value)
        {
            position = brace;
            break;
        }
        filled.append(std::to_string(*value));
        position = placeholder.end;
    }
    filled.append(text.substr(std::min(position, text.size())));
    return filled;
}

std::string SuggestPlaceholder(std::string_view name, std::span<const std::string> known)
{
    constexpr size_t MaxTypo = 2;
    std::string nearest;
    size_t nearestDistance = MaxTypo + 1;
    for (const std::string& candidate : known)
    {
        const size_t distance = EditDistance(name, candidate);
        if (distance < nearestDistance)
        {
            nearest = candidate;
            nearestDistance = distance;
        }
    }
    return nearest;
}

bool NamesTable(std::string_view text, std::string_view table)
{
    const std::vector<std::string> names = ScanPlaceholders(text).names;
    return std::ranges::any_of(names, [&](const std::string& name) { return GetPlaceholderTable(name) == table; });
}

std::string RenameTablePlaceholders(std::string_view text, std::string_view from, std::string_view to)
{
    const std::string old = "{" + MakeTablePlaceholder(from) + "}";
    const std::string renamed = "{" + MakeTablePlaceholder(to) + "}";
    std::string result(text);
    for (size_t found = result.find(old); found != std::string::npos; found = result.find(old, found + renamed.size()))
    {
        result.replace(found, old.size(), renamed);
    }
    return result;
}
} // namespace Data::Items
