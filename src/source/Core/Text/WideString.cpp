#include "Core/Text/WideString.h"

#include <algorithm>
#include <cwchar>

namespace
{
constexpr unsigned int kLastOneByteCodePoint = 0x7F;
constexpr unsigned int kLastTwoByteCodePoint = 0x7FF;
constexpr unsigned int kFirstFourByteCodePoint = 0x10000;  // reached directly by a 32-bit wchar_t
constexpr unsigned int kHighSurrogateFirst = 0xD800;
constexpr unsigned int kHighSurrogateLast = 0xDBFF;
constexpr unsigned int kLowSurrogateFirst = 0xDC00;
constexpr unsigned int kLowSurrogateLast = 0xDFFF;

bool IsSurrogatePairAt(const wchar_t* text)
{
    const auto high = static_cast<unsigned int>(text[0]);
    const auto low = static_cast<unsigned int>(text[1]);
    return high >= kHighSurrogateFirst && high <= kHighSurrogateLast && low >= kLowSurrogateFirst &&
           low <= kLowSurrogateLast;
}

// UTF-8 bytes of the character starting at `text`; `units` receives how many wchar_t it spans.
std::size_t Utf8BytesOfCharacterAt(const wchar_t* text, std::size_t& units)
{
    const auto codePoint = static_cast<unsigned int>(text[0]);
    units = 1;
    if (codePoint <= kLastOneByteCodePoint)
        return 1;
    if (codePoint <= kLastTwoByteCodePoint)
        return 2;
    if (codePoint >= kFirstFourByteCodePoint)
        return 4;
    if (IsSurrogatePairAt(text))
    {
        units = 2;
        return 4;
    }
    return 3;
}
} // namespace

bool Core::Text::CopyWideString(wchar_t* destination, std::size_t destinationCount, const wchar_t* source)
{
    if (destination == nullptr || destinationCount == 0)
        return false;

    if (source == nullptr)
    {
        destination[0] = L'\0';
        return false;
    }

    const std::size_t sourceLength = std::wcslen(source);
    const std::size_t copyLength = std::min(sourceLength, destinationCount - 1);
    std::wmemcpy(destination, source, copyLength);
    destination[copyLength] = L'\0';
    return copyLength == sourceLength;
}

bool Core::Text::AppendWideString(wchar_t* destination, std::size_t destinationCount, const wchar_t* source)
{
    if (destination == nullptr || destinationCount == 0)
        return false;

    const wchar_t* end = std::find(destination, destination + destinationCount, L'\0');
    const auto used = static_cast<std::size_t>(end - destination);
    if (used == destinationCount)
    {
        // Not terminated: the last unit becomes the end.
        destination[destinationCount - 1] = L'\0';
        return source == nullptr || source[0] == L'\0';
    }
    if (source == nullptr)
        return true;

    return CopyWideString(destination + used, destinationCount - used, source);
}

bool Core::Text::CutWideStringToUtf8Bytes(wchar_t* text, std::size_t maxBytes)
{
    if (text == nullptr)
        return false;

    std::size_t bytes = 0;
    std::size_t index = 0;
    while (text[index] != L'\0')
    {
        std::size_t units = 0;
        const std::size_t characterBytes = Utf8BytesOfCharacterAt(text + index, units);
        if (bytes + characterBytes > maxBytes)
        {
            text[index] = L'\0';
            return true;
        }
        bytes += characterBytes;
        index += units;
    }
    return false;
}
