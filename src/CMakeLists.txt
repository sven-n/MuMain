cmake_minimum_required(VERSION 3.16)

project(MuMain)

# Initialize imgui submodule automatically if missing
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui.cpp")
  find_package(Git QUIET)
  if(NOT GIT_FOUND)
    message(FATAL_ERROR "Git not found. Please run 'git submodule update --init' manually from repository root.")
  endif()

  # Use CMake's file(LOCK) for atomic locking across processes
  set(LOCK_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/.cmake_init_lock")

  message(STATUS "Initializing imgui submodule (without --recursive to exclude examples)...")
  file(LOCK "${LOCK_FILE}" GUARD PROCESS TIMEOUT 60)

  # Check again inside lock - another process may have initialized it
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui.cpp")
    # Clean any stale git state, then initialize
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule deinit -f "src/ThirdParty/imgui"
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/.. OUTPUT_QUIET ERROR_QUIET)
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init "src/ThirdParty/imgui"
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
                    RESULT_VARIABLE GIT_RESULT ERROR_VARIABLE GIT_ERROR)

    if(NOT GIT_RESULT EQUAL "0")
      message(FATAL_ERROR "Submodule initialization failed:\n${GIT_ERROR}\nPlease run 'git submodule update --init' manually.")
    endif()
    message(STATUS "ImGui submodule initialized successfully")
  endif()

  # Lock is automatically released here

  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui.cpp")
    message(FATAL_ERROR "ImGui submodule missing. Please run 'git submodule update --init' manually.")
  endif()
endif()

# Force 32-bit build for MSVC to match available libraries
if (MSVC)
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This project requires 32-bit libraries. Please configure CMake with: -DCMAKE_GENERATOR_PLATFORM=Win32 (for Visual Studio generator) or use the x86 MSVC toolchain in CLion settings")
  endif()
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Build type (default RelWithDebInfo for useful symbols)" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Collect all C++ source files under source/
file(GLOB_RECURSE MU_MAIN_SOURCES
  CONFIGURE_DEPENDS
  "source/*.cpp"
)

# ImGui static library (required when _EDITOR is defined)
add_library(imgui STATIC
  "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_draw.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_tables.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_widgets.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_win32.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_opengl2.cpp"
)

target_include_directories(imgui PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui"
  "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends"
)

add_executable(Main ${MU_MAIN_SOURCES})

if (NOT MSVC)
  set(MU_MAIN_MAP_FILE "${CMAKE_CURRENT_BINARY_DIR}/Main.map")
  target_link_options(Main PRIVATE "-Wl,-Map,${MU_MAIN_MAP_FILE}")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  # Build as a Windows GUI application (no console window) and let CMake
  # add the default system libraries like kernel32, user32, gdi32, etc.
  set(WORKING_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")
  message(STATUS "Setting debugger working directory to: ${WORKING_DIR}")
  set_target_properties(Main PROPERTIES
    WIN32_EXECUTABLE TRUE
    VS_DEBUGGER_WORKING_DIRECTORY "${WORKING_DIR}"
  )

  enable_language(RC)
  target_sources(Main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/source/resource.rc"
  )
endif()

# Skeleton Linux-native linkage. This intentionally depends only on
# ubiquitous system libraries so we can focus on getting the project to
# compile; more platform-specific adjustments will follow as we iterate.
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(Main PRIVATE
    pthread
    dl
    m
  )
endif()

target_include_directories(Main PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/source"
  "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/netcore/includes"
)

# Basic compile definitions, roughly mirroring the original MSVC project
# (fine-tuning per configuration can be done later if needed.)
target_compile_definitions(Main PRIVATE
  _LANGUAGE_FOREIGN
  _LANGUAGE_ENG
  UNICODE
  _USE_32BIT_TIME_T  # Use 32-bit time_t for 32-bit builds
  $<$<CONFIG:Debug>:_DEBUG;_FOREIGN_DEBUG;_EDITOR>
  $<$<CONFIG:Release>:NDEBUG;_FOREIGN_NDEBUG;LDS_PATCH_GLOBAL_100520>
)

if (MSVC)
  target_compile_definitions(Main PRIVATE
    WIN32
    _WINDOWS
  )

  # Link against Windows system libraries and local static libs
  target_link_directories(Main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib"
  )

  target_link_libraries(Main PRIVATE
    imm32
    vfw32
    dsound
    dxguid
    opengl32
    glu32
    winmm
    ws2_32
    version
    shlwapi
    crypt32
    wininet
    urlmon
    turbojpeg-static
    wzAudio
    imgui
  )

  # Locate required DLLs in bin directory
  set(DLL_SEARCH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")
  set(REQUIRED_DLL_NAMES wzAudio glew32 ogg vorbisfile)
  set(REQUIRED_DLLS)

  foreach(DLL_NAME IN LISTS REQUIRED_DLL_NAMES)
    find_file(${DLL_NAME}_DLL
      NAMES "${DLL_NAME}.dll"
      PATHS "${DLL_SEARCH_PATH}"
      NO_DEFAULT_PATH
      REQUIRED
    )
    list(APPEND REQUIRED_DLLS "${${DLL_NAME}_DLL}")
  endforeach()

  # Copy required DLLs to build directory for MSVC builds
  add_custom_command(TARGET Main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${REQUIRED_DLLS}
      $<TARGET_FILE_DIR:Main>
    COMMENT "Copying required DLLs to build directory"
  )

  # 1. Define the output path using a variable CMake can understand early
  set(DOTNET_DLL_PATH "${CMAKE_CURRENT_BINARY_DIR}/MUnique.Client.Library.dll")
  set(DOTNET_PROJ "${CMAKE_CURRENT_SOURCE_DIR}/../ClientLibrary/MUnique.Client.Library.csproj")

  # 2. Tell CMake to watch ALL .cs files in the ClientLibrary folder
  file(GLOB_RECURSE DOTNET_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../ClientLibrary/*.cs")

  # 3. This only runs if DOTNET_SOURCES or the .csproj change
  add_custom_command(
          OUTPUT "${DOTNET_DLL_PATH}"
          COMMAND ${CMAKE_COMMAND} -E echo "--- C# Changes Detected: Rebuilding Client Library ---"
          COMMAND dotnet publish "${DOTNET_PROJ}" -c $<CONFIG> -r win-x86 -o "${CMAKE_CURRENT_BINARY_DIR}" --nologo
          DEPENDS "${DOTNET_PROJ}" ${DOTNET_SOURCES}
          COMMENT "Checking for .NET Client Library updates..."
          VERBATIM
  )

  # 4. Create a target for the DLL and link it to Main
  add_custom_target(BuildDotNetLib DEPENDS "${DOTNET_DLL_PATH}")
  add_dependencies(Main BuildDotNetLib)
endif()

# Custom target to run the application with correct working directory (cross-platform)
add_custom_target(run
  COMMAND "$<TARGET_FILE:Main>"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
  DEPENDS Main
  COMMENT "Running Main from bin directory"
  USES_TERMINAL
)

# MinGW (32-bit or 64-bit) cross-toolchains: link against the Windows/OpenGL
# system libraries so that gl*, wgl*, Imm*, timeGetTime, etc. resolve.
# For turbojpeg and wzAudio we link against the logically named libraries,
# which must be provided as MinGW-compatible .a import libraries.
if (CMAKE_CXX_COMPILER MATCHES "mingw32" OR CMAKE_CXX_COMPILER MATCHES "w64-mingw32")
  target_link_directories(Main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib"
  )

  set(MU_TURBOJPEG_STATIC_LIB "" CACHE FILEPATH "Path to MinGW-w64 static libturbojpeg.a")
  if (MU_TURBOJPEG_STATIC_LIB STREQUAL "" OR MU_TURBOJPEG_STATIC_LIB MATCHES "-NOTFOUND$")
    unset(MU_TURBOJPEG_STATIC_LIB CACHE)
  endif()
  if (NOT MU_TURBOJPEG_STATIC_LIB)
    find_library(MU_TURBOJPEG_STATIC_LIB
      NAMES turbojpeg
      PATHS
        "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib"
      NO_CMAKE_FIND_ROOT_PATH
    )
  endif()
  if (NOT MU_TURBOJPEG_STATIC_LIB)
    set(_MU_OLD_FIND_LIBRARY_SUFFIXES "${CMAKE_FIND_LIBRARY_SUFFIXES}")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(_MU_TURBOJPEG_SEARCH_PATHS "")
    foreach(_mu_root IN LISTS CMAKE_FIND_ROOT_PATH)
      list(APPEND _MU_TURBOJPEG_SEARCH_PATHS "${_mu_root}/lib" "${_mu_root}/usr/lib")
    endforeach()
    list(APPEND _MU_TURBOJPEG_SEARCH_PATHS "/usr/i686-w64-mingw32/lib" "/usr/x86_64-w64-mingw32/lib")
    find_library(MU_TURBOJPEG_STATIC_LIB
      NAMES turbojpeg
      PATHS ${_MU_TURBOJPEG_SEARCH_PATHS}
      NO_CMAKE_FIND_ROOT_PATH
    )
    unset(_MU_TURBOJPEG_SEARCH_PATHS)
    unset(_mu_root)
    set(CMAKE_FIND_LIBRARY_SUFFIXES "${_MU_OLD_FIND_LIBRARY_SUFFIXES}")
    unset(_MU_OLD_FIND_LIBRARY_SUFFIXES)
  endif()
  if (NOT MU_TURBOJPEG_STATIC_LIB OR MU_TURBOJPEG_STATIC_LIB MATCHES "\\.dll\\.a$")
    message(FATAL_ERROR "Static libturbojpeg not found (libturbojpeg.a) or CMake picked an import library (.dll.a): '${MU_TURBOJPEG_STATIC_LIB}'. Install/build a MinGW-w64 static libjpeg-turbo to avoid shipping libturbojpeg.dll, or set MU_TURBOJPEG_STATIC_LIB to the full path of libturbojpeg.a.")
  endif()

  target_link_libraries(Main PRIVATE
    imm32
    vfw32
    dsound
    dxguid
    opengl32
    glu32
    winmm
    ws2_32
    version
    shlwapi
    crypt32
    wininet
    urlmon
    gdi32
    user32
    ole32
    oleaut32
    uuid
    comdlg32
    advapi32
    ${MU_TURBOJPEG_STATIC_LIB}
    wzAudio
    imgui
  )
endif()
