#include "stdafx.h"
#include "UI/RmlBridge/ThemeInheritance.h"
#include "Core/Platform/PathResolve.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>
#include <string_view>

namespace UI::RmlBridge
{
namespace
{
std::string Lower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

std::string Trim(std::string text)
{
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return {};
    text = text.substr(first, text.find_last_not_of(" \t\r\n") - first + 1);
    if (text.size() >= 2 && ((text.front() == '"' && text.back() == '"') ||
                            (text.front() == '\'' && text.back() == '\'')))
        text = text.substr(1, text.size() - 2);
    return text;
}

bool IsFile(const std::filesystem::path& path)
{
    std::error_code error;
#ifndef _WIN32
    return std::filesystem::is_regular_file(MuResolvePath(path.string().c_str()), error);
#else
    return std::filesystem::is_regular_file(path, error);
#endif
}

std::filesystem::path TextureFile(std::filesystem::path path)
{
    const auto extension = Lower(path.extension().string());
    if (extension == ".tga")
        path.replace_extension(".OZT");
    else if (extension == ".jpg")
        path.replace_extension(".OZJ");
    return path;
}
} // namespace

bool ThemeInheritance::IsValidName(const std::string& name)
{
    return !name.empty() && std::all_of(name.begin(), name.end(), [](unsigned char c)
        { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-'; });
}

ThemeInheritance::Values ThemeInheritance::ReadValues(const std::filesystem::path& directory)
{
    Values values;
    for (const char* name : {"theme.ini", "tokens.ini"})
    {
        std::ifstream input(directory / name, std::ios::binary);
        std::string section, line;
        while (std::getline(input, line))
        {
            if (line.starts_with("\xef\xbb\xbf"))
                line.erase(0, 3);
            line = Trim(std::move(line));
            if (line.empty() || line.front() == ';' || line.front() == '#')
                continue;
            if (line.front() == '[' && line.back() == ']')
            {
                section = Lower(Trim(line.substr(1, line.size() - 2)));
                continue;
            }
            const auto equals = line.find('=');
            const bool tokensFile = std::string_view(name) == "tokens.ini";
            if (equals != std::string::npos && !section.empty() && (!tokensFile || section == "tokens"))
                values[section][Lower(Trim(line.substr(0, equals)))] = Trim(line.substr(equals + 1));
        }
    }
    return values;
}

std::optional<std::string> ThemeInheritance::ReadSetting(const std::filesystem::path& directory,
                                                      const std::string& section, const std::string& key)
{
    const auto values = ReadValues(directory);
    const auto found = values.find(Lower(section));
    if (found == values.end())
        return std::nullopt;
    const auto entry = found->second.find(Lower(key));
    return entry == found->second.end() ? std::nullopt : std::optional(entry->second);
}

ThemeInheritance::ThemeInheritance(std::filesystem::path directory)
{
    Build(std::move(directory));
}

void ThemeInheritance::Build(std::filesystem::path directory)
{
    const auto root = directory.parent_path();
    std::set<std::string> seen;
    while (true)
    {
        const std::string name = Lower(directory.filename().string());
        std::error_code error;
        if (!IsValidName(name) || !std::filesystem::is_directory(directory, error))
        {
            m_Error = "Invalid or missing theme: " + directory.generic_string();
            return;
        }
        if (!seen.insert(name).second)
        {
            m_Error = "Theme inheritance cycle at: " + name;
            return;
        }
        m_Directories.push_back(directory);
        m_Values.push_back(ReadValues(directory));
        const auto& theme = m_Values.back()["theme"];
        const auto base = theme.find("base");
        if (base == theme.end() || base->second.empty())
            return;
        const std::string parent = Lower(base->second);
        if (!IsValidName(parent))
        {
            m_Error = "Invalid base theme: " + parent;
            return;
        }
        directory = root / parent;
    }
}

std::optional<std::filesystem::path> ThemeInheritance::ResolveFile(const std::filesystem::path& relative,
                                                                 bool texture) const
{
    if (!IsValid() || relative.empty() || relative.is_absolute())
        return std::nullopt;
    for (const auto& component : relative)
        if (component == "..")
            return std::nullopt;
    for (const auto& directory : m_Directories)
    {
        const auto candidate = directory / relative;
        if (IsFile(texture ? TextureFile(candidate) : candidate))
            return candidate;
    }
    return std::nullopt;
}

std::map<std::string, ThemeInheritance::Setting> ThemeInheritance::Section(const std::string& section) const
{
    std::map<std::string, Setting> result;
    if (!IsValid())
        return result;
    for (size_t i = 0; i < m_Values.size(); ++i)
    {
        const auto found = m_Values[i].find(Lower(section));
        if (found == m_Values[i].end())
            continue;
        for (const auto& [key, value] : found->second)
            result.try_emplace(key, Setting{value, m_Directories[i]});
    }
    return result;
}

std::optional<ThemeInheritance::Setting> ThemeInheritance::GetSetting(const std::string& section,
                                                                    const std::string& key) const
{
    const auto entries = Section(section);
    const auto found = entries.find(Lower(key));
    return found == entries.end() ? std::nullopt : std::optional(found->second);
}

std::optional<std::filesystem::path> ThemeDirectoryForResource(const std::string& path)
{
    std::string normalized = path;
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    normalized = ThemePathUtf8(std::filesystem::u8path(normalized).lexically_normal());
    const std::string lower = Lower(normalized);
    constexpr std::string_view root = "data/interface/rmlui/themes/";
    const auto start = lower.find(root);
    if (start == std::string::npos || (start != 0 && lower[start - 1] != '/'))
        return std::nullopt;
    const auto themeStart = start + root.size();
    const auto end = lower.find('/', themeStart);
    if (end == std::string::npos || !ThemeInheritance::IsValidName(lower.substr(themeStart, end - themeStart)))
        return std::nullopt;
    return std::filesystem::u8path(normalized.substr(0, end));
}

std::string ResolveThemeResource(const std::string& path, bool texture)
{
    const auto directory = ThemeDirectoryForResource(path);
    if (!directory)
        return path;
    std::string normalized = path;
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    const auto relative = std::filesystem::u8path(normalized).lexically_normal().lexically_relative(*directory);
    const ThemeInheritance theme(*directory);
    if (!theme.IsValid())
    {
        g_ErrorReport.Write(L"> [RmlTheme] %hs.\r\n", theme.Error().c_str());
        return {};
    }
    const auto resolved = theme.ResolveFile(relative, texture);
    return resolved ? ThemePathUtf8(*resolved) : path;
}

std::string ThemePathUtf8(const std::filesystem::path& path)
{
    const auto utf8 = path.generic_u8string();
    return {utf8.begin(), utf8.end()};
}
} // namespace UI::RmlBridge
