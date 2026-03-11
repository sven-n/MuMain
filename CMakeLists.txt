cmake_minimum_required(VERSION 3.25)

project(MuMain LANGUAGES CXX)

#Set global C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MU_ENABLE_DOTNET ".NET AOT library integration (disable for CI/rendering-only builds)" ON)

# ============================================================
# Story 4.3.1: SDL_gpu backend / OpenGL backend toggle.
# OFF (default): MuRendererSDLGpu is the active backend; GLEW is not linked.
# ON: MuRendererGL (legacy OpenGL/GLEW) backend is compiled and linked.
# ============================================================
option(MU_USE_OPENGL_BACKEND "Use legacy OpenGL/GLEW rendering backend instead of SDL_gpu" OFF)
if(MU_USE_OPENGL_BACKEND)
    add_compile_definitions(MU_USE_OPENGL_BACKEND)
endif()

# ============================================================
# Story 4.3.2: Shader Programs (HLSL + SDL_shadercross)
# MU_ENABLE_SHADER_COMPILATION=OFF (default): Copy pre-compiled blobs from
#   src/shaders/compiled/ to ${CMAKE_BINARY_DIR}/shaders/ at configure time.
#   CI-safe: no SDL_shadercross tool required.
# MU_ENABLE_SHADER_COMPILATION=ON: Compile HLSL shaders via SDL_shadercross.
#   Requires SDL_shadercross binary on PATH or findable via find_program.
#
# MU_SHADER_DIR is propagated to MuRendererSDLGpu.cpp via compile definition
# so runtime shader loading uses the correct blob directory.
# ============================================================
option(MU_ENABLE_SHADER_COMPILATION
    "Compile HLSL shaders via SDL_shadercross (requires tool on PATH; OFF for CI)" OFF)

set(MU_SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")
set(MU_SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders")
set(MU_SHADER_COMPILED_DIR "${MU_SHADER_SOURCE_DIR}/compiled")

# Always propagate the shader output directory path to C++ source.
# The path is used by LoadShaderBlob() in MuRendererSDLGpu.cpp to locate blobs.
add_compile_definitions(MU_SHADER_DIR="${MU_SHADER_OUTPUT_DIR}/")

# Create the output directory at configure time.
file(MAKE_DIRECTORY "${MU_SHADER_OUTPUT_DIR}")

if(MU_ENABLE_SHADER_COMPILATION)
    # Live shader compilation via SDL_shadercross.
    # Requires the tool to be installed (brew install sdl3-shadercross on macOS,
    # or built from source). Falls back gracefully: if not found, emit a warning
    # and skip compilation (the pre-compiled blobs in src/shaders/compiled/ are used).
    find_program(SDL_SHADERCROSS_EXE NAMES SDL_shadercross sdl_shadercross shadercross
        DOC "SDL_shadercross shader compilation tool")

    if(SDL_SHADERCROSS_EXE)
        message(STATUS "SDL_shadercross found: ${SDL_SHADERCROSS_EXE} — shader compilation enabled")

        # Helper function: add one compile command per shader × format combination.
        # Usage: compile_hlsl_shader(<name> <stage> <format> <output_blob>)
        function(compile_hlsl_shader shader_name stage fmt output_blob)
            set(src "${MU_SHADER_SOURCE_DIR}/${shader_name}.${stage}.hlsl")
            add_custom_command(
                OUTPUT "${output_blob}"
                COMMAND "${SDL_SHADERCROSS_EXE}"
                    --source "${src}"
                    --stage "${stage}"
                    --format "${fmt}"
                    --output "${output_blob}"
                DEPENDS "${src}"
                COMMENT "Compiling ${shader_name}.${stage}.hlsl -> ${fmt}"
                VERBATIM
            )
        endfunction()

        # Compile all 5 shaders × 3 formats = 15 blobs.
        set(MU_SHADER_BLOBS)
        foreach(shader IN ITEMS basic_textured basic_colored)
            foreach(stage IN ITEMS vert frag)
                foreach(fmt ext IN ITEMS spirv spv dxil dxil msl msl)
                    set(blob "${MU_SHADER_OUTPUT_DIR}/${shader}.${stage}.${ext}")
                    compile_hlsl_shader("${shader}" "${stage}" "${fmt}" "${blob}")
                    list(APPEND MU_SHADER_BLOBS "${blob}")
                endforeach()
            endforeach()
        endforeach()
        foreach(fmt ext IN ITEMS spirv spv dxil dxil msl msl)
            set(blob "${MU_SHADER_OUTPUT_DIR}/shadow_volume.vert.${ext}")
            compile_hlsl_shader("shadow_volume" "vert" "${fmt}" "${blob}")
            list(APPEND MU_SHADER_BLOBS "${blob}")
        endforeach()

        add_custom_target(ShaderCompilation ALL DEPENDS ${MU_SHADER_BLOBS})
    else()
        message(WARNING
            "MU_ENABLE_SHADER_COMPILATION=ON but SDL_shadercross not found. "
            "Falling back to pre-compiled blobs in src/shaders/compiled/.")
        set(MU_ENABLE_SHADER_COMPILATION OFF)
    endif()
endif()

if(NOT MU_ENABLE_SHADER_COMPILATION)
    # CI-safe fallback: copy pre-compiled blobs from src/shaders/compiled/ to
    # ${CMAKE_BINARY_DIR}/shaders/ at configure time. No tool dependency.
    file(GLOB MU_PRECOMPILED_BLOBS "${MU_SHADER_COMPILED_DIR}/*")
    # Story 4.3.2 (HIGH-1 fix): warn if pre-compiled blobs are all empty (0 bytes).
    # Empty blobs mean LoadShaderBlob() will fail at runtime and Init() will return false.
    # To fix: run SDL_shadercross to compile the HLSL shaders and replace the placeholder files
    # in src/shaders/compiled/, or enable MU_ENABLE_SHADER_COMPILATION on a capable machine.
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
            "All pre-compiled shader blobs in src/shaders/compiled/ are empty (0 bytes). "
            "LoadShaderBlob() will fail at runtime — renderer Init() will return false. "
            "To fix: compile HLSL shaders via SDL_shadercross and commit the blobs, "
            "or enable MU_ENABLE_SHADER_COMPILATION=ON on a machine with SDL_shadercross installed.")
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
    # Default to .dll when dotnet is disabled (CI MinGW cross-compile builds)
    add_compile_definitions(MU_DOTNET_LIB_EXT=".dll")
endif()

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

    add_custom_target(BuildDotNetAOT
        COMMAND "${DOTNETAOT_EXECUTABLE}" publish ${DOTNETAOT_PUBLISH_ARGS}
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
