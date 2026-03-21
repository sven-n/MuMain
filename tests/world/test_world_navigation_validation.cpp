// Story 6.1.2: World Navigation Validation [VS1-GAME-VALIDATE-NAVIGATION]
// RED PHASE: Tests define expected logical contracts for world navigation validation.
//
// AC coverage (unit-testable subset — see manual test scenarios for full AC validation):
//   AC-1       — Pathfinding: PATH grid geometry (bounds checking, index math, heuristic)
//                + MovePoint() direction-to-coordinate mapping (click-to-move)
//   AC-2       — Map transitions: TW_* terrain attribute flags, MOVEINFODATA gate index contract
//   AC-5       — Key maps: ENUM_WORLD IDs for Lorencia, Devias, Noria, Dungeon, Lost Tower, Atlans
//   AC-STD-2   — Catch2 test suite validates navigation logic without live server
//
// Manual validation (full AC-1..5 on macOS/Linux/Windows):
//   See: _bmad-output/test-scenarios/epic-6/world-navigation-validation.md
//
// PCC Constraints:
//   - Platform-compatible includes: #ifdef _WIN32 / PlatformTypes.h on non-Win32
//   - No Win32 API calls in test logic — test logic only
//   - Catch2 TEST_CASE / SECTION / REQUIRE structure
//   - Allman brace style, 4-space indent, 120-column limit
//   - No raw new/delete in test code — PATH uses legacy internal allocation (legacy exemption)
//   - #pragma once not used (this is a .cpp file)
//
// Design notes:
//   - ENUM_WORLD: Pure enum in MapManager.h — no linkage needed, testable standalone.
//   - PATH geometry: CheckXYPos, GetIndex, EstimateCostToGoal are inline methods in ZzzPath.h.
//     Tested without calling FindPath() to avoid PATH::s_iDir (defined in zzzpath.cpp/MUGame).
//   - MovePoint(): Inline function in ZzzPath.h — testable standalone.
//   - MOVEINFODATA: Nested struct in CMoveCommandData — testable without singleton instantiation.
//   - CMapManager, CPortalMgr, PATH::FindPath: Implementation in MUGame target —
//     gated by MU_WORLD_NAVIGATION_TESTS_ENABLED (requires MUGame linkage).
//
// MUCommon INTERFACE propagates src/source/{Core,World,Data}/ to MuTests automatically.
// No additional target_include_directories entries needed for this test file.

#include <catch2/catch_test_macros.hpp>

#include <list>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include "Platform/PlatformTypes.h"
#endif

#include "mu_define.h"      // PATH constants, TW_* flags, EPathNodeState, EPathDirection,
                            // CHAOS_NUM, HELLAS_NUM, FACTOR_PATH_DIST, MAX_COUNT_PATH
#include "MapManager.h"     // ENUM_WORLD enum, CMapManager class
#include "ZzzPath.h"        // PATH class (inline methods), MovePoint()
#include "MoveCommandData.h" // SEASON3B::CMoveCommandData::MOVEINFODATA

// =============================================================================
// AC-5 [6-1-2]: ENUM_WORLD defines correct IDs for the 6 key game maps
// Story AC-5 specifies: Lorencia, Devias, Noria, Dungeon, Lost Tower, Atlans.
// These map IDs are used by LoadWorld(), portal targeting, and minimap display.
// =============================================================================

TEST_CASE("AC-5 [6-1-2]: ENUM_WORLD defines correct IDs for the 6 key game maps",
          "[world][navigation][maps][6-1-2]")
{
    SECTION("Lorencia is map 0 (primary starting city)")
    {
        REQUIRE(static_cast<int>(WD_0LORENCIA) == 0);
    }

    SECTION("Dungeon is map 1")
    {
        REQUIRE(static_cast<int>(WD_1DUNGEON) == 1);
    }

    SECTION("Devias is map 2")
    {
        REQUIRE(static_cast<int>(WD_2DEVIAS) == 2);
    }

    SECTION("Noria is map 3")
    {
        REQUIRE(static_cast<int>(WD_3NORIA) == 3);
    }

    SECTION("Lost Tower is map 4")
    {
        REQUIRE(static_cast<int>(WD_4LOSTTOWER) == 4);
    }

    SECTION("Atlans is map 7")
    {
        REQUIRE(static_cast<int>(WD_7ATLANSE) == 7);
    }
}

TEST_CASE("AC-5 [6-1-2]: ENUM_WORLD covers 82+ game world map slots",
          "[world][navigation][maps][6-1-2]")
{
    // NUM_WD must be >= 82: AC-5 validates all 82 maps are accessible on any platform.
    REQUIRE(static_cast<int>(NUM_WD) >= 82);
}

TEST_CASE("AC-5 [6-1-2]: ENUM_WORLD event map ranges use correct base IDs",
          "[world][navigation][maps][6-1-2]")
{
    SECTION("Blood Castle levels span 7 consecutive IDs from WD_11BLOODCASTLE1 == 11")
    {
        REQUIRE(static_cast<int>(WD_11BLOODCASTLE1) == 11);
        REQUIRE(static_cast<int>(WD_11BLOODCASTLE_END) == static_cast<int>(WD_11BLOODCASTLE1) + 6);
    }

    SECTION("Chaos Castle levels span CHAOS_NUM IDs from WD_18CHAOS_CASTLE == 18")
    {
        REQUIRE(static_cast<int>(WD_18CHAOS_CASTLE) == 18);
        REQUIRE(static_cast<int>(WD_18CHAOS_CASTLE_END) ==
                static_cast<int>(WD_18CHAOS_CASTLE) + (CHAOS_NUM - 1));
    }

    SECTION("Karutan maps (WD_80KARUTAN1, WD_81KARUTAN2) are the highest numbered standard maps")
    {
        REQUIRE(static_cast<int>(WD_80KARUTAN1) == 80);
        REQUIRE(static_cast<int>(WD_81KARUTAN2) == 81);
    }

    SECTION("Hellas map range spans (HELLAS_NUM - 1) entries from WD_24HELLAS == 24")
    {
        REQUIRE(static_cast<int>(WD_24HELLAS) == 24);
        REQUIRE(static_cast<int>(WD_24HELLAS_END) == static_cast<int>(WD_24HELLAS) + (HELLAS_NUM - 2));
    }
}

// =============================================================================
// AC-1 [6-1-2]: PATH grid geometry — bounds checking and index calculation
// PATH is the A* pathfinding class used for click-to-move character navigation.
// These tests validate the grid utility methods that underpin all movement decisions:
//   CheckXYPos() — wall/bounds detection for each step candidate
//   GetIndex()   — coordinate-to-array-index mapping used in all node lookups
//   EstimateCostToGoal() — A* heuristic weight for node prioritisation
// Tests call only inline methods (no FindPath) to avoid PATH::s_iDir (MUGame).
// =============================================================================

TEST_CASE("AC-1 [6-1-2]: PATH CheckXYPos enforces map grid boundaries",
          "[world][navigation][pathfinding][6-1-2]")
{
    // Set up a 10x10 walkable grid (all cells zero = passable)
    constexpr int kWidth = 10;
    constexpr int kHeight = 10;
    WORD mapData[kWidth * kHeight] = {};
    PATH path;
    path.SetMapDimensions(kWidth, kHeight, mapData);

    SECTION("Centre position (5, 5) is within bounds")
    {
        REQUIRE(path.CheckXYPos(5, 5) == TRUE);
    }

    SECTION("Origin (0, 0) is within bounds")
    {
        REQUIRE(path.CheckXYPos(0, 0) == TRUE);
    }

    SECTION("Bottom-right corner (width-1, height-1) is within bounds")
    {
        REQUIRE(path.CheckXYPos(kWidth - 1, kHeight - 1) == TRUE);
    }

    SECTION("Negative x position is out of bounds")
    {
        REQUIRE(path.CheckXYPos(-1, 5) == FALSE);
    }

    SECTION("Negative y position is out of bounds")
    {
        REQUIRE(path.CheckXYPos(5, -1) == FALSE);
    }

    SECTION("x == width is out of bounds (off-by-one guard)")
    {
        REQUIRE(path.CheckXYPos(kWidth, 5) == FALSE);
    }

    SECTION("y == height is out of bounds (off-by-one guard)")
    {
        REQUIRE(path.CheckXYPos(5, kHeight) == FALSE);
    }
}

TEST_CASE("AC-1 [6-1-2]: PATH GetIndex maps 2D coordinates to flat array index",
          "[world][navigation][pathfinding][6-1-2]")
{
    // Typical MU Online map dimensions: 256x256. Use a 16x16 grid for the test.
    constexpr int kWidth = 16;
    constexpr int kHeight = 16;
    WORD mapData[kWidth * kHeight] = {};
    PATH path;
    path.SetMapDimensions(kWidth, kHeight, mapData);

    SECTION("Origin (0, 0) maps to index 0")
    {
        REQUIRE(path.GetIndex(0, 0) == 0);
    }

    SECTION("(1, 0) maps to index 1 (row-major, first row)")
    {
        REQUIRE(path.GetIndex(1, 0) == 1);
    }

    SECTION("(0, 1) maps to index width (first cell of second row)")
    {
        REQUIRE(path.GetIndex(0, 1) == kWidth);
    }

    SECTION("General case: (x, y) maps to x + y * width")
    {
        REQUIRE(path.GetIndex(3, 4) == 3 + 4 * kWidth);
        REQUIRE(path.GetIndex(7, 2) == 7 + 2 * kWidth);
    }
}

TEST_CASE("AC-1 [6-1-2]: PATH EstimateCostToGoal heuristic properties",
          "[world][navigation][pathfinding][6-1-2]")
{
    // EstimateCostToGoal(xGoal, yGoal, xCurrent, yCurrent) — A* heuristic.
    // Computes Chebyshev-style cost weighted by FACTOR_PATH_DIST / FACTOR_PATH_DIST_DIAG.
    constexpr int kWidth = 20;
    constexpr int kHeight = 20;
    WORD mapData[kWidth * kHeight] = {};
    PATH path;
    path.SetMapDimensions(kWidth, kHeight, mapData);

    SECTION("Cost from current position to itself is 0")
    {
        // xDist=0, yDist=0 → (0*FACTOR_PATH_DIST + 0*FACTOR_PATH_DIST_DIAG + 1)*3/4 = 0
        REQUIRE(path.EstimateCostToGoal(10, 10, 10, 10) == 0);
    }

    SECTION("Cost to adjacent cell is positive")
    {
        REQUIRE(path.EstimateCostToGoal(10, 10, 11, 10) > 0);
    }

    SECTION("Cost increases monotonically with distance along an axis")
    {
        int nearCost = path.EstimateCostToGoal(10, 10, 11, 10);
        int farCost = path.EstimateCostToGoal(10, 10, 15, 10);
        REQUIRE(farCost > nearCost);
    }

    SECTION("Cost is symmetric: swapping goal and current gives same result")
    {
        int forwardCost = path.EstimateCostToGoal(10, 10, 14, 10);
        int reverseCost = path.EstimateCostToGoal(14, 10, 10, 10);
        REQUIRE(forwardCost == reverseCost);
    }
}

// =============================================================================
// AC-1 [6-1-2]: MovePoint direction mapping — character movement direction logic
// MovePoint() translates EPathDirection enum values to coordinate deltas.
// Used by the movement system to advance a character along the computed path.
// This is a pure function with no external dependencies — fully testable standalone.
// =============================================================================

TEST_CASE("AC-1 [6-1-2]: MovePoint maps all 8 EPathDirection values to correct coordinate deltas",
          "[world][navigation][movement][6-1-2]")
{
    const POINT origin = {10, 10};

    SECTION("EAST moves (+x, +y)")
    {
        POINT result = MovePoint(EPathDirection::EAST, origin);
        REQUIRE(result.x == 11);
        REQUIRE(result.y == 11);
    }

    SECTION("WEST moves (-x, -y)")
    {
        POINT result = MovePoint(EPathDirection::WEST, origin);
        REQUIRE(result.x == 9);
        REQUIRE(result.y == 9);
    }

    SECTION("NORTH moves (-x, +y)")
    {
        POINT result = MovePoint(EPathDirection::NORTH, origin);
        REQUIRE(result.x == 9);
        REQUIRE(result.y == 11);
    }

    SECTION("SOUTH moves (+x, -y)")
    {
        POINT result = MovePoint(EPathDirection::SOUTH, origin);
        REQUIRE(result.x == 11);
        REQUIRE(result.y == 9);
    }

    SECTION("NORTHEAST moves (0, +y)")
    {
        POINT result = MovePoint(EPathDirection::NORTHEAST, origin);
        REQUIRE(result.x == 10);
        REQUIRE(result.y == 11);
    }

    SECTION("SOUTHWEST moves (0, -y)")
    {
        POINT result = MovePoint(EPathDirection::SOUTHWEST, origin);
        REQUIRE(result.x == 10);
        REQUIRE(result.y == 9);
    }

    SECTION("SOUTHEAST moves (+x, 0)")
    {
        POINT result = MovePoint(EPathDirection::SOUTHEAST, origin);
        REQUIRE(result.x == 11);
        REQUIRE(result.y == 10);
    }

    SECTION("NORTHWEST moves (-x, 0)")
    {
        POINT result = MovePoint(EPathDirection::NORTHWEST, origin);
        REQUIRE(result.x == 9);
        REQUIRE(result.y == 10);
    }
}

// =============================================================================
// AC-2 [6-1-2]: Map transition gate data — CMoveCommandData::MOVEINFODATA contract
// MOVEINFODATA stores warp gate requirements: level, zen, and gate index.
// The operator==(int) is used by std::list::find to locate gates by index.
// This validates the struct layout and equality contract used in portal lookups.
// =============================================================================

TEST_CASE("AC-2 [6-1-2]: MOVEINFODATA index equality operator matches gate by index",
          "[world][navigation][portal][warp][6-1-2]")
{
    SEASON3B::CMoveCommandData::MOVEINFODATA data = {};
    data._ReqInfo.index = 17;
    data._bCanMove = false;
    data._bStrife = false;
    data._bSelected = false;

    SECTION("Matches when iIndex equals _ReqInfo.index")
    {
        REQUIRE((data == 17) == true);
    }

    SECTION("Does not match when iIndex differs")
    {
        REQUIRE((data == 0) == false);
        REQUIRE((data == 18) == false);
    }

    SECTION("Adjacent gate index is rejected (no off-by-one tolerance)")
    {
        REQUIRE((data == 16) == false);
    }
}

TEST_CASE("AC-2 [6-1-2]: MOVEINFODATA _bCanMove flag is default-constructible and distinguishes gate states",
          "[world][navigation][portal][warp][6-1-2]")
{
    SECTION("Default-constructed MOVEINFODATA has _bCanMove == false (gates locked by default)")
    {
        SEASON3B::CMoveCommandData::MOVEINFODATA data = {};
        REQUIRE(data._bCanMove == false);
    }

    SECTION("Passable and blocked gates are distinguishable via _bCanMove")
    {
        SEASON3B::CMoveCommandData::MOVEINFODATA passable = {};
        passable._bCanMove = true;

        SEASON3B::CMoveCommandData::MOVEINFODATA blocked = {};
        blocked._bCanMove = false;

        REQUIRE(passable._bCanMove != blocked._bCanMove);
    }

    SECTION("_bCanMove is independent of gate index")
    {
        SEASON3B::CMoveCommandData::MOVEINFODATA data = {};
        data._ReqInfo.index = 42;
        data._bCanMove = true;
        REQUIRE(data._bCanMove == true);
        REQUIRE((data == 42) == true);
    }
}

// =============================================================================
// AC-2 [6-1-2]: Terrain attribute flags — TW_* constants used by pathfinding
// PATH::FindPath() uses TW_* flags to classify each map cell's walkability.
// These constants must have correct values for cross-platform builds to navigate
// identically on macOS/Linux/Windows (same bit positions = same path decisions).
// =============================================================================

TEST_CASE("AC-2 [6-1-2]: TW_* terrain attribute flags are distinct non-overlapping bitmasks",
          "[world][navigation][terrain][6-1-2]")
{
    SECTION("TW_SAFEZONE is bit 0 (0x0001)")
    {
        REQUIRE(TW_SAFEZONE == 0x0001);
    }

    SECTION("TW_CHARACTER is bit 1 (0x0002) — character collision")
    {
        REQUIRE(TW_CHARACTER == 0x0002);
    }

    SECTION("TW_NOMOVE is bit 2 (0x0004) — blocks pathfinding")
    {
        REQUIRE(TW_NOMOVE == 0x0004);
    }

    SECTION("TW_ACTION is bit 5 (0x0020) — interactive zone, subtracted in path search")
    {
        REQUIRE(TW_ACTION == 0x0020);
    }

    SECTION("TW_HEIGHT is bit 6 (0x0040) — elevation marker, subtracted in path search")
    {
        REQUIRE(TW_HEIGHT == 0x0040);
    }

    SECTION("TW_CAMERA_UP is bit 7 (0x0080) — camera hint, subtracted in path search")
    {
        REQUIRE(TW_CAMERA_UP == 0x0080);
    }

    SECTION("All TW_* flags used in FindPath() pathfinding are mutually non-overlapping")
    {
        // All 15 pairs among the 6 TW_* flags
        REQUIRE((TW_SAFEZONE & TW_CHARACTER) == 0);
        REQUIRE((TW_SAFEZONE & TW_NOMOVE) == 0);
        REQUIRE((TW_SAFEZONE & TW_ACTION) == 0);
        REQUIRE((TW_SAFEZONE & TW_HEIGHT) == 0);
        REQUIRE((TW_SAFEZONE & TW_CAMERA_UP) == 0);
        REQUIRE((TW_CHARACTER & TW_NOMOVE) == 0);
        REQUIRE((TW_CHARACTER & TW_ACTION) == 0);
        REQUIRE((TW_CHARACTER & TW_HEIGHT) == 0);
        REQUIRE((TW_CHARACTER & TW_CAMERA_UP) == 0);
        REQUIRE((TW_NOMOVE & TW_ACTION) == 0);
        REQUIRE((TW_NOMOVE & TW_HEIGHT) == 0);
        REQUIRE((TW_NOMOVE & TW_CAMERA_UP) == 0);
        REQUIRE((TW_ACTION & TW_HEIGHT) == 0);
        REQUIRE((TW_ACTION & TW_CAMERA_UP) == 0);
        REQUIRE((TW_HEIGHT & TW_CAMERA_UP) == 0);
    }
}

// =============================================================================
// AC-1, AC-2, AC-4 [6-1-2]: Tests requiring MUGame linkage
// These tests need non-inline implementations from MUGame:
//   - CMapManager::InChaosCastle/InBloodCastle/InHellas (MapManager.cpp)
//   - CPortalMgr::SavePortalPosition/IsPortalPositionSaved (PortalMgr.cpp)
//   - PATH::FindPath (uses PATH::s_iDir from zzzpath.cpp)
//
// Enable by defining MU_WORLD_NAVIGATION_TESTS_ENABLED and linking MuTests against MUGame.
// =============================================================================

#ifdef MU_WORLD_NAVIGATION_TESTS_ENABLED

// MUGame-linked tests for story 6-1-2 are not yet implemented.
// When MUGame linkage is available, replace this static_assert with real test
// implementations for CMapManager range queries, CPortalMgr state, and PATH::FindPath.
static_assert(false,
              "MU_WORLD_NAVIGATION_TESTS_ENABLED is defined but MUGame-linked tests "
              "for story 6-1-2 are not yet implemented. Remove this flag or add the tests.");

#else

TEST_CASE("AC-2 [6-1-2]: CMapManager map range queries — requires MUGame linkage",
          "[world][navigation][maps][portal][6-1-2]")
{
    SKIP("MU_WORLD_NAVIGATION_TESTS_ENABLED not defined: CMapManager::InChaosCastle/"
         "InBloodCastle/InHellas are non-inline (MapManager.cpp in MUGame). "
         "Set -DMU_WORLD_NAVIGATION_TESTS_ENABLED and link MuTests against MUGame to enable.");
}

TEST_CASE("AC-2 [6-1-2]: CPortalMgr portal state save/restore — requires MUGame linkage",
          "[world][navigation][portal][6-1-2]")
{
    SKIP("MU_WORLD_NAVIGATION_TESTS_ENABLED not defined: CPortalMgr constructor/methods "
         "are non-inline (PortalMgr.cpp in MUGame). "
         "Set -DMU_WORLD_NAVIGATION_TESTS_ENABLED and link MuTests against MUGame to enable.");
}

TEST_CASE("AC-1 [6-1-2]: PATH FindPath A* navigation — requires MUGame linkage",
          "[world][navigation][pathfinding][6-1-2]")
{
    SKIP("MU_WORLD_NAVIGATION_TESTS_ENABLED not defined: PATH::FindPath uses PATH::s_iDir "
         "defined in zzzpath.cpp (MUGame target). "
         "Set -DMU_WORLD_NAVIGATION_TESTS_ENABLED and link MuTests against MUGame to enable.");
}

#endif // MU_WORLD_NAVIGATION_TESTS_ENABLED
