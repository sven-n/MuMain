file(READ "${SOURCE_DIR}/src/source/Core/Utilities/UsefulDef.cpp" source)

if(NOT source MATCHES "const wchar_t szNewlineDelimiters\\[\\] = \\{szNewlineChar, L'\\\\0'\\};")
    message(FATAL_ERROR "DivideStringByPixel must construct a NUL-terminated delimiter array")
endif()

if(source MATCHES "wcstok_s\\([^\\n]*&szNewlineChar")
    message(FATAL_ERROR "DivideStringByPixel must not pass a single character as a delimiter string")
endif()
