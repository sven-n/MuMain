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
- **Gotcha**: Calling legacy functions like `glEnable(GL_BLEND)` or `glDisable(GL_DEPTH_TEST)` while the D3D11 backend is active does **not** crash — it silently does nothing, because there is no current GL context for the driver to act on. The GPU simply retains whatever state was last set through a path that *did* have a D3D11 branch. This is far more dangerous than an access violation: it fails invisibly and only manifests as a scene-dependent visual artifact much later.
- **Rule**: All global state mutators in `ZzzOpenglUtil.cpp` MUST guard their legacy GL execution behind `if (g_RenderBackend == RenderBackend::D3D11) { ...RHI call...; return; }` (note: the enum is `RenderBackend::GL` / `RenderBackend::D3D11` — there is no `RenderBackend::OpenGL`) or route to `RHI::SetBlendMode()` to abstract the call safely.
- **Real occurrence (2026-08-14)**: Four of the eight blend-mode wrappers — `EnableLightMap()`, `EnableAlphaBlendMinus()`, `EnableAlphaBlend2()`, `EnableAlphaBlend4()` — shipped with **no D3D11 branch at all**, falling through to raw `glBlendFunc` no-ops. `RHI::BlendMode` and `RHI_D3D11.cpp`'s blend-state table implemented all eight correctly the whole time; only the wrappers were incomplete. Symptom: smoke/dust particles rendered as opaque black rectangles, because `EnableAlphaBlendMinus()`'s subtractive blend (`ZERO, ONE_MINUS_SRC_COLOR`) was never actually applied to the D3D11 pipeline. **When auditing this family, check every member — a partial port looks identical to a complete one at the call site.**

### 4.3 `RHI::SetBlendMode()` Is Immediate — Flush Deferred Batches First
- **Gotcha**: `RHI::SetBlendMode()` binds an `ID3D11BlendState` on the device context *immediately*. `IR::` (`ImmediateRenderer`), since GLP-19, **defers** its draw: `IR::End()` only marks a batch pending, and the actual `RHI::Draw` happens at the next `IR::Flush()`. A blend-mode change between a batch's `End()` and its eventual flush therefore submits those quads under the **new** blend state.
- **Rule**: Every D3D11 branch that changes pipeline state must call `IR::Flush()` *before* the state change — and must dirty-check first, or it flushes on every particle and destroys the batching win entirely:
  ```cpp
  if (g_RenderBackend == RenderBackend::D3D11)
  {
      if (AlphaBlendType == 4) return;   // dirty-check FIRST
      IR::Flush();                       // then flush the pending batch
      AlphaBlendType = 4;
      RHI::SetBlendMode(RHI::BlendMode::Minus);
      return;
  }
  ```
- **Scope of the invariant** (GLP-19's own phrasing: *"a deferred IR batch may never survive another draw"*): this applies to `RHI_D3D11.cpp`'s `Draw()`, `DrawIndexed()` and `UpdateTexture()`; `PassthroughShader`'s `BindD3D11()` / `SetUseTexture()` / `SetUseFog()` D3D11 branches; and all eight `ZzzOpenglUtil.cpp` blend-mode wrappers. `BindState.cpp` and `GlobalUBO.cpp` are backend-shared, so their existing flush hooks already cover D3D11.
- **Note on texture content**: `UpdateTexture()` needs a flush even though it never changes texture *identity* — the font atlas is a single texture whose pixels are rewritten per string, so a merged batch would sample whatever was uploaded last. Symptom: each line of UI text displays a later line's glyphs. **Texture content is batch state, not just texture identity.**

### 4.2 EndFrame vs PlatformSwapBuffers
- **Gotcha**: Pacing and buffer swapping happen via `SDL_GL_SwapWindow` natively. D3D11 requires `IDXGISwapChain::Present`.
- **Rule**: Both backends are synchronized by hooking `PlatformSwapBuffers()`. It unconditionally calls `RHI::EndFrame()`, allowing the active backend to handle its specific present mechanics without littering `#ifdef` logic across the main game loop.

---

## 5. Cross-Backend Lockstep (GL ↔ D3D11 Drift)

The single largest source of D3D11-only bugs is not D3D11 itself — it is **GL-side work landing without its HLSL/D3D11 counterpart**. Optimization work is authored and measured against GL, so a change that is correct-by-construction there can silently invalidate an assumption the D3D11 path depends on. Every bug in §4.1, §4.3 and §5.1 below came from this pattern.

### 5.1 Shared Resources Bind Both Backends to One Layout
- **Gotcha**: `BoneUBO`, `GlobalUBO` and `SceneUBO` are backend-agnostic — one CPU-side upload feeds both the GLSL UBO and the HLSL cbuffer. Changing the *layout* of that upload is therefore a change to both shaders, even when only the GLSL twin is edited.
- **Real occurrence (GLP-11)**: The bone palette was compacted from a full 4×4 matrix per bone to **3 packed `float4` affine rows** (`u_Bones[600]`, 200 bones × 3 rows). The GLSL shader was updated to reconstruct via dot products; the HLSL twins in `BMDMeshShader.cpp` and `PlanarShadowShader.cpp` still declared `column_major matrix u_Bones[200]` and read via `mul(u_Bones[i], pos)`. Both backends kept reading the *same* buffer, so D3D11 was indexing the palette at wrong byte offsets — every GPU-skinned mesh received garbage transforms and collapsed to degenerate geometry. Symptom: **all BMD objects invisible under D3D11; terrain and particles (no bones) unaffected.**
- **Rule**: When changing any shared UBO/cbuffer layout, update **all** shader twins in the same commit, and grep for the uniform name across `Render/Shaders/` before assuming a single file is the only consumer. `PlanarShadowShader` reads the same slot-2 bone cbuffer as `BMDMeshShader` and is easy to miss.

### 5.2 Git Merges Do Not Flag Semantic Conflicts
- **Gotcha**: A GLSL edit and its HLSL twin live in different string literals in the *same* `.cpp` file. Non-overlapping line ranges merge cleanly with **no conflict markers**, so a lockstep break survives the merge silently and the build still succeeds.
- **Real occurrences (2026-08-14 merge)**: `TerrainShader.h` ended up declaring `m_LastBaseIsWater` / `m_LastOverlayIsWater` **twice** (each branch added its own copy for the same dirty-check purpose, at different line ranges) → `C2086: redefinition`. `ZzzOpenglUtil.cpp`'s `EnableVSync()` / `DisableVSync()` kept bare `return;` statements in their D3D11 branches after the other branch changed the return type to `bool` → `C2561`. Both were caught only by compiling, not by the merge.
- **Rule**: After any merge touching `Render/`, **build both configs before trusting a clean `git status`** — and treat a compile error in a file you did not consciously resolve as evidence of a semantic conflict, not a trivial typo. A merge that reports zero conflicts is not the same as a merge that is correct.

### 5.3 Which GL Optimizations Actually Reach D3D11
Not every GL-side optimization implies a D3D11 gap. Classifying the change first avoids both missed ports and pointless ones:

| Category | Examples | D3D11 action |
|---|---|---|
| **Backend-agnostic — applies for free** | GLP-16 terrain texture-pair bucketing, GLP-19 IR batching (draws via `RHI::DrawIndexed` / `RHI::Draw`) | None; D3D11 inherits the win. **But** verify any new invariant it introduces holds on the D3D11 path (GLP-19's flush rule did not — see §4.3). |
| **D3D11 already had an equivalent** | GLS-09 uniform consolidation (D3D11 used a cbuffer from day one), GLP-04 dirty-checked uniform writes (`UploadD3D11Flags()` had its own) | Reconcile into one shared implementation rather than porting. |
| **GL-only concept** | GLP-05 `glActiveTexture` monopoly, GLP-06 VAO attribute baking, GLP-08 GL context version probing, GLP-09/10/25 UBO ring allocator | None. D3D11 uses static input layouts and per-buffer `Map`/`WRITE_DISCARD`; there is no ring to grow and no VAO to bake. |
| **Instrumentation, soft-fails** | GLP-01 GPU pass timers (`SDL_GL_GetProcAddress` returns null with no GL context) | None; reads 0 ms under D3D11 by design. |
