# Flow Code: VS0-QUAL-BUILD-HEADERS
# Story 7.8.2: Gameplay Header Cross-Platform Fixes
# AC-2: World/ZzzPath.h — must include ErrorReport.h explicitly
#
# ZzzPath.h uses g_ErrorReport.Write() at multiple call sites but does not
# include ErrorReport.h, relying on transitive includes from the PCH or other
# headers. On macOS/Linux with fresh include paths, this transitive dependency
# is absent, causing compilation failures.
#
# Project include convention: flat by directory ("ErrorReport.h" not "Core/ErrorReport.h")
#
# RED PHASE: #include "ErrorReport.h" absent → g_ErrorReport undeclared on platforms
#            that do not provide it transitively
# GREEN PHASE: #include "ErrorReport.h" present → ZzzPath.h is self-contained

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED ZZZ_PATH_H)
    message(FATAL_ERROR "AC-2: ZZZ_PATH_H variable not defined")
endif()

if(NOT EXISTS "${ZZZ_PATH_H}")
    message(FATAL_ERROR "AC-2: ZzzPath.h not found at: ${ZZZ_PATH_H}")
endif()

file(READ "${ZZZ_PATH_H}" content)

# ---------------------------------------------------------------------------
# Check 1: ZzzPath.h must include ErrorReport.h
# Project convention: flat includes — "ErrorReport.h" not "Core/ErrorReport.h"
# In RED phase: no #include "ErrorReport.h" → FAIL
# In GREEN phase: #include "ErrorReport.h" present → PASS
# ---------------------------------------------------------------------------
string(FIND "${content}" "#include \"ErrorReport.h\"" pos_flat)
string(FIND "${content}" "#include \"Core/ErrorReport.h\"" pos_path)

if(pos_flat EQUAL -1 AND pos_path EQUAL -1)
    message(FATAL_ERROR
        "AC-2 FAILED: ZzzPath.h does not include ErrorReport.h\n"
        "Fix: Add '#include \"ErrorReport.h\"' to ZzzPath.h (flat-style per project convention)\n"
        "Why: ZzzPath.h uses g_ErrorReport.Write() but has no explicit include for ErrorReport.h.\n"
        "     On macOS/Linux, the transitive include chain differs from Windows, causing\n"
        "     'g_ErrorReport undeclared identifier' compilation errors.")
endif()

if(NOT pos_flat EQUAL -1 AND NOT pos_path EQUAL -1)
    message(WARNING
        "AC-2: Both flat and path-prefixed includes found in ZzzPath.h.\n"
        "Prefer the flat form '#include \"ErrorReport.h\"' per project convention.")
endif()

if(pos_flat EQUAL -1 AND NOT pos_path EQUAL -1)
    message(WARNING
        "AC-2: Found '#include \"Core/ErrorReport.h\"' in ZzzPath.h.\n"
        "Project convention prefers flat form '#include \"ErrorReport.h\"'.\n"
        "Passing for now — consider changing to flat include.")
endif()

# ---------------------------------------------------------------------------
# Check 2: g_ErrorReport.Write usages are present (regression guard)
# ---------------------------------------------------------------------------
string(FIND "${content}" "g_ErrorReport" pos_usage)
if(pos_usage EQUAL -1)
    message(WARNING "AC-2: g_ErrorReport not found in ZzzPath.h — may have been removed")
endif()

# ---------------------------------------------------------------------------
# Regression guard: file is non-trivial
# ---------------------------------------------------------------------------
string(LENGTH "${content}" content_len)
if(content_len LESS 500)
    message(FATAL_ERROR "AC-2: ZzzPath.h appears too short (${content_len} bytes) — file may be truncated")
endif()

message(STATUS "AC-2 PASSED: ZzzPath.h includes ErrorReport.h (g_ErrorReport dependency is explicit)")
