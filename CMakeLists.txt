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
        # SDL binds uniform buffer, texture and sampler slot N to MSL index N: keep the declared
        # bindings (spirv-cross otherwise numbers them in order of use). Not with storage buffers,
        # which share the MSL buffer indices and follow the uniform buffers.
        file(READ "${src}" shader_source)
        # Storage buffers may be declared in an included .hlsli.
        string(REGEX MATCHALL "#include \"[^\"]+\"" shader_includes "${shader_source}")
        foreach(shader_include IN LISTS shader_includes)
            string(REGEX REPLACE "#include \"([^\"]+)\"" "\\1" shader_include "${shader_include}")
            file(READ "${MU_SHADER_SOURCE_DIR}/${shader_include}" include_source)
            string(APPEND shader_source "${include_source}")
        endforeach()
        set(msl_flags)
        if(NOT shader_source MATCHES "StructuredBuffer|ByteAddressBuffer")
            set(msl_flags --msl-decoration-binding)
        endif()

        add_custom_command(
            OUTPUT "${spv}"
            COMMAND "${GLSLANG_EXE}" -V -D -S "${stage}" -e main -o "${spv}" "${src}"
            DEPENDS "${src}" ${MU_SHADER_INCLUDES} "${MU_SHADER_COMPILATION_STAMP}" "${GLSLANG_EXE}"
            COMMENT "HLSL to SPIR-V: ${shader_name}.${stage}.hlsl"
            VERBATIM
        )
        add_custom_command(
            OUTPUT "${msl}"
            COMMAND "${SPIRV_CROSS_EXE}" --msl ${msl_flags} "${spv}" --output "${msl}"
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

    file(GLOB MU_SHADER_INCLUDES CONFIGURE_DEPENDS "${MU_SHADER_SOURCE_DIR}/*.hlsli")
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
    # The time of unpacking, as for SDL3_ttf and SDL3_image below. The build reads only the copies of miniaudio's
    # files made below, which get the time they are written anyway, so here it only keeps every archive alike.
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
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
# UnpackArchive.cmake downloads with the configure's TLS and netrc settings, as ExternalProject does for SDL_ttf's
# own archive (e.g. CMAKE_TLS_CAINFO behind a proxy that inspects TLS).
set(MU_UNPACK_ARCHIVE_SETTINGS "")
foreach(setting IN ITEMS CMAKE_TLS_VERIFY CMAKE_TLS_CAINFO CMAKE_TLS_VERSION CMAKE_NETRC CMAKE_NETRC_FILE)
    if(DEFINED ${setting})
        list(APPEND MU_UNPACK_ARCHIVE_SETTINGS "-D${setting}=${${setting}}")
    endif()
endforeach()
# Each patch's SHA-256 is part of its patch command, so an edited patch changes the patch step's commands and the step
# runs again, from the archives' sources (below): in existing build folders and in CI's restored library caches too.
# An edited patch also runs the configure again.
foreach(mu_patch IN ITEMS
        sdl_ttf-3.2.2-batch-glyph-uploads sdl_ttf-3.2.2-fallback-baseline harfbuzz-8.5.0-no-meson-message)
    set(mu_patch_file "${CMAKE_SOURCE_DIR}/cmake/patches/${mu_patch}.patch")
    file(SHA256 "${mu_patch_file}" MU_PATCH_SHA256_${mu_patch})
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${mu_patch_file}")
endforeach()
# SDL_ttf comes as its release archive, which holds the sources of the commit release-3.2.2 points to (its .git-hash
# names a1ce3670) but none of the four libraries SDL_ttf vendors as git submodules. After SDL_ttf's own archive, four
# patch commands unpack an archive of each, at the commit release-3.2.2 pins it to (git ls-tree release-3.2.2
# external/), into external/, where the submodule would be: 39 MB to download and 121 MB unpacked, where the git clones
# with their history took 392 MB. The archives stay in the download folder beside SDL_ttf's, so a patch step that runs
# again unpacks them again without downloading. The hashes pin the sources. Updating SDL_ttf means updating those four
# commits and hashes too; tests/test_sdl_ttf_archives.cmake lists the commits of the release, and checks them and the
# order of the patch commands. The patches keep LF line endings (.gitattributes), as the archives do.
FetchContent_Declare(SDL3_ttf
    URL      https://github.com/libsdl-org/SDL_ttf/releases/download/release-3.2.2/SDL3_ttf-3.2.2.tar.gz
    URL_HASH SHA256=63547d58d0185c833213885b635a2c0548201cc8f301e6587c0be1a67e1e045d
    # The unpacked files get the time of unpacking, as UnpackArchive.cmake's do, not the archive's older dates: with
    # those, a build after a version change would keep objects compiled from the previous sources. Every archive here
    # does the same; tests/test_archive_extract_times.cmake checks it.
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
    # Every patch step starts from the archives' sources: SDL_ttf's own archive, which the download step keeps, is
    # unpacked again first (external/ goes with it), so an edited patch never lands on top of its previous version.
    PATCH_COMMAND
        "${CMAKE_COMMAND}"
        "-DARCHIVE=<DOWNLOAD_DIR>/SDL3_ttf-3.2.2.tar.gz"
        "-DDESTINATION=<SOURCE_DIR>"
        -P "${CMAKE_SOURCE_DIR}/cmake/UnpackArchive.cmake"
    COMMAND
        "${CMAKE_COMMAND}"
        ${MU_UNPACK_ARCHIVE_SETTINGS}
        "-DURL=https://github.com/libsdl-org/freetype/archive/9973564cfa63763a3e4ac67c09147899539b1e07.tar.gz"
        "-DSHA256=026a05a49d114a1235d2926f4c03a9330e4b1a6efe7c217ec9607904c32907d4"
        "-DARCHIVE_DIR=<DOWNLOAD_DIR>"
        "-DDESTINATION=<SOURCE_DIR>/external/freetype"
        -P "${CMAKE_SOURCE_DIR}/cmake/UnpackArchive.cmake"
    COMMAND
        "${CMAKE_COMMAND}"
        ${MU_UNPACK_ARCHIVE_SETTINGS}
        "-DURL=https://github.com/libsdl-org/harfbuzz/archive/564bf9818a18709776856533829c0c04950773d6.tar.gz"
        "-DSHA256=a448dd6c22d8e1e1cf39438c662251c1f97f810b8780eed4a6d6ada948c99ddc"
        "-DARCHIVE_DIR=<DOWNLOAD_DIR>"
        "-DDESTINATION=<SOURCE_DIR>/external/harfbuzz"
        -P "${CMAKE_SOURCE_DIR}/cmake/UnpackArchive.cmake"
    COMMAND
        "${CMAKE_COMMAND}"
        ${MU_UNPACK_ARCHIVE_SETTINGS}
        "-DURL=https://github.com/libsdl-org/plutosvg/archive/2983eb6919feea272d793bc386384e3f5b97b03c.tar.gz"
        "-DSHA256=cd416a2bbc63f5ca9c7a3582ce0c06e69e15b6087aef991cf36c38f68851dc17"
        "-DARCHIVE_DIR=<DOWNLOAD_DIR>"
        "-DDESTINATION=<SOURCE_DIR>/external/plutosvg"
        -P "${CMAKE_SOURCE_DIR}/cmake/UnpackArchive.cmake"
    COMMAND
        "${CMAKE_COMMAND}"
        ${MU_UNPACK_ARCHIVE_SETTINGS}
        "-DURL=https://github.com/libsdl-org/plutovg/archive/3e6f922f453da1c9e7d1d7f66cac1d9724a18b47.tar.gz"
        "-DSHA256=491ee12761688a61f98bf956fa858d4610d106ade950e0530f5b19ce195d0164"
        "-DARCHIVE_DIR=<DOWNLOAD_DIR>"
        "-DDESTINATION=<SOURCE_DIR>/external/plutovg"
        -P "${CMAKE_SOURCE_DIR}/cmake/UnpackArchive.cmake"
    COMMAND
        "${CMAKE_COMMAND}"
        ${MU_APPLY_GIT_PATCH_GIT}
        "-DSOURCE_DIR=<SOURCE_DIR>"
        "-DPATCH_FILE=${CMAKE_SOURCE_DIR}/cmake/patches/sdl_ttf-3.2.2-batch-glyph-uploads.patch"
        "-DPATCH_SHA256=${MU_PATCH_SHA256_sdl_ttf-3.2.2-batch-glyph-uploads}"
        -P "${CMAKE_SOURCE_DIR}/cmake/ApplyGitPatch.cmake"
    COMMAND
        "${CMAKE_COMMAND}"
        ${MU_APPLY_GIT_PATCH_GIT}
        "-DSOURCE_DIR=<SOURCE_DIR>"
        "-DPATCH_FILE=${CMAKE_SOURCE_DIR}/cmake/patches/sdl_ttf-3.2.2-fallback-baseline.patch"
        "-DPATCH_SHA256=${MU_PATCH_SHA256_sdl_ttf-3.2.2-fallback-baseline}"
        -P "${CMAKE_SOURCE_DIR}/cmake/ApplyGitPatch.cmake"
    # The vendored HarfBuzz printed "WARNHarfBuzz has a Meson port ..." in every configure: a message(WARN) that asks
    # its users to build with Meson. SDL_ttf builds it with CMake, so the message goes.
    COMMAND
        "${CMAKE_COMMAND}"
        ${MU_APPLY_GIT_PATCH_GIT}
        "-DSOURCE_DIR=<SOURCE_DIR>/external/harfbuzz"
        "-DPATCH_FILE=${CMAKE_SOURCE_DIR}/cmake/patches/harfbuzz-8.5.0-no-meson-message.patch"
        "-DPATCH_SHA256=${MU_PATCH_SHA256_harfbuzz-8.5.0-no-meson-message}"
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
set(SDLIMAGE_TIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_WEBP OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_SVG OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_BACKEND_IMAGEIO OFF CACHE BOOL "" FORCE)
# PNG and JPEG load through stb_image, not the vendored libpng, jpeg and zlib, and JXL, which would build libjxl,
# stays off. Both are SDL_image's defaults, set here because nothing below fetches its vendored libraries.
set(SDLIMAGE_BACKEND_STB ON CACHE BOOL "" FORCE)
set(SDLIMAGE_JXL OFF CACHE BOOL "" FORCE)
# With these settings no part of SDL_image builds from its vendored libraries (aom, dav1d, libavif, libjxl, libpng,
# jpeg, libtiff, libwebp, zlib), so SDL_image comes as its release archive, which leaves them out: 11 MB to download
# and 36 MB unpacked, where the tag's git clone took 84 MB and, with its submodules, 1.1 GB. The hash pins the
# sources, which a tag does not. To build a format that needs a vendored library, fetch SDL_image with it first (e.g.
# the tag's git clone with that submodule); tests/test_sdl_image_archive.cmake checks these settings.
FetchContent_Declare(SDL3_image
    URL      https://github.com/libsdl-org/SDL_image/releases/download/release-3.2.4/SDL3_image-3.2.4.tar.gz
    URL_HASH SHA256=a725bd6d04261fdda0dd8d950659e1dc15a8065d025275ef460d32ae7dcfc182
    # The time of unpacking, as for SDL3_ttf above: SDL_image compiles straight from the unpacked tree.
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
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
