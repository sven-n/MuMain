#include "stdafx.h"
#include "PlatformLibrary.h"

#include <cstdlib>
#include <dlfcn.h>

#include "ErrorReport.h"

namespace mu::platform
{

LibraryHandle Load(const char* path)
{
    if (path == nullptr)
    {
        g_ErrorReport.Write(L"PLAT: PlatformLibrary::Load() failed -- path is null\r\n");
        return nullptr;
    }

    void* handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
    if (handle == nullptr)
    {
        const char* error = dlerror();
        wchar_t wPath[512] = {};
        wchar_t wError[512] = {};
        mbstowcs(wPath, path, (sizeof(wPath) / sizeof(wchar_t)) - 1);
        mbstowcs(wError, error ? error : "unknown error", (sizeof(wError) / sizeof(wchar_t)) - 1);
        g_ErrorReport.Write(L"PLAT: PlatformLibrary::Load() failed -- %ls (%ls)\r\n", wPath, wError);
        return nullptr;
    }

    return handle;
}

void* GetSymbol(LibraryHandle handle, const char* name)
{
    if (handle == nullptr || name == nullptr)
    {
        g_ErrorReport.Write(L"PLAT: PlatformLibrary::GetSymbol() failed -- handle or name is null\r\n");
        return nullptr;
    }

    // Clear any existing error
    dlerror();

    void* symbol = dlsym(handle, name);

    // dlsym can return nullptr for a valid symbol that has value 0,
    // so we must check dlerror() to distinguish real errors.
    const char* error = dlerror();
    if (error != nullptr)
    {
        wchar_t wName[512] = {};
        wchar_t wError[512] = {};
        mbstowcs(wName, name, (sizeof(wName) / sizeof(wchar_t)) - 1);
        mbstowcs(wError, error, (sizeof(wError) / sizeof(wchar_t)) - 1);
        g_ErrorReport.Write(L"PLAT: PlatformLibrary::GetSymbol(%ls) failed -- %ls\r\n", wName, wError);
        return nullptr;
    }

    return symbol;
}

void Unload(LibraryHandle handle)
{
    if (handle == nullptr)
    {
        return;
    }

    if (dlclose(handle) != 0)
    {
        const char* error = dlerror();
        wchar_t wError[512] = {};
        mbstowcs(wError, error ? error : "unknown error", (sizeof(wError) / sizeof(wchar_t)) - 1);
        g_ErrorReport.Write(L"PLAT: PlatformLibrary::Unload() failed -- %ls\r\n", wError);
    }
}

} // namespace mu::platform
