#include "stdafx.h"

#include "DotNetRuntime.h"

#include <iostream>
#include <Windows.h>
#include <shlwapi.h>

#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>
#include <cassert>

DotNetRuntime::~DotNetRuntime()
{
    if (cxt_ != nullptr)
    {
        close_fptr_(cxt_);
        cxt_ = nullptr;
    }
}

using int_action = void(int32_t);

struct FooStruct
{
    static void Foo1(int32_t bar)
    {
        printf("Foo1, param: %d\r\n", bar);
    }

    static void Foo2(int32_t bar)
    {
        printf("Foo2, param: %d\r\n", bar);
    }

    void (*f1)(int) = Foo1;
    void (*f2)(int) = Foo2;
};

void _cdecl foo(int32_t bar)
{
    printf("foo bar %d\r\n", bar);
}

DotNetRuntime::DotNetRuntime()
{
    // STEP 0: Load HostFxr and get exported hosting functions
    if (!load_hostfxr())
    {
        assert(false && "Failure: load_hostfxr()");
        return;
    }

    // STEP 1: Get the current executable directory
    string_t runtimePath(MAX_PATH, '\0');
    GetModuleFileNameW(nullptr, runtimePath.data(), MAX_PATH);
    PathRemoveFileSpecW(runtimePath.data());

    // Since PathRemoveFileSpecW() removes from data(), the size is not updated, so we must manually update it
    const size_t pathSize = std::wcslen(runtimePath.data());
    runtimePath.resize(pathSize);

    // STEP 2: Initialize and start the .NET Core runtime
    const string_t config_path = runtimePath + L"\\Native.NETHostFxr.runtimeconfig.json";

    int rc = init_runtime_cfg_fptr_(config_path.c_str(), nullptr, &cxt_);
    if (rc != 0 || cxt_ == nullptr)
    {
        std::cerr << "Init failed: " << std::hex << std::showbase << rc << std::endl;
        close_fptr_(cxt_);
        return;
    }

    load_func = get_dotnet_load_assembly(config_path.c_str());
    assert(load_func != nullptr && "Failure: get_dotnet_load_assembly()");
	dotnetlib_path = runtimePath + L"\\MUnique.Client.ManagedLibrary.dll";

    is_initialized_ = true;
}

bool DotNetRuntime::is_initialized() const
{
    return is_initialized_;
}

void* DotNetRuntime::get_method(const char_t* type_name, const char_t* method_name) const
{
    void* result = nullptr;
    //const load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = get_dotnet_load_assembly(config_path.c_str());
    const int resultCode = load_func(
        dotnetlib_path.c_str(),
        type_name,
        method_name,
        UNMANAGEDCALLERSONLY_METHOD,
        nullptr,
        &result);
    assert(resultCode == 0 && result != nullptr && "Failure: couldn't get method");
    return result;
}

bool DotNetRuntime::load_hostfxr()
{
    // Pre-allocate a large buffer for the path to hostfxr
    char_t buffer[MAX_PATH];
    size_t buffer_size = sizeof(buffer) / sizeof(char_t);
    const int rc = get_hostfxr_path(buffer, &buffer_size, nullptr);
    if (rc != 0)
    {
        return false;
    }

    // Load hostfxr and get desired exports
    void* lib = load_library(buffer);
    init_runtime_cfg_fptr_ = (hostfxr_initialize_for_runtime_config_fn)get_export(lib, "hostfxr_initialize_for_runtime_config");
    get_delegate_fptr_ = (hostfxr_get_runtime_delegate_fn)get_export(lib, "hostfxr_get_runtime_delegate");
    close_fptr_ = (hostfxr_close_fn)get_export(lib, "hostfxr_close");

    return (init_runtime_cfg_fptr_ && get_delegate_fptr_ && close_fptr_);
}

void* DotNetRuntime::load_library(const char_t* path)
{
	const auto h = ::LoadLibraryW(path);
    assert(h != nullptr);
    return h;
}

void* DotNetRuntime::get_export(void* h, const char* name)
{
    void* f = ::GetProcAddress(static_cast<HMODULE>(h), name);
    assert(f != nullptr);
    return f;
}

// Load and initialize .NET Core and get desired function pointer for scenario
load_assembly_and_get_function_pointer_fn DotNetRuntime::get_dotnet_load_assembly(const char_t* config_path) const
{
    void* load_assembly_and_get_function_pointer = nullptr;

    // Get the load assembly function pointer
    const int rc = get_delegate_fptr_(
        cxt_,
        hdt_load_assembly_and_get_function_pointer,
        &load_assembly_and_get_function_pointer);
    if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
        std::cerr << "Get delegate failed: " << std::hex << std::showbase << rc << std::endl;

    return (load_assembly_and_get_function_pointer_fn)load_assembly_and_get_function_pointer;
}

get_function_pointer_fn DotNetRuntime::get_function_pointer_function() const
{
    void* result = nullptr;

    // Get the load assembly function pointer
    const int resultCode = get_delegate_fptr_(
        cxt_,
        hdt_get_function_pointer,
        &result);
    if (resultCode != 0 || result == nullptr)
        std::cerr << "Get delegate failed: " << std::hex << std::showbase << resultCode << std::endl;

    return (get_function_pointer_fn)result;
}