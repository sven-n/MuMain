#include "stdafx.h"
#include "ThemeFileInterface.h"

#include "Core/Platform/WinIni.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <filesystem>
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>

namespace UI::RmlBridge
{
namespace
{
struct FileState
{
    std::FILE* stream = nullptr;
    std::string contents;
    size_t position = 0;
};

std::optional<std::string> ThemeForStylesheet(const Rml::String& path)
{
    std::string normalized(path);
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    std::string lower = normalized;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    constexpr std::string_view themeRoot = "data/interface/rmlui/themes/";
    const size_t start = lower.find(themeRoot);
    if (start == std::string::npos || (start != 0 && lower[start - 1] != '/'))
        return std::nullopt;

    const size_t themeStart = start + themeRoot.size();
    const size_t themeEnd = lower.find('/', themeStart);
    if (themeEnd == std::string::npos || themeEnd == themeStart ||
        lower.compare(lower.size() - 5, 5, ".rcss") != 0)
        return std::nullopt;

    const std::string theme = normalized.substr(themeStart, themeEnd - themeStart);
    if (!std::all_of(theme.begin(), theme.end(), [](unsigned char c)
                     { return std::isalnum(c) || c == '_' || c == '-'; }))
        return std::nullopt;
    if (lower.find("/../", themeEnd) != std::string::npos || lower.find("/./", themeEnd) != std::string::npos)
        return std::nullopt;
    return normalized.substr(0, themeEnd);
}

std::wstring WidenAscii(const std::string& value)
{
    return std::wstring(value.begin(), value.end());
}

std::string NarrowAscii(const std::wstring& value)
{
    std::string result(value.size(), '\0');
    std::transform(value.begin(), value.end(), result.begin(),
                   [](wchar_t c) { return static_cast<char>(c); });
    return result;
}

std::string ResolveToken(const std::wstring& iniPath, const std::string& tokenName,
                         const std::string& stylesheetPath)
{
    constexpr wchar_t missingToken[] = L"__MU_THEME_TOKEN_NOT_FOUND__";
    wchar_t buffer[256] = {};
    GetPrivateProfileStringW(L"Tokens", WidenAscii(tokenName).c_str(), missingToken, buffer,
                             static_cast<DWORD>(std::size(buffer)), iniPath.c_str());
    if (std::wcscmp(buffer, missingToken) == 0)
    {
        g_ErrorReport.Write(L"> [RmlTheme] Missing token '%hs' in stylesheet '%hs' (tokens.ini: '%ls').\r\n",
                            tokenName.c_str(), stylesheetPath.c_str(), iniPath.c_str());
        return {};
    }
    return NarrowAscii(buffer);
}

std::string SubstituteTokens(const std::string& source, const std::string& themeDirectory,
                             const std::string& stylesheetPath)
{
    static const std::regex tokenPattern(R"(token\(([a-zA-Z0-9_-]+)\))");
    const std::wstring iniPath = std::filesystem::absolute(std::filesystem::path(themeDirectory) / "tokens.ini").wstring();
    std::unordered_map<std::string, std::string> values;
    std::string result;
    result.reserve(source.size());
    size_t lastEnd = 0;
    for (auto it = std::sregex_iterator(source.begin(), source.end(), tokenPattern);
         it != std::sregex_iterator(); ++it)
    {
        const std::smatch& match = *it;
        result.append(source, lastEnd, static_cast<size_t>(match.position()) - lastEnd);
        const std::string name = match[1].str();
        auto [value, inserted] = values.try_emplace(name);
        if (inserted)
            value->second = ResolveToken(iniPath, name, stylesheetPath);
        result += value->second;
        lastEnd = static_cast<size_t>(match.position() + match.length());
    }
    result.append(source, lastEnd, source.size() - lastEnd);
    return result;
}

std::optional<std::string> ReadAll(std::FILE* stream)
{
    if (std::fseek(stream, 0, SEEK_END) != 0)
        return std::nullopt;
    const long length = std::ftell(stream);
    if (length < 0 || std::fseek(stream, 0, SEEK_SET) != 0)
        return std::nullopt;
    std::string result(static_cast<size_t>(length), '\0');
    if (length != 0 && std::fread(result.data(), 1, result.size(), stream) != result.size())
        return std::nullopt;
    return result;
}
} // namespace

Rml::FileHandle ThemeFileInterface::Open(const Rml::String& path)
{
    std::FILE* stream = std::fopen(path.c_str(), "rb");
    if (!stream)
        return 0;

    auto file = std::make_unique<FileState>();
    file->stream = stream;
    if (const auto theme = ThemeForStylesheet(path))
    {
        const auto source = ReadAll(stream);
        if (!source)
        {
            std::fclose(stream);
            return 0;
        }
        if (source->find("token(") != std::string::npos)
        {
            file->contents = SubstituteTokens(*source, *theme, path);
            std::fclose(stream);
            file->stream = nullptr;
        }
        else
            std::rewind(stream);
    }
    return reinterpret_cast<Rml::FileHandle>(file.release());
}

void ThemeFileInterface::Close(Rml::FileHandle file)
{
    auto* state = reinterpret_cast<FileState*>(file);
    if (state->stream)
        std::fclose(state->stream);
    delete state;
}

size_t ThemeFileInterface::Read(void* buffer, size_t size, Rml::FileHandle file)
{
    auto* state = reinterpret_cast<FileState*>(file);
    if (state->stream)
        return std::fread(buffer, 1, size, state->stream);
    const size_t count = std::min(size, state->contents.size() - state->position);
    if (count != 0)
        std::memcpy(buffer, state->contents.data() + state->position, count);
    state->position += count;
    return count;
}

bool ThemeFileInterface::Seek(Rml::FileHandle file, long offset, int origin)
{
    auto* state = reinterpret_cast<FileState*>(file);
    if (state->stream)
        return std::fseek(state->stream, offset, origin) == 0;

    size_t base = 0;
    if (origin == SEEK_CUR)
        base = state->position;
    else if (origin == SEEK_END)
        base = state->contents.size();
    else if (origin != SEEK_SET)
        return false;

    const auto target = static_cast<int64_t>(base) + static_cast<int64_t>(offset);
    if (target < 0 || static_cast<uint64_t>(target) > state->contents.size())
        return false;
    state->position = static_cast<size_t>(target);
    return true;
}

size_t ThemeFileInterface::Tell(Rml::FileHandle file)
{
    auto* state = reinterpret_cast<FileState*>(file);
    if (!state->stream)
        return state->position;
    const long position = std::ftell(state->stream);
    return position < 0 ? 0 : static_cast<size_t>(position);
}
} // namespace UI::RmlBridge
