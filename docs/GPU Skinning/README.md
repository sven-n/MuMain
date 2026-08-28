# GPU Skinning & Modern Graphics Subsystem Documentation

Welcome to the documentation for the MU Online Client **GPU Skinning & Modern Graphics Subsystem**.

This documentation details the architecture, design decisions, milestone catalog, and technical invariants behind the migration of the client rendering engine from the legacy fixed-function pipeline (FFP) to modern GPU skeletal skinning, Uniform Buffer Objects (UBOs), Core Profile shaders, and the Render Hardware Interface (RHI) abstraction layer.

---

## Document Index

1. **[DXP Milestone Catalog](dxp-milestones.md)**  
   *The complete reference catalog for all `DXP-xx` milestone tags referenced in source code inline comments.* Contains background, scope, and resolution details for DXP-01 through DXP-27.

2. **[GPU Skinning Architecture](gpu-skinning-architecture.md)**  
   *Deep technical specification of GPU-based skeletal animation.* Details matrix palette management, UBO layouts (`GlobalUBO`, `BoneUBO`), vertex shader transformation equations, item specular / chrome GPU shaders, and skeleton invariants (`MAX_BONES=200`).

3. **[Core Profile & Pipeline Migration](core-profile-migration.md)**  
   *Guide to modern shader pipeline migration and legacy FFP retirement.* Covers the removal of the fixed-function matrix stack, `ImmediateRenderer` (`IR::`) topology decomposition, RHI state encapsulation, and automated build-time wrapper enforcement.

4. **[ImmediateRenderer (IR::) Architecture](immediate-renderer-architecture.md)**  
   *Detailed technical specification of the ImmediateRenderer subsystem.* Covers Quad and Triangle Fan primitive decomposition math, interleaved dynamic vertex stream layouts, streaming ring-buffer orphan-on-wrap mechanics, RHI integration, and complete IR milestone task history.

5. **[Gotchas & Invariants Catalog](gotchas-and-patterns.md)**  
   *Comprehensive catalog of known gotchas, threading synchronization rules, cache invalidation pitfalls, and performance invariants.*

6. **[GLP Milestone Catalog](glperf/README.md)**  
   *Reference catalog for the `GLP-xx` Core Profile FPS regression fix series.* Covers milestones with a landed implementation (build-verified at minimum, some still pending target-hardware confirmation) — not-yet-started and paused-investigation tickets are tracked separately, not here.

---

## Architectural Highlights

- **Zero-CPU Skeletal Skinning**: Character, monster, equipment, weapon, wing, and mount meshes perform skeletal bone transformation directly in the vertex shader, freeing significant CPU resources. The authoritative shader source is the embedded `g_szBMDMeshVert` string in [`Render/Shaders/BMDMeshShader.cpp`](../../src/source/Render/Shaders/BMDMeshShader.cpp) — **not** `Render/Shaders/glsl/bmd_mesh.vert`, which is a stale, non-runtime-loaded snapshot (see the caution in [GPU Skinning Architecture](gpu-skinning-architecture.md)).
- **Render Hardware Interface (RHI)**: Modernized graphics abstraction layer (`RHI.h`) decoupling low-level driver state, shaders, textures, and vertex buffers from client application code.
- **Core Profile Conformance**: Retirement of legacy immediate mode (`glBegin`/`glEnd`), `glPushMatrix`/`glPopMatrix`, and FFP matrix stack operations in favor of UBO-driven GLSL 3.3 Core Profile pipelines.
- **Interleaved Streaming VBOs**: High-efficiency dynamic vertex buffer streaming (`[Pos | UV | Color]` interleaving) eliminating runtime heap allocations during draw passes.
- **Decoupled Animation & Render Cadence**: Thread-safe parallel character animation calculation (`AnimationTaskPool`) decoupled from rendering FPS.

---

## Primary Subsystems & Source Map

| Subsystem | Key Files | Description |
|---|---|---|
| **RHI Core** | [`Render/RHI/RHI.h`](../../src/source/Render/RHI/RHI.h) | Abstract rendering hardware interface definitions. |
| **Model & Mesh Shader** | [`Render/Shaders/BMDMeshShader.cpp`](../../src/source/Render/Shaders/BMDMeshShader.cpp)<br>[`Render/Models/ZzzBMD.cpp`](../../src/source/Render/Models/ZzzBMD.cpp) | BMD mesh skinning shader, static/dynamic VBO management, matrix caching. |
| **Uniform Buffers** | [`Render/Core/GlobalUBO.h`](../../src/source/Render/Core/GlobalUBO.h)<br>[`Render/Core/BoneUBO.h`](../../src/source/Render/Core/BoneUBO.h) | Uniform Buffer Objects for Camera/Model matrices (`Slot 0`), SceneData (`Slot 1`), Skeletal Bones (`Slot 2`, `vec4[600]` — 3 affine rows per bone since `GLP-11`) and per-draw `BMDFlags` (`Slot 6`). |
| **Immediate Renderer** | [`Render/Core/ImmediateRenderer.cpp`](../../src/source/Render/Core/ImmediateRenderer.cpp) | Dynamic quad/fan topology decomposition and 2D/3D immediate-mode submission. |
| **Item Specular Shader** | [`Render/Shaders/ItemSpecularShader.cpp`](../../src/source/Render/Shaders/ItemSpecularShader.cpp) | Modern single-pass GPU shader for +7 through +15 equipment shine & chrome effects. |

---

## Graphics & Performance Configuration Switches

The graphics and performance engine exposes runtime switches via `config.ini` and command-line flags:

### 1. `config.ini` Switches
- **`[Render] CoreProfile`** (Default: `1`):
  - `1`: Enforces **OpenGL 3.3 Core Profile** (`SDL_GL_CONTEXT_PROFILE_CORE`). All rendering runs via UBOs, GLSL 3.3 shaders, GPU skeletal skinning, and `ImmediateRenderer` (`IR::`).
  - `0`: Requests **OpenGL Compatibility Profile** (`SDL_GL_CONTEXT_PROFILE_COMPATIBILITY`). Re-enables legacy FFP driver state toggles (e.g. `glAlphaTest`, `glEnable(GL_TEXTURE_2D)`) for legacy driver hook compatibility; all primary rendering remains shader and UBO driven.
- **`[Render] MaxGLVersion`** (`GLP-08`): caps the GL context version the client will request. Without it the client walks a descending `{4,5} → {4,3} → {3,3}` chain and takes the highest that succeeds; set this to force a lower ceiling as a rollback path if a driver misbehaves on the newer context.
- **`[Render] SortParticleDraws`** (Default: `0`):
  - `1`: Groups particle draws by texture before submitting them, so consecutive sprites sharing a texture merge into one `IR::` batch instead of costing a draw call each. Particles are otherwise drawn in slot (allocation) order, which interleaves textures and breaks the batch on nearly every sprite.
  - `0`: Draws particles in slot order, exactly as before.
  - Only particles whose result cannot depend on draw order are moved: additive sprites whose effect type does not switch blend mode or toggle the depth test. Everything else keeps its position and acts as a barrier no draw is moved across. Opt-in until the reordering has been confirmed by eye against real effects on target hardware.
- **`[UI] EnableAnimationTaskPool`** (Default: `0`):
  - `1`: Enables the multi-threaded character animation task pool (`AnimationTaskPool`) when $\ge 20$ active characters are present.
  - `0`: Runs character skeletal animation updates sequentially on the main thread.

### 2. Command Line Flags
- **`--enable-taskpool`**: Forces `AnimationTaskPool` on at launch regardless of `config.ini`.
- **`--editor`**: Enables the in-game ImGui editor overlay (**F12**) on `*_mueditor` builds.
