#include "Core/Text/Utf8.h"

#ifdef _WIN32
#include <windows.h> // MultiByteToWideChar / WideCharToMultiByte
#else
#include "Core/Platform/WinNls.h" // the portable stand-ins for them
#endif

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
    std::size_t i = 0;
    while (i < text.size())
    {
        const std::size_t length = Utf8LengthOf(text[i]);
        if (length == 0 || i + length > text.size())
        {
            return false;
        }
        for (std::size_t j = 1; j < length; ++j)
        {
            if (!IsContinuationByte(text[i + j]))
            {
                return false;
            }
        }
        i += length;
    }
    return true;
}
