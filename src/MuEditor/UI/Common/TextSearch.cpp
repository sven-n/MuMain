#include "stdafx.h"

#ifdef _EDITOR

#include "TextSearch.h"

#include "Data/GameData/Common/LocalizedString.h"

#include <algorithm>
#include <charconv>

namespace MuEditor::TextSearch
{
namespace
{
// Only A to Z: std::tolower follows the process locale, which the client takes
// from the system; with a Windows code page it changes bytes of UTF-8 letters.
char ToLowerAscii(char character)
{
    return character >= 'A' && character <= 'Z' ? static_cast<char>(character - 'A' + 'a') : character;
}
} // namespace

bool ContainsIgnoringCase(std::string_view text, std::string_view search)
{
    const auto found = std::search(text.begin(), text.end(), search.begin(), search.end(),
                                   [](char a, char b) { return ToLowerAscii(a) == ToLowerAscii(b); });
    return found != text.end();
}

bool AnyNameContains(const Data::LocalizedString& names, std::string_view search)
{
    if (ContainsIgnoringCase(names.GetNeutral(), search))
    {
        return true;
    }
    const auto& translations = names.GetTranslations();
    return std::any_of(translations.begin(), translations.end(),
                       [&](const auto& translation) { return ContainsIgnoringCase(translation.second, search); });
}

bool ReadWholeNumber(std::string_view text, int& value)
{
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
    return error == std::errc{} && end == text.data() + text.size();
}
} // namespace MuEditor::TextSearch

#endif // _EDITOR
