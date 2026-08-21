file(READ "${SOURCE_DIR}/src/source/UI/NewUI/Dialogs/NewUIChatCommandWindow.cpp" source)

string(FIND "${source}" "SetRelatedWnd(g_hWnd);" restore_pos)
if(restore_pos EQUAL -1)
    message(FATAL_ERROR "Chat command input does not restore keyboard events to the game window")
endif()
