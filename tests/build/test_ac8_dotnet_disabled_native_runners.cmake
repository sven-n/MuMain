# AC-8: Validate native CI runners do not require external .NET setup steps
#
# Story: 7.4.1 - Native Platform CI Runners
# Flow Code: VS0-QUAL-CI-NATIVE
# Story tag: [7-4-1]
#
# UPDATED by Story 7.8.4: MU_ENABLE_DOTNET is now ON for all platforms
# (macOS, Linux, Windows). The preset check for MU_ENABLE_DOTNET=OFF has
# been removed — .NET Native AOT now builds natively on all platforms.
# CI checks (no setup-dotnet action, no direct dotnet invocation) remain valid.
#
# Validates:
#   AC-8 — No 'setup-dotnet' or 'dotnet build' step in native runner jobs

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CI_WORKFLOW_FILE)
    message(FATAL_ERROR "AC-8: CI_WORKFLOW_FILE variable not defined")
endif()

if(NOT EXISTS "${CI_WORKFLOW_FILE}")
    message(FATAL_ERROR "AC-8: ci.yml does not exist: ${CI_WORKFLOW_FILE}")
endif()

file(READ "${CI_WORKFLOW_FILE}" CI_CONTENT)

# --- Check 1: No dotnet setup action in ci.yml for native jobs ---
string(FIND "${CI_CONTENT}" "dotnet/setup-dotnet" _pos_setup_dotnet)
if(NOT _pos_setup_dotnet EQUAL -1)
    message(FATAL_ERROR "AC-8 FAIL [7-4-1]: 'dotnet/setup-dotnet' action found in ci.yml — native runners must not require .NET SDK")
endif()

# --- Check 2: (Removed by Story 7.8.4) ---
# Previously validated MU_ENABLE_DOTNET=OFF in native presets.
# Story 7.8.4 enables .NET on all platforms — this check is no longer applicable.

# --- Check 3: Native CI jobs do not invoke 'dotnet' command directly ---
string(FIND "${CI_CONTENT}" "run: dotnet" _pos_dotnet_run)
if(NOT _pos_dotnet_run EQUAL -1)
    message(FATAL_ERROR "AC-8 FAIL [7-4-1]: 'run: dotnet' command found in ci.yml — native runners must not invoke dotnet directly")
endif()

message(STATUS "AC-8 PASS [7-4-1]: No dotnet setup or invocation in native runner CI jobs (MU_ENABLE_DOTNET check removed by Story 7.8.4)")
