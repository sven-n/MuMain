// RHI_GL: the OpenGL backend for Render/RHI/RHI.h. This file's functions sit in
// `RHI_GL_Impl`, forwarded to by `RHI.cpp`'s `namespace RHI` dispatch layer -- the only
// backend implementation today.
//
// The GL context itself is created and owned by Winmain.cpp (SDL_GL_CreateContext,
// SDL_GL_MakeCurrent) -- RHI_GL_Impl::Init() does not duplicate that; it assumes a
// current GL context and just records the frame dimensions.

#include "stdafx.h"
#include "Render/RHI/RHI.h"
#include "Render/Textures/ZzzOpenglUtil.h"  // PlatformSwapBuffers()
#include "Render/Core/BindState.h"          // BindVAO() / InvalidateVAOCache()
#include "Core/Utilities/FrameProfiler.h"
#include "Core/Utilities/Log/ErrorReport.h"
#include <SDL3/SDL.h>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <cstdio>

using namespace RHI;

namespace RHI_GL_Impl {

namespace {
    int g_Width = 0;
    int g_Height = 0;
    bool g_Initialized = false;

    // ---- GLP-08: capability probe ----
    Caps g_Caps;

    #ifndef APIENTRY
    #define APIENTRY
    #endif
    typedef const GLubyte* (APIENTRY* PFNGLGETSTRINGIPROC)(GLenum name, GLuint index);

    bool HasGLExtension(const char* name, PFNGLGETSTRINGIPROC fn_glGetStringi, GLint numExtensions)
    {
        if (!fn_glGetStringi) return false;
        for (GLint i = 0; i < numExtensions; i++)
        {
            const GLubyte* ext = fn_glGetStringi(GL_EXTENSIONS, (GLuint)i);
            if (ext && strcmp((const char*)ext, name) == 0) return true;
        }
        return false;
    }

    bool AtLeastGLVersion(int major, int minor, int reqMajor, int reqMinor)
    {
        if (major != reqMajor) return major > reqMajor;
        return minor >= reqMinor;
    }

    // Both the version/extension AND every listed entry point must resolve, or the flag stays
    // false -- a driver can report a high version and still hand back null for a symbol on a
    // bad install (RHI.h's Caps contract). Logs which half failed, since "false" alone doesn't
    // say whether it's a genuinely old driver or a broken one.
    bool ProbeOneCap(const wchar_t* label, bool versionOrExtPresent,
                      const char* fn1, const char* fn2 = nullptr)
    {
        const bool fn1Resolved = fn1 ? (SDL_GL_GetProcAddress(fn1) != nullptr) : true;
        const bool fn2Resolved = fn2 ? (SDL_GL_GetProcAddress(fn2) != nullptr) : true;
        const bool fnsResolved = fn1Resolved && fn2Resolved;

        if (versionOrExtPresent && !fnsResolved)
        {
            g_ErrorReport.Write(L"[RHI_GL Caps] %ls: version/extension present but an entry point "
                                 L"did not resolve (bad driver install?)\r\n", label);
        }
        return versionOrExtPresent && fnsResolved;
    }

    void ProbeCaps()
    {
        g_Caps = Caps{};

        const char* versionStr = (const char*)glGetString(GL_VERSION);
        int major = 0, minor = 0;
        if (versionStr) sscanf(versionStr, "%d.%d", &major, &minor);
        g_Caps.glMajor = major;
        g_Caps.glMinor = minor;

        PFNGLGETSTRINGIPROC fn_glGetStringi = (PFNGLGETSTRINGIPROC)SDL_GL_GetProcAddress("glGetStringi");
        GLint numExtensions = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

        g_Caps.bufferStorage = ProbeOneCap(L"bufferStorage",
            AtLeastGLVersion(major, minor, 4, 4) || HasGLExtension("GL_ARB_buffer_storage", fn_glGetStringi, numExtensions),
            "glBufferStorage");

        g_Caps.vertexAttribBinding = ProbeOneCap(L"vertexAttribBinding",
            AtLeastGLVersion(major, minor, 4, 3) || HasGLExtension("GL_ARB_vertex_attrib_binding", fn_glGetStringi, numExtensions),
            "glBindVertexBuffer", "glVertexAttribFormat");

        g_Caps.programBinary = ProbeOneCap(L"programBinary",
            AtLeastGLVersion(major, minor, 4, 1) || HasGLExtension("GL_ARB_get_program_binary", fn_glGetStringi, numExtensions),
            "glGetProgramBinary", "glProgramBinary");

        g_Caps.timerQuery = ProbeOneCap(L"timerQuery",
            AtLeastGLVersion(major, minor, 3, 3) || HasGLExtension("GL_ARB_timer_query", fn_glGetStringi, numExtensions),
            "glQueryCounter", "glGetQueryObjectui64v");

        GLint uboAlign = 0;
        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uboAlign);
        if (uboAlign > 0) g_Caps.uboOffsetAlignment = uboAlign;

        GLint maxBlockSize = 0;
        glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxBlockSize);
        if (maxBlockSize > 0) g_Caps.maxUniformBlockSize = maxBlockSize;
    }
}

const Caps& GetCaps()
{
    return g_Caps;
}

bool Init(void* /*nativeWindowHandle*/, int width, int height)
{
    // nativeWindowHandle is unused on the GL backend -- the SDL-owned GL context
    // is already current by the time any RHI:: call can happen.
    g_Width = width;
    g_Height = height;
    g_Initialized = true;

    ProbeCaps(); // GLP-08 -- nothing consumes g_Caps yet; ErrorReport::WriteOpenGLInfo() logs it.

    return true;
}

void Shutdown()
{
    // GL context teardown stays in Winmain.cpp (SDL_GL_DeleteContext) until
    // device ownership itself moves behind RHI -- nothing owned here to release.
    g_Initialized = false;
}

void BeginFrame()
{
    // Reserved for per-frame backend prep once a caller migrates to it
    // (e.g. GL has none today; D3D11 will need its per-frame context reset).
}

void EndFrame()
{
    PlatformSwapBuffers();
}

void SetViewport(int x, int y, int w, int h)
{
    // Deliberately raw glViewport, NOT glViewport2() -- glViewport2 also
    // recomputes camera/perspective state (CameraProjection::SetViewport),
    // which is a game-level concern, not a device-level one. RHI::SetViewport
    // is the device-level primitive glViewport2 will eventually be built on
    // top of, once its caller (ZzzOpenglUtil.cpp) migrates in a later increment.
    glViewport(x, y, w, h);
    g_Width = w;
    g_Height = h;
}

void OnResize(int /*width*/, int /*height*/)
{
    // No-op: GL never needed resize-time work (viewport is re-applied per-draw via
    // glViewport2(), not tracked here) -- matches pre-DXP-13 behavior exactly.
}

void Clear(bool color, bool depth, float r, float g, float b, float a)
{
    if (color) {
        glClearColor(r, g, b, a);
    }
    GLbitfield mask = 0;
    if (color) mask |= GL_COLOR_BUFFER_BIT;
    if (depth) mask |= GL_DEPTH_BUFFER_BIT;
    if (mask != 0) {
        glClear(mask);
    }
}

// ---- Buffers + Uniform blocks ----
// Self-contained function-pointer loading, matching the per-file convention used by
// GlobalUBO.cpp/BoneUBO.cpp/SceneUBO.cpp/ImmediateRenderer.cpp/PlanarShadowShader.cpp
// elsewhere in Render/ -- each file loads its own extension entry points via
// SDL_GL_GetProcAddress rather than relying on glewInit() having run.
#ifndef APIENTRY
#define APIENTRY
#endif

typedef void (APIENTRY* PFNGLGENBUFFERSPROC)(GLsizei n, GLuint* buffers);
typedef void (APIENTRY* PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void (APIENTRY* PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
typedef void (APIENTRY* PFNGLBUFFERSUBDATAPROC)(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
typedef void (APIENTRY* PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint* buffers);
typedef void (APIENTRY* PFNGLBINDBUFFERBASEPROC)(GLenum target, GLuint index, GLuint buffer);

namespace {
    PFNGLGENBUFFERSPROC     fn_glGenBuffers     = nullptr;
    PFNGLBINDBUFFERPROC     fn_glBindBuffer     = nullptr;
    PFNGLBUFFERDATAPROC     fn_glBufferData     = nullptr;
    PFNGLBUFFERSUBDATAPROC  fn_glBufferSubData  = nullptr;
    PFNGLDELETEBUFFERSPROC  fn_glDeleteBuffers  = nullptr;
    PFNGLBINDBUFFERBASEPROC fn_glBindBufferBase = nullptr;

    bool LoadBufferGLFunctions()
    {
        static bool loaded = false;
        if (loaded) return true;
        fn_glGenBuffers     = (PFNGLGENBUFFERSPROC)SDL_GL_GetProcAddress("glGenBuffers");
        fn_glBindBuffer     = (PFNGLBINDBUFFERPROC)SDL_GL_GetProcAddress("glBindBuffer");
        fn_glBufferData     = (PFNGLBUFFERDATAPROC)SDL_GL_GetProcAddress("glBufferData");
        fn_glBufferSubData  = (PFNGLBUFFERSUBDATAPROC)SDL_GL_GetProcAddress("glBufferSubData");
        fn_glDeleteBuffers  = (PFNGLDELETEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteBuffers");
        fn_glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)SDL_GL_GetProcAddress("glBindBufferBase");
        loaded = (fn_glGenBuffers != nullptr && fn_glBindBuffer != nullptr &&
                  fn_glBufferData != nullptr && fn_glBufferSubData != nullptr &&
                  fn_glDeleteBuffers != nullptr && fn_glBindBufferBase != nullptr);
        return loaded;
    }

    // Allocated capacity per vertex/index buffer, for UpdateBuffer's auto-grow path --
    // mirrors ImmediateRenderer.cpp's g_VBOCapacity pattern (grow to 2x needed size on overflow).
    std::unordered_map<uint32_t, GLsizeiptr> g_BufferCapacity;

    // Running write offset per buffer, AppendBuffer-only (ring-buffer bookkeeping formerly owned
    // by ImmediateRenderer.cpp's g_VBOWriteOffset, DXP-26). Not touched by UpdateBuffer.
    std::unordered_map<uint32_t, GLsizeiptr> g_BufferWriteOffset;

    BufferHandle CreateBufferImpl(GLenum target, const void* initialData, size_t sizeBytes, BufferUsage usage)
    {
        if (!LoadBufferGLFunctions()) return {};
        GLuint id = 0;
        fn_glGenBuffers(1, &id);
        fn_glBindBuffer(target, id);
        fn_glBufferData(target, (GLsizeiptr)sizeBytes, initialData,
                         usage == BufferUsage::Static ? GL_STATIC_DRAW : GL_STREAM_DRAW);
        FrameProfiler::CountGLCall(FrameProfiler::Counter::BufferUpdates);
        fn_glBindBuffer(target, 0);
        g_BufferCapacity[id] = (GLsizeiptr)sizeBytes;
        return BufferHandle{ id };
    }
}

BufferHandle CreateVertexBuffer(const void* initialData, size_t sizeBytes, BufferUsage usage)
{
    return CreateBufferImpl(GL_ARRAY_BUFFER, initialData, sizeBytes, usage);
}

BufferHandle CreateIndexBuffer(const void* initialData, size_t sizeBytes, BufferUsage usage)
{
    return CreateBufferImpl(GL_ELEMENT_ARRAY_BUFFER, initialData, sizeBytes, usage);
}

void UpdateBuffer(BufferHandle handle, const void* data, size_t sizeBytes)
{
    // Dynamic-only per the header contract; GL buffer objects have no fixed target, so
    // binding as GL_ARRAY_BUFFER here is valid regardless of whether this handle was created
    // via CreateVertexBuffer or CreateIndexBuffer.
    if (!handle.IsValid() || !LoadBufferGLFunctions()) return;
    GLsizeiptr& capacity = g_BufferCapacity[handle.id];
    fn_glBindBuffer(GL_ARRAY_BUFFER, handle.id);
    if ((GLsizeiptr)sizeBytes > capacity)
    {
        capacity = (GLsizeiptr)sizeBytes * 2;
        fn_glBufferData(GL_ARRAY_BUFFER, capacity, nullptr, GL_STREAM_DRAW);
        FrameProfiler::CountGLCall(FrameProfiler::Counter::BufferUpdates);
        FrameProfiler::TagBufferOrphan();
    }
    fn_glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)sizeBytes, data);
    FrameProfiler::CountGLCall(FrameProfiler::Counter::BufferUpdates);
    fn_glBindBuffer(GL_ARRAY_BUFFER, 0);
}

size_t AppendBuffer(BufferHandle handle, const void* data, size_t sizeBytes)
{
    // Ring-buffer append (DXP-26 pattern, moved here from ImmediateRenderer.cpp verbatim):
    // write at the current offset instead of always offset 0, so consecutive appends don't
    // overwrite data the GPU may still be reading from a previous draw. Grow (doesn't fit even
    // in an empty buffer) or wrap (fits, but not from the current write position -- a same-size
    // respecify hands back a fresh backing allocation) and restart at 0 when needed.
    if (!handle.IsValid() || !LoadBufferGLFunctions()) return 0;

    GLsizeiptr& capacity = g_BufferCapacity[handle.id];
    GLsizeiptr& writeOffset = g_BufferWriteOffset[handle.id];
    const GLsizeiptr neededSize = (GLsizeiptr)sizeBytes;

    fn_glBindBuffer(GL_ARRAY_BUFFER, handle.id);

    if (neededSize > capacity)
    {
        capacity = neededSize * 2;
        fn_glBufferData(GL_ARRAY_BUFFER, capacity, nullptr, GL_STREAM_DRAW);
        FrameProfiler::CountGLCall(FrameProfiler::Counter::BufferUpdates);
        FrameProfiler::TagBufferOrphan();
        writeOffset = 0;
    }
    else if (writeOffset + neededSize > capacity)
    {
        fn_glBufferData(GL_ARRAY_BUFFER, capacity, nullptr, GL_STREAM_DRAW);
        FrameProfiler::CountGLCall(FrameProfiler::Counter::BufferUpdates);
        FrameProfiler::TagBufferOrphan();
        writeOffset = 0;
    }

    fn_glBufferSubData(GL_ARRAY_BUFFER, writeOffset, neededSize, data);
    FrameProfiler::CountGLCall(FrameProfiler::Counter::BufferUpdates);
    fn_glBindBuffer(GL_ARRAY_BUFFER, 0);

    const GLsizeiptr offsetWritten = writeOffset;
    writeOffset += neededSize;
    return (size_t)offsetWritten;
}

void DestroyBuffer(BufferHandle handle)
{
    if (!handle.IsValid() || !LoadBufferGLFunctions()) return;
    GLuint id = handle.id;
    fn_glDeleteBuffers(1, &id);
    g_BufferCapacity.erase(handle.id);
    g_BufferWriteOffset.erase(handle.id);
}

BufferHandle CreateUniformBlock(size_t sizeBytes, int bindingSlot)
{
    if (!LoadBufferGLFunctions()) return {};
    GLuint id = 0;
    fn_glGenBuffers(1, &id);
    fn_glBindBuffer(GL_UNIFORM_BUFFER, id);
    fn_glBufferData(GL_UNIFORM_BUFFER, (GLsizeiptr)sizeBytes, nullptr, GL_DYNAMIC_DRAW);
    fn_glBindBufferBase(GL_UNIFORM_BUFFER, (GLuint)bindingSlot, id);
    fn_glBindBuffer(GL_UNIFORM_BUFFER, 0);
    return BufferHandle{ id };
}

void UpdateUniformBlock(BufferHandle handle, const void* data, size_t sizeBytes)
{
    // Whole-block update, per the header contract -- callers keep their own dirty-checking.
    if (!handle.IsValid() || !LoadBufferGLFunctions()) return;
    fn_glBindBuffer(GL_UNIFORM_BUFFER, handle.id);
    // GLS-08: orphan the previous GPU allocation before writing, so the driver hands back a
    // fresh, un-synchronized memory block instead of possibly stalling the CPU until the GPU
    // finishes reading the old contents from a draw call still in flight -- matches the D3D11
    // backend's Map(WRITE_DISCARD) semantics that BoneUBO::UploadBones()'s comment already
    // documents as this function's implicit contract. Safe because every caller (GlobalUBO/
    // SceneUBO/BoneUBO/BMDMeshShader's BMDFlagsCB) fully repacks and uploads its whole buffer
    // every call -- no partial-write caller exists that this could break.
    fn_glBufferData(GL_UNIFORM_BUFFER, (GLsizeiptr)sizeBytes, nullptr, GL_DYNAMIC_DRAW);
    FrameProfiler::CountGLCall(FrameProfiler::Counter::BufferUpdates);
    FrameProfiler::TagBufferOrphan();
    fn_glBufferSubData(GL_UNIFORM_BUFFER, 0, (GLsizeiptr)sizeBytes, data);
    FrameProfiler::CountGLCall(FrameProfiler::Counter::BufferUpdates);
    fn_glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void DestroyUniformBlock(BufferHandle handle)
{
    if (!handle.IsValid() || !LoadBufferGLFunctions()) return;
    GLuint id = handle.id;
    fn_glDeleteBuffers(1, &id);
}

// ---- Pipeline / blend-mode state ----
// Deliberately delegates to the EXISTING ZzzOpenglUtil.cpp wrapper-monopoly functions
// (EnableAlphaBlend() family, EnableDepthTest()/DisableDepthTest()) rather than reimplementing
// the GL calls with a second, independent state cache. Those wrappers already own the single
// source of truth for AlphaBlendType/CullFaceEnable/DepthMaskEnable/AlphaTestEnable (DXP-10's
// wrapper monopoly exists for exactly this reason). Per-subsystem RHI migration (Approach step 3)
// means legacy call sites and RHI call sites will coexist for a while; if RHI_GL kept its own
// last-set-combo cache, it could drift out of sync with GL's real state whenever a legacy wrapper
// call ran in between two RHI::SetBlendMode calls, causing a stale-state visual bug. Delegating
// keeps exactly one cache, so migration order can't matter.
void SetBlendMode(BlendMode mode)
{
    switch (mode)
    {
    case BlendMode::Opaque:    DisableAlphaBlend();     break;
    case BlendMode::LightMap:  EnableLightMap();        break;
    case BlendMode::AlphaTest: EnableAlphaTest();       break;
    case BlendMode::Additive:  EnableAlphaBlend();      break;
    case BlendMode::Minus:     EnableAlphaBlendMinus(); break;
    case BlendMode::Blend2:    EnableAlphaBlend2();     break;
    case BlendMode::Blend3:    EnableAlphaBlend3();     break;
    case BlendMode::Blend4:    EnableAlphaBlend4();     break;
    }
}

void SetDepthTestEnabled(bool enabled)
{
    if (enabled) EnableDepthTest();
    else         DisableDepthTest();
}

// DXP-15 increment 3: standalone toggles for the direct EnableCullFace/DisableCullFace/
// EnableDepthMask/DisableDepthMask call sites outside the blend-mode wrapper family -- same
// delegation reasoning as SetBlendMode/SetDepthTestEnabled above (one state cache, owned by
// ZzzOpenglUtil.cpp, not duplicated here).
void SetCullEnabled(bool enabled)
{
    if (enabled) EnableCullFace();
    else         DisableCullFace();
}

void SetDepthWriteEnabled(bool enabled)
{
    if (enabled) EnableDepthMask();
    else         DisableDepthMask();
}

void SetFogEnabled(bool enabled)
{
    // FogEnable is the same extern the Enable*() blend wrappers already read as their fog
    // side-effect switch, and the same global scene code assigns to directly today
    // (CharacterScene.cpp/LoginScene.cpp/MainScene.cpp/etc.) -- RHI::SetFogEnabled is just the
    // RHI-shaped equivalent for call sites that migrate later, not a new mechanism.
    FogEnable = enabled;
}

void SetPolygonOffset(bool enabled, float factor, float units)
{
    // GL_POLYGON_OFFSET_FILL is core-profile-safe (unlike GL_ALPHA_TEST/GL_FOG/GL_TEXTURE_2D
    // above), so no g_CoreProfile guard here -- matches PlanarShadowShader.cpp's existing raw
    // usage, the one caller of this today.
    if (enabled)
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(factor, units);
    }
    else
    {
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
}

// ---- Vertex layout + binding ----
typedef void (APIENTRY* PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint* arrays);
typedef void (APIENTRY* PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint* arrays);
typedef void (APIENTRY* PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
typedef void (APIENTRY* PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);

namespace {
    PFNGLGENVERTEXARRAYSPROC         fn_glGenVertexArrays         = nullptr;
    PFNGLDELETEVERTEXARRAYSPROC      fn_glDeleteVertexArrays      = nullptr;
    PFNGLVERTEXATTRIBPOINTERPROC     fn_glVertexAttribPointer     = nullptr;
    PFNGLENABLEVERTEXATTRIBARRAYPROC fn_glEnableVertexAttribArray = nullptr;

    bool LoadVertexLayoutGLFunctions()
    {
        static bool loaded = false;
        if (loaded) return true;
        fn_glGenVertexArrays         = (PFNGLGENVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glGenVertexArrays");
        fn_glDeleteVertexArrays      = (PFNGLDELETEVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glDeleteVertexArrays");
        fn_glVertexAttribPointer     = (PFNGLVERTEXATTRIBPOINTERPROC)SDL_GL_GetProcAddress("glVertexAttribPointer");
        fn_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)SDL_GL_GetProcAddress("glEnableVertexAttribArray");
        loaded = (fn_glGenVertexArrays != nullptr && fn_glDeleteVertexArrays != nullptr &&
                  fn_glVertexAttribPointer != nullptr && fn_glEnableVertexAttribArray != nullptr);
        return loaded;
    }

    // One VAO per VertexLayout value (design doc: "RHI_GL maps each enum value to a prebuilt
    // VAO config"), not per buffer -- attrib pointers are captured into the VAO once, at
    // configure time, so a layout's VAO only needs reconfiguring if the buffer bound to it
    // changes (never happens for IR::'s single persistent streaming VBO).
    constexpr int kVertexLayoutCount = 4; // PosUvColor, BMDMesh, Terrain, PosOnly
    GLuint   g_LayoutVAO[kVertexLayoutCount]           = {};
    uint32_t g_LayoutBoundBufferId[kVertexLayoutCount] = {};

    void ConfigurePosUvColorVAO(GLuint vbo)
    {
        // pos3+uv2+rgba4, 36B stride -- IR::/PassthroughShader's only vertex format
        // (ImmediateRenderer.cpp's IRVertex; mirrored here rather than shared, since RHI
        // doesn't depend on IR::'s internal type).
        const GLsizei stride = sizeof(float) * 9;
        fn_glBindBuffer(GL_ARRAY_BUFFER, vbo);
        fn_glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        fn_glEnableVertexAttribArray(0);
        fn_glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
        fn_glEnableVertexAttribArray(1);
        fn_glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 5));
        fn_glEnableVertexAttribArray(2);
        fn_glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void BindVertexBuffer(BufferHandle handle, VertexLayout layout)
{
    if (!handle.IsValid()) return;
    const int idx = (int)layout;
    if (idx < 0 || idx >= kVertexLayoutCount) return;

    if (layout != VertexLayout::PosUvColor)
    {
        // Not yet implemented -- lands with the terrain/BMD/shadow subsystem conversions
        // (Approach step 3). Logged once per layout so a premature call is visible immediately
        // rather than silently rendering nothing.
        static bool warned[kVertexLayoutCount] = {};
        if (!warned[idx])
        {
            g_ErrorReport.Write(L"[RHI_GL] BindVertexBuffer: VertexLayout %d not yet implemented\r\n", idx);
            warned[idx] = true;
        }
        return;
    }

    if (!LoadBufferGLFunctions() || !LoadVertexLayoutGLFunctions()) return;

    if (g_LayoutVAO[idx] == 0)
    {
        fn_glGenVertexArrays(1, &g_LayoutVAO[idx]);
    }

    if (g_LayoutBoundBufferId[idx] != handle.id)
    {
        BindVAO(g_LayoutVAO[idx]);
        ConfigurePosUvColorVAO(handle.id);
        g_LayoutBoundBufferId[idx] = handle.id;
        // Falls through to the BindVAO below -- redundant with the one just above (already
        // dirty-checked cache) but keeps this function's exit path single instead of doubling
        // the bind call site.
    }

    BindVAO(g_LayoutVAO[idx]);
}

void BindIndexBuffer(BufferHandle /*handle*/)
{
    // Not yet implemented -- no caller in the tree yet (lands with terrain's indexed draw).
}

// ---- Draw ----
void Draw(Topology topology, uint32_t vertexCount, uint32_t firstVertex)
{
    GLenum mode;
    switch (topology)
    {
    case Topology::TriangleList: mode = GL_TRIANGLES;  break;
    case Topology::LineList:     mode = GL_LINES;      break;
    case Topology::LineStrip:    mode = GL_LINE_STRIP; break;
    default: return;
    }
    glDrawArrays(mode, (GLint)firstVertex, (GLsizei)vertexCount);
    FrameProfiler::CountGLCall(FrameProfiler::Counter::DrawCalls);
}

void DrawIndexed(Topology /*topology*/, uint32_t /*indexCount*/, uint32_t /*firstIndex*/)
{
    // Not yet implemented -- lands with a subsystem that needs indexed draws (terrain).
}

// ---- Textures ----
// glGenTextures/glDeleteTextures/glTexImage2D/glTexSubImage2D/glTexParameteri are all GL 1.1/1.3
// core (exported directly by opengl32.dll), same as glBindTexture in BindState.cpp -- no
// SDL_GL_GetProcAddress loading needed here.
TextureHandle CreateTexture(const TextureDesc& desc, const void* initialPixelsRGBA)
{
    GLuint id = 0;
    glGenTextures(1, &id);
    BindTexture2D(0, id);

    const GLint filter = (desc.filter == TexFilter::Linear) ? GL_LINEAR : GL_NEAREST;
    const GLint wrap    = (desc.wrap == TexWrap::Repeat) ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, desc.width, desc.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, initialPixelsRGBA);

    return TextureHandle{ id };
}

void UpdateTexture(TextureHandle handle, int x, int y, int w, int h, const void* pixelsRGBA)
{
    if (!handle.IsValid()) return;
    BindTexture2D(0, handle.id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRGBA);
}

void DestroyTexture(TextureHandle handle)
{
    if (!handle.IsValid()) return;
    GLuint id = handle.id;
    glDeleteTextures(1, &id);
    // Matches BindState.h's documented contract: a delete affecting a cached bind must
    // invalidate the cache, or a later Gen reusing this numeric id would be wrongly treated as
    // already-bound (the exact bug class that produced DXP-22's "infinity shadow").
    InvalidateTextureCache();
    InvalidateActiveTextureUnitCache();
}

void BindTexture(TextureHandle handle, int slot)
{
    BindTexture2D(slot, handle.IsValid() ? handle.id : 0);
}

// ---- Readback ----
bool ReadDepthPixel(int x, int y, float* outDepth)
{
    if (!outDepth) return false;
    glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, outDepth);
    return true;
}

bool ReadColorFramebuffer(int x, int y, int w, int h, void* outRGB)
{
    if (!outRGB || w <= 0 || h <= 0) return false;

    // glReadPixels is bottom-up; the RHI contract is top-down (RHI.h's ReadColorFramebuffer
    // comment) so every caller gets the same row order on both backends. Read directly into
    // outRGB, then flip rows in place -- one extra row-sized temp buffer, no second full-image copy.
    unsigned char* rows = static_cast<unsigned char*>(outRGB);
    glReadPixels(x, y, w, h, GL_RGB, GL_UNSIGNED_BYTE, rows);

    const size_t rowBytes = (size_t)w * 3;
    std::vector<unsigned char> temp(rowBytes);
    for (int top = 0, bottom = h - 1; top < bottom; ++top, --bottom)
    {
        unsigned char* rowTop    = rows + (size_t)top * rowBytes;
        unsigned char* rowBottom = rows + (size_t)bottom * rowBytes;
        memcpy(temp.data(), rowTop, rowBytes);
        memcpy(rowTop, rowBottom, rowBytes);
        memcpy(rowBottom, temp.data(), rowBytes);
    }
    return true;
}

} // namespace RHI_GL_Impl
