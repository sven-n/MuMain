// MuRendererSDLGpu.cpp: SDL_gpu backend implementation of IMuRenderer.
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
//   - Separate 2D/3D pipeline sets: s_pipelines2D / the world sets (WorldPipelineSets()).
//   - Deferred draw command recording: vertices collected during frame, copied to GPU
//     in EndFrame() BEFORE the render pass, then draw commands replayed.
//
// GUARD STRUCTURE:
//   SDL_gpu is the only renderer backend (MuRenderer.cpp deleted).

// Include SDL3 GPU header only in this file — not exposed to game logic.
// SDL3 is a required project dependency, so these includes are unconditional.
#include <optional>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL.h>
#if __has_include(<SDL3_ttf/SDL_ttf.h>)
#include <SDL3_ttf/SDL_ttf.h>
#define MU_HAS_SDL_TTF 1
#else
#define MU_HAS_SDL_TTF 0
#endif

#include "Antialiasing.h"
#include "BitmapUpload.h"
#include "D3D12Diagnostics.h"
#include "DrawCommandHistory.h"
#include "MuRenderer.h"
#include "QuadTopology.h"
#include "RangeAllocator.h"
#include "SdlGpuPixelFormat.h"
#include "SdlGpuPostProcessing.h"
#include "SdlGpuReplayState.h"
#include "SdlGpuValidation.h"
#include "SingleSampleTwins.h"
#include "WorldPipelineSet.h"
#include "Core/Utilities/FrameProfiler.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Render/PostProcessing/BloomSource.h"
#include "Render/PostProcessing/SceneBoundary.h"
#include "Render/Terrain/TerrainHeightGrid.h"
#include "Render/Text/SdlTtfFontSet.h"
#ifdef _EDITOR
#include "Core/MuEditorCore.h"
#endif

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "Render/Renderer/ReorderableDrawRuns.h"
#include "Render/Renderer/SkinnedBonePalette.h"
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
// Initial vertex capacity. CPU staging grows so stress frames keep late UI draws.
constexpr Uint32 k_InitialVertexBufferSize = 64u * 1024u * 1024u;
constexpr Uint32 k_InitialBoneBufferSize = 64u * 1024u;
// Number of blend pipelines: 8 blend modes + 1 disabled.
constexpr int k_PipelineCount = 9;
// Pipeline index for "blend disabled".
constexpr int k_PipelineDisabled = 8;

// Vertex uniform layout matching cbuffer Transform in HLSL.
// Contains MVP matrix + fog params, pushed per-draw via SDL_PushGPUVertexUniformData.
struct VertexUniforms
{
    glm::mat4 mvp;
    float fogStart;
    float fogEnd;
    float fogPad[2]; // 16-byte alignment padding
};
static_assert(sizeof(VertexUniforms) == 80, "VertexUniforms must be 80 bytes");

struct SkinningVertexUniforms
{
    glm::mat4 mvp;
    float bodyOriginAndScale[4]{};
    float skinningScales[4]{};
    std::uint32_t palette[4]{};
    float lightDirection[4]{};
    float fogParameters[4]{};
    std::uint32_t textureCoordinates[4]{};
    float chromeParameters[4]{};
    float textureCoordinateParameters[4]{}; // offset u, v; chrome time term; normal map scale (0: none)
    float eyePositionAndSpecular[4]{};
    // Enhanced lighting: rows of the affine transform from this draw's vertex space to world space.
    float vertexToWorldRows[3][4]{};
};
static_assert(sizeof(SkinningVertexUniforms) == 256, "SkinningVertexUniforms must be 256 bytes");
static_assert(offsetof(SkinningVertexUniforms, eyePositionAndSpecular) == 192,
              "eyePositionAndSpecular must follow textureCoordinateParameters (HLSL cbuffer Skinning)");
static_assert(offsetof(SkinningVertexUniforms, vertexToWorldRows) == 208,
              "vertexToWorldRows must follow eyePositionAndSpecular (HLSL cbuffer Skinning)");

// skinned_lit.frag's normal map (t1, s1 in space2): bound for every draw with that shader.
constexpr Uint32 k_FragmentSamplerSlotNormalMap = 1u;
// The flat tangent-space normal (0, 0, 1) as RGBA8: the normal map of draws without one.
constexpr Uint32 k_FlatNormalPixel = 0xFFFF8080u; // R = G = 128, B = A = 255 (little-endian RGBA)

// Skinning cbuffer palette.w: how skinned_textured.vert.hlsl lights the vertex colour.
constexpr std::uint32_t k_SkinningLightingNone = 0u;
constexpr std::uint32_t k_SkinningLightingClassic = 1u;
constexpr std::uint32_t k_SkinningLightingEnhanced = 2u;        // lit per pixel by skinned_lit.frag
constexpr std::uint32_t k_SkinningLightingEnhancedDynamic = 3u; // and by the frame's point lights

// Fragment uniform slots (cbuffer register bN, space3).
constexpr Uint32 k_FragmentUniformSlotFog = 0u;
constexpr Uint32 k_FragmentUniformSlotSceneLighting = 1u;
constexpr Uint32 k_FragmentUniformSlotDynamicLights = 2u;

using WorldPipelines = Render::WorldPipelineSet<SDL_GPUGraphicsPipeline, k_PipelineCount>;

// skinned_shadow.vert's Shadow cbuffer.
struct ShadowVertexUniforms
{
    float origin[4]{};     // xyz: BodyOrigin, w: height of TW_HEIGHT cells
    float projection[4]{}; // x, y: sx, sy; z: 1 when the scene has terrain
};
static_assert(sizeof(ShadowVertexUniforms) == 32, "ShadowVertexUniforms must be 32 bytes");

} // anonymous namespace

namespace mu
{

// ---------------------------------------------------------------------------
// Fog uniform buffer struct — in mu:: namespace so that
// test_shaderprograms.cpp can forward-declare and verify layout via static_assert.
// Mirrors the FogUniforms cbuffer declared in basic_textured.frag.hlsl (std140).
// HLSL cbuffer layout: uint4 register, float2 + padding register, float4 register.
// Total: 48 bytes.
// ---------------------------------------------------------------------------
struct FogUniform
{
    uint32_t fogEnabled;           // offset  0
    uint32_t alphaDiscardEnabled;  // offset  4
    float alphaThreshold;          // offset  8
    uint32_t alphaToCoverage;      // offset 12 (1: shader sharpens alpha for coverage)
    float fogStart;                // offset 16
    float fogEnd;                  // offset 20
    float fogPadding[2];           // offset 24
    std::array<float, 4> fogColor; // offset 32 (float4 register alignment)
}; // total: 48 bytes

static_assert(offsetof(FogUniform, fogEnabled) == 0, "FogUniform std140 layout");
static_assert(offsetof(FogUniform, alphaDiscardEnabled) == 4, "FogUniform std140 layout");
static_assert(offsetof(FogUniform, alphaThreshold) == 8, "FogUniform std140 layout");
static_assert(offsetof(FogUniform, alphaToCoverage) == 12, "FogUniform std140 layout");
static_assert(offsetof(FogUniform, fogStart) == 16, "FogUniform std140 layout");
static_assert(offsetof(FogUniform, fogEnd) == 20, "FogUniform std140 layout");
static_assert(offsetof(FogUniform, fogPadding) == 24, "FogUniform HLSL cbuffer layout");
static_assert(offsetof(FogUniform, fogColor) == 32, "FogUniform HLSL float4 register alignment");
static_assert(sizeof(FogUniform) == 48, "FogUniform must be 48 bytes (HLSL cbuffer)");

// ---------------------------------------------------------------------------
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
    const char* basePath = SDL_GetBasePath();
    if (basePath != nullptr)
    {
        return (std::filesystem::path(basePath) / "shaders" / filename).string();
    }
    return (std::filesystem::path("shaders") / filename).string();
}

// ---------------------------------------------------------------------------
// Returns the SDL_GPUShaderFormat constant name for the given driver.
// Only used internally — returns the correct enum value for SDL_CreateGPUShader.
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// Loads a compiled shader blob from disk into a byte vector.
// Returns empty vector on failure (caller logs via mu::log).
// ---------------------------------------------------------------------------
[[nodiscard]] static std::vector<Uint8> LoadShaderBlob(const char* name, const char* stage, const char* driver)
{
    const std::string path = GetShaderBlobPath(driver, stage, name);
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        mu::log::Get("render")->error("SDL_gpu -- shader blob not found: {}", path);
        return {};
    }
    const auto fileSize = static_cast<std::streamsize>(file.tellg());
    if (fileSize <= 0)
    {
        mu::log::Get("render")->error("SDL_gpu -- shader blob empty: {}", path);
        return {};
    }
    file.seekg(0, std::ios::beg);
    std::vector<Uint8> blob(static_cast<size_t>(fileSize));
    file.read(reinterpret_cast<char*>(blob.data()), fileSize);
    if (!file)
    {
        mu::log::Get("render")->error("SDL_gpu -- shader blob read error: {}", path);
        return {};
    }
    return blob;
}

// ---------------------------------------------------------------------------
// Static device and resource state for MuRendererSDLGpu.
// ---------------------------------------------------------------------------

static SDL_GPUDevice* s_device = nullptr;
static Render::Renderer::BitmapGpuResources s_bitmapGpuResources;
static SDL_Window* s_window = nullptr;

// Uncapped present mode for VSync off: IMMEDIATE when the window supports it, otherwise
// MAILBOX (also not tied to the display refresh). Vulkan only guarantees VSYNC (FIFO), and
// Wayland compositors without tearing support do not offer IMMEDIATE, so asking for it
// alone left VSync on there.
[[nodiscard]] static SDL_GPUPresentMode PickUncappedPresentMode()
{
    if (SDL_WindowSupportsGPUPresentMode(s_device, s_window, SDL_GPU_PRESENTMODE_IMMEDIATE))
    {
        return SDL_GPU_PRESENTMODE_IMMEDIATE;
    }

    if (SDL_WindowSupportsGPUPresentMode(s_device, s_window, SDL_GPU_PRESENTMODE_MAILBOX))
    {
        mu::log::Get("render")->info("SDL_gpu -- immediate present mode not supported, using mailbox");
        return SDL_GPU_PRESENTMODE_MAILBOX;
    }

    return SDL_GPU_PRESENTMODE_IMMEDIATE;
}

[[nodiscard]] static const char* PresentModeName(SDL_GPUPresentMode mode)
{
    switch (mode)
    {
    case SDL_GPU_PRESENTMODE_VSYNC:
        return "VSync";
    case SDL_GPU_PRESENTMODE_MAILBOX:
        return "mailbox";
    default:
        return "immediate";
    }
}

// RmlUi port: see SetPreSubmitCallback's own comment (MuRenderer.h) for what this is and why.
static std::function<void()> s_preSubmitCallback;

// RmlUi port: see SetPostRmlUiCallback's own comment (MuRenderer.h). Fires after RmlUi's own
// render pass has closed, for content that must sit visually on top of RmlUi (the game cursor,
// legacy CUITextInputBox text) -- see EndFrame()'s own comment at the call site for why this
// needs its own small render pass rather than just calling RenderQuad2D-style functions from
// inside s_preSubmitCallback itself.
static std::function<void()> s_postRmlUiCallback;
static std::function<void()> s_offscreenRenderCallback;

// The texture every pass of this frame renders into: the main pass, FlushRenderCommands'
// mid-frame passes, and the UI seams (RmlUi's pre-submit callback, the post-RmlUi pass). Normally
// the swapchain texture; on a frame that captures its pixels (screenshot readback, reconnect
// capture) it is that frame's substitute colour target, chosen once in BeginFrame() so the
// capture includes everything the player sees. nullptr means the
// swapchain (GetFrameGpuContext() falls back to it); reset at the end of EndFrame().
static SDL_GPUTexture* s_frameColorTarget = nullptr;

// Per-frame command buffer and render pass handles (valid between BeginFrame/EndFrame).
static SDL_GPUCommandBuffer* s_cmdBuf = nullptr;
static SDL_GPURenderPass* s_renderPass = nullptr;
static SDL_GPUTexture* s_swapchainTexture = nullptr;

// Swapchain dimensions in physical pixels (from SDL_WaitAndAcquireGPUSwapchainTexture).
// Used for viewport calculations — distinct from logical window size on HiDPI/Retina.
static Uint32 s_swapW = 0u;
static Uint32 s_swapH = 0u;
static std::uint32_t s_pendingFrameCaptureTextureId = 0u;
static FrameReadbackState s_frameReadbackState;
static SDL_GPUTexture* s_frameReadbackTexture = nullptr;
// Resolved in BeginFrame() from the requests pending at that point (see ResolveFrameColorTarget).
static SDL_GPUTextureFormat s_frameReadbackFormat = SDL_GPU_TEXTUREFORMAT_INVALID;
static SDL_GPUTexture* s_reconnectCaptureTexture = nullptr;

constexpr Uint32 FrameReadbackBytesPerPixel = 4u;
constexpr Uint32 FrameReadbackRowAlignment = 256u;

struct FramePixelDownload
{
    SDL_GPUTransferBuffer* transferBuffer;
    Uint32 rowPitch;
    Uint32 byteCount;
    PixelChannelOrder channelOrder;
};

static void ReleaseFrameReadbackTexture()
{
    if (s_frameReadbackTexture && s_device)
    {
        SDL_ReleaseGPUTexture(s_device, s_frameReadbackTexture);
    }
    s_frameReadbackTexture = nullptr;
}

static void FailPendingFrameReadback()
{
    if (s_frameReadbackState.IsPending())
    {
        s_frameReadbackState.Fail();
    }
    ReleaseFrameReadbackTexture();
}

[[nodiscard]] static bool CreateFrameReadbackTexture(SDL_GPUTextureFormat format)
{
    const auto textureInfo = GetSdlGpuFrameCaptureTextureInfo(format, s_swapW, s_swapH);
    if (!textureInfo)
    {
        mu::log::Get("render")->warn("SDL_gpu -- unsupported format or dimensions for frame readback");
        return false;
    }

    s_frameReadbackTexture = SDL_CreateGPUTexture(s_device, &*textureInfo);
    if (!s_frameReadbackTexture)
    {
        mu::log::Get("render")->warn("SDL_gpu -- frame readback texture creation failed: {}", SDL_GetError());
        return false;
    }
    return true;
}

static void BlitTextureToSwapchain(SDL_GPUCommandBuffer* commandBuffer, SDL_GPUTexture* sourceTexture)
{
    SDL_GPUBlitInfo blit{};
    blit.source.texture = sourceTexture;
    blit.source.w = s_swapW;
    blit.source.h = s_swapH;
    blit.destination.texture = s_swapchainTexture;
    blit.destination.w = s_swapW;
    blit.destination.h = s_swapH;
    blit.load_op = SDL_GPU_LOADOP_DONT_CARE;
    blit.flip_mode = SDL_FLIP_NONE;
    blit.filter = SDL_GPU_FILTER_NEAREST;
    SDL_BlitGPUTexture(commandBuffer, &blit);
}

[[nodiscard]] static std::optional<FramePixelDownload> EncodeFramePixelDownload(SDL_GPUCommandBuffer* commandBuffer,
                                                                                SDL_GPUTexture* sourceTexture,
                                                                                SDL_GPUTextureFormat format)
{
    const auto channelOrder = GetSdlGpuPixelChannelOrder(format);
    if (!channelOrder)
    {
        mu::log::Get("render")->warn("SDL_gpu -- unsupported swapchain format for frame readback");
        return std::nullopt;
    }

    const std::uint64_t rowBytes = static_cast<std::uint64_t>(s_swapW) * FrameReadbackBytesPerPixel;
    const std::uint64_t alignedRowPitch =
        (rowBytes + FrameReadbackRowAlignment - 1u) & ~(static_cast<std::uint64_t>(FrameReadbackRowAlignment) - 1u);
    const std::uint64_t maximumByteCount = std::numeric_limits<Uint32>::max();
    if (s_swapW == 0u || s_swapH == 0u || alignedRowPitch > maximumByteCount / s_swapH)
    {
        mu::log::Get("render")->warn("SDL_gpu -- invalid frame readback dimensions {}x{}", s_swapW, s_swapH);
        return std::nullopt;
    }
    const std::uint64_t byteCount = alignedRowPitch * s_swapH;

    SDL_GPUTransferBufferCreateInfo transferInfo{};
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
    transferInfo.size = static_cast<Uint32>(byteCount);
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(s_device, &transferInfo);
    if (!transferBuffer)
    {
        mu::log::Get("render")->warn("SDL_gpu -- frame readback transfer buffer creation failed: {}", SDL_GetError());
        return std::nullopt;
    }

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);
    if (!copyPass)
    {
        mu::log::Get("render")->warn("SDL_gpu -- frame readback copy pass failed: {}", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(s_device, transferBuffer);
        return std::nullopt;
    }

    SDL_GPUTextureRegion source{};
    source.texture = sourceTexture;
    source.w = s_swapW;
    source.h = s_swapH;
    source.d = 1u;

    SDL_GPUTextureTransferInfo destination{};
    destination.transfer_buffer = transferBuffer;
    destination.pixels_per_row = static_cast<Uint32>(alignedRowPitch) / FrameReadbackBytesPerPixel;
    destination.rows_per_layer = s_swapH;

    SDL_DownloadFromGPUTexture(copyPass, &source, &destination);
    SDL_EndGPUCopyPass(copyPass);

    return FramePixelDownload{transferBuffer, static_cast<Uint32>(alignedRowPitch), static_cast<Uint32>(byteCount),
                              *channelOrder};
}

[[nodiscard]] static bool SubmitFramePixelDownload(SDL_GPUCommandBuffer* commandBuffer, SDL_GPUTexture* sourceTexture,
                                                   SDL_GPUTextureFormat format)
{
    const auto download = EncodeFramePixelDownload(commandBuffer, sourceTexture, format);
    if (!download)
    {
        s_frameReadbackState.Fail();
        return false;
    }

    SDL_GPUFence* fence = nullptr;
    {
        FRAME_PROFILE(RendererSubmit);
        fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
    }
    if (!fence)
    {
        mu::log::Get("render")->warn("SDL_gpu -- frame readback fence acquisition failed: {}", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(s_device, download->transferBuffer);
        s_frameReadbackState.Fail();
        return true;
    }

    SDL_GPUFence* fences[] = {fence};
    const bool waitSucceeded = SDL_WaitForGPUFences(s_device, true, fences, 1u);
    if (!waitSucceeded)
    {
        mu::log::Get("render")->warn("SDL_gpu -- frame readback fence wait failed: {}", SDL_GetError());
    }
    void* mapped = waitSucceeded ? SDL_MapGPUTransferBuffer(s_device, download->transferBuffer, false) : nullptr;
    if (waitSucceeded && !mapped)
    {
        mu::log::Get("render")->warn("SDL_gpu -- frame readback map failed: {}", SDL_GetError());
    }

    FramePixels pixels;
    bool converted = false;
    if (mapped)
    {
        const auto* bytes = static_cast<const std::uint8_t*>(mapped);
        converted = ConvertToTopDownRgb(std::span<const std::uint8_t>(bytes, download->byteCount), s_swapW, s_swapH,
                                        download->rowPitch, download->channelOrder, false, pixels);
        SDL_UnmapGPUTransferBuffer(s_device, download->transferBuffer);
    }

    SDL_ReleaseGPUFence(s_device, fence);
    SDL_ReleaseGPUTransferBuffer(s_device, download->transferBuffer);

    if (!converted)
    {
        if (mapped)
        {
            mu::log::Get("render")->warn("SDL_gpu -- frame readback pixel conversion failed");
        }
        s_frameReadbackState.Fail();
        return true;
    }

    s_frameReadbackState.Complete(std::move(pixels));
    return true;
}

// Diagnostics: per-frame counters, reset in BeginFrame, logged every 300 frames.
static Uint32 s_dbgFrameCount = 0u;
static Uint32 s_dbgDrawCallsThisFrame = 0u;
static Uint32 s_dbgVtxBytesThisFrame = 0u;
static Uint32 s_dbgFallbackTextureThisFrame = 0u;
static Uint32 s_dbgTextureUploadsThisFrame = 0u;
static Uint32 s_dbgTextureCreatesThisFrame = 0u;
static Uint32 s_dbgTextureReleasesThisFrame = 0u;
static Uint32 s_dbgRenderCmdsReplayedThisFrame = 0u;
static Uint32 s_dbgGpuDrawCallsThisFrame = 0u;
static Uint32 s_dbgMergedDrawsThisFrame = 0u;
static Uint32 s_dbgMerged2DDrawsThisFrame = 0u;
static Uint32 s_dbgWhiteTextureDrawsThisFrame = 0u;
static Uint32 s_dbgRealTextureDrawsThisFrame = 0u;
static Uint32 s_dbgPipelineBindsThisFrame = 0u;
static Uint32 s_dbgSamplerBindsThisFrame = 0u;
static Uint32 s_dbgVertexUniformPushesThisFrame = 0u;
static Uint32 s_dbgFragmentUniformPushesThisFrame = 0u;
static bool s_dbgNullPipelineWarned = false;
static bool s_frameTimingInitialized = false;
static bool s_frameTimingEnabled = false;
static bool s_statsEnabled = false;
static bool s_disableD3D12Culling = false;
static bool s_disableD3D12TriangleMerging = false;
static mu::RendererStats s_lastFrameStats;
static std::chrono::steady_clock::time_point s_frameBeginTime;
static std::chrono::steady_clock::time_point s_renderReplayBeginTime;
static std::chrono::steady_clock::time_point s_submitTime;

static void ConfigureD3D12Diagnostics(const char* driverName)
{
    s_disableD3D12Culling = Render::D3D12DiagnosticEnabled(driverName, std::getenv("MU_D3D12_DISABLE_CULLING"));
    s_disableD3D12TriangleMerging =
        Render::D3D12DiagnosticEnabled(driverName, std::getenv("MU_D3D12_DISABLE_TRIANGLE_MERGING"));

    if (s_disableD3D12Culling)
    {
        mu::log::Get("render")->info("SDL_gpu -- D3D12 culling diagnostic: disabled");
    }
    if (s_disableD3D12TriangleMerging)
    {
        mu::log::Get("render")->info("SDL_gpu -- D3D12 triangle merging diagnostic: disabled");
    }
}

// Pipeline sets, one pipeline per blend mode: the index matches BlendMode cast to int; index 8 =
// disabled (no-blend).
// s_pipelines2D: depth ON (test+write), Vertex2D layout (pitch=20).
// s_pipelines2DDepthOff: depth OFF, Vertex2D layout.
static SDL_GPUGraphicsPipeline* s_pipelines2D[k_PipelineCount] = {};
static SDL_GPUGraphicsPipeline* s_pipelines2DDepthOff[k_PipelineCount] = {};
// Body shadows (skinned_shadow.vert): the depth states RenderBodyShadow() draws with.
static SDL_GPUGraphicsPipeline* s_pipelinesSkinnedShadow[k_PipelineCount] = {};
static SDL_GPUGraphicsPipeline* s_pipelinesSkinnedShadowDepthOff[k_PipelineCount] = {};
// The same two for SkinnedWeightedVertex3D (skinned_weighted_shadow.vert).
static SDL_GPUGraphicsPipeline* s_pipelinesSkinnedWeightedShadow[k_PipelineCount] = {};
static SDL_GPUGraphicsPipeline* s_pipelinesSkinnedWeightedShadowDepthOff[k_PipelineCount] = {};

// The world sets: 3D (Vertex3D, pitch=40) and GPU-skinned geometry in the four depth/cull states,
// depth-read-only for transparent and additive draws such as particles. Every world set is listed
// once in WorldPipelineSets(), from which they are built, released and enumerated.
static WorldPipelines s_pipelines3D;
static WorldPipelines s_pipelinesSkinned;
// Enhanced lighting: per-pixel exponential fog for 3D and unlit skinned draws, per-pixel lighting
// for lit skinned draws. Optional — when any is incomplete, Enhanced lighting stays unavailable and
// every draw keeps the Classic sets above.
static WorldPipelines s_pipelines3DEnhancedFog;
static WorldPipelines s_pipelinesSkinnedEnhancedFog;
static WorldPipelines s_pipelinesSkinnedLit;
// The same three for SkinnedWeightedVertex3D (glTF models).
static WorldPipelines s_pipelinesSkinnedWeighted;
static WorldPipelines s_pipelinesSkinnedWeightedEnhancedFog;
static WorldPipelines s_pipelinesSkinnedWeightedLit;
static bool s_enhancedLightingAvailable = false;
static bool s_enhancedLightingEnabled = false;
// The frame's scene lighting, pushed once per render pass to fragment uniform slot 1.
static mu::SceneLightingUniform s_sceneLightingUniform = mu::PackSceneLightingUniform({}, {});
// The frame's point lights (none until SetDynamicLights), pushed with it to fragment uniform slot 2.
static mu::DynamicLightsUniform s_dynamicLightsUniform{};

// Post-processing of the 3D scene: with it on, a frame whose scene marked its end
// (EndSceneGeometry) is replayed in separate scene, bloom and UI passes, see EndFrame.
static mu::SdlGpuPostProcessing s_postProcessing;
static bool s_postProcessingEnabled = false;
static mu::PostProcessingLook s_postProcessingLook{};
static mu::PostProcessingView s_postProcessingView = mu::PostProcessingView::Scene;
static Render::PostProcessing::SceneBoundary s_sceneBoundary;
static int s_bloomExclusionDepth = 0;

// Alpha-to-coverage variants of the depth-writing world pipelines (each world set's culled and
// no-cull states), used while anti-aliasing is on by alpha-tested cutout draws (foliage, hair,
// fences) instead of their alpha test with blending: see ChooseAlphaCoverage. Each variant
// replaces its state's blending-off and Alpha-blend pipelines; they exist only multisampled.
struct AlphaCoveragePipeline
{
    SDL_GPUGraphicsPipeline** opaque;
    SDL_GPUGraphicsPipeline** alphaBlend;
    SDL_GPUGraphicsPipeline* variant;
};
constexpr int k_PipelineAlpha = static_cast<int>(mu::BlendMode::Alpha);
static std::vector<AlphaCoveragePipeline> s_alphaCoveragePipelines;

// Single pre-frame vertex upload.
// Draws accumulate in growable CPU memory before one GPU upload.
static SDL_GPUTransferBuffer* s_vtxTransferBuf = nullptr;
static SDL_GPUBuffer* s_vtxGpuBuf = nullptr;
static Uint32 s_vtxCapacity = 0u;
static Uint32 s_vtxOffset = 0u;
static std::vector<Uint8> s_vtxScratch;

// How much of a per-frame scratch buffer is already on its GPU buffer this frame (StageDeferredGpuData).
struct StagedRange
{
    SDL_GPUBuffer* buffer = nullptr;
    Uint32 bytes = 0u;
};
static StagedRange s_stagedVertices;
static StagedRange s_stagedBones;
static StagedRange s_stagedStripIndices;

struct BufferUpload
{
    SDL_GPUTransferBuffer* transfer;
    SDL_GPUBuffer* buffer;
    Uint32 offset;
    Uint32 size;
    bool cycle;
    Uint32 transferOffset = 0u;
};

// Copies the part of `scratch` not yet staged into `transfer` and queues its upload. The frame's first upload
// cycles the buffer (the GPU may still read last frame's) and so sends everything; a later one sends only the
// appended tail into that same instance, which this frame's earlier passes read. False if the map failed.
static bool StageAppended(SDL_GPUTransferBuffer* transfer, SDL_GPUBuffer* buffer, const void* scratch, Uint32 totalBytes,
                          StagedRange& staged, std::vector<BufferUpload>& uploads)
{
    // A different buffer means a regrow since the last call, which kept none of the staged data.
    const Uint32 start = staged.buffer == buffer ? staged.bytes : 0u;
    if (totalBytes <= start)
    {
        return true;
    }
    void* mapped = SDL_MapGPUTransferBuffer(s_device, transfer, true);
    if (!mapped)
    {
        return false;
    }
    std::memcpy(mapped, static_cast<const Uint8*>(scratch) + start, totalBytes - start);
    SDL_UnmapGPUTransferBuffer(s_device, transfer);
    uploads.push_back({transfer, buffer, start, totalBytes - start, start == 0u});
    staged = {buffer, totalBytes};
    return true;
}

// Rest-pose vertices of cached skinned meshes (CacheSkinnedMesh, CacheWeightedSkinnedMesh), sub-allocated from a few
// large vertex buffers in units that both vertex sizes fill exactly. A draw then uploads only its colour.
constexpr Uint32 k_SkinnedMeshUnitBytes = 8u;
static_assert(sizeof(mu::SkinnedVertex3D) % k_SkinnedMeshUnitBytes == 0 &&
              sizeof(mu::SkinnedWeightedVertex3D) % k_SkinnedMeshUnitBytes == 0);
constexpr Uint32 k_SkinnedMeshBlockUnits = 4u * 1024u * 1024u / k_SkinnedMeshUnitBytes;
// A released range stays untouched until no frame still in flight can read it.
constexpr std::uint64_t k_SkinnedMeshFreeDelayFrames = 3u;

struct SkinnedMeshBlock
{
    SDL_GPUBuffer* buffer;
    Render::RangeAllocator ranges;
};

struct CachedSkinnedMesh
{
    Uint32 block;
    Uint32 firstUnit;
    Uint32 unitCount;
    Uint32 vertexCount;
    Uint32 stride; // sizeof(SkinnedVertex3D) or sizeof(SkinnedWeightedVertex3D)
    // One past the highest bone the vertices index; 0 when unknown.
    Uint32 bonesUsed = 0u;
};

struct PendingSkinnedMeshUpload
{
    CachedSkinnedMesh range;
    std::size_t stagingFirst; // byte offset into s_skinnedMeshStaging
};

struct DeferredSkinnedMeshFree
{
    CachedSkinnedMesh range;
    std::uint64_t releasedAtFrame;
};

static std::vector<SkinnedMeshBlock> s_skinnedMeshBlocks;
static std::unordered_map<const void*, CachedSkinnedMesh> s_skinnedMeshes;
// Meshes cached since the last StageDeferredGpuData, waiting for its copy pass.
static std::vector<std::byte> s_skinnedMeshStaging;
static std::vector<PendingSkinnedMeshUpload> s_pendingSkinnedMeshUploads;
static std::vector<DeferredSkinnedMeshFree> s_deferredSkinnedMeshFrees;
static SDL_GPUTransferBuffer* s_skinnedMeshTransferBuf = nullptr;
static Uint32 s_skinnedMeshTransferCapacity = 0u;
static std::uint64_t s_submittedFrames = 0u;

// Per-frame packed float4 bone rows. Three rows represent one affine 3x4 bone transform.
static SDL_GPUTransferBuffer* s_boneTransferBuf = nullptr;
static SDL_GPUBuffer* s_boneGpuBuf = nullptr;
static Uint32 s_boneCapacity = 0u;
static std::vector<float> s_boneRowScratch;
static const float* s_lastBonePalette = nullptr;
static std::size_t s_lastBonePaletteSize = 0u;
static std::uint32_t s_lastBonePaletteVersion = 0u;
static Uint32 s_lastBonePaletteRowOffset = 0u;
// Bones of the last palette uploaded; a draw indexing more uploads it again, longer.
static Uint32 s_lastBonePaletteBones = 0u;
// The skinned draw keys seen this frame, for the DistinctSkinnedDraws counter.
static std::unordered_set<std::uint64_t> s_skinnedDrawKeys;

// The Render::TerrainHeightGrid that ground-following shaders read (SetTerrainHeights).
static SDL_GPUTransferBuffer* s_terrainHeightTransferBuf = nullptr;
static SDL_GPUBuffer* s_terrainHeightGpuBuf = nullptr;
static std::optional<std::uint32_t> s_terrainHeightVersion;
static bool s_terrainHeightUploadPending = false;

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
    SetScissor, // pixel-level rect clip — Vulkan/Metal/D3D12 viewport alone doesn't clip
    DrawTriangles, // non-indexed 3D (Vertex3D)
    DrawSkinnedTriangles,
    DrawIndexedQuads, // indexed 2D or 3D with static quad index buffer
    DrawIndexedStrip, // indexed 3D with per-frame strip indices (Vertex3D)
    DrawTriangles2D,  // non-indexed 2D triangles (Vertex2D) for text atlas
};

struct RenderCmd
{
    RenderCmdType type;
    SDL_GPUGraphicsPipeline* pipeline;
    SDL_GPUTexture* texture;
    SDL_GPUSampler* sampler;
    // skinned_lit.frag's second texture (the normal map, or the flat default); null for other draws.
    SDL_GPUTexture* normalTexture = nullptr;
    SDL_GPUSampler* normalSampler = nullptr;
    SDL_GPUBuffer* vtxBuffer = nullptr; // null: the frame's s_vtxGpuBuf
    Uint32 vtxOffset;      // in bytes, a multiple of vtxStride
    Uint32 vtxStride;
    Uint32 colorOffset = 0u; // DrawSkinnedTriangles' colour, in bytes into s_vtxGpuBuf
    Uint32 vtxCount;       // for DrawTriangles
    Uint32 idxCount;       // for DrawIndexed*
    Uint32 stripIdxOffset; // byte offset into strip index scratch buffer
    // DrawTriangles / DrawSkinnedTriangles drawn through idxCount indices of the
    // strip index scratch at stripIdxOffset (RenderIndexed*Triangles).
    bool trianglesIndexed{};
    VertexUniforms vu;
    Uint32 skinningIndex = 0u; // into s_skinningUniforms, for DrawSkinnedTriangles
    Uint32 shadowIndex = ~0u;  // into s_shadowUniforms when a DrawSkinnedTriangles is a body shadow
    FogUniform fogUniform;
    SDL_GPUViewport viewport; // for SetViewport only
    SDL_Rect scissor;         // for SetScissor only
    BlendMode blendMode{};
    bool blendEnabled{};
    bool depthTestEnabled{};
    bool depthMaskEnabled{};
    bool cullFaceEnabled{};
    bool bloomExcluded{}; // recorded inside a ScopedBloomExclusion: never a bloom source
    // Set once this command has been replayed into an offscreen capture (see
    // BeginOffscreenCapture/EndOffscreenCapture); the main frame's replay pass then
    // skips it instead of drawing it a second time into the frame.
    bool consumedByOffscreenCapture = false;
};

static std::vector<RenderCmd> s_renderCmds;
// Kept apart from RenderCmd: only skinned draws carry them, and they would nearly double every command.
static std::vector<SkinningVertexUniforms> s_skinningUniforms;
static std::vector<ShadowVertexUniforms> s_shadowUniforms;
static constexpr std::size_t kNoDrawCommand = std::numeric_limits<std::size_t>::max();
static Render::DrawCommandHistory s_previousDrawCommands;

[[nodiscard]] static bool IsDrawCommand(RenderCmdType type)
{
    return type != RenderCmdType::SetViewport && type != RenderCmdType::SetScissor;
}

[[nodiscard]] static bool IsUnsafeInvalidatedDrawCommand(RenderCmdType type)
{
    return IsDrawCommand(type);
}

[[nodiscard]] static FrameProfiler::Counter ClassifyBatchBreak(const RenderCmd& command, std::size_t previousCommand,
                                                               bool geometryCanMerge)
{
    using Counter = FrameProfiler::Counter;

    const RenderCmd& previous = s_renderCmds[previousCommand];
    if (previous.texture != command.texture || previous.sampler != command.sampler)
    {
        return Counter::BatchBreakTexture;
    }
    if (previous.blendEnabled != command.blendEnabled || previous.blendMode != command.blendMode ||
        previous.bloomExcluded != command.bloomExcluded)
    {
        return Counter::BatchBreakBlend;
    }
    if (previous.depthTestEnabled != command.depthTestEnabled ||
        previous.depthMaskEnabled != command.depthMaskEnabled || previous.cullFaceEnabled != command.cullFaceEnabled)
    {
        return Counter::BatchBreakDepth;
    }
    if (previous.pipeline != command.pipeline)
    {
        return Counter::BatchBreakProgram;
    }
    if (std::memcmp(&previous.vu.mvp, &command.vu.mvp, sizeof(command.vu.mvp)) != 0)
    {
        return Counter::BatchBreakMatrix;
    }
    if (previous.vu.fogStart != command.vu.fogStart || previous.vu.fogEnd != command.vu.fogEnd ||
        std::memcmp(&previous.fogUniform, &command.fogUniform, sizeof(FogUniform)) != 0)
    {
        return Counter::BatchBreakUniform;
    }

    if (!geometryCanMerge)
    {
        return Counter::BatchBreakDraw;
    }
    for (std::size_t index = previousCommand + 1; index < s_renderCmds.size(); ++index)
    {
        if (IsDrawCommand(s_renderCmds[index].type))
        {
            return Counter::BatchBreakDraw;
        }
    }
    if (previousCommand + 1 != s_renderCmds.size())
    {
        return Counter::BatchBreakOther;
    }
    return Counter::Count_;
}

[[nodiscard]] static bool MergeAdjacentTriangleCommand(const RenderCmd& command, std::size_t& previousCommand,
                                                       Uint32 vertexStride)
{
    if (previousCommand == kNoDrawCommand)
    {
        return false;
    }

    const RenderCmd& previous = s_renderCmds[previousCommand];
    const bool geometryCanMerge =
        Render::Topology::CanMergeTriangleDraws(previous.vtxOffset, previous.vtxCount, command.vtxOffset, vertexStride);
    const FrameProfiler::Counter breakCause = ClassifyBatchBreak(command, previousCommand, geometryCanMerge);
    if (breakCause != FrameProfiler::Counter::Count_)
    {
        FrameProfiler::Count(breakCause);
        return false;
    }

    s_renderCmds[previousCommand].vtxCount += command.vtxCount;
    return true;
}

[[nodiscard]] static bool MergeAdjacentQuadCommand(const RenderCmd& command, std::size_t& previousCommand,
                                                   Uint32 vertexStride)
{
    if (previousCommand == kNoDrawCommand)
    {
        return false;
    }

    const RenderCmd& previous = s_renderCmds[previousCommand];
    const bool geometryCanMerge = Render::Topology::CanMergeQuadDraws(
        previous.vtxOffset, previous.idxCount, command.vtxOffset, command.idxCount, vertexStride, k_MaxQuads);
    const FrameProfiler::Counter breakCause = ClassifyBatchBreak(command, previousCommand, geometryCanMerge);
    if (breakCause != FrameProfiler::Counter::Count_)
    {
        FrameProfiler::Count(breakCause);
        return false;
    }

    s_renderCmds[previousCommand].idxCount += command.idxCount;
    return true;
}
// True between BeginFrame/EndFrame — replaces s_renderPass as the "frame active" guard
// during the collection phase (render pass is only opened in EndFrame now).
static bool s_frameActive = false;

// RmlUi-behind-3D-icons seam (FlushRenderCommands): lets a caller open a real render pass
// mid-recording instead of waiting for EndFrame's single one. s_replayedCmdCount is how far
// into s_renderCmds the swapchain/depth targets already reflect; s_mainColorPassOpenedThisFrame
// is whether any such pass (a flush, or EndFrame's own) has opened yet this frame -- the first
// one CLEARs, every one after LOADs. Both reset only in BeginFrame, so multiple flushes (and
// EndFrame's own final one) each pick up exactly the range recorded since the last one.
static std::size_t s_replayedCmdCount = 0u;
static bool s_mainColorPassOpenedThisFrame = false;
#ifdef _EDITOR
// The editor's ImGui frame waits to be drawn over this frame: after RmlUi, in a pass of its own
// (RenderEditorOverlayPass, from EndFrame), so the editor's windows sit on top of the game's UI.
static bool s_editorDrawPending = false;

void QueueEditorRenderCommand()
{
    if (!s_frameActive)
    {
        return;
    }

    s_editorDrawPending = true;
}

// Uploads and draws the editor's draw data onto the frame's colour target. Runs once RmlUi's
// passes have closed (PrepareDrawData opens a copy pass) and before the post-RmlUi pass, so the
// game cursor still goes over the editor. Colour only: the ImGui pipeline has no depth target.
static void RenderEditorOverlayPass(SDL_GPUCommandBuffer* commandBuffer, SDL_GPUTexture* colorTexture)
{
    if (!s_editorDrawPending)
    {
        return;
    }
    s_editorDrawPending = false;
    if (!commandBuffer || !colorTexture)
    {
        return;
    }

    g_MuEditorCore.PrepareDrawData(commandBuffer);

    SDL_GPUColorTargetInfo colorTarget{};
    colorTarget.texture = colorTexture;
    colorTarget.load_op = SDL_GPU_LOADOP_LOAD;
    colorTarget.store_op = SDL_GPU_STOREOP_STORE;
    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(commandBuffer, &colorTarget, 1, nullptr);
    if (!pass)
    {
        return;
    }
    g_MuEditorCore.RenderDrawData(commandBuffer, pass);
    SDL_EndGPURenderPass(pass);
}
#endif

static bool BindReplayPipeline(SDL_GPUGraphicsPipeline* pipeline, Render::SdlGpuReplayState& state,
                               const SDL_Rect& scissor)
{
    if (!pipeline)
        return false;
    if (!state.SelectPipeline(pipeline))
        return true;

    SDL_BindGPUGraphicsPipeline(s_renderPass, pipeline);
    SDL_SetGPUScissor(s_renderPass, &scissor);
    ++s_dbgPipelineBindsThisFrame;
    FrameProfiler::Count(FrameProfiler::Counter::PipelineBinds);
    return true;
}

static void PushReplayVertexUniforms(Uint32 slot, const void* data, std::size_t size, Render::SdlGpuReplayState& state)
{
    const auto* bytes = static_cast<const std::byte*>(data);
    if (!state.SelectVertexUniforms(slot, {bytes, size}))
        return;

    SDL_PushGPUVertexUniformData(s_cmdBuf, slot, data, static_cast<Uint32>(size));
    ++s_dbgVertexUniformPushesThisFrame;
}

static void PushReplayFragmentUniforms(const FogUniform& uniforms, Render::SdlGpuReplayState& state)
{
    const auto bytes = std::as_bytes(std::span{&uniforms, 1u});
    if (!state.SelectFragmentUniforms(bytes))
        return;

    SDL_PushGPUFragmentUniformData(s_cmdBuf, k_FragmentUniformSlotFog, &uniforms, sizeof(uniforms));
    ++s_dbgFragmentUniformPushesThisFrame;
}

// Classic fragment shaders declare one uniform buffer and never read slots 1 and 2; only
// skinned_lit.frag reads slot 2.
static void PushSceneLightingUniforms()
{
    SDL_PushGPUFragmentUniformData(s_cmdBuf, k_FragmentUniformSlotSceneLighting, &s_sceneLightingUniform,
                                   sizeof(s_sceneLightingUniform));
    SDL_PushGPUFragmentUniformData(s_cmdBuf, k_FragmentUniformSlotDynamicLights, &s_dynamicLightsUniform,
                                   sizeof(s_dynamicLightsUniform));
    s_dbgFragmentUniformPushesThisFrame += 2;
}

static void BindReplayFragmentSampler(const RenderCmd& command, Render::SdlGpuReplayState& state)
{
    if (command.normalTexture && state.SelectNormalMapSampler(command.normalTexture, command.normalSampler))
    {
        const SDL_GPUTextureSamplerBinding normalBinding{command.normalTexture, command.normalSampler};
        SDL_BindGPUFragmentSamplers(s_renderPass, k_FragmentSamplerSlotNormalMap, &normalBinding, 1);
        ++s_dbgSamplerBindsThisFrame;
    }
    if (!state.SelectFragmentSampler(command.texture, command.sampler))
        return;

    const SDL_GPUTextureSamplerBinding samplerBinding{command.texture, command.sampler};
    SDL_BindGPUFragmentSamplers(s_renderPass, 0, &samplerBinding, 1);
    ++s_dbgSamplerBindsThisFrame;
    FrameProfiler::Count(FrameProfiler::Counter::SamplerBinds);
}

static void BindReplayIndexBuffer(SDL_GPUBuffer* buffer, Uint32 offset, SDL_GPUIndexElementSize elementSize,
                                  Render::SdlGpuReplayState& state)
{
    if (!state.SelectIndexBuffer(buffer, offset, elementSize))
        return;

    const SDL_GPUBufferBinding indexBinding{buffer, offset};
    SDL_BindGPUIndexBuffer(s_renderPass, &indexBinding, elementSize);
}

// Passes outside the frame's main pass draw with one sample even while anti-aliasing is on: the
// offscreen captures and render targets, the post-RmlUi pass and FlushRenderCommands render into
// single-sample textures. Each multisampled pipeline maps to its single-sample twin, built with
// it (an alpha-to-coverage variant maps to its blending-off pipeline's twin); empty with one
// sample. s_singleSamplePass makes ReplayDrawCommand use the twins.
static std::unordered_map<SDL_GPUGraphicsPipeline*, SDL_GPUGraphicsPipeline*> s_singleSamplePipelines;
static bool s_singleSamplePass = false;
// Whether the built pipelines are multisampled; set by CreatePipelinesForSampleCount.
static bool s_pipelinesMultisampled = false;

// Replays a recorded draw with `pipeline`: its own, or a variant of it for another pass.
static void ReplayDrawCommand(const RenderCmd& command, SDL_GPUGraphicsPipeline* pipeline, bool boneDataReady,
                              const SDL_Rect& scissor, Render::SdlGpuReplayState& state)
{
    // With one sample the frame's own pipelines already match; with more, a pass outside the
    // main one draws only through a twin (CreateSingleSamplePipelines guarantees a complete set,
    // or turns anti-aliasing off).
    if (s_singleSamplePass && s_pipelinesMultisampled)
    {
        const auto twin = s_singleSamplePipelines.find(pipeline);
        pipeline = twin != s_singleSamplePipelines.end() ? twin->second : nullptr;
    }
    const bool skinned = command.type == RenderCmdType::DrawSkinnedTriangles;
    const bool shadow = skinned && command.shadowIndex != ~0u;
    if (!command.texture || !command.sampler || (skinned && (!boneDataReady || !s_boneGpuBuf)) ||
        (shadow && !s_terrainHeightGpuBuf) || !BindReplayPipeline(pipeline, state, scissor))
    {
        return;
    }

    if (skinned)
        PushReplayVertexUniforms(0, &s_skinningUniforms[command.skinningIndex], sizeof(SkinningVertexUniforms), state);
    else
        PushReplayVertexUniforms(0, &command.vu, sizeof(command.vu), state);
    if (shadow)
        PushReplayVertexUniforms(1, &s_shadowUniforms[command.shadowIndex], sizeof(ShadowVertexUniforms), state);

    // Bound again only when the buffer or the vertex size changes; draws in between say where their vertices start.
    SDL_GPUBuffer* const vertexBuffer = command.vtxBuffer ? command.vtxBuffer : s_vtxGpuBuf;
    if (state.SelectVertexBuffer(vertexBuffer, command.vtxStride, command.vtxOffset))
    {
        const SDL_GPUBufferBinding vertexBinding{vertexBuffer, state.VertexBufferOffset()};
        SDL_BindGPUVertexBuffers(s_renderPass, 0, &vertexBinding, 1);
    }
    const Uint32 firstVertex = (command.vtxOffset - state.VertexBufferOffset()) / command.vtxStride;
    // A skinned draw's one colour is a per-instance attribute: its single instance starts at it.
    Uint32 firstInstance = 0u;
    if (skinned)
    {
        if (state.SelectInstanceBuffer(s_vtxGpuBuf))
        {
            const SDL_GPUBufferBinding colorBinding{s_vtxGpuBuf, 0u};
            SDL_BindGPUVertexBuffers(s_renderPass, 1, &colorBinding, 1);
        }
        firstInstance = command.colorOffset / sizeof(Uint32);
    }

    if (skinned)
    {
        SDL_GPUBuffer* const storageBuffers[] = {s_boneGpuBuf, s_terrainHeightGpuBuf};
        const std::span<SDL_GPUBuffer* const> bound{storageBuffers, shadow ? 2u : 1u};
        if (state.SelectVertexStorageBuffers(bound))
            SDL_BindGPUVertexStorageBuffers(s_renderPass, 0, bound.data(), static_cast<Uint32>(bound.size()));
    }

    Uint32 firstIndex = 0u;
    if (command.type == RenderCmdType::DrawIndexedQuads)
        BindReplayIndexBuffer(s_quadIdxBuf, 0, SDL_GPU_INDEXELEMENTSIZE_16BIT, state);
    else if (command.type == RenderCmdType::DrawIndexedStrip || command.trianglesIndexed)
    {
        BindReplayIndexBuffer(s_stripIdxBuf, 0, SDL_GPU_INDEXELEMENTSIZE_16BIT, state);
        firstIndex = command.stripIdxOffset / sizeof(Uint16);
    }

    BindReplayFragmentSampler(command, state);
    PushReplayFragmentUniforms(command.fogUniform, state);

    if (command.type == RenderCmdType::DrawIndexedQuads || command.type == RenderCmdType::DrawIndexedStrip ||
        command.trianglesIndexed)
        SDL_DrawGPUIndexedPrimitives(s_renderPass, command.idxCount, 1, firstIndex, static_cast<Sint32>(firstVertex), 0);
    else
        SDL_DrawGPUPrimitives(s_renderPass, command.vtxCount, 1, firstVertex, firstInstance);
    ++s_dbgGpuDrawCallsThisFrame;
}

// CPU-side scratch buffer for quad strip indices accumulated during the frame.
// Copied to GPU in one shot in EndFrame before the render pass.
static std::vector<Uint16> s_stripIdxScratch;

// ---------------------------------------------------------------------------
// Deferred texture updates.
// Queued during the frame when CPU-side BITMAP_t.Buffer changes. Processed in
// EndFrame's copy pass before the render pass, so draw commands see the updated
// texture data.
// ---------------------------------------------------------------------------
struct TextureUpdateCmd
{
    SDL_GPUTexture* gpuTexture;
    std::vector<std::uint8_t> pixelsCopy; // snapshot of CPU-side pixel data at queue time
    Uint32 width;
    Uint32 height;
    Uint32 bytesPerRow; // row pitch in bytes
};

static std::vector<TextureUpdateCmd> s_textureUpdates;

// Default white 1×1 texture used for textureId==0 and unknown IDs.
static SDL_GPUTexture* s_whiteTexture = nullptr;
// 1×1 flat normal map (k_FlatNormalPixel) bound to skinned_lit.frag for draws without a map.
static SDL_GPUTexture* s_flatNormalTexture = nullptr;

// Shader handles for basic_textured (2D path) and basic_colored — released after pipeline creation.
static SDL_GPUShader* s_vertShader2D = nullptr;      // basic_textured.vert
static SDL_GPUShader* s_fragShaderTex = nullptr;     // basic_textured.frag
static SDL_GPUShader* s_vertShader2DCol = nullptr;   // basic_colored.vert
static SDL_GPUShader* s_fragShaderCol = nullptr;     // basic_colored.frag
static SDL_GPUShader* s_vertShaderShadow = nullptr;  // shadow_volume.vert
static SDL_GPUShader* s_vertShaderSkinned = nullptr; // skinned_textured.vert
static SDL_GPUShader* s_vertShaderSkinnedWeighted = nullptr; // skinned_weighted.vert
static SDL_GPUShader* s_vertShaderSkinnedShadow = nullptr; // skinned_shadow.vert
static SDL_GPUShader* s_vertShaderSkinnedWeightedShadow = nullptr; // skinned_weighted_shadow.vert
static SDL_GPUShader* s_fragShaderTexFog = nullptr;  // basic_textured_fog.frag (Enhanced, optional)
static SDL_GPUShader* s_fragShaderLit = nullptr;     // skinned_lit.frag (Enhanced, optional)
// Post-processing (optional): without them post-processing is unavailable.
static SDL_GPUShader* s_vertShaderPostFullscreen = nullptr; // post_fullscreen.vert
static SDL_GPUShader* s_fragShaderBloomDown = nullptr;      // post_bloom_down.frag
static SDL_GPUShader* s_fragShaderBloomUp = nullptr;        // post_bloom_up.frag
static SDL_GPUShader* s_fragShaderComposite = nullptr;      // post_composite.frag

// Depth buffer texture for correct 3D depth testing.
// Created in Init() at swapchain dimensions, recreated on window resize.
static SDL_GPUTexture* s_depthTexture = nullptr;
static Uint32 s_depthW = 0u;
static Uint32 s_depthH = 0u;
static int s_depthSampleCount = Render::Antialiasing::OffSampleCount;
// Ordinary frames render here rather than into the swapchain: SDL creates swapchain textures
// without SAMPLER usage, so they can't be a blit source, and the RmlUi backend reads the frame
// back to composite over it. Blitted to the swapchain at the end of EndFrame().
static SDL_GPUTexture* s_mainColorTarget = nullptr;
static Uint32 s_mainColorW = 0u;
static Uint32 s_mainColorH = 0u;
static SDL_GPUTextureFormat s_mainColorFormat = SDL_GPU_TEXTUREFORMAT_INVALID;
static SDL_FColor s_clearColor{0.0f, 0.0f, 0.0f, 1.0f};

// Multisample anti-aliasing. s_sampleCount is what the pipelines, the depth
// buffer and the multisampled colour target are built for; the setting
// (s_requestedSampleCount) is applied between frames, at BeginFrame, by
// rebuilding them. With one sample nothing multisampled exists and the frame
// renders exactly as before.
static int s_requestedSampleCount = Render::Antialiasing::OffSampleCount;
static int s_sampleCount = Render::Antialiasing::OffSampleCount;
static bool s_sampleCountChangePending = false;
// Logs the first alpha-to-coverage draw after each sample-count change.
static bool s_alphaCoverageDrawLogged = false;
static Render::Antialiasing::SupportedSampleCounts s_supportedSampleCounts{};
static SDL_GPUTexture* s_multisampleColorTexture = nullptr;
static SDL_GPUTextureCreateInfo s_multisampleColorInfo{};
// The depth of a single-sample pass at the frame's size while the frame's own depth is multisampled.
static SDL_GPUTexture* s_singleSampleDepthTexture = nullptr;
static Uint32 s_singleSampleDepthW = 0u;
static Uint32 s_singleSampleDepthH = 0u;

// Isolated offscreen render captures. A capture batches a sub-range of s_renderCmds -
// recorded between BeginOffscreenCapture()/EndOffscreenCapture() - to be replayed into a
// dedicated texture instead of the frame: the Map Editor's object thumbnails, and render
// targets (CreateRenderTarget()) an RmlUi document shows.
struct PendingOffscreenCapture
{
    std::size_t startCmd;
    std::size_t endCmd;
    std::uint32_t textureId;
    Uint32 width;
    Uint32 height;
};
static std::vector<PendingOffscreenCapture> s_pendingOffscreenCaptures;

static bool s_drawReorderingEnabled = true;
// What a reorderable block must not span: a flush, or an offscreen capture's start or end.
struct ReorderableDrawsMark
{
    std::size_t firstCommand = 0u;
    std::size_t replayedCommands = 0u;
    std::size_t pendingCaptures = 0u;
    std::uint32_t captureTexture = 0u;
};
static ReorderableDrawsMark s_reorderableDraws;
static std::size_t s_offscreenCaptureStart = 0u;
static std::uint32_t s_offscreenCaptureTextureId = 0u;
// SetOffscreen2DRect()'s projection for the open capture's 2D draws.
static std::optional<glm::mat4> s_offscreen2DProjection;
static Uint32 s_offscreenCaptureWidth = 0u;
static Uint32 s_offscreenCaptureHeight = 0u;

// Shared depth buffer for one-shot captures, resized on demand. Captures are
// processed strictly one at a time (never concurrently), so one is enough.
static SDL_GPUTexture* s_offscreenDepthTexture = nullptr;
static Uint32 s_offscreenDepthW = 0u;
static Uint32 s_offscreenDepthH = 0u;

// A render target's own depth buffer, sized to it. Several are captured every frame at
// different sizes, which the shared one above would recreate for each.
static std::unordered_map<std::uint32_t, SDL_GPUTexture*> s_renderTargetDepth;
// Drained at the start of the next frame: releasing an owned texture mid-frame sets
// s_texturesInvalidated, which drops that whole frame's replay.
static std::vector<std::uint32_t> s_pendingRenderTargetReleases;

// Fog uniform buffer and transfer buffer.
static SDL_GPUBuffer* s_fogUniformBuf = nullptr;
static SDL_GPUTransferBuffer* s_fogTransferBuf = nullptr;
static bool s_fogDirty = true; // upload on first draw if SetFog not called

// SDL_ttf GPU text engine and font variants.
// s_textEngine: atlas-based text engine created after SDL_GPUDevice.
// s_ttfFonts: pre-loaded fonts for UI text rendering (normal, bold, big, fixed).
static TTF_TextEngine* s_textEngine = nullptr;
static Render::Text::SdlTtfFontSet s_ttfFonts;

// F-7 fix: Cached window dimensions, updated once per frame in BeginFrame().
static int s_cachedWinW = 0;
static int s_cachedWinH = 0;

#if MU_HAS_SDL_TTF
static void WarmTtfFonts()
{
    static constexpr const char* k_WarmupGlyphs = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
                                                  "0123456789 !@#$%^&*()-_=+[]{}|;:',.<>?/~`\"\\한글";
    using Render::Text::SdlTtfFontRole;
    for (SdlTtfFontRole role :
         {SdlTtfFontRole::Normal, SdlTtfFontRole::Bold, SdlTtfFontRole::Big, SdlTtfFontRole::Fixed})
    {
        TTF_Font* font = s_ttfFonts.Get(role);
        TTF_Text* warmup = TTF_CreateText(s_textEngine, font, k_WarmupGlyphs, 0);
        if (!warmup)
            continue;
        TTF_GetGPUTextDrawData(warmup);
        TTF_DestroyText(warmup);
    }
}

[[nodiscard]] static bool LoadTtfFonts(std::string_view configuredFamily, float normalPointSize, float bigPointSize,
                                       float fixedPointSize)
{
    if (!s_ttfFonts.Load(configuredFamily, normalPointSize, bigPointSize, fixedPointSize))
        return false;

    WarmTtfFonts();
    return true;
}
#endif

// ---------------------------------------------------------------------------
// TextureRegistry: maps caller-provided uint32_t ids to SDL_GPUTexture*.
// Accessible from test TU via forward declarations in mu namespace.
// ---------------------------------------------------------------------------
static std::unordered_map<std::uint32_t, void*> s_textureMap;
static std::unordered_map<std::uint32_t, std::pair<std::uint32_t, std::uint32_t>> s_textureSizes;

[[nodiscard]] static SDL_GPUTexture* FrameColorTexture()
{
    return s_frameColorTarget ? s_frameColorTarget : s_swapchainTexture;
}

// Called by BeginFrame() once the swapchain texture is known: a screenshot readback or a
// reconnect capture requested before this frame claims the whole frame's colour target, so
// every pass renders into the capture texture and nothing has to be replayed after the fact.
static void ResolveFrameColorTarget()
{
    s_frameColorTarget = nullptr;
    s_reconnectCaptureTexture = nullptr;
    s_frameReadbackFormat = SDL_GPU_TEXTUREFORMAT_INVALID;

    if (s_frameReadbackState.IsPending())
    {
        s_frameReadbackFormat = SDL_GetGPUSwapchainTextureFormat(s_device, s_window);
        if (CreateFrameReadbackTexture(s_frameReadbackFormat))
        {
            s_frameColorTarget = s_frameReadbackTexture;
        }
        else
        {
            FailPendingFrameReadback();
        }
    }

    if (s_pendingFrameCaptureTextureId != 0u)
    {
        const auto texture = s_textureMap.find(s_pendingFrameCaptureTextureId);
        const auto size = s_textureSizes.find(s_pendingFrameCaptureTextureId);
        if (texture != s_textureMap.end() && size != s_textureSizes.end() && size->second.first == s_swapW &&
            size->second.second == s_swapH)
        {
            s_reconnectCaptureTexture = static_cast<SDL_GPUTexture*>(texture->second);
            if (!s_frameColorTarget)
            {
                s_frameColorTarget = s_reconnectCaptureTexture;
            }
        }
        s_pendingFrameCaptureTextureId = 0u;
    }

    if (!s_frameColorTarget)
    {
        s_frameColorTarget = s_mainColorTarget;
    }
}
static std::unordered_set<std::uint32_t> s_ownedTextureIds;
static std::uint32_t s_cachedTextureId = 0u;
static void* s_cachedTexture = nullptr;
static bool s_textureCacheValid = false;
constexpr std::uint32_t kFirstOwnedDynamicTextureId = 0x60000000u;
constexpr std::uint32_t kLastOwnedDynamicTextureId = 0x7FFFFFFFu;
static std::uint32_t s_nextOwnedDynamicTextureId = kFirstOwnedDynamicTextureId;

[[nodiscard]] static std::uint32_t AllocateOwnedDynamicTextureId()
{
    const std::uint32_t start = s_nextOwnedDynamicTextureId;
    do
    {
        const std::uint32_t candidate = s_nextOwnedDynamicTextureId;
        s_nextOwnedDynamicTextureId =
            candidate == kLastOwnedDynamicTextureId ? kFirstOwnedDynamicTextureId : candidate + 1u;
        if (!s_textureMap.contains(candidate))
        {
            return candidate;
        }
    } while (s_nextOwnedDynamicTextureId != start);

    return 0u;
}

// Flag: set when textures are replaced or unregistered mid-frame. EndFrame skips
// render replay to avoid dangling GPU resource pointers. Reset at BeginFrame.
static bool s_texturesInvalidated = false;

// ---------------------------------------------------------------------------
// TextureRegistry free functions (exposed for test linkage).
// Using void* as the texture pointer type so the test TU can include this
// without requiring SDL3 headers. The actual stored type is SDL_GPUTexture*.
// ---------------------------------------------------------------------------

[[nodiscard]] void* LookupTexture(std::uint32_t id)
{
    if (s_textureCacheValid && s_cachedTextureId == id)
    {
        return s_cachedTexture;
    }

    auto it = s_textureMap.find(id);
    s_cachedTextureId = id;
    s_cachedTexture = it != s_textureMap.end() ? it->second : nullptr;
    s_textureCacheValid = true;
    return s_cachedTexture;
}

static void InvalidateTextureLookupCache()
{
    s_textureCacheValid = false;
}

[[nodiscard]] static void* LookupTextureForDraw(std::uint32_t id)
{
    void* texture = LookupTexture(id);
    if (texture)
    {
        return texture;
    }

    ++s_dbgFallbackTextureThisFrame;
    return s_whiteTexture;
}

static void DiscardQueuedTextureUpdates(void* texture)
{
    if (!texture)
    {
        return;
    }

    const auto newEnd = std::remove_if(s_textureUpdates.begin(), s_textureUpdates.end(),
                                       [texture](const auto& update) { return update.gpuTexture == texture; });
    s_textureUpdates.erase(newEnd, s_textureUpdates.end());
}

static bool ReleaseOwnedTextureById(std::uint32_t id)
{
    auto owned = s_ownedTextureIds.find(id);
    if (owned == s_ownedTextureIds.end())
    {
        return false;
    }

    auto texture = s_textureMap.find(id);
    if (texture != s_textureMap.end())
    {
        DiscardQueuedTextureUpdates(texture->second);
        if (s_device && texture->second)
        {
            SDL_ReleaseGPUTexture(s_device, static_cast<SDL_GPUTexture*>(texture->second));
            ++s_dbgTextureReleasesThisFrame;
        }
        s_textureMap.erase(texture);
        InvalidateTextureLookupCache();
    }

    s_textureSizes.erase(id);
    s_ownedTextureIds.erase(owned);
    if (const auto depth = s_renderTargetDepth.find(id); depth != s_renderTargetDepth.end())
    {
        if (s_device && depth->second)
        {
            SDL_ReleaseGPUTexture(s_device, depth->second);
        }
        s_renderTargetDepth.erase(depth);
    }
    s_texturesInvalidated = true;
    return true;
}

static void ReleaseOwnedTextures()
{
    while (!s_ownedTextureIds.empty())
    {
        ReleaseOwnedTextureById(*s_ownedTextureIds.begin());
    }
}

void RegisterTexture(std::uint32_t id, void* pTex)
{
    auto existing = s_textureMap.find(id);
    if (existing != s_textureMap.end() && existing->second == pTex)
    {
        return;
    }

    ReleaseOwnedTextureById(id);
    existing = s_textureMap.find(id);
    if (existing != s_textureMap.end())
    {
        DiscardQueuedTextureUpdates(existing->second);
        s_texturesInvalidated = true;
    }

    s_textureMap[id] = pTex;
    InvalidateTextureLookupCache();
    s_textureSizes.erase(id);
}

void UnregisterTexture(std::uint32_t id)
{
    if (ReleaseOwnedTextureById(id))
    {
        return;
    }

    auto texture = s_textureMap.find(id);
    if (texture != s_textureMap.end())
    {
        DiscardQueuedTextureUpdates(texture->second);
    }
    s_textureMap.erase(id);
    InvalidateTextureLookupCache();
    s_textureSizes.erase(id);
    s_ownedTextureIds.erase(id);
    // Mark that GPU resources were freed — deferred commands may hold dangling pointers.
    s_texturesInvalidated = true;
}

void ClearTextureRegistry()
{
    const bool hadTextures = !s_textureMap.empty();
    ReleaseOwnedTextures();
    s_textureMap.clear();
    InvalidateTextureLookupCache();
    s_textureSizes.clear();
    if (hadTextures)
    {
        s_texturesInvalidated = true;
    }
}

// ---------------------------------------------------------------------------
// SamplerRegistry — parallel to TextureRegistry.
// Maps caller-provided uint32_t ids to SDL_GPUSampler* (stored as void* for test linkage).
// RegisterSampler / LookupSampler / UnregisterSampler follow the same pattern as the texture registry.
// Sampler binding in draw calls uses LookupSampler(textureId) instead of the hardcoded s_defaultSampler.
// ---------------------------------------------------------------------------
static std::unordered_map<std::uint32_t, void*> s_samplerMap;
static std::uint32_t s_cachedSamplerId = 0u;
static void* s_cachedSampler = nullptr;
static bool s_samplerCacheValid = false;

[[nodiscard]] void* LookupSampler(std::uint32_t id)
{
    if (s_samplerCacheValid && s_cachedSamplerId == id)
    {
        return s_cachedSampler;
    }

    auto it = s_samplerMap.find(id);
    if (it == s_samplerMap.end())
    {
        s_cachedSampler = s_defaultSampler; // fallback to default sampler for unknown IDs
    }
    else
    {
        s_cachedSampler = it->second;
    }
    s_cachedSamplerId = id;
    s_samplerCacheValid = true;
    return s_cachedSampler;
}

// ---------------------------------------------------------------------------
// Enhanced texture filtering: world draws of a texture that has a filtered
// sampler (mip chain, trilinear, anisotropy) use it instead of the Classic one
// above, unless they are screen-space (2D quads, UI item previews). A texture
// that is eligible but still lacks its mip chain is "pending": its first world
// draw records a request (and still draws Classic) so the texture owner can
// build the chain between frames.
// ---------------------------------------------------------------------------
static std::unordered_map<std::uint32_t, void*> s_filteredSamplerMap;
static std::unordered_set<std::uint32_t> s_filteredSamplerPending;
static std::vector<std::uint32_t> s_filteredSamplerRequests;
// Textures updated after creation (QueueTextureUpdate): never filtered.
static std::unordered_set<std::uint32_t> s_updatedTextureIds;
static bool s_textureFilteringEnabled = false;
static int s_screenSpaceSamplingDepth = 0;
static std::uint32_t s_cachedWorldSamplerId = 0u;
static void* s_cachedWorldSampler = nullptr;
static bool s_worldSamplerCacheValid = false;

static void InvalidateSamplerLookupCaches()
{
    s_samplerCacheValid = false;
    s_worldSamplerCacheValid = false;
}

// True when LookupWorldSampler gives this texture its Enhanced (mipmapped,
// linearly filtered) sampler rather than the Classic one.
[[nodiscard]] static bool UsesEnhancedSampler(std::uint32_t id)
{
    return s_textureFilteringEnabled && s_screenSpaceSamplingDepth == 0 &&
           s_filteredSamplerMap.find(id) != s_filteredSamplerMap.end();
}

[[nodiscard]] static void* LookupWorldSampler(std::uint32_t id)
{
    if (!s_textureFilteringEnabled || s_screenSpaceSamplingDepth > 0)
    {
        return LookupSampler(id);
    }
    if (s_worldSamplerCacheValid && s_cachedWorldSamplerId == id)
    {
        return s_cachedWorldSampler;
    }

    void* sampler = nullptr;
    auto filtered = s_filteredSamplerMap.find(id);
    if (filtered != s_filteredSamplerMap.end())
    {
        sampler = filtered->second;
    }
    else
    {
        if (s_filteredSamplerPending.erase(id) > 0u)
        {
            s_filteredSamplerRequests.push_back(id);
        }
        sampler = LookupSampler(id);
    }

    s_cachedWorldSamplerId = id;
    s_cachedWorldSampler = sampler;
    s_worldSamplerCacheValid = true;
    return sampler;
}

void RegisterSampler(std::uint32_t id, void* pSampler)
{
    s_samplerMap[id] = pSampler;
    InvalidateSamplerLookupCaches();
}

void UnregisterSampler(std::uint32_t id)
{
    s_samplerMap.erase(id);
    s_filteredSamplerMap.erase(id);
    s_filteredSamplerPending.erase(id);
    s_updatedTextureIds.erase(id);
    InvalidateSamplerLookupCaches();
}

void ClearSamplerRegistry()
{
    s_samplerMap.clear();
    s_filteredSamplerMap.clear();
    s_filteredSamplerPending.clear();
    s_filteredSamplerRequests.clear();
    s_updatedTextureIds.clear();
    InvalidateSamplerLookupCaches();
}

void RegisterFilteredSampler(std::uint32_t id, void* pSampler)
{
    s_filteredSamplerPending.erase(id);
    if (pSampler && !s_updatedTextureIds.contains(id))
    {
        s_filteredSamplerMap[id] = pSampler;
    }
    else
    {
        s_filteredSamplerMap.erase(id);
    }
    InvalidateSamplerLookupCaches();
}

void MarkFilteredSamplerPending(std::uint32_t id)
{
    if (!s_filteredSamplerMap.contains(id) && !s_updatedTextureIds.contains(id))
    {
        s_filteredSamplerPending.insert(id);
        InvalidateSamplerLookupCaches();
    }
}

void TakeFilteredSamplerRequests(std::vector<std::uint32_t>& requests)
{
    requests.insert(requests.end(), s_filteredSamplerRequests.begin(), s_filteredSamplerRequests.end());
    s_filteredSamplerRequests.clear();
}

void SetTextureFilteringEnabled(bool enabled)
{
    if (s_textureFilteringEnabled != enabled)
    {
        s_textureFilteringEnabled = enabled;
        s_filteredSamplerRequests.clear();
        InvalidateSamplerLookupCaches();
    }
}

// A texture whose pixels change after creation updates level 0 only, so it
// keeps sampling level 0 through its Classic sampler until it is unloaded.
static void MarkTextureUpdatedAfterCreation(std::uint32_t id)
{
    if (s_updatedTextureIds.insert(id).second)
    {
        s_filteredSamplerMap.erase(id);
        s_filteredSamplerPending.erase(id);
        InvalidateSamplerLookupCaches();
    }
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
        mu::log::Get("render")->warn("SDL_gpu::GetBlendFactors -- unknown BlendMode {}", static_cast<int>(mode));
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
    [[nodiscard]] static bool Init(void* pNativeWindow, std::string_view fontFamily,
                                   std::string_view renderBackend, float normalPointSize,
                                   float bigPointSize, float fixedPointSize)
    {
        s_window = static_cast<SDL_Window*>(pNativeWindow);
        if (!s_window)
        {
            mu::log::Get("render")->error("SDL_gpu -- Init called with null window pointer");
            return false;
        }

        // "default" (or empty) means this app's own platform-aware pick; Windows prefers Vulkan
        // (SDL's own default D3D12 pick doesn't vsync-cap FPS correctly -- see the
        // FPS_ANIMATION_FACTOR/CCreditWin fix commits), other platforms pass straight through to
        // SDL's own auto-pick. Any other value forces that specific SDL_gpu driver name
        // ("vulkan"/"direct3d12"/"metal") on any platform. config.ini's [Render] Backend key
        // (GameConfig::GetRenderBackend()) is the source of this value; GameConfigValidation
        // already normalized case/whitespace/the "d3d12" alias.
        std::string requestedBackend(renderBackend);
        if (requestedBackend.empty() || requestedBackend == "default")
        {
#ifdef _WIN32
            requestedBackend = "vulkan";
#else
            requestedBackend.clear();
#endif
        }
        const char* requestedDriverName = requestedBackend.empty() ? nullptr : requestedBackend.c_str();

        // SDL_CreateGPUDevice() with a specific driver name has no built-in fallback (fails
        // outright if that driver isn't available), so retry with nullptr (SDL's own auto-pick)
        // if the requested one fails -- still works on a machine without it, just loses whatever
        // benefit the requested backend had until that's addressed directly.
        mu::log::Get("render")->info("SDL_gpu -- validation: {}, requested backend: {}",
                                     Render::kGpuValidationEnabled ? "enabled" : "disabled",
                                     requestedDriverName ? requestedDriverName : "(SDL auto-pick)");
        s_device =
            SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
                                Render::kGpuValidationEnabled, requestedDriverName);

        if (!s_device && requestedDriverName != nullptr)
        {
            mu::log::Get("render")->warn("SDL_gpu -- '{}' device creation failed ({}), falling back to "
                                         "SDL's own auto-picked driver", requestedDriverName, SDL_GetError());
            s_device = SDL_CreateGPUDevice(
                SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
                Render::kGpuValidationEnabled, nullptr);
        }

        if (!s_device)
        {
            mu::log::Get("render")->error("SDL_gpu -- device creation failed: {}", SDL_GetError());
            return false;
        }

        mu::log::Get("render")->info("SDL_gpu -- device driver: {}", SDL_GetGPUDeviceDriver(s_device));

        const char* driverName = SDL_GetGPUDeviceDriver(s_device);
        ConfigureD3D12Diagnostics(driverName);

        // Claim the window for the GPU device.
        if (!SDL_ClaimWindowForGPUDevice(s_device, s_window))
        {
            mu::log::Get("render")->error("SDL_gpu -- SDL_ClaimWindowForGPUDevice failed: {}", SDL_GetError());
            SDL_DestroyGPUDevice(s_device);
            s_device = nullptr;
            return false;
        }

        // Load real HLSL shader blobs from MU_SHADER_DIR.
        // Driver name used to select the correct blob format (SPIR-V/DXIL/MSL).
        if (!LoadShaders(driverName))
        {
            mu::log::Get("render")->error("SDL_gpu -- shader loading failed; cannot build pipelines");
            SDL_ReleaseWindowFromGPUDevice(s_device, s_window);
            SDL_DestroyGPUDevice(s_device);
            s_device = nullptr;
            return false;
        }

        // Anti-aliasing: the sample count the pipelines and frame targets start with.
        s_supportedSampleCounts = QuerySupportedSampleCounts();
        s_sampleCount = SelectFrameSampleCount();
        s_sampleCountChangePending = false;
        LogSampleCount("at start-up");

        // Build nine blend variants for five required core sets, plus the unused
        // 2D depth-on set and any skinned sets whose optional shader loaded.
        if (!CreatePipelinesForSampleCount())
        {
            mu::log::Get("render")->error("SDL_gpu -- pipeline creation failed during Init");
            DestroyPipelines();
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
            mu::log::Get("render")->error("SDL_gpu -- vertex buffer creation failed");
            DestroyPipelines();
            SDL_ReleaseWindowFromGPUDevice(s_device, s_window);
            SDL_DestroyGPUDevice(s_device);
            s_device = nullptr;
            return false;
        }

        // Create static quad index buffer.
        if (!CreateQuadIndexBuffer())
        {
            mu::log::Get("render")->error("SDL_gpu -- quad index buffer creation failed");
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
                mu::log::Get("render")->error("SDL_gpu -- sampler creation failed: {}", SDL_GetError());
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
            mu::log::Get("render")->error("SDL_gpu -- white texture creation failed");
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

        // Create fog uniform GPU buffer and transfer buffer.
        if (!CreateFogUniformBuffers())
        {
            mu::log::Get("render")->error("SDL_gpu -- fog uniform buffer creation failed");
            if (s_whiteTexture)
            {
                SDL_ReleaseGPUTexture(s_device, s_whiteTexture);
                s_whiteTexture = nullptr;
            }
            if (s_flatNormalTexture)
            {
                SDL_ReleaseGPUTexture(s_device, s_flatNormalTexture);
                s_flatNormalTexture = nullptr;
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

        // Create initial depth texture at window size.
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

#if MU_HAS_SDL_TTF
        if (!TTF_Init())
        {
            mu::log::Get("render")->error("SDL_ttf -- TTF_Init failed: {}", SDL_GetError());
            Shutdown();
            return false;
        }
        s_textEngine = TTF_CreateGPUTextEngine(s_device);
        if (!s_textEngine)
        {
            mu::log::Get("render")->error("SDL_ttf -- TTF_CreateGPUTextEngine failed: {}", SDL_GetError());
            TTF_Quit();
            Shutdown();
            return false;
        }
        if (!LoadTtfFonts(fontFamily, normalPointSize, bigPointSize, fixedPointSize))
        {
            Shutdown();
            return false;
        }
#endif

        if (!s_bitmapGpuResources.Initialize(s_device))
        {
            mu::log::Get("render")->error("SDL_gpu -- bitmap uploader initialization failed at {}: {} (driver {})",
                                          s_bitmapGpuResources.LastStage(), s_bitmapGpuResources.LastError(),
                                          SDL_GetGPUDeviceDriver(s_device));
            Shutdown();
            return false;
        }

        mu::log::Get("render")->info("SDL_gpu -- Init complete");
        return true;
    }

    // -----------------------------------------------------------------------
    // Shutdown: Release all GPU resources and destroy the device.
    // Called at application exit before window destruction.
    // -----------------------------------------------------------------------
    static void Shutdown()
    {
        if (!s_device)
        {
            s_frameReadbackState.Reset();
            s_frameReadbackTexture = nullptr;
            return;
        }

        ReleaseFrameReadbackTexture();
        s_frameReadbackState.Reset();

#if MU_HAS_SDL_TTF
        // Destroy SDL_ttf resources before the GPU device: the fonts first, then the engine.
        s_ttfFonts.Close();
        if (s_textEngine)
        {
            TTF_DestroyGPUTextEngine(s_textEngine);
            s_textEngine = nullptr;
            TTF_Quit();
        }
#endif

        s_textureUpdates.clear();
        ReleaseOwnedTextures();
        ClearTextureRegistry();
        ClearSamplerRegistry();
        if (!s_bitmapGpuResources.Shutdown())
        {
            mu::log::Get("render")->error("SDL_gpu -- bitmap uploader shutdown failed at {}: {}",
                                          s_bitmapGpuResources.LastStage(), s_bitmapGpuResources.LastError());
        }

        // Release the independently owned white fallback texture.
        if (s_whiteTexture)
        {
            SDL_ReleaseGPUTexture(s_device, s_whiteTexture);
            s_whiteTexture = nullptr;
        }
        if (s_flatNormalTexture)
        {
            SDL_ReleaseGPUTexture(s_device, s_flatNormalTexture);
            s_flatNormalTexture = nullptr;
        }

        if (s_defaultSampler)
        {
            SDL_ReleaseGPUSampler(s_device, s_defaultSampler);
            s_defaultSampler = nullptr;
        }

        ReleaseMultisampleColorTexture();

        // Release depth texture.
        if (s_depthTexture)
        {
            SDL_ReleaseGPUTexture(s_device, s_depthTexture);
            s_depthTexture = nullptr;
            s_depthW = 0u;
            s_depthH = 0u;
        }
        if (s_mainColorTarget)
        {
            SDL_ReleaseGPUTexture(s_device, s_mainColorTarget);
            s_mainColorTarget = nullptr;
            s_mainColorW = 0u;
            s_mainColorH = 0u;
            s_mainColorFormat = SDL_GPU_TEXTUREFORMAT_INVALID;
        }
        if (s_offscreenDepthTexture)
        {
            SDL_ReleaseGPUTexture(s_device, s_offscreenDepthTexture);
            s_offscreenDepthTexture = nullptr;
            s_offscreenDepthW = 0u;
            s_offscreenDepthH = 0u;
        }

        // Release fog uniform buffers.
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
        DestroyBoneBuffers();
        DestroyTerrainHeightBuffers();
        DestroySkinnedMeshCache();
        DestroyPipelines();
        ReleaseShaders();
        s_postProcessing.Release();

        SDL_ReleaseWindowFromGPUDevice(s_device, s_window);
        SDL_DestroyGPUDevice(s_device);
        s_device = nullptr;
        s_window = nullptr;
        s_disableD3D12Culling = false;
        s_disableD3D12TriangleMerging = false;

        mu::log::Get("render")->info("SDL_gpu -- Shutdown complete");
    }

    // Waits on the same frame-in-flight fence as BeginFrame's acquire, so the acquire after it returns at once.
    void WaitForSwapchain() override
    {
        if (!s_device || !s_window)
        {
            return;
        }
        if (!SDL_WaitForGPUSwapchain(s_device, s_window))
        {
            mu::log::Get("render")->error("SDL_gpu -- SDL_WaitForGPUSwapchain failed: {}", SDL_GetError());
        }
    }

    // -----------------------------------------------------------------------
    // BeginFrame: Acquire command buffer, swapchain texture, and begin render pass.
    // Called once per frame before any draw calls.
    // -----------------------------------------------------------------------
    void BeginFrame() override
    {
        if (!s_device || !s_window)
        {
            FailPendingFrameReadback();
            return;
        }

        if (!s_frameTimingInitialized)
        {
            s_frameTimingEnabled = std::getenv("MU_RENDER_TIMING") != nullptr;
            s_frameTimingInitialized = true;
        }
        s_frameBeginTime = std::chrono::steady_clock::now();

        // Between frames: no draw has recorded a pipeline of the old sample count yet.
        ApplyPendingSampleCount();

        s_cmdBuf = SDL_AcquireGPUCommandBuffer(s_device);
        if (!s_cmdBuf)
        {
            mu::log::Get("render")->error("SDL_gpu -- SDL_AcquireGPUCommandBuffer failed: {}", SDL_GetError());
            FailPendingFrameReadback();
            return;
        }

        // F-7 fix: Cache window dimensions once per frame for text Y-flip.
        SDL_GetWindowSize(s_window, &s_cachedWinW, &s_cachedWinH);

        // Reset vertex scratch offset, deferred command list, and per-frame diagnostics.
        // Texture uploads may be queued by legacy asset loading before BeginFrame,
        // so keep s_textureUpdates until EndFrame has submitted them.
        s_vtxOffset = 0u;
        s_stagedVertices = {};
        s_stagedBones = {};
        s_stagedStripIndices = {};
        s_renderCmds.clear();
        s_skinningUniforms.clear();
        s_shadowUniforms.clear();
        s_previousDrawCommands.fill(kNoDrawCommand);
        s_sceneBoundary.Reset();
        if (s_bloomExclusionDepth != 0)
        {
            mu::log::Get("render")->warn("SDL_gpu -- {} bloom exclusion scope(s) left open; reset",
                                         s_bloomExclusionDepth);
            s_bloomExclusionDepth = 0;
        }

        s_replayedCmdCount = 0u;
        s_mainColorPassOpenedThisFrame = false;
#ifdef _EDITOR
        s_editorDrawPending = false;
#endif
        s_stripIdxScratch.clear();
        s_boneRowScratch.clear();
        s_lastBonePalette = nullptr;
        s_lastBonePaletteSize = 0u;
        s_lastBonePaletteVersion = 0u;
        s_lastBonePaletteRowOffset = 0u;
        s_lastBonePaletteBones = 0u;
        // Before the reset below, so these invalidate nothing this frame records.
        for (const std::uint32_t textureId : s_pendingRenderTargetReleases)
        {
            ReleaseOwnedTextureById(textureId);
        }
        s_pendingRenderTargetReleases.clear();
        s_texturesInvalidated = false; // Reset per-frame texture invalidation flag
        if (s_screenSpaceSamplingDepth != 0)
        {
            mu::log::Get("render")->warn("SDL_gpu -- {} screen-space sampling scope(s) left open; reset",
                                         s_screenSpaceSamplingDepth);
            s_screenSpaceSamplingDepth = 0;
            InvalidateSamplerLookupCaches();
        }
        s_dbgDrawCallsThisFrame = 0u;
        s_dbgVtxBytesThisFrame = 0u;
        s_dbgFallbackTextureThisFrame = 0u;
        s_dbgTextureUploadsThisFrame = 0u;
        s_dbgTextureCreatesThisFrame = 0u;
        s_dbgTextureReleasesThisFrame = 0u;
        s_dbgRenderCmdsReplayedThisFrame = 0u;
        s_dbgGpuDrawCallsThisFrame = 0u;
        s_dbgMergedDrawsThisFrame = 0u;
        s_dbgMerged2DDrawsThisFrame = 0u;
        s_dbgWhiteTextureDrawsThisFrame = 0u;
        s_dbgRealTextureDrawsThisFrame = 0u;
        s_dbgPipelineBindsThisFrame = 0u;
        s_dbgSamplerBindsThisFrame = 0u;
        s_skinnedDrawKeys.clear();
        s_dbgVertexUniformPushesThisFrame = 0u;
        s_dbgFragmentUniformPushesThisFrame = 0u;
        ++s_dbgFrameCount;

        s_swapW = 0u;
        s_swapH = 0u;
        s_swapchainTexture = nullptr;

        // Waits until a swapchain texture is free: with VSync on and no lower FPS cap, that wait is what holds the
        // game loop to the display's refresh rate. SDL_AcquireGPUSwapchainTexture returns no texture instead while
        // the frames in flight are full, and the loop ran on without drawing, several times per displayed frame on
        // Vulkan and Direct3D 12 alike. The game loop already waited in WaitForSwapchain, before reading input, so
        // this returns at once there; it still paces the frames drawn outside it (the title and loading screens).
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(s_cmdBuf, s_window, &s_swapchainTexture, &s_swapW, &s_swapH))
        {
            mu::log::Get("render")->error("SDL_gpu -- SDL_WaitAndAcquireGPUSwapchainTexture failed: {}",
                                          SDL_GetError());
            SDL_CancelGPUCommandBuffer(s_cmdBuf);
            s_cmdBuf = nullptr;
            FailPendingFrameReadback();
            return;
        }

        // If swapchain texture is null, skip the frame: a hidden window, or on Vulkan a minimized one whose
        // surface the driver reports as 0x0 (Direct3D 12 keeps rendering a minimized window).
        if (!s_swapchainTexture)
        {
            // A requested readback stays pending for the next frame that renders;
            // its consumer decides how long to wait.
            SDL_CancelGPUCommandBuffer(s_cmdBuf);
            s_cmdBuf = nullptr;
            return;
        }

        // Fog/alpha uniform is now pushed per-draw-call via
        // SDL_PushGPUFragmentUniformData — no copy pass needed here.

        // Ensure depth texture matches swapchain dimensions.
        // Recreates on first frame or when window is resized.
        CreateOrResizeDepthTexture(s_swapW, s_swapH);
        if (s_sampleCount != Render::Antialiasing::OffSampleCount && !EnsureMultisampleColorTexture())
        {
            // Without its colour target the frame cannot use multisampled
            // pipelines: drop to one sample before anything is recorded.
            mu::log::Get("render")->error("SDL_gpu -- anti-aliasing turned off: the multisampled target is missing");
            s_requestedSampleCount = Render::Antialiasing::OffSampleCount;
            RebuildForSampleCount(Render::Antialiasing::OffSampleCount);
            CreateOrResizeDepthTexture(s_swapW, s_swapH);
        }

        CreateOrResizeMainColorTarget(s_swapW, s_swapH);

        ResolveFrameColorTarget();

        // Deferred rendering: do NOT begin the render pass here.
        // Draw calls record RenderCmds into s_renderCmds during the frame.
        // EndFrame will: copy vertex data → begin render pass → replay → end → submit.
        s_frameActive = true;
    }

    // Where the replayed draws are placed and clipped; carried from one pass to the next.
    struct ReplayView
    {
        SDL_GPUViewport viewport;
        SDL_Rect scissor;
    };

    enum class ReplaySelection : std::uint8_t
    {
        AllCommands,
        BloomSources,
    };

    [[nodiscard]] static ReplayView FullFrameView()
    {
        return {{0.0f, 0.0f, static_cast<float>(s_swapW), static_cast<float>(s_swapH), 0.0f, 1.0f},
                {0, 0, static_cast<int>(s_swapW), static_cast<int>(s_swapH)}};
    }

    [[nodiscard]] static bool IsMultisampled()
    {
        return s_sampleCount != Render::Antialiasing::OffSampleCount;
    }

    [[nodiscard]] static bool IsPostProcessingActive()
    {
        return s_postProcessingEnabled && s_postProcessing.IsAvailable();
    }

    // The pipeline a bloom source replays with into the bloom pass, or null for
    // a draw that is none. A draw that wrote depth uses the read-only variant of
    // its set: the scene depth already holds its own surface, which the writing
    // variant's strict test would reject.
    [[nodiscard]] static SDL_GPUGraphicsPipeline* GetBloomSourcePipeline(const RenderCmd& command)
    {
        if (!Render::PostProcessing::IsBloomSource(command.blendEnabled, command.blendMode, command.bloomExcluded))
        {
            return nullptr;
        }
        if (!command.depthTestEnabled || !command.depthMaskEnabled)
        {
            return command.pipeline;
        }

        const auto index = static_cast<std::size_t>(command.blendMode);
        for (const WorldPipelineSetSpec& spec : WorldPipelineSets())
        {
            if (SDL_GPUGraphicsPipeline* readOnly = spec.set->DepthReadOnlyFor(command.pipeline, index))
            {
                return readOnly;
            }
        }
        return nullptr;
    }

    // Replays commands [begin, end) into s_renderPass, starting from `view`;
    // `view` ends as the last replayed command left it.
    static void ReplayCommands(std::size_t begin, std::size_t end, bool boneDataReady, ReplaySelection selection,
                               ReplayView& view)
    {
        // Track explicit state. Pipeline changes re-apply the active scissor because
        // SDL GPU backends may discard it while binding a new pipeline.
        Render::SdlGpuReplayState replayState;
        if (replayState.SelectViewport(view.viewport))
            SDL_SetGPUViewport(s_renderPass, &view.viewport);
        if (replayState.SelectScissor(view.scissor))
            SDL_SetGPUScissor(s_renderPass, &view.scissor);

        const bool bloomSourcesOnly = selection == ReplaySelection::BloomSources;
        for (std::size_t index = begin; index < end; ++index)
        {
            const RenderCmd& cmd = s_renderCmds[index];
            // Replay state and editor commands after texture invalidation, but skip
            // game draws because their deferred texture pointers may be dangling.
            if (s_texturesInvalidated && IsUnsafeInvalidatedDrawCommand(cmd.type))
            {
                continue;
            }
            if (cmd.consumedByOffscreenCapture)
            {
                continue; // already drawn into its own offscreen texture above
            }
            if (!bloomSourcesOnly)
            {
                ++s_dbgRenderCmdsReplayedThisFrame;
            }

            switch (cmd.type)
            {
            case RenderCmdType::SetViewport:
            {
                view.viewport = cmd.viewport;
                if (replayState.SelectViewport(view.viewport))
                    SDL_SetGPUViewport(s_renderPass, &view.viewport);
                break;
            }

            case RenderCmdType::SetScissor:
            {
                view.scissor = cmd.scissor;
                if (replayState.SelectScissor(view.scissor))
                    SDL_SetGPUScissor(s_renderPass, &view.scissor);
                break;
            }


            case RenderCmdType::DrawTriangles:
            case RenderCmdType::DrawSkinnedTriangles:
            case RenderCmdType::DrawIndexedQuads:
            case RenderCmdType::DrawIndexedStrip:
            case RenderCmdType::DrawTriangles2D:
            {
                SDL_GPUGraphicsPipeline* pipeline = bloomSourcesOnly ? GetBloomSourcePipeline(cmd) : cmd.pipeline;
                if (pipeline)
                {
                    ReplayDrawCommand(cmd, pipeline, boneDataReady, view.scissor, replayState);
                }
                break;
            }
            } // switch
        }
    }

    [[nodiscard]] static SDL_GPUDepthStencilTargetInfo MakeDepthTarget(SDL_GPULoadOp loadOp, SDL_GPUStoreOp storeOp)
    {
        SDL_GPUDepthStencilTargetInfo depthTarget{};
        depthTarget.texture = s_depthTexture;
        depthTarget.clear_depth = 1.0f;
        depthTarget.load_op = loadOp;
        depthTarget.store_op = storeOp;
        depthTarget.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
        depthTarget.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
        // A cleared depth buffer may be swapped for a fresh one instead of waiting
        // for an earlier frame. A loaded one must not: outside debug mode SDL
        // would cycle it anyway and the pass would load undefined depth.
        depthTarget.cycle = loadOp != SDL_GPU_LOADOP_LOAD;
        return depthTarget;
    }

    // A frame-private target is replaced by a fresh one instead of waiting for an
    // earlier frame still reading it; its old content is not loaded anyway.
    [[nodiscard]] static SDL_GPUColorTargetInfo CycledTarget(SDL_GPUColorTargetInfo target)
    {
        if (target.store_op == SDL_GPU_STOREOP_RESOLVE)
        {
            target.cycle_resolve_texture = true;
        }
        else
        {
            target.cycle = target.load_op != SDL_GPU_LOADOP_LOAD;
        }
        return target;
    }

    [[nodiscard]] static bool BeginReplayPass(const SDL_GPUColorTargetInfo& colorTarget,
                                              const SDL_GPUDepthStencilTargetInfo& depthTarget)
    {
        s_renderPass = SDL_BeginGPURenderPass(s_cmdBuf, &colorTarget, 1, s_depthTexture ? &depthTarget : nullptr);
        if (!s_renderPass)
        {
            return false;
        }
        PushSceneLightingUniforms();
        return true;
    }

    static void EndReplayPass()
    {
        SDL_EndGPURenderPass(s_renderPass);
        s_renderPass = nullptr;
    }

    // The whole frame in one pass into `destination`. Multisampled, the pass
    // resolves into it, so the readback, capture and swapchain paths see one sample.
    [[nodiscard]] static bool ReplayFrame(SDL_GPUTexture* destination, bool boneDataReady)
    {
        const SDL_GPUColorTargetInfo colorTarget = mu::GetSdlGpuFrameColorTarget(
            {destination, IsMultisampled() ? s_multisampleColorTexture : nullptr}, s_clearColor);
        // Nothing reads depth after the pass; skip storing every sample of it.
        const SDL_GPUDepthStencilTargetInfo depthTarget =
            MakeDepthTarget(SDL_GPU_LOADOP_CLEAR, IsMultisampled() ? SDL_GPU_STOREOP_DONT_CARE : SDL_GPU_STOREOP_STORE);
        if (!BeginReplayPass(colorTarget, depthTarget))
        {
            return false;
        }
        ReplayView view = FullFrameView();
        ReplayCommands(0u, s_renderCmds.size(), boneDataReady, ReplaySelection::AllCommands, view);
        EndReplayPass();
        return true;
    }

    // The commands recorded since the last FlushRenderCommands(), on top of what it drew.
    [[nodiscard]] bool ContinueFlushedFrame(SDL_GPUTexture* destination, bool boneDataReady)
    {
        SDL_GPUColorTargetInfo colorTarget{};
        colorTarget.texture = destination;
        colorTarget.load_op = SDL_GPU_LOADOP_LOAD;
        colorTarget.store_op = SDL_GPU_STOREOP_STORE;
        SDL_GPUTexture* const depthTexture = SingleSamplePassDepth();
        SDL_GPUDepthStencilTargetInfo depthTarget{};
        depthTarget.texture = depthTexture;
        depthTarget.load_op = SDL_GPU_LOADOP_LOAD;
        depthTarget.store_op = SDL_GPU_STOREOP_STORE;
        depthTarget.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
        depthTarget.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
        s_renderPass = SDL_BeginGPURenderPass(s_cmdBuf, &colorTarget, 1, depthTexture ? &depthTarget : nullptr);
        if (!s_renderPass)
        {
            return false;
        }
        ReplayCommandRange(s_replayedCmdCount, s_renderCmds.size(), boneDataReady);
        EndReplayPass();
        return true;
    }

    // The depth a single-sample pass at the frame's size uses: the frame's own, unless that one
    // is multisampled. Null when neither exists.
    [[nodiscard]] static SDL_GPUTexture* SingleSamplePassDepth()
    {
        if (!IsMultisampled())
        {
            return s_depthTexture;
        }
        if (s_singleSampleDepthTexture && s_singleSampleDepthW == s_swapW && s_singleSampleDepthH == s_swapH)
        {
            return s_singleSampleDepthTexture;
        }
        ReleaseSingleSampleDepthTexture();
        SDL_GPUTextureCreateInfo depthInfo{};
        depthInfo.type = SDL_GPU_TEXTURETYPE_2D;
        depthInfo.format = mu::SdlGpuDepthFormat;
        depthInfo.width = s_swapW;
        depthInfo.height = s_swapH;
        depthInfo.layer_count_or_depth = 1;
        depthInfo.num_levels = 1;
        depthInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
        depthInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
        s_singleSampleDepthTexture = SDL_CreateGPUTexture(s_device, &depthInfo);
        if (!s_singleSampleDepthTexture)
        {
            mu::log::Get("render")->error("SDL_gpu -- single-sample depth creation failed ({}x{}): {}", s_swapW,
                                          s_swapH, SDL_GetError());
            return nullptr;
        }
        s_singleSampleDepthW = s_swapW;
        s_singleSampleDepthH = s_swapH;
        return s_singleSampleDepthTexture;
    }

    static void ReleaseSingleSampleDepthTexture()
    {
        if (s_singleSampleDepthTexture)
        {
            SDL_ReleaseGPUTexture(s_device, s_singleSampleDepthTexture);
            s_singleSampleDepthTexture = nullptr;
        }
        s_singleSampleDepthW = 0u;
        s_singleSampleDepthH = 0u;
    }

    // The scene's additive draws once more, into the bloom source, tested
    // against the scene depth; then the bloom chain.
    static void RenderBloomSources(std::size_t sceneEnd, bool boneDataReady)
    {
        constexpr SDL_FColor noLight{0.0f, 0.0f, 0.0f, 0.0f};
        const SDL_GPUColorTargetInfo colorTarget = CycledTarget(mu::GetSdlGpuFrameColorTarget(
            {s_postProcessing.GetBloomSourceTexture(), IsMultisampled() ? s_multisampleColorTexture : nullptr},
            noLight));
        // The UI pass tests against the same depth, so it is kept.
        if (!BeginReplayPass(colorTarget, MakeDepthTarget(SDL_GPU_LOADOP_LOAD, SDL_GPU_STOREOP_STORE)))
        {
            return;
        }
        ReplayView view = FullFrameView();
        ReplayCommands(0u, sceneEnd, boneDataReady, ReplaySelection::BloomSources, view);
        EndReplayPass();
        s_postProcessing.RenderBloom(s_cmdBuf, s_postProcessingLook.bloomLevelFalloff);
    }

    // The scene into the scene texture, its bloom, then the processed scene and
    // the UI on top of it into `destination`.
    [[nodiscard]] static bool ReplayPostProcessedFrame(SDL_GPUTexture* destination, std::size_t sceneEnd,
                                                       bool boneDataReady)
    {
        SDL_GPUTexture* const multisampled = IsMultisampled() ? s_multisampleColorTexture : nullptr;
        ReplayView view = FullFrameView();

        // Its depth stays for the bloom and UI passes, which test against it as
        // the UI did when it shared the scene's pass.
        const SDL_GPUColorTargetInfo sceneTarget = CycledTarget(
            mu::GetSdlGpuFrameColorTarget({s_postProcessing.GetSceneTexture(), multisampled}, s_clearColor));
        if (!BeginReplayPass(sceneTarget, MakeDepthTarget(SDL_GPU_LOADOP_CLEAR, SDL_GPU_STOREOP_STORE)))
        {
            return false;
        }
        ReplayCommands(0u, sceneEnd, boneDataReady, ReplaySelection::AllCommands, view);
        EndReplayPass();

        if (s_postProcessingLook.bloomStrength > 0.0f)
        {
            RenderBloomSources(sceneEnd, boneDataReady);
        }

        // The composite covers every pixel: the target's old content is not loaded.
        // A diagnostic view without its image this frame leaves black instead.
        constexpr SDL_FColor black{0.0f, 0.0f, 0.0f, 1.0f};
        SDL_GPUColorTargetInfo uiTarget = mu::GetSdlGpuFrameColorTarget({destination, multisampled}, black);
        if (s_postProcessingView == mu::PostProcessingView::Scene)
        {
            uiTarget.load_op = SDL_GPU_LOADOP_DONT_CARE;
        }
        if (!BeginReplayPass(uiTarget, MakeDepthTarget(SDL_GPU_LOADOP_LOAD, SDL_GPU_STOREOP_DONT_CARE)))
        {
            return false;
        }
        s_postProcessing.DrawComposite(s_cmdBuf, s_renderPass, {s_swapW, s_swapH}, s_postProcessingLook,
                                       s_postProcessingView);
        ReplayCommands(sceneEnd, s_renderCmds.size(), boneDataReady, ReplaySelection::AllCommands, view);
        EndReplayPass();
        return true;
    }

    // -----------------------------------------------------------------------
    // EndFrame: End render pass, flush vertex transfer buffer, and submit.
    // Called once per frame after all draw calls.
    // Replaces SDL_GL_SwapWindow / SwapBuffers in the game loop.
    //
    // After ending the render pass, unmap the vertex
    // transfer buffer and issue a single copy pass to flush the frame's
    // accumulated vertex data to the GPU vertex buffer. The next frame's
    // render pass will read from the updated GPU buffer.
    // -----------------------------------------------------------------------
    // Queues the meshes cached since the last call into their block ranges. Those ranges are new, so no
    // frame reads them yet and the shared blocks are written in place instead of cycled. False keeps them
    // pending for the next call.
    [[nodiscard]] static bool StageSkinnedMeshes(std::vector<BufferUpload>& uploads)
    {
        if (s_skinnedMeshStaging.size() > std::numeric_limits<Uint32>::max())
        {
            return false;
        }
        const Uint32 bytes = static_cast<Uint32>(s_skinnedMeshStaging.size());
        if (s_skinnedMeshTransferCapacity < bytes)
        {
            const Uint32 capacity =
                std::max({bytes, k_SkinnedMeshBlockUnits * k_SkinnedMeshUnitBytes, s_skinnedMeshTransferCapacity * 2u});
            SDL_GPUTransferBufferCreateInfo transferInfo{};
            transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
            transferInfo.size = capacity;
            SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(s_device, &transferInfo);
            if (!transfer)
            {
                return false;
            }
            if (s_skinnedMeshTransferBuf)
            {
                SDL_ReleaseGPUTransferBuffer(s_device, s_skinnedMeshTransferBuf);
            }
            s_skinnedMeshTransferBuf = transfer;
            s_skinnedMeshTransferCapacity = capacity;
        }

        void* mapped = SDL_MapGPUTransferBuffer(s_device, s_skinnedMeshTransferBuf, true);
        if (!mapped)
        {
            return false;
        }
        std::memcpy(mapped, s_skinnedMeshStaging.data(), bytes);
        SDL_UnmapGPUTransferBuffer(s_device, s_skinnedMeshTransferBuf);
        s_dbgVtxBytesThisFrame += bytes;
        FrameProfiler::Count(FrameProfiler::Counter::VertexBytes, bytes);

        for (const PendingSkinnedMeshUpload& pending : s_pendingSkinnedMeshUploads)
        {
            uploads.push_back({s_skinnedMeshTransferBuf, s_skinnedMeshBlocks[pending.range.block].buffer,
                               pending.range.firstUnit * k_SkinnedMeshUnitBytes,
                               pending.range.vertexCount * pending.range.stride, false,
                               static_cast<Uint32>(pending.stagingFirst)});
        }
        s_pendingSkinnedMeshUploads.clear();
        s_skinnedMeshStaging.clear();
        return true;
    }

    // Stages this frame's accumulated CPU-side vertex/bone/strip-index/texture-update scratch
    // buffers to the GPU via one copy pass. Safe to call more than once per frame (EndFrame and
    // the post-RmlUi pass both do): scratch data is only ever appended, so a later call sends just
    // what was recorded since the last one. Returns whether bone data is ready, for
    // ReplayCommandRange to pass into ReplayDrawCommand.
    bool StageDeferredGpuData()
    {
        // Staged BEFORE the render pass so the GPU reads current-frame data, eliminating the
        // 1-frame vertex delay that caused streak artifacts when vertex counts varied between frames.
        std::vector<BufferUpload> bufferUploads;

        if (s_vtxOffset > 0u && EnsureVertexBufferCapacity(s_vtxOffset) &&
            !StageAppended(s_vtxTransferBuf, s_vtxGpuBuf, s_vtxScratch.data(), s_vtxOffset, s_stagedVertices,
                           bufferUploads))
        {
            mu::log::Get("render")->error("SDL_gpu -- failed to map vertex transfer buffer: {}", SDL_GetError());
        }

        bool boneDataReady = s_boneRowScratch.empty();
        if (!s_boneRowScratch.empty())
        {
            const std::size_t boneBytes = s_boneRowScratch.size() * sizeof(float);
            if (boneBytes <= std::numeric_limits<Uint32>::max())
            {
                const Uint32 boneDataSize = static_cast<Uint32>(boneBytes);
                boneDataReady = EnsureBoneBufferCapacity(boneDataSize) &&
                                StageAppended(s_boneTransferBuf, s_boneGpuBuf, s_boneRowScratch.data(), boneDataSize,
                                              s_stagedBones, bufferUploads);
            }
        }

        if (!s_pendingSkinnedMeshUploads.empty() && !StageSkinnedMeshes(bufferUploads))
        {
            mu::log::Get("render")->error("SDL_gpu -- failed to stage cached skinned meshes: {}", SDL_GetError());
            // Their ranges hold nothing yet; skinned draws wait for the retry rather than read them.
            boneDataReady = false;
        }

        if (s_terrainHeightUploadPending)
        {
            constexpr Uint32 gridBytes = sizeof(Render::TerrainHeightGrid);
            bufferUploads.push_back({s_terrainHeightTransferBuf, s_terrainHeightGpuBuf, 0u, gridBytes, true});
            s_terrainHeightUploadPending = false;
            s_dbgVtxBytesThisFrame += gridBytes;
            FrameProfiler::Count(FrameProfiler::Counter::VertexBytes, gridBytes);
        }

        // EnsureStripIndexBuffer may release/create GPU resources, so it runs before the copy pass.
        if (!s_stripIdxScratch.empty() && EnsureStripIndexBuffer(static_cast<Uint32>(s_stripIdxScratch.size())))
        {
            const Uint32 totalIdxBytes = static_cast<Uint32>(s_stripIdxScratch.size() * sizeof(Uint16));
            StageAppended(s_stripIdxTransfer, s_stripIdxBuf, s_stripIdxScratch.data(), totalIdxBytes,
                          s_stagedStripIndices, bufferUploads);
        }

        // Prepare texture update transfer buffers BEFORE the copy pass.
        // Each queued texture update gets a temporary transfer buffer.
        struct PreparedTexUpload
        {
            SDL_GPUTransferBuffer* transfer;
            SDL_GPUTexture* gpuTexture;
            Uint32 width;
            Uint32 height;
            Uint32 bytesPerRow;
        };
        std::vector<PreparedTexUpload> preparedTexUploads;

        for (const auto& tu : s_textureUpdates)
        {
            const Uint32 dataSize = tu.bytesPerRow * tu.height;
            SDL_GPUTransferBufferCreateInfo tbInfo{};
            tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
            tbInfo.size = dataSize;
            SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(s_device, &tbInfo);
            if (!tb)
            {
                continue;
            }
            void* mapped = SDL_MapGPUTransferBuffer(s_device, tb, false);
            if (mapped)
            {
                std::memcpy(mapped, tu.pixelsCopy.data(), dataSize);
                SDL_UnmapGPUTransferBuffer(s_device, tb);
                preparedTexUploads.push_back({tb, tu.gpuTexture, tu.width, tu.height, tu.bytesPerRow});
            }
            else
            {
                SDL_ReleaseGPUTransferBuffer(s_device, tb);
            }
        }

        if (!bufferUploads.empty() || !preparedTexUploads.empty())
        {
            SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(s_cmdBuf);
            if (copyPass)
            {
                for (const BufferUpload& upload : bufferUploads)
                {
                    SDL_GPUTransferBufferLocation source{};
                    source.transfer_buffer = upload.transfer;
                    source.offset = upload.transferOffset;

                    SDL_GPUBufferRegion destination{};
                    destination.buffer = upload.buffer;
                    destination.offset = upload.offset;
                    destination.size = upload.size;

                    SDL_UploadToGPUBuffer(copyPass, &source, &destination, upload.cycle);
                }

                // Upload queued dynamic texture updates.
                for (const auto& pu : preparedTexUploads)
                {
                    SDL_GPUTextureTransferInfo texSrc{};
                    texSrc.transfer_buffer = pu.transfer;
                    texSrc.offset = 0;
                    texSrc.pixels_per_row = pu.width;
                    texSrc.rows_per_layer = pu.height;

                    SDL_GPUTextureRegion texDst{};
                    texDst.texture = pu.gpuTexture;
                    texDst.w = pu.width;
                    texDst.h = pu.height;
                    texDst.d = 1;

                    SDL_UploadToGPUTexture(copyPass, &texSrc, &texDst, true);
                }

                SDL_EndGPUCopyPass(copyPass);
            }

            // Release temporary transfer buffers.
            for (const auto& pu : preparedTexUploads)
            {
                SDL_ReleaseGPUTransferBuffer(s_device, pu.transfer);
            }
        }
        s_textureUpdates.clear();

        return boneDataReady;
    }

    // Replays s_renderCmds[startIdx, endIdx) into the currently-open s_renderPass. Every render
    // pass boundary (the main pass, the post-RmlUi pass, and FlushRenderCommands' own pass) starts
    // its own range with a fresh default full-window viewport/scissor and its own
    // SdlGpuReplayState -- consistent with how the pre-existing post-RmlUi pass boundary has
    // always worked, not a new limitation introduced here: a SetViewport/SetScissor command
    // mid-range still applies correctly: it's only *implicit* state carried over from before the
    // range (never re-declared) that resets to full-window at each new pass, same as it always has
    // at the one pre-existing pass boundary.
    void ReplayCommandRange(std::size_t startIdx, std::size_t endIdx, bool boneDataReady)
    {
        // These passes render into single-sample textures (see s_singleSamplePipelines).
        PushSceneLightingUniforms();
        s_singleSamplePass = true;
        ReplayView view = FullFrameView();
        ReplayCommands(startIdx, endIdx, boneDataReady, ReplaySelection::AllCommands, view);
        s_singleSamplePass = false;
    }

    // RmlUi-behind-3D-icons seam: opens a real render pass NOW, mid-recording, instead of waiting
    // for EndFrame's single one -- lets a caller paint behind a live 3D render despite RmlUi's
    // main context always compositing last. CLEARs on the first flush of the frame, LOADs on every one after
    // (including EndFrame's own final one, see its own call to ReplayCommandRange below) --
    // s_mainColorPassOpenedThisFrame/s_replayedCmdCount track that across calls, reset only in
    // BeginFrame. A caller (e.g. a second Rml::Context's Render(), or any other content that must
    // sit behind whatever legacy drawing happens right after this call returns) should follow this
    // with its own draw calls before the frame's recording continues.
    //
    // Same characterization as the pre-existing pre-submit callback's own comment: on a frame that
    // took the screenshot/readback branch, this always targets s_swapchainTexture directly, not
    // the substituted readback/capture texture -- content flushed here on such a frame won't
    // appear in the captured image. Accepted, matching that already-established precedent, not a
    // new limitation.
    void FlushRenderCommands() override
    {
        if (!s_frameActive || !s_cmdBuf || !s_swapchainTexture)
        {
            return; // no frame in progress (minimized window, or called outside Begin/EndFrame)
        }
        if (s_renderCmds.size() <= s_replayedCmdCount)
        {
            return; // nothing recorded since the last flush -- avoid an empty pass
        }

        const bool boneDataReady = StageDeferredGpuData();

        SDL_GPUColorTargetInfo colorTarget{};
        colorTarget.texture = FrameColorTexture();
        colorTarget.load_op = s_mainColorPassOpenedThisFrame ? SDL_GPU_LOADOP_LOAD : SDL_GPU_LOADOP_CLEAR;
        if (!s_mainColorPassOpenedThisFrame)
        {
            colorTarget.clear_color = s_clearColor;
        }
        colorTarget.store_op = SDL_GPU_STOREOP_STORE;

        // One sample even while the frame is multisampled: see s_singleSamplePipelines.
        SDL_GPUTexture* const depthTexture = SingleSamplePassDepth();
        SDL_GPUDepthStencilTargetInfo depthTarget{};
        depthTarget.texture = depthTexture;
        depthTarget.load_op = s_mainColorPassOpenedThisFrame ? SDL_GPU_LOADOP_LOAD : SDL_GPU_LOADOP_CLEAR;
        if (!s_mainColorPassOpenedThisFrame)
        {
            depthTarget.clear_depth = 1.0f;
        }
        depthTarget.store_op = SDL_GPU_STOREOP_STORE;
        depthTarget.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
        depthTarget.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;

        s_renderPass = SDL_BeginGPURenderPass(s_cmdBuf, &colorTarget, 1, depthTexture ? &depthTarget : nullptr);
        if (s_renderPass)
        {
            ReplayCommandRange(s_replayedCmdCount, s_renderCmds.size(), boneDataReady);
            SDL_EndGPURenderPass(s_renderPass);
            s_renderPass = nullptr;
        }

        s_mainColorPassOpenedThisFrame = true;
        s_replayedCmdCount = s_renderCmds.size();

        // Commands recorded after this point must not merge backward into a command from the
        // pass that just closed -- same reasoning as the post-RmlUi callback's identical reset
        // below (MergeAdjacentQuadCommand/MergeAdjacentTriangleCommand would otherwise grow an
        // already-replayed, already-closed command instead of appending a new one).
        s_previousDrawCommands.fill(kNoDrawCommand);
    }

    void EndFrame() override
    {
        if (!s_frameActive)
        {
            // Frame was not started (minimized window or error). BeginFrame
            // already failed a pending readback on an error; a skipped frame
            // keeps it for the next one.
            if (s_cmdBuf)
            {
                SDL_CancelGPUCommandBuffer(s_cmdBuf);
                s_cmdBuf = nullptr;
            }
            return;
        }
        // Loading screens can submit several renderer frames within one game-loop sample.
        const float submittedBefore = FrameProfiler::AccumulatorMs(FrameProfiler::Pass::RendererSubmit);
        // Still recording, and after the last FlushRenderCommands(), so nothing this records has
        // been replayed yet -- see SetOffscreenRenderCallback (MuRenderer.h).
        if (s_offscreenRenderCallback)
        {
            FRAME_PROFILE(RendererOffscreen);
            s_offscreenRenderCallback();
        }
        s_frameActive = false;

        // ---------------------------------------------------------------
        // Stage this frame's remaining accumulated vertex/bone/strip/texture data
        // (StageDeferredGpuData is safe to call again even if FlushRenderCommands already
        // called it earlier this same frame -- see its own comment).
        // ---------------------------------------------------------------
        bool boneDataReady = false;
        {
            FRAME_PROFILE(RendererStage);
            boneDataReady = StageDeferredGpuData();
        }

        // Replay pending offscreen captures into their own textures now that the GPU vertex
        // buffer holds their data: before the main pass below, which must skip them, and before
        // the UI seams, which sample them.
        {
            FRAME_PROFILE(RendererOffscreen);
            ProcessPendingOffscreenCaptures(boneDataReady);
        }

        // ---------------------------------------------------------------
        // Render pass — replay all recorded draw commands.
        // The GPU vertex/index buffers now contain current-frame data.
        // ---------------------------------------------------------------
        // The frame's colour target was chosen in BeginFrame() (ResolveFrameColorTarget); a
        // request that arrived mid-frame is honoured late. The main pass below always clears its
        // target and replays every recorded command (multisampled and post-processed frames
        // have no partial pass to continue), so a late target needs nothing special.
        SDL_GPUTextureFormat frameReadbackFormat = s_frameReadbackFormat;
        if (s_frameReadbackState.IsPending() && !s_frameReadbackTexture)
        {
            frameReadbackFormat = SDL_GetGPUSwapchainTextureFormat(s_device, s_window);
            if (!CreateFrameReadbackTexture(frameReadbackFormat))
            {
                FailPendingFrameReadback();
            }
        }

        SDL_GPUTexture* reconnectCaptureTexture = s_reconnectCaptureTexture;
        if (s_pendingFrameCaptureTextureId != 0u)
        {
            const auto texture = s_textureMap.find(s_pendingFrameCaptureTextureId);
            const auto size = s_textureSizes.find(s_pendingFrameCaptureTextureId);
            if (texture != s_textureMap.end() && size != s_textureSizes.end() && size->second.first == s_swapW &&
                size->second.second == s_swapH)
            {
                reconnectCaptureTexture = static_cast<SDL_GPUTexture*>(texture->second);
            }
            s_pendingFrameCaptureTextureId = 0u;
        }

        SDL_GPUTexture* const frameColorTexture = s_frameReadbackTexture    ? s_frameReadbackTexture
                                                  : reconnectCaptureTexture ? reconnectCaptureTexture
                                                  : s_mainColorTarget       ? s_mainColorTarget
                                                                            : s_swapchainTexture;

        bool renderPassCompleted = false;
        s_renderReplayBeginTime = std::chrono::steady_clock::now();
        std::optional<FrameProfiler::Scope> replayScope(std::in_place, FrameProfiler::Pass::RendererReplay);
        const Render::PostProcessing::FrameSplit split =
            s_sceneBoundary.Split(s_renderCmds.size(), IsPostProcessingActive());
        if (s_mainColorPassOpenedThisFrame && frameColorTexture == FrameColorTexture())
        {
            // FlushRenderCommands already drew part of this frame into its target with one
            // sample: the rest follows the same way, without multisampling or post-processing.
            renderPassCompleted = ContinueFlushedFrame(frameColorTexture, boneDataReady);
        }
        else if (split.postProcessed && s_postProcessing.EnsureTargets({s_swapW, s_swapH}))
        {
            renderPassCompleted = ReplayPostProcessedFrame(frameColorTexture, split.sceneEnd, boneDataReady);
        }
        else
        {
            renderPassCompleted = ReplayFrame(frameColorTexture, boneDataReady);
        }

        s_mainColorPassOpenedThisFrame = true;
        s_replayedCmdCount = s_renderCmds.size();
        replayScope.reset();

        // RmlUi port: fires after this frame's own game content is fully recorded onto s_cmdBuf
        // but before it's submitted -- see SetPreSubmitCallback's own comment (MuRenderer.h) for
        // why this exact spot is the only correct one. Both UI seams below render into
        // frameColorTexture (published through s_frameColorTarget/GetFrameGpuContext()), not the
        // swapchain directly: on a screenshot/capture frame that is the substitute colour target
        // whose pixels get read back, so the capture shows the UI the player saw. The blit to the
        // swapchain and the readback download deliberately come after these seams.
        s_frameColorTarget = frameColorTexture;
        if (s_cmdBuf && s_preSubmitCallback)
        {
            s_preSubmitCallback();
        }

#ifdef _EDITOR
        // The editor goes over RmlUi, and the game cursor of the pass below over the editor.
        RenderEditorOverlayPass(s_cmdBuf, frameColorTexture);
#endif

        // RmlUi port: content that must sit visually on top of RmlUi (the game cursor, legacy
        // CUITextInputBox text -- see SetPostRmlUiCallback's own comment, MuRenderer.h). Fires
        // after RmlUi's own render pass (opened/closed inside s_preSubmitCallback above) has
        // closed, so this needs its OWN render pass rather than reusing the main one above: the
        // main pass's replay loop (and the s_renderCmds it consumes) already ran and closed
        // before s_preSubmitCallback fired, so anything the callback below pushes via the normal
        // RenderQuad2D-style functions lands at the *tail* of s_renderCmds, past what that loop
        // already replayed -- it would otherwise sit unreplayed until next frame's BeginFrame()
        // clears it away unseen (dropped, not delayed). LOAD_OP_LOAD (not CLEAR) on both targets
        // so this stacks on top of the main pass's content and RmlUi's, rather than erasing them.
        if (s_cmdBuf && s_postRmlUiCallback)
        {
            FRAME_PROFILE(RendererPostUi);
            const std::size_t postUiCmdStart = s_renderCmds.size();

            // RenderQuad2D/RenderTriangles/etc. all early-return on !s_frameActive, which was
            // set false at the very top of this function -- correct for the normal "recording"
            // window (BeginFrame..EndFrame's own replay), but this callback runs deep inside
            // EndFrame, after that window closed. Without this, every draw call the callback
            // makes (RenderCursor, CLoginWin::RenderTextOnTop) is silently dropped -- no warning,
            // no crash, just nothing on screen. Restored to false right after: nothing past this
            // point should still be recording new frame content.
            // Also reset the per-family "previous draw command" merge tracking (normally only
            // reset once per frame, in BeginFrame): it's a persistent index into s_renderCmds
            // that survives across this whole function, unaware of postUiCmdStart above. Without
            // this, MergeAdjacentQuadCommand/MergeAdjacentTriangleCommand happily merge the
            // callback's draws (RenderCursor's quad, say) backward into the LAST matching
            // command from the main pass -- one that was already replayed and closed several
            // lines up -- growing its vtxCount/idxCount instead of appending a new command here.
            // That command is never replayed again, so the merged-in geometry silently never
            // renders: s_renderCmds.size() doesn't even grow, so postUiCmdStart's own "were any
            // commands pushed" check below sees nothing happened, no crash, no warning.
            s_previousDrawCommands.fill(kNoDrawCommand);

            s_frameActive = true;
            s_postRmlUiCallback();
            s_frameActive = false;

            // Re-stage EVERYTHING the callback above may have recorded, not just vertex data.
            // UploadVertices()/the bone-row append behind a skinned RenderTriangles call/etc. all
            // only touch their own CPU-side scratch buffer and advance the matching offset -- none
            // of that reaches the GPU. The actual transfer-buffer map+memcpy and copy-pass that
            // moves each one onto its real GPU buffer already ran once, early in this function
            // (StageDeferredGpuData(), the same call the main pass above makes), using whatever
            // those offsets were BEFORE this callback grew them further.
            //
            // This used to re-stage vertex data only (by hand, duplicating a slice of
            // StageDeferredGpuData()'s own vertex-handling code) -- correct for this seam's
            // original 2D-only callers (RenderCursor, CMsgWin/CCharMakeWin's text overlays), but
            // silently wrong the moment a skinned 3D model entered the picture
            // (CGenericConfirmDialog's item3D preview, 2026-09-14): a skinned draw command indexes
            // into s_boneGpuBuf via ReplayDrawCommand's own `boneDataReady` guard, and the
            // `boneDataReady` computed by the main pass's earlier StageDeferredGpuData() call --
            // true whenever bone data was empty or fully staged *before* this callback ran --
            // stayed true even though this callback's own bone rows were never uploaded and
            // s_boneGpuBuf was never grown to fit them. That combination told ReplayDrawCommand
            // the (undersized, stale) buffer was safe to bind and index into -- an out-of-bounds
            // GPU buffer read, which reproduced as exactly what was seen: the item silently not
            // rendering most frames, and an intermittent crash on whichever frame that
            // out-of-bounds read landed somewhere the driver didn't tolerate (validation is off,
            // see the "SDL_gpu -- validation: disabled" startup log line).
            //
            // Calling StageDeferredGpuData() again here instead re-stages vertex/bone/strip-index/
            // texture data together -- it's explicitly documented as safe to call more than once
            // per frame, the same guarantee FlushRenderCommands already relies on mid-recording --
            // and returns a boneDataReady that's actually correct for what THIS callback recorded.
            bool postUiBoneDataReady = false;
            {
                FRAME_PROFILE(RendererStage);
                postUiBoneDataReady = StageDeferredGpuData();
            }

            if (s_renderCmds.size() > postUiCmdStart)
            {
                SDL_GPUColorTargetInfo postUiColorTarget{};
                postUiColorTarget.texture = frameColorTexture;
                postUiColorTarget.load_op = SDL_GPU_LOADOP_LOAD;
                postUiColorTarget.store_op = SDL_GPU_STOREOP_STORE;

                // A multisampled frame's depth has the frame's samples; this pass draws with one
                // (SingleSamplePassDepth), on top of everything, so it starts from a cleared depth.
                SDL_GPUTexture* const postUiDepth = SingleSamplePassDepth();
                SDL_GPUDepthStencilTargetInfo postUiDepthTarget{};
                postUiDepthTarget.texture = postUiDepth;
                postUiDepthTarget.load_op = postUiDepth == s_depthTexture ? SDL_GPU_LOADOP_LOAD : SDL_GPU_LOADOP_CLEAR;
                postUiDepthTarget.clear_depth = 1.0f;
                postUiDepthTarget.store_op = SDL_GPU_STOREOP_DONT_CARE;
                postUiDepthTarget.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
                postUiDepthTarget.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;

                // ReplayDrawCommand/BindReplayPipeline (inside ReplayCommandRange) bind against
                // the global s_renderPass directly, not a parameter -- assign it here (matching
                // the main pass's own SDL_BeginGPURenderPass call above, which does the same), not
                // just a local. Assigning only a local here left every SDL_Bind*/SDL_Draw* call
                // inside them targeting the *already-nulled* s_renderPass from the main pass's own
                // close (a few lines up) -- SDL_GPU silently no-ops a null render pass rather
                // than crashing, which is why this had no visible symptom beyond "nothing drawn".
                s_renderPass =
                    SDL_BeginGPURenderPass(s_cmdBuf, &postUiColorTarget, 1, postUiDepth ? &postUiDepthTarget : nullptr);
                if (s_renderPass)
                {
                    ReplayCommandRange(postUiCmdStart, s_renderCmds.size(), postUiBoneDataReady);
                    s_replayedCmdCount = s_renderCmds.size();

                    SDL_EndGPURenderPass(s_renderPass);
                    s_renderPass = nullptr;
                }
            }
        }

        s_frameColorTarget = nullptr;

        if (reconnectCaptureTexture && frameColorTexture != reconnectCaptureTexture)
        {
            SDL_GPUBlitInfo blit{};
            blit.source.texture = frameColorTexture;
            blit.source.w = s_swapW;
            blit.source.h = s_swapH;
            blit.destination.texture = reconnectCaptureTexture;
            blit.destination.w = s_swapW;
            blit.destination.h = s_swapH;
            blit.load_op = SDL_GPU_LOADOP_DONT_CARE;
            blit.flip_mode = SDL_FLIP_NONE;
            blit.filter = SDL_GPU_FILTER_LINEAR;
            SDL_BlitGPUTexture(s_cmdBuf, &blit);
        }

        if (frameColorTexture != s_swapchainTexture)
        {
            BlitTextureToSwapchain(s_cmdBuf, frameColorTexture);
        }

        s_submitTime = std::chrono::steady_clock::now();
        if (s_frameReadbackTexture)
        {
            if (!renderPassCompleted)
            {
                mu::log::Get("render")->warn("SDL_gpu -- frame readback render pass failed: {}", SDL_GetError());
                FailPendingFrameReadback();
            }
            else
            {
                if (SubmitFramePixelDownload(s_cmdBuf, s_frameReadbackTexture, frameReadbackFormat))
                {
                    s_cmdBuf = nullptr;
                }
                ReleaseFrameReadbackTexture();
            }
        }

        if (s_cmdBuf)
        {
            FRAME_PROFILE(RendererSubmit);
            SDL_SubmitGPUCommandBuffer(s_cmdBuf);
            s_cmdBuf = nullptr;
        }

        s_swapchainTexture = nullptr;

        ++s_submittedFrames;
        std::erase_if(s_deferredSkinnedMeshFrees,
                      [](const DeferredSkinnedMeshFree& released)
                      {
                          if (s_submittedFrames < released.releasedAtFrame + k_SkinnedMeshFreeDelayFrames)
                              return false;
                          s_skinnedMeshBlocks[released.range.block].ranges.Free(released.range.firstUnit,
                                                                                released.range.unitCount);
                          return true;
                      });

        const auto frameCompletedAt = std::chrono::steady_clock::now();
        s_lastFrameStats.requestedDrawCalls = s_dbgDrawCallsThisFrame;
        s_lastFrameStats.submittedDrawCalls = s_dbgGpuDrawCallsThisFrame;
        s_lastFrameStats.mergedDrawCalls = s_dbgMergedDrawsThisFrame;
        s_lastFrameStats.merged2DDrawCalls = s_dbgMerged2DDrawsThisFrame;
        s_lastFrameStats.commandCount = static_cast<std::uint32_t>(s_renderCmds.size());
        s_lastFrameStats.vertexBytes = s_dbgVtxBytesThisFrame;
        s_lastFrameStats.textureUploads = s_dbgTextureUploadsThisFrame;
        s_lastFrameStats.textureCreates = s_dbgTextureCreatesThisFrame;
        s_lastFrameStats.textureReleases = s_dbgTextureReleasesThisFrame;
        s_lastFrameStats.pipelineBinds = s_dbgPipelineBindsThisFrame;
        s_lastFrameStats.samplerBinds = s_dbgSamplerBindsThisFrame;
        s_lastFrameStats.vertexUniformPushes = s_dbgVertexUniformPushesThisFrame;
        s_lastFrameStats.fragmentUniformPushes = s_dbgFragmentUniformPushesThisFrame;
        const auto milliseconds = [](auto begin, auto end)
        { return std::chrono::duration<double, std::milli>(end - begin).count(); };
        s_lastFrameStats.frameMilliseconds = milliseconds(s_frameBeginTime, frameCompletedAt);
        s_lastFrameStats.replayMilliseconds = milliseconds(s_renderReplayBeginTime, s_submitTime);
        s_lastFrameStats.submitMilliseconds =
            FrameProfiler::AccumulatorMs(FrameProfiler::Pass::RendererSubmit) - submittedBefore;

        const bool emitTimingDiagnostics = s_frameTimingEnabled && s_dbgFrameCount % 60 == 0;
        if (emitTimingDiagnostics)
        {
            const auto logger = mu::log::Get("render");
            logger->debug(
                "[RENDER diag] frame={} draw_calls={} replayed={} merged={} merged_2d={} cmds={} vtx_bytes={} tex={} "
                "fallback={} "
                "white_draws={} real_draws={} uploads={} creates={} releases={} invalidated={} tex2d={} bound={} "
                "pipeline_binds={} sampler_binds={} vertex_uniform_pushes={} fragment_uniform_pushes={}",
                s_dbgFrameCount, s_dbgDrawCallsThisFrame, s_dbgRenderCmdsReplayedThisFrame, s_dbgMergedDrawsThisFrame,
                s_dbgMerged2DDrawsThisFrame, s_renderCmds.size(), s_dbgVtxBytesThisFrame, s_textureMap.size(),
                s_dbgFallbackTextureThisFrame, s_dbgWhiteTextureDrawsThisFrame, s_dbgRealTextureDrawsThisFrame,
                s_dbgTextureUploadsThisFrame, s_dbgTextureCreatesThisFrame, s_dbgTextureReleasesThisFrame,
                s_texturesInvalidated, m_texture2DEnabled, m_boundTextureId, s_dbgPipelineBindsThisFrame,
                s_dbgSamplerBindsThisFrame, s_dbgVertexUniformPushesThisFrame, s_dbgFragmentUniformPushesThisFrame);

            logger->debug("[RENDER timing] total={:.2f}ms replay={:.2f}ms submit={:.2f}ms",
                          s_lastFrameStats.frameMilliseconds, s_lastFrameStats.replayMilliseconds,
                          s_lastFrameStats.submitMilliseconds);
        }
        if (s_dbgFrameCount == 10 && s_dbgDrawCallsThisFrame == 0)
        {
            mu::log::Get("render")->warn(
                "10 frames elapsed with zero draw calls; game may not be calling RenderQuad2D/RenderTriangles");
        }
    }

    [[nodiscard]] bool RequestFramePixels() override
    {
        return s_frameReadbackState.Request();
    }

    [[nodiscard]] bool ConsumeFramePixels(FramePixels& pixels) override
    {
        FramePixels completed = s_frameReadbackState.Consume();
        if (completed.rgb.empty())
        {
            return false;
        }

        pixels = std::move(completed);
        return true;
    }

    void CancelFramePixels() override
    {
        s_frameReadbackState.Reset();
        ReleaseFrameReadbackTexture();
    }

    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
    // BeginScene — 3D viewport and projection setup.
    // SDL_gpu backend: sets viewport on the render pass. Projection/camera
    // transforms are handled via uniform buffers (not immediate-mode matrices).
    // -----------------------------------------------------------------------
    void BeginScene(int x, int y, int w, int h) override
    {
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
    }

    // -----------------------------------------------------------------------
    // EndScene — restore state after 3D pass.
    // SDL_gpu backend: reset viewport to full window.
    // -----------------------------------------------------------------------
    void EndScene() override
    {
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
    }

    // -----------------------------------------------------------------------
    // UI-embedded 3D previews (CUIPhotoViewer, CharMakeWin, UI::Items::ItemCameraTarget)
    // call SetRenderViewport(...) shrinks the GPU
    // viewport around the preview region, then set a narrow-FOV projection.
    // The classic OpenGL backend honored this via glViewport; without a real
    // override here the base-class no-op leaves the 2D-pass full-swapchain
    // viewport in effect, so a 1-degree FOV applied to the whole window
    // produced a screen-filling character (see CUIPhotoViewer::RenderPhotoCharacter).
    //
    // Input coordinates are in OS-window pixels — callers multiply design-space
    // values by g_fScreenRate_x/y before arrival. Scale to swapchain physical
    // pixels so HiDPI/Retina displays target the correct region.
    // -----------------------------------------------------------------------
    void SetViewport(int x, int y, int w, int h) override
    {
        if (!s_frameActive)
        {
            return;
        }

        const float xScale = (s_cachedWinW > 0) ? static_cast<float>(s_swapW) / static_cast<float>(s_cachedWinW) : 1.0f;
        const float yScale = (s_cachedWinH > 0) ? static_cast<float>(s_swapH) / static_cast<float>(s_cachedWinH) : 1.0f;

        RenderCmd cmd{};
        cmd.type = RenderCmdType::SetViewport;
        cmd.viewport.x = static_cast<float>(x) * xScale;
        cmd.viewport.y = static_cast<float>(y) * yScale;
        cmd.viewport.w = static_cast<float>(w) * xScale;
        cmd.viewport.h = static_cast<float>(h) * yScale;
        cmd.viewport.min_depth = 0.0f;
        cmd.viewport.max_depth = 1.0f;
        s_renderCmds.push_back(cmd);
    }

    // -----------------------------------------------------------------------
    // Pixel-level clip rect paired with SetViewport. In Vulkan/Metal/D3D12 the
    // viewport is only a coordinate-mapping transform — it does NOT clip
    // fragments outside the rect. The classic OpenGL driver clipped to the
    // viewport implicitly, so game code (CUIPhotoViewer, CharMakeWin, etc.)
    // relied on "small viewport = small drawable region" behavior that we
    // recover here by applying a matching scissor.
    //
    // The replay loop tracks the most recently set scissor in s_currentScissor
    // and re-applies it before each draw command — some GPU backends reset
    // scissor state on pipeline binds, so sticky-scissor re-application keeps
    // the clip deterministic regardless of backend quirks.
    //
    // Input is in OS-window pixels (same convention as SetViewport). Scale to
    // swapchain physical pixels for HiDPI correctness.
    // -----------------------------------------------------------------------
    void SetScissor(int x, int y, int w, int h) override
    {
        if (!s_frameActive)
        {
            return;
        }

        const float xScale = (s_cachedWinW > 0) ? static_cast<float>(s_swapW) / static_cast<float>(s_cachedWinW) : 1.0f;
        const float yScale = (s_cachedWinH > 0) ? static_cast<float>(s_swapH) / static_cast<float>(s_cachedWinH) : 1.0f;

        RenderCmd cmd{};
        cmd.type = RenderCmdType::SetScissor;
        cmd.scissor.x = static_cast<int>(static_cast<float>(x) * xScale);
        cmd.scissor.y = static_cast<int>(static_cast<float>(y) * yScale);
        cmd.scissor.w = static_cast<int>(static_cast<float>(w) * xScale);
        cmd.scissor.h = static_cast<int>(static_cast<float>(h) * yScale);
        s_renderCmds.push_back(cmd);
    }

    // -----------------------------------------------------------------------
    // Begin2DPass — mark 2D mode for pipeline selection.
    // SDL_gpu uses separate 2D pipelines (Vertex2D layout, depth OFF).
    // -----------------------------------------------------------------------
    void Begin2DPass() override
    {
        // Reset viewport to full swapchain dimensions for 2D UI rendering.
        // The original OpenGL BeginBitmap() called glViewport(0, 0, WindowWidth, WindowHeight)
        // before setting up the orthographic projection. Without this, 2D UI elements
        // (inventory, skill bar, chat) render inside the shrunken 3D viewport.
        RenderCmd cmd{};
        cmd.type = RenderCmdType::SetViewport;
        cmd.viewport.x = 0.0f;
        cmd.viewport.y = 0.0f;
        cmd.viewport.w = static_cast<float>(s_swapW);
        cmd.viewport.h = static_cast<float>(s_swapH);
        cmd.viewport.min_depth = 0.0f;
        cmd.viewport.max_depth = 1.0f;
        s_renderCmds.push_back(cmd);

        RenderCmd scissorCmd{};
        scissorCmd.type = RenderCmdType::SetScissor;
        scissorCmd.scissor = {0, 0, static_cast<int>(s_swapW), static_cast<int>(s_swapH)};
        s_renderCmds.push_back(scissorCmd);
    }

    // -----------------------------------------------------------------------
    // End2DPass — restore 3D mode.
    // -----------------------------------------------------------------------
    void End2DPass() override
    {
        // No-op: pipeline selection returns to 3D automatically via
        // RenderTriangles/RenderQuadStrip calls.
    }

    void BeginScreenSpaceSampling() override
    {
        ++s_screenSpaceSamplingDepth;
        InvalidateSamplerLookupCaches();
    }

    void EndScreenSpaceSampling() override
    {
        if (s_screenSpaceSamplingDepth > 0)
        {
            --s_screenSpaceSamplingDepth;
            InvalidateSamplerLookupCaches();
        }
    }

    // -----------------------------------------------------------------------
    // ClearScreen — no-op on SDL_gpu.
    // SDL_gpu clears the swapchain texture at BeginFrame (LOADOP_CLEAR).
    // -----------------------------------------------------------------------
    void ClearScreen() override
    {
        // No-op: SDL_gpu clears in BeginFrame via SDL_GPU_LOADOP_CLEAR.
    }

    void SetClearColor(float r, float g, float b, float a) override
    {
        s_clearColor = {r, g, b, a};
    }

    // -----------------------------------------------------------------------
    // RenderLines — line primitive rendering.
    // SDL_gpu backend: emit line primitives using existing 3D pipeline.
    // For now, renders as thin triangles (SDL_gpu line support varies).
    // -----------------------------------------------------------------------
    void RenderLines(std::span<const Vertex3D> vertices, std::uint32_t textureId) override
    {
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

            Vertex3D verts[4];
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
            verts[3] = b;
            verts[3].x += perpX;
            verts[3].y += perpY;
            verts[3].z += perpZ;

            RenderQuad3D(verts, textureId);
        }
    }

    // -----------------------------------------------------------------------
    // IsFrameActive — frame lifecycle query.
    // Returns true when a render pass is open (between BeginFrame/EndFrame).
    // -----------------------------------------------------------------------
    [[nodiscard]] bool IsFrameActive() const override
    {
        return s_frameActive;
    }

    void SetStatsEnabled(bool enabled) override
    {
        s_statsEnabled = enabled;
    }

    [[nodiscard]] RendererStats GetFrameStats() const override
    {
        return s_lastFrameStats;
    }

    void SetAntialiasingSamples(int samples) override
    {
        s_requestedSampleCount = Render::Antialiasing::RequestedSampleCount(samples);
        // Before Init this only sets what Init starts with.
        s_sampleCountChangePending = s_device != nullptr;
    }

    [[nodiscard]] int GetAntialiasingSamples() const override
    {
        return s_sampleCount;
    }

    [[nodiscard]] std::size_t BeginReorderableDraws() override
    {
        s_reorderableDraws = {s_renderCmds.size(), s_replayedCmdCount, s_pendingOffscreenCaptures.size(),
                              s_offscreenCaptureTextureId};
        return s_renderCmds.size();
    }

    void EndReorderableDraws(std::size_t mark) override
    {
        const ReorderableDrawsMark& begun = s_reorderableDraws;
        if (!s_drawReorderingEnabled || mark != begun.firstCommand || mark >= s_renderCmds.size() ||
            begun.replayedCommands != s_replayedCmdCount ||
            begun.pendingCaptures != s_pendingOffscreenCaptures.size() ||
            begun.captureTexture != s_offscreenCaptureTextureId)
        {
            return;
        }
        const auto reorderable = [](const RenderCmd& command)
        {
            return command.type == RenderCmdType::DrawSkinnedTriangles && command.shadowIndex == ~0u &&
                   !command.blendEnabled && command.depthTestEnabled && command.depthMaskEnabled;
        };
        const auto stateKey = [](const RenderCmd& command)
        {
            return std::tuple{
                reinterpret_cast<std::uintptr_t>(command.pipeline), reinterpret_cast<std::uintptr_t>(command.texture),
                reinterpret_cast<std::uintptr_t>(command.sampler), reinterpret_cast<std::uintptr_t>(command.vtxBuffer)};
        };
        Render::GroupReorderableFirst(std::span(s_renderCmds).subspan(mark), reorderable,
                                      [&](const RenderCmd& left, const RenderCmd& right)
                                      { return stateKey(left) < stateKey(right); });
        // A later draw must not merge into a command that moved.
        s_previousDrawCommands.fill(kNoDrawCommand);
    }

    void SetDrawReorderingEnabled(bool enabled) override
    {
        s_drawReorderingEnabled = enabled;
    }

    [[nodiscard]] bool IsDrawReorderingEnabled() const override
    {
        return s_drawReorderingEnabled;
    }

    void EndSceneGeometry() override
    {
        if (!s_frameActive)
        {
            return;
        }
        s_sceneBoundary.Mark(s_renderCmds.size());
    }

    [[nodiscard]] bool SetPostProcessing(bool enabled) override
    {
        s_postProcessingEnabled = enabled;
        return !enabled || s_postProcessing.IsAvailable();
    }

    void SetPostProcessingLook(const mu::PostProcessingLook& look) override
    {
        s_postProcessingLook = look;
        s_postProcessingLook.bloomStrength = std::max(look.bloomStrength, 0.0f);
    }

    void SetPostProcessingView(mu::PostProcessingView view) override
    {
        s_postProcessingView = view;
    }

    void BeginBloomExclusion() override
    {
        ++s_bloomExclusionDepth;
    }

    void EndBloomExclusion() override
    {
        if (s_bloomExclusionDepth > 0)
        {
            --s_bloomExclusionDepth;
        }
    }

    [[nodiscard]] bool IsBloomExcluded() const override
    {
        return s_bloomExclusionDepth > 0;
    }

    [[nodiscard]] bool SetColorGradingTable(std::span<const std::uint8_t> rgba) override
    {
        return s_postProcessing.SetGradingTable(rgba);
    }

    [[nodiscard]] bool SetVSyncEnabled(bool enabled) override
    {
        if (!s_device || !s_window)
        {
            return false;
        }

        const SDL_GPUPresentMode presentMode = enabled ? SDL_GPU_PRESENTMODE_VSYNC : PickUncappedPresentMode();
        if (!SDL_SetGPUSwapchainParameters(s_device, s_window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, presentMode))
        {
            mu::log::Get("render")->warn("SDL_gpu -- failed to set {} present mode: {}", PresentModeName(presentMode),
                                         SDL_GetError());
            // Vulkan keeps the failed mode and has already destroyed the old swapchain, so build one in
            // VSync, which every driver supports, as the caller now records.
            if (!enabled &&
                !SDL_SetGPUSwapchainParameters(s_device, s_window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
                                               SDL_GPU_PRESENTMODE_VSYNC))
            {
                mu::log::Get("render")->error("SDL_gpu -- failed to restore VSync present mode: {}", SDL_GetError());
            }
            return false;
        }

        return true;
    }

    // GetDevice override — returns s_device.
    // Allows GlobalBitmap.cpp to obtain the SDL_GPUDevice* via mu::GetRenderer().GetDevice()
    // without a direct dependency on MuRendererSDLGpu.cpp internals.
    // Logs a warning via mu::log if s_device is nullptr (renderer not initialized).
    // -----------------------------------------------------------------------
    [[nodiscard]] void* GetDevice() override
    {
        if (!s_device)
        {
            mu::log::Get("render")->warn("SDL_gpu -- GetDevice() called before Init() or after Shutdown()");
        }
        return s_device;
    }

    // RmlUi port: SDL_Window* accessor, mirrors GetDevice() above.
    [[nodiscard]] SDL_Window* GetWindow() override
    {
        return s_window;
    }

    // RmlUi port: current frame's command buffer + swapchain texture, for
    // RenderInterface_SDL_GPU::BeginFrame(). Null/zero outside BeginFrame()/EndFrame() --
    // s_cmdBuf and s_swapchainTexture are reset at the top of BeginFrame() and s_cmdBuf is
    // cleared to nullptr once submitted in EndFrame(), so this naturally reflects that window
    // without extra bookkeeping here.
    [[nodiscard]] FrameGpuContext GetFrameGpuContext() override
    {
        // Every frame target has the swapchain's format, and the main one exists only when that format can be sampled.
        return FrameGpuContext{s_cmdBuf, s_frameColorTarget ? s_frameColorTarget : s_swapchainTexture, s_swapW,
                               s_swapH, s_frameColorTarget != nullptr && s_mainColorTarget != nullptr};
    }

    // RmlUi port: looks up s_textureMap directly rather than going through ResolveTextureId(),
    // since RmlUi needs the actual SDL_GPUTexture* to hand to its own vendored backend, not a
    // resolved logical id.
    [[nodiscard]] void* GetRawTexture(std::uint32_t textureId) override
    {
        auto it = s_textureMap.find(textureId);
        return it != s_textureMap.end() ? it->second : nullptr;
    }

    void SetPreSubmitCallback(std::function<void()> callback) override
    {
        s_preSubmitCallback = std::move(callback);
    }

    void SetPostRmlUiCallback(std::function<void()> callback) override
    {
        s_postRmlUiCallback = std::move(callback);
    }

    // SDL_ttf text engine accessor.
    [[nodiscard]] TTF_TextEngine* GetTextEngine() override
    {
        return s_textEngine;
    }

    // Default TTF font accessor.
    [[nodiscard]] TTF_Font* GetTtfFont() override
    {
        return s_ttfFonts.Get(Render::Text::SdlTtfFontRole::Normal);
    }

    // F-1 fix: Font variant accessors for bold, big, and fixed-width text.
    [[nodiscard]] TTF_Font* GetTtfFontBold() override
    {
        return s_ttfFonts.Get(Render::Text::SdlTtfFontRole::Bold);
    }
    [[nodiscard]] TTF_Font* GetTtfFontBig() override
    {
        return s_ttfFonts.Get(Render::Text::SdlTtfFontRole::Big);
    }
    [[nodiscard]] TTF_Font* GetTtfFontFixed() override
    {
        return s_ttfFonts.Get(Render::Text::SdlTtfFontRole::Fixed);
    }

    [[nodiscard]] bool ReloadTtfFonts(std::string_view fontFamily, float normalPointSize, float bigPointSize,
                                      float fixedPointSize) override
    {
#if MU_HAS_SDL_TTF
        return s_textEngine && LoadTtfFonts(fontFamily, normalPointSize, bigPointSize, fixedPointSize);
#else
        (void)fontFamily;
        (void)normalPointSize;
        (void)bigPointSize;
        (void)fixedPointSize;
        return false;
#endif
    }

    // F-7 fix: Cached window dimensions accessor (updated per-frame in BeginFrame).
    [[nodiscard]] int GetCachedWindowHeight() override
    {
        return s_cachedWinH;
    }

    // Submit text atlas triangles as deferred draw commands.
    void SubmitTextTriangles(std::span<const Vertex2D> vertices, void* atlasTexture, void* sampler = nullptr) override
    {
        if (vertices.empty() || !s_frameActive || !atlasTexture)
        {
            return;
        }

        const Uint32 byteSize = static_cast<Uint32>(vertices.size() * sizeof(Vertex2D));
        const Uint32 vtxOffset = UploadVertices(vertices.data(), byteSize, sizeof(Vertex2D));
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
        cmd.vtxStride = sizeof(Vertex2D);
        cmd.vtxCount = static_cast<Uint32>(vertices.size());
        cmd.fogUniform = m_fogUniform;
        cmd.blendMode = BlendMode::Alpha;
        cmd.blendEnabled = true;
        // 2D ortho projection for text — Y-up to match SDL_ttf GPU convention.
        // SDL_ttf negates Y in the vertex data (see SDL_gpu_textengine.c: "In the GPU API
        // positive y-axis is upwards so the signs of the y-coords is reversed").
        // Vertex positions are in Y-up space with the text origin at (0,0).
        // drawX/drawY offset the text to the correct screen position.
        cmd.vu.mvp = s_offscreen2DProjection.value_or(
            glm::ortho(0.0f, static_cast<float>(s_cachedWinW), 0.0f, static_cast<float>(s_cachedWinH), -1.0f, 1.0f));
        FrameProfiler::Count(FrameProfiler::Counter::BatchVertices, cmd.vtxCount);
        std::size_t& previousCommand =
            Render::PreviousDrawCommand(s_previousDrawCommands, Render::DrawCommandFamily::TextTriangles2D);
        if (MergeAdjacentTriangleCommand(cmd, previousCommand, sizeof(Vertex2D)))
        {
            ++s_dbgMergedDrawsThisFrame;
            ++s_dbgMerged2DDrawsThisFrame;
            FrameProfiler::Count(FrameProfiler::Counter::MergedDraws);
            FrameProfiler::Count(FrameProfiler::Counter::Merged2DDraws);
        }
        else
        {
            s_renderCmds.push_back(cmd);
            previousCommand = s_renderCmds.size() - 1;
            FrameProfiler::Count(FrameProfiler::Counter::DrawCalls);
            FrameProfiler::Count(FrameProfiler::Counter::BatchDraws);
        }

        ++s_dbgDrawCallsThisFrame;
        s_dbgVtxBytesThisFrame += byteSize;
        FrameProfiler::Count(FrameProfiler::Counter::VertexBytes, byteSize);
    }

    // -----------------------------------------------------------------------
    // QueueTextureUpdate: Queue a CPU→GPU texture upload for the next EndFrame copy pass.
    // -----------------------------------------------------------------------
    void QueueTextureUpdate(std::uint32_t textureId, const void* pixels, std::uint32_t width,
                            std::uint32_t height) override
    {
        if (!pixels || width == 0 || height == 0)
        {
            return;
        }

        void* pTex = LookupTexture(textureId);
        if (!pTex)
        {
            mu::log::Get("render")->warn("SDL_gpu -- rejecting update for unknown texture {}", textureId);
            return;
        }

        MarkTextureUpdatedAfterCreation(textureId);

        const Uint32 dataSize = width * height * 4u; // RGBA8
        TextureUpdateCmd cmd{};
        cmd.gpuTexture = static_cast<SDL_GPUTexture*>(pTex);
        cmd.pixelsCopy.resize(dataSize);
        std::memcpy(cmd.pixelsCopy.data(), pixels, dataSize);
        cmd.width = width;
        cmd.height = height;
        cmd.bytesPerRow = width * 4; // RGBA8
        s_textureUpdates.push_back(std::move(cmd));
        ++s_dbgTextureUploadsThisFrame;
        FrameProfiler::Count(FrameProfiler::Counter::TextureUploads);
    }

    void EnsureTexture(std::uint32_t textureId, std::uint32_t width, std::uint32_t height) override
    {
        if (!s_device || textureId == 0 || width == 0 || height == 0)
        {
            return;
        }

        auto existing = s_textureMap.find(textureId);
        if (existing != s_textureMap.end())
        {
            if (!s_ownedTextureIds.contains(textureId))
            {
                return;
            }

            auto sizeIt = s_textureSizes.find(textureId);
            if (sizeIt != s_textureSizes.end() && sizeIt->second.first == width && sizeIt->second.second == height)
            {
                return;
            }

            ReleaseOwnedTextureById(textureId);
        }

        SDL_GPUTextureCreateInfo texInfo{};
        texInfo.type = SDL_GPU_TEXTURETYPE_2D;
        texInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        texInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
        texInfo.width = width;
        texInfo.height = height;
        texInfo.layer_count_or_depth = 1;
        texInfo.num_levels = 1;
        texInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;

        SDL_GPUTexture* texture = SDL_CreateGPUTexture(s_device, &texInfo);
        if (!texture)
        {
            mu::log::Get("render")->warn("SDL_gpu -- texture {} creation failed ({}x{}): {}", textureId, width, height,
                                         SDL_GetError());
            return;
        }

        s_textureMap[textureId] = texture;
        InvalidateTextureLookupCache();
        s_textureSizes[textureId] = {width, height};
        s_ownedTextureIds.insert(textureId);
        ++s_dbgTextureCreatesThisFrame;
    }

    void ReleaseTexture(std::uint32_t textureId) override
    {
        if (!s_device || textureId == 0)
        {
            return;
        }

        ReleaseOwnedTextureById(textureId);
    }

    // Every replay pipeline is built against the swapchain's colour format, and a pass whose
    // target differs from it is invalid -- so an offscreen target takes that format, not a fixed one.
    [[nodiscard]] static SDL_GPUTextureFormat OffscreenColorFormat()
    {
        constexpr SDL_GPUTextureUsageFlags usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
        const SDL_GPUTextureFormat format = SDL_GetGPUSwapchainTextureFormat(s_device, s_window);
        if (format != SDL_GPU_TEXTUREFORMAT_INVALID &&
            SDL_GPUTextureSupportsFormat(s_device, format, SDL_GPU_TEXTURETYPE_2D, usage))
        {
            return format;
        }
        return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    }

    // Same shape as EnsureTexture, but with COLOR_TARGET usage added so the result
    // can be rendered into (not just sampled) - needed for offscreen captures.
    void EnsureOffscreenColorTexture(std::uint32_t textureId, std::uint32_t width, std::uint32_t height)
    {
        if (!s_device || textureId == 0 || width == 0 || height == 0)
        {
            return;
        }

        auto existing = s_textureMap.find(textureId);
        if (existing != s_textureMap.end())
        {
            if (!s_ownedTextureIds.contains(textureId))
            {
                return;
            }

            auto sizeIt = s_textureSizes.find(textureId);
            if (sizeIt != s_textureSizes.end() && sizeIt->second.first == width && sizeIt->second.second == height)
            {
                return;
            }

            ReleaseOwnedTextureById(textureId);
        }

        SDL_GPUTextureCreateInfo texInfo{};
        texInfo.type = SDL_GPU_TEXTURETYPE_2D;
        texInfo.format = OffscreenColorFormat();
        texInfo.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
        texInfo.width = width;
        texInfo.height = height;
        texInfo.layer_count_or_depth = 1;
        texInfo.num_levels = 1;
        texInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;

        SDL_GPUTexture* texture = SDL_CreateGPUTexture(s_device, &texInfo);
        if (!texture)
        {
            mu::log::Get("render")->warn("SDL_gpu -- offscreen capture texture {} creation failed ({}x{}): {}",
                                         textureId, width, height, SDL_GetError());
            return;
        }

        s_textureMap[textureId] = texture;
        InvalidateTextureLookupCache();
        s_textureSizes[textureId] = {width, height};
        s_ownedTextureIds.insert(textureId);
        ++s_dbgTextureCreatesThisFrame;
    }

    [[nodiscard]] std::uint32_t BeginOffscreenCapture(std::uint32_t textureId, std::uint32_t width,
                                                       std::uint32_t height) override
    {
        if (!s_device || !s_frameActive || width == 0u || height == 0u)
        {
            mu::log::Get("render")->warn(
                "SDL_gpu -- BeginOffscreenCapture precondition failed: device={} frameActive={} w={} h={}",
                s_device != nullptr, s_frameActive, width, height);
            return 0u;
        }
        if (s_offscreenCaptureTextureId != 0u)
        {
            // A capture is already open - BeginOffscreenCapture/EndOffscreenCapture
            // pairs don't nest. Refuse rather than silently corrupting the other one.
            mu::log::Get("render")->warn("SDL_gpu -- BeginOffscreenCapture called while another is still open");
            return 0u;
        }

        if (textureId == 0u)
        {
            textureId = AllocateOwnedDynamicTextureId();
            if (textureId == 0u)
            {
                mu::log::Get("render")->warn("SDL_gpu -- BeginOffscreenCapture: AllocateOwnedDynamicTextureId failed");
                return 0u;
            }
        }

        EnsureOffscreenColorTexture(textureId, width, height);
        if (!IsTextureRegistered(textureId))
        {
            mu::log::Get("render")->warn("SDL_gpu -- BeginOffscreenCapture: texture {} not registered after "
                                         "EnsureOffscreenColorTexture ({}x{})",
                                         textureId, width, height);
            return 0u;
        }

        // Neither side of a capture may merge into the other: a draw merged backward into a
        // command outside the range would land on the wrong target.
        s_previousDrawCommands.fill(kNoDrawCommand);
        s_offscreenCaptureStart = s_renderCmds.size();
        s_offscreenCaptureTextureId = textureId;
        s_offscreen2DProjection.reset();
        s_offscreenCaptureWidth = width;
        s_offscreenCaptureHeight = height;
        return textureId;
    }

    void EndOffscreenCapture() override
    {
        if (s_offscreenCaptureTextureId == 0u)
        {
            return;
        }

        s_pendingOffscreenCaptures.push_back({s_offscreenCaptureStart, s_renderCmds.size(),
                                              s_offscreenCaptureTextureId, s_offscreenCaptureWidth,
                                              s_offscreenCaptureHeight});
        s_offscreenCaptureTextureId = 0u;
        s_offscreen2DProjection.reset();
        s_previousDrawCommands.fill(kNoDrawCommand);
    }

    void SetOffscreen2DRect(float left, float top, float width, float height) override
    {
        if (s_offscreenCaptureTextureId == 0u || width <= 0.f || height <= 0.f)
            return;
        // 2D vertices are window pixels with y up from the bottom edge.
        const float bottom = static_cast<float>(s_cachedWinH) - (top + height);
        s_offscreen2DProjection = glm::ortho(left, left + width, bottom, bottom + height, -1.0f, 1.0f);
    }

    [[nodiscard]] std::uint32_t CreateRenderTarget(std::uint32_t width, std::uint32_t height) override
    {
        if (!s_device || width == 0u || height == 0u)
        {
            return 0u;
        }
        const std::uint32_t textureId = AllocateOwnedDynamicTextureId();
        if (textureId == 0u)
        {
            return 0u;
        }
        EnsureOffscreenColorTexture(textureId, width, height);
        if (!IsTextureRegistered(textureId))
        {
            return 0u;
        }

        SDL_GPUTextureCreateInfo depthInfo{};
        depthInfo.type = SDL_GPU_TEXTURETYPE_2D;
        depthInfo.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
        depthInfo.width = width;
        depthInfo.height = height;
        depthInfo.layer_count_or_depth = 1;
        depthInfo.num_levels = 1;
        depthInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
        SDL_GPUTexture* depth = SDL_CreateGPUTexture(s_device, &depthInfo);
        if (!depth)
        {
            mu::log::Get("render")->warn("SDL_gpu -- render target depth creation failed ({}x{}): {}", width,
                                         height, SDL_GetError());
            s_pendingRenderTargetReleases.push_back(textureId);
            return 0u;
        }
        s_renderTargetDepth[textureId] = depth;
        return textureId;
    }

    void ReleaseRenderTarget(std::uint32_t textureId) override
    {
        if (textureId != 0u)
        {
            s_pendingRenderTargetReleases.push_back(textureId);
        }
    }

    void SetOffscreenRenderCallback(std::function<void()> callback) override
    {
        s_offscreenRenderCallback = std::move(callback);
    }

#ifdef _EDITOR
    [[nodiscard]] void* GetTexturePointer(std::uint32_t textureId) const override
    {
        const auto it = s_textureMap.find(textureId);
        return it != s_textureMap.end() ? it->second : nullptr;
    }

    [[nodiscard]] bool HasPendingOffscreenCaptures() const override
    {
        return !s_pendingOffscreenCaptures.empty();
    }
#endif // _EDITOR

    [[nodiscard]] std::uint32_t CreateTexture(std::uint32_t width, std::uint32_t height, const void* pixels) override
    {
        const std::uint32_t textureId = AllocateOwnedDynamicTextureId();
        if (textureId == 0u)
        {
            return 0u;
        }

        EnsureTexture(textureId, width, height);
        if (!IsTextureRegistered(textureId))
        {
            return 0u;
        }
        QueueTextureUpdate(textureId, pixels, width, height);
        return textureId;
    }

    [[nodiscard]] std::uint32_t CaptureFrameTexture(std::uint32_t textureId) override
    {
        if (!s_device || !s_window || !s_frameActive || s_swapW == 0u || s_swapH == 0u)
        {
            return 0u;
        }

        if (textureId != 0u)
        {
            const auto size = s_textureSizes.find(textureId);
            if (!s_ownedTextureIds.contains(textureId) || size == s_textureSizes.end())
            {
                textureId = 0u;
            }
            else if (size->second.first != s_swapW || size->second.second != s_swapH)
            {
                ReleaseOwnedTextureById(textureId);
                textureId = 0u;
            }
        }

        if (textureId == 0u)
        {
            textureId = AllocateOwnedDynamicTextureId();
            const auto textureInfo = GetSdlGpuFrameCaptureTextureInfo(
                SDL_GetGPUSwapchainTextureFormat(s_device, s_window), s_swapW, s_swapH);
            if (textureId == 0u || !textureInfo)
            {
                return 0u;
            }

            SDL_GPUTexture* texture = SDL_CreateGPUTexture(s_device, &*textureInfo);
            if (!texture)
            {
                mu::log::Get("render")->warn("SDL_gpu -- reconnect capture texture creation failed: {}",
                                             SDL_GetError());
                return 0u;
            }

            s_textureMap[textureId] = texture;
            InvalidateTextureLookupCache();
            s_textureSizes[textureId] = {s_swapW, s_swapH};
            s_ownedTextureIds.insert(textureId);
            ++s_dbgTextureCreatesThisFrame;
        }

        s_pendingFrameCaptureTextureId = textureId;
        return textureId;
    }

    [[nodiscard]] bool IsTextureRegistered(std::uint32_t textureId) const override
    {
        return LookupTexture(textureId) != nullptr;
    }

    // GPU backend driver name for error reporting.
    [[nodiscard]] const char* GetGPUDriverName() const override
    {
        return s_device ? SDL_GetGPUDeviceDriver(s_device) : "unknown";
    }

    [[nodiscard]] const char* GetGPUDeviceName() const override
    {
        if (!s_device)
        {
            return "";
        }
        return SDL_GetStringProperty(SDL_GetGPUDeviceProperties(s_device), SDL_PROP_GPU_DEVICE_NAME_STRING, "");
    }

    // -----------------------------------------------------------------------
    // RenderQuad2D: Render a screen-space textured quad (4 vertices per quad).
    // Vertex count must be a multiple of 4.
    // -----------------------------------------------------------------------
    void RenderQuad2D(std::span<const Vertex2D> vertices, std::uint32_t textureId) override
    {
        if (vertices.empty() || !s_frameActive || !m_colorWriteEnabled || m_stencilTestEnabled)
        {
            return;
        }

        if (!Render::Topology::IsValidQuadVertexCount(vertices.size()))
        {
            mu::log::Get("render")->warn("SDL_gpu::RenderQuad2D -- vertex count {} not divisible by 4",
                                         vertices.size());
            return;
        }

        void* pTex = LookupTextureForDraw(textureId);
        if (!pTex)
        {
            mu::log::Get("render")->warn("SDL_gpu::RenderQuad2D -- unknown textureId {}, skipping", textureId);
            return;
        }
        if (textureId == 0u)
            ++s_dbgWhiteTextureDrawsThisFrame;
        else
            ++s_dbgRealTextureDrawsThisFrame;

        const Uint32 byteSize = static_cast<Uint32>(vertices.size() * sizeof(Vertex2D));
        const Uint32 vtxOffset = UploadVertices(vertices.data(), byteSize, sizeof(Vertex2D));
        if (vtxOffset == ~0u)
        {
            return;
        }

        // RenderQuad2D uses the 2D pipeline set (Vertex2D layout).
        // Always disable depth test for 2D sprites — they must render on top of 3D
        // geometry regardless of depth buffer state. The 3D pass fills the depth buffer
        // with near values (characters close to camera) that would occlude 2D UI.
        const int pipelineIdx = GetActivePipelineIndex();
        SDL_GPUGraphicsPipeline* pipeline = GetActiveQuadPipeline(Render::Topology::QuadSpace::Screen);
        if (!pipeline)
        {
            if (!s_dbgNullPipelineWarned)
            {
                mu::log::Get("render")->error(
                    "SDL_gpu -- RenderQuad2D pipeline is null: index={} depth_test={} pipeline creation failed",
                    pipelineIdx, m_depthTestEnabled);
                s_dbgNullPipelineWarned = true;
            }
            return;
        }

        const Uint32 numQuads = static_cast<Uint32>(vertices.size() / 4);
        if (numQuads > static_cast<Uint32>(k_MaxQuads))
        {
            mu::log::Get("render")->warn("SDL_gpu::RenderQuad2D -- numQuads {} exceeds k_MaxQuads {}; clamping draw",
                                         numQuads, k_MaxQuads);
        }
        const Uint32 drawQuads =
            (numQuads <= static_cast<Uint32>(k_MaxQuads)) ? numQuads : static_cast<Uint32>(k_MaxQuads);

        void* pSampler = LookupSampler(textureId);

        // Record deferred draw command — replayed in EndFrame after vertex data is on the GPU.
        RenderCmd cmd{};
        cmd.type = RenderCmdType::DrawIndexedQuads;
        cmd.pipeline = pipeline;
        cmd.texture = static_cast<SDL_GPUTexture*>(pTex);
        cmd.sampler = pSampler ? static_cast<SDL_GPUSampler*>(pSampler) : s_defaultSampler;
        cmd.vtxOffset = vtxOffset;
        cmd.vtxStride = sizeof(Vertex2D);
        cmd.idxCount = drawQuads * 6;
        cmd.fogUniform = m_fogUniform;
        RecordDrawState(cmd);
        // 2D ortho MVP: maps [0,W]×[0,H] to NDC, replicating gluOrtho2D.
        // GLM_FORCE_DEPTH_ZERO_TO_ONE → correct Z [0,1] for Metal/Vulkan.
        // fogStart=fogEnd=0 → range=0 → vertex shader sets fogFactor=1.0 (no fog for 2D).
        cmd.vu.mvp = s_offscreen2DProjection.value_or(
            glm::ortho(0.0f, static_cast<float>(s_cachedWinW), 0.0f, static_cast<float>(s_cachedWinH), -1.0f, 1.0f));
        FrameProfiler::Count(FrameProfiler::Counter::BatchVertices, drawQuads * 4u);
        std::size_t& previousCommand =
            Render::PreviousDrawCommand(s_previousDrawCommands, Render::DrawCommandFamily::ScreenQuads2D);
        if (MergeAdjacentQuadCommand(cmd, previousCommand, sizeof(Vertex2D)))
        {
            ++s_dbgMergedDrawsThisFrame;
            ++s_dbgMerged2DDrawsThisFrame;
            FrameProfiler::Count(FrameProfiler::Counter::MergedDraws);
            FrameProfiler::Count(FrameProfiler::Counter::Merged2DDraws);
        }
        else
        {
            s_renderCmds.push_back(cmd);
            previousCommand = s_renderCmds.size() - 1;
            FrameProfiler::Count(FrameProfiler::Counter::DrawCalls);
            FrameProfiler::Count(FrameProfiler::Counter::BatchDraws);
        }

        ++s_dbgDrawCallsThisFrame;
        s_dbgVtxBytesThisFrame += byteSize;
        FrameProfiler::Count(FrameProfiler::Counter::VertexBytes, byteSize);
    }

    // -----------------------------------------------------------------------
    // RenderTriangles: Render world-space triangles (vertex count divisible by 3).
    // -----------------------------------------------------------------------
    void RenderTriangles(std::span<const Vertex3D> vertices, std::uint32_t textureId) override
    {
        RecordTriangles(vertices, {}, textureId);
    }

    void RenderIndexedTriangles(std::span<const Vertex3D> vertices, std::span<const std::uint16_t> indices,
                                std::uint32_t textureId) override
    {
        if (indices.empty() || !ValidTriangleIndices(vertices.size(), indices))
        {
            return;
        }
        RecordTriangles(vertices, indices, textureId);
    }

    // Indices that make whole triangles of vertices that exist.
    [[nodiscard]] static bool ValidTriangleIndices(std::size_t vertexCount, std::span<const std::uint16_t> indices)
    {
        return indices.size() % 3 == 0 && std::all_of(indices.begin(), indices.end(), [vertexCount](std::uint16_t index)
                                                      { return index < vertexCount; });
    }

    // Appends `indices` to the frame's strip index scratch and makes `cmd` draw them.
    static void RecordTriangleIndices(RenderCmd& cmd, std::span<const std::uint16_t> indices)
    {
        cmd.trianglesIndexed = true;
        cmd.idxCount = static_cast<Uint32>(indices.size());
        cmd.stripIdxOffset = static_cast<Uint32>(s_stripIdxScratch.size() * sizeof(Uint16));
        s_stripIdxScratch.insert(s_stripIdxScratch.end(), indices.begin(), indices.end());
    }

    // RenderTriangles, of `indices` into `vertices` when there are indices.
    void RecordTriangles(std::span<const Vertex3D> vertices, std::span<const std::uint16_t> indices,
                         std::uint32_t textureId)
    {
        if (vertices.empty() || !s_frameActive || !m_colorWriteEnabled || m_stencilTestEnabled)
        {
            return;
        }

        if (indices.empty() && vertices.size() % 3 != 0)
        {
            mu::log::Get("render")->warn("SDL_gpu::RenderTriangles -- vertex count {} not divisible by 3",
                                         vertices.size());
            return;
        }

        const std::uint32_t resolvedTexId = ResolveTextureId(textureId);
        void* pTex = LookupTextureForDraw(resolvedTexId);
        if (!pTex)
        {
            mu::log::Get("render")->warn("SDL_gpu::RenderTriangles -- unknown textureId {}, skipping", textureId);
            return;
        }
        if (resolvedTexId == 0u)
            ++s_dbgWhiteTextureDrawsThisFrame;
        else
            ++s_dbgRealTextureDrawsThisFrame;

        const Uint32 byteSize = static_cast<Uint32>(vertices.size() * sizeof(Vertex3D));
        const Uint32 vtxOffset = UploadVertices(vertices.data(), byteSize, sizeof(Vertex3D));
        if (vtxOffset == ~0u)
        {
            return;
        }

        // RenderTriangles uses the 3D pipeline set (Vertex3D layout).
        const int pipelineIdx = GetActivePipelineIndex();
        SDL_GPUGraphicsPipeline* pipeline = GetActive3DPipeline();
        if (!pipeline)
        {
            if (!s_dbgNullPipelineWarned)
            {
                mu::log::Get("render")->error("SDL_gpu -- RenderTriangles pipeline is null: index={} depth_test={}",
                                              pipelineIdx, m_depthTestEnabled);
                s_dbgNullPipelineWarned = true;
            }
            return;
        }
        const AlphaCoverageChoice coverage = ChooseAlphaCoverage(pipeline, resolvedTexId, GetVertexAlpha(vertices));
        pipeline = coverage.pipeline;

        void* pSampler = LookupWorldSampler(resolvedTexId);

        // Record deferred draw command — replayed in EndFrame after vertex data is on the GPU.
        RenderCmd cmd{};
        cmd.type = RenderCmdType::DrawTriangles;
        cmd.pipeline = pipeline;
        cmd.texture = static_cast<SDL_GPUTexture*>(pTex);
        cmd.sampler = pSampler ? static_cast<SDL_GPUSampler*>(pSampler) : s_defaultSampler;
        cmd.vtxOffset = vtxOffset;
        cmd.vtxStride = sizeof(Vertex3D);
        cmd.vtxCount = static_cast<Uint32>(vertices.size());
        cmd.vu.mvp = m_mvpMatrix;
        cmd.vu.fogStart = m_fogUniform.fogStart;
        cmd.vu.fogEnd = m_fogUniform.fogEnd;
        cmd.fogUniform = m_fogUniform;
        cmd.fogUniform.alphaToCoverage = coverage.alphaToCoverage ? 1u : 0u;
        RecordDrawState(cmd);
        FrameProfiler::Count(FrameProfiler::Counter::BatchVertices, cmd.vtxCount);
        std::size_t& previousCommand =
            Render::PreviousDrawCommand(s_previousDrawCommands, Render::DrawCommandFamily::Triangles3D);
        if (!indices.empty())
        {
            // An indexed draw merges with nothing, before or after it.
            RecordTriangleIndices(cmd, indices);
            s_renderCmds.push_back(cmd);
            previousCommand = kNoDrawCommand;
            FrameProfiler::Count(FrameProfiler::Counter::DrawCalls);
            FrameProfiler::Count(FrameProfiler::Counter::BatchDraws);
        }
        else if (!s_disableD3D12TriangleMerging && MergeAdjacentTriangleCommand(cmd, previousCommand, sizeof(Vertex3D)))
        {
            ++s_dbgMergedDrawsThisFrame;
            FrameProfiler::Count(FrameProfiler::Counter::MergedDraws);
        }
        else
        {
            s_renderCmds.push_back(cmd);
            previousCommand = s_renderCmds.size() - 1;
            FrameProfiler::Count(FrameProfiler::Counter::DrawCalls);
            FrameProfiler::Count(FrameProfiler::Counter::BatchDraws);
        }

        ++s_dbgDrawCallsThisFrame;
        s_dbgVtxBytesThisFrame += byteSize;
        FrameProfiler::Count(FrameProfiler::Counter::VertexBytes, byteSize);
    }

    // -----------------------------------------------------------------------
    // RenderQuad3D: Render independent world-space quads in perimeter order.
    // -----------------------------------------------------------------------
    void RenderQuad3D(std::span<const Vertex3D> vertices, std::uint32_t textureId) override
    {
        if (vertices.empty() || !s_frameActive || !m_colorWriteEnabled || m_stencilTestEnabled)
        {
            return;
        }

        if (!Render::Topology::IsValidQuadVertexCount(vertices.size()))
        {
            mu::log::Get("render")->warn("SDL_gpu::RenderQuad3D -- vertex count {} not divisible by 4",
                                         vertices.size());
            return;
        }

        const std::uint32_t resolvedTexId = ResolveTextureId(textureId);
        void* pTex = LookupTextureForDraw(resolvedTexId);
        if (!pTex)
        {
            mu::log::Get("render")->warn("SDL_gpu::RenderQuad3D -- unknown textureId {}, skipping", textureId);
            return;
        }
        if (resolvedTexId == 0u)
            ++s_dbgWhiteTextureDrawsThisFrame;
        else
            ++s_dbgRealTextureDrawsThisFrame;

        const Uint32 numQuads = static_cast<Uint32>(vertices.size() / 4);
        if (numQuads > static_cast<Uint32>(k_MaxQuads))
        {
            mu::log::Get("render")->warn("SDL_gpu::RenderQuad3D -- numQuads {} exceeds k_MaxQuads {}; clamping draw",
                                         numQuads, k_MaxQuads);
        }
        const Uint32 drawQuads =
            (numQuads <= static_cast<Uint32>(k_MaxQuads)) ? numQuads : static_cast<Uint32>(k_MaxQuads);
        const Uint32 byteSize = drawQuads * 4u * static_cast<Uint32>(sizeof(Vertex3D));
        const Uint32 vtxOffset = UploadVertices(vertices.data(), byteSize, sizeof(Vertex3D));
        if (vtxOffset == ~0u)
        {
            return;
        }

        const int pipelineIdx = GetActivePipelineIndex();
        SDL_GPUGraphicsPipeline* pipeline = GetActiveQuadPipeline(Render::Topology::QuadSpace::World);
        if (!pipeline)
        {
            if (!s_dbgNullPipelineWarned)
            {
                mu::log::Get("render")->error("SDL_gpu -- RenderQuad3D pipeline is null: index={} depth_test={}",
                                              pipelineIdx, m_depthTestEnabled);
                s_dbgNullPipelineWarned = true;
            }
            return;
        }

        void* pSampler = LookupWorldSampler(resolvedTexId);

        RenderCmd cmd{};
        cmd.type = RenderCmdType::DrawIndexedQuads;
        cmd.pipeline = pipeline;
        cmd.texture = static_cast<SDL_GPUTexture*>(pTex);
        cmd.sampler = pSampler ? static_cast<SDL_GPUSampler*>(pSampler) : s_defaultSampler;
        cmd.vtxOffset = vtxOffset;
        cmd.vtxStride = sizeof(Vertex3D);
        cmd.idxCount = drawQuads * 6u;
        cmd.vu.mvp = m_mvpMatrix;
        cmd.vu.fogStart = m_fogUniform.fogStart;
        cmd.vu.fogEnd = m_fogUniform.fogEnd;
        cmd.fogUniform = m_fogUniform;
        RecordDrawState(cmd);
        FrameProfiler::Count(FrameProfiler::Counter::BatchVertices, drawQuads * 4u);
        std::size_t& previousCommand =
            Render::PreviousDrawCommand(s_previousDrawCommands, Render::DrawCommandFamily::Quads3D);
        if (MergeAdjacentQuadCommand(cmd, previousCommand, sizeof(Vertex3D)))
        {
            ++s_dbgMergedDrawsThisFrame;
            FrameProfiler::Count(FrameProfiler::Counter::MergedDraws);
        }
        else
        {
            s_renderCmds.push_back(cmd);
            previousCommand = s_renderCmds.size() - 1;
            FrameProfiler::Count(FrameProfiler::Counter::DrawCalls);
            FrameProfiler::Count(FrameProfiler::Counter::BatchDraws);
        }

        ++s_dbgDrawCallsThisFrame;
        s_dbgVtxBytesThisFrame += byteSize;
        FrameProfiler::Count(FrameProfiler::Counter::VertexBytes, byteSize);
    }

    // The frame's point lights are placed in the scene's world; models drawn for the UI (between
    // BeginScreenSpaceSampling and EndScreenSpaceSampling) have their own view and skip them. The
    // lights themselves arrive after the frame's draws (SetDynamicLights), so this does not look
    // at them; without lights the shader's loop is empty.
    [[nodiscard]] static std::uint32_t EnhancedSkinningLighting()
    {
        return s_screenSpaceSamplingDepth == 0 ? k_SkinningLightingEnhancedDynamic : k_SkinningLightingEnhanced;
    }

    // World position and normal of Enhanced-lit skinned vertices, for the frame's point lights.
    void PackVertexToWorldRows(SkinningVertexUniforms& uniforms) const
    {
        const glm::mat4 vertexToWorld = m_worldFromViewMatrix * m_modelViewMatrix;
        for (int row = 0; row < 3; ++row)
        {
            for (int column = 0; column < 4; ++column)
            {
                uniforms.vertexToWorldRows[row][column] = vertexToWorld[column][row];
            }
        }
    }

    // skinned_lit.frag's normal map for a draw: the draw's map with its scale, or the flat
    // default with scale 0 (the shader then lights the unperturbed normal).
    static void BindNormalMap(RenderCmd& cmd, SkinningVertexUniforms& skinningVu, const SkinningParameters& parameters)
    {
        cmd.normalTexture = s_flatNormalTexture;
        cmd.normalSampler = s_defaultSampler;
        skinningVu.textureCoordinateParameters[3] = 0.0f;
        if (parameters.normalMapScale == 0.0f || parameters.normalTextureId == 0u)
        {
            return;
        }
        auto* normalTexture = static_cast<SDL_GPUTexture*>(LookupTextureForDraw(parameters.normalTextureId));
        if (!normalTexture)
        {
            return;
        }
        void* normalSampler = LookupWorldSampler(parameters.normalTextureId);
        cmd.normalTexture = normalTexture;
        cmd.normalSampler = normalSampler ? static_cast<SDL_GPUSampler*>(normalSampler) : s_defaultSampler;
        skinningVu.textureCoordinateParameters[3] = parameters.normalMapScale;
    }

    [[nodiscard]] bool RenderSkinnedTriangles(std::span<const SkinnedVertex3D> vertices, std::uint32_t color,
                                              std::uint32_t textureId, const SkinningParameters& parameters) override
    {
        return RecordSkinnedDraw(vertices.data(), Render::BonesIndexed(vertices), FrameMesh(vertices),
                                 static_cast<Uint32>(vertices.size()), color, textureId, parameters);
    }

    [[nodiscard]] bool RenderWeightedSkinnedTriangles(std::span<const SkinnedWeightedVertex3D> vertices,
                                                      std::uint32_t color, std::uint32_t textureId,
                                                      const SkinningParameters& parameters) override
    {
        return RecordSkinnedDraw(vertices.data(), Render::BonesIndexed(vertices), FrameMesh(vertices),
                                 static_cast<Uint32>(vertices.size()), color, textureId, parameters);
    }

    // What RecordSkinnedDraw needs of vertices uploaded with the frame: their count and stride.
    template <typename Vertex> [[nodiscard]] static CachedSkinnedMesh FrameMesh(std::span<const Vertex> vertices)
    {
        return CachedSkinnedMesh{0u, 0u, 0u, static_cast<Uint32>(vertices.size()), sizeof(Vertex)};
    }

    [[nodiscard]] bool IsSkinnedMeshCached(const void* mesh) const override
    {
        return s_skinnedMeshes.contains(mesh);
    }

    [[nodiscard]] bool CacheSkinnedMesh(const void* mesh, std::span<const SkinnedVertex3D> vertices) override
    {
        return CacheMesh(mesh, vertices);
    }

    [[nodiscard]] bool CacheWeightedSkinnedMesh(const void* mesh,
                                                std::span<const SkinnedWeightedVertex3D> vertices) override
    {
        return CacheMesh(mesh, vertices);
    }

    template <typename Vertex> [[nodiscard]] bool CacheMesh(const void* mesh, std::span<const Vertex> vertices)
    {
        constexpr Uint32 stride = sizeof(Vertex);
        if (!s_device || vertices.empty() || vertices.size() > std::numeric_limits<Uint32>::max() / stride ||
            s_skinnedMeshes.contains(mesh))
        {
            return false;
        }

        const Uint32 vertexCount = static_cast<Uint32>(vertices.size());
        const Uint32 unitCount = vertexCount * (stride / k_SkinnedMeshUnitBytes);
        const Uint32 bonesUsed = Render::BonesIndexed(vertices);
        std::optional<CachedSkinnedMesh> range;
        for (Uint32 block = 0u; block < s_skinnedMeshBlocks.size() && !range; ++block)
        {
            if (const auto first = s_skinnedMeshBlocks[block].ranges.Allocate(unitCount))
                range = CachedSkinnedMesh{block, *first, unitCount, vertexCount, stride, bonesUsed};
        }
        if (!range)
        {
            // shortcut: blocks are never released before Shutdown, fine while the loaded meshes fit a few blocks;
            // release empty ones if a long session keeps growing them.
            const Uint32 blockUnits = std::max(k_SkinnedMeshBlockUnits, unitCount);
            SDL_GPUBufferCreateInfo bufferInfo{};
            bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
            bufferInfo.size = blockUnits * k_SkinnedMeshUnitBytes;
            SDL_GPUBuffer* buffer = SDL_CreateGPUBuffer(s_device, &bufferInfo);
            if (!buffer)
            {
                mu::log::Get("render")->error("SDL_gpu -- skinned mesh buffer creation failed: {}", SDL_GetError());
                return false;
            }
            s_skinnedMeshBlocks.push_back({buffer, Render::RangeAllocator(blockUnits)});
            const auto first = s_skinnedMeshBlocks.back().ranges.Allocate(unitCount);
            range = CachedSkinnedMesh{static_cast<Uint32>(s_skinnedMeshBlocks.size() - 1u),
                                      *first,
                                      unitCount,
                                      vertexCount,
                                      stride,
                                      bonesUsed};
        }

        s_pendingSkinnedMeshUploads.push_back({*range, s_skinnedMeshStaging.size()});
        const auto bytes = std::as_bytes(vertices);
        s_skinnedMeshStaging.insert(s_skinnedMeshStaging.end(), bytes.begin(), bytes.end());
        s_skinnedMeshes.emplace(mesh, *range);
        return true;
    }

    [[nodiscard]] bool RenderCachedSkinnedMesh(const void* mesh, std::uint32_t color, std::uint32_t textureId,
                                               const SkinningParameters& parameters) override
    {
        const auto cached = s_skinnedMeshes.find(mesh);
        if (cached == s_skinnedMeshes.end())
        {
            return false;
        }
        return RecordSkinnedDraw(nullptr, 0u, cached->second, cached->second.vertexCount, color, textureId, parameters);
    }

    [[nodiscard]] bool RenderCachedSkinnedShadow(const void* mesh, std::uint32_t color,
                                                 const SkinningParameters& parameters,
                                                 const ShadowProjection& projection) override
    {
        const auto cached = s_skinnedMeshes.find(mesh);
        if (cached == s_skinnedMeshes.end())
        {
            return false;
        }
        return RecordSkinnedDraw(nullptr, 0u, cached->second, cached->second.vertexCount, color, 0u, parameters,
                                 &projection);
    }

    // CPU bookkeeping only, so a model released after Shutdown finds an empty cache.
    void ReleaseSkinnedMesh(const void* mesh) override
    {
        const auto cached = s_skinnedMeshes.find(mesh);
        if (cached == s_skinnedMeshes.end())
        {
            return;
        }
        s_deferredSkinnedMeshFrees.push_back({cached->second, s_submittedFrames});
        s_skinnedMeshes.erase(cached);
    }

    [[nodiscard]] bool SetTerrainHeights(std::span<const float> heights, std::span<const std::uint16_t> walls,
                                         std::uint32_t version) override
    {
        if (s_terrainHeightVersion == version)
        {
            return true;
        }
        if (!s_device || heights.size() != Render::kTerrainCells || walls.size() != Render::kTerrainCells)
        {
            return false;
        }

        if (!s_terrainHeightGpuBuf)
        {
            SDL_GPUTransferBufferCreateInfo transferInfo{};
            transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
            transferInfo.size = sizeof(Render::TerrainHeightGrid);
            SDL_GPUBufferCreateInfo bufferInfo{};
            bufferInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
            bufferInfo.size = sizeof(Render::TerrainHeightGrid);
            s_terrainHeightTransferBuf = SDL_CreateGPUTransferBuffer(s_device, &transferInfo);
            s_terrainHeightGpuBuf = s_terrainHeightTransferBuf ? SDL_CreateGPUBuffer(s_device, &bufferInfo) : nullptr;
            if (!s_terrainHeightGpuBuf)
            {
                mu::log::Get("render")->error("SDL_gpu -- terrain height buffer creation failed: {}", SDL_GetError());
                DestroyTerrainHeightBuffers();
                return false;
            }
        }

        // Cycled, so frames still in flight keep reading the previous heights.
        auto* grid = static_cast<Render::TerrainHeightGrid*>(
            SDL_MapGPUTransferBuffer(s_device, s_terrainHeightTransferBuf, true));
        if (!grid)
        {
            mu::log::Get("render")->error("SDL_gpu -- failed to map terrain height buffer: {}", SDL_GetError());
            return false;
        }
        Render::BuildTerrainHeightGrid(heights.data(), walls.data(), *grid);
        SDL_UnmapGPUTransferBuffer(s_device, s_terrainHeightTransferBuf);

        s_terrainHeightVersion = version;
        s_terrainHeightUploadPending = true;
        mu::log::Get("render")->info("SDL_gpu -- terrain heights uploaded (version {})", version);
        return true;
    }

    // -----------------------------------------------------------------------
    // RenderQuadStrip: Render a quad strip as triangle list.
    // Converts vertex pairs (0,1), (2,3), ... into triangles.
    // Strip-to-triangle index pattern: (0,1,2),(1,3,2),(2,3,4),(3,5,4),...
    // -----------------------------------------------------------------------
    void RenderQuadStrip(std::span<const Vertex3D> vertices, std::uint32_t textureId) override
    {
        if (vertices.size() < 2 || !s_frameActive || !m_colorWriteEnabled || m_stencilTestEnabled)
        {
            return;
        }

        const std::uint32_t resolvedTexId = ResolveTextureId(textureId);
        void* pTex = LookupTextureForDraw(resolvedTexId);
        if (!pTex)
        {
            mu::log::Get("render")->warn("SDL_gpu::RenderQuadStrip -- unknown textureId {}, skipping", textureId);
            return;
        }
        if (resolvedTexId == 0u)
            ++s_dbgWhiteTextureDrawsThisFrame;
        else
            ++s_dbgRealTextureDrawsThisFrame;

        const Uint32 byteSize = static_cast<Uint32>(vertices.size() * sizeof(Vertex3D));
        const Uint32 vtxOffset = UploadVertices(vertices.data(), byteSize, sizeof(Vertex3D));
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

        // RenderQuadStrip uses the 3D pipeline set (Vertex3D layout).
        const AlphaCoverageChoice coverage =
            ChooseAlphaCoverage(GetActive3DPipeline(), resolvedTexId, GetVertexAlpha(vertices));
        SDL_GPUGraphicsPipeline* pipeline = coverage.pipeline;
        if (!pipeline)
        {
            return;
        }

        void* pSampler = LookupWorldSampler(resolvedTexId);

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
        cmd.vtxStride = sizeof(Vertex3D);
        cmd.idxCount = numIndices;
        cmd.stripIdxOffset = stripIdxByteOffset;
        cmd.vu.mvp = m_mvpMatrix;
        cmd.vu.fogStart = m_fogUniform.fogStart;
        cmd.vu.fogEnd = m_fogUniform.fogEnd;
        cmd.fogUniform = m_fogUniform;
        cmd.fogUniform.alphaToCoverage = coverage.alphaToCoverage ? 1u : 0u;
        RecordDrawState(cmd);
        s_renderCmds.push_back(cmd);

        ++s_dbgDrawCallsThisFrame;
        s_dbgVtxBytesThisFrame += byteSize;
        FrameProfiler::Count(FrameProfiler::Counter::DrawCalls);
        FrameProfiler::Count(FrameProfiler::Counter::VertexBytes, byteSize);
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
    // Fog uniform buffer support.
    // -----------------------------------------------------------------------
    void SetDepthMask(bool enabled) override
    {
        m_depthMaskEnabled = enabled;
    }
    void SetCullFace(bool enabled) override
    {
        m_cullFaceEnabled = enabled;
    }
    // SetColorMask — track color write state.
    // When all channels are disabled (shadow volume stencil passes), draw calls
    // are skipped entirely since we have no stencil buffer support yet.
    void SetColorMask(bool r, bool g, bool b, bool a) override
    {
        m_colorWriteEnabled = (r || g || b || a);
    }
    // SetStencilTest — track stencil state.
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
        // Propagate alpha test state to the fog uniform
        // so the fragment shader's `if (alphaDiscardEnabled && color.a <= alphaThreshold) discard;`
        // actually fires for particle sprites.
        m_fogUniform.alphaDiscardEnabled = enabled ? 1u : 0u;
        s_fogDirty = true;
    }

    // Override SetAlphaFunc to propagate alpha threshold
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
        m_fogUniform.fogEnabled = enabled ? 1u : 0u;
        s_fogDirty = true;
    }
    void BindTexture(int texId) override
    {
        m_boundTextureId = texId;
    }

    void SetFog(const FogParams& params) override
    {
        m_fogParams = params;
        m_fogEnabled = params.mode != 0;

        // Map GL-style FogParams (mode/start/end/density/color) to the
        // HLSL FogUniforms cbuffer layout used by basic_textured.frag.hlsl.
        // fogEnabled: true when mode != 0 (mode 0 = no fog / GL_LINEAR from caller).
        // alphaDiscardEnabled / alphaThreshold: not in FogParams; default off.
        m_fogUniform.fogEnabled = m_fogEnabled ? 1u : 0u;
        // Preserve alpha discard state — SetFog must NOT reset
        // alphaDiscardEnabled/alphaThreshold set by SetAlphaTest/SetAlphaFunc.
        m_fogUniform.alphaToCoverage = 0u;
        m_fogUniform.fogStart = params.start;
        m_fogUniform.fogEnd = params.end;
        m_fogUniform.fogPadding[0] = 0.0f;
        m_fogUniform.fogPadding[1] = 0.0f;
        m_fogUniform.fogColor[0] = params.color[0];
        m_fogUniform.fogColor[1] = params.color[1];
        m_fogUniform.fogColor[2] = params.color[2];
        m_fogUniform.fogColor[3] = params.color[3];
        s_fogDirty = true;
    }

    void SetSceneLighting(const SceneLighting& lighting) override
    {
        SceneFog fog{};
        fog.enabled = m_fogEnabled;
        fog.start = m_fogUniform.fogStart;
        fog.end = m_fogUniform.fogEnd;
        fog.color[0] = m_fogUniform.fogColor[0];
        fog.color[1] = m_fogUniform.fogColor[1];
        fog.color[2] = m_fogUniform.fogColor[2];
        s_sceneLightingUniform = PackSceneLightingUniform(lighting, fog);
        // The scene sets its lighting right after its camera, so the model-view is the view.
        m_worldFromViewMatrix = glm::inverse(m_modelViewMatrix);
    }

    void SetDynamicLights(const DynamicLightsUniform& lights) override
    {
        s_dynamicLightsUniform = lights;
    }

    [[nodiscard]] bool SetEnhancedLighting(bool enabled) override
    {
        s_enhancedLightingEnabled = enabled;
        return !enabled || s_enhancedLightingAvailable;
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
                mu::log::Get("render")->warn("modelview matrix stack overflow (depth {})", k_MatrixStackDepth);
        }
        else
        {
            if (m_projStackTop < k_MatrixStackDepth)
                m_projStack[m_projStackTop++] = m_projMatrix;
            else
                mu::log::Get("render")->warn("projection matrix stack overflow (depth {})", k_MatrixStackDepth);
        }
    }

    void PopMatrix() override
    {
        if (m_matrixMode == 0x1700) // GL_MODELVIEW
        {
            if (m_mvStackTop > 0)
                m_modelViewMatrix = m_mvStack[--m_mvStackTop];
            else
                mu::log::Get("render")->warn("modelview matrix stack underflow");
        }
        else
        {
            if (m_projStackTop > 0)
                m_projMatrix = m_projStack[--m_projStackTop];
            else
                mu::log::Get("render")->warn("projection matrix stack underflow");
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
    // Records one skinned draw. `frameVertices` (indexing `frameBonesUsed` bones) are uploaded with this frame's
    // vertices; without them the draw reads `cached`, already on the GPU. `cached.stride` names the vertex type.
    // With `shadow`, it is drawn as that ground shadow.
    [[nodiscard]] bool RecordSkinnedDraw(const void* frameVertices, Uint32 frameBonesUsed,
                                         const CachedSkinnedMesh& cached, Uint32 vertexCount, std::uint32_t color,
                                         std::uint32_t textureId, const SkinningParameters& parameters,
                                         const ShadowProjection* shadow = nullptr)
    {
        if (vertexCount == 0u || vertexCount % 3 != 0 || !s_frameActive || !m_colorWriteEnabled ||
            m_stencilTestEnabled || parameters.boneMatrices.empty() || parameters.boneMatrices.size() % 12 != 0)
        {
            return false;
        }

        const std::uint32_t resolvedTexId = ResolveTextureId(textureId);
        auto* texture = static_cast<SDL_GPUTexture*>(LookupTextureForDraw(resolvedTexId));
        if (!texture)
        {
            return false;
        }

        const int pipelineIdx = GetActivePipelineIndex();
        const bool cullFaceEnabled = m_cullFaceEnabled && !s_disableD3D12Culling;
        const bool enhanced = !shadow && UsesEnhancedLighting();
        const bool enhancedLit = enhanced && parameters.lightEnabled;
        const bool weighted = cached.stride == sizeof(SkinnedWeightedVertex3D);
        const WorldPipelines& set =
            !enhanced     ? (weighted ? s_pipelinesSkinnedWeighted : s_pipelinesSkinned)
            : enhancedLit ? (weighted ? s_pipelinesSkinnedWeightedLit : s_pipelinesSkinnedLit)
                          : (weighted ? s_pipelinesSkinnedWeightedEnhancedFog : s_pipelinesSkinnedEnhancedFog);
        SDL_GPUGraphicsPipeline* pipeline =
            set.Select(static_cast<std::size_t>(pipelineIdx), m_depthTestEnabled, m_depthMaskEnabled, cullFaceEnabled);
        // One colour for the whole draw: its alpha is the vertices' alpha.
        const AlphaCoverageChoice coverage =
            shadow ? AlphaCoverageChoice{pipeline, false}
                   : ChooseAlphaCoverage(pipeline, resolvedTexId,
                                         (color >> 24u) == 0xFFu ? VertexAlpha::Opaque : VertexAlpha::Translucent);
        pipeline = coverage.pipeline;
        if (shadow)
        {
            // Only the states the CPU shadow is drawn in; the heights must be on the GPU.
            SDL_GPUGraphicsPipeline* const* depthOn =
                weighted ? s_pipelinesSkinnedWeightedShadow : s_pipelinesSkinnedShadow;
            SDL_GPUGraphicsPipeline* const* depthOff =
                weighted ? s_pipelinesSkinnedWeightedShadowDepthOff : s_pipelinesSkinnedShadowDepthOff;
            pipeline = !s_terrainHeightVersion ? nullptr
                       : !m_depthTestEnabled   ? depthOff[pipelineIdx]
                       : !m_depthMaskEnabled   ? depthOn[pipelineIdx]
                                               : nullptr;
        }
        if (!pipeline)
        {
            return false;
        }

        const Uint32 bonesUsed = frameVertices ? frameBonesUsed : cached.bonesUsed;
        const std::optional<BonePaletteSlot> palette = RecordBonePalette(parameters, bonesUsed);
        if (!palette)
        {
            return false;
        }

        Uint32 byteSize = 0u;
        Uint32 vtxOffset = cached.firstUnit * k_SkinnedMeshUnitBytes;
        if (frameVertices)
        {
            byteSize = vertexCount * cached.stride;
            vtxOffset = UploadVertices(frameVertices, byteSize, cached.stride);
            if (vtxOffset == ~0u)
            {
                return false;
            }
        }
        const Uint32 colorOffset = UploadVertices(&color, sizeof(color), sizeof(color));
        if (colorOffset == ~0u)
        {
            return false;
        }
        byteSize += sizeof(color);

        RenderCmd cmd{};
        cmd.type = RenderCmdType::DrawSkinnedTriangles;
        SkinningVertexUniforms& skinningVu = s_skinningUniforms.emplace_back();
        cmd.skinningIndex = static_cast<Uint32>(s_skinningUniforms.size() - 1u);
        cmd.pipeline = pipeline;
        cmd.texture = texture;
        void* sampler = LookupWorldSampler(resolvedTexId);
        cmd.sampler = sampler ? static_cast<SDL_GPUSampler*>(sampler) : s_defaultSampler;
        cmd.vtxBuffer = frameVertices ? nullptr : s_skinnedMeshBlocks[cached.block].buffer;
        cmd.vtxOffset = vtxOffset;
        cmd.vtxStride = cached.stride;
        cmd.colorOffset = colorOffset;
        cmd.vtxCount = vertexCount;
        skinningVu.mvp = m_mvpMatrix;
        skinningVu.bodyOriginAndScale[0] = parameters.bodyOrigin[0];
        skinningVu.bodyOriginAndScale[1] = parameters.bodyOrigin[1];
        skinningVu.bodyOriginAndScale[2] = parameters.bodyOrigin[2];
        skinningVu.bodyOriginAndScale[3] = parameters.bodyScale;
        skinningVu.skinningScales[0] = parameters.boneScale;
        skinningVu.skinningScales[1] = parameters.restPoseScale;
        skinningVu.palette[0] = palette->rowOffset;
        skinningVu.palette[1] = palette->bones;
        skinningVu.palette[2] = parameters.translate ? 1u : 0u;
        skinningVu.palette[3] = enhancedLit               ? EnhancedSkinningLighting()
                                : parameters.lightEnabled ? k_SkinningLightingClassic
                                                          : k_SkinningLightingNone;
        skinningVu.lightDirection[0] = parameters.lightDirection[0];
        skinningVu.lightDirection[1] = parameters.lightDirection[1];
        skinningVu.lightDirection[2] = parameters.lightDirection[2];
        skinningVu.lightDirection[3] = parameters.highlightKnee;
        skinningVu.fogParameters[0] = m_fogUniform.fogStart;
        skinningVu.fogParameters[1] = m_fogUniform.fogEnd;
        skinningVu.textureCoordinates[0] = static_cast<std::uint32_t>(parameters.textureCoordinates);
        skinningVu.chromeParameters[0] = parameters.chromeWave;
        skinningVu.chromeParameters[1] = parameters.chromeWave2;
        skinningVu.chromeParameters[2] = parameters.chromeLight[0];
        skinningVu.chromeParameters[3] = parameters.chromeLight[1];
        skinningVu.textureCoordinateParameters[0] = parameters.textureCoordinateOffset[0];
        skinningVu.textureCoordinateParameters[1] = parameters.textureCoordinateOffset[1];
        skinningVu.textureCoordinateParameters[2] = parameters.chromeTimeTerm;
        if (enhancedLit)
        {
            BindNormalMap(cmd, skinningVu, parameters);
            const glm::vec4 eyePosition = glm::inverse(m_modelViewMatrix) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            skinningVu.eyePositionAndSpecular[0] = eyePosition.x;
            skinningVu.eyePositionAndSpecular[1] = eyePosition.y;
            skinningVu.eyePositionAndSpecular[2] = eyePosition.z;
            skinningVu.eyePositionAndSpecular[3] = parameters.specularStrength;
            PackVertexToWorldRows(skinningVu);
        }
        if (shadow)
        {
            ShadowVertexUniforms& shadowVu = s_shadowUniforms.emplace_back();
            cmd.shadowIndex = static_cast<Uint32>(s_shadowUniforms.size() - 1u);
            shadowVu.origin[0] = shadow->origin[0];
            shadowVu.origin[1] = shadow->origin[1];
            shadowVu.origin[2] = shadow->origin[2];
            shadowVu.origin[3] = shadow->specialHeight;
            shadowVu.projection[0] = shadow->shearX;
            shadowVu.projection[1] = shadow->shearHeight;
            shadowVu.projection[2] = shadow->terrain ? 1.0f : 0.0f;
        }
        cmd.fogUniform = m_fogUniform;
        cmd.fogUniform.alphaToCoverage = coverage.alphaToCoverage ? 1u : 0u;
        RecordDrawState(cmd);
        s_renderCmds.push_back(cmd);

        if (FrameProfiler::g_CountersEnabled)
        {
            // shortcut: a hash of the key, not the key; a collision only undercounts by one.
            const auto mix = [](std::uint64_t seed, std::uint64_t value)
            { return seed ^ (value + 0x9e3779b97f4a7c15ull + (seed << 6) + (seed >> 2)); };
            std::uint64_t key = static_cast<std::uint64_t>(FrameProfiler::CurrentPass());
            key = mix(key, reinterpret_cast<std::uintptr_t>(cmd.vtxBuffer));
            key = mix(key, cmd.vtxOffset);
            key = mix(key, reinterpret_cast<std::uintptr_t>(cmd.texture));
            key = mix(key, reinterpret_cast<std::uintptr_t>(cmd.pipeline));
            if (s_skinnedDrawKeys.insert(key).second)
                FrameProfiler::Count(FrameProfiler::Counter::DistinctSkinnedDraws);
        }

        ++s_dbgDrawCallsThisFrame;
        s_dbgVtxBytesThisFrame += byteSize;
        FrameProfiler::Count(FrameProfiler::Counter::DrawCalls);
        FrameProfiler::Count(FrameProfiler::Counter::VertexBytes, byteSize);
        return true;
    }

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
    // CPU-side fog uniform data, uploaded to GPU when dirty.
    FogUniform m_fogUniform{};

    // Matrix stack for 3D rendering (replaces OpenGL fixed-function matrix stack).
    static constexpr int k_MatrixStackDepth = 16;
    int m_matrixMode = 0x1700; // GL_MODELVIEW
    glm::mat4 m_projMatrix{1.0f};
    glm::mat4 m_modelViewMatrix{1.0f};
    // Inverse of the scene's view matrix, captured by SetSceneLighting().
    glm::mat4 m_worldFromViewMatrix{1.0f};
    glm::mat4 m_projStack[k_MatrixStackDepth]{};
    glm::mat4 m_mvStack[k_MatrixStackDepth]{};
    int m_projStackTop = 0;
    int m_mvStackTop = 0;
    glm::mat4 m_mvpMatrix{1.0f};

    // The blend and depth state a draw was recorded with: batching compares it, and
    // post-processing picks the draws that feed the bloom by it.
    void RecordDrawState(RenderCmd& command) const
    {
        command.blendMode = m_activeBlendMode;
        command.blendEnabled = m_blendEnabled;
        command.depthTestEnabled = m_depthTestEnabled;
        command.depthMaskEnabled = m_depthMaskEnabled;
        command.cullFaceEnabled = m_cullFaceEnabled;
        command.bloomExcluded = s_bloomExclusionDepth > 0;
    }

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

    [[nodiscard]] static bool UsesEnhancedLighting()
    {
        return s_enhancedLightingEnabled && s_enhancedLightingAvailable;
    }

    [[nodiscard]] SDL_GPUGraphicsPipeline* GetActive3DPipeline() const
    {
        const WorldPipelines& set = UsesEnhancedLighting() ? s_pipelines3DEnhancedFog : s_pipelines3D;
        return set.Select(static_cast<std::size_t>(GetActivePipelineIndex()), m_depthTestEnabled, m_depthMaskEnabled,
                          m_cullFaceEnabled && !s_disableD3D12Culling);
    }

    // Alpha-to-coverage for an alpha-tested cutout while anti-aliasing is on
    // (Render::Antialiasing::UsesAlphaToCoverage): the draw uses the opaque
    // alpha-to-coverage variant of its pipeline and the shader's sharpening.
    struct AlphaCoverageChoice
    {
        SDL_GPUGraphicsPipeline* pipeline;
        bool alphaToCoverage;
    };

    enum class VertexAlpha : std::uint8_t
    {
        Opaque,
        Translucent,
    };

    template <typename Vertex> [[nodiscard]] static VertexAlpha GetVertexAlpha(std::span<const Vertex> vertices)
    {
        // Packed ABGR: alpha is the top byte.
        const bool opaque = std::all_of(vertices.begin(), vertices.end(),
                                        [](const Vertex& vertex) { return (vertex.color >> 24u) == 0xFFu; });
        return opaque ? VertexAlpha::Opaque : VertexAlpha::Translucent;
    }

    [[nodiscard]] AlphaCoverageChoice ChooseAlphaCoverage(SDL_GPUGraphicsPipeline* pipeline, std::uint32_t textureId,
                                                          VertexAlpha vertexAlpha) const
    {
        const AlphaCoverageChoice unchanged{pipeline, false};
        if (!pipeline || s_sampleCount == Render::Antialiasing::OffSampleCount)
        {
            return unchanged;
        }
        using Render::Antialiasing::Blending;
        Render::Antialiasing::CutoutDraw draw{};
        draw.multisampled = true;
        draw.alphaTest = m_fogUniform.alphaDiscardEnabled != 0u;
        draw.depthTest = m_depthTestEnabled;
        draw.depthWrite = m_depthMaskEnabled;
        draw.blending = !m_blendEnabled                             ? Blending::Off
                        : m_activeBlendMode == mu::BlendMode::Alpha ? Blending::Alpha
                                                                    : Blending::Other;
        draw.opaqueVertices = vertexAlpha == VertexAlpha::Opaque;
        draw.enhancedTexture = UsesEnhancedSampler(textureId);
        if (!Render::Antialiasing::UsesAlphaToCoverage(draw))
        {
            return unchanged;
        }
        for (const AlphaCoveragePipeline& entry : s_alphaCoveragePipelines)
        {
            if ((*entry.opaque == pipeline || *entry.alphaBlend == pipeline) && entry.variant)
            {
                if (!s_alphaCoverageDrawLogged)
                {
                    s_alphaCoverageDrawLogged = true;
                    mu::log::Get("render")->info("SDL_gpu -- alpha-to-coverage: first cutout draw (texture {}) at {}x",
                                                 textureId, s_sampleCount);
                }
                return {entry.variant, true};
            }
        }
        return unchanged;
    }

    [[nodiscard]] SDL_GPUGraphicsPipeline* GetActiveQuadPipeline(Render::Topology::QuadSpace space) const
    {
        if (Render::Topology::Uses3DPipeline(space))
        {
            return GetActive3DPipeline();
        }
        return s_pipelines2DDepthOff[GetActivePipelineIndex()];
    }

    // -----------------------------------------------------------------------
    // UploadVertices: Append vertex data to per-frame CPU staging memory.
    // Returns the byte offset in the GPU vertex buffer, or ~0u on failure.
    //
    // CPU staging grows as needed, preventing world geometry from consuming fixed
    // transfer capacity and dropping UI commands recorded later in the frame.
    // EndFrame() copies all accumulated vertex data to the GPU buffer in a
    // single copy pass BEFORE beginning the render pass, so draw commands
    // always read current-frame vertex data (no 1-frame latency).
    // -----------------------------------------------------------------------
    [[nodiscard]] static Uint32 UploadVertices(const void* pData, Uint32 byteSize, Uint32 stride)
    {
        if (!pData || byteSize == 0u || stride == 0u)
        {
            return ~0u;
        }

        // A whole number of strides in, so the replay can address the data by vertex index.
        const Uint32 alignedOffset = (s_vtxOffset + stride - 1u) / stride * stride;
        if (byteSize > std::numeric_limits<Uint32>::max() - alignedOffset)
        {
            mu::log::Get("render")->error("SDL_gpu -- vertex staging size exceeds Uint32 range");
            return ~0u;
        }

        const Uint32 requiredSize = alignedOffset + byteSize;
        // Kept at its largest size across frames (s_vtxOffset is the used part): resize() zero-fills
        // what it adds, which the copy below would overwrite anyway.
        if (s_vtxScratch.size() < requiredSize)
            s_vtxScratch.resize(std::max<std::size_t>(requiredSize, s_vtxScratch.size() * 2u));
        std::memcpy(s_vtxScratch.data() + alignedOffset, pData, byteSize);

        s_vtxOffset = requiredSize;
        return alignedOffset;
    }

    struct BonePaletteSlot
    {
        Uint32 rowOffset;
        Uint32 bones;
    };

    // Uploads the bones the draw indexes (`bonesUsed`, 0 when unknown: all of them). Meshes of one
    // object share the upload while it holds every bone they index.
    [[nodiscard]] static std::optional<BonePaletteSlot> RecordBonePalette(const SkinningParameters& parameters,
                                                                          Uint32 bonesUsed)
    {
        const auto palette = parameters.boneMatrices;
        if (palette.empty() || palette.size() % 12 != 0)
        {
            return std::nullopt;
        }
        const Uint32 bones = Render::PaletteBonesToUpload(static_cast<Uint32>(palette.size() / 12), bonesUsed);
        if (palette.data() == s_lastBonePalette && palette.size() == s_lastBonePaletteSize &&
            parameters.paletteVersion == s_lastBonePaletteVersion && bones <= s_lastBonePaletteBones)
        {
            return BonePaletteSlot{s_lastBonePaletteRowOffset, s_lastBonePaletteBones};
        }

        const std::size_t floats = static_cast<std::size_t>(bones) * 12u;
        const std::size_t rowOffset = s_boneRowScratch.size() / 4;
        if (rowOffset > std::numeric_limits<Uint32>::max() ||
            floats > std::numeric_limits<Uint32>::max() / sizeof(float) - s_boneRowScratch.size())
        {
            return std::nullopt;
        }

        s_boneRowScratch.insert(s_boneRowScratch.end(), palette.begin(), palette.begin() + floats);
        FrameProfiler::Count(FrameProfiler::Counter::BonePaletteBytes,
                             static_cast<std::uint32_t>(floats * sizeof(float)));
        s_lastBonePalette = palette.data();
        s_lastBonePaletteSize = palette.size();
        s_lastBonePaletteVersion = parameters.paletteVersion;
        s_lastBonePaletteRowOffset = static_cast<Uint32>(rowOffset);
        s_lastBonePaletteBones = bones;
        return BonePaletteSlot{s_lastBonePaletteRowOffset, bones};
    }

    // -----------------------------------------------------------------------
    // Static resource creation/destruction helpers.
    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------
    // Loads all 6 HLSL shader blobs from MU_SHADER_DIR and creates
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
                    mu::log::Get("render")->error("SDL_gpu -- FATAL: failed to load shader blob {}.{}", name, stage);
                }
                else
                {
                    mu::log::Get("render")->warn("SDL_gpu -- failed to load shader blob {}.{} (non-fatal)", name,
                                                 stage);
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
                    mu::log::Get("render")->error("SDL_gpu -- FATAL: SDL_CreateGPUShader failed for {}.{}: {}", name,
                                                  stage, SDL_GetError());
                }
                else
                {
                    mu::log::Get("render")->warn("SDL_gpu -- SDL_CreateGPUShader failed for {}.{}: {} (non-fatal)",
                                                 name, stage, SDL_GetError());
                }
            }
            return shader;
        };

        // basic_textured.vert — fatal: required for all 2D textured draws.
        // Inputs: pos(TEXCOORD0), uv(TEXCOORD1), color(TEXCOORD2)
        // Uniform buffers: b0, space1 (Transform)
        s_vertShader2D = createShader("basic_textured", "vert", SDL_GPU_SHADERSTAGE_VERTEX, 0, 0, 1, /*fatal=*/true);
        if (!s_vertShader2D)
        {
            return false;
        }

        // basic_textured.frag — fatal: required for textured 2D draws.
        // Samplers: t0 (texture), s0 (sampler); Uniform buffers: FogUniforms (pushed per-draw)
        // Changed from numStorageBuffers=1 to numUniformBuffers=1 so fog/alpha
        // data can be pushed per-draw-call via SDL_PushGPUFragmentUniformData (not a GPU buffer).
        s_fragShaderTex = createShader("basic_textured", "frag", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 1, /*fatal=*/true);
        if (!s_fragShaderTex)
        {
            SDL_ReleaseGPUShader(s_device, s_vertShader2D);
            s_vertShader2D = nullptr;
            return false;
        }

        // basic_colored.vert — non-fatal (colored path degrades gracefully).
        // Inputs: pos(TEXCOORD0), color(TEXCOORD1)
        // Uniform buffers: b0, space1 (ScreenSize)
        // NOTE: Shader handles below are loaded as pipeline hooks for
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

        // Rest-pose BMD geometry: one vertex storage buffer for packed bone rows, one uniform buffer.
        s_vertShaderSkinned =
            createShader("skinned_textured", "vert", SDL_GPU_SHADERSTAGE_VERTEX, 0, 1, 1, /*fatal=*/true);
        if (!s_vertShaderSkinned)
        {
            ReleaseShaders();
            return false;
        }
        // glTF geometry: the same resources, plus the joints, weights and tangent of each vertex.
        s_vertShaderSkinnedWeighted =
            createShader("skinned_weighted", "vert", SDL_GPU_SHADERSTAGE_VERTEX, 0, 1, 1, /*fatal=*/true);
        if (!s_vertShaderSkinnedWeighted)
        {
            ReleaseShaders();
            return false;
        }

        // Body shadows: bone rows and terrain heights, then the skinning and shadow uniforms. Without it,
        // shadows stay on the CPU.
        s_vertShaderSkinnedShadow =
            createShader("skinned_shadow", "vert", SDL_GPU_SHADERSTAGE_VERTEX, 0, 2, 2, /*fatal=*/false);
        s_vertShaderSkinnedWeightedShadow =
            createShader("skinned_weighted_shadow", "vert", SDL_GPU_SHADERSTAGE_VERTEX, 0, 2, 2, /*fatal=*/false);

        // Enhanced lighting — non-fatal: without them Enhanced lighting is unavailable.
        // Uniform buffers: FogUniforms (b0) per draw, SceneLighting (b1) per frame.
        s_fragShaderTexFog =
            createShader("basic_textured_fog", "frag", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 2, /*fatal=*/false);
        // skinned_lit.frag also reads the frame's point lights, DynamicLights (b2).
        // skinned_lit.frag samples the texture and the normal map (k_FragmentSamplerSlotNormalMap).
        s_fragShaderLit = createShader("skinned_lit", "frag", SDL_GPU_SHADERSTAGE_FRAGMENT, 2, 0, 3, /*fatal=*/false);

        // Post-processing — non-fatal: without them post-processing is unavailable.
        // Bloom passes: source texture (t0) and BloomUniforms (b0); composite: scene, bloom and
        // grading textures (t0-t2) and CompositeUniforms (b0).
        s_vertShaderPostFullscreen =
            createShader("post_fullscreen", "vert", SDL_GPU_SHADERSTAGE_VERTEX, 0, 0, 0, /*fatal=*/false);
        s_fragShaderBloomDown =
            createShader("post_bloom_down", "frag", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 1, /*fatal=*/false);
        s_fragShaderBloomUp =
            createShader("post_bloom_up", "frag", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 1, /*fatal=*/false);
        s_fragShaderComposite =
            createShader("post_composite", "frag", SDL_GPU_SHADERSTAGE_FRAGMENT, 3, 0, 1, /*fatal=*/false);

        mu::log::Get("render")->info("SDL_gpu -- shaders loaded for driver: {}", driverName ? driverName : "unknown");
        return true;
    }

    // -----------------------------------------------------------------------
    // ReleaseShaders — release all 7 shader handles.
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
        if (s_vertShaderSkinned)
        {
            SDL_ReleaseGPUShader(s_device, s_vertShaderSkinned);
            s_vertShaderSkinned = nullptr;
        }
        if (s_vertShaderSkinnedWeighted)
        {
            SDL_ReleaseGPUShader(s_device, s_vertShaderSkinnedWeighted);
            s_vertShaderSkinnedWeighted = nullptr;
        }
        if (s_vertShaderSkinnedShadow)
        {
            SDL_ReleaseGPUShader(s_device, s_vertShaderSkinnedShadow);
            s_vertShaderSkinnedShadow = nullptr;
        }
        if (s_vertShaderSkinnedWeightedShadow)
        {
            SDL_ReleaseGPUShader(s_device, s_vertShaderSkinnedWeightedShadow);
            s_vertShaderSkinnedWeightedShadow = nullptr;
        }
        if (s_fragShaderTexFog)
        {
            SDL_ReleaseGPUShader(s_device, s_fragShaderTexFog);
            s_fragShaderTexFog = nullptr;
        }
        if (s_fragShaderLit)
        {
            SDL_ReleaseGPUShader(s_device, s_fragShaderLit);
            s_fragShaderLit = nullptr;
        }
        for (SDL_GPUShader** shader :
             {&s_vertShaderPostFullscreen, &s_fragShaderBloomDown, &s_fragShaderBloomUp, &s_fragShaderComposite})
        {
            if (*shader)
            {
                SDL_ReleaseGPUShader(s_device, *shader);
                *shader = nullptr;
            }
        }
    }

    // -----------------------------------------------------------------------
    // Creates one textured blend/depth/cull variant and captures SDL's immediate
    // error before another pipeline build can overwrite it.
    // -----------------------------------------------------------------------
    enum class VertexLayout
    {
        TwoDimensional,
        ThreeDimensional,
        Skinned,
        SkinnedWeighted,       // SkinnedWeightedVertex3D, with skinned_weighted.vert
        SkinnedShadow,         // the skinned layout, with skinned_shadow.vert
        SkinnedWeightedShadow, // the weighted layout, with skinned_weighted_shadow.vert
    };

    enum class FragmentShader
    {
        Textured,
        TexturedEnhancedFog,
        SkinnedLit,
    };

    struct PipelineBuildDescription
    {
        const char* setName;
        const char* blendModeName;
        int blendIndex;
        bool depthTestEnabled;
        bool depthWriteEnabled;
        VertexLayout vertexLayout;
        bool cullFaceEnabled;
        FragmentShader fragmentShader = FragmentShader::Textured;
        bool alphaToCoverage = false;
    };

    // The world sets built together, and what a failed pipeline means for them.
    enum class WorldSetFamily
    {
        Classic,          // required: a failed pipeline fails the renderer
        EnhancedLighting, // optional: Enhanced lighting is available only when every set of it is complete
    };

    // One world set: the four depth/cull states of 3D or skinned world geometry with one fragment
    // shader. Its pipelines are named "<name>-culled", "<name>-no-cull", "<name>-depth-off" and
    // "<name>-depth-read-only", its alpha-to-coverage variants "<name>-culled-alpha-coverage" and
    // "<name>-no-cull-alpha-coverage".
    struct WorldPipelineSetSpec
    {
        WorldPipelines* set;
        const char* name;
        VertexLayout vertexLayout;
        FragmentShader fragmentShader;
        WorldSetFamily family;
    };

    // Every world set. Building, releasing, the replay slots, the alpha-to-coverage variants and the
    // bloom sources all walk this list, so a new world set is one row here.
    [[nodiscard]] static std::span<const WorldPipelineSetSpec> WorldPipelineSets()
    {
        static constexpr WorldPipelineSetSpec sets[] = {
            {&s_pipelines3D, "3d", VertexLayout::ThreeDimensional, FragmentShader::Textured, WorldSetFamily::Classic},
            {&s_pipelinesSkinned, "skinned", VertexLayout::Skinned, FragmentShader::Textured, WorldSetFamily::Classic},
            {&s_pipelines3DEnhancedFog, "3d-enhanced-fog", VertexLayout::ThreeDimensional,
             FragmentShader::TexturedEnhancedFog, WorldSetFamily::EnhancedLighting},
            {&s_pipelinesSkinnedEnhancedFog, "skinned-enhanced-fog", VertexLayout::Skinned,
             FragmentShader::TexturedEnhancedFog, WorldSetFamily::EnhancedLighting},
            {&s_pipelinesSkinnedLit, "skinned-lit", VertexLayout::Skinned, FragmentShader::SkinnedLit,
             WorldSetFamily::EnhancedLighting},
            {&s_pipelinesSkinnedWeighted, "skinned-weighted", VertexLayout::SkinnedWeighted, FragmentShader::Textured,
             WorldSetFamily::Classic},
            {&s_pipelinesSkinnedWeightedEnhancedFog, "skinned-weighted-enhanced-fog", VertexLayout::SkinnedWeighted,
             FragmentShader::TexturedEnhancedFog, WorldSetFamily::EnhancedLighting},
            {&s_pipelinesSkinnedWeightedLit, "skinned-weighted-lit", VertexLayout::SkinnedWeighted,
             FragmentShader::SkinnedLit, WorldSetFamily::EnhancedLighting},
        };
        return sets;
    }

    // The depth/cull states of every world set, with their name suffix and pipeline state.
    struct WorldStateSpec
    {
        WorldPipelines::States WorldPipelines::* states;
        const char* suffix;
        bool depthTestEnabled;
        bool depthWriteEnabled;
        bool cullFaceEnabled;
    };
    static constexpr WorldStateSpec kWorldStates[] = {
        {&WorldPipelines::culled, "culled", true, true, true},
        {&WorldPipelines::noCull, "no-cull", true, true, false},
        {&WorldPipelines::depthOff, "depth-off", false, false, false},
        {&WorldPipelines::depthReadOnly, "depth-read-only", true, false, false},
    };

    [[nodiscard]] static bool IsWorldFamilyComplete(WorldSetFamily family)
    {
        return std::ranges::all_of(WorldPipelineSets(), [family](const WorldPipelineSetSpec& spec)
                                   { return spec.family != family || spec.set->IsComplete(); });
    }

    static void ReleaseWorldFamily(WorldSetFamily family)
    {
        const auto release = [](SDL_GPUGraphicsPipeline* pipeline)
        { SDL_ReleaseGPUGraphicsPipeline(s_device, pipeline); };
        for (const WorldPipelineSetSpec& spec : WorldPipelineSets())
        {
            if (spec.family == family)
            {
                spec.set->Release(release);
            }
        }
    }

    [[nodiscard]] static SDL_GPUShader* GetFragmentShader(FragmentShader shader)
    {
        switch (shader)
        {
        case FragmentShader::TexturedEnhancedFog:
            return s_fragShaderTexFog;
        case FragmentShader::SkinnedLit:
            return s_fragShaderLit;
        case FragmentShader::Textured:
            break;
        }
        return s_fragShaderTex;
    }

    [[nodiscard]] static const char* GetFragmentShaderName(FragmentShader shader)
    {
        switch (shader)
        {
        case FragmentShader::TexturedEnhancedFog:
            return "basic_textured_fog.frag";
        case FragmentShader::SkinnedLit:
            return "skinned_lit.frag";
        case FragmentShader::Textured:
            break;
        }
        return "basic_textured.frag";
    }

    struct PipelineBuildResult
    {
        SDL_GPUGraphicsPipeline* pipeline;
        PipelineBuildDescription description;
        std::string error;
    };

    // Every variant compares depth with LESS_OR_EQUAL, as the classic MU OpenGL client set
    // glDepthFunc(GL_LEQUAL) at the start of each 3D scene (SetDepthFunc, which this backend
    // doesn't propagate). Depth-read-only and 2D pipelines always did. Depth-writing ones need it
    // for models drawn in several depth-writing passes over the same mesh, like the Bill of
    // Balrog's textured body over its chrome body: with LESS the later pass lost every
    // equal-depth fragment, so only the first pass showed.
    [[nodiscard]] static SDL_GPUDepthStencilState MakeDepthStencilState(const PipelineBuildDescription& description)
    {
        SDL_GPUDepthStencilState depthState{};
        depthState.enable_depth_test = description.depthTestEnabled;
        depthState.enable_depth_write = description.depthWriteEnabled;
        depthState.enable_stencil_test = false;
        depthState.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL;
        return depthState;
    }

    [[nodiscard]] static PipelineBuildResult BuildBlendPipeline(SDL_GPUColorTargetBlendState blendState,
                                                                const PipelineBuildDescription& description)
    {
        PipelineBuildResult result{nullptr, description, {}};

        // Get swapchain texture format for pipeline target.
        const SDL_GPUTextureFormat swapchainFmt = SDL_GetGPUSwapchainTextureFormat(s_device, s_window);

        SDL_GPUColorTargetDescription colorTargetDesc{};
        colorTargetDesc.format = swapchainFmt;
        colorTargetDesc.blend_state = blendState;

        SDL_GPUVertexAttribute vertexAttribs[9];
        Uint32 numAttribs = 0;
        SDL_GPUVertexBufferDescription vtxBufDescs[2]{};
        SDL_GPUVertexBufferDescription& vtxBufDesc = vtxBufDescs[0];
        Uint32 numVtxBufs = 1;
        vtxBufDesc.slot = 0;
        vtxBufDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
        vtxBufDesc.instance_step_rate = 0;

        if (description.vertexLayout == VertexLayout::TwoDimensional)
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
        else if (description.vertexLayout == VertexLayout::ThreeDimensional)
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
        else
        {
            vertexAttribs[0] = {};
            vertexAttribs[0].location = 0;
            vertexAttribs[0].buffer_slot = 0;
            vertexAttribs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
            vertexAttribs[0].offset = static_cast<Uint32>(offsetof(SkinnedVertex3D, x));

            vertexAttribs[1] = {};
            vertexAttribs[1].location = 1;
            vertexAttribs[1].buffer_slot = 0;
            vertexAttribs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
            vertexAttribs[1].offset = static_cast<Uint32>(offsetof(SkinnedVertex3D, nx));

            vertexAttribs[2] = {};
            vertexAttribs[2].location = 2;
            vertexAttribs[2].buffer_slot = 0;
            vertexAttribs[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
            vertexAttribs[2].offset = static_cast<Uint32>(offsetof(SkinnedVertex3D, u));

            // One colour per draw, from slot 1: the draw is a single instance that starts at it.
            vertexAttribs[3] = {};
            vertexAttribs[3].location = 3;
            vertexAttribs[3].buffer_slot = 1;
            vertexAttribs[3].format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM;
            vertexAttribs[3].offset = 0;
            vtxBufDescs[1].slot = 1;
            vtxBufDescs[1].pitch = sizeof(std::uint32_t);
            vtxBufDescs[1].input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE;
            vtxBufDescs[1].instance_step_rate = 0;
            numVtxBufs = 2;

            vertexAttribs[4] = {};
            vertexAttribs[4].location = 4;
            vertexAttribs[4].buffer_slot = 0;
            vertexAttribs[4].format = SDL_GPU_VERTEXELEMENTFORMAT_INT;
            vertexAttribs[4].offset = static_cast<Uint32>(offsetof(SkinnedVertex3D, positionBoneIndex));

            vertexAttribs[5] = {};
            vertexAttribs[5].location = 5;
            vertexAttribs[5].buffer_slot = 0;
            vertexAttribs[5].format = SDL_GPU_VERTEXELEMENTFORMAT_INT;
            vertexAttribs[5].offset = static_cast<Uint32>(offsetof(SkinnedVertex3D, normalBoneIndex));

            vtxBufDesc.pitch = sizeof(SkinnedVertex3D);
            numAttribs = 6;

            // The weighted vertex starts with the same fields.
            if (description.vertexLayout == VertexLayout::SkinnedWeighted ||
                description.vertexLayout == VertexLayout::SkinnedWeightedShadow)
            {
                vertexAttribs[6] = {};
                vertexAttribs[6].location = 6;
                vertexAttribs[6].buffer_slot = 0;
                vertexAttribs[6].format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4;
                vertexAttribs[6].offset = static_cast<Uint32>(offsetof(SkinnedWeightedVertex3D, joints));

                vertexAttribs[7] = {};
                vertexAttribs[7].location = 7;
                vertexAttribs[7].buffer_slot = 0;
                vertexAttribs[7].format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM;
                vertexAttribs[7].offset = static_cast<Uint32>(offsetof(SkinnedWeightedVertex3D, weights));

                vertexAttribs[8] = {};
                vertexAttribs[8].location = 8;
                vertexAttribs[8].buffer_slot = 0;
                vertexAttribs[8].format = SDL_GPU_VERTEXELEMENTFORMAT_SHORT4_NORM;
                vertexAttribs[8].offset = static_cast<Uint32>(offsetof(SkinnedWeightedVertex3D, tangent));

                vtxBufDesc.pitch = sizeof(SkinnedWeightedVertex3D);
                numAttribs = 9;
            }
        }

        SDL_GPUVertexInputState vtxInputState{};
        vtxInputState.vertex_buffer_descriptions = vtxBufDescs;
        vtxInputState.num_vertex_buffers = numVtxBufs;
        vtxInputState.vertex_attributes = vertexAttribs;
        vtxInputState.num_vertex_attributes = numAttribs;

        SDL_GPUGraphicsPipelineTargetInfo targetInfo{};
        targetInfo.color_target_descriptions = &colorTargetDesc;
        targetInfo.num_color_targets = 1;
        // Enable depth-stencil target so pipelines match
        // the render pass that now includes a depth buffer.
        targetInfo.has_depth_stencil_target = true;
        targetInfo.depth_stencil_format = mu::SdlGpuDepthFormat;

        // Back-face culling as the descriptor asks: on for opaque 3D geometry (depth write ON),
        // off for transparent/glow passes (depth write OFF) and in every 2D descriptor.
        SDL_GPURasterizerState rasterState{};
        rasterState.fill_mode = SDL_GPU_FILLMODE_FILL;
        rasterState.cull_mode = description.cullFaceEnabled ? SDL_GPU_CULLMODE_BACK : SDL_GPU_CULLMODE_NONE;
        rasterState.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

        SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.vertex_shader =
            description.vertexLayout == VertexLayout::Skinned                 ? s_vertShaderSkinned
            : description.vertexLayout == VertexLayout::SkinnedWeighted       ? s_vertShaderSkinnedWeighted
            : description.vertexLayout == VertexLayout::SkinnedShadow         ? s_vertShaderSkinnedShadow
            : description.vertexLayout == VertexLayout::SkinnedWeightedShadow ? s_vertShaderSkinnedWeightedShadow
                                                                              : s_vertShader2D;
        pipelineInfo.fragment_shader = GetFragmentShader(description.fragmentShader);
        pipelineInfo.vertex_input_state = vtxInputState;
        pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
        pipelineInfo.rasterizer_state = rasterState;
        pipelineInfo.depth_stencil_state = MakeDepthStencilState(description);
        pipelineInfo.target_info = targetInfo;
        pipelineInfo.props = 0;

        pipelineInfo.multisample_state.sample_count =
            mu::GetSdlGpuSampleCount(s_sampleCount).value_or(SDL_GPU_SAMPLECOUNT_1);
        pipelineInfo.multisample_state.enable_alpha_to_coverage = description.alphaToCoverage;

        result.pipeline = SDL_CreateGPUGraphicsPipeline(s_device, &pipelineInfo);
        if (!result.pipeline)
        {
            result.error = SDL_GetError();
            if (result.error.empty())
            {
                result.error = "SDL reported no pipeline error";
            }
        }
        return result;
    }

    static void LogPipelineFailure(const char* driverName, const PipelineBuildResult& result, bool required)
    {
        const PipelineBuildDescription& description = result.description;
        const char* layoutName = description.vertexLayout == VertexLayout::TwoDimensional     ? "2D"
                                 : description.vertexLayout == VertexLayout::ThreeDimensional ? "3D"
                                                                                              : "skinned";
        const char* vertexShaderName =
            description.vertexLayout == VertexLayout::Skinned                 ? "skinned_textured.vert"
            : description.vertexLayout == VertexLayout::SkinnedWeighted       ? "skinned_weighted.vert"
            : description.vertexLayout == VertexLayout::SkinnedShadow         ? "skinned_shadow.vert"
            : description.vertexLayout == VertexLayout::SkinnedWeightedShadow ? "skinned_weighted_shadow.vert"
                                                                              : "basic_textured.vert";
        const char* fragmentShaderName = GetFragmentShaderName(description.fragmentShader);
        const char* cullMode = description.cullFaceEnabled ? "back" : "none";
        if (required)
        {
            mu::log::Get("render")->error(
                "SDL_gpu -- required pipeline failed: driver={} set={} layout={} vertex_shader={} fragment_shader={} "
                "blend={} index={} depth_test={} depth_write={} cull={} error={}",
                driverName, description.setName, layoutName, vertexShaderName, fragmentShaderName,
                description.blendModeName, description.blendIndex, description.depthTestEnabled,
                description.depthWriteEnabled, cullMode, result.error);
            return;
        }

        mu::log::Get("render")->warn(
            "SDL_gpu -- optional pipeline failed: driver={} set={} layout={} vertex_shader={} fragment_shader={} "
            "blend={} index={} depth_test={} depth_write={} cull={} error={}",
            driverName, description.setName, layoutName, vertexShaderName, fragmentShaderName,
            description.blendModeName, description.blendIndex, description.depthTestEnabled,
            description.depthWriteEnabled, cullMode, result.error);
    }

    // -----------------------------------------------------------------------
    // Builds every blend variant. The selected 3D, 2D depth-off, and skinned sets
    // are required; unused 2D depth-on remains optional.
    // -----------------------------------------------------------------------
    [[nodiscard]] static bool CreatePipelines()
    {
        // Blend mode table matching this renderer's blend pipeline.
        // Indices match BlendMode enum cast to int; index 8 = disabled.
        //
        // SDL_GPUBlendFactor values (INVALID=0, ZERO=1, ONE=2, SRC_COLOR=3,
        //   ONE_MINUS_SRC_COLOR=4, DST_COLOR=5, ONE_MINUS_DST_COLOR=6,
        //   SRC_ALPHA=7, ONE_MINUS_SRC_ALPHA=8)
        //
        // Modes: Alpha=0, Additive=1, Subtract=2, InverseColor=3,
        //        Mixed=4, LightMap=5, Glow=6, Luminance=7, Disabled=8

        // Alpha is coverage, which only an offscreen capture keeps (a render target RmlUi
        // composites premultiplied): the alpha-style modes accumulate it over, the additive and
        // multiplying ones leave it, so their colour lands on whatever is behind the image as it
        // would have on screen.
        struct BlendEntry
        {
            const char* name;
            SDL_GPUBlendFactor src;
            SDL_GPUBlendFactor dst;
            bool enableBlend;
            SDL_GPUBlendFactor srcAlpha = SDL_GPU_BLENDFACTOR_ZERO;
            SDL_GPUBlendFactor dstAlpha = SDL_GPU_BLENDFACTOR_ONE;
        };

        const BlendEntry table[k_PipelineCount] = {
            // Alpha
            {"alpha", SDL_GPU_BLENDFACTOR_SRC_ALPHA, SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, true,
             SDL_GPU_BLENDFACTOR_ONE, SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA},
            // Additive
            {"additive", SDL_GPU_BLENDFACTOR_SRC_ALPHA, SDL_GPU_BLENDFACTOR_ONE, true},
            // Subtract
            {"subtract", SDL_GPU_BLENDFACTOR_ZERO, SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR, true},
            // InverseColor
            {"inverse-color", SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_COLOR, SDL_GPU_BLENDFACTOR_ZERO, true},
            // Mixed
            {"mixed", SDL_GPU_BLENDFACTOR_ONE, SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, true,
             SDL_GPU_BLENDFACTOR_ONE, SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA},
            // LightMap
            {"light-map", SDL_GPU_BLENDFACTOR_ZERO, SDL_GPU_BLENDFACTOR_SRC_COLOR, true},
            // Glow
            {"glow", SDL_GPU_BLENDFACTOR_ONE, SDL_GPU_BLENDFACTOR_ONE, true},
            // Luminance
            {"luminance", SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR, SDL_GPU_BLENDFACTOR_ONE, true},
            // Disabled (index 8, no blend)
            {"disabled", SDL_GPU_BLENDFACTOR_ONE, SDL_GPU_BLENDFACTOR_ZERO, false, SDL_GPU_BLENDFACTOR_ONE,
             SDL_GPU_BLENDFACTOR_ZERO},
        };

        const char* deviceDriver = SDL_GetGPUDeviceDriver(s_device);
        const char* driverName = deviceDriver ? deviceDriver : "unknown";
        int requiredFailureCount = 0;
        std::string firstRequiredError;

        const auto buildRequiredPipeline = [&](SDL_GPUGraphicsPipeline*& destination,
                                               SDL_GPUColorTargetBlendState blendState,
                                               const PipelineBuildDescription& description)
        {
            const PipelineBuildResult result = BuildBlendPipeline(blendState, description);
            destination = result.pipeline;
            if (destination)
            {
                return;
            }

            ++requiredFailureCount;
            if (firstRequiredError.empty())
            {
                firstRequiredError = result.error;
            }
            LogPipelineFailure(driverName, result, true);
        };

        const auto buildOptionalPipeline = [&](SDL_GPUGraphicsPipeline*& destination,
                                               SDL_GPUColorTargetBlendState blendState,
                                               const PipelineBuildDescription& description)
        {
            const PipelineBuildResult result = BuildBlendPipeline(blendState, description);
            destination = result.pipeline;
            if (!destination)
            {
                LogPipelineFailure(driverName, result, false);
            }
        };

        // A Classic set is required; an optional family is skipped without its fragment shader.
        const auto buildWorldSet = [&](const WorldPipelineSetSpec& spec, SDL_GPUColorTargetBlendState blendState,
                                       int blendIndex, const char* blendName)
        {
            const bool required = spec.family == WorldSetFamily::Classic;
            if (!required && !GetFragmentShader(spec.fragmentShader))
            {
                return;
            }
            const auto index = static_cast<std::size_t>(blendIndex);
            for (const WorldStateSpec& state : kWorldStates)
            {
                const std::string setName = std::string(spec.name) + "-" + state.suffix;
                const PipelineBuildDescription description{setName.c_str(),
                                                           blendName,
                                                           blendIndex,
                                                           state.depthTestEnabled,
                                                           state.depthWriteEnabled,
                                                           spec.vertexLayout,
                                                           state.cullFaceEnabled,
                                                           spec.fragmentShader};
                SDL_GPUGraphicsPipeline*& destination = (spec.set->*state.states)[index];
                if (required)
                {
                    buildRequiredPipeline(destination, blendState, description);
                }
                else
                {
                    buildOptionalPipeline(destination, blendState, description);
                }
            }
        };

        for (int i = 0; i < k_PipelineCount; ++i)
        {
            SDL_GPUColorTargetBlendState blendState{};
            blendState.src_color_blendfactor = table[i].src;
            blendState.dst_color_blendfactor = table[i].dst;
            blendState.color_blend_op = SDL_GPU_BLENDOP_ADD;
            blendState.src_alpha_blendfactor = table[i].srcAlpha;
            blendState.dst_alpha_blendfactor = table[i].dstAlpha;
            blendState.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
            blendState.enable_blend = table[i].enableBlend;

            buildOptionalPipeline(s_pipelines2D[i], blendState,
                                  {"2d-depth-on", table[i].name, i, true, true, VertexLayout::TwoDimensional, false});
            buildRequiredPipeline(
                s_pipelines2DDepthOff[i], blendState,
                {"2d-depth-off", table[i].name, i, false, false, VertexLayout::TwoDimensional, false});
            for (const WorldPipelineSetSpec& spec : WorldPipelineSets())
            {
                buildWorldSet(spec, blendState, i, table[i].name);
            }

            if (s_vertShaderSkinnedShadow)
            {
                buildOptionalPipeline(
                    s_pipelinesSkinnedShadow[i], blendState,
                    {"skinned-shadow", table[i].name, i, true, false, VertexLayout::SkinnedShadow, false});
                buildOptionalPipeline(
                    s_pipelinesSkinnedShadowDepthOff[i], blendState,
                    {"skinned-shadow-depth-off", table[i].name, i, false, false, VertexLayout::SkinnedShadow, false});
            }
            if (s_vertShaderSkinnedWeightedShadow)
            {
                buildOptionalPipeline(s_pipelinesSkinnedWeightedShadow[i], blendState,
                                      {"skinned-weighted-shadow", table[i].name, i, true, false,
                                       VertexLayout::SkinnedWeightedShadow, false});
                buildOptionalPipeline(s_pipelinesSkinnedWeightedShadowDepthOff[i], blendState,
                                      {"skinned-weighted-shadow-depth-off", table[i].name, i, false, false,
                                       VertexLayout::SkinnedWeightedShadow, false});
            }
        }

        s_enhancedLightingAvailable = IsWorldFamilyComplete(WorldSetFamily::EnhancedLighting);
        if (s_enhancedLightingAvailable)
        {
            mu::log::Get("render")->info("SDL_gpu -- Enhanced lighting pipelines ready for driver {}", driverName);
        }
        else
        {
            mu::log::Get("render")->warn("SDL_gpu -- Enhanced lighting unavailable for driver {}; Classic only",
                                         driverName);
            DestroyEnhancedPipelines();
        }

        if (requiredFailureCount != 0)
        {
            mu::log::Get("render")->error("SDL_gpu -- {} required pipelines failed for driver {}; first error: {}",
                                          requiredFailureCount, driverName, firstRequiredError);
            return false;
        }

        return true;
    }

    static void DestroyEnhancedPipelines()
    {
        ReleaseWorldFamily(WorldSetFamily::EnhancedLighting);
        s_enhancedLightingAvailable = false;
    }

    static void DestroyPipelines()
    {
        DestroySingleSamplePipelines();
        s_pipelinesMultisampled = false;
        DestroyEnhancedPipelines();
        ReleaseWorldFamily(WorldSetFamily::Classic);
        for (SDL_GPUGraphicsPipeline** set :
             {s_pipelines2D, s_pipelines2DDepthOff, s_pipelinesSkinnedShadow, s_pipelinesSkinnedShadowDepthOff,
              s_pipelinesSkinnedWeightedShadow, s_pipelinesSkinnedWeightedShadowDepthOff})
        {
            for (int i = 0; i < k_PipelineCount; ++i)
            {
                if (set[i])
                {
                    SDL_ReleaseGPUGraphicsPipeline(s_device, set[i]);
                    set[i] = nullptr;
                }
            }
        }
        DestroyAlphaCoveragePipelines();
        s_postProcessing.DestroyPipelines();
    }

    // -----------------------------------------------------------------------
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

        // Skip if the existing depth texture already matches the requested size
        // and the frame's sample count.
        if (s_depthTexture && s_depthW == width && s_depthH == height && s_depthSampleCount == s_sampleCount)
        {
            return true;
        }

        // Release old depth texture if resizing.
        if (s_depthTexture)
        {
            SDL_ReleaseGPUTexture(s_device, s_depthTexture);
            s_depthTexture = nullptr;
        }

        const auto depthInfo = mu::GetSdlGpuDepthTextureInfo(
            {width, height}, mu::GetSdlGpuSampleCount(s_sampleCount).value_or(SDL_GPU_SAMPLECOUNT_1));
        s_depthTexture = depthInfo ? SDL_CreateGPUTexture(s_device, &*depthInfo) : nullptr;
        if (!s_depthTexture)
        {
            mu::log::Get("render")->error("SDL_gpu -- depth texture creation failed ({}x{}): {}", width, height,
                                          SDL_GetError());
            s_depthW = 0u;
            s_depthH = 0u;
            return false;
        }

        s_depthW = width;
        s_depthH = height;
        s_depthSampleCount = s_sampleCount;
        return true;
    }

    // -----------------------------------------------------------------------
    // Multisample anti-aliasing
    // -----------------------------------------------------------------------

    // Counts usable for both attachment formats of the main pass.
    [[nodiscard]] static Render::Antialiasing::SupportedSampleCounts QuerySupportedSampleCounts()
    {
        const SDL_GPUTextureFormat colorFormat = SDL_GetGPUSwapchainTextureFormat(s_device, s_window);
        const auto supports = [colorFormat](SDL_GPUSampleCount sampleCount)
        {
            return SDL_GPUTextureSupportsSampleCount(s_device, colorFormat, sampleCount) &&
                   SDL_GPUTextureSupportsSampleCount(s_device, mu::SdlGpuDepthFormat, sampleCount);
        };
        return {supports(SDL_GPU_SAMPLECOUNT_2), supports(SDL_GPU_SAMPLECOUNT_4), supports(SDL_GPU_SAMPLECOUNT_8)};
    }

    // The sample count the setting gets on this GPU.
    [[nodiscard]] static int SelectFrameSampleCount()
    {
#ifdef _EDITOR
        // The ImGui overlay draws in the main pass and the object thumbnails
        // replay recorded draws into one-sample targets; both need one-sample
        // pipelines, so editor builds stay at one sample.
        return Render::Antialiasing::OffSampleCount;
#else
        return Render::Antialiasing::SelectSampleCount(s_requestedSampleCount, s_supportedSampleCounts);
#endif
    }

    static void LogSampleCount(const char* when)
    {
        const int highest = Render::Antialiasing::HighestSupportedSampleCount(s_supportedSampleCounts);
        if (s_sampleCount != Render::Antialiasing::OffSampleCount)
        {
            mu::log::Get("render")->info(
                "Anti-aliasing {}: {}x multisampling ({}x requested; this GPU supports up to {}x)", when, s_sampleCount,
                s_requestedSampleCount, highest);
            return;
        }
        if (s_requestedSampleCount == Render::Antialiasing::OffSampleCount)
        {
            mu::log::Get("render")->info("Anti-aliasing {}: off (this GPU supports up to {}x)", when, highest);
            return;
        }
#ifdef _EDITOR
        constexpr const char* reason = "editor builds render with one sample";
#else
        constexpr const char* reason = "multisampling is unavailable";
#endif
        mu::log::Get("render")->info("Anti-aliasing {}: off ({}x requested; {})", when, s_requestedSampleCount, reason);
    }

    // Builds every pipeline for s_sampleCount; when that fails multisampled,
    // drops to one sample and builds again.
    [[nodiscard]] static bool CreatePipelinesForSampleCount()
    {
        if (!CreatePipelines())
        {
            if (s_sampleCount == Render::Antialiasing::OffSampleCount)
            {
                return false;
            }

            mu::log::Get("render")->error("SDL_gpu -- {}x multisampled pipelines failed; anti-aliasing turned off",
                                          s_sampleCount);
            DestroyPipelines();
            s_sampleCount = Render::Antialiasing::OffSampleCount;
            if (!CreatePipelines())
            {
                return false;
            }
        }
        CreateAlphaCoveragePipelines();
        CreatePostProcessingPipelines();
        if (!CreateSingleSamplePipelines())
        {
            // A half-built twin set would drop or mis-pair draws: run without multisampling
            // until another sample count is requested.
            mu::log::Get("render")->error(
                "SDL_gpu -- single-sample pipelines incomplete at {}x; anti-aliasing turned off", s_sampleCount);
            DestroyPipelines();
            s_sampleCount = Render::Antialiasing::OffSampleCount;
            s_requestedSampleCount = Render::Antialiasing::OffSampleCount;
            if (!CreatePipelines())
            {
                return false;
            }
            CreatePostProcessingPipelines();
        }
        s_pipelinesMultisampled = s_sampleCount != Render::Antialiasing::OffSampleCount;
        return true;
    }

    // Every pipeline slot CreatePipelines() fills, in one fixed order.
    [[nodiscard]] static std::vector<SDL_GPUGraphicsPipeline**> ReplayPipelineSlots()
    {
        std::vector<SDL_GPUGraphicsPipeline**> slots;
        for (SDL_GPUGraphicsPipeline** set :
             {s_pipelines2D, s_pipelines2DDepthOff, s_pipelinesSkinnedShadow, s_pipelinesSkinnedShadowDepthOff,
              s_pipelinesSkinnedWeightedShadow, s_pipelinesSkinnedWeightedShadowDepthOff})
        {
            for (int i = 0; i < k_PipelineCount; ++i)
            {
                slots.push_back(&set[i]);
            }
        }
        for (const WorldPipelineSetSpec& spec : WorldPipelineSets())
        {
            for (const WorldStateSpec& state : kWorldStates)
            {
                for (SDL_GPUGraphicsPipeline*& pipeline : spec.set->*state.states)
                {
                    slots.push_back(&pipeline);
                }
            }
        }
        return slots;
    }

    // Builds the single-sample twin of every multisampled pipeline (s_singleSamplePipelines).
    // False, with no twins left, when any twin failed; true with one sample (none needed).
    [[nodiscard]] static bool CreateSingleSamplePipelines()
    {
        if (s_sampleCount == Render::Antialiasing::OffSampleCount)
        {
            return true;
        }

        const std::vector<SDL_GPUGraphicsPipeline**> slots = ReplayPipelineSlots();
        std::vector<SDL_GPUGraphicsPipeline*> multisampled;
        multisampled.reserve(slots.size());
        for (SDL_GPUGraphicsPipeline** slot : slots)
        {
            multisampled.push_back(*slot);
            *slot = nullptr;
        }
        const bool enhancedLightingAvailable = s_enhancedLightingAvailable;
        const int sampleCount = s_sampleCount;

        s_sampleCount = Render::Antialiasing::OffSampleCount;
        const bool created = CreatePipelines();
        s_sampleCount = sampleCount;
        for (std::size_t i = 0; i < slots.size(); ++i)
        {
            if (multisampled[i] && *slots[i])
            {
                s_singleSamplePipelines[multisampled[i]] = *slots[i];
            }
            else if (*slots[i])
            {
                SDL_ReleaseGPUGraphicsPipeline(s_device, *slots[i]);
            }
            *slots[i] = multisampled[i];
        }
        s_enhancedLightingAvailable = enhancedLightingAvailable;

        for (const AlphaCoveragePipeline& entry : s_alphaCoveragePipelines)
        {
            if (!entry.variant)
            {
                continue;
            }
            multisampled.push_back(entry.variant);
            const auto twin = s_singleSamplePipelines.find(*entry.opaque);
            if (twin != s_singleSamplePipelines.end())
            {
                s_singleSamplePipelines[entry.variant] = twin->second;
            }
        }
        if (!created ||
            !Render::SingleSampleTwinsComplete<SDL_GPUGraphicsPipeline>(multisampled, s_singleSamplePipelines))
        {
            DestroySingleSamplePipelines();
            return false;
        }
        mu::log::Get("render")->info("SDL_gpu -- {} single-sample pipelines for offscreen and UI passes",
                                     s_singleSamplePipelines.size());
        return true;
    }

    static void DestroySingleSamplePipelines()
    {
        std::unordered_set<SDL_GPUGraphicsPipeline*> released;
        for (const auto& [multisampled, twin] : s_singleSamplePipelines)
        {
            if (twin && released.insert(twin).second)
            {
                SDL_ReleaseGPUGraphicsPipeline(s_device, twin);
            }
        }
        s_singleSamplePipelines.clear();
    }

    // Optional: without them every frame stays unprocessed.
    static void CreatePostProcessingPipelines()
    {
        const mu::SdlGpuPostShaders shaders{s_vertShaderPostFullscreen, s_fragShaderBloomDown, s_fragShaderBloomUp,
                                            s_fragShaderComposite};
        const bool created =
            s_postProcessing.CreatePipelines(s_device, shaders, SDL_GetGPUSwapchainTextureFormat(s_device, s_window),
                                             mu::GetSdlGpuSampleCount(s_sampleCount).value_or(SDL_GPU_SAMPLECOUNT_1));
        mu::log::Get("render")->info("SDL_gpu -- post-processing {} at {}x", created ? "available" : "unavailable",
                                     s_sampleCount);
    }

    // Alpha-to-coverage variants for s_sampleCount; none with one sample.
    // Optional: a variant that fails leaves its draws on the opaque pipeline.
    static void CreateAlphaCoveragePipelines()
    {
        DestroyAlphaCoveragePipelines();
        if (s_sampleCount == Render::Antialiasing::OffSampleCount)
        {
            return;
        }

        SDL_GPUColorTargetBlendState opaque{};
        opaque.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
        opaque.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
        opaque.color_blend_op = SDL_GPU_BLENDOP_ADD;
        opaque.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
        opaque.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
        opaque.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
        opaque.enable_blend = false;

        const char* deviceDriver = SDL_GetGPUDeviceDriver(s_device);
        int built = 0;
        int wanted = 0;
        for (const WorldPipelineSetSpec& spec : WorldPipelineSets())
        {
            for (const WorldStateSpec& state : kWorldStates)
            {
                WorldPipelines::States& states = spec.set->*state.states;
                // Only the depth-writing states draw cutouts; a set that was not built has nothing to replace.
                if (!state.depthWriteEnabled || !states[k_PipelineDisabled])
                {
                    continue;
                }
                ++wanted;
                const std::string setName = std::string(spec.name) + "-" + state.suffix + "-alpha-coverage";
                const PipelineBuildResult result =
                    BuildBlendPipeline(opaque, {setName.c_str(), "disabled", k_PipelineDisabled, state.depthTestEnabled,
                                                state.depthWriteEnabled, spec.vertexLayout, state.cullFaceEnabled,
                                                spec.fragmentShader, true});
                if (!result.pipeline)
                {
                    LogPipelineFailure(deviceDriver ? deviceDriver : "unknown", result, false);
                    continue;
                }
                ++built;
                s_alphaCoveragePipelines.push_back(
                    {&states[k_PipelineDisabled], &states[k_PipelineAlpha], result.pipeline});
            }
        }
        s_alphaCoverageDrawLogged = false;
        mu::log::Get("render")->info("SDL_gpu -- alpha-to-coverage: {} of {} pipelines at {}x", built, wanted,
                                     s_sampleCount);
    }

    static void DestroyAlphaCoveragePipelines()
    {
        for (const AlphaCoveragePipeline& entry : s_alphaCoveragePipelines)
        {
            SDL_ReleaseGPUGraphicsPipeline(s_device, entry.variant);
        }
        s_alphaCoveragePipelines.clear();
    }

    static void ReleaseMultisampleColorTexture()
    {
        if (s_multisampleColorTexture)
        {
            SDL_ReleaseGPUTexture(s_device, s_multisampleColorTexture);
            s_multisampleColorTexture = nullptr;
        }
        s_multisampleColorInfo = {};
        ReleaseSingleSampleDepthTexture();
    }

    // The multisampled colour target for this frame's swapchain format, size
    // and s_sampleCount; recreated when any of them changed.
    [[nodiscard]] static bool EnsureMultisampleColorTexture()
    {
        const auto info = mu::GetSdlGpuMultisampleColorTextureInfo(
            SDL_GetGPUSwapchainTextureFormat(s_device, s_window), {s_swapW, s_swapH},
            mu::GetSdlGpuSampleCount(s_sampleCount).value_or(SDL_GPU_SAMPLECOUNT_1));
        if (!info)
        {
            ReleaseMultisampleColorTexture();
            return false;
        }
        if (s_multisampleColorTexture && s_multisampleColorInfo.format == info->format &&
            s_multisampleColorInfo.width == info->width && s_multisampleColorInfo.height == info->height &&
            s_multisampleColorInfo.sample_count == info->sample_count)
        {
            return true;
        }

        ReleaseMultisampleColorTexture();
        s_multisampleColorTexture = SDL_CreateGPUTexture(s_device, &*info);
        if (!s_multisampleColorTexture)
        {
            mu::log::Get("render")->error("SDL_gpu -- {}x multisampled colour target creation failed ({}x{}): {}",
                                          s_sampleCount, info->width, info->height, SDL_GetError());
            return false;
        }
        s_multisampleColorInfo = *info;
        return true;
    }

    // Rebuilds the pipelines for another sample count. The depth buffer and the
    // multisampled colour target follow at the next BeginFrame. Releasing
    // pipelines and textures a submitted frame still uses is safe: SDL defers
    // their destruction until that frame has finished.
    static void RebuildForSampleCount(int sampleCount)
    {
        const char* driverName = SDL_GetGPUDeviceDriver(s_device);
        if (!LoadShaders(driverName))
        {
            mu::log::Get("render")->error("SDL_gpu -- shader loading failed; anti-aliasing stays at {}x",
                                          s_sampleCount);
            ReleaseShaders();
            return;
        }

        DestroyPipelines();
        s_sampleCount = sampleCount;
        if (!CreatePipelinesForSampleCount())
        {
            mu::log::Get("render")->error("SDL_gpu -- pipeline rebuild failed for {}x anti-aliasing", s_sampleCount);
        }
        ReleaseShaders();
        ReleaseMultisampleColorTexture();
    }

    static void ApplyPendingSampleCount()
    {
        if (!s_sampleCountChangePending)
        {
            return;
        }
        s_sampleCountChangePending = false;

        const int sampleCount = SelectFrameSampleCount();
        if (sampleCount != s_sampleCount)
        {
            RebuildForSampleCount(sampleCount);
        }
        LogSampleCount("switched");
    }

    // Must match the swapchain's format: every replay pipeline is built against it. If that format
    // can't be sampled, no target is made and frames fall back to the swapchain directly.
    static bool CreateOrResizeMainColorTarget(Uint32 width, Uint32 height)
    {
        if (width == 0 || height == 0)
        {
            return false;
        }

        const SDL_GPUTextureFormat format = SDL_GetGPUSwapchainTextureFormat(s_device, s_window);
        if (s_mainColorTarget && s_mainColorW == width && s_mainColorH == height && s_mainColorFormat == format)
        {
            return true;
        }

        if (s_mainColorTarget)
        {
            SDL_ReleaseGPUTexture(s_device, s_mainColorTarget);
            s_mainColorTarget = nullptr;
        }
        s_mainColorW = 0u;
        s_mainColorH = 0u;
        s_mainColorFormat = SDL_GPU_TEXTUREFORMAT_INVALID;

        constexpr SDL_GPUTextureUsageFlags usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
        if (format == SDL_GPU_TEXTUREFORMAT_INVALID ||
            !SDL_GPUTextureSupportsFormat(s_device, format, SDL_GPU_TEXTURETYPE_2D, usage))
        {
            return false;
        }

        SDL_GPUTextureCreateInfo colorInfo{};
        colorInfo.type = SDL_GPU_TEXTURETYPE_2D;
        colorInfo.format = format;
        colorInfo.width = width;
        colorInfo.height = height;
        colorInfo.layer_count_or_depth = 1;
        colorInfo.num_levels = 1;
        colorInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
        colorInfo.usage = usage;

        s_mainColorTarget = SDL_CreateGPUTexture(s_device, &colorInfo);
        if (!s_mainColorTarget)
        {
            mu::log::Get("render")->error("SDL_gpu -- main color target creation failed ({}x{}): {}", width, height,
                                          SDL_GetError());
            return false;
        }

        s_mainColorW = width;
        s_mainColorH = height;
        s_mainColorFormat = format;
        return true;
    }

    // Depth buffer for one-shot offscreen captures (see BeginOffscreenCapture). Same
    // shape as CreateOrResizeDepthTexture but keeps its own texture, since it's
    // sized for a small thumbnail, not the swapchain.
    static bool EnsureOffscreenDepthTexture(Uint32 width, Uint32 height)
    {
        if (width == 0 || height == 0)
        {
            return false;
        }
        if (s_offscreenDepthTexture && s_offscreenDepthW == width && s_offscreenDepthH == height)
        {
            return true;
        }
        if (s_offscreenDepthTexture)
        {
            SDL_ReleaseGPUTexture(s_device, s_offscreenDepthTexture);
            s_offscreenDepthTexture = nullptr;
        }

        SDL_GPUTextureCreateInfo depthInfo{};
        depthInfo.type = SDL_GPU_TEXTURETYPE_2D;
        depthInfo.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
        depthInfo.width = width;
        depthInfo.height = height;
        depthInfo.layer_count_or_depth = 1;
        depthInfo.num_levels = 1;
        depthInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;

        s_offscreenDepthTexture = SDL_CreateGPUTexture(s_device, &depthInfo);
        if (!s_offscreenDepthTexture)
        {
            mu::log::Get("render")->error("SDL_gpu -- offscreen capture depth texture creation failed ({}x{}): {}",
                                          width, height, SDL_GetError());
            s_offscreenDepthW = 0u;
            s_offscreenDepthH = 0u;
            return false;
        }

        s_offscreenDepthW = width;
        s_offscreenDepthH = height;
        return true;
    }

    // Replays each pending offscreen capture's recorded command range (see
    // BeginOffscreenCapture) into its own dedicated render pass, then marks those
    // commands consumed so the main pass later skips them instead of drawing them
    // a second time into the swapchain. Called from EndFrame() after the vertex
    // buffer upload, before the main render pass begins.
    void ProcessPendingOffscreenCaptures(bool boneDataReady)
    {
        if (s_pendingOffscreenCaptures.empty())
        {
            return;
        }

        for (const auto& capture : s_pendingOffscreenCaptures)
        {
            const auto textureIt = s_textureMap.find(capture.textureId);
            if (textureIt == s_textureMap.end())
            {
                continue;
            }
            // A render target brings its own depth and clears to transparent, so a document
            // composites it over its own background; a one-shot capture keeps the shared depth
            // and the editor's opaque backdrop.
            const auto targetDepth = s_renderTargetDepth.find(capture.textureId);
            const bool isRenderTarget = targetDepth != s_renderTargetDepth.end();
            if (!isRenderTarget && !EnsureOffscreenDepthTexture(capture.width, capture.height))
            {
                continue;
            }

            SDL_GPUColorTargetInfo colorTarget{};
            colorTarget.texture = static_cast<SDL_GPUTexture*>(textureIt->second);
            colorTarget.clear_color = isRenderTarget ? SDL_FColor{0.0f, 0.0f, 0.0f, 0.0f}
                                                     : SDL_FColor{0.10f, 0.10f, 0.12f, 1.0f};
            colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
            colorTarget.store_op = SDL_GPU_STOREOP_STORE;

            SDL_GPUDepthStencilTargetInfo depthTarget{};
            depthTarget.texture = isRenderTarget ? targetDepth->second : s_offscreenDepthTexture;
            depthTarget.clear_depth = 1.0f;
            depthTarget.load_op = SDL_GPU_LOADOP_CLEAR;
            depthTarget.store_op = SDL_GPU_STOREOP_DONT_CARE;
            depthTarget.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
            depthTarget.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
            depthTarget.cycle = true;

            s_renderPass = SDL_BeginGPURenderPass(s_cmdBuf, &colorTarget, 1, &depthTarget);
            if (!s_renderPass)
            {
                mu::log::Get("render")->warn("SDL_gpu -- offscreen capture render pass failed: {}", SDL_GetError());
                continue;
            }

            const SDL_GPUViewport viewport{0.0f, 0.0f, static_cast<float>(capture.width),
                                           static_cast<float>(capture.height), 0.0f, 1.0f};
            SDL_SetGPUViewport(s_renderPass, &viewport);
            const SDL_Rect scissor{0, 0, static_cast<int>(capture.width), static_cast<int>(capture.height)};
            SDL_SetGPUScissor(s_renderPass, &scissor);
            PushSceneLightingUniforms();

            s_singleSamplePass = true;
            Render::SdlGpuReplayState replayState;
            for (std::size_t i = capture.startCmd; i < capture.endCmd && i < s_renderCmds.size(); ++i)
            {
                RenderCmd& cmd = s_renderCmds[i];
                const bool isGeometryDraw = cmd.type == RenderCmdType::DrawTriangles ||
                                           cmd.type == RenderCmdType::DrawSkinnedTriangles ||
                                           cmd.type == RenderCmdType::DrawIndexedQuads ||
                                           cmd.type == RenderCmdType::DrawIndexedStrip ||
                                           cmd.type == RenderCmdType::DrawTriangles2D;
                // Consumed either way: nothing recorded inside a capture belongs on the frame, and a
                // viewport or scissor left to the main pass would apply to whatever it draws next.
                cmd.consumedByOffscreenCapture = true;
                if (!isGeometryDraw)
                {
                    continue; // SetViewport/SetScissor - the capture sets its own viewport
                }
                ReplayDrawCommand(cmd, cmd.pipeline, boneDataReady, scissor, replayState);
            }
            s_singleSamplePass = false;

            SDL_EndGPURenderPass(s_renderPass);
            s_renderPass = nullptr;
        }

        s_pendingOffscreenCaptures.clear();
    }

    // -----------------------------------------------------------------------
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
            mu::log::Get("render")->error("SDL_gpu -- fog transfer buffer creation failed: {}", SDL_GetError());
            return false;
        }

        SDL_GPUBufferCreateInfo bufInfo{};
        bufInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
        bufInfo.size = sizeof(FogUniform);
        s_fogUniformBuf = SDL_CreateGPUBuffer(s_device, &bufInfo);
        if (!s_fogUniformBuf)
        {
            mu::log::Get("render")->error("SDL_gpu -- fog uniform GPU buffer creation failed: {}", SDL_GetError());
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
        return EnsureVertexBufferCapacity(k_InitialVertexBufferSize);
    }

    [[nodiscard]] static bool EnsureVertexBufferCapacity(Uint32 requiredSize)
    {
        if (s_vtxCapacity >= requiredSize)
        {
            return true;
        }

        Uint32 newCapacity = s_vtxCapacity > 0u ? s_vtxCapacity : k_InitialVertexBufferSize;
        while (newCapacity < requiredSize)
        {
            if (newCapacity > std::numeric_limits<Uint32>::max() / 2u)
            {
                newCapacity = requiredSize;
                break;
            }
            newCapacity *= 2u;
        }

        SDL_GPUTransferBufferCreateInfo tbInfo{};
        tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        tbInfo.size = newCapacity;
        SDL_GPUTransferBuffer* newTransferBuffer = SDL_CreateGPUTransferBuffer(s_device, &tbInfo);
        if (!newTransferBuffer)
        {
            mu::log::Get("render")->error("SDL_gpu -- vertex transfer buffer creation failed: {}", SDL_GetError());
            return false;
        }

        SDL_GPUBufferCreateInfo bufInfo{};
        bufInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        bufInfo.size = newCapacity;
        SDL_GPUBuffer* newGpuBuffer = SDL_CreateGPUBuffer(s_device, &bufInfo);
        if (!newGpuBuffer)
        {
            mu::log::Get("render")->error("SDL_gpu -- vertex GPU buffer creation failed: {}", SDL_GetError());
            SDL_ReleaseGPUTransferBuffer(s_device, newTransferBuffer);
            return false;
        }

        if (s_vtxGpuBuf)
            SDL_ReleaseGPUBuffer(s_device, s_vtxGpuBuf);
        if (s_vtxTransferBuf)
            SDL_ReleaseGPUTransferBuffer(s_device, s_vtxTransferBuf);

        s_vtxGpuBuf = newGpuBuffer;
        s_vtxTransferBuf = newTransferBuffer;
        s_vtxCapacity = newCapacity;
        mu::log::Get("render")->info("SDL_gpu -- vertex buffers resized to {} bytes", newCapacity);
        return true;
    }

    [[nodiscard]] static bool EnsureBoneBufferCapacity(Uint32 requiredSize)
    {
        if (s_boneCapacity >= requiredSize)
        {
            return true;
        }

        Uint32 newCapacity = s_boneCapacity > 0u ? s_boneCapacity : k_InitialBoneBufferSize;
        while (newCapacity < requiredSize)
        {
            if (newCapacity > std::numeric_limits<Uint32>::max() / 2u)
            {
                newCapacity = requiredSize;
                break;
            }
            newCapacity *= 2u;
        }

        SDL_GPUTransferBufferCreateInfo transferInfo{};
        transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        transferInfo.size = newCapacity;
        SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(s_device, &transferInfo);
        if (!transferBuffer)
        {
            return false;
        }

        SDL_GPUBufferCreateInfo bufferInfo{};
        bufferInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
        bufferInfo.size = newCapacity;
        SDL_GPUBuffer* gpuBuffer = SDL_CreateGPUBuffer(s_device, &bufferInfo);
        if (!gpuBuffer)
        {
            SDL_ReleaseGPUTransferBuffer(s_device, transferBuffer);
            return false;
        }

        if (s_boneGpuBuf)
        {
            SDL_ReleaseGPUBuffer(s_device, s_boneGpuBuf);
        }
        if (s_boneTransferBuf)
        {
            SDL_ReleaseGPUTransferBuffer(s_device, s_boneTransferBuf);
        }
        s_boneGpuBuf = gpuBuffer;
        s_boneTransferBuf = transferBuffer;
        s_boneCapacity = newCapacity;
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
        s_vtxCapacity = 0u;
        s_vtxScratch.clear();
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

    static void DestroyBoneBuffers()
    {
        if (s_boneGpuBuf)
        {
            SDL_ReleaseGPUBuffer(s_device, s_boneGpuBuf);
            s_boneGpuBuf = nullptr;
        }
        if (s_boneTransferBuf)
        {
            SDL_ReleaseGPUTransferBuffer(s_device, s_boneTransferBuf);
            s_boneTransferBuf = nullptr;
        }
        s_boneCapacity = 0u;
        s_boneRowScratch.clear();
    }

    static void DestroyTerrainHeightBuffers()
    {
        if (s_terrainHeightGpuBuf)
        {
            SDL_ReleaseGPUBuffer(s_device, s_terrainHeightGpuBuf);
            s_terrainHeightGpuBuf = nullptr;
        }
        if (s_terrainHeightTransferBuf)
        {
            SDL_ReleaseGPUTransferBuffer(s_device, s_terrainHeightTransferBuf);
            s_terrainHeightTransferBuf = nullptr;
        }
        s_terrainHeightVersion.reset();
        s_terrainHeightUploadPending = false;
    }

    static void DestroySkinnedMeshCache()
    {
        for (const SkinnedMeshBlock& block : s_skinnedMeshBlocks)
        {
            SDL_ReleaseGPUBuffer(s_device, block.buffer);
        }
        s_skinnedMeshBlocks.clear();
        s_skinnedMeshes.clear();
        s_skinnedMeshStaging.clear();
        s_pendingSkinnedMeshUploads.clear();
        s_deferredSkinnedMeshFrees.clear();
        if (s_skinnedMeshTransferBuf)
        {
            SDL_ReleaseGPUTransferBuffer(s_device, s_skinnedMeshTransferBuf);
            s_skinnedMeshTransferBuf = nullptr;
        }
        s_skinnedMeshTransferCapacity = 0u;
    }

    [[nodiscard]] static bool CreateQuadIndexBuffer()
    {
        std::vector<Uint16> indices(static_cast<std::size_t>(k_MaxQuads) * 6);
        Render::Topology::FillQuadIndices(indices);

        const Uint32 idxByteSize = static_cast<Uint32>(indices.size() * sizeof(Uint16));

        // Upload via a one-time command buffer.
        SDL_GPUCommandBuffer* uploadCmd = SDL_AcquireGPUCommandBuffer(s_device);
        if (!uploadCmd)
        {
            mu::log::Get("render")->error("SDL_gpu -- failed to acquire command buffer for index upload");
            return false;
        }

        SDL_GPUTransferBufferCreateInfo tbInfo{};
        tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        tbInfo.size = idxByteSize;
        SDL_GPUTransferBuffer* idxTransfer = SDL_CreateGPUTransferBuffer(s_device, &tbInfo);
        if (!idxTransfer)
        {
            mu::log::Get("render")->error("SDL_gpu -- quad index transfer buffer creation failed: {}", SDL_GetError());
            SDL_CancelGPUCommandBuffer(uploadCmd);
            return false;
        }

        void* pMapped = SDL_MapGPUTransferBuffer(s_device, idxTransfer, false);
        if (!pMapped)
        {
            mu::log::Get("render")->error("SDL_gpu -- failed to map quad index transfer buffer");
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
            mu::log::Get("render")->error("SDL_gpu -- quad index GPU buffer creation failed: {}", SDL_GetError());
            SDL_ReleaseGPUTransferBuffer(s_device, idxTransfer);
            SDL_CancelGPUCommandBuffer(uploadCmd);
            return false;
        }

        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmd);
        if (!copyPass)
        {
            mu::log::Get("render")->error("SDL_gpu -- failed to begin copy pass for index upload");
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
            mu::log::Get("render")->error("SDL_gpu -- strip index transfer buffer creation failed: {}", SDL_GetError());
            return false;
        }

        SDL_GPUBufferCreateInfo bufInfo{};
        bufInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
        bufInfo.size = byteSize;
        s_stripIdxBuf = SDL_CreateGPUBuffer(s_device, &bufInfo);
        if (!s_stripIdxBuf)
        {
            mu::log::Get("render")->error("SDL_gpu -- strip index GPU buffer creation failed: {}", SDL_GetError());
            SDL_ReleaseGPUTransferBuffer(s_device, s_stripIdxTransfer);
            s_stripIdxTransfer = nullptr;
            return false;
        }

        s_stripIdxCapacity = requiredIndices;
        return true;
    }

    // A 1×1 RGBA8 texture of one pixel (`pixel`, little-endian RGBA), uploaded at once; nullptr
    // (logged) when it cannot be created.
    [[nodiscard]] static SDL_GPUTexture* CreateSolidTexture(Uint32 pixel, const char* what)
    {
        SDL_GPUTextureCreateInfo texInfo{};
        texInfo.type = SDL_GPU_TEXTURETYPE_2D;
        texInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        texInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
        texInfo.width = 1;
        texInfo.height = 1;
        texInfo.layer_count_or_depth = 1;
        texInfo.num_levels = 1;
        texInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;

        SDL_GPUTexture* texture = SDL_CreateGPUTexture(s_device, &texInfo);
        if (!texture)
        {
            mu::log::Get("render")->error("SDL_gpu -- {} texture creation failed: {}", what, SDL_GetError());
            return nullptr;
        }

        constexpr Uint32 kPixelByteSize = 4u;

        SDL_GPUTransferBufferCreateInfo tbInfo{};
        tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        tbInfo.size = kPixelByteSize;
        SDL_GPUTransferBuffer* pixelTransfer = SDL_CreateGPUTransferBuffer(s_device, &tbInfo);
        if (!pixelTransfer)
        {
            mu::log::Get("render")->error("SDL_gpu -- {} texture transfer buffer failed: {}", what, SDL_GetError());
            SDL_ReleaseGPUTexture(s_device, texture);
            return nullptr;
        }

        void* pMapped = SDL_MapGPUTransferBuffer(s_device, pixelTransfer, false);
        if (!pMapped)
        {
            SDL_ReleaseGPUTransferBuffer(s_device, pixelTransfer);
            SDL_ReleaseGPUTexture(s_device, texture);
            return nullptr;
        }
        std::memcpy(pMapped, &pixel, kPixelByteSize);
        SDL_UnmapGPUTransferBuffer(s_device, pixelTransfer);

        SDL_GPUCommandBuffer* uploadCmd = SDL_AcquireGPUCommandBuffer(s_device);
        if (!uploadCmd)
        {
            SDL_ReleaseGPUTransferBuffer(s_device, pixelTransfer);
            SDL_ReleaseGPUTexture(s_device, texture);
            return nullptr;
        }

        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmd);
        if (!copyPass)
        {
            SDL_ReleaseGPUTransferBuffer(s_device, pixelTransfer);
            SDL_ReleaseGPUTexture(s_device, texture);
            SDL_CancelGPUCommandBuffer(uploadCmd);
            return nullptr;
        }

        SDL_GPUTextureTransferInfo texTransferInfo{};
        texTransferInfo.transfer_buffer = pixelTransfer;
        texTransferInfo.offset = 0;
        texTransferInfo.pixels_per_row = 1;
        texTransferInfo.rows_per_layer = 1;

        SDL_GPUTextureRegion texRegion{};
        texRegion.texture = texture;
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
        return texture;
    }

    // The default textures: white (textureId 0) and the flat normal map of skinned_lit.frag.
    [[nodiscard]] static bool CreateWhiteTexture()
    {
        constexpr Uint32 kWhitePixel = 0xFFFFFFFFu; // RGBA = 255,255,255,255
        s_whiteTexture = CreateSolidTexture(kWhitePixel, "white");
        if (!s_whiteTexture)
        {
            return false;
        }
        s_flatNormalTexture = CreateSolidTexture(k_FlatNormalPixel, "flat normal map");
        if (!s_flatNormalTexture)
        {
            SDL_ReleaseGPUTexture(s_device, s_whiteTexture);
            s_whiteTexture = nullptr;
            return false;
        }

        // Register the white texture as textureId=0 (sentinel for "no texture").
        RegisterTexture(0u, s_whiteTexture);

        return true;
    }
};

// ---------------------------------------------------------------------------
// GetRenderer / InitSDLGpuRenderer / ShutdownSDLGpuRenderer:
// MU_USE_OPENGL_BACKEND removed — SDL_gpu is the only backend.
// ---------------------------------------------------------------------------

[[nodiscard]] IMuRenderer& GetRenderer()
{
    static MuRendererSDLGpu s_instance;
    return s_instance;
}

Render::Renderer::BitmapGpuResources::Result UploadBitmapPixels(std::span<const std::uint8_t> pixels, int width,
                                                                int height, SDL_GPUFilter filter,
                                                                SDL_GPUSamplerAddressMode wrap,
                                                                std::span<const Render::Textures::MipLevel> mips,
                                                                bool generateMips)
{
    return s_bitmapGpuResources.Upload(pixels, width, height, filter, wrap, mips, generateMips);
}

void ReleaseBitmapTexture(SDL_GPUDevice* device, SDL_GPUTexture* texture)
{
    SDL_ReleaseGPUTexture(device, texture);
}

const char* BitmapUploadDriverName()
{
    return s_device ? SDL_GetGPUDeviceDriver(s_device) : "no device";
}

// C++ linkage entry points for MuMain.cpp (no class forward declaration needed).
[[nodiscard]] bool InitSDLGpuRenderer(void* pNativeWindow, std::string_view fontFamily,
                                      std::string_view renderBackend, float normalPointSize,
                                      float bigPointSize, float fixedPointSize)
{
    return MuRendererSDLGpu::Init(pNativeWindow, fontFamily, renderBackend, normalPointSize, bigPointSize,
                                  fixedPointSize);
}

void WaitForSDLGpuIdle()
{
    if (s_device && !SDL_WaitForGPUIdle(s_device))
    {
        mu::log::Get("render")->error("SDL_gpu -- failed to wait for idle before resource teardown: {}",
                                      SDL_GetError());
    }
}

void ShutdownSDLGpuRenderer()
{
    MuRendererSDLGpu::Shutdown();
}

} // namespace mu
