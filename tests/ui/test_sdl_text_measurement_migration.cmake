file(GLOB_RECURSE source_files
    "${SOURCE_DIR}/src/source/*.cpp"
    "${SOURCE_DIR}/src/source/*.h"
)

foreach(source_file IN LISTS source_files)
    if(source_file MATCHES "/Core/Platform/")
        continue()
    endif()

    file(READ "${source_file}" source)
    if(source MATCHES "GetTextExtentPoint32|GetFontDC\\(|GetFontBuffer\\(")
        file(RELATIVE_PATH relative_file "${SOURCE_DIR}" "${source_file}")
        message(FATAL_ERROR "${relative_file} still uses legacy GDI text measurement")
    endif()

    if(source MATCHES "(^|[^A-Za-z0-9_])FontHeight([^A-Za-z0-9_]|$)")
        file(RELATIVE_PATH relative_file "${SOURCE_DIR}" "${source_file}")
        message(FATAL_ERROR "${relative_file} still uses the legacy global FontHeight")
    endif()
endforeach()

file(READ "${SOURCE_DIR}/src/source/UI/NewUI/Inventory/NewUIItemEnduranceInfo.cpp" endurance_source)
if(endurance_source MATCHES "7[ \t]*\\*[ \t]*iTextlen")
    message(FATAL_ERROR "Item endurance tooltip still estimates text width by character count")
endif()
