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
# Implementation complete: g_platformAudio wired in Winmain.cpp; BGM delegation done.
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
#   Task 4: Remove InitDirectSound/FreeDirectSound from Winmain.cpp
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

# Story 2.2.1: Propagate MU_ENABLE_SDL3 to MuTests so test_platform_input.cpp
# #ifdef guards are active when SDL3 is enabled. [VS1-SDL-INPUT-KEYBOARD]
if(MU_ENABLE_SDL3)
    target_compile_definitions(MuTests PRIVATE MU_ENABLE_SDL3)
endif()

include(CTest)
include(Catch)
catch_discover_tests(MuTests)

# Build system validation tests (CMake script mode, no Catch2)
add_subdirectory(build)

# Platform abstraction header tests (Story 1.2.1)
add_subdirectory(platform)
