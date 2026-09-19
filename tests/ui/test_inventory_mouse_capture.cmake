file(READ "${MU_INVENTORY_SOURCE}" inventory_source)

set(update_contract "    if (WindowProcess())\n        return false;")
set(window_contract
    "    if (CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT) == false)\n    {\n        return false;\n    }")

string(FIND "${inventory_source}" "${update_contract}" update_position)
string(FIND "${inventory_source}" "${window_contract}" window_position)

if(update_position EQUAL -1 OR window_position EQUAL -1)
    message(FATAL_ERROR
        "Inventory hover must consume mouse input across the complete window")
endif()
