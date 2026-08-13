# GLP Milestone Catalog — Core Profile FPS Regression Series

This document is the reference catalog for the **`GLP-xx`** ("glperf", GL performance) milestone
series. GLP investigates and fixes a Core Profile FPS regression reported on weak/integrated
graphics hardware — **Intel HD 530 / i5-6500 / 8GB** — after the client's fixed-function-pipeline
(FFP) retirement (see the [DXP series](../dxp-milestones.md)): "older version 100-60fps, newer
version 40-23fps" at the same in-game location. A secondary symptom, multi-client throttling worse
than a proportional GPU split, is tracked alongside it.

For the broader rendering-engine migration this series sits inside, see the
[GPU Skinning & Modern Graphics Subsystem index](../README.md).

**Scope of this document**: only milestones with a landed implementation are cataloged below —
some build-verified only, some further along, none yet confirmed against the actual Intel HD 530
target hardware that motivated the series. Not-yet-started tickets and paused investigations are
tracked separately and are not listed here.

## Series structure

GLP is organized into five phases, each targeting a different suspected cost center. Only phases
with at least one landed milestone appear in the catalog below (Phase 3, fragment-stage
occupancy/early-Z, has no landed work yet and so has no entries here):

- **Phase 0 — Ground truth.** Instrumentation only, no render-behavior change. Every later task is
  measured against the counters and GPU pass timers this phase adds.
- **Phase 1 — Independent cheap removals.** Small, low-risk per-call GL overhead cuts, each its
  own commit, landable in any order.
- **Phase 2 — Kill the per-draw allocation.** The primary suspected fix: replaces per-update
  uniform-buffer orphaning with a ring allocator, and shrinks the bone-palette upload.
- **Phase 3 — Fragment stage occupancy and early-Z.** Shader variant infrastructure to cut register
  pressure and restore early depth testing that an unconditional `discard` currently defeats.
- **Phase 4 — Collapse the draw count.** Terrain draw-call batching and IR (ImmediateRenderer)
  batching, plus mesh geometry/vertex-format bandwidth cuts.

**Fast path.** The series originally planned Phase 1 before Phase 2, but on 2026-08-11 the owner
chose to skip straight from Phase 0 to Phase 2 — verified to have no dependency on Phase 1 — to get
the fastest possible yes/no answer on the Intel HD 530 regression: **GLP-01 → GLP-08 → GLP-09 →
measure.** GLP-09's ring allocator is the entire "Core Profile added per-draw driver overhead"
hypothesis, isolated to one file and one commit so its result is attributable.

**Stopping rule.** Phases 1 and 2 target the driver overhead a Core Profile migration regression
is expected to come from. If Intel HD 530 numbers return to the FFP-era baseline after Phase 2, the
series stops there and re-measures before starting Phase 3. Phases 3 and 4 raise the performance
ceiling past the pre-migration numbers on their own merits, but are not regression fixes and are
not to be justified as such.

---

## Milestone Summary Table

### Phase 0 — Ground Truth (August 11, 2026)

| Milestone | Subsystem | Focus / Objective | Status |
|---|---|---|---|
| **[GLP-01](#glp-01--gl-call-counters--gpu-pass-timers)** | Profiling | GL call counters and `GL_TIMESTAMP` GPU pass timers in `FrameProfiler`. | Implemented — in-game overlay confirmed; Release-build Intel HD 530 baseline table outstanding |

### Phase 1 — Independent Cheap Removals (August 11, 2026)

| Milestone | Subsystem | Focus / Objective | Status |
|---|---|---|---|
| **[GLP-03](#glp-03--stop-rewriting-sampler-uniforms-per-bind)** | Shaders | Stop rewriting sampler-unit uniforms on every texture bind. | Completed |
| **[GLP-04](#glp-04--dirty-check-terrain-water-flags)** | Terrain | Dirty-check `TerrainShader::SetWaterFlags`. | Completed |
| **[GLP-05](#glp-05--active-texture-unit-cache-monopoly)** | State Cache | Make `BindState` the sole owner of the active texture unit. | Implemented — partial smoke test only, broad in-game check outstanding |
| **[GLP-06](#glp-06--remove-redundant-per-draw-calls-in-rendermesh)** | Model & Mesh | Remove redundant per-draw VAO/attribute/unbind calls in `BMD::RenderMesh`. | Implemented — reached live gameplay with zero GL errors; CPU dynamic-VBO fallback path not isolated |

### Phase 2 — Kill the Per-Draw Allocation (August 11, 2026)

| Milestone | Subsystem | Focus / Objective | Status |
|---|---|---|---|
| **[GLP-08](#glp-08--gl-context-version-chain--capability-probe)** | RHI / Context | Request the highest available GL core context and publish an `RHI::Caps` capability probe. | Implemented — dev-machine verified; Intel HD 530 boot-log capture outstanding |
| **[GLP-09](#glp-09--ring-buffer-ubo-allocator)** | Uniform Buffers | Replace per-update UBO orphaning with a persistently-mapped ring allocator. | Implemented — dev-machine verified via counters and a short smoke test; extended soak test and Intel HD 530 numbers outstanding |
| **[GLP-10](#glp-10--content-dirty-check-on-uniform-block-updates)** | Uniform Buffers | Skip the ring write when a uniform block's contents are unchanged since the same frame. | Implemented — build-verified only; visual pass and skip-rate capture pending |
| **[GLP-11](#glp-11--bone-palette-as-3vec4-affine-rows)** | GPU Skinning | Store the bone palette as 3×vec4 affine rows instead of `mat4`. | Implemented — build-verified, shaders confirmed compiling; skinning correctness not yet visually verified |

### Phase 4 — Collapse the Draw Count (August 11, 2026)

| Milestone | Subsystem | Focus / Objective | Status |
|---|---|---|---|
| **[GLP-16](#glp-16--bucket-terrain-tiles-by-texture-pair)** | Terrain | Bucket visible terrain tiles by `(baseTexture, overlayTexture, waterFlags)` into one draw per bucket. | Implemented, GPU-ms finding root-caused and fixed, re-confirmed Debug+Release — visual identity checklist and Intel HD 530 numbers still outstanding |
| **[GLP-19](#glp-19--batch-ir-draws-across-beginend-pairs)** | ImmediateRenderer | Defer `IR::End()`; merge consecutive `Begin`/`End` pairs into one draw when render state is unchanged. | Implemented and correct — **measured to deliver nothing for particles** (see status), real win for UI/joints |

### Phase 5 — Skill-Cast Cost (August 12, 2026)

Opened after the first target-hardware report: static maps improved, but casting Chaotic Diseier
dropped framerate roughly 50%. These milestones came out of that investigation.

| Milestone | Subsystem | Focus / Objective | Status |
|---|---|---|---|
| **[GLP-24](#glp-24--glstats-pass-attribution)** | Profiling | Make `$glstats` account for the whole frame; fix the GPU query ring read. | Completed — was blocking; every earlier reading in the series had this blind spot |
| **[GLP-25](#glp-25--ir-streaming-ring-growth-policy)** | ImmediateRenderer | Grow the IR streaming ring when it wraps repeatedly instead of only on oversized appends. | Completed — orphans 21 → 1 in a like-for-like capture |
| **[GLP-29](#glp-29--particle--joint-vertex-generation-cpu)** | Effects | Cut per-particle CPU vertex-generation cost (2D rotation collapse, heap-free quad scratch). | Implemented, paused — −35% Debug frame CPU, **no measurable Release win** |

---

## Detailed Milestone Catalog

### Phase 0 — Ground Truth

#### GLP-01 — GL Call Counters + GPU Pass Timers
- **Scope**: `Core/Utilities/FrameProfiler.h`, `Render/Core/BindState.cpp`, `Render/RHI/RHI_GL.cpp`, the four shader classes' `glUniform*` sites, `Render/Terrain/ZzzLodTerrain.cpp`, `Render/Models/ZzzBMD.cpp`, `Render/Shaders/PlanarShadowShader.cpp`.
- **Purpose**: Extends the existing `FrameProfiler` (which predates this series — see [Corrections to the Source Analysis](#corrections-to-the-source-analysis)) with per-pass GL entry-point/draw-call/buffer-update counters and `GL_TIMESTAMP`-based GPU pass timers, surfaced through a new `$glstats` console toggle. This is the blocking prerequisite for the rest of the series: every later task's win is measured against it rather than inferred from reading code.
- **Status**: Implemented. Debug and Release both build clean, counters are gated behind `$glstats` so the disabled path is a no-op, and a live in-game capture in Lorencia (Debug, dev machine) confirms the overlay produces internally consistent numbers. The actual deliverable — a Release-build baseline table (idle/crowded/2-client/4-client) captured on the Intel HD 530 / i5-6500 machine — is still empty; the `$glstats off` perf-neutrality check has also not been separately measured.

---

### Phase 1 — Independent Cheap Removals

#### GLP-03 — Stop Rewriting Sampler Uniforms per Bind
- **Scope**: `Render/Shaders/BMDMeshShader.cpp`, `Render/Shaders/PassthroughShader.cpp`.
- **Purpose**: Removes the per-bind `glUniform1i(m_LocTex, slot)` call from both shaders' `SetTexture`. Sampler unit assignment is a program-level binding-table write, done once permanently at `CreateGL()`; the per-bind rewrite was pure overhead and forced an undocumented call-ordering constraint (base texture bound last) that this change also removes. Item-specular render modes 6/7 previously paid four such rewrites per draw.
- **Status**: Completed. Built clean, and verified in-game at the worst-case path (render mode 7, all four textures bound) — the owner confirmed a +15-gear character rendered correctly. The `$glstats` capture showed `UniformWrites` dropping by 465, matching the predicted order of magnitude. Modes 0/2/3/5/6 were not separately re-confirmed but exercise a strict subset of the same code path as mode 7.

#### GLP-04 — Dirty-Check Terrain Water Flags
- **Scope**: `Render/Shaders/TerrainShader.h`, `Render/Shaders/TerrainShader.cpp`.
- **Purpose**: Adds the same tri-state dirty-check pattern already used elsewhere in `TerrainShader` to `SetWaterFlags`, which previously issued two unconditional `glUniform1i` calls per visible terrain tile even though both flags are `0/0` for almost every tile on almost every map.
- **Status**: Completed. Verified in-game by walking the Lorencia moat (the dominant plain-water path) with no stuck or bleeding water flags, and the counter capture showed `UniformWrites` dropping by 2,318 — closely matching the predicted removable-call count. Three map-specific water branches (PK-field/Doppelganger2, Atlans/Doppelganger3 overlay substitution, Crywolf splash particles) and the map-switch stale-flag case were not independently re-tested.

#### GLP-05 — Active Texture Unit Cache Monopoly
- **Scope**: `Render/Core/BindState.cpp`/`.h`, `Render/Shaders/TerrainShader.cpp`, `Render/Shaders/PassthroughShader.cpp`.
- **Purpose**: Removes two raw `glActiveTexture(GL_TEXTURE0)` restore calls that bypassed `BindState`'s bind-state monopoly, which is what previously forced `BindState` to leave the active texture unit uncached. With both restores gone, `BindTexture2D` now also caches and dirty-checks the active unit, cutting three `glActiveTexture` calls per terrain tile down to only those that change state.
- **Status**: Implemented, but flagged in its own source as the highest-risk task in Phase 1 — a bind-cache desync would silently render the wrong texture on the wrong unit. Both GL-call audit greps came back clean and Debug/Release build clean. A partial smoke test reached a live Character Select screen with correctly colored dragon/wing mounts and no visible corruption, and an owner re-capture in Lorencia showed no visible regression, but neither exercised item-specular render modes 5–7 (the densest multi-unit user in the tree), the grass-map fixed-function fallback, or texture deletion during a map change — all still outstanding per the task's own mandatory checklist.

#### GLP-06 — Remove Redundant Per-Draw Calls in RenderMesh
- **Scope**: `Render/Models/ZzzBMD.cpp`.
- **Purpose**: Bakes the dynamic (CPU-fallback) draw path's vertex attribute format into `m_VAO_Static` once, at buffer-creation time, instead of re-issuing three `glVertexAttribPointer` + three `glEnableVertexAttribArray` calls on every mesh draw. Also removes a trailing `glBindBuffer(GL_ARRAY_BUFFER, 0)` on both the GPU-skinned and dynamic draw paths, which affects nothing once a VAO owns the attribute state. Absorbs and supersedes the prior `GLS-11` task.
- **Status**: Implemented. Reached live gameplay with zero logged GL errors before an unrelated session-kick ended the run, and Debug/Release build clean with no new warnings. The specific CPU dynamic-VBO fallback path this task targets (chrome4/oil-flagged meshes, or any mesh lacking a static GPU VBO) was not isolated and exercised — the Lorencia test scenes used for verification mostly hit the GPU-skinned path instead, so this task's actual target surface remains unconfirmed.

---

### Phase 2 — Kill the Per-Draw Allocation

#### GLP-08 — GL Context Version Chain + Capability Probe
- **Scope**: `App/Platform/Windows/Winmain.cpp`, `Render/RHI/RHI.h`/`.cpp`/`RHI_GL.cpp`, `Core/Utilities/Log/ErrorReport.cpp`, `config.ini`.
- **Purpose**: Replaces the client's fixed request for exactly GL 3.3 core with a descending `{4,5} → {4,3} → {3,3}` attempt chain, and publishes an `RHI::Caps` struct (`bufferStorage`, `vertexAttribBinding`, `programBinary`, `timerQuery`, UBO offset alignment, max uniform block size) so later tasks can branch on real hardware capability instead of assuming a version. Adds a `config.ini [Render] MaxGLVersion` override as a rollback path.
- **Status**: Implemented and dev-machine verified — on an NVIDIA RTX 5070 Ti, the context loop, capability probe, and `MaxGLVersion` rollback all confirmed working, and a KHR_debug soak test (which also fixed a second, unrelated pre-existing compile bug in the disabled debug callback) through a full boot-to-gameplay session logged zero GL errors or Core Profile violations. The actual deliverable — booting on the Intel HD 530 machine and recording which GL version and which capability flags it returns — has not been done; whether `bufferStorage` (which GLP-09 depends on) is true on that hardware is still unknown.

#### GLP-09 — Ring-Buffer UBO Allocator
- **Scope**: `Render/RHI/RHI_GL.cpp` only (plus a doc-comment update in `RHI.h`).
- **Purpose**: Replaces per-update uniform-buffer orphaning (`glBufferData(nullptr)` + `glBufferSubData`, landed as the prior `GLS-08` fix) with a persistently-mapped, per-binding-slot ring buffer plus `glBindBufferRange` — one GL call and a `memcpy` per update instead of a driver-side storage respecify per call. Falls back to unsynchronized mapping, then to the exact prior orphaning behavior, on machines without `ARB_buffer_storage`. This is considered the primary suspected fix for the reported regression: `BMDFlagsCB` alone previously allocated once per BMD mesh draw with no dedup, hundreds of times per frame per client.
- **Status**: Implemented. Dev-machine counters confirm the mechanism is active and behaving as designed — `BufferOrphans` dropped from 912 to 4 in a like-for-like capture — and a short smoke test showed no visual artifacts. This task's own verification section explicitly calls for several outstanding items: several minutes of continuous play to exercise a ring wrap (the failure mode — torn uniform data — only appears on wrap, not on frame one), targeted checks of crowded-town bone churn, item-specular gear, fog transitions, and 2D/3D mode switches, Intel HD 530 hardware numbers, and multi-client (2/4 client) numbers. None of these have been captured yet, and which fallback rung is actually active on Intel HD 530 is inferred rather than confirmed.

#### GLP-10 — Content Dirty-Check on Uniform Block Updates
- **Scope**: `Render/RHI/RHI_GL.cpp`, plus counter/overlay plumbing in `FrameProfiler.h` and `Scenes/SceneManager.cpp`.
- **Purpose**: Skips the ring write and `glBindBufferRange` entirely when a uniform block's bytes are identical to what that slot already holds *and* the cached range is guaranteed still live (same-frame only — ring wraps and segment rotation invalidate the cache). Targets `BMDFlagsCB`, which a body and its equipped armor pieces frequently upload as byte-identical blocks.
- **Status**: Implemented, build-verified only. A dev-machine capture showed a plausible skip rate (196 skips against 2,302 draws — nowhere near the "skip count equals draw count" red flag for over-aggressive deduping) and no regression in general play, but this task's own verification section calls its failure mode "subtle" (stale uniforms: an armor piece rendering with a previous mesh's render mode, or lighting from the wrong body) and explicitly requires a careful, dedicated visual pass plus a deliberate negative test with item-specular gear. Neither has been done; frame-boundary drift over several minutes and Intel HD 530 numbers are also outstanding.

#### GLP-11 — Bone Palette as 3×vec4 Affine Rows
- **Scope**: `Render/Core/BoneUBO.h`/`.cpp`, `Render/Shaders/BMDMeshShader.cpp`, `Render/Shaders/PlanarShadowShader.cpp`, `Render/Shaders/glsl/bmd_mesh.vert`.
- **Purpose**: Stops expanding the engine's native 3×4 affine bone matrices into 4×4 on the CPU solely to upload a constant `(0,0,0,1)` column the shader doesn't need. The GLSL `BoneMatrices` block changes from `mat4 u_Bones[200]` to `vec4 u_Bones[600]` (3 rows per bone), reconstructed in the vertex shader via dot products, cutting the largest single per-object upload in the renderer by 25% (12,800 → 9,600 bytes per skeleton) and turning a 26-line CPU repack loop into a single `memcpy`.
- **Status**: Implemented. Both consuming shaders (`BMDMeshShader`, `PlanarShadowShader`) are confirmed compiling and linking against the new layout, and the row/column transpose math (the task's own named highest risk) was hand-derived and algebraically verified before implementation, not just tested. However, skinning correctness — this task's actual test surface, and one the task itself calls "visually obvious" if wrong — has only had a general, undirected in-game pass ("looks good," no exploded/detached geometry). The specific tests still outstanding: a deliberate 360° rotation under directional light to isolate a normal-transform (lighting-only) bug from a position bug, equipped armor on every slot, wings/mounts/cloth, and planar shadows specifically.

---

### Phase 4 — Collapse the Draw Count

#### GLP-16 — Bucket Terrain Tiles by Texture Pair
- **Scope**: `Render/Terrain/ZzzLodTerrain.cpp`, `Core/Utilities/FrameProfiler.h`, `Scenes/MainScene.cpp`.
- **Purpose**: Splits `RenderTerrainFace` into a gather phase (unchanged per-tile logic: alpha collapse, water-flag derivation, overlay substitution, side-effect calls) and a draw phase that groups visible tiles into buckets keyed by `(baseTexture, overlayTexture, waterFlags)` and issues one `glDrawElements` per bucket instead of per tile — roughly eight GL calls per two-triangle tile down to one draw per distinct texture pair. No asset changes.
- **Status**: Implemented and confirmed. A dev-machine capture showed terrain draw calls dropping 1,226 → 48 (25.5×) at the same test spot with no reported corruption in a general pass. An independent re-verification then found `$glstats` reporting terrain GPU ms *elevated* (3–5ms) despite the collapsed draw count — a real "regression" by the numbers, contradicting the draw-call win. Three code-change attempts targeting plausible GPU-side causes (index-order locality, single-upload batching, texture-bind-order sort) produced only small, inconclusive improvements. An Nsight Graphics GPU Trace capture then isolated terrain's real draw cost at **<0.01ms**, meaning the elevated `$glstats` reading was not describing terrain's actual GPU work at all. Root cause: `FrameProfiler::Scope`'s `GL_TIMESTAMP` pair was bracketing the *entire* CPU-side gather/sort scope, not just the real draw submission — any CPU-bound gap between the GPU finishing its begin-marker and the CPU submitting the next draw reads as "GPU time." Fixed by decoupling the GPU timer from the CPU-scope RAII lifetime (`autoGpuTimer` flag + `FRAME_PROFILE_CPU_ONLY` macro on `FrameProfiler::Scope`), with Terrain's three call sites switched to place `GpuTimerBegin`/`GpuTimerEnd` manually around just the draw loop. Re-measured at 0.01ms, matching the Nsight finding almost exactly — confirmed in **both Debug and Release** builds. The two now-unnecessary fix attempts (index-order sort, texture-bind-order sort) were reverted, keeping only the single-upload batching change (independently justified) and the real instrumentation fix. Net result across the whole branch (GLP-01 through GLP-16), clean re-capture: **avg FPS +4.4%, 1% Low +28.0%, frame time −4.1%**, plus the 25.5× terrain draw-call reduction itself. Still outstanding: the task's own before/after visual checklist (multi-layer blending at bucket-crossing tile boundaries, water scroll/shoreline behavior, the Atlans/Doppelganger3 overlay substitution, Crywolf splash-particle rate, grass maps, dev editor mode) and Intel HD 530 hardware numbers — all measurement in this task was done on dev hardware (NVIDIA RTX 5070 Ti).

#### GLP-19 — Batch IR Draws Across Begin/End Pairs
- **Scope**: `Render/Core/ImmediateRenderer.cpp`, plus `IR::Flush()` hooks at every state-mutation site that can invalidate an accumulated batch (`Render/Textures/ZzzOpenglUtil.cpp`, `Render/Core/BindState.cpp`, `Render/Core/GlobalUBO.cpp`, `Render/Shaders/PassthroughShader.cpp`, `Render/RHI/RHI_GL.cpp`, `App/Platform/Windows/Winmain.cpp`).
- **Purpose**: `IR::End()` used to issue one buffer append and one draw per `Begin`/`End` pair — one draw per quad. `End()` now only marks a batch pending; the next `Begin()` compares live render state against a field-wise `IRBatchKey` snapshot and appends to the existing vertex run when nothing that affects the pixels has changed. Any state writer that *would* change the result flushes first, so a batch is never submitted under state it was not built with.
- **Status**: Implemented and correct, and it works — for UI, text and joints. **It does not help particles at all, and that is measured, not assumed.** A Release capture during a Chaotic Diseier cast shows the `Particles` pass at **2,918 quads → 2,918 draws (quads-per-draw 1.0)** while `Joints` in the same frame reaches **1904.9** (≈26,700 quads in 14 draws). The cause is that `RenderParticles()` changes blend mode roughly once per particle, and the `AlphaBlendType` guards in `ZzzOpenglUtil.cpp` correctly flush the pending batch *before* mutating state. Two visual regressions found and fixed during implementation (a stray black polygon from unhooked `Enable*`/`Disable*` state, and UI text corruption because `RHI::UpdateTexture` re-uploads into an existing texture id and so never tripped the bind hook) — both were cases of batch state changing through a path with no flush hook. See [GLP-20](#see-also) for the unfinished half.

---

### Phase 5 — Skill-Cast Cost

#### GLP-24 — `$glstats` Pass Attribution
- **Scope**: `Core/Utilities/FrameProfiler.h`, `Scenes/SceneManager.cpp`, `Scenes/MainScene.cpp`.
- **Purpose**: The overlay's per-pass table omitted `Pass::Other`, and `RenderSprites()`/`RenderParticles()`/`RenderJoints()` sat outside every `FRAME_PROFILE` scope — so all three landed in a bucket the overlay never printed. On the capture that motivated this series they accounted for ~94% of the frame's draw-call increase while being completely invisible. Adds `Sprites`/`Particles`/`Joints`/`Overlay` passes, prints `Other`, tags the overlay's own cost so it stops contaminating the bucket under investigation, and allows multiple GPU query entries per pass (a pass entered twice per frame previously reported only its last entry against a CPU figure that summed both).
- **Status**: Completed, and it was blocking — **every measurement taken earlier in this series carries the blind spot it fixes.** A second defect was found and fixed separately (`e15814af`): `AdvanceGpuTimers()` read the ring slot the current frame had just written, giving the GPU zero frames to retire the queries, so late-submitted passes were discarded rather than retried and their GPU ms stayed at 0.00 permanently. That is why `Particles` reported 0.00 ms against ~2,970 draws. Now reads the oldest slot.

#### GLP-25 — IR Streaming Ring Growth Policy
- **Scope**: `Render/RHI/RHI_GL.cpp`, `Render/Core/ImmediateRenderer.cpp`.
- **Purpose**: `RHI::AppendBuffer` only grew the streaming buffer when a *single* append exceeded the whole buffer, so a ring that was merely too small for a frame's total volume wrapped repeatedly forever instead of growing out of it. An effect-dense frame wrapped ~23 times. Adds a wrap counter per buffer that doubles capacity after repeated same-frame wraps, up to a cap, and raises IR's initial reservation to 65,536 vertices (~2.25 MB, ~10,900 quads per wrap).
- **Status**: Completed and verified by counter: `BufferOrphans` dropped from 21 to 1 in a like-for-like capture at the same spot.

#### GLP-29 — Particle + Joint Vertex-Generation CPU
- **Scope**: `Render/Textures/ZzzOpenglUtil.cpp`, `Render/Core/ImmediateRenderer.cpp`.
- **Purpose**: Once GLP-19 showed submission was not the particle bottleneck, this targeted the CPU cost of *generating* the vertices. Two changes landed: `RenderSprite`'s rotated path built a full 3×4 `AngleMatrix` and ran four `VectorRotate` calls per sprite, which for a Z-only angle reduces exactly to a 2D rotation (an algebraic identity, not an approximation); and IR's 4-element quad scratch was a `std::vector` doing a `push_back` per vertex and a `clear` per quad — ~11,900 vector operations per frame at 2,975 particles. Also fixed a GLP-19 regression found in passing: `Begin()` built a state snapshot unconditionally, charging two `CaptureKey()` calls per quad in a pass that never batches.
- **Status**: Implemented, then **paused rather than continued down its candidate list**. Debug frame CPU 21.47 → 13.91 ms (−35%); Release frame 5.99 → 5.92 ms, which at the frame level is nothing — `Joints` moved +0.33 ms in the same pair despite being untouched, so run-to-run variance on the test machine exceeds the effect. The removed overhead is overwhelmingly MSVC Debug instrumentation, not real work. Kept because the code is strictly better and a 35% Debug win buys real iteration speed, **not because it improves the shipped frame.** Rotating smoke, clouds and explosions visually confirmed correct.

---

## Where the skill-cast cost actually is (measured 2026-08-12, Release, RTX 5070 Ti)

A `$glstats` capture during a Chaotic Diseier cast in Noria, frame 6.03 ms / 161 FPS:

| Pass | CPU ms | GPU ms | Draws | Quads/draw |
|---|---|---|---|---|
| Particles | 1.59 | 1.88 | 2,918 | **1.0** |
| Joints | 1.10 | 0.01 | 14 | 1904.9 |
| Chars | 0.24 | **1.51** | 69 | 3.6 |
| Overlay | 1.03 | 0.00 | 62 | 4.9 |

Three conclusions, each of which contradicts something previously assumed:

1. **Particles is submission-bound and GLP-19 does not fix it.** Blend mode changes ~once per
   particle; each change correctly flushes the pending batch. Fixing this requires grouping
   particles by blend state ([GLP-20](#see-also)), not more batching machinery. An attempt to do
   this by reordering the draw loop was built, measured to change nothing, and reverted.
2. **Joints is *not* submission cost.** 14 draws cannot cost 1.10 ms. It is CPU geometry
   generation, and it is untouched and unexplained — the largest single unclaimed item.
3. **Chars is genuinely GPU-bound** (GPU 6× CPU). This is the one shape the GPU-timer
   scope-bracketing artifact cannot manufacture — the artifact inflates GPU toward CPU, it cannot
   invent 6× more. The standing lead is the unconditional `discard` in `BMDMeshShader` defeating
   early-Z, which is Phase 3's subject and has no landed work.

**Reading the overlay honestly**: `Overlay` costs 1.03 ms of that 6.03 ms frame. That is the
instrument, not the game. Judge fixes on the per-pass figures, not on the FPS number shown while
the overlay is up.

---

## Corrections to the Source Analysis

The original analysis this series was planned from made three claims that did not survive
verification against the actual source tree. They are recorded here so a future reader does not
re-derive or re-assert them:

1. **The per-frame depth-buffer readback does not execute.** The suspected stall path
   (`RHI::ReadDepthPixel` → `CameraProjection::TestDepthBuffer` → `ZzzLodTerrain.cpp`'s
   `RenderSun()`) was ranked as a major driver stall, but `RenderSun()` has zero callers anywhere
   in the tree and its own sprite draw is commented out. It is dead code, not a live performance
   issue — a cleanup, not a fix, and not yet scheduled.
2. **`FrameProfiler` already existed before this series.** `Core/Utilities/FrameProfiler.h` already
   had per-pass CPU timing wired to the `$details` overlay. [GLP-01](#glp-01--gl-call-counters--gpu-pass-timers)
   extends it with GL-call counters and GPU-side timers; it does not build a profiler from scratch.
3. **The prior `GLS-08` buffer-orphaning fix was not wrong — it was superseded.** `GLS-08` landed
   `glBufferData(nullptr)` orphaning in `UpdateUniformBlock` and measured a real, verified win
   (~11% average FPS in a busy scene). [GLP-09](#glp-09--ring-buffer-ubo-allocator) replaces the
   mechanism with a ring allocator that is strictly better on the same hardware, but `GLS-08` was
   the correct fix for a single fixed-size buffer at the time, and its measurements remain the
   baseline GLP-09's own result must be compared against, not discarded.
