#pragma once

#include <cstddef>
#include <string_view>

// The tooltip lines of an item's options from the option definitions: their texts (Data/Items/Tooltips/
// OptionTexts.json) in the UI language with the option's value. Each text is split at its placeholders once and kept
// until the item data or the language changes, so a tooltip drawn every frame only copies it (CODING_RULES.md,
// section 12).
namespace Engine::Items::OptionLines
{
// The numbers an option line shows: {value} and {divisor}.
struct LineValues
{
    int value = 0;
    int divisor = 0;
};

// Writes the text `name` with `values` for its placeholders into `out` (at most `capacity` characters with the
// terminating 0); false, with nothing written, for a name the texts lack.
bool Write(std::string_view name, const LineValues& values, wchar_t* out, size_t capacity);
} // namespace Engine::Items::OptionLines
