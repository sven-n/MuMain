#pragma once

#ifdef _EDITOR

#include <string_view>

namespace Data
{
class LocalizedString;
}

// The search fields of the editor's lists (items, model files, monsters).
namespace MuEditor::TextSearch
{
// Whether the UTF-8 text contains the search; only the letters A to Z ignore
// case.
bool ContainsIgnoringCase(std::string_view text, std::string_view search);

// Whether the English name or any translation contains the search
// (ContainsIgnoringCase).
bool AnyNameContains(const Data::LocalizedString& names, std::string_view search);

// Whether the whole text is a decimal number ("12", not "12a" or ""); the
// number goes into `value`.
bool ReadWholeNumber(std::string_view text, int& value);
} // namespace MuEditor::TextSearch

#endif // _EDITOR
