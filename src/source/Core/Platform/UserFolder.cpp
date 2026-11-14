#include "Core/Platform/UserFolder.h"

#include "Core/Content/ContentStore.h"
#include "Core/Utilities/Log/MuLogger.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_stdinc.h>

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <system_error>

namespace Core::Platform
{
namespace
{
std::string_view Trim(std::string_view text)
{
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front())))
    {
        text.remove_prefix(1);
    }
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())))
    {
        text.remove_suffix(1);
    }
    return text;
}

bool EqualsIgnoringCase(std::string_view a, std::string_view b)
{
    if (a.size() != b.size())
    {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
        {
            return false;
        }
    }
    return true;
}

// Letters, digits, space, '-', '_' and '.', not starting with '.': one folder name on every file system.
bool IsPlainFolderName(std::string_view name)
{
    if (name.empty() || name.size() > 64 || name.front() == '.')
    {
        return false;
    }
    for (const char c : name)
    {
        const auto byte = static_cast<unsigned char>(c);
        if (!std::isalnum(byte) && c != ' ' && c != '-' && c != '_' && c != '.')
        {
            return false;
        }
    }
    return true;
}

std::filesystem::path PathFromUtf8(const char* text)
{
    return std::filesystem::path(reinterpret_cast<const char8_t*>(text));
}

std::filesystem::path HomeFolder()
{
    const char* home = SDL_GetUserFolder(SDL_FOLDER_HOME);
    return home != nullptr ? PathFromUtf8(home) : std::filesystem::path();
}

std::filesystem::path ResolveOnce()
{
    std::error_code error;
    const std::filesystem::path workingDirectory = std::filesystem::current_path(error);
    // SDL's copy of the environment, in UTF-8 on every platform.
    const char* variable = SDL_getenv(UserFolderVariable);
    std::filesystem::path targetFolder;
    if (variable == nullptr || variable[0] == '\0')
    {
        const std::optional<std::string> defaults = Content::ReadFile(ShippedDefaultsFile);
        const std::string name =
            ClientNameFromDefaults(defaults.value_or("")).value_or(InstallFolderName(workingDirectory));
        targetFolder = TargetUserFolder(name);
    }

    const ResolvedUserFolder resolved = ResolveUserFolder(variable, targetFolder, workingDirectory);
    // Absolute, so a later change of the working directory (a file dialog) doesn't move the player's files.
    const std::filesystem::path folder = std::filesystem::absolute(resolved.folder, error);
    std::filesystem::create_directories(folder, error);
    const std::string shown = DisplayPath(folder, HomeFolder());
    if (resolved.fallback)
    {
        mu::log::Get("core")->error("User folder: this platform gave none ({}); using {}", SDL_GetError(), shown);
    }
    else
    {
        mu::log::Get("core")->info("User folder: {}{}", shown,
                                   variable != nullptr && variable[0] != '\0' ? " (MU_USER_FOLDER)" : "");
    }
    return folder;
}
} // namespace

const std::filesystem::path& UserFolder()
{
    static const std::filesystem::path folder = ResolveOnce();
    return folder;
}

std::optional<std::string> ClientNameFromDefaults(std::string_view defaultsIni)
{
    bool inClientSection = false;
    while (!defaultsIni.empty())
    {
        const std::size_t end = defaultsIni.find('\n');
        const std::string_view line = Trim(defaultsIni.substr(0, end));
        defaultsIni.remove_prefix(end == std::string_view::npos ? defaultsIni.size() : end + 1);

        if (line.size() >= 2 && line.front() == '[' && line.back() == ']')
        {
            inClientSection = EqualsIgnoringCase(Trim(line.substr(1, line.size() - 2)), "Client");
            continue;
        }
        const std::size_t equals = line.find('=');
        if (inClientSection && equals != std::string_view::npos &&
            EqualsIgnoringCase(Trim(line.substr(0, equals)), "Name"))
        {
            const std::string_view name = Trim(line.substr(equals + 1));
            if (IsPlainFolderName(name))
            {
                return std::string(name);
            }
            if (!name.empty())
            {
                mu::log::Get("core")->warn("{}: [Client] Name is not a plain folder name; using the install's own",
                                           ShippedDefaultsFile);
            }
            return std::nullopt;
        }
    }
    return std::nullopt;
}

std::string InstallFolderName(const std::filesystem::path& clientFolder)
{
    std::u8string full = clientFolder.u8string();
    while (full.size() > 1 && (full.back() == u8'/' || full.back() == u8'\\'))
    {
        full.pop_back();
    }

    // FNV-1a over the path's UTF-8 bytes; scripts/wayland-diag.sh computes the same.
    std::uint64_t hash = 14695981039346656037ULL;
    for (const char8_t byte : full)
    {
        hash ^= static_cast<std::uint8_t>(byte);
        hash *= 1099511628211ULL;
    }

    std::string name;
    for (const char8_t unit : std::filesystem::path(full).filename().u8string())
    {
        const auto byte = static_cast<unsigned char>(unit);
        name += std::isalnum(byte) || byte == ' ' || byte == '-' || byte == '_' || byte == '.' ? static_cast<char>(byte)
                                                                                               : '_';
    }
    if (name.size() > 32)
    {
        name.resize(32);
    }
    if (name.empty() || name.front() == '.')
    {
        name = "MuMain";
    }

    char suffix[10];
    std::snprintf(suffix, sizeof suffix, "-%08x", static_cast<unsigned>(hash & 0xFFFFFFFFULL));
    return name + suffix;
}

ResolvedUserFolder ResolveUserFolder(const char* variable, const std::filesystem::path& targetFolder,
                                     const std::filesystem::path& workingDirectory)
{
    if (variable != nullptr && variable[0] != '\0')
    {
        return {PathFromUtf8(variable), false};
    }
    if (!targetFolder.empty())
    {
        return {targetFolder, false};
    }
    return {workingDirectory, true};
}

std::string DisplayPath(const std::filesystem::path& path, const std::filesystem::path& home)
{
    const std::u8string full = path.u8string();
    std::u8string prefix = home.u8string();
    while (!prefix.empty() && (prefix.back() == u8'/' || prefix.back() == u8'\\'))
    {
        prefix.pop_back();
    }
    const bool underHome = !prefix.empty() && full.size() > prefix.size() && full.starts_with(prefix) &&
                           (full[prefix.size()] == u8'/' || full[prefix.size()] == u8'\\');
    const std::u8string shown = underHome ? u8"~" + full.substr(prefix.size()) : full;
    return std::string(shown.begin(), shown.end());
}

Seeded SeedUserFile(const std::filesystem::path& file, const std::filesystem::path& legacy,
                    const std::optional<std::string>& defaults)
{
    std::error_code error;
    if (std::filesystem::exists(file, error))
    {
        return Seeded::Existing;
    }
    std::filesystem::create_directories(file.parent_path(), error);
    // A half-written file would count as the player's own at every later start, so a failed one goes.
    const auto failed = [&file]
    {
        std::error_code ignored;
        std::filesystem::remove(file, ignored);
        return Seeded::Failed;
    };
    if (!legacy.empty() && std::filesystem::is_regular_file(legacy, error))
    {
        if (std::filesystem::copy_file(legacy, file, error))
        {
            return Seeded::FromLegacy;
        }
        // Another client starting at the same moment made it first: theirs stays.
        return error == std::errc::file_exists ? Seeded::Existing : failed();
    }
    if (!defaults)
    {
        return Seeded::Nothing;
    }
    std::ofstream stream(file, std::ios::binary | std::ios::trunc);
    stream << *defaults;
    stream.close();
    return stream.fail() ? failed() : Seeded::FromDefaults;
}
} // namespace Core::Platform
