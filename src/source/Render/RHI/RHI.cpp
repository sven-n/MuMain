// DXP-13 -- RHI dispatch shim: this file owns the actual `namespace RHI { ... }` that
// satisfies Render/RHI/RHI.h's declarations. Each function forwards to exactly one backend
// implementation, chosen once via g_RenderBackend (read at startup by InitRenderConfig(),
// before either backend's device exists -- same timing guarantee as g_CoreProfile).

#include "stdafx.h"
#include "Render/RHI/RHI.h"
#include "Render/Core/RenderConfig.h"

namespace RHI_GL_Impl {
    const RHI::Caps& GetCaps();

    bool Init(void* nativeWindowHandle, int width, int height);
    void Shutdown();
    void BeginFrame();
    void EndFrame();
    void SetViewport(int x, int y, int w, int h);
    void OnResize(int width, int height);
    void Clear(bool color, bool depth, float r, float g, float b, float a);

    RHI::BufferHandle CreateVertexBuffer(const void* initialData, size_t sizeBytes, RHI::BufferUsage usage);
    RHI::BufferHandle CreateIndexBuffer(const void* initialData, size_t sizeBytes, RHI::BufferUsage usage);
    void UpdateBuffer(RHI::BufferHandle handle, const void* data, size_t sizeBytes);
    size_t AppendBuffer(RHI::BufferHandle handle, const void* data, size_t sizeBytes);
    void DestroyBuffer(RHI::BufferHandle handle);

    RHI::BufferHandle CreateUniformBlock(size_t sizeBytes, int bindingSlot);
    void UpdateUniformBlock(RHI::BufferHandle handle, const void* data, size_t sizeBytes);
    void DestroyUniformBlock(RHI::BufferHandle handle);

    RHI::TextureHandle CreateTexture(const RHI::TextureDesc& desc, const void* initialPixelsRGBA);
    void UpdateTexture(RHI::TextureHandle handle, int x, int y, int w, int h, const void* pixelsRGBA);
    void DestroyTexture(RHI::TextureHandle handle);
    void BindTexture(RHI::TextureHandle handle, int slot);

    void SetBlendMode(RHI::BlendMode mode);
    void SetDepthTestEnabled(bool enabled);
    void SetCullEnabled(bool enabled);
    void SetDepthWriteEnabled(bool enabled);
    void SetFogEnabled(bool enabled);
    void SetPolygonOffset(bool enabled, float factor, float units);

    void BindVertexBuffer(RHI::BufferHandle handle, RHI::VertexLayout layout);
    void BindIndexBuffer(RHI::BufferHandle handle);

    void Draw(RHI::Topology topology, uint32_t vertexCount, uint32_t firstVertex);
    void DrawIndexed(RHI::Topology topology, uint32_t indexCount, uint32_t firstIndex);

    bool ReadDepthPixel(int x, int y, float* outDepth);
    bool ReadColorFramebuffer(int x, int y, int w, int h, void* outRGB);

    void RegisterVertexShaderBytecode(RHI::VertexLayout layout, const void* bytecode, size_t bytecodeSize);
    void* GetD3D11Device();
    void* GetD3D11DeviceContext();
    RHI::BufferHandle RegisterExternalD3D11VertexBuffer(void* d3d11Buffer, size_t capacityBytes);
}

namespace RHI_D3D11_Impl {
    bool Init(void* nativeWindowHandle, int width, int height);
    void Shutdown();
    void BeginFrame();
    void EndFrame();
    void SetViewport(int x, int y, int w, int h);
    void OnResize(int width, int height);
    void Clear(bool color, bool depth, float r, float g, float b, float a);

    RHI::BufferHandle CreateVertexBuffer(const void* initialData, size_t sizeBytes, RHI::BufferUsage usage);
    RHI::BufferHandle CreateIndexBuffer(const void* initialData, size_t sizeBytes, RHI::BufferUsage usage);
    void UpdateBuffer(RHI::BufferHandle handle, const void* data, size_t sizeBytes);
    size_t AppendBuffer(RHI::BufferHandle handle, const void* data, size_t sizeBytes);
    void DestroyBuffer(RHI::BufferHandle handle);

    RHI::BufferHandle CreateUniformBlock(size_t sizeBytes, int bindingSlot);
    void UpdateUniformBlock(RHI::BufferHandle handle, const void* data, size_t sizeBytes);
    void DestroyUniformBlock(RHI::BufferHandle handle);

    RHI::TextureHandle CreateTexture(const RHI::TextureDesc& desc, const void* initialPixelsRGBA);
    void UpdateTexture(RHI::TextureHandle handle, int x, int y, int w, int h, const void* pixelsRGBA);
    void DestroyTexture(RHI::TextureHandle handle);
    void BindTexture(RHI::TextureHandle handle, int slot);

    void SetBlendMode(RHI::BlendMode mode);
    void SetDepthTestEnabled(bool enabled);
    void SetCullEnabled(bool enabled);
    void SetDepthWriteEnabled(bool enabled);
    void SetFogEnabled(bool enabled);
    void SetPolygonOffset(bool enabled, float factor, float units);

    void BindVertexBuffer(RHI::BufferHandle handle, RHI::VertexLayout layout);
    void BindIndexBuffer(RHI::BufferHandle handle);

    void Draw(RHI::Topology topology, uint32_t vertexCount, uint32_t firstVertex);
    void DrawIndexed(RHI::Topology topology, uint32_t indexCount, uint32_t firstIndex);

    bool ReadDepthPixel(int x, int y, float* outDepth);
    bool ReadColorFramebuffer(int x, int y, int w, int h, void* outRGB);

    void RegisterVertexShaderBytecode(RHI::VertexLayout layout, const void* bytecode, size_t bytecodeSize);
    void* GetD3D11Device();
    void* GetD3D11DeviceContext();
    RHI::BufferHandle RegisterExternalD3D11VertexBuffer(void* d3d11Buffer, size_t capacityBytes);
}

namespace {
    inline bool UsingD3D11() { return g_RenderBackend == RenderBackend::D3D11; }
}

namespace RHI {

const Caps& GetCaps()
{
    return RHI_GL_Impl::GetCaps();
}

bool Init(void* nativeWindowHandle, int width, int height)
{
    return UsingD3D11() ? RHI_D3D11_Impl::Init(nativeWindowHandle, width, height)
                         : RHI_GL_Impl::Init(nativeWindowHandle, width, height);
}

void Shutdown()
{
    UsingD3D11() ? RHI_D3D11_Impl::Shutdown() : RHI_GL_Impl::Shutdown();
}

void BeginFrame()
{
    UsingD3D11() ? RHI_D3D11_Impl::BeginFrame() : RHI_GL_Impl::BeginFrame();
}

void EndFrame()
{
    UsingD3D11() ? RHI_D3D11_Impl::EndFrame() : RHI_GL_Impl::EndFrame();
}

void SetViewport(int x, int y, int w, int h)
{
    UsingD3D11() ? RHI_D3D11_Impl::SetViewport(x, y, w, h) : RHI_GL_Impl::SetViewport(x, y, w, h);
}

void OnResize(int width, int height)
{
    UsingD3D11() ? RHI_D3D11_Impl::OnResize(width, height) : RHI_GL_Impl::OnResize(width, height);
}

void Clear(bool color, bool depth, float r, float g, float b, float a)
{
    UsingD3D11() ? RHI_D3D11_Impl::Clear(color, depth, r, g, b, a) : RHI_GL_Impl::Clear(color, depth, r, g, b, a);
}

BufferHandle CreateVertexBuffer(const void* initialData, size_t sizeBytes, BufferUsage usage)
{
    return UsingD3D11() ? RHI_D3D11_Impl::CreateVertexBuffer(initialData, sizeBytes, usage)
                        : RHI_GL_Impl::CreateVertexBuffer(initialData, sizeBytes, usage);
}

BufferHandle CreateIndexBuffer(const void* initialData, size_t sizeBytes, BufferUsage usage)
{
    return UsingD3D11() ? RHI_D3D11_Impl::CreateIndexBuffer(initialData, sizeBytes, usage)
                        : RHI_GL_Impl::CreateIndexBuffer(initialData, sizeBytes, usage);
}

void UpdateBuffer(BufferHandle handle, const void* data, size_t sizeBytes)
{
    UsingD3D11() ? RHI_D3D11_Impl::UpdateBuffer(handle, data, sizeBytes) : RHI_GL_Impl::UpdateBuffer(handle, data, sizeBytes);
}

size_t AppendBuffer(BufferHandle handle, const void* data, size_t sizeBytes)
{
    return UsingD3D11() ? RHI_D3D11_Impl::AppendBuffer(handle, data, sizeBytes)
                        : RHI_GL_Impl::AppendBuffer(handle, data, sizeBytes);
}

void DestroyBuffer(BufferHandle handle)
{
    UsingD3D11() ? RHI_D3D11_Impl::DestroyBuffer(handle) : RHI_GL_Impl::DestroyBuffer(handle);
}

BufferHandle CreateUniformBlock(size_t sizeBytes, int bindingSlot)
{
    return UsingD3D11() ? RHI_D3D11_Impl::CreateUniformBlock(sizeBytes, bindingSlot)
                        : RHI_GL_Impl::CreateUniformBlock(sizeBytes, bindingSlot);
}

void UpdateUniformBlock(BufferHandle handle, const void* data, size_t sizeBytes)
{
    UsingD3D11() ? RHI_D3D11_Impl::UpdateUniformBlock(handle, data, sizeBytes) : RHI_GL_Impl::UpdateUniformBlock(handle, data, sizeBytes);
}

void DestroyUniformBlock(BufferHandle handle)
{
    UsingD3D11() ? RHI_D3D11_Impl::DestroyUniformBlock(handle) : RHI_GL_Impl::DestroyUniformBlock(handle);
}

TextureHandle CreateTexture(const TextureDesc& desc, const void* initialPixelsRGBA)
{
    return UsingD3D11() ? RHI_D3D11_Impl::CreateTexture(desc, initialPixelsRGBA)
                        : RHI_GL_Impl::CreateTexture(desc, initialPixelsRGBA);
}

void UpdateTexture(TextureHandle handle, int x, int y, int w, int h, const void* pixelsRGBA)
{
    UsingD3D11() ? RHI_D3D11_Impl::UpdateTexture(handle, x, y, w, h, pixelsRGBA)
                 : RHI_GL_Impl::UpdateTexture(handle, x, y, w, h, pixelsRGBA);
}

void DestroyTexture(TextureHandle handle)
{
    UsingD3D11() ? RHI_D3D11_Impl::DestroyTexture(handle) : RHI_GL_Impl::DestroyTexture(handle);
}

void BindTexture(TextureHandle handle, int slot)
{
    UsingD3D11() ? RHI_D3D11_Impl::BindTexture(handle, slot) : RHI_GL_Impl::BindTexture(handle, slot);
}

void SetBlendMode(BlendMode mode)
{
    UsingD3D11() ? RHI_D3D11_Impl::SetBlendMode(mode) : RHI_GL_Impl::SetBlendMode(mode);
}

void SetDepthTestEnabled(bool enabled)
{
    UsingD3D11() ? RHI_D3D11_Impl::SetDepthTestEnabled(enabled) : RHI_GL_Impl::SetDepthTestEnabled(enabled);
}

void SetCullEnabled(bool enabled)
{
    UsingD3D11() ? RHI_D3D11_Impl::SetCullEnabled(enabled) : RHI_GL_Impl::SetCullEnabled(enabled);
}

void SetDepthWriteEnabled(bool enabled)
{
    UsingD3D11() ? RHI_D3D11_Impl::SetDepthWriteEnabled(enabled) : RHI_GL_Impl::SetDepthWriteEnabled(enabled);
}

void SetFogEnabled(bool enabled)
{
    UsingD3D11() ? RHI_D3D11_Impl::SetFogEnabled(enabled) : RHI_GL_Impl::SetFogEnabled(enabled);
}

void SetPolygonOffset(bool enabled, float factor, float units)
{
    UsingD3D11() ? RHI_D3D11_Impl::SetPolygonOffset(enabled, factor, units) : RHI_GL_Impl::SetPolygonOffset(enabled, factor, units);
}

void BindVertexBuffer(BufferHandle handle, VertexLayout layout)
{
    UsingD3D11() ? RHI_D3D11_Impl::BindVertexBuffer(handle, layout) : RHI_GL_Impl::BindVertexBuffer(handle, layout);
}

void BindIndexBuffer(BufferHandle handle)
{
    UsingD3D11() ? RHI_D3D11_Impl::BindIndexBuffer(handle) : RHI_GL_Impl::BindIndexBuffer(handle);
}

void Draw(Topology topology, uint32_t vertexCount, uint32_t firstVertex)
{
    UsingD3D11() ? RHI_D3D11_Impl::Draw(topology, vertexCount, firstVertex) : RHI_GL_Impl::Draw(topology, vertexCount, firstVertex);
}

void DrawIndexed(Topology topology, uint32_t indexCount, uint32_t firstIndex)
{
    UsingD3D11() ? RHI_D3D11_Impl::DrawIndexed(topology, indexCount, firstIndex) : RHI_GL_Impl::DrawIndexed(topology, indexCount, firstIndex);
}

bool ReadDepthPixel(int x, int y, float* outDepth)
{
    return UsingD3D11() ? RHI_D3D11_Impl::ReadDepthPixel(x, y, outDepth)
                        : RHI_GL_Impl::ReadDepthPixel(x, y, outDepth);
}

bool ReadColorFramebuffer(int x, int y, int w, int h, void* outRGB)
{
    return UsingD3D11() ? RHI_D3D11_Impl::ReadColorFramebuffer(x, y, w, h, outRGB)
                        : RHI_GL_Impl::ReadColorFramebuffer(x, y, w, h, outRGB);
}

void RegisterVertexShaderBytecode(VertexLayout layout, const void* bytecode, size_t bytecodeSize)
{
    UsingD3D11() ? RHI_D3D11_Impl::RegisterVertexShaderBytecode(layout, bytecode, bytecodeSize)
                 : RHI_GL_Impl::RegisterVertexShaderBytecode(layout, bytecode, bytecodeSize);
}

void* GetD3D11Device()
{
    return UsingD3D11() ? RHI_D3D11_Impl::GetD3D11Device() : RHI_GL_Impl::GetD3D11Device();
}

void* GetD3D11DeviceContext()
{
    return UsingD3D11() ? RHI_D3D11_Impl::GetD3D11DeviceContext() : RHI_GL_Impl::GetD3D11DeviceContext();
}

BufferHandle RegisterExternalD3D11VertexBuffer(void* d3d11Buffer, size_t capacityBytes)
{
    return UsingD3D11() ? RHI_D3D11_Impl::RegisterExternalD3D11VertexBuffer(d3d11Buffer, capacityBytes)
                         : RHI_GL_Impl::RegisterExternalD3D11VertexBuffer(d3d11Buffer, capacityBytes);
}

} // namespace RHI
