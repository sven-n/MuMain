#include "Core/Text/NumberParse.h"

#include <cerrno>
#include <climits>
#include <cwchar>
#include <cwctype>
#include <string>

namespace Core::Text
{
namespace
{
bool IsWhitespace(wchar_t character)
{
    return std::iswspace(static_cast<wint_t>(character)) != 0;
}
} // namespace

std::wstring_view TrimWhitespace(std::wstring_view text)
{
    while (!text.empty() && IsWhitespace(text.front()))
    {
        text.remove_prefix(1);
    }
    while (!text.empty() && IsWhitespace(text.back()))
    {
        text.remove_suffix(1);
    }
    return text;
}

std::optional<double> TryParseDouble(std::wstring_view text)
{
    text = TrimWhitespace(text);
    if (text.empty())
    {
        return std::nullopt;
    }

    // The trim can leave the view in the middle of its buffer, so parse a terminated copy.
    const std::wstring terminated(text);
    wchar_t* end = nullptr;
    errno = 0;
    const double value = std::wcstod(terminated.c_str(), &end);
    if (end == terminated.c_str() || *end != L'\0' || errno == ERANGE)
    {
        return std::nullopt;
    }

    return value;
}

std::optional<int> TryParseInt(std::wstring_view text)
{
    text = TrimWhitespace(text);
    if (text.empty())
    {
        return std::nullopt;
    }

    const std::wstring terminated(text);
    wchar_t* end = nullptr;
    errno = 0;
    const long value = std::wcstol(terminated.c_str(), &end, 10);
    if (end == terminated.c_str() || *end != L'\0' || errno == ERANGE || value < INT_MIN || value > INT_MAX)
    {
        return std::nullopt;
    }

    return static_cast<int>(value);
}
} // namespace Core::Text
