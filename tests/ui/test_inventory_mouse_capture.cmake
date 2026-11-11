file(READ "${MU_INVENTORY_SOURCE}" inventory_source)

set(failure_message "Inventory hover must consume mouse input across the complete window")

# UpdateMouseEvent() consumes the mouse whenever WindowProcess() reports it over the window.
set(update_contract "    if (WindowProcess())\n        return false;")
string(FIND "${inventory_source}" "${update_contract}" update_position)
if(update_position EQUAL -1)
    message(FATAL_ERROR "${failure_message}: UpdateMouseEvent() no longer returns on WindowProcess()")
endif()

# WindowProcess() tests the whole window: the panel's drawn box, which takes no pointer events
# itself so the grid's clicks stay native.
set(window_signature "bool CMyInventory::WindowProcess()
{")
string(FIND "${inventory_source}" "${window_signature}" window_begin)
if(window_begin EQUAL -1)
    message(FATAL_ERROR "${failure_message}: CMyInventory::WindowProcess() not found")
endif()
string(SUBSTRING "${inventory_source}" ${window_begin} -1 window_tail)
string(FIND "${window_tail}" "
}
" window_end)
string(SUBSTRING "${window_tail}" 0 ${window_end} window_process)
string(FIND "${window_process}" "    if (!IsPointerOverPanel())
    {
        return false;
    }" position)
if(position EQUAL -1)
    message(FATAL_ERROR "${failure_message}: WindowProcess() no longer returns on IsPointerOverPanel()")
endif()

string(FIND "${inventory_source}" "return UI::RmlBridge::IsPointerWithin(document != nullptr ? document->GetElementById(\"panel\") : nullptr);" position)
if(position EQUAL -1)
    message(FATAL_ERROR "${failure_message}: IsPointerOverPanel() no longer tests #panel's drawn box")
endif()
