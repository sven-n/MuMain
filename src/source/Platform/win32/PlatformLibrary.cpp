#include "stdafx.h"
#include "PlatformLibrary.h"

#include <memory>
#ifdef _WIN32
#include <windows.h>
#else
#include "Platform/PlatformTypes.h"
#include "Platform/PlatformCompat.h"
#endif

#include "MuLogger.h"

namespace mu::platform
{

LibraryHandle Load(const char* path)
{
    if (path == nullptr)
    {
        mu::log::Get("platform")->error("PLAT: PlatformLibrary::Load() failed -- path is null");
        return nullptr;
    }

    // Convert UTF-8 path to wide string for LoadLibraryW
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, path, -1, nullptr, 0);
    if (wideLen <= 0)
    {
        mu::log::Get("platform")
            ->error("PLAT: PlatformLibrary::Load() failed -- unable to convert path to wide string");
        return nullptr;
    }

    auto widePath = std::make_unique<wchar_t[]>(static_cast<size_t>(wideLen));
    MultiByteToWideChar(CP_UTF8, 0, path, -1, widePath.get(), wideLen);

    HMODULE hModule = LoadLibraryW(widePath.get());
    if (hModule == nullptr)
    {
        DWORD error = GetLastError();
        mu::log::Get("platform")
            ->error("PLAT: PlatformLibrary::Load() failed -- LoadLibraryW error={} for '{}'", error, path);
        return nullptr;
    }

    return static_cast<LibraryHandle>(hModule);
}

void* GetSymbol(LibraryHandle handle, const char* name)
{
    if (handle == nullptr || name == nullptr)
    {
        mu::log::Get("platform")->error("PLAT: PlatformLibrary::GetSymbol() failed -- handle or name is null");
        return nullptr;
    }

    // GetProcAddress returns FARPROC which needs cast to void*
    FARPROC proc = GetProcAddress(static_cast<HMODULE>(handle), name);
    if (proc == nullptr)
    {
        DWORD error = GetLastError();
        mu::log::Get("platform")
            ->error("PLAT: PlatformLibrary::GetSymbol({}) failed -- GetProcAddress error={}", name, error);
        return nullptr;
    }

    return reinterpret_cast<void*>(proc);
}

void Unload(LibraryHandle handle)
{
    if (handle == nullptr)
    {
        return;
    }

    if (!FreeLibrary(static_cast<HMODULE>(handle)))
    {
        DWORD error = GetLastError();
        mu::log::Get("platform")->error("PLAT: PlatformLibrary::Unload() failed -- FreeLibrary error={}", error);
    }
}

} // namespace mu::platform
