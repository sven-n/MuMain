#include "stdafx.h"
#include "PlatformLibrary.h"

#include <memory>
#include <windows.h>

#include "ErrorReport.h"

namespace mu::platform
{

LibraryHandle Load(const char* path)
{
    if (path == nullptr)
    {
        g_ErrorReport.Write(L"PLAT: PlatformLibrary::Load() - path is null\r\n");
        return nullptr;
    }

    // Convert UTF-8 path to wide string for LoadLibraryW
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, path, -1, nullptr, 0);
    if (wideLen <= 0)
    {
        g_ErrorReport.Write(L"PLAT: PlatformLibrary::Load() - failed to convert path to wide string\r\n");
        return nullptr;
    }

    auto widePath = std::make_unique<wchar_t[]>(static_cast<size_t>(wideLen));
    MultiByteToWideChar(CP_UTF8, 0, path, -1, widePath.get(), wideLen);

    HMODULE hModule = LoadLibraryW(widePath.get());
    if (hModule == nullptr)
    {
        DWORD error = GetLastError();
        g_ErrorReport.Write(L"PLAT: PlatformLibrary::Load() - LoadLibraryW failed for '%hs' (error=%lu)\r\n", path,
                            error);
        return nullptr;
    }

    return static_cast<LibraryHandle>(hModule);
}

void* GetSymbol(LibraryHandle handle, const char* name)
{
    if (handle == nullptr || name == nullptr)
    {
        g_ErrorReport.Write(L"PLAT: PlatformLibrary::GetSymbol() - handle or name is null\r\n");
        return nullptr;
    }

    // GetProcAddress returns FARPROC which needs cast to void*
    FARPROC proc = GetProcAddress(static_cast<HMODULE>(handle), name);
    if (proc == nullptr)
    {
        DWORD error = GetLastError();
        g_ErrorReport.Write(L"PLAT: PlatformLibrary::GetSymbol() - GetProcAddress failed for '%hs' (error=%lu)\r\n",
                            name, error);
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

    FreeLibrary(static_cast<HMODULE>(handle));
}

} // namespace mu::platform
