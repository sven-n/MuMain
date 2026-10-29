# GPU Skeletal Skinning Architecture

The active implementation spans:

- `Render/Models/ZzzBMD.cpp` and `ZzzBMD.h`;
- `Render/Renderer/MuRenderer.h`;
- `Render/Renderer/MuRendererSDLGpu.cpp`;
- `Render/Terrain/TerrainHeightGrid.h`, for body shadows;
- `src/shaders/skinned_textured.vert.hlsl`, `skinned_weighted.vert.hlsl`, and their
  shadow variants `skinned_shadow.vert.hlsl` and `skinned_weighted_shadow.vert.hlsl`.

It ports upstream GPU-skinning behavior into SDL GPU without restoring the
upstream `BoneUBO`, RHI, or OpenGL shader classes.

## Pipeline

```text
animation palette
  -> BMD rest vertices and position/normal bone indices
  -> IMuRenderer::RenderSkinnedTriangles, or RenderCachedSkinnedMesh
  -> per-frame vertex scratch or cached mesh + packed palette rows
  -> deferred DrawSkinnedTriangles command
  -> SDL GPU vertex/storage bindings
  -> skinned_textured.vert.hlsl
```

The renderer returns `false` when any requirement is unavailable. `BMD` then
uses the established CPU-skinned path.

## Vertex and palette layout

`SkinnedVertex3D` carries:

- rest position: three floats;
- rest normal: three floats;
- source UV: two floats;
- position bone index;
- normal bone index.

That is 40 bytes. A glTF model's vertex (`SkinnedWeightedVertex3D`, 56 bytes)
adds four joints, their weights and a tangent, and is drawn by its own
pipelines with `skinned_weighted.vert.hlsl`, so BMD vertices do not carry them.
Both shaders share their code through `skinned_vertex.hlsli`. The mesh cache
holds both sizes, in 8-byte units.

The colour is one packed ABGR value per draw, read as a per-instance attribute
from vertex buffer slot 1. A mesh without a scrolling texture is cached on the
GPU under its `Mesh_t*` the first time it is drawn (`CacheSkinnedMesh`), so
later draws upload only that colour; `BMD::Release()` releases the entry before
it frees the mesh data.

`SkinningParameters::boneMatrices` contains 12 floats per bone in row-major
3x4 affine form:

```text
[ r00 r01 r02 tx ]
[ r10 r11 r12 ty ]
[ r20 r21 r22 tz ]
```

The renderer appends these rows to per-frame bone scratch storage. The command
stores the starting row and bone count. `EndFrame()` uploads the combined data
to one SDL GPU storage buffer and binds it at vertex storage slot 0.

Upstream's corrected `BoneUBO` slot-2 documentation does not transfer directly.
The downstream HLSL binds `boneRows` at `register(t0, space0)`, using a raw
`ByteAddressBuffer` for D3D12 and a structured buffer elsewhere. Both match the
SDL GPU vertex-storage slot-0 binding.

## Palette deduplication

`SetActiveBoneTransform()` unconditionally stores the pointer and increments
`g_BoneTransformVersion` on every call. This preserves the upstream audit
correction: pointer equality alone is not treated as unchanged content.

Consecutive meshes often share one animated skeleton. `RecordBonePalette()`
reuses the previous offset only when the matrix pointer, palette size, and
`paletteVersion` all match.

The version is required because stack-local palettes can reuse an address after
their contents change. Pointer-only reuse would animate a mesh with stale bones.

## Shader transforms

For a translated vertex, the shader evaluates the three palette rows against
`float4(restPosition, 1)`. For the legacy non-translate form, translation and
bone scale follow the original affine equation. The normal uses the separate
normal bone index and the rows' 3x3 portion.

Body placement then applies the captured origin and scale before the current MVP
matrix:

```text
world = bodyOrigin + bodyScale * skinnedPosition
clip  = MVP * float4(world, 1)
```

When placement is already encoded in the palette, the submitted translate flag
prevents applying body origin twice.

## Lighting, fog, and texture coordinates

Each command snapshots:

- light-enabled flag and direction;
- fog start and end;
- body origin and scale;
- bone and rest-pose scales;
- palette offset and count;
- texture-coordinate mode and animated chrome parameters.

Downstream does not use the upstream render-mode enum corrected in #546. It
passes a direct `SkinningTextureCoordinates` value to the shader:

| Shader mode | Downstream value |
|---:|---|
| 0 | Mesh UV |
| 1 | Chrome |
| 2 | Chrome2 |
| 3 | Chrome3 |
| 4 | Chrome4 |
| 5 | Chrome5 |
| 6 | Chrome6 |
| 7 | Chrome7 |
| 8 | Oil |
| 9 | Metal |

The shader has a distinct formula for every row. Eligible chrome, oil, and
metal draws therefore stay on the GPU path instead of collapsing aliases into a
single variant. Unsupported state still falls back to CPU materialization.

## Pipeline selection

Dedicated skinned pipeline arrays cover the same blend index, cull state, depth
test, and depth-write combinations as ordinary 3D draws. GPU submission is
rejected when the required pipeline, texture, sampler, frame, palette, or vertex
allocation is unavailable.

## Body shadows

`BMD::RenderBodyShadow()` draws a model's projected shadow. `CalcShadowPosition()`
shears each skinned vertex away from the body origin and places it 5 units above
the terrain height from `RequestTerrainHeight()`. `skinned_shadow.vert.hlsl`
applies the same projection to the cached mesh, so the shadow no longer skins
and uploads its vertices on the CPU every frame.

- **Shadow streams.** `BMD::RenderGpuMeshShadow()` caches one rest-pose stream
  per model and mesh selection: range, hidden mesh, and blend texture. It draws
  the stream with one `RenderCachedSkinnedShadow()` call, using the skinning
  parameters `EnsureCpuVertices()` would apply at that moment. The streams are
  tracked outside `BMD`, and `BMD::Release()` frees them.
- **glTF models.** `BMD::RenderWeightedMeshShadow()` draws each mesh's shadow
  from the mesh its body draw caches, at the LOD being drawn, with
  `skinned_weighted_shadow.vert.hlsl` and the weighted palette. Both shadow
  shaders share `skinned_shadow.hlsli`. A mesh the GPU cannot draw gets its
  CPU shadow, alone.
- **Terrain heights.** The shader reads a `Render::TerrainHeightGrid`: every
  cell's height, then one `TW_HEIGHT` bit per cell. `RequestTerrainHeight()`
  uses the same sampler over the raw map arrays, so both sides follow one set of
  rules. `SetTerrainHeights()` uploads the grid only when
  `TerrainHeightVersion()` changes, which happens at map loads and when a
  catapult crater or a terrain attribute changes the map.
- **Bindings.** The draw binds the bone rows at vertex storage slot 0 and the
  grid at slot 1. Vertex uniform slot 1 carries the body origin, the shear, the
  height of `TW_HEIGHT` cells, and whether the scene has terrain at all.
- **Pipelines.** Shadow pipelines, one pair per vertex layout, use the active blend mode with depth writes
  off, the state the CPU shadow draws in, with depth testing on or off. Both
  sets are optional and are skipped when the shader does not load.

## CPU fallback boundaries

GPU skinning is an optimization, not a replacement for every transformed-vertex
consumer. CPU materialization remains authoritative for:

- cloth simulation and cloth rendering, including clothes shadows;
- body-shadow meshes whose CPU vertices already exist this frame, such as cloth
  write-back or a mesh skinned earlier at another body origin;
- shadow volumes and shadow-map-specific paths;
- vertex-wave deformation;
- unsupported render states or missing GPU resources.

`EnsureCpuVertices()` and `EnsureCpuNormals()` provide the lazy fallback data.

## Verification

- `tests/render/test_gpu_skinning_path.cpp` checks which meshes select CPU
  skinning.
- `tests/render/test_sdl_gpu_replay_state.cpp` checks replay bindings, including
  the colour slot and a shadow's second uniform slot and storage buffer.
- `tests/render/test_terrain_height_grid.cpp` compares the shadow grid with
  `RequestTerrainHeight()`'s lookup at the map edges and at random points.
- `cmake/ValidateShaderReflection.cmake` and `ValidateDxilReflection.cmake`
  check the SPIR-V, MSL, and DXIL bindings of the skinned shaders against the
  renderer's shader metadata.
