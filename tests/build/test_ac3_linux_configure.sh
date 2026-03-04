#!/usr/bin/env bash
# AC-3: Validate cmake --preset linux-x64 configure step succeeds
#
# Story: 1.1.2 - Create Linux CMake Toolchain & Presets
# RED phase: This test FAILS until:
#   1. The linux-x64.cmake toolchain file exists
#   2. The linux-x64 preset is defined in CMakePresets.json
#   3. Running on a Linux x64 host with GCC and Ninja installed
#
# This test is designed to run on Linux only. On non-Linux hosts,
# it skips (exits 0) to avoid false failures.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Navigate to MuMain source root (tests/build/ -> tests/ -> MuMain/)
MUMAIN_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

# --- Guard: Skip on non-Linux hosts ---
if [[ "$(uname -s)" != "Linux" ]]; then
    echo "AC-3: SKIPPED -- Not running on Linux (host: $(uname -s))"
    exit 0
fi

# --- Guard: Check required tools ---
if ! command -v cmake &>/dev/null; then
    echo "AC-3: FAILED -- cmake not found in PATH"
    exit 1
fi

if ! command -v ninja &>/dev/null; then
    echo "AC-3: FAILED -- ninja not found in PATH"
    exit 1
fi

if ! command -v g++ &>/dev/null; then
    echo "AC-3: FAILED -- g++ not found in PATH"
    exit 1
fi

# Verify GCC version >= 10 (required for C++20 support)
GCC_VERSION=$(g++ -dumpversion | cut -d. -f1)
if [[ "${GCC_VERSION}" -lt 10 ]]; then
    echo "AC-3: FAILED -- g++ version ${GCC_VERSION} is too old (requires GCC 10+ for C++20)"
    exit 1
fi

# --- Check: Toolchain file exists ---
TOOLCHAIN_FILE="${MUMAIN_ROOT}/cmake/toolchains/linux-x64.cmake"
if [[ ! -f "${TOOLCHAIN_FILE}" ]]; then
    echo "AC-3: FAILED -- Toolchain file does not exist: ${TOOLCHAIN_FILE}"
    exit 1
fi

# --- Check: Preset exists in CMakePresets.json ---
PRESETS_FILE="${MUMAIN_ROOT}/CMakePresets.json"
if ! grep -q '"linux-x64"' "${PRESETS_FILE}"; then
    echo "AC-3: FAILED -- linux-x64 preset not found in CMakePresets.json"
    exit 1
fi

# --- Main test: cmake --preset linux-x64 configure ---
# Use the preset's own binaryDir (${sourceDir}/out/build/linux-x64) to validate
# the full preset configuration including binaryDir. Clean up after.
PRESET_BUILD_DIR="${MUMAIN_ROOT}/out/build/linux-x64"
trap 'rm -rf "${PRESET_BUILD_DIR}"' EXIT

echo "AC-3: Running cmake --preset linux-x64 from ${MUMAIN_ROOT}..."
cd "${MUMAIN_ROOT}"

if cmake --preset linux-x64 2>&1; then
    echo "AC-3: PASSED -- cmake --preset linux-x64 configure succeeded"
    exit 0
else
    echo "AC-3: FAILED -- cmake --preset linux-x64 configure failed"
    exit 1
fi
