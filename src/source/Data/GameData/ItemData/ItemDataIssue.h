#pragma once

#include <span>
#include <string>

namespace Data::Items
{
// How bad a problem of game data is. The checks show the three levels apart;
// only Fatal makes Save refuse the data and stops the game at start, except
// for the look files, which then give way to the built-in look values.
enum class ItemDataIssueSeverity
{
    // The data works, but is probably wrong.
    Warning,
    // The data can be read, but something it names is missing (an item, a
    // model file): the game runs without it.
    Error,
    // The data cannot be used.
    Fatal,
};

// "fatal", "error" or "warning", as logs and ToString write the level.
const char* GetSeverityName(ItemDataIssueSeverity severity);

// One problem found while reading or validating item data.
struct ItemDataIssue
{
    static constexpr int NoItem = -1;

    ItemDataIssueSeverity severity = ItemDataIssueSeverity::Fatal;
    std::string source; // file name, or empty
    int group = NoItem;
    int number = NoItem;
    std::string field; // JSON field, or empty
    std::string message;

    // "<level>: <source>: item (<group>,<number>) <field>: <message>", leaving
    // out the parts that are not set; the level is "fatal", "error" or
    // "warning".
    std::string ToString() const;
};

// Whether any problem is Fatal: data the game and Save refuse.
bool HasFatal(std::span<const ItemDataIssue> issues);
} // namespace Data::Items
