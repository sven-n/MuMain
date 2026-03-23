// Story 6.4.1: UI Windows Comprehensive Validation [VS1-GAME-VALIDATE-UI]
// RED PHASE: Component tests define expected structural contracts for all 84+ UI windows.
//
// AC coverage (unit-testable subset — see manual test scenarios for full AC validation):
//   AC-1  — UI framework class hierarchy: INewUIBase is pure-virtual (abstract), CNewUIObj
//            is also abstract (delegates Render/Update/GetLayerDepth to concrete subclasses),
//            CNewUIObj default visibility=true/enabled=true, GetKeyEventOrder() default=3.0f,
//            Show()/Enable() state transitions
//            (class hierarchy tests gated by MU_GAME_AVAILABLE)
//   AC-2  — Window dimension constants validated: INVENTORY_SQUARE_WIDTH=20,
//            INVENTORY_SQUARE_HEIGHT=20, CNewUIMiniMap::MASTER_DATA skill icon geometry,
//            TOOLTIP_TYPE range, SQUARE_COLOR_STATE range
//            (constants tests gated by MU_GAME_AVAILABLE)
//   AC-3  — UI framework component enum integrity: BUTTON_STATE (UP=0, DOWN=1, OVER=2),
//            RADIOGROUPEVENT_NONE=-1, TOOLTIP_TYPE enum coverage, SQUARE_COLOR_STATE coverage
//            (component enum tests gated by MU_GAME_AVAILABLE)
//   AC-4  — INTERFACE_LIST enum completeness: INTERFACE_BEGIN=0x00, INTERFACE_COUNT>=84,
//            INTERFACE_END==INTERFACE_COUNT+2, camera range spans 25 slots,
//            key window IDs (inventory, character, chat, minimap, skills) pairwise distinct
//            across all categories (HUD, Social, Castle, Events, Quest)
//            (INTERFACE_LIST from mu_enum.h — standalone, no MU_GAME_AVAILABLE needed)
//   AC-5  — SSIM ground truth infrastructure validated for key UI window buffer sizes:
//            identical 190x429 inventory buffers score >= 0.99, perceptible difference
//            detected (black vs white scores < 0.5), identical 320x240 minimap buffers
//            score >= 0.99; 5 key window IDs pairwise distinct in INTERFACE_LIST
//
// Manual validation (full AC-1..5 on macOS/Linux/Windows with live server):
//   See: _bmad-output/test-scenarios/epic-6/ui-windows-validation.md
//
// PCC Constraints:
//   - Platform-compatible includes: #ifdef _WIN32 / PlatformTypes.h on non-Win32
//   - No Win32 API calls in test logic — test logic only
//   - Catch2 TEST_CASE / SECTION / REQUIRE / static_assert structure
//   - Allman braces, 4-space indent, 120-column limit
//   - No raw new/delete in test code
//   - #ifdef MU_GAME_AVAILABLE gates tests requiring full game includes (UI class headers)
//
// Design notes:
//   - mu_enum.h: INTERFACE_LIST enum (SEASON3B namespace) — platform-compatible standalone.
//   - NewUIBase.h: INewUIBase (abstract), CNewUIObj (abstract base) — gated by MU_GAME_AVAILABLE.
//   - NewUIButton.h: BUTTON_STATE, RADIOGROUPEVENT — gated by MU_GAME_AVAILABLE.
//   - NewUIInventoryCtrl.h: INVENTORY_SQUARE_*, TOOLTIP_TYPE, SQUARE_COLOR_STATE — gated.
//   - NewUIMiniMap.h: CNewUIMiniMap::MASTER_DATA enum — gated by MU_GAME_AVAILABLE.
//   - GroundTruthCapture.h: ComputeSSIM() — standalone (no Win32/GL) for AC-5 SSIM tests.
//
// MUCommon INTERFACE propagates all src/source/ subdirectory include paths to MuTests.
// No additional target_include_directories entries needed for this test file.

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <cstdint>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include "Platform/PlatformTypes.h"
#endif

#include "mu_enum.h" // INTERFACE_LIST (SEASON3B namespace), SEASON3A::eCursedTempleState

// GroundTruthCapture.h is standalone — ComputeSSIM() has no Win32/GL dependencies.
// ComputeSSIM() is always compiled (no ENABLE_GROUND_TRUTH_CAPTURE guard).
#include "GroundTruthCapture.h"

// UI class headers depend on Win32 types (POINT, HWND) and deep include chains
// (ZzzTexture.h, NewUI3DRenderMng.h → OpenGL). Gate them under MU_GAME_AVAILABLE.
// To enable: build MuTests with -DMU_GAME_AVAILABLE (requires Win32 + OpenGL + MUGame).
#ifdef MU_GAME_AVAILABLE
#include <type_traits>
#include "NewUIBase.h"          // INewUIBase (pure-virtual interface), CNewUIObj (abstract base)
#include "NewUIButton.h"        // BUTTON_STATE, RADIOGROUPEVENT_NONE, CNewUIBaseButton
#include "NewUIInventoryCtrl.h" // INVENTORY_SQUARE_WIDTH/HEIGHT, TOOLTIP_TYPE, SQUARE_COLOR_STATE
#include "NewUIMiniMap.h"       // CNewUIMiniMap::MASTER_DATA skill icon constants
#endif                          // MU_GAME_AVAILABLE

// =============================================================================
// AC-4 [6-4-1]: INTERFACE_LIST enum completeness and window type registry coverage
// The INTERFACE_LIST enum in mu_enum.h (SEASON3B namespace) defines the unique DWORD key
// for every UI window registered in CNewUIManager. All 84+ windows must have distinct IDs.
// INTERFACE_COUNT captures the total registerable window slots (excluding sentinels).
// =============================================================================

TEST_CASE("AC-4 [6-4-1]: INTERFACE_LIST boundary values and count validate 84+ window coverage",
    "[ui][interface][enum][6-4-1]")
{
    using namespace SEASON3B;

    SECTION("INTERFACE_BEGIN is 0x00 — ID space starts at zero (sentinel, not a real window)")
    {
        REQUIRE(INTERFACE_BEGIN == 0x00);
    }

    SECTION("INTERFACE_COUNT is at least 84 — covers all 84+ registered UI window types")
    {
        REQUIRE(INTERFACE_COUNT >= 84);
    }

    SECTION("INTERFACE_END equals INTERFACE_COUNT plus 2 — sentinel bookkeeping is self-consistent")
    {
        REQUIRE(INTERFACE_END == INTERFACE_COUNT + 2);
    }

    SECTION("INTERFACE_3DRENDERING_CAMERA range spans exactly 25 slots (BEGIN to BEGIN+24)")
    {
        REQUIRE(INTERFACE_3DRENDERING_CAMERA_END == INTERFACE_3DRENDERING_CAMERA_BEGIN + 24);
    }

    SECTION("Camera range BEGIN is positive (allocated after standard window IDs)")
    {
        REQUIRE(INTERFACE_3DRENDERING_CAMERA_BEGIN > 0);
    }
}

TEST_CASE("AC-4 [6-4-1]: HUD and core UI window IDs are present and pairwise distinct",
    "[ui][interface][hud][distinct][6-4-1]")
{
    using namespace SEASON3B;

    SECTION("Core HUD and gameplay window IDs are pairwise distinct")
    {
        const int coreWindows[] = {
            INTERFACE_MAINFRAME,
            INTERFACE_INVENTORY,
            INTERFACE_CHARACTER,
            INTERFACE_CHATINPUTBOX,
            INTERFACE_CHATLOGWINDOW,
            INTERFACE_MINI_MAP,
            INTERFACE_SKILL_LIST,
            INTERFACE_BUFF_WINDOW,
            INTERFACE_OPTION,
            INTERFACE_WINDOW_MENU,
            INTERFACE_HELP,
            INTERFACE_NAME_WINDOW,
        };
        constexpr int n = static_cast<int>(sizeof(coreWindows) / sizeof(coreWindows[0]));
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                REQUIRE(coreWindows[i] != coreWindows[j]);
            }
        }
    }
}

TEST_CASE("AC-4 [6-4-1]: Inventory and commerce window IDs are pairwise distinct",
    "[ui][interface][inventory][commerce][distinct][6-4-1]")
{
    using namespace SEASON3B;

    SECTION("Inventory and commerce window IDs are pairwise distinct")
    {
        const int inventoryWindows[] = {
            INTERFACE_NPCSHOP,
            INTERFACE_TRADE,
            INTERFACE_NPCBREEDER,
            INTERFACE_INVENTORY_EXT,
            INTERFACE_MYSHOP_INVENTORY,
            INTERFACE_PURCHASESHOP_INVENTORY,
            INTERFACE_MIXINVENTORY,
            INTERFACE_STORAGE,
            INTERFACE_STORAGE_EXT,
            INTERFACE_LUCKYCOIN_REGISTRATION,
            INTERFACE_EXCHANGE_LUCKYCOIN,
            INTERFACE_LUCKYITEMWND,
        };
        constexpr int n = static_cast<int>(sizeof(inventoryWindows) / sizeof(inventoryWindows[0]));
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                REQUIRE(inventoryWindows[i] != inventoryWindows[j]);
            }
        }
    }
}

TEST_CASE("AC-4 [6-4-1]: Social window IDs are pairwise distinct",
    "[ui][interface][social][distinct][6-4-1]")
{
    using namespace SEASON3B;

    SECTION("Social window IDs are pairwise distinct")
    {
        const int socialWindows[] = {
            INTERFACE_FRIEND,
            INTERFACE_PARTY,
            INTERFACE_GUILDINFO,
            INTERFACE_PARTY_INFO_WINDOW,
            INTERFACE_GENSRANKING,
            INTERFACE_NPCGUILDMASTER,
        };
        constexpr int n = static_cast<int>(sizeof(socialWindows) / sizeof(socialWindows[0]));
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                REQUIRE(socialWindows[i] != socialWindows[j]);
            }
        }
    }
}

TEST_CASE("AC-4 [6-4-1]: Castle window IDs are pairwise distinct",
    "[ui][interface][castle][distinct][6-4-1]")
{
    using namespace SEASON3B;

    SECTION("Castle window IDs are pairwise distinct")
    {
        const int castleWindows[] = {
            INTERFACE_GUARDSMAN,
            INTERFACE_SENATUS,
            INTERFACE_GATEKEEPER,
            INTERFACE_GATESWITCH,
            INTERFACE_CATAPULT,
            INTERFACE_SIEGEWARFARE,
        };
        constexpr int n = static_cast<int>(sizeof(castleWindows) / sizeof(castleWindows[0]));
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                REQUIRE(castleWindows[i] != castleWindows[j]);
            }
        }
    }
}

TEST_CASE("AC-4 [6-4-1]: Event window IDs are pairwise distinct",
    "[ui][interface][events][distinct][6-4-1]")
{
    using namespace SEASON3B;

    SECTION("Event window IDs are pairwise distinct")
    {
        const int eventWindows[] = {
            INTERFACE_BLOODCASTLE,
            INTERFACE_BLOODCASTLE_TIME,
            INTERFACE_DEVILSQUARE,
            INTERFACE_CHAOSCASTLE_TIME,
            INTERFACE_CURSEDTEMPLE_NPC,
            INTERFACE_CURSEDTEMPLE_GAMESYSTEM,
            INTERFACE_CURSEDTEMPLE_RESULT,
            INTERFACE_DUEL_WINDOW,
            INTERFACE_CRYWOLF,
            INTERFACE_DOPPELGANGER_NPC,
            INTERFACE_DOPPELGANGER_FRAME,
            INTERFACE_EMPIREGUARDIAN_NPC,
            INTERFACE_EMPIREGUARDIAN_TIMER,
            INTERFACE_BATTLE_SOCCER_SCORE,
            INTERFACE_DUELWATCH,
            INTERFACE_DUELWATCH_MAINFRAME,
            INTERFACE_DUELWATCH_USERLIST,
            INTERFACE_KANTURU2ND_ENTERNPC,
            INTERFACE_KANTURU_INFO,
        };
        constexpr int n = static_cast<int>(sizeof(eventWindows) / sizeof(eventWindows[0]));
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                REQUIRE(eventWindows[i] != eventWindows[j]);
            }
        }
    }
}

TEST_CASE("AC-4 [6-4-1]: Quest and MuHelper window IDs are pairwise distinct",
    "[ui][interface][quest][muhelper][distinct][6-4-1]")
{
    using namespace SEASON3B;

    SECTION("Quest window IDs are pairwise distinct")
    {
        const int questWindows[] = {
            INTERFACE_MYQUEST,
            INTERFACE_NPCQUEST,
            INTERFACE_QUEST_PROGRESS,
            INTERFACE_QUEST_PROGRESS_ETC,
        };
        constexpr int n = static_cast<int>(sizeof(questWindows) / sizeof(questWindows[0]));
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                REQUIRE(questWindows[i] != questWindows[j]);
            }
        }
    }

    SECTION("MuHelper window IDs are pairwise distinct")
    {
        const int muHelperWindows[] = {
            INTERFACE_MUHELPER,
            INTERFACE_MUHELPER_EXT,
            INTERFACE_MUHELPER_SKILL_LIST,
        };
        constexpr int n = static_cast<int>(sizeof(muHelperWindows) / sizeof(muHelperWindows[0]));
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                REQUIRE(muHelperWindows[i] != muHelperWindows[j]);
            }
        }
    }
}

// =============================================================================
// AC-5 [6-4-1]: SSIM ground truth comparison infrastructure for key UI windows
// The 5 key windows (inventory, character info, skills, map, chat) are identified by
// their INTERFACE_LIST IDs. ComputeSSIM validates that identical buffers score >= 0.99
// and perceptibly different buffers score < 0.5 — confirming the SSIM infrastructure
// is ready for UI regression testing with live rendered frames.
// =============================================================================

TEST_CASE("AC-5 [6-4-1]: The 5 key UI window IDs for SSIM comparison are registered and distinct",
    "[ui][ssim][key-windows][6-4-1]")
{
    using namespace SEASON3B;

    SECTION("5 key SSIM ground truth window IDs are pairwise distinct")
    {
        // Inventory, character info, skills, map, chat — the 5 priority windows per AC-5
        const int keyWindows[] = {
            INTERFACE_INVENTORY,    // Inventory window (190x429 px)
            INTERFACE_CHARACTER,    // Character info window
            INTERFACE_SKILL_LIST,   // Skills / hotbar skill list
            INTERFACE_MINI_MAP,     // Map window
            INTERFACE_CHATINPUTBOX, // Chat input window
        };
        constexpr int n = static_cast<int>(sizeof(keyWindows) / sizeof(keyWindows[0]));
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                REQUIRE(keyWindows[i] != keyWindows[j]);
            }
        }
    }

    SECTION("All 5 key window IDs fall within the valid INTERFACE_LIST range")
    {
        using namespace SEASON3B;
        const int keyWindows[] = {
            INTERFACE_INVENTORY,
            INTERFACE_CHARACTER,
            INTERFACE_SKILL_LIST,
            INTERFACE_MINI_MAP,
            INTERFACE_CHATINPUTBOX,
        };
        constexpr int n = static_cast<int>(sizeof(keyWindows) / sizeof(keyWindows[0]));
        for (int i = 0; i < n; ++i)
        {
            REQUIRE(keyWindows[i] > INTERFACE_BEGIN);
            REQUIRE(keyWindows[i] < INTERFACE_END);
        }
    }
}

TEST_CASE("AC-5 [6-4-1]: SSIM infrastructure validates identical inventory-sized buffers",
    "[ui][ssim][inventory][6-4-1]")
{
    // Inventory window dimensions: INVENTORY_WIDTH=190.0f, INVENTORY_HEIGHT=429.0f
    // (per CNewUIMyInventory private constexpr in NewUIMyInventory.h)
    constexpr int width = 190;
    constexpr int height = 429;
    constexpr int channels = 3;
    const std::size_t size = static_cast<std::size_t>(width * height * channels);

    SECTION("Identical 190x429 inventory-sized buffers produce SSIM >= 0.99")
    {
        std::vector<unsigned char> imgA(size, 64u);
        std::vector<unsigned char> imgB(size, 64u);

        double score = mu::GroundTruthCapture::ComputeSSIM(
            imgA.data(), imgB.data(), width, height, channels);

        REQUIRE(score >= 0.99);
    }

    SECTION("Perceptibly different 190x429 inventory buffers produce SSIM < 0.5")
    {
        std::vector<unsigned char> imgA(size, 0u);   // black background
        std::vector<unsigned char> imgB(size, 255u); // white background

        double score = mu::GroundTruthCapture::ComputeSSIM(
            imgA.data(), imgB.data(), width, height, channels);

        REQUIRE(score < 0.5);
    }
}

TEST_CASE("AC-5 [6-4-1]: SSIM infrastructure validates identical minimap-sized buffers",
    "[ui][ssim][minimap][6-4-1]")
{
    // Minimap uses a typical 320x240 display region for ground truth comparison
    constexpr int width = 320;
    constexpr int height = 240;
    constexpr int channels = 3;
    const std::size_t size = static_cast<std::size_t>(width * height * channels);

    SECTION("Identical 320x240 minimap-sized buffers produce SSIM >= 0.99")
    {
        std::vector<unsigned char> imgA(size, 128u);
        std::vector<unsigned char> imgB(size, 128u);

        double score = mu::GroundTruthCapture::ComputeSSIM(
            imgA.data(), imgB.data(), width, height, channels);

        REQUIRE(score >= 0.99);
    }
}

// =============================================================================
// Tests below require full MUGame includes (UI class headers, Win32 types, OpenGL).
// Build with -DMU_GAME_AVAILABLE to include them in the test run.
// =============================================================================

#ifdef MU_GAME_AVAILABLE

namespace
{
// Minimal concrete subclass for testing CNewUIObj state methods.
// CNewUIObj is abstract (delegates Render/Update/UpdateMouseEvent/UpdateKeyEvent/
// GetLayerDepth to derived classes). This stub implements those pure virtuals.
struct TestUIWindow : public SEASON3B::CNewUIObj
{
    bool Render() override
    {
        return true;
    }
    bool Update() override
    {
        return true;
    }
    bool UpdateMouseEvent() override
    {
        return true;
    }
    bool UpdateKeyEvent() override
    {
        return true;
    }
    float GetLayerDepth() override
    {
        return 1.0f;
    }
};
} // anonymous namespace

// =============================================================================
// AC-1 [6-4-1]: UI framework class hierarchy validated
// INewUIBase is the pure-virtual interface. CNewUIObj is the abstract base class;
// all 84+ CNewUI* window classes derive from it and implement the remaining pure virtuals.
// CNewUIObj implements: IsVisible(), IsEnabled(), Show(), Enable(), GetKeyEventOrder().
// =============================================================================

TEST_CASE("AC-1 [6-4-1]: INewUIBase is a pure-virtual (abstract) interface",
    "[ui][hierarchy][abstract][6-4-1]")
{
    SECTION("INewUIBase is an abstract type — cannot be instantiated directly")
    {
        REQUIRE(std::is_abstract_v<SEASON3B::INewUIBase>);
    }
}

TEST_CASE("AC-1 [6-4-1]: CNewUIObj is an abstract base class derived from INewUIBase",
    "[ui][hierarchy][base-class][6-4-1]")
{
    SECTION("CNewUIObj derives from INewUIBase (all window classes inherit this base)")
    {
        REQUIRE((std::is_base_of_v<SEASON3B::INewUIBase, SEASON3B::CNewUIObj>));
    }

    SECTION("CNewUIObj is abstract — concrete window classes must implement GetLayerDepth etc.")
    {
        REQUIRE(std::is_abstract_v<SEASON3B::CNewUIObj>);
    }
}

TEST_CASE("AC-1 [6-4-1]: CNewUIObj default visibility and enabled state after construction",
    "[ui][hierarchy][state][6-4-1]")
{
    SECTION("TestUIWindow (concrete CNewUIObj subclass) defaults to visible=true, enabled=true")
    {
        TestUIWindow wnd;
        REQUIRE(wnd.IsVisible() == true);
        REQUIRE(wnd.IsEnabled() == true);
    }

    SECTION("Show(false) hides the window; Show(true) restores visibility")
    {
        TestUIWindow wnd;
        wnd.Show(false);
        REQUIRE(wnd.IsVisible() == false);

        wnd.Show(true);
        REQUIRE(wnd.IsVisible() == true);
    }

    SECTION("Enable(false) disables the window; Enable(true) restores it")
    {
        TestUIWindow wnd;
        wnd.Enable(false);
        REQUIRE(wnd.IsEnabled() == false);

        wnd.Enable(true);
        REQUIRE(wnd.IsEnabled() == true);
    }

    SECTION("GetKeyEventOrder() returns default layer priority of 3.0f")
    {
        TestUIWindow wnd;
        REQUIRE(wnd.GetKeyEventOrder() == 3.0f);
    }
}

// =============================================================================
// AC-2 [6-4-1]: Window positioning, sizing, and layering constants validated
// Inventory grid cells (20x20 pixels), minimap skill icon geometry, and z-layering
// constants must all be within sensible game-rendering bounds.
// =============================================================================

TEST_CASE("AC-2 [6-4-1]: Inventory grid cell dimensions define valid square cells",
    "[ui][inventory][grid][constants][6-4-1]")
{
    using namespace SEASON3B;

    SECTION("INVENTORY_SQUARE_WIDTH is 20 pixels — standard item grid cell width")
    {
        REQUIRE(INVENTORY_SQUARE_WIDTH == 20);
    }

    SECTION("INVENTORY_SQUARE_HEIGHT is 20 pixels — standard item grid cell height")
    {
        REQUIRE(INVENTORY_SQUARE_HEIGHT == 20);
    }

    SECTION("Inventory grid cells are square (width equals height)")
    {
        REQUIRE(INVENTORY_SQUARE_WIDTH == INVENTORY_SQUARE_HEIGHT);
    }

    SECTION("Inventory grid cell dimensions are positive")
    {
        REQUIRE(INVENTORY_SQUARE_WIDTH > 0);
        REQUIRE(INVENTORY_SQUARE_HEIGHT > 0);
    }
}

TEST_CASE("AC-2 [6-4-1]: CNewUIMiniMap MASTER_DATA skill icon geometry constants are valid",
    "[ui][minimap][master-data][constants][6-4-1]")
{
    using namespace SEASON3B;

    SECTION("SKILL_ICON_DATA_WDITH is 4 — data columns per icon entry in the master skill table")
    {
        REQUIRE(CNewUIMiniMap::SKILL_ICON_DATA_WDITH == 4);
    }

    SECTION("SKILL_ICON_DATA_HEIGHT is 8 — data rows per icon entry in the master skill table")
    {
        REQUIRE(CNewUIMiniMap::SKILL_ICON_DATA_HEIGHT == 8);
    }

    SECTION("SKILL_ICON_WIDTH is 20 pixels — rendered skill icon width on the minimap")
    {
        REQUIRE(CNewUIMiniMap::SKILL_ICON_WIDTH == 20);
    }

    SECTION("SKILL_ICON_HEIGHT is 28 pixels — rendered skill icon height on the minimap")
    {
        REQUIRE(CNewUIMiniMap::SKILL_ICON_HEIGHT == 28);
    }

    SECTION("Skill icon start positions are positive screen coordinates")
    {
        REQUIRE(CNewUIMiniMap::SKILL_ICON_STARTX1 > 0);
        REQUIRE(CNewUIMiniMap::SKILL_ICON_STARTY1 > 0);
    }
}

TEST_CASE("AC-2 [6-4-1]: TOOLTIP_TYPE enum covers all inventory tooltip display contexts",
    "[ui][tooltip][enum][constants][6-4-1]")
{
    using namespace SEASON3B;

    SECTION("UNKNOWN_TOOLTIP_TYPE is 0 — sentinel for no active tooltip context")
    {
        REQUIRE(UNKNOWN_TOOLTIP_TYPE == 0);
    }

    SECTION("All 6 TOOLTIP_TYPE values (UNKNOWN through PURCHASE_SHOP) are pairwise distinct")
    {
        const int types[] = {
            UNKNOWN_TOOLTIP_TYPE,
            TOOLTIP_TYPE_INVENTORY,
            TOOLTIP_TYPE_REPAIR,
            TOOLTIP_TYPE_NPC_SHOP,
            TOOLTIP_TYPE_MY_SHOP,
            TOOLTIP_TYPE_PURCHASE_SHOP,
        };
        constexpr int n = static_cast<int>(sizeof(types) / sizeof(types[0]));
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                REQUIRE(types[i] != types[j]);
            }
        }
    }
}

// =============================================================================
// AC-3 [6-4-1]: Button, scroll bar, and list selection UI framework validated
// BUTTON_STATE defines the 3 visual states for CNewUIBaseButton.
// SQUARE_COLOR_STATE defines the 3 slot color states for CNewUIInventoryCtrl.
// RADIOGROUPEVENT_NONE is the sentinel for no active radio group event.
// =============================================================================

TEST_CASE("AC-3 [6-4-1]: BUTTON_STATE enum defines the 3 visual button interaction states",
    "[ui][button][enum][6-4-1]")
{
    using namespace SEASON3B;

    SECTION("BUTTON_STATE_UP is 0 — resting / default button state")
    {
        REQUIRE(BUTTON_STATE_UP == 0);
    }

    SECTION("BUTTON_STATE_DOWN is 1 — pressed / activated button state")
    {
        REQUIRE(BUTTON_STATE_DOWN == 1);
    }

    SECTION("BUTTON_STATE_OVER is 2 — mouse hover button state")
    {
        REQUIRE(BUTTON_STATE_OVER == 2);
    }

    SECTION("All 3 BUTTON_STATE values are pairwise distinct")
    {
        const int states[] = {BUTTON_STATE_UP, BUTTON_STATE_DOWN, BUTTON_STATE_OVER};
        constexpr int n = static_cast<int>(sizeof(states) / sizeof(states[0]));
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                REQUIRE(states[i] != states[j]);
            }
        }
    }
}

TEST_CASE("AC-3 [6-4-1]: RADIOGROUPEVENT_NONE sentinel is -1",
    "[ui][button][radio-group][6-4-1]")
{
    using namespace SEASON3B;

    SECTION("RADIOGROUPEVENT_NONE is -1 — no radio group event active")
    {
        REQUIRE(RADIOGROUPEVENT_NONE == -1);
    }
}

TEST_CASE("AC-3 [6-4-1]: SQUARE_COLOR_STATE enum covers normal and warning inventory slot states",
    "[ui][inventory][color-state][6-4-1]")
{
    using namespace SEASON3B;

    SECTION("UNKNOWN_COLOR_STATE is 0 — uninitialized slot color sentinel")
    {
        REQUIRE(UNKNOWN_COLOR_STATE == 0);
    }

    SECTION("COLOR_STATE_NORMAL is greater than UNKNOWN_COLOR_STATE — active normal coloring")
    {
        REQUIRE(COLOR_STATE_NORMAL > UNKNOWN_COLOR_STATE);
    }

    SECTION("COLOR_STATE_WARNING is greater than COLOR_STATE_NORMAL — escalated warning state")
    {
        REQUIRE(COLOR_STATE_WARNING > COLOR_STATE_NORMAL);
    }

    SECTION("All 3 SQUARE_COLOR_STATE values are pairwise distinct")
    {
        const int states[] = {UNKNOWN_COLOR_STATE, COLOR_STATE_NORMAL, COLOR_STATE_WARNING};
        constexpr int n = static_cast<int>(sizeof(states) / sizeof(states[0]));
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                REQUIRE(states[i] != states[j]);
            }
        }
    }
}

#endif // MU_GAME_AVAILABLE
