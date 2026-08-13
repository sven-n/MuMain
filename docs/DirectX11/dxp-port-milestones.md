# DXP Port Milestones Reference

This document is the technical reference catalog for **`DXP-xx`** milestone tags used in source code comments, commit logs, and architectural document cross-references specifically related to the DirectX 11 Port.

The milestones below are organized **chronologically in the exact order of implementation**, grouped by development phase.

> **Coverage caveat (verified 2026-08-14).** This catalog documents the Phase C milestones below, but the source tree currently references **25 distinct `DXP-xx` tags**. Undocumented here: `DXP-01`, `DXP-02`, `DXP-05`, `DXP-06`, `DXP-07a`–`DXP-07d`, `DXP-08`, `DXP-08a`, `DXP-09`, `DXP-10`, `DXP-11`, `DXP-20`, `DXP-22`, `DXP-23`, `DXP-24`, `DXP-26`. These are predominantly the earlier FFP-retirement and Core-Profile groundwork phases that preceded the D3D11 backend proper. Reproduce the current list with:
> ```bash
> grep -rhoE "DXP-[0-9]+[a-z]?" src/source/ | sort -u -V
> ```
> Treat a tag's absence from this file as "not yet written up," **not** as "not a real milestone."

---

## Milestone Execution Chronology Table

### Phase C: Full Hardware Abstraction Layer & Subsystem Port

| Milestone | Subsystem | Focus / Objective | Status |
|---|---|---|---|
| **[DXP-12](#dxp-12--rhi-texture-management--readback)** | RHI Textures | RHI texture creation, streaming upload, and pixel readback. | Completed |
| **[DXP-13](#dxp-13--rhi-device--windowing-handoff)** | RHI Pipeline | Modern rendering engine init & RHI device creation (D3D11). | Completed |
| **[DXP-14](#dxp-14--rhi-shader-pipeline--ubo-layout-parity)** | RHI Shaders | Core shader pipeline port and constant buffer layout matching. | Completed |
| **[DXP-15](#dxp-15--rhi-immediate-renderer--2d-pipeline)** | RHI 2D Pipeline | `ImmediateRenderer` integration into D3D11 RHI abstraction. | Completed |
| **[DXP-16](#dxp-16--rhi-world-pipelines--blend-encapsulation)** | RHI 3D World | Terrain, BMD mesh, and Planar Shadow D3D11 pipeline port. | Completed |
| **[DXP-17](#dxp-17--depth--projection-matrix-clip-space-alignment)** | Projection | Clip-space perspective projection helper unification. | Completed |
| **[DXP-21](#dxp-21--cloth-mesh-compute-shader)** | Cloth Physics | GPU Hardware Compute Cloth Simulation via DirectCompute. | Completed |



---

## Detailed Milestone Chronology

### Phase C: Full Hardware Abstraction Layer & Subsystem Port

#### DXP-12 — RHI Texture Management & Readback
- **Scope**: `GlobalBitmap.cpp`, `ZzzInventory.cpp`, `UIControls.cpp`.
- **Purpose**: Moved texture creation, dynamic sub-image streaming, and screen readbacks (screenshots, disconnect blur) behind the abstract `RHI` interface.

#### DXP-13 — RHI Device & Windowing Handoff
- **Scope**: `Render/RHI/`, `Winmain.cpp`.
- **Purpose**: Encapsulated device creation, swapchain management, context lifecycle, and window resize events into the RHI subsystem layer (D3D11 device).

#### DXP-14 — RHI Shader Pipeline & UBO Layout Parity
- **Scope**: `Render/Core/GlobalUBO.h`, `Render/Core/SceneUBO.h`, `Render/Core/BoneUBO.h`.
- **Purpose**: Hand-ported the 4 core shaders to HLSL and enforced byte-exact layout alignment between shader uniform blocks and hardware constant buffer slots.

#### DXP-15 — RHI Immediate Renderer & 2D Pipeline
- **Scope**: `Render/Core/ImmediateRenderer.cpp`.
- **Purpose**: Integrated 2D sprite, UI box, and font rendering through the abstract RHI pipeline state objects and dynamic vertex stream buffers for D3D11. Coordinated UI Y-flip projection.

#### DXP-16 — RHI World Pipelines & Blend Encapsulation
- **Scope**: `Scenes/CharacterScene.cpp`, `Scenes/SceneManager.cpp`, `UI/Legacy/UIControls.cpp`.
- **Purpose**: Fully converted terrain rendering, BMD mesh skinning, planar shadow projection, and RHI blend mode state objects to unified backend execution for DirectX 11.

#### DXP-17 — Depth & Projection Matrix Clip Space Alignment
- **Scope**: `Render/Core/RenderConfig.h/.cpp` & `UI/NewUI/NewUI3DRenderMng.cpp`.
- **Purpose**: Unified 3D perspective projection construction across world and 3D UI item preview cameras to handle backend clip-space depth conventions cleanly (`[-1, 1]` for GL, `[0, 1]` for D3D11).

#### DXP-21 — Cloth Mesh Compute Shader
- **Scope**: `Engine/Physics/PhysicsManager.cpp`, `ClothComputeShader.cpp`.
- **Purpose**: Replaced legacy CPU spring-mass solver with DirectCompute GPU simulation for wings and capes, eliminating CPU bottlenecks during dense character events.

---

## Phase D: GLP Performance Series Integration (2026-08-14)

The `perf/gl-uniforms-into-ubo` branch (GLP-01 → GLP-29, upstream PR #560) was merged into the D3D11 port branch, which had diverged 38 commits earlier at `39107a81`. This is not a DXP milestone in itself, but it materially changed the D3D11 backend and is the origin of the cross-backend invariants now documented in [Gotchas §4.3 and §5](gotchas-and-patterns.md).

### What D3D11 gained
| GLP task | Effect on D3D11 |
|---|---|
| **GLP-16** — terrain texture-pair bucketing | **Direct win.** `FlushTerrainBuckets()` draws via the backend-agnostic `RHI::DrawIndexed`, so D3D11 inherits the same ~25× terrain draw-call reduction (1,226 → 48). The port branch had still been issuing an immediate per-tile draw; keeping both would have double-rendered every tile. |
| **GLP-19** — IR cross-`Begin`/`End` batching | Applies (`ImmediateRenderer` is backend-agnostic per DXP-05/DXP-15), **but** introduced the deferred-batch invariant that D3D11 initially violated in eight places. |
| **GLS-09** — uniform consolidation into one UBO | No port needed; D3D11 already used a single `BMDFlags` cbuffer. The GLSL UBO layout was deliberately authored to match the existing D3D11 `BMDFlagsCB` byte-for-byte so the two could be reconciled rather than duplicated. |
| **GLP-04** — dirty-checked water-flag writes | Merged with D3D11's pre-existing `UploadD3D11Flags()` dirty-check into one shared sentinel pair. |
| **GLP-05 / 06 / 08 / 09 / 10 / 25** | Not applicable — GL-only concepts (see [Gotchas §5.3](gotchas-and-patterns.md)). |

### Regressions this exposed, and their fixes
All four were D3D11-only, invisible on GL, and none produced a merge conflict:

1. **All BMD objects invisible** — GLP-11's compacted bone-palette layout was never ported to the HLSL twins. → [Gotchas §5.1](gotchas-and-patterns.md)
2. **UI text showing later lines' glyphs** — `RHI_D3D11::UpdateTexture()` lacked GLP-19's flush; the GL twin had carried it since the original fix. → [Gotchas §4.3](gotchas-and-patterns.md)
3. **Black rectangles on smoke/dust particles** — four blend-mode wrappers had no D3D11 branch at all (pre-existing gap, exposed once GPU-skinned geometry began rendering). → [Gotchas §4.1](gotchas-and-patterns.md)
4. **Two compile-breaking semantic merges** — duplicate member declarations and a stale `return;` after a return-type change. → [Gotchas §5.2](gotchas-and-patterns.md)

> **Verification status**: all fixes build clean in Debug and Release; items 1 and 2 are owner-confirmed in-game. Item 3's fix is built but **not yet visually re-confirmed** at the time of writing.


