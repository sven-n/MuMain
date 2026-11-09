#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// Item references in texts (docs/translation-system.md, "Texts that name items"): a text names an item by its number
// instead of spelling its name, and the game puts the item's name in when it shows the text.
//
//   {item:14,25}        the name of item 14,25
//   {item:13,20+3}      its name at item level 3 (the name line of that level, "Ring of glory")
//   {item:14,25:acc}    a form of the name (nameForms in the item data), here the accusative
//   {item:14,21+2:acc}  both
//
// The grammar and the case keys are the same as ResxGen's check (tools/ResxGen/ItemReferenceSyntax.cs).
namespace Data::Translation
{
// The name of an item reference placeholder starts with this ("item:14,25" in "{item:14,25}").
constexpr std::string_view ItemReferencePrefix = "item:";
// Every reference starts with this; a text that has it must continue with a valid reference.
constexpr std::string_view ItemReferenceOpening = "{item:";
static_assert(ItemReferenceOpening.substr(1) == ItemReferencePrefix);

// The grammatical cases a reference may ask for: genitive, dative, accusative, instrumental, locative.
constexpr std::array<std::string_view, 5> ItemCaseKeys = {"gen", "dat", "acc", "ins", "loc"};

struct ItemReference
{
    int group = 0;
    int number = 0;
    // The item level whose name line names the item; none: the item's name.
    std::optional<int> level;
    // The form of the name (an ItemCaseKeys key); empty: the name itself.
    std::string form;

    bool operator==(const ItemReference& other) const = default;
};

// A reference as a text writes it: where it starts, how many bytes it takes, and what it names (none when it does
// not follow the grammar).
struct ItemReferenceMatch
{
    std::size_t position = 0;
    std::size_t length = 0;
    std::optional<ItemReference> reference;
};

bool IsItemCaseKey(std::string_view key);

// The reference written as "{item:...}", or none when it does not follow the grammar.
std::optional<ItemReference> ParseItemReference(std::string_view written);

// The same without its braces: "item:14,25:acc", as a placeholder name of the item data's texts.
std::optional<ItemReference> ParseItemReferenceName(std::string_view name);

// Every reference of the text, in order; a malformed one runs to its "}", or without one to the next reference or the
// end of the text. The matches never overlap.
std::vector<ItemReferenceMatch> FindItemReferences(std::string_view text);

bool HasItemReference(std::string_view text);

// "{item:14,25+3:acc}"
std::string FormatItemReference(const ItemReference& reference);
} // namespace Data::Translation
