# GPU Rendering Gotchas and Invariants

## Bone palettes

- A palette is a contiguous span of row-major 3x4 affine matrices: exactly 12
  floats per bone. `RenderSkinnedTriangles()` rejects empty or misaligned spans.
- Position and normal bone indices are separate. Reusing the position index for
  normals changes lighting on models whose source data distinguishes them.
- Pointer-only palette deduplication is unsafe. Stack-local arrays may reuse an
  address with different contents. Cache identity therefore includes both the
  palette pointer and `paletteVersion`.
- The per-frame bone storage buffer must grow before recording an offset. A
  failed growth or upload returns the draw to CPU fallback rather than recording
  a partial command.
- Uniform arrays and the nested skinning uniform block are value-initialized.
  The shader reads full 16-byte lanes even when C++ assigns only the used fields.

## Lazy CPU materialization

- `TransformCheap()` records animation state without eagerly transforming every
  vertex.
- `EnsureCpuVertices()` and `EnsureCpuNormals()` materialize data for fallback
  consumers. State such as body scale, bone scale, and translate mode must be
  captured when the deferred transform is requested, not read later from
  mutable globals.
- Cloth, shadow-volume, shadow-map, and vertex-wave paths intentionally remain
  CPU consumers. GPU eligibility must not bypass their materialization calls.
- A body shadow whose meshes already have CPU vertices this frame stays on the
  CPU. Those positions may come from cloth write-back, or from a skin at an
  earlier body origin; the GPU would skin them again from the current state.

## Shader compatibility

- `SkinningTextureCoordinates` distinguishes mesh UVs, Chrome through Chrome7,
  Oil, and Metal. Do not collapse modes merely because legacy render flags share
  bits.
- Chrome UVs depend on animated wave, light, offset, and time values. A GPU path
  is equivalent only when those parameters are forwarded.
- `translate == false` means placement is already represented by the palette;
  the shader must not add body origin a second time.
- C++ uniform layout, HLSL declarations, and generated MSL/SPIR-V bindings form
  one contract. Keep size assertions and shader validation together.
- glslang numbers HLSL vertex inputs in declaration order and ignores
  `TEXCOORDn`. A shader that reads only some attributes of a layout must pin them
  with `[[vk::location(n)]]`; dxc warns that it ignores the attribute.
- spirv-cross numbers MSL buffers in order of first use, and SDL expects uniform
  buffers before storage buffers. A vertex shader with both must read its
  cbuffers before any storage load, or the reflection validation fails.

## Terrain heights

- Body shadows read a GPU copy of `BackTerrainHeight` and the `TW_HEIGHT` cells,
  uploaded again only when `TerrainHeightVersion()` changes. Code that changes
  either must go through the ZzzLodTerrain.cpp functions that increase it, or
  call `MarkTerrainHeightsChanged()` after writing the arrays directly, as the
  map editor's brushes and undo do; otherwise shadows keep following the old
  ground.
- `g_fSpecialHeight` follows the camera, so it travels in the shadow uniforms,
  not in the grid.

## Deferred command lifetime

- Draw inputs must be copied into renderer-owned per-frame scratch storage before
  the caller's span expires.
- A `RenderCmd` snapshots pipeline, texture, sampler, offsets, fog, and vertex
  uniforms. Replay must not consult mutable draw state from a later call.
- Commands replay in submission order. Merging is allowed only for adjacent
  triangle commands with identical compatible state.
- `BeginFrame()` and `EndFrame()` bound all queued data. Submission outside an
  active frame is rejected.

## Batching and profiling

- Terrain draws its ground first and its blended overlays after, each in its
  original order; do not sort either by texture. The split relies on the
  overlays staying depth-tested with depth writes off (`LESS_OR_EQUAL` against
  their coplanar ground); an overlay drawn with the depth test off would paint
  over nearer ground. `terrain_depth_state_contract` guards this. Only adjacent
  compatible commands may merge.
- A merge key must include pipeline, texture, sampler, MVP, fog, and contiguous
  vertex storage. Omitting captured state can replay earlier geometry with a
  later draw's settings.
- `FrameProfiler` is renderer-neutral. Do not add raw GL or SDL GPU query calls
  to it; backend statistics belong in `RendererStats`.
- `$glstats` reports CPU pass and SDL GPU submission timings. It does not claim
  per-pass GPU time.
- Reset profiler values after every overlay has consumed them, including frames
  where neither overlay is visible.

## Textures and resources

- Game bitmap IDs are not SDL GPU handles. Resolve them through the renderer's
  texture and sampler registries.
- Unknown textures, unavailable pipelines, or failed buffer growth skip the GPU
  draw safely. Never enqueue a command with incomplete resources.
- Frame readback is request-driven. Ordinary frames must not allocate transfer
  buffers or wait on readback fences.
- `OpenPlayers()` zero-fills the `Models` array after constructing it. A `BMD`
  member must survive being zeroed, so per-model renderer state such as shadow
  streams lives outside `BMD`.

## Compatibility boundary

- Historical GL-shaped helpers are wrappers, not permission for gameplay code
  to call raw graphics APIs.
- `tools/check_gl_wrapper_monopoly.py` must remain green. New rendering behavior
  belongs in `IMuRenderer`, its SDL GPU backend, or an existing render-layer
  compatibility wrapper.

## Animation synchronization

- `AnimationTaskPool` workers may update bone transforms asynchronously. Callers
  must honor the established wait barrier before reading shared palettes.
- Render-frequency code must not advance fixed-step simulation once per rendered
  frame. Existing timing controls preserve animation and cloth cadence when FPS
  changes.

## Batch-break attribution

- `$glstats` reports SDL GPU triangle batch draws, vertices per batch, and break
  causes for texture, blend, depth, pipeline, uniform, matrix, intervening draw,
  or other barriers.
- Attribute breaks where deferred SDL GPU commands fail to merge. State-wrapper
  calls only update logical renderer state; adding flush hooks there would restore
  the retired OpenGL ownership model and double-count attempted changes.
- Do not count the first triangle command of a frame as a break. A break requires
  an existing triangle batch candidate that the next submission cannot join.
