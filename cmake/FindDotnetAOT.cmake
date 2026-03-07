# FindDotnetAOT.cmake
# Flow Code: VS1-NET-CMAKE-RID
# Story: 3.1.1 - CMake RID Detection & .NET AOT Build Integration
#
# Detects the current platform Runtime Identifier (RID) for .NET AOT builds.
# Sets MU_DOTNET_RID and MU_DOTNET_LIB_EXT cache variables, locates the
# dotnet executable (including WSL interop), and sets DOTNETAOT_FOUND.
#
# Also defines DOTNETAOT_CSPROJ_PATH (WSL-converted path when needed) so that
# the calling CMakeLists.txt can create the BuildDotNetAOT custom target.
#
# Output variables:
#   DOTNETAOT_FOUND        - TRUE if dotnet executable was found
#   MU_DOTNET_RID          - .NET Runtime Identifier for current platform
#   MU_DOTNET_LIB_EXT      - Library extension (.dll / .dylib / .so)
#   DOTNETAOT_EXECUTABLE   - Full path to dotnet or dotnet.exe
#   DOTNETAOT_CSPROJ_PATH  - Path to MUnique.Client.Library.csproj (Windows-format under WSL)
#   MU_IS_WSL              - TRUE when running inside WSL

# ============================================================
# Step 1: Platform RID and library extension detection
# ============================================================

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(MU_DOTNET_LIB_EXT ".dll")
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(MU_DOTNET_RID "win-x86")
    else()
        set(MU_DOTNET_RID "win-x64")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(MU_DOTNET_LIB_EXT ".dylib")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(MU_DOTNET_RID "osx-arm64")
    else()
        set(MU_DOTNET_RID "osx-x64")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(MU_DOTNET_LIB_EXT ".so")
    set(MU_DOTNET_RID "linux-x64")
else()
    message(WARNING "PLAT: FindDotnetAOT — unrecognized platform: ${CMAKE_SYSTEM_NAME}. Defaulting to linux-x64.")
    set(MU_DOTNET_LIB_EXT ".so")
    set(MU_DOTNET_RID "linux-x64")
endif()

message(STATUS "PLAT: FindDotnetAOT — RID=${MU_DOTNET_RID}, LIB_EXT=${MU_DOTNET_LIB_EXT}")

# ============================================================
# Step 2: WSL detection
# ============================================================

set(MU_IS_WSL FALSE)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    if(EXISTS "/proc/version")
        file(READ "/proc/version" _proc_version_content)
        if(_proc_version_content MATCHES "[Mm]icrosoft|[Ww][Ss][Ll]")
            set(MU_IS_WSL TRUE)
            message(STATUS "PLAT: FindDotnetAOT — WSL environment detected")
        endif()
    endif()
endif()

# ============================================================
# Step 3: Locate dotnet executable
# ============================================================

if(MU_IS_WSL)
    # WSL: locate Windows dotnet.exe via interop mount
    set(_dotnet_wsl_candidates
        "/mnt/c/Program Files/dotnet/dotnet.exe"
        "/mnt/c/Program Files (x86)/dotnet/dotnet.exe"
    )
    foreach(_candidate ${_dotnet_wsl_candidates})
        if(EXISTS "${_candidate}")
            set(DOTNETAOT_EXECUTABLE "${_candidate}" CACHE FILEPATH "Path to dotnet executable")
            break()
        endif()
    endforeach()
else()
    find_program(DOTNETAOT_EXECUTABLE dotnet
        PATHS
            "$ENV{DOTNET_ROOT}"
            "/usr/local/share/dotnet"
            "/usr/share/dotnet"
            "$ENV{HOME}/.dotnet"
        DOC "Path to dotnet executable"
    )
endif()

if(NOT DOTNETAOT_EXECUTABLE)
    message(WARNING
        "PLAT: FindDotnetAOT — dotnet not found at searched paths. "
        ".NET AOT library will not be built. "
        "Game compiles but cannot connect to servers without MUnique.Client.Library."
    )
    set(DOTNETAOT_FOUND FALSE)
    return()
endif()

set(DOTNETAOT_FOUND TRUE)
message(STATUS "PLAT: FindDotnetAOT — dotnet found at: ${DOTNETAOT_EXECUTABLE}")

# ============================================================
# Step 4: Resolve .csproj path (WSL path conversion)
# ============================================================

if(MU_IS_WSL)
    execute_process(
        COMMAND wslpath -w "${CMAKE_CURRENT_SOURCE_DIR}/ClientLibrary/MUnique.Client.Library.csproj"
        OUTPUT_VARIABLE DOTNETAOT_CSPROJ_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(NOT DOTNETAOT_CSPROJ_PATH)
        # Fallback: use Linux path and hope dotnet accepts it
        set(DOTNETAOT_CSPROJ_PATH
            "${CMAKE_CURRENT_SOURCE_DIR}/ClientLibrary/MUnique.Client.Library.csproj")
    endif()
else()
    set(DOTNETAOT_CSPROJ_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/ClientLibrary/MUnique.Client.Library.csproj")
endif()

message(STATUS "PLAT: FindDotnetAOT — csproj path: ${DOTNETAOT_CSPROJ_PATH}")
