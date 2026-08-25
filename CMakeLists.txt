cmake_minimum_required(VERSION 3.25)

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
            DEPENDS "${src}" "${MU_SHADER_COMPILATION_STAMP}" "${GLSLANG_EXE}"
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
            COMMAND "${DXC_EXE}" -T "${dxil_profile}" -E main -Fo "${dxil}" "${src}"
            DEPENDS "${src}" "${MU_SHADER_COMPILATION_STAMP}" "${DXC_EXE}"
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

FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE
)
set(GLM_ENABLE_CXX_20 ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)

set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.15.3
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

set(SDLTTF_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDLTTF_VENDORED ON CACHE BOOL "" FORCE)
FetchContent_Declare(SDL3_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG        release-3.2.2
    GIT_SHALLOW    TRUE
)

add_subdirectory("src")

add_dependencies(Main ValidateStagedShaderBlobs)
if(TARGET ValidateStagedDxilReflection)
    add_dependencies(Main ValidateStagedDxilReflection)
endif()

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
