#include "stdafx.h"
#include "PlatformLibrary.h"

#include <dlfcn.h>
#include <cstdio>
#include <cwchar>

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

    void* handle = dlopen(path, RTLD_NOW);
    if (handle == nullptr)
    {
        const char* error = dlerror();
        g_ErrorReport.Write(L"PLAT: PlatformLibrary::Load() - dlopen failed for '%hs' (%hs)\r\n", path,
                            error ? error : "unknown error");
        return nullptr;
    }

    return handle;
}

void* GetSymbol(LibraryHandle handle, const char* name)
{
    if (handle == nullptr || name == nullptr)
    {
        g_ErrorReport.Write(L"PLAT: PlatformLibrary::GetSymbol() - handle or name is null\r\n");
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
        g_ErrorReport.Write(L"PLAT: PlatformLibrary::GetSymbol() - dlsym failed for '%hs' (%hs)\r\n", name, error);
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

    dlclose(handle);
}

} // namespace mu::platform
