cmake_minimum_required(VERSION 3.25)

project(MuMain LANGUAGES CXX)

#Set global C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MU_ENABLE_DOTNET ".NET AOT library integration (disable for CI/rendering-only builds)" ON)

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
