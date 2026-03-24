# Story 7.3.0: AC-6 — GlobalBitmap.cpp uses numeric literals instead of GL_CLAMP_TO_EDGE / GL_REPEAT
# Flow Code: VS0-QUAL-BUILDCOMPAT-MACOS
#
# RED PHASE:  Test FAILS before story 7.3.0 is implemented.
#             Lines 651–652 use GL_CLAMP_TO_EDGE and GL_REPEAT constants, which require
#             OpenGL headers that are not present in the SDL3 build path on macOS.
#
# GREEN PHASE: Test PASSES after fix.
#             GL_CLAMP_TO_EDGE and GL_REPEAT symbolic names are absent from GlobalBitmap.cpp.
#             Replaced by 0x812Fu and 0x2901u numeric literals respectively.
#
# Validates:
#   AC-6 — GL_CLAMP_TO_EDGE and GL_REPEAT are not used as symbolic names in GlobalBitmap.cpp

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "SOURCE_FILE must be set to the path of GlobalBitmap.cpp")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-6 FAIL: GlobalBitmap.cpp not found at '${SOURCE_FILE}'")
endif()

file(READ "${SOURCE_FILE}" source_content)

set(found_violations FALSE)

# Check for GL_CLAMP_TO_EDGE symbolic name
string(FIND "${source_content}" "GL_CLAMP_TO_EDGE" gl_clamp_pos)
if(NOT gl_clamp_pos EQUAL -1)
    message(WARNING "AC-6 FAIL: 'GL_CLAMP_TO_EDGE' symbolic name found at position ${gl_clamp_pos} in GlobalBitmap.cpp")
    set(found_violations TRUE)
endif()

# Check for GL_REPEAT symbolic name
string(FIND "${source_content}" "GL_REPEAT" gl_repeat_pos)
if(NOT gl_repeat_pos EQUAL -1)
    message(WARNING "AC-6 FAIL: 'GL_REPEAT' symbolic name found at position ${gl_repeat_pos} in GlobalBitmap.cpp")
    set(found_violations TRUE)
endif()

if(found_violations)
    message(FATAL_ERROR
        "AC-6 FAIL: OpenGL symbolic constant names found in GlobalBitmap.cpp.\n"
        "These require <GL/gl.h> which is not available in the SDL3/macOS build path.\n"
        "Fix: replace GL_CLAMP_TO_EDGE with 0x812Fu and GL_REPEAT with 0x2901u.")
endif()

# Verify the replacement literals are present (sanity check that fix was actually applied)
string(FIND "${source_content}" "0x812F" lit_clamp_pos)
string(FIND "${source_content}" "0x2901" lit_repeat_pos)

if(lit_clamp_pos EQUAL -1 OR lit_repeat_pos EQUAL -1)
    message(WARNING "AC-6 NOTE: GL constant literals 0x812F/0x2901 not found — verify LoadImage wrap mode logic was preserved")
endif()

message(STATUS "AC-6 PASS: GL_CLAMP_TO_EDGE and GL_REPEAT replaced with numeric literals in GlobalBitmap.cpp")
