file(READ "${SOURCE_FILE}" bitmap_source)

if(NOT bitmap_source MATCHES "void CGlobalBitmap::UnloadAllImages\\(\\)[ \t\r\n]*\\{[ \t\r\n]*if \\(m_mapBitmap.empty\\(\\)\\)")
    message(FATAL_ERROR "Empty bitmap shutdown must return before accessing renderer device")
endif()
