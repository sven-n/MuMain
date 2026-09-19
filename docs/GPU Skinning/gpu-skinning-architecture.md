# GPU Skeletal Skinning Architecture

The active implementation spans:

- `Render/Models/ZzzBMD.cpp` and `ZzzBMD.h`;
- `Render/Renderer/MuRenderer.h`;
- `Render/Renderer/MuRendererSDLGpu.cpp`;
- `src/shaders/skinned_textured.vert.hlsl`.

It ports upstream GPU-skinning behavior into SDL GPU without restoring the
upstream `BoneUBO`, RHI, or OpenGL shader classes.

## Pipeline

```text
animation palette
  -> BMD rest vertices and position/normal bone indices
  -> IMuRenderer::RenderSkinnedTriangles
  -> per-frame vertex scratch + packed palette rows
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
- packed ABGR color;
- position bone index;
- normal bone index.

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

## CPU fallback boundaries

GPU skinning is an optimization, not a replacement for every transformed-vertex
consumer. CPU materialization remains authoritative for:

- cloth simulation and cloth rendering;
- shadow volumes and shadow-map-specific paths;
- vertex-wave deformation;
- unsupported render states or missing GPU resources.

`EnsureCpuVertices()` and `EnsureCpuNormals()` provide the lazy fallback data.

## Verification

- `tests/render/test_gpu_skinning_contract.cpp` checks the renderer contract.
- `tests/render/test_gpu_skinning_path.cmake` checks the source-to-shader path,
  CPU fallback, and chrome-mode coverage.
- `glslangValidator`, `spirv-val`, and `spirv-cross` validate the HLSL and
  checked-in SPIR-V/MSL artifacts.
