# AC-2: Marshal.PtrToStringUni verification — ConnectionManager.cs
# AC-STD-3: No PtrToStringAuto remains for hostPtr at .NET interop boundary
#
# Story: 3.2.1 - char16_t Encoding at .NET Interop Boundary
#
# L-2 fix: Automated guard for the inspection-based AC-2 check so that a
# regression (accidentally reverting PtrToStringUni back to PtrToStringAuto)
# is caught by the build test suite without requiring manual code inspection.
#
# Run manually: cmake -P tests/build/test_ac2_marshal_ptr_3_2_1.cmake

cmake_minimum_required(VERSION 3.25)

# Path passed from CTest or defaulted relative to this script
if(NOT DEFINED CONNECTION_MANAGER_CS)
    set(CONNECTION_MANAGER_CS "${CMAKE_CURRENT_LIST_DIR}/../../ClientLibrary/ConnectionManager.cs")
endif()

# --- Check 1: ConnectionManager.cs exists ---
if(NOT EXISTS "${CONNECTION_MANAGER_CS}")
    message(FATAL_ERROR "AC-2 FAIL: ConnectionManager.cs not found at '${CONNECTION_MANAGER_CS}' -- file must exist")
endif()

file(READ "${CONNECTION_MANAGER_CS}" CS_CONTENT)

# --- Check 2: Marshal.PtrToStringUni is present (AC-2) ---
string(FIND "${CS_CONTENT}" "PtrToStringUni" _pos_uni)
if(_pos_uni EQUAL -1)
    message(FATAL_ERROR "AC-2 FAIL: 'PtrToStringUni' not found in ConnectionManager.cs -- ConnectionManager_Connect must use Marshal.PtrToStringUni(hostPtr) for explicit UTF-16LE marshaling")
endif()

# --- Check 3: Marshal.PtrToStringAuto is absent for hostPtr (AC-STD-3) ---
string(FIND "${CS_CONTENT}" "PtrToStringAuto" _pos_auto)
if(NOT _pos_auto EQUAL -1)
    message(FATAL_ERROR "AC-2 / AC-STD-3 FAIL: 'PtrToStringAuto' found in ConnectionManager.cs -- replace with 'Marshal.PtrToStringUni' for explicit UTF-16LE encoding. PtrToStringAuto is platform-dependent and must not appear at the .NET interop boundary.")
endif()

message(STATUS "AC-2 PASS: 'PtrToStringUni' found in ConnectionManager.cs (explicit UTF-16LE marshaling)")
message(STATUS "AC-STD-3 PASS: 'PtrToStringAuto' absent from ConnectionManager.cs (no platform-dependent marshaling)")

# ==========================================================================
# SUMMARY
# ==========================================================================

message(STATUS "")
message(STATUS "=== 3.2.1 AC-2 Marshal.PtrToStringUni PASS ===")
message(STATUS "  ConnectionManager.cs: PtrToStringUni present (UTF-16LE guaranteed)")
message(STATUS "  ConnectionManager.cs: PtrToStringAuto absent (no platform-dependent marshaling)")
message(STATUS "==============================================")
