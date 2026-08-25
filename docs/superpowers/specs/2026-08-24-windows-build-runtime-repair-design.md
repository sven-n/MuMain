# Windows Build and Runtime Repair Design

## Status

Approved for one coherent branch change. Implementation remains phased so each
failure has an isolated proof and rollback point.

## Goal

Make every documented Windows build configuration compile, package, and pass
its deterministic tests. Produce a runnable native MSVC client. Verify D3D12,
Vulkan, and MuEditor rendering on an interactive Windows machine without
regressing Linux or macOS.

## Evidence

- Upstream report: [issue comment 5395165036](https://github.com/sven-n/MuMain/pull/572#issuecomment-5395165036)
  reports a failed Windows build and black output on Vulkan and D3D12.
- Follow-up: [issue comment 5399114029](https://github.com/sven-n/MuMain/pull/572#issuecomment-5399114029)
  promised native runtime validation that has not yet happened on this branch.
- Native CI failure: [run 32768004628](https://github.com/yesid-bocanegra/MuMain/actions/runs/32768004628)
  configures GNU 15.2 on `windows-latest`. NativeAOT then invokes GNU `link`,
  which rejects `/DEF:...`.
- MinGW failure: [run 32768004592](https://github.com/yesid-bocanegra/MuMain/actions/runs/32768004592)
  fails all four jobs while locating OpenSSL.
- Experimental Windows patch: [PR 10](https://github.com/yesid-bocanegra/MuMain/pull/10)
  records additional MSVC compile blockers, a pre-main logger crash, real DXIL
  generation, and a remaining cross-backend pipeline-creation failure. Its
  changes are evidence, not an implementation to copy without retesting.

## Confirmed root causes

### Native Windows toolchain

The `windows-x64` Ninja preset expects a Visual Studio developer environment.
GitHub Actions does not activate one, so CMake finds `C:/mingw64/bin/cc.exe`.
`IlcUseEnvironmentalTools=true` compounds the problem by telling NativeAOT to
run whichever `link` appears on `PATH`.

The current source also retains the MSVC blockers documented by PR 10:

- a GCC-only warning flag applied to MSVC;
- `MuPlatform::CreateWindow` colliding with the Win32 macro;
- the wrong `GetSystemInfo` function in `CpuUsage.cpp`;
- missing native-Windows UTF-8/path helpers;
- one missing helper include;
- declaration/definition type mismatches hidden by POSIX aliases;
- one narrowing conversion rejected by MSVC;
- logger state exposed to cross-translation-unit static-initialization order.

Each blocker must be reproduced or compiler-proven against the current branch.
The prior patch must not be cherry-picked blindly. Its Windows UTF-8 helper, in
particular, must avoid writing a terminator past the logical string size.

### MinGW dependency discovery

The workflow installs OpenSSL under a vcpkg triplet prefix outside the MinGW
toolchain's `CMAKE_FIND_ROOT_PATH`. CMake's `FindOpenSSL` MinGW branch searches
its internal `LIB_EAY` variable, then overwrites the supplied
`OPENSSL_CRYPTO_LIBRARY`. Fixing OpenSSL will expose the next missing target
dependency: curl is required by `src/CMakeLists.txt` but is not installed for
the MinGW triplet.

### SDL_GPU shaders and pipelines

All six checked-in DXIL files are zero bytes. `MU_ENABLE_SHADER_COMPILATION=ON`
also creates empty DXIL files with `cmake -E touch`.

The nonempty SPIR-V files are structurally valid but violate SDL_GPU's graphics
shader binding contract. Reflection currently shows vertex uniform buffers in
set 0 and fragment resources in set 0. SDL_GPU requires:

| Stage | Resource | SPIR-V set | DXIL register space |
|-------|----------|---------------|---------------------|
| Vertex | sampled/storage resources | 0 | 0 |
| Vertex | uniform buffers | 1 | 1 |
| Fragment | sampled/storage resources | 2 | 2 |
| Fragment | uniform buffers | 3 | 3 |

The engine pushes uniform slot 0, so vertex `b1` must become binding/register
`b0` in vertex uniform space/set 1. Fragment textures/samplers must use space
2; fragment uniforms must use space 3. This explains a pipeline failure shared
by Vulkan and D3D12 more completely than the empty DXIL files alone.

`CreatePipelines()` currently returns success even after required pipeline
creation fails. The process can therefore continue with null pipelines and
show only the clear color. Required pipeline failures must stop renderer
initialization with the original SDL error.

### MuEditor

The editor builds `imgui_impl_opengl2`, calls
`ImGui_ImplSDL3_InitForOpenGL`, and rejects a null `SDL_GLContext`. The SDL_GPU
window never creates an OpenGL context. The vendored ImGui checkout already
contains `imgui_impl_sdlgpu3`; no dependency is needed.

### Packaging and validation

Native CI checks that a `shaders` directory exists but not that its files are
nonempty or valid. The native artifact omits `Data`. MinGW artifacts upload
only `Main.exe`. Dynamic vcpkg dependencies are not explicitly staged or
validated. Neither workflow proves a rendered frame.

## Repair design

### Phase 0: preserve the baseline

Record the exact current revision, clean-worktree state, local macOS/Linux test
results available from this host, the two failed Actions runs, and fresh native
Windows Debug/Release logs. Capture `MuError.log` from forced D3D12 and Vulkan
launches before changing renderer code.

The branch currently also contains in-progress SDL 3D-quad work that overlaps
`MuRendererSDLGpu.cpp` and render tests. Before Windows production edits, either
finish and verify that work or record its exact diff as part of the baseline.
Never reset or overwrite it. Its focused topology/readback tests become
regression gates for every later renderer and frame-capture phase.

Every later test report compares against this baseline. Pre-existing failures
remain distinguished from failures introduced by the repair.

### Phase 1: make native Windows genuinely MSVC

Activate the Visual Studio developer environment in the native workflow before
configuration. Assert all three facts before building:

- CMake compiler ID is MSVC;
- `cl.exe` resolves from Visual Studio;
- `link.exe` resolves from Visual Studio, not MinGW or GNU coreutils.

Remove `IlcUseEnvironmentalTools=true`. NativeAOT already includes
`findvcvarsall.bat`; its default behavior resolves the exact Visual Studio
linker and SDK library paths. This avoids coupling NativeAOT correctness to
ambient `PATH` order.

Reapply the PR 10 source fixes individually against current code. Prefer the
smallest safe correction. Preserve Unicode conversion correctness, logger
pre-init safety, and existing public behavior. Compiler-only failures are
proved by the native matrix; behavior changes receive focused tests.

Native CI covers every documented row in both Debug and Release:

- x64, editor OFF;
- x64, editor ON;
- x86, editor OFF;
- x86, editor ON.

Each of the resulting eight jobs uses an isolated build directory, builds
NativeAOT, and runs CTest. This avoids a Debug/Release result being reused from
the same multi-config custom-command output. The x64 non-editor rows supply the
primary runnable artifacts.

### Phase 2: repair MinGW target dependency discovery

Install both `openssl` and `curl[ssl]` for each vcpkg MinGW triplet. Pass the
triplet installation prefix to the existing MinGW toolchain through one
explicit target-root variable. Append only that prefix to
`CMAKE_FIND_ROOT_PATH`; retain `ONLY` modes so host Linux headers and libraries
cannot leak into Windows binaries.

Let `find_package(OpenSSL)` and `find_package(CURL)` consume the target prefix.
Remove the ineffective direct `OPENSSL_CRYPTO_LIBRARY` workaround. Keep the
existing explicit static turbojpeg path.

After configure, CI asserts every cached OpenSSL library path and `CURL_DIR`
is below the selected vcpkg triplet prefix. `curl[ssl]` is intentional: the
current vcpkg port maps that feature to Schannel on Windows/MinGW, while the
separately linked OpenSSL Crypto port continues to serve the client library's
cryptographic API. No requirement assumes curl itself uses OpenSSL.

The existing x86/x64 and editor ON/OFF matrix remains unchanged. It continues
as the cross-compiler compile/link/test guard; it does not pretend to produce a
NativeAOT DLL.

### Phase 3: make the shader matrix real

Correct HLSL register bindings once at source:

- vertex uniforms: `b0, space1`;
- vertex storage buffers: `t0, space0`;
- fragment textures/samplers: `t0/s0, space2`;
- fragment uniforms: `b0, space3`.

Generate:

- SPIR-V with `glslangValidator`;
- MSL from corrected SPIR-V with `spirv-cross`;
- DXIL Shader Model 6.0 with `dxc` (`vs_6_0`/`ps_6_0`).

`MU_ENABLE_SHADER_COMPILATION=ON` fails if any required compiler is absent. It
must never create placeholder blobs. Regenerate all 18 checked-in artifacts.
Declare `.spv` and `.dxil` binary in `.gitattributes`.

A CMake validation script checks the exact manifest, not a glob:

- six expected shaders times three formats;
- file exists and is nonempty;
- SPIR-V starts with little-endian magic bytes `03 02 23 07`;
- DXIL container signature is `DXBC`;
- MSL contains the runtime-selected `main0` Metal entry point;
- generated SPIR-V reflection matches the SDL_GPU set/binding table.

Generated MSL is also checked against SDL_GPU's Metal resource order: textures
and samplers remain contiguous, uniform buffers precede storage buffers, and
vertex inputs use `[[stage_in]]`. Every consumed HLSL `TEXCOORD[n]` semantic
matches the renderer's SDL attribute location; unused locations may remain in
the vertex layout. This is required by SDL's default D3D12 input layout.

Reflection counts must also match the existing `SDL_GPUShaderCreateInfo`
metadata: basic textured vertex `0/0/1`, basic textured fragment `1/0/1`,
basic colored vertex `0/0/1`, basic colored fragment `0/0/0`, shadow vertex
`0/0/1`, and skinned vertex `0/1/1` for samplers/storage buffers/uniform
buffers. Register-set repairs must not silently change these counts.

DXIL generation runs on Windows with the Windows SDK `dxc.exe`. Normal
macOS/Linux builds consume validated checked-in blobs and do not acquire a new
DXC dependency. Shader-compilation mode remains strict: when explicitly
enabled, every required compiler must exist.

Renderer initialization aggregates required pipeline failures and returns
false if any core 2D/3D pipeline is unavailable. Optional paths may remain
optional only where the corresponding shader is genuinely optional. Logs name
the driver, shader, layout, blend mode, depth mode, and first SDL error.
Failure cleanup releases every pipeline created earlier in the same attempt
before releasing shaders, the window claim, and the device.

### Phase 4: migrate MuEditor to SDL_GPU

Build `imgui_impl_sdlgpu3.cpp` instead of `imgui_impl_opengl2.cpp`. Initialize
the SDL3 platform backend with `ImGui_ImplSDL3_InitForSDLGPU`, then initialize
the renderer backend from the existing SDL GPU device and swapchain format.
Remove the editor's null GL-context parameter and `g_sdlGLContext` state.

Frame ownership remains singular:

1. Editor `Update()` starts the ImGui frame before scene construction.
2. Editor `Render()` finalizes `ImDrawData` and queues one editor marker at its
   current position in the game command stream.
3. Renderer `EndFrame()` prepares ImGui upload data once before opening its
   render pass.
4. Renderer replays game commands; the editor marker draws ImGui in the same
   pass. Commands recorded after it, including the game cursor, remain later.
5. Renderer submits once.

The editor owns ImGui backend calls. One editor-only renderer marker plus two
editor hooks are compiled only under `_EDITOR`; no second render pass or GPU
submission is introduced. A ready flag prevents stale draw data from being
reused when a scene does not render the editor.

Because the ImGui backend binds its own pipeline, buffers, viewport, and
scissor, command replay tracks the active game viewport and scissor and
restores both immediately after the editor marker. Later game draws continue
to bind their own pipelines and resources as before.

Shutdown waits for the SDL GPU device to become idle before the ImGui SDL_GPU
backend releases its pipelines, buffers, textures, and samplers. The renderer
device remains alive until editor shutdown completes.

### Phase 5: produce runnable artifacts and diagnostics

On native MSVC builds, use CMake's `$<TARGET_RUNTIME_DLLS:Main>` to stage
transitive runtime DLLs next to `Main.exe`. Release CI also copies the active
architecture's redistributable DLLs from exactly one immediate
`$VCToolsRedistDir/<arch>/Microsoft.VC*.CRT` directory. Missing or ambiguous
active CRT directories fail before artifact validation. The static MinGW build
skips this command. MinGW packaging recursively inspects every staged PE and
copies only missing compiler-runtime imports resolved by the target compiler's
`${triple}-g++ -print-file-name=<dll>`; unknown non-system imports fail closed.

Native import validation accepts root-local files first, requires release
`vcruntime`, `msvcp`, `concrt`, and `vcomp` families root-local, then accepts
API sets.
Windows OS imports require both membership in one explicit reviewed allowlist
and existence in the architecture-correct target system directory. A DLL's
mere presence in `System32` or `SysWOW64` is never sufficient.

Artifact validation requires:

- `Main.exe` with the expected PE architecture;
- `MUnique.Client.Library.dll` for native builds;
- `config.ini`;
- `Data` plus representative required assets;
- the exact validated shader manifest;
- every non-system imported DLL available beside the executable.

Upload runnable directory contents only from native Release rows. Native Debug
rows remain compile/CTest and interactive developer-build coverage; they are
not published artifacts. The native job contains exactly one artifact uploader,
and that action belongs to the isolated Release-gated upload step. Semantic
release consumes the native x64 Release editor-OFF artifact. MinGW artifacts
include assets, configuration, and shaders but explicitly omit NativeAOT. The
release job archives the complete downloaded native runtime, including hidden
files, and publishes that single native x64 Release editor-OFF archive.

Enable the existing `MU_CAPTURE_FRAME` path on Windows. Use a portable default
output name when `MU_CAPTURE_PATH` is absent. GitHub-hosted Windows runners are
accepted only as deterministic compile/test/package proof; GPU frame proof is
required on the interactive Windows machine.

### Phase 6: end-to-end verification

Deterministic gates:

- macOS plus Linux editor-OFF/editor-ON before/after build and CTest comparison;
- eight native MSVC jobs;
- four MinGW rows under Wine;
- existing quad-topology and frame-readback tests;
- shader generation, validation, and reflection checks;
- NativeAOT load test;
- artifact architecture, manifest, and DLL closure checks;
- `git diff --check`.

Interactive Windows gates, x64 Debug and Release:

- default backend selects D3D12 and reaches the login scene;
- `SDL_GPU_DRIVER=vulkan` reaches the same scene;
- each backend writes a non-black frame capture;
- no shader/pipeline errors appear in `MuError.log`;
- editor build starts with `--editor`, draws the overlay, accepts keyboard and
  mouse input, toggles with F12, survives resize, then shuts down cleanly;
- packaged Release artifact runs from a clean directory without developer-only
  DLLs on `PATH`.

x86 Release must build, load NativeAOT, start on the same 64-bit Windows host,
and reach renderer initialization. Visual parity is required only if the GPU
driver supports the x86 process path.

## Risk controls

- Shader changes affect every GPU backend. Change only resource declarations;
  preserve shader math. Regenerate and validate SPIR-V/MSL before Windows
  runtime testing.
- The prior Windows patch is advisory. Reproduce each failure; reject or amend
  any unsafe implementation.
- Toolchain-root changes are opt-in and scoped to MinGW CI. Host search modes
  remain `ONLY`.
- Editor rendering stays in the engine's existing command buffer and pass.
  Avoid a second GPU submission or OpenGL compatibility context.
- Required renderer resources fail fast. No blank-frame fallback hides errors.
- One branch/PR is preserved, but phase-sized commits are allowed if requested;
  no phase is declared complete without its own evidence.

## Out of scope

- New OpenGL or D3D11 renderer backends.
- Renderer architecture replacement.
- New package managers or shader libraries.
- Large cleanup of dead platform abstractions.
- Treating hosted Actions runners as authoritative GPU visual validation.

## Completion criteria

The repair is complete only when deterministic CI gates pass, both D3D12 and
Vulkan produce visible login-scene captures on interactive Windows, MuEditor
renders through SDL_GPU, packaged artifacts run from a clean directory, and
Linux/macOS show no new failures from the recorded baseline.
