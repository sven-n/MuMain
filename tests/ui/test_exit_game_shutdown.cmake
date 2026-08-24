file(READ "${SOURCE_DIR}/src/source/UI/NewUI/Dialogs/NewUICustomMessageBox.cpp" source)

string(FIND "${source}" "SocketClient->ToGameServer()->SendLogOut(LogOutType::CloseGame);" logout_pos)
if(logout_pos EQUAL -1)
    message(FATAL_ERROR "Exit Game does not notify the game server")
endif()

string(FIND "${source}" "PostMessage(g_hWnd, WM_CLOSE, 0, 0);" shutdown_pos)
if(shutdown_pos EQUAL -1 OR shutdown_pos LESS logout_pos)
    message(FATAL_ERROR "Exit Game does not request local shutdown after notifying the server")
endif()
