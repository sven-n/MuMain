#!/usr/bin/env python3

import json
import re
import sys
from pathlib import Path


ROOT = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[1]
CI_WORKFLOW = ROOT / ".github/workflows/ci.yml"
MINGW_WORKFLOW = ROOT / ".github/workflows/windows-build.yml"
RELEASE_CONFIG = ROOT / ".releaserc.json"
VCPKG_MANIFEST = ROOT / "vcpkg.json"
RENDERER_SOURCE = ROOT / "src/source/Render/Renderer/MuRendererSDLGpu.cpp"
PERSONAL_SHOP_SOURCE = (
    ROOT / "src/source/GameLogic/Items/PersonalShopTitleImp.cpp"
)
SHADER_BLOB_VALIDATOR = ROOT / "cmake/ValidateShaderBlobs.cmake"
SRC_CMAKE = ROOT / "src/CMakeLists.txt"
NETWORK_TEST_CMAKE = ROOT / "tests/network/CMakeLists.txt"
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

EXPECTED_RELEASE_ASSET = {
    "path": "MuMain-windows-native-x64-release-editor-off.tar.gz",
    "name": "MuMain-windows-native-x64-release-editor-off.tar.gz",
    "label": "MuMain (Windows native x64 Release, editor OFF)",
}

errors = []


def check(condition, message):
    if not condition:
        errors.append(message)


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


def folded_run_arguments(step_body, job_name, step_name):
    match = re.search(r"(?ms)^        run: >\n(.*)$", step_body)
    check(match is not None, f"Missing folded run scalar in {job_name} step: {step_name}")
    if not match:
        return []
    return [
        line.strip()
        for line in match.group(1).splitlines()
        if line.startswith("          ")
        and line.strip()
        and not line.strip().startswith("#")
    ]


def manifest(block, assignment):
    match = re.search(
        rf"(?ms)^\s*{re.escape(assignment)}\s*=\s*@?\((.*?)^\s*\)", block
    )
    check(match is not None, f"Missing shader manifest: {assignment}")
    if not match:
        return []
    return re.findall(r'^\s*["\']([^"\']+)["\']\s*$', match.group(1), re.M)


ci = CI_WORKFLOW.read_text(encoding="utf-8")
mingw = MINGW_WORKFLOW.read_text(encoding="utf-8")
release_config = json.loads(RELEASE_CONFIG.read_text(encoding="utf-8"))
vcpkg_manifest = json.loads(VCPKG_MANIFEST.read_text(encoding="utf-8"))
shader_blob_validator = SHADER_BLOB_VALIDATOR.read_text(encoding="utf-8")
src_cmake = SRC_CMAKE.read_text(encoding="utf-8")
network_test_cmake = NETWORK_TEST_CMAKE.read_text(encoding="utf-8")
personal_shop_source = PERSONAL_SHOP_SOURCE.read_text(encoding="utf-8")
quality_job = job(ci, "quality")
native_job = job(ci, "build-windows")
linux_job = job(ci, "build-linux")
release_job = job(ci, "release")
mingw_job = job(mingw, "build-mingw")

check(
    shader_blob_validator.startswith("cmake_minimum_required(VERSION 3.25)\n"),
    "Shader blob validation must declare CMake 3.25 policies in script mode",
)
for script_path in SCRIPT_MODE_TESTS:
    check(
        script_path.read_text(encoding="utf-8").startswith(
            "cmake_minimum_required(VERSION 3.25)\n"
        ),
        f"Script-mode test must declare CMake 3.25 policies: {script_path.name}",
    )
check(
    "std::max<int>(1, RenderIconSize.cy)" in personal_shop_source,
    "Personal-shop line height must resolve the Windows LONG/int std::max type",
)
for color_expression in (
    "RGBA(230, 230, 255, 255) : RGBA(230, 230, 0, 255)",
    "RGBA(108, 57, 41, 255) : RGBA(250, 150, 0, 255)",
    "RGBA(250, 150, 0, 128) : RGBA(108, 57, 41, 128)",
):
    check(
        color_expression in personal_shop_source,
        "Personal-shop colors must use the unsigned RGBA packer",
    )
check(
    "(255 << 24)" not in personal_shop_source
    and "(128 << 24)" not in personal_shop_source,
    "Personal-shop colors must not use signed left-shift packing",
)

quality_detection = step(quality_job, "quality", "Detect changed C++ files")
check(
    "diff_base=$DIFF_BASE" in quality_detection,
    "Quality detection must expose the comparison base",
)
quality_formatting = step(quality_job, "quality", "Check formatting")
for required in (
    "git diff --unified=0 --no-color",
    'format_args+=(--lines="${start}:${end}")',
    'clang-format --dry-run --Werror "${format_args[@]}" "$file"',
):
    check(
        required in quality_formatting,
        f"Formatting gate must inspect changed lines only: {required}",
    )
check(
    "xargs clang-format --dry-run --Werror" not in quality_formatting,
    "Formatting gate must not reject untouched legacy lines",
)

vcpkg_dependency_names = {
    dependency if isinstance(dependency, str) else dependency.get("name")
    for dependency in vcpkg_manifest.get("dependencies", [])
}
check(
    "directx-dxc" in vcpkg_dependency_names,
    "Windows dependency manifest must install the required DXC shader compiler",
)
directx_dxc_dependencies = [
    dependency
    for dependency in vcpkg_manifest.get("dependencies", [])
    if isinstance(dependency, dict) and dependency.get("name") == "directx-dxc"
]
check(
    len(directx_dxc_dependencies) == 1
    and directx_dxc_dependencies[0].get("platform") == "windows & x64",
    "DXC must install only for the x64 Windows shader-compilation rows",
)
glslang_dependencies = [
    dependency
    for dependency in vcpkg_manifest.get("dependencies", [])
    if isinstance(dependency, dict) and dependency.get("name") == "glslang"
]
check(
    len(glslang_dependencies) == 1
    and glslang_dependencies[0].get("features") == ["tools"],
    "Windows dependency manifest must install the glslang command-line tools",
)

linux_rows = re.findall(
    r"(?ms)^          - editor: (\S+)\n(.*?)(?=^          - editor: |^\s{4}steps:)",
    linux_job,
)
expected_linux_rows = [
    {
        "editor": "OFF",
        "build_directory": "out/build/linux-ci-editor-off",
        "artifact_name": "mu-client-linux-native-x64-debug-editor-off",
    },
    {
        "editor": "ON",
        "build_directory": "out/build/linux-ci-editor-on",
        "artifact_name": "mu-client-linux-native-x64-debug-editor-on",
    },
]
actual_linux_rows = []
for editor, body in linux_rows:
    values = {"editor": editor.strip("'\"")}
    values.update(
        {
            key: value.strip("'\"")
            for key, value in re.findall(r"(?m)^            ([^:\s]+): (.*)$", body)
        }
    )
    actual_linux_rows.append(values)
check(
    actual_linux_rows == expected_linux_rows,
    "Linux matrix must contain exact isolated editor-OFF/ON rows",
)
check(
    "name: Linux Native Build (editor ${{ matrix.editor }})" in linux_job,
    "Linux job name must expose editor state",
)
linux_cache = step(linux_job, "build-linux", "Cache SDL3 FetchContent")
for required in (
    "path: ${{ matrix.build_directory }}/_deps",
    "key: sdl3-linux-editor-${{ matrix.editor }}-${{ hashFiles('CMakeLists.txt', 'src/CMakeLists.txt') }}",
    "restore-keys: sdl3-linux-editor-${{ matrix.editor }}-",
):
    check(required in linux_cache, f"Linux matrix cache missing {required}")
linux_configure = step(linux_job, "build-linux", "Configure CMake")
linux_configure_arguments = folded_run_arguments(
    linux_configure, "build-linux", "Configure CMake"
)
for required in (
    "cmake --preset linux-x64",
    "-B ${{ matrix.build_directory }}",
    "-DENABLE_EDITOR=${{ matrix.editor }}",
    "-DBUILD_TESTING=ON",
):
    check(required in linux_configure_arguments, f"Linux configure missing {required}")
linux_build = step(linux_job, "build-linux", "Build")
check(
    "cmake --build ${{ matrix.build_directory }} --config Debug" in linux_build,
    "Linux matrix build must use its isolated build directory",
)
linux_tests = step(linux_job, "build-linux", "Run tests")
check(
    "ctest --test-dir ${{ matrix.build_directory }} --build-config Debug --output-on-failure --no-tests=error"
    in linux_tests,
    "Linux editor-OFF/ON rows must both run CTest",
)
linux_validation = step(linux_job, "build-linux", "Validate artifacts")
check(
    'EXE_DIR="${{ matrix.build_directory }}/src/Debug"' in linux_validation,
    "Linux validation must use its isolated build directory",
)
linux_upload = step(linux_job, "build-linux", "Upload artifact")
for required in (
    "name: ${{ matrix.artifact_name }}-${{ github.ref_name }}",
    "${{ matrix.build_directory }}/src/Debug/Main",
    "${{ matrix.build_directory }}/src/Debug/MUnique.Client.Library.so",
    "if-no-files-found: error",
):
    check(required in linux_upload, f"Linux matrix upload missing {required}")

renderer_source = RENDERER_SOURCE.read_text(encoding="utf-8")
check(
    "// Uniform buffers: b0, space1 (Transform)" in renderer_source,
    "basic_textured.vert comment must document b0, space1 Transform",
)
check(
    "// Uniform buffers: b1 (ScreenSize)" not in renderer_source,
    "basic_textured.vert comment must not retain stale b1 ScreenSize binding",
)

native_rows = re.findall(
    r"(?ms)^          - architecture: (\S+)\n(.*?)(?=^          - architecture: |^\s{4}steps:)",
    native_job,
)
check(len(native_rows) == 8, f"Native matrix must contain exactly 8 rows; found {len(native_rows)}")
required_native_row_keys = (
    "architecture",
    "configuration",
    "editor",
    "shader_compilation",
    "preset",
    "triplet",
    "build_directory",
    "artifact_name",
)
expected_native_rows = [
    {
        "architecture": "x64",
        "configuration": "Debug",
        "editor": "OFF",
        "shader_compilation": "OFF",
        "preset": "windows-x64",
        "triplet": "x64-windows",
        "build_directory": "out/build/windows-ci-x64-debug",
        "artifact_name": "mu-client-windows-native-x64-debug-editor-off",
    },
    {
        "architecture": "x64",
        "configuration": "Release",
        "editor": "OFF",
        "shader_compilation": "ON",
        "preset": "windows-x64",
        "triplet": "x64-windows",
        "build_directory": "out/build/windows-ci-x64-release",
        "artifact_name": "mu-client-windows-native-x64-release-editor-off",
    },
    {
        "architecture": "x64",
        "configuration": "Debug",
        "editor": "ON",
        "shader_compilation": "OFF",
        "preset": "windows-x64-mueditor",
        "triplet": "x64-windows",
        "build_directory": "out/build/windows-ci-x64-editor-debug",
        "artifact_name": "mu-client-windows-native-x64-debug-editor-on",
    },
    {
        "architecture": "x64",
        "configuration": "Release",
        "editor": "ON",
        "shader_compilation": "OFF",
        "preset": "windows-x64-mueditor",
        "triplet": "x64-windows",
        "build_directory": "out/build/windows-ci-x64-editor-release",
        "artifact_name": "mu-client-windows-native-x64-release-editor-on",
    },
    {
        "architecture": "x86",
        "configuration": "Debug",
        "editor": "OFF",
        "shader_compilation": "OFF",
        "preset": "windows-x86",
        "triplet": "x86-windows",
        "build_directory": "out/build/windows-ci-x86-debug",
        "artifact_name": "mu-client-windows-native-x86-debug-editor-off",
    },
    {
        "architecture": "x86",
        "configuration": "Release",
        "editor": "OFF",
        "shader_compilation": "OFF",
        "preset": "windows-x86",
        "triplet": "x86-windows",
        "build_directory": "out/build/windows-ci-x86-release",
        "artifact_name": "mu-client-windows-native-x86-release-editor-off",
    },
    {
        "architecture": "x86",
        "configuration": "Debug",
        "editor": "ON",
        "shader_compilation": "OFF",
        "preset": "windows-x86-mueditor",
        "triplet": "x86-windows",
        "build_directory": "out/build/windows-ci-x86-editor-debug",
        "artifact_name": "mu-client-windows-native-x86-debug-editor-on",
    },
    {
        "architecture": "x86",
        "configuration": "Release",
        "editor": "ON",
        "shader_compilation": "OFF",
        "preset": "windows-x86-mueditor",
        "triplet": "x86-windows",
        "build_directory": "out/build/windows-ci-x86-editor-release",
        "artifact_name": "mu-client-windows-native-x86-release-editor-on",
    },
]
actual_native_rows = []
for row_number, (architecture, body) in enumerate(native_rows, start=1):
    matches_by_key = {"architecture": [architecture]}
    for key, value in re.findall(r"(?m)^            ([^:\s]+): (.*)$", body):
        matches_by_key.setdefault(key, []).append(value)
    actual_keys = set(matches_by_key)
    check(
        actual_keys == set(required_native_row_keys),
        f"Native matrix row {row_number} keys must equal {sorted(required_native_row_keys)}; found {sorted(actual_keys)}",
    )
    for key in required_native_row_keys:
        matches = matches_by_key.get(key, [])
        check(
            len(matches) == 1,
            f"Native matrix row {row_number} key {key} must appear exactly once; found {len(matches)}",
        )
    if actual_keys != set(required_native_row_keys) or any(
        len(matches_by_key.get(key, [])) != 1 for key in required_native_row_keys
    ):
        continue
    values = {key: matches[0].strip("'\"") for key, matches in matches_by_key.items()}
    actual_native_rows.append(values)
check(
    actual_native_rows == expected_native_rows,
    "Native matrix rows must match the exact expected values and order",
)
expected_native_artifacts = {
    (row["architecture"], row["configuration"], row["editor"]): row["artifact_name"]
    for row in expected_native_rows
}
actual_native_artifacts = {
    (row["architecture"], row["configuration"], row["editor"]): row["artifact_name"]
    for row in actual_native_rows
}
check(
    actual_native_artifacts == expected_native_artifacts,
    "Native artifact names must encode toolchain, architecture, configuration, and editor state",
)
expected_native_shader_compilation = {
    (row["architecture"], row["configuration"], row["editor"]): row["shader_compilation"]
    for row in expected_native_rows
}
actual_native_shader_compilation = {
    (row["architecture"], row["configuration"], row["editor"]): row["shader_compilation"]
    for row in actual_native_rows
}
check(
    actual_native_shader_compilation == expected_native_shader_compilation,
    "Native shader compilation must be ON only for x64 Release editor-OFF",
)
check(
    [row for row, value in actual_native_shader_compilation.items() if value == "ON"]
    == [("x64", "Release", "OFF")],
    "Native matrix must contain exactly one shader-compilation ON tuple",
)

native_environment_assertion = step(
    native_job, "build-windows", "Assert MSVC developer environment"
)
check(
    native_environment_assertion.rstrip().endswith("exit 0"),
    "Native MSVC identity probes must not leak their expected nonzero exit codes",
)
native_environment_activation = step(
    native_job, "build-windows", "Activate Visual Studio developer environment"
)
check(
    "arch: ${{ matrix.architecture == 'x86' && 'amd64_x86' || 'x64' }}"
    in native_environment_activation,
    "Native x86 rows must use the x64-hosted x86 compiler so host .NET tools remain x64",
)

native_configure = step(native_job, "build-windows", "Configure CMake")
native_configure_arguments = folded_run_arguments(
    native_configure, "build-windows", "Configure CMake"
)
for required in (
    "-DMU_ENABLE_SHADER_COMPILATION=${{ matrix.shader_compilation }}",
    "-DGLSLANG_EXE=C:/vcpkg/installed/${{ matrix.triplet }}/tools/glslang/glslangValidator.exe",
    "-DSPIRV_CROSS_EXE=C:/vcpkg/installed/${{ matrix.triplet }}/tools/spirv-cross/spirv-cross.exe",
    "-DDXC_EXE=C:/vcpkg/installed/${{ matrix.triplet }}/tools/directx-dxc/dxc.exe",
):
    count = native_configure_arguments.count(required)
    check(
        count == 1,
        f"Native strict shader configure argument must appear exactly once: {required}; found {count}",
    )
check("DXC_EXE" not in mingw, "MinGW must not require the native MSVC DXC tool")

check(
    re.search(
        r'(?ms)COMMAND \$\{CMAKE_COMMAND\} -E env\n\s*--unset=Platform\n\s*"NUGET_PACKAGES=.*?"\$\{DOTNET_EXECUTABLE\}" run --project "\$\{RESXGEN_PROJ_NATIVE\}"',
        src_cmake,
    )
    is not None,
    "ResxGen must discard the MSVC target Platform before running as a host tool",
)
check(
    re.search(
        r'(?ms)NAME outbound_flush_loop\n\s*COMMAND "\$\{CMAKE_COMMAND\}" -E env --unset=Platform\n\s*"\$\{DOTNET_EXECUTABLE\}" run',
        network_test_cmake,
    )
    is not None,
    "The .NET network self-check must discard the MSVC target Platform",
)

native_crt_stage = step(
    native_job, "build-windows", "Stage MSVC redistributable runtime"
)
check(
    "if: matrix.configuration == 'Release'" in native_crt_stage,
    "Native MSVC redistributable staging must run only for Release rows",
)
for required in (
    "if ([string]::IsNullOrWhiteSpace($env:VCToolsRedistDir))",
    'Join-Path $env:VCToolsRedistDir "${{ matrix.architecture }}"',
    "if (-not (Test-Path -LiteralPath $redistArchitectureDirectory -PathType Container))",
    'Get-ChildItem -LiteralPath $redistArchitectureDirectory -Directory -Filter "Microsoft.VC*.CRT"',
    "if ($crtDirectories.Count -ne 1)",
    "$crtDirectory = $crtDirectories[0]",
    'Get-ChildItem -LiteralPath $crtDirectory.FullName -File -Filter "*.dll"',
    "if ($crtDlls.Count -eq 0)",
    'Copy-Item -LiteralPath $crtDlls.FullName -Destination $runtimeDirectory -Force',
):
    check(required in native_crt_stage, f"Native CRT staging missing {required}")
for forbidden in ("$env:PATH", "Get-Command", "-Recurse", "System32", "SysWOW64"):
    check(
        forbidden not in native_crt_stage,
        f"Native CRT staging must use only the active VCToolsRedistDir: {forbidden}",
    )

native_validation = step(native_job, "build-windows", "Validate artifacts")
check(
    "if: matrix.configuration == 'Release'" in native_validation,
    "Native runnable-artifact validation must run only for Release rows",
)
for required in (
    '"$runtimeDirectory/Main.exe"',
    '"$runtimeDirectory/MUnique.Client.Library.dll"',
    '"$runtimeDirectory/config.ini"',
    '"$runtimeDirectory/Data/Dec2.dat"',
    '"$runtimeDirectory/fonts/DejaVuSans.ttf"',
    '"$runtimeDirectory/Data"',
    '"$runtimeDirectory/shaders"',
):
    check(required in native_validation, f"Native validation missing {required}")
for required in (
    "dumpbin.exe",
    "/headers $binary",
    "/DEPENDENTS $binary",
    r"8664 machine \(x64\)",
    r"14C machine \(x86\)",
    "GetRelativePath",
    "Compare-Object",
    "-Force -File -Recurse",
    "PathType Leaf",
    "PathType Container",
    "foreach ($path in $requiredFiles)",
    "foreach ($path in $requiredDirectories)",
    "$runtimeBinaries = @(",
    "Get-ChildItem -LiteralPath $runtimeDirectory -Force -File -Recurse",
    'Where-Object { $_.Extension -in @(".exe", ".dll") }',
    "Sort-Object FullName",
    "if ($runtimeBinaries.Count -eq 0)",
    "foreach ($binary in $runtimeBinaries)",
    "$rootRuntimeFiles = @{}",
    "Get-ChildItem -LiteralPath $runtimeDirectory -Force -File |",
    "$rootRuntimeFiles[$_.Name] = $_.FullName",
    '$rootRuntimeFiles.ContainsKey($importName)',
    "Image has the following dependencies:",
    '$dependencySection.Groups["body"].Value',
    "$importFileName = [IO.Path]::GetFileName($importName)",
    "[IO.Path]::IsPathRooted($importName)",
    "$importName.Contains('/')",
    "$importName.Contains('\\')",
    "[StringComparer]::Ordinal.Equals($importFileName, $importName)",
    '$missingImports += "$($binary.FullName): invalid import name: $importName"',
    "ucrtbased",
    "vcruntime",
    "msvcp",
    "concrt",
    "vcomp",
    '$missingImports += "$($binary.FullName): debug CRT import in Release runtime: $importName"',
    "api-ms-win-*",
    "ext-ms-win-*",
    "$windowsOsDllAllowlist.ContainsKey($importName)",
    "$systemDllPath = Join-Path $targetSystemDirectory $importName",
    "Test-Path -LiteralPath $systemDllPath -PathType Leaf",
    "allowlisted Windows OS DLL missing from target system directory",
    "VC runtime import must be staged root-local",
    "$missingImports = @(",
    '$missingImports += "$($binary.FullName): $importName"',
    "$missingImports = @($missingImports | Sort-Object -Unique)",
    "if ($missingImports.Count -ne 0)",
    'throw "Missing runtime imports:',
):
    check(required in native_validation, f"Native validation missing {required}")
native_machine_switch = re.search(
    r'(?ms)\$expectedMachinePattern = switch \("\$\{\{ matrix\.architecture \}\}"\) \{(.*?)^\s*\}',
    native_validation,
)
check(native_machine_switch is not None, "Native validation missing architecture switch")
if native_machine_switch:
    machine_patterns = dict(
        re.findall(r'^\s*"(x64|x86)" \{ \'([^\']+)\' \}$', native_machine_switch.group(1), re.M)
    )
    check(
        machine_patterns
        == {
            "x64": r"^\s*8664 machine \(x64\)\s*$",
            "x86": r"^\s*14C machine \(x86\)\s*$",
        },
        "Native architecture-to-machine mappings must remain exact",
    )
check(
    re.search(
        r'(?ms)\$targetSystemDirectory = switch \("\$\{\{ matrix\.architecture \}\}"\) \{.*?"x64" \{.*?\[Environment\]::Is64BitProcess.*?"System32".*?"Sysnative".*?^\s*\}.*?"x86" \{.*?\[Environment\]::Is64BitOperatingSystem.*?"SysWOW64".*?"System32".*?^\s*\}',
        native_validation,
    )
    is not None,
    "Native target system-directory selection must follow the target architecture",
)
check(
    re.search(
        r"(?ms)\$headers = @\(& \$dumpbin /headers \$binary\.FullName 2>&1\)\n\s*if \(\$LASTEXITCODE -ne 0\) \{\n\s*throw ",
        native_validation,
    )
    is not None,
    "Native dumpbin execution must fail closed on nonzero exit",
)
check(
    re.search(
        r"(?ms)\$machineMatches = @\(\n\s*\$headers \| Select-String -Pattern \$expectedMachinePattern\n\s*\)\n\s*if \(\$machineMatches\.Count -ne 1\) \{\n\s*throw ",
        native_validation,
    )
    is not None,
    "Native machine matching must fail unless exactly one expected header exists",
)
check(
    re.search(
        r"(?ms)\$dependents = @\(& \$dumpbin /DEPENDENTS \$binary\.FullName 2>&1\)\n\s*if \(\$LASTEXITCODE -ne 0\) \{\n\s*throw ",
        native_validation,
    )
    is not None,
    "Native dependency dump must fail closed immediately on nonzero exit",
)
check(
    re.search(
        r"(?ms)\$dependencySection = \[regex\]::Match\(.*?Image has the following dependencies:.*?\)\n\s*if \(-not \$dependencySection\.Success\) \{\n\s*throw ",
        native_validation,
    )
    is not None,
    "Native dependency parsing must fail closed when the dependency heading is absent",
)
check(
    re.search(
        r"(?ms)foreach \(\$binary in \$runtimeBinaries\) \{\n\s*\$headers = @\(& \$dumpbin /headers \$binary\.FullName 2>&1\).*?\$machineMatches = @\(.*?if \(\$machineMatches\.Count -ne 1\) \{\n\s*throw .*?\n\s*\}\n\n\s*\$dependents = @\(& \$dumpbin /DEPENDENTS \$binary\.FullName 2>&1\)",
        native_validation,
    )
    is not None,
    "Native validation must check architecture and imports for every staged PE",
)
check(
    re.search(
        r"(?m)^\s*\$rootRuntimeFiles = @\{\}\n\s*Get-ChildItem -LiteralPath \$runtimeDirectory -Force -File \|\n\s*ForEach-Object \{ \$rootRuntimeFiles\[\$_.Name\] = \$_.FullName \}$",
        native_validation,
    )
    is not None,
    "Native root runtime map must include hidden files",
)
check(
    re.search(
        r'''(?m)^\s*\$runtimeBinaries = @\(\n\s*Get-ChildItem -LiteralPath \$runtimeDirectory -Force -File -Recurse \|\n\s*Where-Object \{ \$_.Extension -in @\("\.exe", "\.dll"\) \} \|\n\s*Sort-Object FullName\n\s*\)$''',
        native_validation,
    )
    is not None,
    "Native recursive PE enumeration must include hidden files",
)
check(
    re.search(
        r'''(?ms)\$importFileName = \[IO\.Path\]::GetFileName\(\$importName\)\n\s*if \(\n\s*\[IO\.Path\]::IsPathRooted\(\$importName\) -or\n\s*\$importName\.Contains\('/'\) -or\n\s*\$importName\.Contains\('\\'\) -or\n\s*-not \[StringComparer\]::Ordinal\.Equals\(\$importFileName, \$importName\)\n\s*\) \{\n\s*\$missingImports \+= "\$\(\$binary\.FullName\): invalid import name: \$importName"\n\s*continue\n\s*\}''',
        native_validation,
    )
    is not None,
    "Native dependency validation must aggregate and reject every non-leaf import name",
)
debug_crt_guard = re.search(
    r'''(?ms)if \(\$importName -match '([^']+)'\) \{\n\s*\$missingImports \+= "\$\(\$binary\.FullName\): debug CRT import in Release runtime: \$importName"\n\s*continue\n\s*\}''',
    native_validation,
)
check(
    debug_crt_guard is not None,
    "Native Release closure must aggregate and reject debug CRT imports",
)
if debug_crt_guard:
    debug_crt_pattern = re.compile(debug_crt_guard.group(1), re.IGNORECASE)
    for debug_crt_name in (
        "ucrtbased.dll",
        "vcruntime140d.dll",
        "vcruntime140_1d.dll",
        "msvcp140d.dll",
        "msvcp140_atomic_waitd.dll",
        "msvcp140_codecvt_idsd.dll",
        "concrt140d.dll",
        "vcomp140d.dll",
    ):
        check(
            debug_crt_pattern.fullmatch(debug_crt_name) is not None,
            f"Native Release closure must reject debug CRT import {debug_crt_name}",
        )
    for release_crt_name in (
        "ucrtbase.dll",
        "vcruntime140.dll",
        "msvcp140.dll",
    ):
        check(
            debug_crt_pattern.fullmatch(release_crt_name) is None,
            f"Native Release closure must not reject release CRT import {release_crt_name}",
        )
release_crt_guard = re.search(
    r'''(?ms)if \(\$importName -match '([^']+)'\) \{\n\s*\$missingImports \+= "\$\(\$binary\.FullName\): VC runtime import must be staged root-local: \$importName"\n\s*continue\n\s*\}''',
    native_validation,
)
check(
    release_crt_guard is not None,
    "Native Release closure must require VC runtime imports root-local",
)
if release_crt_guard:
    release_crt_pattern = re.compile(release_crt_guard.group(1), re.IGNORECASE)
    for release_crt_name in (
        "vcruntime140.dll",
        "vcruntime140_1.dll",
        "msvcp140.dll",
        "msvcp140_atomic_wait.dll",
        "msvcp140_codecvt_ids.dll",
        "concrt140.dll",
        "vcomp140.dll",
    ):
        check(
            release_crt_pattern.fullmatch(release_crt_name) is not None,
            f"Native Release closure must require root-local {release_crt_name}",
        )
    for windows_os_name in ("ucrtbase.dll", "kernel32.dll"):
        check(
            release_crt_pattern.fullmatch(windows_os_name) is None,
            f"Native Release CRT root guard must not classify {windows_os_name}",
        )
windows_os_dlls = manifest(native_validation, "$windowsOsDlls")
check(
    len(windows_os_dlls) == len(EXPECTED_WINDOWS_OS_DLLS)
    and set(windows_os_dlls) == EXPECTED_WINDOWS_OS_DLLS,
    "Native Windows OS DLL allowlist must equal the explicit reviewed set",
)
check(
    not any(
        re.fullmatch(r"(?:vcruntime|msvcp|concrt|vcomp)[0-9]+(?:_[a-z0-9_]+)?\.dll", name)
        for name in windows_os_dlls
    ),
    "Native Windows OS DLL allowlist must not contain redistributable VC runtime families",
)
check(
    re.search(
        r'''(?ms)if \(\$windowsOsDllAllowlist\.ContainsKey\(\$importName\)\) \{\n\s*\$systemDllPath = Join-Path \$targetSystemDirectory \$importName\n\s*if \(Test-Path -LiteralPath \$systemDllPath -PathType Leaf\) \{\n\s*continue\n\s*\}\n\s*\$missingImports \+= "\$\(\$binary\.FullName\): allowlisted Windows OS DLL missing from target system directory: \$importName"\n\s*continue\n\s*\}''',
        native_validation,
    )
    is not None,
    "Native target system lookup must be nested under the explicit OS DLL allowlist",
)
import_validation_markers = (
    "$importFileName = [IO.Path]::GetFileName($importName)",
    "debug CRT import in Release runtime",
    "$rootRuntimeFiles.ContainsKey($importName)",
    "VC runtime import must be staged root-local",
    '$importName -like "api-ms-win-*"',
    "$windowsOsDllAllowlist.ContainsKey($importName)",
)
import_validation_positions = [
    native_validation.find(marker) for marker in import_validation_markers
]
check(
    all(position >= 0 for position in import_validation_positions)
    and import_validation_positions == sorted(import_validation_positions),
    "Native non-leaf import rejection must precede root, API-set, and system allowlists",
)
for forbidden in (
    "$env:PATH",
    "Get-Command $importName",
    "Get-Command $import",
    "C:/vcpkg",
    "C:\\vcpkg",
    "Visual Studio",
    "VC\\Tools",
):
    check(
        forbidden not in native_validation,
        f"Native dependency validation must not resolve imports through developer paths: {forbidden}",
    )
native_stage_position = native_job.find("- name: Stage MSVC redistributable runtime")
native_validation_position = native_job.find("- name: Validate artifacts")
native_upload_position = native_job.find("- name: Upload artifact")
check(
    -1 < native_stage_position < native_validation_position < native_upload_position,
    "Native Release flow must stage MSVC CRTs before validation and upload",
)
native_shaders = manifest(native_validation, "$expectedShaders")
check(
    len(native_shaders) == 18 and set(native_shaders) == EXPECTED_SHADERS,
    "Native shader manifest must equal the exact 18-file runtime set",
)
check(
    native_validation.count('"$runtimeDirectory/Main.exe"') == 1
    and native_validation.count('"$runtimeDirectory/MUnique.Client.Library.dll"') == 1,
    "Native required-file validation must not hard-code the PE scan",
)

native_upload = step(native_job, "build-windows", "Upload artifact")
native_step_upload_actions = re.findall(
    r"(?m)^        uses: actions/upload-artifact@\S+\s*$", native_upload
)
check(
    len(native_step_upload_actions) == 1,
    "Native Release-gated upload step must contain exactly one artifact upload action; "
    f"found {len(native_step_upload_actions)}",
)
native_job_upload_actions = re.findall(
    r"(?m)^        uses: actions/upload-artifact@\S+\s*$", native_job
)
check(
    len(native_job_upload_actions) == 1,
    f"Native job must contain exactly one artifact upload action; found {len(native_job_upload_actions)}",
)
check(
    "${{ matrix.build_directory }}/src/${{ matrix.configuration }}/Main.exe"
    in native_upload,
    "Native upload must include Main.exe",
)
check(
    "${{ matrix.build_directory }}/src/${{ matrix.configuration }}/MUnique.Client.Library.dll"
    in native_upload,
    "Native upload must include the network library",
)
check("path: |" in native_upload, "Native upload must list the two required binaries")
check(
    "include-hidden-files" not in native_upload,
    "Native binary-only upload must not include unrelated hidden files",
)
check(
    "if-no-files-found: error" in native_upload,
    "Native upload must fail when either required binary is missing",
)
check(
    "name: ${{ matrix.artifact_name }}-${{ github.ref_name }}" in native_upload,
    "Native upload must use the fully qualified matrix artifact name",
)
check(
    "if: github.event_name == 'push' && matrix.configuration == 'Release'"
    in native_upload,
    "Native runnable artifacts must upload only from Release rows",
)
release_download = step(release_job, "release", "Download build artifact")
check(
    re.search(
        r"(?m)^\s*name: mu-client-windows-native-x64-release-editor-off-main\s*$",
        release_download,
    )
    is not None,
    "Release must download the native x64 Release editor-OFF artifact",
)
check(
    re.search(r"(?m)^\s*path: artifacts\s*$", release_download) is not None,
    "Release must download the complete native runtime into artifacts",
)
release_archive = step(release_job, "release", "Archive native Windows runtime")
archive_command = (
    "run: tar -czf MuMain-windows-native-x64-release-editor-off.tar.gz "
    "-C artifacts ."
)
check(
    archive_command in release_archive,
    "Release must archive the complete artifacts directory with the exact native x64 output name",
)
check(
    release_job.count("tar -czf ") == 1,
    "Release job must create exactly one runtime archive",
)
release_semantic = step(release_job, "release", "Semantic Release")
release_download_position = release_job.find("- name: Download build artifact")
release_archive_position = release_job.find("- name: Archive native Windows runtime")
release_semantic_position = release_job.find("- name: Semantic Release")
check(
    -1 < release_download_position < release_archive_position < release_semantic_position,
    "Release flow must download, archive, then publish in order",
)
check(
    "uses: cycjimmy/semantic-release-action@v4" in release_semantic,
    "Release flow must publish the archive through semantic-release",
)
github_plugins = [
    plugin
    for plugin in release_config.get("plugins", [])
    if isinstance(plugin, list)
    and plugin
    and plugin[0] == "@semantic-release/github"
]
check(
    len(github_plugins) == 1,
    f"Release config must contain exactly one GitHub plugin; found {len(github_plugins)}",
)
release_assets = []
if len(github_plugins) == 1 and len(github_plugins[0]) == 2:
    github_options = github_plugins[0][1]
    if isinstance(github_options, dict):
        release_assets = github_options.get("assets", [])
check(
    release_assets == [EXPECTED_RELEASE_ASSET],
    "Semantic release must publish exactly the native x64 Release editor-OFF runtime archive",
)
for stale_release_token in (
    "artifacts/Main.exe",
    "MuMain-mingw-i686.exe",
    "MinGW i686",
):
    check(
        stale_release_token not in RELEASE_CONFIG.read_text(encoding="utf-8"),
        f"Release config must not retain stale asset metadata: {stale_release_token}",
    )

mingw_configure = step(mingw_job, "build-mingw", "Configure CMake")
check(
    "- name: Validate MinGW dependency roots" not in mingw_job,
    "MinGW must rely on configure and link results instead of cache internals",
)
mingw_stage = step(mingw_job, "build-mingw", "Stage runnable directory")
check(
    "-DCMAKE_BUILD_TYPE=Release" in mingw_configure,
    "MinGW exact asset-stamp exclusion requires the Release configuration",
)
for required in (
    '-DCMAKE_TOOLCHAIN_FILE="${VCPKG_INSTALLATION_ROOT}/scripts/buildsystems/vcpkg.cmake"',
    '-DVCPKG_CHAINLOAD_TOOLCHAIN_FILE="${GITHUB_WORKSPACE}/cmake/toolchains/mingw-w64-${{ matrix.arch }}.cmake"',
    '-DVCPKG_TARGET_TRIPLET="${{ matrix.vcpkg_triplet }}"',
    '-DVCPKG_INSTALLED_DIR="${VCPKG_CLASSIC_INSTALL_ROOT}"',
    "-DVCPKG_MANIFEST_MODE=OFF",
):
    check(
        mingw_configure.count(required) == 1,
        f"MinGW configure must use vcpkg chainloading exactly once: {required}",
    )
check(
    "MINGW_ADDITIONAL_TARGET_ROOT" not in mingw_configure,
    "MinGW configure must not replace vcpkg package integration with a find root",
)
for toolchain_path in MINGW_TOOLCHAINS:
    toolchain = toolchain_path.read_text(encoding="utf-8")
    check(
        "MINGW_ADDITIONAL_TARGET_ROOT" not in toolchain,
        f"MinGW toolchain must not retain the obsolete extra find root: {toolchain_path.name}",
    )
    check(
        "set(CMAKE_FIND_ROOT_PATH " not in toolchain
        and "list(APPEND CMAKE_FIND_ROOT_PATH /usr/" in toolchain
        and "list(REMOVE_DUPLICATES CMAKE_FIND_ROOT_PATH)" in toolchain,
        f"MinGW toolchain must preserve vcpkg roots across repeated chainloads: {toolchain_path.name}",
    )
for required in (
    'source_directory="build-mingw/src"',
    'runtime_directory="${RUNNER_TEMP}/mu-client-mingw-${{ matrix.arch }}-editor-${{ matrix.editor }}"',
    'mkdir -p "${runtime_directory}"',
    'tar -C "${source_directory}"',
    "--anchored",
    "--exclude='./CMakeFiles'",
    "--exclude='./ThirdParty'",
    "--exclude='./CTestTestfile.cmake'",
    "--exclude='./cmake_install.cmake'",
    "--exclude='./libMuClient.a'",
    "--exclude='./libimgui.a'",
    "--exclude='./Main.map'",
    "--exclude='./.assets_copied_Release.stamp'",
    "# ponytail: current CMake/Ninja metadata only; add an install target when packaging grows.",
    "stage_compiler_runtime_closure",
    '"${{ matrix.triple }}-objdump" -p "${binary}"',
    "DLL Name:",
    "libgcc_s_*.dll",
    "libstdc++-6.dll",
    "libwinpthread-1.dll",
    '"${{ matrix.triple }}-g++" "-print-file-name=${import_name}"',
    'cp -- "${compiler_path}" "${runtime_directory}/${import_name}"',
    "find \"${runtime_directory}\" -type f \\",
    "-iname '*.exe' -o -iname '*.dll'",
):
    check(required in mingw_stage, f"MinGW staging missing {required}")
for forbidden in (
    "--exclude='./*.a'",
    "--exclude='./*.map'",
    "--exclude='./.assets_copied_*.stamp'",
):
    check(forbidden not in mingw_stage, f"MinGW staging uses recursive wildcard exclusion: {forbidden}")
check("--no-anchored" not in mingw_stage, "MinGW staging must not disable root-anchored exclusions")
check(
    re.search(
        r'(?m)^\s*tar -C "\$\{source_directory\}" \\\n\s*--anchored \\\n\s*--exclude=\'\./CMakeFiles\' \\$',
        mingw_stage,
    )
    is not None,
    "MinGW staging must anchor exclusions before the exclusion list",
)
check("rm " not in mingw_stage and "rm -" not in mingw_stage, "MinGW staging must reject stale output instead of deleting it")
check(
    re.search(
        r'(?m)^\s*test -f "\$\{source_directory\}/Main\.exe"\s*$', mingw_stage
    )
    is not None,
    "MinGW staging must fail closed when the source executable is missing",
)
check(
    re.search(
        r'(?ms)^\s*if \[ -e "\$\{runtime_directory\}" \]; then\n\s*echo .* >&2\n\s*exit 1\n\s*fi\s*$',
        mingw_stage,
    )
    is not None,
    "MinGW staging must reject a stale destination",
)
check(
    re.search(
        r'(?m)^\s*-cf - \. \| tar -C "\$\{runtime_directory\}" -xf -\s*$',
        mingw_stage,
    )
    is not None,
    "MinGW staging tar pipeline must fail closed",
)
check(
    re.search(
        r'(?m)^\s*test -f "\$\{runtime_directory\}/Main\.exe"\s*$', mingw_stage
    )
    is not None,
    "MinGW staging must fail closed when the staged executable is missing",
)

mingw_validation = step(mingw_job, "build-mingw", "Validate runnable directory")
for required in (
    'runtime_directory="${RUNNER_TEMP}/mu-client-mingw-${{ matrix.arch }}-editor-${{ matrix.editor }}"',
    '"${runtime_directory}/Main.exe"',
    '"${runtime_directory}/config.ini"',
    '"${runtime_directory}/Data/Dec2.dat"',
    '"${runtime_directory}/fonts/DejaVuSans.ttf"',
    '"${runtime_directory}/Data"',
    '"${runtime_directory}/shaders"',
    '"${{ matrix.triple }}-objdump" -f "${binary}"',
    '"${{ matrix.triple }}-objdump" -p "${binary}"',
    "DLL Name:",
    "runtime_binaries",
    'for binary in "${runtime_binaries[@]}"',
    "if [ ${#runtime_binaries[@]} -eq 0 ]",
    "is_windows_system_dll",
    "kernel32.dll",
    "user32.dll",
    "api-ms-win-*",
    "ext-ms-win-*",
    "basename -- \"${import_name}\"",
    '"${import_name}" == *"/"*',
    r'"${import_name}" == *"\\"*',
    "remaining_imports",
    "invalid import name",
    "missing runtime import",
    "pei-x86-64",
    "i386:x86-64",
    "pei-i386",
    "i386, flags",
    "find \"${shader_directory}\" -type f",
    "diff -u",
    'for path in "${required_files[@]}"',
    'for path in "${required_directories[@]}"',
    'if [ ! -f "${path}" ]',
    'if [ ! -d "${path}" ]',
    "grep -Eq",
    "grep -Fq",
):
    check(required in mingw_validation, f"MinGW validation missing {required}")
check(
    re.search(
        r'(?m)^\s*grep -Eq "\[\[:space:\]\]file format \$\{expected_file_format\}\$" <<< "\$\{headers\}"\s*$',
        mingw_validation,
    )
    is not None,
    "MinGW file-format validation must fail closed",
)
check(
    re.search(
        r'(?m)^\s*grep -Fq "\$\{expected_architecture_line\}" <<< "\$\{headers\}"\s*$',
        mingw_validation,
    )
    is not None,
    "MinGW architecture validation must fail closed",
)
check(
    re.search(
        r'(?ms)^\s*x86_64\)\n\s*expected_file_format="pei-x86-64"\n\s*expected_architecture_line="architecture: i386:x86-64, flags"\n\s*;;',
        mingw_validation,
    )
    is not None,
    "MinGW x86_64 architecture mapping must remain exact",
)
check(
    re.search(
        r'(?ms)^\s*i686\)\n\s*expected_file_format="pei-i386"\n\s*expected_architecture_line="architecture: i386, flags"\n\s*;;',
        mingw_validation,
    )
    is not None,
    "MinGW i686 architecture mapping must remain exact",
)
check(
    re.search(
        r'(?ms)for binary in "\$\{runtime_binaries\[@\]\}"; do.*?objdump" -f "\$\{binary\}".*?objdump" -p "\$\{binary\}".*?done',
        mingw_validation,
    )
    is not None,
    "MinGW validation must architecture-check and close imports for every staged PE",
)
check(
    re.search(
        r'''(?ms)mapfile -d '' -t runtime_binaries < <\(\n\s*find "\$\{runtime_directory\}" -type f \\\n\s*\\\( -iname '\*\.exe' -o -iname '\*\.dll' \\\) \\\n\s*-print0 \| LC_ALL=C sort -z\n\s*\)''',
        mingw_validation,
    )
    is not None,
    "MinGW validation must recursively enumerate every staged EXE and DLL",
)
check(
    re.search(
        r'''(?ms)stage_compiler_runtime_closure\(\) \{.*?for iteration in \{1\.\.8\}; do.*?find "\$\{runtime_directory\}" -type f.*?-iname '\*\.exe'.*?-iname '\*\.dll'.*?objdump" -p "\$\{binary\}".*?libgcc_s_\*\.dll\|libstdc\+\+-6\.dll\|libwinpthread-1\.dll.*?g\+\+" "-print-file-name=\$\{import_name\}".*?cp -- "\$\{compiler_path\}" "\$\{runtime_directory\}/\$\{import_name\}".*?if ! "\$\{copied\}"; then\n\s*return 0''',
        mingw_stage,
    )
    is not None,
    "MinGW staging must iterate recursive compiler-runtime closure to convergence",
)
check(
    re.search(r"(?m)^\s*stage_compiler_runtime_closure\s*$", mingw_stage)
    is not None,
    "MinGW staging must execute compiler-runtime closure",
)
mingw_import_markers = (
    '"$(basename -- "${import_name}")" != "${import_name}"',
    'if runtime_has_root_import "${import_name}"; then',
    "api-ms-win-*|ext-ms-win-*",
    'if is_windows_system_dll "${import_name}"; then',
)
mingw_import_positions = [
    mingw_validation.find(marker) for marker in mingw_import_markers
]
check(
    all(position >= 0 for position in mingw_import_positions)
    and mingw_import_positions == sorted(mingw_import_positions),
    "MinGW non-leaf rejection must precede root, API-set, and system allowlists",
)
check(
    re.search(
        r'''(?ms)if \[ \$\{#remaining_imports\[@\]\} -ne 0 \]; then\n\s*printf '%s\\n' "\$\{remaining_imports\[@\]\}" \| LC_ALL=C sort -u >&2\n\s*exit 1\n\s*fi''',
        mingw_validation,
    )
    is not None,
    "MinGW validation must aggregate and fail remaining imports",
)
check(
    mingw_job.count('"-print-file-name=${import_name}"') == 1
    and mingw_job.count('cp -- "${compiler_path}" "${runtime_directory}/${import_name}"') == 1,
    "MinGW closure must stage compiler runtimes through one explicit compiler lookup",
)
compiler_path_assignments = re.findall(
    r"(?m)^\s*(?!#)(?:if ! )?compiler_path=.*$", mingw_stage
)
check(
    [assignment.strip() for assignment in compiler_path_assignments]
    == [
        'if ! compiler_path=$("${{ matrix.triple }}-g++" "-print-file-name=${import_name}"); then'
    ],
    "MinGW closure must have exactly one executable compiler_path assignment from target g++",
)
for forbidden in (
    "$PATH",
    "${PATH}",
    "command -v ${import_name}",
    "which --",
    "type -P",
    "drive_c/windows/system32",
    "drive_c/windows/syswow64",
):
    check(
        forbidden not in mingw_stage + mingw_validation,
        f"MinGW closure must not resolve imports through PATH or Wine directories: {forbidden}",
    )
mingw_shaders = manifest(mingw_validation, "expected_shaders")
check(
    len(mingw_shaders) == 18 and set(mingw_shaders) == EXPECTED_SHADERS,
    "MinGW shader manifest must equal the exact 18-file runtime set",
)
native_aot_rejection = 'if [ -e "${runtime_directory}/MUnique.Client.Library.dll" ]; then'
check(native_aot_rejection in mingw_validation, "MinGW validation must reject a staged NativeAOT DLL")
check(
    mingw_job.count("MUnique.Client.Library.dll") == 1,
    "MinGW must mention NativeAOT only in its explicit rejection check",
)

mingw_upload = step(mingw_job, "build-mingw", "Upload client binary")
check(
    "name: mu-client-windows-mingw-${{ matrix.arch }}-release-editor-${{ matrix.editor }}" in mingw_upload,
    "MinGW artifact name must encode toolchain, architecture, configuration, and editor state",
)
check(
    "path: ${{ runner.temp }}/mu-client-mingw-${{ matrix.arch }}-editor-${{ matrix.editor }}/Main.exe"
    in mingw_upload,
    "MinGW upload must include only Main.exe",
)
check("path: |" not in mingw_upload, "MinGW upload must not hand-pick runtime files")
check(
    "if-no-files-found: error" in mingw_upload,
    "MinGW upload must fail when the runtime directory is missing",
)
check("include-hidden-files" not in mingw_upload, "MinGW upload must exclude unrelated files")
check(
    "MUnique.Client.Library.dll" not in mingw_upload,
    "MinGW upload must not package or claim NativeAOT",
)

if "- name: Validate artifacts" in native_job and "- name: Upload artifact" in native_job:
    check(
        native_job.index("- name: Validate artifacts")
        < native_job.index("- name: Upload artifact"),
        "Native validation must gate upload",
    )
if all(
    name in mingw_job
    for name in (
        "- name: Stage runnable directory",
        "- name: Validate runnable directory",
        "- name: Upload client binary",
    )
):
    check(
        mingw_job.index("- name: Stage runnable directory")
        < mingw_job.index("- name: Validate runnable directory")
        < mingw_job.index("- name: Upload client binary"),
        "MinGW staging and validation must gate upload",
    )

if errors:
    raise SystemExit("Windows workflow contract failed:\n- " + "\n- ".join(errors))

print("Windows workflow contract: OK")
