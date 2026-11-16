#include "stdafx.h"

#include "ManagedLibrarySource.h"

#include "Dotnet/ManagedSymbols.h"

#include <filesystem>

mu::platform::LibraryHandle munique_client_library_handle = nullptr;

std::string ManagedLibraryPath()
{
    // Both defined by src/CMakeLists.txt for the target.
    const std::string libraryFile = MU_MANAGED_LIBRARY_FILE;
    if constexpr (MU_MANAGED_LIBRARY_BY_NAME)
    {
        return libraryFile;
    }
    else
    {
        constexpr DWORD executablePathCapacity = 4096;
        wchar_t executablePath[executablePathCapacity] = {};
        if (GetModuleFileNameW(nullptr, executablePath, executablePathCapacity) == 0)
        {
            return libraryFile;
        }
        return (std::filesystem::path(mu_narrow_path(executablePath)).parent_path() / libraryFile).string();
    }
}

void LoadManagedLibrary()
{
    static bool attempted = false;
    if (attempted)
    {
        return;
    }
    attempted = true;
    munique_client_library_handle = mu::platform::Load(ManagedLibraryPath().c_str());
}

namespace
{
void* FindInLibrary(const char* name)
{
    return mu::platform::FindSymbol(munique_client_library_handle, name);
}
} // namespace

DotNetBridge::ManagedSymbolSource DotNetBridge::TargetManagedSymbolSource()
{
    LoadManagedLibrary();
    return {ManagedLibraryPath(), munique_client_library_handle != nullptr ? &FindInLibrary : nullptr};
}
