#!/usr/bin/env python3

import json
import re
import sys
from pathlib import Path


ROOT = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[1]
CI_WORKFLOW = ROOT / ".github/workflows/ci.yml"
DATA_WORKFLOW = ROOT / ".github/workflows/data-assets.yml"
MINGW_WORKFLOW = ROOT / ".github/workflows/windows-build.yml"
LINUX_WORKFLOW = ROOT / ".github/workflows/linux-build.yml"
RELEASE_CONFIG = ROOT / ".releaserc.json"
VCPKG_MANIFEST = ROOT / "vcpkg.json"
CMAKE_PRESETS = ROOT / "CMakePresets.json"
RENDERER_SOURCE = ROOT / "src/source/Render/Renderer/MuRendererSDLGpu.cpp"
PERSONAL_SHOP_SOURCE = ROOT / "src/source/GameLogic/Items/PersonalShopTitleImp.cpp"
SHADER_BLOB_VALIDATOR = ROOT / "cmake/ValidateShaderBlobs.cmake"
COPY_RUNTIME_ASSETS = ROOT / "cmake/CopyRuntimeAssets.cmake"
SRC_CMAKE = ROOT / "src/CMakeLists.txt"
NETWORK_TEST_CMAKE = ROOT / "tests/network/CMakeLists.txt"
BUILD_GUIDE = ROOT / "docs/build/README.md"
README = ROOT / "README.md"
CHAT_COMMAND_CATALOG_HEADER = ROOT / "src/source/GameLogic/Commands/ChatCommandCatalog.h"
CHAT_COMMAND_CATALOG_SOURCE = ROOT / "src/source/GameLogic/Commands/ChatCommandCatalog.cpp"
MULTI_LANGUAGE_HEADER = ROOT / "src/source/Data/Translation/MultiLanguage.h"
SCRIPT_MODE_TESTS = (
    ROOT / "tests/core/test_msvc_runtime_dll_staging.cmake",
    ROOT / "tests/editor/test_leak.cmake",
    ROOT / "tests/render/test_imgui_sdlgpu_backend.cmake",
    ROOT / "tests/render/test_pipeline_fail_fast.cmake",
)
MINGW_TOOLCHAINS = (
    ROOT / "cmake/toolchains/mingw-w64-i686.cmake",
    ROOT / "cmake/toolchains/mingw-w64-x86_64.cmake",
)

EXPECTED_SHADERS = {
    "basic_colored.frag.dxil",
    "basic_colored.frag.msl",
    "basic_colored.frag.spv",
    "basic_colored.vert.dxil",
    "basic_colored.vert.msl",
    "basic_colored.vert.spv",
    "basic_textured.frag.dxil",
    "basic_textured.frag.msl",
    "basic_textured.frag.spv",
    "basic_textured.vert.dxil",
    "basic_textured.vert.msl",
    "basic_textured.vert.spv",
    "shadow_volume.vert.dxil",
    "shadow_volume.vert.msl",
    "shadow_volume.vert.spv",
    "skinned_textured.vert.dxil",
    "skinned_textured.vert.msl",
    "skinned_textured.vert.spv",
}

EXPECTED_WINDOWS_OS_DLLS = {
    "advapi32.dll",
    "avrt.dll",
    "bcrypt.dll",
    "cfgmgr32.dll",
    "combase.dll",
    "comctl32.dll",
    "comdlg32.dll",
    "crypt32.dll",
    "d3d12.dll",
    "dbghelp.dll",
    "dinput8.dll",
    "dnsapi.dll",
    "dwmapi.dll",
    "dxcore.dll",
    "dxgi.dll",
    "dxguid.dll",
    "gdi32.dll",
    "glu32.dll",
    "hid.dll",
    "imagehlp.dll",
    "imm32.dll",
    "iphlpapi.dll",
    "kernel32.dll",
    "mf.dll",
    "mfplat.dll",
    "mfreadwrite.dll",
    "mfsensorgroup.dll",
    "msvcrt.dll",
    "normaliz.dll",
    "ntdll.dll",
    "ole32.dll",
    "oleaut32.dll",
    "opengl32.dll",
    "powrprof.dll",
    "propsys.dll",
    "rpcrt4.dll",
    "sechost.dll",
    "secur32.dll",
    "setupapi.dll",
    "shell32.dll",
    "shcore.dll",
    "shlwapi.dll",
    "ucrtbase.dll",
    "uiautomationcore.dll",
    "urlmon.dll",
    "user32.dll",
    "usp10.dll",
    "uuid.dll",
    "version.dll",
    "vfw32.dll",
    "wininet.dll",
    "winmm.dll",
    "wintrust.dll",
    "wldap32.dll",
    "ws2_32.dll",
    "wtsapi32.dll",
}

EXPECTED_RELEASE_ASSETS = [
    {
        "path": "MuMain-windows-native-x64-release-editor-off-no-data.tar.gz",
        "name": "MuMain-windows-native-x64-release-editor-off-no-data.tar.gz",
        "label": "MuMain (Windows native x64 Release, editor OFF; requires separate data release)",
    },
    {
        "path": "MuMain-linux-native-x64-release-editor-off-no-data.tar.gz",
        "name": "MuMain-linux-native-x64-release-editor-off-no-data.tar.gz",
        "label": "MuMain (Linux native x64 Release, editor OFF; requires separate data release)",
    },
    {
        "path": "MuMain-macos-native-arm64-release-editor-off-no-data.tar.gz",
        "name": "MuMain-macos-native-arm64-release-editor-off-no-data.tar.gz",
        "label": "MuMain (macOS native arm64 Release, editor OFF; requires separate data release)",
    },
]

errors = []


def check(condition, message):
    if not condition:
        errors.append(message)


def read_optional(path):
    return path.read_text(encoding="utf-8") if path.exists() else ""


def job(workflow, name):
    matches = re.findall(
        rf"(?ms)^  {re.escape(name)}:\n(.*?)(?=^  [\w-]+:\n|\Z)", workflow
    )
    check(len(matches) == 1, f"Expected exactly one workflow job: {name}")
    return matches[0] if len(matches) == 1 else ""


def step(job_body, job_name, step_name):
    match = re.search(
        rf"(?ms)^      - name: {re.escape(step_name)}\n(.*?)(?=^      - name: |\Z)",
        job_body,
    )
    check(match is not None, f"Missing {job_name} step: {step_name}")
    return match.group(1) if match else ""


def manifest(block, assignment):
    match = re.search(
        rf"(?ms)^\s*{re.escape(assignment)}\s*=\s*@?\((.*?)^\s*\)", block
    )
    check(match is not None, f"Missing shader or DLL manifest: {assignment}")
    if not match:
        return []
    return re.findall(r'^\s*["\']([^"\']+)["\']\s*$', match.group(1), re.M)


ci = read_optional(CI_WORKFLOW)
data_workflow = read_optional(DATA_WORKFLOW)
release_config = json.loads(RELEASE_CONFIG.read_text(encoding="utf-8"))
vcpkg_manifest = json.loads(VCPKG_MANIFEST.read_text(encoding="utf-8"))
cmake_presets = json.loads(CMAKE_PRESETS.read_text(encoding="utf-8"))
src_cmake = SRC_CMAKE.read_text(encoding="utf-8")
copy_runtime_assets = COPY_RUNTIME_ASSETS.read_text(encoding="utf-8")
build_guide = BUILD_GUIDE.read_text(encoding="utf-8")
readme = README.read_text(encoding="utf-8")

configure_presets = {
    preset["name"]: preset for preset in cmake_presets["configurePresets"]
}
for preset_name, architecture, triplet in (
    ("windows-x86", "x86", "x86-windows"),
    ("windows-x86-mueditor", "x86", "x86-windows"),
    ("windows-x64", "x64", "x64-windows"),
    ("windows-x64-mueditor", "x64", "x64-windows"),
):
    preset = configure_presets[preset_name]
    check(
        preset.get("toolchainFile")
        == "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
        f"{preset_name} must configure through the vcpkg toolchain",
    )
    cache_variables = preset.get("cacheVariables", {})
    check(
        cache_variables.get("VCPKG_CHAINLOAD_TOOLCHAIN_FILE")
        == f"${{sourceDir}}/toolchain-{architecture}.cmake",
        f"{preset_name} must chainload the project {architecture} toolchain",
    )
    check(
        cache_variables.get("VCPKG_TARGET_TRIPLET") == triplet,
        f"{preset_name} must select {triplet}",
    )

check(not MINGW_WORKFLOW.exists(), "Hosted MinGW matrix workflow must be removed")
check(not LINUX_WORKFLOW.exists(), "Hosted Linux matrix workflow must be removed")
check(DATA_WORKFLOW.exists(), "Dedicated data-assets workflow must exist")

jobs_block_match = re.search(r"(?ms)^jobs:\n(.*)$", ci)
jobs_block = jobs_block_match.group(1) if jobs_block_match else ""
job_names = re.findall(r"(?m)^  ([\w-]+):\n", jobs_block)
check(
    job_names
    == ["quality", "build-windows", "build-linux", "build-macos", "release"],
    "CI jobs must be quality, build-windows, build-linux, build-macos, "
    f"release; found {job_names}",
)
for ignored_path in ("src/bin/Data/**", "src/bin/fonts/**"):
    check(
        "paths-ignore:" in ci and ignored_path in ci,
        f"Code CI must ignore data-only pushes: {ignored_path}",
    )

quality_job = job(ci, "quality")
native_job = job(ci, "build-windows")
linux_job = job(ci, "build-linux")
macos_job = job(ci, "build-macos")
release_job = job(ci, "release")

quality_detection = step(quality_job, "quality", "Detect changed C++ files")
check("diff_base=$DIFF_BASE" in quality_detection, "Quality detection must expose its diff base")
quality_formatting = step(quality_job, "quality", "Check formatting")
for required in (
    "git diff --unified=0 --no-color",
    'format_args+=(--lines="${start}:${end}")',
    'clang-format --dry-run --Werror "${format_args[@]}" "$file"',
):
    check(required in quality_formatting, f"Formatting gate missing {required}")

check("strategy:" not in native_job, "Main Windows asset job must not use a matrix")
check("matrix." not in native_job, "Main Windows asset job must use fixed values")
for required in (
    "name: Windows Native Build (x64, Release, editor OFF)",
    "runs-on: windows-latest",
    "if: github.event_name == 'push' && github.ref == 'refs/heads/main'",
):
    check(required in native_job, f"Main Windows job missing {required}")

activation = step(native_job, "build-windows", "Activate Visual Studio developer environment")
check("arch: x64" in activation, "Main Windows build must activate the x64 compiler")
environment_assertion = step(
    native_job, "build-windows", "Assert MSVC developer environment"
)
check(
    environment_assertion.rstrip().endswith("exit 0"),
    "MSVC identity probes must not leak their external command exit code",
)

native_configure = step(native_job, "build-windows", "Configure CMake")
for required in (
    "cmake --preset windows-x64",
    "-B out/build/windows-ci-x64-release",
    "-DVCPKG_TARGET_TRIPLET=x64-windows",
    "-DVCPKG_INSTALLED_DIR=C:/vcpkg/installed",
    "-DMU_ENABLE_SHADER_COMPILATION=ON",
    "-DGLSLANG_EXE=C:/vcpkg/installed/x64-windows/tools/glslang/glslangValidator.exe",
    "-DSPIRV_CROSS_EXE=C:/vcpkg/installed/x64-windows/tools/spirv-cross/spirv-cross.exe",
    "-DDXC_EXE=C:/vcpkg/installed/x64-windows/tools/directx-dxc/dxc.exe",
    "-DMU_COPY_RUNTIME_ASSETS=OFF",
    "-DBUILD_TESTING=ON",
):
    check(required in native_configure, f"Main Windows configure missing {required}")

native_build = step(native_job, "build-windows", "Build")
check(
    "cmake --build out/build/windows-ci-x64-release --config Release" in native_build,
    "Main Windows build must compile the fixed Release directory",
)
native_tests = step(native_job, "build-windows", "Run tests")
check(
    "ctest --test-dir out/build/windows-ci-x64-release --build-config Release"
    in native_tests,
    "Main Windows build must run Release tests",
)

native_validation = step(native_job, "build-windows", "Validate artifacts")
for required in (
    '"$runtimeDirectory/Main.exe"',
    '"$runtimeDirectory/MUnique.Client.Library.dll"',
    '"$runtimeDirectory/config.ini"',
    '"$runtimeDirectory/shaders"',
    '"$runtimeDirectory/Data"',
    '"$runtimeDirectory/fonts"',
    "Unexpected bundled game data",
    "8664 machine (x64)",
    "dumpbin.exe",
    "/DEPENDENTS $binary",
    "VC runtime import must be staged root-local",
):
    check(required in native_validation, f"Main Windows validation missing {required}")
check("14C machine" not in native_validation, "Main Windows validation must not retain x86 logic")
native_shaders = manifest(native_validation, "$expectedShaders")
check(
    len(native_shaders) == 18 and set(native_shaders) == EXPECTED_SHADERS,
    "Main Windows shader manifest must equal the reviewed 18-file set",
)
windows_os_dlls = manifest(native_validation, "$windowsOsDlls")
check(
    len(windows_os_dlls) == len(EXPECTED_WINDOWS_OS_DLLS)
    and set(windows_os_dlls) == EXPECTED_WINDOWS_OS_DLLS,
    "Main Windows OS DLL allowlist must equal the reviewed set",
)

native_upload = step(native_job, "build-windows", "Upload runtime artifact")
native_archive = step(native_job, "build-windows", "Archive runtime")
check(
    "tar -czf MuMain-windows-native-x64-release-editor-off-no-data.tar.gz "
    "-C out/build/windows-ci-x64-release/src/Release ."
    in native_archive,
    "Main Windows build must archive the runtime before artifact upload",
)
for required in (
    "uses: actions/upload-artifact@v4",
    "name: mu-client-windows-native-x64-release-editor-off-no-data-main",
    "path: MuMain-windows-native-x64-release-editor-off-no-data.tar.gz",
    "if-no-files-found: error",
):
    check(required in native_upload, f"Main Windows upload missing {required}")
check(
    native_job.count("uses: actions/upload-artifact@v4") == 1,
    "Main Windows job must upload exactly one runtime artifact",
)

for (
    platform_job,
    job_name,
    display_name,
    runner,
    artifact_name,
    archive_name,
    archive_command,
) in (
    (
        linux_job,
        "build-linux",
        "Linux Native Build (x64, Release, editor OFF)",
        "ubuntu-latest",
        "mu-client-linux-native-x64-release-editor-off-no-data-main",
        "MuMain-linux-native-x64-release-editor-off-no-data.tar.gz",
        "tar -czf MuMain-linux-native-x64-release-editor-off-no-data.tar.gz "
        "-C out/build/linux-ci/src/Release .",
    ),
    (
        macos_job,
        "build-macos",
        "macOS Native Build (arm64, Release, editor OFF)",
        "macos-latest",
        "mu-client-macos-native-arm64-release-editor-off-no-data-main",
        "MuMain-macos-native-arm64-release-editor-off-no-data.tar.gz",
        "tar -czf MuMain-macos-native-arm64-release-editor-off-no-data.tar.gz "
        "-C out/build/macos-ci/src/Release Main.app",
    ),
):
    check("strategy:" not in platform_job, f"{display_name} must not use a matrix")
    check("matrix." not in platform_job, f"{display_name} must use fixed values")
    for required in (
        f"name: {display_name}",
        f"runs-on: {runner}",
        "if: github.event_name == 'pull_request' || github.ref == 'refs/heads/main'",
    ):
        check(required in platform_job, f"{display_name} missing {required}")
    platform_archive = step(platform_job, job_name, "Archive runtime")
    check(
        "if: github.event_name == 'push'" in platform_archive,
        f"{display_name} must archive only for artifact-producing pushes",
    )
    check(
        archive_command in platform_archive,
        f"{display_name} must archive the runtime before artifact upload",
    )
    platform_upload = step(platform_job, job_name, "Upload runtime artifact")
    for required in (
        "if: github.event_name == 'push'",
        "uses: actions/upload-artifact@v4",
        f"name: {artifact_name}",
        f"path: {archive_name}",
        "if-no-files-found: error",
    ):
        check(required in platform_upload, f"{display_name} upload missing {required}")
    check(
        platform_job.count("uses: actions/upload-artifact@v4") == 1,
        f"{display_name} must upload exactly one runtime artifact",
    )

linux_configure = step(linux_job, "build-linux", "Configure CMake")
for required in (
    "cmake --preset linux-x64",
    "-B out/build/linux-ci",
    "-DENABLE_EDITOR=OFF",
    "-DMU_COPY_RUNTIME_ASSETS=OFF",
    "-DBUILD_TESTING=ON",
):
    check(required in linux_configure, f"Linux configure missing {required}")
check(
    "cmake --build out/build/linux-ci --config Release"
    in step(linux_job, "build-linux", "Build"),
    "Linux check must build Release",
)
check(
    "ctest --test-dir out/build/linux-ci --build-config Release"
    in step(linux_job, "build-linux", "Run tests"),
    "Linux check must run Release tests",
)
linux_validation = step(linux_job, "build-linux", "Validate runtime")
for required in (
    'runtime_directory="out/build/linux-ci/src/Release"',
    '"$runtime_directory/Main"',
    '"$runtime_directory/MUnique.Client.Library.so"',
    '"$runtime_directory/config.ini"',
    '"$runtime_directory/shaders"',
):
    check(required in linux_validation, f"Linux validation missing {required}")

macos_configure = step(macos_job, "build-macos", "Configure CMake")
for required in (
    "cmake --preset macos-arm64",
    "-B out/build/macos-ci",
    "-DENABLE_EDITOR=OFF",
    "-DMU_COPY_RUNTIME_ASSETS=OFF",
    "-DBUILD_TESTING=ON",
    "-DOPENSSL_ROOT_DIR=$(brew --prefix openssl@3)",
):
    check(required in macos_configure, f"macOS configure missing {required}")
check(
    "cmake --build out/build/macos-ci --config Release"
    in step(macos_job, "build-macos", "Build"),
    "macOS check must build Release",
)
check(
    "ctest --test-dir out/build/macos-ci --build-config Release"
    in step(macos_job, "build-macos", "Run tests"),
    "macOS check must run Release tests",
)
macos_validation = step(macos_job, "build-macos", "Validate runtime")
for required in (
    'runtime_directory="out/build/macos-ci/src/Release/Main.app/Contents/MacOS"',
    '"$runtime_directory/Main"',
    '"$runtime_directory/MUnique.Client.Library.dylib"',
    '"$runtime_directory/config.ini"',
    '"$runtime_directory/shaders"',
):
    check(required in macos_validation, f"macOS validation missing {required}")

check(
    "needs: [quality, build-windows, build-linux, build-macos]" in release_job,
    "Release must need quality and all hosted platform checks",
)
for platform, artifact_name in (
    ("Windows", "mu-client-windows-native-x64-release-editor-off-no-data-main"),
    ("Linux", "mu-client-linux-native-x64-release-editor-off-no-data-main"),
    ("macOS", "mu-client-macos-native-arm64-release-editor-off-no-data-main"),
):
    release_download = step(
        release_job, "release", f"Download {platform} runtime archive"
    )
    for required in (f"name: {artifact_name}", "path: ."):
        check(required in release_download, f"Release download missing {required}")
check("tar -czf " not in release_job, "Release must consume runner-created archives")

data_release = step(release_job, "release", "Resolve compatible data release")
for required in (
    "git rev-parse HEAD:src/bin/Data",
    "git rev-parse HEAD:src/bin/fonts",
    "sha256sum",
    'data_tag="data-${asset_id}"',
    'gh release view "$data_tag" --json url --jq .url',
    'echo "DATA_ASSET_ID=${asset_id}"',
    'echo "DATA_RELEASE_URL=${data_release_url}"',
):
    check(required in data_release, f"Compatible data lookup missing {required}")
release_semantic = step(release_job, "release", "Semantic Release")
check(
    "uses: cycjimmy/semantic-release-action@v4" in release_semantic,
    "Release must use Semantic Release",
)
check("id: semantic" in release_semantic, "Semantic Release must expose action outputs")
data_link = step(release_job, "release", "Link compatible data release")
for required in (
    "if: steps.semantic.outputs.new_release_published == 'true'",
    "RELEASE_TAG: ${{ steps.semantic.outputs.new_release_git_tag }}",
    'gh release view "$RELEASE_TAG" --json body --jq .body',
    "## Compatible game data",
    "DATA_ASSET_ID",
    "DATA_RELEASE_URL",
    'gh release edit "$RELEASE_TAG" --notes-file "$release_notes"',
):
    check(required in data_link, f"Compatible data link step missing {required}")

github_plugins = [
    plugin
    for plugin in release_config.get("plugins", [])
    if isinstance(plugin, list) and plugin and plugin[0] == "@semantic-release/github"
]
check(len(github_plugins) == 1, "Release config must contain one GitHub plugin")
github_options = {}
release_assets = []
if len(github_plugins) == 1 and len(github_plugins[0]) == 2:
    options = github_plugins[0][1]
    if isinstance(options, dict):
        github_options = options
        release_assets = options.get("assets", [])
check(
    release_assets == EXPECTED_RELEASE_ASSETS,
    "Semantic Release must publish every hosted no-data runtime",
)
check(
    github_options.get("successCommentCondition") is False,
    "Semantic Release must skip fork-incompatible success comments",
)

for required in (
    "workflow_dispatch:",
    "src/bin/Data/**",
    "src/bin/fonts/**",
    ".github/workflows/data-assets.yml",
    "contents: write",
    "git rev-parse HEAD:src/bin/Data",
    "git rev-parse HEAD:src/bin/fonts",
    "sha256sum",
    "MuMain-data-${asset_id}.tar.gz",
    "data-${asset_id}",
    "tar -czf",
    "-C src/bin Data fonts",
    "gh release view",
    "gh release create",
    "--latest=false",
    "gh release upload",
    "--clobber",
):
    check(required in data_workflow, f"Data release workflow missing {required}")
check("matrix." not in data_workflow, "Data release workflow must not use a matrix")
check("strategy:" not in data_workflow, "Data release workflow must remain a single job")

for required in (
    'option(MU_COPY_RUNTIME_ASSETS "Copy Data and fonts beside Main" ON)',
    'set(ASSETS_MODE "with-data")',
    'set(ASSETS_MODE "no-data")',
    'set(ASSETS_MODE_FILE "${CMAKE_CURRENT_BINARY_DIR}/.assets_mode")',
    'file(GENERATE OUTPUT "${ASSETS_MODE_FILE}" CONTENT "${ASSETS_MODE}\\n")',
    'set(ASSETS_STAMP "${CMAKE_CURRENT_BINARY_DIR}/.assets_copied_$<CONFIG>.stamp")',
    'DEPENDS ${ASSET_FILES} "${ASSETS_MODE_FILE}"',
    '"-DMU_COPY_RUNTIME_ASSETS=${MU_COPY_RUNTIME_ASSETS}"',
):
    check(required in src_cmake, f"CMake runtime asset option missing {required}")
for required in (
    "if(NOT DEFINED MU_COPY_RUNTIME_ASSETS)",
    'file(REMOVE_RECURSE "${MU_ASSET_OUTPUT}/Data" "${MU_ASSET_OUTPUT}/fonts")',
    'PATTERN "Data" EXCLUDE',
    'PATTERN "fonts" EXCLUDE',
):
    check(required in copy_runtime_assets, f"Runtime asset copy script missing {required}")

for required in (
    "Windows native x64 Release",
    "Linux x64 Release",
    "MuMain-linux-native-x64-release-editor-off-no-data.tar.gz",
    "MuMain-macos-native-arm64-release-editor-off-no-data.tar.gz",
    "mu-client-linux-native-x64-release-editor-off-no-data-main",
    "mu-client-macos-native-arm64-release-editor-off-no-data-main",
    "Compatible game data",
    "data-<id>",
    "Main.app/Contents/MacOS",
    "MU_COPY_RUNTIME_ASSETS=OFF",
    "build locally",
):
    check(required in build_guide, f"Build guide missing distribution policy: {required}")
for required in (
    "Windows native x64 Release",
    "Linux x64 Release",
    "macOS arm64 Release",
    "Semantic Release publishes all three",
    "Compatible game data",
    "data release tagged",
    "platform-specific assembly steps",
    "build locally",
):
    check(required in readme, f"README missing release policy: {required}")

shader_blob_validator = SHADER_BLOB_VALIDATOR.read_text(encoding="utf-8")
check(
    shader_blob_validator.startswith("cmake_minimum_required(VERSION 3.25)\n"),
    "Shader blob validation must declare CMake 3.25 policies",
)
for script_path in SCRIPT_MODE_TESTS:
    check(
        script_path.read_text(encoding="utf-8").startswith("cmake_minimum_required(VERSION 3.25)\n"),
        f"Script-mode test must declare CMake 3.25 policies: {script_path.name}",
    )

personal_shop_source = PERSONAL_SHOP_SOURCE.read_text(encoding="utf-8")
check(
    "std::max<int>(1, RenderIconSize.cy)" in personal_shop_source,
    "Personal-shop line height must resolve the Windows LONG/int type",
)
for color_expression in (
    "RGBA(230, 230, 255, 255) : RGBA(230, 230, 0, 255)",
    "RGBA(108, 57, 41, 255) : RGBA(250, 150, 0, 255)",
    "RGBA(250, 150, 0, 128) : RGBA(108, 57, 41, 128)",
):
    check(color_expression in personal_shop_source, "Personal-shop colors must use RGBA")

dependency_names = {
    dependency if isinstance(dependency, str) else dependency.get("name")
    for dependency in vcpkg_manifest.get("dependencies", [])
}
check("directx-dxc" in dependency_names, "vcpkg must install DXC")
check("glslang" in dependency_names, "vcpkg must install glslang")
glslang_dependencies = [
    dependency
    for dependency in vcpkg_manifest.get("dependencies", [])
    if isinstance(dependency, dict) and dependency.get("name") == "glslang"
]
check(
    len(glslang_dependencies) == 1
    and glslang_dependencies[0].get("features") == ["tools"],
    "vcpkg must install the glslang command-line tools",
)

renderer_source = RENDERER_SOURCE.read_text(encoding="utf-8")
check(
    "// Uniform buffers: b0, space1 (Transform)" in renderer_source,
    "basic_textured.vert comment must document its Transform binding",
)
check(
    "// Uniform buffers: b1 (ScreenSize)" not in renderer_source,
    "basic_textured.vert comment must not retain the stale ScreenSize binding",
)

for source_path in (
    CHAT_COMMAND_CATALOG_HEADER,
    CHAT_COMMAND_CATALOG_SOURCE,
    MULTI_LANGUAGE_HEADER,
):
    check(
        "#include <cstdint>" in source_path.read_text(encoding="utf-8"),
        f"Windows integer declarations must include <cstdint>: {source_path}",
    )

check(
    re.search(
        r'(?ms)COMMAND \$\{CMAKE_COMMAND\} -E env\n\s*--unset=Platform\n\s*"NUGET_PACKAGES=.*?"\$\{DOTNET_EXECUTABLE\}" run --project "\$\{RESXGEN_PROJ_NATIVE\}"',
        src_cmake,
    )
    is not None,
    "ResxGen must discard the MSVC target Platform",
)
network_test_cmake = NETWORK_TEST_CMAKE.read_text(encoding="utf-8")
check(
    re.search(
        r'(?ms)NAME outbound_flush_loop\n\s*COMMAND "\$\{CMAKE_COMMAND\}" -E env --unset=Platform\n\s*"\$\{DOTNET_EXECUTABLE\}" run',
        network_test_cmake,
    )
    is not None,
    "The .NET network self-check must discard the MSVC target Platform",
)

for toolchain_path in MINGW_TOOLCHAINS:
    toolchain = toolchain_path.read_text(encoding="utf-8")
    check(
        "MINGW_ADDITIONAL_TARGET_ROOT" not in toolchain,
        f"MinGW toolchain must not retain the obsolete find root: {toolchain_path.name}",
    )
    check(
        "set(CMAKE_FIND_ROOT_PATH " not in toolchain
        and "list(APPEND CMAKE_FIND_ROOT_PATH /usr/" in toolchain
        and "list(REMOVE_DUPLICATES CMAKE_FIND_ROOT_PATH)" in toolchain,
        f"MinGW toolchain must preserve package roots: {toolchain_path.name}",
    )

if errors:
    raise SystemExit("Windows workflow contract failed:\n- " + "\n- ".join(errors))

print("Windows workflow contract: OK")
