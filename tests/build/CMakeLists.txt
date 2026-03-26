# Build system validation tests
# These use cmake -P (script mode) and shell scripts, not Catch2.

set(MUMAIN_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

# ==========================================================================
# Story 1.1.2 - Linux CMake Toolchain
# ==========================================================================

# AC-1: Linux x64 toolchain file validation
add_test(
    NAME "AC-1:linux-toolchain-file"
    COMMAND ${CMAKE_COMMAND}
        -DTOOLCHAIN_FILE=${MUMAIN_SOURCE_DIR}/cmake/toolchains/linux-x64.cmake
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac1_linux_toolchain_file.cmake
)

# AC-2: CMakePresets.json Linux preset validation
add_test(
    NAME "AC-2:linux-presets"
    COMMAND ${CMAKE_COMMAND}
        -DPRESETS_FILE=${MUMAIN_SOURCE_DIR}/CMakePresets.json
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac2_linux_presets.cmake
)

# AC-3: cmake --preset linux-x64 configure succeeds (Linux hosts only)
add_test(
    NAME "AC-3:linux-configure"
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/test_ac3_linux_configure.sh
)

# ==========================================================================
# Story 1.3.1 - SDL3 Dependency Integration
# ==========================================================================

# AC-1: SDL3 FetchContent with pinned version in CMakeLists.txt
add_test(
    NAME "1.3.1-AC-1:sdl3-fetchcontent"
    COMMAND ${CMAKE_COMMAND}
        -DCMAKELISTS_FILE=${MUMAIN_SOURCE_DIR}/src/CMakeLists.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac1_sdl3_fetchcontent.cmake
)

# AC-3: SDL3 link visibility (MUPlatform + MURenderFX PRIVATE only)
add_test(
    NAME "1.3.1-AC-3:sdl3-link-visibility"
    COMMAND ${CMAKE_COMMAND}
        -DCMAKELISTS_FILE=${MUMAIN_SOURCE_DIR}/src/CMakeLists.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac3_sdl3_link_visibility.cmake
)

# AC-4: SDL3 headers not included in game logic directories
add_test(
    NAME "1.3.1-AC-4:sdl3-no-game-logic-includes"
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/test_ac4_sdl3_no_game_logic_includes.sh
)

# AC-5: MU_ENABLE_SDL3 option for CI gating
add_test(
    NAME "1.3.1-AC-5:sdl3-ci-option"
    COMMAND ${CMAKE_COMMAND}
        -DCMAKELISTS_FILE=${MUMAIN_SOURCE_DIR}/src/CMakeLists.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac5_sdl3_ci_option.cmake
)

# ==========================================================================
# Story 3.1.1 - CMake RID Detection & .NET AOT Build Integration
# ==========================================================================

# AC-1 + AC-5: FindDotnetAOT.cmake RID detection + WSL detection logic
add_test(
    NAME "3.1.1-AC-1:dotnet-rid-detection"
    COMMAND ${CMAKE_COMMAND}
        -DMODULE_FILE=${MUMAIN_SOURCE_DIR}/cmake/FindDotnetAOT.cmake
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac1_dotnet_rid_detection.cmake
)

# AC-2: MU_DOTNET_LIB_EXT set per platform and passed to C++ compiler
add_test(
    NAME "3.1.1-AC-2:dotnet-lib-ext"
    COMMAND ${CMAKE_COMMAND}
        -DMODULE_FILE=${MUMAIN_SOURCE_DIR}/cmake/FindDotnetAOT.cmake
        -DCMAKELISTS_FILE=${MUMAIN_SOURCE_DIR}/CMakeLists.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac2_dotnet_lib_ext.cmake
)

# AC-6: Graceful failure when dotnet not found
add_test(
    NAME "3.1.1-AC-6:dotnet-graceful-failure"
    COMMAND ${CMAKE_COMMAND}
        -DMODULE_FILE=${MUMAIN_SOURCE_DIR}/cmake/FindDotnetAOT.cmake
        -DCMAKELISTS_FILE=${MUMAIN_SOURCE_DIR}/CMakeLists.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac6_dotnet_graceful_failure.cmake
)

# AC-STD-11: Flow code VS1-NET-CMAKE-RID traceability
add_test(
    NAME "3.1.1-AC-STD-11:flow-code-traceability"
    COMMAND ${CMAKE_COMMAND}
        -DMODULE_FILE=${MUMAIN_SOURCE_DIR}/cmake/FindDotnetAOT.cmake
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_3_1_1.cmake
)

# ==========================================================================
# Story 3.1.2 - Connection.h Cross-Platform Updates [VS1-NET-CONNECTION-XPLAT]
# ==========================================================================

# AC-STD-11 / AC-4 / AC-STD-3 / AC-VAL-3: Flow code traceability + no platform ifdefs
add_test(
    NAME "3.1.2-AC-STD-11:flow-code-traceability"
    COMMAND ${CMAKE_COMMAND}
        -DCONNECTION_H=${MUMAIN_SOURCE_DIR}/src/source/Dotnet/Connection.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_3_1_2.cmake
)

# ==========================================================================
# Story 7.1.1 - Cross-Platform Error Reporting [VS0-QUAL-ERRORREPORT-XPLAT]
# RED PHASE: AC-3 and AC-STD-11 tests fail until ErrorReport.cpp is refactored.
# ==========================================================================

# AC-3 / AC-STD-3: No Win32 file I/O in ErrorReport.cpp cross-platform path
add_test(
    NAME "7.1.1-AC-3:no-win32-error-report"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MUMAIN_SOURCE_DIR}/src/source/Core/ErrorReport.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac3_no_win32_error_report.cmake
)

# AC-STD-11: Flow code VS0-QUAL-ERRORREPORT-XPLAT traceability
add_test(
    NAME "7.1.1-AC-STD-11:flow-code-traceability"
    COMMAND ${CMAKE_COMMAND}
        -DTEST_DIR=${MUMAIN_SOURCE_DIR}/tests/core
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_7_1_1.cmake
)

# ==========================================================================
# Story 3.2.1 - char16_t Encoding at .NET Interop Boundary [VS1-NET-CHAR16T-ENCODING]
# ==========================================================================

# AC-STD-11 / AC-1 / AC-3 / AC-STD-3 / AC-VAL-3:
#   Flow code traceability + no wchar_t at interop boundary in Connection.h and Common.xslt
add_test(
    NAME "3.2.1-AC-STD-11:flow-code-traceability"
    COMMAND ${CMAKE_COMMAND}
        -DCONNECTION_H=${MUMAIN_SOURCE_DIR}/src/source/Dotnet/Connection.h
        -DCOMMON_XSLT=${MUMAIN_SOURCE_DIR}/ClientLibrary/Common.xslt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_3_2_1.cmake
)

# AC-2: Marshal.PtrToStringUni regression guard — ConnectionManager.cs
# Verifies PtrToStringUni present and PtrToStringAuto absent (automated guard for inspection-based AC-2 check)
add_test(
    NAME "3.2.1-AC-2:marshal-ptr-to-string-uni"
    COMMAND ${CMAKE_COMMAND}
        -DCONNECTION_MANAGER_CS=${MUMAIN_SOURCE_DIR}/ClientLibrary/ConnectionManager.cs
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac2_marshal_ptr_3_2_1.cmake
)

# ==========================================================================
# Story 3.3.1 - macOS Server Connectivity Validation [VS1-NET-VALIDATE-MACOS]
# ==========================================================================

# AC-STD-11 / AC-VAL-3: Flow code traceability — VS1-NET-VALIDATE-MACOS in test file header
add_test(
    NAME "3.3.1-AC-STD-11:flow-code-traceability"
    COMMAND ${CMAKE_COMMAND}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_3_3_1.cmake
)

# ==========================================================================
# Story 3.3.2 - Linux Server Connectivity Validation [VS1-NET-VALIDATE-LINUX]
# ==========================================================================

# AC-STD-11 / AC-VAL-5: Flow code traceability — VS1-NET-VALIDATE-LINUX in test file header
add_test(
    NAME "3.3.2-AC-STD-11:flow-code-traceability"
    COMMAND ${CMAKE_COMMAND}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_3_3_2.cmake
)

# ==========================================================================
# Story 3.4.1 - Connection Error Messaging & Graceful Degradation [VS1-NET-ERROR-MESSAGING]
# ==========================================================================

# AC-STD-11 / AC-VAL-4: Flow code traceability — VS1-NET-ERROR-MESSAGING in Connection.cpp header
add_test(
    NAME "3.4.1-AC-STD-11:flow-code-traceability"
    COMMAND ${CMAKE_COMMAND}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_3_4_1.cmake
)

# ==========================================================================
# Story 3.4.2 - Server Connection Configuration [VS1-NET-CONFIG-SERVER]
# ==========================================================================

# AC-STD-11 / AC-VAL-4: Flow code traceability — VS1-NET-CONFIG-SERVER in GameConfig.cpp header
add_test(
    NAME "3.4.2-AC-STD-11:flow-code-traceability"
    COMMAND ${CMAKE_COMMAND}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_3_4_2.cmake
)

# ==========================================================================
# Story 7.4.1 - Native Platform CI Runners [VS0-QUAL-CI-NATIVE]
# RED PHASE: Tests validate ci.yml structure for native macOS and Linux runners.
# ==========================================================================

# AC-1: build-macos job with macos-latest runner and macos-arm64 preset
add_test(
    NAME "7.4.1-AC-1:macos-ci-runner"
    COMMAND ${CMAKE_COMMAND}
        -DCI_WORKFLOW_FILE=${MUMAIN_SOURCE_DIR}/.github/workflows/ci.yml
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac1_macos_ci_runner.cmake
)

# AC-2: build-linux job with ubuntu-latest runner and linux-x64 preset
add_test(
    NAME "7.4.1-AC-2:linux-ci-runner"
    COMMAND ${CMAKE_COMMAND}
        -DCI_WORKFLOW_FILE=${MUMAIN_SOURCE_DIR}/.github/workflows/ci.yml
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac2_linux_ci_runner.cmake
)

# AC-3: Both native runners execute configure, build, and ctest --output-on-failure
add_test(
    NAME "7.4.1-AC-3:native-runners-execute-ctest"
    COMMAND ${CMAKE_COMMAND}
        -DCI_WORKFLOW_FILE=${MUMAIN_SOURCE_DIR}/.github/workflows/ci.yml
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac3_native_runners_execute_ctest.cmake
)

# AC-5: Existing MinGW cross-compile job unchanged (regression guard)
add_test(
    NAME "7.4.1-AC-5:mingw-job-unchanged"
    COMMAND ${CMAKE_COMMAND}
        -DCI_WORKFLOW_FILE=${MUMAIN_SOURCE_DIR}/.github/workflows/ci.yml
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac5_mingw_job_unchanged.cmake
)

# AC-6: release job requires all three platform jobs (quality, build, build-macos, build-linux)
add_test(
    NAME "7.4.1-AC-6:release-needs-all-platforms"
    COMMAND ${CMAKE_COMMAND}
        -DCI_WORKFLOW_FILE=${MUMAIN_SOURCE_DIR}/.github/workflows/ci.yml
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac6_release_needs_all_platforms.cmake
)

# AC-7: SDL3 FetchContent caching on both native runners
add_test(
    NAME "7.4.1-AC-7:sdl3-cache-native-runners"
    COMMAND ${CMAKE_COMMAND}
        -DCI_WORKFLOW_FILE=${MUMAIN_SOURCE_DIR}/.github/workflows/ci.yml
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac7_sdl3_cache_native_runners.cmake
)

# AC-8: .NET SDK not required on native runners (MU_ENABLE_DOTNET=OFF)
add_test(
    NAME "7.4.1-AC-8:dotnet-disabled-native-runners"
    COMMAND ${CMAKE_COMMAND}
        -DCI_WORKFLOW_FILE=${MUMAIN_SOURCE_DIR}/.github/workflows/ci.yml
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac8_dotnet_disabled_native_runners.cmake
)

# AC-STD-11: Flow code VS0-QUAL-CI-NATIVE traceability + AC-STD-1 pattern consistency
add_test(
    NAME "7.4.1-AC-STD-11:flow-code-traceability"
    COMMAND ${CMAKE_COMMAND}
        -DCI_WORKFLOW_FILE=${MUMAIN_SOURCE_DIR}/.github/workflows/ci.yml
        -DTEST_BUILD_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_7_4_1.cmake
)

# ==========================================================================
# Story 7.3.0 - macOS Native Build Compatibility Fixes [VS0-QUAL-BUILDCOMPAT-MACOS]
# RED PHASE: Tests fail until build compatibility fixes are implemented.
# ==========================================================================

# AC-2: MU_ENABLE_SDL3 defined at project scope in src/CMakeLists.txt
add_test(
    NAME "7.3.0-AC-2:cmake-mu-enable-sdl3-project-scope"
    COMMAND ${CMAKE_COMMAND}
        -DCMAKELISTS_FILE=${MUMAIN_SOURCE_DIR}/src/CMakeLists.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac2_cmake_mu_enable_sdl3_7_3_0.cmake
)

# AC-3: PlatformTypes.h defines CONST and CP_UTF8 in non-Win32 #else block
add_test(
    NAME "7.3.0-AC-3:platform-types-const-cp-utf8"
    COMMAND ${CMAKE_COMMAND}
        -DPLATFORM_TYPES_H=${MUMAIN_SOURCE_DIR}/src/source/Platform/PlatformTypes.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac3_platform_types_const_cp_utf8.cmake
)

# AC-4: PlatformCompat.h provides _wcsicmp/wcsicmp, _TRUNCATE, OutputDebugString stubs
add_test(
    NAME "7.3.0-AC-4:platform-compat-wcsicmp-stubs"
    COMMAND ${CMAKE_COMMAND}
        -DPLATFORM_COMPAT_H=${MUMAIN_SOURCE_DIR}/src/source/Platform/PlatformCompat.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac4_platform_compat_wcsicmp_stubs.cmake
)

# AC-5: GlobalBitmap.cpp NarrowPath uses mu_wchar_to_utf8, no deprecated wstring_convert
add_test(
    NAME "7.3.0-AC-5:globalbitmaps-no-wstring-convert"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MUMAIN_SOURCE_DIR}/src/source/Data/GlobalBitmap.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac5_globalbitmaps_no_wstring_convert.cmake
)

# AC-6: GlobalBitmap.cpp uses numeric literals instead of GL_CLAMP_TO_EDGE/GL_REPEAT
add_test(
    NAME "7.3.0-AC-6:globalbitmaps-gl-literals"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MUMAIN_SOURCE_DIR}/src/source/Data/GlobalBitmap.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac6_globalbitmaps_gl_literals.cmake
)

# AC-7: LoadData.cpp has no #include <shlwapi.h>
add_test(
    NAME "7.3.0-AC-7:loaddata-no-shlwapi"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MUMAIN_SOURCE_DIR}/src/source/Data/LoadData.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac7_loaddata_no_shlwapi.cmake
)

# AC-8: GameConfig.cpp DPAPI calls guarded by #ifdef _WIN32
add_test(
    NAME "7.3.0-AC-8:gameconfig-dpapi-guarded"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MUMAIN_SOURCE_DIR}/src/source/Data/GameConfig.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac8_gameconfig_dpapi_guarded.cmake
)

# AC-9: No new unguarded Win32 calls in game logic files (MinGW regression guard)
add_test(
    NAME "7.3.0-AC-9:mingw-no-regression"
    COMMAND ${CMAKE_COMMAND}
        -DMUMAIN_SOURCE_DIR=${MUMAIN_SOURCE_DIR}/src/source
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac9_mingw_no_regression_7_3_0.cmake
)

# AC-STD-11: Flow code VS0-QUAL-BUILDCOMPAT-MACOS traceability
add_test(
    NAME "7.3.0-AC-STD-11:flow-code-traceability"
    COMMAND ${CMAKE_COMMAND}
        -DTEST_BUILD_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_7_3_0.cmake
)

# ==========================================================================
# Story 7.5.1 - Fix Remaining macOS Build Failures and Remove Quality Gate Bypass
#              [VS0-QUAL-BUILDFIXREM-MACOS]
# RED PHASE: Tests fail until build compatibility fixes are implemented.
# ==========================================================================

# AC-1: SkillDataLoader.cpp swprintf uses 3-arg POSIX form (not 2-arg Windows form)
add_test(
    NAME "7.5.1-AC-1:swprintf-signature"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MUMAIN_SOURCE_DIR}/src/source/Data/Skills/SkillDataLoader.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac1_swprintf_signature_7_5_1.cmake
)

# AC-2: ZzzOpenData.cpp MODEL_TYPE_CHARM_MIXWING uses static_cast<int>
add_test(
    NAME "7.5.1-AC-2:enum-cast-zzzopen"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MUMAIN_SOURCE_DIR}/src/source/Data/ZzzOpenData.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac2_enum_cast_zzzopen_7_5_1.cmake
)

# AC-3: ZzzInfomation.cpp wchar_t null comparisons use L'\0' not NULL
add_test(
    NAME "7.5.1-AC-3:wchar-null-compare"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MUMAIN_SOURCE_DIR}/src/source/Data/ZzzInfomation.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac3_wchar_null_compare_7_5_1.cmake
)

# AC-4: ZzzInfomation.cpp unused variables at line 237 resolved
add_test(
    NAME "7.5.1-AC-4:unused-vars"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MUMAIN_SOURCE_DIR}/src/source/Data/ZzzInfomation.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac4_unused_vars_7_5_1.cmake
)

# AC-5: ZzzInfomation.cpp && within || precedence warnings resolved with explicit parens
add_test(
    NAME "7.5.1-AC-5:precedence-parens"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MUMAIN_SOURCE_DIR}/src/source/Data/ZzzInfomation.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac5_precedence_parens_7_5_1.cmake
)

# AC-6: ZzzInfomation.cpp sign comparison between DWORD and PET_TYPE fixed
add_test(
    NAME "7.5.1-AC-6:sign-compare"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MUMAIN_SOURCE_DIR}/src/source/Data/ZzzInfomation.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac6_sign_compare_7_5_1.cmake
)

# AC-7: ZzzInfomation.cpp includes _GlobalFunctions.h for g_isCharacterBuff
add_test(
    NAME "7.5.1-AC-7:char-buff-include"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MUMAIN_SOURCE_DIR}/src/source/Data/ZzzInfomation.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac7_char_buff_include_7_5_1.cmake
)

# AC-9: .pcc-config.yaml skip_checks bypass removed
add_test(
    NAME "7.5.1-AC-9:skip-checks-removed"
    COMMAND ${CMAKE_COMMAND}
        -DROOT=${MUMAIN_SOURCE_DIR}/..
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac9_skip_checks_removed_7_5_1.cmake
)

# AC-10: No new Win32 guards in modified game logic files (MinGW regression guard)
add_test(
    NAME "7.5.1-AC-10:mingw-no-regression"
    COMMAND ${CMAKE_COMMAND}
        -DMUMAIN_SOURCE_DIR=${MUMAIN_SOURCE_DIR}/src/source
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac10_mingw_no_regression_7_5_1.cmake
)

# AC-STD-11: Flow code VS0-QUAL-BUILDFIXREM-MACOS traceability
add_test(
    NAME "7.5.1-AC-STD-11:flow-code-traceability"
    COMMAND ${CMAKE_COMMAND}
        -DTEST_BUILD_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_7_5_1.cmake
)

# ==========================================================================
# Story 7.6.1 - macOS Native Build — Remaining Compilation Gaps
#              [VS0-QUAL-BUILDCOMP-MACOS]
# Tests verify cross-platform fixes for MURenderFX, MUAudio, MUThirdParty targets.
# ==========================================================================

# AC-2: ./ctl build explicitly uses Homebrew LLVM compiler paths
add_test(
    NAME "7.6.1-AC-2:ctl-uses-homebrew-llvm"
    COMMAND ${CMAKE_COMMAND}
        -DCTL_SCRIPT=${MUMAIN_SOURCE_DIR}/../ctl
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac2_ctl_homebrew_llvm_7_6_1.cmake
)

# AC-3: .pcc-config.yaml build command specifies Homebrew LLVM; no stale macOS comment
add_test(
    NAME "7.6.1-AC-3:pcc-config-build-homebrew"
    COMMAND ${CMAKE_COMMAND}
        -DPCC_CONFIG_FILE=${MUMAIN_SOURCE_DIR}/../.pcc-config.yaml
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac3_pcc_config_build_homebrew_7_6_1.cmake
)

# AC-4: quality_gate in .pcc-config.yaml includes cmake --build build
add_test(
    NAME "7.6.1-AC-4:pcc-config-quality-gate-includes-build"
    COMMAND ${CMAKE_COMMAND}
        -DPCC_CONFIG_FILE=${MUMAIN_SOURCE_DIR}/../.pcc-config.yaml
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac4_pcc_config_quality_gate_build_7_6_1.cmake
)

# AC-5: ZzzOpenglUtil.cpp excluded from macOS/non-Win32 CMake build
add_test(
    NAME "7.6.1-AC-5:wgl-cmake-exclusion"
    COMMAND ${CMAKE_COMMAND}
        -DCMAKELISTS_FILE=${MUMAIN_SOURCE_DIR}/src/CMakeLists.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac5_wgl_cmake_exclusion_7_6_1.cmake
)

# AC-6: DSwaveIO.h includes mmsystem.h only inside #ifdef _WIN32 guard
add_test(
    NAME "7.6.1-AC-6:dswaveio-mmsystem-guard"
    COMMAND ${CMAKE_COMMAND}
        -DDSWAVEIO_H=${MUMAIN_SOURCE_DIR}/src/source/Audio/DSwaveIO.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac6_dswaveio_mmsystem_guard_7_6_1.cmake
)

# AC-7: PlatformCompat.h provides all required GDI/Win32 stubs for UIControls.cpp
add_test(
    NAME "7.6.1-AC-7:platform-compat-gdi-stubs"
    COMMAND ${CMAKE_COMMAND}
        -DPLATFORM_COMPAT_H=${MUMAIN_SOURCE_DIR}/src/source/Platform/PlatformCompat.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac7_platform_compat_gdi_stubs_7_6_1.cmake
)

# AC-8: xstreambuf.cpp delete void* fixed with typed cast
add_test(
    NAME "7.6.1-AC-8:xstreambuf-no-delete-void"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MUMAIN_SOURCE_DIR}/src/source/ThirdParty/xstreambuf.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac8_xstreambuf_no_delete_void_7_6_1.cmake
)

# AC-9: PosixSignalHandlers.cpp uses SA_SIGINFO (not SA_SIGACTION)
add_test(
    NAME "7.6.1-AC-9:posix-sa-siginfo"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MUMAIN_SOURCE_DIR}/src/source/Platform/posix/PosixSignalHandlers.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac9_posix_sa_siginfo_7_6_1.cmake
)

# AC-10: ZzzOpenData.cpp Wnontrivial-memcall pragma guarded with __has_warning
add_test(
    NAME "7.6.1-AC-10:pragma-has-warning-guard"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MUMAIN_SOURCE_DIR}/src/source/Data/ZzzOpenData.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac10_pragma_has_warning_7_6_1.cmake
)

# AC-11: No new Win32 guards in modified game logic files (MinGW regression guard)
add_test(
    NAME "7.6.1-AC-11:mingw-no-regression"
    COMMAND ${CMAKE_COMMAND}
        -DMUMAIN_SOURCE_DIR=${MUMAIN_SOURCE_DIR}/src/source
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac11_mingw_no_regression_7_6_1.cmake
)

# AC-STD-11: Flow code VS0-QUAL-BUILDCOMP-MACOS traceability
add_test(
    NAME "7.6.1-AC-STD-11:flow-code-traceability"
    COMMAND ${CMAKE_COMMAND}
        -DTEST_BUILD_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_7_6_1.cmake
)

# ==========================================================================
# Story 7.6.2 - Win32 String Conversion and Include Guard Cleanup
#              [VS0-QUAL-WIN32CLEAN-STRINCLUDE]
# Tests verify that Win32 string conversion guards and bare include guards
# have been removed from Core, Data, GameShop, Gameplay, RenderFX, and Scenes.
# ==========================================================================

set(MU_SOURCE_ROOT "${MUMAIN_SOURCE_DIR}/src/source")

# AC-1: check-win32-guards.py exists and all story-scoped source files are present
add_test(
    NAME "7.6.2-AC-1:win32-guard-check-script"
    COMMAND ${CMAKE_COMMAND}
        -DSCRIPT_PATH=${MUMAIN_SOURCE_DIR}/scripts/check-win32-guards.py
        -DSOURCE_ROOT=${MU_SOURCE_ROOT}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac1_win32_guard_check_7_6_2.cmake
)

# AC-2: Core/muConsoleDebug.cpp uses mu_wchar_to_utf8() and removes <io.h> guard
add_test(
    NAME "7.6.2-AC-2:console-debug-migration"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MU_SOURCE_ROOT}/Core/muConsoleDebug.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac2_console_debug_migration_7_6_2.cmake
)

# AC-3: Core/StringUtils.h uses mu_wchar_to_utf8()
add_test(
    NAME "7.6.2-AC-3:string-utils-migration"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MU_SOURCE_ROOT}/Core/StringUtils.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac3_string_utils_migration_7_6_2.cmake
)

# AC-4: Data/GlobalBitmap.cpp uses mu_wchar_to_utf8()
add_test(
    NAME "7.6.2-AC-4:global-bitmap-migration"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MU_SOURCE_ROOT}/Data/GlobalBitmap.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac4_global_bitmap_migration_7_6_2.cmake
)

# AC-5: GameShop/MsgBoxIGSBuyConfirm.cpp uses mu_swprintf; no StringCbPrintf or <strsafe.h>
add_test(
    NAME "7.6.2-AC-5:msgbox-swprintf-migration"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MU_SOURCE_ROOT}/GameShop/MsgBoxIGSBuyConfirm.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac5_msgbox_swprintf_migration_7_6_2.cmake
)

# AC-6: Gameplay/Characters/ZzzCharacter.cpp has no <eh.h> include
add_test(
    NAME "7.6.2-AC-6:zzz-character-no-eh"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MU_SOURCE_ROOT}/Gameplay/Characters/ZzzCharacter.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac6_zzz_character_no_eh_7_6_2.cmake
)

# AC-7: RenderFX/MuRendererSDLGpu.cpp has unconditional SDL3 includes
add_test(
    NAME "7.6.2-AC-7:sdl-gpu-unconditional-includes"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${MU_SOURCE_ROOT}/RenderFX/MuRendererSDLGpu.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac7_sdl_gpu_unconditional_7_6_2.cmake
)

# AC-8: Scene headers have complete #else / PlatformCompat.h branch
add_test(
    NAME "7.6.2-AC-8:scene-headers-else-branch"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_ROOT=${MU_SOURCE_ROOT}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac8_scene_headers_else_branch_7_6_2.cmake
)

# AC-9: Data struct headers have complete #else / PlatformCompat.h branch
add_test(
    NAME "7.6.2-AC-9:data-headers-else-branch"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_ROOT=${MU_SOURCE_ROOT}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac9_data_headers_else_branch_7_6_2.cmake
)

# AC-STD-11: Flow code VS0-QUAL-WIN32CLEAN-STRINCLUDE traceability
add_test(
    NAME "7.6.2-AC-STD-11:flow-code-traceability"
    COMMAND ${CMAKE_COMMAND}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_7_6_2.cmake
)

# ==========================================================================
# Story 7.8.1 - Audio Interface Win32 Type Cleanup [VS0-QUAL-BUILD-AUDIO]
# RED PHASE: Tests verify that Win32 types (HRESULT, BOOL, OBJECT*) are removed
# from IPlatformAudio.h and MiniAudioBackend.h, and that DirectSound-specific
# declarations in DSPlaySound.h are guarded by #ifdef _WIN32.
# ==========================================================================

set(MU_AUDIO_SOURCE_ROOT "${MUMAIN_SOURCE_DIR}/src/source")

# AC-1: DSPlaySound.h HRESULT/HWND/OBJECT* declarations wrapped in #ifdef _WIN32
add_test(
    NAME "7.8.1-AC-1:dsplaysound-win32-guard"
    COMMAND ${CMAKE_COMMAND}
        -DDSPLAYSOUND_H=${MU_AUDIO_SOURCE_ROOT}/Audio/DSPlaySound.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac1_dsplaysound_win32_guard_7_8_1.cmake
)

# AC-2: IPlatformAudio.h uses only portable types (no HRESULT, BOOL, OBJECT*)
add_test(
    NAME "7.8.1-AC-2:iplatformaudio-portable-types"
    COMMAND ${CMAKE_COMMAND}
        -DIPLATFORMAUDIO_H=${MU_AUDIO_SOURCE_ROOT}/Platform/IPlatformAudio.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac2_iplatformaudio_portable_types_7_8_1.cmake
)

# AC-3: MiniAudioBackend.h override declarations have no Win32 types
add_test(
    NAME "7.8.1-AC-3:miniaudiobackend-no-win32-types"
    COMMAND ${CMAKE_COMMAND}
        -DMINIAUDIO_BACKEND_H=${MU_AUDIO_SOURCE_ROOT}/Platform/MiniAudio/MiniAudioBackend.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac3_miniaudiobackend_no_win32_types_7_8_1.cmake
)

# AC-5: check-win32-guards.py script exists; audio headers present; no bare Win32 guards
add_test(
    NAME "7.8.1-AC-5:check-win32-guards-audio"
    COMMAND ${CMAKE_COMMAND}
        -DSCRIPT_PATH=${MUMAIN_SOURCE_DIR}/scripts/check-win32-guards.py
        -DSOURCE_ROOT=${MU_AUDIO_SOURCE_ROOT}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac5_check_win32_guards_audio_7_8_1.cmake
)

# AC-STD-11: Flow code VS0-QUAL-BUILD-AUDIO traceability
add_test(
    NAME "7.8.1-AC-STD-11:flow-code-traceability"
    COMMAND ${CMAKE_COMMAND}
        -DTEST_BUILD_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_7_8_1.cmake
)

# ==========================================================================
# Story 7.8.2 - Gameplay Header Cross-Platform Fixes [VS0-QUAL-BUILD-HEADERS]
# RED PHASE: Tests verify that gameplay headers are self-contained and ODR-safe:
#   AC-1: mu_enum.h SKILL_REPLACEMENTS is declared inline (ODR-safe)
#   AC-2: ZzzPath.h explicitly includes ErrorReport.h (no transitive dep on g_ErrorReport)
#   AC-3: SkillStructs.h includes MultiLanguage.h (CMultiLanguage dependency explicit)
#   AC-4: CSItemOption.h includes mu_enum.h and ITEM header (ActionSkillType, ITEM types)
# ==========================================================================

set(MU_GAMEPLAY_SOURCE_ROOT "${MUMAIN_SOURCE_DIR}/src/source")

# AC-1: Core/mu_enum.h SKILL_REPLACEMENTS declared with 'inline' keyword (ODR-safe)
add_test(
    NAME "7.8.2-AC-1:mu-enum-inline-skill-replacements"
    COMMAND ${CMAKE_COMMAND}
        -DMU_ENUM_H=${MU_GAMEPLAY_SOURCE_ROOT}/Core/mu_enum.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac1_mu_enum_inline_7_8_2.cmake
)

# AC-2: World/ZzzPath.h explicitly includes ErrorReport.h
add_test(
    NAME "7.8.2-AC-2:zzzpath-errorreport-include"
    COMMAND ${CMAKE_COMMAND}
        -DZZZ_PATH_H=${MU_GAMEPLAY_SOURCE_ROOT}/World/ZzzPath.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac2_zzzpath_errorreport_include_7_8_2.cmake
)

# AC-3: Data/Skills/SkillStructs.h includes MultiLanguage.h (CMultiLanguage defined)
add_test(
    NAME "7.8.2-AC-3:skillstructs-multilanguage-include"
    COMMAND ${CMAKE_COMMAND}
        -DSKILL_STRUCTS_H=${MU_GAMEPLAY_SOURCE_ROOT}/Data/Skills/SkillStructs.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac3_skillstructs_multilanguage_7_8_2.cmake
)

# AC-4: Gameplay/Items/CSItemOption.h includes types for ActionSkillType and ITEM
add_test(
    NAME "7.8.2-AC-4:csitemoption-type-includes"
    COMMAND ${CMAKE_COMMAND}
        -DCS_ITEM_OPTION_H=${MU_GAMEPLAY_SOURCE_ROOT}/Gameplay/Items/CSItemOption.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac4_csitemoption_type_includes_7_8_2.cmake
)

# AC-STD-11: Flow code VS0-QUAL-BUILD-HEADERS traceability
add_test(
    NAME "7.8.2-AC-STD-11:flow-code-traceability"
    COMMAND ${CMAKE_COMMAND}
        -DTEST_BUILD_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_7_8_2.cmake
)

# ==========================================================================
# Story 7.8.4 - .NET Client Library Native Build [VS0-QUAL-BUILD-DOTNET]
# RED PHASE: Tests verify cmake RID detection and .NET build enable on
#   macOS/Linux. Tests fail until src/CMakeLists.txt and CMakePresets.json
#   are updated and Winmain.cpp has the resource.h guard.
# ==========================================================================

set(MU_DOTNET_SOURCE_ROOT "${MUMAIN_SOURCE_DIR}/src/source")

# AC-1: src/CMakeLists.txt uses CMAKE_SYSTEM_NAME for cross-platform DOTNET_RID detection
add_test(
    NAME "7.8.4-AC-1:src-cmake-rid-detection"
    COMMAND ${CMAKE_COMMAND}
        -DCMAKELISTS_FILE=${MUMAIN_SOURCE_DIR}/src/CMakeLists.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac1_src_cmake_rid_detection_7_8_4.cmake
)

# AC-2: src/CMakeLists.txt copy command uses MU_DOTNET_LIB_EXT (no hardcoded .dll)
add_test(
    NAME "7.8.4-AC-2:src-cmake-lib-ext-copy"
    COMMAND ${CMAKE_COMMAND}
        -DCMAKELISTS_FILE=${MUMAIN_SOURCE_DIR}/src/CMakeLists.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac2_src_cmake_lib_ext_copy_7_8_4.cmake
)

# AC-3: CMakePresets.json macos-base and linux-base no longer disable MU_ENABLE_DOTNET
add_test(
    NAME "7.8.4-AC-3:presets-dotnet-enabled"
    COMMAND ${CMAKE_COMMAND}
        -DPRESETS_FILE=${MUMAIN_SOURCE_DIR}/CMakePresets.json
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac3_presets_dotnet_enabled_7_8_4.cmake
)

# AC-4: Winmain.cpp wraps #include "resource.h" in #ifdef _WIN32 guard
add_test(
    NAME "7.8.4-AC-4:resource-h-win32-guard"
    COMMAND ${CMAKE_COMMAND}
        -DWINMAIN_CPP=${MU_DOTNET_SOURCE_ROOT}/Main/Winmain.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac4_resource_h_guard_7_8_4.cmake
)

# AC-STD-11: Flow code VS0-QUAL-BUILD-DOTNET traceability
add_test(
    NAME "7.8.4-AC-STD-11:flow-code-traceability"
    COMMAND ${CMAKE_COMMAND}
        -DTEST_BUILD_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_7_8_4.cmake
)
