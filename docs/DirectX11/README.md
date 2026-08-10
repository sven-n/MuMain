# DirectX 11 Rendering Backend Documentation

This folder contains the technical documentation for the DirectX 11 (D3D11) Rendering Backend implemented in the MU Online client.

## Quick Links

- [DirectX 11 Backend Architecture](dx11-backend-architecture.md)
  Overview of the `RHI` (Render Hardware Interface) design, swapchain management, rendering loop, and the HLSL shader infrastructure.

- [GPU Compute Cloth Shader](compute-cloth-shader.md)
  Details the compute shader physics simulation for capes, wings, and cloth accessories.

- [DXP Port Milestones Reference](dxp-port-milestones.md)
  Authoritative technical reference catalog for all `DXP-xx` milestone tags used in source code comments related to the DirectX 11 Port.

- [Gotchas & Architectural Patterns](gotchas-and-patterns.md)
  Catalogues critical technical gotchas, state invariants, and rendering anomalies discovered and mitigated during the D3D11 port.

## Overview

Historically, MU Online ran entirely on OpenGL fixed-function and legacy vertex arrays. As part of a massive modernization initiative, a completely new DirectX 11 rendering backend was built (`RenderBackend::D3D11`) to live side-by-side with the OpenGL backend.

The goals of the D3D11 port include:
- Better driver stability and frame pacing on modern Windows environments.
- Support for modern hardware GPU compute (via DirectCompute) for cloth physics simulation.
- Unifying all draw calls behind a standardized `RHI` interface to ease potential future porting to other APIs.
- Paving the way for spatial and temporal upscaling technologies like FSR1, FSR2, and DLSS.

The backend can be toggled via `config.ini`:
```ini
[Render]
Backend=D3D11 ; GL = OpenGL, D3D11 = DirectX 11
```
