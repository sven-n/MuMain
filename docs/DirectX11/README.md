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
  Catalogues critical technical gotchas, state invariants, and rendering anomalies discovered and mitigated during the D3D11 port. **Read §5 (Cross-Backend Lockstep) before merging any GL-side rendering work into this backend** — it is the single largest source of D3D11-only bugs.

## Overview

Historically, MU Online ran entirely on OpenGL fixed-function and legacy vertex arrays. As part of a massive modernization initiative, a completely new DirectX 11 rendering backend was built (`RenderBackend::D3D11`) to live side-by-side with the OpenGL backend.

The goals of the D3D11 port include:
- Better driver stability and frame pacing on modern Windows environments.
- Support for modern hardware GPU compute (via DirectCompute) for cloth physics simulation.
- Unifying all draw calls behind a standardized `RHI` interface to ease potential future porting to other APIs.
- Paving the way for spatial and temporal upscaling technologies like FSR1, FSR2, and DLSS.

## Configuration

The renderer is selected in `config.ini` under `[graphics]`, and each backend's own settings live in a dedicated `[graphics.<renderer>]` subsection:

```ini
[graphics]
renderer=directx        ; "opengl" (default) or "directx"

[graphics.opengl]
CoreProfile=1           ; 1 = GL Core Profile (default), 0 = Compatibility rollback
MaxGLVersion=           ; empty = highest available; e.g. "4.3" to cap (GLP-08)

[graphics.directx]
D3D11DebugLayer=0       ; 1 = D3D11_CREATE_DEVICE_DEBUG (slow; for correctness bugs only)
ClothComputeSelfTest=0  ; 1 = one-shot GPU-vs-CPU cloth self-test at startup
GpuCloth=0              ; 1 = GPU cloth simulation/draw (opt-in, still soaking)
```

`renderer` accepts `opengl`/`gl` and `directx`/`d3d11`/`dx11`, matched case-insensitively. **Any unrecognized value falls back to OpenGL** — a typo must never silently hand the frame to the other backend.

Both backends' subsections are read regardless of which renderer is active; the inactive set simply goes unconsulted. (`CoreProfile` is the one exception worth knowing about: it is still consulted by backend-shared FFP-guard code, so it holds its configured value even under DirectX.)

### Migrating from the old `[Render]` section
The previous flat schema (`[Render]` with `Backend=GL|D3D11`) is **migrated automatically on first launch** — values are carried into the new sections and `[Render]` is deleted. `Backend=D3D11` becomes `renderer=directx`, so an existing install keeps the backend it was already using. Keys that were never explicitly set are not written, so they continue to pick up their compiled-in defaults. No manual edit is required.

## Working on This Backend

The GL and D3D11 paths are **twins that must move in lockstep**. Most shaders exist twice in the same `.cpp` file — a GLSL string literal and its HLSL counterpart — and the UBO/cbuffer contents are fed by one shared, backend-agnostic upload (`GlobalUBO`, `SceneUBO`, `BoneUBO`). Two consequences follow, both of which have caused real shipped bugs:

- **Editing one twin without the other compiles cleanly and fails silently at runtime.** Because the two literals occupy different line ranges in one file, git merges them without conflict.
- **A GL-side optimization can invalidate a D3D11 assumption without touching D3D11 code at all** — most often by changing a shared buffer's layout, or by introducing an ordering invariant (such as flushing deferred draw batches) that the D3D11 branch does not honour.

Before landing rendering work here, check it against [Gotchas §5.3](gotchas-and-patterns.md), which classifies whether a given GL change needs a D3D11 port, is already covered, or is a GL-only concept. **Always build both configs and test under `renderer=directx` specifically** — the majority of these defects are invisible on GL.

The boot log records which backend actually ran, so a report from a bug you're triaging is unambiguous:
```
Renderer 		: DirectX 11 (RHI_D3D11)
Renderer 		: OpenGL (Core Profile) (RHI_GL)
```
