#!/usr/bin/env bash
# AC-4: Validate SDL3 headers are NOT included in any game logic file
#
# Story: 1.3.1 - SDL3 Dependency Integration
# RED phase: This test PASSES initially (no SDL3 includes anywhere).
#            It acts as a regression guard to ensure SDL3 headers stay
#            confined to Platform/ and RenderFX/ source directories.
#
# Expected: Zero occurrences of #include <SDL3/ or #include "SDL3/ in
#           game logic directories (Network, World, Gameplay, UI, Scenes,
#           Core, Data, Audio, Dotnet, Main, ThirdParty, Protocol, Translation)

set -e

# Determine repo root relative to this script's location
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
SOURCE_ROOT="$REPO_ROOT/src/source"

# Game logic directories -- SDL3 must NOT appear in these
GAME_LOGIC_DIRS=(
    "$SOURCE_ROOT/Network"
    "$SOURCE_ROOT/World"
    "$SOURCE_ROOT/Gameplay"
    "$SOURCE_ROOT/UI"
    "$SOURCE_ROOT/Scenes"
    "$SOURCE_ROOT/Core"
    "$SOURCE_ROOT/Data"
    "$SOURCE_ROOT/Audio"
    "$SOURCE_ROOT/Dotnet"
    "$SOURCE_ROOT/Main"
    "$SOURCE_ROOT/ThirdParty"
    "$SOURCE_ROOT/Protocol"
    "$SOURCE_ROOT/Translation"
)

FOUND=0
for DIR in "${GAME_LOGIC_DIRS[@]}"; do
    if [ ! -d "$DIR" ]; then
        continue
    fi
    # Search for SDL3 include patterns
    if grep -r --include="*.h" --include="*.cpp" --include="*.hpp" \
        -E '#include\s*[<"]SDL3/' "$DIR" 2>/dev/null; then
        echo "ERROR: SDL3 headers found in game logic directory: $DIR"
        FOUND=1
    fi
done

# Also check that MUCommon headers don't propagate SDL3
COMMON_HEADERS="$SOURCE_ROOT/Common"
if [ -d "$COMMON_HEADERS" ]; then
    if grep -r --include="*.h" --include="*.hpp" \
        -E '#include\s*[<"]SDL3/' "$COMMON_HEADERS" 2>/dev/null; then
        echo "ERROR: SDL3 headers found in Common/ headers (would propagate via MUCommon INTERFACE)"
        FOUND=1
    fi
fi

if [ "$FOUND" -eq 1 ]; then
    echo ""
    echo "AC-4: FAILED -- SDL3 headers found in game logic directories"
    echo "SDL3 headers should only appear in Platform/ and RenderFX/ source directories"
    exit 1
fi

echo "AC-4: PASSED -- No SDL3 headers in game logic directories"
exit 0
