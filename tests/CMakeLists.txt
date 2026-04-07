include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
)
FetchContent_MakeAvailable(Catch2)

add_executable(MuTests
    core/test_timer.cpp
    platform/test_platform_types.cpp
    platform/test_platform_compat.cpp
    platform/test_platform_keys.cpp
    platform/test_platform_library.cpp
    platform/test_platform_window.cpp
)

# Story 2.2.1: SDL3 Keyboard Input Migration [VS1-SDL-INPUT-KEYBOARD]
# test_platform_input.cpp is guarded by #ifdef MU_ENABLE_SDL3 internally,
# but we add it to MuTests unconditionally so the translation unit is always
# compiled. The #ifdef inside the file ensures SDL3-specific code is excluded
# when MU_ENABLE_SDL3 is OFF (CI / MinGW Strategy B).
target_sources(MuTests PRIVATE platform/test_platform_input.cpp)

# Story 2.2.2: SDL3 Mouse Input Migration [VS1-SDL-INPUT-MOUSE]
# test_platform_mouse.cpp is guarded by #ifdef MU_ENABLE_SDL3 internally.
# Added unconditionally so the TU is always compiled; code is excluded when
# MU_ENABLE_SDL3 is OFF (CI / MinGW Strategy B).
target_sources(MuTests PRIVATE platform/test_platform_mouse.cpp)

# Story 2.2.3: SDL3 Text Input Migration [VS1-SDL-INPUT-TEXT]
# test_platform_text_input.cpp is guarded by #ifdef MU_ENABLE_SDL3 internally.
# Added unconditionally so the TU is always compiled; code is excluded when
# MU_ENABLE_SDL3 is OFF (CI / MinGW Strategy B).
target_sources(MuTests PRIVATE platform/test_platform_text_input.cpp)

# Story 7.1.1: Cross-Platform Error Reporting [VS0-QUAL-ERRORREPORT-XPLAT]
# RED PHASE: test_error_report.cpp exercises post-refactor API (std::ofstream/std::filesystem).
# Tests fail until ErrorReport.cpp is refactored in story 7.1.1.
target_sources(MuTests PRIVATE core/test_error_report.cpp)

# Story 7.2.1: Frame Time Instrumentation [VS0-QUAL-FRAMETIMER]
# GREEN PHASE: mu::MuTimer is implemented in MuTimer.h and MuTimer.cpp. All 6 tests pass.
target_sources(MuTests PRIVATE core/test_mu_timer.cpp)

# Story 4.1.1: Ground Truth Capture Mechanism [VS1-RENDER-GROUNDTRUTH-CAPTURE]
# GREEN PHASE: GroundTruthCapture.h/cpp implemented. All SSIM tests pass.
# ComputeSSIM() is always compiled (no ENABLE_GROUND_TRUTH_CAPTURE guard).
# glReadPixels-dependent code is guarded by #ifdef ENABLE_GROUND_TRUTH_CAPTURE in
# GroundTruthCapture.cpp; the glReadPixels stub in stdafx.h ensures compilation on all platforms.
target_sources(MuTests PRIVATE core/test_ground_truth.cpp)

# Story 3.1.2: Connection.h Cross-Platform Updates [VS1-NET-CONNECTION-XPLAT]
# Tests mu::platform::Load/GetSymbol graceful failure paths used by Connection.h.
# No actual .NET library required at test time.
target_sources(MuTests PRIVATE platform/test_connection_library_load.cpp)

# Story 3.4.1: Connection Error Messaging & Graceful Degradation [VS1-NET-ERROR-MESSAGING]
# RED PHASE: Tests FormatLibraryNotFoundMessage() and FormatSymbolNotFoundMessage() helpers
# extracted from Connection.cpp. Tests compile but FAIL until Connection.cpp exposes these
# free functions in the DotNetBridge namespace. No .NET library required at test time.
target_sources(MuTests PRIVATE network/test_connection_error_messages.cpp)

# Story 3.4.2: Server Connection Configuration [VS1-NET-CONFIG-SERVER]
# RED PHASE: Tests ValidateServerPort() and ValidateServerIP() helpers extracted into
# Core/GameConfigValidation.h/.cpp. Tests compile but FAIL until GameConfigValidation.cpp
# is present with the correct implementations. No config.ini required at test time.
target_sources(MuTests PRIVATE network/test_server_config_validation.cpp)

# Story 7.1.2: POSIX Signal Handlers for Crash Diagnostics [VS0-QUAL-SIGNAL-HANDLERS]
# RED PHASE: test_posix_signal_handlers.cpp verifies InstallSignalHandlers() installs
# SA_SIGACTION handlers for SIGSEGV/SIGABRT/SIGBUS. Guarded by #ifndef _WIN32 internally
# so the TU compiles on Windows (MinGW CI) — the test body is a no-op stub on Windows.
target_sources(MuTests PRIVATE platform/test_posix_signal_handlers.cpp)

# Story 4.2.1: MuRenderer Core API with OpenGL Backend [VS1-RENDER-ABSTRACT-CORE]
# GREEN PHASE: MuRenderer.h, MuRenderer.cpp, MatrixStack.h, MatrixStack.cpp implemented.
# 8 TEST_CASEs covering MatrixStack push/pop, nested translations, IsEmpty(), BlendMode
# state tracking round-trip, IMuRenderer interface completeness, mu:: namespace, and
# IMuRenderer no-GL-types compile check. All 8 pass.
target_sources(MuTests PRIVATE render/test_murenderer.cpp)

# Story 4.2.2: Migrate RenderBitmap Variants to RenderQuad2D [VS1-RENDER-MIGRATE-QUAD2D]
# RED PHASE: Tests document the RenderQuad2D call-through contract and vertex layout
# expected after migration of the 9 RenderBitmap* variants + RenderColor.
# Tests compile and run on macOS/Linux — no gl* calls in the test TU.
# Tests become GREEN once Tasks 2-11 in the story are implemented.
target_sources(MuTests PRIVATE render/test_renderbitmap_migration.cpp)

# Story 4.2.3: Migrate Skeletal Mesh Rendering to RenderTriangles [VS1-RENDER-MIGRATE-TRIANGLES]
# GREEN PHASE: Tests verify Vertex3D struct layout contract, RenderTriangles call-through,
# ABGR color packing conventions, vertex count = NumTriangles * 3, and shadow path
# textureId=0 + zero UV/normal fields. All tests are pure logic; no gl* calls in this TU.
# Tests compile and run on macOS/Linux without Win32 or OpenGL.
target_sources(MuTests PRIVATE render/test_skeletalmesh_migration.cpp)

# Story 4.2.4: Migrate Trail Effects to RenderQuadStrip [VS1-RENDER-MIGRATE-QUADSTRIP]
# GREEN PHASE: All 7 TEST_CASEs pass. Tests verify RenderQuadStrip call-through contract,
# UV mapping from Light1/Light2, Luminosity ABGR packing, and per-branch call patterns for
# all 4 migrated glBegin(GL_QUADS) trail segment blocks in RenderJoints():
#   BITMAP_JOINT_FORCE SubType==0, GUILD_WAR_EVENT BITMAP_FLARE SubType==22 double-face,
#   RENDER_FACE_ONE, RENDER_FACE_TWO. Tests compile and run on macOS/Linux — no gl* calls.
# Tasks 1–7 in story 4.2.4 fully implemented and quality gate PASSED (705 files, 0 errors).
target_sources(MuTests PRIVATE render/test_traileffects_migration.cpp)

# Story 4.3.1: SDL_gpu Backend Implementation [VS1-RENDER-SDLGPU-BACKEND]
# RED PHASE: Tests verify TextureRegistry map contract (register/lookup/unregister),
# BlendMode -> SDL_GPUBlendFactor mapping table (all 8 modes), and FogParams
# storage round-trip via SetFog(). Tests compile but FAIL until:
#   Task 3.1/3.2: GetBlendFactors(BlendMode) free function implemented in MuRendererSDLGpu.cpp
#   Task 5.1/5.2: RegisterTexture / UnregisterTexture / LookupTexture / ClearTextureRegistry
#     implemented in MuRendererSDLGpu.cpp and exposed for test linkage
# No actual SDL_GPUDevice required; FogCaptureMock bypasses device init.
# Tests compile and run on macOS/Linux without Win32, OpenGL, or a real GPU device.
target_sources(MuTests PRIVATE render/test_sdlgpubackend.cpp)

# Story 4.3.2: Shader Programs (HLSL + SDL_shadercross) [VS1-RENDER-SHADERS]
# RED PHASE: Tests verify GetShaderBlobPath driver-to-extension mapping (AC-6),
# FogUniform struct layout static_assert vs HLSL cbuffer std140 alignment (AC-10),
# and pipeline set selection for 2D vs 3D draw modes (AC-8).
# Tests compile and run on macOS/Linux — no GPU device, no Win32, no OpenGL required.
# Tests fail until GetShaderBlobPath, GetPipelineSetFor, and FogUniform are implemented
# in MuRendererSDLGpu.cpp and exposed in the mu:: namespace.
target_sources(MuTests PRIVATE render/test_shaderprograms.cpp)

# Story 4.4.1 — Texture system migration [VS1-RENDER-TEXTURE-MIGRATE]
# RED PHASE: Tests verify TextureRegistry roundtrip (GREEN — 4.3.1 impl), GL->SDL
# filter/wrap mapping (MapGLFilterToSDL/MapGLWrapToSDL), and RGB->RGBA8 pixel padding
# (PadRGBToRGBA). Format mapping and padding tests fail until Task 2.1 in story 4.4.1
# implements the anonymous-namespace helpers in GlobalBitmap.cpp.
# Tests compile and run on macOS/Linux — no GPU device, no Win32, no OpenGL required.
target_sources(MuTests PRIVATE render/test_texturesystemmigration.cpp)

# Story 4.2.5: Migrate Blend & Pipeline State to MuRenderer [VS1-RENDER-MIGRATE-STATE]
# RED PHASE: Tests verify SetBlendMode() routing for all 7 blend helper mappings,
# SetDepthTest() bool routing (enable/disable), SetFog() FogParams population for the
# GMBattleCastle GL_LINEAR fog migration, and DisableBlend() routing (new IMuRenderer method).
# Tests compile but FAIL until Tasks 2–5 in story 4.2.5 are implemented:
#   Task 2: add BlendMode::Glow, BlendMode::Luminance, DisableBlend() to MuRenderer.h/.cpp
#   Task 3: wrap 7 blend helpers in ZzzOpenglUtil.cpp to delegate to mu::GetRenderer()
#   Task 4: wrap depth test helpers; fix CameraMove.cpp direct GL calls
#   Task 5: migrate GMBattleCastle.cpp fog setup to mu::GetRenderer().SetFog()
# Tests compile and run on macOS/Linux — no gl* calls in this TU.
target_sources(MuTests PRIVATE render/test_blendpipelinestate_migration.cpp)

# Story 7.9.2: OpenGL Immediate-Mode → MuRenderer Abstraction Migration [VS0-QUAL-RENDER-GLMIGRATE]
# GREEN PHASE: Tests verify new IMuRenderer methods (BeginScene/EndScene, Begin2DPass/End2DPass,
# RenderLines, ClearScreen, IsFrameActive), CSprite coordinate conversion formula
# (640×480 → screen pixels with Y-axis inversion), and ABGR color packing.
# All tests use MigrationCaptureMock — no gl* calls in this TU.
# Tests compile and run on macOS/Linux without Win32, OpenGL, or a GPU device.
target_sources(MuTests PRIVATE render/test_gl_migration_7_9_2.cpp)

# Story 7.9.6: Migrate All Raw OpenGL Calls to MuRenderer [VS0-RENDER-GL-MIGRATE]
# RED PHASE: Tests verify ALL new IMuRenderer methods required to eliminate 250+ raw
# OpenGL calls (state management, clear color, matrix stack, depth/stencil, viewport,
# scissor, screenshot) from 35 files. Interface tests use GLMigration796Mock — no gl*
# calls in this TU. File-scan tests (AC-1, AC-2, AC-9) are guarded by #ifndef _WIN32
# so MinGW CI compiles cleanly. MU_SOURCE_DIR already injected by Story 7.9.3 entry.
# Tests compile on macOS/Linux without Win32, OpenGL, or a GPU device.
# Tests FAIL TO COMPILE until IMuRenderer gains: SetClearColor, PushMatrix, PopMatrix,
# LoadIdentity, Translate, Rotate, Scale, MultMatrix, LoadMatrix, GetMatrix,
# SetMatrixMode, SetDepthFunc, SetAlphaFunc, SetStencilFunc, SetStencilOp,
# SetColorMask, SetViewport, SetScissor, SetScissorEnabled, ReadPixels.
target_sources(MuTests PRIVATE render/test_gl_migration_7_9_6.cpp)

# Story 7.9.7: Adopt GLM and Harden Renderer Matrix Pipeline [VS0-RENDER-GLM-MATRIX]
# RED PHASE: Tests verify GLM matrix math correctness (AC-STD-2, Task 9).
#   Task 9.2: Perspective Z range [0,1] — glm::perspective with GLM_FORCE_DEPTH_ZERO_TO_ONE
#             maps eye-space near to NDC z=0 and far to NDC z=1 (Metal/Vulkan convention).
#   Task 9.3: Ortho projection — glm::ortho maps 0..W/0..H corners to NDC [-1,1].
#   Task 9.4: Matrix stack push/pop — PushMatrix/PopMatrix call counts via MatrixMath797Mock.
# All tests use GLM directly (no gl* calls, no GPU device, no Win32).
# Matrix stack interface tests use MatrixMath797Mock implementing IMuRenderer.
# Tests compile and run on macOS/Linux without Win32, OpenGL, or a GPU device.
target_sources(MuTests PRIVATE render/test_matrix_math_7_9_7.cpp)
# GLM is PRIVATE in MURenderFX — must also link to MuTests for test_matrix_math_7_9_7.
target_link_libraries(MuTests PRIVATE glm::glm)
target_compile_definitions(MuTests PRIVATE GLM_FORCE_DEPTH_ZERO_TO_ONE)

# Story 7.9.8: Adopt SDL_ttf for Cross-Platform Font Rendering [VS1-RENDER-FONT-SDLTTF]
# RED PHASE: Tests verify:
#   AC-3: PackColorDWORD — ABGR DWORD color packing for CUIRenderTextSDLTtf (6 test cases)
#   AC-2, AC-4, AC-5, AC-6, AC-STD-NFR-1: GPU-dependent or Win32-dependent — SKIP markers
# AC-1 (SDL_ttf FetchContent) verified by CMake script test: 7.9.8-AC-1:sdl-ttf-fetchcontent
# Tests compile and run on macOS/Linux — no Win32, no OpenGL, no SDL3 GPU device required.
# PackColorDWORD tests FAIL TO LINK until mu::sdlttf::PackColorDWORD is implemented
# in UIControls.cpp as part of CUIRenderTextSDLTtf.
target_sources(MuTests PRIVATE render/test_sdl_ttf_7_9_8.cpp)

# Story 5.1.1: MuAudio Abstraction Layer [VS1-AUDIO-ABSTRACT-CORE]
# GREEN PHASE: Tests verify IPlatformAudio interface purity (std::is_abstract_v),
# g_platformAudio starts as nullptr, MiniAudioBackend default-constructs cleanly,
# Initialize() fails gracefully without an audio device (CI headless), Shutdown() is
# safe to call without prior Initialize(), and mu:: namespace is accessible.
# Tests compile and run on macOS/Linux — no Win32, no DirectSound, no audio device required.
target_sources(MuTests PRIVATE audio/test_muaudio_abstraction.cpp)

# Story 5.2.1: miniaudio BGM Implementation [VS1-AUDIO-MINIAUDIO-BGM]
# GREEN PHASE: Tests verify BGM lifecycle — IsEndMusic() before play, StopMusic on unloaded
# stream, PlayMusic with non-existent file (no crash), GetMusicPosition before play returns 0.
# All tests run headless (no audio device needed — Initialize() may return false on CI).
# Implementation complete: g_platformAudio wired in MuMain.cpp; BGM delegation done.
target_sources(MuTests PRIVATE audio/test_miniaudio_bgm.cpp)

# Story 5.4.1: Volume Controls & Audio State Management [VS1-AUDIO-VOLUME-CONTROLS]
# RED PHASE: Tests verify SetBGMVolume/SetSFXVolume/GetBGMVolume/GetSFXVolume on
# MiniAudioBackend — default values (1.0f), clamping, get/set round-trip, and
# safety on an uninitialized backend. No audio device or Win32 required.
# Tests fail until Tasks 1-2 in story 5.4.1 add m_bgmVolume/m_sfxVolume and the
# four new methods to IPlatformAudio and MiniAudioBackend.
target_sources(MuTests PRIVATE audio/test_volume_controls.cpp)

# Story 5.2.2: miniaudio SFX Implementation [VS1-AUDIO-MINIAUDIO-SFX]
# RED PHASE: Tests verify SFX lifecycle safety — LoadSound non-existent file graceful,
# PlaySound before Initialize returns S_FALSE, StopSound on unloaded slot is safe,
# AllStopSound on empty backend is safe, Set3DSoundPosition with no loaded sounds is safe,
# Set3DSoundPosition skips nullptr m_soundObjects, volume controls before Initialize are safe.
# All tests run headless (no audio device required — Initialize() may return false on CI).
# Tests become GREEN once Tasks 1-5 in story 5.2.2 are implemented:
#   Task 1: Add m_soundObjects array to MiniAudioBackend
#   Task 2: Upgrade Set3DSoundPosition() stub to full per-frame position update
#   Task 3: Redirect SFX free functions in DSplaysound.cpp to g_platformAudio
#   Task 4: Remove InitDirectSound/FreeDirectSound from MuMain.cpp
#   Task 5: Add backslash→forward-slash path normalization in LoadSound()
target_sources(MuTests PRIVATE audio/test_miniaudio_sfx.cpp)

# Story 6.1.1: Auth & Character Management Validation [VS1-GAME-VALIDATE-AUTH]
# RED PHASE: Tests verify CLASS_TYPE enum has 5 base classes, MAX_CHARACTERS_PER_ACCOUNT == 5,
# and CharacterSelectionState / SceneInitializationState contracts.
# AC-3/AC-4: CLASS_TYPE enum + MAX_CHARACTERS_PER_ACCOUNT tests are always compiled and run.
# AC-1/AC-5/AC-6: Scene state tests gated by MU_SCENE_TESTS_ENABLED (requires SceneCommon.h headers).
# Story 6-1-1 code review fix: SceneCommon.h now uses mu_define.h instead of ZzzInfomation.h,
# enabling header-only compilation without full MUGame linkage. All tests now execute.
target_sources(MuTests PRIVATE scenes/test_auth_character_validation.cpp)
target_compile_definitions(MuTests PRIVATE MU_SCENE_TESTS_ENABLED)
target_include_directories(MuTests PRIVATE "${CMAKE_SOURCE_DIR}/src/source/Scenes")

# Story 6.1.2: World Navigation Validation [VS1-GAME-VALIDATE-NAVIGATION]
# RED PHASE: Tests verify ENUM_WORLD map IDs for all 6 key maps (AC-5), PATH grid
# geometry (CheckXYPos/GetIndex/EstimateCostToGoal — AC-1), MovePoint() direction
# mapping (AC-1), MOVEINFODATA gate index contract (AC-2), and TW_* terrain flags (AC-2).
# AC-1/AC-2 partial: FindPath A*, CMapManager range queries, CPortalMgr state are gated
# by MU_WORLD_NAVIGATION_TESTS_ENABLED (requires MUGame linkage — not set by default).
# MUCommon INTERFACE provides src/source/{Core,World,Data}/ include paths — no extra
# target_include_directories needed.
target_sources(MuTests PRIVATE world/test_world_navigation_validation.cpp)

# Story 6.2.1: Combat System Validation [VS1-GAME-VALIDATE-COMBAT]
# RED PHASE: Tests verify ActionSkillType melee skill enum values (AC-1), SKILL_ATTRIBUTE
# struct constants (MAX_CLASS, MAX_DUTY_CLASS, MAX_SKILL_NAME) and DemendConditionInfo
# requirement operator (AC-2), MonsterSkillType base enum values (AC-3),
# AT_SKILL_UNDEFINED sentinel and AT_SKILL_MASTER_END range (AC-4), MAX_SKILLS
# capacity and SKILL_ATTRIBUTE Mana/Damage fields (AC-5), and ESound combat enum values
# SOUND_BRANDISH_SWORD01..04, SOUND_ATTACK_MELEE_HIT1..5, SOUND_MONSTER range (AC-6).
# AC-1/AC-3 script struct, CSkillManager runtime, ZzzCharacter state machine: gated by
# MU_COMBAT_TESTS_ENABLED (requires MUGame linkage — not set by default).
# MUCommon INTERFACE provides src/source/{Audio,Gameplay/Skills,Data/Skills,Core}/ include
# paths — no extra target_include_directories needed for this test file.
target_sources(MuTests PRIVATE gameplay/test_combat_system_validation.cpp)

# Story 6.2.2: Inventory & Trading Validation [VS1-GAME-VALIDATE-INVENTORY]
# RED PHASE: Tests verify inventory slot/bag constants, item struct layouts, STORAGE_TYPE
# pairwise distinctness, and trade system data structures. No server or Win32 required
# for component-level struct/enum tests; full AC validation gated by MU_GAME_AVAILABLE.
target_sources(MuTests PRIVATE gameplay/test_inventory_trading_validation.cpp)

# Story 6.3.1: Social Systems Validation [VS1-GAME-VALIDATE-SOCIAL]
# RED PHASE: Tests verify chat MESSAGE_TYPE enum (10 values + TYPE_UNKNOWN), INPUT_MESSAGE_TYPE
# channel count, MAX_CHAT_SIZE=90, PARTY_t struct layout, GuildConstants namespace
# (GUILD_NAME_LENGTH=8, GUILD_MARK_SIZE=64, Capacity::MAX_CAPACITY=80), GuildTab/GuildInfoButton/
# RelationshipType enums, GUILD_LIST_t/MARK_t struct layouts, GuildConstants::Colors ARGB values.
# MU_GAME_AVAILABLE gates UI and packet struct tests (NewUIChatLogWindow, WSclient).
target_sources(MuTests PRIVATE gameplay/test_social_systems_validation.cpp)

# Story 6.3.2: Advanced Game Systems Validation [VS1-GAME-VALIDATE-SYSTEMS]
# RED PHASE: Tests verify quest constants (MAX_QUESTS=200, CONDITION/REQUEST=16),
# TYPE_QUEST/QUEST_VIEW enum completeness, QUEST_CLASS_ACT/REQUEST/ATTRIBUTE struct layouts,
# pet state/PET_TYPE/PET_COMMAND enum integrity, DUEL_PLAYER_INFO/DUEL_CHANNEL_INFO struct
# layouts, MAX_DUEL_CHANNELS=4, _DUEL_PLAYER_TYPE enum values.
# MU_GAME_AVAILABLE gates CSQuest state packing constants, PET_INFO struct, PetObject::ActionType,
# pet rendering constants (PC4_ELF..SKELETON), and CSBaseMatch inheritance static_assert checks.
target_sources(MuTests PRIVATE gameplay/test_advanced_systems_validation.cpp)

# Story 7.6.3: Data Layer Win32 Removal [VS0-QUAL-WIN32CLEAN-DATALAYER]
# RED PHASE: Tests verify mu_encrypt_blob / mu_decrypt_blob round-trip (AC-3, AC-STD-2).
# Includes round-trip for ASCII, binary, and wstring payloads; tamper detection (GCM auth tag);
# and random-IV guarantee. Tests compile but FAIL until:
#   Task 2.1: PlatformCompat.h gains mu_encrypt_blob / mu_decrypt_blob (AES-256-GCM or no-op)
#   Task 3.2: CMake links OpenSSL::Crypto to MUCore when MU_HAS_OPENSSL=1
# No Win32 APIs or DPAPI — runs on macOS/Linux/Windows (MinGW CI).
target_sources(MuTests PRIVATE data/test_gameconfig_crypto.cpp)

# Story 7.6.6: ShopListManager Cross-Platform HTTP Downloader [VS0-QUAL-WIN32CLEAN-SHOPHTTP]
# RED PHASE: Tests compile on all platforms but use Catch2 SKIP until:
#   Task 3: TCHAR/INTERNET_PORT/HANDLE replaced with portable types in DownloadInfo.h + headers
#   Task 4: #ifdef _WIN32 guards removed from all 17 ShopListManager .cpp files
#   Task 1: libcurl linked via find_package(CURL REQUIRED) in CMakeLists.txt
# To activate GREEN phase: add target_compile_definitions(MuTests PRIVATE MU_SHOPLIST_CROSS_PLATFORM_READY)
# after Tasks 3+4 complete. No Win32 APIs in the test TU — compiles on macOS/Linux/Windows (MinGW CI).
target_sources(MuTests PRIVATE gameshop/test_shoplist_download.cpp)
target_compile_definitions(MuTests PRIVATE MU_SHOPLIST_CROSS_PLATFORM_READY)

# Story 7.6.5: Cross-Platform Terminal / Console [VS0-QUAL-WIN32CLEAN-CONSOLE]
# GREEN PHASE: mu_console_init() and mu_get_console_size() implemented in PlatformCompat.h.
# test_console.cpp verifies mu_console_init() does not crash (AC-STD-2, AC-3)
# and mu_get_console_size() returns positive dimensions (AC-STD-2, AC-6).
# No Win32 APIs in the test TU — runs on macOS/Linux/Windows (MinGW CI).
target_sources(MuTests PRIVATE core/test_console.cpp)

# Story 7.6.4: Cross-Platform CPU Usage Monitoring [VS0-QUAL-WIN32CLEAN-CPUUSAGE]
# RED PHASE: Tests verify hardware_concurrency returns positive value (AC-3, AC-STD-2),
# CPU usage GetUsage() returns a value in [0.0, 1.0] (AC-4, AC-STD-2), and multiple
# rapid calls never crash and always return non-negative values (AC-5).
# Tests compile and run on macOS/Linux — no Win32 APIs required in test TU.
# Range test [0.0, 1.0] is RED on Windows until Task 3 normalises GetUsage() from
# percentage (0–100+) to fractional ratio (0.0–1.0). Tests become GREEN once:
#   Task 2.1: PlatformCompat.h gains mu_get_process_cpu_times() abstraction
#   Task 3.1–3.4: CpuUsage.cpp fully migrated to std::thread::hardware_concurrency()
#                 and mu_get_process_cpu_times() with all #ifdef _WIN32 blocks removed.
target_sources(MuTests PRIVATE core/test_cpu_usage.cpp)

# Story 7.6.2: Win32 String Conversion and Include Guard Cleanup [VS0-QUAL-WIN32CLEAN-STRINCLUDE]
# RED PHASE: Tests verify that mu_wchar_to_utf8() correctly converts wide strings to UTF-8
# (replacing WideCharToMultiByte) and that mu_swprintf() formats correctly (replacing
# StringCbPrintf/StringCchPrintf). Tests compile and run on macOS/Linux using only
# PlatformCompat.h shims — no Win32 or game loop required. Tests become the authoritative
# regression guard after Win32 guards are removed in Tasks 1-8.
# AC-2 (muConsoleDebug), AC-3 (StringUtils), AC-4 (GlobalBitmap), AC-5 (MsgBoxIGSBuyConfirm).
target_sources(MuTests PRIVATE platform/test_win32_string_cleanup_7_6_2.cpp)

# Story 7.9.3: Unify Entry Point [VS0-QUAL-RENDER-UNIFYENTRY]
# RED PHASE: Tests verify that WinMain() is deleted (AC-2), MuMain() is the universal
# entry point (AC-3), and zero #ifdef _WIN32 guards remain in game code outside Platform/
# and Audio/ directories (AC-5).
# AC-1 tests (screen rate calculation) are pure math — always GREEN, serve as regression guards.
# File-scan tests (AC-2, AC-3, AC-5) are guarded by #ifndef _WIN32 internally so MinGW CI
# compiles cleanly. MU_SOURCE_DIR is injected so the scanner can locate source files.
# No Win32 APIs in the test TU — compiles and runs on macOS/Linux/Windows (MinGW CI).
target_sources(MuTests PRIVATE platform/test_entry_point_unification_7_9_3.cpp)
target_compile_definitions(MuTests PRIVATE
    MU_SOURCE_DIR="${CMAKE_SOURCE_DIR}/src/source"
)

# Story 7.9.4: Kill DirectSound — Miniaudio-Only Audio Layer [VS0-QUAL-AUDIO-KILLDSOUND]
# RED PHASE: Tests verify all DirectSound code is removed from Audio/ (AC-1, AC-2, AC-3)
# and all audio paths route through IPlatformAudio/MiniAudioBackend (AC-4).
# AC-1a tests (DbToLinear conversion math) are pure arithmetic — always GREEN, serve as
# regression guards for the dB→linear volume scale formula.
# File-scan tests (AC-1 DS types, AC-2 wave I/O types, AC-3 guard count) are guarded by
# #ifndef _WIN32 internally so MinGW CI compiles cleanly. MU_SOURCE_DIR already injected.
# AC-4 tests (interface conformance, default construction) are compile-time checks — always GREEN.
# No Win32 or DirectSound APIs in the test TU — compiles on macOS/Linux/Windows (MinGW CI).
target_sources(MuTests PRIVATE audio/test_directsound_removal_7_9_4.cpp)

# Story 7.9.5: Eliminate All Cross-Platform Stubs [VS0-PLAT-COMPAT-KILLSTUBS]
# RED PHASE: Tests verify GDI text rendering pipeline (AC-1, AC-2, AC-3, AC-4),
# clipboard operations via SDL3 (AC-5), and dead stub removal from PlatformCompat.h (AC-7).
# AC-6 (WebzenScene visual) uses Catch2 SKIP — requires manual game binary verification.
# GDI/clipboard tests FAIL until CrossPlatformGDI.cpp provides real implementations:
#   AC-1: CreateCompatibleDC/CreateDIBSection return non-null; SelectObject/DeleteDC safe
#   AC-2: CreateFont() returns a real non-null HFONT descriptor
#   AC-3: GetTextExtentPoint32() returns non-zero SIZE, multi-char > single-char width
#   AC-4: TextOut() returns TRUE after rasterizing glyphs into DC+bitmap pipeline
#   AC-5: OpenClipboard returns TRUE (SDL3 path); GlobalLock returns real data pointer
#   AC-7: wglCreateContext, gluPerspective, RegOpenKeyEx, CreateFile stubs absent (file-scan)
# File-scan tests (AC-7) are guarded by #ifndef _WIN32 so MinGW CI compiles cleanly.
# MU_SOURCE_DIR already injected by Story 7.9.3 CMake entry above.
# No Win32 APIs in test TU — compiles on macOS/Linux/Windows (MinGW CI).
target_sources(MuTests PRIVATE platform/test_platformcompat_no_stubs_7_9_5.cpp)

# Story 6.4.1: UI Windows Comprehensive Validation [VS1-GAME-VALIDATE-UI]
# RED PHASE: Tests verify INTERFACE_LIST enum completeness and coverage (AC-4),
# CNewUIObj class hierarchy and Show/Enable state transitions (AC-1, MU_GAME_AVAILABLE),
# inventory grid and minimap MASTER_DATA dimension constants (AC-2, MU_GAME_AVAILABLE),
# BUTTON_STATE/RADIOGROUPEVENT_NONE/SQUARE_COLOR_STATE enums (AC-3, MU_GAME_AVAILABLE),
# and SSIM infrastructure validity for 190x429 inventory and 320x240 minimap buffers (AC-5).
# AC-1/AC-2/AC-3 class-level tests are gated by MU_GAME_AVAILABLE (require Win32 + MUGame).
# AC-4 (INTERFACE_LIST from mu_enum.h) and AC-5 (GroundTruthCapture.h ComputeSSIM) are
# always compiled and run — no Win32 or OpenGL required.
target_sources(MuTests PRIVATE gameplay/test_ui_windows_validation.cpp)

# Story 7.8.2: Gameplay Header Cross-Platform Fixes [VS0-QUAL-BUILD-HEADERS]
# RED PHASE: test_gameplay_header_crossplatform_7_8_2.cpp includes mu_enum.h from a
# third translation unit (alongside test_combat_system_validation.cpp and
# test_ui_windows_validation.cpp, which also include mu_enum.h). On macOS/Linux,
# building MuTests fails at link time with a duplicate-symbol error for SKILL_REPLACEMENTS
# until 'inline' is added to its declaration in mu_enum.h (AC-1). The compilation
# success of this TU IS the RED phase failure; linker error resolves in GREEN phase.
# AC-2/AC-3/AC-4 (ZzzPath.h, SkillStructs.h, CSItemOption.h) are verified by CMake
# script tests in tests/build/ — those headers have Win32 deps, not included here.
target_sources(MuTests PRIVATE gameplay/test_gameplay_header_crossplatform_7_8_2.cpp)

# Story 7.8.1: Audio Interface Win32 Type Cleanup [VS0-QUAL-BUILD-AUDIO]
# RED PHASE: AC-2 primary test: PlaySound return type must be bool (sizeof == 1), not
# HRESULT (= long, sizeof == 4 or 8). sizeof(result) != sizeof(bool) fails in RED phase.
# AC-4 call-site tests: nullptr/false args compile with both old (OBJECT*, BOOL) and
# new (void*, bool) signatures — these are compatibility guards, not type-assertion tests.
# AC-STD-2: Headers compile on this platform — test file inclusion IS the compilation check.
# Tests become GREEN once Tasks 2-5 in story 7.8.1 replace Win32 types with portable types.
# No audio device required. Compiles and runs on macOS/Linux/Windows (MinGW CI).
target_sources(MuTests PRIVATE audio/test_audio_interface_portable_types_7_8_1.cpp)

# Story 5.3.1: Audio Format Validation [VS1-AUDIO-FORMAT-VALIDATE]
# RED PHASE: Tests verify miniaudio correctly decodes WAV, MP3, and OGG Vorbis audio
# formats used by the game, and that error paths are handled gracefully.
# AC-1: MiniAudioBackend::LoadSound() works for WAV (mono + stereo), MP3, and OGG
# AC-2: Test assets generated at runtime (WAV in-code, MP3/OGG as embedded hex arrays)
# AC-3: Non-existent and corrupt files handled gracefully (no crash)
# AC-4: PlayMusic() streaming path (MA_SOUND_FLAG_STREAM) works for MP3 and OGG Vorbis
# AC-5: ma_decoder direct pipeline validation (frame count, sample format, channel count)
# AC-6: All tests run headless on macOS/Linux/Windows CI without audio device
# AC-7: Tests use standalone MiniAudioBackend instances — do NOT touch g_platformAudio
# Tests fail until miniaudio correctly decodes all three formats in the build under test.
target_sources(MuTests PRIVATE audio/test_audio_format_validation.cpp)

# Story 5.1.1: MuAudio target provides miniaudio symbols needed by test_muaudio_abstraction.cpp
# (MiniAudioBackend, g_platformAudio, ma_engine/ma_sound types from miniaudio.h).
# MuTests must link MUAudio to resolve these symbols.
target_link_libraries(MuTests PRIVATE MUAudio)
# Include paths for audio test: miniaudio.h (included by MiniAudioBackend.h)
# and MiniAudio/ backend directory (not propagated via MUAudio PRIVATE includes).
target_include_directories(MuTests PRIVATE
    "${CMAKE_SOURCE_DIR}/src/dependencies/miniaudio"
    "${CMAKE_SOURCE_DIR}/src/source/Platform/MiniAudio"
)

# Story 3.2.1: char16_t Encoding at .NET Interop Boundary [VS1-NET-CHAR16T-ENCODING]
# RED PHASE: Tests mu_wchar_to_char16 / mu_char16_to_wchar round-trips and byte-level
# UTF-16LE baseline for Korean, Latin, mixed-script, null, and IP address strings.
# Tests fail until PlatformCompat.h has the mu_wchar_to_char16 / mu_char16_to_wchar utilities.
target_sources(MuTests PRIVATE platform/test_char16t_encoding.cpp)

# Stories 3.3.1 / 3.3.2: Platform connectivity smoke tests
# MU_TEST_LIBRARY_PATH is set to the absolute path of the platform's .NET AOT library.
# Using a single if/elseif block avoids potential duplicate-definition warnings if
# a future platform satisfies both conditions. (Story 3.3.2 fix L-1)
#
# MEDIUM-2 fix (Story 3.3.1 code review): MU_TEST_LIBRARY_PATH is now set unconditionally
# (without EXISTS check) when DOTNETAOT_FOUND and platform match. The previous EXISTS guard
# was evaluated at CMake configure time, but `dotnet publish` runs at build time via
# add_custom_command — so the dylib never exists at configure time on a clean build, causing
# both smoke tests to always SKIP. The test code already handles absent files gracefully
# via std::filesystem::exists() + SKIP macro.
if(APPLE AND DOTNETAOT_FOUND)
    # Story 3.3.1: macOS [VS1-NET-VALIDATE-MACOS]
    target_compile_definitions(MuTests PRIVATE
        MU_TEST_LIBRARY_PATH="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MUnique.Client.Library.dylib"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND DOTNETAOT_FOUND)
    # Story 3.3.2: Linux — MU_TEST_LIBRARY_PATH must be absolute; Linux dlopen() does NOT
    # search the executable directory for bare filenames. (Risk R6 mitigation)
    target_compile_definitions(MuTests PRIVATE
        MU_TEST_LIBRARY_PATH="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MUnique.Client.Library.so"
    )
endif()
# Both test files are always compiled; platform-specific code is guarded by #ifdef internally.
target_sources(MuTests PRIVATE platform/test_macos_connectivity.cpp)
target_sources(MuTests PRIVATE platform/test_linux_connectivity.cpp)

# Story 4.3.1/4.3.2: MURenderFX provides MuRendererSDLGpu.cpp symbols
# (GetBlendFactors, TextureRegistry, GetShaderBlobPath, GetPipelineSetFor, etc.)
# needed by test_sdlgpubackend.cpp and test_shaderprograms.cpp.
target_link_libraries(MuTests PRIVATE Catch2::Catch2WithMain MUCore MUPlatform MURenderFX)

# Story 7-8-3: Stub definitions for game globals referenced by MUCore/MUPlatform/MURenderFX
# that are defined in game source files (MuMain.cpp, ZzzInterface.cpp, etc.) not linked
# into MuTests. Required for test binary to link on all platforms.
target_sources(MuTests PRIVATE stubs/test_game_stubs.cpp)

# Story 2.2.1: Propagate MU_ENABLE_SDL3 to MuTests so test_platform_input.cpp
# #ifdef guards are active when SDL3 is enabled. [VS1-SDL-INPUT-KEYBOARD]
# Story 7-3-1: SDL3 include paths needed by test TUs that include PlatformCompat.h
if(MU_ENABLE_SDL3)
    target_compile_definitions(MuTests PRIVATE MU_ENABLE_SDL3)
    target_link_libraries(MuTests PRIVATE SDL3::SDL3-static)
    # Story 7-9-8: SDL_ttf needed by test_sdl_ttf_7_9_8.cpp GPU text engine tests
    if(TARGET SDL3_ttf::SDL3_ttf)
        target_link_libraries(MuTests PRIVATE SDL3_ttf::SDL3_ttf)
    endif()
endif()
# Story 7-8-3: MURenderFX references OpenGL symbols (glAlphaFunc, glBegin, etc.)
# On macOS, these come from the OpenGL framework.
if(APPLE)
    target_link_libraries(MuTests PRIVATE "-framework OpenGL")
endif()

# Story 7.3.1: macOS 60-Minute Stability Session [VS0-QUAL-STABILITY-MACOS]
# RED PHASE: Infrastructure tests verify MuTimer hitch/FPS APIs (AC-4) and
# log-scan utility for ERROR entry counting (AC-5). Manual session ACs (AC-1,
# AC-2, AC-3, AC-6, AC-VAL-1, AC-VAL-2, AC-VAL-3) use Catch2 SKIP until the
# stability session is conducted and SESSION_* constants are populated.
# Tests compile and run on macOS/Linux/Windows (MinGW CI) — no Win32 or GUI required.
# GREEN PHASE: Populate SESSION_* constants, remove SKIP markers, all tests pass.
# Test file is in MuStabilityTests standalone target only — not added to MuTests to avoid duplicate CTest registration.

# Standalone stability test target — compiles on all platforms without Win32 dependencies.
# Links same modules as MuTests to resolve MUCore transitive dependencies (SDL3, GL, audio).
add_executable(MuStabilityTests stability/test_macos_stability_session.cpp)
target_sources(MuStabilityTests PRIVATE stubs/test_game_stubs.cpp)
target_link_libraries(MuStabilityTests PRIVATE Catch2::Catch2WithMain MUCore MUPlatform MURenderFX MUAudio)
if(MU_ENABLE_SDL3)
    target_link_libraries(MuStabilityTests PRIVATE SDL3::SDL3-static)
endif()
if(APPLE)
    target_link_libraries(MuStabilityTests PRIVATE "-framework OpenGL")
endif()
target_include_directories(MuStabilityTests PRIVATE
    "${CMAKE_SOURCE_DIR}/src/source/Core"
    "${CMAKE_SOURCE_DIR}/src/dependencies/miniaudio"
    "${CMAKE_SOURCE_DIR}/src/source/Platform/MiniAudio"
)

# Story 7.3.2: Linux 60-Minute Stability Session [VS0-QUAL-STABILITY-LINUX]
# RED PHASE: Infrastructure tests verify MuTimer hitch/FPS APIs (AC-4) and
# log-scan utility for ERROR entry counting (AC-5). Manual session ACs (AC-1,
# AC-2, AC-3, AC-6, AC-VAL-1, AC-VAL-2, AC-VAL-3) use Catch2 SKIP until the
# stability session is conducted and SESSION_* constants are populated.
# Tests compile and run on macOS/Linux/Windows (MinGW CI) — no Win32 or GUI required.
# GREEN PHASE: Populate SESSION_* constants, remove SKIP markers, all tests pass.
# Standalone target mirrors MuStabilityTests pattern from story 7.3.1.
add_executable(MuLinuxStabilityTests stability/test_linux_stability_session.cpp)
target_sources(MuLinuxStabilityTests PRIVATE stubs/test_game_stubs.cpp)
target_link_libraries(MuLinuxStabilityTests PRIVATE Catch2::Catch2WithMain MUCore MUPlatform MURenderFX MUAudio)
if(MU_ENABLE_SDL3)
    target_link_libraries(MuLinuxStabilityTests PRIVATE SDL3::SDL3-static)
endif()
if(APPLE)
    target_link_libraries(MuLinuxStabilityTests PRIVATE "-framework OpenGL")
endif()
target_include_directories(MuLinuxStabilityTests PRIVATE
    "${CMAKE_SOURCE_DIR}/src/source/Core"
    "${CMAKE_SOURCE_DIR}/src/dependencies/miniaudio"
    "${CMAKE_SOURCE_DIR}/src/source/Platform/MiniAudio"
)

include(CTest)
include(Catch)
catch_discover_tests(MuTests)
catch_discover_tests(MuStabilityTests)
catch_discover_tests(MuLinuxStabilityTests)

# Build system validation tests (CMake script mode, no Catch2)
add_subdirectory(build)

# Platform abstraction header tests (Story 1.2.1)
add_subdirectory(platform)
