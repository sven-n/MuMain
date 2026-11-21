#include "Core/Platform/UserFolder.h"

#include "Core/Content/ContentStore.h"
#include "Core/Text/AsciiText.h"
#include "Core/Utilities/Log/MuLogger.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_stdinc.h>

#include <atomic>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <system_error>

namespace Core::Platform
{
namespace
{
using Core::Text::EqualsIgnoringAsciiCase;
using Core::Text::TrimAsciiWhitespace;

// A distributor's [Client] Name, and the install folder's name in a name without one.
constexpr std::size_t MaxClientNameLength = 64;
constexpr std::size_t MaxInstallNameLength = 32;
// FNV-1a, 64 bits; scripts/wayland-diag.sh computes the same.
constexpr std::uint64_t FnvOffsetBasis = 14695981039346656037ULL;
constexpr std::uint64_t FnvPrime = 1099511628211ULL;
// The hash's low 32 bits in an install's name: "-3f9a2c1b".
constexpr char HashSuffixFormat[] = "-%08x";
constexpr std::size_t HashSuffixSize = sizeof("-00000000");

// Letters, digits, space, '-', '_' and '.', not starting with '.': one folder name on every file system.
bool IsPlainFolderName(std::string_view name)
{
    if (name.empty() || name.size() > MaxClientNameLength || name.front() == '.')
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

namespace
{
std::atomic<bool> g_userFilesChanged{false};
} // namespace

void UserFilesChanged()
{
    g_userFilesChanged.store(true, std::memory_order_release);
}

bool CommitUserFiles()
{
    if (!g_userFilesChanged.exchange(false, std::memory_order_acq_rel))
    {
        return false;
    }
    TargetCommitUserFiles(UserFolder());
    return true;
}

std::optional<std::string> ClientNameFromDefaults(std::string_view defaultsIni)
{
    bool inClientSection = false;
    while (!defaultsIni.empty())
    {
        const std::size_t end = defaultsIni.find('\n');
        const std::string_view line = TrimAsciiWhitespace(defaultsIni.substr(0, end));
        defaultsIni.remove_prefix(end == std::string_view::npos ? defaultsIni.size() : end + 1);

        if (line.size() >= 2 && line.front() == '[' && line.back() == ']')
        {
            inClientSection = EqualsIgnoringAsciiCase(TrimAsciiWhitespace(line.substr(1, line.size() - 2)), "Client");
            continue;
        }
        const std::size_t equals = line.find('=');
        if (inClientSection && equals != std::string_view::npos &&
            EqualsIgnoringAsciiCase(TrimAsciiWhitespace(line.substr(0, equals)), "Name"))
        {
            const std::string_view name = TrimAsciiWhitespace(line.substr(equals + 1));
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

    std::uint64_t hash = FnvOffsetBasis;
    for (const char8_t byte : full)
    {
        hash ^= static_cast<std::uint8_t>(byte);
        hash *= FnvPrime;
    }

    std::string name;
    for (const char8_t unit : std::filesystem::path(full).filename().u8string())
    {
        const auto byte = static_cast<unsigned char>(unit);
        name += std::isalnum(byte) || byte == ' ' || byte == '-' || byte == '_' || byte == '.' ? static_cast<char>(byte)
                                                                                               : '_';
    }
    if (name.size() > MaxInstallNameLength)
    {
        name.resize(MaxInstallNameLength);
    }
    if (name.empty() || name.front() == '.')
    {
        name = "MuMain";
    }

    char suffix[HashSuffixSize];
    std::snprintf(suffix, sizeof suffix, HashSuffixFormat, static_cast<unsigned>(hash & 0xFFFFFFFFULL));
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
            UserFilesChanged();
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
    if (stream.fail())
    {
        return failed();
    }
    UserFilesChanged();
    return Seeded::FromDefaults;
}
} // namespace Core::Platform
