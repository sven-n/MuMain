#pragma once

#include <string>
#include <string_view>

// Narrow (UTF-8) text compared and trimmed by its ASCII letters and whitespace only, as ini keys and
// file names are; bytes above 127 are left as they are.
namespace Core::Text
{
// `text` without leading and trailing ASCII whitespace (space, tab, CR, LF, vertical tab, form feed).
std::string_view TrimAsciiWhitespace(std::string_view text);
// True when `left` and `right` differ at most in the case of ASCII letters.
bool EqualsIgnoringAsciiCase(std::string_view left, std::string_view right);
// `text` with its ASCII letters in small letters.
std::string ToLowerAscii(std::string_view text);
} // namespace Core::Text
