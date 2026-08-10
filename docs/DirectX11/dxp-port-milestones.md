# DXP Port Milestones Reference

This document serves as the authoritative technical reference catalog for all **`DXP-xx`** milestone tags used in source code comments, commit logs, and architectural document cross-references specifically related to the DirectX 11 Port.

The milestones below are organized **chronologically in the exact order of implementation**, grouped by development phase.

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


