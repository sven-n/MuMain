# Windows Repair Validation Log

This file is the durable before/after record for the phased Windows build and
runtime repair. Run commands from the repository root. Keep pre-existing
failures separate from failures introduced by later phases.

## Phase 0: Baseline

### Repository snapshot

Captured at `2026-08-24T21:10:21Z`, before Windows production, workflow,
renderer, shader, editor, capture, or packaging changes.

```text
git rev-parse HEAD
4f3d1e5922c99b0fe72f5fd08dd814ad13779d5a

git rev-parse 'HEAD^{tree}'
0705984049c89e97c01c7ba914d606188bf560b6

git status --short --branch
## main...origin/main
?? docs/build/windows-repair-validation.md
?? docs/superpowers/plans/2026-08-24-windows-build-runtime-repair.md
?? docs/superpowers/specs/2026-08-24-windows-build-runtime-repair-design.md
```

The tracked tree is clean. Both the staged and unstaged binary patch streams
are empty: `0` bytes, SHA-256
`e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`.
`git diff --check` reports no error. Ignored build products are excluded.

Recursive submodules:

```text
git submodule status --recursive
 d9d5536704d585616d4db3c8ba3c4ff6fc2757e1 src/ThirdParty/SDL (release-3.4.8)
 21d3299e588b5c702dcca0f448b4f937af369b4a src/ThirdParty/imgui (v1.62-5145-g21d3299e5)
```

Untracked documentation checksums before this log refresh:

```text
7a78dd93c6f6b91c7c0fb6c2efc7a09b0350ac9be00e009a0cebcfb33541247e  docs/superpowers/plans/2026-08-24-windows-build-runtime-repair.md
2938bc0c6150514a29116f92f0e3bf289bbbd62e0f91db98be037565ed00e5fe  docs/superpowers/specs/2026-08-24-windows-build-runtime-repair-design.md
```

The current validation log is excluded because self-hashing is
non-reproducible. The plan and design were not changed during the refresh.

### Committed SDL 3D-quad baseline

The Aura/independent-3D-quad work is no longer concurrent working-tree input.
It is committed at `4f3d1e59` (`fix(render): restore independent 3D quads`) and
is part of the immutable Windows repair baseline. No Aura, renderer, or test
source file was modified while capturing Phase 0.

The old baseline process PID `60686` no longer exists. Exact-name checks for
`cmake`, `ctest`, and `ninja` found no lingering baseline process before the
refresh. The existing ignored build directory was retained and reconfigured;
no old ignored output was deleted.

Focused baseline commands:

```sh
cmake -S . -B out/build/windows-repair-baseline-macos \
  -G 'Ninja Multi-Config' \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/macos-arm64.cmake \
  -DBUILD_TESTING=ON \
  -DENABLE_EDITOR=OFF \
  -DOPENSSL_ROOT_DIR="$(brew --prefix openssl@3)"
cmake --build out/build/windows-repair-baseline-macos --config Debug \
  --target quad_topology_tests frame_pixel_readback_tests
out/build/windows-repair-baseline-macos/tests/render/Debug/quad_topology_tests
out/build/windows-repair-baseline-macos/tests/render/Debug/frame_pixel_readback_tests
```

Results:

- Configure: exit `0`; Clang `22.1.8`; `BUILD_TESTING=ON`;
  `ENABLE_EDITOR=OFF`. The exact portable command shown above was rerun from
  `2026-08-24T21:18:01Z` through `2026-08-24T21:18:05Z`; exit `0`.
- Focused build: exit `0`; current targets rebuilt as needed.
- `quad_topology_tests`: exit `0`; 4/4 cases, 11/11 assertions.
- `frame_pixel_readback_tests`: exit `0`; 20/20 cases, 75/75 assertions.

An earlier `ctest -R '^(quad_topology_tests|frame_pixel_readback_tests)$'`
reported `No tests were found` because `doctest_discover_tests` registers case
names, not executable names. Direct executable results above are the focused
gate. Later regression runs must use the executables or matching doctest case
names.

### Host baseline

Host:

```text
macOS 26.5 (25F71), Darwin 25.5.0, arm64, Apple M2
```

Tools:

```text
CMake                 4.4.2
CTest                 4.4.2
Ninja                 1.13.2
Git                   2.55.0
configured C/C++      Homebrew Clang 22.1.8
system c++            Apple Clang 21.0.0
Apple ld              ld-1267
.NET SDK              10.0.302
.NET runtime          10.0.10 arm64
glslangValidator      16.4.0, SPIR-V 1.6
spirv-cross           build timestamp 2026-07-06T12:43:32
dxc                    unavailable
pkg-config             unavailable
vulkaninfo             unavailable
Wine                   unavailable
Xcode                  unavailable; Command Line Tools selected
```

Full isolated host baseline:

```sh
cmake --build out/build/windows-repair-baseline-macos --config Debug -- -j1
ctest --test-dir out/build/windows-repair-baseline-macos \
  --build-config Debug --output-on-failure --no-tests=error
```

Results:

- Full Debug build: exit `0`. Ninja completed the current incremental graph;
  link steps warned that local Homebrew OpenSSL and turbojpeg were built for
  a newer macOS deployment target.
- CTest: exit `0`; 87/87 tests passed, 0 failed, 16.55 seconds.
- This is a retained-directory baseline, not a clean-room rebuild. The build
  directory was not deleted because it is ignored local output and may contain
  useful prior evidence.

### Existing Windows CI failures

- Source report:
  <https://github.com/sven-n/MuMain/pull/572#issuecomment-5395165036>
- Follow-up requesting native validation:
  <https://github.com/sven-n/MuMain/pull/572#issuecomment-5399114029>
- Native workflow run
  [32768004628](https://github.com/yesid-bocanegra/MuMain/actions/runs/32768004628),
  Windows job
  [97561758263](https://github.com/yesid-bocanegra/MuMain/actions/runs/32768004628/job/97561758263):
  selected GNU 15.2 on Windows; NativeAOT reached GNU `link`, which rejected
  `/DEF:`.
- MinGW workflow run
  [32768004592](https://github.com/yesid-bocanegra/MuMain/actions/runs/32768004592)
  failed all four architecture/editor jobs during OpenSSL discovery:
  [i686 editor OFF](https://github.com/yesid-bocanegra/MuMain/actions/runs/32768004592/job/97561756647),
  [i686 editor ON](https://github.com/yesid-bocanegra/MuMain/actions/runs/32768004592/job/97561756865),
  [x86_64 editor OFF](https://github.com/yesid-bocanegra/MuMain/actions/runs/32768004592/job/97561756893), and
  [x86_64 editor ON](https://github.com/yesid-bocanegra/MuMain/actions/runs/32768004592/job/97561756912).
- Both failed runs started on `2026-08-24` at commit
  `3256fc5312b04c8fa87f4d7bf73ece66c2ac71b0`. They predate baseline HEAD
  `4f3d1e59` by three commits. They are historical root-cause evidence, not a
  reproduction at baseline HEAD.
- Experimental evidence only:
  <https://github.com/yesid-bocanegra/MuMain/pull/10>

Current baseline-HEAD Actions state, queried at `2026-08-24T21:23:13Z`:

- [CI run 32777595726](https://github.com/yesid-bocanegra/MuMain/actions/runs/32777595726):
  status `completed`, conclusion `failure`.
  - [Windows Native Build job 97592130487](https://github.com/yesid-bocanegra/MuMain/actions/runs/32777595726/job/97592130487):
    status `completed`, conclusion `failure`; `Build` failed. The first failed
    output was the `MUnique.Client.Library.dll` custom command. Its
    `dotnet publish` invocation included `-p:IlcUseEnvironmentalTools=true`;
    environmental GNU `link` then reported
    `extra operand '/DEF:obj\\Debug\\native\\MUnique.Client.Library.def'`.
  - [Quality Gates job 97592130952](https://github.com/yesid-bocanegra/MuMain/actions/runs/32777595726/job/97592130952):
    status `completed`, conclusion `failure`; `Check formatting` failed. This
    is an unrelated baseline quality failure, not the Windows build failure.
    Its first error was
    `src/source/App/Platform/Windows/Winmain.cpp:2020:44: error: code should be clang-formatted`.
- [MinGW run 32777595707](https://github.com/yesid-bocanegra/MuMain/actions/runs/32777595707):
  status `completed`, conclusion `failure`.
  - [build-mingw-x86_64 (editor-ON), job 97592129641](https://github.com/yesid-bocanegra/MuMain/actions/runs/32777595707/job/97592129641)
  - [build-mingw-i686 (editor-ON), job 97592129834](https://github.com/yesid-bocanegra/MuMain/actions/runs/32777595707/job/97592129834)
  - [build-mingw-x86_64 (editor-OFF), job 97592129858](https://github.com/yesid-bocanegra/MuMain/actions/runs/32777595707/job/97592129858)
  - [build-mingw-i686 (editor-OFF), job 97592129988](https://github.com/yesid-bocanegra/MuMain/actions/runs/32777595707/job/97592129988)

  All four jobs have status `completed`, conclusion `failure`; each failed at
  `Configure CMake`. A non-fatal LibUSB discovery warning appeared first. The
  first fatal error in every row was
  `Could NOT find OpenSSL ... (missing: OPENSSL_CRYPTO_LIBRARY Crypto)`.
- [Linux x64 run 32777595694](https://github.com/yesid-bocanegra/MuMain/actions/runs/32777595694):
  status `completed`, conclusion `success`.

All three runs target
`4f3d1e5922c99b0fe72f5fd08dd814ad13779d5a`. The Linux result does not imply
Windows build or runtime success. These statuses are baseline evidence, not
Windows-repair phase results.

### Shader artifact baseline

All six checked-in DXIL files exist with size zero:

```text
0 src/shaders/compiled/basic_colored.frag.dxil
0 src/shaders/compiled/basic_colored.vert.dxil
0 src/shaders/compiled/basic_textured.frag.dxil
0 src/shaders/compiled/basic_textured.vert.dxil
0 src/shaders/compiled/shadow_volume.vert.dxil
0 src/shaders/compiled/skinned_textured.vert.dxil
```

This is baseline failure evidence, not a valid shader manifest.

### Interactive Windows baseline: pending

No interactive Windows host is available from this macOS machine. Native MSVC
configure/build, D3D12/Vulkan launch, OS/GPU inventory, driver versions,
`MuError.log`, process exit codes, and frame captures remain pending. Do not
infer runtime success from cross-compilation or macOS tests.

Run this inventory from the architecture-matching Visual Studio Developer
PowerShell before configuration:

```powershell
cmd /c ver
where cl
cl /Bv
where link
link
where cmake
cmake --version
where ninja
ninja --version
where dotnet
dotnet --info
where dxc
dxc --version
where vulkaninfo
vulkaninfo --summary
Get-CimInstance Win32_OperatingSystem |
  Format-List Caption,Version,BuildNumber,OSArchitecture
Get-CimInstance Win32_VideoController |
  Format-Table Name,DriverVersion,AdapterRAM
```

Attempt the eight isolated native rows with these exact case values. Each row
uses the existing named preset plus a unique build directory:

| Preset | Configuration | Directory |
|---|---|---|
| `windows-x64` | `Debug` | `out/build/windows-baseline-x64-debug` |
| `windows-x64` | `Release` | `out/build/windows-baseline-x64-release` |
| `windows-x64-mueditor` | `Debug` | `out/build/windows-baseline-x64-editor-debug` |
| `windows-x64-mueditor` | `Release` | `out/build/windows-baseline-x64-editor-release` |
| `windows-x86` | `Debug` | `out/build/windows-baseline-x86-debug` |
| `windows-x86` | `Release` | `out/build/windows-baseline-x86-release` |
| `windows-x86-mueditor` | `Debug` | `out/build/windows-baseline-x86-editor-debug` |
| `windows-x86-mueditor` | `Release` | `out/build/windows-baseline-x86-editor-release` |

For each x64 row:

```powershell
cmake --preset <preset> -B <directory> `
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE="$pwd/toolchain-x64.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-windows `
  -DVCPKG_MANIFEST_MODE=ON `
  -DBUILD_TESTING=ON
cmake --build <directory> --config <configuration>
ctest --test-dir <directory> --build-config <configuration> `
  --output-on-failure --no-tests=error
```

For each x86 row, use the same commands with `toolchain-x86.cmake` and
`x86-windows`. Preserve the first configure/build/test error and do not reuse
outputs between rows.

Launch the best current x64 non-editor executable from its staged runtime
directory:

```powershell
$runtimeDirectory = '<staged-runtime-directory>'
Set-Location -LiteralPath $runtimeDirectory

Remove-Item Env:SDL_GPU_DRIVER -ErrorAction SilentlyContinue
Remove-Item -LiteralPath .\MuError.log -ErrorAction SilentlyContinue
Remove-Item -LiteralPath .\default-MuError.log -ErrorAction SilentlyContinue
Remove-Item -LiteralPath .\default-MuError.missing.txt -ErrorAction SilentlyContinue
& .\Main.exe
$defaultExitCode = $LASTEXITCODE
Set-Content -LiteralPath .\exit-default.txt -Value $defaultExitCode
if (Test-Path -LiteralPath .\MuError.log) {
    Copy-Item -LiteralPath .\MuError.log -Destination .\default-MuError.log -Force
} else {
    $missingLog = "UTC=$([DateTime]::UtcNow.ToString('o'))`nExitCode=$defaultExitCode"
    Set-Content -LiteralPath .\default-MuError.missing.txt -Value $missingLog
}

$env:SDL_GPU_DRIVER = 'direct3d12'
Remove-Item -LiteralPath .\MuError.log -ErrorAction SilentlyContinue
Remove-Item -LiteralPath .\direct3d12-MuError.log -ErrorAction SilentlyContinue
Remove-Item -LiteralPath .\direct3d12-MuError.missing.txt -ErrorAction SilentlyContinue
& .\Main.exe
$direct3d12ExitCode = $LASTEXITCODE
Set-Content -LiteralPath .\exit-direct3d12.txt -Value $direct3d12ExitCode
if (Test-Path -LiteralPath .\MuError.log) {
    Copy-Item -LiteralPath .\MuError.log -Destination .\direct3d12-MuError.log -Force
} else {
    $missingLog = "UTC=$([DateTime]::UtcNow.ToString('o'))`nExitCode=$direct3d12ExitCode"
    Set-Content -LiteralPath .\direct3d12-MuError.missing.txt -Value $missingLog
}

$env:SDL_GPU_DRIVER = 'vulkan'
Remove-Item -LiteralPath .\MuError.log -ErrorAction SilentlyContinue
Remove-Item -LiteralPath .\vulkan-MuError.log -ErrorAction SilentlyContinue
Remove-Item -LiteralPath .\vulkan-MuError.missing.txt -ErrorAction SilentlyContinue
& .\Main.exe
$vulkanExitCode = $LASTEXITCODE
Set-Content -LiteralPath .\exit-vulkan.txt -Value $vulkanExitCode
if (Test-Path -LiteralPath .\MuError.log) {
    Copy-Item -LiteralPath .\MuError.log -Destination .\vulkan-MuError.log -Force
} else {
    $missingLog = "UTC=$([DateTime]::UtcNow.ToString('o'))`nExitCode=$vulkanExitCode"
    Set-Content -LiteralPath .\vulkan-MuError.missing.txt -Value $missingLog
}
```

After each launch, preserve `MuError.log`, the exit code, whether a visible
frame appeared, and any requested diagnostic frame. These results are required
before Phase 3 or Phase 5 can be declared complete.

## Phase 1: Native MSVC and NativeAOT repair

Local implementation evidence captured through `2026-08-24T23:30:35Z` at
baseline HEAD `4f3d1e5922c99b0fe72f5fd08dd814ad13779d5a` plus the uncommitted
Phase 1 diff. No commit or push was made. Native Windows execution remains
pending; this section does not claim the Phase 1 Windows gate has passed.

### Task 1.1: Native workflow contract

The native job now has eight explicit, isolated rows:

```text
x64 Debug   editor OFF  windows-x64           out/build/windows-ci-x64-debug
x64 Release editor OFF  windows-x64           out/build/windows-ci-x64-release
x64 Debug   editor ON   windows-x64-mueditor  out/build/windows-ci-x64-editor-debug
x64 Release editor ON   windows-x64-mueditor  out/build/windows-ci-x64-editor-release
x86 Debug   editor OFF  windows-x86           out/build/windows-ci-x86-debug
x86 Release editor OFF  windows-x86           out/build/windows-ci-x86-release
x86 Debug   editor ON   windows-x86-mueditor  out/build/windows-ci-x86-editor-debug
x86 Release editor ON   windows-x86-mueditor  out/build/windows-ci-x86-editor-release
```

Static validation:

```sh
ruby -e 'require "yaml"; YAML.safe_load(File.read(".github/workflows/ci.yml"), aliases: true); puts "ci.yml YAML: OK"'
yq '.jobs."build-windows".strategy.matrix.include | length' .github/workflows/ci.yml
yq -r '.jobs."build-windows".strategy.matrix.include[] | [.architecture,.configuration,.editor,.preset,.triplet,.build_directory] | @tsv' .github/workflows/ci.yml
```

Results:

- YAML parse: `ci.yml YAML: OK`.
- Matrix length: `8`; all rows above are unique.
- `strategy.fail-fast`: `false`.
- Job permission: `contents: read`.
- Visual Studio activation uses pinned
  `ilammy/msvc-dev-cmd@0b201ec74fa43914dc39ae48a89fd1d8cb592756`
  (`v1.13.0`).
- Separate steps assert Visual Studio `cl.exe` and Microsoft `link.exe`,
  configure, assert generated compiler ID `MSVC`, build, run CTest, validate
  artifacts, then upload.
- `BUILD_TESTING=ON`, the matching preset/triplet, and one build directory per
  row are explicit.

The existing vcpkg CLI plus `VCPKG_CHAINLOAD_TOOLCHAIN_FILE` pattern was
preserved. Prior Actions logs proved that pattern reaches vcpkg and the
architecture chainload; `CMakePresets.json` did not require speculative edits.

### Task 1.2: NativeAOT tool isolation

Both Windows RID branches retain `win-x64`/`x64` and `win-x86`/`x86`, but no
longer pass `-p:IlcUseEnvironmentalTools=true`.

```sh
if rg -n 'IlcUseEnvironmentalTools' src/CMakeLists.txt; then exit 1; else echo 'IlcUseEnvironmentalTools absent'; fi
cmake --build out/build/macos-arm64 --config Debug --target test_connection_library_load -j4
ctest --test-dir out/build/macos-arm64/tests/network -C Debug \
  --output-on-failure -R 'connection|Connection|library load|Library load'
```

Results:

- Source check: `IlcUseEnvironmentalTools absent`.
- Focused loader test: 1/1 passed.
- NativeAOT PE production and architecture/load proof remain pending the x64
  and x86 MSVC Actions rows.

### Task 1.3: Current MSVC compile blockers

Confirmed blockers were repaired with the smallest source changes:

- GCC/Clang-only MiniAudio warning option is compiler-gated.
- `MuPlatform::CreateWindow` became `CreatePlatformWindow`; repository search
  found no callers, avoiding Win32 macro leakage without `#undef`.
- `CpuUsage.cpp` calls Win32 `::GetSystemInfo`.
- native Windows path conversion uses `WideCharToMultiByte`,
  `WC_ERR_INVALID_CHARS`, terminator-sized storage, and clears output on
  failure; the portable encoder rejects surrogates and scalars above
  `U+10FFFF`.
- the buff loader owns its required `WinCompat.h` include.
- ListManager and WZResult definitions match their `uint32_t`/`wchar_t`
  declarations.
- `UIPopup.cpp` performs the required explicit enum-to-`DWORD` conversion.
- both spdlog rotating sinks receive narrow `.string()` filenames, matching
  vendored spdlog's `filename_t = std::string` with
  `SPDLOG_WCHAR_FILENAMES=OFF`.

The Unicode quality correction had a real RED result: 3/4 focused cases
passed; the lone-surrogate case failed because the old portable encoder emitted
invalid UTF-8. After the correction:

```sh
cmake --build out/build/macos-arm64 --config Debug --target test_narrow_path -j4
ctest --test-dir out/build/macos-arm64/tests/core -C Debug \
  --output-on-failure -R 'Narrow paths|Wide paths|Wide-to-UTF-8'
cmake --build out/build/macos-arm64 --config Debug --target test_mu_logger -j4
ctest --test-dir out/build/macos-arm64/tests/core -C Debug \
  --output-on-failure -R '^named loggers write rotating file$'
```

Results before Task 1.4 changed the logger test registration:

- UTF-8/path cases: 4/4 passed.
- Logger filename compatibility case: 1/1 passed.
- Core CTest: 20/20 passed.
- Full macOS Debug build: exit `0`.
- Task spec review: approved with external Windows/MSVC gate pending.
- Code-quality review: approved; no remaining issue.

All current MSVC source blockers are locally addressed. Only a real MSVC build
can expose the next compiler/linker error; no claim is made that `Main.exe`
links on Windows yet.

### Task 1.4: Logger pre-init protection

A second test translation unit obtains `mu::log::Get("core")` during static
initialization. Against the old namespace-scope state, doctest discovery
aborted before test execution:

```text
libc++abi: terminating due to uncaught exception of type std::overflow_error:
__next_prime overflow
Result: Subprocess aborted
```

This reproduced access to the unconstructed `std::unordered_map`. A separate
regression mutation restored the old lazy-init early return; the focused test
then failed because the message was absent from the explicitly requested log
directory. The mutation was reverted.

The logger now owns one function-local `LoggerState`. Existing logger objects
use vendored spdlog's `dist_sink_mt`; its locked `set_sinks()` retargets an
early returned logger to the explicit directory without replacing the caller's
`shared_ptr`. Repeated explicit `Init()` remains idempotent. `Shutdown()` and
explicit reinitialization remain covered.

```sh
cmake --build out/build/macos-arm64 --config Debug --target test_mu_logger -j4
ctest --test-dir out/build/macos-arm64/tests/core -C Debug \
  --output-on-failure \
  -R 'early logger adopts explicit log directory|logger shutdown allows explicit reinitialization'
ctest --test-dir out/build/macos-arm64/tests/core -C Debug \
  --output-on-failure --no-tests=error
```

Results:

- Focused cases: 2/2 passed; implementer repetition: each case passed 20 times.
- Core CTest: 21/21 passed.
- Spec review: approved with external Windows/MSVC gate pending.
- Code-quality review initially found open-file cleanup and fixed temp-path
  collisions on Windows. Tests now close streams before deletion and suffix
  paths with the process ID. Re-review approved with no remaining issue.

### Phase 1 host regression

```sh
cmake --build out/build/macos-arm64 --config Debug -- -j1
ctest --test-dir out/build/macos-arm64 -C Debug \
  --output-on-failure --no-tests=error
git diff --check
```

Results:

- Full macOS Debug build: exit `0`.
- Full CTest: 91/91 passed in 5.61 seconds. Baseline was 87/87; the four added
  registrations are three UTF-8/path edge cases plus one additional logger
  lifecycle case.
- Existing source warnings in untouched legacy UI headers and existing local
  Homebrew deployment-target linker warnings were observed. No build/test
  failure resulted; warning comparison remains a Phase 6 audit item.
- `git diff --check`: exit `0`.

### PR 10 comparison

[Experimental PR 10](https://github.com/yesid-bocanegra/MuMain/pull/10) was
reviewed as evidence, not merged or cherry-picked:

- it has no reported status checks;
- its Windows UTF-8 helper resizes to `len - 1` then asks
  `WideCharToMultiByte` to write `len` bytes, and does not reject malformed
  UTF-16;
- its logger fix still lets early `Get()` cause later `Init(directory)` to
  return without adopting the requested directory;
- missing `dxc` still creates empty DXIL placeholders;
- shader register spaces remain incompatible with SDL_GPU;
- it does not repair the native matrix, NativeAOT environment selection,
  MinGW dependencies, pipeline fail-fast behavior, packaging, or runtime
  evidence.

Its six DXIL blobs are nonempty (`2964` to `7912` bytes) and start with `DXBC`,
but they were generated before the required shader-binding correction. They
remain comparison evidence only; Phase 3 will regenerate all formats from
corrected source.

### Phase 1 pending gate

Phase 1 is locally implemented and reviewed, not Windows-validated. After
explicit push authorization, the eight MSVC jobs must prove compiler identity,
NativeAOT x64/x86 production and loading, C++ compilation, `Main.exe` linking,
CTest, and artifacts. Interactive Windows runtime validation remains pending.

## Phase 2: MinGW target dependencies

Local workflow/toolchain evidence captured through `2026-08-25T00:09:00Z`.
The current host is macOS and does not provide the Ubuntu MinGW/Wine runner
environment. Phase 2 is implemented and reviewed locally; the four live
configure/build/link/CTest rows remain pending authorized Actions execution.

### Task 2.1: Scoped target-root extension

Both MinGW toolchains now define one optional cache path:

```cmake
set(MINGW_ADDITIONAL_TARGET_ROOT "" CACHE PATH "Additional MinGW target root")
```

When nonempty, it is appended to the architecture's existing `/usr/<triple>`
root. Program search remains `NEVER`; library, include, and package searches
remain `ONLY`. The deliberate ceiling is documented in both toolchains:

```text
ponytail: one extra target root is current ceiling; use full vcpkg
chainloading only if more target package behavior is needed.
```

Focused CMake-script probes:

```text
-- x64 default root contract OK
-- x86 extended root contract OK
```

The x64 probe supplied no extension and observed only
`/usr/x86_64-w64-mingw32`. The x86 probe supplied
`/tmp/vcpkg/x86-mingw-static` and observed the ordered pair
`/usr/i686-w64-mingw32;/tmp/vcpkg/x86-mingw-static`.

The workflow owns one classic install root:

```text
${{ github.workspace }}/_deps/vcpkg-installed
```

Each row derives the exact target prefix as
`${VCPKG_CLASSIC_INSTALL_ROOT}/${{ matrix.vcpkg_triplet }}` and passes it as
`MINGW_ADDITIONAL_TARGET_ROOT`.

After configure, a separate validation step parses `CMakeCache.txt`. It:

- checks every populated `OPENSSL_*LIBRARY*` path;
- checks populated `CURL_DIR`;
- handles semicolon lists and ignores empty/`-NOTFOUND` cache entries;
- requires at least one populated OpenSSL library and one populated
  `CURL_DIR`;
- accepts only exact children of the selected triplet prefix.

The temporary RED/GREEN contract exercised valid target paths plus `/usr`,
wrong architecture, wrong vcpkg root, missing OpenSSL, and missing curl
fixtures. RED failed because neither toolchain exposed an optional cache PATH.
GREEN passed all fixtures. The temporary contract file was removed.

Task 2.1 spec and quality reviews approved the change. Live configure was
intentionally deferred until Task 2.2 installed curl.

### Task 2.2: Complete MinGW package set

The classic install command is now:

```sh
"${VCPKG_INSTALLATION_ROOT}/vcpkg" install \
  "openssl:${{ matrix.vcpkg_triplet }}" \
  "curl[ssl]:${{ matrix.vcpkg_triplet }}" \
  --classic \
  --x-install-root="${VCPKG_CLASSIC_INSTALL_ROOT}"
```

The literal `curl[ssl]` feature is retained. The current vcpkg curl port maps
Windows SSL support to SSPI/Schannel; the separately installed OpenSSL package
continues to satisfy the client's direct Crypto dependency. No guessed manual
curl transitive libraries were added.

The cache key includes:

- the MinGW triplet;
- the actual vcpkg checkout revision from
  `git -C "${VCPKG_INSTALLATION_ROOT}" rev-parse HEAD`;
- literal `openssl` and `curl[ssl]` selections;
- `hashFiles('vcpkg.json')`.

Revision discovery fails closed if the runner's vcpkg root is not a Git
checkout or returns an empty revision. This correction came from code-quality
review; without it, classic installed ports could be restored after the runner
updated its vcpkg registry.

The obsolete `Locate OpenSSL` step and direct
`OPENSSL_INCLUDE_DIR`/`OPENSSL_CRYPTO_LIBRARY` configure overrides are removed.
`OPENSSL_USE_STATIC_LIBS=TRUE` remains. Discovery now flows only through the
selected target root and is checked immediately after configure.

Static verification:

```sh
ruby -e 'require "yaml"; YAML.safe_load(File.read(".github/workflows/windows-build.yml"), aliases: true); puts "windows-build.yml YAML: OK"'
bash -n <(yq -r '.jobs."build-mingw".steps[] | select(.run != null) | .run' .github/workflows/windows-build.yml)
if rg -n 'OPENSSL_(INCLUDE_DIR|CRYPTO_LIBRARY)=|steps\.openssl|Locate OpenSSL' .github/workflows/windows-build.yml; then exit 1; fi
git diff --check
```

Results:

- YAML parse: `windows-build.yml YAML: OK`.
- Extracted Bash steps: syntax valid.
- Manual OpenSSL discovery/overrides: absent.
- Matrix axes remain `arch: [x86_64, i686]` and `editor: [OFF, ON]`, yielding
  four rows.
- CTest remains `ctest --test-dir build-mingw --output-on-failure`; the
  toolchain/test harness supplies Wine as the cross-compiling emulator.
- Artifact names remain unique by architecture/editor state.
- Task 2.2 contract after the cache-review correction: 13/13 checks passed.
- `git diff --check`: exit `0`.
- Spec review: approved with live ordered MinGW/Actions gate pending.
- Code-quality re-review: approved; no remaining issue.

References checked during implementation:

- [vcpkg install command](https://learn.microsoft.com/en-us/vcpkg/commands/install)
- [current curl port manifest](https://github.com/microsoft/vcpkg/blob/master/ports/curl/vcpkg.json)

### Phase 2 pending gate

The baseline failure was four configure errors ending in
`Could NOT find OpenSSL ... missing: OPENSSL_CRYPTO_LIBRARY Crypto`. The next
authorized Actions run must validate in this order:

1. x86_64 and i686 editor-OFF configure, cache-root assertions, build/link,
   then CTest under Wine.
2. Only after those base rows link, assess editor-ON for each architecture.
3. Preserve any static curl transitive-link failure as the next root-cause
   input; do not mask it with guessed libraries.

Phase 2 is not declared passed until all four Actions rows are green and their
cache paths prove no host or wrong-architecture leakage.

## Phase 3: Shader generation and renderer startup

### Task 3.1: Exact shader-blob validation

`cmake/ValidateShaderBlobs.cmake` defines the six shader/stage pairs and their
exact 18-file SPIR-V/MSL/DXIL manifest. It rejects missing, empty, unexpected,
symlink, and directory entries, then checks SPIR-V magic `03 02 23 07`, DXIL
`DXBC`, and the MSL `main0` entry point.

The pre-Task-3.3 checked-in validation was deliberately RED:

```text
six empty DXIL files reported
exit 1
```

An exact synthetic 18-file fixture passed. Separate invalid-MSL and
symlink/directory/empty fixtures failed with the expected aggregated errors.
The staged-blob regression test first reproduced stale files surviving the
old glob copy, then passed after staging changed to the exact manifest.

Checked-in validation runs during configure. The exact staged manifest is
copied with `configure_file(COPYONLY)` and validated by a target that gates
`Main`. Checked-in and staged validation are also registered with CTest.
Task 3.1 spec and quality reviews approved the implementation.

Task 3.3 replaced the six empty checked-in DXIL files. Normal configure now
passes this Task 3.1 gate.

### Task 3.2: SDL_GPU binding and reflection contract

RED was reproduced from isolated `HEAD` shader sources with the new validator:

```sh
cmake \
  -DMU_SHADER_SOURCE_DIR="${RED_ROOT}/src/shaders" \
  -DMU_SHADER_REFLECTION_OUTPUT_DIR="${RED_OUTPUT}" \
  -DMU_RENDERER_SOURCE="${RED_ROOT}/src/source/Render/Renderer/MuRendererSDLGpu.cpp" \
  -DGLSLANG_EXE="$(command -v glslangValidator)" \
  -DSPIRV_CROSS_EXE="$(command -v spirv-cross)" \
  -P cmake/ValidateShaderReflection.cmake
```

Result: exit `1`. Reflection reported seven binding errors:

- vertex UBOs were in set 0 at binding 1 or 0 instead of set 1, binding 0;
- fragment texture and sampler were in set 0 instead of set 2;
- fragment UBO was in set 0 instead of set 3.

Direct HLSL semantic validation found an eighth error:
`basic_colored.vert` consumed `TEXCOORD0,2`; SDL's D3D12 input contract and
the renderer layout require sequential `TEXCOORD0,1`.

The production shader diff changes only register annotations plus that one
semantic index:

- vertex uniforms: `b0, space1`;
- skinned vertex storage remains `t0, space0`;
- fragment texture/sampler: `t0/s0, space2`;
- fragment uniform: `b0, space3`;
- colored vertex inputs: `TEXCOORD0,1`.

Shader calculations, parameter types, and output signatures remain unchanged.
No checked-in compiled blob or renderer source was modified.

GREEN used the same command against the working-tree shader sources:

```text
-- Validated SDL_GPU reflection and MSL layout for 6 shaders
exit 0
```

`cmake/ValidateShaderReflection.cmake` compiles temporary SPIR-V, writes
reflection JSON, generates temporary MSL, then verifies:

- exact stage-specific sets and bindings;
- vertex input locations and source `TEXCOORD` indices;
- the six `LoadShaders()` sampler/storage/uniform count triples;
- MSL `[[stage_in]]`, contiguous texture/sampler/buffer indices, and uniform
  buffers before storage buffers.

Negative binding, semantic, and renderer-metadata mutations each exited `1`.
Code-quality review found that clean default builds initially skipped the test
because `find_program()` received pre-defined empty variables. An isolated
probe reproduced only two blob tests. After clearing empty discovery variables,
the probe registered `shader_reflection_validation` as test 3 and ran it:

```text
1/1 Test #3: shader_reflection_validation ..... Passed
100% tests passed out of 1
```

Task 3.2 spec review approved. Code-quality re-review approved with no open
critical or important issue. `git diff --check` exits `0`.

### Task 3.3: Required DXC generation and DXIL reflection

RED reproduced both original failures before production changes:

```text
DXC discovery missing
CMakeLists.txt: cmake -E touch DXIL fallback present
ValidateShaderBlobs.cmake: six empty DXIL files
ValidateDxilReflection.cmake: six dxc -dumpbin failures, Invalid bitcode signature
```

Explicit shader compilation now discovers `dxc` with glslangValidator and
SPIRV-Cross. On Windows, PATH discovery is attempted first. If that fails,
CMake uses the active `WindowsSdkDir` plus `WindowsSDKVersion` (or the CMake
Windows target-platform version) and searches its `bin/<version>/x64`
directory. The host x64 DXC path is independent of the x64/x86 target
architecture. Missing any one of the three shader tools is fatal when
`MU_ENABLE_SHADER_COMPILATION=ON`.

The old `cmake -E touch` command is absent. Vertex shaders compile with
`vs_6_0`; fragment shaders compile with `ps_6_0`. Generated DXIL, the exact
18-file manifest, and DXIL reflection all gate an explicit-compilation build.
SPIR-V and DXIL are marked binary in `.gitattributes`; generated MSL retains
the exact SPIRV-Cross output, including its trailing blank line where emitted.

No DXIL compiler was installed on the macOS host. Regeneration used the
official Microsoft DirectXShaderCompiler `v1.9.2607` Linux x86_64 release in a
temporary Docker amd64 container. The downloaded asset SHA-256 matched the
release digest:

```text
55665c87824051ed4774ff3280a79ccbbb7d39243b9736ca5e98222134112d54
libdxcompiler.so: 1.9(1-0d3ee6b5)(1.9.0.1)
```

All 18 artifacts were generated in a temporary directory from the corrected
Task 3.2 HLSL. No PR 10 binary was copied. The temporary set passed all three
validators before any checked-in artifact was replaced:

```text
Validated 18 shader blobs
Validated SDL_GPU reflection and MSL layout for 6 shaders
Validated DXIL resource bindings for 6 shaders
```

DXIL sizes and magic after regeneration:

| Shader | Bytes | Magic |
|---|---:|---|
| `basic_colored.frag.dxil` | 2964 | `DXBC` |
| `basic_colored.vert.dxil` | 3748 | `DXBC` |
| `basic_textured.frag.dxil` | 5076 | `DXBC` |
| `basic_textured.vert.dxil` | 4892 | `DXBC` |
| `shadow_volume.vert.dxil` | 3656 | `DXBC` |
| `skinned_textured.vert.dxil` | 7916 | `DXBC` |

`dxc -dumpbin` reflection verifies the exact resource contract:

- all vertex uniform buffers: `b0, space1`;
- skinned vertex storage: `t0, space0`;
- textured fragment texture/sampler: `t0/s0, space2`;
- textured fragment uniform buffer: `b0, space3`;
- the colored fragment shader has no resource binding.

The first strict missing-tool configure used explicit glslang/SPIRV-Cross
paths with a PATH that did not contain DXC. It exited `1` at configure with:

```text
MU_ENABLE_SHADER_COMPILATION=ON requires glslangValidator, spirv-cross, and dxc.
```

Local explicit-compilation orchestration used the temporary official DXC
wrapper and exited `0`:

```sh
cmake --build out/build/phase3-task3.3-on --config Debug \
  --target ShaderCompilation ValidateStagedShaderBlobs \
  ValidateStagedDxilReflection -- -j1
```

The generated staging directory was byte-identical to all 18 checked-in
artifacts. Five focused script tests passed: checked-in/staged manifest,
SPIR-V/MSL reflection, and checked-in/staged DXIL reflection.

Code-quality review found three incremental-build defects. Empty, missing, or
directory-valued cached shader-tool paths could bypass discovery; switching an
existing build directory from checked-in blobs to explicit compilation could
reuse the copied outputs; switching back to checked-in blobs could retain two
DXIL tests through the cached `DXC_EXE` value.

Shader-tool cache entries are now discarded unless they name an existing
non-directory path, then all three discovered tools receive the same strict
check. A configured stamp records `OFF`, or `ON` plus the three resolved tool
paths. Every shader compiler command depends on that stamp and its executable,
so mode changes, tool-path changes, and executable updates invalidate the
staged artifacts. DXIL tests are registered only while explicit compilation is
enabled with a valid DXC path. DXC dump failures now include the process result,
and checked-in MSL is fixed to LF in `.gitattributes`.

Post-review targeted evidence:

```text
invalid cached GLSLANG/SPIRV-Cross/DXC paths: configure exit 1
directory GLSLANG + empty SPIRV-Cross cache: valid tools rediscovered
OFF -> ON compiler invocations: glslang 6, SPIRV-Cross 6, DXC 6
ON focused shader tests: 5/5 passed
ON staged artifacts: byte-identical 18/18
ON -> OFF DXIL tests: none registered
```

The normal macOS `MU_ENABLE_SHADER_COMPILATION=OFF` workflow does not require
DXC. Fresh current-tree results:

```text
configure: exit 0; checked-in 18-file manifest validated
Debug incremental full build: exit 0; staged 18-file manifest validated
CTest: 94/94 passed, 0 failed, 5.52 seconds
git diff --check: exit 0
```

This proves local generation, checked-in artifact consumption, and reflection
contracts. It does not prove native Windows compilation or D3D12 runtime
behavior. Native x64/x86 DXC builds and interactive D3D12/Vulkan launches
remain pending.

### Task 3.4: required pipeline fail-fast

The available PR 10 body, commits, issue comments, review comments, and commit
comments do not preserve the exact first SDL pipeline error from either the
D3D12 or Vulkan run. Those two strings remain pending interactive Windows
capture; no error text was reconstructed.

Caller audit established the required/optional boundary:

- screen-space quads use only `s_pipelines2DDepthOff`;
- world-space draws select `s_pipelines3D`, `s_pipelines3DNoCull`,
  `s_pipelines3DDepthOff`, or `s_pipelines3DDepthReadOnly`;
- `s_pipelines2D` has no draw caller and remains optional;
- `RenderSkinnedTriangles` checks a null skinned pipeline and returns `false`,
  so all four skinned sets remain optional;
- colored and shadow shaders still have no pipeline caller.

Pipeline construction now returns the created handle, the complete build
description, and the immediate `SDL_GetError()` value. Every failed build logs
this shape before any later SDL call can replace the error:

```text
SDL_gpu -- required pipeline failed: driver={} set={} layout={} blend={} index={} depth_test={} depth_write={} cull={} error={}
SDL_gpu -- optional pipeline failed: driver={} set={} layout={} blend={} index={} depth_test={} depth_write={} cull={} error={}
```

All 45 required builds are attempted, so diagnostics report every failed core
set/blend variant. `CreatePipelines()` then returns `false` with the count and
first required SDL error. `Init()` calls `DestroyPipelines()` before releasing
shaders, releasing the window, or destroying the device, which releases every
pipeline created before the failure. Optional failures remain warnings.

TDD and mutation evidence:

```text
pre-task Debug build: exit 0
pre-task CTest: 94/94 passed, 0 failed, 5.72 seconds
RED renderer_pipeline_fail_fast: failed; structured result missing
GREEN renderer_pipeline_fail_fast: 1/1 passed
mutation rejected: non-fatal required-failure return
mutation rejected: missing partial-pipeline cleanup
mutation rejected: required 2D depth-off set made optional
mutation rejected: optional skinned set made required
quad_topology_tests: 4/4 cases, 11/11 assertions
frame_pixel_readback_tests: 20/20 cases, 75/75 assertions
post-task Debug full build: exit 0
post-task CTest: 95/95 passed, 0 failed, 5.67 seconds
focused clang-format check: exit 0
```

### Phase 3 pending gate

Tasks 3.3 and 3.4 are locally implemented and validated. Phase 3 remains
incomplete until native Windows compilation proves the pipeline code on MSVC
and interactive D3D12/Vulkan runs capture the real first-error diagnostics and
successful scene output.

## Phase 4: MuEditor SDL_GPU migration

### Task 4.1: Replace the editor renderer backend

The editor-ON macOS baseline configured successfully, then failed before the
backend migration compiled:

```sh
cmake --preset macos-arm64 \
  -B out/build/windows-repair-phase4-editor-on-macos \
  -DENABLE_EDITOR=ON \
  -DOPENSSL_ROOT_DIR=/opt/homebrew/opt/openssl@3
cmake --build out/build/windows-repair-phase4-editor-on-macos \
  --config Debug --target Main -- -j4
```

Configure exited `0`. Build exited `1`. The first errors were six redundant
`extern EGameScene SceneFlag` declarations inside the `extern "C"` block in
`DevEditorUI.cpp` (lines 679, 688, 696, 751, 765, and 778), conflicting with
the existing C++ declaration in `Scenes/SceneCore.h`. Removing only those six
local declarations was the prerequisite fix.

The Task 4.1 source-contract test was deliberately RED before production
changes:

```sh
cmake -DMU_SOURCE_DIR="${PWD}/src" \
  -P tests/render/test_imgui_sdlgpu_backend.cmake
```

Result: exit `1` with
`Missing SDL_GPU editor contract symbol: imgui_impl_sdlgpu3.cpp`.

The editor now builds the vendored SDL_GPU backend, initializes both ImGui
backends from the existing SDL window/device/swapchain format, checks both init
results, starts SDL_GPU ImGui frames, and waits for GPU idle before backend
shutdown. The GL-context parameter/state and all OpenGL2 backend calls are
absent. Existing engine compatibility OpenGL system-library links remain.

Task 4.1 intentionally stops after `ImGui::Render()`. Upload and draw calls are
not issued through a second pass or submission; the existing engine-pass marker
integration remains Task 4.2.

GREEN evidence:

```text
source-contract script: exit 0
-- Validated SDL_GPU ImGui backend source contract

ctest --test-dir out/build/windows-repair-phase4-editor-on-macos \
  --build-config Debug -R '^imgui_sdlgpu_backend_contract$' \
  --output-on-failure --no-tests=error
1/1 Test #8: imgui_sdlgpu_backend_contract .... Passed
100% tests passed, 0 failed

same editor-ON reconfigure: exit 0
same Debug Main build: exit 0
post-refactor incremental Main build: exit 0
```

Quality-review mutation hardening first proved the original token-concatenation
test accepted three invalid variants with exit `0`: a required new-frame call
replaced by a comment, Task 4.2 draw/pass/submission calls inserted into the
editor, and renderer shutdown moved before editor shutdown. The hardened test
checks comment-stripped files independently. It rejects the comment mutation
and reversed shutdown with exit `1`; separate PrepareDrawData, RenderDrawData,
BeginGPURenderPass, and SubmitGPUCommandBuffer mutations each exit `1` naming
the forbidden symbol.

The build produced only unrelated existing warnings in legacy/editor source
and local macOS deployment-target linker warnings. `MuEditorCore.cpp` compiled
without a diagnostic. Native MSVC, MinGW, Linux editor-ON, interactive Windows
D3D12/Vulkan initialization, and visible ImGui rendering remain pending. Do
not infer Windows runtime success from this macOS compile gate.

## Later phase evidence

### Phase 4 Task 4.2 — editor draw-data marker replay

The Task 4.2 source contract was written before production changes. The exact
RED command was:

```sh
cmake -DMU_SOURCE_DIR="$PWD/src" \
  -P tests/render/test_imgui_sdlgpu_backend.cmake
```

It exited `1` with:

```text
Missing SDL_GPU editor contract: MuEditorCore exposes the SDL_GPU draw-data
prepare hook
```

The editor now owns prepare/render hooks using `SDL_GPUCommandBuffer*` and
`SDL_GPURenderPass*`, invalidates unconsumed draw data at the next editor frame,
queues one render-command marker after `ImGui::Render()`, and clears readiness
after marker consumption. `EndFrame()` prepares a pending marker before the
existing render pass. Replay renders the marker in vector order, then restores
the tracked viewport and scissor before later commands. The marker is a draw
batch boundary. All marker and backend integration in the renderer is guarded
by `_EDITOR`; no `MuRenderer.h` API was added.

Focused GREEN:

```text
source-contract script: exit 0
-- Validated SDL_GPU ImGui backend source contract
```

Manual source mutations all exited `1`: prepare after render-pass begin; render
outside marker replay; viewport restoration removed; scissor restoration
removed; restoration order reversed; extra render pass; extra direct frame
submission; extra fence-acquiring frame submission; ready-state clear removed;
marker changed to a non-draw command; marker replay changed from `break` to
`return`.

A reviewer follow-up exposed one production gap: the existing outer
`if (!s_texturesInvalidated)` suppressed the entire replay loop, including a
prepared editor marker. The strengthened contract was RED before the follow-up
production change. The exact focused command exited `1` with:

```text
Forbidden SDL_GPU editor symbol remains: if (!s_texturesInvalidated)
```

Replay now always processes state commands and editor markers. When textures
were invalidated, only game draw commands are skipped, so dangling texture and
sampler pointers are never used while prepared ImGui data is consumed in the
same pass. The contract also requires exactly one editor queue call, exactly
one marker vector append, scissor tracking/application, and editor-before-cursor
ordering in comment-stripped `SceneManager.cpp` and `LoadingScene.cpp` regions.

Follow-up GREEN evidence:

```text
source-contract script: exit 0
-- Validated SDL_GPU ImGui backend source contract

editor-ON Debug Main build: exit 0
editor-OFF Debug Main build: exit 0
editor-ON focused contract: 1/1 passed
editor-OFF editor_leak and contract: 2/2 passed
quad_topology_tests: 4/4 test cases, 11/11 assertions
frame_pixel_readback_tests: 20/20 test cases, 75/75 assertions
```

Eight reviewer-specific source mutations each exited `1`: restoring the outer
texture-invalidation gate; treating the editor marker as unsafe after texture
invalidation; removing the invalidated-game-draw skip; duplicating the editor
queue call; duplicating the marker vector append; removing scissor tracking;
moving the SceneManager cursor before the editor; moving the LoadingScene cursor
before the editor.

The quality follow-up first injected `g_MuEditorCore` into an outer `_EDITOR`
`#else`; the old editor-OFF extraction incorrectly passed with exit `0`. A
permanent synthetic probe then failed RED with
`Missing SDL_GPU editor contract: editor-OFF extraction retains an outer
_EDITOR #else branch`. After the minimal branch-state fix, the same source
mutation exited `1` with `Forbidden SDL_GPU editor symbol remains:
g_MuEditorCore`.

The new pending-draw helper contract failed RED with `Could not isolate the
pending editor draw-data helper`. `EndFrame()` now calls one file-local editor
helper before the sole render-pass begin; skipped invalidated game draws are not
counted as replayed, and the replay loop has no redundant scope. Direct prepare,
removed helper scan, pre-skip replay count, restored outer invalidation gate,
unsafe editor marker, duplicate queue/append, and removed scissor tracking
mutations each exited `1`.

macOS editor-ON build:

```text
cmake --build out/build/windows-repair-phase4-editor-on-macos \
  --config Debug --target Main -- -j4
exit 0
```

macOS editor-OFF isolation build:

```text
cmake --build out/build/windows-repair-baseline-macos \
  --config Debug --target Main -- -j4
exit 0
```

The editor-OFF build recompiled `MuRendererSDLGpu.cpp` without editor symbols.
Its focused CTest gate passed:

```text
editor_leak ...................... Passed
imgui_sdlgpu_backend_contract .... Passed
100% tests passed out of 2
```

Existing baseline render tests passed unchanged:

```text
quad_topology_tests: 4/4 test cases, 11/11 assertions
frame_pixel_readback_tests: 20/20 test cases, 75/75 assertions
```

The requested full editor-ON CTest command could not complete because its test
executables were not all built by the `Main` target. Building all targets then
exited `1` while linking the pre-existing `test_mu_timer` target. The first
linker error was:

```text
Undefined symbols for architecture arm64:
  "_CPU_AVG", referenced from:
      RenderDebugInfo() in libMuClient.a[236](SceneManager.cpp.o)
```

After that partial test build, the exact requested CTest command ran 87 entries:
79 executed tests passed, including `imgui_sdlgpu_backend_contract`; eight
unbuilt legacy core tests were `Not Run`. CTest reported `91% tests passed, 8
tests failed out of 87` and exited `8`. No Task 4.2 test failed.

Final controller verification after specification and quality approval reran
the current worktree rather than relying on implementer output:

```text
editor-ON Debug Main build: exit 0
editor-OFF Debug Main build: exit 0
source-contract script: exit 0
git diff --check: exit 0

quad_topology_tests: 4/4 test cases, 11/11 assertions
frame_pixel_readback_tests: 20/20 test cases, 75/75 assertions
editor-ON focused contract: 1/1 passed
editor-OFF editor_leak and contract: 2/2 passed

editor-OFF full CTest: exit 0, 96/96 passed, 0 failed, 5.68 seconds
editor-ON full CTest: exit 8, 79 executed passed, 8 unbuilt tests Not Run,
91% reported passed out of 87, 4.73 seconds
```

The eight editor-ON failures are CTest placeholder entries for binaries that
the `Main` target did not build. The focused Task 4.2 contract passed in that
same directory. The earlier all-target attempt remains blocked at the
documented pre-existing `test_mu_timer` `_CPU_AVG` link error.

Build output contained existing legacy/editor warnings and local macOS
deployment-target linker warnings. Native Windows, Linux, MSVC, MinGW, and
interactive visible-overlay behavior remain unproven by these macOS checks.

## Phase 5: Packaging and deterministic workflow validation

### Task 5.1: Stage runtime DLL closure

The existing native MSVC block copied only the local `${REQUIRED_DLLS}` list,
currently `glew32.dll`. Linked imported targets such as `SDL3::SDL3`,
`OpenSSL::Crypto`, `CURL::libcurl`, and optional `SDL3_ttf::SDL3_ttf` were not
represented by that manual list. PR 10 commit `5195a211` did not address this
runtime closure; it staged shaders and changed workflow file lists only. No PR
10 commit was merged or cherry-picked for this task.

The focused source contract was registered with CTest before the production
edit. The exact direct RED command was:

```sh
cmake -DMU_CMAKE_SOURCE="$PWD/src/CMakeLists.txt" \
  -P tests/core/test_msvc_runtime_dll_staging.cmake
```

It exited `1` for the missing production behavior, not a script error:

```text
Expected exactly one $<TARGET_RUNTIME_DLLS:Main> occurrence, found 0
```

The initial production edit added a separate `Main` `POST_BUILD` command that
copied `$<TARGET_RUNTIME_DLLS:Main>` to `$<TARGET_FILE_DIR:Main>` with
`COMMAND_EXPAND_LISTS` and `VERBATIM`. It retained the existing
`${REQUIRED_DLLS}` copy and `REQUIRED_DLL_NAMES glew32` staging.

The direct GREEN command exited `0` with:

```text
-- Validated native MSVC runtime DLL staging source contract
```

Four temporary production mutations proved the contract rejects weakened or
mis-scoped staging. Each mutation was fully reverted, followed by a direct
GREEN rerun:

```text
runtime command moved outside if (MSVC): exit 1
  $<TARGET_RUNTIME_DLLS:Main> must remain inside the manual-staging MSVC block
COMMAND_EXPAND_LISTS removed: exit 1
  Missing MSVC runtime DLL staging contract: runtime closure expands the DLL list
$<TARGET_RUNTIME_DLLS:Main> removed: exit 1
  Expected exactly one $<TARGET_RUNTIME_DLLS:Main> occurrence, found 0
manual ${REQUIRED_DLLS} copy removed: exit 1
  Could not isolate the MSVC block containing manual glew32 staging
```

Spec review found that the initial parser collected all lexical content inside
the outer `if(MSVC)`, including branches that do not execute for MSVC. Before
hardening, wrapping only the runtime command in nested `if(NOT MSVC)` while
retaining its original indentation incorrectly exited `0`. Moving the command
under the outer `else()` also incorrectly exited `0`.

The first parser hardening retained the full MSVC block for the
outside-occurrence check and separately extracted direct statements from the
outer MSVC true branch. It excluded the outer `else()`/`elseif()` and every
nested `if()` branch.

After hardening, both wrong-branch production mutations exited `1` with:

```text
$<TARGET_RUNTIME_DLLS:Main> must remain in the direct true branch of the
manual-staging if(MSVC) block
```

Both temporary mutations were fully reverted. The valid production source then
returned to direct GREEN with the unchanged success message.

A quality follow-up found that the separate transitive copy could expand to no
source arguments when a linked configuration has no runtime DLLs. That would
leave only the destination argument and make `cmake -E copy_if_different`
fail. The contract was changed first to require exactly one relevant copy
command containing both `${REQUIRED_DLLS}` and
`$<TARGET_RUNTIME_DLLS:Main>`. Against the still-split production source it
failed RED with:

```text
Expected exactly one MSVC staging copy_if_different command, found 2
```

Production now uses one `Main POST_BUILD` copy command. `${REQUIRED_DLLS}` is
first and contains the REQUIRED-found `glew32.dll`, so the command always has
at least one source. `$<TARGET_RUNTIME_DLLS:Main>` follows it. The command
retains the destination, `COMMAND_EXPAND_LISTS`, `VERBATIM`, and existing debug
echo. The duplicate custom command was deleted.

The remaining anchor-depth approximation still accepted one invalid shape:
wrapping the entire manual and transitive staging bundle in nested
`if(NOT MSVC)` incorrectly exited `0`. The final contract was written before
the production layout change. Against staging inside the large MSVC
link/configuration block it failed RED with:

```text
Manual DLL staging must be the first substantive content in a dedicated flat
if(MSVC) block
```

The large MSVC block now closes immediately after its link configuration. An
adjacent dedicated flat `if (MSVC)` block owns `DLL_SEARCH_PATH`,
`REQUIRED_DLL_NAMES glew32`, `REQUIRED_DLLS`, the `foreach()` lookup, and the
single combined `POST_BUILD` copy. It contains no nested conditional. Behavior
is unchanged: MinGW and non-MSVC builds skip the entire staging concern.

The final 89-line contract has no depth parser. It isolates the dedicated block
from its `if (MSVC)` header through the first top-level `endif()`, requires
`DLL_SEARCH_PATH` as the first substantive statement, forbids conditional
directives in the body, and validates the single combined command. Command
closing indentation is flexible; a zero-indented closing `)` mutation remained
GREEN.

Final mutation evidence:

```text
whole bundle nested in if(NOT MSVC): exit 1; dedicated block must start with staging
outer else before combined command: exit 1; dedicated block must remain flat
combined command moved outside block: exit 1; runtime expression must remain inside
split into two copy commands: exit 1; expected one copy, found 2
remove ${REQUIRED_DLLS}: exit 1; combined manual/transitive copy missing
remove $<TARGET_RUNTIME_DLLS:Main>: exit 1; expected one occurrence, found 0
remove COMMAND_EXPAND_LISTS: exit 1; combined expansion missing
```

All temporary mutations were reverted. Final retained-directory verification:

```text
direct contract: exit 0
editor-OFF Debug Main build: exit 0
editor-ON Debug Main build: exit 0
editor-OFF focused contract: 1/1 passed
editor-ON focused contract: 1/1 passed
editor-OFF full CTest: 97/97 passed, 0 failed, 5.67 seconds
editor-ON all-target/full CTest: not retried; existing _CPU_AVG blocker remains
git diff --check: exit 0
```

This macOS evidence validates source scoping, CMake registration, and
non-MSVC isolation only. A clean native Windows MSVC build,
`dumpbin /DEPENDENTS`, and proof that every non-system DLL resolves inside the
runtime directory remain pending. Task 5.1's packaging gate and package closure
are therefore not yet proven.

### Task 5.2: Validate and upload runnable directories

Local implementation evidence was captured at `2026-08-25T11:18:00Z` from
baseline HEAD `4f3d1e5922c99b0fe72f5fd08dd814ad13779d5a` plus the uncommitted
combined repair diff. PR 10 commit `5195a211` remained evidence only; it was
not merged or cherry-picked.

The root gap was in the workflow package boundary. Native validation checked
only four paths and uploaded four selected entries with
`if-no-files-found: warn`. MinGW uploaded only `Main.exe`, performed no runtime
layout or PE validation, and its artifact name did not identify MinGW or the
Release configuration. Neither workflow enforced the exact shader set.

The focused static contract was created and registered with CTest before the
workflow edit. The direct RED command was:

```sh
python3 tests/test_windows_workflow_contract.py "$PWD"
```

It exited `1` with the expected missing-behavior report, including absent PE
architecture checks, `Data/Dec2.dat`, `fonts/DejaVuSans.ttf`, exact 18-file
shader manifests, complete-directory uploads, strict missing-file handling,
NativeAOT separation, fully qualified artifact names, and the renamed release
download consumer.

Native MSVC rows now validate `Main.exe` and
`MUnique.Client.Library.dll` with `dumpbin /headers`: x64 requires PE machine
`8664 (x64)`; x86 requires `14C (x86)`. Both native and MinGW rows require
`config.ini`, `Data`, `Data/Dec2.dat`, `fonts/DejaVuSans.ttf`, `shaders`, and
set equality with the 18 checked-in shader names. The MinGW rows validate only
`Main.exe` with the matrix toolchain's `objdump`, require `pei-x86-64` plus
`i386:x86-64` for x86_64 or `pei-i386` plus `i386` for i686, and fail if a
stale `MUnique.Client.Library.dll` appears.

Uploads now point at the complete native or MinGW runtime directory and use
`if-no-files-found: error`. Native names encode native toolchain,
architecture, Debug/Release configuration, and editor state. MinGW names
encode MinGW, architecture, Release configuration, and editor state. The
semantic-release consumer now downloads the exact renamed native x64 Debug
editor-OFF artifact, preserving its previous configuration selection.

Final local static verification:

```text
direct workflow contract: exit 0
  Windows workflow contract: OK
registered focused CTest: exit 0, 1/1 passed
Ruby YAML parse, both workflows: exit 0
  Windows workflow YAML: OK
MinGW validation bash syntax: exit 0
Python contract AST parse: exit 0
actionlint: unavailable
pwsh: unavailable
```

This proves the local workflow contract, YAML structure, Bash syntax, and
upload/validation intent only. Native Windows/MSVC PowerShell execution,
MinGW Actions execution, downloaded artifact inspection, DLL closure, and
interactive launch proof remain pending. Task 5.2 does not claim live Windows
or runnable-artifact success from these macOS/static checks.

#### Task 5.2 quality-review follow-up

Review found that `build-mingw/src/` was not a clean runtime boundary: it can
contain `CMakeFiles`, static archives, `Main.map`, CTest/CMake metadata, and the
`ThirdParty` build subtree. Review also found that the first static contract
looked for steps and the release artifact token across whole workflow files and
did not require fail-closed architecture guards.

Before changing the contract, six isolated workflow mutations demonstrated the
false-GREEN behavior. Every mutation exited `0` with
`Windows workflow contract: OK`:

```text
native dumpbin exit guard changed from -ne 0 to -eq 0
native machine-count guard changed from -ne 1 to -eq 1
both MinGW architecture greps suffixed with || true
native validation/upload moved from build-windows to quality
MinGW validation/upload moved from build-mingw to a wrong job
release consumer made stale while the expected token was injected in the native job
```

MinGW now stages into a new architecture/editor-specific directory under
`RUNNER_TEMP`. The stage refuses a pre-existing destination, requires the
source and staged `Main.exe`, then recursively copies with GNU tar. The copy is
allow-by-default for future runtime files and DLLs. `--anchored` makes the
exclusions root-specific: `CMakeFiles`, `ThirdParty`, CTest/CMake install files,
root `libMuClient.a`/`libimgui.a`, root `Main.map`, and the exact Release
asset-copy stamp. Validation and upload use only this staged directory. Both
Windows upload steps explicitly include hidden files so directory upload does
not silently omit runtime content. A `ponytail:` comment records the ceiling
and install-target upgrade path.

The hardened contract isolates `build-windows`, `build-mingw`, and `release`;
requires the exact native and MinGW architecture mappings; anchors the native
`$LASTEXITCODE` and match-count guards; rejects MinGW grep bypasses; verifies
the stage's stale, source, pipeline, and output guards; and requires native
PowerShell shader enumeration with `-Force`.

After hardening, the original six mutations and five staging regressions each
exited `1` for the intended reason:

```text
inverted native dumpbin guard: rejected
inverted native machine-count guard: rejected
MinGW greps with || true: rejected
native steps in quality: rejected as missing from build-windows
MinGW steps in wrong job: rejected as missing from build-mingw
stale release plus injected token: rejected in release
disabled source Main.exe guard: rejected
inverted stale-directory guard: rejected
ignored tar-pipeline failure: rejected
disabled staged Main.exe guard: rejected
mixed build-mingw/src validation/upload without staging: rejected
```

A synthetic tar-stage check exited `0`. It retained `Main.exe`, configuration,
data/font/shader assets, a future runtime DLL, a hidden runtime file, a nested
terrain `.map`, and a nested runtime `.a`; it excluded all modeled root and
CMake/Ninja metadata. This remains local tar semantics, not downloaded
Actions-artifact evidence.

Fresh review-follow-up verification:

```text
direct workflow contract: exit 0; Windows workflow contract: OK
registered focused CTest: exit 0; 1/1 passed
all 11 final mutations: exit 1 with intended rejection
Stage runnable directory bash syntax: exit 0
Validate runnable directory bash syntax: exit 0
both Windows workflow YAML files: exit 0
Python contract compile: exit 0
synthetic MinGW staging manifest: exit 0
scoped git diff --check: exit 0
full git diff --check: exit 0
actionlint: unavailable
pwsh: unavailable
```

Native Windows PowerShell execution, MinGW Actions execution, downloaded
artifact inspection, DLL closure, and interactive launch proof remain pending.

##### Root-exact MinGW exclusions

Spec re-review found that GNU tar treats `--exclude='./*.map'` and
`--exclude='./*.a'` recursively. The repository currently contains 90 runtime
terrain `.map` files below `src/bin/Data`; the broad map exclusion would make
the MinGW artifact unrunnable.

The synthetic RED stage used the then-current wildcard exclusions. It exited
`1` after proving the bad output shape:

```text
current wildcard stage files: Main.exe
missing nested runtime fixtures: Data/World1/Terrain.map, Data/Plugins/runtime.a
```

The contract was changed before the workflow. Against the wildcard workflow it
exited `1`, reporting all four missing exact root exclusions and all three
forbidden recursive wildcard exclusions.

The workflow now excludes only exact current root build artifacts:

```text
./libMuClient.a
./libimgui.a
./Main.map
./.assets_copied_Release.stamp
```

`CMakeFiles`, `ThirdParty`, `CTestTestfile.cmake`, and `cmake_install.cmake`
remain exact path exclusions. GNU tar exclusions are unanchored by default, so
a second synthetic RED audit showed that literal root names still removed
nested same-name fixtures. The workflow now places `--anchored` before the
exclusion list. Three mutations that broadened the map, archive, or stamp
exclusion and two mutations that removed or disabled anchoring each exited `1`
with the intended contract error.

One follow-up configuration mutation initially exited `0`: changing the client
configure step to Debug still left libjpeg-turbo's unrelated Release token in
the same job. The contract now scopes this requirement to the MinGW
`Configure CMake` step. Repeating the targeted mutation exited `1` with
`MinGW exact asset-stamp exclusion requires the Release configuration`.

The final synthetic stage used GNU tar 1.35 from an already-local Debian
container. It exited `0`, retained ten nested same-name runtime fixtures, and
excluded eight exact root/build metadata fixtures:

```text
tar (GNU tar) 1.35
GNU tar root-anchored staging: OK
nested same-name runtime fixtures retained: 10
root/build metadata fixtures excluded: 8
```

The real asset inventory found 90 `.map` files, zero `.a` files, and no root
name conflict with any exclusion. Broad suffix filtering is no longer used.

Controller verification after final spec and quality approval:

```text
direct workflow contract: exit 0; Windows workflow contract: OK
both workflow YAML files: parsed successfully
editor-OFF full CTest: 98/98 passed, 0 failed, 6.16 seconds
editor-ON focused workflow contract: 1/1 passed
git diff --check: exit 0
```

These host results close Task 5.2's local static gate only. Native Windows,
MinGW Actions, artifact download inspection, and runtime proof remain pending.

### Task 5.3: Portable Windows diagnostic frame capture

Local implementation evidence was captured through `2026-08-25T12:21:45Z`
from baseline HEAD `4f3d1e5922c99b0fe72f5fd08dd814ad13779d5a` plus the uncommitted combined
repair diff. No commit, push, merge, cherry-pick, reset, or clean was performed.

The root boundary was the application bootstrap: the capture scheduler,
request, consume, and PPM writer were all inside a `_WIN32` exclusion. The GPU
readback implementation already had a portable renderer contract. Task 5.3
therefore moved only PPM validation/serialization into a focused helper,
removed only capture-related Windows guards, and retained request-before-frame
and consume-after-frame ownership in `Winmain.cpp`.

Focused baseline before edits:

```sh
cmake --build ../build-test --target frame_pixel_readback_tests -j2
../build-test/tests/render/frame_pixel_readback_tests --no-skip --no-version
ctest --test-dir ../build-test \
  -R '\[frame readback\]|\[pixel readback\]|\[diagnostic capture\]' \
  --output-on-failure
```

Results:

```text
focused build: exit 0
direct executable: 20/20 test cases, 75/75 assertions
filtered CTest: 17/17 passed, 0 failed, 0.09 seconds
```

The test and CMake registration were changed before the helper existed. The
RED build exited `1` for the intended missing production source, not a test
syntax failure:

```text
Cannot find source file:
  .../src/source/App/Platform/DiagnosticFrameCaptureWriter.cpp
No SOURCES given to target: frame_pixel_readback_tests
CMake Generate step failed.
```

The minimum production helper is one `[[nodiscard]]` free function. It rejects
null/empty paths, zero dimensions, overflow in `width * height * 3`, byte
counts that are not exactly equal, stream-size overflow, open failure, write
failure, and close failure. It emits a binary PPM with header
`P6\n<width> <height>\n255\n` followed by the RGB payload. It uses only the C++
standard library.

The test extension preserves the existing minimal renderer contract and all
prior readback/screenshot/schedule cases. New cases verify exact bytes for a
1x1 frame:

```text
50 36 0a 31 20 31 0a 32 35 35 0a 00 80 ff
 P  6 \n  1     1 \n  2  5  5 \n
```

It also verifies both zero-dimension shapes, short and long RGB payloads,
null/empty paths, and an existing temporary directory as the reliably invalid
file target. Temporary output uses the host temporary directory with a unique
timestamped filename; tests do not rely on permission or root behavior.

`Winmain.cpp` now uses the exact relative default `mu-frame.ppm`. Startup sets
the process working directory to the executable base path before the render
loop, so the default lands in the executable working directory. The fallback
condition is exactly `path == nullptr`; an existing empty `MU_CAPTURE_PATH`
value reaches the writer unchanged and fails. Without `MU_CAPTURE_FRAME`, the
schedule target remains zero, so no renderer request, consume, or file write
occurs.

The schedule is finished and renderer pixels are consumed before serialization
is attempted. A path/write failure therefore cannot leave an outstanding GPU
readback or retry the capture. Readback and write failures now use
`g_ErrorReport.Write`; that path routes through the initialized core spdlog
logger and rotating `MuError.log`, making failures available to GUI Windows
builds where `stderr` is not durable. Empty paths are logged as `<empty>`.

Fresh GREEN verification after all mutations were restored:

```text
single-config editor-OFF Debug focused build: exit 0
single-config editor-OFF Debug: 25/25 cases, 86/86 assertions

multi-config editor-OFF Debug Main + focused build: exit 0
multi-config editor-OFF Debug: 25/25 cases, 86/86 assertions

multi-config editor-ON Debug Main + focused build: exit 0
multi-config editor-ON Debug: 25/25 cases, 86/86 assertions

multi-config editor-OFF Release focused build: exit 0
multi-config editor-OFF Release: 25/25 cases, 86/86 assertions

editor-ON filtered CTest: 22/22 passed, 0 failed, 0.13 seconds
editor-OFF full CTest: 103/103 passed, 0 failed, 6.13 seconds
capture source contract: exit 0
scoped clang-format 22.1.8 checks: exit 0
```

Mutation/self-review evidence:

```text
exact byte count changed from != to <: intended mismatch test exited 1;
  a four-byte 1x1 payload incorrectly returned true and created a file
open failure changed to return true: intended existing-directory test exited 1
default changed to /tmp/mu-frame.ppm: focused source contract exited 1
empty path added to the fallback condition: focused source contract exited 1
_WIN32 guards restored around request/consume calls: focused source contract exited 1
_WIN32 guard restored around capture definitions: focused source contract exited 1
```

All temporary mutations were reverted. Close failure is not portably injectable
without adding a stream seam or platform-specific target; the final function
explicitly closes and checks stream state afterward. Exact-file bytes cover the
successful write path, and the existing-directory case covers fail-closed open
behavior without a new abstraction.

Two unrelated retained-build issues remain separate:

- `../build` has the pre-existing `MU_ENABLE_SHADER_COMPILATION=ON` cache and
  cannot regenerate because `dxc` is unavailable. Its attempted focused build
  exited `1`; no cache/source workaround was made.
- `python3 scripts/check-win32-guards.py` still exits `1` for three unrelated
  existing guards in `UI/Windows/CBTMessageBox.h`,
  `UI/Windows/CBTMessageBox.cpp`, and `Core/Input/Input.cpp`. The Task 5.3
  capture source contract confirms no Windows guard remains around owned
  capture definitions or calls.

The earlier editor-ON all-target `_CPU_AVG` link blocker was not speculatively
changed or retried as part of this focused task. Editor-ON `Main` and all
focused readback/capture tests build and pass as recorded above.

Native Windows MSVC compilation, D3D12/Vulkan launch, visible frame output,
default-path capture, custom nonempty path, empty-path `MuError.log` evidence,
and captured PPM inspection remain pending on an interactive Windows host.
These macOS results do not claim live Windows renderer or capture success.

Controller verification after spec and quality approval:

```text
editor-OFF Debug Main + focused build: exit 0
editor-ON Debug Main + focused build: exit 0
editor-OFF direct writer/readback tests: 25/25 cases, 86/86 assertions
editor-ON direct writer/readback tests: 25/25 cases, 86/86 assertions
editor-OFF full CTest: 103/103 passed, 0 failed, 6.10 seconds
git diff --check: exit 0
```

Quality review found no critical or important issue. One non-blocking coverage
note remains: the overflow guards are implemented but lack a huge-dimension,
small-payload regression case. Add it if those guards change; no production
change is justified now.

## Phase 6: Full verification and evidence

### Task 6.1: Local non-Windows regression comparison

Final local comparison was refreshed through `2026-08-25T12:56:03Z` from
baseline HEAD `4f3d1e5922c99b0fe72f5fd08dd814ad13779d5a` plus the uncommitted
combined repair diff. No commit, push, merge, reset, clean, or ignored-build
deletion was performed.

The exact retained Phase 0 host sequence was rerun after the final local
quality edits:

```sh
cmake -S . -B out/build/windows-repair-baseline-macos \
  -G 'Ninja Multi-Config' \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/macos-arm64.cmake \
  -DBUILD_TESTING=ON \
  -DENABLE_EDITOR=OFF \
  -DOPENSSL_ROOT_DIR="$(brew --prefix openssl@3)"
cmake --build out/build/windows-repair-baseline-macos --config Debug -- -j1
ctest --test-dir out/build/windows-repair-baseline-macos \
  --build-config Debug --output-on-failure --no-tests=error
```

Fresh results:

```text
configure: exit 0; checked-in 18-file shader manifest validated
full Debug build: exit 0
CTest: 103/103 passed, 0 failed, 6.36 seconds
```

Phase 0 had 87/87 tests pass with zero failures. The additional 16
registrations are the planned Windows repair contracts and focused regression
cases documented in Phases 1 through 5. No existing test regressed.

Configuration/build warnings remained environmental or pre-existing: vendored
GLM deprecation notices, HarfBuzz's Meson migration notice, unavailable
`pkg-config`, unchanged legacy compiler warnings, and Homebrew OpenSSL/turbojpeg
deployment-target linker warnings. No warning or error was attributed to a
changed line in the touched subsystems.

The Actions-equivalent quality audit initially exposed baseline debt because
the workflow checks every line of each touched source file. With exact
`clang-format 21.1.8`, 7 of the 11 tracked changed `src/source` files failed;
each of those same files also failed when its baseline-HEAD content was checked.
Local cppcheck 2.21.0, using the workflow options, reported only these two
baseline C-style casts:

```text
src/source/App/Platform/Windows/Winmain.cpp:1950 dangerousTypeCast
src/source/App/Platform/Windows/Winmain.cpp:1952 dangerousTypeCast
```

The workflow was not weakened. The seven already-touched legacy files were
mechanically formatted with `clang-format 21.1.8`; the two casts were deleted
because both source variables already have the destination pointer type. No
new abstraction, dependency, workflow behavior, or unrelated cleanup was
added.

Final quality scope included 12 tracked changed source files and the two
untracked diagnostic-writer files that will become visible to the workflow
after commit:

```text
future-CI src/source inventory: 14 files
clang-format 21.1.8 --dry-run --Werror: exit 0
cppcheck 2.21.0 with workflow options: 14/14 checked, exit 0
git diff --check: exit 0
future-CI text whitespace audit: 14/14 files clean
```

Independent spec review found the quality remediation compliant. Independent
code-quality review found no critical, important, or minor issue; the pointer
cast deletion is type-safe, macro continuations remain intact, and prior phase
edits remain present.

This closes Task 6.1's local host gate only. Native MSVC, MinGW Actions,
artifact inspection, DLL closure, and interactive D3D12/Vulkan runtime proof
remain pending. These macOS and static-analysis results do not establish that
the Windows build or renderer works.

### Task 6.4.1: Editor-ON unit-test linkage

Validation ran on `2026-08-25` using the retained macOS multi-config build
trees. Before the production edit, the editor-OFF focused control remained
green:

```sh
cmake --build out/build/windows-repair-baseline-macos \
  --config Debug --target test_mu_timer -- -j1
out/build/windows-repair-baseline-macos/tests/core/Debug/test_mu_timer \
  --no-skip --no-version
```

```text
focused build: exit 0
direct doctest: 4/4 cases, 11/11 assertions, exit 0
```

The same target reproduced the editor-ON linker failure before the edit:

```sh
cmake --build out/build/windows-repair-phase4-editor-on-macos \
  --config Debug --target test_mu_timer -- -j1
```

```text
exit 1
first undefined symbol: _CPU_AVG from SceneManager.cpp.o
additional undefined symbols: executable-bootstrap functions and globals from
  Winmain.cpp, including Destroy, RandomTable, Time_Effect, GetFPSLimit,
  DestroyWindow, g_hWnd, g_pTimer, and UI/font globals
```

The failure was not a missing `CPU_AVG` definition in timer code. Static-link
dependency tracing showed `MuTimer::LogStats()` referenced `g_ErrorReport`,
which pulled `ErrorReport.cpp`; that translation unit references the renderer,
the editor-enabled renderer references `CMuEditorCore`, and the resulting
archive extraction reaches broad game/editor objects whose executable-owned
bootstrap symbols are absent from the unit-test link.

The minimum fix removes only that unnecessary dependency. `MuTimer.cpp` now
logs directly through the existing core `MuLogger` with `MU_LOG_ERROR`.
`CErrorReport::WriteDebugInfoStr()` previously forwarded the same message to
the same core logger at error level after removing the line ending. The direct
format preserves all existing fields and values: elapsed seconds, frame count,
average/minimum/maximum frame milliseconds, hitch count, and FPS. No stub,
global move, new abstraction, or dependency was added.

Fresh focused editor-ON verification after the edit:

```sh
cmake --build out/build/windows-repair-phase4-editor-on-macos \
  --config Debug --target test_mu_timer -- -j1
out/build/windows-repair-phase4-editor-on-macos/tests/core/Debug/test_mu_timer \
  --no-skip --no-version
```

```text
focused build: exit 0
direct doctest: 4/4 cases, 11/11 assertions, exit 0
```

Fresh full editor-ON verification:

```sh
cmake --build out/build/windows-repair-phase4-editor-on-macos \
  --config Debug -- -j1
ctest --test-dir out/build/windows-repair-phase4-editor-on-macos \
  --build-config Debug --output-on-failure --no-tests=error
```

```text
all-target Debug build: exit 0; staged 18-shader manifest validated
CTest: 102/102 passed, 0 failed, 6.01 seconds
```

Fresh editor-OFF focused regression after the final source edit:

```text
focused build: exit 0
direct doctest: 4/4 cases, 11/11 assertions, exit 0
```

Final local hygiene:

```sh
clang-format-21 --dry-run --Werror src/source/Core/Time/MuTimer.cpp
git diff --check
```

```text
clang-format 21.1.8: exit 0
git diff --check: exit 0
```

The full build retained only previously observed macOS deployment-target
linker warnings for Homebrew OpenSSL and turbojpeg. This is macOS linkage and
regression evidence only. It does not establish native Windows build or
interactive D3D12/Vulkan runtime success.

### Task 6.4.2: Strict native shader regeneration

Validation ran on `2026-08-25T13:38:23Z`. Before changing the contract or
workflow, the existing static contract passed:

```sh
python3 tests/test_windows_workflow_contract.py "$PWD"
```

```text
Windows workflow contract: OK
```

The contract was then extended first. Its RED run failed only on the missing
strict-shader requirements:

```text
Windows workflow contract failed:
- Native shader compilation must be ON only for x64 Release editor-OFF
- Native matrix must contain exactly one shader-compilation ON tuple
- Native strict shader configure missing -DMU_ENABLE_SHADER_COMPILATION=${{ matrix.shader_compilation }}
- Native strict shader configure missing -DGLSLANG_EXE=C:/vcpkg/installed/${{ matrix.triplet }}/tools/glslang/glslangValidator.exe
- Native strict shader configure missing -DSPIRV_CROSS_EXE=C:/vcpkg/installed/${{ matrix.triplet }}/tools/spirv-cross/spirv-cross.exe
```

The minimum workflow edit adds an explicit `shader_compilation` value to all
eight native rows. Only `(x64, Release, editor OFF)` is `ON`; all other rows
are explicitly `OFF`. The native configure command passes that value plus the
vcpkg CLI paths for glslang and SPIRV-Cross. It does not pass `DXC_EXE`.

Fresh GREEN and syntax checks:

```text
python3 tests/test_windows_workflow_contract.py "$PWD"
Windows workflow contract: OK

PyYAML safe_load(.github/workflows/ci.yml): parsed 5 jobs
PyYAML safe_load(.github/workflows/windows-build.yml): parsed 1 job
parsed native rows: 8
parsed strict shader rows: [('x64', 'Release', 'OFF')]

git diff --check: exit 0
```

The later spec review identified that dictionary comparison could hide a
duplicate matrix tuple. An in-memory ninth-row mutation exercised the added
exact row-count guard without modifying the workflow:

```text
Synthetic RED: Native matrix must contain exactly 8 rows; found 9
```

After discarding the synthetic row, the real eight-row workflow remained
GREEN:

```text
Windows workflow contract: OK
```

The quality re-review at `2026-08-25T13:57:02Z` then closed two remaining
contract false-greens. A temporary duplicate key after the strict row's `ON`
value made YAML's effective value `OFF`; the hardened contract failed before
the workflow was restored:

```text
Windows workflow contract failed:
- Native matrix row 2 key shader_compilation must appear exactly once; found 2
- Native artifact names must encode toolchain, architecture, configuration, and editor state
- Native shader compilation must be ON only for x64 Release editor-OFF
- Native matrix must contain exactly one shader-compilation ON tuple
Duplicate-key YAML effective shader_compilation: OFF
```

Temporarily commenting all three strict configure arguments produced the
required active-argument RED:

```text
Windows workflow contract failed:
- Native strict shader configure argument must appear exactly once: -DMU_ENABLE_SHADER_COMPILATION=${{ matrix.shader_compilation }}; found 0
- Native strict shader configure argument must appear exactly once: -DGLSLANG_EXE=C:/vcpkg/installed/${{ matrix.triplet }}/tools/glslang/glslangValidator.exe; found 0
- Native strict shader configure argument must appear exactly once: -DSPIRV_CROSS_EXE=C:/vcpkg/installed/${{ matrix.triplet }}/tools/spirv-cross/spirv-cross.exe; found 0
```

A temporary duplicate active argument also failed closed:

```text
Windows workflow contract failed:
- Native strict shader configure argument must appear exactly once: -DMU_ENABLE_SHADER_COMPILATION=${{ matrix.shader_compilation }}; found 2
```

All temporary workflow mutations were removed. Fresh verification of the real
files:

```text
Windows workflow contract: OK
.github/workflows/ci.yml: YAML OK; jobs=5
.github/workflows/windows-build.yml: YAML OK; jobs=1
native rows: 8
strict rows: [('x64', 'Release', 'OFF')]
python3 -m py_compile tests/test_windows_workflow_contract.py: exit 0
git diff --check: exit 0
```

The final contract re-review at `2026-08-25T14:03:50Z` required exact values
for every matrix field and rejection of unknown keys. Temporarily changing the
strict row to `triplet: x86-windows` produced this RED:

```text
Windows workflow contract failed:
- Native matrix rows must match the exact expected values and order
```

Temporarily adding `unexpected_key: rejected` to the strict row produced this
schema RED:

```text
Windows workflow contract failed:
- Native matrix row 2 keys must equal ['architecture', 'artifact_name', 'build_directory', 'configuration', 'editor', 'preset', 'shader_compilation', 'triplet']; found ['architecture', 'artifact_name', 'build_directory', 'configuration', 'editor', 'preset', 'shader_compilation', 'triplet', 'unexpected_key']
- Native matrix rows must match the exact expected values and order
- Native artifact names must encode toolchain, architecture, configuration, and editor state
- Native shader compilation must be ON only for x64 Release editor-OFF
- Native matrix must contain exactly one shader-compilation ON tuple
```

Both temporary mutations were removed. Fresh restored-file verification:

```text
Windows workflow contract: OK
.github/workflows/ci.yml: YAML OK; jobs=5
.github/workflows/windows-build.yml: YAML OK; jobs=1
native rows: 8
strict rows: [('x64', 'Release', 'OFF')]
python3 -m py_compile tests/test_windows_workflow_contract.py: exit 0
git diff --check: exit 0
```

The matrix values are quoted YAML strings, so `ON` and `OFF` cannot be
coerced to booleans. GitHub expands each matrix expression before the default
Windows PowerShell shell invokes the folded configure command; the added
paths contain no spaces or PowerShell metacharacters. vcpkg manifest mode is
already enabled and `vcpkg.json` already provides `glslang` and
`spirv-cross`, so no dependency or install step was added. The vcpkg
toolchain processes the manifest during `project()` before the top-level
strict tool existence checks run.

DXC remains fail-closed in CMake. The existing MSVC developer-environment
step supplies `WindowsSdkDir` and `WindowsSDKVersion`; strict CMake discovery
uses those values to search the active Windows SDK. The contract rejects any
`DXC_EXE` override in either Windows workflow.

No push or Actions run was authorized. This static contract and YAML evidence
does not establish that the Windows SDK contains DXC, shader regeneration
succeeds on Windows, or either interactive renderer works.

### Task 6.4.3: Native runtime DLL import closure

Validation ran on `2026-08-25T14:24:37Z`. Before the contract changes, both
existing static checks passed:

```text
Windows workflow contract: OK
-- Validated native MSVC runtime DLL staging source contract
```

The CMake contract was extended first. Its RED run rejected the obsolete
manual runtime staging at the first violation:

```text
Forbidden MSVC runtime DLL staging contract: manual DLL_SEARCH_PATH staging
```

The workflow contract was also extended first. Its RED run reported the
missing recursive PE enumeration, `/DEPENDENTS` invocation and immediate exit
guard, dependency-heading parser, root-local lookup, API-set exceptions,
architecture-specific system-directory lookup, aggregated missing-import
failure, and fixed two-file PE scan.

The minimum CMake change removes manual `DLL_SEARCH_PATH`,
`REQUIRED_DLL_NAMES`, `REQUIRED_DLLS`, and `glew32` staging. MSVC retains one
`Main` `POST_BUILD` copy using `$<TARGET_RUNTIME_DLLS:Main>`,
`COMMAND_EXPAND_LISTS`, and `VERBATIM`. `CopyRuntimeAssets.cmake` removes stale
runtime copies corresponding to `src/bin/*.dll`, then copies the asset tree
with `*.dll` excluded. It does not delete or modify the source DLLs. The
targeted removal avoids deleting valid linked-target DLLs when an asset-only
rebuild does not relink `Main`.

The native workflow now enumerates every staged `.exe` and `.dll` recursively,
sorts them, rejects an empty set, validates each PE against the matrix
architecture, and runs `dumpbin /DEPENDENTS` on each PE. Imports resolve only
against root-level runtime files, API sets, or the target architecture's
Windows system directory: x64 uses `System32` (`Sysnative` from a 32-bit
process); x86 uses `SysWOW64` on a 64-bit OS and `System32` otherwise. It does
not resolve imports through `PATH`, Visual Studio, or vcpkg. Missing imports
are aggregated before the final throw.

Temporary mutations produced the required RED evidence, then were restored:

```text
remove /DEPENDENTS:
- Native validation missing /DEPENDENTS $binary
- Native dependency dump must fail closed immediately on nonzero exit
- Native validation must check architecture and imports for every staged PE

ignore dumpbin exit:
- Native dependency dump must fail closed immediately on nonzero exit

scan only Main.exe:
- Native validation missing Get-ChildItem -LiteralPath $runtimeDirectory -File -Recurse
- Native required-file validation must not hard-code the PE scan

x86 uses System32 on a 64-bit OS:
- Native validation missing SysWOW64
- Native system-directory selection must follow the target architecture

resolve an import with Get-Command/PATH:
- Native dependency validation must not resolve imports through developer paths: Get-Command $importName

remove aggregate final throw:
- Native validation missing throw "Missing runtime imports:

skip DLL architecture/import validation:
- Native validation must check architecture and imports for every staged PE

remove the asset-copy DLL exclusion:
Missing MSVC runtime DLL staging contract: asset staging excludes DLLs from the bulk copy
```

Both preserved macOS build trees were reconfigured, then `Main` was rebuilt so
the changed asset staging command executed:

```sh
cmake --build out/build/windows-repair-baseline-macos \
  --config Debug --target Main -- -j1
cmake --build out/build/windows-repair-phase4-editor-on-macos \
  --config Debug --target Main -- -j1
```

```text
editor-OFF Main build: exit 0
editor-ON Main build: exit 0
editor-OFF runtime DLL count: 0
editor-ON runtime DLL count: 0
source DLLs retained: 3/3
```

The editor-OFF link retained only the previously observed macOS deployment
target warnings for Homebrew OpenSSL and turbojpeg. No native Windows command,
Actions job, or interactive D3D12/Vulkan runtime was run. This evidence does
not establish Windows build or runtime success; Actions must validate the real
CRT and third-party import closure.

Phase 6.4.3 quality follow-up ran on `2026-08-25T14:44:04Z`. The workflow
contract was tightened before the workflow edit. Its RED run identified both
hidden-PE enumeration gaps and every missing non-leaf import-name guard:

```text
- Native validation missing Get-ChildItem -LiteralPath $runtimeDirectory -Force -File -Recurse
- Native validation missing Get-ChildItem -LiteralPath $runtimeDirectory -Force -File |
- Native validation missing $importFileName = [IO.Path]::GetFileName($importName)
- Native validation missing [IO.Path]::IsPathRooted($importName)
- Native validation missing $importName.Contains('/')
- Native validation missing $importName.Contains('\')
- Native validation missing [StringComparer]::Ordinal.Equals($importFileName, $importName)
- Native validation missing $missingImports += "$($binary.FullName): invalid import name: $importName"
- Native dependency validation must aggregate and reject every non-leaf import name
- Native non-leaf import rejection must precede root, API-set, and system allowlists
```

The minimum workflow edit adds `-Force` to both the root-level runtime map and
recursive `.exe`/`.dll` enumeration, matching the hidden-file artifact upload.
Each parsed import is now rejected and aggregated before any root, API-set, or
system allowlist when it is rooted, contains `/` or `\`, or differs from
`[IO.Path]::GetFileName(...)`. This prevents rooted or path-shaped names from
escaping `systemDirectory` through `Join-Path` or bypassing validation through
an API-set prefix.

Temporary mutations produced the expected REDs, then were restored:

```text
remove -Force from both PE enumerators:
- Native validation missing Get-ChildItem -LiteralPath $runtimeDirectory -Force -File -Recurse
- Native validation missing Get-ChildItem -LiteralPath $runtimeDirectory -Force -File |

remove non-leaf import rejection:
- Native validation missing all rooted, separator, GetFileName, and aggregate guards
- Native dependency validation must aggregate and reject every non-leaf import name
- Native non-leaf import rejection must precede root, API-set, and system allowlists

move non-leaf rejection after root/API allowlists:
- Native non-leaf import rejection must precede root, API-set, and system allowlists
```

No Windows or Actions execution was authorized. These static checks do not
establish native Windows build, import closure, or interactive renderer
success.

### Task 6.4.4: Shipped shader reflection

Validation ran on `2026-08-25T14:55:37Z`. Before registration, the retained
editor-OFF and editor-ON macOS trees each passed the existing focused control:

```text
checked_in_shader_blob_validation: passed
staged_shader_blob_validation: passed
shader_reflection_validation: passed
3/3 tests passed in each tree
```

The two shipped reflection tests were registered before validator changes.
After reconfiguring both trees, both tests failed for the intended missing mode:

```text
checked_in_shader_reflection_validation: MU_SHADER_SOURCE_DIR is required
staged_shader_reflection_validation: MU_SHADER_SOURCE_DIR is required
0/2 tests passed in each tree
```

`ValidateShaderReflection.cmake` now selects blob mode only when
`MU_SHADER_BLOB_DIR` is supplied. Blob mode reads each shipped `.spv` and its
paired shipped `.msl`, then applies the existing SPIR-V binding, vertex input,
renderer metadata, and MSL index/order checks. It does not require glslang,
create a reflection output directory, compile HLSL, write reflection JSON, or
generate MSL. Source mode retains HLSL semantic validation, temporary SPIR-V
compilation, reflection JSON output, and temporary MSL generation.

CTest gating matches those dependencies:

- `checked_in_shader_reflection_validation` and
  `staged_shader_reflection_validation` require only `spirv-cross`;
- `shader_reflection_validation` requires both glslang and `spirv-cross`.

Mutation proof used SPIRV-Tools `v2026.3` and only an isolated copy created by:

```sh
MUTATION_DIR=$(mktemp -d "${TMPDIR%/}/mu-shader-reflection.XXXXXX")
cp src/shaders/compiled/* "${MUTATION_DIR}/"
spirv-dis "${MUTATION_DIR}/basic_textured.frag.spv" \
  -o "${MUTATION_DIR}/basic_textured.frag.spvasm"
```

The copied assembly changed only the texture descriptor set before reassembly:

```diff
-               OpDecorate %tex DescriptorSet 2
+               OpDecorate %tex DescriptorSet 1
```

The mutation remained valid SPIR-V, then failed the shipped validator:

```sh
spirv-as "${MUTATION_DIR}/basic_textured.frag.spvasm" \
  -o "${MUTATION_DIR}/basic_textured.frag.spv"
spirv-val "${MUTATION_DIR}/basic_textured.frag.spv"
cmake \
  -DMU_SHADER_BLOB_DIR="${MUTATION_DIR}" \
  -DMU_RENDERER_SOURCE="${PWD}/src/source/Render/Renderer/MuRendererSDLGpu.cpp" \
  -DSPIRV_CROSS_EXE="$(command -v spirv-cross)" \
  -P cmake/ValidateShaderReflection.cmake
```

```text
spirv-as: exit 0
spirv-val: exit 0
validator: exit 1
- basic_textured.frag: separate_images[0] set/binding 1/0, expected 2/0
```

After restoring only the copied SPIR-V with
`cp src/shaders/compiled/basic_textured.frag.spv
"${MUTATION_DIR}/basic_textured.frag.spv"`, the copied paired MSL changed one
texture index:

```diff
-texture2d<float> tex [[texture(0)]]
+texture2d<float> tex [[texture(1)]]
```

The same blob-mode validator exited `1` with:

```text
- basic_textured.frag: non-contiguous MSL texture index at [[texture(1)]]
```

The mutation directory cleanup used only the resolved temporary location:

```sh
test -d "${MUTATION_DIR}"
test ! -L "${MUTATION_DIR}"
cmake -E remove_directory "${MUTATION_DIR}"
test ! -e "${MUTATION_DIR}"
```

Cleanup exited `0`; no tracked shader file was edited by either mutation.

Fresh focused verification after the implementation:

```text
editor-OFF: 5/5 tests passed
editor-ON: 5/5 tests passed
editor-OFF staged blobs: 18/18 byte-identical to checked-in blobs
editor-ON staged blobs: 18/18 byte-identical to checked-in blobs
```

No Windows command, push, Actions job, or interactive renderer run was
performed. This macOS artifact/reflection evidence does not establish native
Windows build, D3D12/Vulkan runtime, or interactive rendering success.

Phase 6.4.4 quality follow-up ran on `2026-08-25T15:11:46Z`. RED used a new
temporary copy of all 18 blobs. The copied `basic_colored.frag.spv` assembly
changed its entry point from fragment to vertex and removed the now-invalid
fragment execution mode:

```diff
-               OpEntryPoint Fragment %main "main" %input_color %_entryPointOutput
-               OpExecutionMode %main OriginUpperLeft
+               OpEntryPoint Vertex %main "main" %input_color %_entryPointOutput
```

Reassembly and validation both exited `0`:

```sh
spirv-as "${MUTATION_DIR}/basic_colored.frag.spvasm" \
  -o "${MUTATION_DIR}/basic_colored.frag.spv"
spirv-val "${MUTATION_DIR}/basic_colored.frag.spv"
spirv-cross "${MUTATION_DIR}/basic_colored.frag.spv" --reflect
```

Reflection reported exactly one `main` entry point with mode `vert`, but the
pre-fix shipped validator incorrectly exited `0`. After restoring the copied
SPIR-V, changing only the copied paired MSL qualifier also incorrectly exited
`0`:

```diff
-fragment main0_out main0(main0_in in [[stage_in]])
+vertex main0_out main0(main0_in in [[stage_in]])
```

The minimum validator addition is one shared helper used after both shipped and
source-regenerated reflection. It requires exactly one SPIR-V entry point named
`main` with the shader spec's `vert` or `frag` mode, and requires the paired or
generated MSL `main0` declaration to use the matching `vertex` or `fragment`
qualifier. Failures use the existing aggregated error list.

GREEN repeated both isolated mutations. The valid SPIR-V mutation exited `1`:

```text
- basic_colored.frag: SPIR-V entry point main/vert, expected main/frag
```

The MSL mutation exited `1`:

```text
- basic_colored.frag: MSL main0 entry point stage vertex, expected fragment
```

The retained editor-OFF and editor-ON trees each passed all five focused shader
blob/reflection tests after the change, including source regeneration (`5/5`
per tree). The temporary mutation directory was verified as non-symlink,
removed with `cmake -E remove_directory`, then verified absent. No tracked
shader blob was edited. No Windows command, push, Actions job, or interactive
renderer run was performed; this remains macOS/static validation only.

### Phase 6.4.5: pipeline shader identifiers

Phase 6.4.5 ran on `2026-08-25T15:23:03Z`. The existing direct pipeline
contract exited `0` before the new assertion. RED then required the intended
pipeline shader names inside `LogPipelineFailure()` and tied the diagnostic
mapping to the `BuildBlendPipeline()` shader-handle selection. This initial
contract did not bind those handles to their `LoadShaders()` asset assignments;
the quality follow-up below closes that gap. The direct run and both retained
multi-config trees failed exactly one test each (`0/1`):

```text
pipeline diagnostics must map the built vertex layout to its exact shader identifier
```

The renderer change derives the vertex identifier from
`PipelineBuildDescription::vertexLayout`: 2D and 3D layouts report
`basic_textured.vert`; the skinned layout reports `skinned_textured.vert`.
Every pipeline reports `basic_textured.frag`. Required errors and optional
warnings preserve the existing driver, set, layout, blend, index, depth, cull,
and first SDL error fields while adding `vertex_shader={}` and
`fragment_shader={}` with their argument values.

GREEN passed the direct contract (`1/1`). The retained editor-OFF and editor-ON
trees each passed these six CTest entries:

```text
renderer_pipeline_fail_fast
checked_in_shader_blob_validation
staged_shader_blob_validation
checked_in_shader_reflection_validation
staged_shader_reflection_validation
shader_reflection_validation
```

Each tree reported `6/6` passed. Focused executable results in each tree were:

```text
quad_topology_tests: 4/4 cases, 11/11 assertions
frame_pixel_readback_tests: 25/25 cases, 86/86 assertions
```

The exact `MuRendererSDLGpu.cpp` RelWithDebInfo object target compiled in both
retained trees. At this intermediate point, full editor-OFF and editor-ON
`Main` builds each stopped before renderer compilation in the NativeAOT custom
command with:

```text
ld: library 'brotlienc' not found
```

Object-level compilation was therefore the available evidence at that point.
The final controller reruns below later linked both full applications, showing
that this `brotlienc` failure was transient pre-existing environment state, not
a remaining build blocker.

Mutation proof copied the renderer to a new temporary directory and changed
only the copied skinned diagnostic mapping from `skinned_textured.vert` to
`basic_textured.vert`. The contract exited `1` with the same exact mapping
failure shown in RED. The tracked renderer SHA-256 was identical before and
after mutation:

```text
09fd34c88f77112b4c7fa9743c9685b3daed5ebd51e666dcba5c3931a3e3d5c1
```

The temporary directory was verified as a non-symlink, removed with
`cmake -E remove_directory`, then verified absent. `clang-format-21` formatted
only the changed C++ lines; `git diff --check` exited `0`.

No Windows command, push, Actions job, or interactive renderer run was
performed. This macOS compile/static-contract evidence does not establish
native Windows build, D3D12/Vulkan pipeline creation, or visible rendering
success.

Phase 6.4.5 quality follow-up ran on `2026-08-25T15:37:47Z`. RED copied the
renderer to a new temporary directory and changed only this executable shader
load assignment:

```diff
 s_vertShaderSkinned =
-    createShader("skinned_textured", "vert", SDL_GPU_SHADERSTAGE_VERTEX, ...);
+    createShader("basic_textured", "vert", SDL_GPU_SHADERSTAGE_VERTEX, ...);
```

The shader handle, pipeline layout selection, and logged diagnostic name stayed
unchanged. The pre-follow-up contract incorrectly exited `0`, proving it could
report `skinned_textured.vert` while the handle actually loaded another asset.

The contract now isolates `LoadShaders()` between its declaration and
`ReleaseShaders()`, strips line and block comments, then requires these exact
executable handle/asset/stage bindings:

```text
s_vertShader2D <- basic_textured / vert / SDL_GPU_SHADERSTAGE_VERTEX
s_fragShaderTex <- basic_textured / frag / SDL_GPU_SHADERSTAGE_FRAGMENT
s_vertShaderSkinned <- skinned_textured / vert / SDL_GPU_SHADERSTAGE_VERTEX
```

GREEN rejected the same temp-copy mutation with exit `1` and:

```text
s_vertShaderSkinned must load skinned_textured.vert
```

The tracked source passed directly (`1/1`); retained editor-OFF and editor-ON
`renderer_pipeline_fail_fast` each passed (`1/1`). The renderer file was not
edited during this follow-up; its SHA-256 remained:

```text
09fd34c88f77112b4c7fa9743c9685b3daed5ebd51e666dcba5c3931a3e3d5c1
```

The temporary directory was verified as a non-symlink, removed with
`cmake -E remove_directory`, then verified absent. No Windows command, push,
Actions job, or interactive renderer run was performed.

## Final local-controller verification

Fresh controller verification ran on `2026-08-25` after Phases 6.4.3 through
6.4.5.

```text
editor-OFF Debug full build: exit 0; Main linked
editor-OFF full CTest: 105/105 on the verbose run; quiet rerun exit 0
editor-ON Debug full build: exit 0; Main linked
editor-ON full CTest: 104/104 on the verbose run; quiet rerun exit 0
```

The commands were:

```sh
cmake --build out/build/windows-repair-baseline-macos \
  --config Debug -- -j1
ctest --test-dir out/build/windows-repair-baseline-macos \
  --build-config Debug --output-on-failure --no-tests=error
cmake --build out/build/windows-repair-phase4-editor-on-macos \
  --config Debug -- -j1
ctest --test-dir out/build/windows-repair-phase4-editor-on-macos \
  --build-config Debug --output-on-failure --no-tests=error
```

Final deterministic checks:

```text
future-CI source inventory: 14 files (12 tracked changed plus
  DiagnosticFrameCaptureWriter.cpp/.h)
clang-format 21.1.8: exit 0
cppcheck 2.21.0 with exact workflow flags: 14/14 checked, exit 0
Windows workflow contract: OK
MSVC runtime DLL staging source contract: OK
.github/workflows/ci.yml and windows-build.yml: YAML parsed
python3 -m py_compile: exit 0
ImGui SDL_GPU contract: OK
renderer pipeline contract: OK
git diff --check: exit 0
future-CI text whitespace audit: 14/14 clean
```

Artifact and source inventory:

```text
checked-in blob validation: 18/18
editor-OFF staged blob validation: 18/18
editor-ON staged blob validation: 18/18
checked-in reflection: 6/6 shaders
editor-OFF staged reflection: 6/6 shaders
editor-ON staged reflection: 6/6 shaders
source-regeneration reflection: 6/6 shaders
editor-OFF staged artifacts byte-identical to checked-in: 18/18
editor-ON staged artifacts byte-identical to checked-in: 18/18
src/bin DLLs: exactly glew32.dll, vorbisfile.dll, ogg.dll
editor-OFF macOS runtime DLL count: 0
editor-ON macOS runtime DLL count: 0
.spv/.dxil diff attribute: unset (binary)
```

Production/editor source contains no OpenGL2 backend calls. The audited tree
also contains no `IlcUseEnvironmentalTools`, DXIL touch placeholder, or MinGW
direct OpenSSL override. Local DXC remains unavailable, so fresh local DXIL
reflection was not run; the strict Windows shader-regeneration row is the real
gate.

`scripts/check-win32-guards.py` still exits `1` only on the same three
pre-existing unrelated guards:

```text
src/source/UI/Windows/CBTMessageBox.h
src/source/UI/Windows/CBTMessageBox.cpp
src/source/Core/Input/Input.cpp
```

No Windows command, Actions run, push, or interactive D3D12/Vulkan validation
was performed. These results close the original five local final-audit
remediations; complete-diff final reviews, GitHub Actions, Windows artifact
inspection, and interactive Windows gates remain pending. They do not
establish that Windows builds or renderers work.

### Final-review workflow remediation

The workflow contract was extended before production edits. Its RED output
identified the missing Linux editor-OFF/editor-ON matrix and isolated paths,
stale `basic_textured.vert` binding comment, native Release-only artifact gates
and debug CRT rejection, and recursive MinGW import closure.

Linux CI now has exact editor-OFF/editor-ON rows with isolated build
directories, dependency caches, artifact names, configure/build commands, and
CTest execution. The renderer comment now records the actual
`b0, space1 (Transform)` vertex uniform binding.

Native Debug rows remain compile/CTest-only. Runtime validation and upload run
only for Release rows; semantic release downloads native x64 Release
editor-OFF. Before root, API-set, or system lookup, Release import closure
aggregates and rejects debug UCRT, VCRuntime, MSVC++, Concurrency Runtime, and
OpenMP DLL forms.

MinGW staging iterates recursive `.exe`/`.dll` imports to convergence. It
copies only `libgcc_s_*`, `libstdc++-6`, or `libwinpthread-1` imports resolved
by `${triple}-g++ -print-file-name`. Validation architecture-checks every
staged PE, reruns `objdump -p`, rejects non-leaf imports, accepts root-local
files and API sets, and uses an explicit reviewed Windows system-DLL allowlist.
Unknown imports are aggregated and fail the job. NativeAOT remains explicitly
absent from MinGW artifacts.

Mutation RED proof:

```text
remove Linux editor-ON row:
- Linux matrix must contain exact isolated editor-OFF/ON rows

allow native Debug upload:
- Native runnable artifacts must upload only from Release rows

remove Release debug CRT rejection:
- Native Release closure must aggregate and reject debug CRT imports
- Native non-leaf import rejection must precede root, API-set, and system allowlists

remove MinGW closure invocation:
- MinGW staging must execute compiler-runtime closure

change libwinpthread-1.dll to libwinpthread-2.dll:
- MinGW staging missing libwinpthread-1.dll
- MinGW staging must iterate recursive compiler-runtime closure to convergence

resolve through Wine system32:
- MinGW closure must not resolve imports through PATH or Wine directories: drive_c/windows/system32
```

Controller mutation follow-up found two contract false-greens: MinGW
validation could enumerate only the runtime root, or only `.exe` files, while
the workflow contract still passed. The contract was hardened before any
production change. Both mutations now fail with:

```text
MinGW validation must recursively enumerate every staged EXE and DLL
```

The restored workflow still passes. This follow-up changed only the contract;
the production workflow already used recursive `.exe`/`.dll` enumeration.

Fresh restored-file checks:

```text
Windows workflow contract: OK
.github/workflows/ci.yml: YAML OK; jobs=5
.github/workflows/windows-build.yml: YAML OK; jobs=1
MinGW stage/validation Bash syntax: OK
python3 -m py_compile tests/test_windows_workflow_contract.py: exit 0
git diff --check: exit 0
```

No push or Actions run was authorized. Static workflow contracts and YAML/bash
syntax do not establish Linux, native Windows, MinGW, artifact, or interactive
renderer success.

#### Native artifact uploader identity and uniqueness contract

The final spec review identified that checking only the step named
`Upload artifact` could miss a second, differently named Debug uploader. The
expected Release step remains `actions/upload-artifact@v4`; independently, the
workflow contract requires exactly one `actions/upload-artifact@<ref>` action
inside `build-windows`. A different action version cannot bypass the
Release-only rule.

The contract also requires exactly one uploader inside the isolated
Release-gated `Upload artifact` step. Otherwise, replacing that step's action
with `actions/cache@v4` while adding the sole uploader to a Debug-only step
would preserve the job-wide count but publish the wrong configuration.

A temporary `Upload Debug diagnostics` step produced the required RED:

```text
Windows workflow contract failed:
- Native job must contain exactly one artifact upload action; found 2
```

The mutation was removed. Debug remains compile/CTest-only; the sole native
uploader remains Release-gated. No Actions run or artifact execution was
performed.

The same mutation using `actions/upload-artifact@v5` produced the identical
`found 2` RED and was also removed.

A second temporary mutation replaced the Release step's uploader with
`actions/cache@v4` and added a Debug-only `actions/upload-artifact@v5` step.
The new step-association check produced the required RED while the older
job-wide count remained one:

```text
Windows workflow contract failed:
- Native Release-gated upload step must contain exactly one artifact upload action; found 0
```

The mutation was removed. The restored workflow contract passes.

#### Debug CRT family and compiler lookup hardening

The native Release debug-CRT contract now compiles the workflow regex and
checks concrete names. The prior numeric/underscore-only MSVC++ family pattern
produced RED for the real auxiliary debug CRT DLLs:

```text
Windows workflow contract failed:
- Native Release closure must reject debug CRT import msvcp140_atomic_waitd.dll
- Native Release closure must reject debug CRT import msvcp140_codecvt_idsd.dll
```

The minimal family pattern now accepts optional alphanumeric/underscore
suffixes before the final debug `d`, while concrete negative samples keep
`ucrtbase.dll`, `vcruntime140.dll`, and `msvcp140.dll` allowed.

The MinGW contract now requires exactly one executable `compiler_path=`
assignment: the target `${triple}-g++ -print-file-name=${import_name}` lookup.
A temporary effective override after that line produced RED:

```text
Windows workflow contract failed:
- MinGW closure must have exactly one executable compiler_path assignment from target g++
- MinGW closure must not resolve imports through PATH or Wine directories: which --
```

Both mutations were removed. No Actions or Windows execution was performed.

### Final-review native CRT and release-archive remediation

Two Important final-review findings remained:

1. Native validation accepted any imported DLL present in the selected Windows
   system directory, even when that DLL was not a reviewed Windows OS runtime.
2. Semantic release downloaded the native x64 Release editor-OFF artifact but
   published only `artifacts/Main.exe` under stale MinGW i686 metadata.

The workflow contract was extended before production edits. Its baseline RED
reported the missing MSVC CRT staging step, explicit Windows OS DLL allowlist,
root-local release CRT guard, archive step, exact release asset metadata, and
download/archive/publish ordering.

Native Release rows now require `VCToolsRedistDir` from the active MSVC
developer environment. Staging selects the matrix architecture's immediate
directory, requires exactly one `Microsoft.VC*.CRT` child, requires at least
one DLL, then copies all DLLs root-local before validation. It does not search
`PATH`, recurse, or inspect unrelated Visual Studio directories. Missing or
ambiguous active redist paths fail closed.

Import validation still rejects non-leaf and Debug CRT imports before every
allowlist. Root-local imports follow. Missing release `vcruntime`, `msvcp`,
`concrt`, and `vcomp` families fail with a root-local staging error. API sets
remain allowed. Other Windows OS imports require both membership in the exact
reviewed allowlist and existence in the architecture-correct `System32`,
`Sysnative`, or `SysWOW64` directory. System-directory presence alone cannot
authorize an import.

Semantic release now downloads the complete native x64 Release editor-OFF
runtime, runs exactly:

```sh
tar -czf MuMain-windows-native-x64-release-editor-off.tar.gz -C artifacts .
```

The GitHub release plugin publishes only that archive, using the same native
x64 Release editor-OFF filename and label. A synthetic archive check retained
visible, hidden, and nested runtime files.

Mutation RED proof:

```text
replace allowlist-gated target-system lookup with unrestricted target-system lookup:
- Native validation missing $windowsOsDllAllowlist.ContainsKey($importName)
- Native target system lookup must be nested under the explicit OS DLL allowlist

remove CRT copy:
- Native CRT staging missing Copy-Item -LiteralPath $crtDlls.FullName -Destination $runtimeDirectory -Force

allow multiple Microsoft.VC*.CRT directories:
- Native CRT staging missing if ($crtDirectories.Count -ne 1)

omit vcomp from the root-local VC runtime family:
- Native Release closure must require root-local vcomp140.dll

restore Main.exe / MinGW i686 release publishing:
- Release must archive the complete artifacts directory with the exact native x64 output name
- Semantic release must publish exactly the native x64 Release editor-OFF runtime archive
- Release config must not retain stale asset metadata: artifacts/Main.exe
- Release config must not retain stale asset metadata: MuMain-mingw-i686.exe
- Release config must not retain stale asset metadata: MinGW i686
```

Fresh restored-file checks:

```text
Windows workflow contract: OK
python3 -m py_compile tests/test_windows_workflow_contract.py: exit 0
python3 -m json.tool .releaserc.json: exit 0
.github/workflows/ci.yml: YAML OK; jobs=5
.github/workflows/windows-build.yml: YAML OK; jobs=1
Relevant Bash syntax: OK
Release archive includes visible, hidden, nested runtime files: OK
git diff --check: exit 0
Touched untracked-file whitespace: OK
```

`pwsh` is unavailable on the macOS controller, so the new PowerShell staging
and validation paths require Windows Actions execution. No push, Actions run,
artifact inspection, or interactive D3D12/Vulkan validation was performed.

### Final controller verification after 6.4.10

Fresh controller verification ran after the final specification and quality
reviews and after every workflow/contract/documentation edit:

```text
editor-OFF Debug full build: exit 0; Main linked
editor-OFF CTest: 105/105 passed, 0 failed, 5.45 seconds
editor-ON Debug full build: exit 0; Main linked
editor-ON CTest: 104/104 passed, 0 failed, 5.45 seconds
```

Both builds retained only the documented Homebrew OpenSSL/turbojpeg deployment
target linker warnings. No changed source produced a compiler diagnostic.

Final local/static gates:

```text
Windows workflow contract: OK
MSVC runtime/asset staging contract: OK
renderer pipeline fail-fast contract: OK
.github/workflows/ci.yml and windows-build.yml: YAML OK; jobs=5/1
.releaserc.json: JSON parse exit 0
tests/test_windows_workflow_contract.py: py_compile exit 0
MinGW stage/validation Bash syntax: exit 0
clang-format 21.1.8: 14/14 future-CI source files, exit 0
cppcheck 2.21.0 with workflow flags: 14/14 checked, exit 0
git diff --check: exit 0
```

Controller mutations rejected extra/moved native uploaders, weak auxiliary
Debug CRT matching, an effective MinGW `which` override, root-only MinGW PE
enumeration, and EXE-only MinGW PE enumeration. Final specification review
reported `Spec compliant`; final quality review reported no Critical,
Important, or Minor finding.

These results close the local/static implementation gate only. Windows
PowerShell, native MSVC, MinGW/Wine, Linux editor-ON/OFF, downloaded-artifact,
and interactive D3D12/Vulkan validation remain pending explicit push/Actions
authorization and Windows-host execution. They do not establish that the
Windows build or renderer works.

### CI follow-up after `5c373cc9`

The first pushed repair commit produced three failed workflow runs on
2026-08-25:

- CI run `32879280969`;
- Linux Build run `32879280987`;
- Windows Build (MinGW) run `32879280984`.

Quality Gates and the macOS native job passed. Four Linux jobs failed while
running `ValidateShaderBlobs.cmake` under CMake 3.31 because the standalone
script did not declare its CMake 3.25 policy baseline. `CMP0057` therefore used
OLD behavior and rejected `IN_LIST` at line 77. The script now starts with
`cmake_minimum_required(VERSION 3.25)`, matching the project and the other
standalone shader validators.

All eight native Windows rows activated the correct MSVC architecture and
resolved `cl.exe` plus `link.exe` under the Visual Studio 18 toolchain. Their
identity assertions emitted no exception. The expected nonzero status from the
help probes remained in `$LASTEXITCODE`, so the GitHub PowerShell wrapper failed
the otherwise successful step. The assertion step now exits zero only after all
identity and architecture checks pass; thrown assertion failures still stop the
step first.

All four MinGW rows installed OpenSSL, curl, and the transitive zlib package.
Configuration found zlib 1.3.2 headers but not `ZLIB_LIBRARY` when curl's config
called stock `FindZLIB`. The workflow had added the classic installation as a
generic find root instead of activating vcpkg's package wrappers. MinGW now uses
the existing vcpkg toolchain with the existing MinGW toolchain chainloaded,
classic manifest installation disabled, and the exact matrix triplet plus
installation root selected. The obsolete extra find-root option was removed
from both MinGW toolchain files.

The workflow contract was extended before production edits. Its RED output
named the missing CMake policy declaration, leaked MSVC probe status, five
missing vcpkg-chainload arguments, and obsolete manual find roots. After the
edits:

```text
Windows workflow contract: OK
CMake 4.4.2 shader validation: 18/18 blobs
CMake 3.31.10 container shader validation: 18/18 blobs
workflow YAML parse: exit 0
workflow contract py_compile: exit 0
```

Native PowerShell execution, both MinGW architectures, and all hosted workflow
matrices still require a new Actions run. These local checks do not establish
that the Windows build works.

### CI follow-up after `18e4532f`

The second pushed repair commit produced failures in CI run `32888081203`,
Linux run `32888081136`, and MinGW run `32888081146` on 2026-08-25.

macOS ran 105 tests, including `imgui_sdlgpu_backend_contract`, and passed all
of them. Linux ran the same contract under CMake 3.31, but the standalone test
script had no policy baseline; its regex was interpreted under old policy
behavior and failed. All four standalone CMake test scripts now declare the
project's CMake 3.25 baseline so their semantics do not vary by runner CMake
version.

The native matrix reached compilation after the MSVC environment assertion
fix. Its remaining failures had three independent causes:

- `PersonalShopTitleImp.cpp` passed an `int` and Windows `LONG` to an
  unqualified `std::max`; the call now explicitly selects `int`.
- the x64 shader-compilation row required DXC, but the dependency manifest did
  not install it; `directx-dxc` is now an x64-Windows-only vcpkg dependency and
  normal vcpkg tool discovery supplies `dxc`. The condition avoids installing
  an unused target-architecture DXC in x86 rows, where shader compilation is
  disabled.
- x86 rows activated the x86-hosted compiler, which set
  `__DOTNET_PREFERRED_BITNESS=32` while only the x64 .NET host was installed;
  they now use the documented `amd64_x86` cross-tools environment.

MinGW successfully installed OpenSSL, curl, and zlib and entered the OpenSSL
vcpkg wrapper. The chainloaded MinGW toolchain was then loaded again and
overwrote `CMAKE_FIND_ROOT_PATH`, discarding the roots vcpkg had added. Both
toolchains now append their sysroot idempotently, preserving vcpkg roots across
repeated toolchain loads.

The hosted build matrix remains asymmetric: macOS covers arm64 Debug
editor-OFF; Linux covers x64 editor-OFF/ON in Debug and again in Release in a
separate workflow; native Windows covers x86/x64, Debug/Release,
editor-OFF/ON; MinGW covers x86/x64 Release editor-OFF/ON. History shows this
grew incrementally: the consolidated workflow began with one Debug lane per
OS, then Windows expanded for its shipping architectures/configurations and a
separate Linux Release workflow was retained. No repository documentation
defines a principled policy for the final asymmetry.

Shared tests must remain deterministic across every lane that registers them.
Platform-, compiler-, architecture-, configuration-, and feature-specific
tests can legitimately expose different failures because the compiled code
and environment differ. The Linux/macOS contract discrepancy was not one of
those legitimate differences: both lanes registered the same standalone CMake
test, but the missing policy baseline made its semantics depend on the runner's
CMake version. The policy declarations fix that root cause without multiplying
the build matrix.

Correct full-target local macOS verification after these follow-up changes:

```text
Debug editor-OFF default build: exit 0
Debug editor-OFF CTest: 105/105 passed, 0 failed, 6.45 seconds
Debug editor-ON default build: exit 0
Debug editor-ON CTest: 104/104 passed, 0 failed, 6.45 seconds
```

The one-test count difference is expected: `editor_leak` is registered only
when `ENABLE_EDITOR=OFF`. An earlier local command built only target `Main`
before running the complete CTest registry; the resulting `*_NOT_BUILT`
failures were invalid verification evidence, not a source or workflow failure.

Quality-gate preflight found that formatting checked every line of a changed
legacy file, so a one-line MSVC fix inherited unrelated historical formatting
debt. The formatting step now derives `clang-format --lines` ranges from the
Git diff and rejects only changed hunks. Full-file cppcheck also exposed three
signed left-shift color packings in the touched personal-shop source; those now
use the existing unsigned `RGBA()` packer with byte-identical channel values.
The current file passes the workflow's cppcheck command.

### CI follow-up after `a630774b`

The next pushed Windows repair commit produced failures in
[CI run 32899019191](https://github.com/yesid-bocanegra/MuMain/actions/runs/32899019191)
and
[MinGW run 32899019156](https://github.com/yesid-bocanegra/MuMain/actions/runs/32899019156)
on 2026-08-25. Linux and macOS passed. All eight native Windows rows built
successfully. Seven rows then failed the three shader-reflection tests; the
strict x64 Release editor-OFF row passed CTest and failed runtime-import
validation. All four MinGW rows failed compilation.

The native workflow passed shader executables under `C:/vcpkg/installed`, but
manifest mode installed packages under each build directory because
`VCPKG_INSTALLED_DIR` was unset. Shader-compilation-OFF rows retained the
nonexistent paths and registered reflection tests against them. Native CMake
now explicitly selects `C:/vcpkg/installed`, matching the existing cache and
tool arguments.

The remaining native artifact failure was `Main.exe: USP10.dll`. Uniscribe is
a Windows OS component. The reviewed OS-DLL allowlist now contains
`usp10.dll`; validation still requires the DLL to exist in the
architecture-correct Windows system directory.

MinGW exposed source files relying on indirect integer declarations.
`ChatCommandCatalog.h`, `ChatCommandCatalog.cpp`, and `MultiLanguage.h` now
include `<cstdint>` directly.

The workflow contract was changed before production files. Its RED output
named the missing vcpkg root, three missing `<cstdint>` includes, and missing
`USP10.dll` allowlist entry. After the edits:

```text
Windows workflow contract: OK
workflow contract py_compile: exit 0
workflow YAML parse: exit 0
git diff --check: exit 0
editor-OFF chat command catalog: 4/4 passed
editor-ON chat command catalog: 4/4 passed
editor-OFF tests/all build: exit 0
editor-ON tests/all build: exit 0
editor-OFF CTest: 106/106 passed
editor-ON CTest: 105/105 passed
```

Both full local macOS builds stopped in the pre-existing NativeAOT link step
with `ld: library 'brotlienc' not found`. The affected C++ target compiled and
linked independently in both editor configurations. Hosted MSVC and MinGW
matrices remain the execution gate; no Windows-working claim is made.
