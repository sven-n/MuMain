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
// SDL3 is a required project dependency, so these includes are unconditional.
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL.h>
#if __has_include(<SDL3_ttf/SDL_ttf.h>)
#include <SDL3_ttf/SDL_ttf.h>
#define MU_HAS_SDL_TTF 1
#else
#define MU_HAS_SDL_TTF 0
#endif

#include "DrawCommandHistory.h"
#include "MuRenderer.h"
#include "QuadTopology.h"
#include "SdlGpuPixelFormat.h"
#include "SdlGpuReplayState.h"
#include "SdlGpuValidation.h"
#include "Core/Platform/BundledFonts.h"
#include "Core/Utilities/FrameProfiler.h"
#include "Core/Utilities/Log/MuLogger.h"
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
    float textureCoordinateParameters[4]{};
};
static_assert(sizeof(SkinningVertexUniforms) == 192, "SkinningVertexUniforms must be 192 bytes");

} // anonymous namespace

namespace mu
{

// ---------------------------------------------------------------------------
// Story 4.3.2 (AC-10): Fog uniform buffer struct — in mu:: namespace so that
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
    float pad0;                    // offset 12
    float fogStart;                // offset 16
    float fogEnd;                  // offset 20
    float fogPadding[2];           // offset 24
    std::array<float, 4> fogColor; // offset 32 (float4 register alignment)
}; // total: 48 bytes

static_assert(offsetof(FogUniform, fogEnabled) == 0, "FogUniform std140 layout");
static_assert(offsetof(FogUniform, alphaDiscardEnabled) == 4, "FogUniform std140 layout");
static_assert(offsetof(FogUniform, alphaThreshold) == 8, "FogUniform std140 layout");
static_assert(offsetof(FogUniform, pad0) == 12, "FogUniform std140 layout");
static_assert(offsetof(FogUniform, fogStart) == 16, "FogUniform std140 layout");
static_assert(offsetof(FogUniform, fogEnd) == 20, "FogUniform std140 layout");
static_assert(offsetof(FogUniform, fogPadding) == 24, "FogUniform HLSL cbuffer layout");
static_assert(offsetof(FogUniform, fogColor) == 32, "FogUniform HLSL float4 register alignment");
static_assert(sizeof(FogUniform) == 48, "FogUniform must be 48 bytes (HLSL cbuffer)");

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
    const char* basePath = SDL_GetBasePath();
    if (basePath != nullptr)
    {
        return (std::filesystem::path(basePath) / "shaders" / filename).string();
    }
    return (std::filesystem::path("shaders") / filename).string();
}

// ---------------------------------------------------------------------------
// Story 4.3.2 (AC-6): GetShaderFormat
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
// Story 4.3.2 (AC-6): LoadShaderBlob
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
static SDL_Window* s_window = nullptr;

// Per-frame command buffer and render pass handles (valid between BeginFrame/EndFrame).
static SDL_GPUCommandBuffer* s_cmdBuf = nullptr;
static SDL_GPURenderPass* s_renderPass = nullptr;
static SDL_GPUTexture* s_swapchainTexture = nullptr;

// Swapchain dimensions in physical pixels (from SDL_AcquireGPUSwapchainTexture).
// Used for viewport calculations — distinct from logical window size on HiDPI/Retina.
static Uint32 s_swapW = 0u;
static Uint32 s_swapH = 0u;
static std::uint32_t s_pendingFrameCaptureTextureId = 0u;
static FrameReadbackState s_frameReadbackState;
static SDL_GPUTexture* s_frameReadbackTexture = nullptr;

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

    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
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
static mu::RendererStats s_lastFrameStats;
static std::chrono::steady_clock::time_point s_frameBeginTime;
static std::chrono::steady_clock::time_point s_renderReplayBeginTime;
static std::chrono::steady_clock::time_point s_submitTime;

[[nodiscard]] static bool IsFrameTimingEnabled()
{
    return s_frameTimingEnabled || s_statsEnabled;
}

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
static SDL_GPUGraphicsPipeline* s_pipelines3DNoCull[k_PipelineCount] = {};
static SDL_GPUGraphicsPipeline* s_pipelines3DDepthOff[k_PipelineCount] = {};
static SDL_GPUGraphicsPipeline* s_pipelines3DDepthReadOnly[k_PipelineCount] = {};
static SDL_GPUGraphicsPipeline* s_pipelinesSkinned[k_PipelineCount] = {};
static SDL_GPUGraphicsPipeline* s_pipelinesSkinnedNoCull[k_PipelineCount] = {};
static SDL_GPUGraphicsPipeline* s_pipelinesSkinnedDepthOff[k_PipelineCount] = {};
static SDL_GPUGraphicsPipeline* s_pipelinesSkinnedDepthReadOnly[k_PipelineCount] = {};

// Story 4.3.2 (AC-7): Single pre-frame vertex upload.
// Draws accumulate in growable CPU memory before one GPU upload.
static SDL_GPUTransferBuffer* s_vtxTransferBuf = nullptr;
static SDL_GPUBuffer* s_vtxGpuBuf = nullptr;
static Uint32 s_vtxCapacity = 0u;
static Uint32 s_vtxOffset = 0u;
static std::vector<Uint8> s_vtxScratch;

// Per-frame packed float4 bone rows. Three rows represent one affine 3x4 bone transform.
static SDL_GPUTransferBuffer* s_boneTransferBuf = nullptr;
static SDL_GPUBuffer* s_boneGpuBuf = nullptr;
static Uint32 s_boneCapacity = 0u;
static std::vector<float> s_boneRowScratch;
static const float* s_lastBonePalette = nullptr;
static std::size_t s_lastBonePaletteSize = 0u;
static std::uint32_t s_lastBonePaletteVersion = 0u;
static Uint32 s_lastBonePaletteRowOffset = 0u;

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
#ifdef _EDITOR
    EditorOverlay,
#endif
    DrawTriangles, // non-indexed 3D (Vertex3D)
    DrawSkinnedTriangles,
    DrawIndexedQuads, // indexed 2D or 3D with static quad index buffer
    DrawIndexedStrip, // indexed 3D with per-frame strip indices (Vertex3D)
    DrawTriangles2D,  // Story 7.9.8: non-indexed 2D triangles (Vertex2D) for text atlas
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
    SkinningVertexUniforms skinningVu{};
    FogUniform fogUniform;
    SDL_GPUViewport viewport; // for SetViewport only
    SDL_Rect scissor;         // for SetScissor only
    BlendMode blendMode{};
    bool blendEnabled{};
    bool depthTestEnabled{};
    bool depthMaskEnabled{};
    bool cullFaceEnabled{};
};

static std::vector<RenderCmd> s_renderCmds;
static constexpr std::size_t kNoDrawCommand = std::numeric_limits<std::size_t>::max();
static Render::DrawCommandHistory s_previousDrawCommands;

[[nodiscard]] static bool IsDrawCommand(RenderCmdType type)
{
#ifdef _EDITOR
    if (type == RenderCmdType::EditorOverlay)
    {
        return true;
    }
#endif
    return type != RenderCmdType::SetViewport && type != RenderCmdType::SetScissor;
}

[[nodiscard]] static bool IsUnsafeInvalidatedDrawCommand(RenderCmdType type)
{
#ifdef _EDITOR
    if (type == RenderCmdType::EditorOverlay)
    {
        return false;
    }
#endif
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
    if (previous.blendEnabled != command.blendEnabled || previous.blendMode != command.blendMode)
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
    const bool geometryCanMerge = Render::Topology::CanMergeTriangleDraws(
        previous.vtxOffset, previous.vtxCount, command.vtxOffset, vertexStride);
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
#ifdef _EDITOR
void QueueEditorRenderCommand()
{
    if (!s_frameActive)
    {
        return;
    }

    RenderCmd command{};
    command.type = RenderCmdType::EditorOverlay;
    s_renderCmds.push_back(command);
}

static void PreparePendingEditorDrawData(SDL_GPUCommandBuffer* commandBuffer)
{
    const bool editorRenderPending = std::any_of(s_renderCmds.begin(), s_renderCmds.end(), [](const RenderCmd& command)
                                                 { return command.type == RenderCmdType::EditorOverlay; });
    if (!editorRenderPending)
    {
        return;
    }

    g_MuEditorCore.PrepareDrawData(commandBuffer);
}
#endif

static bool BindReplayPipeline(const RenderCmd& command, Render::SdlGpuReplayState& state, const SDL_Rect& scissor)
{
    if (!command.pipeline)
        return false;
    if (!state.SelectPipeline(command.pipeline))
        return true;

    SDL_BindGPUGraphicsPipeline(s_renderPass, command.pipeline);
    SDL_SetGPUScissor(s_renderPass, &scissor);
    ++s_dbgPipelineBindsThisFrame;
    return true;
}

static void PushReplayVertexUniforms(const void* data, std::size_t size, Render::SdlGpuReplayState& state)
{
    const auto* bytes = static_cast<const std::byte*>(data);
    if (!state.SelectVertexUniforms({bytes, size}))
        return;

    SDL_PushGPUVertexUniformData(s_cmdBuf, 0, data, static_cast<Uint32>(size));
    ++s_dbgVertexUniformPushesThisFrame;
}

static void PushReplayFragmentUniforms(const FogUniform& uniforms, Render::SdlGpuReplayState& state)
{
    const auto bytes = std::as_bytes(std::span{&uniforms, 1u});
    if (!state.SelectFragmentUniforms(bytes))
        return;

    SDL_PushGPUFragmentUniformData(s_cmdBuf, 0, &uniforms, sizeof(uniforms));
    ++s_dbgFragmentUniformPushesThisFrame;
}

static void BindReplayFragmentSampler(const RenderCmd& command, Render::SdlGpuReplayState& state)
{
    if (!state.SelectFragmentSampler(command.texture, command.sampler))
        return;

    const SDL_GPUTextureSamplerBinding samplerBinding{command.texture, command.sampler};
    SDL_BindGPUFragmentSamplers(s_renderPass, 0, &samplerBinding, 1);
    ++s_dbgSamplerBindsThisFrame;
}

static void BindReplayIndexBuffer(SDL_GPUBuffer* buffer, Uint32 offset, SDL_GPUIndexElementSize elementSize,
                                  Render::SdlGpuReplayState& state)
{
    if (!state.SelectIndexBuffer(buffer, offset, elementSize))
        return;

    const SDL_GPUBufferBinding indexBinding{buffer, offset};
    SDL_BindGPUIndexBuffer(s_renderPass, &indexBinding, elementSize);
}

static void ReplayDrawCommand(const RenderCmd& command, bool boneDataReady, const SDL_Rect& scissor,
                              Render::SdlGpuReplayState& state)
{
    const bool skinned = command.type == RenderCmdType::DrawSkinnedTriangles;
    if (!command.texture || !command.sampler || (skinned && (!boneDataReady || !s_boneGpuBuf)) ||
        !BindReplayPipeline(command, state, scissor))
    {
        return;
    }

    if (skinned)
        PushReplayVertexUniforms(&command.skinningVu, sizeof(command.skinningVu), state);
    else
        PushReplayVertexUniforms(&command.vu, sizeof(command.vu), state);

    const SDL_GPUBufferBinding vertexBinding{s_vtxGpuBuf, command.vtxOffset};
    SDL_BindGPUVertexBuffers(s_renderPass, 0, &vertexBinding, 1);

    if (skinned && state.SelectVertexStorageBuffer(s_boneGpuBuf))
    {
        SDL_GPUBuffer* boneBuffer = s_boneGpuBuf;
        SDL_BindGPUVertexStorageBuffers(s_renderPass, 0, &boneBuffer, 1);
    }

    if (command.type == RenderCmdType::DrawIndexedQuads)
        BindReplayIndexBuffer(s_quadIdxBuf, 0, SDL_GPU_INDEXELEMENTSIZE_16BIT, state);
    else if (command.type == RenderCmdType::DrawIndexedStrip)
        BindReplayIndexBuffer(s_stripIdxBuf, command.stripIdxOffset, SDL_GPU_INDEXELEMENTSIZE_16BIT, state);

    BindReplayFragmentSampler(command, state);
    PushReplayFragmentUniforms(command.fogUniform, state);

    if (command.type == RenderCmdType::DrawIndexedQuads || command.type == RenderCmdType::DrawIndexedStrip)
        SDL_DrawGPUIndexedPrimitives(s_renderPass, command.idxCount, 1, 0, 0, 0);
    else
        SDL_DrawGPUPrimitives(s_renderPass, command.vtxCount, 1, 0, 0);
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

// Shader handles for basic_textured (2D path) and basic_colored — released after pipeline creation.
static SDL_GPUShader* s_vertShader2D = nullptr;      // basic_textured.vert
static SDL_GPUShader* s_fragShaderTex = nullptr;     // basic_textured.frag
static SDL_GPUShader* s_vertShader2DCol = nullptr;   // basic_colored.vert
static SDL_GPUShader* s_fragShaderCol = nullptr;     // basic_colored.frag
static SDL_GPUShader* s_vertShaderShadow = nullptr;  // shadow_volume.vert
static SDL_GPUShader* s_vertShaderSkinned = nullptr; // skinned_textured.vert

// Story 7.9.7 (AC-3): Depth buffer texture for correct 3D depth testing.
// Created in Init() at swapchain dimensions, recreated on window resize.
static SDL_GPUTexture* s_depthTexture = nullptr;
static Uint32 s_depthW = 0u;
static Uint32 s_depthH = 0u;
static SDL_FColor s_clearColor{0.0f, 0.0f, 0.0f, 1.0f};

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

#ifdef NDEBUG
inline constexpr bool kAllowSystemFontFallback = false;
#else
inline constexpr bool kAllowSystemFontFallback = true;
#endif

// F-7 fix: Cached window dimensions, updated once per frame in BeginFrame().
static int s_cachedWinW = 0;
static int s_cachedWinH = 0;

#if MU_HAS_SDL_TTF
[[nodiscard]] static std::string BundledFontPath(const char* relativePath)
{
    const char* basePath = SDL_GetBasePath();
    const std::filesystem::path executableDir = basePath ? basePath : ".";
    return (executableDir / relativePath).string();
}

[[nodiscard]] static std::string FindDeveloperFontPath()
{
#ifndef NDEBUG
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
#endif

    return {};
}

static void CloseTtfFont(TTF_Font*& font)
{
    if (!font)
        return;
    TTF_CloseFont(font);
    font = nullptr;
}

[[nodiscard]] static TTF_Font* OpenTtfFontRole(std::string_view family, std::string_view role,
                                               const char* relativePath, float pointSize)
{
    const std::string packagedPath = BundledFontPath(relativePath);
    if (TTF_Font* font = TTF_OpenFont(packagedPath.c_str(), pointSize))
    {
        mu::log::Get("render")->info("SDL_ttf -- bundled family='{}' role='{}' path='{}'", family, role,
                                      packagedPath);
        return font;
    }

    mu::log::Get("render")->error("SDL_ttf -- bundled family='{}' role='{}' path='{}' failed: {}", family, role,
                                   packagedPath, SDL_GetError());
    if (!kAllowSystemFontFallback)
        return nullptr;

    const std::string fallbackPath = FindDeveloperFontPath();
    if (fallbackPath.empty())
        return nullptr;

    mu::log::Get("render")->warn(
        "SDL_ttf -- NON-PARITY developer font fallback family='{}' role='{}' path='{}'", family, role,
        fallbackPath);
    TTF_Font* fallback = TTF_OpenFont(fallbackPath.c_str(), pointSize);
    if (!fallback)
    {
        mu::log::Get("render")->error(
            "SDL_ttf -- NON-PARITY developer font fallback family='{}' role='{}' path='{}' failed: {}", family,
            role, fallbackPath, SDL_GetError());
    }
    return fallback;
}

static void WarmTtfFonts()
{
    static constexpr const char* k_WarmupGlyphs =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
        "0123456789 !@#$%^&*()-_=+[]{}|;:',.<>?/~`\"\\";
    TTF_Font* fonts[] = {s_ttfFont, s_ttfFontBold, s_ttfFontBig, s_ttfFontFixed};
    for (TTF_Font* font : fonts)
    {
        TTF_Text* warmup = TTF_CreateText(s_textEngine, font, k_WarmupGlyphs, 0);
        if (!warmup)
            continue;
        TTF_GetGPUTextDrawData(warmup);
        TTF_DestroyText(warmup);
    }
}

[[nodiscard]] static bool LoadTtfFonts(std::string_view configuredFamily)
{
    const BundledFont& family = ResolveBundledFont(configuredFamily);
    TTF_Font* normal = OpenTtfFontRole(family.family, "normal", family.regular, k_DefaultFontPtSize);
    TTF_Font* bold = OpenTtfFontRole(family.family, "bold", family.bold, k_DefaultFontPtSize);
    TTF_Font* big = OpenTtfFontRole(family.family, "big-bold", family.bold, k_BigFontPtSize);
    TTF_Font* fixed =
        OpenTtfFontRole(kBundledFixedFont.family, "fixed", kBundledFixedFont.regular, k_DefaultFontPtSize);
    if (!normal || !bold || !big || !fixed)
    {
        CloseTtfFont(fixed);
        CloseTtfFont(big);
        CloseTtfFont(bold);
        CloseTtfFont(normal);
        return false;
    }

    CloseTtfFont(s_ttfFontFixed);
    CloseTtfFont(s_ttfFontBig);
    CloseTtfFont(s_ttfFontBold);
    CloseTtfFont(s_ttfFont);
    s_ttfFont = normal;
    s_ttfFontBold = bold;
    s_ttfFontBig = big;
    s_ttfFontFixed = fixed;
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
// Story 4.4.1 (AC-4, Task 6): SamplerRegistry — parallel to TextureRegistry.
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

void RegisterSampler(std::uint32_t id, void* pSampler)
{
    s_samplerMap[id] = pSampler;
    s_samplerCacheValid = false;
}

void UnregisterSampler(std::uint32_t id)
{
    s_samplerMap.erase(id);
    s_samplerCacheValid = false;
}

void ClearSamplerRegistry()
{
    s_samplerMap.clear();
    s_samplerCacheValid = false;
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
    [[nodiscard]] static bool Init(void* pNativeWindow, std::string_view fontFamily)
    {
        s_window = static_cast<SDL_Window*>(pNativeWindow);
        if (!s_window)
        {
            mu::log::Get("render")->error("SDL_gpu -- Init called with null window pointer");
            return false;
        }

        // Create GPU device with all supported shader formats.
        // SDL_gpu selects the platform backend automatically:
        //   Metal on macOS, Vulkan on Linux, D3D12 on Windows.
        mu::log::Get("render")->info("SDL_gpu -- validation: {}",
                                      Render::kGpuValidationEnabled ? "enabled" : "disabled");
        s_device = SDL_CreateGPUDevice(
            SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
            Render::kGpuValidationEnabled, nullptr);

        if (!s_device)
        {
            mu::log::Get("render")->error("SDL_gpu -- device creation failed: {}", SDL_GetError());
            return false;
        }

        mu::log::Get("render")->info("SDL_gpu -- device driver: {}", SDL_GetGPUDeviceDriver(s_device));

        // Claim the window for the GPU device.
        if (!SDL_ClaimWindowForGPUDevice(s_device, s_window))
        {
            mu::log::Get("render")->error("SDL_gpu -- SDL_ClaimWindowForGPUDevice failed: {}", SDL_GetError());
            SDL_DestroyGPUDevice(s_device);
            s_device = nullptr;
            return false;
        }

        // Story 4.3.2: Load real HLSL shader blobs from MU_SHADER_DIR.
        // Driver name used to select the correct blob format (SPIR-V/DXIL/MSL).
        const char* driverName = SDL_GetGPUDeviceDriver(s_device);
        if (!LoadShaders(driverName))
        {
            mu::log::Get("render")->error("SDL_gpu -- shader loading failed; cannot build pipelines");
            SDL_ReleaseWindowFromGPUDevice(s_device, s_window);
            SDL_DestroyGPUDevice(s_device);
            s_device = nullptr;
            return false;
        }

        // Build nine blend variants for five required core sets, plus the unused
        // 2D depth-on set and any skinned sets whose optional shader loaded.
        if (!CreatePipelines())
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

        // Story 4.3.2 (AC-10): Create fog uniform GPU buffer and transfer buffer.
        if (!CreateFogUniformBuffers())
        {
            mu::log::Get("render")->error("SDL_gpu -- fog uniform buffer creation failed");
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
        if (!LoadTtfFonts(fontFamily))
        {
            Shutdown();
            return false;
        }
#endif

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
        // Story 7.9.8 (AC-2): Destroy SDL_ttf resources before the GPU device.
        // Close font variants first, then default font, then engine.
        CloseTtfFont(s_ttfFontFixed);
        CloseTtfFont(s_ttfFontBig);
        CloseTtfFont(s_ttfFontBold);
        CloseTtfFont(s_ttfFont);
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

        // Release the independently owned white fallback texture.
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
        DestroyBoneBuffers();
        DestroyPipelines();
        ReleaseShaders();

        SDL_ReleaseWindowFromGPUDevice(s_device, s_window);
        SDL_DestroyGPUDevice(s_device);
        s_device = nullptr;
        s_window = nullptr;

        mu::log::Get("render")->info("SDL_gpu -- Shutdown complete");
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
        if (IsFrameTimingEnabled())
        {
            s_frameBeginTime = std::chrono::steady_clock::now();
        }

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
        s_renderCmds.clear();
        s_previousDrawCommands.fill(kNoDrawCommand);
        s_stripIdxScratch.clear();
        s_boneRowScratch.clear();
        s_lastBonePalette = nullptr;
        s_lastBonePaletteSize = 0u;
        s_lastBonePaletteVersion = 0u;
        s_lastBonePaletteRowOffset = 0u;
        s_texturesInvalidated = false; // Reset per-frame texture invalidation flag
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
        s_dbgVertexUniformPushesThisFrame = 0u;
        s_dbgFragmentUniformPushesThisFrame = 0u;
        ++s_dbgFrameCount;
        s_vtxScratch.clear();

        s_swapW = 0u;
        s_swapH = 0u;
        s_swapchainTexture = nullptr;

        if (!SDL_AcquireGPUSwapchainTexture(s_cmdBuf, s_window, &s_swapchainTexture, &s_swapW, &s_swapH))
        {
            mu::log::Get("render")->error("SDL_gpu -- SDL_AcquireGPUSwapchainTexture failed: {}", SDL_GetError());
            SDL_CancelGPUCommandBuffer(s_cmdBuf);
            s_cmdBuf = nullptr;
            FailPendingFrameReadback();
            return;
        }

        // If swapchain texture is null, window is minimized/occluded — skip frame.
        if (!s_swapchainTexture)
        {
            // Debug-level only — this happens normally when window is minimized.
            SDL_CancelGPUCommandBuffer(s_cmdBuf);
            s_cmdBuf = nullptr;
            FailPendingFrameReadback();
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
        if (!s_frameActive)
        {
            // Frame was not started (minimized window or error).
            if (s_cmdBuf)
            {
                SDL_CancelGPUCommandBuffer(s_cmdBuf);
                s_cmdBuf = nullptr;
            }
            FailPendingFrameReadback();
            return;
        }
        s_frameActive = false;

        // ---------------------------------------------------------------
        // Phase 1: Grow GPU buffers if needed, then stage recorded CPU vertices.
        // ---------------------------------------------------------------
        // ---------------------------------------------------------------
        // This happens BEFORE the render pass so the GPU reads current-
        // frame data, eliminating the 1-frame vertex delay that caused
        // streak artifacts when vertex counts varied between frames.
        // ---------------------------------------------------------------
        bool vertexDataReady = s_vtxOffset == 0u;
        if (s_vtxOffset > 0u && EnsureVertexBufferCapacity(s_vtxOffset))
        {
            void* mapped = SDL_MapGPUTransferBuffer(s_device, s_vtxTransferBuf, true);
            if (mapped)
            {
                std::memcpy(mapped, s_vtxScratch.data(), s_vtxOffset);
                SDL_UnmapGPUTransferBuffer(s_device, s_vtxTransferBuf);
                vertexDataReady = true;
            }
            else
            {
                mu::log::Get("render")->error("SDL_gpu -- failed to map vertex transfer buffer: {}", SDL_GetError());
            }
        }

        Uint32 boneDataSize = 0u;
        bool boneDataReady = s_boneRowScratch.empty();
        if (!s_boneRowScratch.empty())
        {
            const std::size_t boneBytes = s_boneRowScratch.size() * sizeof(float);
            if (boneBytes <= std::numeric_limits<Uint32>::max())
            {
                boneDataSize = static_cast<Uint32>(boneBytes);
                if (EnsureBoneBufferCapacity(boneDataSize))
                {
                    void* mapped = SDL_MapGPUTransferBuffer(s_device, s_boneTransferBuf, true);
                    if (mapped)
                    {
                        std::memcpy(mapped, s_boneRowScratch.data(), boneDataSize);
                        SDL_UnmapGPUTransferBuffer(s_device, s_boneTransferBuf);
                        boneDataReady = true;
                    }
                }
            }
        }

        bool stripIdxReady = false;
        if (!s_stripIdxScratch.empty())
        {
            // Ensure GPU-side strip index buffer is large enough BEFORE the copy pass
            // (EnsureStripIndexBuffer may release/create GPU resources).
            if (EnsureStripIndexBuffer(static_cast<Uint32>(s_stripIdxScratch.size())))
            {
                void* pIdxMapped = SDL_MapGPUTransferBuffer(s_device, s_stripIdxTransfer, true);
                if (pIdxMapped)
                {
                    const Uint32 totalIdxBytes = static_cast<Uint32>(s_stripIdxScratch.size() * sizeof(Uint16));
                    std::memcpy(pIdxMapped, s_stripIdxScratch.data(), totalIdxBytes);
                    SDL_UnmapGPUTransferBuffer(s_device, s_stripIdxTransfer);
                    stripIdxReady = true;
                }
            }
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

        if (s_vtxOffset > 0u || boneDataSize > 0u || stripIdxReady || !preparedTexUploads.empty())
        {
            SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(s_cmdBuf);
            if (copyPass)
            {
                // Copy vertex data.
                if (s_vtxOffset > 0u && vertexDataReady)
                {
                    SDL_GPUTransferBufferLocation vtxSrc{};
                    vtxSrc.transfer_buffer = s_vtxTransferBuf;
                    vtxSrc.offset = 0;

                    SDL_GPUBufferRegion vtxDst{};
                    vtxDst.buffer = s_vtxGpuBuf;
                    vtxDst.offset = 0;
                    vtxDst.size = s_vtxOffset;

                    SDL_UploadToGPUBuffer(copyPass, &vtxSrc, &vtxDst, true);
                }

                if (boneDataSize > 0u && boneDataReady)
                {
                    SDL_GPUTransferBufferLocation boneSrc{};
                    boneSrc.transfer_buffer = s_boneTransferBuf;
                    boneSrc.offset = 0;

                    SDL_GPUBufferRegion boneDst{};
                    boneDst.buffer = s_boneGpuBuf;
                    boneDst.offset = 0;
                    boneDst.size = boneDataSize;

                    SDL_UploadToGPUBuffer(copyPass, &boneSrc, &boneDst, true);
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

                    SDL_UploadToGPUBuffer(copyPass, &idxSrc, &idxDst, true);
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

        // ---------------------------------------------------------------
        // Phase 3: Render pass — replay all recorded draw commands.
        // The GPU vertex/index buffers now contain current-frame data.
        // ---------------------------------------------------------------
        SDL_GPUTextureFormat frameReadbackFormat = SDL_GPU_TEXTUREFORMAT_INVALID;
        if (s_frameReadbackState.IsPending())
        {
            frameReadbackFormat = SDL_GetGPUSwapchainTextureFormat(s_device, s_window);
            if (!CreateFrameReadbackTexture(frameReadbackFormat))
            {
                FailPendingFrameReadback();
            }
        }

        SDL_GPUTexture* reconnectCaptureTexture = nullptr;
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
                                                                            : s_swapchainTexture;
        bool renderPassCompleted = false;
        if (IsFrameTimingEnabled())
        {
            s_renderReplayBeginTime = std::chrono::steady_clock::now();
        }
#ifdef _EDITOR
        PreparePendingEditorDrawData(s_cmdBuf);
#endif
        {
            SDL_GPUColorTargetInfo colorTarget{};
            colorTarget.texture = frameColorTexture;
            colorTarget.clear_color = s_clearColor;
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
            // Track explicit state. Pipeline changes re-apply the active scissor because
            // SDL GPU backends may discard it while binding a new pipeline.
            SDL_GPUViewport s_currentViewport{0.0f, 0.0f, static_cast<float>(s_swapW), static_cast<float>(s_swapH),
                                              0.0f, 1.0f};
            SDL_Rect s_currentScissor{0, 0, static_cast<int>(s_swapW), static_cast<int>(s_swapH)};

            // Replay state and editor commands after texture invalidation, but skip
            // game draws because their deferred texture pointers may be dangling.
            Render::SdlGpuReplayState replayState;
            for (const auto& cmd : s_renderCmds)
            {
                if (s_texturesInvalidated && IsUnsafeInvalidatedDrawCommand(cmd.type))
                {
                    continue;
                }
                ++s_dbgRenderCmdsReplayedThisFrame;

                switch (cmd.type)
                {
                case RenderCmdType::SetViewport:
                {
                    s_currentViewport = cmd.viewport;
                    if (replayState.SelectViewport(s_currentViewport))
                        SDL_SetGPUViewport(s_renderPass, &s_currentViewport);
                    break;
                }

                case RenderCmdType::SetScissor:
                {
                    s_currentScissor = cmd.scissor;
                    if (replayState.SelectScissor(s_currentScissor))
                        SDL_SetGPUScissor(s_renderPass, &s_currentScissor);
                    break;
                }

#ifdef _EDITOR
                case RenderCmdType::EditorOverlay:
                {
                    g_MuEditorCore.RenderDrawData(s_cmdBuf, s_renderPass);
                    replayState.Invalidate();
                    if (replayState.SelectViewport(s_currentViewport))
                        SDL_SetGPUViewport(s_renderPass, &s_currentViewport);
                    if (replayState.SelectScissor(s_currentScissor))
                        SDL_SetGPUScissor(s_renderPass, &s_currentScissor);
                    break;
                }
#endif

                case RenderCmdType::DrawTriangles:
                case RenderCmdType::DrawSkinnedTriangles:
                case RenderCmdType::DrawIndexedQuads:
                case RenderCmdType::DrawIndexedStrip:
                case RenderCmdType::DrawTriangles2D:
                {
                    ReplayDrawCommand(cmd, boneDataReady, s_currentScissor, replayState);
                    break;
                }
                } // switch
            } // for

            SDL_EndGPURenderPass(s_renderPass);
            s_renderPass = nullptr;
            renderPassCompleted = true;
        }

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
            SDL_SubmitGPUCommandBuffer(s_cmdBuf);
            s_cmdBuf = nullptr;
        }

        if (IsFrameTimingEnabled())
        {
            s_submitTime = std::chrono::steady_clock::now();
        }

        s_swapchainTexture = nullptr;

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
        if (IsFrameTimingEnabled())
        {
            const auto milliseconds = [](auto begin, auto end)
            { return std::chrono::duration<double, std::milli>(end - begin).count(); };
            s_lastFrameStats.frameMilliseconds = milliseconds(s_frameBeginTime, frameCompletedAt);
            s_lastFrameStats.replayMilliseconds = milliseconds(s_renderReplayBeginTime, s_submitTime);
            s_lastFrameStats.submitMilliseconds = milliseconds(s_submitTime, frameCompletedAt);
        }

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
                s_dbgSamplerBindsThisFrame,
                s_dbgVertexUniformPushesThisFrame, s_dbgFragmentUniformPushesThisFrame);

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

    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
    // Story 7-9-2 (AC-1): BeginScene — 3D viewport and projection setup.
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
    // Story 7-9-2 (AC-1): EndScene — restore state after 3D pass.
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
    // UI-embedded 3D previews (CUIPhotoViewer, CharMakeWin, NewUIInGameShop,
    // NewUIGoldBowmanLena, NewUIRegistrationLuckyCoin, NewUI3DRenderMng) call
    // SetRenderViewport(...) shrinks the GPU
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
    // Story 7-9-2 (AC-2): Begin2DPass — mark 2D mode for pipeline selection.
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

    void SetClearColor(float r, float g, float b, float a) override
    {
        s_clearColor = {r, g, b, a};
    }

    // -----------------------------------------------------------------------
    // Story 7-9-2 (AC-5): RenderLines — line primitive rendering.
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
    // Story 7-9-2 (AC-6): IsFrameActive — frame lifecycle query.
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

    [[nodiscard]] bool SetVSyncEnabled(bool enabled) override
    {
        if (!s_device || !s_window)
        {
            return false;
        }

        const SDL_GPUPresentMode presentMode = enabled ? SDL_GPU_PRESENTMODE_VSYNC : SDL_GPU_PRESENTMODE_IMMEDIATE;
        if (!SDL_SetGPUSwapchainParameters(s_device, s_window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, presentMode))
        {
            mu::log::Get("render")->warn("SDL_gpu -- failed to set {} present mode: {}",
                                         enabled ? "VSync" : "immediate", SDL_GetError());
            return false;
        }

        return true;
    }

    // Story 4.4.1 (AC-2, Task 6.2/6.3): GetDevice override — returns s_device.
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

    [[nodiscard]] bool ReloadTtfFonts(std::string_view fontFamily) override
    {
#if MU_HAS_SDL_TTF
        return s_textEngine && LoadTtfFonts(fontFamily);
#else
        (void)fontFamily;
        return false;
#endif
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
        cmd.blendMode = BlendMode::Alpha;
        cmd.blendEnabled = true;
        // 2D ortho projection for text — Y-up to match SDL_ttf GPU convention.
        // SDL_ttf negates Y in the vertex data (see SDL_gpu_textengine.c: "In the GPU API
        // positive y-axis is upwards so the signs of the y-coords is reversed").
        // Vertex positions are in Y-up space with the text origin at (0,0).
        // drawX/drawY offset the text to the correct screen position.
        cmd.vu.mvp =
            glm::ortho(0.0f, static_cast<float>(s_cachedWinW), 0.0f, static_cast<float>(s_cachedWinH), -1.0f, 1.0f);
        FrameProfiler::Count(FrameProfiler::Counter::BatchVertices, cmd.vtxCount);
        std::size_t& previousCommand = Render::PreviousDrawCommand(
            s_previousDrawCommands, Render::DrawCommandFamily::TextTriangles2D);
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
        const Uint32 vtxOffset = UploadVertices(vertices.data(), byteSize);
        if (vtxOffset == ~0u)
        {
            return;
        }

        // Story 4.3.2 (AC-8): RenderQuad2D uses the 2D pipeline set (Vertex2D layout).
        // Always disable depth test for 2D sprites — they must render on top of 3D
        // geometry regardless of depth buffer state. The 3D pass fills the depth buffer
        // with near values (characters close to camera) that would occlude 2D UI.
        const int pipelineIdx = GetActivePipelineIndex();
        SDL_GPUGraphicsPipeline* pipeline = GetActiveQuadPipeline(Render::Topology::QuadSpace::Screen);
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
        cmd.idxCount = drawQuads * 6;
        cmd.fogUniform = m_fogUniform;
        cmd.blendMode = m_activeBlendMode;
        cmd.blendEnabled = m_blendEnabled;
        cmd.depthTestEnabled = m_depthTestEnabled;
        cmd.depthMaskEnabled = m_depthMaskEnabled;
        cmd.cullFaceEnabled = m_cullFaceEnabled;
        // 2D ortho MVP: maps [0,W]×[0,H] to NDC, replicating gluOrtho2D.
        // GLM_FORCE_DEPTH_ZERO_TO_ONE → correct Z [0,1] for Metal/Vulkan.
        // fogStart=fogEnd=0 → range=0 → vertex shader sets fogFactor=1.0 (no fog for 2D).
        cmd.vu.mvp =
            glm::ortho(0.0f, static_cast<float>(s_cachedWinW), 0.0f, static_cast<float>(s_cachedWinH), -1.0f, 1.0f);
        FrameProfiler::Count(FrameProfiler::Counter::BatchVertices, drawQuads * 4u);
        std::size_t& previousCommand = Render::PreviousDrawCommand(
            s_previousDrawCommands, Render::DrawCommandFamily::ScreenQuads2D);
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
        if (vertices.empty() || !s_frameActive || !m_colorWriteEnabled || m_stencilTestEnabled)
        {
            return;
        }

        if (vertices.size() % 3 != 0)
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
        const Uint32 vtxOffset = UploadVertices(vertices.data(), byteSize);
        if (vtxOffset == ~0u)
        {
            return;
        }

        // Story 4.3.2 (AC-8): RenderTriangles uses the 3D pipeline set (Vertex3D layout).
        const int pipelineIdx = GetActivePipelineIndex();
        SDL_GPUGraphicsPipeline* pipeline = GetActive3DPipeline();
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
        cmd.blendMode = m_activeBlendMode;
        cmd.blendEnabled = m_blendEnabled;
        cmd.depthTestEnabled = m_depthTestEnabled;
        cmd.depthMaskEnabled = m_depthMaskEnabled;
        cmd.cullFaceEnabled = m_cullFaceEnabled;
        FrameProfiler::Count(FrameProfiler::Counter::BatchVertices, cmd.vtxCount);
        std::size_t& previousCommand =
            Render::PreviousDrawCommand(s_previousDrawCommands, Render::DrawCommandFamily::Triangles3D);
        if (MergeAdjacentTriangleCommand(cmd, previousCommand, sizeof(Vertex3D)))
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
        const Uint32 vtxOffset = UploadVertices(vertices.data(), byteSize);
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
                SDL_Log("[RENDER diag] WARNING: RenderQuad3D pipeline is null (idx=%d depth=%d)", pipelineIdx,
                        m_depthTestEnabled ? 1 : 0);
                s_dbgNullPipelineWarned = true;
            }
            return;
        }

        void* pSampler = LookupSampler(resolvedTexId);

        RenderCmd cmd{};
        cmd.type = RenderCmdType::DrawIndexedQuads;
        cmd.pipeline = pipeline;
        cmd.texture = static_cast<SDL_GPUTexture*>(pTex);
        cmd.sampler = pSampler ? static_cast<SDL_GPUSampler*>(pSampler) : s_defaultSampler;
        cmd.vtxOffset = vtxOffset;
        cmd.idxCount = drawQuads * 6u;
        cmd.vu.mvp = m_mvpMatrix;
        cmd.vu.fogStart = m_fogUniform.fogStart;
        cmd.vu.fogEnd = m_fogUniform.fogEnd;
        cmd.fogUniform = m_fogUniform;
        cmd.blendMode = m_activeBlendMode;
        cmd.blendEnabled = m_blendEnabled;
        cmd.depthTestEnabled = m_depthTestEnabled;
        cmd.depthMaskEnabled = m_depthMaskEnabled;
        cmd.cullFaceEnabled = m_cullFaceEnabled;
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

    [[nodiscard]] bool RenderSkinnedTriangles(std::span<const SkinnedVertex3D> vertices, std::uint32_t textureId,
                                              const SkinningParameters& parameters) override
    {
        if (vertices.empty() || vertices.size() % 3 != 0 || !s_frameActive || !m_colorWriteEnabled ||
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
        SDL_GPUGraphicsPipeline* pipeline =
            m_depthTestEnabled
                ? (m_depthMaskEnabled
                       ? (m_cullFaceEnabled ? s_pipelinesSkinned[pipelineIdx] : s_pipelinesSkinnedNoCull[pipelineIdx])
                       : s_pipelinesSkinnedDepthReadOnly[pipelineIdx])
                : s_pipelinesSkinnedDepthOff[pipelineIdx];
        if (!pipeline)
        {
            return false;
        }

        const Uint32 paletteRowOffset = RecordBonePalette(parameters);
        if (paletteRowOffset == ~0u)
        {
            return false;
        }

        const Uint32 byteSize = static_cast<Uint32>(vertices.size() * sizeof(SkinnedVertex3D));
        const Uint32 vtxOffset = UploadVertices(vertices.data(), byteSize);
        if (vtxOffset == ~0u)
        {
            return false;
        }

        RenderCmd cmd{};
        cmd.type = RenderCmdType::DrawSkinnedTriangles;
        cmd.pipeline = pipeline;
        cmd.texture = texture;
        void* sampler = LookupSampler(resolvedTexId);
        cmd.sampler = sampler ? static_cast<SDL_GPUSampler*>(sampler) : s_defaultSampler;
        cmd.vtxOffset = vtxOffset;
        cmd.vtxCount = static_cast<Uint32>(vertices.size());
        cmd.skinningVu.mvp = m_mvpMatrix;
        cmd.skinningVu.bodyOriginAndScale[0] = parameters.bodyOrigin[0];
        cmd.skinningVu.bodyOriginAndScale[1] = parameters.bodyOrigin[1];
        cmd.skinningVu.bodyOriginAndScale[2] = parameters.bodyOrigin[2];
        cmd.skinningVu.bodyOriginAndScale[3] = parameters.bodyScale;
        cmd.skinningVu.skinningScales[0] = parameters.boneScale;
        cmd.skinningVu.skinningScales[1] = parameters.restPoseScale;
        cmd.skinningVu.palette[0] = paletteRowOffset;
        cmd.skinningVu.palette[1] = static_cast<std::uint32_t>(parameters.boneMatrices.size() / 12);
        cmd.skinningVu.palette[2] = parameters.translate ? 1u : 0u;
        cmd.skinningVu.palette[3] = parameters.lightEnabled ? 1u : 0u;
        cmd.skinningVu.lightDirection[0] = parameters.lightDirection[0];
        cmd.skinningVu.lightDirection[1] = parameters.lightDirection[1];
        cmd.skinningVu.lightDirection[2] = parameters.lightDirection[2];
        cmd.skinningVu.fogParameters[0] = m_fogUniform.fogStart;
        cmd.skinningVu.fogParameters[1] = m_fogUniform.fogEnd;
        cmd.skinningVu.textureCoordinates[0] = static_cast<std::uint32_t>(parameters.textureCoordinates);
        cmd.skinningVu.chromeParameters[0] = parameters.chromeWave;
        cmd.skinningVu.chromeParameters[1] = parameters.chromeWave2;
        cmd.skinningVu.chromeParameters[2] = parameters.chromeLight[0];
        cmd.skinningVu.chromeParameters[3] = parameters.chromeLight[1];
        cmd.skinningVu.textureCoordinateParameters[0] = parameters.textureCoordinateOffset[0];
        cmd.skinningVu.textureCoordinateParameters[1] = parameters.textureCoordinateOffset[1];
        cmd.skinningVu.textureCoordinateParameters[2] = parameters.chromeTimeTerm;
        cmd.fogUniform = m_fogUniform;
        s_renderCmds.push_back(cmd);

        ++s_dbgDrawCallsThisFrame;
        s_dbgVtxBytesThisFrame += byteSize;
        FrameProfiler::Count(FrameProfiler::Counter::DrawCalls);
        FrameProfiler::Count(FrameProfiler::Counter::VertexBytes, byteSize);
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
        SDL_GPUGraphicsPipeline* pipeline = GetActive3DPipeline();
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
        // Story 7.9.7: Preserve alpha discard state — SetFog must NOT reset
        // alphaDiscardEnabled/alphaThreshold set by SetAlphaTest/SetAlphaFunc.
        m_fogUniform.pad0 = 0.0f;
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

    [[nodiscard]] SDL_GPUGraphicsPipeline* GetActive3DPipeline() const
    {
        const int pipelineIdx = GetActivePipelineIndex();
        if (!m_depthTestEnabled)
        {
            return s_pipelines3DDepthOff[pipelineIdx];
        }
        if (!m_depthMaskEnabled)
        {
            return s_pipelines3DDepthReadOnly[pipelineIdx];
        }
        return m_cullFaceEnabled ? s_pipelines3D[pipelineIdx] : s_pipelines3DNoCull[pipelineIdx];
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
    [[nodiscard]] static Uint32 UploadVertices(const void* pData, Uint32 byteSize)
    {
        if (!pData || byteSize == 0u)
        {
            return ~0u;
        }

        const Uint32 alignedOffset = (s_vtxOffset + 3u) & ~3u; // 4-byte alignment
        if (byteSize > std::numeric_limits<Uint32>::max() - alignedOffset)
        {
            mu::log::Get("render")->error("SDL_gpu -- vertex staging size exceeds Uint32 range");
            return ~0u;
        }

        const Uint32 requiredSize = alignedOffset + byteSize;
        s_vtxScratch.resize(requiredSize);
        std::memcpy(s_vtxScratch.data() + alignedOffset, pData, byteSize);

        s_vtxOffset = requiredSize;
        return alignedOffset;
    }

    [[nodiscard]] static Uint32 RecordBonePalette(const SkinningParameters& parameters)
    {
        const auto bones = parameters.boneMatrices;
        if (bones.empty() || bones.size() % 12 != 0)
        {
            return ~0u;
        }
        if (bones.data() == s_lastBonePalette && bones.size() == s_lastBonePaletteSize &&
            parameters.paletteVersion == s_lastBonePaletteVersion)
        {
            return s_lastBonePaletteRowOffset;
        }

        const std::size_t rowOffset = s_boneRowScratch.size() / 4;
        if (rowOffset > std::numeric_limits<Uint32>::max() ||
            bones.size() > std::numeric_limits<Uint32>::max() / sizeof(float) - s_boneRowScratch.size())
        {
            return ~0u;
        }

        s_boneRowScratch.insert(s_boneRowScratch.end(), bones.begin(), bones.end());
        s_lastBonePalette = bones.data();
        s_lastBonePaletteSize = bones.size();
        s_lastBonePaletteVersion = parameters.paletteVersion;
        s_lastBonePaletteRowOffset = static_cast<Uint32>(rowOffset);
        return s_lastBonePaletteRowOffset;
    }

    // -----------------------------------------------------------------------
    // Static resource creation/destruction helpers.
    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------
    // Story 4.3.2 (AC-2, AC-5): LoadShaders
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
        // Inputs: pos(TEXCOORD0), color(TEXCOORD1)
        // Uniform buffers: b0, space1 (ScreenSize)
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

        // Rest-pose BMD geometry: one vertex storage buffer for packed bone rows, one uniform buffer.
        s_vertShaderSkinned =
            createShader("skinned_textured", "vert", SDL_GPU_SHADERSTAGE_VERTEX, 0, 1, 1, /*fatal=*/true);
        if (!s_vertShaderSkinned)
        {
            ReleaseShaders();
            return false;
        }

        mu::log::Get("render")->info("SDL_gpu -- shaders loaded for driver: {}", driverName ? driverName : "unknown");
        return true;
    }

    // -----------------------------------------------------------------------
    // Story 4.3.2: ReleaseShaders — release all 6 shader handles.
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
    }

    // -----------------------------------------------------------------------
    // Story 4.3.2 (AC-8): BuildBlendPipeline
    // Creates one textured blend/depth/cull variant and captures SDL's immediate
    // error before another pipeline build can overwrite it.
    // -----------------------------------------------------------------------
    enum class VertexLayout
    {
        TwoDimensional,
        ThreeDimensional,
        Skinned,
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
    };

    struct PipelineBuildResult
    {
        SDL_GPUGraphicsPipeline* pipeline;
        PipelineBuildDescription description;
        std::string error;
    };

    [[nodiscard]] static PipelineBuildResult BuildBlendPipeline(SDL_GPUColorTargetBlendState blendState,
                                                                const PipelineBuildDescription& description)
    {
        PipelineBuildResult result{nullptr, description, {}};

        // Get swapchain texture format for pipeline target.
        const SDL_GPUTextureFormat swapchainFmt = SDL_GetGPUSwapchainTextureFormat(s_device, s_window);

        SDL_GPUColorTargetDescription colorTargetDesc{};
        colorTargetDesc.format = swapchainFmt;
        colorTargetDesc.blend_state = blendState;

        SDL_GPUVertexAttribute vertexAttribs[6];
        Uint32 numAttribs = 0;
        SDL_GPUVertexBufferDescription vtxBufDesc{};
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

            vertexAttribs[3] = {};
            vertexAttribs[3].location = 3;
            vertexAttribs[3].buffer_slot = 0;
            vertexAttribs[3].format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM;
            vertexAttribs[3].offset = static_cast<Uint32>(offsetof(SkinnedVertex3D, color));

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
        }

        SDL_GPUVertexInputState vtxInputState{};
        vtxInputState.vertex_buffer_descriptions = &vtxBufDesc;
        vtxInputState.num_vertex_buffers = 1;
        vtxInputState.vertex_attributes = vertexAttribs;
        vtxInputState.num_vertex_attributes = numAttribs;

        SDL_GPUDepthStencilState depthState{};
        depthState.enable_depth_test = description.depthTestEnabled;
        depthState.enable_depth_write = description.depthWriteEnabled;
        depthState.enable_stencil_test = false;
        // Depth compare selection:
        //   LESS for opaque 3D (depth write ON): strict layering, normal opaque z-sort.
        //   LESS_OR_EQUAL for depth-read-only 3D (write OFF) and all 2D: these are additive/
        //   glow overlays drawn on top of geometry at the same depth (forge-level sprites,
        //   weapon chrome/bright mesh passes, terrain texture layering). LESS would reject
        //   every equal-depth fragment before the blend stage — the classic MU OpenGL client
        //   ran these passes with GL_LEQUAL effectively by calling SetDepthFunc(GL_LEQUAL)
        //   before them, which the SDL3 backend doesn't propagate. Using LESS_OR_EQUAL on
        //   read-only variants recovers the intended behavior uniformly.
        const bool usesWorldDepth = description.vertexLayout != VertexLayout::TwoDimensional;
        const bool strictLayering = usesWorldDepth && description.depthWriteEnabled;
        depthState.compare_op = strictLayering ? SDL_GPU_COMPAREOP_LESS : SDL_GPU_COMPAREOP_LESS_OR_EQUAL;

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
        rasterState.cull_mode =
            (usesWorldDepth && description.cullFaceEnabled) ? SDL_GPU_CULLMODE_BACK : SDL_GPU_CULLMODE_NONE;
        rasterState.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

        SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.vertex_shader =
            description.vertexLayout == VertexLayout::Skinned ? s_vertShaderSkinned : s_vertShader2D;
        pipelineInfo.fragment_shader = s_fragShaderTex;
        pipelineInfo.vertex_input_state = vtxInputState;
        pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
        pipelineInfo.rasterizer_state = rasterState;
        pipelineInfo.depth_stencil_state = depthState;
        pipelineInfo.target_info = targetInfo;
        pipelineInfo.props = 0;

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
            description.vertexLayout == VertexLayout::Skinned ? "skinned_textured.vert" : "basic_textured.vert";
        constexpr const char* fragmentShaderName = "basic_textured.frag";
        const char* cullMode =
            description.vertexLayout != VertexLayout::TwoDimensional && description.cullFaceEnabled ? "back" : "none";
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
    // Story 4.3.2 (AC-8): CreatePipelines
    // Builds every blend variant. The selected 3D, 2D depth-off, and skinned sets
    // are required; unused 2D depth-on remains optional.
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
            const char* name;
            SDL_GPUBlendFactor src;
            SDL_GPUBlendFactor dst;
            bool enableBlend;
        };

        const BlendEntry table[k_PipelineCount] = {
            // Alpha
            {"alpha", SDL_GPU_BLENDFACTOR_SRC_ALPHA, SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, true},
            // Additive
            {"additive", SDL_GPU_BLENDFACTOR_SRC_ALPHA, SDL_GPU_BLENDFACTOR_ONE, true},
            // Subtract
            {"subtract", SDL_GPU_BLENDFACTOR_ZERO, SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR, true},
            // InverseColor
            {"inverse-color", SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_COLOR, SDL_GPU_BLENDFACTOR_ZERO, true},
            // Mixed
            {"mixed", SDL_GPU_BLENDFACTOR_ONE, SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, true},
            // LightMap
            {"light-map", SDL_GPU_BLENDFACTOR_ZERO, SDL_GPU_BLENDFACTOR_SRC_COLOR, true},
            // Glow
            {"glow", SDL_GPU_BLENDFACTOR_ONE, SDL_GPU_BLENDFACTOR_ONE, true},
            // Luminance
            {"luminance", SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR, SDL_GPU_BLENDFACTOR_ONE, true},
            // Disabled (index 8, no blend)
            {"disabled", SDL_GPU_BLENDFACTOR_ONE, SDL_GPU_BLENDFACTOR_ZERO, false},
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

            buildOptionalPipeline(s_pipelines2D[i], blendState,
                                  {"2d-depth-on", table[i].name, i, true, true, VertexLayout::TwoDimensional, false});
            buildRequiredPipeline(
                s_pipelines2DDepthOff[i], blendState,
                {"2d-depth-off", table[i].name, i, false, false, VertexLayout::TwoDimensional, false});
            buildRequiredPipeline(s_pipelines3D[i], blendState,
                                  {"3d-culled", table[i].name, i, true, true, VertexLayout::ThreeDimensional, true});
            buildRequiredPipeline(s_pipelines3DNoCull[i], blendState,
                                  {"3d-no-cull", table[i].name, i, true, true, VertexLayout::ThreeDimensional, false});
            buildRequiredPipeline(
                s_pipelines3DDepthOff[i], blendState,
                {"3d-depth-off", table[i].name, i, false, false, VertexLayout::ThreeDimensional, false});
            buildRequiredPipeline(
                s_pipelines3DDepthReadOnly[i], blendState,
                {"3d-depth-read-only", table[i].name, i, true, false, VertexLayout::ThreeDimensional, false});

            buildRequiredPipeline(s_pipelinesSkinned[i], blendState,
                                  {"skinned-culled", table[i].name, i, true, true, VertexLayout::Skinned, true});
            buildRequiredPipeline(s_pipelinesSkinnedNoCull[i], blendState,
                                  {"skinned-no-cull", table[i].name, i, true, true, VertexLayout::Skinned, false});
            buildRequiredPipeline(
                s_pipelinesSkinnedDepthOff[i], blendState,
                {"skinned-depth-off", table[i].name, i, false, false, VertexLayout::Skinned, false});
            buildRequiredPipeline(
                s_pipelinesSkinnedDepthReadOnly[i], blendState,
                {"skinned-depth-read-only", table[i].name, i, true, false, VertexLayout::Skinned, false});
        }

        if (requiredFailureCount != 0)
        {
            mu::log::Get("render")->error("SDL_gpu -- {} required pipelines failed for driver {}; first error: {}",
                                          requiredFailureCount, driverName, firstRequiredError);
            return false;
        }

        return true;
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
            if (s_pipelines3DNoCull[i])
            {
                SDL_ReleaseGPUGraphicsPipeline(s_device, s_pipelines3DNoCull[i]);
                s_pipelines3DNoCull[i] = nullptr;
            }
            if (s_pipelines3DDepthOff[i])
            {
                SDL_ReleaseGPUGraphicsPipeline(s_device, s_pipelines3DDepthOff[i]);
                s_pipelines3DDepthOff[i] = nullptr;
            }
            if (s_pipelines3DDepthReadOnly[i])
            {
                SDL_ReleaseGPUGraphicsPipeline(s_device, s_pipelines3DDepthReadOnly[i]);
                s_pipelines3DDepthReadOnly[i] = nullptr;
            }
            if (s_pipelinesSkinned[i])
            {
                SDL_ReleaseGPUGraphicsPipeline(s_device, s_pipelinesSkinned[i]);
                s_pipelinesSkinned[i] = nullptr;
            }
            if (s_pipelinesSkinnedNoCull[i])
            {
                SDL_ReleaseGPUGraphicsPipeline(s_device, s_pipelinesSkinnedNoCull[i]);
                s_pipelinesSkinnedNoCull[i] = nullptr;
            }
            if (s_pipelinesSkinnedDepthOff[i])
            {
                SDL_ReleaseGPUGraphicsPipeline(s_device, s_pipelinesSkinnedDepthOff[i]);
                s_pipelinesSkinnedDepthOff[i] = nullptr;
            }
            if (s_pipelinesSkinnedDepthReadOnly[i])
            {
                SDL_ReleaseGPUGraphicsPipeline(s_device, s_pipelinesSkinnedDepthReadOnly[i]);
                s_pipelinesSkinnedDepthReadOnly[i] = nullptr;
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
            mu::log::Get("render")->error("SDL_gpu -- depth texture creation failed ({}x{}): {}", width, height,
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
            mu::log::Get("render")->error("SDL_gpu -- white texture creation failed: {}", SDL_GetError());
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
            mu::log::Get("render")->error("SDL_gpu -- white texture transfer buffer failed: {}", SDL_GetError());
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
[[nodiscard]] bool InitSDLGpuRenderer(void* pNativeWindow, std::string_view fontFamily)
{
    return MuRendererSDLGpu::Init(pNativeWindow, fontFamily);
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
