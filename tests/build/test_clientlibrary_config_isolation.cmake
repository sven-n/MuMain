file(READ "${CMAKE_CURRENT_LIST_DIR}/../../src/CMakeLists.txt" source_cmake)

foreach(required_text
    "dotnet_stage/$<CONFIG>"
    "\${DOTNET_TEMP_OUTPUT}/$<CONFIG>"
    "publish \"\${DOTNET_PROJ_NATIVE}\" -c $<CONFIG>")
  string(FIND "${source_cmake}" "${required_text}" match_index)
  if(match_index EQUAL -1)
    message(FATAL_ERROR "Missing config-isolated ClientLibrary build fragment: ${required_text}")
  endif()
endforeach()
