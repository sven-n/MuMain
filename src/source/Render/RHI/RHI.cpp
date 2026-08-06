// RHI dispatch shim: this file owns the actual `namespace RHI { ... }` that satisfies
// Render/RHI/RHI.h's declarations. Each function forwards to the GL backend implementation
// in RHI_GL.cpp -- the only backend today.

#include "stdafx.h"
#include "Render/RHI/RHI.h"
#include "Render/Core/RenderConfig.h"

namespace RHI_GL_Impl {
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
}

namespace RHI {

bool Init(void* nativeWindowHandle, int width, int height)
{
    return RHI_GL_Impl::Init(nativeWindowHandle, width, height);
}

void Shutdown()
{
    RHI_GL_Impl::Shutdown();
}

void BeginFrame()
{
    RHI_GL_Impl::BeginFrame();
}

void EndFrame()
{
    RHI_GL_Impl::EndFrame();
}

void SetViewport(int x, int y, int w, int h)
{
    RHI_GL_Impl::SetViewport(x, y, w, h);
}

void OnResize(int width, int height)
{
    RHI_GL_Impl::OnResize(width, height);
}

void Clear(bool color, bool depth, float r, float g, float b, float a)
{
    RHI_GL_Impl::Clear(color, depth, r, g, b, a);
}

BufferHandle CreateVertexBuffer(const void* initialData, size_t sizeBytes, BufferUsage usage)
{
    return RHI_GL_Impl::CreateVertexBuffer(initialData, sizeBytes, usage);
}

BufferHandle CreateIndexBuffer(const void* initialData, size_t sizeBytes, BufferUsage usage)
{
    return RHI_GL_Impl::CreateIndexBuffer(initialData, sizeBytes, usage);
}

void UpdateBuffer(BufferHandle handle, const void* data, size_t sizeBytes)
{
    RHI_GL_Impl::UpdateBuffer(handle, data, sizeBytes);
}

size_t AppendBuffer(BufferHandle handle, const void* data, size_t sizeBytes)
{
    return RHI_GL_Impl::AppendBuffer(handle, data, sizeBytes);
}

void DestroyBuffer(BufferHandle handle)
{
    RHI_GL_Impl::DestroyBuffer(handle);
}

BufferHandle CreateUniformBlock(size_t sizeBytes, int bindingSlot)
{
    return RHI_GL_Impl::CreateUniformBlock(sizeBytes, bindingSlot);
}

void UpdateUniformBlock(BufferHandle handle, const void* data, size_t sizeBytes)
{
    RHI_GL_Impl::UpdateUniformBlock(handle, data, sizeBytes);
}

void DestroyUniformBlock(BufferHandle handle)
{
    RHI_GL_Impl::DestroyUniformBlock(handle);
}

TextureHandle CreateTexture(const TextureDesc& desc, const void* initialPixelsRGBA)
{
    return RHI_GL_Impl::CreateTexture(desc, initialPixelsRGBA);
}

void UpdateTexture(TextureHandle handle, int x, int y, int w, int h, const void* pixelsRGBA)
{
    RHI_GL_Impl::UpdateTexture(handle, x, y, w, h, pixelsRGBA);
}

void DestroyTexture(TextureHandle handle)
{
    RHI_GL_Impl::DestroyTexture(handle);
}

void BindTexture(TextureHandle handle, int slot)
{
    RHI_GL_Impl::BindTexture(handle, slot);
}

void SetBlendMode(BlendMode mode)
{
    RHI_GL_Impl::SetBlendMode(mode);
}

void SetDepthTestEnabled(bool enabled)
{
    RHI_GL_Impl::SetDepthTestEnabled(enabled);
}

void SetCullEnabled(bool enabled)
{
    RHI_GL_Impl::SetCullEnabled(enabled);
}

void SetDepthWriteEnabled(bool enabled)
{
    RHI_GL_Impl::SetDepthWriteEnabled(enabled);
}

void SetFogEnabled(bool enabled)
{
    RHI_GL_Impl::SetFogEnabled(enabled);
}

void SetPolygonOffset(bool enabled, float factor, float units)
{
    RHI_GL_Impl::SetPolygonOffset(enabled, factor, units);
}

void BindVertexBuffer(BufferHandle handle, VertexLayout layout)
{
    RHI_GL_Impl::BindVertexBuffer(handle, layout);
}

void BindIndexBuffer(BufferHandle handle)
{
    RHI_GL_Impl::BindIndexBuffer(handle);
}

void Draw(Topology topology, uint32_t vertexCount, uint32_t firstVertex)
{
    RHI_GL_Impl::Draw(topology, vertexCount, firstVertex);
}

void DrawIndexed(Topology topology, uint32_t indexCount, uint32_t firstIndex)
{
    RHI_GL_Impl::DrawIndexed(topology, indexCount, firstIndex);
}

bool ReadDepthPixel(int x, int y, float* outDepth)
{
    return RHI_GL_Impl::ReadDepthPixel(x, y, outDepth);
}

bool ReadColorFramebuffer(int x, int y, int w, int h, void* outRGB)
{
    return RHI_GL_Impl::ReadColorFramebuffer(x, y, w, h, outRGB);
}

} // namespace RHI
