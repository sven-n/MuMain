#include "Core/Text/Utf8.h"

#ifdef _WIN32
#include <windows.h> // MultiByteToWideChar / WideCharToMultiByte
#else
#include "Core/Platform/WinNls.h" // the portable stand-ins for them
#endif

#include <array>
#include <vector>

namespace
{
// The bits of a UTF-8 byte that tell whether it continues a character, and
// whether it starts one of four bytes.
constexpr unsigned char Utf8ContinuationMask = 0xC0;
constexpr unsigned char Utf8ContinuationBits = 0x80;
constexpr unsigned char Utf8FourByteMask = 0xF8;
constexpr unsigned char Utf8FourByteBits = 0xF0;
// The lead bytes of one-, two- and three-byte characters.
constexpr unsigned char Utf8SingleByteEnd = 0x80;
constexpr unsigned char Utf8TwoByteMask = 0xE0;
constexpr unsigned char Utf8TwoByteBits = 0xC0;
constexpr unsigned char Utf8ThreeByteMask = 0xF0;
constexpr unsigned char Utf8ThreeByteBits = 0xE0;
// A character outside the Basic Multilingual Plane takes two UTF-16 units.
constexpr std::size_t SurrogatePairUnits = 2;
// RFC 3629 allows characters of one to four bytes. By the bytes of a
// character: the bits of its lead byte that belong to the code point, and the
// smallest code point that needs that many bytes (a smaller one is an overlong
// form). Index 0 is unused.
constexpr std::size_t Utf8MaxLength = 4;
constexpr std::array<unsigned char, Utf8MaxLength + 1> Utf8LeadPayloadMasks = {0x00, 0x7F, 0x1F, 0x0F, 0x07};
constexpr std::array<char32_t, Utf8MaxLength + 1> Utf8SmallestCodePoints = {0, 0, 0x80, 0x800, 0x10000};
// The bits of a continuation byte that belong to the code point.
constexpr unsigned char Utf8ContinuationPayloadMask = 0x3F;
constexpr int Utf8ContinuationPayloadBits = 6;
// The UTF-16 surrogates are no characters of their own, and Unicode ends at
// U+10FFFF.
constexpr char32_t FirstSurrogate = 0xD800;
constexpr char32_t LastSurrogate = 0xDFFF;
constexpr char32_t LastCodePoint = 0x10FFFF;

bool IsContinuationByte(char byte)
{
    return (static_cast<unsigned char>(byte) & Utf8ContinuationMask) == Utf8ContinuationBits;
}

// The UTF-16 units of the character a UTF-8 lead byte starts: a four-byte
// character (most emoji) is outside the Basic Multilingual Plane.
std::size_t Utf16UnitsOf(char leadByte)
{
    return (static_cast<unsigned char>(leadByte) & Utf8FourByteMask) == Utf8FourByteBits ? SurrogatePairUnits : 1;
}

// The bytes of the UTF-8 character that `leadByte` starts, or 0 when it cannot
// start one.
std::size_t Utf8LengthOf(char leadByte)
{
    const auto lead = static_cast<unsigned char>(leadByte);
    if (lead < Utf8SingleByteEnd)
    {
        return 1;
    }
    if ((lead & Utf8TwoByteMask) == Utf8TwoByteBits)
    {
        return 2;
    }
    if ((lead & Utf8ThreeByteMask) == Utf8ThreeByteBits)
    {
        return 3;
    }
    if ((lead & Utf8FourByteMask) == Utf8FourByteBits)
    {
        return 4;
    }
    return 0;
}

// Whether RFC 3629 allows the code point in a character of `length` bytes: in
// its shortest form, not a UTF-16 surrogate and not above U+10FFFF.
bool IsAllowedCodePoint(char32_t codePoint, std::size_t length)
{
    const bool overlong = codePoint < Utf8SmallestCodePoints[length];
    const bool surrogate = codePoint >= FirstSurrogate && codePoint <= LastSurrogate;
    return !overlong && !surrogate && codePoint <= LastCodePoint;
}

// The bytes of the UTF-8 character the text starts with, or 0 when it does not
// start with one RFC 3629 allows.
std::size_t ValidUtf8LengthAtStart(std::string_view text)
{
    const std::size_t length = Utf8LengthOf(text.front());
    if (length == 0 || length > text.size())
    {
        return 0;
    }
    auto codePoint = static_cast<char32_t>(static_cast<unsigned char>(text.front()) & Utf8LeadPayloadMasks[length]);
    for (std::size_t i = 1; i < length; ++i)
    {
        if (!IsContinuationByte(text[i]))
        {
            return 0;
        }
        const auto payload = static_cast<char32_t>(static_cast<unsigned char>(text[i]) & Utf8ContinuationPayloadMask);
        codePoint = (codePoint << Utf8ContinuationPayloadBits) | payload;
    }
    return IsAllowedCodePoint(codePoint, length) ? length : 0;
}
} // namespace

std::string Core::Text::ToUtf8(const wchar_t* text)
{
    if (text == nullptr || text[0] == L'\0')
    {
        return {};
    }

    const int bytesWithTerminator = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (bytesWithTerminator <= 1)
    {
        return {};
    }

    std::vector<char> buffer(static_cast<std::size_t>(bytesWithTerminator));
    const int written = WideCharToMultiByte(CP_UTF8, 0, text, -1, buffer.data(), bytesWithTerminator, nullptr, nullptr);
    if (written <= 1)
    {
        return {};
    }

    return std::string(buffer.data(), static_cast<std::size_t>(written - 1));
}

std::wstring Core::Text::FromUtf8(const std::string& text)
{
    if (text.empty())
    {
        return {};
    }

    const int charactersWithTerminator = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (charactersWithTerminator <= 1)
    {
        return {};
    }

    std::vector<wchar_t> buffer(static_cast<std::size_t>(charactersWithTerminator));
    const int written = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, buffer.data(), charactersWithTerminator);
    if (written <= 1)
    {
        return {};
    }

    return std::wstring(buffer.data(), static_cast<std::size_t>(written - 1));
}

bool Core::Text::CutToUtf16Length(std::string& text, std::size_t maxUnits)
{
    std::size_t units = 0;
    for (std::size_t i = 0; i < text.size(); ++i)
    {
        if (IsContinuationByte(text[i]))
        {
            continue;
        }
        units += Utf16UnitsOf(text[i]);
        if (units > maxUnits)
        {
            text.resize(i);
            return true;
        }
    }
    return false;
}

bool Core::Text::IsValidUtf8(std::string_view text)
{
    while (!text.empty())
    {
        const std::size_t length = ValidUtf8LengthAtStart(text);
        if (length == 0)
        {
            return false;
        }
        text.remove_prefix(length);
    }
    return true;
}
