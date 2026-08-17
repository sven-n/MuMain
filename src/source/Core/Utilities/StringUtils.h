#pragma once

#include <string>
#include "Core/Platform/WinCompat.h"

// Deliberately separate from Data/Translation/MultiLanguage.h's CMultiLanguage::ConvertFromUtf8/
// ConvertToUtf8, not a duplicate of it: CMultiLanguage's converters write into a caller-owned
// fixed-size buffer (matching network-packet/struct string fields like Data->ID[MAX_USERNAME_SIZE]),
// while these return an owned std::string/std::wstring for callers with no natural fixed size to
// target (RmlUi label/log text, file paths). CMultiLanguage also separately owns UI-language
// selection state (byLanguage) unrelated to generic encoding conversion -- folding these into it
// would conflate the two concerns. Both ultimately call the same Win32 MultiByteToWideChar/
// WideCharToMultiByte; if a third conversion need ever appears, extend one of these two, don't add
// a third implementation.
namespace StringUtils
{
    // Convert wide string (UTF-16) to narrow string (UTF-8)
    inline std::string WideToNarrow(const wchar_t* wstr)
    {
        if (!wstr) return "";
        size_t len = wcslen(wstr);
        if (len == 0) return "";

        int size = WideCharToMultiByte(CP_UTF8, 0, wstr, (int)len, NULL, 0, NULL, NULL);
        std::string result(size, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr, (int)len, &result[0], size, NULL, NULL);
        return result;
    }

    // Convert narrow string (UTF-8) to wide string (UTF-16)
    inline std::wstring NarrowToWide(const char* str)
    {
        if (!str) return L"";
        size_t len = strlen(str);
        if (len == 0) return L"";

        int size = MultiByteToWideChar(CP_UTF8, 0, str, (int)len, NULL, 0);
        std::wstring result(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, str, (int)len, &result[0], size);
        return result;
    }

    inline std::wstring NarrowToWide(const std::string& str)
    {
        return NarrowToWide(str.c_str());
    }
}
