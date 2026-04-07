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
//   - Deferred draw command recording: vertices collected during frame, copied to GPU
//     in EndFrame() BEFORE the render pass, then draw commands replayed (AC-7 fix).
//
// GUARD STRUCTURE:
//   Story 7.9.3: SDL_gpu is the only renderer backend (MuRenderer.cpp deleted).

// Include SDL3 GPU header only in this file — not exposed to game logic.
// MU_ENABLE_SDL3 is defined at CMake project scope; the preprocessor guard
// around these includes was redundant and has been removed (Story 7.6.2, AC-7).
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

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
// Note: GLM_FORCE_DEPTH_ZERO_TO_ONE is defined via target_compile_definitions in CMakeLists.txt
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

// Story 7.9.7 (AC-7): Vertex uniform layout matching cbuffer Transform in HLSL.
// Contains MVP matrix + fog params, pushed per-draw via SDL_PushGPUVertexUniformData.
struct VertexUniforms
{
    glm::mat4 mvp;
    float fogStart;
    float fogEnd;
    float fogPad[2]; // 16-byte alignment padding
};
static_assert(sizeof(VertexUniforms) == 80, "VertexUniforms must be 80 bytes");

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
// Story 7.9.7: Added DepthReadOnly variants (depth test ON, depth write OFF) for particles.
// s_pipelines2D: depth ON (test+write), Vertex2D layout (pitch=20).
// s_pipelines2DDepthOff: depth OFF, Vertex2D layout.
// s_pipelines3D: depth ON (test+write), Vertex3D layout (pitch=40).
// s_pipelines3DDepthOff: depth OFF, Vertex3D layout.
// s_pipelines3DDepthReadOnly: depth test ON, write OFF — for transparent/additive particles.
// Index matches BlendMode enum cast to int; index 8 = disabled (no-blend).
static SDL_GPUGraphicsPipeline* s_pipelines2D[k_PipelineCount] = {};
static SDL_GPUGraphicsPipeline* s_pipelines2DDepthOff[k_PipelineCount] = {};
static SDL_GPUGraphicsPipeline* s_pipelines3D[k_PipelineCount] = {};
static SDL_GPUGraphicsPipeline* s_pipelines3DDepthOff[k_PipelineCount] = {};
static SDL_GPUGraphicsPipeline* s_pipelines3DDepthReadOnly[k_PipelineCount] = {};

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

// ---------------------------------------------------------------------------
// Deferred draw command recording.
// Draw calls record RenderCmds during the frame; EndFrame replays them after
// copying vertex data to the GPU buffer. This eliminates the 1-frame vertex
// data delay that caused streak artifacts when vertex counts varied per frame.
// ---------------------------------------------------------------------------
enum class RenderCmdType : uint8_t
{
    SetViewport,
    DrawTriangles,      // non-indexed 3D (Vertex3D)
    DrawIndexedQuads2D, // indexed 2D with static quad index buffer (Vertex2D)
    DrawIndexedStrip,   // indexed 3D with per-frame strip indices (Vertex3D)
    DrawTriangles2D,    // Story 7.9.8: non-indexed 2D triangles (Vertex2D) for text atlas
};

struct RenderCmd
{
    RenderCmdType type;
    SDL_GPUGraphicsPipeline* pipeline;
    SDL_GPUTexture* texture;
    SDL_GPUSampler* sampler;
    Uint32 vtxOffset;
    Uint32 vtxCount;       // for DrawTriangles
    Uint32 idxCount;       // for DrawIndexed*
    Uint32 stripIdxOffset; // byte offset into strip index scratch buffer
    VertexUniforms vu;
    FogUniform fogUniform;
    SDL_GPUViewport viewport; // for SetViewport only
};

static std::vector<RenderCmd> s_renderCmds;
// True between BeginFrame/EndFrame — replaces s_renderPass as the "frame active" guard
// during the collection phase (render pass is only opened in EndFrame now).
static bool s_frameActive = false;
// CPU-side scratch buffer for quad strip indices accumulated during the frame.
// Copied to GPU in one shot in EndFrame before the render pass.
static std::vector<Uint16> s_stripIdxScratch;

// Default white 1×1 texture used for textureId==0 and unknown IDs.
static SDL_GPUTexture* s_whiteTexture = nullptr;

// Shader handles for basic_textured (2D path) and basic_colored — released after pipeline creation.
static SDL_GPUShader* s_vertShader2D = nullptr;     // basic_textured.vert
static SDL_GPUShader* s_fragShaderTex = nullptr;    // basic_textured.frag
static SDL_GPUShader* s_vertShader2DCol = nullptr;  // basic_colored.vert
static SDL_GPUShader* s_fragShaderCol = nullptr;    // basic_colored.frag
static SDL_GPUShader* s_vertShaderShadow = nullptr; // shadow_volume.vert

// Story 7.9.7 (AC-3): Depth buffer texture for correct 3D depth testing.
// Created in Init() at swapchain dimensions, recreated on window resize.
static SDL_GPUTexture* s_depthTexture = nullptr;
static Uint32 s_depthW = 0u;
static Uint32 s_depthH = 0u;

// Story 4.3.2 (AC-10): Fog uniform buffer and transfer buffer.
static SDL_GPUBuffer* s_fogUniformBuf = nullptr;
static SDL_GPUTransferBuffer* s_fogTransferBuf = nullptr;
static bool s_fogDirty = true; // upload on first draw if SetFog not called

// Story 7.9.8 (AC-2): SDL_ttf GPU text engine and font variants.
// s_textEngine: atlas-based text engine created after SDL_GPUDevice.
// s_ttfFont*: pre-loaded fonts for UI text rendering (normal, bold, big, fixed).
static TTF_TextEngine* s_textEngine = nullptr;
static TTF_Font* s_ttfFont = nullptr;      // normal (default)
static TTF_Font* s_ttfFontBold = nullptr;  // bold weight
static TTF_Font* s_ttfFontBig = nullptr;   // larger size, bold
static TTF_Font* s_ttfFontFixed = nullptr; // monospace
static constexpr float k_DefaultFontPtSize = 14.0f;
static constexpr float k_BigFontPtSize = 18.0f;

// F-7 fix: Cached window dimensions, updated once per frame in BeginFrame().
static int s_cachedWinW = 0;
static int s_cachedWinH = 0;

// Story 7.9.8 (AC-2): Discover a usable .ttf font file.
// Searches game directory first (via SDL_GetBasePath), then system font paths.
[[nodiscard]] static std::string FindFontPath()
{
    // 1. Game-bundled font (preferred). Use SDL_GetBasePath() for exe-relative resolution
    // instead of CWD-relative (F-6 fix).
    std::filesystem::path gameFontDir;
    const char* basePath = SDL_GetBasePath();
    if (basePath)
    {
        gameFontDir = std::filesystem::path(basePath) / "Data" / "Font";
    }
    else
    {
        gameFontDir = "Data/Font"; // fallback to CWD if SDL_GetBasePath fails
    }
    if (std::filesystem::exists(gameFontDir))
    {
        for (const auto& entry : std::filesystem::directory_iterator(gameFontDir))
        {
            auto ext = entry.path().extension();
            if (entry.is_regular_file() && (ext == ".ttf" || ext == ".otf" || ext == ".ttc"))
            {
                return entry.path().string();
            }
        }
    }

    // 2. System font fallback (platform-specific).
    static const char* const k_SystemFontPaths[] = {
#ifdef __APPLE__
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/Library/Fonts/Arial Unicode.ttf",
#elif defined(__linux__)
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
#else // Windows
        "C:\\Windows\\Fonts\\arial.ttf",
        "C:\\Windows\\Fonts\\segoeui.ttf",
#endif
    };

    for (const char* path : k_SystemFontPaths)
    {
        if (std::filesystem::exists(path))
        {
            return path;
        }
    }

    return {};
}

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
    // SDL_GPU_BLENDFACTOR enum (from SDL_gpu.h):
    //   INVALID=0, ZERO=1, ONE=2, SRC_COLOR=3, ONE_MINUS_SRC_COLOR=4,
    //   DST_COLOR=5, ONE_MINUS_DST_COLOR=6, SRC_ALPHA=7, ONE_MINUS_SRC_ALPHA=8,
    //   DST_ALPHA=9, ONE_MINUS_DST_ALPHA=10, CONSTANT_COLOR=11,
    //   ONE_MINUS_CONSTANT_COLOR=12, SRC_ALPHA_SATURATE=13
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
        return {7, 8}; // default to alpha blend
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

        // Story 7.9.7 (AC-3): Create initial depth texture at window size.
        // BeginFrame() will recreate it if the swapchain dimensions differ.
        {
            int winW = 0;
            int winH = 0;
            SDL_GetWindowSizeInPixels(s_window, &winW, &winH);
            if (winW > 0 && winH > 0)
            {
                CreateOrResizeDepthTexture(static_cast<Uint32>(winW), static_cast<Uint32>(winH));
            }
        }

        // Story 7.9.8 (AC-2): Initialize SDL_ttf GPU text engine.
        if (!TTF_Init())
        {
            g_ErrorReport.Write(L"RENDER: SDL_ttf -- TTF_Init failed: %hs", SDL_GetError());
            // Non-fatal: renderer works, text won't render via SDL_ttf.
        }
        else
        {
            s_textEngine = TTF_CreateGPUTextEngine(s_device);
            if (!s_textEngine)
            {
                g_ErrorReport.Write(L"RENDER: SDL_ttf -- TTF_CreateGPUTextEngine failed: %hs", SDL_GetError());
                TTF_Quit();
            }
            else
            {
                const std::string fontPath = FindFontPath();
                if (fontPath.empty())
                {
                    g_ErrorReport.Write(L"RENDER: SDL_ttf -- no .ttf font found (checked Data/Font/ and system paths)");
                }
                else
                {
                    s_ttfFont = TTF_OpenFont(fontPath.c_str(), k_DefaultFontPtSize);
                    if (!s_ttfFont)
                    {
                        g_ErrorReport.Write(L"RENDER: SDL_ttf -- TTF_OpenFont(\"%hs\") failed: %hs", fontPath.c_str(),
                                            SDL_GetError());
                    }
                    else
                    {
                        g_ErrorReport.Write(L"RENDER: SDL_ttf -- loaded font \"%hs\" at %.0f pt", fontPath.c_str(),
                                            k_DefaultFontPtSize);

                        // F-1 fix: Pre-load font variants for bold, big, and fixed-width text.
                        // All variants use the same .ttf file at different sizes.
                        // Known limitation (F-4): SDL_ttf 3.x doesn't support weight selection
                        // from a single .ttf, so bold appears identical to normal. To fix,
                        // bundle a separate bold .ttf (e.g., NotoSans-Bold.ttf) in a follow-up.
                        s_ttfFontBold = TTF_OpenFont(fontPath.c_str(), k_DefaultFontPtSize);
                        s_ttfFontBig = TTF_OpenFont(fontPath.c_str(), k_BigFontPtSize);
                        s_ttfFontFixed = TTF_OpenFont(fontPath.c_str(), k_DefaultFontPtSize);
                        // Log which variants loaded (non-fatal if some fail).
                        if (!s_ttfFontBold || !s_ttfFontBig || !s_ttfFontFixed)
                        {
                            g_ErrorReport.Write(
                                L"RENDER: SDL_ttf -- some font variants failed to load (bold=%d big=%d fixed=%d)",
                                s_ttfFontBold != nullptr, s_ttfFontBig != nullptr, s_ttfFontFixed != nullptr);
                        }

                        // Story 7.9.8 (AC-STD-NFR-1): Warm up glyph atlas with common glyphs.
                        // Forces FreeType rasterization + GPU atlas upload at init rather than
                        // incurring a latency spike on the first frame that renders text.
                        static constexpr const char* k_WarmupGlyphs =
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
                            "0123456789 !@#$%^&*()-_=+[]{}|;:',.<>?/~`\"\\";
                        TTF_Font* fontsToWarm[] = {s_ttfFont, s_ttfFontBold, s_ttfFontBig, s_ttfFontFixed};
                        for (TTF_Font* f : fontsToWarm)
                        {
                            if (!f)
                                continue;
                            TTF_Text* warmup = TTF_CreateText(s_textEngine, f, k_WarmupGlyphs, 0);
                            if (warmup)
                            {
                                TTF_GetGPUTextDrawData(warmup); // populate atlas; discard draw data
                                TTF_DestroyText(warmup);
                            }
                        }
                    }
                }
            }
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

        // Story 7.9.8 (AC-2): Destroy SDL_ttf resources before the GPU device.
        // Close font variants first, then default font, then engine.
        if (s_ttfFontFixed)
        {
            TTF_CloseFont(s_ttfFontFixed);
            s_ttfFontFixed = nullptr;
        }
        if (s_ttfFontBig)
        {
            TTF_CloseFont(s_ttfFontBig);
            s_ttfFontBig = nullptr;
        }
        if (s_ttfFontBold)
        {
            TTF_CloseFont(s_ttfFontBold);
            s_ttfFontBold = nullptr;
        }
        if (s_ttfFont)
        {
            TTF_CloseFont(s_ttfFont);
            s_ttfFont = nullptr;
        }
        if (s_textEngine)
        {
            TTF_DestroyGPUTextEngine(s_textEngine);
            s_textEngine = nullptr;
            TTF_Quit();
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

        // Story 7.9.7 (AC-3): Release depth texture.
        if (s_depthTexture)
        {
            SDL_ReleaseGPUTexture(s_device, s_depthTexture);
            s_depthTexture = nullptr;
            s_depthW = 0u;
            s_depthH = 0u;
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

        // F-7 fix: Cache window dimensions once per frame for text Y-flip.
        SDL_GetWindowSize(s_window, &s_cachedWinW, &s_cachedWinH);

        // Reset vertex scratch offset, deferred command list, and per-frame diagnostics.
        s_vtxOffset = 0u;
        s_renderCmds.clear();
        s_stripIdxScratch.clear();
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

        // Story 7.9.7: Fog/alpha uniform is now pushed per-draw-call via
        // SDL_PushGPUFragmentUniformData — no copy pass needed here.

        // Story 7.9.7 (AC-3): Ensure depth texture matches swapchain dimensions.
        // Recreates on first frame or when window is resized.
        CreateOrResizeDepthTexture(s_swapW, s_swapH);

        // Deferred rendering: do NOT begin the render pass here.
        // Draw calls record RenderCmds into s_renderCmds during the frame.
        // EndFrame will: copy vertex data → begin render pass → replay → end → submit.
        s_frameActive = true;
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
        if (!s_frameActive)
        {
            // Frame was not started (minimized window or error).
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
        s_frameActive = false;

        // ---------------------------------------------------------------
        // Phase 1: Unmap the vertex transfer buffer (done writing vertices).
        // ---------------------------------------------------------------
        if (s_vtxMappedPtr)
        {
            SDL_UnmapGPUTransferBuffer(s_device, s_vtxTransferBuf);
            s_vtxMappedPtr = nullptr;
        }

        // ---------------------------------------------------------------
        // Phase 2: Copy pass — upload vertex + strip index data to GPU.
        // This happens BEFORE the render pass so the GPU reads current-
        // frame data, eliminating the 1-frame vertex delay that caused
        // streak artifacts when vertex counts varied between frames.
        // ---------------------------------------------------------------
        bool stripIdxReady = false;
        if (!s_stripIdxScratch.empty())
        {
            // Ensure GPU-side strip index buffer is large enough BEFORE the copy pass
            // (EnsureStripIndexBuffer may release/create GPU resources).
            if (EnsureStripIndexBuffer(static_cast<Uint32>(s_stripIdxScratch.size())))
            {
                void* pIdxMapped = SDL_MapGPUTransferBuffer(s_device, s_stripIdxTransfer, false);
                if (pIdxMapped)
                {
                    const Uint32 totalIdxBytes = static_cast<Uint32>(s_stripIdxScratch.size() * sizeof(Uint16));
                    std::memcpy(pIdxMapped, s_stripIdxScratch.data(), totalIdxBytes);
                    SDL_UnmapGPUTransferBuffer(s_device, s_stripIdxTransfer);
                    stripIdxReady = true;
                }
            }
        }

        if (s_vtxOffset > 0u || stripIdxReady)
        {
            SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(s_cmdBuf);
            if (copyPass)
            {
                // Copy vertex data.
                if (s_vtxOffset > 0u)
                {
                    SDL_GPUTransferBufferLocation vtxSrc{};
                    vtxSrc.transfer_buffer = s_vtxTransferBuf;
                    vtxSrc.offset = 0;

                    SDL_GPUBufferRegion vtxDst{};
                    vtxDst.buffer = s_vtxGpuBuf;
                    vtxDst.offset = 0;
                    vtxDst.size = s_vtxOffset;

                    SDL_UploadToGPUBuffer(copyPass, &vtxSrc, &vtxDst, false);
                }

                // Copy accumulated strip index data (all strips for this frame).
                if (stripIdxReady)
                {
                    const Uint32 totalIdxBytes = static_cast<Uint32>(s_stripIdxScratch.size() * sizeof(Uint16));

                    SDL_GPUTransferBufferLocation idxSrc{};
                    idxSrc.transfer_buffer = s_stripIdxTransfer;
                    idxSrc.offset = 0;

                    SDL_GPUBufferRegion idxDst{};
                    idxDst.buffer = s_stripIdxBuf;
                    idxDst.offset = 0;
                    idxDst.size = totalIdxBytes;

                    SDL_UploadToGPUBuffer(copyPass, &idxSrc, &idxDst, false);
                }

                SDL_EndGPUCopyPass(copyPass);
            }
        }

        // ---------------------------------------------------------------
        // Phase 3: Render pass — replay all recorded draw commands.
        // The GPU vertex/index buffers now contain current-frame data.
        // ---------------------------------------------------------------
        {
            SDL_GPUColorTargetInfo colorTarget{};
            colorTarget.texture = s_swapchainTexture;
            colorTarget.clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
            colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
            colorTarget.store_op = SDL_GPU_STOREOP_STORE;

            SDL_GPUDepthStencilTargetInfo depthTarget{};
            depthTarget.texture = s_depthTexture;
            depthTarget.clear_depth = 1.0f;
            depthTarget.load_op = SDL_GPU_LOADOP_CLEAR;
            depthTarget.store_op = SDL_GPU_STOREOP_STORE;
            depthTarget.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
            depthTarget.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
            depthTarget.cycle = true;

            s_renderPass = SDL_BeginGPURenderPass(s_cmdBuf, &colorTarget, 1, s_depthTexture ? &depthTarget : nullptr);
        }

        if (s_renderPass)
        {
            // Replay all recorded draw commands in submission order.
            for (const auto& cmd : s_renderCmds)
            {
                switch (cmd.type)
                {
                case RenderCmdType::SetViewport:
                {
                    // const_cast: SDL_SetGPUViewport takes a non-const pointer but
                    // does not modify the viewport struct.
                    auto vp = cmd.viewport;
                    SDL_SetGPUViewport(s_renderPass, &vp);
                    break;
                }

                case RenderCmdType::DrawTriangles:
                {
                    SDL_BindGPUGraphicsPipeline(s_renderPass, cmd.pipeline);
                    SDL_PushGPUVertexUniformData(s_cmdBuf, 0, &cmd.vu, sizeof(VertexUniforms));

                    SDL_GPUBufferBinding vtxBind{};
                    vtxBind.buffer = s_vtxGpuBuf;
                    vtxBind.offset = cmd.vtxOffset;
                    SDL_BindGPUVertexBuffers(s_renderPass, 0, &vtxBind, 1);

                    SDL_GPUTextureSamplerBinding sampBind{};
                    sampBind.texture = cmd.texture;
                    sampBind.sampler = cmd.sampler;
                    SDL_BindGPUFragmentSamplers(s_renderPass, 0, &sampBind, 1);

                    SDL_PushGPUFragmentUniformData(s_cmdBuf, 0, &cmd.fogUniform, sizeof(FogUniform));
                    SDL_DrawGPUPrimitives(s_renderPass, cmd.vtxCount, 1, 0, 0);
                    break;
                }

                case RenderCmdType::DrawIndexedQuads2D:
                {
                    SDL_BindGPUGraphicsPipeline(s_renderPass, cmd.pipeline);
                    SDL_PushGPUVertexUniformData(s_cmdBuf, 0, &cmd.vu, sizeof(VertexUniforms));

                    SDL_GPUBufferBinding vtxBind{};
                    vtxBind.buffer = s_vtxGpuBuf;
                    vtxBind.offset = cmd.vtxOffset;
                    SDL_BindGPUVertexBuffers(s_renderPass, 0, &vtxBind, 1);

                    SDL_GPUBufferBinding idxBind{};
                    idxBind.buffer = s_quadIdxBuf;
                    idxBind.offset = 0;
                    SDL_BindGPUIndexBuffer(s_renderPass, &idxBind, SDL_GPU_INDEXELEMENTSIZE_16BIT);

                    SDL_GPUTextureSamplerBinding sampBind{};
                    sampBind.texture = cmd.texture;
                    sampBind.sampler = cmd.sampler;
                    SDL_BindGPUFragmentSamplers(s_renderPass, 0, &sampBind, 1);

                    SDL_PushGPUFragmentUniformData(s_cmdBuf, 0, &cmd.fogUniform, sizeof(FogUniform));
                    SDL_DrawGPUIndexedPrimitives(s_renderPass, cmd.idxCount, 1, 0, 0, 0);
                    break;
                }

                case RenderCmdType::DrawIndexedStrip:
                {
                    SDL_BindGPUGraphicsPipeline(s_renderPass, cmd.pipeline);
                    SDL_PushGPUVertexUniformData(s_cmdBuf, 0, &cmd.vu, sizeof(VertexUniforms));

                    SDL_GPUBufferBinding vtxBind{};
                    vtxBind.buffer = s_vtxGpuBuf;
                    vtxBind.offset = cmd.vtxOffset;
                    SDL_BindGPUVertexBuffers(s_renderPass, 0, &vtxBind, 1);

                    SDL_GPUBufferBinding idxBind{};
                    idxBind.buffer = s_stripIdxBuf;
                    idxBind.offset = cmd.stripIdxOffset;
                    SDL_BindGPUIndexBuffer(s_renderPass, &idxBind, SDL_GPU_INDEXELEMENTSIZE_16BIT);

                    SDL_GPUTextureSamplerBinding sampBind{};
                    sampBind.texture = cmd.texture;
                    sampBind.sampler = cmd.sampler;
                    SDL_BindGPUFragmentSamplers(s_renderPass, 0, &sampBind, 1);

                    SDL_PushGPUFragmentUniformData(s_cmdBuf, 0, &cmd.fogUniform, sizeof(FogUniform));
                    SDL_DrawGPUIndexedPrimitives(s_renderPass, cmd.idxCount, 1, 0, 0, 0);
                    break;
                }

                case RenderCmdType::DrawTriangles2D:
                {
                    // Story 7.9.8: Non-indexed 2D triangles for text atlas rendering.
                    // Uses the same Vertex2D layout as DrawIndexedQuads2D but draws non-indexed.
                    SDL_BindGPUGraphicsPipeline(s_renderPass, cmd.pipeline);
                    SDL_PushGPUVertexUniformData(s_cmdBuf, 0, &cmd.vu, sizeof(VertexUniforms));

                    SDL_GPUBufferBinding vtxBind{};
                    vtxBind.buffer = s_vtxGpuBuf;
                    vtxBind.offset = cmd.vtxOffset;
                    SDL_BindGPUVertexBuffers(s_renderPass, 0, &vtxBind, 1);

                    SDL_GPUTextureSamplerBinding sampBind{};
                    sampBind.texture = cmd.texture;
                    sampBind.sampler = cmd.sampler;
                    SDL_BindGPUFragmentSamplers(s_renderPass, 0, &sampBind, 1);

                    SDL_PushGPUFragmentUniformData(s_cmdBuf, 0, &cmd.fogUniform, sizeof(FogUniform));
                    SDL_DrawGPUPrimitives(s_renderPass, cmd.vtxCount, 1, 0, 0);
                    break;
                }
                } // switch
            } // for

            SDL_EndGPURenderPass(s_renderPass);
            s_renderPass = nullptr;
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
        if (!s_frameActive || !s_window)
        {
            return;
        }

        // Use swapchain physical pixels for viewport (correct on HiDPI/Retina).
        const float scaleX = static_cast<float>(s_swapW) / 640.0f;
        const float scaleY = static_cast<float>(s_swapH) / 480.0f;

        RenderCmd cmd{};
        cmd.type = RenderCmdType::SetViewport;
        cmd.viewport.x = static_cast<float>(x) * scaleX;
        cmd.viewport.y = static_cast<float>(y) * scaleY;
        cmd.viewport.w = static_cast<float>(w) * scaleX;
        cmd.viewport.h = static_cast<float>(h) * scaleY;
        cmd.viewport.min_depth = 0.0f;
        cmd.viewport.max_depth = 1.0f;
        s_renderCmds.push_back(cmd);
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
        if (!s_frameActive)
        {
            return;
        }

        // Reset viewport to full swapchain (physical pixels).
        RenderCmd cmd{};
        cmd.type = RenderCmdType::SetViewport;
        cmd.viewport.x = 0.0f;
        cmd.viewport.y = 0.0f;
        cmd.viewport.w = static_cast<float>(s_swapW);
        cmd.viewport.h = static_cast<float>(s_swapH);
        cmd.viewport.min_depth = 0.0f;
        cmd.viewport.max_depth = 1.0f;
        s_renderCmds.push_back(cmd);
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
        if (vertices.empty() || !s_frameActive)
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
        return s_frameActive;
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

    // Story 7.9.8 (AC-2): SDL_ttf text engine accessor.
    [[nodiscard]] TTF_TextEngine* GetTextEngine() override
    {
        return s_textEngine;
    }

    // Story 7.9.8 (AC-2): Default TTF font accessor.
    [[nodiscard]] TTF_Font* GetTtfFont() override
    {
        return s_ttfFont;
    }

    // F-1 fix: Font variant accessors for bold, big, and fixed-width text.
    [[nodiscard]] TTF_Font* GetTtfFontBold() override
    {
        return s_ttfFontBold ? s_ttfFontBold : s_ttfFont;
    }
    [[nodiscard]] TTF_Font* GetTtfFontBig() override
    {
        return s_ttfFontBig ? s_ttfFontBig : s_ttfFont;
    }
    [[nodiscard]] TTF_Font* GetTtfFontFixed() override
    {
        return s_ttfFontFixed ? s_ttfFontFixed : s_ttfFont;
    }

    // F-7 fix: Cached window dimensions accessor (updated per-frame in BeginFrame).
    [[nodiscard]] int GetCachedWindowHeight() override
    {
        return s_cachedWinH;
    }

    // Story 7.9.8 (AC-6): Submit text atlas triangles as deferred draw commands.
    void SubmitTextTriangles(std::span<const Vertex2D> vertices, void* atlasTexture, void* sampler = nullptr) override
    {
        if (vertices.empty() || !s_frameActive || !atlasTexture)
        {
            return;
        }

        const Uint32 byteSize = static_cast<Uint32>(vertices.size() * sizeof(Vertex2D));
        const Uint32 vtxOffset = UploadVertices(vertices.data(), byteSize);
        if (vtxOffset == ~0u)
        {
            return;
        }

        // Text always renders with alpha blending, depth off.
        const int pipelineIdx = static_cast<int>(BlendMode::Alpha);
        SDL_GPUGraphicsPipeline* pipeline = s_pipelines2DDepthOff[pipelineIdx];
        if (!pipeline)
        {
            return;
        }

        RenderCmd cmd{};
        cmd.type = RenderCmdType::DrawTriangles2D;
        cmd.pipeline = pipeline;
        cmd.texture = static_cast<SDL_GPUTexture*>(atlasTexture);
        cmd.sampler = sampler ? static_cast<SDL_GPUSampler*>(sampler) : s_defaultSampler;
        cmd.vtxOffset = vtxOffset;
        cmd.vtxCount = static_cast<Uint32>(vertices.size());
        cmd.fogUniform = m_fogUniform;
        // 2D ortho projection for text — Y-up to match SDL_ttf GPU convention.
        // SDL_ttf negates Y in the vertex data (see SDL_gpu_textengine.c: "In the GPU API
        // positive y-axis is upwards so the signs of the y-coords is reversed").
        // Vertex positions are in Y-up space with the text origin at (0,0).
        // drawX/drawY offset the text to the correct screen position.
        cmd.vu.mvp =
            glm::ortho(0.0f, static_cast<float>(s_cachedWinW), 0.0f, static_cast<float>(s_cachedWinH), -1.0f, 1.0f);
        s_renderCmds.push_back(cmd);

        ++s_dbgDrawCallsThisFrame;
        s_dbgVtxBytesThisFrame += byteSize;
    }

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
        if (vertices.empty() || !s_frameActive || !m_colorWriteEnabled || m_stencilTestEnabled)
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

        const Uint32 numQuads = static_cast<Uint32>(vertices.size() / 4);
        if (numQuads > static_cast<Uint32>(k_MaxQuads))
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu::RenderQuad2D -- numQuads %u exceeds k_MaxQuads %d; clamping draw",
                                numQuads, k_MaxQuads);
        }
        const Uint32 drawQuads =
            (numQuads <= static_cast<Uint32>(k_MaxQuads)) ? numQuads : static_cast<Uint32>(k_MaxQuads);

        void* pSampler = LookupSampler(textureId);

        // Record deferred draw command — replayed in EndFrame after vertex data is on the GPU.
        RenderCmd cmd{};
        cmd.type = RenderCmdType::DrawIndexedQuads2D;
        cmd.pipeline = pipeline;
        cmd.texture = static_cast<SDL_GPUTexture*>(pTex);
        cmd.sampler = pSampler ? static_cast<SDL_GPUSampler*>(pSampler) : s_defaultSampler;
        cmd.vtxOffset = vtxOffset;
        cmd.idxCount = drawQuads * 6;
        cmd.fogUniform = m_fogUniform;
        // 2D ortho MVP: maps [0,W]×[0,H] to NDC, replicating gluOrtho2D.
        // GLM_FORCE_DEPTH_ZERO_TO_ONE → correct Z [0,1] for Metal/Vulkan.
        // fogStart=fogEnd=0 → range=0 → vertex shader sets fogFactor=1.0 (no fog for 2D).
        int winW = 0, winH = 0;
        SDL_GetWindowSize(s_window, &winW, &winH);
        cmd.vu.mvp = glm::ortho(0.0f, static_cast<float>(winW), 0.0f, static_cast<float>(winH), -1.0f, 1.0f);
        s_renderCmds.push_back(cmd);

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
        if (vertices.empty() || !s_frameActive || !m_colorWriteEnabled || m_stencilTestEnabled)
        {
            return;
        }

        if (vertices.size() % 3 != 0)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu::RenderTriangles -- vertex count %zu not divisible by 3",
                                vertices.size());
            return;
        }

        const std::uint32_t resolvedTexId = ResolveTextureId(textureId);
        void* pTex = LookupTexture(resolvedTexId);
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
        // Force depth-read-only for additive/subtract blends — these produce transparent output
        // and must not write to the depth buffer (would occlude geometry behind them).
        const int pipelineIdx = GetActivePipelineIndex();
        SDL_GPUGraphicsPipeline* pipeline =
            m_depthTestEnabled
                ? (m_depthMaskEnabled ? s_pipelines3D[pipelineIdx] : s_pipelines3DDepthReadOnly[pipelineIdx])
                : s_pipelines3DDepthOff[pipelineIdx];
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

        void* pSampler = LookupSampler(resolvedTexId);

        // Record deferred draw command — replayed in EndFrame after vertex data is on the GPU.
        RenderCmd cmd{};
        cmd.type = RenderCmdType::DrawTriangles;
        cmd.pipeline = pipeline;
        cmd.texture = static_cast<SDL_GPUTexture*>(pTex);
        cmd.sampler = pSampler ? static_cast<SDL_GPUSampler*>(pSampler) : s_defaultSampler;
        cmd.vtxOffset = vtxOffset;
        cmd.vtxCount = static_cast<Uint32>(vertices.size());
        cmd.vu.mvp = m_mvpMatrix;
        cmd.vu.fogStart = m_fogUniform.fogStart;
        cmd.vu.fogEnd = m_fogUniform.fogEnd;
        cmd.fogUniform = m_fogUniform;
        s_renderCmds.push_back(cmd);

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
        if (vertices.size() < 2 || !s_frameActive || !m_colorWriteEnabled || m_stencilTestEnabled)
        {
            return;
        }

        const std::uint32_t resolvedTexId = ResolveTextureId(textureId);
        void* pTex = LookupTexture(resolvedTexId);
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

        // Story 4.3.2 (AC-8): RenderQuadStrip uses the 3D pipeline set (Vertex3D layout).
        const int pipelineIdx = GetActivePipelineIndex();
        SDL_GPUGraphicsPipeline* pipeline =
            m_depthTestEnabled
                ? (m_depthMaskEnabled ? s_pipelines3D[pipelineIdx] : s_pipelines3DDepthReadOnly[pipelineIdx])
                : s_pipelines3DDepthOff[pipelineIdx];
        if (!pipeline)
        {
            return;
        }

        void* pSampler = LookupSampler(resolvedTexId);

        // Accumulate strip indices into the per-frame scratch buffer.
        // Record the byte offset so the replay knows where this strip's indices start.
        const Uint32 stripIdxByteOffset = static_cast<Uint32>(s_stripIdxScratch.size() * sizeof(Uint16));

        // Strip pattern: quad i uses vertices (2i, 2i+1, 2i+2, 2i+3).
        // Two triangles: (2i, 2i+1, 2i+2) and (2i+1, 2i+3, 2i+2).
        s_stripIdxScratch.reserve(s_stripIdxScratch.size() + numIndices);
        for (Uint32 i = 0; i < numQuads; ++i)
        {
            const auto v0 = static_cast<Uint16>(i * 2 + 0);
            const auto v1 = static_cast<Uint16>(i * 2 + 1);
            const auto v2 = static_cast<Uint16>(i * 2 + 2);
            const auto v3 = static_cast<Uint16>(i * 2 + 3);
            s_stripIdxScratch.push_back(v0);
            s_stripIdxScratch.push_back(v1);
            s_stripIdxScratch.push_back(v2);
            s_stripIdxScratch.push_back(v1);
            s_stripIdxScratch.push_back(v3);
            s_stripIdxScratch.push_back(v2);
        }

        // Record deferred draw command — replayed in EndFrame after data is on the GPU.
        RenderCmd cmd{};
        cmd.type = RenderCmdType::DrawIndexedStrip;
        cmd.pipeline = pipeline;
        cmd.texture = static_cast<SDL_GPUTexture*>(pTex);
        cmd.sampler = pSampler ? static_cast<SDL_GPUSampler*>(pSampler) : s_defaultSampler;
        cmd.vtxOffset = vtxOffset;
        cmd.idxCount = numIndices;
        cmd.stripIdxOffset = stripIdxByteOffset;
        cmd.vu.mvp = m_mvpMatrix;
        cmd.vu.fogStart = m_fogUniform.fogStart;
        cmd.vu.fogEnd = m_fogUniform.fogEnd;
        cmd.fogUniform = m_fogUniform;
        s_renderCmds.push_back(cmd);

        ++s_dbgDrawCallsThisFrame;
        s_dbgVtxBytesThisFrame += byteSize;
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
    // Story 7.9.7: SetColorMask — track color write state.
    // When all channels are disabled (shadow volume stencil passes), draw calls
    // are skipped entirely since we have no stencil buffer support yet.
    void SetColorMask(bool r, bool g, bool b, bool a) override
    {
        m_colorWriteEnabled = (r || g || b || a);
    }
    // Story 7.9.7: SetStencilTest — track stencil state.
    // All stencil-dependent rendering (shadow volumes, shadow darkening) is skipped
    // since we have no stencil buffer. Without this, RenderShadowToScreen() draws
    // a full-screen darkening quad that covers the entire scene.
    void SetStencilTest(bool enabled) override
    {
        m_stencilTestEnabled = enabled;
    }
    void SetAlphaTest(bool enabled) override
    {
        m_alphaTestEnabled = enabled;
        // Story 7.9.7 (AC-5): Propagate alpha test state to the fog uniform
        // so the fragment shader's `if (alphaDiscardEnabled && color.a <= alphaThreshold) discard;`
        // actually fires for particle sprites.
        m_fogUniform.alphaDiscardEnabled = enabled ? 1u : 0u;
        s_fogDirty = true;
    }

    // Story 7.9.7 (AC-7): Override SetAlphaFunc to propagate alpha threshold
    // to the fog uniform. Game code calls SetAlphaFunc(GL_GREATER, 0.25f)
    // via EnableAlphaTest() in ZzzOpenglUtil.cpp.
    void SetAlphaFunc(int /*func*/, float ref) override
    {
        m_fogUniform.alphaThreshold = ref;
        s_fogDirty = true;
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
        // Story 7.9.7: Preserve alpha discard state — SetFog must NOT reset
        // alphaDiscardEnabled/alphaThreshold set by SetAlphaTest/SetAlphaFunc.
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
    void SetMatrixMode(int mode) override
    {
        m_matrixMode = mode;
    }

    void LoadIdentity() override
    {
        ActiveMatrix() = glm::mat4(1.0f);
        UpdateMVP();
    }

    void PushMatrix() override
    {
        if (m_matrixMode == 0x1700) // GL_MODELVIEW
        {
            if (m_mvStackTop < k_MatrixStackDepth)
                m_mvStack[m_mvStackTop++] = m_modelViewMatrix;
            else
                g_ErrorReport.Write(L"RENDER: modelview matrix stack overflow (depth %d)", k_MatrixStackDepth);
        }
        else
        {
            if (m_projStackTop < k_MatrixStackDepth)
                m_projStack[m_projStackTop++] = m_projMatrix;
            else
                g_ErrorReport.Write(L"RENDER: projection matrix stack overflow (depth %d)", k_MatrixStackDepth);
        }
    }

    void PopMatrix() override
    {
        if (m_matrixMode == 0x1700) // GL_MODELVIEW
        {
            if (m_mvStackTop > 0)
                m_modelViewMatrix = m_mvStack[--m_mvStackTop];
            else
                g_ErrorReport.Write(L"RENDER: modelview matrix stack underflow");
        }
        else
        {
            if (m_projStackTop > 0)
                m_projMatrix = m_projStack[--m_projStackTop];
            else
                g_ErrorReport.Write(L"RENDER: projection matrix stack underflow");
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
        if (!m)
            return;
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

    void UpdateMVP()
    {
        m_mvpMatrix = m_projMatrix * m_modelViewMatrix;
    }

    // Resolve texture ID for draw calls.
    // - textureId > 0: use it directly (explicit texture from caller)
    // - textureId == 0 AND texture disabled: use 0 → white texture (intentional: vertex color only)
    // - textureId == 0 AND texture enabled: use m_boundTextureId (migration compat: BindTexture state)
    [[nodiscard]] std::uint32_t ResolveTextureId(std::uint32_t textureId) const
    {
        if (textureId != 0u)
            return textureId;
        if (!m_texture2DEnabled)
            return 0u; // DisableTexture() was called — use white texture (registered at ID 0)
        return static_cast<std::uint32_t>(m_boundTextureId);
    }

    // Per-instance render state.
    BlendMode m_activeBlendMode = BlendMode::Alpha;
    bool m_blendEnabled = true;
    bool m_depthTestEnabled = true;
    bool m_depthMaskEnabled = true;
    bool m_cullFaceEnabled = true;
    bool m_alphaTestEnabled = false;
    bool m_texture2DEnabled = true;
    bool m_fogEnabled = false;
    bool m_colorWriteEnabled = true;
    bool m_stencilTestEnabled = false;
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
    // EndFrame() copies all accumulated vertex data to the GPU buffer in a
    // single copy pass BEFORE beginning the render pass, so draw commands
    // always read current-frame vertex data (no 1-frame latency).
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
        // Samplers: t0 (texture), s0 (sampler); Uniform buffers: FogUniforms (pushed per-draw)
        // Story 7.9.7: Changed from numStorageBuffers=1 to numUniformBuffers=1 so fog/alpha
        // data can be pushed per-draw-call via SDL_PushGPUFragmentUniformData (not a GPU buffer).
        s_fragShaderTex = createShader("basic_textured", "frag", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 1, /*fatal=*/true);
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
                                                                     bool depthTestEnabled, bool depthWriteEnabled,
                                                                     bool bUse3DLayout)
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
        depthState.enable_depth_write = depthWriteEnabled;
        depthState.enable_stencil_test = false;
        // LESS for 3D: rejects same-depth glow fragments from overlaying their own opaque pass.
        // LESS_OR_EQUAL for 2D: allows overlapping UI elements at the same Z to draw correctly.
        depthState.compare_op = bUse3DLayout ? SDL_GPU_COMPAREOP_LESS : SDL_GPU_COMPAREOP_LESS_OR_EQUAL;

        SDL_GPUGraphicsPipelineTargetInfo targetInfo{};
        targetInfo.color_target_descriptions = &colorTargetDesc;
        targetInfo.num_color_targets = 1;
        // Story 7.9.7 (AC-3): Enable depth-stencil target so pipelines match
        // the render pass that now includes a depth buffer.
        targetInfo.has_depth_stencil_target = true;
        targetInfo.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;

        // Back-face culling: enable for opaque 3D geometry (depth write ON).
        // Disable for transparent/glow passes (depth write OFF) and all 2D.
        SDL_GPURasterizerState rasterState{};
        rasterState.fill_mode = SDL_GPU_FILLMODE_FILL;
        rasterState.cull_mode = (bUse3DLayout && depthWriteEnabled) ? SDL_GPU_CULLMODE_BACK : SDL_GPU_CULLMODE_NONE;
        rasterState.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

        SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.vertex_shader = s_vertShader2D;
        pipelineInfo.fragment_shader = s_fragShaderTex;
        pipelineInfo.vertex_input_state = vtxInputState;
        pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
        pipelineInfo.rasterizer_state = rasterState;
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

            // 2D depth ON (test+write).
            s_pipelines2D[i] = BuildBlendPipeline(blendState, true, true, /*bUse3DLayout=*/false);
            if (!s_pipelines2D[i])
            {
                g_ErrorReport.Write(L"RENDER: SDL_gpu -- 2D pipeline[%d] creation failed: %hs", i, SDL_GetError());
            }

            // 2D depth OFF.
            s_pipelines2DDepthOff[i] = BuildBlendPipeline(blendState, false, false, /*bUse3DLayout=*/false);
            if (!s_pipelines2DDepthOff[i])
            {
                g_ErrorReport.Write(L"RENDER: SDL_gpu -- 2D depth-off pipeline[%d] creation failed: %hs", i,
                                    SDL_GetError());
            }

            // 3D depth ON (test+write) — opaque geometry.
            s_pipelines3D[i] = BuildBlendPipeline(blendState, true, true, /*bUse3DLayout=*/true);
            if (!s_pipelines3D[i])
            {
                g_ErrorReport.Write(L"RENDER: SDL_gpu -- 3D pipeline[%d] creation failed: %hs", i, SDL_GetError());
            }

            // 3D depth OFF.
            s_pipelines3DDepthOff[i] = BuildBlendPipeline(blendState, false, false, /*bUse3DLayout=*/true);
            if (!s_pipelines3DDepthOff[i])
            {
                g_ErrorReport.Write(L"RENDER: SDL_gpu -- 3D depth-off pipeline[%d] creation failed: %hs", i,
                                    SDL_GetError());
            }

            // Story 7.9.7: 3D depth read-only (test ON, write OFF) — for transparent/additive particles.
            // Particles need depth test (to go behind walls) but must NOT write to depth buffer
            // (which would occlude geometry behind them, causing solid rectangle artifacts).
            s_pipelines3DDepthReadOnly[i] = BuildBlendPipeline(blendState, true, false, /*bUse3DLayout=*/true);
            if (!s_pipelines3DDepthReadOnly[i])
            {
                g_ErrorReport.Write(L"RENDER: SDL_gpu -- 3D depth-readonly pipeline[%d] creation failed: %hs", i,
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
    // Story 7.9.7 (AC-3): CreateOrResizeDepthTexture
    // Creates (or recreates on resize) an SDL_GPUTexture with depth format
    // matching the current swapchain dimensions. Called from Init() and
    // BeginFrame() when swapchain size changes.
    // -----------------------------------------------------------------------
    static bool CreateOrResizeDepthTexture(Uint32 width, Uint32 height)
    {
        if (width == 0 || height == 0)
        {
            return false;
        }

        // Skip if the existing depth texture already matches the requested size.
        if (s_depthTexture && s_depthW == width && s_depthH == height)
        {
            return true;
        }

        // Release old depth texture if resizing.
        if (s_depthTexture)
        {
            SDL_ReleaseGPUTexture(s_device, s_depthTexture);
            s_depthTexture = nullptr;
        }

        SDL_GPUTextureCreateInfo depthInfo{};
        depthInfo.type = SDL_GPU_TEXTURETYPE_2D;
        depthInfo.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
        depthInfo.width = width;
        depthInfo.height = height;
        depthInfo.layer_count_or_depth = 1;
        depthInfo.num_levels = 1;
        depthInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;

        s_depthTexture = SDL_CreateGPUTexture(s_device, &depthInfo);
        if (!s_depthTexture)
        {
            g_ErrorReport.Write(L"RENDER: SDL_gpu -- depth texture creation failed (%ux%u): %hs", width, height,
                                SDL_GetError());
            s_depthW = 0u;
            s_depthH = 0u;
            return false;
        }

        s_depthW = width;
        s_depthH = height;
        return true;
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
