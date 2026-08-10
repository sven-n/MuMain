# Gotchas, Technical Invariants & Architectural Patterns (DirectX 11)

This document catalogues critical technical gotchas, state invariants, and rendering anomalies discovered and mitigated during the implementation of the DirectX 11 backend (`RenderBackend::D3D11`) and Compute Cloth engines.

---

## 1. Coordinate Systems & Matrix Invariants

### 1.1 Clip Space Depth `[-1, 1]` vs `[0, 1]`
- **Gotcha**: OpenGL projects $Z$ into a clip space of `[-1, 1]`, whereas D3D11 strictly uses `[0, 1]`. When reusing OpenGL-computed projection matrices directly in D3D11, geometry depth values are improperly mapped, leading to severe z-fighting, shadow clipping, and objects bleeding through terrain (e.g., underwater geometry appearing on top).
- **Rule**: Projection matrices must be conditionally adjusted. Use the abstracted `BuildPerspectiveProjection` which maps to the correct clip-space depth ranges depending on the active `g_RenderBackend`.

### 1.2 The UI / 2D Y-Flip
- **Gotcha**: In OpenGL, texture coordinate $(0,0)$ is at the bottom-left. In D3D11, it is at the top-left. Because UI sprites heavily depend on hardcoded $(U, V)$ coordinates mapped to window pixels, simply flipping the textures upside down breaks sprite-sheet atlas offsets.
- **Rule**: `ImmediateRenderer` resolves this via an Orthographic Matrix Y-Flip. When `Backend == D3D11`, the top and bottom parameters of the orthographic projection are inverted during setup, rendering the geometry upside down in clip-space, which perfectly cancels out D3D11's top-left texture convention without requiring sprite UV rewrites.

### 1.3 Matrix Major Ordering (HLSL vs GLSL)
- **Gotcha**: GLSL handles matrices in column-major order by default, while HLSL expects them in row-major order. If constant buffers upload GLM matrices directly, the shader will multiply transformations incorrectly.
- **Rule**: All matrix multiplications in HLSL must remain identical to their GLSL counterparts, but the Constant Buffers (`cbuffer`) in HLSL must be declared with `pragma pack_matrix(column_major)` or explicitly handle transposition. The current port uses memory-layout exact matches (`std140`) to safely upload GLM column-major matrices.

---

## 2. Buffer Geometry & Memory Layouts

### 2.1 Dynamic Vertex Interleaving (9-float to 13-float)
- **Gotcha**: Legacy OpenGL `ZzzBMD` uses separate arrays or 9-float interleaved buffers (Position3, UV2, Color4). D3D11 lacks VAO separation and requires a strict single input layout format (e.g., `RHI::VertexLayout::BMDMesh` which demands a 13-float stride: Position3, UV2, Color4, Normal3, BoneIndex1).
- **Rule**: The CPU dynamic-VBO fallback paths (used for CHROME4 / OIL) must explicitly expand the 9-float array into a 13-float staging array (`m_D3D11DynStaging`) by padding normals and bone indices with zeros before uploading to `RHI::UpdateBuffer`.

### 2.2 Constant Buffer Padding (`std140`)
- **Gotcha**: D3D11 constant buffers allocate memory in 16-byte chunks (similar to `std140` in GL). If a shader declares a `float3` followed by a `float`, D3D11 packs it into 16 bytes. If the C++ `struct` does not have an explicit padding float, the memory upload boundary shifts, corrupting subsequent variables.
- **Rule**: Every struct bound to an `RHI::UpdateConstantBuffer` call must have a `static_assert(sizeof(Struct) == HLSL_BYTE_SIZE)` to prevent padding alignment drift.

---

## 3. GPU Compute (Cloth Physics)

### 3.1 Cloth Compute Zero-Readback Handoff
- **Gotcha**: Simulating physics on the GPU and then reading vertex positions back to the CPU via `Map(READ)` stalls the CPU and ruins frame pacing, negating the performance benefits of compute shaders.
- **Rule**: The CPU must never read simulated positions back. `CClothComputeShader` utilizes a final `CS_ScatterDraw` compute pass to write directly into an `ID3D11Buffer` (UAV), which is then immediately bound as a vertex buffer (`RHI::BindVertexBuffer`) for drawing.

### 3.2 Thread Group Sizing vs Geometry Depth
- **Gotcha**: `CS_StrictDistance` (the Jacobi constraint solver) operates on horizontal topology layers. If the thread group size mismatches the cloth width, or if the dispatch count is less than the constraint chain depth (`m_iNumVer`), the cloth stretches infinitely.
- **Rule**: The CPU must dynamically dispatch `CS_StrictDistance` with iterations proportional to the cape's vertical segment count (`m_iNumVer`), ensuring forces propagate entirely down the mesh chain in a single frame.

---

## 4. State Management & Abstraction

### 4.1 Fixed-Function State Guarding (`ZzzOpenglUtil.cpp`)
- **Gotcha**: Calling legacy functions like `glEnable(GL_BLEND)` or `glDisable(GL_DEPTH_TEST)` while the D3D11 backend is active will result in access violations or silent OpenGL context corruption (as the SDL GL context is not active).
- **Rule**: All global state mutators in `ZzzOpenglUtil.cpp` MUST guard their legacy GL execution behind `if (g_RenderBackend == RenderBackend::OpenGL)` or route to `RHI::SetBlendMode()` to abstract the call safely.

### 4.2 EndFrame vs PlatformSwapBuffers
- **Gotcha**: Pacing and buffer swapping happen via `SDL_GL_SwapWindow` natively. D3D11 requires `IDXGISwapChain::Present`.
- **Rule**: Both backends are synchronized by hooking `PlatformSwapBuffers()`. It unconditionally calls `RHI::EndFrame()`, allowing the active backend to handle its specific present mechanics without littering `#ifdef` logic across the main game loop.
