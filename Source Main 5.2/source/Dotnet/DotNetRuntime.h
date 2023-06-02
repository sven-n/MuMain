#pragma once

#include <coreclr_delegates.h>
#include <hostfxr.h>

using string_t = std::basic_string<char_t>;

/**
 * \brief Class to initialize the .net runtime.
 */
class DotNetRuntime
{
public:

    DotNetRuntime();
    ~DotNetRuntime();
    [[nodiscard]] bool is_initialized() const;

    void* get_method(const char_t* type_name, const char_t* method_name) const;

private:
    string_t dotnetlib_path;

    hostfxr_handle cxt_ = nullptr;
    hostfxr_initialize_for_runtime_config_fn init_runtime_cfg_fptr_ = nullptr;
    hostfxr_get_runtime_delegate_fn get_delegate_fptr_ = nullptr;
    hostfxr_close_fn close_fptr_ = nullptr;

    load_assembly_and_get_function_pointer_fn load_func = nullptr;

    bool is_initialized_ = false;

    bool load_hostfxr();

    static void* load_library(const char_t* path);
    static void* get_export(void* h, const char* name);

    // Load and initialize .NET Core and get desired function pointer for scenario
    load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(const char_t* config_path) const;
    get_function_pointer_fn get_function_pointer_function() const;
};

inline DotNetRuntime* g_dotnet = new DotNetRuntime();
