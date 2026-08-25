# Windows Build and Runtime Repair Implementation Plan

> **Execution rule:** Complete phases in order. Record actual commands and
> outputs. Run relevant tests before and after each production change. Do not
> claim a phase passes from inspection alone.

**Goal:** Repair native MSVC, MinGW, D3D12/Vulkan rendering, Windows packaging,
and MuEditor SDL_GPU integration in one branch without regressing Linux/macOS.

**Design:**
[`2026-08-24-windows-build-runtime-repair-design.md`](../specs/2026-08-24-windows-build-runtime-repair-design.md)

**Primary stack:** C++20, CMake 3.25+, Ninja Multi-Config, MSVC, MinGW-w64,
.NET 10 NativeAOT, SDL3 GPU, HLSL, glslang, SPIRV-Cross, DXC, ImGui, GitHub
Actions.

---

## Phase 0: Freeze and record the baseline

### Task 0.1: Record repository and CI baseline

**Files:**
- Create during execution: `docs/build/windows-repair-validation.md`
- Inspect: `.github/workflows/ci.yml`
- Inspect: `.github/workflows/windows-build.yml`

- [ ] Record `git status --short --branch`, `git rev-parse HEAD`, submodule
  status, compiler/tool versions, and the current failed Actions job URLs.
- [ ] Record the exact pre-existing working-tree diff and its checksum. Preserve
  all unrelated edits; never reset or overwrite them.
- [ ] Reconcile the current in-progress SDL 3D-quad work before touching its
  overlapping renderer/test files. Either finish and verify it first, or name
  its exact diff as part of the Windows baseline.
- [ ] Run `quad_topology_tests` and `frame_pixel_readback_tests` before any
  Windows renderer or frame-capture edit. Preserve their results as focused
  regression evidence.
- [ ] Run the current host's configure/build/tests before modifications. Save
  exact pass/fail counts; do not repair unrelated failures.
- [ ] On interactive Windows, record `where cl`, `where link`, `cmake --version`,
  `dotnet --info`, `dxc --version`, `vulkaninfo --summary`, OS version, and GPU
  driver versions.
- [ ] Attempt every documented native preset: x64/x86, Debug/Release, editor
  OFF/ON. Use isolated build directories. Preserve the first error from each
  configuration.
- [ ] Launch the best current build once with default selection, once with
  `SDL_GPU_DRIVER=direct3d12`, once with `SDL_GPU_DRIVER=vulkan`. Preserve
  `MuError.log` and process exit behavior.
- [ ] Commit no baseline-generated build products or logs containing local
  paths. Summarize evidence in `docs/build/windows-repair-validation.md`.

**Gate:** Every later result has a named baseline comparison.

## Phase 1: Repair native MSVC and NativeAOT

### Task 1.1: Add a true-MSVC workflow contract

**Files:**
- Modify: `.github/workflows/ci.yml`
- Modify if required: `CMakePresets.json`

- [ ] Expand native Windows CI to x64/x86, Debug/Release, and editor OFF/ON
  with `fail-fast: false`; use one isolated build directory per matrix job.
- [ ] Activate the Visual Studio developer environment for the matching
  architecture before CMake.
- [ ] Add a pre-configure assertion step printing and validating `cl.exe` and
  Microsoft `link.exe` locations.
- [ ] Configure through the matching existing preset and vcpkg triplet.
- [ ] After configure, assert `CMAKE_CXX_COMPILER_ID=MSVC` from generated CMake
  state. Fail immediately if GNU/Clang is selected.
- [ ] Keep build, CTest, and artifact validation separate so logs retain the
  failing stage.

**RED:** Current workflow log identifies GNU 15.2 and GNU `link`.

**GREEN:** All eight native jobs configure with MSVC; no `C:/mingw64/bin`
compiler or GNU linker appears.

### Task 1.2: Decouple NativeAOT from ambient `PATH`

**Files:**
- Modify: `src/CMakeLists.txt`
- Inspect: `ClientLibrary/MUnique.Client.Library.csproj`

- [ ] Remove `-p:IlcUseEnvironmentalTools=true` for win-x64/win-x86.
- [ ] Preserve RID and `PlatformTarget` selection.
- [ ] Build the NativeAOT target before changing other MSVC compile blockers.
- [ ] Verify logs show NativeAOT's Visual Studio discovery and produce a PE DLL
  of the expected architecture.
- [ ] Run `test_connection_library_load` for each native architecture.

**RED:** Current NativeAOT fails because GNU `link` rejects `/DEF:`.

**GREEN:** `MUnique.Client.Library.dll` builds and loads for win-x64/win-x86.

### Task 1.3: Reproduce and fix current MSVC compile blockers

**Candidate files from PR 10; confirm each before editing:**
- `src/CMakeLists.txt`
- `src/source/Core/Platform/MuPlatform.h`
- `src/source/Core/Platform/MuPlatform.cpp`
- `src/source/Core/Utilities/CpuUsage.cpp`
- `src/source/Core/Utilities/Log/MuLogger.cpp`
- `src/source/Core/Platform/WinCompat.h`
- `src/source/GameLogic/Buffs/w_BuffScriptLoader.cpp`
- `src/source/GameShop/ShopListManager/ListManager.cpp`
- `src/source/GameShop/ShopListManager/interface/WZResult/WZResult.cpp`
- `src/source/UI/Legacy/UIPopup.cpp`

- [ ] Build until the first current MSVC compiler error.
- [ ] Apply only its smallest safe fix under `docs/CODING_RULES.md`.
- [ ] Rebuild to expose the next error. Repeat; do not batch speculative edits.
- [ ] Guard GCC-only options with compiler generator expressions.
- [ ] Resolve the Win32 `CreateWindow` macro collision without `#undef` leakage.
- [ ] Call the real Win32 `::GetSystemInfo` for `SYSTEM_INFO`.
- [ ] Implement native Windows UTF-8 conversion with
  `WideCharToMultiByte`; allocate space for its terminator safely, then remove
  the terminator from the returned `std::string`.
- [ ] Match `.cpp` signatures to their `uint32_t`/`wchar_t` declarations.
- [ ] Add the explicit enum-to-DWORD conversion required by MSVC.
- [ ] Rebuild editor OFF/ON after every fix affecting shared headers.

**Gate:** All four native configurations compile past C++ and link `Main.exe`.

### Task 1.4: Protect logger pre-init behavior

**Files:**
- Modify: `src/source/Core/Utilities/Log/MuLogger.cpp`
- Modify: relevant `tests/core/CMakeLists.txt`
- Create or extend: focused logger test source

- [ ] Write a test translation unit that obtains a logger during static
  initialization, before explicit `mu::log::Init()`.
- [ ] Run it against current code on Windows and record failure/crash behavior.
- [ ] Move logger-owned state into one function-local static aggregate.
- [ ] Preserve locking, registration, flush, shutdown, and lazy initialization.
- [ ] Verify explicit `Init(logDirectory)` semantics after early access; if the
  requested directory would otherwise be ignored, reconfigure sinks safely
  rather than accepting silent behavior change.
- [ ] Run the focused test repeatedly plus the full native CTest suite.

**Gate:** Early logging cannot access unconstructed state; normal initialization
still writes to the requested log directory.

## Phase 2: Repair MinGW dependencies

### Task 2.1: Add one scoped cross-root extension

**Files:**
- Modify: `cmake/toolchains/mingw-w64-x86_64.cmake`
- Modify: `cmake/toolchains/mingw-w64-i686.cmake`
- Modify: `.github/workflows/windows-build.yml`

- [ ] Add one optional cache path for an additional target sysroot.
- [ ] Append it to `CMAKE_FIND_ROOT_PATH` only when supplied.
- [ ] Keep program search `NEVER`; library/include/package searches `ONLY`.
- [ ] Add a `ponytail:` comment: one extra target root is the current ceiling;
  use full vcpkg chainloading only if more target package behavior is needed.
- [ ] In CI, derive the exact triplet prefix from vcpkg's install root and pass
  it through this variable.
- [ ] After configure, inspect `CMakeCache.txt`; require all populated
  `OPENSSL_*LIBRARY*` paths and `CURL_DIR` to start with the selected triplet
  prefix. Fail on `/usr`, another architecture, or another vcpkg root.

**RED:** Current `FindOpenSSL` ignores the supplied crypto path and reports
`missing: OPENSSL_CRYPTO_LIBRARY Crypto`.

**GREEN:** CMake resolves target, not host, OpenSSL from the vcpkg triplet root.

### Task 2.2: Install the full target dependency set

**Files:**
- Modify: `.github/workflows/windows-build.yml`

- [ ] Install `openssl` and `curl[ssl]` for `x64-mingw-static` and
  `x86-mingw-static`.
- [ ] Keep `curl[ssl]` rather than assuming an OpenSSL curl backend. Current
  vcpkg maps `ssl` to Schannel on Windows/MinGW; OpenSSL remains a separate
  direct Crypto dependency.
- [ ] Include both ports and `vcpkg.json` in cache keys.
- [ ] Remove direct `OPENSSL_INCLUDE_DIR` and `OPENSSL_CRYPTO_LIBRARY` command
  line overrides after root discovery works.
- [ ] Configure editor OFF first for each architecture.
- [ ] Build/link to discover any static curl transitive dependency gaps.
- [ ] Configure/build editor ON only after base linking passes.
- [ ] Run all four CTest jobs under Wine.

**Gate:** Four MinGW jobs configure, build, link, and test without host-library
leakage.

## Phase 3: Repair shader generation and renderer startup

### Task 3.1: Add failing shader-manifest validation

**Files:**
- Create: `cmake/ValidateShaderBlobs.cmake`
- Modify: root `CMakeLists.txt`
- Modify: test registration under `tests/render/`

- [ ] Define the six shader/stage pairs explicitly; derive the exact 18-file
  manifest.
- [ ] Validate existence, positive size, SPIR-V little-endian magic bytes
  `03 02 23 07`, DXIL `DXBC` signature, and MSL `main0` entry point.
- [ ] Register validation for checked-in blobs and staged runtime blobs.
- [ ] Run against current files and record failure on all six zero-byte DXIL
  files.

**Gate:** Invalid checked-in shader artifacts stop configure/test before a
runnable artifact is published.

### Task 3.2: Correct SDL_GPU resource bindings

**Files:**
- Modify: `src/shaders/basic_colored.vert.hlsl`
- Modify: `src/shaders/basic_textured.vert.hlsl`
- Modify: `src/shaders/basic_textured.frag.hlsl`
- Modify: `src/shaders/shadow_volume.vert.hlsl`
- Modify: `src/shaders/skinned_textured.vert.hlsl`
- Extend shader validation/reflection tests

- [ ] Capture current `spirv-cross --reflect` JSON as RED evidence.
- [ ] Change vertex uniform declarations to register `b0, space1`.
- [ ] Keep vertex storage at `t0, space0`.
- [ ] Change fragment texture/sampler declarations to `t0/s0, space2`.
- [ ] Change fragment uniform declarations to `b0, space3`.
- [ ] Preserve all shader calculations and entry-point signatures.
- [ ] Recompile SPIR-V to a temporary output.
- [ ] Assert reflection matches SDL_GPU's stage-specific set/binding contract.
- [ ] Assert each consumed `TEXCOORD[n]` semantic matches the renderer's SDL
  vertex-attribute location for D3D12; allow unused layout locations.
- [ ] Compare reflection counts with the `LoadShaders()` metadata table:
  textured vertex `0/0/1`, textured fragment `1/0/1`, colored vertex
  `0/0/1`, colored fragment `0/0/0`, shadow vertex `0/0/1`, skinned vertex
  `0/1/1` for samplers/storage buffers/uniform buffers.
- [ ] Regenerate MSL and assert SDL_GPU's Metal ordering: textures/samplers are
  contiguous, uniform buffers precede storage buffers, and vertex input uses
  `[[stage_in]]` rather than manually occupying vertex-buffer indices.

**Gate:** No generated shader resource remains in an invalid set/space.

### Task 3.3: Replace DXIL stubs with required DXC compilation

**Files:**
- Modify: root `CMakeLists.txt`
- Modify: `.gitattributes`
- Replace: `src/shaders/compiled/*.spv`
- Replace: `src/shaders/compiled/*.msl`
- Replace: `src/shaders/compiled/*.dxil`

- [ ] Discover `dxc` alongside glslang and SPIRV-Cross.
- [ ] On Windows, search the active Windows SDK `bin/<version>/x64` directory
  when `dxc.exe` is not already on `PATH`; use the host x64 compiler for both
  target architectures.
- [ ] Map `vert` to `vs_6_0`; `frag` to `ps_6_0`.
- [ ] Make missing `dxc` fatal when shader compilation is enabled.
- [ ] Delete the `cmake -E touch` fallback.
- [ ] Generate all 18 artifacts from corrected source.
- [ ] Mark `.spv` and `.dxil` binary in `.gitattributes`.
- [ ] Run manifest validation.
- [ ] Run `dxc -dumpbin` and reflection checks for register spaces.
- [ ] On Windows with DXC, build once with
  `MU_ENABLE_SHADER_COMPILATION=ON`; on Windows, macOS, and Linux, build with it
  `OFF` to verify the checked-in artifact workflow without adding DXC to
  non-Windows hosts.

**Gate:** D3D12 never receives an empty or resource-incompatible shader.

### Task 3.4: Fail renderer initialization on unusable pipelines

**Files:**
- Modify: `src/source/Render/Renderer/MuRendererSDLGpu.cpp`
- Extend: `tests/render/test_shaderprograms.cpp` or nearest functional helpers

- [ ] Record exact current D3D12 and Vulkan first pipeline errors from Windows.
- [ ] Make pipeline construction return structured success/failure information
  sufficient to log layout, blend index, depth state, cull state, driver, and
  the first SDL error.
- [ ] Aggregate required core pipeline failures.
- [ ] Return false from `CreatePipelines()` when required 2D/3D pipelines fail.
- [ ] On failure, call `DestroyPipelines()` before shader/window/device teardown
  so successfully created earlier entries do not leak.
- [ ] Keep genuinely optional skinned/shadow paths optional only if callers
  already degrade safely.
- [ ] Verify renderer startup now either succeeds fully or exits with a precise
  error; it must not continue to a blank frame with null core pipelines.
- [ ] Re-run `quad_topology_tests` and `frame_pixel_readback_tests`; pipeline
  diagnostics must not alter indexed-quad replay or readback behavior.

**Gate:** Both Windows drivers create required pipelines before runtime visual
testing begins.

## Phase 4: Move MuEditor to SDL_GPU

### Task 4.1: Replace the editor renderer backend

**Files:**
- Modify: `src/CMakeLists.txt`
- Modify: `src/MuEditor/Core/MuEditorCore.h`
- Modify: `src/MuEditor/Core/MuEditorCore.cpp`
- Modify: `src/source/App/Platform/Windows/Winmain.cpp`

- [ ] Build editor ON before changes; preserve current failure evidence.
- [ ] Compile `imgui_impl_sdlgpu3.cpp`; remove `imgui_impl_opengl2.cpp`.
- [ ] Remove only ImGui-specific OpenGL link requirements. Preserve OpenGL
  system libraries still required by untouched compatibility code.
- [ ] Change editor initialization to accept the SDL window only.
- [ ] Obtain `SDL_GPUDevice*` from `mu::GetRenderer().GetDevice()`.
- [ ] Initialize `ImGui_ImplSDL3_InitForSDLGPU` and
  `ImGui_ImplSDLGPU3_Init` using the live device, swapchain color format, and
  `SDL_GPU_SAMPLECOUNT_1`; check both boolean results.
- [ ] Remove `g_sdlGLContext`; do not create a compatibility GL context.
- [ ] Replace OpenGL2 shutdown/new-frame/render calls with SDL_GPU equivalents.
- [ ] Before `ImGui_ImplSDLGPU3_Shutdown()`, wait for the SDL GPU device to be
  idle; keep the device alive until both ImGui backends have shut down.

**Gate:** Editor initialization succeeds without an OpenGL context on all
editor-enabled platforms.

### Task 4.2: Integrate ImGui into the engine pass

**Files:**
- Modify: `src/MuEditor/Core/MuEditorCore.h`
- Modify: `src/MuEditor/Core/MuEditorCore.cpp`
- Modify: `src/source/Render/Renderer/MuRendererSDLGpu.cpp`

- [ ] Add editor-owned prepare and render hooks accepting the exact vendored
  backend handles: `SDL_GPUCommandBuffer*`, then
  `SDL_GPUCommandBuffer*` plus `SDL_GPURenderPass*`.
- [ ] Track whether current-frame ImGui draw data is ready.
- [ ] When editor `Render()` finalizes draw data, queue one editor marker at the
  current render-command position.
- [ ] In `EndFrame()`, call `ImGui_ImplSDLGPU3_PrepareDrawData()` once before
  `SDL_BeginGPURenderPass` when a marker is pending.
- [ ] During command replay, render the marker with
  `ImGui_ImplSDLGPU3_RenderDrawData()`. Preserve commands recorded afterward,
  especially the game cursor, after the editor overlay.
- [ ] Track the active game viewport and scissor during replay; reapply both
  immediately after ImGui, because the backend overwrites them.
- [ ] Clear ready state after consumption; no stale draw-data replay.
- [ ] Preserve single submission and existing frame-readback behavior.
- [ ] Re-run `quad_topology_tests` and `frame_pixel_readback_tests` after ImGui
  insertion into `EndFrame()`.
- [ ] Build/test editor OFF to prove `_EDITOR` isolation.
- [ ] Build/test editor ON on Linux, macOS, MSVC, and MinGW.

**Gate:** No OpenGL2 symbol remains in editor production code; editor OFF still
passes `editor_leak`.

## Phase 5: Repair packaging and deterministic workflow validation

### Task 5.1: Stage runtime DLL closure

**Files:**
- Modify: `src/CMakeLists.txt`
- Add focused CMake packaging validation if needed

- [ ] On native MSVC only, stage `$<TARGET_RUNTIME_DLLS:Main>` with
  `COMMAND_EXPAND_LISTS`. Skip the expression for static MinGW and non-Windows
  builds.
- [ ] Retain deliberate local DLL staging only for files not represented by a
  linked target.
- [ ] Build in a clean Windows output directory.
- [ ] Use `dumpbin /DEPENDENTS` to enumerate imports.
- [ ] Verify every non-system DLL resolves inside the runtime directory.
- [ ] For MinGW, recursively inspect every staged `.exe`/`.dll`, validate each
  PE architecture, and stage only compiler-runtime imports resolved by the
  target `${triple}-g++ -print-file-name`; reject every other unresolved import.

**Gate:** Packaged `Main.exe` does not depend on developer-machine vcpkg paths.

### Task 5.2: Validate and upload runnable directories

**Files:**
- Modify: `.github/workflows/ci.yml`
- Modify: `.github/workflows/windows-build.yml`

- [ ] Validate PE architecture for `Main.exe` and NativeAOT DLL.
- [ ] Require `config.ini`, `Data`, representative assets, and the exact shader
  manifest.
- [ ] Require NativeAOT only in native artifacts.
- [ ] For native Release rows, stage redistributable DLLs from exactly one
  active `$VCToolsRedistDir/<arch>/Microsoft.VC*.CRT` directory; reject missing
  or ambiguous paths and require VC runtime imports root-local.
- [ ] Accept Windows OS imports only when explicitly allowlisted and present in
  the architecture-correct target system directory.
- [ ] Upload complete runtime contents for native Release and MinGW jobs.
- [ ] Keep native Debug rows compile/CTest-only; use them for interactive and
  developer validation, not published artifacts.
- [ ] Make semantic release archive and publish the complete native x64 Release
  editor-OFF runtime, including hidden files.
- [ ] Set `if-no-files-found: error`.
- [ ] Name artifacts with architecture and editor state.

**Gate:** Downloaded artifacts are self-contained according to their documented
native/MinGW capabilities.

### Task 5.3: Enable portable Windows frame capture

**Files:**
- Modify: `src/source/App/Platform/Windows/Winmain.cpp`
- Create: `src/source/App/Platform/DiagnosticFrameCaptureWriter.h`
- Create: `src/source/App/Platform/DiagnosticFrameCaptureWriter.cpp`
- Modify: `tests/render/CMakeLists.txt`
- Extend: `tests/render/test_frame_pixel_readback.cpp`

- [ ] Add a failing test that writes a 1x1 RGB frame and compares the exact
  `P6\n1 1\n255\n` header plus three payload bytes.
- [ ] Add failing tests for zero dimensions, mismatched RGB byte count, and an
  unwritable path; no invalid input may report success.
- [ ] Move only PPM serialization from `Winmain.cpp` into the focused helper;
  keep capture scheduling and GPU readback ownership unchanged.
- [ ] Remove the Windows exclusion around capture request and consume calls.
- [ ] Use `mu-frame.ppm` in the executable working directory as the portable
  default; when `MU_CAPTURE_PATH` exists, pass its value unchanged, including
  an empty value that must fail and log rather than silently choosing another
  destination.
- [ ] Preserve opt-in behavior: no capture cost without `MU_CAPTURE_FRAME`.
- [ ] Verify capture path errors are logged and do not corrupt renderer state.
- [ ] Run existing readback tests on all available hosts.
- [ ] Preserve all pre-existing additions to
  `tests/render/test_frame_pixel_readback.cpp`; extend rather than replace the
  current minimal renderer contract.

**Gate:** Interactive Windows can produce a PPM through the same renderer
readback path used elsewhere.

## Phase 6: Full verification and evidence

### Task 6.1: Local non-Windows regression comparison

- [ ] Run the exact Phase 0 macOS/Linux configure/build/test commands.
- [ ] Diff pass/fail counts and logs against baseline.
- [ ] Investigate every new warning/error in touched subsystems.
- [ ] Run `git diff --check`.

### Task 6.2: GitHub Actions verification

- [ ] After explicit authorization to push, run the branch workflows.
- [ ] Require eight native MSVC jobs green.
- [ ] Require four MinGW jobs green.
- [ ] Require Linux editor-OFF/editor-ON, macOS, and quality jobs to show no new
  failure.
- [ ] Download and inspect at least native x64 editor OFF and both MinGW
  architectures.
- [ ] Record exact run/job URLs and conclusions in
  `docs/build/windows-repair-validation.md`.

### Task 6.3: Interactive Windows D3D12/Vulkan verification

**Native x64 Debug and Release, editor OFF:**

- [ ] Run default backend. Confirm log reports `direct3d12`.
- [ ] Run with `SDL_GPU_DRIVER=direct3d12` and capture a login-scene frame.
- [ ] Run with `SDL_GPU_DRIVER=vulkan` and capture the same scene.
- [ ] Inspect captures for non-black scene pixels, correct UI, textures, text,
  models, alpha, fog, and resize behavior.
- [ ] Confirm no shader, pipeline, device-loss, null-pipeline, or missing-DLL
  errors in `MuError.log`.

**Native x64 Debug and Release, editor ON:**

- [ ] Launch with `--editor` on D3D12 and Vulkan.
- [ ] Confirm editor overlay, F12 toggle, mouse capture/release, keyboard input,
  font rendering, resize, game cursor behavior, and clean shutdown.

**Native x86 Release:**

- [ ] Confirm `Main.exe` and NativeAOT DLL architecture is x86.
- [ ] Launch on the x64 Windows host and reach renderer initialization.
- [ ] Capture a frame when supported by both GPU drivers.

**Packaged artifact:**

- [ ] Extract to a clean directory outside the repository.
- [ ] Launch with Visual Studio/vcpkg tool directories removed from `PATH`.
- [ ] Repeat D3D12, Vulkan, NativeAOT load, and editor smoke checks.

### Task 6.4: Final audit

- [ ] Review the complete diff against `docs/CODING_RULES.md`.
- [ ] Confirm no placeholder shaders, OpenGL2 editor calls, GNU native compiler,
  ineffective OpenSSL overrides, or incomplete artifact paths remain.
- [ ] Confirm documentation matches actual commands and outputs.
- [ ] Report unresolved pre-existing failures separately.
- [ ] Do not state "working" until Actions evidence and interactive Windows
  captures both exist.

Final-audit remediation tasks discovered before authorized Actions execution:

- [x] **6.4.1 — Decouple timer logging.** `MuTimer::LogStats()` no longer pulls
  renderer/editor/bootstrap symbols through `CErrorReport`; editor-ON and
  editor-OFF `test_mu_timer` pass while preserving the error level and fields.
  Evidence: [Task 6.4.1](../../build/windows-repair-validation.md#task-641-editor-on-unit-test-linkage)
  and [final controller verification](../../build/windows-repair-validation.md#final-local-controller-verification).
- [x] **6.4.2 — Add one strict native shader row.** Exactly the native x64
  Release editor-OFF matrix row enables `MU_ENABLE_SHADER_COMPILATION=ON` with
  vcpkg glslang/SPIRV-Cross and Windows SDK DXC discovery. Local DXC is
  unavailable; Windows remains the execution gate. Evidence:
  [Task 6.4.2](../../build/windows-repair-validation.md#task-642-strict-native-shader-regeneration)
  and [final controller verification](../../build/windows-repair-validation.md#final-local-controller-verification).
- [x] **6.4.3 — Close native runtime DLL validation.** MSVC linked-target
  staging remains; obsolete manual GLEW staging and bulk `src/bin/*.dll` asset
  copying are absent. The workflow enumerates every staged PE, checks matrix
  architecture, runs `dumpbin /DEPENDENTS`, rejects non-leaf import names, and
  resolves imports only from the runtime root, API sets, or the target Windows
  system directory. Evidence:
  [Task 6.4.3](../../build/windows-repair-validation.md#task-643-native-runtime-dll-import-closure)
  and [final controller verification](../../build/windows-repair-validation.md#final-local-controller-verification).
- [x] **6.4.4 — Validate shipped shader reflection.** Checked-in and both
  staged sets validate 18/18 blobs, reflect 6/6 shaders, match regenerated
  reflection for 6/6 shaders, and remain byte-identical for 18/18 artifacts.
  Evidence: [Task 6.4.4](../../build/windows-repair-validation.md#task-644-shipped-shader-reflection)
  and [final controller verification](../../build/windows-repair-validation.md#final-local-controller-verification).
- [x] **6.4.5 — Include exact pipeline shader names.** Required and optional
  pipeline diagnostics bind the selected handles to `basic_textured.vert`,
  `skinned_textured.vert`, and `basic_textured.frag`; the pipeline contract and
  later full editor-OFF/editor-ON builds pass locally. Evidence:
  [Phase 6.4.5](../../build/windows-repair-validation.md#phase-645-pipeline-shader-identifiers)
  and [final controller verification](../../build/windows-repair-validation.md#final-local-controller-verification).
- [x] **6.4.6 — Cover Linux editor builds.** Linux CI uses isolated editor-OFF
  and editor-ON build/cache/artifact paths; both rows configure, build, and run
  CTest.
- [x] **6.4.7 — Publish native Release only.** Native Debug rows compile and
  test without artifact validation/upload. Release closure rejects debug CRT
  imports. The sole native uploader is bound to the isolated Release-gated
  upload step, and semantic release consumes x64 Release editor-OFF.
- [x] **6.4.8 — Close MinGW runtime imports.** Packaging recursively stages
  target compiler-runtime DLLs to convergence; validation checks every PE and
  rejects unresolved, path-shaped, or non-allowlisted imports. The workflow
  contract rejects root-only and EXE-only validation regressions.
- [x] **6.4.9 — Stage and constrain native CRT imports.** Release rows copy the
  active architecture's exact MSVC CRT redistributable directory before
  validation. VC runtime imports must resolve root-local; reviewed Windows OS
  imports must also exist in the target architecture's system directory.
- [x] **6.4.10 — Publish the complete native runtime.** Release downloads the
  native x64 Release editor-OFF artifact, archives the entire directory with
  hidden files, then publishes exactly that archive with native metadata.
- [x] **6.4.11 — Repair the latest hosted Windows regressions.** Native manifest
  installs now use the same `C:/vcpkg/installed` root as the cached shader-tool
  paths, `USP10.dll` is reviewed as a target-system DLL, and the MinGW-rejected
  `int32_t` declarations include `<cstdint>` directly. The workflow contract
  reproduced all five missing requirements before the edits and passes after
  them. Hosted Windows execution remains pending the next authorized push.

These eleven local remediation items are complete. Task 6.4 remains open pending
complete-diff final reviews, authorized GitHub Actions, Windows artifact
inspection, and interactive D3D12/Vulkan validation. No Windows-working claim
is made.

## Definition of done

All deterministic gates pass. Interactive Windows produces visible D3D12 and
Vulkan login-scene captures. MuEditor renders through SDL_GPU. Native and
MinGW artifacts match their documented capabilities. Linux/macOS have no new
failures relative to Phase 0.
