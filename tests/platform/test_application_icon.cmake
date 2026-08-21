foreach(required_file IN ITEMS
    "${SOURCE_DIR}/source/App/Resources/MuMainIcon.svg"
    "${SOURCE_DIR}/source/App/Resources/MuMainIcon1024.png"
    "${SOURCE_DIR}/source/App/Platform/macOS/MuMain.icns"
    "${SOURCE_DIR}/source/App/Platform/Windows/icon1.ico"
    "${SOURCE_DIR}/bin/MuMainIcon.png")
    if(NOT EXISTS "${required_file}")
        message(FATAL_ERROR "Missing application icon asset: ${required_file}")
    endif()
endforeach()

file(READ "${SOURCE_DIR}/CMakeLists.txt" cmake_source)
foreach(required_text IN ITEMS
    "MACOSX_BUNDLE"
    "MACOSX_BUNDLE_ICON_FILE"
    "MuMain.icns")
    string(FIND "${cmake_source}" "${required_text}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "CMake application icon integration missing: ${required_text}")
    endif()
endforeach()

file(READ "${SOURCE_DIR}/source/App/Platform/Windows/Winmain.cpp" bootstrap_source)
foreach(required_text IN ITEMS
    "SDL_LoadPNG(\"MuMainIcon.png\")"
    "SDL_SetWindowIcon")
    string(FIND "${bootstrap_source}" "${required_text}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "SDL application icon integration missing: ${required_text}")
    endif()
endforeach()
