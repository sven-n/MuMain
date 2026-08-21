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
    "Compile HLSL shaders via glslang+spirv-cross; OFF copies checked-in shader blobs" OFF)

include(FetchContent)

set(MU_SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")
set(MU_SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders")
set(MU_SHADER_COMPILED_DIR "${MU_SHADER_SOURCE_DIR}/compiled")
add_compile_definitions(MU_SHADER_DIR="${MU_SHADER_OUTPUT_DIR}/")
file(MAKE_DIRECTORY "${MU_SHADER_OUTPUT_DIR}")

if(MU_ENABLE_SHADER_COMPILATION)
    find_program(GLSLANG_EXE NAMES glslangValidator glslang
        DOC "glslang HLSL/GLSL to SPIR-V compiler")
    find_program(SPIRV_CROSS_EXE NAMES spirv-cross
        DOC "SPIR-V cross-compiler")

    if(NOT GLSLANG_EXE OR NOT SPIRV_CROSS_EXE)
        message(FATAL_ERROR
            "MU_ENABLE_SHADER_COMPILATION=ON requires glslangValidator and spirv-cross. "
            "Install them or configure with MU_ENABLE_SHADER_COMPILATION=OFF.")
    endif()

    function(mu_add_shader shader_name stage)
        set(src  "${MU_SHADER_SOURCE_DIR}/${shader_name}.${stage}.hlsl")
        set(spv  "${MU_SHADER_OUTPUT_DIR}/${shader_name}.${stage}.spv")
        set(msl  "${MU_SHADER_OUTPUT_DIR}/${shader_name}.${stage}.msl")
        set(dxil "${MU_SHADER_OUTPUT_DIR}/${shader_name}.${stage}.dxil")

        add_custom_command(
            OUTPUT "${spv}"
            COMMAND "${GLSLANG_EXE}" -V -D -S "${stage}" -e main -o "${spv}" "${src}"
            DEPENDS "${src}"
            COMMENT "HLSL to SPIR-V: ${shader_name}.${stage}.hlsl"
            VERBATIM
        )
        add_custom_command(
            OUTPUT "${msl}"
            COMMAND "${SPIRV_CROSS_EXE}" --msl "${spv}" --output "${msl}"
            DEPENDS "${spv}"
            COMMENT "SPIR-V to MSL: ${shader_name}.${stage}.spv"
            VERBATIM
        )
        add_custom_command(
            OUTPUT "${dxil}"
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
    mu_add_shader("skinned_textured" "vert")
    mu_add_shader("shadow_volume" "vert")
    add_custom_target(ShaderCompilation ALL DEPENDS ${MU_SHADER_BLOBS})
else()
    file(GLOB MU_PRECOMPILED_BLOBS CONFIGURE_DEPENDS "${MU_SHADER_COMPILED_DIR}/*")
    foreach(blob IN LISTS MU_PRECOMPILED_BLOBS)
        get_filename_component(blob_name "${blob}" NAME)
        configure_file("${blob}" "${MU_SHADER_OUTPUT_DIR}/${blob_name}" COPYONLY)
    endforeach()
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

if(MU_ENABLE_SHADER_COMPILATION AND TARGET ShaderCompilation)
    add_dependencies(Main ShaderCompilation)
endif()

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
