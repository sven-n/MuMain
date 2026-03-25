# Story 7.6.1: AC-7 — PlatformCompat.h provides GDI/Win32 stubs for UIControls.cpp
# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
#
# RED PHASE:  Test FAILS if PlatformCompat.h (non-Windows section) is missing GDI stubs
#             needed by UIControls.cpp: RGB, SetBkColor, SetTextColor, TextOut,
#             WM_PAINT, WM_ERASEBKGND, SB_VERT, GCS_COMPSTR, SetTimer.
#             Without these stubs, MUThirdParty fails to compile on macOS.
#
# GREEN PHASE: Test PASSES when all 9 required stubs are present in PlatformCompat.h.
#
# Validates:
#   AC-7 — PlatformCompat.h provides all required GDI/Win32 stubs in non-Windows section

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED PLATFORM_COMPAT_H)
    message(FATAL_ERROR "PLATFORM_COMPAT_H must be set to the path of PlatformCompat.h")
endif()

if(NOT EXISTS "${PLATFORM_COMPAT_H}")
    message(FATAL_ERROR "AC-7 FAIL: PlatformCompat.h not found at '${PLATFORM_COMPAT_H}'")
endif()

file(READ "${PLATFORM_COMPAT_H}" header_content)

set(missing_stubs "")

# Check for each required GDI/Win32 stub (story 7.6.1 requirement)
foreach(stub_name
    "RGB"
    "SetBkColor"
    "SetTextColor"
    "TextOut"
    "WM_PAINT"
    "WM_ERASEBKGND"
    "SB_VERT"
    "GCS_COMPSTR"
    "SetTimer"
)
    string(FIND "${header_content}" "${stub_name}" stub_pos)
    if(stub_pos EQUAL -1)
        list(APPEND missing_stubs "${stub_name}")
    endif()
endforeach()

if(missing_stubs)
    list(JOIN missing_stubs ", " missing_list)
    message(FATAL_ERROR
        "AC-7 FAIL: PlatformCompat.h is missing the following GDI/Win32 stubs: ${missing_list}\n"
        "These stubs are required by UIControls.cpp (MUThirdParty) to compile on macOS.\n"
        "Fix: Add no-op stubs to the #else // !_WIN32 section of PlatformCompat.h:\n"
        "  - RGB(r,g,b) → color macro\n"
        "  - SetBkColor(hdc,color) → inline returning 0\n"
        "  - SetTextColor(hdc,color) → inline returning 0\n"
        "  - TextOut(hdc,x,y,str,len) → inline returning FALSE\n"
        "  - WM_PAINT → 0x000F\n"
        "  - WM_ERASEBKGND → 0x0014\n"
        "  - SB_VERT → 1\n"
        "  - GCS_COMPSTR → 0x0008\n"
        "  - SetTimer(hwnd,id,ms,fn) → inline returning 0")
endif()

message(STATUS "AC-7 PASS: PlatformCompat.h provides all required GDI/Win32 stubs for UIControls.cpp")
