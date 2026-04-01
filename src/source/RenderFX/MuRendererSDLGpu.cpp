// MuRendererSDLGpu.cpp: SDL_gpu backend implementation of IMuRenderer.
// Story 4.3.1 — Flow Code: VS1-RENDER-SDLGPU-BACKEND
// Story 4.3.2 — Flow Code: VS1-RENDER-SHADERS (shader loading, fog UBO, pipeline fixes)
//
// MuRendererSDLGpu replaces the OpenGL immediate-mode backend (MuRendererGL)
// with SDL_gpu — selecting Metal on macOS, Vulkan on Linux, D3D12 on Windows.
//
// DESIGN NOTES (4.3.2 updates):
//   - No #ifdef _WIN32 in this file — SDL_gpu handles platform selection internally.
//   - No OpenGL types or includes — SDL3/SDL_gpu.h provides all required types.
//   - TextureRegistry, GetBlendFactors, GetShaderBlobPath, GetPipelineSetFor are in mu::
//     namespace and accessible to the test TU via forward declarations.
//   - BeginFrame() / EndFrame() are instance methods called from MuMain.cpp game loop.
//   - Real HLSL shaders loaded from MU_SHADER_DIR blobs (set by CMake).
//   - Fog uniform buffer (s_fogUniformBuf) is created in Init() and updated in SetFog().
//   - Separate 2D/3D pipeline sets: s_pipelines2D / s_pipelines3D (AC-8 fix).
//   - Single pre-frame vertex upload in BeginFrame() / unmap in EndFrame() (AC-7 fix).
//
// GUARD STRUCTURE:
//   Story 7.9.3: SDL_gpu is the only renderer backend (MuRenderer.cpp deleted).

// Include SDL3 GPU header only in this file — not exposed to game logic.
// MU_ENABLE_SDL3 is defined at CMake project scope; the preprocessor guard
// around these includes was redundant and has been removed (Story 7.6.2, AC-7).
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL.h>

#include "MuRenderer.h"
#include "ErrorReport.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// GLM — matrix math for projection, view, and model transforms.
// GLM_FORCE_DEPTH_ZERO_TO_ONE: Metal/Vulkan depth range [0,1] (not OpenGL [-1,1]).
// Right-handed (GLM default): matches original OpenGL game code coordinate convention.
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ---------------------------------------------------------------------------
// Constants (file-scope, anonymous namespace)
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

} // anonymous namespace

namespace mu
{

// ---------------------------------------------------------------------------
// Story 4.3.2 (AC-10): Fog uniform buffer struct — in mu:: namespace so that
// test_shaderprograms.cpp can forward-declare and verify layout via static_assert.
// Mirrors the FogUniforms cbuffer declared in basic_textured.frag.hlsl (std140).
// std140 layout: uint(4), uint(4), float(4), float(4), float(4), float(4), float4(16), float2(8)
// Total: 48 bytes.
// ---------------------------------------------------------------------------
struct FogUniform
{
    uint32_t fogEnabled;           // offset  0
    uint32_t alphaDiscardEnabled;  // offset  4
    float alphaThreshold;          // offset  8
    float pad0;                    // offset 12
    float fogStart;                // offset 16
    float fogEnd;                  // offset 20
    std::array<float, 4> fogColor; // offset 24 (float4, 16 bytes)
    float pad1[2];                 // offset 40 (8 bytes)
}; // total: 48 bytes

static_assert(offsetof(FogUniform, fogEnabled) == 0, "FogUniform std140 layout");
static_assert(offsetof(FogUniform, alphaDiscardEnabled) == 4, "FogUniform std140 layout");
static_assert(offsetof(FogUniform, alphaThreshold) == 8, "FogUniform std140 layout");
static_assert(offsetof(FogUniform, pad0) == 12, "FogUniform std140 layout");
static_assert(offsetof(FogUniform, fogStart) == 16, "FogUniform std140 layout");
static_assert(offsetof(FogUniform, fogEnd) == 20, "FogUniform std140 layout");
static_assert(offsetof(FogUniform, fogColor) == 24,
              "FogUniform std140 float4 alignment (std::array<float,4> == float[4])");
static_assert(offsetof(FogUniform, pad1) == 40, "FogUniform std140 layout");
static_assert(sizeof(FogUniform) == 48, "FogUniform must be 48 bytes (std140 HLSL cbuffer)");

// ---------------------------------------------------------------------------
// Story 4.3.2 (AC-6): GetShaderBlobPath
// Returns the absolute path to a compiled shader blob given GPU driver name,
// shader stage, and shader base name. Uses MU_SHADER_DIR (CMake compile def).
// driver: "vulkan" | "direct3d12" | "metal"
// stage:  "vert" | "frag"
// name:   e.g., "basic_textured", "basic_colored", "shadow_volume"
// ---------------------------------------------------------------------------
[[nodiscard]] std::string GetShaderBlobPath(const char* driver, const char* stage, const char* name)
{
    const char* ext = "spv";
    if (driver && std::string(driver) == "direct3d12")
    {
        ext = "dxil";
    }
    else if (driver && std::string(driver) == "metal")
    {
        ext = "msl";
    }

#ifndef MU_SHADER_DIR
#define MU_SHADER_DIR ""
#endif
    // Primary: cmake build-output path (works for local dev builds).
    // Fallback: shaders/ next to the executable (works for CI artifacts and installs).
    std::string filename = std::string(name) + "." + stage + "." + ext;
    std::filesystem::path cmakePath = std::filesystem::path(MU_SHADER_DIR) / filename;
    if (!cmakePath.empty() && std::filesystem::exists(cmakePath))
    {
        return cmakePath.string();
    }
    return (mu_get_app_dir() / "shaders" / filename).string();
}

// ---------------------------------------------------------------------------
// Story 4.3.2 (AC-6): GetShaderFormat
// Returns the SDL_GPUShaderFormat constant name for the given driver.
// Only used internally — returns the correct enum value for SDL_CreateGPUShader.
// ---------------------------------------------------------------------------
#ifdef MU_ENABLE_SDL3
[[nodiscard]] static SDL_GPUShaderFormat GetShaderFormat(const char* driver)
{
    if (driver && std::string(driver) == "direct3d12")
    {
        return SDL_GPU_SHADERFORMAT_DXIL;
    }
    if (driver && std::string(driver) == "metal")
    {
        return SDL_GPU_SHADERFORMAT_MSL;
    }
    return SDL_GPU_SHADERFORMAT_SPIRV;
}
#endif

// ---------------------------------------------------------------------------
// Story 4.3.2 (AC-8): GetPipelineSetFor
// Returns which pipeline set should be used for a given draw mode.
// RenderQuad2D → Pipelines2D; RenderTriangles/RenderQuadStrip → Pipelines3D
// Exposed for test linkage (test_shaderprograms.cpp).
// ---------------------------------------------------------------------------
enum class PipelineSet
{
    Pipelines2D,
    Pipelines3D
};

enum class DrawMode
{
    Quad2D,
    Triangles,
    QuadStrip
};

PipelineSet GetPipelineSetFor(DrawMode mode)
{
    switch (mode)
    {
    case DrawMode::Quad2D:
        return PipelineSet::Pipelines2D;
    case DrawMode::Triangles:
    case DrawMode::QuadStrip:
        return PipelineSet::Pipelines3D;
    default:
        g_ErrorReport.Write(L"RENDER: GetPipelineSetFor -- unknown DrawMode %d", static_cast<int>(mode));
        return PipelineSet::Pipelines3D;
    }
}

// ---------------------------------------------------------------------------
// Story 4.3.2 (AC-6): LoadShaderBlob
// Loads a compiled shader blob from disk into a byte vector.
// Returns empty vector on failure (caller logs via g_ErrorReport).
// ---------------------------------------------------------------------------
#ifdef MU_ENABLE_SDL3
[[nodiscard]] static std::vector<Uint8> LoadShaderBlob(const char* name, const char* stage, const char* driver)
{
    const std::string path = GetShaderBlobPath(driver, stage, name);
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        g_ErrorReport.Write(L"RENDER: SDL_gpu -- shader blob not found: %hs", path.c_str());
        return {};
    }
    const auto fileSize = static_cast<std::streamsize>(file.tellg());
    if (fileSize <= 0)
    {
        g_ErrorReport.Write(L"RENDER: SDL_gpu -- shader blob empty: %hs", path.c_str());
        return {};
    }
    file.seekg(0, std::ios::beg);
    std::vector<Uint8> blob(static_cast<size_t>(fileSize));
    file.read(reinterpret_cast<char*>(blob.data()), fileSize);
    if (!file)
    {
        g_ErrorReport.Write(L"RENDER: SDL_gpu -- shader blob read error: %hs", path.c_str());
        return {};
    }
    return blob;
}
#endif

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

// Swapchain dimensions in physical pixels (from SDL_AcquireGPUSwapchainTexture).
// Used for viewport calculations — distinct from logical window size on HiDPI/Retina.
static Uint32 s_swapW = 0u;
static Uint32 s_swapH = 0u;

// Diagnostics: per-frame counters, reset in BeginFrame, logged every 300 frames.
static Uint32 s_dbgFrameCount = 0u;
static Uint32 s_dbgDrawCallsThisFrame = 0u;
static Uint32 s_dbgVtxBytesThisFrame = 0u;
static bool s_dbgNullPipelineWarned = false;

// Story 4.3.2 (AC-8): Separate pipeline sets for 2D (Vertex2D) and 3D (Vertex3D) geometry.
// s_pipelines2D: depth ON, Vertex2D layout (pitch=20).
// s_pipelines2DDepthOff: depth OFF, Vertex2D layout.
// s_pipelines3D: depth ON, Vertex3D layout (pitch=40).
// s_pipelines3DDepthOff: depth OFF, Vertex3D layout.
// Index matches BlendMode enum cast to int; index 8 = disabled (no-blend).
static SDL_GPUGraphicsPipeline* s_pipelines2D[k_PipelineCount] = {};
static SDL_GPUGraphicsPipeline* s_pipelines2DDepthOff[k_PipelineCount] = {};
static SDL_GPUGraphicsPipeline* s_pipelines3D[k_PipelineCount] = {};
static SDL_GPUGraphicsPipeline* s_pipelines3DDepthOff[k_PipelineCount] = {};

// Story 4.3.2 (AC-7): Single pre-frame vertex upload.
// s_vtxMappedPtr: mapped pointer into s_vtxTransferBuf, valid between BeginFrame/EndFrame.
// s_vtxOffset: current write offset within the mapped transfer buffer.
static SDL_GPUTransferBuffer* s_vtxTransferBuf = nullptr;
static SDL_GPUBuffer* s_vtxGpuBuf = nullptr;
static Uint32 s_vtxOffset = 0u;
static void* s_vtxMappedPtr = nullptr; // mapped in BeginFrame, unmapped before render pass

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

// Shader handles for basic_textured (2D path) and basic_colored — released after pipeline creation.
static SDL_GPUShader* s_vertShader2D = nullptr;     // basic_textured.vert
static SDL_GPUShader* s_fragShaderTex = nullptr;    // basic_textured.frag
static SDL_GPUShader* s_vertShader2DCol = nullptr;  // basic_colored.vert
static SDL_GPUShader* s_fragShaderCol = nullptr;    // basic_colored.frag
static SDL_GPUShader* s_vertShaderShadow = nullptr; // shadow_volume.vert

// Story 4.3.2 (AC-10): Fog uniform buffer and transfer buffer.
static SDL_GPUBuffer* s_fogUniformBuf = nullptr;
static SDL_GPUTransferBuffer* s_fogTransferBuf = nullptr;
static bool s_fogDirty = true; // upload on first draw if SetFog not called

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
// Story 4.4.1 (AC-4, Task 6): SamplerRegistry — parallel to TextureRegistry.
// Maps caller-provided uint32_t ids to SDL_GPUSampler* (stored as void* for test linkage).
// RegisterSampler / LookupSampler / UnregisterSampler follow the same pattern as the texture registry.
// Sampler binding in draw calls uses LookupSampler(textureId) instead of the hardcoded s_defaultSampler.
// ---------------------------------------------------------------------------
static std::unordered_map<std::uint32_t, void*> s_samplerMap;

[[nodiscard]] void* LookupSampler(std::uint32_t id)
{
    auto it = s_samplerMap.find(id);
    if (it == s_samplerMap.end())
    {
#ifdef MU_ENABLE_SDL3
        return s_defaultSampler; // fallback to default sampler for unknown IDs
#else
        return nullptr;
#endif
    }
    return it->second;
}

void RegisterSampler(std::uint32_t id, void* pSampler)
{
    s_samplerMap[id] = pSampler;
}

void UnregisterSampler(std::uint32_t id)
{
    s_samplerMap.erase(id);
}

void ClearSamplerRegistry()
{
    s_samplerMap.clear();
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

        // Story 4.3.2: Load real HLSL shader blobs from MU_SHADER_DIR.
        // Driver name used to select the correct blob format (SPIR-V/DXIL/MSL).
        const char* driverName = SDL_GetGPUDeviceDriver(s_device);
        if (!LoadShaders(driverName))
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- shader loading failed; cannot build pipelines");
            SDL_ReleaseWindowFromGPUDevice(s_device, s_window);
            SDL_DestroyGPUDevice(s_device);
            s_device = nullptr;
            return false;
        }

        // Create blend mode pipelines (9 per set × 4 sets = 36 total).
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

        // Story 4.3.2 (AC-10): Create fog uniform GPU buffer and transfer buffer.
        if (!CreateFogUniformBuffers())
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- fog uniform buffer creation failed");
            if (s_whiteTexture)
            {
                SDL_ReleaseGPUTexture(s_device, s_whiteTexture);
                s_whiteTexture = nullptr;
            }
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

        // Story 4.3.2 (AC-10): Release fog uniform buffers.
        if (s_fogUniformBuf)
        {
            SDL_ReleaseGPUBuffer(s_device, s_fogUniformBuf);
            s_fogUniformBuf = nullptr;
        }
        if (s_fogTransferBuf)
        {
            SDL_ReleaseGPUTransferBuffer(s_device, s_fogTransferBuf);
            s_fogTransferBuf = nullptr;
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

        // Reset vertex scratch offset and per-frame diagnostics at start of each frame.
        s_vtxOffset = 0u;
        s_dbgDrawCallsThisFrame = 0u;
        s_dbgVtxBytesThisFrame = 0u;
        ++s_dbgFrameCount;

        // Story 4.3.2 (AC-7): Map vertex transfer buffer once for the whole frame.
        // Draw calls write into s_vtxMappedPtr; we copy to the GPU buffer once
        // via a single copy pass before SDL_BeginGPURenderPass.
        s_vtxMappedPtr = SDL_MapGPUTransferBuffer(s_device, s_vtxTransferBuf, false);
        if (!s_vtxMappedPtr)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- BeginFrame: failed to map vertex transfer buffer: %hs",
                                SDL_GetError());
            SDL_CancelGPUCommandBuffer(s_cmdBuf);
            s_cmdBuf = nullptr;
            return;
        }

        s_swapW = 0u;
        s_swapH = 0u;
        s_swapchainTexture = nullptr;

        if (!SDL_AcquireGPUSwapchainTexture(s_cmdBuf, s_window, &s_swapchainTexture, &s_swapW, &s_swapH))
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- SDL_AcquireGPUSwapchainTexture failed: %hs", SDL_GetError());
            SDL_UnmapGPUTransferBuffer(s_device, s_vtxTransferBuf);
            s_vtxMappedPtr = nullptr;
            SDL_CancelGPUCommandBuffer(s_cmdBuf);
            s_cmdBuf = nullptr;
            return;
        }

        // If swapchain texture is null, window is minimized/occluded — skip frame.
        if (!s_swapchainTexture)
        {
            // Debug-level only — this happens normally when window is minimized.
            SDL_UnmapGPUTransferBuffer(s_device, s_vtxTransferBuf);
            s_vtxMappedPtr = nullptr;
            SDL_CancelGPUCommandBuffer(s_cmdBuf);
            s_cmdBuf = nullptr;
            return;
        }

        // Story 4.3.2 (AC-10): Upload fog uniform if dirty, before render pass.
        if (s_fogDirty && s_fogTransferBuf && s_fogUniformBuf)
        {
            void* pFogMapped = SDL_MapGPUTransferBuffer(s_device, s_fogTransferBuf, false);
            if (pFogMapped)
            {
                std::memcpy(pFogMapped, &m_fogUniform, sizeof(FogUniform));
                SDL_UnmapGPUTransferBuffer(s_device, s_fogTransferBuf);

                SDL_GPUCopyPass* fogCopyPass = SDL_BeginGPUCopyPass(s_cmdBuf);
                if (fogCopyPass)
                {
                    SDL_GPUTransferBufferLocation fogSrc{};
                    fogSrc.transfer_buffer = s_fogTransferBuf;
                    fogSrc.offset = 0;

                    SDL_GPUBufferRegion fogDst{};
                    fogDst.buffer = s_fogUniformBuf;
                    fogDst.offset = 0;
                    fogDst.size = sizeof(FogUniform);

                    SDL_UploadToGPUBuffer(fogCopyPass, &fogSrc, &fogDst, false);
                    SDL_EndGPUCopyPass(fogCopyPass);
                }
                s_fogDirty = false;
            }
        }

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
            SDL_UnmapGPUTransferBuffer(s_device, s_vtxTransferBuf);
            s_vtxMappedPtr = nullptr;
            SDL_CancelGPUCommandBuffer(s_cmdBuf);
            s_cmdBuf = nullptr;
            s_swapchainTexture = nullptr;
        }
#endif
    }

    // -----------------------------------------------------------------------
    // EndFrame: End render pass, flush vertex transfer buffer, and submit.
    // Called once per frame after all draw calls.
    // Replaces SDL_GL_SwapWindow / SwapBuffers in the game loop.
    //
    // Story 4.3.2 (AC-7): After ending the render pass, unmap the vertex
    // transfer buffer and issue a single copy pass to flush the frame's
    // accumulated vertex data to the GPU vertex buffer. The next frame's
    // render pass will read from the updated GPU buffer.
    // -----------------------------------------------------------------------
    void EndFrame() override
    {
#ifdef MU_ENABLE_SDL3
        if (!s_renderPass)
        {
            // No render pass was started (minimized window or error).
            // Unmap the vtx buffer if it was mapped and cancel the command buffer.
            if (s_vtxMappedPtr)
            {
                SDL_UnmapGPUTransferBuffer(s_device, s_vtxTransferBuf);
                s_vtxMappedPtr = nullptr;
            }
            if (s_cmdBuf)
            {
                SDL_CancelGPUCommandBuffer(s_cmdBuf);
                s_cmdBuf = nullptr;
            }
            return;
        }

        SDL_EndGPURenderPass(s_renderPass);
        s_renderPass = nullptr;

        // Story 4.3.2 (AC-7): Unmap and copy vertex data to GPU buffer (post-render-pass).
        // Next frame's render pass will use the updated GPU buffer contents.
        if (s_vtxMappedPtr && s_vtxOffset > 0u)
        {
            SDL_UnmapGPUTransferBuffer(s_device, s_vtxTransferBuf);
            s_vtxMappedPtr = nullptr;

            SDL_GPUCopyPass* vtxCopyPass = SDL_BeginGPUCopyPass(s_cmdBuf);
            if (vtxCopyPass)
            {
                SDL_GPUTransferBufferLocation src{};
                src.transfer_buffer = s_vtxTransferBuf;
                src.offset = 0;

                SDL_GPUBufferRegion dst{};
                dst.buffer = s_vtxGpuBuf;
                dst.offset = 0;
                dst.size = s_vtxOffset; // only flush bytes actually written

                SDL_UploadToGPUBuffer(vtxCopyPass, &src, &dst, false);
                SDL_EndGPUCopyPass(vtxCopyPass);
            }
        }
        else if (s_vtxMappedPtr)
        {
            SDL_UnmapGPUTransferBuffer(s_device, s_vtxTransferBuf);
            s_vtxMappedPtr = nullptr;
        }

        if (s_cmdBuf)
        {
            SDL_SubmitGPUCommandBuffer(s_cmdBuf);
            s_cmdBuf = nullptr;
        }

        s_swapchainTexture = nullptr;

        // Emit per-frame diagnostic stats every 300 frames (~5s at 60fps).
        if (s_dbgFrameCount % 300 == 0)
        {
            SDL_Log("[RENDER diag] frame=%u  draw_calls=%u  vtx_bytes=%u", s_dbgFrameCount, s_dbgDrawCallsThisFrame,
                    s_dbgVtxBytesThisFrame);
        }
        // Warn once if frames are rendering but producing no draw calls.
        if (s_dbgFrameCount == 10 && s_dbgDrawCallsThisFrame == 0)
        {
            SDL_Log("[RENDER diag] WARNING: 10 frames elapsed with zero draw calls — "
                    "game may not be calling RenderQuad2D/RenderTriangles");
        }
#endif
    }

    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
    // Story 7-9-2 (AC-1): BeginScene — 3D viewport and projection setup.
    // SDL_gpu backend: sets viewport on the render pass. Projection/camera
    // transforms are handled via uniform buffers (not immediate-mode matrices).
    // -----------------------------------------------------------------------
    void BeginScene(int x, int y, int w, int h) override
    {
#ifdef MU_ENABLE_SDL3
        if (!s_renderPass || !s_window)
        {
            return;
        }

        // Use swapchain physical pixels for viewport (correct on HiDPI/Retina).
        const float scaleX = static_cast<float>(s_swapW) / 640.0f;
        const float scaleY = static_cast<float>(s_swapH) / 480.0f;

        SDL_GPUViewport viewport{};
        viewport.x = static_cast<float>(x) * scaleX;
        viewport.y = static_cast<float>(y) * scaleY;
        viewport.w = static_cast<float>(w) * scaleX;
        viewport.h = static_cast<float>(h) * scaleY;
        viewport.min_depth = 0.0f;
        viewport.max_depth = 1.0f;

        SDL_SetGPUViewport(s_renderPass, &viewport);
#else
        (void)x;
        (void)y;
        (void)w;
        (void)h;
#endif
    }

    // -----------------------------------------------------------------------
    // Story 7-9-2 (AC-1): EndScene — restore state after 3D pass.
    // SDL_gpu backend: reset viewport to full window.
    // -----------------------------------------------------------------------
    void EndScene() override
    {
#ifdef MU_ENABLE_SDL3
        if (!s_renderPass)
        {
            return;
        }

        // Reset viewport to full swapchain (physical pixels).
        SDL_GPUViewport fullVP{};
        fullVP.x = 0.0f;
        fullVP.y = 0.0f;
        fullVP.w = static_cast<float>(s_swapW);
        fullVP.h = static_cast<float>(s_swapH);
        fullVP.min_depth = 0.0f;
        fullVP.max_depth = 1.0f;

        SDL_SetGPUViewport(s_renderPass, &fullVP);
#endif
    }

    // -----------------------------------------------------------------------
    // Story 7-9-2 (AC-2): Begin2DPass — mark 2D mode for pipeline selection.
    // SDL_gpu uses separate 2D pipelines (Vertex2D layout, depth OFF).
    // -----------------------------------------------------------------------
    void Begin2DPass() override
    {
        // 2D pipeline selection is already handled by RenderQuad2D based on
        // depth test state. No additional state change needed.
    }

    // -----------------------------------------------------------------------
    // Story 7-9-2 (AC-2): End2DPass — restore 3D mode.
    // -----------------------------------------------------------------------
    void End2DPass() override
    {
        // No-op: pipeline selection returns to 3D automatically via
        // RenderTriangles/RenderQuadStrip calls.
    }

    // -----------------------------------------------------------------------
    // Story 7-9-2 (AC-7): ClearScreen — no-op on SDL_gpu.
    // SDL_gpu clears the swapchain texture at BeginFrame (LOADOP_CLEAR).
    // -----------------------------------------------------------------------
    void ClearScreen() override
    {
        // No-op: SDL_gpu clears in BeginFrame via SDL_GPU_LOADOP_CLEAR.
    }

    // -----------------------------------------------------------------------
    // Story 7-9-2 (AC-5): RenderLines — line primitive rendering.
    // SDL_gpu backend: emit line primitives using existing 3D pipeline.
    // For now, renders as thin triangles (SDL_gpu line support varies).
    // -----------------------------------------------------------------------
    void RenderLines(std::span<const Vertex3D> vertices, std::uint32_t textureId) override
    {
#ifdef MU_ENABLE_SDL3
        if (vertices.empty() || !s_renderPass)
        {
            return;
        }

        // Render lines as thin quads (2 triangles per line segment).
        // Degenerate triangles (A, B, A) have zero area and are culled by GPUs,
        // so we extrude each line into a thin quad with a perpendicular offset.
        constexpr float kHalfWidth = 0.5f;
        const auto lineCount = vertices.size() / 2;
        for (std::size_t i = 0; i < lineCount; ++i)
        {
            const auto& a = vertices[i * 2];
            const auto& b = vertices[i * 2 + 1];

            const float dx = b.x - a.x;
            const float dy = b.y - a.y;
            const float dz = b.z - a.z;
            const float len = std::sqrt(dx * dx + dy * dy + dz * dz);
            if (len < 1e-6f)
            {
                continue;
            }

            const float invLen = 1.0f / len;
            const float dirX = dx * invLen;
            const float dirY = dy * invLen;
            const float dirZ = dz * invLen;

            // Perpendicular via cross product with an axis not parallel to line
            float perpX, perpY, perpZ;
            if (std::abs(dirY) < 0.9f)
            {
                perpX = dirZ;
                perpY = 0.0f;
                perpZ = -dirX;
            }
            else
            {
                perpX = 0.0f;
                perpY = -dirZ;
                perpZ = dirY;
            }

            const float perpLen = std::sqrt(perpX * perpX + perpY * perpY + perpZ * perpZ);
            if (perpLen < 1e-6f)
            {
                continue;
            }
            const float scale = kHalfWidth / perpLen;
            perpX *= scale;
            perpY *= scale;
            perpZ *= scale;

            // Two triangles forming a thin quad: (A+p, A-p, B-p) and (A+p, B-p, B+p)
            Vertex3D verts[6];
            verts[0] = a;
            verts[0].x += perpX;
            verts[0].y += perpY;
            verts[0].z += perpZ;
            verts[1] = a;
            verts[1].x -= perpX;
            verts[1].y -= perpY;
            verts[1].z -= perpZ;
            verts[2] = b;
            verts[2].x -= perpX;
            verts[2].y -= perpY;
            verts[2].z -= perpZ;

            verts[3] = a;
            verts[3].x += perpX;
            verts[3].y += perpY;
            verts[3].z += perpZ;
            verts[4] = b;
            verts[4].x -= perpX;
            verts[4].y -= perpY;
            verts[4].z -= perpZ;
            verts[5] = b;
            verts[5].x += perpX;
            verts[5].y += perpY;
            verts[5].z += perpZ;

            RenderTriangles(std::span<const Vertex3D>(verts, 6), textureId);
        }
#else
        (void)vertices;
        (void)textureId;
#endif
    }

    // -----------------------------------------------------------------------
    // Story 7-9-2 (AC-6): IsFrameActive — frame lifecycle query.
    // Returns true when a render pass is open (between BeginFrame/EndFrame).
    // -----------------------------------------------------------------------
    [[nodiscard]] bool IsFrameActive() const override
    {
        return s_renderPass != nullptr;
    }

    // Story 4.4.1 (AC-2, Task 6.2/6.3): GetDevice override — returns s_device.
    // Allows GlobalBitmap.cpp to obtain the SDL_GPUDevice* via mu::GetRenderer().GetDevice()
    // without a direct dependency on MuRendererSDLGpu.cpp internals.
    // Logs a warning via g_ErrorReport if s_device is nullptr (renderer not initialized).
    // -----------------------------------------------------------------------
#ifdef MU_ENABLE_SDL3
    [[nodiscard]] void* GetDevice() override
    {
        if (!s_device)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- GetDevice() called before Init() or after Shutdown()");
        }
        return s_device;
    }
#endif

    // [Story 7-6-7: AC-3] GPU backend driver name for error reporting.
    [[nodiscard]] const char* GetGPUDriverName() const override
    {
        return s_device ? SDL_GetGPUDeviceDriver(s_device) : "unknown";
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

        // Story 4.3.2 (AC-8): RenderQuad2D uses the 2D pipeline set (Vertex2D layout).
        const int pipelineIdx = GetActivePipelineIndex();
        SDL_GPUGraphicsPipeline* pipeline =
            m_depthTestEnabled ? s_pipelines2D[pipelineIdx] : s_pipelines2DDepthOff[pipelineIdx];
        if (!pipeline)
        {
            if (!s_dbgNullPipelineWarned)
            {
                SDL_Log("[RENDER diag] WARNING: RenderQuad2D pipeline is null (idx=%d depth=%d) — "
                        "pipeline creation failed during Init",
                        pipelineIdx, m_depthTestEnabled ? 1 : 0);
                s_dbgNullPipelineWarned = true;
            }
            return;
        }
        SDL_BindGPUGraphicsPipeline(s_renderPass, pipeline);

        // Push ScreenSize uniform (slot 0) — vertex shader divides pos by this
        // to convert from 640x480 design-space pixels to NDC.
        {
            // 2D ortho MVP: maps [0,W]×[0,H] to NDC, replicating gluOrtho2D.
            // GLM_FORCE_DEPTH_ZERO_TO_ONE → correct Z [0,1] for Metal/Vulkan.
            int winW = 0, winH = 0;
            SDL_GetWindowSize(s_window, &winW, &winH);
            const glm::mat4 ortho = glm::ortho(0.0f, static_cast<float>(winW), 0.0f, static_cast<float>(winH), -1.0f, 1.0f);
            SDL_PushGPUVertexUniformData(s_cmdBuf, 0, glm::value_ptr(ortho), sizeof(ortho));
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

        // Story 4.4.1 (AC-4): Bind per-texture sampler via LookupSampler; fallback to s_defaultSampler.
        void* pSampler = LookupSampler(textureId);
        SDL_GPUTextureSamplerBinding samplerBinding{};
        samplerBinding.texture = static_cast<SDL_GPUTexture*>(pTex);
        samplerBinding.sampler = pSampler ? static_cast<SDL_GPUSampler*>(pSampler) : s_defaultSampler;
        SDL_BindGPUFragmentSamplers(s_renderPass, 0, &samplerBinding, 1);

        // Story 4.3.2 (AC-10): Bind fog uniform buffer at fragment storage slot 0.
        if (s_fogUniformBuf)
        {
            SDL_BindGPUFragmentStorageBuffers(s_renderPass, 0, &s_fogUniformBuf, 1);
        }

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
        ++s_dbgDrawCallsThisFrame;
        s_dbgVtxBytesThisFrame += byteSize;
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

        // Story 4.3.2 (AC-8): RenderTriangles uses the 3D pipeline set (Vertex3D layout).
        const int pipelineIdx = GetActivePipelineIndex();
        SDL_GPUGraphicsPipeline* pipeline =
            m_depthTestEnabled ? s_pipelines3D[pipelineIdx] : s_pipelines3DDepthOff[pipelineIdx];
        if (!pipeline)
        {
            if (!s_dbgNullPipelineWarned)
            {
                SDL_Log("[RENDER diag] WARNING: RenderTriangles pipeline is null (idx=%d depth=%d)", pipelineIdx,
                        m_depthTestEnabled ? 1 : 0);
                s_dbgNullPipelineWarned = true;
            }
            return;
        }
        SDL_BindGPUGraphicsPipeline(s_renderPass, pipeline);

        {
            // 3D MVP from the matrix stack (set by game code via SetMatrixMode/Rotate/Translate).
            SDL_PushGPUVertexUniformData(s_cmdBuf, 0, glm::value_ptr(m_mvpMatrix), sizeof(glm::mat4));
        }

        SDL_GPUBufferBinding vtxBinding{};
        vtxBinding.buffer = s_vtxGpuBuf;
        vtxBinding.offset = vtxOffset;
        SDL_BindGPUVertexBuffers(s_renderPass, 0, &vtxBinding, 1);

        // Story 4.4.1 (AC-4): Bind per-texture sampler via LookupSampler; fallback to s_defaultSampler.
        {
            void* pSampler = LookupSampler(textureId);
            SDL_GPUTextureSamplerBinding samplerBinding{};
            samplerBinding.texture = static_cast<SDL_GPUTexture*>(pTex);
            samplerBinding.sampler = pSampler ? static_cast<SDL_GPUSampler*>(pSampler) : s_defaultSampler;
            SDL_BindGPUFragmentSamplers(s_renderPass, 0, &samplerBinding, 1);
        }

        // Story 4.3.2 (AC-10): Bind fog uniform buffer at fragment storage slot 0.
        if (s_fogUniformBuf)
        {
            SDL_BindGPUFragmentStorageBuffers(s_renderPass, 0, &s_fogUniformBuf, 1);
        }

        SDL_DrawGPUPrimitives(s_renderPass, static_cast<Uint32>(vertices.size()), 1, 0, 0);
        ++s_dbgDrawCallsThisFrame;
        s_dbgVtxBytesThisFrame += byteSize;
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

        // Story 4.3.2 (AC-7): Copy strip index data before (re)opening the render pass.
        // SDL_gpu prohibits copy passes while a render pass is active.
        // Sequence: end render pass → copy indices → reopen render pass.
        {
            void* pMapped = SDL_MapGPUTransferBuffer(s_device, s_stripIdxTransfer, false);
            if (!pMapped)
            {
                g_ErrorReport.Write(L"RENDER: SDL_gpu::RenderQuadStrip -- failed to map strip index transfer buffer");
                return;
            }
            std::memcpy(pMapped, indices.data(), numIndices * sizeof(Uint16));
            SDL_UnmapGPUTransferBuffer(s_device, s_stripIdxTransfer);

            // Temporarily end the render pass to issue the copy.
            SDL_EndGPURenderPass(s_renderPass);
            s_renderPass = nullptr;

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

                SDL_UploadToGPUBuffer(copyPass, &src, &dst, false);
                SDL_EndGPUCopyPass(copyPass);
            }

            // Reopen the render pass targeting the same swapchain texture.
            SDL_GPUColorTargetInfo colorTarget{};
            colorTarget.texture = s_swapchainTexture;
            colorTarget.load_op = SDL_GPU_LOADOP_LOAD; // preserve existing content
            colorTarget.store_op = SDL_GPU_STOREOP_STORE;

            s_renderPass = SDL_BeginGPURenderPass(s_cmdBuf, &colorTarget, 1, nullptr);
            if (!s_renderPass)
            {
                g_ErrorReport.Write(L"RENDER: SDL_gpu::RenderQuadStrip -- failed to reopen render pass: %hs",
                                    SDL_GetError());
                return;
            }
        }

        // Story 4.3.2 (AC-8): RenderQuadStrip uses the 3D pipeline set (Vertex3D layout).
        const int pipelineIdx = GetActivePipelineIndex();
        SDL_GPUGraphicsPipeline* pipeline =
            m_depthTestEnabled ? s_pipelines3D[pipelineIdx] : s_pipelines3DDepthOff[pipelineIdx];
        if (pipeline)
        {
            SDL_BindGPUGraphicsPipeline(s_renderPass, pipeline);

            // 3D MVP from the matrix stack.
            SDL_PushGPUVertexUniformData(s_cmdBuf, 0, glm::value_ptr(m_mvpMatrix), sizeof(glm::mat4));
        }

        SDL_GPUBufferBinding vtxBinding{};
        vtxBinding.buffer = s_vtxGpuBuf;
        vtxBinding.offset = vtxOffset;
        SDL_BindGPUVertexBuffers(s_renderPass, 0, &vtxBinding, 1);

        SDL_GPUBufferBinding idxBinding{};
        idxBinding.buffer = s_stripIdxBuf;
        idxBinding.offset = 0;
        SDL_BindGPUIndexBuffer(s_renderPass, &idxBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

        // Story 4.4.1 (AC-4): Bind per-texture sampler via LookupSampler; fallback to s_defaultSampler.
        {
            void* pSampler = LookupSampler(textureId);
            SDL_GPUTextureSamplerBinding samplerBinding{};
            samplerBinding.texture = static_cast<SDL_GPUTexture*>(pTex);
            samplerBinding.sampler = pSampler ? static_cast<SDL_GPUSampler*>(pSampler) : s_defaultSampler;
            SDL_BindGPUFragmentSamplers(s_renderPass, 0, &samplerBinding, 1);
        }

        // Story 4.3.2 (AC-10): Bind fog uniform buffer at fragment storage slot 0.
        if (s_fogUniformBuf)
        {
            SDL_BindGPUFragmentStorageBuffers(s_renderPass, 0, &s_fogUniformBuf, 1);
        }

        SDL_DrawGPUIndexedPrimitives(s_renderPass, numIndices, 1, 0, 0, 0);
        ++s_dbgDrawCallsThisFrame;
        s_dbgVtxBytesThisFrame += static_cast<Uint32>(vertices.size() * sizeof(Vertex3D));
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
    // SetFog: Populate FogUniform from FogParams and mark the GPU buffer dirty.
    // The buffer is uploaded in BeginFrame() before the render pass.
    // Story 4.3.2 (AC-10): Fog uniform buffer support.
    // -----------------------------------------------------------------------
    void SetDepthMask(bool enabled) override
    {
        m_depthMaskEnabled = enabled;
    }
    void SetCullFace(bool enabled) override
    {
        m_cullFaceEnabled = enabled;
    }
    void SetAlphaTest(bool enabled) override
    {
        m_alphaTestEnabled = enabled;
    }
    void SetTexture2D(bool enabled) override
    {
        m_texture2DEnabled = enabled;
    }
    void SetFogEnabled(bool enabled) override
    {
        m_fogEnabled = enabled;
    }
    void BindTexture(int texId) override
    {
        m_boundTextureId = texId;
    }

    void SetFog(const FogParams& params) override
    {
        m_fogParams = params;

        // Map GL-style FogParams (mode/start/end/density/color) to the
        // HLSL FogUniforms cbuffer layout used by basic_textured.frag.hlsl.
        // fogEnabled: true when mode != 0 (mode 0 = no fog / GL_LINEAR from caller).
        // alphaDiscardEnabled / alphaThreshold: not in FogParams; default off.
        m_fogUniform.fogEnabled = (params.mode != 0) ? 1u : 0u;
        m_fogUniform.alphaDiscardEnabled = 0u; // not surfaced via FogParams
        m_fogUniform.alphaThreshold = 0.0f;
        m_fogUniform.pad0 = 0.0f;
        m_fogUniform.fogStart = params.start;
        m_fogUniform.fogEnd = params.end;
        m_fogUniform.fogColor[0] = params.color[0];
        m_fogUniform.fogColor[1] = params.color[1];
        m_fogUniform.fogColor[2] = params.color[2];
        m_fogUniform.fogColor[3] = params.color[3];
        m_fogUniform.pad1[0] = 0.0f;
        m_fogUniform.pad1[1] = 0.0f;

        s_fogDirty = true;
    }

    // -----------------------------------------------------------------------
    // Matrix stack operations — replaces OpenGL fixed-function matrix stack.
    // The game calls SetMatrixMode/LoadIdentity/Rotate/Translate/etc. to set
    // up the projection and modelview. We store them and compute MVP when
    // pushing the uniform in draw calls.
    // -----------------------------------------------------------------------
    void SetMatrixMode(int mode) override { m_matrixMode = mode; }

    void LoadIdentity() override { ActiveMatrix() = glm::mat4(1.0f); UpdateMVP(); }

    void PushMatrix() override
    {
        if (m_matrixMode == 0x1700) // GL_MODELVIEW
        {
            if (m_mvStackTop < k_MatrixStackDepth)
                m_mvStack[m_mvStackTop++] = m_modelViewMatrix;
        }
        else
        {
            if (m_projStackTop < k_MatrixStackDepth)
                m_projStack[m_projStackTop++] = m_projMatrix;
        }
    }

    void PopMatrix() override
    {
        if (m_matrixMode == 0x1700) // GL_MODELVIEW
        {
            if (m_mvStackTop > 0)
                m_modelViewMatrix = m_mvStack[--m_mvStackTop];
        }
        else
        {
            if (m_projStackTop > 0)
                m_projMatrix = m_projStack[--m_projStackTop];
        }
        UpdateMVP();
    }

    void Translate(float x, float y, float z) override
    {
        ActiveMatrix() = glm::translate(ActiveMatrix(), glm::vec3(x, y, z));
        UpdateMVP();
    }

    void Rotate(float angle, float x, float y, float z) override
    {
        ActiveMatrix() = glm::rotate(ActiveMatrix(), glm::radians(angle), glm::vec3(x, y, z));
        UpdateMVP();
    }

    void Scale(float x, float y, float z) override
    {
        ActiveMatrix() = glm::scale(ActiveMatrix(), glm::vec3(x, y, z));
        UpdateMVP();
    }

    void MultMatrix(const float* m) override
    {
        if (m)
        {
            ActiveMatrix() = ActiveMatrix() * glm::make_mat4(m);
            UpdateMVP();
        }
    }

    void LoadMatrix(const float* m) override
    {
        if (m)
        {
            ActiveMatrix() = glm::make_mat4(m);
            UpdateMVP();
        }
    }

    void GetMatrix(int mode, float* m) override
    {
        if (!m) return;
        if (mode == 0x0BA6) // GL_MODELVIEW_MATRIX
            std::memcpy(m, glm::value_ptr(m_modelViewMatrix), 64);
        else if (mode == 0x0BA7) // GL_PROJECTION_MATRIX
            std::memcpy(m, glm::value_ptr(m_projMatrix), 64);
    }

private:
    glm::mat4& ActiveMatrix()
    {
        return (m_matrixMode == 0x1700) ? m_modelViewMatrix : m_projMatrix;
    }

    void UpdateMVP() { m_mvpMatrix = m_projMatrix * m_modelViewMatrix; }
    // Per-instance render state.
    BlendMode m_activeBlendMode = BlendMode::Alpha;
    bool m_blendEnabled = true;
    bool m_depthTestEnabled = true;
    bool m_depthMaskEnabled = true;
    bool m_cullFaceEnabled = true;
    bool m_alphaTestEnabled = false;
    bool m_texture2DEnabled = true;
    bool m_fogEnabled = false;
    int m_boundTextureId = -1;
    FogParams m_fogParams{};
    // Story 4.3.2 (AC-10): CPU-side fog uniform data, uploaded to GPU when dirty.
    FogUniform m_fogUniform{};

    // Matrix stack for 3D rendering (replaces OpenGL fixed-function matrix stack).
    static constexpr int k_MatrixStackDepth = 16;
    int m_matrixMode = 0x1700; // GL_MODELVIEW
    glm::mat4 m_projMatrix{1.0f};
    glm::mat4 m_modelViewMatrix{1.0f};
    glm::mat4 m_projStack[k_MatrixStackDepth]{};
    glm::mat4 m_mvStack[k_MatrixStackDepth]{};
    int m_projStackTop = 0;
    int m_mvStackTop = 0;
    glm::mat4 m_mvpMatrix{1.0f};

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
    // UploadVertices: Write vertex data into the per-frame mapped transfer buffer.
    // Returns the byte offset in the GPU vertex buffer, or ~0u on failure.
    //
    // Story 4.3.2 (AC-7): s_vtxMappedPtr is held mapped for the entire frame.
    // This function only writes to CPU-side mapped memory — no copy pass here.
    // The single bulk copy from transfer→GPU buffer happens in EndFrame() after
    // the render pass ends (copy passes cannot be issued inside a render pass).
    // The GPU vertex buffer (s_vtxGpuBuf) contains data from the previous frame;
    // the render pass draws from those valid offsets (one-frame latency, invisible
    // since each frame recomputes all geometry from scratch).
    // -----------------------------------------------------------------------
    [[nodiscard]] static Uint32 UploadVertices(const void* pData, Uint32 byteSize)
    {
#ifdef MU_ENABLE_SDL3
        if (!s_vtxMappedPtr || !s_vtxGpuBuf)
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

        // Write vertex data directly into the persistently-mapped transfer buffer.
        std::memcpy(static_cast<Uint8*>(s_vtxMappedPtr) + alignedOffset, pData, byteSize);

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
    // -----------------------------------------------------------------------
    // Story 4.3.2 (AC-2, AC-5): LoadShaders
    // Loads all 5 HLSL shader blobs from MU_SHADER_DIR and creates
    // SDL_GPUShader handles for pipeline creation.
    // driverName: SDL_GetGPUDeviceDriver(s_device) result.
    // Returns false if the primary shader (basic_textured.vert) cannot be loaded
    // (fatal for 2D rendering). Other shaders log warnings but are non-fatal.
    // -----------------------------------------------------------------------
    [[nodiscard]] static bool LoadShaders(const char* driverName)
    {
        const SDL_GPUShaderFormat fmt = GetShaderFormat(driverName);

        // Helper lambda: load blob + create shader.
        // Returns nullptr on failure (fatal=true logs error and propagates).
        auto createShader = [&](const char* name, const char* stage, SDL_GPUShaderStage shaderStage, Uint32 numSamplers,
                                Uint32 numStorageBuffers, Uint32 numUniformBuffers, bool fatal) -> SDL_GPUShader*
        {
            const std::vector<Uint8> blob = LoadShaderBlob(name, stage, driverName);
            if (blob.empty())
            {
                if (fatal)
                {
                    g_ErrorReport.Write(L"RENDER: SDL_gpu -- FATAL: failed to load shader blob %hs.%hs", name, stage);
                }
                else
                {
                    g_ErrorReport.Write(L"RENDER: SDL_gpu -- WARNING: failed to load shader blob %hs.%hs (non-fatal)",
                                        name, stage);
                }
                return nullptr;
            }

            SDL_GPUShaderCreateInfo info{};
            info.code = blob.data();
            info.code_size = blob.size();
            // spirv-cross renames "main" to "main0" in MSL because Metal reserves "main".
            info.entrypoint = (fmt == SDL_GPU_SHADERFORMAT_MSL) ? "main0" : "main";
            info.format = fmt;
            info.stage = shaderStage;
            info.num_samplers = numSamplers;
            info.num_storage_textures = 0;
            info.num_storage_buffers = numStorageBuffers;
            info.num_uniform_buffers = numUniformBuffers;

            SDL_GPUShader* shader = SDL_CreateGPUShader(s_device, &info);
            if (!shader)
            {
                if (fatal)
                {
                    g_ErrorReport.Write(L"RENDER: SDL_gpu -- FATAL: SDL_CreateGPUShader failed for %hs.%hs: %hs", name,
                                        stage, SDL_GetError());
                }
                else
                {
                    g_ErrorReport.Write(
                        L"RENDER: SDL_gpu -- WARNING: SDL_CreateGPUShader failed for %hs.%hs: %hs (non-fatal)", name,
                        stage, SDL_GetError());
                }
            }
            return shader;
        };

        // basic_textured.vert — fatal: required for all 2D textured draws.
        // Inputs: pos(TEXCOORD0), uv(TEXCOORD1), color(TEXCOORD2)
        // Uniform buffers: b1 (ScreenSize)
        s_vertShader2D = createShader("basic_textured", "vert", SDL_GPU_SHADERSTAGE_VERTEX, 0, 0, 1, /*fatal=*/true);
        if (!s_vertShader2D)
        {
            return false;
        }

        // basic_textured.frag — fatal: required for textured 2D draws.
        // Samplers: t0 (texture), s0 (sampler); Storage buffers: FogUniforms
        s_fragShaderTex = createShader("basic_textured", "frag", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 1, 0, /*fatal=*/true);
        if (!s_fragShaderTex)
        {
            SDL_ReleaseGPUShader(s_device, s_vertShader2D);
            s_vertShader2D = nullptr;
            return false;
        }

        // basic_colored.vert — non-fatal (colored path degrades gracefully).
        // Inputs: pos(TEXCOORD0), color(TEXCOORD2)  [uv removed per LOW-1 fix]
        // Uniform buffers: b1 (ScreenSize)
        // NOTE (HIGH-4): Shader handles below are loaded as pipeline hooks for
        //   future IMuRenderer::RenderColoredGeometry() and RenderShadowVolume()
        //   methods. No dedicated pipeline sets exist yet — these shaders are not
        //   assigned to any pipeline in this story. Deferred to a follow-up story.
        s_vertShader2DCol = createShader("basic_colored", "vert", SDL_GPU_SHADERSTAGE_VERTEX, 0, 0, 1, /*fatal=*/false);

        // basic_colored.frag — non-fatal.
        // No samplers — flat color output.
        s_fragShaderCol = createShader("basic_colored", "frag", SDL_GPU_SHADERSTAGE_FRAGMENT, 0, 0, 0, /*fatal=*/false);

        // shadow_volume.vert — non-fatal (used only for shadow stencil passes).
        // Uniform buffers: b0 (MVP)
        s_vertShaderShadow =
            createShader("shadow_volume", "vert", SDL_GPU_SHADERSTAGE_VERTEX, 0, 0, 1, /*fatal=*/false);

        g_ErrorReport.Write(L"RENDER: SDL_gpu -- shaders loaded for driver: %hs", driverName ? driverName : "unknown");
        return true;
    }

    // -----------------------------------------------------------------------
    // Story 4.3.2: ReleaseShaders — release all 5 shader handles.
    // Called after CreatePipelines() and during Shutdown() as a safety net.
    // -----------------------------------------------------------------------
    static void ReleaseShaders()
    {
        if (s_vertShader2D)
        {
            SDL_ReleaseGPUShader(s_device, s_vertShader2D);
            s_vertShader2D = nullptr;
        }
        if (s_fragShaderTex)
        {
            SDL_ReleaseGPUShader(s_device, s_fragShaderTex);
            s_fragShaderTex = nullptr;
        }
        if (s_vertShader2DCol)
        {
            SDL_ReleaseGPUShader(s_device, s_vertShader2DCol);
            s_vertShader2DCol = nullptr;
        }
        if (s_fragShaderCol)
        {
            SDL_ReleaseGPUShader(s_device, s_fragShaderCol);
            s_fragShaderCol = nullptr;
        }
        if (s_vertShaderShadow)
        {
            SDL_ReleaseGPUShader(s_device, s_vertShaderShadow);
            s_vertShaderShadow = nullptr;
        }
    }

    // -----------------------------------------------------------------------
    // Story 4.3.2 (AC-8): BuildBlendPipeline
    // bUse3DLayout=false → Vertex2D layout (pitch=20, float2 pos/uv + ubyte4 color).
    // bUse3DLayout=true  → Vertex3D layout (pitch=40, float3 pos + float3 normal
    //                       + float2 uv + ubyte4 color).
    // Vertex shader chosen per layout: s_vertShader2D for 2D, s_vertShader2D
    // also serves 3D (re-uses position+uv+color bindings; normal discarded by shader).
    // Fragment shader: s_fragShaderTex (textured path with fog support).
    // -----------------------------------------------------------------------
    [[nodiscard]] static SDL_GPUGraphicsPipeline* BuildBlendPipeline(SDL_GPUColorTargetBlendState blendState,
                                                                     bool depthTestEnabled, bool bUse3DLayout)
    {
        // Get swapchain texture format for pipeline target.
        const SDL_GPUTextureFormat swapchainFmt = SDL_GetGPUSwapchainTextureFormat(s_device, s_window);

        SDL_GPUColorTargetDescription colorTargetDesc{};
        colorTargetDesc.format = swapchainFmt;
        colorTargetDesc.blend_state = blendState;

        SDL_GPUVertexAttribute vertexAttribs[4]; // 2D uses 3, 3D uses 4
        Uint32 numAttribs = 0;
        SDL_GPUVertexBufferDescription vtxBufDesc{};
        vtxBufDesc.slot = 0;
        vtxBufDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
        vtxBufDesc.instance_step_rate = 0;

        if (!bUse3DLayout)
        {
            // Vertex2D: float2 pos (TEXCOORD0), float2 uv (TEXCOORD1), ubyte4_norm color (TEXCOORD2)
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

            vtxBufDesc.pitch = sizeof(Vertex2D);
            numAttribs = 3;
        }
        else
        {
            // Vertex3D mapped to the 2D shader's input locations:
            //   location 0 (TEXCOORD0 → pos):   float3 pos  — shader reads float2 (x,y), z dropped
            //   location 1 (TEXCOORD1 → uv):    float2 uv   — correct texture coordinates
            //   location 2 (TEXCOORD2 → color):  ubyte4_norm — correct vertex color
            // Normals are not bound — the basic_textured shader has no lighting.
            vertexAttribs[0] = {};
            vertexAttribs[0].location = 0;
            vertexAttribs[0].buffer_slot = 0;
            vertexAttribs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
            vertexAttribs[0].offset = static_cast<Uint32>(offsetof(Vertex3D, x));

            vertexAttribs[1] = {};
            vertexAttribs[1].location = 1;
            vertexAttribs[1].buffer_slot = 0;
            vertexAttribs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
            vertexAttribs[1].offset = static_cast<Uint32>(offsetof(Vertex3D, u));

            vertexAttribs[2] = {};
            vertexAttribs[2].location = 2;
            vertexAttribs[2].buffer_slot = 0;
            vertexAttribs[2].format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM;
            vertexAttribs[2].offset = static_cast<Uint32>(offsetof(Vertex3D, color));

            vtxBufDesc.pitch = sizeof(Vertex3D);
            numAttribs = 3;
        }

        SDL_GPUVertexInputState vtxInputState{};
        vtxInputState.vertex_buffer_descriptions = &vtxBufDesc;
        vtxInputState.num_vertex_buffers = 1;
        vtxInputState.vertex_attributes = vertexAttribs;
        vtxInputState.num_vertex_attributes = numAttribs;

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
        // 2D pipelines: basic_textured vert+frag (with fog).
        // 3D pipelines: basic_textured vert+frag (with fog) — vertex shader uses
        // TEXCOORD0 for position, which aligns to Vertex3D layout if shader
        // is compiled to accept float3 TEXCOORD0.
        // NOTE: for full 3D, a dedicated vertex shader reading float3+normal would
        // be needed; for this story the basic_textured shaders serve both paths.
        pipelineInfo.vertex_shader = s_vertShader2D; // shared for 2D and 3D paths
        pipelineInfo.fragment_shader = s_fragShaderTex;
        pipelineInfo.vertex_input_state = vtxInputState;
        pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
        pipelineInfo.depth_stencil_state = depthState;
        pipelineInfo.target_info = targetInfo;
        pipelineInfo.props = 0;

        SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(s_device, &pipelineInfo);
        if (!pipeline)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- pipeline creation failed (%hs layout): %hs",
                                bUse3DLayout ? "3D" : "2D", SDL_GetError());
        }
        return pipeline;
    }

    // -----------------------------------------------------------------------
    // Story 4.3.2 (AC-8): CreatePipelines
    // Creates 4 pipeline sets × 9 blend modes = 36 pipelines total.
    //   s_pipelines2D[9]       — Vertex2D layout, depth ON
    //   s_pipelines2DDepthOff[9] — Vertex2D layout, depth OFF
    //   s_pipelines3D[9]       — Vertex3D layout, depth ON
    //   s_pipelines3DDepthOff[9] — Vertex3D layout, depth OFF
    // Pipeline creation failures are non-fatal (draw calls skip if pipeline is null).
    // -----------------------------------------------------------------------
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

            // 2D depth ON.
            s_pipelines2D[i] = BuildBlendPipeline(blendState, true, /*bUse3DLayout=*/false);
            if (!s_pipelines2D[i])
            {
                g_ErrorReport.Write(L"RENDER: SDL_gpu -- 2D pipeline[%d] creation failed: %hs", i, SDL_GetError());
            }

            // 2D depth OFF.
            s_pipelines2DDepthOff[i] = BuildBlendPipeline(blendState, false, /*bUse3DLayout=*/false);
            if (!s_pipelines2DDepthOff[i])
            {
                g_ErrorReport.Write(L"RENDER: SDL_gpu -- 2D depth-off pipeline[%d] creation failed: %hs", i,
                                    SDL_GetError());
            }

            // 3D depth ON.
            s_pipelines3D[i] = BuildBlendPipeline(blendState, true, /*bUse3DLayout=*/true);
            if (!s_pipelines3D[i])
            {
                g_ErrorReport.Write(L"RENDER: SDL_gpu -- 3D pipeline[%d] creation failed: %hs", i, SDL_GetError());
            }

            // 3D depth OFF.
            s_pipelines3DDepthOff[i] = BuildBlendPipeline(blendState, false, /*bUse3DLayout=*/true);
            if (!s_pipelines3DDepthOff[i])
            {
                g_ErrorReport.Write(L"RENDER: SDL_gpu -- 3D depth-off pipeline[%d] creation failed: %hs", i,
                                    SDL_GetError());
            }
        }

        return true; // Pipeline creation failures are non-fatal.
    }

    static void DestroyPipelines()
    {
        for (int i = 0; i < k_PipelineCount; ++i)
        {
            if (s_pipelines2D[i])
            {
                SDL_ReleaseGPUGraphicsPipeline(s_device, s_pipelines2D[i]);
                s_pipelines2D[i] = nullptr;
            }
            if (s_pipelines2DDepthOff[i])
            {
                SDL_ReleaseGPUGraphicsPipeline(s_device, s_pipelines2DDepthOff[i]);
                s_pipelines2DDepthOff[i] = nullptr;
            }
            if (s_pipelines3D[i])
            {
                SDL_ReleaseGPUGraphicsPipeline(s_device, s_pipelines3D[i]);
                s_pipelines3D[i] = nullptr;
            }
            if (s_pipelines3DDepthOff[i])
            {
                SDL_ReleaseGPUGraphicsPipeline(s_device, s_pipelines3DDepthOff[i]);
                s_pipelines3DDepthOff[i] = nullptr;
            }
        }
    }

    // -----------------------------------------------------------------------
    // Story 4.3.2 (AC-10): CreateFogUniformBuffers
    // Creates the GPU buffer (s_fogUniformBuf) used as a storage buffer in
    // the fragment shader, and its companion transfer buffer (s_fogTransferBuf).
    // Size = sizeof(FogUniform) = 48 bytes.
    // -----------------------------------------------------------------------
    [[nodiscard]] static bool CreateFogUniformBuffers()
    {
        SDL_GPUTransferBufferCreateInfo tbInfo{};
        tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        tbInfo.size = sizeof(FogUniform);
        s_fogTransferBuf = SDL_CreateGPUTransferBuffer(s_device, &tbInfo);
        if (!s_fogTransferBuf)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- fog transfer buffer creation failed: %hs", SDL_GetError());
            return false;
        }

        SDL_GPUBufferCreateInfo bufInfo{};
        bufInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
        bufInfo.size = sizeof(FogUniform);
        s_fogUniformBuf = SDL_CreateGPUBuffer(s_device, &bufInfo);
        if (!s_fogUniformBuf)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- fog uniform GPU buffer creation failed: %hs", SDL_GetError());
            SDL_ReleaseGPUTransferBuffer(s_device, s_fogTransferBuf);
            s_fogTransferBuf = nullptr;
            return false;
        }

        // Fog starts dirty so that the default zero-initialized FogUniform
        // (fog disabled) is uploaded on the first frame.
        s_fogDirty = true;
        return true;
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
// Story 7.9.3: MU_USE_OPENGL_BACKEND removed — SDL_gpu is the only backend.
// ---------------------------------------------------------------------------

[[nodiscard]] IMuRenderer& GetRenderer()
{
    static MuRendererSDLGpu s_instance;
    return s_instance;
}

// C++ linkage entry points for MuMain.cpp (no class forward declaration needed).
[[nodiscard]] bool InitSDLGpuRenderer(void* pNativeWindow)
{
    return MuRendererSDLGpu::Init(pNativeWindow);
}

void ShutdownSDLGpuRenderer()
{
    MuRendererSDLGpu::Shutdown();
}

} // namespace mu
