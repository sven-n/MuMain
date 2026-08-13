#pragma once

// Render hardware interface -- a backend-agnostic buffer/texture/uniform-block/draw
// abstraction so rendering code targets neither GL nor a future backend directly.
// RHI_GL (Render/RHI/RHI_GL.h/.cpp) is the only backend implementation today.
//
// Implementation status: Device/frame, Buffers+Uniform blocks, Pipeline/blend,
// VertexLayout/BindVertexBuffer + Draw, BindIndexBuffer + DrawIndexed, and Textures +
// ReadColorFramebuffer/ReadDepthPixel are implemented in RHI_GL. IR:: (ImmediateRenderer.cpp)
// is the first caller of the buffer/draw half; terrain tile bucketing (GLP-16) is the first
// caller of BindIndexBuffer/DrawIndexed.

// DXVK/vkd3d-proton on Linux -- is a one-line change instead of hunting down every guard.
// Every RHI_D3D11.cpp/BMDMeshShader.cpp/PassthroughShader.cpp/PlanarShadowShader.cpp/
// TerrainShader.cpp D3D11 code path is gated on this macro, never on _WIN32 directly.
#ifdef _WIN32
#define RHI_D3D11_AVAILABLE 1
#endif

#include <cstddef>
#include <cstdint>

namespace RHI {

// Opaque, backend-owned handles. Strongly typed so a BufferHandle can't be
// passed where a TextureHandle is expected -- catches backend-swap mistakes
// at compile time, not runtime.
template<typename Tag>
struct Handle {
    uint32_t id = 0;
    bool IsValid() const { return id != 0; }
    bool operator==(const Handle&) const = default;
};
using BufferHandle  = Handle<struct BufferTag>;
using TextureHandle = Handle<struct TextureTag>;

// ---- Capabilities (GLP-08) ----
// Populated once, in Init(), by probing the actual context/driver -- never hardcoded to "assume
// the latest". bufferStorage/uboOffsetAlignment are consumed by GLP-09's UBO ring allocator
// (RHI_GL.cpp); the rest exist so GLP-15/22 can be written as "use the fast path if Caps says
// so, keep today's path otherwise" instead of guessing.
// Each bool requires BOTH the version/extension being present AND the entry points that
// capability needs actually resolving via SDL_GL_GetProcAddress -- a driver can report a high
// version and still hand back null for a symbol on a bad install.
struct Caps {
    int  glMajor = 0, glMinor = 0;
    bool bufferStorage        = false;  // ARB_buffer_storage / core 4.4       -> GLP-09
    bool vertexAttribBinding  = false;  // ARB_vertex_attrib_binding / core 4.3 -> GLP-22
    bool programBinary        = false;  // ARB_get_program_binary / core 4.1   -> GLP-15
    bool timerQuery           = false;  // ARB_timer_query / core 3.3          -> GLP-01
    int  uboOffsetAlignment   = 256;    // GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT -- GLP-09 ring stride, do not hardcode 256 elsewhere
    int  maxUniformBlockSize  = 16384;  // GL_MAX_UNIFORM_BLOCK_SIZE
};
const Caps& GetCaps();

// ---- Device/frame (implemented in RHI_GL) ----
bool Init(void* nativeWindowHandle, int width, int height);
void Shutdown();
void BeginFrame();
void EndFrame();                          // Present() -- wraps PlatformSwapBuffers()
void SetViewport(int x, int y, int w, int h);
void Clear(bool color, bool depth, float r = 0, float g = 0, float b = 0, float a = 1);
// DXP-13: window resize/alt-tab hook. GL is a no-op (matches its pre-DXP-13 behavior --
// nothing GL-specific happened at resize time either). D3D11 releases and recreates its
// swapchain buffers/views (IDXGISwapChain::ResizeBuffers) -- required there or Present()
// either stretches the old-sized backbuffer or errors outright.
void OnResize(int width, int height);

// ---- Buffers ----
// Static: immutable after creation. Dynamic: UpdateBuffer uses map-discard semantics
// (orphan-then-glBufferSubData) -- IR::'s existing streaming VBO pattern and
// PlanarShadowShader's own VBO both become Dynamic buffers, unchanged behavior.
enum class BufferUsage { Static, Dynamic };
BufferHandle CreateVertexBuffer(const void* initialData, size_t sizeBytes, BufferUsage);
BufferHandle CreateIndexBuffer(const void* initialData, size_t sizeBytes, BufferUsage); // GL_UNSIGNED_INT only -- the one indexed site in the tree uses no other type
void UpdateBuffer(BufferHandle, const void* data, size_t sizeBytes);   // Dynamic only; MAY exceed creation size (backends re-allocate)
// Discard semantics: each call may assume the buffer's previous contents are gone. Correct
// for a buffer that's fully rewritten once per use (a UBO, a per-frame full-geometry upload)
// -- wrong for a buffer written many times per frame at growing offsets, where a discard-per-
// call would force the GPU to sync on data it may still be reading from the previous
// sub-write, exactly the stall DXP-26 eliminated for IR::'s streaming VBO. Use AppendBuffer
// for that pattern instead.
//
// AppendBuffer: writes `data` at a running byte offset into a Dynamic buffer without
// discarding prior writes, growing (2x needed size) or wrapping back to offset 0 only when
// the write wouldn't fit -- exactly IR::End()'s pre-RHI ring-buffer logic (DXP-26), now owned
// by RHI_GL so IR:: itself holds no raw GL. Returns the byte offset the data was written at
// (divide by the vertex stride for Draw's firstVertex). Do not mix AppendBuffer and
// UpdateBuffer on the same handle -- UpdateBuffer's discard-and-reset-to-0 would corrupt
// AppendBuffer's offset bookkeeping.
size_t AppendBuffer(BufferHandle, const void* data, size_t sizeBytes);
void DestroyBuffer(BufferHandle);

// ---- Uniform blocks (the 3 existing UBOs become opaque handles; std140 layout stays the
// caller's contract -- RHI does not know or care about field layout, just byte size) ----
// GLP-09: RHI_GL sub-allocates every binding slot from a persistently-mapped per-slot ring
// buffer instead of giving each handle its own dedicated GL buffer object -- purely a backend
// implementation detail, this whole-block-update contract is unchanged. One consequence IS
// caller-visible though: UpdateUniformBlock is what establishes the binding now (a
// glBindBufferRange per update), not CreateUniformBlock (which used to glBindBufferBase once,
// at creation). A block that is created and never updated has no binding. Every caller in the
// tree today updates immediately after creating (GlobalUBO/SceneUBO/BoneUBO/BMDMeshShader) --
// keep that invariant for any future caller.
BufferHandle CreateUniformBlock(size_t sizeBytes, int bindingSlot);
void UpdateUniformBlock(BufferHandle, const void* data, size_t sizeBytes);  // whole-block; callers keep their own dirty-checking (e.g. BoneUBO's ptr+version check)
void DestroyUniformBlock(BufferHandle);  // GLP-09: frees a slot reservation, not the shared ring -- the ring itself is torn down at Shutdown()

// ---- Textures (DXP-12 GL; DXP-15 increment 1 D3D11) ----
// Format is fixed at RGBA8 -- no other internal format exists anywhere in the tree today.
// CORRECTION (DXP-12 census, 2026-08-03): one call site was missed by the DXP-09 audit this
// claim originally rested on -- ZzzBMD.cpp's BindLightMaps() still uploads GL_RGB. DXGI has no
// 24-bit RGB format either, so the fix is the same CPU-side 3->4 expansion already planned for
// GlobalBitmap.cpp's JPEG path (alpha=255), not a second RHI format. Once that lands, this
// "RGBA8 always" contract is actually true tree-wide, not just assumed.
// D3D11: TexFilter/TexWrap are not applied per-texture (no per-texture sampler object in this
// backend's design) -- see RHI_D3D11.cpp's CreateTexture comment. Every D3D11 texture samples
// through its binding shader's own shared linear/clamp sampler instead.
enum class TexFilter { Nearest, Linear };
enum class TexWrap   { Clamp, Repeat };        // no Mirror in use
struct TextureDesc { int width, height; TexFilter filter = TexFilter::Nearest; TexWrap wrap = TexWrap::Clamp; };
TextureHandle CreateTexture(const TextureDesc&, const void* initialPixelsRGBA);
void UpdateTexture(TextureHandle, int x, int y, int w, int h, const void* pixelsRGBA); // sub-rect; full re-upload = (0,0,Width,Height)
void DestroyTexture(TextureHandle);
void BindTexture(TextureHandle, int slot);     // slots 0-3 (BMDMeshShader's max observed usage)

// ---- Pipeline / blend-mode state ----
// Promotes ZzzOpenglUtil.cpp's existing internal AlphaBlendType cache (0-7) to a
// first-class named type -- not new design. The GL combos also toggle GL_FOG and
// GL_ALPHA_TEST as side effects: fog is already a SceneUBO uniform (fogEnabled) and
// alpha-test is already a shader-side discard (g_AlphaRef), so SetBlendMode's GL
// implementation sets blend/cull/depth-mask state and mirrors those same uniform writes.
enum class BlendMode : uint8_t {
    Opaque    = 0,  // DisableAlphaBlend()      no blend; cull+depthmask on
    LightMap  = 1,  // EnableLightMap()         ZERO, SRC_COLOR;               cull+depthmask on
    AlphaTest = 2,  // EnableAlphaTest()        SRC_ALPHA, ONE_MINUS_SRC_ALPHA; cull off, alpha-test on
    Additive  = 3,  // EnableAlphaBlend()       ONE, ONE;                       cull+depthmask off, fog off
    Minus     = 4,  // EnableAlphaBlendMinus()  ZERO, ONE_MINUS_SRC_COLOR;      cull+depthmask off, fog on
    Blend2    = 5,  // EnableAlphaBlend2()      ONE_MINUS_SRC_COLOR, ONE;       cull+depthmask off, fog on
    Blend3    = 6,  // EnableAlphaBlend3()      SRC_ALPHA, ONE_MINUS_SRC_ALPHA; cull+depthmask off, fog on
    Blend4    = 7,  // EnableAlphaBlend4()      ONE, ONE_MINUS_SRC_COLOR;       cull+depthmask off, fog on
};
void SetBlendMode(BlendMode);

// No depth-func API at all: GL_LEQUAL is the ONLY glDepthFunc value in the tree (set
// per-frame in BeginOpengl). RHI backends hardwire LESS_EQUAL.
void SetDepthTestEnabled(bool);

// DXP-15 increment 3: standalone cull/depth-write toggles, independent of SetBlendMode's
// combo. EnableCullFace/DisableCullFace/EnableDepthMask/DisableDepthMask (ZzzOpenglUtil.cpp)
// have real call sites outside the blend-mode wrapper family (world/UI code that toggles cull
// or depth-write without changing blend mode) -- SetBlendMode alone can't back those, since it
// always sets all three (blend+cull+depthwrite) as one combo.
void SetCullEnabled(bool);
void SetDepthWriteEnabled(bool);

void SetFogEnabled(bool);
void SetPolygonOffset(bool enabled, float factor = -1.f, float units = -1.f); // PlanarShadowShader's one caller

// ---- Shaders ----
// The 5 existing shader classes KEEP their bespoke Bind(...) signatures (design doc Q1,
// decided: BMDMeshShader::Bind()'s 16-param signature stays as-is for the first RHI pass).
// RHI owns only the GL plumbing inside each class's .cpp, not their public surface.

// ---- Vertex layout + binding (design doc Q6, decided) ----
// Layout is paired with "which shader consumes this" up front rather than being a
// free-floating buffer property. Exactly 4 concrete layouts exist in the tree today.
// Implementation status: only PosUvColor is implemented in RHI_GL (IR::'s format, this
// increment). BMDMesh/Terrain/PosOnly land with their own subsystem-conversion increments
// (Approach step 3) -- calling BindVertexBuffer with one of those before then is a no-op
// logged to MuError.log, not a silent wrong-format bind.
enum class VertexLayout {
    PosUvColor,   // IR:: / PassthroughShader -- pos3+uv2+rgba4, ImmediateRenderer.cpp -- IMPLEMENTED
    BMDMesh,      // BMDMeshShader's mesh vertex format
    Terrain,      // TerrainShader's mesh vertex format
    PosOnly,      // PlanarShadowShader -- position-only
};
void BindVertexBuffer(BufferHandle, VertexLayout);
void BindIndexBuffer(BufferHandle);   // GL_UNSIGNED_INT only -- GLP-16: implemented, binds GL_ELEMENT_ARRAY_BUFFER (captured into the currently-bound VAO's state)

// DXP-14: D3D11's ID3D11InputLayout is validated against a specific vertex shader's input
// signature at creation time -- GL has no equivalent (attrib pointers bind directly to a VBO
// in BindVertexBuffer, no shader involved), so RHI_GL no-ops this. Shader classes call it once,
// right after compiling the vertex shader that owns `layout`, before any BindVertexBuffer(...,
// layout) call for it. PassthroughShader/PosUvColor is the first caller (DXP-14 increment 1).
void RegisterVertexShaderBytecode(VertexLayout layout, const void* bytecode, size_t bytecodeSize);

// ---- Backend-native device access (D3D11 only) ----
// The 5 shader classes own their own GL-vs-D3D11 plumbing inside each class's .cpp (see the
// Shaders section above) -- for GL that means SDL_GL_GetProcAddress as today; for D3D11 it
// means compiling HLSL and creating ID3D11VertexShader/PixelShader objects (needs the device)
// and binding them each Bind() call (needs the context). Returned as void* so this header
// stays platform-generic (no <d3d11.h> include here) -- callers cast to ID3D11Device*/
// ID3D11DeviceContext* under their own #ifdef _WIN32 block. Both return nullptr under
// Backend=GL or off Windows; callers must guard on g_RenderBackend first.
void* GetD3D11Device();
void* GetD3D11DeviceContext();

// DXP-21 phase 3b: registers an externally-created D3D11 buffer (e.g. ClothComputeShader's Pass E
// output -- a compute-shader UAV target also flagged D3D11_BIND_VERTEX_BUFFER at creation) into
// RHI's own buffer table, so BindVertexBuffer/DestroyBuffer can address it exactly like any
// RHI::CreateVertexBuffer-made buffer. AddRefs the buffer once on registration (COM refcounting)
// so the returned handle's eventual RHI::DestroyBuffer and the caller's own Release() are two
// independent, order-independent releases -- caller keeps owning/creating/destroying the
// underlying buffer exactly as before, this only gives RHI a second reference to track. D3D11
// only (no GL equivalent -- GL has no compute-writable vertex buffer on this codepath); the
// pointer is void* for the same reason GetD3D11Device() is -- callers reinterpret under their own
// #ifdef _WIN32/RHI_D3D11_AVAILABLE block. Returns an invalid handle under Backend=GL.
BufferHandle RegisterExternalD3D11VertexBuffer(void* d3d11Buffer, size_t capacityBytes);

// ---- Draw ----
// No strips/fans/points anywhere post-DXP-09; no instancing anywhere in the tree.
enum class Topology { TriangleList, LineList, LineStrip };
void Draw(Topology, uint32_t vertexCount, uint32_t firstVertex = 0);
void DrawIndexed(Topology, uint32_t indexCount, uint32_t firstIndex = 0); // GLP-16: implemented. firstIndex is an element offset, not bytes; requires a prior BindIndexBuffer call, same two-call shape as BindVertexBuffer+Draw

// ---- Readback (DXP-12) ----
bool ReadDepthPixel(int x, int y, float* outDepth);                   // CameraProjection::TestDepthBuffer, until DXP-16
// outRGB: 3 bytes/pixel (matches Winmain.cpp/SceneManager.cpp's existing PPM writer -- no RGBA
// readback caller exists). Row order is TOP-DOWN by contract (matches D3D and image-file
// convention) regardless of backend -- GL's native glReadPixels is bottom-up, so RHI_GL flips
// rows before returning. Any caller relying on this row order (screenshots, and
// ReconnectDialog::CaptureBackground's planned ReadColorFramebuffer+CreateTexture composition,
// DXP-12) gets consistent top-down data on both backends without its own flip logic.
bool ReadColorFramebuffer(int x, int y, int w, int h, void* outRGB);   // screenshot / MU_CAPTURE_FRAME debug capture / CaptureBackground compose source

} // namespace RHI
