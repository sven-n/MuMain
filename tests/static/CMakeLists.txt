# Source-only checks: architecture guards and contracts that read the repository and need no compiler,
# dependencies or build output. Quality Gates and the pre-push hook configure this directory on its own:
#   cmake -S tests/static -B out/static-checks && ctest --test-dir out/static-checks
# The full test tree includes it too, labelled `static`; native CI jobs skip that label.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.25)
    project(MuStaticChecks NONE)
    enable_testing()
endif()

get_filename_component(MU_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(MU_TESTS "${MU_REPO_ROOT}/tests")
set(MU_SOURCE "${MU_REPO_ROOT}/src/source")
find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_test(
    NAME windows_workflow_contract
    COMMAND "${Python3_EXECUTABLE}" "${MU_TESTS}/test_windows_workflow_contract.py" "${MU_REPO_ROOT}"
)
add_test(
    NAME ci_build_plan
    COMMAND "${Python3_EXECUTABLE}" "${MU_TESTS}/test_ci_build_plan.py"
)
add_test(
    NAME ci_warning_scripts
    COMMAND "${Python3_EXECUTABLE}" "${MU_TESTS}/test_ci_warning_scripts.py"
)
add_test(
    NAME ci_cache_prune
    COMMAND "${Python3_EXECUTABLE}" "${MU_TESTS}/test_ci_cache_prune.py"
)
add_test(
    NAME ci_apt_cache
    COMMAND "${Python3_EXECUTABLE}" "${MU_TESTS}/test_ci_apt_cache.py"
)
add_test(
    NAME stamp_release_version
    COMMAND "${Python3_EXECUTABLE}" "${MU_TESTS}/test_stamp_release_version.py"
)
add_test(
    NAME static_check_scripts
    COMMAND "${Python3_EXECUTABLE}" "${MU_TESTS}/test_static_check_scripts.py" "${CMAKE_COMMAND}"
)

# doctest_discover_tests reads case names as a CMake list: a name it cuts or joins registers entries that run no case.
add_test(NAME test_case_names_reach_ctest_whole
    COMMAND "${Python3_EXECUTABLE}" "${MU_TESTS}/check_test_case_names.py" "${MU_TESTS}")
# The check on its fixtures: it reports each offence and exits 1, and passes the clean names.
add_test(NAME test_case_names_check_fixtures
    COMMAND "${Python3_EXECUTABLE}" "${MU_TESTS}/test_check_test_case_names.py")

add_test(
    NAME msvc_runtime_dll_staging_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_CMAKE_SOURCE=${MU_REPO_ROOT}/src/CMakeLists.txt"
        -P "${MU_TESTS}/core/test_msvc_runtime_dll_staging.cmake"
)
add_test(
    NAME dotnet_first_use_before_build_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_CMAKE_SOURCE=${MU_REPO_ROOT}/src/CMakeLists.txt"
        -P "${MU_TESTS}/test_dotnet_first_use.cmake"
)
add_test(
    NAME login_diagnostics_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_LOGIN_SOURCE=${MU_REPO_ROOT}/ClientLibrary/ConnectionManager.ClientToServer.Custom.cs"
        -P "${MU_TESTS}/network/test_login_diagnostics.cmake"
)

# Platform layering: input goes through the platform layer, never raw Win32 APIs.
add_test(
    NAME no_raw_getasynckeystate
    COMMAND "${CMAKE_COMMAND}"
        "-DSOURCE_DIR=${MU_SOURCE}"
        -P "${MU_TESTS}/platform/test_ac_std3_no_raw_getasynckeystate.cmake"
)
add_test(
    NAME no_raw_win32_mouse
    COMMAND "${CMAKE_COMMAND}"
        "-DSOURCE_DIR=${MU_SOURCE}"
        -P "${MU_TESTS}/platform/test_ac_std3_no_raw_win32_mouse.cmake"
)
add_test(
    NAME no_raw_imm_apis
    COMMAND "${CMAKE_COMMAND}"
        "-DSOURCE_DIR=${MU_SOURCE}"
        -P "${MU_TESTS}/platform/test_ac_std3_no_raw_imm.cmake"
)
add_test(
    NAME copy_runtime_assets
    COMMAND "${CMAKE_COMMAND}" -P "${MU_TESTS}/test_copy_runtime_assets.cmake"
)
add_test(
    NAME write_git_commit
    COMMAND "${CMAKE_COMMAND}" "-DMU_WORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/write-git-commit"
        "-DMU_SCRIPT=${MU_REPO_ROOT}/cmake/WriteGitCommit.cmake"
        -P "${MU_TESTS}/test_write_git_commit.cmake"
)
add_test(
    NAME ensure_submodule_leaves_history_out
    COMMAND "${CMAKE_COMMAND}" "-DMU_WORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/ensure-submodule"
        -P "${MU_TESTS}/test_ensure_submodule.cmake"
)
add_test(
    NAME archive_extract_times_contract
    COMMAND "${CMAKE_COMMAND}" "-DMU_ROOT_CMAKE=${MU_REPO_ROOT}/CMakeLists.txt"
        -P "${MU_TESTS}/test_archive_extract_times.cmake"
)
add_test(
    NAME sdl_image_archive_contract
    COMMAND "${CMAKE_COMMAND}" "-DMU_ROOT_CMAKE=${MU_REPO_ROOT}/CMakeLists.txt"
        -P "${MU_TESTS}/test_sdl_image_archive.cmake"
)
add_test(
    NAME sdl_ttf_archives_contract
    COMMAND "${CMAKE_COMMAND}" "-DMU_ROOT_CMAKE=${MU_REPO_ROOT}/CMakeLists.txt"
        -P "${MU_TESTS}/test_sdl_ttf_archives.cmake"
)

# Render and effects contracts.
add_test(
    NAME imgui_sdlgpu_backend_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_SOURCE_DIR=${MU_REPO_ROOT}/src"
        -P "${MU_TESTS}/render/test_imgui_sdlgpu_backend.cmake"
)
add_test(
    NAME particle_cull_keeps_depth_state
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_PARTICLE_SOURCE=${MU_SOURCE}/Render/Effects/ZzzEffectParticle.cpp"
        -P "${MU_TESTS}/render/test_particle_cull_state.cmake"
)
set(MU_EFFECTS_DIR "${MU_SOURCE}/Render/Effects")
add_test(
    NAME create_functions_have_no_fps_gate
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_EFFECT_SOURCES=CreateEffect=${MU_EFFECTS_DIR}/ZzzEffect.cpp\;CreateJoint=${MU_EFFECTS_DIR}/ZzzEffectJoint.cpp\;CreateParticle=${MU_EFFECTS_DIR}/ZzzEffectParticle.cpp"
        -P "${MU_TESTS}/render/test_create_has_no_fps_gate.cmake"
)
add_test(
    NAME fps_gates_are_not_nested
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_SOURCE_DIR=${MU_SOURCE}"
        -P "${MU_TESTS}/render/test_no_nested_fps_gate.cmake"
)
add_test(
    NAME renderer_pipeline_fail_fast
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_RENDERER_SOURCE=${MU_SOURCE}/Render/Renderer/MuRendererSDLGpu.cpp"
        -P "${MU_TESTS}/render/test_pipeline_fail_fast.cmake"
)
add_test(
    NAME d3d12_culling_diagnostic_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_RENDERER_SOURCE=${MU_SOURCE}/Render/Renderer/MuRendererSDLGpu.cpp"
        -P "${MU_TESTS}/render/test_d3d12_culling_diagnostic.cmake"
)
add_test(
    NAME world_clear_color_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_SCENE_MANAGER_SOURCE=${MU_SOURCE}/Scenes/SceneManager.cpp"
        -P "${MU_TESTS}/render/test_world_clear_color.cmake"
)
add_test(
    NAME scene_setup_before_frame_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_SCENE_MANAGER_SOURCE=${MU_SOURCE}/Scenes/SceneManager.cpp"
        -P "${MU_TESTS}/render/test_scene_setup_before_frame.cmake"
)
add_test(
    NAME render_diagnostics_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_SCENE_MANAGER_SOURCE=${MU_SOURCE}/Scenes/SceneManager.cpp"
        "-DMU_DIAGNOSTICS_OVERLAY_SOURCE=${MU_SOURCE}/UI/Diagnostics/DiagnosticsOverlay.cpp"
        "-DMU_DIAGNOSTICS_DOCUMENT=${MU_REPO_ROOT}/src/bin/Data/Interface/RmlUi/diagnostics.rml"
        -P "${MU_TESTS}/render/test_render_diagnostics.cmake"
)
add_test(
    NAME projected_shadow_state_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_BMD_SOURCE=${MU_SOURCE}/Render/Models/ZzzBMD.cpp"
        "-DMU_SHADOW_VOLUME_SOURCE=${MU_SOURCE}/Render/Models/ShadowVolume.cpp"
        -P "${MU_TESTS}/render/test_projected_shadow_state.cmake"
)
add_test(
    NAME terrain_depth_state_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_TERRAIN_SOURCE=${MU_SOURCE}/Render/Terrain/ZzzLodTerrain.cpp"
        -P "${MU_TESTS}/render/test_terrain_depth_state.cmake"
)
add_test(
    NAME coin_heap_skin_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_BMD_SOURCE=${MU_SOURCE}/Render/Models/ZzzBMD.cpp"
        -P "${MU_TESTS}/render/test_coin_heap_skin.cmake"
)

# Draws take the game's objects read-only (ADR 0003): the game's sources, and fixtures the check must catch or pass.
set(MU_DRAW_WRITES_FIXTURES "${MU_TESTS}/render/fixtures/draw_writes")
add_test(NAME draws_take_game_objects_read_only
    COMMAND "${Python3_EXECUTABLE}" "${MU_TESTS}/render/check_draws_read_only.py" "${MU_SOURCE}")
# Each fixture with the offence the check must report for it (a crash would not match).
foreach(case IN ITEMS
        "writable_parameter|takes a writable OBJECT[*] o"
        "const_pointer_parameter|takes a writable OBJECT[*] const o"
        "indented_member|RenderThing takes a writable"
        "indented_namespace|RenderThing takes a writable OBJECT[*] o"
        "comment_hides_parameter|takes a writable OBJECT[*] p"
        "writable_local|declares a writable CHARACTER[*] c"
        "auto_pointer|declares a writable auto[*] o"
        "auto_reference|declares a writable auto& c"
        "copy_reaches_owner|reaches Owner through its copy copy"
        "copy_owner_address|declares a writable OBJECT[*] owner = &copy.Owner"
        "copy_out_of_scope|declares a writable OBJECT[*] p = &live"
        "comment_brace|declares a writable OBJECT[*] o = &Objects"
        "comment_value|declares a writable OBJECT[*] o = &global"
        "auto_const_pointer|declares a writable const auto owner = Hero"
        "auto_pointer_const|declares a writable auto[*] const owner = Hero"
        "alias_reaches_owner|reaches Owner through its copy alias"
        "extern_object|declares a writable OBJECT[*] pointer = &live"
        "wide_character_brace|declares a writable OBJECT[*] pointer = &Objects"
        "continued_comment|declares a writable OBJECT[*] pointer = &Objects"
        "attributed_function|RenderThing takes a writable OBJECT[*] object"
        "attributed_member|DrawThing takes a writable CHARACTER[*] character"
        "split_return_type|RenderThing takes a writable OBJECT[*] object"
        "split_return_namespace|DescribeThing takes a writable PARTICLE[*] particle")
    string(REPLACE "|" ";" case "${case}")
    list(GET case 0 fixture)
    list(GET case 1 offence)
    add_test(NAME draws_read_only_check_catches_${fixture}
        COMMAND "${Python3_EXECUTABLE}" "${MU_TESTS}/render/check_draws_read_only.py"
                "${MU_DRAW_WRITES_FIXTURES}/${fixture}")
    set_tests_properties(draws_read_only_check_catches_${fixture} PROPERTIES PASS_REGULAR_EXPRESSION "${offence}")
endforeach()
add_test(NAME draws_read_only_check_passes_copies
    COMMAND "${Python3_EXECUTABLE}" "${MU_TESTS}/render/check_draws_read_only.py"
            "${MU_DRAW_WRITES_FIXTURES}/clean")

# UI documents and window contracts.
add_test(
    NAME rml_contract_guard_tests
    COMMAND "${Python3_EXECUTABLE}" "${MU_TESTS}/ui/test_rml_contracts.py"
)
# The mistakes RmlUi only reports at runtime, or not at all, in every shipped document and stylesheet.
add_test(
    NAME rml_rcss_syntax
    COMMAND "${Python3_EXECUTABLE}" "${MU_REPO_ROOT}/tools/check_rml_rcss_syntax.py"
)
add_test(
    NAME rml_preview
    COMMAND "${Python3_EXECUTABLE}" "${MU_TESTS}/ui/test_rml_preview.py" "${MU_REPO_ROOT}"
)
add_test(
    NAME no_legacy_text_measurement
    COMMAND "${CMAKE_COMMAND}"
        "-DSOURCE_DIR=${MU_SOURCE}"
        -P "${MU_TESTS}/ui/test_no_legacy_text_measurement.cmake"
)
add_test(
    NAME inventory_window_captures_hover
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_INVENTORY_SOURCE=${MU_SOURCE}/UI/Inventory/MyInventory.cpp"
        -P "${MU_TESTS}/ui/test_inventory_mouse_capture.cmake"
)
add_test(
    NAME display_settings_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_CONFIG_CONSTANTS=${MU_SOURCE}/Data/GameConfig/GameConfigConstants.h"
        "-DMU_CONFIG_HEADER=${MU_SOURCE}/Data/GameConfig/GameConfig.h"
        "-DMU_CONFIG_SOURCE=${MU_SOURCE}/Data/GameConfig/GameConfig.cpp"
        "-DMU_LOOK_SETTING_SOURCE=${MU_SOURCE}/Render/Look/LookPresetSetting.cpp"
        "-DMU_WINMAIN_SOURCE=${MU_SOURCE}/App/Platform/Windows/Winmain.cpp"
        "-DMU_CONSOLE_SOURCE=${MU_SOURCE}/Core/Utilities/Log/muConsoleDebug.cpp"
        -P "${MU_TESTS}/ui/test_display_settings_contract.cmake"
)

get_property(mu_static_tests DIRECTORY PROPERTY TESTS)
set_tests_properties(${mu_static_tests} PROPERTIES LABELS static)
