file(READ "${HEADER_FILE}" header_source)
file(READ "${SOURCE_FILE}" implementation_source)

string(FIND "${header_source}" "int32_t ConnectionHandle" packet_handle_pos)
if(packet_handle_pos EQUAL -1)
    message(FATAL_ERROR "Queued packets must retain their source connection handle")
endif()

string(FIND "${implementation_source}" "Packet->ConnectionHandle = Handle;" handle_assignment_pos)
if(handle_assignment_pos EQUAL -1)
    message(FATAL_ERROR "Incoming packets must record their source connection handle")
endif()

string(FIND "${implementation_source}" "SocketClient == nullptr || Packet->ConnectionHandle != SocketClient->GetHandle()" stale_guard_pos)
if(stale_guard_pos EQUAL -1)
    message(FATAL_ERROR "Packet dispatch must reject packets from stale connections")
endif()
