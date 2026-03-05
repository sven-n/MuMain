#!/usr/bin/env bash
# Story 1.2.1: AC-5 + AC-STD-3 — Verify no #ifdef _WIN32 in game logic files
# RED PHASE: This test validates that platform conditionals stay contained in Platform/ headers.
#
# This test checks that no game logic source files (outside Platform/) contain
# new #ifdef _WIN32 directives that were not there before.

set -euo pipefail

MUMAIN_SRC="${1:-$(dirname "$0")/../../src/source}"

# Count #ifdef _WIN32 in game logic files (excluding Platform/ directory and ThirdParty/)
# Use || true to handle grep returning exit 1 when no matches remain after filtering
count=$(grep -r '#ifdef _WIN32' "$MUMAIN_SRC" \
    --include='*.cpp' --include='*.h' \
    -l 2>/dev/null | \
    grep -v '/Platform/' | \
    grep -v '/ThirdParty/' | \
    grep -v '/Dotnet/' | \
    wc -l | tr -d ' ' || true)
count=${count:-0}

# Baseline: 0 game logic files contain #ifdef _WIN32 (established 2026-03-04)
BASELINE=0

echo "AC-5/AC-STD-3: Found $count game logic files with #ifdef _WIN32 (baseline: $BASELINE)"

if [ "$count" -gt "$BASELINE" ]; then
    echo "FAIL: New #ifdef _WIN32 leaked into game logic ($count > $BASELINE)"
    exit 1
fi

echo "AC-5/AC-STD-3: PASS — no new platform conditionals in game logic"
exit 0
