cmake_minimum_required(VERSION 3.25)

# Each static library once on a link line where the linker needs no repeats, e.g. Apple's ld, which warns about
# duplicates (CMake 3.29+; src/CMakeLists.txt sets the same after its own cmake_minimum_required).
foreach(policy IN ITEMS CMP0156 CMP0179)
    if(POLICY ${policy})
        cmake_policy(SET ${policy} NEW)
    endif()
endforeach()

project(MuMain LANGUAGES CXX)

if(MINGW)
    add_compile_options(-Wa,-mbig-obj)
endif()

#Set global C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTING "Build unit tests" OFF)
option(MU_ENABLE_SHADER_COMPILATION
    "Compile HLSL shaders via glslang, SPIRV-Cross, and DXC; OFF copies checked-in shader blobs" OFF)
option(MU_WARNINGS_AS_ERRORS
    "Treat warnings as errors in the game's own code, its tests and DXC's shader compiles (CI turns this on)" OFF)
# CMake turns linker warnings into errors (LINK_WARNING_AS_ERROR) from 4.0 on and ignores the setting before.
if(MU_WARNINGS_AS_ERRORS AND CMAKE_VERSION VERSION_LESS 4.0)
    message(WARNING "MU_WARNINGS_AS_ERRORS makes linker warnings errors only with CMake 4.0 or newer; "
        "CMake ${CMAKE_VERSION} keeps them warnings, so a build can pass here and fail in CI.")
endif()

include(FetchContent)

set(MU_SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")
set(MU_SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders")
set(MU_SHADER_COMPILED_DIR "${MU_SHADER_SOURCE_DIR}/compiled")
set(MU_SHADER_VALIDATION_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ValidateShaderBlobs.cmake")
set(MU_SHADER_COMPILATION_STAMP "${CMAKE_BINARY_DIR}/shader-compilation.config")
add_compile_definitions(MU_SHADER_DIR="${MU_SHADER_OUTPUT_DIR}/")
file(MAKE_DIRECTORY "${MU_SHADER_OUTPUT_DIR}")

set(MU_SHADER_BLOB_DIR "${MU_SHADER_COMPILED_DIR}")
include("${MU_SHADER_VALIDATION_SCRIPT}")
unset(MU_SHADER_BLOB_DIR)

if(MU_ENABLE_SHADER_COMPILATION)
    foreach(shader_tool_variable IN ITEMS GLSLANG_EXE SPIRV_CROSS_EXE DXC_EXE)
        if(DEFINED ${shader_tool_variable}
                AND (NOT EXISTS "${${shader_tool_variable}}"
                    OR IS_DIRECTORY "${${shader_tool_variable}}"))
            unset(${shader_tool_variable} CACHE)
            unset(${shader_tool_variable})
        endif()
    endforeach()

    find_program(GLSLANG_EXE NAMES glslangValidator glslang
        DOC "glslang HLSL/GLSL to SPIR-V compiler")
    find_program(SPIRV_CROSS_EXE NAMES spirv-cross
        DOC "SPIR-V cross-compiler")
    find_program(DXC_EXE NAMES dxc dxc.exe
        DOC "DirectX Shader Compiler")

    if(WIN32 AND NOT DXC_EXE AND DEFINED ENV{WindowsSdkDir})
        file(TO_CMAKE_PATH "$ENV{WindowsSdkDir}" windows_sdk_dir)
        if(DEFINED ENV{WindowsSDKVersion})
            set(windows_sdk_version "$ENV{WindowsSDKVersion}")
        elseif(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
            set(windows_sdk_version "${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
        endif()
        string(REGEX REPLACE "[/\\\\]+$" "" windows_sdk_version "${windows_sdk_version}")
        if(windows_sdk_version)
            unset(DXC_EXE CACHE)
            unset(DXC_EXE)
            find_program(DXC_EXE NAMES dxc.exe
                HINTS "${windows_sdk_dir}/bin/${windows_sdk_version}/x64"
                NO_DEFAULT_PATH
                DOC "DirectX Shader Compiler")
        endif()
    endif()

    if(NOT GLSLANG_EXE
            OR NOT EXISTS "${GLSLANG_EXE}"
            OR IS_DIRECTORY "${GLSLANG_EXE}"
            OR NOT SPIRV_CROSS_EXE
            OR NOT EXISTS "${SPIRV_CROSS_EXE}"
            OR IS_DIRECTORY "${SPIRV_CROSS_EXE}"
            OR NOT DXC_EXE
            OR NOT EXISTS "${DXC_EXE}"
            OR IS_DIRECTORY "${DXC_EXE}")
        message(FATAL_ERROR
            "MU_ENABLE_SHADER_COMPILATION=ON requires glslangValidator, spirv-cross, and dxc. "
            "Install them or configure with MU_ENABLE_SHADER_COMPILATION=OFF.")
    endif()

    file(CONFIGURE
        OUTPUT "${MU_SHADER_COMPILATION_STAMP}"
        CONTENT
            "mode=ON\nGLSLANG_EXE=${GLSLANG_EXE}\nSPIRV_CROSS_EXE=${SPIRV_CROSS_EXE}\nDXC_EXE=${DXC_EXE}\n"
        NEWLINE_STYLE UNIX)

    # Shared code the shaders #include; a change rebuilds every shader.
    file(GLOB MU_SHADER_INCLUDES CONFIGURE_DEPENDS "${MU_SHADER_SOURCE_DIR}/*.hlsli")

    # A list, not a generator expression: an empty one adds no argument to DXC's command line, where an empty
    # generator expression would pass "".
    set(MU_DXC_WARNING_FLAGS)
    if(MU_WARNINGS_AS_ERRORS)
        set(MU_DXC_WARNING_FLAGS -WX)
    endif()

    function(mu_add_shader shader_name stage)
        if(stage STREQUAL "vert")
            set(dxil_profile vs_6_0)
        elseif(stage STREQUAL "frag")
            set(dxil_profile ps_6_0)
        else()
            message(FATAL_ERROR "Unsupported shader stage: ${stage}")
        endif()

        set(src  "${MU_SHADER_SOURCE_DIR}/${shader_name}.${stage}.hlsl")
        set(spv  "${MU_SHADER_OUTPUT_DIR}/${shader_name}.${stage}.spv")
        set(msl  "${MU_SHADER_OUTPUT_DIR}/${shader_name}.${stage}.msl")
        set(dxil "${MU_SHADER_OUTPUT_DIR}/${shader_name}.${stage}.dxil")

        add_custom_command(
            OUTPUT "${spv}"
            COMMAND "${GLSLANG_EXE}" -V -D -S "${stage}" -e main -o "${spv}" "${src}"
            DEPENDS "${src}" ${MU_SHADER_INCLUDES} "${MU_SHADER_COMPILATION_STAMP}" "${GLSLANG_EXE}"
            COMMENT "HLSL to SPIR-V: ${shader_name}.${stage}.hlsl"
            VERBATIM
        )
        add_custom_command(
            OUTPUT "${msl}"
            COMMAND "${SPIRV_CROSS_EXE}" --msl "${spv}" --output "${msl}"
            DEPENDS "${spv}" "${MU_SHADER_COMPILATION_STAMP}" "${SPIRV_CROSS_EXE}"
            COMMENT "SPIR-V to MSL: ${shader_name}.${stage}.spv"
            VERBATIM
        )
        add_custom_command(
            OUTPUT "${dxil}"
            COMMAND "${DXC_EXE}" -T "${dxil_profile}" -E main ${MU_DXC_WARNING_FLAGS}
                -D MU_D3D12_RAW_STORAGE_BUFFER=1 -Fo "${dxil}" "${src}"
            DEPENDS "${src}" ${MU_SHADER_INCLUDES} "${MU_SHADER_COMPILATION_STAMP}" "${DXC_EXE}"
            COMMENT "HLSL to DXIL: ${shader_name}.${stage}.hlsl"
            VERBATIM
        )
        set(MU_SHADER_BLOBS "${MU_SHADER_BLOBS}" "${spv}" "${msl}" "${dxil}" PARENT_SCOPE)
    endfunction()

    set(MU_SHADER_BLOBS)
    foreach(shader_stage IN LISTS MU_SHADER_STAGE_NAMES)
        string(REGEX REPLACE "\\.[^.]+$" "" shader_name "${shader_stage}")
        string(REGEX REPLACE "^.*\\." "" stage "${shader_stage}")
        mu_add_shader("${shader_name}" "${stage}")
    endforeach()
    add_custom_target(ShaderCompilation ALL DEPENDS ${MU_SHADER_BLOBS})
else()
    file(CONFIGURE
        OUTPUT "${MU_SHADER_COMPILATION_STAMP}"
        CONTENT "mode=OFF\n"
        NEWLINE_STYLE UNIX)

    set(MU_SHADER_BLOBS)
    foreach(blob_name IN LISTS MU_SHADER_BLOB_NAMES)
        set(source_blob "${MU_SHADER_COMPILED_DIR}/${blob_name}")
        set(staged_blob "${MU_SHADER_OUTPUT_DIR}/${blob_name}")
        configure_file("${source_blob}" "${staged_blob}" COPYONLY)
        list(APPEND MU_SHADER_BLOBS "${staged_blob}")
    endforeach()
endif()

add_custom_target(ValidateStagedShaderBlobs ALL
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_SHADER_BLOB_DIR=${MU_SHADER_OUTPUT_DIR}"
        -P "${MU_SHADER_VALIDATION_SCRIPT}"
    DEPENDS ${MU_SHADER_BLOBS} "${MU_SHADER_VALIDATION_SCRIPT}"
    COMMENT "Validating staged shader blobs"
    VERBATIM
)

if(MU_ENABLE_SHADER_COMPILATION)
    add_custom_target(ValidateStagedDxilReflection ALL
        COMMAND "${CMAKE_COMMAND}"
            "-DMU_SHADER_BLOB_DIR=${MU_SHADER_OUTPUT_DIR}"
            "-DDXC_EXE=${DXC_EXE}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ValidateDxilReflection.cmake"
        DEPENDS ${MU_SHADER_BLOBS} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ValidateDxilReflection.cmake"
        COMMENT "Validating staged DXIL resource bindings"
        VERBATIM
    )
endif()

# glm is header-only. Its own CMakeLists.txt asks for CMake 3.6, which CMake 4 warns is deprecated, and only builds
# an empty library around the headers, so the game takes the headers alone.
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  _fetchcontent_only
)
FetchContent_MakeAvailable(glm)
add_library(glm INTERFACE)
add_library(glm::glm ALIAS glm)
target_include_directories(glm SYSTEM INTERFACE "${glm_SOURCE_DIR}")

set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.17.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(miniaudio
    URL https://github.com/mackron/miniaudio/archive/9634bedb5b5a2ca38c1ee7108a9358a4e233f14d.tar.gz
    URL_HASH SHA256=1a3a79b80fc6f0b0cc155e28b954a598e0ddfa2db64e2afa8466be88c476fa55
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    SOURCE_SUBDIR _fetchcontent_only
)
FetchContent_MakeAvailable(miniaudio)

# Without this patch a failed music load can hang the next one. MuClient includes only the patched
# copy: the fetched tree stays off the include path, because its extras include the unpatched header
# by relative path. MiniAudioImpl.cpp also needs extras/stb_vorbis.c, which does not include
# miniaudio.h, so it is copied next to the patched header. MiniAudioImpl.cpp stops the build if the
# patch is missing.
include("${CMAKE_SOURCE_DIR}/cmake/PatchedCopy.cmake")
set(MU_MINIAUDIO_INCLUDE_DIR "${CMAKE_BINARY_DIR}/miniaudio-patched")
mu_patched_copy(
    SOURCE "${miniaudio_SOURCE_DIR}/miniaudio.h"
    PATCH "${CMAKE_SOURCE_DIR}/cmake/patches/miniaudio-0.11.25-stream-load-signal-order.patch"
    OUTPUT_DIR "${MU_MINIAUDIO_INCLUDE_DIR}"
    MARKER "MU_MINIAUDIO_STREAM_LOAD_PATCHED"
)
configure_file("${miniaudio_SOURCE_DIR}/extras/stb_vorbis.c" "${MU_MINIAUDIO_INCLUDE_DIR}/stb_vorbis.c" COPYONLY)
# configure_file() does not rerun configure when its output is deleted; this does.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${MU_MINIAUDIO_INCLUDE_DIR}/stb_vorbis.c")

set(SDLTTF_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDLTTF_VENDORED ON CACHE BOOL "" FORCE)
# ApplyGitPatch.cmake finds git on its own; pass a configured one so a git outside PATH works too.
set(MU_APPLY_GIT_PATCH_GIT "")
if(GIT_EXECUTABLE)
    set(MU_APPLY_GIT_PATCH_GIT "-DGIT_EXECUTABLE=${GIT_EXECUTABLE}")
endif()
FetchContent_Declare(SDL3_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG        a1ce3670aec736ecbf0936c43f2f0cc53aa61e5b
    GIT_SHALLOW    FALSE
    PATCH_COMMAND
        "${CMAKE_COMMAND}"
        ${MU_APPLY_GIT_PATCH_GIT}
        "-DSOURCE_DIR=<SOURCE_DIR>"
        "-DPATCH_FILE=${CMAKE_SOURCE_DIR}/cmake/patches/sdl_ttf-3.2.2-batch-glyph-uploads.patch"
        -P "${CMAKE_SOURCE_DIR}/cmake/ApplyGitPatch.cmake"
    COMMAND
        "${CMAKE_COMMAND}"
        ${MU_APPLY_GIT_PATCH_GIT}
        "-DSOURCE_DIR=<SOURCE_DIR>"
        "-DPATCH_FILE=${CMAKE_SOURCE_DIR}/cmake/patches/sdl_ttf-3.2.2-fallback-baseline.patch"
        -P "${CMAKE_SOURCE_DIR}/cmake/ApplyGitPatch.cmake"
    # The vendored HarfBuzz printed "WARNHarfBuzz has a Meson port ..." in every configure: a message(WARN) that asks
    # its users to build with Meson. SDL_ttf builds it with CMake, so the message goes.
    COMMAND
        "${CMAKE_COMMAND}"
        ${MU_APPLY_GIT_PATCH_GIT}
        "-DSOURCE_DIR=<SOURCE_DIR>/external/harfbuzz"
        "-DPATCH_FILE=${CMAKE_SOURCE_DIR}/cmake/patches/harfbuzz-8.5.0-no-meson-message.patch"
        -P "${CMAKE_SOURCE_DIR}/cmake/ApplyGitPatch.cmake"
)

# RmlUi port: RmlUi's own vendored SDL_GPU renderer backend (ThirdParty/RmlUi/Backends/
# RmlUi_Renderer_SDL_GPU.cpp) includes SDL3_image/SDL_image.h for its stock LoadTexture -- this
# engine's RmlUiRenderInterface overrides that method to route through CGlobalBitmap instead
# (see its own comment), so SDL_image is never actually called at runtime, but the base class's
# unused method body still has to compile. Same FetchContent pattern as SDL3_ttf above rather
# than vcpkg (vcpkg.json is scoped to Windows MSVC native deps; every other SDL-family library
# here -- SDL3 itself, SDL3_ttf -- is a submodule/FetchContent, not vcpkg).
set(SDLIMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_VENDORED ON CACHE BOOL "" FORCE)
# AVIF's vendored dav1d decoder needs a NASM assembler this machine doesn't have. Nothing here
# ever actually calls RenderInterface_SDL_GPU's stock LoadTexture (see its own comment) --
# disabling formats we'll never load avoids requiring build tools purely to satisfy an unused
# code path's link-time dependencies.
set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
# For the same reason: the vendored libtiff and libwebp ask for a CMake older than CMake 4 supports without a
# deprecation warning, the SVG loader redefines strstr, and macOS's ImageIO loader uses deprecated UTType constants.
# PNG and JPEG still load through stb_image.
set(SDLIMAGE_TIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_WEBP OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_SVG OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_BACKEND_IMAGEIO OFF CACHE BOOL "" FORCE)
# SDL_image's own defaults, set here too because the submodules below are not fetched: without stb_image, PNG and
# JPEG would build from the vendored libpng, jpeg and zlib, and JXL from libjxl.
set(SDLIMAGE_BACKEND_STB ON CACHE BOOL "" FORCE)
set(SDLIMAGE_JXL OFF CACHE BOOL "" FORCE)
# With those formats off, no part of SDL_image builds from its git submodules (aom, dav1d, libavif, libjxl, libpng,
# jpeg, libtiff, libwebp, zlib): stb_image covers PNG and JPEG without libpng, jpeg or zlib. Cloned with their whole
# history, they were about 1 GB of each build folder's 1.1 GB of SDL_image, so they are not fetched.
FetchContent_Declare(SDL3_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG        release-3.2.4
    GIT_SHALLOW    TRUE
    GIT_SUBMODULES ""
)

add_subdirectory("src")

add_dependencies(Main ValidateStagedShaderBlobs)
if(TARGET ValidateStagedDxilReflection)
    add_dependencies(Main ValidateStagedDxilReflection)
endif()

if(BUILD_TESTING)
    enable_testing()
    # The tests are the game's own code too (src/CMakeLists.txt sets the same before its first own target).
    set(CMAKE_COMPILE_WARNING_AS_ERROR ${MU_WARNINGS_AS_ERRORS})
    set(CMAKE_LINK_WARNING_AS_ERROR ${MU_WARNINGS_AS_ERRORS})
    add_subdirectory(tests)
endif()
