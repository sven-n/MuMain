file(READ "${MU_INVENTORY_SOURCE}" inventory_source)

set(failure_message "Inventory hover must consume mouse input across the complete window")

# UpdateMouseEvent() consumes the mouse whenever WindowProcess() reports it over the window.
set(update_contract "    if (WindowProcess())\n        return false;")
string(FIND "${inventory_source}" "${update_contract}" update_position)
if(update_position EQUAL -1)
    message(FATAL_ERROR "${failure_message}: UpdateMouseEvent() no longer returns on WindowProcess()")
endif()

# WindowProcess() tests the whole window: the live #panel size, which starts from the full
# INVENTORY_WIDTH x INVENTORY_HEIGHT until RmlUi has laid the panel out.
set(window_signature "bool CMyInventory::WindowProcess()\n{")
string(FIND "${inventory_source}" "${window_signature}" window_begin)
if(window_begin EQUAL -1)
    message(FATAL_ERROR "${failure_message}: CMyInventory::WindowProcess() not found")
endif()
string(SUBSTRING "${inventory_source}" ${window_begin} -1 window_tail)
string(FIND "${window_tail}" "\n}\n" window_end)
string(SUBSTRING "${window_tail}" 0 ${window_end} window_process)

function(require_in_window_process needle)
    string(FIND "${window_process}" "${needle}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "${failure_message}: WindowProcess() lacks \"${needle}\"")
    endif()
endfunction()

require_in_window_process("float panelWidth = INVENTORY_WIDTH;")
require_in_window_process("float panelHeight = INVENTORY_HEIGHT;")
require_in_window_process("UI::RmlBridge::RefreshLogicalPanelSize(m_pRmlDoc, \"panel\", panelWidth, panelHeight);")
require_in_window_process(
    "mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, static_cast<int>(panelWidth), static_cast<int>(panelHeight)).Contains(MouseX, MouseY) == false)\n    {\n        return false;\n    }")
