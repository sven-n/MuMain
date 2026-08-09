# Gotchas, Technical Invariants & Architectural Patterns

This document catalogues critical technical gotchas, thread safety rules, cache invalidation pitfalls, and performance invariants established during the implementation of the GPU skinning engine and modern graphics subsystem.

---

## 1. Skeletal Skinning & Memory Invariants

### 1.1 `MAX_BONES = 200` Allocation & Upload Rule
- **Gotcha**: Equipped armor models (`bmdArmor`) store small internal `NumBones` counts (e.g. 12 to 15), but armor vertices reference bone indices across the entire character skeleton (0 to 199).
- **Rule**: `BoneUBO::UploadBones()` must upload all `MAX_BONES` (200) matrices. Heap allocations for `BoneTransform` across all character, pet, and object structures must allocate `new vec34_t[MAX_BONES]` to prevent out-of-bounds heap reads during upload.

### 1.2 Bone Palette Deduplication Must Compare Content, Not Identity
- **Gotcha**: Sub-item rendering (weapons, wings, attached effect meshes) shares a single fixed-address scratch buffer (`BoneTransform[MAX_BONES][3][4]` in `ZzzBMD.cpp`) across genuinely different objects. Consecutive rendering calls frequently receive the *exact same memory address* populated with *different matrix data* one call apart.
- **Rule**: Never use pointer-only comparison (`pTransform == g_pLastTransform`) to skip uploading bone matrix uniform buffers — the same address legitimately carries different content from one call to the next. A version-stamp-plus-pointer scheme doesn't reliably fix this either unless the stamp is *only* bumped when the content actually changes — a stamp bumped unconditionally on every call defeats its own purpose (never dedups, even for genuinely identical consecutive uploads). `BoneUBO::UploadBones()` instead compares the incoming bone-matrix **bytes** directly against a cached snapshot of the last upload: unconditionally correct (no identity/aliasing hazard possible) and still cheap, since the comparison is far smaller than the GPU upload it's deciding whether to skip.

### 1.3 `TransformCheap` Lazy Materialization & Ambient Global State Snapshotting
- **Pattern**: `BMD::TransformCheap()` defers CPU vertex skinning calculations until a consumer explicitly requests CPU vertex/normal positions via `EnsureCpuVertices()`.
- **Gotcha**: Ambient global state (such as `BoneScale`) set prior to `Transform()` and reset immediately after will be lost if read later during deferred materialization.
- **Rule**: Deferred computations depending on ambient global state **MUST** snapshot that state into instance variables (e.g., `m_LastBoneScale`) at the moment the request is made.

### 1.4 Planar Shadow Division Explosion & Dummy Bones
- **Gotcha**: Planar shadow projection applies a GPU skew matrix involving division: $\text{skewX} = \frac{z_{\text{rel}} \cdot (x_{\text{rel}} + s_x)}{z_{\text{rel}} - s_y}$. Models with uninitialized or Dummy bone slots carry stale memory into `u_Bones`, causing division near zero and stretching shadows across billions of world units ("infinity shadow").
- **Rule**: `BoneTransform` arrays must be fully initialized/zeroed, including Dummy bone slots that do not contain mesh vertex weights.

---

## 2. Rendering, Shaders & Color Pipeline Gotchas

### 2.1 Vertex Color Clamping in GLSL Shaders
- **Gotcha**: Fixed-function OpenGL clamped vertex colors `glColor3fv()` to `[0, 1]` automatically before texture modulation. In GLSL shaders, unclamped vertex color values $> 1.0$ (from intense point lights) multiply texture colors to pure blown-out white.
- **Rule**: All fragment shaders modulating vertex colors must explicitly clamp input colors: `clamp(v_Color, 0.0, 1.0)`.

### 2.2 Collapsed/Aliased Render-Mode Enum Dispatch
- **Gotcha**: `BMD::RenderMesh()` evaluates GPU skinning eligibility against `finalRenderFlags`. Multiple distinct visual flags (CHROME2, CHROME3, CHROME5, CHROME6, CHROME7, METAL) collapse into the single enum value `RENDER_CHROME`.
- **Rule**: When adding GPU shader eligibility checks keyed on an enum, verify every flag that maps to that bucket shares identical downstream shader mathematics.

### 2.3 Bind Cache Invalidation on Resource Deletion (`glDelete*`)
- **Gotcha**: Driver resource deletion functions (`glDeleteVertexArrays`, `glDeleteTextures`) silently reset active driver bindings to 0 when deleting the currently bound resource. A driver bind cache unaware of deletion retains stale resource IDs, causing subsequent `Bind*(id)` calls to falsely hit the cache and skip real binding.
- **Rule**: Every deletion call site must explicitly invalidate binding caches via `InvalidateVAOCache()`, `InvalidateTextureCache()`, etc.

---

## 3. Concurrency, Animation & Frame Decoupled Sync

### 3.1 `AnimationTaskPool` Thread Safety & Read Barriers
- **Gotcha**: `AnimationTaskPool` computes character skeletal animations asynchronously across background worker threads. `WaitCharactersAnimation()` provides the mandatory synchronization barrier.
- **Rule**: Code executing outside the main render pass (e.g., logic ticks or mouse updates) **MUST NEVER** read `o->BoneTransform` matrices directly while worker threads are actively mutating them.

### 3.2 Decoupled Render FPS & Fixed-Timestep Animation Acceleration
- **Gotcha**: Functions advancing state by a fixed per-call increment scaled by `FPS_ANIMATION_FACTOR` (cloth physics `Move2()`, wing flap animation, tour cameras) will speed up proportionally when render FPS increases from 60 FPS to 250+ FPS if invoked from a render-frequency call site.
- **Rule**: All render-frequency animation/physics updates must be throttled using real-time timers (`std::chrono::steady_clock`) to maintain a constant ~60 Hz simulation rate independent of rendering frame rate.

### 3.3 One-Shot UI Render Flags
- **Gotcha**: UI flags that are set by logic ticks and immediately cleared on the first render frame (e.g., `m_bRenderSkillInfo`) cause visual flickering/strobing when rendering uncapped above the 50 Hz/60 Hz logic tick rate.
- **Rule**: Render pass code must only **read** UI state flags; logic tick handlers remain the sole authority responsible for clearing interaction flags.
