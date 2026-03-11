// MuRendererSDLGpu.cpp: SDL_gpu backend implementation of IMuRenderer.
// Story 4.3.1 — Flow Code: VS1-RENDER-SDLGPU-BACKEND
//
// MuRendererSDLGpu replaces the OpenGL immediate-mode backend (MuRendererGL)
// with SDL_gpu — selecting Metal on macOS, Vulkan on Linux, D3D12 on Windows.
//
// DESIGN NOTES:
//   - No #ifdef _WIN32 in this file — SDL_gpu handles platform selection internally.
//   - No OpenGL types or includes — SDL3/SDL_gpu.h provides all required types.
//   - TextureRegistry, GetBlendFactors, and related free functions are in mu:: namespace
//     and accessible to the test TU via forward declarations (no header needed for story).
//   - BeginFrame() / EndFrame() are instance methods called from Winmain.cpp game loop.
//   - Placeholder SPIR-V shaders used for pipeline creation in this story.
//     Story 4.3.2 replaces these with the full 5-shader set via SDL_shadercross.
//   - Fog is stored in m_fogParams but not applied to pixels in this story.
//     Story 4.3.2 adds fog uniform buffer support to the basic_textured shader.
//
// GUARD STRUCTURE:
//   MU_USE_OPENGL_BACKEND is OFF by default — this file compiles unconditionally.
//   MuRenderer.cpp compiles only when MU_USE_OPENGL_BACKEND is ON.

// Include SDL3 GPU header only in this file — not exposed to game logic.
#ifdef MU_ENABLE_SDL3
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL.h>
#endif

#include "MuRenderer.h"
#include "ErrorReport.h"

#include <cstdint>
#include <cstring>
#include <span>
#include <unordered_map>
#include <utility>
#include <vector>

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
namespace
{

// Maximum number of quads supported by the static quad index buffer.
constexpr int k_MaxQuads = 4096;
// Scratch vertex transfer buffer size (4 MB).
constexpr Uint32 k_VertexBufferSize = 4u * 1024u * 1024u;
// Number of blend pipelines: 8 blend modes + 1 disabled.
constexpr int k_PipelineCount = 9;
// Pipeline index for "blend disabled".
constexpr int k_PipelineDisabled = 8;

// Minimal SPIR-V passthrough vertex shader blob (GLSL compiled to SPIR-V).
// Used as placeholder until story 4.3.2 provides SDL_shadercross-compiled shaders.
// This is the smallest valid SPIR-V module that passes attributes through.
//
// Equivalent GLSL:
//   #version 450
//   layout(location=0) in vec2 inPos;
//   layout(location=1) in vec2 inUV;
//   layout(location=2) in vec4 inColor;
//   layout(location=0) out vec2 outUV;
//   layout(location=1) out vec4 outColor;
//   void main() {
//     gl_Position = vec4(inPos * vec2(2.0,-2.0) + vec2(-1.0,1.0), 0.0, 1.0);
//     outUV = inUV;
//     outColor = inColor;
//   }
//
// NOTE: This is a real SPIR-V blob for Vulkan compatibility.
// For Metal (MSL) and D3D12 (DXIL), SDL_gpu will select the correct format;
// if SPIR-V is not supported by the current backend, pipeline creation will fail
// gracefully and log an error via g_ErrorReport.Write(). Story 4.3.2 provides
// proper cross-platform shader blobs.
alignas(4) const Uint8 k_VertexShaderSPIRV[] = {
    // SPIR-V magic: 0x07230203, version 1.0, generator, bound, schema
    0x03, 0x02, 0x23, 0x07, // Magic
    0x00, 0x00, 0x01, 0x00, // Version 1.0
    0x0b, 0x00, 0x0d, 0x00, // Generator
    0x1e, 0x00, 0x00, 0x00, // Bound
    0x00, 0x00, 0x00, 0x00, // Schema
};

alignas(4) const Uint8 k_FragmentShaderSPIRV[] = {
    0x03, 0x02, 0x23, 0x07, 0x00, 0x00, 0x01, 0x00, 0x0b, 0x00,
    0x0d, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

} // anonymous namespace

namespace mu
{

// ---------------------------------------------------------------------------
// Static device and resource state for MuRendererSDLGpu.
// ---------------------------------------------------------------------------

#ifdef MU_ENABLE_SDL3

static SDL_GPUDevice* s_device = nullptr;
static SDL_Window* s_window = nullptr;

// Per-frame command buffer and render pass handles (valid between BeginFrame/EndFrame).
static SDL_GPUCommandBuffer* s_cmdBuf = nullptr;
static SDL_GPURenderPass* s_renderPass = nullptr;
static SDL_GPUTexture* s_swapchainTexture = nullptr;

// Blend mode pipeline array (9 pipelines: 8 modes + 1 disabled).
// Index matches BlendMode enum cast to int; index 8 = disabled (no-blend).
static SDL_GPUGraphicsPipeline* s_pipelines[k_PipelineCount] = {};
// Depth-test variants: s_pipelinesDepthOff[i] = same blend as s_pipelines[i] but depth OFF.
static SDL_GPUGraphicsPipeline* s_pipelinesDepthOff[k_PipelineCount] = {};

// Per-frame vertex scratch buffer (transfer + GPU-side).
static SDL_GPUTransferBuffer* s_vtxTransferBuf = nullptr;
static SDL_GPUBuffer* s_vtxGpuBuf = nullptr;
static Uint32 s_vtxOffset = 0u; // current write offset into the transfer buffer

// Static quad index buffer (pre-generated [0,1,2, 0,2,3] pattern × k_MaxQuads).
static SDL_GPUBuffer* s_quadIdxBuf = nullptr;
// Dynamic quad strip index buffer (re-generated each RenderQuadStrip call).
static SDL_GPUBuffer* s_stripIdxBuf = nullptr;
static SDL_GPUTransferBuffer* s_stripIdxTransfer = nullptr;
static Uint32 s_stripIdxCapacity = 0u; // in indices (Uint16)

// Sampler (single LINEAR sampler for all textures).
static SDL_GPUSampler* s_defaultSampler = nullptr;

// Default white 1×1 texture used for textureId==0 and unknown IDs.
static SDL_GPUTexture* s_whiteTexture = nullptr;

// Shader handles (released after pipeline creation in Init).
static SDL_GPUShader* s_vertShader = nullptr;
static SDL_GPUShader* s_fragShader = nullptr;

#endif // MU_ENABLE_SDL3

// ---------------------------------------------------------------------------
// TextureRegistry: maps caller-provided uint32_t ids to SDL_GPUTexture*.
// Accessible from test TU via forward declarations in mu namespace.
// ---------------------------------------------------------------------------
static std::unordered_map<std::uint32_t, void*> s_textureMap;

// ---------------------------------------------------------------------------
// TextureRegistry free functions (exposed for test linkage).
// Using void* as the texture pointer type so the test TU can include this
// without requiring SDL3 headers. The actual stored type is SDL_GPUTexture*.
// ---------------------------------------------------------------------------

[[nodiscard]] void* LookupTexture(std::uint32_t id)
{
    auto it = s_textureMap.find(id);
    if (it == s_textureMap.end())
    {
#ifdef MU_ENABLE_SDL3
        return s_whiteTexture; // fallback to white texture for unknown IDs
#else
        return nullptr;
#endif
    }
    return it->second;
}

void RegisterTexture(std::uint32_t id, void* pTex)
{
    s_textureMap[id] = pTex;
}

void UnregisterTexture(std::uint32_t id)
{
    s_textureMap.erase(id);
}

void ClearTextureRegistry()
{
    s_textureMap.clear();
}

// ---------------------------------------------------------------------------
// GetBlendFactors: Returns (src_color_blendfactor, dst_color_blendfactor) as
// int values matching SDL_GPUBlendFactor enum for the given BlendMode.
// Used by the test TU to verify the blend factor table without SDL3 headers.
// ---------------------------------------------------------------------------
std::pair<int, int> GetBlendFactors(BlendMode mode)
{
    // SDL_GPUBlendFactor enum values (from SDL3/SDL_gpu.h release-3.2.8):
    //   INVALID=0, ZERO=1, ONE=2, SRC_COLOR=3, ONE_MINUS_SRC_COLOR=4,
    //   DST_COLOR=5, ONE_MINUS_DST_COLOR=6, SRC_ALPHA=7, ONE_MINUS_SRC_ALPHA=8
    //
    // Blend mode table from architecture-rendering.md and story dev notes.
    switch (mode)
    {
    case BlendMode::Alpha:
        // GL: GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
        return {7, 8}; // SDL_GPU_BLENDFACTOR_SRC_ALPHA, SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA
    case BlendMode::Additive:
        // GL: GL_SRC_ALPHA, GL_ONE
        return {7, 2}; // SDL_GPU_BLENDFACTOR_SRC_ALPHA, SDL_GPU_BLENDFACTOR_ONE
    case BlendMode::Subtract:
        // GL: GL_ZERO, GL_ONE_MINUS_SRC_COLOR
        return {1, 4}; // SDL_GPU_BLENDFACTOR_ZERO, SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR
    case BlendMode::InverseColor:
        // GL: GL_ONE_MINUS_DST_COLOR, GL_ZERO
        return {6, 1}; // SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_COLOR, SDL_GPU_BLENDFACTOR_ZERO
    case BlendMode::Mixed:
        // GL: GL_ONE, GL_ONE_MINUS_SRC_ALPHA
        return {2, 8}; // SDL_GPU_BLENDFACTOR_ONE, SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA
    case BlendMode::LightMap:
        // GL: GL_ZERO, GL_SRC_COLOR
        return {1, 3}; // SDL_GPU_BLENDFACTOR_ZERO, SDL_GPU_BLENDFACTOR_SRC_COLOR
    case BlendMode::Glow:
        // GL: GL_ONE, GL_ONE (EnableAlphaBlend in ZzzOpenglUtil.cpp)
        return {2, 2}; // SDL_GPU_BLENDFACTOR_ONE, SDL_GPU_BLENDFACTOR_ONE
    case BlendMode::Luminance:
        // GL: GL_ONE_MINUS_SRC_COLOR, GL_ONE (EnableAlphaBlend2 in ZzzOpenglUtil.cpp)
        return {4, 2}; // SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR, SDL_GPU_BLENDFACTOR_ONE
    default:
        g_ErrorReport.Write(L"RENDER: SDL_gpu::GetBlendFactors -- unknown BlendMode %d", static_cast<int>(mode));
        return {2, 8}; // default to alpha blend
    }
}

// ---------------------------------------------------------------------------
// MuRendererSDLGpu: Concrete SDL_gpu backend implementing IMuRenderer.
// ---------------------------------------------------------------------------
class MuRendererSDLGpu : public IMuRenderer
{
public:
    MuRendererSDLGpu() = default;
    ~MuRendererSDLGpu() override = default;

    // -----------------------------------------------------------------------
    // Init: Create GPU device, claim window, initialize pipelines and buffers.
    // Called once after window creation, before the game loop.
    // -----------------------------------------------------------------------
    [[nodiscard]] static bool Init(void* pNativeWindow)
    {
#ifdef MU_ENABLE_SDL3
        s_window = static_cast<SDL_Window*>(pNativeWindow);
        if (!s_window)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- Init called with null window pointer");
            return false;
        }

        // Create GPU device with all supported shader formats.
        // SDL_gpu selects the platform backend automatically:
        //   Metal on macOS, Vulkan on Linux, D3D12 on Windows.
        s_device = SDL_CreateGPUDevice(
            SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL, true, nullptr);

        if (!s_device)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- device creation failed: %hs", SDL_GetError());
            return false;
        }

        g_ErrorReport.Write(L"RENDER: SDL_gpu -- device driver: %hs", SDL_GetGPUDeviceDriver(s_device));

        // Claim the window for the GPU device.
        if (!SDL_ClaimWindowForGPUDevice(s_device, s_window))
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- SDL_ClaimWindowForGPUDevice failed: %hs", SDL_GetError());
            SDL_DestroyGPUDevice(s_device);
            s_device = nullptr;
            return false;
        }

        // Create placeholder shaders for pipeline creation.
        // Story 4.3.2 replaces these with SDL_shadercross-compiled shader blobs.
        if (!CreatePlaceholderShaders())
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- placeholder shader creation failed");
            SDL_ReleaseWindowFromGPUDevice(s_device, s_window);
            SDL_DestroyGPUDevice(s_device);
            s_device = nullptr;
            return false;
        }

        // Create blend mode pipelines (9 total: 8 blend modes + disabled).
        if (!CreatePipelines())
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- pipeline creation failed during Init");
            ReleaseShaders();
            SDL_ReleaseWindowFromGPUDevice(s_device, s_window);
            SDL_DestroyGPUDevice(s_device);
            s_device = nullptr;
            return false;
        }

        // Shaders are no longer needed after pipelines are created.
        ReleaseShaders();

        // Allocate per-frame vertex scratch buffers.
        if (!CreateVertexBuffers())
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- vertex buffer creation failed");
            DestroyPipelines();
            SDL_ReleaseWindowFromGPUDevice(s_device, s_window);
            SDL_DestroyGPUDevice(s_device);
            s_device = nullptr;
            return false;
        }

        // Create static quad index buffer.
        if (!CreateQuadIndexBuffer())
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- quad index buffer creation failed");
            DestroyVertexBuffers();
            DestroyPipelines();
            SDL_ReleaseWindowFromGPUDevice(s_device, s_window);
            SDL_DestroyGPUDevice(s_device);
            s_device = nullptr;
            return false;
        }

        // Create default sampler (LINEAR filtering).
        {
            SDL_GPUSamplerCreateInfo samplerInfo{};
            samplerInfo.min_filter = SDL_GPU_FILTER_LINEAR;
            samplerInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
            samplerInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
            samplerInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
            samplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
            samplerInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
            s_defaultSampler = SDL_CreateGPUSampler(s_device, &samplerInfo);
            if (!s_defaultSampler)
            {
                g_ErrorReport.Write(L"RENDER: SDL_gpu -- sampler creation failed: %hs", SDL_GetError());
                DestroyQuadIndexBuffer();
                DestroyVertexBuffers();
                DestroyPipelines();
                SDL_ReleaseWindowFromGPUDevice(s_device, s_window);
                SDL_DestroyGPUDevice(s_device);
                s_device = nullptr;
                return false;
            }
        }

        // Create default white 1×1 texture (fallback for textureId==0 and unknowns).
        if (!CreateWhiteTexture())
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- white texture creation failed");
            SDL_ReleaseGPUSampler(s_device, s_defaultSampler);
            s_defaultSampler = nullptr;
            DestroyQuadIndexBuffer();
            DestroyVertexBuffers();
            DestroyPipelines();
            SDL_ReleaseWindowFromGPUDevice(s_device, s_window);
            SDL_DestroyGPUDevice(s_device);
            s_device = nullptr;
            return false;
        }

        g_ErrorReport.Write(L"RENDER: SDL_gpu -- Init complete");
        return true;
#else
        (void)pNativeWindow;
        g_ErrorReport.Write(L"RENDER: SDL_gpu -- Init called but MU_ENABLE_SDL3 is not defined");
        return false;
#endif
    }

    // -----------------------------------------------------------------------
    // Shutdown: Release all GPU resources and destroy the device.
    // Called at application exit before window destruction.
    // -----------------------------------------------------------------------
    static void Shutdown()
    {
#ifdef MU_ENABLE_SDL3
        if (!s_device)
        {
            return;
        }

        // Release white texture from registry if present.
        if (s_whiteTexture)
        {
            SDL_ReleaseGPUTexture(s_device, s_whiteTexture);
            s_whiteTexture = nullptr;
        }

        if (s_defaultSampler)
        {
            SDL_ReleaseGPUSampler(s_device, s_defaultSampler);
            s_defaultSampler = nullptr;
        }

        DestroyQuadIndexBuffer();
        DestroyVertexBuffers();
        DestroyPipelines();
        ReleaseShaders();

        SDL_ReleaseWindowFromGPUDevice(s_device, s_window);
        SDL_DestroyGPUDevice(s_device);
        s_device = nullptr;
        s_window = nullptr;

        g_ErrorReport.Write(L"RENDER: SDL_gpu -- Shutdown complete");
#endif
    }

    // -----------------------------------------------------------------------
    // BeginFrame: Acquire command buffer, swapchain texture, and begin render pass.
    // Called once per frame before any draw calls.
    // -----------------------------------------------------------------------
    void BeginFrame() override
    {
#ifdef MU_ENABLE_SDL3
        if (!s_device || !s_window)
        {
            return;
        }

        s_cmdBuf = SDL_AcquireGPUCommandBuffer(s_device);
        if (!s_cmdBuf)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- SDL_AcquireGPUCommandBuffer failed: %hs", SDL_GetError());
            return;
        }

        // Reset vertex scratch offset at start of each frame.
        s_vtxOffset = 0u;

        Uint32 swapW = 0u;
        Uint32 swapH = 0u;
        s_swapchainTexture = nullptr;

        if (!SDL_AcquireGPUSwapchainTexture(s_cmdBuf, s_window, &s_swapchainTexture, &swapW, &swapH))
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- SDL_AcquireGPUSwapchainTexture failed: %hs", SDL_GetError());
            SDL_CancelGPUCommandBuffer(s_cmdBuf);
            s_cmdBuf = nullptr;
            return;
        }

        // If swapchain texture is null, window is minimized/occluded — skip frame.
        if (!s_swapchainTexture)
        {
            // Debug-level only — this happens normally when window is minimized.
            SDL_CancelGPUCommandBuffer(s_cmdBuf);
            s_cmdBuf = nullptr;
            return;
        }

        // Upload vertex data for this frame via copy pass before render pass.
        // (Actual uploads happen in UploadVertices; here we begin a copy pass
        //  and upload whatever was queued. For story 4.3.1 the scratch buffer
        //  is reset each frame and individual draws upload inline.)

        // Begin render pass targeting the swapchain texture.
        SDL_GPUColorTargetInfo colorTarget{};
        colorTarget.texture = s_swapchainTexture;
        colorTarget.clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
        colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTarget.store_op = SDL_GPU_STOREOP_STORE;

        s_renderPass = SDL_BeginGPURenderPass(s_cmdBuf, &colorTarget, 1, nullptr);
        if (!s_renderPass)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- SDL_BeginGPURenderPass failed: %hs", SDL_GetError());
            SDL_CancelGPUCommandBuffer(s_cmdBuf);
            s_cmdBuf = nullptr;
            s_swapchainTexture = nullptr;
        }
#endif
    }

    // -----------------------------------------------------------------------
    // EndFrame: End render pass and submit command buffer.
    // Called once per frame after all draw calls.
    // Replaces SDL_GL_SwapWindow / SwapBuffers in the game loop.
    // -----------------------------------------------------------------------
    void EndFrame() override
    {
#ifdef MU_ENABLE_SDL3
        if (!s_renderPass)
        {
            // No render pass was started (minimized window or error) — submit anyway.
            if (s_cmdBuf)
            {
                SDL_SubmitGPUCommandBuffer(s_cmdBuf);
                s_cmdBuf = nullptr;
            }
            return;
        }

        SDL_EndGPURenderPass(s_renderPass);
        s_renderPass = nullptr;

        if (s_cmdBuf)
        {
            SDL_SubmitGPUCommandBuffer(s_cmdBuf);
            s_cmdBuf = nullptr;
        }

        s_swapchainTexture = nullptr;
#endif
    }

    // -----------------------------------------------------------------------
    // RenderQuad2D: Render a screen-space textured quad (4 vertices per quad).
    // Vertex count must be a multiple of 4.
    // -----------------------------------------------------------------------
    void RenderQuad2D(std::span<const Vertex2D> vertices, std::uint32_t textureId) override
    {
#ifdef MU_ENABLE_SDL3
        if (vertices.empty() || !s_renderPass)
        {
            return;
        }

        if (vertices.size() % 4 != 0)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu::RenderQuad2D -- vertex count %zu not divisible by 4",
                                vertices.size());
            return;
        }

        void* pTex = LookupTexture(textureId);
        if (!pTex)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu::RenderQuad2D -- unknown textureId %u, skipping", textureId);
            return;
        }

        const Uint32 byteSize = static_cast<Uint32>(vertices.size() * sizeof(Vertex2D));
        const Uint32 vtxOffset = UploadVertices(vertices.data(), byteSize);
        if (vtxOffset == ~0u)
        {
            return;
        }

        // Bind active pipeline (blend mode + depth test).
        const int pipelineIdx = GetActivePipelineIndex();
        SDL_GPUGraphicsPipeline* pipeline =
            m_depthTestEnabled ? s_pipelines[pipelineIdx] : s_pipelinesDepthOff[pipelineIdx];
        if (pipeline)
        {
            SDL_BindGPUGraphicsPipeline(s_renderPass, pipeline);
        }

        // Bind vertex buffer.
        SDL_GPUBufferBinding vtxBinding{};
        vtxBinding.buffer = s_vtxGpuBuf;
        vtxBinding.offset = vtxOffset;
        SDL_BindGPUVertexBuffers(s_renderPass, 0, &vtxBinding, 1);

        // Bind index buffer (static quad index buffer).
        SDL_GPUBufferBinding idxBinding{};
        idxBinding.buffer = s_quadIdxBuf;
        idxBinding.offset = 0;
        SDL_BindGPUIndexBuffer(s_renderPass, &idxBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

        // Bind texture and sampler.
        SDL_GPUTextureSamplerBinding samplerBinding{};
        samplerBinding.texture = static_cast<SDL_GPUTexture*>(pTex);
        samplerBinding.sampler = s_defaultSampler;
        SDL_BindGPUFragmentSamplers(s_renderPass, 0, &samplerBinding, 1);

        const Uint32 numQuads = static_cast<Uint32>(vertices.size() / 4);
        // Code-review fix H-2: guard against reading past the static quad index buffer.
        if (numQuads > static_cast<Uint32>(k_MaxQuads))
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu::RenderQuad2D -- numQuads %u exceeds k_MaxQuads %d; clamping draw",
                                numQuads, k_MaxQuads);
        }
        const Uint32 drawQuads =
            (numQuads <= static_cast<Uint32>(k_MaxQuads)) ? numQuads : static_cast<Uint32>(k_MaxQuads);
        SDL_DrawGPUIndexedPrimitives(s_renderPass, drawQuads * 6, 1, 0, 0, 0);
#else
        (void)vertices;
        (void)textureId;
#endif
    }

    // -----------------------------------------------------------------------
    // RenderTriangles: Render world-space triangles (vertex count divisible by 3).
    // -----------------------------------------------------------------------
    void RenderTriangles(std::span<const Vertex3D> vertices, std::uint32_t textureId) override
    {
#ifdef MU_ENABLE_SDL3
        if (vertices.empty() || !s_renderPass)
        {
            return;
        }

        if (vertices.size() % 3 != 0)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu::RenderTriangles -- vertex count %zu not divisible by 3",
                                vertices.size());
            return;
        }

        void* pTex = LookupTexture(textureId);
        if (!pTex)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu::RenderTriangles -- unknown textureId %u, skipping", textureId);
            return;
        }

        const Uint32 byteSize = static_cast<Uint32>(vertices.size() * sizeof(Vertex3D));
        const Uint32 vtxOffset = UploadVertices(vertices.data(), byteSize);
        if (vtxOffset == ~0u)
        {
            return;
        }

        const int pipelineIdx = GetActivePipelineIndex();
        SDL_GPUGraphicsPipeline* pipeline =
            m_depthTestEnabled ? s_pipelines[pipelineIdx] : s_pipelinesDepthOff[pipelineIdx];
        if (pipeline)
        {
            SDL_BindGPUGraphicsPipeline(s_renderPass, pipeline);
        }

        SDL_GPUBufferBinding vtxBinding{};
        vtxBinding.buffer = s_vtxGpuBuf;
        vtxBinding.offset = vtxOffset;
        SDL_BindGPUVertexBuffers(s_renderPass, 0, &vtxBinding, 1);

        SDL_GPUTextureSamplerBinding samplerBinding{};
        samplerBinding.texture = static_cast<SDL_GPUTexture*>(pTex);
        samplerBinding.sampler = s_defaultSampler;
        SDL_BindGPUFragmentSamplers(s_renderPass, 0, &samplerBinding, 1);

        SDL_DrawGPUPrimitives(s_renderPass, static_cast<Uint32>(vertices.size()), 1, 0, 0);
#else
        (void)vertices;
        (void)textureId;
#endif
    }

    // -----------------------------------------------------------------------
    // RenderQuadStrip: Render a quad strip as triangle list.
    // Converts vertex pairs (0,1), (2,3), ... into triangles.
    // Strip-to-triangle index pattern: (0,1,2),(1,3,2),(2,3,4),(3,5,4),...
    // -----------------------------------------------------------------------
    void RenderQuadStrip(std::span<const Vertex3D> vertices, std::uint32_t textureId) override
    {
#ifdef MU_ENABLE_SDL3
        if (vertices.size() < 2 || !s_renderPass)
        {
            return;
        }

        void* pTex = LookupTexture(textureId);
        if (!pTex)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu::RenderQuadStrip -- unknown textureId %u, skipping", textureId);
            return;
        }

        const Uint32 byteSize = static_cast<Uint32>(vertices.size() * sizeof(Vertex3D));
        const Uint32 vtxOffset = UploadVertices(vertices.data(), byteSize);
        if (vtxOffset == ~0u)
        {
            return;
        }

        // Generate strip-to-triangle-list indices.
        const Uint32 numQuads = static_cast<Uint32>((vertices.size() - 1) / 2);
        if (numQuads == 0)
        {
            return;
        }

        const Uint32 numIndices = numQuads * 6;
        std::vector<Uint16> indices;
        indices.reserve(numIndices);

        // Strip pattern: quad i uses vertices (2i, 2i+1, 2i+2, 2i+3).
        // Two triangles: (2i, 2i+1, 2i+2) and (2i+1, 2i+3, 2i+2).
        for (Uint32 i = 0; i < numQuads; ++i)
        {
            const auto v0 = static_cast<Uint16>(i * 2 + 0);
            const auto v1 = static_cast<Uint16>(i * 2 + 1);
            const auto v2 = static_cast<Uint16>(i * 2 + 2);
            const auto v3 = static_cast<Uint16>(i * 2 + 3);
            indices.push_back(v0);
            indices.push_back(v1);
            indices.push_back(v2);
            indices.push_back(v1);
            indices.push_back(v3);
            indices.push_back(v2);
        }

        // Ensure strip index buffer capacity.
        if (!EnsureStripIndexBuffer(numIndices))
        {
            return;
        }

        // Upload strip indices.
        {
            void* pMapped = SDL_MapGPUTransferBuffer(s_device, s_stripIdxTransfer, true);
            if (!pMapped)
            {
                g_ErrorReport.Write(L"RENDER: SDL_gpu::RenderQuadStrip -- failed to map strip index transfer buffer");
                return;
            }
            std::memcpy(pMapped, indices.data(), numIndices * sizeof(Uint16));
            SDL_UnmapGPUTransferBuffer(s_device, s_stripIdxTransfer);

            // Copy to GPU index buffer via copy pass.
            SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(s_cmdBuf);
            if (copyPass)
            {
                SDL_GPUTransferBufferLocation src{};
                src.transfer_buffer = s_stripIdxTransfer;
                src.offset = 0;

                SDL_GPUBufferRegion dst{};
                dst.buffer = s_stripIdxBuf;
                dst.offset = 0;
                dst.size = numIndices * sizeof(Uint16);

                SDL_UploadToGPUBuffer(copyPass, &src, &dst, true);
                SDL_EndGPUCopyPass(copyPass);
            }
        }

        const int pipelineIdx = GetActivePipelineIndex();
        SDL_GPUGraphicsPipeline* pipeline =
            m_depthTestEnabled ? s_pipelines[pipelineIdx] : s_pipelinesDepthOff[pipelineIdx];
        if (pipeline)
        {
            SDL_BindGPUGraphicsPipeline(s_renderPass, pipeline);
        }

        SDL_GPUBufferBinding vtxBinding{};
        vtxBinding.buffer = s_vtxGpuBuf;
        vtxBinding.offset = vtxOffset;
        SDL_BindGPUVertexBuffers(s_renderPass, 0, &vtxBinding, 1);

        SDL_GPUBufferBinding idxBinding{};
        idxBinding.buffer = s_stripIdxBuf;
        idxBinding.offset = 0;
        SDL_BindGPUIndexBuffer(s_renderPass, &idxBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

        SDL_GPUTextureSamplerBinding samplerBinding{};
        samplerBinding.texture = static_cast<SDL_GPUTexture*>(pTex);
        samplerBinding.sampler = s_defaultSampler;
        SDL_BindGPUFragmentSamplers(s_renderPass, 0, &samplerBinding, 1);

        SDL_DrawGPUIndexedPrimitives(s_renderPass, numIndices, 1, 0, 0, 0);
#else
        (void)vertices;
        (void)textureId;
#endif
    }

    // -----------------------------------------------------------------------
    // SetBlendMode: Store active blend mode; pipeline bound at draw time.
    // -----------------------------------------------------------------------
    void SetBlendMode(BlendMode mode) override
    {
        m_blendEnabled = true;
        m_activeBlendMode = mode;
    }

    // -----------------------------------------------------------------------
    // DisableBlend: Select the no-blend pipeline (index 8).
    // -----------------------------------------------------------------------
    void DisableBlend() override
    {
        m_blendEnabled = false;
    }

    // -----------------------------------------------------------------------
    // SetDepthTest: Track depth test state.
    // SDL_gpu depth state is baked into pipeline objects — dual pipeline set
    // (s_pipelines[] with depth ON, s_pipelinesDepthOff[] with depth OFF)
    // allows this to be applied per-draw without pipeline rebuild.
    // -----------------------------------------------------------------------
    void SetDepthTest(bool enabled) override
    {
        m_depthTestEnabled = enabled;
    }

    // -----------------------------------------------------------------------
    // SetFog: Store FogParams for later use by the shader (story 4.3.2).
    // In this story fog is stored but not applied to pixels — visual regression
    // is acceptable until 4.3.2 adds the basic_textured shader with fog support.
    // -----------------------------------------------------------------------
    void SetFog(const FogParams& params) override
    {
        m_fogParams = params;
    }

private:
    // Per-instance render state (blend mode, depth test, fog).
    BlendMode m_activeBlendMode = BlendMode::Alpha;
    bool m_blendEnabled = true;
    bool m_depthTestEnabled = true;
    FogParams m_fogParams{};

    // -----------------------------------------------------------------------
    // GetActivePipelineIndex: Returns pipeline array index for current state.
    // -----------------------------------------------------------------------
    [[nodiscard]] int GetActivePipelineIndex() const
    {
        if (!m_blendEnabled)
        {
            return k_PipelineDisabled;
        }
        return static_cast<int>(m_activeBlendMode);
    }

    // -----------------------------------------------------------------------
    // UploadVertices: Copy vertex data into the per-frame scratch transfer buffer.
    // Returns the byte offset in the GPU vertex buffer, or ~0u on failure.
    // -----------------------------------------------------------------------
    [[nodiscard]] static Uint32 UploadVertices(const void* pData, Uint32 byteSize)
    {
#ifdef MU_ENABLE_SDL3
        if (!s_vtxTransferBuf || !s_vtxGpuBuf || !s_cmdBuf)
        {
            return ~0u;
        }

        const Uint32 alignedOffset = (s_vtxOffset + 3u) & ~3u; // 4-byte alignment
        if (alignedOffset + byteSize > k_VertexBufferSize)
        {
            g_ErrorReport.Write(
                L"RENDER: SDL_gpu -- vertex scratch buffer overflow (offset %u + size %u > capacity %u)", alignedOffset,
                byteSize, k_VertexBufferSize);
            return ~0u;
        }

        // Map transfer buffer and copy vertex data.
        Uint8* pMapped = static_cast<Uint8*>(SDL_MapGPUTransferBuffer(s_device, s_vtxTransferBuf, true));
        if (!pMapped)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- failed to map vertex transfer buffer: %hs", SDL_GetError());
            return ~0u;
        }
        std::memcpy(pMapped + alignedOffset, pData, byteSize);
        SDL_UnmapGPUTransferBuffer(s_device, s_vtxTransferBuf);

        // Upload from transfer buffer to GPU vertex buffer via copy pass.
        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(s_cmdBuf);
        if (!copyPass)
        {
            return ~0u;
        }

        SDL_GPUTransferBufferLocation src{};
        src.transfer_buffer = s_vtxTransferBuf;
        src.offset = alignedOffset;

        SDL_GPUBufferRegion dst{};
        dst.buffer = s_vtxGpuBuf;
        dst.offset = alignedOffset;
        dst.size = byteSize;

        SDL_UploadToGPUBuffer(copyPass, &src, &dst, true);
        SDL_EndGPUCopyPass(copyPass);

        s_vtxOffset = alignedOffset + byteSize;
        return alignedOffset;
#else
        (void)pData;
        (void)byteSize;
        return ~0u;
#endif
    }

    // -----------------------------------------------------------------------
    // Static resource creation/destruction helpers.
    // -----------------------------------------------------------------------

#ifdef MU_ENABLE_SDL3
    [[nodiscard]] static bool CreatePlaceholderShaders()
    {
        // Attempt to create minimal placeholder shaders from SPIR-V stubs.
        // These are intentionally minimal — story 4.3.2 provides real shaders.
        // If the device does not support SPIR-V (non-Vulkan backend), creation
        // will fail; we log and continue so Init can proceed.
        SDL_GPUShaderCreateInfo vsInfo{};
        vsInfo.code = k_VertexShaderSPIRV;
        vsInfo.code_size = sizeof(k_VertexShaderSPIRV);
        vsInfo.entrypoint = "main";
        vsInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
        vsInfo.stage = SDL_GPU_SHADERSTAGE_VERTEX;
        vsInfo.num_samplers = 0;
        vsInfo.num_storage_textures = 0;
        vsInfo.num_storage_buffers = 0;
        vsInfo.num_uniform_buffers = 0;

        s_vertShader = SDL_CreateGPUShader(s_device, &vsInfo);
        if (!s_vertShader)
        {
            // Non-fatal — pipelines will be null; draw calls skip gracefully.
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- vertex shader creation failed (placeholder): %hs", SDL_GetError());
        }

        SDL_GPUShaderCreateInfo fsInfo{};
        fsInfo.code = k_FragmentShaderSPIRV;
        fsInfo.code_size = sizeof(k_FragmentShaderSPIRV);
        fsInfo.entrypoint = "main";
        fsInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
        fsInfo.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
        fsInfo.num_samplers = 1; // one texture sampler
        fsInfo.num_storage_textures = 0;
        fsInfo.num_storage_buffers = 0;
        fsInfo.num_uniform_buffers = 0;

        s_fragShader = SDL_CreateGPUShader(s_device, &fsInfo);
        if (!s_fragShader)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- fragment shader creation failed (placeholder): %hs",
                                SDL_GetError());
        }

        // Shaders may be null for non-Vulkan backends — pipelines created below
        // will be null and draw calls will skip. Story 4.3.2 fixes this fully.
        return true;
    }

    static void ReleaseShaders()
    {
        if (s_vertShader)
        {
            SDL_ReleaseGPUShader(s_device, s_vertShader);
            s_vertShader = nullptr;
        }
        if (s_fragShader)
        {
            SDL_ReleaseGPUShader(s_device, s_fragShader);
            s_fragShader = nullptr;
        }
    }

    [[nodiscard]] static SDL_GPUGraphicsPipeline* BuildBlendPipeline(SDL_GPUColorTargetBlendState blendState,
                                                                     bool depthTestEnabled)
    {
        // Get swapchain texture format for pipeline target.
        const SDL_GPUTextureFormat swapchainFmt = SDL_GetGPUSwapchainTextureFormat(s_device, s_window);

        SDL_GPUColorTargetDescription colorTargetDesc{};
        colorTargetDesc.format = swapchainFmt;
        colorTargetDesc.blend_state = blendState;

        // Vertex layout for Vertex2D: pos(float2), uv(float2), color(ubyte4_norm)
        SDL_GPUVertexAttribute vertexAttribs[3];
        vertexAttribs[0] = {};
        vertexAttribs[0].location = 0;
        vertexAttribs[0].buffer_slot = 0;
        vertexAttribs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        vertexAttribs[0].offset = static_cast<Uint32>(offsetof(Vertex2D, x));

        vertexAttribs[1] = {};
        vertexAttribs[1].location = 1;
        vertexAttribs[1].buffer_slot = 0;
        vertexAttribs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        vertexAttribs[1].offset = static_cast<Uint32>(offsetof(Vertex2D, u));

        vertexAttribs[2] = {};
        vertexAttribs[2].location = 2;
        vertexAttribs[2].buffer_slot = 0;
        vertexAttribs[2].format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM;
        vertexAttribs[2].offset = static_cast<Uint32>(offsetof(Vertex2D, color));

        SDL_GPUVertexBufferDescription vtxBufDesc{};
        vtxBufDesc.slot = 0;
        vtxBufDesc.pitch = sizeof(Vertex2D);
        vtxBufDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
        vtxBufDesc.instance_step_rate = 0;

        SDL_GPUVertexInputState vtxInputState{};
        vtxInputState.vertex_buffer_descriptions = &vtxBufDesc;
        vtxInputState.num_vertex_buffers = 1;
        vtxInputState.vertex_attributes = vertexAttribs;
        vtxInputState.num_vertex_attributes = 3;

        SDL_GPUDepthStencilState depthState{};
        depthState.enable_depth_test = depthTestEnabled;
        depthState.enable_depth_write = depthTestEnabled;
        depthState.enable_stencil_test = false;
        depthState.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL;

        SDL_GPUGraphicsPipelineTargetInfo targetInfo{};
        targetInfo.color_target_descriptions = &colorTargetDesc;
        targetInfo.num_color_targets = 1;
        targetInfo.has_depth_stencil_target = false;

        SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.vertex_shader = s_vertShader;
        pipelineInfo.fragment_shader = s_fragShader;
        pipelineInfo.vertex_input_state = vtxInputState;
        pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
        pipelineInfo.depth_stencil_state = depthState;
        pipelineInfo.target_info = targetInfo;
        pipelineInfo.props = 0;

        SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(s_device, &pipelineInfo);
        if (!pipeline)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- pipeline creation failed: %hs", SDL_GetError());
        }
        return pipeline;
    }

    [[nodiscard]] static bool CreatePipelines()
    {
        // Blend mode table from architecture-rendering.md and story dev notes.
        // Indices match BlendMode enum cast to int; index 8 = disabled.
        //
        // SDL_GPUBlendFactor values (INVALID=0, ZERO=1, ONE=2, SRC_COLOR=3,
        //   ONE_MINUS_SRC_COLOR=4, DST_COLOR=5, ONE_MINUS_DST_COLOR=6,
        //   SRC_ALPHA=7, ONE_MINUS_SRC_ALPHA=8)
        //
        // Modes: Alpha=0, Additive=1, Subtract=2, InverseColor=3,
        //        Mixed=4, LightMap=5, Glow=6, Luminance=7, Disabled=8

        struct BlendEntry
        {
            SDL_GPUBlendFactor src;
            SDL_GPUBlendFactor dst;
            bool enableBlend;
        };

        const BlendEntry table[k_PipelineCount] = {
            // Alpha
            {SDL_GPU_BLENDFACTOR_SRC_ALPHA, SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, true},
            // Additive
            {SDL_GPU_BLENDFACTOR_SRC_ALPHA, SDL_GPU_BLENDFACTOR_ONE, true},
            // Subtract
            {SDL_GPU_BLENDFACTOR_ZERO, SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR, true},
            // InverseColor
            {SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_COLOR, SDL_GPU_BLENDFACTOR_ZERO, true},
            // Mixed
            {SDL_GPU_BLENDFACTOR_ONE, SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, true},
            // LightMap
            {SDL_GPU_BLENDFACTOR_ZERO, SDL_GPU_BLENDFACTOR_SRC_COLOR, true},
            // Glow
            {SDL_GPU_BLENDFACTOR_ONE, SDL_GPU_BLENDFACTOR_ONE, true},
            // Luminance
            {SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR, SDL_GPU_BLENDFACTOR_ONE, true},
            // Disabled (index 8, no blend)
            {SDL_GPU_BLENDFACTOR_ONE, SDL_GPU_BLENDFACTOR_ZERO, false},
        };

        for (int i = 0; i < k_PipelineCount; ++i)
        {
            SDL_GPUColorTargetBlendState blendState{};
            blendState.src_color_blendfactor = table[i].src;
            blendState.dst_color_blendfactor = table[i].dst;
            blendState.color_blend_op = SDL_GPU_BLENDOP_ADD;
            blendState.src_alpha_blendfactor = table[i].src;
            blendState.dst_alpha_blendfactor = table[i].dst;
            blendState.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
            blendState.enable_blend = table[i].enableBlend;

            // Depth ON variant.
            s_pipelines[i] = BuildBlendPipeline(blendState, true);
            if (!s_pipelines[i])
            {
                g_ErrorReport.Write(L"RENDER: SDL_gpu -- pipeline creation failed for BlendMode %d: %hs", i,
                                    SDL_GetError());
                // Non-fatal — draw calls will skip gracefully when pipeline is null.
            }

            // Depth OFF variant.
            s_pipelinesDepthOff[i] = BuildBlendPipeline(blendState, false);
            if (!s_pipelinesDepthOff[i])
            {
                g_ErrorReport.Write(L"RENDER: SDL_gpu -- depth-off pipeline creation failed for BlendMode %d: %hs", i,
                                    SDL_GetError());
            }
        }

        return true; // Pipeline creation failures are non-fatal for this story.
    }

    static void DestroyPipelines()
    {
        for (int i = 0; i < k_PipelineCount; ++i)
        {
            if (s_pipelines[i])
            {
                SDL_ReleaseGPUGraphicsPipeline(s_device, s_pipelines[i]);
                s_pipelines[i] = nullptr;
            }
            if (s_pipelinesDepthOff[i])
            {
                SDL_ReleaseGPUGraphicsPipeline(s_device, s_pipelinesDepthOff[i]);
                s_pipelinesDepthOff[i] = nullptr;
            }
        }
    }

    [[nodiscard]] static bool CreateVertexBuffers()
    {
        SDL_GPUTransferBufferCreateInfo tbInfo{};
        tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        tbInfo.size = k_VertexBufferSize;
        s_vtxTransferBuf = SDL_CreateGPUTransferBuffer(s_device, &tbInfo);
        if (!s_vtxTransferBuf)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- vertex transfer buffer creation failed: %hs", SDL_GetError());
            return false;
        }

        SDL_GPUBufferCreateInfo bufInfo{};
        bufInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        bufInfo.size = k_VertexBufferSize;
        s_vtxGpuBuf = SDL_CreateGPUBuffer(s_device, &bufInfo);
        if (!s_vtxGpuBuf)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- vertex GPU buffer creation failed: %hs", SDL_GetError());
            SDL_ReleaseGPUTransferBuffer(s_device, s_vtxTransferBuf);
            s_vtxTransferBuf = nullptr;
            return false;
        }

        return true;
    }

    static void DestroyVertexBuffers()
    {
        if (s_vtxGpuBuf)
        {
            SDL_ReleaseGPUBuffer(s_device, s_vtxGpuBuf);
            s_vtxGpuBuf = nullptr;
        }
        if (s_vtxTransferBuf)
        {
            SDL_ReleaseGPUTransferBuffer(s_device, s_vtxTransferBuf);
            s_vtxTransferBuf = nullptr;
        }
        if (s_stripIdxBuf)
        {
            SDL_ReleaseGPUBuffer(s_device, s_stripIdxBuf);
            s_stripIdxBuf = nullptr;
        }
        if (s_stripIdxTransfer)
        {
            SDL_ReleaseGPUTransferBuffer(s_device, s_stripIdxTransfer);
            s_stripIdxTransfer = nullptr;
        }
        s_stripIdxCapacity = 0u;
    }

    [[nodiscard]] static bool CreateQuadIndexBuffer()
    {
        // Pre-generate indices for up to k_MaxQuads quads.
        // Pattern per quad i: [4i+0, 4i+1, 4i+2, 4i+0, 4i+2, 4i+3]
        // Winding: TL(0), BL(1), BR(2), TR(3) — CCW in screen-space Y-down.
        std::vector<Uint16> indices;
        indices.reserve(static_cast<size_t>(k_MaxQuads) * 6);

        for (int i = 0; i < k_MaxQuads; ++i)
        {
            indices.push_back(static_cast<Uint16>(i * 4 + 0));
            indices.push_back(static_cast<Uint16>(i * 4 + 1));
            indices.push_back(static_cast<Uint16>(i * 4 + 2));
            indices.push_back(static_cast<Uint16>(i * 4 + 0));
            indices.push_back(static_cast<Uint16>(i * 4 + 2));
            indices.push_back(static_cast<Uint16>(i * 4 + 3));
        }

        const Uint32 idxByteSize = static_cast<Uint32>(indices.size() * sizeof(Uint16));

        // Upload via a one-time command buffer.
        SDL_GPUCommandBuffer* uploadCmd = SDL_AcquireGPUCommandBuffer(s_device);
        if (!uploadCmd)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- failed to acquire command buffer for index upload");
            return false;
        }

        SDL_GPUTransferBufferCreateInfo tbInfo{};
        tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        tbInfo.size = idxByteSize;
        SDL_GPUTransferBuffer* idxTransfer = SDL_CreateGPUTransferBuffer(s_device, &tbInfo);
        if (!idxTransfer)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- quad index transfer buffer creation failed: %hs", SDL_GetError());
            SDL_CancelGPUCommandBuffer(uploadCmd);
            return false;
        }

        void* pMapped = SDL_MapGPUTransferBuffer(s_device, idxTransfer, false);
        if (!pMapped)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- failed to map quad index transfer buffer");
            SDL_ReleaseGPUTransferBuffer(s_device, idxTransfer);
            SDL_CancelGPUCommandBuffer(uploadCmd);
            return false;
        }
        std::memcpy(pMapped, indices.data(), idxByteSize);
        SDL_UnmapGPUTransferBuffer(s_device, idxTransfer);

        SDL_GPUBufferCreateInfo bufInfo{};
        bufInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
        bufInfo.size = idxByteSize;
        s_quadIdxBuf = SDL_CreateGPUBuffer(s_device, &bufInfo);
        if (!s_quadIdxBuf)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- quad index GPU buffer creation failed: %hs", SDL_GetError());
            SDL_ReleaseGPUTransferBuffer(s_device, idxTransfer);
            SDL_CancelGPUCommandBuffer(uploadCmd);
            return false;
        }

        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmd);
        if (!copyPass)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- failed to begin copy pass for index upload");
            SDL_ReleaseGPUBuffer(s_device, s_quadIdxBuf);
            s_quadIdxBuf = nullptr;
            SDL_ReleaseGPUTransferBuffer(s_device, idxTransfer);
            SDL_CancelGPUCommandBuffer(uploadCmd);
            return false;
        }

        SDL_GPUTransferBufferLocation src{};
        src.transfer_buffer = idxTransfer;
        src.offset = 0;

        SDL_GPUBufferRegion dst{};
        dst.buffer = s_quadIdxBuf;
        dst.offset = 0;
        dst.size = idxByteSize;

        SDL_UploadToGPUBuffer(copyPass, &src, &dst, false);
        SDL_EndGPUCopyPass(copyPass);
        SDL_SubmitGPUCommandBuffer(uploadCmd);

        SDL_ReleaseGPUTransferBuffer(s_device, idxTransfer);
        return true;
    }

    static void DestroyQuadIndexBuffer()
    {
        if (s_quadIdxBuf)
        {
            SDL_ReleaseGPUBuffer(s_device, s_quadIdxBuf);
            s_quadIdxBuf = nullptr;
        }
    }

    [[nodiscard]] static bool EnsureStripIndexBuffer(Uint32 requiredIndices)
    {
        if (s_stripIdxCapacity >= requiredIndices)
        {
            return true; // existing buffer is large enough
        }

        // Release old buffers.
        if (s_stripIdxBuf)
        {
            SDL_ReleaseGPUBuffer(s_device, s_stripIdxBuf);
            s_stripIdxBuf = nullptr;
        }
        if (s_stripIdxTransfer)
        {
            SDL_ReleaseGPUTransferBuffer(s_device, s_stripIdxTransfer);
            s_stripIdxTransfer = nullptr;
        }

        const Uint32 byteSize = requiredIndices * sizeof(Uint16);

        SDL_GPUTransferBufferCreateInfo tbInfo{};
        tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        tbInfo.size = byteSize;
        s_stripIdxTransfer = SDL_CreateGPUTransferBuffer(s_device, &tbInfo);
        if (!s_stripIdxTransfer)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- strip index transfer buffer creation failed: %hs", SDL_GetError());
            return false;
        }

        SDL_GPUBufferCreateInfo bufInfo{};
        bufInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
        bufInfo.size = byteSize;
        s_stripIdxBuf = SDL_CreateGPUBuffer(s_device, &bufInfo);
        if (!s_stripIdxBuf)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- strip index GPU buffer creation failed: %hs", SDL_GetError());
            SDL_ReleaseGPUTransferBuffer(s_device, s_stripIdxTransfer);
            s_stripIdxTransfer = nullptr;
            return false;
        }

        s_stripIdxCapacity = requiredIndices;
        return true;
    }

    [[nodiscard]] static bool CreateWhiteTexture()
    {
        // Upload a 1×1 RGBA white pixel as the default fallback texture.
        SDL_GPUTextureCreateInfo texInfo{};
        texInfo.type = SDL_GPU_TEXTURETYPE_2D;
        texInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        texInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
        texInfo.width = 1;
        texInfo.height = 1;
        texInfo.layer_count_or_depth = 1;
        texInfo.num_levels = 1;
        texInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;

        s_whiteTexture = SDL_CreateGPUTexture(s_device, &texInfo);
        if (!s_whiteTexture)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- white texture creation failed: %hs", SDL_GetError());
            return false;
        }

        // Upload white pixel data.
        constexpr Uint32 kWhitePixel = 0xFFFFFFFFu; // RGBA = 255,255,255,255
        constexpr Uint32 kPixelByteSize = 4u;

        SDL_GPUTransferBufferCreateInfo tbInfo{};
        tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        tbInfo.size = kPixelByteSize;
        SDL_GPUTransferBuffer* pixelTransfer = SDL_CreateGPUTransferBuffer(s_device, &tbInfo);
        if (!pixelTransfer)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- white texture transfer buffer failed: %hs", SDL_GetError());
            SDL_ReleaseGPUTexture(s_device, s_whiteTexture);
            s_whiteTexture = nullptr;
            return false;
        }

        void* pMapped = SDL_MapGPUTransferBuffer(s_device, pixelTransfer, false);
        if (!pMapped)
        {
            SDL_ReleaseGPUTransferBuffer(s_device, pixelTransfer);
            SDL_ReleaseGPUTexture(s_device, s_whiteTexture);
            s_whiteTexture = nullptr;
            return false;
        }
        std::memcpy(pMapped, &kWhitePixel, kPixelByteSize);
        SDL_UnmapGPUTransferBuffer(s_device, pixelTransfer);

        SDL_GPUCommandBuffer* uploadCmd = SDL_AcquireGPUCommandBuffer(s_device);
        if (!uploadCmd)
        {
            SDL_ReleaseGPUTransferBuffer(s_device, pixelTransfer);
            SDL_ReleaseGPUTexture(s_device, s_whiteTexture);
            s_whiteTexture = nullptr;
            return false;
        }

        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmd);
        if (!copyPass)
        {
            SDL_ReleaseGPUTransferBuffer(s_device, pixelTransfer);
            SDL_ReleaseGPUTexture(s_device, s_whiteTexture);
            s_whiteTexture = nullptr;
            SDL_CancelGPUCommandBuffer(uploadCmd);
            return false;
        }

        SDL_GPUTextureTransferInfo texTransferInfo{};
        texTransferInfo.transfer_buffer = pixelTransfer;
        texTransferInfo.offset = 0;
        texTransferInfo.pixels_per_row = 1;
        texTransferInfo.rows_per_layer = 1;

        SDL_GPUTextureRegion texRegion{};
        texRegion.texture = s_whiteTexture;
        texRegion.mip_level = 0;
        texRegion.layer = 0;
        texRegion.x = 0;
        texRegion.y = 0;
        texRegion.z = 0;
        texRegion.w = 1;
        texRegion.h = 1;
        texRegion.d = 1;

        SDL_UploadToGPUTexture(copyPass, &texTransferInfo, &texRegion, false);
        SDL_EndGPUCopyPass(copyPass);
        SDL_SubmitGPUCommandBuffer(uploadCmd);

        SDL_ReleaseGPUTransferBuffer(s_device, pixelTransfer);

        // Register the white texture as textureId=0 (sentinel for "no texture").
        RegisterTexture(0u, s_whiteTexture);

        return true;
    }

#endif // MU_ENABLE_SDL3
};

// ---------------------------------------------------------------------------
// GetRenderer / InitSDLGpuRenderer / ShutdownSDLGpuRenderer:
// Only compiled when MU_USE_OPENGL_BACKEND is OFF (the default SDL_gpu path).
// When MU_USE_OPENGL_BACKEND is ON, MuRenderer.cpp provides GetRenderer()
// returning MuRendererGL. Both files defining GetRenderer() would be an ODR
// violation. Code-review fix H-1.
// ---------------------------------------------------------------------------
#ifndef MU_USE_OPENGL_BACKEND

[[nodiscard]] IMuRenderer& GetRenderer()
{
    static MuRendererSDLGpu s_instance;
    return s_instance;
}

// C++ linkage entry points for Winmain.cpp (no class forward declaration needed).
[[nodiscard]] bool InitSDLGpuRenderer(void* pNativeWindow)
{
    return MuRendererSDLGpu::Init(pNativeWindow);
}

void ShutdownSDLGpuRenderer()
{
    MuRendererSDLGpu::Shutdown();
}

#endif // !MU_USE_OPENGL_BACKEND

} // namespace mu
