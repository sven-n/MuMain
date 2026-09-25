#pragma once

#include <span>
#include <string>

namespace Data::Items
{
enum class ItemDataIssueSeverity
{
    // The data works, but is probably wrong.
    Warning,
    // The data cannot be used.
    Error,
};

// One problem found while reading or validating item data.
struct ItemDataIssue
{
    static constexpr int NoItem = -1;

    ItemDataIssueSeverity severity = ItemDataIssueSeverity::Error;
    std::string source; // file name, or empty
    int group = NoItem;
    int number = NoItem;
    std::string field; // JSON field, or empty
    std::string message;

    // "<source>: item (<group>,<number>) <field>: <message>", leaving out
    // the parts that are not set.
    std::string ToString() const;
};

bool HasErrors(std::span<const ItemDataIssue> issues);
} // namespace Data::Items
