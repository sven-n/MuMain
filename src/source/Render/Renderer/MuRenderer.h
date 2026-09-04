// MuRenderer.h: Rendering abstraction interface for the MU Online game client.
// Story 4.2.1 — Flow Code: VS1-RENDER-ABSTRACT-CORE
// Story 4.4.1 — Flow Code: VS1-RENDER-TEXTURE-MIGRATE (GetDevice accessor added)
//
// IMuRenderer defines the stable rendering API surface that game code calls.
// MuRendererSDLGpu.cpp implements it using the active SDL GPU backend.
//
// DESIGN CONTRACT:
//   - No OpenGL types (GLenum, GLuint, GLint, etc.) in this header.
//   - All parameters expressed in project-defined enums and plain C++ types.
//   - No #ifdef _WIN32 in game logic — cross-platform via stubs.
#pragma once

#include "FramePixelReadback.h"

#include <cstdint>
#include <functional>
#include <span>
#include <string_view>

// Story 4.4.1: Forward declaration of SDL_GPUDevice so IMuRenderer::GetDevice()
// can be declared without pulling SDL3 headers into every TU that includes MuRenderer.h.
// The returned pointer is opaque — callers cast to SDL_GPUDevice* after including SDL3 headers.
// Story 7.9.8: Forward declarations for SDL_ttf text engine and font handles.
struct SDL_GPUDevice;
struct TTF_TextEngine;
struct TTF_Font;

// RmlUi port: forward declarations for the per-frame GPU context accessor below, same
// opaque-pointer convention as GetDevice() -- callers cast after including SDL3 headers.
struct SDL_GPUCommandBuffer;
struct SDL_GPUTexture;
struct SDL_Window;

namespace mu
{

// ---------------------------------------------------------------------------
// BlendMode: Rendering blend equation presets.
// Maps to GL blend factor pairs documented in docs/architecture-rendering.md.
// ---------------------------------------------------------------------------
enum class BlendMode : std::uint8_t
{
    Alpha,        // GL_SRC_ALPHA,          GL_ONE_MINUS_SRC_ALPHA
    Additive,     // GL_SRC_ALPHA,          GL_ONE
    Subtract,     // GL_ZERO,               GL_ONE_MINUS_SRC_COLOR
    InverseColor, // GL_ONE_MINUS_DST_COLOR, GL_ZERO
    Mixed,        // GL_ONE,                GL_ONE_MINUS_SRC_ALPHA
    LightMap,     // GL_ZERO,               GL_SRC_COLOR
    Glow,         // GL_ONE,                GL_ONE             (Story 4.2.5 — EnableAlphaBlend)
    Luminance,    // GL_ONE_MINUS_SRC_COLOR, GL_ONE            (Story 4.2.5 — EnableAlphaBlend2)
};

// ---------------------------------------------------------------------------
// FogParams: Parameters for hardware fog setup.
// mode is one of GL_LINEAR / GL_EXP / GL_EXP2 (stored as plain int so that
// MuRenderer.h does not expose GL types; MuRenderer.cpp translates to GLenum).
// ---------------------------------------------------------------------------
struct FogParams
{
    int mode; // GL_LINEAR=0x2601, GL_EXP=0x0800, GL_EXP2=0x0801
    float start;
    float end;
    float density;
    float color[4]; // RGBA (same layout as GL_FOG_COLOR float[4])
};

// ---------------------------------------------------------------------------
// Vertex2D: Screen-space quad vertex (texture-mapped 2D).
// (x, y) are final screen pixels (post-conversion). Callers convert from
// their source coordinate space (e.g., 640x480 logical) before constructing.
// ---------------------------------------------------------------------------
struct Vertex2D
{
    float x, y;          // screen position (final pixels)
    float u, v;          // texture coordinates
    std::uint32_t color; // packed ABGR (matches GL vertex colour layout)
};

// ---------------------------------------------------------------------------
// Vertex3D: World-space vertex with normals (models, effects).
// ---------------------------------------------------------------------------
struct Vertex3D
{
    float x, y, z;       // world position
    float nx, ny, nz;    // surface normal
    float u, v;          // texture coordinates
    std::uint32_t color; // packed ABGR
};

// Rest-pose model vertex for GPU bone skinning.
struct SkinnedVertex3D
{
    float x, y, z;
    float nx, ny, nz;
    float u, v;
    std::uint32_t color;
    std::int32_t positionBoneIndex;
    std::int32_t normalBoneIndex;
};

enum class SkinningTextureCoordinates : std::uint32_t
{
    Mesh,
    Chrome,
    Chrome2,
    Chrome3,
    Chrome4,
    Chrome5,
    Chrome6,
    Chrome7,
    Oil,
    Metal,
};

// Bone matrices are contiguous row-major 3x4 affine transforms: 12 floats per bone.
struct SkinningParameters
{
    std::span<const float> boneMatrices;
    std::uint32_t paletteVersion = 0;
    float bodyOrigin[3]{};
    float bodyScale = 1.0f;
    float boneScale = 1.0f;
    float restPoseScale = 0.0f;
    float lightDirection[3]{};
    float textureCoordinateOffset[2]{};
    float chromeWave = 0.0f;
    float chromeWave2 = 0.0f;
    float chromeLight[2]{};
    float chromeTimeTerm = 0.0f;
    SkinningTextureCoordinates textureCoordinates = SkinningTextureCoordinates::Mesh;
    bool translate = false;
    bool lightEnabled = false;
};

// RmlUi port: the current frame's raw SDL_GPU submission context. RmlUi's own vendored
// RenderInterface_SDL_GPU (ThirdParty/RmlUi/Backends/) manages its own pipelines/buffers but
// needs the active command buffer + swapchain texture to record into and present through --
// this is how it gets them without a direct dependency on MuRendererSDLGpu.cpp internals
// (same pattern as GetDevice() below). All fields are null/zero outside BeginFrame()/EndFrame().
struct FrameGpuContext
{
    SDL_GPUCommandBuffer* commandBuffer = nullptr;
    SDL_GPUTexture* swapchainTexture = nullptr;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
};

struct RendererStats
{
    std::uint32_t requestedDrawCalls = 0;
    std::uint32_t submittedDrawCalls = 0;
    std::uint32_t mergedDrawCalls = 0;
    std::uint32_t merged2DDrawCalls = 0;
    std::uint32_t commandCount = 0;
    std::uint32_t vertexBytes = 0;
    std::uint32_t textureUploads = 0;
    std::uint32_t textureCreates = 0;
    std::uint32_t textureReleases = 0;
    std::uint32_t pipelineBinds = 0;
    std::uint32_t samplerBinds = 0;
    std::uint32_t vertexUniformPushes = 0;
    std::uint32_t fragmentUniformPushes = 0;
    double frameMilliseconds = 0.0;
    double replayMilliseconds = 0.0;
    double submitMilliseconds = 0.0;
};

// ---------------------------------------------------------------------------
// IMuRenderer: Pure abstract rendering interface.
// Game code obtains the active backend via GetRenderer() (see below).
// ---------------------------------------------------------------------------
class IMuRenderer
{
public:
    virtual ~IMuRenderer() = default;

    // Render a screen-space textured quad (4 vertices, GL_QUADS).
    virtual void RenderQuad2D(std::span<const Vertex2D> vertices, std::uint32_t textureId) = 0;

    // Render world-space triangles (vertex count must be divisible by 3).
    virtual void RenderTriangles(std::span<const Vertex3D> vertices, std::uint32_t textureId) = 0;

    // Render independent world-space quads in perimeter order.
    virtual void RenderQuad3D(std::span<const Vertex3D> vertices, std::uint32_t textureId) = 0;

    // Render rest-pose triangles using a GPU bone palette. False means an eligible draw failed
    // submission and must not trigger CPU fallback.
    [[nodiscard]] virtual bool RenderSkinnedTriangles(std::span<const SkinnedVertex3D> vertices,
                                                      std::uint32_t textureId, const SkinningParameters& parameters)
    {
        (void)vertices;
        (void)textureId;
        (void)parameters;
        return false;
    }

    // Render a quad strip from world-space vertices (requires >= 4 vertices, even count ideal).
    virtual void RenderQuadStrip(std::span<const Vertex3D> vertices, std::uint32_t textureId) = 0;

    // Set the active alpha-blending equation.
    virtual void SetBlendMode(BlendMode mode) = 0;

    // Disable alpha blending entirely (glDisable(GL_BLEND) path).
    // Distinct from SetBlendMode — "no blending" is a render state, not a blend mode.
    virtual void DisableBlend() = 0;

    // Enable or disable depth testing (glDepthFunc defaults to GL_LEQUAL).
    virtual void SetDepthTest(bool enabled) = 0;

    // GL state forwarding — SDL_gpu pipeline handles these per-draw.
    // Default implementations are no-ops; OpenGL backend overrides them.
    virtual void SetDepthMask(bool /*enabled*/) {}
    virtual void SetCullFace(bool /*enabled*/) {}
    virtual void SetAlphaTest(bool /*enabled*/) {}
    virtual void SetTexture2D(bool /*enabled*/) {}
    virtual void SetFogEnabled(bool /*enabled*/) {}

    // Bind texture by game bitmap index. SDL_gpu resolves this to SDL_GPUTexture*
    // via the texture registry; OpenGL calls glBindTexture directly.
    virtual void BindTexture(int /*texId*/) {}

    // Configure hardware fog for the current scene.
    virtual void SetFog(const FogParams& params) = 0;

    // Story 7-9-2 (AC-1): 3D scene projection setup — replaces BeginOpengl()/EndOpengl().
    // BeginScene sets viewport and projection (perspective + modelview).
    // EndScene restores matrix state.
    virtual void BeginScene(int x, int y, int w, int h) = 0;
    virtual void EndScene() = 0;

    // Story 7-9-2 (AC-2): 2D orthographic pass — replaces BeginBitmap()/EndBitmap().
    // Begin2DPass sets up orthographic projection for screen-space rendering.
    // End2DPass restores the previous projection state.
    // NOTE: OpenGL backend disables depth test in Begin2DPass and re-enables in End2DPass
    // as a side effect. SDL_gpu backend handles depth per-pipeline; these are no-ops there.
    virtual void Begin2DPass() = 0;
    virtual void End2DPass() = 0;

    // Story 7-9-2 (AC-7): Clear the color and depth buffers.
    // OpenGL backend: glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT).
    // SDL_gpu backend: no-op (SDL_gpu clears in BeginFrame).
    virtual void ClearScreen() = 0;

    // Story 7-9-2 (AC-7): Clear depth buffer only (mid-frame depth reset).
    // Used by UI 3D render panels to clear depth before rendering 3D items on top.
    // OpenGL backend: glClear(GL_DEPTH_BUFFER_BIT).
    // SDL_gpu backend: no-op (depth handled by render pass management).
    virtual void ClearDepthBuffer() {}

    // Story 7-9-2 (AC-5): Render line primitives (GL_LINES replacement).
    // Used by debug visualizations (collision, skeleton, waypoint gizmos).
    // Vertex count should be even (pairs); odd count logs a warning, last vertex ignored.
    virtual void RenderLines(std::span<const Vertex3D> vertices, std::uint32_t textureId) = 0;

    // Query whether the renderer is between BeginFrame() and EndFrame().
    // Used by RenderTitleSceneUI() to self-manage frame submission.
    [[nodiscard]] virtual bool IsFrameActive() const
    {
        return false;
    }

    // Per-frame lifecycle. Called from the MuMain.cpp game loop.
    virtual void BeginFrame() {}
    virtual void EndFrame() {}
    virtual void SetStatsEnabled(bool /*enabled*/) {}
    [[nodiscard]] virtual RendererStats GetFrameStats() const
    {
        return {};
    }
    [[nodiscard]] virtual bool SetVSyncEnabled(bool /*enabled*/)
    {
        return false;
    }
    [[nodiscard]] virtual bool RequestFramePixels()
    {
        return false;
    }
    [[nodiscard]] virtual bool ConsumeFramePixels(FramePixels& pixels)
    {
        (void)pixels;
        return false;
    }

    // Story 4.4.1 — Texture System Migration: SDL_gpu device accessor.
    // Returns the SDL_GPUDevice* used by the active backend, or nullptr if not available.
    // Default implementation returns nullptr (OpenGL backend has no SDL_GPUDevice).
    // MuRendererSDLGpu overrides to return s_device.
    // Used by GlobalBitmap.cpp to upload textures via SDL_gpu without a layering violation.
    // [[nodiscard]]: callers must check for nullptr before using the device.
    [[nodiscard]] virtual void* GetDevice()
    {
        return nullptr;
    }

    // RmlUi port: SDL_Window* accessor, same opaque-pointer/nullptr-default convention as
    // GetDevice() above. RenderInterface_SDL_GPU's constructor needs both.
    [[nodiscard]] virtual SDL_Window* GetWindow()
    {
        return nullptr;
    }

    // RmlUi port: current frame's command buffer + swapchain texture. See FrameGpuContext's
    // own comment above. Valid only between BeginFrame() and EndFrame(); default (all-null)
    // outside that window, same as GetDevice()'s "not initialized" nullptr default.
    [[nodiscard]] virtual FrameGpuContext GetFrameGpuContext()
    {
        return {};
    }

    // RmlUi port: resolves this engine's logical texture id (BITMAP_t::TextureNumber, despite
    // its GLuint-typed name -- CGlobalBitmap sets it from CreateTexture()'s returned id, not a
    // real GL name, on this backend) to the raw SDL_GPUTexture* RmlUi's own vendored
    // RenderInterface_SDL_GPU expects as its TextureHandle. Returns nullptr if textureId isn't
    // currently registered. Opaque void*, same convention as GetDevice().
    [[nodiscard]] virtual void* GetRawTexture(std::uint32_t /*textureId*/)
    {
        return nullptr;
    }

    // RmlUi port: registers a callback the backend invokes once per frame after its own
    // game-content replay/blit is fully recorded onto the frame's command buffer, but before
    // that command buffer is submitted. This is the seam RmlUi needs to render "last" (see
    // README.md's Frame Lifecycle section's ordering invariant) without EndFrame() itself
    // knowing anything about RmlUi -- RmlUiRuntime::Create() is the only caller. Calling
    // RenderScene() (or similar, mid-frame) is too early: game content is only *recorded* there,
    // not yet replayed onto the command buffer, so anything drawn at that point would land
    // before the game's own content instead of on top of it. Calling after EndFrame() returns is
    // too late: the command buffer is already submitted and the swapchain texture is gone.
    // Pass an empty std::function to unregister. Default no-op backend never calls anything.
    virtual void SetPreSubmitCallback(std::function<void()> /*callback*/) {}

    // Fires after RmlUi's own render pass (the one SetPreSubmitCallback above triggers) has
    // already closed, still before the frame's command buffer is submitted -- the seam for
    // content that must sit visually on top of RmlUi (the game cursor, legacy CUITextInputBox
    // text; see README.md's Gotchas section's "pointer-events swallows every click" neighbor
    // bug and its cursor/text-ordering counterpart). Backed by its own small
    // render pass (LOAD, not CLEAR) that replays whatever this callback pushes via the normal
    // RenderQuad2D-style functions -- calling those same functions from inside
    // SetPreSubmitCallback's callback instead does NOT work: the main render pass (and the
    // s_renderCmds list it replays) is already closed by the time that callback fires, so
    // anything pushed there lands unreplayed at the tail of s_renderCmds until next frame's
    // BeginFrame() silently clears it away. Pass an empty std::function to unregister. Default
    // no-op backend never calls anything.
    virtual void SetPostRmlUiCallback(std::function<void()> /*callback*/) {}

    // RmlUi-behind-3D-icons seam (docs/rmlui-ui-system/STATUS.md's "RmlUi renders last" finding):
    // opens a real render pass NOW, mid-recording, replaying only what's been recorded into this
    // frame's command list since the last flush (or frame start), instead of waiting for the one
    // pass EndFrame normally opens once. Content drawn immediately after this call (e.g. a second
    // Rml::Context's Render()) lands behind whatever legacy content the caller records next, and
    // in front of everything recorded before this call -- the two existing seams above only let
    // content render after ALL game content for the frame; this is the one that lets something
    // render in the *middle*. CLEARs the very first time it (or EndFrame's own final pass) runs
    // each frame, LOADs every time after -- callers don't need to know which. Default no-op
    // backend does nothing (nothing to flush). See MuRendererSDLGpu.cpp's implementation comment
    // for the concrete pass-sequencing/state-tracking this requires.
    virtual void FlushRenderCommands() {}

    // Story 7.9.8 (AC-2): SDL_ttf GPU text engine accessor.
    // Returns the TTF_TextEngine* for creating TTF_Text objects, or nullptr if unavailable.
    [[nodiscard]] virtual TTF_TextEngine* GetTextEngine()
    {
        return nullptr;
    }

    // Story 7.9.8 (AC-2): Default TTF font accessor.
    // Returns the TTF_Font* loaded at init, or nullptr if no font was found.
    [[nodiscard]] virtual TTF_Font* GetTtfFont()
    {
        return nullptr;
    }

    // F-1 fix: Font variant accessors for bold, big, and fixed-width text.
    // Returns the default font as fallback if the variant wasn't loaded.
    [[nodiscard]] virtual TTF_Font* GetTtfFontBold()
    {
        return nullptr;
    }
    [[nodiscard]] virtual TTF_Font* GetTtfFontBig()
    {
        return nullptr;
    }
    [[nodiscard]] virtual TTF_Font* GetTtfFontFixed()
    {
        return nullptr;
    }
    [[nodiscard]] virtual bool ReloadTtfFonts(std::string_view /*fontFamily*/, float /*normalPointSize*/,
                                              float /*bigPointSize*/, float /*fixedPointSize*/)
    {
        return false;
    }

    // F-7 fix: Cached window height (updated per-frame in BeginFrame).
    [[nodiscard]] virtual int GetCachedWindowHeight()
    {
        return 0;
    }

    // Story 7.9.8 (AC-6): Submit text triangles for deferred rendering.
    // Vertices are Vertex2D format, atlasTexture is the glyph atlas from TTF draw data.
    // sampler may be null (uses default). Non-indexed triangle list.
    virtual void SubmitTextTriangles(std::span<const Vertex2D> vertices, void* atlasTexture, void* sampler = nullptr)
    {
        (void)vertices;
        (void)atlasTexture;
        (void)sampler;
    }

    // Queue a CPU→GPU texture update for the current frame's copy pass.
    // Used when dynamic BITMAP_t buffers, such as guild marks, change.
    // The implementation copies pixels before QueueTextureUpdate returns.
    virtual void QueueTextureUpdate(std::uint32_t textureId, const void* pixels, std::uint32_t width,
                                    std::uint32_t height)
    {
        (void)textureId;
        (void)pixels;
        (void)width;
        (void)height;
    }
    virtual void EnsureTexture(std::uint32_t textureId, std::uint32_t width, std::uint32_t height)
    {
        (void)textureId;
        (void)width;
        (void)height;
    }
    [[nodiscard]] virtual std::uint32_t CreateTexture(
        std::uint32_t width, std::uint32_t height, const void* pixels)
    {
        (void)width;
        (void)height;
        (void)pixels;
        return 0;
    }
    virtual void ReleaseTexture(std::uint32_t textureId)
    {
        (void)textureId;
    }
    [[nodiscard]] virtual std::uint32_t CaptureFrameTexture(std::uint32_t textureId)
    {
        (void)textureId;
        return 0;
    }
    [[nodiscard]] virtual bool IsTextureRegistered(std::uint32_t textureId) const
    {
        (void)textureId;
        return false;
    }

    // -----------------------------------------------------------------------
    // Story 7-9-6: GL state migration — replaces raw OpenGL calls.
    // Default implementations are no-ops; SDL_gpu backend overrides them.
    // -----------------------------------------------------------------------

    // AC-3: Clear color — replaces glClearColor.
    // SDL_gpu backend stores RGBA and applies in BeginFrame render pass.
    virtual void SetClearColor(float /*r*/, float /*g*/, float /*b*/, float /*a*/) {}

    // AC-4: Matrix stack — replaces glMatrixMode/glPushMatrix/glPopMatrix/etc.
    // SDL_gpu backend maintains internal MatrixStack and uploads to GPU uniform buffer.
    virtual void SetMatrixMode(int /*mode*/) {}
    virtual void PushMatrix() {}
    virtual void PopMatrix() {}
    virtual void LoadIdentity() {}
    virtual void Translate(float /*x*/, float /*y*/, float /*z*/) {}
    virtual void Rotate(float /*angle*/, float /*x*/, float /*y*/, float /*z*/) {}
    virtual void Scale(float /*x*/, float /*y*/, float /*z*/) {}
    virtual void MultMatrix(const float* /*m*/) {}
    virtual void LoadMatrix(const float* /*m*/) {}
    virtual void GetMatrix(int /*mode*/, float* /*m*/) {}

    // AC-6: Depth/stencil/state — replaces glDepthFunc/glAlphaFunc/glStencilFunc/etc.
    virtual void SetStencilTest(bool /*enabled*/) {}
    virtual void SetDepthFunc(int /*func*/) {}
    virtual void SetAlphaFunc(int /*func*/, float /*ref*/) {}
    virtual void SetStencilFunc(int /*func*/, int /*ref*/, unsigned int /*mask*/) {}
    virtual void SetStencilOp(int /*sfail*/, int /*dpfail*/, int /*dppass*/) {}
    virtual void SetColorMask(bool /*r*/, bool /*g*/, bool /*b*/, bool /*a*/) {}

    // Additional state — replaces glPolygonMode/glFrontFace.
    virtual void SetPolygonMode(int /*face*/, int /*mode*/) {}
    virtual void SetFrontFace(int /*mode*/) {}
    virtual void SetMultisample(bool /*enabled*/) {}

    // Texture environment — replaces glTexEnvi/glTexEnvf (fixed-function, no-op in SDL GPU).
    virtual void SetTexEnv(int /*target*/, int /*pname*/, int /*param*/) {}
    virtual void SetTexParameter(int /*target*/, int /*pname*/, int /*param*/) {}

    // AC-6: Viewport/scissor — replaces glViewport/glScissor.
    virtual void SetViewport(int /*x*/, int /*y*/, int /*w*/, int /*h*/) {}
    virtual void SetScissor(int /*x*/, int /*y*/, int /*w*/, int /*h*/) {}
    virtual void SetScissorEnabled(bool /*enabled*/) {}

    // AC-7: Screenshot — replaces glReadPixels.
    // SDL_gpu backend: SDL_GPUDownloadFromGPUTexture or SDL_RenderReadPixels.
    virtual void ReadPixels(int /*x*/, int /*y*/, int /*w*/, int /*h*/, void* /*data*/) {}

    // [Story 7-6-7: AC-3] GPU backend driver name for error reporting.
    // Returns "unknown" by default; SDL GPU backend overrides with SDL_GetGPUDeviceDriver().
    [[nodiscard]] virtual const char* GetGPUDriverName() const
    {
        return "unknown";
    }
};

// ---------------------------------------------------------------------------
// GetRenderer: Singleton accessor — returns the active rendering backend.
// Initially returns MuRendererGL; will return MuRendererSDLGPU after 4.3.1.
// ---------------------------------------------------------------------------
[[nodiscard]] IMuRenderer& GetRenderer();
[[nodiscard]] bool InitSDLGpuRenderer(void* pNativeWindow, std::string_view fontFamily, float normalPointSize,
                                      float bigPointSize, float fixedPointSize);
void WaitForSDLGpuIdle();
void ShutdownSDLGpuRenderer();

} // namespace mu
