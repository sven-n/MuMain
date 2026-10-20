#include "stdafx.h"
#include "ThemeFileInterface.h"

#include "UI/RmlBridge/ThemeInheritance.h"
#include "UI/RmlBridge/ThemeFonts.h"
#include "Core/Platform/PathResolve.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <memory>
#include <optional>
#include <regex>
#include <string>
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

std::FILE* OpenFile(const std::string& path)
{
#ifdef _WIN32
    return _wfopen(std::filesystem::u8path(path).c_str(), L"rb");
#else
    return std::fopen(MuResolvePath(path.c_str()).c_str(), "rb");
#endif
}

std::string ResolveToken(const ThemeInheritance& theme, const std::string& tokenName,
                         const std::string& stylesheetPath)
{
    if (const auto font = ResolveThemeFontToken(theme, tokenName))
        return *font;
    const auto setting = theme.GetSetting("Tokens", tokenName);
    if (setting)
        return setting->value;
    g_ErrorReport.Write(L"> [RmlTheme] Missing token '%hs' in stylesheet '%hs'.\r\n",
        tokenName.c_str(), stylesheetPath.c_str());
    return {};
}

std::string SubstituteTokens(const std::string& source, const std::string& themeDirectory,
                             const std::string& stylesheetPath)
{
    static const std::regex tokenPattern(R"(token\(([a-zA-Z0-9_-]+)\))");
    const ThemeInheritance theme(std::filesystem::u8path(themeDirectory));
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
            value->second = ResolveToken(theme, name, stylesheetPath);
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
    const auto resolved = ResolveThemeResource(path);
    std::FILE* stream = OpenFile(resolved);
    if (!stream)
        return 0;

    auto file = std::make_unique<FileState>();
    file->stream = stream;
    if (const auto theme = ThemeDirectoryForResource(path); theme &&
        std::filesystem::u8path(path).extension() == ".rcss")
    {
        const auto source = ReadAll(stream);
        if (!source)
        {
            std::fclose(stream);
            return 0;
        }
        if (source->find("token(") != std::string::npos)
        {
            file->contents = SubstituteTokens(*source, ThemePathUtf8(*theme), path);
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
