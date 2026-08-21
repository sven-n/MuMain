file(READ "${UI_SOURCE_FILE}" ui_source)
file(READ "${RENDER_SOURCE_FILE}" render_source)

if(NOT ui_source MATCHES "case 2: SetRenderColor\\(106, 97, 88, windowAlpha\\)")
    message(FATAL_ERROR "Legacy selected rows must restore their visible highlight color")
endif()

if(NOT render_source MATCHES "color = g_renderColor")
    message(FATAL_ERROR "RenderColor must consume the color selected by SetLineColor")
endif()

if(render_source MATCHES "color = 0xCC000000u")
    message(FATAL_ERROR "RenderColor must not force every legacy UI quad to black")
endif()
