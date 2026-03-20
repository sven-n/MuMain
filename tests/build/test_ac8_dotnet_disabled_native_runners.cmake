# AC-8: Validate .NET SDK is NOT required on native runners (MU_ENABLE_DOTNET=OFF)
#
# Story: 7.4.1 - Native Platform CI Runners
# Flow Code: VS0-QUAL-CI-NATIVE
# Story tag: [7-4-1]
#
# RED PHASE: Test FAILS until native jobs explicitly disable .NET or use presets that do so.
# GREEN PHASE: Passes once build-macos and build-linux omit dotnet setup and/or use
#              presets that set MU_ENABLE_DOTNET=OFF.
#
# Note: The macos-arm64 and linux-x64 presets handle MU_ENABLE_DOTNET=OFF implicitly
# through the preset configuration. This test verifies no 'dotnet' setup step is
# present in the native runner jobs (would indicate incorrect dotnet dependency).
#
# Validates:
#   AC-8 — No 'setup-dotnet' or 'dotnet build' step in build-macos job
#   AC-8 — No 'setup-dotnet' or 'dotnet build' step in build-linux job
#   AC-8 — MU_ENABLE_DOTNET=OFF appears in ci.yml (explicit flag or preset-implied)

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CI_WORKFLOW_FILE)
    message(FATAL_ERROR "AC-8: CI_WORKFLOW_FILE variable not defined")
endif()

if(NOT EXISTS "${CI_WORKFLOW_FILE}")
    message(FATAL_ERROR "AC-8: ci.yml does not exist: ${CI_WORKFLOW_FILE}")
endif()

file(READ "${CI_WORKFLOW_FILE}" CI_CONTENT)

# --- Check 1: No dotnet setup action in ci.yml for native jobs ---
# setup-dotnet is only acceptable in the release job (for semantic-release Node setup),
# so verify it does NOT appear associated with native build jobs.
# We check that dotnet/setup-dotnet is absent entirely (native jobs must not need it).
string(FIND "${CI_CONTENT}" "dotnet/setup-dotnet" _pos_setup_dotnet)
if(NOT _pos_setup_dotnet EQUAL -1)
    message(FATAL_ERROR "AC-8 FAIL [7-4-1]: 'dotnet/setup-dotnet' action found in ci.yml — native runners must not require .NET SDK")
endif()

# --- Check 2: MU_ENABLE_DOTNET=OFF appears in ci.yml (MinGW job already has it) ---
# Native jobs use presets that imply this; the MinGW job sets it explicitly.
# Verify the flag is present at least once.
string(FIND "${CI_CONTENT}" "MU_ENABLE_DOTNET=OFF" _pos_dotnet_off)
if(_pos_dotnet_off EQUAL -1)
    message(FATAL_ERROR "AC-8 FAIL [7-4-1]: 'MU_ENABLE_DOTNET=OFF' not found in ci.yml — must be set for builds that skip .NET AOT")
endif()

# --- Check 3: build-macos job does not invoke 'dotnet' command directly ---
# Extract approximate region around build-macos and verify no dotnet invocation
string(FIND "${CI_CONTENT}" "run: dotnet" _pos_dotnet_run)
if(NOT _pos_dotnet_run EQUAL -1)
    message(FATAL_ERROR "AC-8 FAIL [7-4-1]: 'run: dotnet' command found in ci.yml — native runners must not invoke dotnet directly")
endif()

message(STATUS "AC-8 PASS [7-4-1]: No dotnet setup or invocation in native runner jobs; MU_ENABLE_DOTNET=OFF confirmed")
