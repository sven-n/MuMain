#pragma once

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Data::Items
{
class ItemStatProfiles;

// Placeholders in the texts of an item's description: "{name}" is a value the tooltip fills in for the item, at the
// item's + level. A placeholder is a built-in item value ("{level}"), or "table:" and the name of a level bonus table
// of the item stat file ("{table:damageIncreaseWeapons}"); names have only letters and digits. The tables are written
// apart, so a table never takes the name of a built-in value, also of one a later version adds. Item texts have no
// braces of their own, so every brace belongs to a placeholder.

// The built-in values: "level" is the item's + level.
inline constexpr std::string_view LevelPlaceholder = "level";
std::span<const std::string_view> GetBuiltInPlaceholders();

// "table:<name>" names a level bonus table.
inline constexpr std::string_view TablePlaceholderPrefix = "table:";
// The placeholder of the table (without its braces): "table:<table>".
std::string MakeTablePlaceholder(std::string_view table);
// The table a placeholder names, or empty for another placeholder.
std::string_view GetPlaceholderTable(std::string_view name);

// The placeholders of a text in their order (a repeated one each time), or the first problem of its braces.
struct PlaceholderScan
{
    std::vector<std::string> names;
    std::string error; // empty when the text is fine
};
PlaceholderScan ScanPlaceholders(std::string_view text);

// The value of a placeholder for an item at `level`, or nothing for a name that is neither built in nor "table:" and a
// table of `profiles`.
std::optional<int> GetPlaceholderValue(std::string_view name, int level, const ItemStatProfiles& profiles);

// The text with each placeholder replaced by its value. A placeholder without a value, or a text ScanPlaceholders
// refuses, stays as written from there on.
std::string FillPlaceholders(std::string_view text, int level, const ItemStatProfiles& profiles);

// The known name nearest to `name` when it is at most two letters away (a typo), else empty.
std::string SuggestPlaceholder(std::string_view name, std::span<const std::string> known);

// Whether the text names the table as a placeholder.
bool NamesTable(std::string_view text, std::string_view table);
// The text with every placeholder of the table `from` naming the table `to` instead.
std::string RenameTablePlaceholders(std::string_view text, std::string_view from, std::string_view to);
} // namespace Data::Items
