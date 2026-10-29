// MuRenderer.h: Rendering abstraction interface for the MU Online game client.
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
#include "SceneLighting.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <string_view>
#include <vector>

// Forward declaration of SDL_GPUDevice so IMuRenderer::GetDevice()
// can be declared without pulling SDL3 headers into every TU that includes MuRenderer.h.
// The returned pointer is opaque — callers cast to SDL_GPUDevice* after including SDL3 headers.
// Also forward-declares SDL_ttf text engine and font handles.
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
// Maps to GL blend factor pairs used by this renderer's blend pipeline.
// ---------------------------------------------------------------------------
enum class BlendMode : std::uint8_t
{
    Alpha,        // GL_SRC_ALPHA,          GL_ONE_MINUS_SRC_ALPHA
    Additive,     // GL_SRC_ALPHA,          GL_ONE
    Subtract,     // GL_ZERO,               GL_ONE_MINUS_SRC_COLOR
    InverseColor, // GL_ONE_MINUS_DST_COLOR, GL_ZERO
    Mixed,        // GL_ONE,                GL_ONE_MINUS_SRC_ALPHA
    LightMap,     // GL_ZERO,               GL_SRC_COLOR
    Glow,         // GL_ONE,                GL_ONE
    Luminance,    // GL_ONE_MINUS_SRC_COLOR, GL_ONE
};

// What the post-processed frame shows in place of the 3D scene: the scene
// itself, or for diagnosis the bloom source (the additive draws alone) or the
// blurred bloom. The UI is drawn on top either way.
enum class PostProcessingView : std::uint8_t
{
    Scene,
    BloomSource,
    Bloom,
};

// The look profile's post-processing values (Render::Look): the bloom strength
// (0: no bloom), the weight each wider bloom level adds to the next narrower one,
// and how much of the map's colour grading applies (0.1 to 1).
struct PostProcessingLook
{
    float bloomStrength = 0.0f;
    float bloomLevelFalloff = 0.5f;
    float gradingStrength = 1.0f;
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

// Rest-pose model vertex for GPU bone skinning (BMD models): it follows one bone for its
// position and one for its normal. The colour is per draw, not per vertex.
struct SkinnedVertex3D
{
    float x, y, z;
    float nx, ny, nz;
    float u, v;
    std::int32_t positionBoneIndex;
    std::int32_t normalBoneIndex;
};

// SkinnedVertex3D with weights (glTF models), drawn with its own pipelines so that BMD
// vertices stay small. A vertex with weights is in bind space and blends up to four palette
// joints by weight (unorm, summing to 255, the largest first); one without follows its bones.
// tangent: the bind-space tangent of a vertex under a normal map, snorm16 xyz and
// w = +-32767 for the handedness; all 0 otherwise.
struct SkinnedWeightedVertex3D
{
    float x, y, z;
    float nx, ny, nz;
    float u, v;
    std::int32_t positionBoneIndex;
    std::int32_t normalBoneIndex;
    std::uint8_t joints[4];
    std::uint8_t weights[4];
    std::int16_t tangent[4];
};
static_assert(sizeof(SkinnedVertex3D) == 40 && sizeof(SkinnedWeightedVertex3D) == 56);
static_assert(offsetof(SkinnedWeightedVertex3D, normalBoneIndex) == offsetof(SkinnedVertex3D, normalBoneIndex),
              "both skinned pipelines read the leading fields at the same offsets");

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
    // Lit colours above this roll off towards 1 instead of being cut off there (the light
    // mood's highlight shoulder); 1 is the Classic cut-off.
    float highlightKnee = 1.0f;
    float textureCoordinateOffset[2]{};
    float chromeWave = 0.0f;
    float chromeWave2 = 0.0f;
    float chromeLight[2]{};
    float chromeTimeTerm = 0.0f;
    SkinningTextureCoordinates textureCoordinates = SkinningTextureCoordinates::Mesh;
    bool translate = false;
    bool lightEnabled = false;
    // Enhanced lighting only: Blinn-Phong strength declared by the item model; 0 = no highlight.
    float specularStrength = 0.0f;
    // Enhanced lighting only: the tangent-space normal map of the draw (a texture id) and its
    // glTF scale; a scale of 0 (or no map) lights with the unperturbed normal. Classic ignores it.
    std::uint32_t normalTextureId = 0;
    float normalMapScale = 0.0f;
};

// Where a body shadow lands, as CalcShadowPosition() puts it there.
struct ShadowProjection
{
    float origin[3]{};
    float shearX = 0.0f;        // sx
    float shearHeight = 0.0f;   // sy
    float specialHeight = 0.0f; // the height of TW_HEIGHT cells
    bool terrain = true;        // false in the scenes where RequestTerrainHeight() returns 0
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
    // The texture has the swapchain's format and can be sampled, so RmlUi can draw straight into it.
    bool drawable = false;
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
// The corners of indexed triangles, in index order: out[i] = vertices[indices[i]]
// (out holds indices.size() vertices). An indexed draw and the draw of these
// corners rasterise the same triangles in the same order.
template <typename Vertex>
void ExpandIndexedCorners(std::span<const Vertex> vertices, std::span<const std::uint16_t> indices,
                          std::vector<Vertex>& out)
{
    out.resize(indices.size());
    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        out[i] = vertices[indices[i]];
    }
}

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

    // Render rest-pose triangles using a GPU bone palette, in one packed ABGR colour. False means an
    // eligible draw failed submission and must not trigger CPU fallback.
    [[nodiscard]] virtual bool RenderSkinnedTriangles(std::span<const SkinnedVertex3D> vertices, std::uint32_t color,
                                                      std::uint32_t textureId, const SkinningParameters& parameters)
    {
        (void)vertices;
        (void)color;
        (void)textureId;
        (void)parameters;
        return false;
    }
    [[nodiscard]] virtual bool RenderWeightedSkinnedTriangles(std::span<const SkinnedWeightedVertex3D> vertices,
                                                              std::uint32_t color, std::uint32_t textureId,
                                                              const SkinningParameters& parameters)
    {
        (void)vertices;
        (void)color;
        (void)textureId;
        (void)parameters;
        return false;
    }

    // RenderTriangles of the triangles `indices` makes of `vertices` (three
    // indices per triangle, in draw order). A backend without index buffers
    // keeps this default: the corners (ExpandIndexedCorners) through
    // RenderTriangles.
    virtual void RenderIndexedTriangles(std::span<const Vertex3D> vertices, std::span<const std::uint16_t> indices,
                                        std::uint32_t textureId)
    {
        static thread_local std::vector<Vertex3D> corners;
        ExpandIndexedCorners(vertices, indices, corners);
        RenderTriangles(corners, textureId);
    }

    // Rest-pose triangles kept on the GPU under `mesh`, so a draw no longer uploads them. The data
    // must not change while cached; ReleaseSkinnedMesh before it is freed. Releasing is safe at any
    // time, also after the renderer has shut down.
    [[nodiscard]] virtual bool IsSkinnedMeshCached(const void* mesh) const
    {
        (void)mesh;
        return false;
    }
    [[nodiscard]] virtual bool CacheSkinnedMesh(const void* mesh, std::span<const SkinnedVertex3D> vertices)
    {
        (void)mesh;
        (void)vertices;
        return false;
    }
    [[nodiscard]] virtual bool CacheWeightedSkinnedMesh(const void* mesh,
                                                        std::span<const SkinnedWeightedVertex3D> vertices)
    {
        (void)mesh;
        (void)vertices;
        return false;
    }
    // Same contract as RenderSkinnedTriangles, for a cached mesh.
    [[nodiscard]] virtual bool RenderCachedSkinnedMesh(const void* mesh, std::uint32_t color, std::uint32_t textureId,
                                                       const SkinningParameters& parameters)
    {
        (void)mesh;
        (void)color;
        (void)textureId;
        (void)parameters;
        return false;
    }
    virtual void ReleaseSkinnedMesh(const void* mesh)
    {
        (void)mesh;
    }

    // A cached mesh drawn as its ground shadow, with the active texture, blend and depth test state and depth
    // writes off. Needs SetTerrainHeights() this frame. False means the caller draws the shadow itself.
    [[nodiscard]] virtual bool RenderCachedSkinnedShadow(const void* mesh, std::uint32_t color,
                                                         const SkinningParameters& parameters,
                                                         const ShadowProjection& projection)
    {
        (void)mesh;
        (void)color;
        (void)parameters;
        (void)projection;
        return false;
    }

    // The map's heights and wall flags (TERRAIN_SIZE * TERRAIN_SIZE each) for shaders that follow the
    // ground, copied again only when `version` changes. False means the GPU has no heights to read.
    [[nodiscard]] virtual bool SetTerrainHeights(std::span<const float> heights, std::span<const std::uint16_t> walls,
                                                 std::uint32_t version)
    {
        (void)heights;
        (void)walls;
        (void)version;
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

    // Frame-wide lighting read by every Enhanced-lit draw. The scene sets it once per frame, after
    // its fog; the fog configured at that moment becomes the frame's scene fog. The last state set
    // in a frame is the one all of that frame's draws use.
    virtual void SetSceneLighting(const SceneLighting& /*lighting*/) {}

    // The frame's point lights for Enhanced-lit draws (Enhanced dynamic lights). The last set in a
    // frame is the one all of that frame's draws use; set an empty block to turn them off.
    virtual void SetDynamicLights(const DynamicLightsUniform& /*lights*/) {}

    // Select Enhanced (per-pixel lighting and fog) or Classic pipelines for the following frames.
    // Returns false when Enhanced lighting was requested but this backend could not build it; the
    // renderer then keeps drawing Classic.
    [[nodiscard]] virtual bool SetEnhancedLighting(bool enabled)
    {
        return !enabled;
    }

    // 3D scene projection setup — replaces BeginOpengl()/EndOpengl().
    // BeginScene sets viewport and projection (perspective + modelview).
    // EndScene restores matrix state.
    virtual void BeginScene(int x, int y, int w, int h) = 0;
    virtual void EndScene() = 0;

    // 2D orthographic pass — replaces BeginBitmap()/EndBitmap().
    // Begin2DPass sets up orthographic projection for screen-space rendering.
    // End2DPass restores the previous projection state.
    // NOTE: OpenGL backend disables depth test in Begin2DPass and re-enables in End2DPass
    // as a side effect. SDL_gpu backend handles depth per-pipeline; these are no-ops there.
    virtual void Begin2DPass() = 0;
    virtual void End2DPass() = 0;

    // Enhanced texture filtering samples world textures through a mipmapped,
    // anisotropic sampler. Draws between BeginScreenSpaceSampling and
    // EndScreenSpaceSampling (3D models shown in the UI, such as inventory
    // item previews) keep each texture's Classic sampler, as 2D quads always
    // do. Scopes nest; use ScopedScreenSpaceSampling.
    virtual void BeginScreenSpaceSampling() {}
    virtual void EndScreenSpaceSampling() {}

    // Draws between BeginBloomExclusion and EndBloomExclusion never feed the
    // post-processing bloom, even when they blend additively: a model's chrome,
    // metal or oil pass (light reflected off its surface, not emitted), and
    // characters with everything they wear and ride, so a character looks as it
    // does in Classic. Scopes nest; use ScopedBloomExclusion.
    virtual void BeginBloomExclusion() {}
    virtual void EndBloomExclusion() {}
    // True while a bloom exclusion is open: what is queued now and drawn later
    // (a character's glow sprites) can be drawn inside one too.
    [[nodiscard]] virtual bool IsBloomExcluded() const
    {
        return false;
    }

    // Clear the color and depth buffers.
    // OpenGL backend: glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT).
    // SDL_gpu backend: no-op (SDL_gpu clears in BeginFrame).
    virtual void ClearScreen() = 0;

    // Clear depth buffer only (mid-frame depth reset).
    // Used by UI 3D render panels to clear depth before rendering 3D items on top.
    // OpenGL backend: glClear(GL_DEPTH_BUFFER_BIT).
    // SDL_gpu backend: no-op (depth handled by render pass management).
    virtual void ClearDepthBuffer() {}

    // Render line primitives (GL_LINES replacement).
    // Used by debug visualizations (collision, skeleton, waypoint gizmos).
    // Vertex count should be even (pairs); odd count logs a warning, last vertex ignored.
    virtual void RenderLines(std::span<const Vertex3D> vertices, std::uint32_t textureId) = 0;

    // Query whether the renderer is between BeginFrame() and EndFrame().
    // Used by RenderTitleSceneUI() to self-manage frame submission.
    [[nodiscard]] virtual bool IsFrameActive() const
    {
        return false;
    }

    // Waits until the window can take another frame (the GPU has finished an
    // earlier one). The game loop calls it before reading input, so with VSync
    // the loop waits there and not in BeginFrame, after the input was read.
    virtual void WaitForSwapchain() {}
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
    // Multisample anti-aliasing: `samples` is the `[Render] Antialiasing`
    // value (0 or 1 off, 2, 4, 8), lowered to what the GPU supports. Before
    // initialisation it sets the count the renderer starts with; afterwards
    // the change applies at the next BeginFrame, between frames.
    virtual void SetAntialiasingSamples(int /*samples*/) {}
    // Samples per pixel the frames render with now; 1 is off.
    [[nodiscard]] virtual int GetAntialiasingSamples() const
    {
        return 1;
    }

    // Post-processing (bloom and colour grading) of the 3D scene. A scene calls
    // EndSceneGeometry once its 3D world is drawn and before its UI: what was
    // drawn before is processed, what is drawn after is drawn on the processed
    // image. The first call of a frame counts; a frame without one is not
    // processed.
    virtual void EndSceneGeometry() {}

    // Opaque, depth-writing draws recorded between these two move ahead of the others and are
    // grouped by state, to cut state changes; depth decides what shows, and the others keep
    // their order after them. Use ScopedReorderableDraws.
    [[nodiscard]] virtual std::size_t BeginReorderableDraws()
    {
        return 0u;
    }
    virtual void EndReorderableDraws(std::size_t /*mark*/) {}
    // On by default; the benchmark turns it off to measure it.
    virtual void SetDrawReorderingEnabled(bool /*enabled*/) {}
    [[nodiscard]] virtual bool IsDrawReorderingEnabled() const
    {
        return false;
    }
    // Returns false when post-processing was requested but this backend could
    // not build it; frames then stay unprocessed.
    [[nodiscard]] virtual bool SetPostProcessing(bool enabled)
    {
        return !enabled;
    }
    // The look profile's bloom and grading values; a bloom strength of 0 is no bloom.
    virtual void SetPostProcessingLook(const PostProcessingLook& /*look*/) {}
    // Diagnosis only, not saved: what post-processed frames show (see PostProcessingView).
    virtual void SetPostProcessingView(PostProcessingView /*view*/) {}
    // The colour grading lookup table of the current map: a 1024x32 RGBA8
    // strip (32 slices of 32x32, blue picks the slice), or empty for none.
    // Returns false when the table could not be used; the scene is then not graded.
    [[nodiscard]] virtual bool SetColorGradingTable(std::span<const std::uint8_t> rgba)
    {
        return rgba.empty();
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
    // Drops a requested readback that has not been consumed, delivered or not,
    // so the next request can start.
    virtual void CancelFramePixels() {}

    // SDL_gpu device accessor.
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
    // that command buffer is submitted. This is the seam RmlUi needs to render "last" without
    // EndFrame() itself
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
    // text -- the cursor/text-ordering counterpart to the "pointer-events swallows every click"
    // hazard). Backed by its own small
    // render pass (LOAD, not CLEAR) that replays whatever this callback pushes via the normal
    // RenderQuad2D-style functions -- calling those same functions from inside
    // SetPreSubmitCallback's callback instead does NOT work: the main render pass (and the
    // s_renderCmds list it replays) is already closed by the time that callback fires, so
    // anything pushed there lands unreplayed at the tail of s_renderCmds until next frame's
    // BeginFrame() silently clears it away. Pass an empty std::function to unregister. Default
    // no-op backend never calls anything.
    virtual void SetPostRmlUiCallback(std::function<void()> /*callback*/) {}

    // Fires at the top of EndFrame, while the frame is still recording but after every ordinary
    // draw -- and after the last FlushRenderCommands(), so nothing recorded here has been replayed
    // yet. The seam for render targets (CreateRenderTarget()): draws recorded here between
    // BeginOffscreenCapture()/EndOffscreenCapture() render into their own textures before the
    // frame's main pass, are kept out of it, and are ready before either UI seam above samples
    // them. Recording a capture any earlier risks a mid-frame flush drawing it onto the frame
    // first. Pass an empty std::function to unregister. Default no-op backend never calls anything.
    virtual void SetOffscreenRenderCallback(std::function<void()> /*callback*/) {}

    // RmlUi-behind-3D-icons seam: RmlUi's main context always composites last in the frame, so a
    // caller that needs content to paint behind a live 3D render needs an earlier seam of its own.
    // Opens a real render pass NOW, mid-recording, replaying only what's been recorded into this
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

    // SDL_ttf GPU text engine accessor.
    // Returns the TTF_TextEngine* for creating TTF_Text objects, or nullptr if unavailable.
    [[nodiscard]] virtual TTF_TextEngine* GetTextEngine()
    {
        return nullptr;
    }

    // Default TTF font accessor.
    // Returns the TTF_Font* loaded at init, or nullptr if no font was found.
    [[nodiscard]] virtual TTF_Font* GetTtfFont()
    {
        return nullptr;
    }

    // Font variant accessors for bold, big, and fixed-width text.
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

    // Cached window height (updated per-frame in BeginFrame).
    [[nodiscard]] virtual int GetCachedWindowHeight()
    {
        return 0;
    }

    // Submit text triangles for deferred rendering.
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
    // Isolated offscreen render captures. Draw calls issued between
    // BeginOffscreenCapture()/EndOffscreenCapture() render into a dedicated
    // texture instead of the main frame and never appear on screen: the Map
    // Editor's object-model preview grid, and render targets an RmlUi document
    // shows like any other image.
    // -----------------------------------------------------------------------

    // `textureId` may be an existing id (reused/resized as needed) or 0 to
    // allocate a new one. Returns the texture id, or 0 on failure. Calls don't
    // nest - only one capture may be open at a time.
    [[nodiscard]] virtual std::uint32_t BeginOffscreenCapture(std::uint32_t /*textureId*/, std::uint32_t /*width*/,
                                                              std::uint32_t /*height*/)
    {
        return 0u;
    }
    // Closes the capture opened by BeginOffscreenCapture.
    virtual void EndOffscreenCapture() {}
    // Until the open capture closes, 2D drawing (quads, text) maps this rectangle of the window, in
    // pixels from its top-left, onto the capture instead of the whole window: native 2D lands in a
    // render target where it would have landed on screen.
    virtual void SetOffscreen2DRect(float /*left*/, float /*top*/, float /*width*/, float /*height*/) {}

    // A texture that captures render into and anything can sample -- GetRawTexture() resolves it.
    // Unlike a one-shot capture's, it owns a depth buffer of its own size and clears to
    // transparent, so it composites over whatever is drawn behind it. Its size is fixed: capture
    // into it with exactly `width` x `height`, and create another to resize. Returns 0 on failure.
    [[nodiscard]] virtual std::uint32_t CreateRenderTarget(std::uint32_t /*width*/, std::uint32_t /*height*/)
    {
        return 0u;
    }
    // Released at the start of the next frame, never during one: releasing an owned texture
    // mid-frame drops that whole frame's replay. A caller that a sampler may still read this frame
    // must hold off releasing until it no longer can.
    virtual void ReleaseRenderTarget(std::uint32_t /*textureId*/) {}

#ifdef _EDITOR
    // Real GPU texture pointer for a texture id (from CreateTexture or
    // BeginOffscreenCapture), for handing to ImGui as ImTextureID. Returns
    // nullptr if the id isn't registered.
    [[nodiscard]] virtual void* GetTexturePointer(std::uint32_t /*textureId*/) const { return nullptr; }

    // True if one or more completed BeginOffscreenCapture()/EndOffscreenCapture()
    // draw-command ranges are still waiting to be replayed into their capture
    // texture (replay happens once per EndFrame(), after this frame's ImGui/game
    // code has already returned). A caller that's about to release/reload a
    // resource a just-recorded capture might still be sampling (e.g. reusing a
    // scratch model slot for a different file) must wait for this to go false
    // first, or the eventual replay reads a freed texture/sampler.
    [[nodiscard]] virtual bool HasPendingOffscreenCaptures() const { return false; }
#endif // _EDITOR

    // -----------------------------------------------------------------------
    // GL state migration — replaces raw OpenGL calls.
    // Default implementations are no-ops; SDL_gpu backend overrides them.
    // -----------------------------------------------------------------------

    // Clear color — replaces glClearColor.
    // SDL_gpu backend stores RGBA and applies in BeginFrame render pass.
    virtual void SetClearColor(float /*r*/, float /*g*/, float /*b*/, float /*a*/) {}

    // Matrix stack — replaces glMatrixMode/glPushMatrix/glPopMatrix/etc.
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

    // Depth/stencil/state — replaces glDepthFunc/glAlphaFunc/glStencilFunc/etc.
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

    // Viewport/scissor — replaces glViewport/glScissor.
    virtual void SetViewport(int /*x*/, int /*y*/, int /*w*/, int /*h*/) {}
    virtual void SetScissor(int /*x*/, int /*y*/, int /*w*/, int /*h*/) {}
    virtual void SetScissorEnabled(bool /*enabled*/) {}

    // Screenshot — replaces glReadPixels.
    // SDL_gpu backend: SDL_GPUDownloadFromGPUTexture or SDL_RenderReadPixels.
    virtual void ReadPixels(int /*x*/, int /*y*/, int /*w*/, int /*h*/, void* /*data*/) {}

    // GPU backend driver name for error reporting.
    // Returns "unknown" by default; SDL GPU backend overrides with SDL_GetGPUDeviceDriver().
    [[nodiscard]] virtual const char* GetGPUDriverName() const
    {
        return "unknown";
    }
    // The adapter's name as the driver reports it; empty when unknown.
    [[nodiscard]] virtual const char* GetGPUDeviceName() const
    {
        return "";
    }
};

// ---------------------------------------------------------------------------
// GetRenderer: Singleton accessor — returns the active rendering backend.
// Initially returns MuRendererGL; will return MuRendererSDLGPU after 4.3.1.
// ---------------------------------------------------------------------------
[[nodiscard]] IMuRenderer& GetRenderer();

// Keeps Classic texture sampling for the draws made while it is alive.
class ScopedScreenSpaceSampling
{
public:
    ScopedScreenSpaceSampling()
    {
        GetRenderer().BeginScreenSpaceSampling();
    }
    ~ScopedScreenSpaceSampling()
    {
        GetRenderer().EndScreenSpaceSampling();
    }
    ScopedScreenSpaceSampling(const ScopedScreenSpaceSampling&) = delete;
    ScopedScreenSpaceSampling& operator=(const ScopedScreenSpaceSampling&) = delete;
};
class ScopedBloomExclusion
{
public:
    ScopedBloomExclusion()
    {
        GetRenderer().BeginBloomExclusion();
    }
    ~ScopedBloomExclusion()
    {
        GetRenderer().EndBloomExclusion();
    }
    ScopedBloomExclusion(const ScopedBloomExclusion&) = delete;
    ScopedBloomExclusion& operator=(const ScopedBloomExclusion&) = delete;
};
// The draws recorded in its lifetime may be reordered (BeginReorderableDraws).
class ScopedReorderableDraws
{
public:
    ScopedReorderableDraws() : m_mark(GetRenderer().BeginReorderableDraws()) {}
    ~ScopedReorderableDraws()
    {
        GetRenderer().EndReorderableDraws(m_mark);
    }
    ScopedReorderableDraws(const ScopedReorderableDraws&) = delete;
    ScopedReorderableDraws& operator=(const ScopedReorderableDraws&) = delete;

private:
    std::size_t m_mark;
};

[[nodiscard]] bool InitSDLGpuRenderer(void* pNativeWindow, std::string_view fontFamily,
                                      std::string_view renderBackend, float normalPointSize,
                                      float bigPointSize, float fixedPointSize);
void WaitForSDLGpuIdle();
void ShutdownSDLGpuRenderer();

} // namespace mu
