#pragma once

#include <cstddef>
#include <cwchar>

#include "Core/Platform/SecureCrt.h"

namespace Core::Text
{
/**
 * @brief Appends formatted text to the string already in buffer.
 *
 * Appends in place instead of formatting buffer from itself: reading and
 * writing the same buffer in one swprintf is undefined, and glibc does
 * interleave the characters. Text that doesn't fit is truncated, and the
 * buffer always stays terminated.
 */
template <std::size_t N, typename... Args>
void AppendFormatted(wchar_t (&buffer)[N], const wchar_t* format, Args... args)
{
    if (format == nullptr)
    {
        return;
    }

    const std::size_t used = wcsnlen(buffer, N);
    if (used + 1 >= N)
    {
        return;
    }

    _snwprintf_s(buffer + used, N - used, _TRUNCATE, format, args...);
}
} // namespace Core::Text
