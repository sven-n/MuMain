cmake_minimum_required(VERSION 3.25)

project(MuMain LANGUAGES CXX)

#Set global C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MU_ENABLE_DOTNET ".NET AOT library integration (disable for CI/rendering-only builds)" ON)

# ============================================================
# Shader Programs: HLSL → SPIR-V (glslang) + SPIR-V → MSL (spirv-cross)
# MU_ENABLE_SHADER_COMPILATION=ON (default): Compile HLSL shaders at build time.
#   glslangValidator + spirv-cross are used if found on PATH; otherwise they are
#   fetched and built from source via FetchContent.
#   Install manually:  brew install glslang spirv-cross            (macOS)
#                      apt-get install glslang-tools spirv-cross   (Linux)
# MU_ENABLE_SHADER_COMPILATION=OFF: Copy pre-compiled blobs from
#   src/shaders/compiled/ at configure time (offline / MinGW builds).
#
# MU_SHADER_DIR is propagated to MuRendererSDLGpu.cpp via compile definition
# so runtime shader loading uses the correct blob directory.
# ============================================================
option(MU_ENABLE_SHADER_COMPILATION
    "Compile HLSL shaders via glslang+spirv-cross (ON by default; OFF for offline/MinGW builds)" ON)

set(MU_SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")
set(MU_SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders")
set(MU_SHADER_COMPILED_DIR "${MU_SHADER_SOURCE_DIR}/compiled")

# Always propagate the shader output directory path to C++ source.
# The path is used by LoadShaderBlob() in MuRendererSDLGpu.cpp to locate blobs.
add_compile_definitions(MU_SHADER_DIR="${MU_SHADER_OUTPUT_DIR}/")

# Create the output directory at configure time.
file(MAKE_DIRECTORY "${MU_SHADER_OUTPUT_DIR}")

if(MU_ENABLE_SHADER_COMPILATION)
    include(FetchContent)

    # -- glslangValidator: HLSL → SPIR-V --
    find_program(GLSLANG_EXE NAMES glslangValidator glslang
        DOC "glslang HLSL/GLSL to SPIR-V compiler")
    set(_MU_GLSLANG_EXTRA_DEP "")
    if(NOT GLSLANG_EXE)
        set(GLSLANG_FETCH_TAG "14.3.0" CACHE STRING "glslang git tag for FetchContent fallback")
        message(STATUS "glslangValidator not on PATH — fetching glslang ${GLSLANG_FETCH_TAG} via FetchContent")
        FetchContent_Declare(glslang
            GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
            GIT_TAG        "${GLSLANG_FETCH_TAG}"
            GIT_SHALLOW    TRUE
        )
        set(ENABLE_GLSLANG_BINARIES ON  CACHE BOOL "" FORCE)
        set(ENABLE_HLSL             ON  CACHE BOOL "" FORCE)
        set(ENABLE_OPT              OFF CACHE BOOL "" FORCE)
        set(BUILD_EXTERNAL          OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(glslang)
        set(GLSLANG_EXE "$<TARGET_FILE:glslangValidator>")
        set(_MU_GLSLANG_EXTRA_DEP "glslangValidator")
    else()
        message(STATUS "glslangValidator: ${GLSLANG_EXE}")
    endif()

    # -- spirv-cross: SPIR-V → MSL --
    find_program(SPIRV_CROSS_EXE NAMES spirv-cross
        DOC "SPIR-V cross-compiler (SPIR-V → MSL)")
    set(_MU_SPIRVCROSS_EXTRA_DEP "")
    if(NOT SPIRV_CROSS_EXE)
        set(SPIRVCROSS_FETCH_TAG "vulkan-sdk-1.3.290.0" CACHE STRING "SPIRV-Cross git tag for FetchContent fallback")
        message(STATUS "spirv-cross not on PATH — fetching SPIRV-Cross ${SPIRVCROSS_FETCH_TAG} via FetchContent")
        FetchContent_Declare(spirvcross
            GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross.git
            GIT_TAG        "${SPIRVCROSS_FETCH_TAG}"
            GIT_SHALLOW    TRUE
        )
        set(SPIRV_CROSS_CLI          ON  CACHE BOOL "" FORCE)
        set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(spirvcross)
        set(SPIRV_CROSS_EXE "$<TARGET_FILE:spirv-cross>")
        set(_MU_SPIRVCROSS_EXTRA_DEP "spirv-cross")
    else()
        message(STATUS "spirv-cross: ${SPIRV_CROSS_EXE}")
    endif()

    # Helper: compile one HLSL stage → SPIR-V + MSL + DXIL stub.
    # Appends output paths to MU_SHADER_BLOBS in the parent scope.
    function(mu_add_shader shader_name stage)
        set(src  "${MU_SHADER_SOURCE_DIR}/${shader_name}.${stage}.hlsl")
        set(spv  "${MU_SHADER_OUTPUT_DIR}/${shader_name}.${stage}.spv")
        set(msl  "${MU_SHADER_OUTPUT_DIR}/${shader_name}.${stage}.msl")
        set(dxil "${MU_SHADER_OUTPUT_DIR}/${shader_name}.${stage}.dxil")

        add_custom_command(
            OUTPUT  "${spv}"
            COMMAND "${GLSLANG_EXE}" -V -D -S "${stage}" -e main -o "${spv}" "${src}"
            DEPENDS "${src}" ${_MU_GLSLANG_EXTRA_DEP}
            COMMENT "HLSL→SPIR-V: ${shader_name}.${stage}.hlsl"
            VERBATIM
        )
        add_custom_command(
            OUTPUT  "${msl}"
            COMMAND "${SPIRV_CROSS_EXE}" --msl "${spv}" --output "${msl}"
            DEPENDS "${spv}" ${_MU_SPIRVCROSS_EXTRA_DEP}
            COMMENT "SPIR-V→MSL: ${shader_name}.${stage}.spv"
            VERBATIM
        )
        # DXIL stub — empty file (DX12 not yet implemented; MinGW target uses OpenGL)
        add_custom_command(
            OUTPUT  "${dxil}"
            COMMAND "${CMAKE_COMMAND}" -E touch "${dxil}"
            COMMENT "DXIL stub: ${shader_name}.${stage}.dxil"
            VERBATIM
        )
        set(MU_SHADER_BLOBS "${MU_SHADER_BLOBS}" "${spv}" "${msl}" "${dxil}" PARENT_SCOPE)
    endfunction()

    set(MU_SHADER_BLOBS)
    foreach(shader IN ITEMS basic_textured basic_colored)
        foreach(stage IN ITEMS vert frag)
            mu_add_shader("${shader}" "${stage}")
        endforeach()
    endforeach()
    mu_add_shader("shadow_volume" "vert")

    add_custom_target(ShaderCompilation ALL DEPENDS ${MU_SHADER_BLOBS})
endif()

if(NOT MU_ENABLE_SHADER_COMPILATION)
    # Offline fallback: copy pre-compiled blobs from src/shaders/compiled/ at configure time.
    file(GLOB MU_PRECOMPILED_BLOBS CONFIGURE_DEPENDS "${MU_SHADER_COMPILED_DIR}/*")
    set(MU_FOUND_NONEMPTY_BLOB FALSE)
    foreach(blob IN LISTS MU_PRECOMPILED_BLOBS)
        if(EXISTS "${blob}")
            file(SIZE "${blob}" blob_size)
            if(blob_size GREATER 0)
                set(MU_FOUND_NONEMPTY_BLOB TRUE)
                break()
            endif()
        endif()
    endforeach()
    if(NOT MU_FOUND_NONEMPTY_BLOB)
        message(WARNING
            "MU_ENABLE_SHADER_COMPILATION=OFF and all pre-compiled blobs in "
            "src/shaders/compiled/ are empty (0 bytes). "
            "The renderer will fail to initialize at runtime. "
            "To fix: enable MU_ENABLE_SHADER_COMPILATION=ON (the default) and install "
            "glslang + spirv-cross (brew install glslang spirv-cross / "
            "apt-get install glslang-tools spirv-cross).")
    endif()
    foreach(blob IN LISTS MU_PRECOMPILED_BLOBS)
        get_filename_component(blob_name "${blob}" NAME)
        configure_file("${blob}" "${MU_SHADER_OUTPUT_DIR}/${blob_name}" COPYONLY)
    endforeach()
    message(STATUS "Shader blobs: copied ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/compiled/ -> ${MU_SHADER_OUTPUT_DIR}/")
endif()

# ============================================================
# .NET AOT Cross-Platform Library (EPIC-3, Story 3.1.1)
# compile definitions MUST be set BEFORE add_subdirectory("src")
# so that Main, MUCore, and MUGame targets inherit them.
# (Story 3.3.2 fix H-1/H-2: add_compile_definitions after
#  add_subdirectory does NOT propagate to subdirectory targets.)
# ============================================================
if(MU_ENABLE_DOTNET)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    include(FindDotnetAOT)
    if(DOTNETAOT_FOUND)
        add_compile_definitions(MU_DOTNET_LIB_EXT="${MU_DOTNET_LIB_EXT}")

        # On UNIX (Linux + macOS), dlopen() with a bare filename does NOT search the
        # executable directory — Linux requires LD_LIBRARY_PATH or $ORIGIN RPATH, and
        # macOS @rpath resolution is install-dependent. Provide an absolute path via
        # MU_DOTNET_LIB_DIR so Connection.h can construct an absolute library path.
        # Story 3.3.2 (Linux) / 3.3.1 (macOS) — Risk R6 mitigation.
        # NOTE: $<TARGET_FILE_DIR:Main> is a generator expression evaluated at build
        # time — Main need not exist yet at configure time for this to be valid.
        if(UNIX)
            add_compile_definitions(MU_DOTNET_LIB_DIR="$<TARGET_FILE_DIR:Main>")
        endif()
    else()
        add_compile_definitions(MU_DOTNET_LIB_EXT="${MU_DOTNET_LIB_EXT}")
    endif()
else()
    # MU_ENABLE_DOTNET is OFF: legacy build path in src/CMakeLists.txt handles dotnet publish.
    # Detect extension from host OS so C++ code loads the correct filename at runtime.
    # MinGW cross-compile (CI) targets Windows and defaults to .dll.
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        add_compile_definitions(MU_DOTNET_LIB_EXT=".dylib")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        add_compile_definitions(MU_DOTNET_LIB_EXT=".so")
    else()
        add_compile_definitions(MU_DOTNET_LIB_EXT=".dll")
    endif()
endif()

# ============================================================
# GLM (OpenGL Mathematics) — header-only matrix math library
# Replaces hand-rolled mat4:: utilities for projection, view,
# and model transforms on SDL_GPU (Metal/Vulkan/D3D12).
# Story 7.9.7: Adopt GLM and Harden Renderer Matrix Pipeline
# ============================================================
include(FetchContent)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE
)
set(GLM_ENABLE_CXX_20 ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)

# ============================================================
# SDL_ttf 3.x — GPU-accelerated TrueType font rendering
# Provides TTF_CreateGPUTextEngine() for SDL_GPU text atlas rendering.
# Replaces the GDI TextOut→DIB→glTexSubImage2D pipeline with a native
# SDL_GPU text engine that manages glyph atlas textures internally.
# Story 7.9.8: Adopt SDL_ttf for Cross-Platform Font Rendering
# ============================================================
set(SDLTTF_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDLTTF_VENDORED ON CACHE BOOL "" FORCE)
FetchContent_Declare(SDL3_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG        release-3.2.2
    GIT_SHALLOW    TRUE
)
# FetchContent_MakeAvailable(SDL3_ttf) is called in src/CMakeLists.txt after SDL3
# is available, because SDL_ttf requires find_package(SDL3) at configure time.

add_subdirectory("src")

# Story 4.3.2: When SDL_shadercross compilation is enabled, the Main binary
# cannot be built without compiled shaders. Add the dependency after
# add_subdirectory("src") so the Main target is already defined.
if(MU_ENABLE_SHADER_COMPILATION AND TARGET ShaderCompilation)
    add_dependencies(Main ShaderCompilation)
endif()

# BuildDotNetAOT custom target — defined AFTER add_subdirectory("src") because
# it references $<TARGET_FILE_DIR:Main> (POST_BUILD copy), and add_custom_command
# POST_BUILD on BuildDotNetAOT also copies to $<TARGET_FILE_DIR:Main>.
# The compile definitions were already set above before add_subdirectory.
if(MU_ENABLE_DOTNET AND DOTNETAOT_FOUND)
    set(DOTNETAOT_PUBLISH_DIR "${CMAKE_BINARY_DIR}/dotnet-publish")

    set(DOTNETAOT_PUBLISH_ARGS
        "${DOTNETAOT_CSPROJ_PATH}"
        --runtime "${MU_DOTNET_RID}"
        --configuration Release
        --output "${DOTNETAOT_PUBLISH_DIR}"
    )
    if(MU_DOTNET_RID STREQUAL "win-x86")
        list(APPEND DOTNETAOT_PUBLISH_ARGS /p:Platform=x86 /p:PlatformTarget=x86)
    endif()

    # Inject LIBRARY_PATH so the AOT linker finds native libraries on macOS/Linux.
    # On macOS (Homebrew): include the Homebrew lib dir (brotli, etc.) and the keg-only
    # OpenSSL dir. On Linux: include the OpenSSL lib dir if outside the default search path.
    # OPENSSL_CRYPTO_LIBRARY is a CACHE variable set by find_package(OpenSSL) in src/CMakeLists.txt.
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(_DOTNETAOT_LIB_DIRS "")

        if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            # Homebrew main lib dir covers brotli, icu4c, and other native deps.
            find_program(_BREW_EXE brew DOC "Homebrew package manager")
            if(_BREW_EXE)
                execute_process(
                    COMMAND "${_BREW_EXE}" --prefix
                    OUTPUT_VARIABLE _HOMEBREW_PREFIX
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET
                )
                if(_HOMEBREW_PREFIX)
                    list(APPEND _DOTNETAOT_LIB_DIRS "${_HOMEBREW_PREFIX}/lib")
                endif()
            endif()
        endif()

        # OpenSSL may be keg-only (not in the Homebrew main lib dir) on macOS,
        # or outside the default search path on Linux.
        if(OPENSSL_CRYPTO_LIBRARY)
            get_filename_component(_DOTNETAOT_OPENSSL_LIB_DIR "${OPENSSL_CRYPTO_LIBRARY}" DIRECTORY)
            list(APPEND _DOTNETAOT_LIB_DIRS "${_DOTNETAOT_OPENSSL_LIB_DIR}")
        endif()

        if(_DOTNETAOT_LIB_DIRS)
            list(REMOVE_DUPLICATES _DOTNETAOT_LIB_DIRS)
            list(JOIN _DOTNETAOT_LIB_DIRS ":" _DOTNETAOT_LIBRARY_PATH_VAL)
            if(DEFINED ENV{LIBRARY_PATH})
                string(APPEND _DOTNETAOT_LIBRARY_PATH_VAL ":$ENV{LIBRARY_PATH}")
            endif()
            set(_DOTNETAOT_PUBLISH_COMMAND
                "${CMAKE_COMMAND}" -E env "LIBRARY_PATH=${_DOTNETAOT_LIBRARY_PATH_VAL}"
                "${DOTNETAOT_EXECUTABLE}" publish ${DOTNETAOT_PUBLISH_ARGS}
            )
        else()
            set(_DOTNETAOT_PUBLISH_COMMAND
                "${DOTNETAOT_EXECUTABLE}" publish ${DOTNETAOT_PUBLISH_ARGS}
            )
        endif()
    else()
        set(_DOTNETAOT_PUBLISH_COMMAND
            "${DOTNETAOT_EXECUTABLE}" publish ${DOTNETAOT_PUBLISH_ARGS}
        )
    endif()

    add_custom_target(BuildDotNetAOT
        COMMAND ${_DOTNETAOT_PUBLISH_COMMAND}
        COMMENT "Building .NET AOT library for RID: ${MU_DOTNET_RID}"
        VERBATIM
    )

    add_custom_command(
        TARGET BuildDotNetAOT POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${DOTNETAOT_PUBLISH_DIR}/MUnique.Client.Library${MU_DOTNET_LIB_EXT}"
            "$<TARGET_FILE_DIR:Main>/MUnique.Client.Library${MU_DOTNET_LIB_EXT}"
        COMMENT "Copying .NET AOT library to output directory"
        VERBATIM
    )

    # Build the .NET library whenever Main is built so the dylib is always present at runtime.
    add_dependencies(Main BuildDotNetAOT)
endif()

# ============================================================
# Unit tests (opt-in: -DBUILD_TESTING=ON)
# ============================================================
option(BUILD_TESTING "Build unit tests" OFF)
if(BUILD_TESTING)
  add_subdirectory("tests")
endif()

# ============================================================
# Ground Truth Capture (EPIC-4, Story 4.1.1)
# Windows-only, for capturing OpenGL baseline screenshots.
# NOT for release builds or CI. Enable manually on Windows:
#   cmake -DENABLE_GROUND_TRUTH_CAPTURE=ON ...
# ============================================================
option(ENABLE_GROUND_TRUTH_CAPTURE
    "Enable ground truth screenshot capture (Windows only, not for release)" OFF)
if(ENABLE_GROUND_TRUTH_CAPTURE)
    # MUPlatform owns GroundTruthCapture.cpp (Platform/ glob) and propagates the
    # define to all targets that link it (MUGame, Main, SceneManager instrumentation).
    target_compile_definitions(MUPlatform PUBLIC ENABLE_GROUND_TRUTH_CAPTURE)
endif()
