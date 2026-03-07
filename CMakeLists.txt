cmake_minimum_required(VERSION 3.25)

project(MuMain LANGUAGES CXX)

#Set global C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MU_ENABLE_DOTNET ".NET AOT library integration (disable for CI/rendering-only builds)" ON)

add_subdirectory("src")

# ============================================================
# .NET AOT Cross-Platform Library (EPIC-3, Story 3.1.1)
# ============================================================
if(MU_ENABLE_DOTNET)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    include(FindDotnetAOT)
    if(DOTNETAOT_FOUND)
        add_compile_definitions(MU_DOTNET_LIB_EXT="${MU_DOTNET_LIB_EXT}")

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
    else()
        add_compile_definitions(MU_DOTNET_LIB_EXT="${MU_DOTNET_LIB_EXT}")
    endif()
else()
    # Default to .dll when dotnet is disabled (CI MinGW cross-compile builds)
    add_compile_definitions(MU_DOTNET_LIB_EXT=".dll")
endif()

# ============================================================
# Unit tests (opt-in: -DBUILD_TESTING=ON)
# ============================================================
option(BUILD_TESTING "Build unit tests" OFF)
if(BUILD_TESTING)
  add_subdirectory("tests")
endif()
