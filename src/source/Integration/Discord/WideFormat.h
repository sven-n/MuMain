// printf-style formatting of the translated texts of the presence, whose
// length the code can't know: the result grows with them instead of being
// cut at a fixed buffer size.
#pragma once

#include <cstddef>
#include <cwchar>
#include <string>

namespace Integration::Discord
{
template <typename... Args> [[nodiscard]] std::wstring FormatWide(const wchar_t* format, Args... args)
{
    // Discord shows at most 128 characters per text, so the first try almost
    // always fits; the ceiling only stops a broken format from looping.
    constexpr std::size_t InitialLength = 128;
    constexpr std::size_t MaxLength = 4096;

    std::wstring text(InitialLength, L'\0');
    while (text.size() <= MaxLength)
    {
        // A negative result means the text didn't fit (or couldn't be encoded).
        const int written = std::swprintf(text.data(), text.size(), format, args...);
        if (written >= 0)
        {
            text.resize(static_cast<std::size_t>(written));
            return text;
        }
        text.resize(text.size() * 2);
    }
    return {};
}
} // namespace Integration::Discord
