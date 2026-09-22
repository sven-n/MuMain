#include "Core/Text/Utf8.h"

#ifdef _WIN32
#include <windows.h> // MultiByteToWideChar / WideCharToMultiByte
#else
#include "Core/Platform/WinNls.h" // the portable stand-ins for them
#endif

#include <vector>

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
