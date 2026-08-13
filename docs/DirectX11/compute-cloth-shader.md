# GPU Compute Cloth Shader

This document explains the implementation of the GPU Compute Cloth Simulation engine (`ClothComputeShader.cpp`) built on DirectX 11 DirectCompute, introduced as part of the D3D11 backend modernization.

## 1. Motivation

Capes, wings, and cloth accessories in MU Online historically relied on a CPU-bound spring-mass integration solver (`CPhysicsManager`). As character density and wing-complexity grew, this became a severe bottleneck. 

Offloading the physics integration to a GPU compute shader provides:
- Massively parallel position updates for hundreds of cloth nodes.
- Faster convergence for constraint resolution (Jacobi iterations).
- Freeing up CPU cycles on the main thread for game logic and network packet handling.

## 2. Integration Pipeline

The simulation runs tightly coupled with `CPhysicsManager`. If the D3D11 backend is active and GPU cloth is enabled (`g_GpuClothEnabled`), `CPhysicsManager` delegates ticking to the compute pipeline.

### Enabling it

GPU cloth is **opt-in and off by default** — the CPU solver remains the real path for every cloth instance until this soaks. Both switches live in the DirectX subsection of `config.ini`:

```ini
[graphics]
renderer=directx        ; required -- these are D3D11-only

[graphics.directx]
GpuCloth=1              ; g_GpuClothEnabled -- GPU simulation + draw for CPhysicsCloth
ClothComputeSelfTest=1  ; g_ClothComputeSelfTestEnabled -- one-shot GPU-vs-CPU delta check
```

`ClothComputeSelfTest` is diagnostic only: it runs a single GPU-vs-CPU comparison at startup and logs the delta to `MuError.log`. It is not part of the per-frame cloth path.

Note that `GpuCloth=1` flips **all** `CPhysicsCloth` instances at once (capes, wings, hair). `CPhysicsClothMesh` — the one mesh-topology exception — is unaffected and always uses the CPU solver.

### GPU Buffers

The simulation relies on two sets of GPU Structured Buffers (UAVs & SRVs):

1. **State Buffers (Double-Buffered)**: Contains `pos`, `prevPos`, `force`. Ping-ponged every tick.
2. **Static Topology Buffers**: Uploaded once at instance creation. Contains mass links (`linkV0`, `linkV1`), resting distances (`linkDistMax`, `linkDistMin`), and link styles.

### The Compute Passes

A single physics tick (`DispatchSpringIntegrate`) invokes several compute shader passes sequentially:

1. **Pin Skinning (`CS_PinSkin`)**:
   Transforms the "pinned" vertices (the base of the cape attached to the character's back) using the character's active bone matrix (`u_BoneMatrix`).

2. **Spring Integration (`CS_SpringIntegrate`)**:
   Calculates internal spring forces (Hooke's law based on rest distances), applies gravity/wind, and integrates velocities via Verlet integration.

3. **Loose Distance Constraints (`CS_LooseDistance`)**:
   Enforces maximum stretching constraints on specific topology links.

4. **Collision Resolution (`CS_Collision`)**:
   Reads character bounding spheres from CPU and pushes cloth nodes outside the spheres to prevent clipping through the character's body.

5. **Strict Distance Constraints (`CS_StrictDistance`)**:
   A multi-pass Jacobi solver that iteratively enforces rigid link distances (used primarily for stiff wing structures). Dispatched with variable iterations based on topology depth.

## 3. Render Pipeline Handoff

After compute integration completes, the resulting positions reside directly in VRAM. 

Instead of reading back the positions to the CPU (which would stall the pipeline), a final Compute pass (`CS_ScatterDraw`) expands the simulated positions into an interleaved Vertex Buffer (`ID3D11Buffer` mapped with `UAV`). This buffer is directly bound via `RHI::BindVertexBuffer` and rendered using `RHI::DrawIndexed`.

This provides a true zero-readback, zero-copy CPU-GPU integration path.
