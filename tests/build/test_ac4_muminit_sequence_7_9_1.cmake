# Story 7.9.1: AC-4 — Port minimum game init sequence to MuMain()
# Flow Code: VS0-QUAL-RENDER-GAMELOOP
#
# RED PHASE:  Test FAILS before story 7.9.1 is implemented.
#             MuMain() in Winmain.cpp is a skeleton that initialises SDL3 windowing and
#             enters the event loop, but lacks the game state initialisation that WinMain()
#             runs before MainLoop(). Key missing items verified by this test:
#               - OpenBasicData (texture/item/gate/recipe loading)
#               - CharacterMachine allocation and Init()
#               - g_pUIManager construction
#               - i18n Translator loading
#               - g_platformAudio (MiniAudioBackend) init
#
# GREEN PHASE: Test PASSES after implementation.
#             MuMain() contains all of the above init calls after SDL3 window/renderer
#             creation and before the event loop.
#
# Validates:
#   AC-4 — OpenBasicData(nullptr) called in MuMain()
#   AC-4 — CharacterMachine allocation and Init() in MuMain()
#   AC-4 — g_pUIManager construction in MuMain()
#   AC-4 — i18n Translator loading in MuMain()
#   AC-4 — g_platformAudio MiniAudioBackend init in MuMain()

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED WINMAIN_CPP)
    message(FATAL_ERROR "WINMAIN_CPP must be set to the path of Winmain.cpp")
endif()

if(NOT EXISTS "${WINMAIN_CPP}")
    message(FATAL_ERROR "AC-4 FAIL: Winmain.cpp not found at '${WINMAIN_CPP}'")
endif()

file(READ "${WINMAIN_CPP}" winmain_content)

# ---------------------------------------------------------------------------
# AC-4: Verify that key game init symbols appear AFTER the MuMain() function
# definition. We split the file on 'int MuMain(' and check the suffix.
# ---------------------------------------------------------------------------
string(FIND "${winmain_content}" "int MuMain(" muminit_pos)
if(muminit_pos EQUAL -1)
    message(FATAL_ERROR "AC-4 FAIL: 'int MuMain(' not found in Winmain.cpp — cannot verify init sequence.")
endif()

string(LENGTH "${winmain_content}" total_len)
math(EXPR suffix_len "${total_len} - ${muminit_pos}")
string(SUBSTRING "${winmain_content}" ${muminit_pos} ${suffix_len} muminit_section)

# Required init symbols that must appear in MuMain() body
set(REQUIRED_INIT_SYMBOLS
    "OpenBasicData"
    "CharacterMachine"
    "CharacterMachine->Init"
    "g_pUIManager"
    "i18n"
    "g_platformAudio"
)

set(found_violations FALSE)

foreach(symbol IN LISTS REQUIRED_INIT_SYMBOLS)
    string(FIND "${muminit_section}" "${symbol}" sym_pos)
    if(sym_pos EQUAL -1)
        message(WARNING "AC-4 FAIL: '${symbol}' not found in MuMain() body.\n"
            "Port WinMain() init sequence for '${symbol}' to MuMain() (Story 7.9.1 Task 4).")
        set(found_violations TRUE)
    endif()
endforeach()

if(found_violations)
    message(FATAL_ERROR
        "AC-4 FAIL: Game init sequence incomplete in MuMain().\n"
        "Story 7.9.1 Task 4: port srand/RandomTable, game data arrays, CharacterMachine,\n"
        "g_pUIManager, g_pUIMapName, g_BuffSystem, g_MapProcess, g_petProcess,\n"
        "CUIMng, g_pNewUISystem, i18n Translator, g_platformAudio, and OpenBasicData(nullptr)\n"
        "from WinMain() to MuMain(), placing them after SDL3 window/renderer init,\n"
        "before the event loop. Skip Win32-only init (SetTimer, CreateFont, CInput with HWND, IME).")
endif()

message(STATUS "AC-4 PASS: Game init sequence present in MuMain() — all required symbols found")
