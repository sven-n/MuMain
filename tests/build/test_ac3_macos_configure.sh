#!/usr/bin/env bash
# AC-3: Validate cmake --preset macos-arm64 configure step succeeds
#
# Story: 1.1.1 - Create macOS CMake Toolchain & Presets
# RED phase: This test FAILS until:
#   1. The macos-arm64.cmake toolchain file exists
#   2. The macos-arm64 preset is defined in CMakePresets.json
#   3. Running on a macOS host with Clang and Ninja installed
#
# This test is designed to run on macOS only. On non-macOS hosts,
# it skips (exits 0) to avoid false failures.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Navigate to MuMain source root (tests/build/ -> tests/ -> MuMain/)
MUMAIN_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

# --- Guard: Skip on non-macOS hosts ---
if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "AC-3: SKIPPED -- Not running on macOS (host: $(uname -s))"
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

if ! command -v clang++ &>/dev/null; then
    echo "AC-3: FAILED -- clang++ not found in PATH"
    exit 1
fi

# Verify xcrun is available (macOS SDK detection)
if ! command -v xcrun &>/dev/null; then
    echo "AC-3: FAILED -- xcrun not found in PATH (Xcode Command Line Tools required)"
    exit 1
fi

# --- Check: Toolchain file exists ---
TOOLCHAIN_FILE="${MUMAIN_ROOT}/cmake/toolchains/macos-arm64.cmake"
if [[ ! -f "${TOOLCHAIN_FILE}" ]]; then
    echo "AC-3: FAILED -- Toolchain file does not exist: ${TOOLCHAIN_FILE}"
    exit 1
fi

# --- Check: Preset exists in CMakePresets.json ---
PRESETS_FILE="${MUMAIN_ROOT}/CMakePresets.json"
if ! grep -q '"macos-arm64"' "${PRESETS_FILE}"; then
    echo "AC-3: FAILED -- macos-arm64 preset not found in CMakePresets.json"
    exit 1
fi

# --- Main test: cmake --preset macos-arm64 configure ---
# Use the preset's own binaryDir (${sourceDir}/out/build/macos-arm64) to validate
# the full preset configuration including binaryDir. Clean up after.
PRESET_BUILD_DIR="${MUMAIN_ROOT}/out/build/macos-arm64"
trap 'rm -rf "${PRESET_BUILD_DIR}"' EXIT

echo "AC-3: Running cmake --preset macos-arm64 from ${MUMAIN_ROOT}..."
cd "${MUMAIN_ROOT}"

# Timeout after 120 seconds to avoid hanging in CI
TIMEOUT_CMD=""
if command -v timeout &>/dev/null; then
    TIMEOUT_CMD="timeout 120"
elif command -v gtimeout &>/dev/null; then
    TIMEOUT_CMD="gtimeout 120"
fi

if ${TIMEOUT_CMD} cmake --preset macos-arm64 2>&1; then
    echo "AC-3: PASSED -- cmake --preset macos-arm64 configure succeeded"
    exit 0
else
    echo "AC-3: FAILED -- cmake --preset macos-arm64 configure failed"
    exit 1
fi
