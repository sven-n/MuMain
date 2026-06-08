// Non-Windows implementation of the code-page conversion shim (WinNls.h).
// Converts between UTF-8 and the platform wchar_t (UTF-32 on Linux).
#ifndef _WIN32

#include "Core/Platform/WinNls.h"

#include <cstring>  // strlen, memcpy
#include <cwchar>   // wcslen

namespace
{
    constexpr unsigned int kReplacement = 0xFFFD;

    // Decode one UTF-8 sequence starting at s[i] (within len bytes). Returns the
    // code point and advances i. Malformed/truncated input yields U+FFFD.
    unsigned int DecodeUtf8(const unsigned char* s, size_t len, size_t& i)
    {
        const unsigned char b0 = s[i];
        if (b0 < 0x80) { ++i; return b0; }

        auto cont = [&](size_t k) { return i + k < len && (s[i + k] & 0xC0) == 0x80; };

        if ((b0 & 0xE0) == 0xC0 && cont(1))
        {
            const unsigned int cp = ((b0 & 0x1Fu) << 6) | (s[i + 1] & 0x3Fu);
            i += 2;
            return cp < 0x80 ? kReplacement : cp;  // reject overlong
        }
        if ((b0 & 0xF0) == 0xE0 && cont(1) && cont(2))
        {
            const unsigned int cp = ((b0 & 0x0Fu) << 12) | ((s[i + 1] & 0x3Fu) << 6) | (s[i + 2] & 0x3Fu);
            i += 3;
            return (cp < 0x800 || (cp >= 0xD800 && cp <= 0xDFFF)) ? kReplacement : cp;
        }
        if ((b0 & 0xF8) == 0xF0 && cont(1) && cont(2) && cont(3))
        {
            const unsigned int cp = ((b0 & 0x07u) << 18) | ((s[i + 1] & 0x3Fu) << 12) |
                                    ((s[i + 2] & 0x3Fu) << 6) | (s[i + 3] & 0x3Fu);
            i += 4;
            return (cp < 0x10000 || cp > 0x10FFFF) ? kReplacement : cp;
        }

        ++i;  // invalid lead byte
        return kReplacement;
    }

    // Encode a code point as UTF-8 into buf (>= 4 bytes); returns the byte count.
    int EncodeUtf8(unsigned int cp, char* buf)
    {
        if ((cp >= 0xD800 && cp <= 0xDFFF) || cp > 0x10FFFF) cp = kReplacement;
        if (cp < 0x80)    { buf[0] = static_cast<char>(cp); return 1; }
        if (cp < 0x800)   { buf[0] = static_cast<char>(0xC0 | (cp >> 6));  buf[1] = static_cast<char>(0x80 | (cp & 0x3F)); return 2; }
        if (cp < 0x10000) { buf[0] = static_cast<char>(0xE0 | (cp >> 12)); buf[1] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F)); buf[2] = static_cast<char>(0x80 | (cp & 0x3F)); return 3; }
        buf[0] = static_cast<char>(0xF0 | (cp >> 18));
        buf[1] = static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        buf[2] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        buf[3] = static_cast<char>(0x80 | (cp & 0x3F));
        return 4;
    }
}

int MultiByteToWideChar(UINT /*CodePage*/, DWORD /*dwFlags*/, LPCSTR lpMultiByteStr,
                        int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
{
    if (!lpMultiByteStr) return 0;
    const unsigned char* src = reinterpret_cast<const unsigned char*>(lpMultiByteStr);
    // -1 means null-terminated; include the terminator in the processed length.
    const size_t srcLen = (cbMultiByte < 0) ? (std::strlen(lpMultiByteStr) + 1)
                                            : static_cast<size_t>(cbMultiByte);

    int outCount = 0;
    for (size_t i = 0; i < srcLen; )
    {
        const unsigned int cp = DecodeUtf8(src, srcLen, i);
        if (cchWideChar != 0)
        {
            if (outCount >= cchWideChar) return 0;  // insufficient buffer
            lpWideCharStr[outCount] = static_cast<wchar_t>(cp);
        }
        ++outCount;
    }
    return outCount;
}

int WideCharToMultiByte(UINT /*CodePage*/, DWORD /*dwFlags*/, LPCWSTR lpWideCharStr,
                        int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte,
                        LPCSTR /*lpDefaultChar*/, LPBOOL /*lpUsedDefaultChar*/)
{
    if (!lpWideCharStr) return 0;
    // -1 means null-terminated; include the terminator in the processed length.
    const size_t srcLen = (cchWideChar < 0) ? (std::wcslen(lpWideCharStr) + 1)
                                            : static_cast<size_t>(cchWideChar);

    int outCount = 0;
    char buf[4];
    for (size_t i = 0; i < srcLen; ++i)
    {
        const int n = EncodeUtf8(static_cast<unsigned int>(lpWideCharStr[i]), buf);
        if (cbMultiByte != 0)
        {
            if (outCount + n > cbMultiByte) return 0;  // insufficient buffer
            std::memcpy(lpMultiByteStr + outCount, buf, static_cast<size_t>(n));
        }
        outCount += n;
    }
    return outCount;
}

#endif // !_WIN32
