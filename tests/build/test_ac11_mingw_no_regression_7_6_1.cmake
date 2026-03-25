# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
# Test AC-11: No new #ifdef _WIN32 wrapping game logic (function calls, assignments, etc.)
# Header include guards (#ifdef _WIN32 / #include <header> / #endif) are ALLOWED per Approach B.

cmake_minimum_required(VERSION 3.23)

get_filename_component(WORKSPACE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

set(GAME_LOGIC_FILES
  "${WORKSPACE_ROOT}/MuMain/src/source/Core/muConsoleDebug.cpp"
  "${WORKSPACE_ROOT}/MuMain/src/source/Scenes/SceneManager.cpp"
  "${WORKSPACE_ROOT}/MuMain/src/source/UI/Legacy/UIMng.cpp"
  "${WORKSPACE_ROOT}/MuMain/src/source/UI/Legacy/UIWindows.cpp"
  "${WORKSPACE_ROOT}/MuMain/src/source/World/Maps/GMBattleCastle.cpp"
  "${WORKSPACE_ROOT}/MuMain/src/source/World/Maps/GMCrywolf1st.cpp"
)

foreach(file ${GAME_LOGIC_FILES})
  if(NOT EXISTS "${file}")
    message(FATAL_ERROR "File not found: ${file}")
  endif()

  file(READ "${file}" file_content)

  # Check for #ifdef _WIN32 wrapping game logic (function calls with parentheses + semicolons).
  # Exclude include guards: lines starting with # (preprocessor directives) are allowed.
  # Pattern: #ifdef _WIN32 followed by a non-preprocessor statement (e.g., function call) then #endif
  string(REGEX MATCH "#ifdef _WIN32\n[^#][^\n]*[a-zA-Z_][a-zA-Z0-9_]*\\([^\n]*\\)[^\n]*;" ifdef_match "${file_content}")
  if(ifdef_match)
    message(FATAL_ERROR "AC-11 FAILED: Found #ifdef wrapping game logic in ${file}")
  endif()
endforeach()

message(STATUS "AC-11 PASSED")
