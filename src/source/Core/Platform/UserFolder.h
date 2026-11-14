#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

// The player's own files (config.ini, MuError.log, the chat macros, screenshots) live in one writable folder per OS
// user, never next to the executable: that folder may be read-only (Program Files, a signed app bundle, an APK) or
// thrown away (a browser's memory).
namespace Core::Platform
{
// The environment variable that names the folder instead (tests, several clients side by side).
inline constexpr char UserFolderVariable[] = "MU_USER_FOLDER";
// The shipped defaults next to the game: read-only, the seed of a missing config.ini.
inline constexpr char ShippedDefaultsFile[] = "config.ini.template";

// The folder, resolved and created on the first call: MU_USER_FOLDER, else the target's folder for this client's
// name, else (when the target has none) the working directory, with an error in the log.
const std::filesystem::path& UserFolder();

// The target's folder for `clientName`, from the adapter its CMake picks (MU_TARGET_ADAPTERS); empty when it has none.
std::filesystem::path TargetUserFolder(std::string_view clientName);

// The shipped defaults' [Client] Name, a distributor's fixed name for its client; nullopt when it is missing, empty or
// not a plain folder name.
std::optional<std::string> ClientNameFromDefaults(std::string_view defaultsIni);

// The name of an install without one: its folder's name and a hash of its path, "MyMU-3f9a2c1b". Every install, and
// so every server's client a player has, keeps its own settings, as when they were next to the executable.
std::string InstallFolderName(const std::filesystem::path& clientFolder);

struct ResolvedUserFolder
{
    std::filesystem::path folder;
    // True when neither the variable nor the target gave one, and the working directory stands in.
    bool fallback = false;
};
ResolvedUserFolder ResolveUserFolder(const char* variable, const std::filesystem::path& targetFolder,
                                     const std::filesystem::path& workingDirectory);

// `path` with `home` shown as "~", for the log: a pasted MuError.log then doesn't carry the OS user's name.
std::string DisplayPath(const std::filesystem::path& path, const std::filesystem::path& home);

enum class Seeded
{
    Existing,
    FromLegacy,
    FromDefaults,
    Nothing,
    Failed,
};
// Creates `file` once: a copy of `legacy` when that exists, else `defaults` when given. An existing `file` is never
// touched, so a later start doesn't overwrite what the player saved.
Seeded SeedUserFile(const std::filesystem::path& file, const std::filesystem::path& legacy,
                    const std::optional<std::string>& defaults);
} // namespace Core::Platform
