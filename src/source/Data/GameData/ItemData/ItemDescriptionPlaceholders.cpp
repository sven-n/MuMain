#include "stdafx.h"

#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"

#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemStatProfiles.h"

#include <algorithm>
#include <functional>
#include <array>

namespace Data::Items
{
namespace
{
// {level}, and the values of the tooltip's stat lines, which the game computes (Engine/Object/ItemTooltipValues).
constexpr std::array<std::string_view, 38> BuiltIns = {
    LevelPlaceholder,    "damageMin",           "damageMax",        "defense",
    "magicDefense",      "blockRate",           "attackSpeed",      "walkSpeed",
    "magicPower",        "durability",          "maxDurability",    "iceResistance",
    "poisonResistance",  "lightningResistance", "fireResistance",   "earthResistance",
    "windResistance",    "waterResistance",     "requiredLevel",    "requiredStrength",
    "requiredDexterity", "requiredVitality",    "requiredEnergy",   "requiredLeadership",
    "missingLevel",      "missingStrength",     "missingDexterity", "missingVitality",
    "missingEnergy",     "missingLeadership",   "addOptionValue1",  "addOptionValue2",
    "addOptionDuration", "heroLevel",           "levelPlusOne",     "name",
    "excellentFlags",    "bundleCount",
};
static_assert(BuiltIns.size() == static_cast<size_t>(TooltipValue::Count), "a name for every TooltipValue, in order");

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

PlaceholderSpans FindPlaceholders(std::string_view text)
{
    PlaceholderSpans found;
    size_t position = 0;
    while (position < text.size())
    {
        const size_t brace = text.find_first_of("{}", position);
        if (brace == std::string_view::npos)
        {
            break;
        }
        const Placeholder placeholder = text[brace] == '{' ? ReadPlaceholder(text, brace) : Placeholder{};
        if (!placeholder.error.empty() || placeholder.end == std::string_view::npos)
        {
            found.stop = brace;
            break;
        }
        found.spans.push_back({brace, placeholder.end, placeholder.name});
        position = placeholder.end;
    }
    return found;
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

namespace
{
struct GameDataValuePrefix
{
    GameDataValueKind kind;
    std::string_view prefix;
    int limit; // numbers below it
};
constexpr GameDataValuePrefix GameDataValuePrefixes[] = {
    {GameDataValueKind::SkillMana, "skillMana:", MAX_SKILLS},
    {GameDataValueKind::SkillName, "skillName:", MAX_SKILLS},
    {GameDataValueKind::MonsterName, "monsterName:", MAX_MONSTER},
};
} // namespace

bool HasGameDataValuePrefix(std::string_view name)
{
    return std::any_of(std::begin(GameDataValuePrefixes), std::end(GameDataValuePrefixes),
                       [&](const GameDataValuePrefix& entry) { return name.starts_with(entry.prefix); });
}

std::optional<GameDataValue> ReadGameDataValue(std::string_view name)
{
    for (const GameDataValuePrefix& entry : GameDataValuePrefixes)
    {
        if (!name.starts_with(entry.prefix))
        {
            continue;
        }
        const std::string_view number = name.substr(entry.prefix.size());
        if (number.empty() || number.size() > 4 ||
            !std::all_of(number.begin(), number.end(), [](char c) { return c >= '0' && c <= '9'; }))
        {
            return std::nullopt;
        }
        const int value = std::stoi(std::string(number));
        return value < entry.limit ? std::optional<GameDataValue>(GameDataValue{entry.kind, value}) : std::nullopt;
    }
    return std::nullopt;
}

bool IsBuiltInPlaceholder(std::string_view name)
{
    return FindBuiltInPlaceholder(name).has_value();
}

std::optional<TooltipValue> FindBuiltInPlaceholder(std::string_view name)
{
    const auto found = std::find(BuiltIns.begin(), BuiltIns.end(), name);
    return found == BuiltIns.end() ? std::nullopt
                                   : std::optional<TooltipValue>(static_cast<TooltipValue>(found - BuiltIns.begin()));
}

std::string_view GetBuiltInPlaceholderName(TooltipValue value)
{
    const size_t index = static_cast<size_t>(value);
    return index < BuiltIns.size() ? BuiltIns[index] : std::string_view{};
}

std::string FillPlaceholders(std::string_view text, int level, const ItemStatProfiles& profiles)
{
    return FillPlaceholders(text,
                            [&](std::string_view name) -> std::optional<std::string>
                            {
                                if (const std::optional<int> value = GetPlaceholderValue(name, level, profiles))
                                    return std::to_string(*value);
                                return std::nullopt;
                            });
}

std::string FillPlaceholders(std::string_view text, const PlaceholderValueOf& valueOf)
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
        const std::optional<std::string> value = placeholder.error.empty() && placeholder.end != std::string_view::npos
                                                     ? valueOf(placeholder.name)
                                                     : std::nullopt;
        if (!value)
        {
            position = brace;
            break;
        }
        filled.append(*value);
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
