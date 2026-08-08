# DXP Milestone Implementation Chronology

This document serves as the authoritative technical reference catalog for all **`DXP-xx`** milestone tags used in source code comments, commit logs, and architectural document cross-references across the client codebase.

The milestones below are organized **chronologically in the exact order of implementation**, grouped by development phase.

---

## Milestone Execution Chronology Table

### Phase A: FFP Retirement & Core Profile Baseline (August 1–2, 2026)

> [!NOTE]
> Phase A milestones (DXP-01 through DXP-11) were implemented on the `feature-ffp-shader-port` branch and squash-merged into the `gl-only-port` baseline commit (`0702144`). Their `DXP-xx` tag strings are preserved in the archived DXP task memory files and commit history but were **not carried forward as inline code comments** into the squashed source tree.

| Milestone | Subsystem | Focus / Objective | Status |
|---|---|---|---|
| **[DXP-01](#dxp-01--alpha-test-shader-port)** | Shaders | Shader-level alpha testing (`u_AlphaRef` / `discard`). | Completed |
| **[DXP-02](#dxp-02--gpu-acceleration-for-item-specular-tiers)** | Item Specular | GPU acceleration for all 4 item specular tiers (+7..+15 gear). | Completed |
| **[DXP-03](#dxp-03--cloth--cape-immediaterenderer-port)** | Physics & Cloth | `CPhysicsCloth::RenderFace` port to `ImmediateRenderer`. | Completed |
| **[DXP-04](#dxp-04--event-fog-sceneubo-integration)** | Map Environment | `GMBattleCastle` event fog integration into `SceneUBO`. | Completed |
| **[DXP-05](#dxp-05--kanturu-terrain-pass-vbo-migration)** | Terrain | Kanturu-3rd terrain "after" pass ported off FFP `glBegin`. | Completed |
| **[DXP-06](#dxp-06--planar-shadow-shader-core-profile-rewrite)** | Shadows | `CPlanarShadowShader` GLSL 330 core single-pass rewrite. | Completed |
| **[DXP-07a](#dxp-07a--cpu-ortho-matrix-calculation)** | UI / 2D | `BeginBitmap()` ortho matrix calculation moved to CPU. | Completed |
| **[DXP-07b](#dxp-07b--cpu-camera-matrix-calculation)** | Render Core | `BeginOpengl()` camera view/proj matrices moved to CPU. | Completed |
| **[DXP-07c](#dxp-07c--decoupled-picking--mouse-matrix-chain)** | UI / Mouse | Mouse picking & camera projection decoupled from GL matrix stack. | Completed |
| **[DXP-07d](#dxp-07d--item-preview-panel-cpu-camera-validation)** | UI / 3D Panels | Shadow-compare validation series for all 6 item-preview panel cameras. | Completed |
| **[DXP-09](#dxp-09--pipeline-hygiene--ir-topology-decomposition)** | Renderer Core | Code hygiene, dead code removal, IR quad/fan decomposition. | Completed |
| **[DXP-08a](#dxp-08a--core-profile-soak--illegal-ffp-sweep)** | Core Engine | Core Profile soak testing & illegal FFP state call sweep. | Completed |
| **[DXP-08](#dxp-08--opengl-33-core-profile-migration)** | Core Engine | Core Profile flip & legacy fixed-function branch retirement. | Completed |
| **[DXP-10](#dxp-10--state-wrapper-monopoly--guard-script)** | Build / Guard | Graphics wrapper monopoly enforcement script (`check_gl_wrapper_monopoly.py`). | Completed |

### Phase B: GPU Skeletal Skinning & Performance Engine (August 2, 2026)

| Milestone | Subsystem | Focus / Objective | Status |
|---|---|---|---|
| **[DXP-11](#dxp-11--rhi-interface-extraction)** | RHI Architecture | Design and extraction of Render Hardware Interface (`RHI.h`). | Completed |
| **[DXP-20](#dxp-20--zero-cpu-gpu-skeletal-skinning)** | GPU Skinning | Zero-CPU GPU Skeletal Skinning (`BoneUBO`, `MAX_BONES=200`). | Completed |
| **[DXP-20-inc4](#dxp-20-inc4--lazy-materialization-pattern)** | Optimization | `TransformCheap()` lazy materialization for CPU render caches. | Completed |
| **[DXP-20-inc5](#dxp-20-inc5--chrome-render-mode-aliasing-fix)** | Chrome Shaders | Fix for collapsed/aliased chrome render mode GPU skinning checks. | Completed |
| **[DXP-22](#dxp-22--bind-state-monopoly--cache-invalidation)** | State Cache | VAO/VBO/Program/Texture bind cache invalidation rules. | Completed |
| **[DXP-23](#dxp-23--uniform-upload-profiling--gpu-diagnostics)** | Profiling | FrameProfiler GPU-bound vs CPU-bound bottleneck diagnostics. | Completed |
| **[DXP-24](#dxp-24--shadow-bone-matrix-corruption--dummy-bones)** | Shadow / Bones | Fix for Planar Shadow division explosion via Dummy bone clearing. | Completed |
| **[DXP-25](#dxp-25--logic-render-decoupling-for-ui-flags)** | UI State | Decoupling one-shot UI render flags from logic tick rates. | Completed |
| **[DXP-26](#dxp-26--immediaterenderer-ring-buffer--bind-cache-fix)** | ImmediateRenderer | Streaming VBO ring-buffer & removal of per-draw unbind. | Completed |
| **[DXP-27](#dxp-27--animation-rate-decoupling-from-render-fps)** | Animation Sync | Real-time throttling of render-frequency animation calls. | Completed |

### Phase C: Full Hardware Abstraction Layer & Subsystem Port (August 3–4, 2026)

| Milestone | Subsystem | Focus / Objective | Status |
|---|---|---|---|
| **[DXP-12](#dxp-12--rhi-texture-management--readback)** | RHI Textures | RHI texture creation, streaming upload, and pixel readback. | Completed |
| **[DXP-13](#dxp-13--rhi-device--windowing-handoff)** | RHI Pipeline | Modern rendering engine init & RHI device creation. | Completed |
| **[DXP-14](#dxp-14--rhi-shader-pipeline--ubo-layout-parity)** | RHI Shaders | Core shader pipeline port and UBO/constant buffer matching. | Completed |
| **[DXP-15](#dxp-15--rhi-immediate-renderer--2d-pipeline)** | RHI 2D Pipeline | `ImmediateRenderer` integration into RHI abstraction. | Completed |
| **[DXP-16](#dxp-16--rhi-world-pipelines--blend-encapsulation)** | RHI 3D World | Terrain, BMD mesh, and Planar Shadow RHI pipeline port. | Completed |
| **[DXP-17](#dxp-17--depth--projection-matrix-clip-space-alignment)** | Projection | Clip-space perspective projection helper unification. | Partially Completed |
| **[DXP-21](#dxp-21--cloth-mesh-compute-decoupling)** | Cloth Physics | Seam for dynamic cloth mesh compute optimization. | Deferred |

---

## Detailed Milestone Chronology

### Phase A: FFP Retirement & Core Profile Baseline

#### DXP-01 — Alpha Test Shader Port
- **Scope**: `Render/Shaders/` & `ZzzOpenglUtil.cpp`.
- **Purpose**: Eliminated fixed-function `glAlphaFunc` calls across all 2D and 3D pipelines by passing `u_AlphaRef` uniform to shaders and executing explicit `if (texColor.a < u_AlphaRef) discard;` in fragment shaders.

#### DXP-02 — GPU Acceleration for Item Specular Tiers
- **Scope**: `Render/Shaders/ItemSpecularShader.cpp` & `ZzzBMD.cpp`.
- **Purpose**: Accelerated all 4 item-specular shine tiers (+7 through +15 equipment) directly on the GPU via `BMDMeshShader` render modes 4–7 (`itemSpecularGpuEligible`). Retired legacy fixed-function shader shims.

#### DXP-03 — Cloth & Cape ImmediateRenderer Port
- **Scope**: `Engine/Physics/PhysicsManager.cpp`.
- **Purpose**: Converted `CPhysicsCloth::RenderFace` from legacy `glBegin(GL_QUADS)` to `ImmediateRenderer` (`IR::`), enabling cape and cloth rendering under modern shader pipelines.

#### DXP-04 — Event Fog SceneUBO Integration
- **Scope**: `World/GameMaps/GMBattleCastle.cpp`.
- **Purpose**: Integrated map event fog parameters (Battle Castle 2000–2700 range) into `SceneUBO`, restoring event fog visibility for modern shader-rendered geometry.

#### DXP-05 — Kanturu Terrain Pass VBO Migration
- **Scope**: `World/GameMaps/GM_Kanturu_3rd.cpp` & `ZzzLodTerrain.cpp`.
- **Purpose**: Ported Kanturu-3rd map multi-pass terrain overlay rendering off legacy immediate mode onto modern terrain VBO shader paths.

#### DXP-06 — Planar Shadow Shader Core Profile Rewrite
- **Scope**: `Render/Shaders/PlanarShadowShader.cpp`.
- **Purpose**: Rewrote `CPlanarShadowShader` to GLSL 330 core. Replaced fixed-function vertex attributes (`gl_Vertex`, `gl_ModelViewProjectionMatrix`) with modern vertex layout attributes and UBO matrices.

#### DXP-07a — CPU Ortho Matrix Calculation
- **Scope**: `Render/Textures/ZzzOpenglUtil.cpp`.
- **Purpose**: Replaced `glMatrixMode(GL_PROJECTION)` / `glLoadIdentity()` / `glOrtho()` in `BeginBitmap()` with CPU-side orthographic matrix construction uploaded directly to `GlobalUBO`.

#### DXP-07b — CPU Camera Matrix Calculation
- **Scope**: `Render/Textures/ZzzOpenglUtil.cpp`.
- **Purpose**: Replaced `glMatrixMode(GL_MODELVIEW)` / `gluLookAt()` in `BeginOpengl()` with CPU-side view and projection matrix math uploaded directly to `GlobalUBO`.

#### DXP-07c — Decoupled Picking & Mouse Matrix Chain
- **Scope**: `CameraProjection.cpp` & `ZzzOpenglUtil.cpp`.
- **Purpose**: Removed `glGetFloatv(GL_MODELVIEW_MATRIX)` and `glGetFloatv(GL_PROJECTION_MATRIX)` from 3D ray picking (`UpdateMousePosition`) and replaced them with CPU matrix cache reads.

#### DXP-07d — Item-Preview Panel CPU Camera Validation
- **Scope**: `UI/NewUI/NewUI3DRenderMng.cpp`, `UI/NewUI/Events/NewUIRegistrationLuckyCoin.cpp`, `UI/NewUI/Events/NewUIGoldBowmanLena.cpp`, `UI/Legacy/UIWindows.cpp`, `Scenes/SceneCommon.cpp`, `GameShop/NewUIInGameShop.cpp`, `Render/Shaders/BMDMeshShader.cpp`.
- **Purpose**: Six-increment shadow-compare diagnostic series that validated each 3D item-preview panel's CPU-computed projection and view matrices against the legacy GL matrix stack, confirming bit-for-bit accuracy before the FFP matrix-stack calls were deleted in DXP-08a. Tag string present in source code comments (17 occurrences across 9 files); Phase A squash did not strip these.

#### DXP-09 — Pipeline Hygiene & IR Topology Decomposition
- **Scope**: `Render/Core/ImmediateRenderer.cpp` & `ZzzOpenglUtil.cpp`.
- **Purpose**: Decomposed immediate-mode `GL_QUADS` and `GL_TRIANGLE_FAN` primitives into indexing-compatible triangle lists (`GL_TRIANGLES`) for hardware abstraction compatibility.

#### DXP-08a — Core Profile Soak & Illegal FFP Sweep
- **Scope**: Codebase-wide sweep in `Render/`, `Scenes/`, `World/`.
- **Purpose**: Swept and removed legacy `glEnable(GL_ALPHA_TEST)`, `glMatrixMode`, `glPushMatrix`, and `glColor3f` calls that caused implicit Core Profile state corruption.

#### DXP-08 — OpenGL 3.3 Core Profile Migration
- **Scope**: `Render/Core/`, `Winmain.cpp`, CMake build manifests.
- **Purpose**: Flipped default context creation to OpenGL 3.3 Core Profile (`g_CoreProfile = true`). Retired legacy fixed-function fallback branches across core render loops.

#### DXP-10 — State Wrapper Monopoly & Guard Script
- **Scope**: [`Tools/check_gl_wrapper_monopoly.py`](../../Tools/check_gl_wrapper_monopoly.py).
- **Purpose**: Established the state-wrapper monopoly invariant: no raw graphics API calls permitted outside `Render/`. Added an automated build-time Python verification guard.

---

### Phase B: GPU Skeletal Skinning & Performance Engine

#### DXP-11 — RHI Interface Extraction
- **Scope**: [`Render/RHI/RHI.h`](../../src/source/Render/RHI/RHI.h) & [`Render/Core/ImmediateRenderer.cpp`](../../src/source/Render/Core/ImmediateRenderer.cpp).
- **Purpose**: Designed and extracted the `RHI` (Render Hardware Interface) abstract base, converting `ImmediateRenderer` to use abstract buffer uploads and pipeline state binds.

#### DXP-20 — Zero-CPU GPU Skeletal Skinning
- **Scope**: [`Render/Models/ZzzBMD.cpp`](../../src/source/Render/Models/ZzzBMD.cpp), [`Render/Core/BoneUBO.h`](../../src/source/Render/Core/BoneUBO.h), `bmd_mesh.vert`.
- **Purpose**: Offloaded character, armor, weapon, wing, and mount skeletal transformations to GPU vertex shaders. Introduced `BoneUBO` (Slot 1) uploading up to `MAX_BONES=200` matrices per batch.

#### DXP-20-inc4 — Lazy Materialization Pattern
- **Scope**: [`Render/Models/ZzzBMD.cpp`](../../src/source/Render/Models/ZzzBMD.cpp).
- **Purpose**: Replaced eager per-frame CPU vertex matrix transformations with `BMD::TransformCheap()` lazy materialization, eliminating unnecessary CPU calculations when only GPU skinning is required.

#### DXP-20-inc5 — Chrome Render Mode Aliasing Fix
- **Scope**: [`Render/Models/ZzzBMD.cpp`](../../src/source/Render/Models/ZzzBMD.cpp).
- **Purpose**: Resolved enum aliasing where CHROME2/3/5/6/7 and METAL shared `RENDER_CHROME` flag checks, ensuring accurate GPU shader variant dispatch for specialized reflection effects.

#### DXP-22 — Bind State Monopoly & Cache Invalidation
- **Scope**: [`Render/Core/BindState.h`](../../src/source/Render/Core/BindState.h), [`Render/Core/BindState.cpp`](../../src/source/Render/Core/BindState.cpp), [`Render/Core/ImmediateRenderer.cpp`](../../src/source/Render/Core/ImmediateRenderer.cpp), [`Render/Models/ZzzBMD.cpp`](../../src/source/Render/Models/ZzzBMD.cpp), `Render/Terrain/ZzzLodTerrain.cpp`.
- **Purpose**: Implemented robust bind cache invalidation upon resource deletion (`glDeleteVertexArrays`, `glDeleteTextures`) to prevent false hit state corruption.

#### DXP-23 — Uniform Upload Profiling & GPU Diagnostics
- **Scope**: `Core/Utilities/FrameProfiler.h`, `Scenes/MainScene.cpp`, `Scenes/MainScene.h`, `Scenes/SceneManager.cpp`, `Core/Utilities/Log/muConsoleDebug.cpp`, `Render/Effects/ZzzEffectParticle.cpp`, `Render/Effects/ZzzEffectJoint.cpp`.
- **Purpose**: Profiled per-draw uniform upload costs and established diagnostic criteria to distinguish CPU-bound driver stalls from GPU queue-wait latency in the `FrameProfiler` HUD. Added console debug toggles to isolate effect rendering subsystems (`$effects off`, wing/joint/boid diagnostics).

#### DXP-24 — Shadow Bone Matrix Corruption & Dummy Bones
- **Scope**: [`Render/Models/ZzzBMD.cpp`](../../src/source/Render/Models/ZzzBMD.cpp) & `Render/Shaders/PlanarShadowShader.cpp`.
- **Purpose**: Fixed shadow division explosions caused by uninitialized Dummy bone matrices by ensuring complete palette zeroing and bounds validation before shadow projection.

#### DXP-25 — Logic-Render Decoupling for UI Flags
- **Scope**: `Scenes/MainScene.cpp` & UI Window controls.
- **Purpose**: Decoupled one-shot UI interaction flags (`m_bRenderSkillInfo`) from rendering execution to prevent visual flickering when rendering uncapped above logic tick rates.

#### DXP-26 — ImmediateRenderer Ring-Buffer & Bind Cache Fix
- **Scope**: [`Render/Core/ImmediateRenderer.cpp`](../../src/source/Render/Core/ImmediateRenderer.cpp).
- **Purpose**: Implemented a streaming vertex ring-buffer with orphan-on-wrap semantics and removed redundant per-draw shader program unbinding, boosting immediate rendering performance.

#### DXP-27 — Animation Rate Decoupling from Render FPS
- **Scope**: `ZzzCharacter.cpp`, `PhysicsManager.cpp`, `Scenes/MainScene.cpp`.
- **Purpose**: Added high-resolution timer throttling (`std::chrono::steady_clock`) to render-frequency animation calls to prevent cloth oscillation, wing-flaps, and camera tours from speeding up at high FPS.

---

### Phase C: Full Hardware Abstraction Layer & Subsystem Port

#### DXP-12 — RHI Texture Management & Readback
- **Scope**: [`GlobalBitmap.cpp`](../../src/source/Render/Sprites/GlobalBitmap.cpp), `ZzzInventory.cpp`, `UIControls.cpp`.
- **Purpose**: Moved texture creation, dynamic sub-image streaming, and screen readbacks (screenshots, disconnect blur) behind the abstract `RHI` interface.

#### DXP-13 — RHI Device & Windowing Handoff
- **Scope**: `Render/RHI/`, `Winmain.cpp`.
- **Purpose**: Encapsulated device creation, swapchain management, context lifecycle, and window resize events into the RHI subsystem layer.

#### DXP-14 — RHI Shader Pipeline & Constant Buffer Layout Matching
- **Scope**: `Render/Core/GlobalUBO.h`, `Render/Core/SceneUBO.h`, `Render/Core/BoneUBO.h`.
- **Purpose**: Enforced byte-exact layout alignment between shader uniform blocks (`GlobalUBO`, `BoneUBO`, `SceneUBO`) and hardware constant buffer slots across graphic backends.

#### DXP-15 — RHI Immediate Renderer & 2D Pipeline
- **Scope**: [`Render/Core/ImmediateRenderer.cpp`](../../src/source/Render/Core/ImmediateRenderer.cpp).
- **Purpose**: Integrated 2D sprite, UI box, and font rendering through the abstract RHI pipeline state objects and dynamic vertex stream buffers.

#### DXP-16 — RHI World Pipelines & Blend Encapsulation
- **Scope**: `Scenes/CharacterScene.cpp`, `Scenes/SceneManager.cpp`, `UI/Legacy/UIControls.cpp`.
- **Purpose**: Fully converted terrain rendering, BMD mesh skinning, planar shadow projection, and RHI blend mode state objects to unified backend execution.

#### DXP-17 — Depth & Projection Matrix Clip Space Alignment
- **Scope**: `Render/Core/RenderConfig.h/.cpp` & `UI/NewUI/NewUI3DRenderMng.cpp`.
- **Purpose**: Unified 3D perspective projection construction (`BuildPerspectiveProjection`) across world and 3D UI item preview cameras to handle backend clip-space depth conventions cleanly. 
- **Status Note (Partially Completed)**: Item 1 (projection builders) is complete and fixed the underwater geometry bleed bug. Items 2/3/5/6 (fog formulas, depth readback, gamma, viewport depth range) were explicitly closed by owner decision as accepted low-priority risk. These remain unaudited because they are harmless under OpenGL's `[-1, 1]` matrix. They will be reopened as micro-tasks during Phase C only if a concrete visual symptom (like fog-distance mismatch) appears when the D3D11 `[0, 1]` matrix is active.

#### DXP-21 — Cloth Mesh Compute Decoupling
- **Scope**: `Engine/Physics/PhysicsManager.cpp`.
- **Purpose**: Planned decoupling of dynamic CPU cloth simulation from static GPU mesh skinning pathways. **Status: Deferred by owner decision (2026-08-02)** — waiting for D3D11 compute backend before implementing. No code change landed.
