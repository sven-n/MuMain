#include "stdafx.h"
#include "PlatformLibrary.h"

#include <dlfcn.h>

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

    void* handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
    if (handle == nullptr)
    {
        const char* error = dlerror();
        mu::log::Get("platform")
            ->error("PLAT: PlatformLibrary::Load() failed -- {} ({})", path, error ? error : "unknown error");
        return nullptr;
    }

    return handle;
}

void* GetSymbol(LibraryHandle handle, const char* name)
{
    if (handle == nullptr || name == nullptr)
    {
        mu::log::Get("platform")->error("PLAT: PlatformLibrary::GetSymbol() failed -- handle or name is null");
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
        mu::log::Get("platform")->error("PLAT: PlatformLibrary::GetSymbol({}) failed -- {}", name, error);
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
        mu::log::Get("platform")
            ->error("PLAT: PlatformLibrary::Unload() failed -- {}", error ? error : "unknown error");
    }
}

} // namespace mu::platform
