# CMake toolchain file for x86 Windows builds with MSVC and Ninja
# This file configures the compiler and SDK paths for 32-bit builds

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR X86)

# Force 32-bit pointer size BEFORE compiler detection
set(CMAKE_SIZEOF_VOID_P 4 CACHE INTERNAL "Pointer size in bytes" FORCE)

# Find the latest MSVC installation
file(GLOB MSVC_VERSIONS LIST_DIRECTORIES true "C:/Program Files/Microsoft Visual Studio/*/Community/VC/Tools/MSVC/*")
if (MSVC_VERSIONS)
  list(SORT MSVC_VERSIONS)
  list(REVERSE MSVC_VERSIONS)
  list(GET MSVC_VERSIONS 0 MSVC_PATH)

  # Set x86 compilers
  set(CMAKE_C_COMPILER "${MSVC_PATH}/bin/Hostx64/x86/cl.exe")
  set(CMAKE_CXX_COMPILER "${MSVC_PATH}/bin/Hostx64/x86/cl.exe")

  # Find the latest Windows SDK
  file(GLOB WIN_SDK_VERSIONS LIST_DIRECTORIES true "C:/Program Files (x86)/Windows Kits/10/Lib/*")
  if (WIN_SDK_VERSIONS)
    list(SORT WIN_SDK_VERSIONS)
    list(REVERSE WIN_SDK_VERSIONS)
    list(GET WIN_SDK_VERSIONS 0 WIN_SDK_LIB_PATH)

    # Extract SDK version from path
    string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+" WIN_SDK_VERSION "${WIN_SDK_LIB_PATH}")
    set(WIN_SDK_INCLUDE_PATH "C:/Program Files (x86)/Windows Kits/10/Include/${WIN_SDK_VERSION}")

    # Set linker flags to specify x86 library paths explicitly
    # This is necessary because we clear the LIB environment variable
    set(CMAKE_EXE_LINKER_FLAGS_INIT "/LIBPATH:\"${WIN_SDK_LIB_PATH}/um/x86\" /LIBPATH:\"${WIN_SDK_LIB_PATH}/ucrt/x86\" /LIBPATH:\"${MSVC_PATH}/lib/x86\"")
    set(CMAKE_SHARED_LINKER_FLAGS_INIT "/LIBPATH:\"${WIN_SDK_LIB_PATH}/um/x86\" /LIBPATH:\"${WIN_SDK_LIB_PATH}/ucrt/x86\" /LIBPATH:\"${MSVC_PATH}/lib/x86\"")
    set(CMAKE_MODULE_LINKER_FLAGS_INIT "/LIBPATH:\"${WIN_SDK_LIB_PATH}/um/x86\" /LIBPATH:\"${WIN_SDK_LIB_PATH}/ucrt/x86\" /LIBPATH:\"${MSVC_PATH}/lib/x86\"")

    # Set system library paths for find_library()
    set(CMAKE_SYSTEM_LIBRARY_PATH
      "${WIN_SDK_LIB_PATH}/um/x86"
      "${WIN_SDK_LIB_PATH}/ucrt/x86"
      "${MSVC_PATH}/lib/x86"
      CACHE STRING "System library search path" FORCE
    )

    # Set system include paths
    set(CMAKE_SYSTEM_INCLUDE_PATH
      "${WIN_SDK_INCLUDE_PATH}/ucrt"
      "${WIN_SDK_INCLUDE_PATH}/um"
      "${WIN_SDK_INCLUDE_PATH}/shared"
      "${MSVC_PATH}/include"
      CACHE STRING "System include search path" FORCE
    )

    message(STATUS "x86 Toolchain: Using MSVC ${MSVC_PATH}")
    message(STATUS "x86 Toolchain: Using Windows SDK ${WIN_SDK_VERSION}")
    message(STATUS "x86 Toolchain: Library paths set to x86")
  endif()
endif()
