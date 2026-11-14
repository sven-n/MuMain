#include "Core/Platform/UserFolder.h"

#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_stdinc.h>

#include <string>

namespace Core::Platform
{

// SDL's per-user folder: %APPDATA%\MuMain\<name>\ on Windows, ~/.local/share/MuMain/<name>/ on Linux,
// ~/Library/Application Support/MuMain/<name>/ on macOS.
std::filesystem::path TargetUserFolder(std::string_view clientName)
{
    char* folder = SDL_GetPrefPath("MuMain", std::string(clientName).c_str());
    if (folder == nullptr)
    {
        return {};
    }
    std::filesystem::path result(reinterpret_cast<const char8_t*>(folder));
    SDL_free(folder);
    return result;
}

} // namespace Core::Platform
