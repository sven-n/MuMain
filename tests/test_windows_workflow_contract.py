#!/usr/bin/env python3

import json
import re
import sys
from pathlib import Path


ROOT = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[1]
CI_WORKFLOW = ROOT / ".github/workflows/ci.yml"
FORK_CI_WORKFLOW = ROOT / ".github/workflows/fork-ci.yml"
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
SOURCE_ROOT = ROOT / "src/source"
NETWORK_TEST_CMAKE = ROOT / "tests/network/CMakeLists.txt"
STATIC_CMAKE = ROOT / "tests/static/CMakeLists.txt"
PRE_PUSH_HOOK = ROOT / ".githooks/pre-push"
FORMAT_SCRIPT = ROOT / "scripts/check_format.sh"
BUILD_GUIDE = ROOT / "docs/build/README.md"
LINUX_GUIDE = ROOT / "docs/build/linux/console.md"
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
# The data release id, computed alike by data-assets.yml and the release job. cmake/WriteGitCommit.cmake computes it
# once more for MuError.log, and tests/test_write_git_commit.cmake checks that against this formula's result: a
# change here needs both.
DATA_ASSET_ID_FORMULA = (
    "data_tree=$(git rev-parse HEAD:src/bin/Data)",
    "fonts_tree=$(git rev-parse HEAD:src/bin/fonts)",
    "asset_id=$(printf '%s\\n%s\\n' \"$data_tree\" \"$fonts_tree\" | sha256sum | cut -c1-16)",
)

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

# Packages the Linux CI job installs that the Linux guide leaves out, and why.
CI_ONLY_LINUX_PACKAGES = {
    "gcc": "the guide installs build-essential",
    "g++": "the guide installs build-essential",
    "ccache": "CI-only compiler cache trial",
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


def job_env(job_body, name):
    """A job-level env value; a folded block (>-) is joined into one line, as YAML does."""
    match = re.search(rf"(?m)^      {re.escape(name)}: (?:>-\n((?:^        .*\n)+)|(.*)$)", job_body)
    if match is None:
        return ""
    return " ".join(match.group(1).split()) if match.group(1) is not None else match.group(2).strip()


def restore_prefixes(text):
    """The key prefixes of the restore-keys inputs in text, one-line or multi-line (|)."""
    prefixes = re.findall(r"(?m)^          restore-keys: (?!\|)(.+)$", text)
    for block in re.findall(r"(?m)^          restore-keys: \|\n((?:^            .*\n)+)", text):
        prefixes += [line.strip() for line in block.splitlines() if line.strip()]
    return prefixes


def cache_paths(step_body):
    """The lines of an actions/cache step's multi-line path input, or its one-line path."""
    match = re.search(r"(?ms)^          path: \|\n(.*?)(?=^          \w)", step_body)
    if match:
        return match.group(1)
    match = re.search(r"(?m)^          path: (?!\|)(.+)$", step_body)
    return match.group(1) if match else ""


def manifest(block, assignment):
    match = re.search(
        rf"(?ms)^\s*{re.escape(assignment)}\s*=\s*@?\((.*?)^\s*\)", block
    )
    check(match is not None, f"Missing shader or DLL manifest: {assignment}")
    if not match:
        return []
    return re.findall(r'^\s*["\']([^"\']+)["\']\s*$', match.group(1), re.M)


def apt_packages(text):
    # Every package of every "apt-get install -y" command, including its "\" continuation
    # lines. The command ends at a shell operator or a comment; an inline `command` in
    # prose ends at its closing backtick.
    packages = set()
    for arguments in re.findall(r"apt-get install -y((?:[^\n`]*\\\n)*[^\n`]*)", text):
        command = re.split(r"&&|\|\||[;|#]", arguments.replace("\\\n", " "), maxsplit=1)[0]
        packages.update(word for word in command.split() if not word.startswith("-"))
    return packages


ci = read_optional(CI_WORKFLOW)
data_workflow = read_optional(DATA_WORKFLOW)
release_config = json.loads(RELEASE_CONFIG.read_text(encoding="utf-8"))
vcpkg_manifest = json.loads(VCPKG_MANIFEST.read_text(encoding="utf-8"))
cmake_presets = json.loads(CMAKE_PRESETS.read_text(encoding="utf-8"))
src_cmake = SRC_CMAKE.read_text(encoding="utf-8")
copy_runtime_assets = COPY_RUNTIME_ASSETS.read_text(encoding="utf-8")
build_guide = BUILD_GUIDE.read_text(encoding="utf-8")
readme = README.read_text(encoding="utf-8")

for source_path in SOURCE_ROOT.rglob("*"):
    if source_path.suffix.lower() not in {".c", ".cc", ".cpp", ".h", ".hh", ".hpp"}:
        continue
    try:
        source_path.read_text(encoding="utf-8", errors="strict")
    except UnicodeDecodeError as error:
        errors.append(
            f"C/C++ source must be valid UTF-8: {source_path.relative_to(ROOT)} "
            f"({error})"
        )

check(
    re.search(
        r"(?ms)if\s*\(\s*MSVC\s*\).*?"
        r"target_compile_options\s*\(\s*MuClient\s+PRIVATE\s+/utf-8\s+/we4828\s*\).*?"
        r"endif\s*\(\s*\)",
        src_cmake,
    )
    is not None,
    "MuClient MSVC builds must compile UTF-8 sources and reject invalid bytes",
)

# No CI job builds with MU_LINK_SDL_PLATFORM_BACKENDS=ON, so its Vulkan link is checked here.
check(
    '"LINKER:--push-state,--no-as-needed" "<LINK_ITEM>" "LINKER:--pop-state"' in src_cmake,
    "MuNoAsNeeded must keep its library under --no-as-needed between --push-state and --pop-state",
)
src_cmake_code = "\n".join(
    line for line in src_cmake.splitlines() if not line.lstrip().startswith("#")
)
check(
    src_cmake_code.count("Vulkan::Vulkan") == 1
    and "$<LINK_LIBRARY:MuNoAsNeeded,Vulkan::Vulkan>" in src_cmake_code,
    "Vulkan::Vulkan must be linked once, through $<LINK_LIBRARY:MuNoAsNeeded,...>: a plain link "
    "loses its DT_NEEDED entry under --as-needed, and CMake refuses a mix of both",
)

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
    == ["quality", "build-windows", "build-linux", "build-macos", "caches", "required", "release"],
    "CI jobs must be quality, build-windows, build-linux, build-macos, caches, required, "
    f"release; found {job_names}",
)
check("paths-ignore:" not in ci, "Required CI must report checks even for docs/data-only PRs")
for required in ("branches: [main]", "ready_for_review", "schedule:", "workflow_dispatch:"):
    check(required in ci, f"CI trigger missing {required}")
check(
    "cancel-in-progress: ${{ github.event_name == 'pull_request' }}" in ci,
    "Only a PR's newer run may cancel its older one: main pushes save the caches PRs start from",
)

quality_job = job(ci, "quality")
native_job = job(ci, "build-windows")
linux_job = job(ci, "build-linux")
macos_job = job(ci, "build-macos")
release_job = job(ci, "release")
required_job = job(ci, "required")

quality_history = step(quality_job, "quality", "Fetch history")
check(
    "--filter=blob:none --unshallow --tags" in quality_history,
    "Quality Gates must fetch full commit history without past file versions",
)
check(
    "fetch-depth: 2" in step(quality_job, "quality", "Checkout repository")
    and "if: github.event_name != 'pull_request'" in quality_history,
    "Pull requests diff against HEAD^1 and need no history beyond it",
)
quality_detection = step(quality_job, "quality", "Detect changed C++ files")
check(
    "DIFF_BASE: ${{ steps.plan.outputs.diff_base }}" in quality_detection,
    "Quality detection must use the event's pinned diff base",
)
check("bash scripts/changed_cpp_files.sh" in quality_detection, "Quality detection must share the hook's file list")
# A main push compares against the commit main's last successful run built, which the plan looks up.
quality_plan = step(quality_job, "quality", "Select native builds")
for required in ("GH_TOKEN: ${{ github.token }}", "python3 scripts/ci_build_plan.py"):
    check(required in quality_plan, f"Quality Gates must look up main's last successful run: {required}")
check("actions: read" in quality_job, "Quality Gates must be able to read main's runs")
quality_static = step(quality_job, "quality", "Run static checks")
for required in ("cmake -S tests/static -B out/static-checks", "ctest --test-dir out/static-checks"):
    check(required in quality_static, f"Static checks must run before native builds: {required}")
static_cmake = STATIC_CMAKE.read_text(encoding="utf-8")
for test_name in ("windows_workflow_contract", "ci_build_plan"):
    check(f"NAME {test_name}" in static_cmake, f"Static checks must include {test_name}")
check("PROPERTIES LABELS static" in static_cmake, "Static checks must carry the label native jobs exclude")
pre_push = PRE_PUSH_HOOK.read_text(encoding="utf-8")
for required in ("-S tests/static -B out/static-checks", "scripts/changed_cpp_files.sh", "scripts/check_format.sh"):
    check(required in pre_push, f"pre-push hook must run what Quality Gates runs: {required}")
for platform, platform_job in (("windows", native_job), ("linux", linux_job), ("macos", macos_job)):
    check("needs: quality" in platform_job, f"{platform} must wait for Quality Gates")
    check(
        "--label-exclude static" in step(platform_job, platform, "Run tests"),
        f"{platform} must not repeat the static checks Quality Gates already ran",
    )
    # Linux and Windows runners have four cores, macOS three; tests that share a file name a RESOURCE_LOCK
    # (tests/CMakeLists.txt).
    check("--parallel 4" in step(platform_job, platform, "Run tests"), f"{platform} must run its tests in parallel")
    check(
        f"if: needs.quality.outputs.{platform} == 'true'" in platform_job,
        f"{platform} must follow the tested build plan",
    )
    timeout = "timeout-minutes: 120" if platform == "windows" else "timeout-minutes: 90"
    check(timeout in platform_job, f"{platform} must bound job duration: {timeout}")
for required in ("name: CI Required", "if: always()",
                 "needs: [quality, build-windows, build-linux, build-macos, caches]",
                 "CI_JOB_RESULTS: ${{ toJSON(needs) }}", "python3 scripts/ci_build_plan.py check"):
    check(required in required_job, f"Required CI check missing {required}")
# No warning may come back in the player builds (docs/build/README.md).
for required in ("checks: read", "actions: read", "python3 scripts/ci_job_annotations.py"):
    check(required in required_job, f"Required CI check must fail on warning annotations: {required}")
for platform, platform_job in (("windows", native_job), ("linux", linux_job), ("macos", macos_job)):
    platform_configure = step(platform_job, f"build-{platform}", "Configure CMake")
    platform_build = step(platform_job, f"build-{platform}", "Build")
    for required in ("scripts/ci_warnings.py run", "-DMU_WARNINGS_AS_ERRORS=ON"):
        check(required in platform_configure, f"{platform} configure must fail on warnings: {required}")
    check("scripts/ci_warnings.py run" in platform_build, f"{platform} build must fail on warnings")
# Every job names its runner image: a -latest label moves to a new image whenever GitHub decides, with new
# warnings and, for macOS, a new minimum macOS (cmake/toolchains/macos-arm64.cmake).
for workflow_name, workflow in (("ci.yml", ci), ("fork-ci.yml", read_optional(FORK_CI_WORKFLOW))):
    for runner in re.findall(r"(?m)^\s*runs-on:\s*(\S+)", workflow):
        check(not runner.endswith("-latest"), f"{workflow_name} must name its runner image, not {runner}")
# Fork CI runs only by hand and builds only the editor: ci.yml builds the game on every system for every pull request,
# so a pull request starting Fork CI, or Fork CI building the game, would build it twice. Its last job checks the
# annotations.
fork_ci_workflow = read_optional(FORK_CI_WORKFLOW)
if fork_ci_workflow:
    fork_triggers = re.search(r"(?ms)^on:\n(.*?)^\S", fork_ci_workflow)
    check(fork_triggers is not None and re.findall(r"(?m)^  (\w+):", fork_triggers.group(1)) == ["workflow_dispatch"],
          "Fork CI must run only when started by hand")
    for fork_job in ("windows-editor", "linux-editor", "macos-editor"):
        check("-DENABLE_EDITOR=ON" in step(job(fork_ci_workflow, fork_job), fork_job, "Configure CMake"),
              f"Fork CI's {fork_job} must build the editor")
    check("-DENABLE_EDITOR=OFF" not in fork_ci_workflow and "matrix." not in fork_ci_workflow,
          "Fork CI must not build the game without the editor, which ci.yml builds")
    fork_annotations = job(fork_ci_workflow, "annotations")
    for required in ("needs: [windows-editor, linux-editor, macos-editor]", "if: always()", "actions: read",
                     "checks: read", "python3 scripts/ci_job_annotations.py"):
        check(required in fork_annotations, f"Fork CI must fail on warning annotations: {required}")
check("if: needs.quality.outputs.release == 'true'" in release_job, "Publishing must follow the release plan")
quality_formatting = step(quality_job, "quality", "Check formatting")
check("bash scripts/check_format.sh" in quality_formatting, "Formatting gate must share the hook's script")
format_script = FORMAT_SCRIPT.read_text(encoding="utf-8")
for required in (
    "git diff --unified=0 --no-color",
    'format_args+=(--lines="${start}:${end}")',
    'clang-format --dry-run --Werror "${format_args[@]}" "$file"',
):
    check(required in format_script, f"Formatting gate missing {required}")

check("strategy:" not in native_job, "Main Windows asset job must not use a matrix")
check("matrix." not in native_job, "Main Windows asset job must use fixed values")
for required in (
    "name: Windows Native Build (x64, Release, editor OFF)",
    "runs-on: windows-2025-vs2026",
):
    check(required in native_job, f"Main Windows job missing {required}")

activation = step(native_job, "build-windows", "Activate Visual Studio developer environment")
check("vcvars64.bat" in activation, "Main Windows build must activate the x64 compiler")
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
    "-DCMAKE_DISABLE_PRECOMPILE_HEADERS=${{ needs.quality.outputs.release == 'true' && 'OFF' || 'ON' }}",
    "-DCMAKE_C_COMPILER_LAUNCHER=ccache",
    "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache",
):
    check(required in native_configure, f"Main Windows configure missing {required}")
windows_ccache_install = step(native_job, "build-windows", "Install compiler cache")
for required in ("$version = '", "$sha256 = '", "Get-FileHash", "GITHUB_PATH", "-MaximumRetryCount"):
    check(required in windows_ccache_install, f"Windows must install a pinned, checked ccache: {required}")
windows_cache = step(native_job, "build-windows", "Cache Windows compilation")
for required in ("uses: actions/cache/restore@v6", "steps.vcpkg.outputs.compiler", "github.sha", "restore-keys:"):
    check(required in windows_cache, f"Windows compiler cache missing {required}")
# scripts/ci_vcpkg_cache_key.ps1 hashes cl.exe for both caches; the job does not do it a second time.
check("Get-FileHash -LiteralPath $compiler" not in native_job,
      "Windows must name its compiler cache with the vcpkg script's hash")
windows_cache_save = step(native_job, "build-windows", "Save Windows compilation cache")
for required in ("uses: actions/cache/save@v6", "github.event_name == 'push'", "github.ref == 'refs/heads/main'",
                 "steps.ccache.outputs.cache-primary-key"):
    check(required in windows_cache_save, f"Windows compiler cache must save only validated main builds: {required}")
check("ccache --show-stats" in native_job, "Windows compiler cache must report hit statistics")

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
# The shader list lives only in cmake/ValidateShaderBlobs.cmake; a second
# copy in the workflow fell behind when shaders were added.
for required in (
    '"-DMU_SHADER_BLOB_DIR=$shaderDirectory" -P cmake/ValidateShaderBlobs.cmake',
    "Unexpected entries in the runtime shader directory",
):
    check(required in native_validation, f"Main Windows shader validation missing {required}")
check(
    "$expectedShaders" not in native_validation,
    "Main Windows shader validation must take its list from cmake/ValidateShaderBlobs.cmake",
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
# Windows builds every PR and main push now; like Linux and macOS, only the release runs hand their build on.
for windows_step, body in (("Archive runtime", native_archive), ("Upload runtime artifact", native_upload)):
    check("if: needs.quality.outputs.release == 'true'" in body, f"Windows {windows_step} must run only for releases")
for required in (
    "uses: actions/upload-artifact@v7",
    "name: mu-client-windows-native-x64-release-editor-off-no-data-main",
    "path: MuMain-windows-native-x64-release-editor-off-no-data.tar.gz",
    "if-no-files-found: error",
):
    check(required in native_upload, f"Main Windows upload missing {required}")
check(
    native_job.count("uses: actions/upload-artifact@v7") == 1,
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
        "ubuntu-26.04",
        "mu-client-linux-native-x64-release-editor-off-no-data-main",
        "MuMain-linux-native-x64-release-editor-off-no-data.tar.gz",
        "tar -czf MuMain-linux-native-x64-release-editor-off-no-data.tar.gz "
        "-C out/build/linux-ci/src/Release .",
    ),
    (
        macos_job,
        "build-macos",
        "macOS Native Build (arm64, Release, editor OFF)",
        "macos-26",
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
    ):
        check(required in platform_job, f"{display_name} missing {required}")
    platform_archive = step(platform_job, job_name, "Archive runtime")
    check(
        "if: needs.quality.outputs.release == 'true'" in platform_archive,
        f"{display_name} must archive only for scheduled/manual releases",
    )
    check(
        archive_command in platform_archive,
        f"{display_name} must archive the runtime before artifact upload",
    )
    platform_upload = step(platform_job, job_name, "Upload runtime artifact")
    for required in (
        "if: needs.quality.outputs.release == 'true'",
        "uses: actions/upload-artifact@v7",
        f"name: {artifact_name}",
        f"path: {archive_name}",
        "if-no-files-found: error",
    ):
        check(required in platform_upload, f"{display_name} upload missing {required}")
    check(
        platform_job.count("uses: actions/upload-artifact@v7") == 1,
        f"{display_name} must upload exactly one runtime artifact",
    )

linux_configure = step(linux_job, "build-linux", "Configure CMake")
for required in (
    "cmake --preset linux-x64",
    "-B out/build/linux-ci",
    "-DENABLE_EDITOR=OFF",
    "-DMU_COPY_RUNTIME_ASSETS=OFF",
    "-DBUILD_TESTING=ON",
    "-DCMAKE_DISABLE_PRECOMPILE_HEADERS=${{ needs.quality.outputs.release == 'true' && 'OFF' || 'ON' }}",
    "-DCMAKE_C_COMPILER_LAUNCHER=ccache",
    "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache",
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

linux_guide_packages = apt_packages(LINUX_GUIDE.read_text(encoding="utf-8"))
linux_ci_jobs = [("ci.yml", linux_job, "build-linux")]
fork_ci = read_optional(FORK_CI_WORKFLOW)
if fork_ci:
    linux_ci_jobs.append(("fork-ci.yml", job(fork_ci, "linux-editor"), "linux-editor"))
    for windows_job, vcpkg_step in ((native_job, "Restore vcpkg packages"),
                                    (job(fork_ci, "windows-editor"), "Cache vcpkg packages")):
        cache_key = step(windows_job, "windows", "Identify vcpkg toolchain")
        check("scripts/ci_vcpkg_cache_key.ps1" in cache_key, "Windows must identify its actual toolchain")
        cache = step(windows_job, "windows", vcpkg_step)
        for required in ("steps.vcpkg.outputs.identity", "toolchain-x64.cmake", "~/AppData/Local/vcpkg/archives"):
            check(required in cache, f"vcpkg cache missing {required}")
        check(
            "restore-keys: vcpkg-windows-x64-v2-\n" in cache,
            "vcpkg binary archives must remain available across runner image updates",
        )
linux_cache = step(linux_job, "build-linux", "Cache Linux compilation")
for required in ("uses: actions/cache/restore@v6", "steps.compiler.outputs.identity", "github.sha", "restore-keys:"):
    check(required in linux_cache, f"Compiler cache missing {required}")
linux_cache_save = step(linux_job, "build-linux", "Save Linux compilation cache")
for required in ("uses: actions/cache/save@v6", "github.event_name == 'push'", "github.ref == 'refs/heads/main'",
                 "steps.ccache.outputs.cache-primary-key"):
    check(required in linux_cache_save, f"Compiler cache must save only validated main builds: {required}")
check(
    "uses: actions/cache@v6" not in linux_cache,
    "PR compilation must restore without creating isolated cache entries",
)
check("ccache --show-stats" in linux_job, "Compiler cache trial must report hit statistics")
# PR runs read main's caches and store none of their own: a PR's entry would serve that PR alone and push main's
# entries out of the repository's 10 GB. Only main pushes save them: the nightly runs beside the pushes, so with both
# saving, an older commit's entry could be saved last and be the one the prune job keeps.
CACHE_SAVER = "github.event_name == 'push' && github.ref == 'refs/heads/main'"


def check_cache_pair(job_body, job_name, restore_name, save_name):
    """A restore step with an id, and a save that stores the same paths on a miss, under the restored key."""
    restore = step(job_body, job_name, restore_name)
    save = step(job_body, job_name, save_name)
    restore_id = re.search(r"(?m)^        id: (\w+)$", restore)
    check(restore_id is not None and "uses: actions/cache/restore@v6" in restore,
          f"{job_name} {restore_name} must be a restore step with an id")
    if restore_id is None:
        return
    restore_id = restore_id.group(1)
    for required in ("uses: actions/cache/save@v6", f"steps.{restore_id}.outputs.cache-hit != 'true'",
                     f"key: ${{{{ steps.{restore_id}.outputs.cache-primary-key }}}}"):
        check(required in save, f"{job_name} {save_name} must save on a miss, under the restored key: {required}")
    check(cache_paths(restore) != "" and cache_paths(restore) == cache_paths(save),
          f"{job_name} {save_name} must save the paths {restore_name} restores")


def check_apt_cache(job_body, job_name, kind, install_name):
    """An apt cache: named after the image and the files apt would fetch, restored, fetched into apt's archive folder,
    gathered and saved by main pushes (only with every file), and installed from there."""
    find = step(job_body, job_name, "Find apt packages")
    for required in ("id: apt_packages\n", "sudo apt-get update",
                     'apt-get install --print-uris -qq $APT_PACKAGES > "$RUNNER_TEMP/apt-files.txt"',
                     f'python3 scripts/ci_apt_cache.py name {kind} "$RUNNER_TEMP/apt-files.txt"'):
        check(required in find, f"{job_name} Find apt packages missing {required}")
    # This image's newest entry first (the prune keeps one of each image), then any image's. The restore and save
    # themselves are checked with the other caches (check_cache_pair).
    restore = step(job_body, job_name, "Restore apt packages")
    for required in ("id: apt\n", "path: ${{ env.APT_ARCHIVES }}", "key: ${{ steps.apt_packages.outputs.key }}"):
        check(required in restore, f"{job_name} Restore apt packages missing {required}")
    check(restore_prefixes(restore) == [f"{kind}${{{{ steps.apt_packages.outputs.image }}}}-", kind],
          f"{job_name} Restore apt packages must fall back to the image's newest entry, then any image's")
    fetch = step(job_body, job_name, "Fetch apt packages")
    for required in ('sudo cp "$APT_ARCHIVES"/*.deb /var/cache/apt/archives/',
                     "sudo apt-get install -y --download-only $APT_PACKAGES"):
        check(required in fetch, f"{job_name} Fetch apt packages missing {required}")
    install = step(job_body, job_name, install_name)
    check("sudo apt-get install -y $APT_PACKAGES" in install, f"{job_name} {install_name} must install APT_PACKAGES")
    # Only the runs that save gather the files, so the gather has the save's conditions. It runs before the install,
    # which could clean apt's archive folder.
    gather = step(job_body, job_name, "Gather apt packages")
    for required in ("id: gather\n", f"if: {CACHE_SAVER} && steps.apt.outputs.cache-hit != 'true'\n",
                     'python3 scripts/ci_apt_cache.py gather "$RUNNER_TEMP/apt-files.txt" /var/cache/apt/archives '
                     '"$APT_ARCHIVES"'):
        check(required in gather, f"{job_name} Gather apt packages missing {required}")
    save = step(job_body, job_name, "Save apt packages")
    check(f"if: {CACHE_SAVER} && steps." in save and "steps.gather.outputs.complete == 'true'" in save,
          f"{job_name} Save apt packages must save only in main pushes, when every listed file was gathered")
    # scripts/ci_apt_cache.py says complete=true only with every listed file (tests/test_ci_apt_cache.py); a gather or
    # save that went on after an error would save an entry without them.
    check("continue-on-error" not in gather + save, f"{job_name} must not save its apt packages after a failed gather")
    check("sudo apt-get update" not in fetch + install, f"{job_name} must update the apt index before naming the cache")
    names = re.findall(r"(?m)^      - name: (.+)$", job_body)
    order = ["Find apt packages", "Restore apt packages", "Fetch apt packages", "Gather apt packages",
             "Save apt packages", install_name]
    check(all(name in names for name in order)
          and [names.index(name) for name in order] == sorted(names.index(name) for name in order),
          f"{job_name} must find, restore, fetch, gather and save its apt packages, then install them, in that order")


check_apt_cache(linux_job, "build-linux", "apt-linux-x64-", "Install build tools")
# Quality Gates installs cppcheck from its own apt cache; its restore and save are checked here, as the build jobs'
# caches are below.
check_apt_cache(quality_job, "quality", "apt-cppcheck-linux-x64-", "Install quality tools")
check_cache_pair(quality_job, "quality", "Restore apt packages", "Save apt packages")
# Each compiler cache save holds the objects its build used, not every build's since the cache began: the step before
# the save drops the older ones, in the runs that save.
for job_name, job_body, system in (("build-windows", native_job, "Windows"), ("build-linux", linux_job, "Linux"),
                                   ("build-macos", macos_job, "macOS")):
    names = re.findall(r"(?m)^      - name: (.+)$", job_body)
    save_name = f"Save {system} compilation cache"
    check("Trim compiler cache" in names and save_name in names
          and names.index("Trim compiler cache") + 1 == names.index(save_name),
          f"{job_name} must trim its compiler cache right before saving it")
    trim = step(job_body, job_name, "Trim compiler cache")
    for required in (f"if: {CACHE_SAVER} && steps.ccache.outputs.cache-hit != 'true'", "ccache --evict-older-than",
                     "BUILD_STARTED"):
        check(required in trim, f"{job_name} compiler cache trim missing {required}")
    check("BUILD_STARTED=" in step(job_body, job_name, "Build"), f"{job_name} Build must note when it starts")
    # Only a build that passed its tests and checks saves its objects: a step that fails skips the steps after it.
    validate = "Validate artifacts" if job_name == "build-windows" else "Validate runtime"
    for earlier in ("Run tests", validate):
        check(earlier in names and save_name in names and names.index(earlier) < names.index(save_name),
              f"{job_name} must save its compiler cache after {earlier}")
for job_name, job_body in (("build-windows", native_job), ("build-linux", linux_job), ("build-macos", macos_job)):
    check("uses: actions/cache@v6" not in job_body, f"{job_name} must restore and save its caches in separate steps")
    for job_step in re.split(r"(?m)^      - name: ", job_body)[1:]:
        if "uses: actions/cache/save@v6" in job_step:
            check(f"if: {CACHE_SAVER} && steps." in job_step,
                  f"{job_name} {job_step.splitlines()[0]} must save only in main pushes")
    cache_steps = [("Restore dependencies", "Save dependencies")]
    if job_name == "build-windows":
        cache_steps.append(("Restore vcpkg packages", "Save vcpkg packages"))
    if job_name == "build-linux":
        cache_steps.append(("Restore apt packages", "Save apt packages"))
    for restore_name, save_name in cache_steps:
        check_cache_pair(job_body, job_name, restore_name, save_name)
    # .nuget holds the NativeAOT compiler packages of the .NET SDK in use; a new SDK needs a new entry.
    check("id: dotnet" in step(job_body, job_name, "Setup .NET 10")
          and "steps.dotnet.outputs.dotnet-version" in step(job_body, job_name, "Restore dependencies"),
          f"{job_name} must name the library cache after the .NET SDK version")
# A save under a new key leaves older entries of the cache behind; the caches job deletes them, so it has to know each
# cache the build jobs restore by a key prefix.
caches_job = job(ci, "caches")
for required in ("needs: [build-windows, build-linux, build-macos]", "!cancelled()",
                 "github.event_name != 'pull_request'", "actions: write", "python3 scripts/ci_cache_prune.py",
                 "GH_TOKEN: ${{ github.token }}"):
    check(required in caches_job, f"caches job must prune after the builds of runs on main: {required}")
# Its checkout needs only the scripts, not the game data, and must not keep the token that can delete caches.
for required in ("sparse-checkout: scripts", "persist-credentials: false"):
    check(required in step(caches_job, "caches", "Checkout repository"),
          f"caches job must check out only the scripts, without credentials: {required}")
for job_name, job_body in (("build-windows", native_job), ("build-linux", linux_job), ("build-macos", macos_job)):
    check("actions: write" not in job_body, f"{job_name} must not get a token that can delete caches")
# The prune job leaves an entry that a job read in the last minutes, unless the last read was this run's own: each
# build job notes when its restores ended, after the last of them, and passes that on.
for job_name, job_body, system in (("build-windows", native_job, "WINDOWS"), ("build-linux", linux_job, "LINUX"),
                                   ("build-macos", macos_job, "MACOS")):
    names = re.findall(r"(?m)^      - name: (.+)$", job_body)
    restores = [index for index, name in enumerate(names)
                if "uses: actions/cache/restore@v6" in step(job_body, job_name, name)]
    note = names.index("Note when the caches were read") if "Note when the caches were read" in names else -1
    check(restores and note == max(restores) + 1, f"{job_name} must note when its caches were read, after every restore")
    check("caches-read-at: ${{ steps.caches_read.outputs.at }}" in job_body,
          f"{job_name} must pass on when its caches were read")
    check(f"CACHES_READ_AT_{system}: ${{{{ needs.{job_name}.outputs.caches-read-at }}}}" in caches_job,
          f"caches job must know when {job_name} read its caches")
sys.path.insert(0, str(ROOT / "scripts"))
from ci_cache_prune import KINDS as PRUNED_CACHE_KINDS, PER_COMPILER as PRUNED_PER_COMPILER  # noqa: E402
restore_prefixes_by_kind = {}
# Quality Gates' pip cache is not pruned: it is saved once (actions/cache@v6 under a fixed key).
pruned_restores = native_job + linux_job + macos_job + step(quality_job, "quality", "Restore apt packages")
for restore_prefix in restore_prefixes(pruned_restores):
    restore_prefixes_by_kind.setdefault(restore_prefix.split("${{")[0], []).append(restore_prefix)
for kind, kind_prefixes in restore_prefixes_by_kind.items():
    check(kind in PRUNED_CACHE_KINDS, f"scripts/ci_cache_prune.py must prune the cache kind {kind}")
    # A restore prefix that goes on after the kind (the compiler, or the apt cache's runner image) reads entries of
    # that part, so the prune has to keep the newest entry of each.
    check(any("${{" in prefix for prefix in kind_prefixes) == (kind in PRUNED_PER_COMPILER),
          f"scripts/ci_cache_prune.py must keep the newest {kind} entry of each compiler exactly when a restore "
          f"prefix names the compiler: {', '.join(kind_prefixes)}")
all_linux_ci_packages = set()
for workflow_name, ci_job, job_name in linux_ci_jobs:
    # ci.yml names its packages once, in the job's env, for the install and for the cache's key.
    install = step(ci_job, job_name, "Install build tools").replace("$APT_PACKAGES", job_env(ci_job, "APT_PACKAGES"))
    ci_packages = apt_packages(install)
    check(ci_packages, f"{workflow_name} {job_name} must install its packages with apt-get install -y")
    all_linux_ci_packages |= ci_packages
    for package in sorted(ci_packages - linux_guide_packages - CI_ONLY_LINUX_PACKAGES.keys()):
        errors.append(
            f"{workflow_name} {job_name} installs {package}, but "
            f"{LINUX_GUIDE.relative_to(ROOT).as_posix()} doesn't list it; add it to the guide, "
            "or to CI_ONLY_LINUX_PACKAGES with the reason"
        )
for package in sorted(CI_ONLY_LINUX_PACKAGES.keys() - all_linux_ci_packages):
    errors.append(f"CI_ONLY_LINUX_PACKAGES lists {package}, which no Linux CI job installs")

macos_configure = step(macos_job, "build-macos", "Configure CMake")
for required in (
    "cmake --preset macos-arm64",
    "-B out/build/macos-ci",
    "-DENABLE_EDITOR=OFF",
    "-DMU_COPY_RUNTIME_ASSETS=OFF",
    "-DBUILD_TESTING=ON",
    "-DCMAKE_DISABLE_PRECOMPILE_HEADERS=${{ needs.quality.outputs.release == 'true' && 'OFF' || 'ON' }}",
    "-DOPENSSL_ROOT_DIR=$(brew --prefix openssl@3)",
    "-DCMAKE_C_COMPILER_LAUNCHER=ccache",
    "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache",
):
    check(required in macos_configure, f"macOS configure missing {required}")
macos_cache = step(macos_job, "build-macos", "Cache macOS compilation")
for required in ("uses: actions/cache/restore@v6", "steps.compiler.outputs.identity", "github.sha", "restore-keys:"):
    check(required in macos_cache, f"macOS compiler cache missing {required}")
macos_cache_save = step(macos_job, "build-macos", "Save macOS compilation cache")
for required in ("uses: actions/cache/save@v6", "github.event_name == 'push'", "github.ref == 'refs/heads/main'",
                 "steps.ccache.outputs.cache-primary-key"):
    check(required in macos_cache_save, f"macOS compiler cache must save only validated main builds: {required}")
check("ccache --show-stats" in macos_job, "macOS compiler cache must report hit statistics")
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
    "needs: [quality, build-windows, build-linux, build-macos, required]" in release_job,
    "Release must need quality, all hosted platform checks and CI Required",
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
    *DATA_ASSET_ID_FORMULA,
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
check(
    "@semantic-release/exec@" in release_semantic,
    "Semantic Release must install the exec plugin that stamps the release version",
)
unpublished_check = step(release_job, "release", "Check unpublished release revision")
for required in ("if: steps.semantic.outputs.new_release_published != 'true'",
                 "python3 scripts/ci_build_plan.py check-release"):
    check(required in unpublished_check, f"A stale unpublished release must fail explicitly: {required}")
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

# The release tag goes into every archive before the GitHub plugin uploads them (MuError.log's Release line).
plugin_names = [plugin[0] if isinstance(plugin, list) and plugin else plugin
                for plugin in release_config.get("plugins", [])]
exec_plugins = [
    plugin
    for plugin in release_config.get("plugins", [])
    if isinstance(plugin, list) and plugin and plugin[0] == "@semantic-release/exec"
]
check(len(exec_plugins) == 1, "Release config must contain one exec plugin")
prepare_command = ""
if len(exec_plugins) == 1 and len(exec_plugins[0]) == 2 and isinstance(exec_plugins[0][1], dict):
    prepare_command = exec_plugins[0][1].get("prepareCmd", "")
check(
    prepare_command.startswith("bash scripts/stamp_release_version.sh ${nextRelease.gitTag} "),
    "The exec plugin must stamp the release tag",
)
check(
    prepare_command.split()[3:] == [asset["path"] for asset in EXPECTED_RELEASE_ASSETS],
    "The exec plugin must stamp every published archive",
)
if "@semantic-release/exec" in plugin_names and "@semantic-release/github" in plugin_names:
    check(
        plugin_names.index("@semantic-release/exec") < plugin_names.index("@semantic-release/github"),
        "The release version must be stamped before the GitHub plugin",
    )

for required in (
    "workflow_dispatch:",
    "src/bin/Data/**",
    "src/bin/fonts/**",
    ".github/workflows/data-assets.yml",
    "contents: write",
    *DATA_ASSET_ID_FORMULA,
    "sha256sum",
    "MuMain-data-${asset_id}.tar.gz",
    "data-${asset_id}",
    "tar -czf",
    "-C src/bin Data fonts",
    'printf \'%s\\n\' "$tag" > "${version_folder}/Data/DataVersion.txt"',
    '-C "${version_folder}" Data/DataVersion.txt',
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
    # The checks cover the Windows, Linux and macOS jobs and the Linux guide, not only Windows.
    raise SystemExit("Workflow contract failed:\n- " + "\n- ".join(errors))

print("Workflow contract: OK")
