file(READ "${SOURCE_DIR}/src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.cpp" inventory_source)
file(READ "${SOURCE_DIR}/src/source/UI/NewUI/HUD/NewUIChatLogWindow.cpp" system_log_source)

string(FIND "${inventory_source}" "SetInventorySquareColor(m_afColorStateWarning);" warning_color_pos)
if(warning_color_pos EQUAL -1)
    message(FATAL_ERROR "Inventory drop warning backdrop does not use its configured color")
endif()

string(FIND "${inventory_source}" "SetInventorySquareColor(m_afColorStateNormal);" normal_color_pos)
if(normal_color_pos EQUAL -1)
    message(FATAL_ERROR "Inventory drop backdrop does not use its configured normal color")
endif()

string(FIND "${system_log_source}" "static_cast<BYTE>(255.f * m_fBackAlpha)" configured_alpha_pos)
if(configured_alpha_pos EQUAL -1)
    message(FATAL_ERROR "System log backdrop does not use its configured alpha")
endif()

string(FIND "${system_log_source}" "SetBgColor(0, 0, 0, 100)" hardcoded_alpha_pos)
if(NOT hardcoded_alpha_pos EQUAL -1)
    message(FATAL_ERROR "System log backdrop still uses the bright hard-coded alpha")
endif()
