file(READ "${SOURCE_DIR}/src/source/UI/Legacy/UIControls.cpp" source)

if(NOT source MATCHES "g_pRenderText->MeasureText\\(tempString\\.c_str\\(\\),")
    message(FATAL_ERROR "UI text wrapping must measure with the active text renderer")
endif()

if(source MATCHES "GetTextExtentPoint32\\(g_pRenderText->GetFontDC\\(\\), tempString")
    message(FATAL_ERROR "UI text wrapping must not use the SDL renderer's null GDI handle")
endif()

if(source MATCHES "iTargetPixelWidth \\* g_fScreenRate_x")
    message(FATAL_ERROR "Renderer measurements and wrap widths must use the same logical units")
endif()
