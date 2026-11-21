#include "Core/Text/AsciiText.h"

#include <algorithm>

namespace Core::Text
{
namespace
{
bool IsAsciiWhitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f';
}

char AsciiLower(char c)
{
    return c >= 'A' && c <= 'Z' ? static_cast<char>(c - 'A' + 'a') : c;
}
} // namespace

std::string_view TrimAsciiWhitespace(std::string_view text)
{
    while (!text.empty() && IsAsciiWhitespace(text.front()))
    {
        text.remove_prefix(1);
    }
    while (!text.empty() && IsAsciiWhitespace(text.back()))
    {
        text.remove_suffix(1);
    }
    return text;
}

bool EqualsIgnoringAsciiCase(std::string_view left, std::string_view right)
{
    return left.size() == right.size() && std::equal(left.begin(), left.end(), right.begin(),
                                                     [](char a, char b) { return AsciiLower(a) == AsciiLower(b); });
}

std::string ToLowerAscii(std::string_view text)
{
    std::string lower(text);
    std::transform(lower.begin(), lower.end(), lower.begin(), AsciiLower);
    return lower;
}
} // namespace Core::Text
