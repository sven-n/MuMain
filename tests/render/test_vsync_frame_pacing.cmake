foreach(required_file IN ITEMS RENDERER_HEADER RENDERER_SOURCE RENDER_UTIL_SOURCE WINMAIN_SOURCE CONSOLE_SOURCE SRC_CMAKE)
    if(NOT EXISTS "${${required_file}}")
        message(FATAL_ERROR "VSync frame-pacing contract missing ${required_file}: ${${required_file}}")
    endif()
endforeach()

file(READ "${RENDERER_HEADER}" renderer_header)
file(READ "${RENDERER_SOURCE}" renderer_source)
file(READ "${RENDER_UTIL_SOURCE}" render_util_source)
file(READ "${WINMAIN_SOURCE}" winmain_source)
file(READ "${CONSOLE_SOURCE}" console_source)
file(READ "${SRC_CMAKE}" src_cmake)

if(NOT renderer_header MATCHES "SetVSyncEnabled\\(bool")
    message(FATAL_ERROR "Renderer contract must expose VSync present-mode control")
endif()
if(NOT renderer_source MATCHES "SDL_SetGPUSwapchainParameters\\(")
    message(FATAL_ERROR "SDL GPU backend must apply VSync through the swapchain")
endif()
if(NOT renderer_source MATCHES "SDL_GPU_PRESENTMODE_VSYNC" OR
   NOT renderer_source MATCHES "SDL_GPU_PRESENTMODE_IMMEDIATE")
    message(FATAL_ERROR "SDL GPU backend must support VSync and immediate present modes")
endif()
if(NOT render_util_source MATCHES "bool EnableVSync\\(\\)" OR
   NOT render_util_source MATCHES "bool DisableVSync\\(\\)")
    message(FATAL_ERROR "VSync wrappers must report whether the renderer accepted the request")
endif()
if(NOT render_util_source MATCHES "SetVSyncEnabled\\(true\\)" OR
   NOT render_util_source MATCHES "SetVSyncEnabled\\(false\\)")
    message(FATAL_ERROR "VSync wrappers must delegate to the renderer")
endif()
string(REGEX MATCH
    "InitVSync\\(\\);[ \t\r\n]*if \\(IsVSyncAvailable\\(\\)\\)[ \t\r\n]*\\{[ \t\r\n]*if \\(EnableVSync\\(\\)\\)\\{?[^}]*SetTargetFps\\(-1\\);[^}]*\\}[ \t\r\n]*else[ \t\r\n]*\\{[^}]*SetTargetFps\\(GetFPSLimit\\(\\)\\);[^}]*\\}[ \t\r\n]*\\}[ \t\r\n]*else[ \t\r\n]*\\{[^}]*SetTargetFps\\(GetFPSLimit\\(\\)\\);"
    startup_vsync_block "${winmain_source}")
if(startup_vsync_block STREQUAL "")
    message(FATAL_ERROR "Startup must cap to the display refresh rate when VSync fails")
endif()

string(REGEX MATCH
    "strCommand.compare\\(L\"\\$vsync on\"\\) == 0\\)[ \t\r\n]*\\{[ \t\r\n]*if \\(EnableVSync\\(\\)\\)\\{?[^}]*SetTargetFps\\(-1\\);[^}]*\\}[ \t\r\n]*else[ \t\r\n]*\\{[^}]*SetTargetFps\\(GetFPSLimit\\(\\)\\);"
    console_vsync_on_block "${console_source}")
if(console_vsync_on_block STREQUAL "")
    message(FATAL_ERROR "$vsync on must cap to the display refresh rate when VSync fails")
endif()

string(REGEX MATCH
    "strCommand.compare\\(L\"\\$vsync off\"\\) == 0\\)[ \t\r\n]*\\{[ \t\r\n]*DisableVSync\\(\\);[ \t\r\n]*SetTargetFps\\(-1\\);"
    console_vsync_off_block "${console_source}")
if(console_vsync_off_block STREQUAL "")
    message(FATAL_ERROR "$vsync off must explicitly remove the software frame cap")
endif()

string(REGEX MATCHALL "-p:IlcUseEnvironmentalTools=true" windows_aot_args "${src_cmake}")
list(LENGTH windows_aot_args windows_aot_arg_count)
if(NOT windows_aot_arg_count EQUAL 2)
    message(FATAL_ERROR "Windows x86 and x64 Native AOT builds must use environmental tools")
endif()
