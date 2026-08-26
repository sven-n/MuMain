# Cross-Platform Rendering Parity Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove the confirmed shared rendering bottlenecks and host-dependent font behavior so the SDL GPU client follows one auditable policy on macOS, Linux, and Windows.

**Architecture:** Keep backend selection inside SDL. Add a render-pass-local application cache above SDL GPU, merge adjacent 2D commands before replay, retain prepared SDL_ttf text, patch the pinned SDL_ttf source once to batch missing-glyph uploads, require GPU-skinning resources, and resolve every font role from packaged assets. Extend existing renderer and frame-profiler counters; do not add a second telemetry system.

**Tech Stack:** C++20, SDL3 GPU, SDL_ttf 3.2.2 at `a1ce3670aec736ecbf0936c43f2f0cc53aa61e5b`, CMake 3.25+, doctest, existing `FrameProfiler`, bundled TrueType assets.

---

## File map

**Create**

- `src/source/Render/Renderer/SdlGpuReplayState.h` — render-pass-local equality cache; owns no GPU resources.
- `src/source/Render/Renderer/SdlGpuReplayState.cpp` — cache comparisons and descriptor invalidation on pipeline changes.
- `src/source/Render/Renderer/SdlGpuValidation.h` — one build-policy constant for `SDL_CreateGPUDevice`.
- `src/source/Render/Renderer/DrawCommandHistory.h` — four independent adjacent-draw history slots.
- `src/source/Render/Models/GpuSkinningPath.h` — testable eligible/submitted/failed path policy.
- `src/source/Render/Text/SdlTtfGpuTextProperties.h` — shared name for the patched SDL_ttf upload-count property.
- `src/source/Render/Text/SdlTtfTextCache.h` — bounded `(font, UTF-8 string)` cache API.
- `src/source/Render/Text/SdlTtfTextCache.cpp` — SDL_ttf ownership, layout, eviction, transient fallback.
- `cmake/ApplyGitPatch.cmake` — idempotent `git apply` wrapper used during FetchContent population.
- `cmake/patches/sdl_ttf-3.2.2-batch-glyph-uploads.patch` — one copy pass and command submission per text update.
- `src/bin/fonts/Cousine-Regular.ttf` — fixed-width packaged role, copied from the already-present ImGui submodule asset.
- `src/bin/fonts/Cousine-LICENSE.txt` — Cousine attribution and SIL Open Font License 1.1.
- `tests/render/test_sdl_gpu_replay_state.cpp` — replay-cache behavior.
- `tests/render/test_sdl_gpu_validation.cpp` — Debug/`NDEBUG` policy function.
- `tests/text/test_sdl_ttf_text_cache.cpp` — real SDL_ttf cache/layout/lifetime behavior.
- `tests/text/test_sdl_ttf_fetchcontent_patch.cmake` — exact pin, applied patch, batched submission, property contract.
- `tests/render/test_gpu_skinning_path.cpp` — eligible submission never selects CPU fallback.
- `tests/platform/test_bundled_font_roles.cpp` — deterministic role resolution.
- `tests/platform/test_bundled_font_assets.cmake` — packaged font files and runtime copy.
- `docs/build/rendering-parity.md` — build policy, diagnostics, exception record, native verification table.

**Modify**

- `CMakeLists.txt` — exact SDL_ttf commit and FetchContent patch command.
- `src/source/Render/Renderer/MuRendererSDLGpu.cpp` — cached replay, counters, validation flag, 2D merge histories, required skinning, font-role loading.
- `src/source/Render/Renderer/MuRenderer.h` — new statistics fields, font reload API, updated skinning contract, font family at initialization.
- `src/source/Render/Renderer/QuadTopology.h` — contiguous triangle merge predicate using a caller-provided vertex stride.
- `src/source/Render/Text/CUIRenderTextSDLTtf.h` — replace one mutable `TTF_Text` with the bounded cache.
- `src/source/Render/Text/CUIRenderTextSDLTtf.cpp` — configure/create before measurement, reuse draw data, consume upload counts.
- `src/source/Render/Models/ZzzBMD.cpp` — distinguish ineligible CPU skinning from failed eligible GPU submission.
- `src/source/Core/Utilities/FrameProfiler.h` — 2D, skinning, bind, uniform, and glyph counters.
- `src/source/Scenes/SceneManager.cpp` — show the new counters in `$glstats` and timing logs.
- `src/source/Core/Platform/BundledFonts.h` — default family, fixed role, constexpr resolver.
- `src/source/Core/Platform/GdiText.cpp` — packaged default and Debug-only system fallback.
- `src/source/App/Platform/Windows/Winmain.cpp` — selected family handoff, fixed GDI face, checked registration, live reload order.
- `tests/render/CMakeLists.txt`, `tests/text/CMakeLists.txt`, `tests/platform/CMakeLists.txt` — register focused tests.
- `tests/render/test_quad_topology.cpp` — 2D/3D stride and capacity cases.
- `tests/render/test_pipeline_fail_fast.cmake` — all skinned sets required.
- `tests/render/test_imgui_sdlgpu_backend.cmake` — editor invalidates replay cache before state restoration.
- `tests/core/test_frame_profiler.cpp` — new counter collection.
- `tests/test_copy_runtime_assets.cmake` — font staging assertions.
- `docs/build/README.md` — SDL GPU backend names and parity-guide link.

## Task 1: Record the pre-change baseline

**Files:** None.

- [ ] **Step 1: Confirm the worktree and baseline revision**

Run:

```bash
git status --short
git rev-parse --short HEAD
```

Expected: no status output; revision includes approved design commit `132ea6a3` or its descendant.

- [ ] **Step 2: Configure clean editor-off and editor-on macOS baselines**

Run:

```bash
cmake --preset macos-arm64 -B out/build/render-parity-baseline-editor-off -DENABLE_EDITOR=OFF -DBUILD_TESTING=ON
cmake --preset macos-arm64 -B out/build/render-parity-baseline-editor-on -DENABLE_EDITOR=ON -DBUILD_TESTING=ON
```

Expected: both configure successfully.

- [ ] **Step 3: Build and test both baselines before production edits**

Run:

```bash
cmake --build out/build/render-parity-baseline-editor-off --config Debug -j4
ctest --test-dir out/build/render-parity-baseline-editor-off -C Debug --output-on-failure
cmake --build out/build/render-parity-baseline-editor-on --config Debug -j4
ctest --test-dir out/build/render-parity-baseline-editor-on -C Debug --output-on-failure
```

Expected: capture exact pass/fail output in the implementation notes. Any failure here is pre-existing; compare it verbatim after each task.

## Task 2: Cache repeated SDL GPU replay state

**Files:**

- Create: `src/source/Render/Renderer/SdlGpuReplayState.h`
- Create: `src/source/Render/Renderer/SdlGpuReplayState.cpp`
- Create: `tests/render/test_sdl_gpu_replay_state.cpp`
- Modify: `tests/render/CMakeLists.txt`
- Modify: `src/source/Render/Renderer/MuRendererSDLGpu.cpp:1775-1980`
- Modify: `src/source/Render/Renderer/MuRenderer.h:128-143`
- Modify: `tests/render/test_imgui_sdlgpu_backend.cmake`

- [ ] **Step 1: Write the failing replay-cache tests**

Add this target to `tests/render/CMakeLists.txt`:

```cmake
mu_add_test(
    NAME sdl_gpu_replay_state_tests
    SOURCES
        test_sdl_gpu_replay_state.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/SdlGpuReplayState.cpp
    LINK_LIBS SDL3::SDL3
)
```

Create `tests/render/test_sdl_gpu_replay_state.cpp` with these behaviors:

```cpp
#include <array>
#include <cstddef>
#include <cstdint>

#include <doctest.h>

#include "Render/Renderer/SdlGpuReplayState.h"

namespace
{
template <typename T>
T* Pointer(std::uintptr_t value)
{
    return reinterpret_cast<T*>(value);
}
}

TEST_CASE("identical replay state is skipped")
{
    Render::SdlGpuReplayState state;
    const std::array<std::byte, 4> uniforms{std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}};

    CHECK(state.SelectPipeline(Pointer<SDL_GPUGraphicsPipeline>(1)));
    CHECK(state.SelectFragmentSampler(Pointer<SDL_GPUTexture>(2), Pointer<SDL_GPUSampler>(3)));
    CHECK(state.SelectVertexUniforms(uniforms));
    CHECK(state.SelectFragmentUniforms(uniforms));
    CHECK(state.SelectVertexStorageBuffer(Pointer<SDL_GPUBuffer>(4)));
    CHECK(state.SelectIndexBuffer(Pointer<SDL_GPUBuffer>(5), 16, SDL_GPU_INDEXELEMENTSIZE_16BIT));

    CHECK_FALSE(state.SelectPipeline(Pointer<SDL_GPUGraphicsPipeline>(1)));
    CHECK_FALSE(state.SelectFragmentSampler(Pointer<SDL_GPUTexture>(2), Pointer<SDL_GPUSampler>(3)));
    CHECK_FALSE(state.SelectVertexUniforms(uniforms));
    CHECK_FALSE(state.SelectFragmentUniforms(uniforms));
    CHECK_FALSE(state.SelectVertexStorageBuffer(Pointer<SDL_GPUBuffer>(4)));
    CHECK_FALSE(state.SelectIndexBuffer(Pointer<SDL_GPUBuffer>(5), 16, SDL_GPU_INDEXELEMENTSIZE_16BIT));
}

TEST_CASE("pipeline changes invalidate descriptor-backed state")
{
    Render::SdlGpuReplayState state;
    const std::array<std::byte, 2> uniforms{std::byte{7}, std::byte{8}};

    state.SelectPipeline(Pointer<SDL_GPUGraphicsPipeline>(1));
    state.SelectFragmentSampler(Pointer<SDL_GPUTexture>(2), Pointer<SDL_GPUSampler>(3));
    state.SelectVertexUniforms(uniforms);
    state.SelectFragmentUniforms(uniforms);
    state.SelectVertexStorageBuffer(Pointer<SDL_GPUBuffer>(4));
    state.SelectIndexBuffer(Pointer<SDL_GPUBuffer>(5), 0, SDL_GPU_INDEXELEMENTSIZE_16BIT);

    CHECK(state.SelectPipeline(Pointer<SDL_GPUGraphicsPipeline>(6)));
    CHECK(state.SelectFragmentSampler(Pointer<SDL_GPUTexture>(2), Pointer<SDL_GPUSampler>(3)));
    CHECK(state.SelectVertexUniforms(uniforms));
    CHECK(state.SelectFragmentUniforms(uniforms));
    CHECK(state.SelectVertexStorageBuffer(Pointer<SDL_GPUBuffer>(4)));
    CHECK_FALSE(state.SelectIndexBuffer(Pointer<SDL_GPUBuffer>(5), 0, SDL_GPU_INDEXELEMENTSIZE_16BIT));
}

TEST_CASE("viewport scissor and editor invalidation are explicit")
{
    Render::SdlGpuReplayState state;
    const SDL_GPUViewport viewport{1.0f, 2.0f, 640.0f, 480.0f, 0.0f, 1.0f};
    const SDL_Rect scissor{1, 2, 640, 480};

    CHECK(state.SelectViewport(viewport));
    CHECK(state.SelectScissor(scissor));
    CHECK_FALSE(state.SelectViewport(viewport));
    CHECK_FALSE(state.SelectScissor(scissor));

    state.Invalidate();
    CHECK(state.SelectViewport(viewport));
    CHECK(state.SelectScissor(scissor));
}
```

- [ ] **Step 2: Run the focused test and verify RED**

Run:

```bash
cmake --build out/build/render-parity-baseline-editor-off --config Debug --target sdl_gpu_replay_state_tests -j4
```

Expected: FAIL because `Render/Renderer/SdlGpuReplayState.h` does not exist.

- [ ] **Step 3: Add the minimal cache type**

`SdlGpuReplayState` must expose exactly these operations:

```cpp
class SdlGpuReplayState final
{
public:
    bool SelectPipeline(SDL_GPUGraphicsPipeline* pipeline);
    bool SelectFragmentSampler(SDL_GPUTexture* texture, SDL_GPUSampler* sampler);
    bool SelectVertexUniforms(std::span<const std::byte> bytes);
    bool SelectFragmentUniforms(std::span<const std::byte> bytes);
    bool SelectVertexStorageBuffer(SDL_GPUBuffer* buffer);
    bool SelectIndexBuffer(SDL_GPUBuffer* buffer, Uint32 offset, SDL_GPUIndexElementSize elementSize);
    bool SelectViewport(const SDL_GPUViewport& viewport);
    bool SelectScissor(const SDL_Rect& scissor);
    void Invalidate();
};
```

Use one fixed 192-byte array for vertex uniforms, one 48-byte array for fragment uniforms, stored sizes, pointer/offset fields, and validity bits. `SelectPipeline` must invalidate only fragment sampler, vertex uniforms, fragment uniforms, and vertex storage. Index, viewport, and scissor remain valid. `Invalidate()` clears every validity bit. No allocation, ownership, backend branch, hash map, or virtual interface.

- [ ] **Step 4: Replace duplicated replay calls with focused helpers**

In `MuRendererSDLGpu.cpp`, extract these file-private helpers from `EndFrame()`:

```cpp
static bool BindReplayPipeline(const RenderCmd& command, Render::SdlGpuReplayState& state,
                               const SDL_Rect& scissor);
static void PushReplayVertexUniforms(const void* data, std::size_t size, Render::SdlGpuReplayState& state);
static void PushReplayFragmentUniforms(const FogUniform& uniforms, Render::SdlGpuReplayState& state);
static void BindReplayFragmentSampler(const RenderCmd& command, Render::SdlGpuReplayState& state);
static void BindReplayIndexBuffer(SDL_GPUBuffer* buffer, Uint32 offset, SDL_GPUIndexElementSize elementSize,
                                  Render::SdlGpuReplayState& state);
static void ReplayDrawCommand(const RenderCmd& command, bool boneDataReady, const SDL_Rect& scissor,
                              Render::SdlGpuReplayState& state);
```

Required behavior:

```cpp
const bool pipelineChanged = state.SelectPipeline(command.pipeline);
if (pipelineChanged)
{
    SDL_BindGPUGraphicsPipeline(s_renderPass, command.pipeline);
    SDL_SetGPUScissor(s_renderPass, &scissor);
    ++s_dbgPipelineBindsThisFrame;
}
```

Push uniform bytes only when their `Select*Uniforms` call returns true. Bind texture/sampler only when `SelectFragmentSampler` returns true. Bind the skinned storage buffer only when `SelectVertexStorageBuffer` returns true. Bind static or strip indices only when `SelectIndexBuffer` returns true. Continue binding the vertex buffer every draw because its offset normally changes.

For explicit state commands:

```cpp
if (state.SelectViewport(command.viewport))
    SDL_SetGPUViewport(s_renderPass, &command.viewport);
if (state.SelectScissor(command.scissor))
    SDL_SetGPUScissor(s_renderPass, &command.scissor);
```

After the editor overlay:

```cpp
g_MuEditorCore.RenderDrawData(s_cmdBuf, s_renderPass);
state.Invalidate();
if (state.SelectViewport(currentViewport))
    SDL_SetGPUViewport(s_renderPass, &currentViewport);
if (state.SelectScissor(currentScissor))
    SDL_SetGPUScissor(s_renderPass, &currentScissor);
```

Add per-frame and `RendererStats` fields for `pipelineBinds`, `samplerBinds`, `vertexUniformPushes`, and `fragmentUniformPushes`. Reset them in `BeginFrame`, assign them in `EndFrame`, include them in the 60-frame `MU_RENDER_TIMING` log.

- [ ] **Step 5: Tighten the editor contract**

Extend `tests/render/test_imgui_sdlgpu_backend.cmake` to require `state.Invalidate()` after `RenderDrawData`, followed by viewport and scissor restoration before the next game draw. Keep the existing one-editor-command checks.

- [ ] **Step 6: Run focused tests and verify GREEN**

Run:

```bash
cmake --build out/build/render-parity-baseline-editor-off --config Debug --target sdl_gpu_replay_state_tests -j4
ctest --test-dir out/build/render-parity-baseline-editor-off -C Debug -R 'sdl_gpu_replay_state|imgui_sdlgpu_backend' --output-on-failure
```

Expected: all selected tests PASS.

- [ ] **Step 7: Commit**

```bash
git add src/source/Render/Renderer/SdlGpuReplayState.* src/source/Render/Renderer/MuRendererSDLGpu.cpp src/source/Render/Renderer/MuRenderer.h tests/render
git commit -m "perf(render): cache repeated SDL GPU state"
```

## Task 3: Disable GPU validation in `NDEBUG` builds

**Files:**

- Create: `src/source/Render/Renderer/SdlGpuValidation.h`
- Create: `tests/render/test_sdl_gpu_validation.cpp`
- Modify: `tests/render/CMakeLists.txt`
- Modify: `src/source/Render/Renderer/MuRendererSDLGpu.cpp:1098-1110`

- [ ] **Step 1: Write the failing policy test**

```cpp
#include <doctest.h>

#include "Render/Renderer/SdlGpuValidation.h"

TEST_CASE("GPU validation follows assertion policy")
{
    CHECK(Render::GpuValidationEnabled(false));
    CHECK_FALSE(Render::GpuValidationEnabled(true));
}
```

Register `sdl_gpu_validation_tests` with `mu_add_test`.

- [ ] **Step 2: Run and verify RED**

Run:

```bash
cmake --build out/build/render-parity-baseline-editor-off --config Debug --target sdl_gpu_validation_tests -j4
```

Expected: FAIL because `SdlGpuValidation.h` does not exist.

- [ ] **Step 3: Add the shared build policy**

```cpp
#pragma once

namespace Render
{
[[nodiscard]] constexpr bool GpuValidationEnabled(bool assertionsDisabled)
{
    return !assertionsDisabled;
}

#ifdef NDEBUG
inline constexpr bool kGpuValidationEnabled = GpuValidationEnabled(true);
#else
inline constexpr bool kGpuValidationEnabled = GpuValidationEnabled(false);
#endif
}
```

Pass `Render::kGpuValidationEnabled` as the second argument to `SDL_CreateGPUDevice`. Log exactly:

```cpp
mu::log::Get("render")->info("SDL_gpu -- validation: {}",
                              Render::kGpuValidationEnabled ? "enabled" : "disabled");
```

No environment variables, OS branches, native validation calls, or new CMake option.

- [ ] **Step 4: Run and verify GREEN**

```bash
cmake --build out/build/render-parity-baseline-editor-off --config Debug --target sdl_gpu_validation_tests -j4
ctest --test-dir out/build/render-parity-baseline-editor-off -C Debug -R sdl_gpu_validation --output-on-failure
cmake --build out/build/render-parity-baseline-editor-off --config Release --target Main -j4
```

Expected: test PASS; both Debug and Release compile.

- [ ] **Step 5: Commit**

```bash
git add src/source/Render/Renderer/SdlGpuValidation.h src/source/Render/Renderer/MuRendererSDLGpu.cpp tests/render
git commit -m "fix(render): disable GPU validation in release"
```

## Task 4: Merge adjacent 2D text and screen draws

**Files:**

- Create: `src/source/Render/Renderer/DrawCommandHistory.h`
- Modify: `src/source/Render/Renderer/QuadTopology.h`
- Modify: `src/source/Render/Renderer/MuRendererSDLGpu.cpp:522-665,1460-1480,2430-2475,2647-2730,2775-2910`
- Modify: `tests/render/test_quad_topology.cpp`

- [ ] **Step 1: Write failing geometry tests**

Add:

```cpp
#include "Render/Renderer/DrawCommandHistory.h"

TEST_CASE("adjacent triangle draws require contiguous vertices for their own stride")
{
    constexpr std::uint32_t firstOffset = 64;
    constexpr std::uint32_t firstVertexCount = 6;
    constexpr auto stride2D = static_cast<std::uint32_t>(sizeof(mu::Vertex2D));
    constexpr auto stride3D = static_cast<std::uint32_t>(sizeof(mu::Vertex3D));

    CHECK(Render::Topology::CanMergeTriangleDraws(
        firstOffset, firstVertexCount, firstOffset + firstVertexCount * stride2D, stride2D));
    CHECK(Render::Topology::CanMergeTriangleDraws(
        firstOffset, firstVertexCount, firstOffset + firstVertexCount * stride3D, stride3D));
    CHECK_FALSE(Render::Topology::CanMergeTriangleDraws(
        firstOffset, firstVertexCount, firstOffset + firstVertexCount * stride2D + 1, stride2D));
    CHECK_FALSE(Render::Topology::CanMergeTriangleDraws(firstOffset, firstVertexCount, firstOffset, 0));
}

TEST_CASE("screen-space quads preserve the static index ceiling")
{
    constexpr auto stride = static_cast<std::uint32_t>(sizeof(mu::Vertex2D));
    constexpr std::uint32_t firstOffset = 128;
    constexpr std::uint32_t secondOffset = firstOffset + 4 * stride;

    CHECK(Render::Topology::CanMergeQuadDraws(firstOffset, 6, secondOffset, 6, stride, 4096));
    CHECK_FALSE(Render::Topology::CanMergeQuadDraws(firstOffset, 4096 * 6, secondOffset, 6, stride, 4096));
}

TEST_CASE("2D and 3D command families retain independent history")
{
    Render::DrawCommandHistory history;
    history.fill(99);

    Render::PreviousDrawCommand(history, Render::DrawCommandFamily::TextTriangles2D) = 7;
    Render::PreviousDrawCommand(history, Render::DrawCommandFamily::ScreenQuads2D) = 8;

    CHECK(Render::PreviousDrawCommand(history, Render::DrawCommandFamily::Triangles3D) == 99);
    CHECK(Render::PreviousDrawCommand(history, Render::DrawCommandFamily::Quads3D) == 99);
    CHECK(Render::PreviousDrawCommand(history, Render::DrawCommandFamily::TextTriangles2D) == 7);
    CHECK(Render::PreviousDrawCommand(history, Render::DrawCommandFamily::ScreenQuads2D) == 8);
}
```

- [ ] **Step 2: Run and verify RED**

```bash
cmake --build out/build/render-parity-baseline-editor-off --config Debug --target quad_topology_tests -j4
```

Expected: compile FAIL because `DrawCommandHistory.h` and `CanMergeTriangleDraws` are missing.

- [ ] **Step 3: Add the minimum generic geometry predicate**

Create `DrawCommandHistory.h` with no class or allocation:

```cpp
#pragma once

#include <array>
#include <cstddef>

namespace Render
{
enum class DrawCommandFamily : std::size_t
{
    Triangles3D,
    Quads3D,
    TextTriangles2D,
    ScreenQuads2D,
    Count,
};

using DrawCommandHistory = std::array<std::size_t, static_cast<std::size_t>(DrawCommandFamily::Count)>;

[[nodiscard]] constexpr std::size_t& PreviousDrawCommand(DrawCommandHistory& history, DrawCommandFamily family)
{
    return history[static_cast<std::size_t>(family)];
}

[[nodiscard]] constexpr const std::size_t& PreviousDrawCommand(const DrawCommandHistory& history,
                                                               DrawCommandFamily family)
{
    return history[static_cast<std::size_t>(family)];
}
}
```

```cpp
[[nodiscard]] constexpr bool CanMergeTriangleDraws(std::uint32_t previousVertexOffset,
                                                   std::uint32_t previousVertexCount,
                                                   std::uint32_t nextVertexOffset,
                                                   std::uint32_t vertexStride)
{
    if (previousVertexCount == 0 || vertexStride == 0)
        return false;

    const std::uint64_t previousEnd = static_cast<std::uint64_t>(previousVertexOffset) +
                                      static_cast<std::uint64_t>(previousVertexCount) * vertexStride;
    return previousEnd == nextVertexOffset;
}
```

- [ ] **Step 4: Generalize existing merge helpers**

Use these signatures:

```cpp
[[nodiscard]] static bool MergeAdjacentTriangleCommand(const RenderCmd& command,
                                                       std::size_t& previousCommand,
                                                       Uint32 vertexStride);
[[nodiscard]] static bool MergeAdjacentQuadCommand(const RenderCmd& command,
                                                   std::size_t& previousCommand,
                                                   Uint32 vertexStride);
```

Both helpers must call existing `ClassifyBatchBreak`; triangle geometry uses `CanMergeTriangleDraws`; quad geometry uses `CanMergeQuadDraws`. Maintain four indices, reset all four in `BeginFrame`, update only the matching family after an unmerged push. State commands, another draw family, noncontiguous bytes, incompatible pipeline/texture/sampler/uniforms/blend/depth/cull, and quad capacity remain hard boundaries.

Use one `Render::DrawCommandHistory s_previousDrawCommands`; map call sites exactly:

```cpp
RenderTriangles      -> DrawCommandFamily::Triangles3D,      sizeof(Vertex3D)
RenderQuad3D         -> DrawCommandFamily::Quads3D,          sizeof(Vertex3D)
SubmitTextTriangles  -> DrawCommandFamily::TextTriangles2D,  sizeof(Vertex2D)
RenderQuad2D         -> DrawCommandFamily::ScreenQuads2D,    sizeof(Vertex2D)
```

Increment `s_dbgMerged2DDrawsThisFrame` and `FrameProfiler::Counter::Merged2DDraws` only for text triangles and screen quads. Add `merged2DDrawCalls` to `RendererStats` and logs.

- [ ] **Step 5: Run and verify GREEN**

```bash
cmake --build out/build/render-parity-baseline-editor-off --config Debug --target quad_topology_tests -j4
ctest --test-dir out/build/render-parity-baseline-editor-off -C Debug -R quad_topology --output-on-failure
```

Expected: all selected tests PASS.

- [ ] **Step 6: Commit**

```bash
git add src/source/Render/Renderer/DrawCommandHistory.h src/source/Render/Renderer/QuadTopology.h src/source/Render/Renderer/MuRendererSDLGpu.cpp src/source/Render/Renderer/MuRenderer.h tests/render/test_quad_topology.cpp
git commit -m "perf(render): batch adjacent 2D draws"
```

## Task 5: Patch pinned SDL_ttf glyph uploads into one submission

**Files:**

- Modify: `CMakeLists.txt:196-202`
- Create: `cmake/ApplyGitPatch.cmake`
- Create: `cmake/patches/sdl_ttf-3.2.2-batch-glyph-uploads.patch`
- Create: `src/source/Render/Text/SdlTtfGpuTextProperties.h`
- Create: `tests/text/test_sdl_ttf_fetchcontent_patch.cmake`
- Modify: `tests/text/CMakeLists.txt`

- [ ] **Step 1: Write the failing dependency contract**

The CMake test must verify:

```cmake
set(expected_revision "a1ce3670aec736ecbf0936c43f2f0cc53aa61e5b")
set(upload_property "MuMain.SDL_ttf.gpu_text.uploaded_glyphs")
```

Read root `CMakeLists.txt`, the populated `${SDL3_TTF_SOURCE}/src/SDL_gpu_textengine.c`, the project property header, and the patch. Require the exact revision, `PATCH_COMMAND`, exactly two `SDL_SubmitGPUCommandBuffer` calls in the populated source (atlas creation plus the shared glyph batch), one glyph-batch `SDL_BeginGPUCopyPass`, and the same upload property string in patched source and project header.

Register it with:

```cmake
FetchContent_GetProperties(SDL3_ttf SOURCE_DIR SDL3_TTF_SOURCE)
add_test(
    NAME sdl_ttf_fetchcontent_patch
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_ROOT_CMAKE=${CMAKE_SOURCE_DIR}/CMakeLists.txt"
        "-DSDL3_TTF_SOURCE=${SDL3_TTF_SOURCE}"
        "-DMU_PROPERTY_HEADER=${CMAKE_SOURCE_DIR}/src/source/Render/Text/SdlTtfGpuTextProperties.h"
        "-DMU_PATCH=${CMAKE_SOURCE_DIR}/cmake/patches/sdl_ttf-3.2.2-batch-glyph-uploads.patch"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/test_sdl_ttf_fetchcontent_patch.cmake"
)
```

- [ ] **Step 2: Run and verify RED**

```bash
ctest --test-dir out/build/render-parity-baseline-editor-off -C Debug -R sdl_ttf_fetchcontent_patch --output-on-failure
```

Expected: FAIL because the dependency is tag-pinned without a patch and still submits once per glyph.

- [ ] **Step 3: Add an idempotent FetchContent patch command**

`cmake/ApplyGitPatch.cmake` must:

1. Validate `SOURCE_DIR` and `PATCH_FILE`.
2. `find_package(Git REQUIRED)`.
3. Run `git apply --check` and apply when clean.
4. If forward check fails, run `git apply --reverse --check`; succeed only when already applied.
5. Fail with both command outputs when neither check succeeds.

Change the declaration to:

```cmake
FetchContent_Declare(SDL3_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG        a1ce3670aec736ecbf0936c43f2f0cc53aa61e5b
    GIT_SHALLOW    FALSE
    PATCH_COMMAND
        "${CMAKE_COMMAND}"
        "-DSOURCE_DIR=<SOURCE_DIR>"
        "-DPATCH_FILE=${CMAKE_SOURCE_DIR}/cmake/patches/sdl_ttf-3.2.2-batch-glyph-uploads.patch"
        -P "${CMAKE_SOURCE_DIR}/cmake/ApplyGitPatch.cmake"
)
```

- [ ] **Step 4: Add the platform-neutral SDL_ttf patch**

Patch only `src/SDL_gpu_textengine.c`. Introduce this batch context:

```c
typedef struct GlyphUploadBatch
{
    SDL_GPUDevice *device;
    SDL_GPUCommandBuffer *command_buffer;
    SDL_GPUCopyPass *copy_pass;
    int upload_count;
} GlyphUploadBatch;
```

`UpdateGPUTexture` must accept `GlyphUploadBatch *batch`, create/map/copy one transfer buffer per glyph, record `SDL_UploadToGPUTexture(batch->copy_pass, ...)`, release the transfer buffer, increment `upload_count`, and never acquire, end, or submit a command buffer.

`CreateMissingGlyphs` must create the atlas first, then acquire one command buffer and begin one copy pass, pass the same batch through recursive `ResolveMissingGlyphs`, end the pass once, submit once on success, cancel once on failure, and return `upload_count` through an output parameter. Check null map, command-buffer, copy-pass, submit, and cancel failures without leaking transfer buffers.

Add `uploaded_glyphs` to `TTF_GPUTextEngineTextData`. Store the batch count for the text update. In `CreateText`, set:

```c
SDL_SetNumberProperty(TTF_GetTextProperties(text),
                      "MuMain.SDL_ttf.gpu_text.uploaded_glyphs",
                      data->uploaded_glyphs);
```

Create the matching project constant:

```cpp
#pragma once

namespace Render::Text
{
inline constexpr char kUploadedGlyphCountProperty[] = "MuMain.SDL_ttf.gpu_text.uploaded_glyphs";
}
```

No SDL backend source changes, native API calls, or per-OS patch variants.

- [ ] **Step 5: Re-populate, build, and verify GREEN**

Use a new build directory so FetchContent cannot reuse the unpatched source:

```bash
cmake --preset macos-arm64 -B out/build/render-parity-patched-sdlttf -DENABLE_EDITOR=OFF -DBUILD_TESTING=ON
cmake --build out/build/render-parity-patched-sdlttf --config Debug --target sdl_gpu_replay_state_tests -j4
ctest --test-dir out/build/render-parity-patched-sdlttf -C Debug -R sdl_ttf_fetchcontent_patch --output-on-failure
```

Expected: dependency contract PASS; SDL_ttf and one project target compile against the patched source.

- [ ] **Step 6: Commit**

```bash
git add CMakeLists.txt cmake/ApplyGitPatch.cmake cmake/patches/sdl_ttf-3.2.2-batch-glyph-uploads.patch src/source/Render/Text/SdlTtfGpuTextProperties.h tests/text
git commit -m "perf(text): batch SDL_ttf glyph uploads"
```

## Task 6: Retain configured SDL_ttf text and reuse its layout

**Files:**

- Create: `src/source/Render/Text/SdlTtfTextCache.h`
- Create: `src/source/Render/Text/SdlTtfTextCache.cpp`
- Create: `tests/text/test_sdl_ttf_text_cache.cpp`
- Modify: `tests/text/CMakeLists.txt`
- Modify: `src/source/Render/Text/CUIRenderTextSDLTtf.h`
- Modify: `src/source/Render/Text/CUIRenderTextSDLTtf.cpp`
- Modify: `src/source/Core/Utilities/FrameProfiler.h`
- Modify: `tests/core/test_frame_profiler.cpp`

- [ ] **Step 1: Write failing real SDL_ttf tests**

Build the test with `SdlTtfTextCache.cpp`, `SDL3_ttf::SDL3_ttf`, and:

```cmake
target_compile_definitions(sdl_ttf_text_cache_tests PRIVATE
    MU_TEST_FONT_PATH="${CMAKE_SOURCE_DIR}/src/bin/fonts/DejaVuSans.ttf")
```

Use `TTF_Init`, `TTF_OpenFont(MU_TEST_FONT_PATH, 14.0f)`, and `TTF_CreateSurfaceTextEngine`. Test:

```cpp
#include <SDL3_ttf/SDL_ttf.h>

#include <doctest.h>

#include "Render/Text/SdlTtfTextCache.h"

namespace
{
struct TtfFixture
{
    TtfFixture()
    {
        initialized = TTF_Init();
        if (initialized)
            font = TTF_OpenFont(MU_TEST_FONT_PATH, 14.0f);
        if (font != nullptr)
            engine = TTF_CreateSurfaceTextEngine();
    }

    ~TtfFixture()
    {
        if (engine != nullptr)
            TTF_DestroySurfaceTextEngine(engine);
        if (font != nullptr)
            TTF_CloseFont(font);
        if (initialized)
            TTF_Quit();
    }

    bool initialized = false;
    TTF_Font* font = nullptr;
    TTF_TextEngine* engine = nullptr;
};
}

TEST_CASE("recurring font and string reuse one configured text")
{
    TtfFixture fixture;
    REQUIRE(fixture.engine != nullptr);
    Render::Text::SdlTtfTextCache cache(fixture.engine, 4);
    auto first = cache.Prepare(fixture.font, "Character");
    auto second = cache.Prepare(fixture.font, "Character");

    REQUIRE(first.text != nullptr);
    CHECK(first.width > 0);
    CHECK(first.height > 0);
    CHECK(second.text == first.text);
    CHECK(second.width == first.width);
    CHECK(second.height == first.height);
    CHECK(cache.Size() == 1);
}

TEST_CASE("whole-cache eviction is bounded and deterministic")
{
    TtfFixture fixture;
    REQUIRE(fixture.engine != nullptr);
    Render::Text::SdlTtfTextCache cache(fixture.engine, 2);
    cache.Prepare(fixture.font, "one");
    cache.Prepare(fixture.font, "two");
    CHECK(cache.Size() == 2);
    cache.Prepare(fixture.font, "three");
    CHECK(cache.Size() == 1);
}

TEST_CASE("zero capacity uses a transient text without retaining it")
{
    TtfFixture fixture;
    REQUIRE(fixture.engine != nullptr);
    Render::Text::SdlTtfTextCache cache(fixture.engine, 0);
    auto prepared = cache.Prepare(fixture.font, "transient");
    REQUIRE(prepared.text != nullptr);
    CHECK(prepared.transient != nullptr);
    CHECK(cache.Size() == 0);
}
```

- [ ] **Step 2: Run and verify RED**

```bash
cmake --build out/build/render-parity-patched-sdlttf --config Debug --target sdl_ttf_text_cache_tests -j4
```

Expected: FAIL because `SdlTtfTextCache` does not exist.

- [ ] **Step 3: Implement the bounded cache**

Use this public shape:

```cpp
class SdlTtfTextCache final
{
public:
    using TextOwner = std::unique_ptr<TTF_Text, decltype(&TTF_DestroyText)>;

    struct PreparedText
    {
        TTF_Text* text = nullptr;
        int width = 0;
        int height = 0;
        TextOwner transient{nullptr, TTF_DestroyText};
    };

    explicit SdlTtfTextCache(TTF_TextEngine* engine = nullptr, std::size_t capacity = 256);
    ~SdlTtfTextCache();
    PreparedText Prepare(TTF_Font* font, std::string_view utf8);
    void Reset(TTF_TextEngine* engine);
    void Clear();
    [[nodiscard]] std::size_t Size() const;
};
```

Store a `std::vector<Entry>` where each entry owns `font`, `std::string utf8`, `TTF_Text*`, width, and height. Linear lookup is bounded at 256 entries. Create the text directly with its final font and string, then call `TTF_GetTextSize` on that same object. When full, clear all entries before inserting the new one. Catch `std::bad_alloc`; return the newly created object in `PreparedText::transient` instead of retaining it. Destroy every `TTF_Text` in `Clear` and before changing engines in `Reset`.

Mark the ceiling:

```cpp
// ponytail: 256 retained labels; raise only if post-warmup layout profiling still shows churn.
```

- [ ] **Step 4: Replace mutable-text churn in `CUIRenderTextSDLTtf`**

Remove `m_ttfText` and `ConfigureText`. Add mutable `SdlTtfTextCache m_textCache`. `Create` calls `m_textCache.Reset(engine)` after validating the engine and default font. `Release` clears and resets the cache before nulling the active font.

Both `MeasureText` and `RenderText` must convert UTF-16 to UTF-8, call `Prepare(font, utf8)`, and use the returned width/height. `RenderText` then calls `TTF_GetGPUTextDrawData(prepared.text)` on the same configured object. Position and ABGR color remain in `SubmitTextDrawData`; color is not a cache key and no `TTF_SetTextColor` call remains.

After `TTF_GetGPUTextDrawData`, consume the patch property once:

```cpp
const SDL_PropertiesID properties = TTF_GetTextProperties(prepared.text);
const Sint64 uploadedGlyphs = SDL_GetNumberProperty(
    properties, Render::Text::kUploadedGlyphCountProperty, 0);
if (uploadedGlyphs > 0)
{
    FrameProfiler::Count(FrameProfiler::Counter::GlyphUploads, static_cast<std::uint32_t>(uploadedGlyphs));
    SDL_SetNumberProperty(properties, Render::Text::kUploadedGlyphCountProperty, 0);
}
```

Extend the existing no-legacy-measurement policy to reject `TTF_GetStringSize` inside `RenderText`; `MeasureText` must also use the cache.

- [ ] **Step 5: Extend profiler tests**

Add `GlyphUploads` and `Merged2DDraws` to `FrameProfiler::Counter`. In `test_frame_profiler.cpp`, count both inside `Pass::UI` and assert pass plus total values. Keep collection gated by the existing `$glstats` switch for this task.

- [ ] **Step 6: Run and verify GREEN**

```bash
cmake --build out/build/render-parity-patched-sdlttf --config Debug --target sdl_ttf_text_cache_tests test_frame_profiler -j4
ctest --test-dir out/build/render-parity-patched-sdlttf -C Debug -R 'sdl_ttf_text_cache|frame_profiler|no_legacy_text_measurement' --output-on-failure
```

Expected: all selected tests PASS; no SDL_ttf lifetime warning or leak output.

- [ ] **Step 7: Commit**

```bash
git add src/source/Render/Text src/source/Core/Utilities/FrameProfiler.h tests/text tests/core/test_frame_profiler.cpp tests/ui/test_no_legacy_text_measurement.cmake
git commit -m "perf(text): reuse prepared SDL_ttf text"
```

## Task 7: Require GPU skinning and separate path counters

**Files:**

- Create: `src/source/Render/Models/GpuSkinningPath.h`
- Create: `tests/render/test_gpu_skinning_path.cpp`
- Modify: `tests/render/test_pipeline_fail_fast.cmake`
- Modify: `tests/render/CMakeLists.txt`
- Modify: `src/source/Render/Renderer/MuRendererSDLGpu.cpp:3510-3560,3830-3970`
- Modify: `src/source/Render/Renderer/MuRenderer.h:158-171`
- Modify: `src/source/Render/Models/ZzzBMD.cpp:1640-1710`
- Modify: `src/source/Core/Utilities/FrameProfiler.h`
- Modify: `tests/core/test_frame_profiler.cpp`

- [ ] **Step 1: Write the failing path test and tighten the pipeline contract**

Create `tests/render/test_gpu_skinning_path.cpp`:

```cpp
#include <doctest.h>

#include "Render/Models/GpuSkinningPath.h"

TEST_CASE("only ineligible meshes select CPU skinning")
{
    CHECK(Render::Models::ResolveGpuSkinningPath(false, false) ==
          Render::Models::GpuSkinningPath::CpuIneligible);
    CHECK(Render::Models::ResolveGpuSkinningPath(true, true) ==
          Render::Models::GpuSkinningPath::GpuSubmitted);
    CHECK(Render::Models::ResolveGpuSkinningPath(true, false) ==
          Render::Models::GpuSkinningPath::GpuFailed);
}
```

Register `gpu_skinning_path_tests` with `mu_add_test`.

Update the pipeline test to require `skinned_textured.vert` with `fatal=true`, nine required pipeline sets, one optional `2d-depth-on` set, and all four skinned descriptors through `buildRequiredPipeline`.

Run:

```bash
cmake --build out/build/render-parity-patched-sdlttf --config Debug --target gpu_skinning_path_tests -j4
ctest --test-dir out/build/render-parity-patched-sdlttf -C Debug -R renderer_pipeline_fail_fast --output-on-failure
```

Expected: compile FAIL because `GpuSkinningPath.h` is missing; pipeline contract FAIL under current optional behavior.

- [ ] **Step 2: Make the shader and every selected pipeline required**

Load `skinned_textured.vert` with `fatal=true`; clean up previously loaded shader handles on failure. Build all skinned pipeline arrays with `buildRequiredPipeline` without guarding on a nullable shader. Keep `2d-depth-on` as the only optional set. Existing diagnostics already include driver, set, layout, shader names, blend, index, depth, cull, and immediate SDL error; retain them unchanged.

- [ ] **Step 3: Stop eligible failures from entering CPU skinning**

Create the policy header:

```cpp
#pragma once

namespace Render::Models
{
enum class GpuSkinningPath
{
    CpuIneligible,
    GpuSubmitted,
    GpuFailed,
};

[[nodiscard]] constexpr GpuSkinningPath ResolveGpuSkinningPath(bool eligible, bool submitted)
{
    if (!eligible)
        return GpuSkinningPath::CpuIneligible;
    return submitted ? GpuSkinningPath::GpuSubmitted : GpuSkinningPath::GpuFailed;
}
}
```

Use this control flow in `BMD::RenderMesh`:

```cpp
const bool gpuSkinningEligible = CanGpuSkinMesh(finalRenderFlags, renderFlags, m_pCurrentBoneTransform);
bool gpuSkinningSubmitted = false;
if (gpuSkinningEligible)
{
    const auto textureCoordinates = GetSkinningTextureCoordinates(renderFlags);
    auto skinnedVertices = GetRendererSkinnedVertexScratch(maxVertexCount);
    std::size_t skinnedVertexCount = 0;
    for (int triangleIndex = 0; triangleIndex < m->NumTriangles; ++triangleIndex)
    {
        const auto* triangle = &m->Triangles[triangleIndex];
        for (int polygonIndex = 0; polygonIndex < triangle->Polygon; ++polygonIndex)
        {
            const int vertexIndex = triangle->VertexIndex[polygonIndex];
            const int normalIndex = triangle->NormalIndex[polygonIndex];
            const auto& vertex = m->Vertices[vertexIndex];
            const auto& normal = m->Normals[normalIndex];
            const auto& texCoord = m->TexCoords[triangle->TexCoordIndex[polygonIndex]];
            const bool useMeshTextureCoordinates = textureCoordinates == mu::SkinningTextureCoordinates::Mesh;
            const float u =
                texCoord.TexCoordU + (useMeshTextureCoordinates && EnableWave ? blendMeshTextureCoordU : 0.0f);
            const float v =
                texCoord.TexCoordV + (useMeshTextureCoordinates && EnableWave ? blendMeshTextureCoordV : 0.0f);
            const std::uint32_t color = PackABGR(BodyLight[0] * colorScale, BodyLight[1] * colorScale,
                                                 BodyLight[2] * colorScale, baseAlpha);
            skinnedVertices[skinnedVertexCount++] = {vertex.Position[0],
                                                     vertex.Position[1],
                                                     vertex.Position[2],
                                                     normal.Normal[0],
                                                     normal.Normal[1],
                                                     normal.Normal[2],
                                                     u,
                                                     v,
                                                     color,
                                                     vertex.Node,
                                                     normal.Node};
        }
    }

    mu::SkinningParameters skinning{
        .boneMatrices = {&m_pCurrentBoneTransform[0][0][0], MAX_BONES * 12u},
        .paletteVersion = g_BoneTransformVersion,
        .bodyOrigin = {BodyOrigin[0], BodyOrigin[1], BodyOrigin[2]},
        .bodyScale = BodyScale,
        .boneScale = m_LastBoneScale,
        .restPoseScale = m_LastSkinScale,
        .lightDirection = {m_LastLightPosition[0], m_LastLightPosition[1], m_LastLightPosition[2]},
        .textureCoordinateOffset = {blendMeshTextureCoordU, blendMeshTextureCoordV},
        .chromeWave = wave,
        .chromeWave2 = static_cast<int>(WorldTime) % 5000 * 0.00024f - 0.4f,
        .chromeLight = {static_cast<float>(cos(WorldTime * 0.001f)),
                        static_cast<float>(sin(WorldTime * 0.002f))},
        .chromeTimeTerm = static_cast<float>(WorldTime) * 0.00006f,
        .textureCoordinates = textureCoordinates,
        .translate = m_LastTranslate,
        .lightEnabled = usesCpuLighting,
    };
    gpuSkinningSubmitted =
        mu::GetRenderer().RenderSkinnedTriangles(skinnedVertices.first(skinnedVertexCount), 0u, skinning);
}

const Render::Models::GpuSkinningPath skinningPath =
    Render::Models::ResolveGpuSkinningPath(gpuSkinningEligible, gpuSkinningSubmitted);
if (skinningPath == Render::Models::GpuSkinningPath::GpuSubmitted)
{
    FrameProfiler::Count(FrameProfiler::Counter::GpuSkinningSubmissions);
    return;
}
if (skinningPath == Render::Models::GpuSkinningPath::GpuFailed)
{
    FrameProfiler::Count(FrameProfiler::Counter::GpuSkinningFailures);
    return;
}

FrameProfiler::Count(FrameProfiler::Counter::CpuSkinningIneligible);
EnsureCpuVertices(meshIndex);
```

Update the renderer interface comment: `false` means an eligible draw failed submission and must not trigger CPU fallback. Keep CPU skinning only for `CanGpuSkinMesh == false`.

- [ ] **Step 4: Add and display counters**

Add `GpuSkinningSubmissions`, `CpuSkinningIneligible`, and `GpuSkinningFailures` to `FrameProfiler::Counter`; test pass and total values. Add one `$glstats` line:

```text
Skin GPU:<submitted> CPU-ineligible:<cpu> Failed:<failed>
```

Add the same values to the 60-frame `MU_RENDER_TIMING` output before `ResetCounters`.

- [ ] **Step 5: Run and verify GREEN**

```bash
cmake --build out/build/render-parity-patched-sdlttf --config Debug --target Main test_frame_profiler -j4
ctest --test-dir out/build/render-parity-patched-sdlttf -C Debug -R 'renderer_pipeline_fail_fast|gpu_skinning_path|frame_profiler|shader.*validation' --output-on-failure
```

Expected: all selected tests PASS; exact 18-shader manifest remains unchanged.

- [ ] **Step 6: Commit**

```bash
git add src/source/Render/Renderer src/source/Render/Models/GpuSkinningPath.h src/source/Render/Models/ZzzBMD.cpp src/source/Core/Utilities/FrameProfiler.h src/source/Scenes/SceneManager.cpp tests/render tests/core/test_frame_profiler.cpp
git commit -m "fix(render): require GPU skinning resources"
```

## Task 8: Resolve every font role from packaged assets

**Files:**

- Modify: `src/source/Core/Platform/BundledFonts.h`
- Modify: `src/source/Core/Platform/GdiText.cpp`
- Modify: `src/source/Render/Renderer/MuRenderer.h`
- Modify: `src/source/Render/Renderer/MuRendererSDLGpu.cpp:736-810,1240-1365,2390-2425,4495-4510`
- Modify: `src/source/App/Platform/Windows/Winmain.cpp:1390-1525,1800-1905`
- Create: `src/bin/fonts/Cousine-Regular.ttf`
- Create: `src/bin/fonts/Cousine-LICENSE.txt`
- Create: `tests/platform/test_bundled_font_roles.cpp`
- Create: `tests/platform/test_bundled_font_assets.cmake`
- Modify: `tests/platform/CMakeLists.txt`
- Modify: `tests/test_copy_runtime_assets.cmake`

- [ ] **Step 1: Write failing role and packaging tests**

The C++ test must assert:

```cpp
CHECK(ResolveBundledFont("").family == std::string_view("DejaVu Sans"));
CHECK(ResolveBundledFont("unknown").family == std::string_view("DejaVu Sans"));
CHECK(ResolveBundledFont("Liberation Sans").regular == std::string_view("fonts/LiberationSans-Regular.ttf"));
CHECK(ResolveBundledFont("Liberation Sans").bold == std::string_view("fonts/LiberationSans-Bold.ttf"));
CHECK(std::string_view(kBundledFixedFont.family) == "Cousine");
CHECK(std::string_view(kBundledFixedFont.regular) == "fonts/Cousine-Regular.ttf");
```

The CMake test must require non-empty files for DejaVu regular/bold, Liberation regular/bold, Cousine regular, and all three license files. Run `CopyRuntimeAssets.cmake` with `MU_COPY_RUNTIME_ASSETS=ON` into a temporary directory and require the same font files there.

Run:

```bash
cmake --build out/build/render-parity-patched-sdlttf --config Debug --target bundled_font_role_tests -j4
ctest --test-dir out/build/render-parity-patched-sdlttf -C Debug -R 'bundled_font|copy_runtime_assets' --output-on-failure
```

Expected: FAIL because default/fixed roles and Cousine package files do not exist.

- [ ] **Step 2: Add the already-installed monospace asset**

Copy the exact existing ImGui asset:

```bash
cp src/ThirdParty/imgui/misc/fonts/Cousine-Regular.ttf src/bin/fonts/Cousine-Regular.ttf
shasum -a 256 src/ThirdParty/imgui/misc/fonts/Cousine-Regular.ttf src/bin/fonts/Cousine-Regular.ttf
```

Expected: both hashes identical. Add Cousine attribution plus the complete SIL Open Font License 1.1 text to `src/bin/fonts/Cousine-LICENSE.txt`.

- [ ] **Step 3: Extend the shared registry**

Keep the existing two selectable families. Add:

```cpp
inline constexpr std::string_view kDefaultBundledFontFamily = "DejaVu Sans";
inline constexpr BundledFont kBundledFixedFont{
    "Cousine", "fonts/Cousine-Regular.ttf", "fonts/Cousine-Regular.ttf"};

[[nodiscard]] constexpr const BundledFont& ResolveBundledFont(std::string_view configuredFamily)
{
    for (const BundledFont& font : kBundledFonts)
    {
        if (configuredFamily == font.family)
            return font;
    }
    for (const BundledFont& font : kBundledFonts)
    {
        if (kDefaultBundledFontFamily == font.family)
            return font;
    }
    return kBundledFonts[0];
}
```

- [ ] **Step 4: Load SDL_ttf roles transactionally**

Change initialization to:

```cpp
[[nodiscard]] bool InitSDLGpuRenderer(void* pNativeWindow, std::string_view fontFamily);
virtual bool ReloadTtfFonts(std::string_view fontFamily) { return false; }
```

In the SDL GPU renderer, resolve the executable base path plus explicit `regular`, `bold`, and fixed paths. Open temporary normal, bold, big-bold, and fixed handles. Only after all four succeed: close old handles and assign new ones. Warm all four after assignment. A failed live reload leaves the old set intact.

Packaged mode rules:

```cpp
#ifdef NDEBUG
inline constexpr bool kAllowSystemFontFallback = false;
#else
inline constexpr bool kAllowSystemFontFallback = true;
#endif
```

Release: missing/corrupt role logs family, role, exact path, SDL error, returns false, aborts renderer initialization. Debug: log `NON-PARITY developer font fallback`, then use the existing OS font candidates for availability. Keep this as the only font platform branch.

Pass the configured family from `Winmain.cpp` after `GameConfig::Load`. Empty or unknown selection resolves to DejaVu Sans. On live option changes, enforce this order:

```cpp
g_pRenderText->Release();
const bool fontsReloaded = mu::GetRenderer().ReloadTtfFonts(selectedFamily);
if (!fontsReloaded)
    mu::log::Get("render")->error("SDL_ttf -- keeping the previous font set after reload failure");
const bool textCreated = g_pRenderText->Create(g_hDC);
if (textCreated)
    g_pRenderText->SetFont(g_hFont);
```

This clears retained `TTF_Text` objects before old fonts close. Transactional reload keeps the previous handles valid when opening a new role fails, so text recreation still has a usable set.

- [ ] **Step 5: Make GDI/shim roles match SDL_ttf**

`CreateUIFont` must use `ResolveBundledFont(configured).family`; empty no longer means Tahoma/system default. `CreateNewFonts` must create default/bold/big from the selected family and fixed from `kBundledFixedFont.family`.

Windows `RegisterBundledFonts` must register both files for each selectable family plus Cousine, return false if a file is missing or `AddFontResourceExW` fails, roll back paths registered earlier in that call, and log the exact path. Release startup calls `ShutdownRendererWindow()`, `SDL_Quit()`, and returns `FALSE`; Debug logs non-parity and continues.

Non-Windows `GdiText.cpp` must resolve empty/Tahoma to `kDefaultBundledFontFamily`; `BundledFontPath` must check `kBundledFixedFont.family` before the selectable-family loop. Try the bundled path first, and compile environment/fontconfig/well-known-path candidates only when `!NDEBUG`. Its diagnostic string must name `bundled` or `NON-PARITY developer fallback`.

- [ ] **Step 6: Run and verify GREEN**

```bash
cmake --build out/build/render-parity-patched-sdlttf --config Debug --target bundled_font_role_tests Main -j4
ctest --test-dir out/build/render-parity-patched-sdlttf -C Debug -R 'bundled_font|copy_runtime_assets|sdl_ttf_text_cache' --output-on-failure
cmake --build out/build/render-parity-patched-sdlttf --config Release --target Main -j4
```

Expected: all selected tests PASS; Debug and Release compile; staged runtime contains all role files.

- [ ] **Step 7: Commit**

```bash
git add src/source/Core/Platform src/source/Render/Renderer src/source/App/Platform/Windows/Winmain.cpp src/bin/fonts tests/platform tests/test_copy_runtime_assets.cmake
git commit -m "fix(text): use bundled font roles"
```

## Task 9: Finish diagnostics and build-facing documentation

**Files:**

- Modify: `src/source/Render/Renderer/MuRenderer.h`
- Modify: `src/source/Render/Renderer/MuRendererSDLGpu.cpp`
- Modify: `src/source/Core/Utilities/FrameProfiler.h`
- Modify: `src/source/Scenes/SceneManager.cpp`
- Create: `docs/build/rendering-parity.md`
- Modify: `docs/build/README.md`

- [ ] **Step 1: Write failing counter assertions**

Extend `test_frame_profiler.cpp` to verify totals for:

```text
Merged2DDraws
GpuSkinningSubmissions
CpuSkinningIneligible
GpuSkinningFailures
GlyphUploads
```

Add compile-time checks in a render test for `RendererStats` fields:

```text
pipelineBinds
samplerBinds
vertexUniformPushes
fragmentUniformPushes
merged2DDrawCalls
```

Run the focused tests and confirm RED until every field/counter exists.

- [ ] **Step 2: Display one compact parity block**

Keep existing lines; add:

```text
Bind Pipe:<pipeline> Samp:<sampler> VU:<vertex> FU:<fragment>
2D Merge:<count> Glyph upload:<count>
Skin GPU:<submitted> CPU-ineligible:<cpu> Failed:<failed>
```

Add the same named fields to `[RENDER diag]` every 60 frames under `MU_RENDER_TIMING=1`. Do not add a new command, file logger, JSON exporter, backend branch, or sampling thread.

- [ ] **Step 3: Document policy and native verification**

`docs/build/rendering-parity.md` must contain:

1. SDL GPU backend table: macOS Metal, Linux Vulkan, Windows D3D12; application policy identical.
2. Debug validation enabled, `NDEBUG` disabled.
3. Required bundled roles and Release failure behavior.
4. SDL_ttf exact revision, upstream `main` checked at `a42434b8c96daaf7650dbd0befe480c090d1c2eb`, missing upstream batching, local platform-neutral patch, custom upload property.
5. `$glstats on` plus `MU_RENDER_TIMING=1` capture procedure using the same scene, account, camera, resolution, VSync, editor state, and 300-frame sample.
6. Explicit exception: Debug-only system font fallback is non-parity; release packages may not use it.
7. Verification table columns: date, revision, OS, GPU, driver, resolution, validation, requested/submitted draws, pipeline/sampler binds, vertex/fragment pushes, 2D merges, GPU/CPU/failed skinning, glyph uploads after warmup, replay/submit/frame milliseconds, result.
8. Rows for macOS, Linux, Windows marked `runtime evidence pending` until captured natively.
9. CI evidence note: run `32982393578` proved native Windows x64 Release/editor-off build, tests, exact 18-shader manifest, and artifact upload only. Job `98228153450` failed after publishing because it queried upstream PR 575 under the fork and received 404; neither job launched or profiled the client.

Update `docs/build/README.md`: Linux renders through SDL GPU/Vulkan, macOS through SDL GPU/Metal, Windows through SDL GPU/D3D12. Link the parity guide.

- [ ] **Step 4: Run focused diagnostics tests and verify GREEN**

```bash
cmake --build out/build/render-parity-patched-sdlttf --config Debug --target test_frame_profiler quad_topology_tests -j4
ctest --test-dir out/build/render-parity-patched-sdlttf -C Debug -R 'frame_profiler|quad_topology|sdl_gpu_replay_state|gpu_skinning|sdl_ttf|bundled_font' --output-on-failure
```

Expected: all selected tests PASS.

- [ ] **Step 5: Commit**

```bash
git add src/source/Render/Renderer src/source/Core/Utilities/FrameProfiler.h src/source/Scenes/SceneManager.cpp tests docs/build
git commit -m "docs(render): add parity verification procedure"
```

## Task 10: Full local verification and native handoff

**Files:** No new implementation files.

- [ ] **Step 1: Reconfigure fresh final editor-off and editor-on trees**

```bash
cmake --preset macos-arm64 -B out/build/render-parity-final-editor-off -DENABLE_EDITOR=OFF -DBUILD_TESTING=ON
cmake --preset macos-arm64 -B out/build/render-parity-final-editor-on -DENABLE_EDITOR=ON -DBUILD_TESTING=ON
```

Expected: both configure successfully; SDL_ttf patch reports applied once or already applied, never a partial/rejected patch.

- [ ] **Step 2: Build Debug and Release**

```bash
cmake --build out/build/render-parity-final-editor-off --config Debug -j4
cmake --build out/build/render-parity-final-editor-off --config Release -j4
cmake --build out/build/render-parity-final-editor-on --config Debug -j4
cmake --build out/build/render-parity-final-editor-on --config Release -j4
```

Expected: all four builds succeed.

- [ ] **Step 3: Run full CTest for both editor modes**

```bash
ctest --test-dir out/build/render-parity-final-editor-off -C Debug --output-on-failure
ctest --test-dir out/build/render-parity-final-editor-off -C Release --output-on-failure
ctest --test-dir out/build/render-parity-final-editor-on -C Debug --output-on-failure
ctest --test-dir out/build/render-parity-final-editor-on -C Release --output-on-failure
```

Expected: compare against Task 1 baseline. No new failure is accepted.

- [ ] **Step 4: Validate shader and runtime manifests explicitly**

```bash
ctest --test-dir out/build/render-parity-final-editor-off -C Release -R 'shader.*validation|staged_shader_blob_validation|checked_in_shader_blob_validation|bundled_font_assets|copy_runtime_assets' --output-on-failure
```

Expected: exact 18-shader manifest and every packaged font role PASS.

- [ ] **Step 5: Capture macOS runtime evidence without claiming cross-platform completion**

From the Release runtime directory:

```bash
MU_RENDER_TIMING=1 ./Main.app/Contents/MacOS/Main
```

Enable `$glstats on`, warm the selected scene, record 300 frames, loaded font paths, validation state, and counters in `docs/build/rendering-parity.md`. Expected macOS evidence: validation disabled, bundled roles loaded, zero failed GPU skinning, zero post-warmup glyph uploads for recurring labels, fewer submitted 2D draws and actual binds than requested draws.

- [ ] **Step 6: Record Windows/Linux verification as required external work**

Run the same Release scene and procedure natively on Linux/Vulkan and Windows/D3D12. Do not mark parity complete from macOS-only results. The implementation handoff must say exactly which rows remain pending, including GPU, driver, and revision required for each run.

- [ ] **Step 7: Inspect final diff and side effects**

```bash
git status --short
git diff --stat main...HEAD
git diff --check
git log --oneline --decorate -10
```

Expected: only scoped renderer/text/font/test/docs changes; no generated build output, dependency source, user assets, credentials, or unrelated files tracked.

## Self-review checklist

- [x] Every approved issue maps to one task: replay, validation, 2D batching, text layout/cache, SDL_ttf upload batching, skinning failure, font roles, diagnostics/docs.
- [x] No performance fix uses `_WIN32`, `__linux__`, or `__APPLE__`; only documented Debug font availability fallback does.
- [x] Every production behavior starts with a failing focused test or policy guard.
- [x] Text cache destruction precedes font and engine destruction on shutdown and live reload.
- [x] Pipeline changes invalidate descriptor-backed replay state; editor rendering invalidates all replay state.
- [x] Windows/Linux runtime results remain unclaimed until native evidence fills their table rows.
