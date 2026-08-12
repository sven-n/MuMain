# Gotchas, Technical Invariants & Architectural Patterns

This document catalogues critical technical gotchas, thread safety rules, cache invalidation pitfalls, and performance invariants established during the implementation of the GPU skinning engine and modern graphics subsystem.

---

## 1. Skeletal Skinning & Memory Invariants

### 1.1 `MAX_BONES = 200` Allocation & Upload Rule
- **Gotcha**: Equipped armor models (`bmdArmor`) store small internal `NumBones` counts (e.g. 12 to 15), but armor vertices reference bone indices across the entire character skeleton (0 to 199).
- **Rule**: `BoneUBO::UploadBones()` must upload all `MAX_BONES` (200) matrices. Heap allocations for `BoneTransform` across all character, pet, and object structures must allocate `new vec34_t[MAX_BONES]` to prevent out-of-bounds heap reads during upload.

### 1.2 Version-Stamped Palette Deduplication vs. Stack-Local Pointer Hazard
- **Gotcha**: Sub-item rendering (e.g., wings, attached effect meshes) allocates `BoneTransform` arrays on the CPU execution stack. Consecutive rendering calls frequently receive the *exact same stack memory address* populated with *different matrix data*.
- **Rule**: Never use pointer-only comparison (`pTransform == g_pLastTransform`) to skip uploading bone matrix uniform buffers. Updates MUST bump a version stamp (`g_BoneTransformVersion++`), and cache checks must validate both `(ptr, version)` pairs.

### 1.3 `TransformCheap` Lazy Materialization & Ambient Global State Snapshotting
- **Pattern**: `BMD::TransformCheap()` defers CPU vertex skinning calculations until a consumer explicitly requests CPU vertex/normal positions via `EnsureCpuVertices()`.
- **Gotcha**: Ambient global state (such as `BoneScale`) set prior to `Transform()` and reset immediately after will be lost if read later during deferred materialization.
- **Rule**: Deferred computations depending on ambient global state **MUST** snapshot that state into instance variables (e.g., `m_LastBoneScale`) at the moment the request is made.

### 1.4 Planar Shadow Division Explosion & Dummy Bones
- **Gotcha**: Planar shadow projection applies a GPU skew matrix involving division: $\text{skewX} = \frac{z_{\text{rel}} \cdot (x_{\text{rel}} + s_x)}{z_{\text{rel}} - s_y}$. Models with uninitialized or Dummy bone slots carry stale memory into `u_Bones`, causing division near zero and stretching shadows across billions of world units ("infinity shadow").
- **Rule**: `BoneTransform` arrays must be fully initialized/zeroed, including Dummy bone slots that do not contain mesh vertex weights.

---

## 2. Rendering, Shaders & Color Pipeline Gotchas

### 2.1 Vertex Color Clamping in GLSL Shaders
- **Gotcha**: Fixed-function OpenGL clamped vertex colors `glColor3fv()` to `[0, 1]` automatically before texture modulation. In GLSL shaders, unclamped vertex color values $> 1.0$ (from intense point lights) multiply texture colors to pure blown-out white.
- **Rule**: All fragment shaders modulating vertex colors must explicitly clamp input colors: `clamp(v_Color, 0.0, 1.0)`.

### 2.2 Collapsed/Aliased Render-Mode Enum Dispatch
- **Gotcha**: `BMD::RenderMesh()` evaluates GPU skinning eligibility against `finalRenderFlags`. Multiple distinct visual flags (CHROME2, CHROME3, CHROME5, CHROME6, CHROME7, METAL) collapse into the single enum value `RENDER_CHROME`.
- **Rule**: When adding GPU shader eligibility checks keyed on an enum, verify every flag that maps to that bucket shares identical downstream shader mathematics.

### 2.3 Bind Cache Invalidation on Resource Deletion (`glDelete*`)
- **Gotcha**: Driver resource deletion functions (`glDeleteVertexArrays`, `glDeleteTextures`) silently reset active driver bindings to 0 when deleting the currently bound resource. A driver bind cache unaware of deletion retains stale resource IDs, causing subsequent `Bind*(id)` calls to falsely hit the cache and skip real binding.
- **Rule**: Every deletion call site must explicitly invalidate binding caches via `InvalidateVAOCache()`, `InvalidateTextureCache()`, etc.

---

## 3. Concurrency, Animation & Frame Decoupled Sync

### 3.1 `AnimationTaskPool` Thread Safety & Read Barriers
- **Gotcha**: `AnimationTaskPool` computes character skeletal animations asynchronously across background worker threads. `WaitCharactersAnimation()` provides the mandatory synchronization barrier.
- **Rule**: Code executing outside the main render pass (e.g., logic ticks or mouse updates) **MUST NEVER** read `o->BoneTransform` matrices directly while worker threads are actively mutating them.

### 3.2 Decoupled Render FPS & Fixed-Timestep Animation Acceleration
- **Gotcha**: Functions advancing state by a fixed per-call increment scaled by `FPS_ANIMATION_FACTOR` (cloth physics `Move2()`, wing flap animation, tour cameras) will speed up proportionally when render FPS increases from 60 FPS to 250+ FPS if invoked from a render-frequency call site.
- **Rule**: All render-frequency animation/physics updates must be throttled using real-time timers (`std::chrono::steady_clock`) to maintain a constant ~60 Hz simulation rate independent of rendering frame rate.

### 3.3 One-Shot UI Render Flags
- **Gotcha**: UI flags that are set by logic ticks and immediately cleared on the first render frame (e.g., `m_bRenderSkillInfo`) cause visual flickering/strobing when rendering uncapped above the 50 Hz/60 Hz logic tick rate.
- **Rule**: Render pass code must only **read** UI state flags; logic tick handlers remain the sole authority responsible for clearing interaction flags.

---

## 4. Profiling & Instrumentation Gotchas

### 4.1 GPU Timer Scope Must Not Exceed the Real Draw Submission
- **Gotcha**: `FrameProfiler::Scope`'s default `GL_TIMESTAMP` pair brackets the *entire* CPU-scope lifetime (`FRAME_PROFILE(Pass)`), not just the GL draw calls inside it. `GL_TIMESTAMP` measures wall-clock time between two points in the GPU's command stream — if the CPU does real work (gather, sort, culling) between pushing the begin-marker and submitting the actual draw, and the GPU finishes the begin-marker before the CPU finishes that work, the resulting "GPU time" reading includes that CPU-bound idle wait. This produced a false ~3-5ms "GPU regression" for [GLP-16](glperf/README.md#glp-16--bucket-terrain-tiles-by-texture-pair)'s terrain bucketing, when the terrain draws themselves cost <0.01ms (confirmed via Nsight Graphics GPU Trace) — three code-change attempts chased a cost that didn't exist before the measurement bug itself was found.
- **Rule**: For any pass whose `FRAME_PROFILE` scope spans non-trivial CPU work *before* its GL draw calls, use `FRAME_PROFILE_CPU_ONLY(Pass)` instead of `FRAME_PROFILE(Pass)`, and call `FrameProfiler::GpuTimerBegin(Pass)` / `GpuTimerEnd(Pass)` manually, tightly around just the real draw submission. CPU-ms accounting (`AccumulatorMs`) is unaffected either way — the CPU scope should still cover the whole pass; only the GPU timer's bracket needs narrowing. Trust an outlier GPU-ms reading only as far as the code that produced it — cross-check with an external GPU profiler (Nsight Graphics, RenderDoc) before spending implementation effort chasing it.
- **Corollary — which direction the artifact runs.** It inflates GPU *toward* CPU; it cannot invent GPU time that exceeds the CPU scope. So `GPU ≈ CPU` on a pass that dribbles out many small draws is probably starvation, not GPU work — but `GPU ≫ CPU` (e.g. `Chars` at GPU 1.51 ms against CPU 0.24 ms) is the one shape the artifact **cannot** manufacture, and can be trusted as real GPU cost.
- **Related discriminator, not a GPU-timer issue**: GPU work is build-independent, CPU work is not. If a cost is roughly equal in Debug and Release it is GPU or driver; if it shrinks ~3× in Release it was MSVC Debug instrumentation (`_ITERATOR_DEBUG_LEVEL=2` inflates every `std::vector` operation frame-wide). Several apparent wins in the GLP series evaporated on that test — check it *before* acting on a Debug measurement, not after.

### 4.2 State Writers Flush IR *Before* Mutating — So the Batch Key Never Sees the Mismatch
- **Gotcha**: `IR::Flush()` is called from inside each state writer's dirty check, **before** the state actually changes (`EnableAlphaBlend()` flushes, *then* sets `AlphaBlendType = 3`). This ordering is correct and required — the pending batch must be drawn under the state it was built with. But it means the batch is already gone by the time the next `IR::Begin()` compares `IRBatchKey` against live state, so `Begin()` records **no key mismatch**, only "nothing was pending."
- **Why this misleads**: instrumenting `Begin()`'s merge decision on a frame with 2,918 unbatched particle draws reported `key mismatches: 0`, which reads as "render state never changes" — the exact opposite of the truth. It was read that way, and a correct earlier finding (that `RenderParticles()` churns blend mode per particle) was retracted on the strength of it. The real answer only appeared after tagging the flush *call sites*: `blend: 2,940` against 2,918 particles, i.e. one blend change per particle.
- **Rule**: `Begin()`'s key comparison can only explain batch breaks that `Begin()` itself causes. To find out why a pass is not batching, attribute the **`IR::Flush()` call sites**, not the key fields. A `key mismatch: 0` reading is evidence about `Begin()`, and about nothing else.
- **Related**: `$glstats` has **no blend-mode counter**. `TexBind`/`ProgBind`/`UniWr` staying flat across a burst does not mean state is stable — blend is the state that churns in the particle path, and none of those three counters can see it. Absence of a counter has twice been misread here as absence of change.
