#pragma once

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Data::Items
{
class ItemStatProfiles;

// Placeholders in the texts of an item's description: "{name}" is a value the tooltip fills in for the item. A name is
// letters and digits: a built-in item value, or a level bonus table of the item stat file, at the item's + level. Item
// texts have no braces of their own, so every brace belongs to a placeholder.

// The built-in values: "level" is the item's + level.
inline constexpr std::string_view LevelPlaceholder = "level";
std::span<const std::string_view> GetBuiltInPlaceholders();

// The placeholders of a text in their order (a repeated one each time), or the first problem of its braces.
struct PlaceholderScan
{
    std::vector<std::string> names;
    std::string error; // empty when the text is fine
};
PlaceholderScan ScanPlaceholders(std::string_view text);

// The value of a placeholder for an item at `level`, or nothing for a name that is neither built in nor a table of
// `profiles`.
std::optional<int> GetPlaceholderValue(std::string_view name, int level, const ItemStatProfiles& profiles);

// The text with each placeholder replaced by its value. A placeholder without a value, or a text ScanPlaceholders
// refuses, stays as written from there on.
std::string FillPlaceholders(std::string_view text, int level, const ItemStatProfiles& profiles);

// The known name nearest to `name` when it is at most two letters away (a typo), else empty.
std::string SuggestPlaceholder(std::string_view name, std::span<const std::string> known);
} // namespace Data::Items
