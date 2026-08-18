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
#include "Render/Core/BindState.h"
#include "Render/Core/ImmediateRenderer.h" // GLP-19 -- IR::Flush() before texture content changes          // BindVAO() / InvalidateVAOCache()
#include "Core/Utilities/FrameProfiler.h"
#include "Core/Utilities/Log/ErrorReport.h"
#include <SDL3/SDL.h>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <cstdio>
#include <cassert>

using namespace RHI;

namespace RHI_GL_Impl {

namespace {
    int g_Width = 0;
    int g_Height = 0;
    bool g_Initialized = false;

    // ---- GLP-08: capability probe ----
    Caps g_Caps;
    DriverInfo g_DriverInfo;

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

    // glGetString returns null on a lost or not-yet-current context; an empty field says
    // "the driver did not tell us" rather than crashing a diagnostic path.
    std::string GLString(GLenum name)
    {
        const GLubyte* value = glGetString(name);
        return value ? std::string((const char*)value) : std::string();
    }

    void ProbeDriverInfo()
    {
        g_DriverInfo.vendor = GLString(GL_VENDOR);
        g_DriverInfo.renderer = GLString(GL_RENDERER);
        g_DriverInfo.version = GLString(GL_VERSION);
        g_DriverInfo.shadingLanguageVersion = GLString(GL_SHADING_LANGUAGE_VERSION);
    }

    void ProbeCaps()
    {
        g_Caps = Caps{};
        ProbeDriverInfo();

        const char* versionStr = g_DriverInfo.version.empty() ? nullptr : g_DriverInfo.version.c_str();
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

// GLP-09 -- forward declarations. Implementations live alongside the rest of the UBO ring
// machinery, further down this file next to CreateUniformBlock (needs the UboRing type defined
// there); BeginFrame()/Shutdown() stay up here to match RHI.h's Device/frame section ordering.
namespace {
    void AdvanceUboRings();
    void DestroyAllUboRings();
    void ResetAppendWrapCounters(); // GLP-25 -- defined next to AppendBuffer's bookkeeping maps

    // Ring-path uniform-block handles are slot RESERVATIONS, not GL object names -- but they ride
    // in the same BufferHandle type as real vertex/index buffers, whose GL names also start at 1.
    // Without a tag the two id spaces overlap, and DestroyBuffer(uboHandle) would glDeleteBuffers
    // an unrelated live buffer that happens to share the number. Tagging the high bit keeps them
    // disjoint (no driver hands out names anywhere near 2^31) and lets the vertex/index-buffer
    // entry points below reject a misrouted uniform-block handle outright instead of silently
    // corrupting an unrelated buffer.
    // NOTE: the legacy rung is deliberately NOT tagged -- LegacyCreateUniformBlock returns a real
    // GL buffer name, and its Update/Destroy paths use it as one.
    constexpr uint32_t kUboReservationTag = 0x80000000u;
    inline bool IsUboReservation(BufferHandle h) { return (h.id & kUboReservationTag) != 0; }
}

const Caps& GetCaps()
{
    return g_Caps;
}

const DriverInfo& GetDriverInfo()
{
    return g_DriverInfo;
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
    DestroyAllUboRings(); // GLP-09 -- release the persistent-mapped ring buffers, if any exist.
    // GL context teardown stays in Winmain.cpp (SDL_GL_DeleteContext) until
    // device ownership itself moves behind RHI -- nothing owned here to release.
    g_Initialized = false;
}

void BeginFrame()
{
    // GLP-09: advance each active UBO ring's segment once per frame, fencing the segment just
    // finished and waiting on the segment about to be reused (see AdvanceUboRings() below for
    // why this is anchored to BeginFrame() rather than EndFrame() -- the short version is that
    // EndFrame() has no call site in the game loop today).
    AdvanceUboRings();

    // GLP-25: clear the per-frame wrap tally that drives AppendBuffer's grow-on-repeated-wrap
    // policy. Deliberately NOT hung off FrameProfiler::ResetCounters() -- that is instrumentation
    // and the growth policy has to work identically with $glstats off.
    ResetAppendWrapCounters();
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

    // GLP-25: how many times each buffer has wrapped so far THIS frame. Reset from BeginFrame().
    std::unordered_map<uint32_t, uint32_t> g_BufferWrapCount;

    // Wrap this many times in one frame and the buffer is undersized for the frame's streaming
    // volume (as opposed to for one large append) -- grow instead of respecifying at the same size.
    // 2 rather than 1: a single wrap is the normal steady state for a correctly-sized ring.
    constexpr uint32_t kWrapsBeforeGrow = 2;
    // Ceiling on wrap-driven growth only. A single append larger than this still grows past it --
    // that path is a correctness requirement (the data must fit), not a tuning knob.
    constexpr GLsizeiptr kMaxWrapGrowthCapacity = 16 * 1024 * 1024;

    void ResetAppendWrapCounters()
    {
        for (auto& kv : g_BufferWrapCount) kv.second = 0;
    }

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
    if (IsUboReservation(handle)) return; // uniform-block handle misrouted here -- see UpdateUniformBlock
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
    if (IsUboReservation(handle)) return 0; // uniform-block handle misrouted here -- see UpdateUniformBlock

    GLsizeiptr& capacity = g_BufferCapacity[handle.id];
    GLsizeiptr& writeOffset = g_BufferWriteOffset[handle.id];
    const GLsizeiptr neededSize = (GLsizeiptr)sizeBytes;

    fn_glBindBuffer(GL_ARRAY_BUFFER, handle.id);

    if (neededSize > capacity)
    {
        // Doesn't fit even in an empty buffer. Uncapped by design -- the append has to fit.
        capacity = neededSize * 2;
        fn_glBufferData(GL_ARRAY_BUFFER, capacity, nullptr, GL_STREAM_DRAW);
        FrameProfiler::CountGLCall(FrameProfiler::Counter::BufferUpdates);
        FrameProfiler::TagBufferOrphan();
        writeOffset = 0;
    }
    else if (writeOffset + neededSize > capacity)
    {
        // GLP-25: this branch used to respecify at the SAME capacity, so a buffer that wrapped
        // every frame could never grow out of it -- capacity was set only by the largest single
        // append and never by how much a frame actually streams. Those are different quantities:
        // writeOffset is deliberately never reset per frame (that is what keeps an append from
        // overwriting data an in-flight draw may still be reading), so capacity has to cover a
        // whole frame's appends. On Intel HD 530 the IR VBO wrapped ~23 times in a single
        // skill-cast frame at its 144 KB startup size.
        uint32_t& wraps = g_BufferWrapCount[handle.id];
        wraps++;
        // `==`, not `>=`: grow at most once per frame. With `>=`, a single pathological frame (the
        // HD 530 capture wrapped 23 times) would double on every wrap after the first and slam
        // straight into the cap. Growing once per frame still converges in a handful of frames and
        // settles at a size the workload actually justifies.
        if (wraps == kWrapsBeforeGrow && capacity < kMaxWrapGrowthCapacity)
        {
            const GLsizeiptr grown = capacity * 2;
            capacity = (grown > kMaxWrapGrowthCapacity) ? kMaxWrapGrowthCapacity : grown;
        }
        // The orphaning respecify stays either way -- it is what lets the driver hand back fresh
        // storage instead of stalling until in-flight draws finish reading the old range. Growing
        // reduces how OFTEN this happens; it must never remove the call.
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
    // A ring-path uniform-block handle is a slot reservation, not a GL name -- deleting by its
    // raw value would destroy whatever real buffer shares that number. Route to
    // DestroyUniformBlock() instead; rejecting here keeps the mistake inert.
    if (IsUboReservation(handle)) return;
    GLuint id = handle.id;
    fn_glDeleteBuffers(1, &id);
    g_BufferCapacity.erase(handle.id);
    g_BufferWriteOffset.erase(handle.id);
    g_BufferWrapCount.erase(handle.id); // GLP-25 -- same lifetime as the two maps above
}

// ---- Uniform blocks: GLP-09 per-slot ring allocator ----
// Replaces GLS-08's per-update orphan-then-glBufferSubData (a driver-side allocation plus a
// binding-point re-emit on EVERY call -- for BMDFlagsCB, once per BMD mesh draw, hundreds of
// times per frame per client) with a persistently-mapped ring buffer PER BINDING SLOT,
// sub-allocated with glBindBufferRange: a memcpy and one GL call per update instead of four.
// See GLP-09-ubo-ring-allocator.md for the full design rationale and risk notes.
typedef GLsync    (APIENTRY* PFNGLFENCESYNCPROC)(GLenum condition, GLbitfield flags);
typedef GLenum    (APIENTRY* PFNGLCLIENTWAITSYNCPROC)(GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void      (APIENTRY* PFNGLDELETESYNCPROC)(GLsync sync);
typedef void      (APIENTRY* PFNGLBUFFERSTORAGEPROC)(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags);
typedef void*     (APIENTRY* PFNGLMAPBUFFERRANGEPROC)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef GLboolean (APIENTRY* PFNGLUNMAPBUFFERPROC)(GLenum target);
typedef void      (APIENTRY* PFNGLBINDBUFFERRANGEPROC)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);

namespace {
    PFNGLFENCESYNCPROC       fn_glFenceSync       = nullptr;
    PFNGLCLIENTWAITSYNCPROC  fn_glClientWaitSync  = nullptr;
    PFNGLDELETESYNCPROC      fn_glDeleteSync      = nullptr;
    PFNGLBUFFERSTORAGEPROC   fn_glBufferStorage   = nullptr;
    PFNGLMAPBUFFERRANGEPROC  fn_glMapBufferRange  = nullptr;
    PFNGLUNMAPBUFFERPROC     fn_glUnmapBuffer     = nullptr;
    PFNGLBINDBUFFERRANGEPROC fn_glBindBufferRange = nullptr;

    // Sync objects + glMapBufferRange/glBindBufferRange are core since GL 3.0/3.2, always
    // present given GLP-08's GL >= 3.3 floor -- loaded once, like every other entry-point group
    // in this file.
    bool LoadUboRingSyncFunctions()
    {
        static bool loaded = false;
        if (loaded) return true;
        fn_glFenceSync       = (PFNGLFENCESYNCPROC)SDL_GL_GetProcAddress("glFenceSync");
        fn_glClientWaitSync  = (PFNGLCLIENTWAITSYNCPROC)SDL_GL_GetProcAddress("glClientWaitSync");
        fn_glDeleteSync      = (PFNGLDELETESYNCPROC)SDL_GL_GetProcAddress("glDeleteSync");
        fn_glMapBufferRange  = (PFNGLMAPBUFFERRANGEPROC)SDL_GL_GetProcAddress("glMapBufferRange");
        fn_glUnmapBuffer     = (PFNGLUNMAPBUFFERPROC)SDL_GL_GetProcAddress("glUnmapBuffer");
        fn_glBindBufferRange = (PFNGLBINDBUFFERRANGEPROC)SDL_GL_GetProcAddress("glBindBufferRange");
        loaded = (fn_glFenceSync != nullptr && fn_glClientWaitSync != nullptr &&
                  fn_glDeleteSync != nullptr && fn_glMapBufferRange != nullptr &&
                  fn_glUnmapBuffer != nullptr && fn_glBindBufferRange != nullptr);
        return loaded;
    }

    // Only resolved when Caps::bufferStorage is true (GLP-08 already verified this exact entry
    // point resolves before setting that flag) -- its own loader so the fallback rung never
    // pays for a lookup it can't use.
    bool LoadBufferStorageFunction()
    {
        static bool loaded = false;
        if (loaded) return true;
        fn_glBufferStorage = (PFNGLBUFFERSTORAGEPROC)SDL_GL_GetProcAddress("glBufferStorage");
        loaded = (fn_glBufferStorage != nullptr);
        return loaded;
    }

    // Decided once, globally, at first use -- the capability probe (GLP-08) is fixed for the
    // process lifetime, so there's no scenario where some uniform blocks use the ring and
    // others use the legacy rung. Keeps CreateUniformBlock/UpdateUniformBlock/
    // DestroyUniformBlock from ever having to branch per-handle.
    bool g_UboRingSyncChecked = false;
    bool g_UboRingSyncAvailable = false;

    bool UboRingAvailable()
    {
        if (!g_UboRingSyncChecked)
        {
            g_UboRingSyncAvailable = LoadUboRingSyncFunctions();
            g_UboRingSyncChecked = true;
            if (!g_UboRingSyncAvailable)
            {
                g_ErrorReport.Write(L"[RHI_GL] GLP-09: UBO ring allocator unavailable "
                                     L"(glMapBufferRange/glFenceSync/glBindBufferRange failed to "
                                     L"resolve) -- falling back to GLS-08 per-update orphaning.\r\n");
            }
        }
        return g_UboRingSyncAvailable;
    }

    constexpr int kUboRingSegments = 3;

    struct UboRing
    {
        GLuint     bufferId       = 0;
        bool       persistent     = false;  // fast path: glBufferStorage + persistent-coherent map
        void*      mappedPtr      = nullptr; // fast path only; null on the fallback (map-per-update) path
        GLsizeiptr segmentSize    = 0;       // bytes per segment, already a multiple of uboOffsetAlignment
        GLsizeiptr writeOffset    = 0;       // offset within the CURRENT segment
        int        currentSegment = 0;
        GLsync     fence[kUboRingSegments] = {};
        int        bindingSlot   = -1;

        // GLP-10: content dirty-check shadow state. `lastFrameIndex == g_UboFrameIndex` is what
        // makes a skip safe with a ring buffer -- within a frame the ring only moves forward, so
        // a same-frame previous write is guaranteed not to have been wrapped over or reused by
        // this slot's own later traffic. Across frames the first update always re-uploads (cheap
        // insurance that removes the wrap/fence-reuse reasoning entirely -- see the task file).
        std::vector<uint8_t> lastBytes;
        GLsizeiptr lastOffset     = -1;
        uint64_t   lastFrameIndex = UINT64_MAX;
    };

    // One ring per binding slot (not one global ring) -- keyed by bindingSlot so each block's
    // alignment and lifetime stay independent (GLP-09's design). Handle id -> binding slot is a
    // separate map below, since UpdateUniformBlock/DestroyUniformBlock only see a handle.
    std::unordered_map<int, UboRing> g_UboRings;
    std::unordered_map<uint32_t, int> g_UboHandleSlot;
    uint32_t g_NextUboHandleId = 1; // handles here are slot reservations, not real GL object ids
                                    // (tagged with kUboReservationTag on return -- see above)

    // GLP-10: incremented once per frame in AdvanceUboRings() (itself called from BeginFrame(),
    // confirmed GLP-09 to run exactly once per frame -- SceneManager.cpp's RenderScene). Every
    // ring's dirty-check compares against this, not a per-ring counter, so "same frame" always
    // means the same thing across every slot.
    uint64_t g_UboFrameIndex = 0;

    GLsizeiptr AlignUp(GLsizeiptr value, GLsizeiptr alignment)
    {
        if (alignment <= 0) return value;
        return ((value + alignment - 1) / alignment) * alignment;
    }

    void DeleteRingFence(UboRing& ring, int segment)
    {
        if (ring.fence[segment] != nullptr)
        {
            fn_glDeleteSync(ring.fence[segment]);
            ring.fence[segment] = nullptr;
        }
    }

    // Zero-timeout wait first (the expected case with 3 segments -- see GLP-09's risk note: if
    // this ever genuinely blocks, the fix is a bigger ring, not more segments), falling through
    // to a real blocking wait only if the segment's last writer truly hasn't finished yet.
    void WaitRingSegment(UboRing& ring, int segment)
    {
        GLsync fence = ring.fence[segment];
        if (fence == nullptr) return;
        GLenum result = fn_glClientWaitSync(fence, 0, 0);
        if (result == GL_TIMEOUT_EXPIRED)
        {
            fn_glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000ULL /* 1s */);
        }
        DeleteRingFence(ring, segment);
    }

    void DestroyRingBuffer(UboRing& ring)
    {
        if (ring.bufferId == 0) return;
        if (ring.mappedPtr != nullptr)
        {
            fn_glBindBuffer(GL_UNIFORM_BUFFER, ring.bufferId);
            fn_glUnmapBuffer(GL_UNIFORM_BUFFER);
            fn_glBindBuffer(GL_UNIFORM_BUFFER, 0);
            ring.mappedPtr = nullptr;
        }
        for (int s = 0; s < kUboRingSegments; s++) DeleteRingFence(ring, s);
        GLuint id = ring.bufferId;
        fn_glDeleteBuffers(1, &id);
        ring.bufferId = 0;
    }

    // (Re)allocates the ring's backing buffer at `newSegmentSize` per segment, abandoning any
    // previous buffer -- the driver keeps an abandoned-but-still-referenced buffer alive under
    // the hood until commands that already bound a range of it have retired, the same deferred-
    // delete guarantee DestroyTexture() above already relies on for in-flight draws. Called at
    // first use for a slot and again whenever a segment overflows.
    void GrowRing(UboRing& ring, GLsizeiptr newSegmentSize)
    {
        DestroyRingBuffer(ring);

        ring.segmentSize = newSegmentSize;
        const GLsizeiptr totalSize = newSegmentSize * kUboRingSegments;

        GLuint id = 0;
        fn_glGenBuffers(1, &id);
        fn_glBindBuffer(GL_UNIFORM_BUFFER, id);

        if (g_Caps.bufferStorage && LoadBufferStorageFunction())
        {
            const GLbitfield storageFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
            fn_glBufferStorage(GL_UNIFORM_BUFFER, totalSize, nullptr, storageFlags);
            ring.mappedPtr = fn_glMapBufferRange(GL_UNIFORM_BUFFER, 0, totalSize, storageFlags);
            ring.persistent = (ring.mappedPtr != nullptr);
        }

        if (!ring.persistent)
        {
            // Fallback rung (Caps::bufferStorage == false, or the persistent map itself failed):
            // ordinary mutable storage, mapped per update in UpdateUniformBlock with
            // UNSYNCHRONIZED|INVALIDATE_RANGE instead of held persistently mapped. Still no
            // per-update allocation -- the buffer itself is sized once, here.
            fn_glBufferData(GL_UNIFORM_BUFFER, totalSize, nullptr, GL_DYNAMIC_DRAW);
            ring.mappedPtr = nullptr;
        }

        fn_glBindBuffer(GL_UNIFORM_BUFFER, 0);

        ring.bufferId = id;
        ring.writeOffset = 0;
        ring.currentSegment = 0;
        for (int s = 0; s < kUboRingSegments; s++) ring.fence[s] = nullptr;

        // GLP-10 shadow state describes a range inside the buffer we just destroyed, so it must
        // NOT survive a growth. UpdateUniformBlock calls EnsureRingCapacity() (which lands here)
        // BEFORE its content dirty-check -- without this reset, a same-frame byte-identical
        // update immediately after a growth would take the skip path and return early, skipping
        // both the write and the glBindBufferRange. The new buffer has never been written and
        // deleting the old one unbound it, so that slot would read an unbound/stale UBO for the
        // rest of the frame. Both halves of that sequence are ordinary: warm-up growth is
        // expected (see EnsureRingCapacity), and byte-identical successive uploads are exactly
        // what GLP-10 exists to skip (BMDFlagsCB, body + armor pieces).
        ring.lastBytes.clear();
        ring.lastOffset = -1;
        ring.lastFrameIndex = UINT64_MAX;
    }

    // Ensures the ring for `bindingSlot` exists and its current segment can hold one more
    // `alignedSize` block -- grows 2x-until-it-fits on overflow, matching AppendBuffer's policy.
    UboRing& EnsureRingCapacity(int bindingSlot, GLsizeiptr alignedSize)
    {
        UboRing& ring = g_UboRings[bindingSlot];
        ring.bindingSlot = bindingSlot;

        if (ring.bufferId == 0)
        {
            // First use: size for a handful of updates per frame. BMDFlags-heavy slots outgrow
            // this within their first few frames and settle -- the same curve AppendBuffer's
            // streaming VBO ring already exhibits.
            GrowRing(ring, alignedSize * 8);
        }
        else if (ring.writeOffset + alignedSize > ring.segmentSize)
        {
            GLsizeiptr newSegment = ring.segmentSize;
            while (newSegment < ring.writeOffset + alignedSize) newSegment *= 2;
            GrowRing(ring, newSegment);
        }

        return ring;
    }

    void AdvanceUboRings()
    {
        // Anchored to BeginFrame() rather than EndFrame(): RHI::EndFrame() (which wraps
        // PlatformSwapBuffers()) has no call site in the game loop today -- SceneManager.cpp,
        // LoadingScene.cpp and UIMng.cpp all call PlatformSwapBuffers() directly instead.
        // RHI::BeginFrame() IS called exactly once per frame (SceneManager.cpp's RenderScene),
        // right before that frame's draws are submitted, so fencing "the segment just finished"
        // here means "everything submitted last frame" -- functionally identical to fencing at
        // end-of-frame, without depending on wiring up a currently-dead entry point.
        g_UboFrameIndex++; // GLP-10: drives every ring's same-frame dirty-check window.
        for (auto& kv : g_UboRings)
        {
            UboRing& ring = kv.second;
            if (ring.bufferId == 0) continue;

            DeleteRingFence(ring, ring.currentSegment);
            ring.fence[ring.currentSegment] = fn_glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

            ring.currentSegment = (ring.currentSegment + 1) % kUboRingSegments;
            WaitRingSegment(ring, ring.currentSegment);
            ring.writeOffset = 0;
        }
    }

    void DestroyAllUboRings()
    {
        for (auto& kv : g_UboRings) DestroyRingBuffer(kv.second);
        g_UboRings.clear();
        g_UboHandleSlot.clear();
    }

    // ---- Legacy rung (GLS-08, byte-identical) ----
    // Used only if UboRingAvailable() is false, i.e. glMapBufferRange/glFenceSync/
    // glBindBufferRange themselves fail to resolve -- effectively unreachable on any GL 3.3+
    // driver (GLP-08's floor), kept so a broken/ancient driver install still runs instead of
    // silently rendering nothing.
    BufferHandle LegacyCreateUniformBlock(size_t sizeBytes, int bindingSlot)
    {
        GLuint id = 0;
        fn_glGenBuffers(1, &id);
        fn_glBindBuffer(GL_UNIFORM_BUFFER, id);
        fn_glBufferData(GL_UNIFORM_BUFFER, (GLsizeiptr)sizeBytes, nullptr, GL_DYNAMIC_DRAW);
        fn_glBindBufferBase(GL_UNIFORM_BUFFER, (GLuint)bindingSlot, id);
        fn_glBindBuffer(GL_UNIFORM_BUFFER, 0);
        return BufferHandle{ id };
    }

    void LegacyUpdateUniformBlock(BufferHandle handle, const void* data, size_t sizeBytes)
    {
        fn_glBindBuffer(GL_UNIFORM_BUFFER, handle.id);
        fn_glBufferData(GL_UNIFORM_BUFFER, (GLsizeiptr)sizeBytes, nullptr, GL_DYNAMIC_DRAW);
        FrameProfiler::CountGLCall(FrameProfiler::Counter::BufferUpdates);
        FrameProfiler::TagBufferOrphan();
        fn_glBufferSubData(GL_UNIFORM_BUFFER, 0, (GLsizeiptr)sizeBytes, data);
        FrameProfiler::CountGLCall(FrameProfiler::Counter::BufferUpdates);
        fn_glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void LegacyDestroyUniformBlock(BufferHandle handle)
    {
        GLuint id = handle.id;
        fn_glDeleteBuffers(1, &id);
    }
}

BufferHandle CreateUniformBlock(size_t sizeBytes, int bindingSlot)
{
    if (!LoadBufferGLFunctions()) return {};

    if (!UboRingAvailable())
    {
        return LegacyCreateUniformBlock(sizeBytes, bindingSlot);
    }

    const GLsizeiptr alignedSize = AlignUp((GLsizeiptr)sizeBytes, (GLsizeiptr)g_Caps.uboOffsetAlignment);
    EnsureRingCapacity(bindingSlot, alignedSize);

    const uint32_t id = kUboReservationTag | (g_NextUboHandleId++);
    g_UboHandleSlot[id] = bindingSlot;
    return BufferHandle{ id };
}

void UpdateUniformBlock(BufferHandle handle, const void* data, size_t sizeBytes)
{
    // Whole-block update, per the header contract -- callers keep their own dirty-checking.
    // GLP-09: sub-allocates from the handle's binding-slot ring instead of respecifying a
    // dedicated buffer. This call is what ESTABLISHES the binding now (glBindBufferRange, moved
    // here from CreateUniformBlock's old glBindBufferBase-at-creation-time) -- verified every
    // current caller updates immediately after creating (GlobalUBO::Create, SceneUBO::Create,
    // BoneUBO::Create, BMDMeshShader::CreateGL all do), so this is not a behavior change for any
    // caller in the tree today. A future caller that creates a block and never updates it would
    // have no binding -- keep that in mind before adding one.
    if (!handle.IsValid() || !LoadBufferGLFunctions()) return;

    if (!UboRingAvailable())
    {
        LegacyUpdateUniformBlock(handle, data, sizeBytes);
        return;
    }

    auto it = g_UboHandleSlot.find(handle.id);
    if (it == g_UboHandleSlot.end()) return;
    const int bindingSlot = it->second;

    const GLsizeiptr alignedSize = AlignUp((GLsizeiptr)sizeBytes, (GLsizeiptr)g_Caps.uboOffsetAlignment);
    UboRing& ring = EnsureRingCapacity(bindingSlot, alignedSize);

    // GLP-10: content dirty-check. Skip the write AND the bind entirely when this slot's last
    // upload happened THIS SAME FRAME and had byte-identical contents -- the binding point still
    // points at that upload's range, and same-frame is what guarantees the ring hasn't wrapped
    // back over it since (see the UboRing::lastFrameIndex comment). Any other outcome (different
    // frame, different size, different bytes) falls through to a real write, unconditionally.
    if (ring.lastFrameIndex == g_UboFrameIndex &&
        ring.lastBytes.size() == sizeBytes &&
        memcmp(ring.lastBytes.data(), data, sizeBytes) == 0)
    {
        FrameProfiler::CountSkip(FrameProfiler::Counter::UboSkips);
        return;
    }

    const GLsizeiptr writeOffset = ring.currentSegment * ring.segmentSize + ring.writeOffset;
    assert(writeOffset % g_Caps.uboOffsetAlignment == 0 && "GLP-09: UBO ring offset misaligned");

    if (ring.persistent)
    {
        memcpy(static_cast<unsigned char*>(ring.mappedPtr) + writeOffset, data, sizeBytes);
    }
    else
    {
        fn_glBindBuffer(GL_UNIFORM_BUFFER, ring.bufferId);
        void* dst = fn_glMapBufferRange(GL_UNIFORM_BUFFER, writeOffset, (GLsizeiptr)sizeBytes,
            GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
        if (dst != nullptr)
        {
            memcpy(dst, data, sizeBytes);
            fn_glUnmapBuffer(GL_UNIFORM_BUFFER);
        }
        fn_glBindBuffer(GL_UNIFORM_BUFFER, 0);
        FrameProfiler::CountGLCall(FrameProfiler::Counter::BufferUpdates);
    }

    fn_glBindBufferRange(GL_UNIFORM_BUFFER, (GLuint)bindingSlot, ring.bufferId, writeOffset, (GLsizeiptr)sizeBytes);
    FrameProfiler::CountGLCall(FrameProfiler::Counter::BufferUpdates);

    ring.writeOffset += alignedSize;

    // GLP-10: shadow state for next call's dirty-check. Copies the caller's bytes, not a
    // pointer -- `data` is the caller's own packed-uniform local (e.g. BMDMeshShader::BindGL's
    // stack `cb`), not guaranteed to outlive this call.
    ring.lastBytes.assign(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + sizeBytes);
    ring.lastOffset = writeOffset;
    ring.lastFrameIndex = g_UboFrameIndex;
}

void DestroyUniformBlock(BufferHandle handle)
{
    // Frees a SLOT RESERVATION, not the shared ring -- the ring itself is only torn down at
    // RHI_GL_Impl::Shutdown(). The four current callers only reach this from their own
    // destructors at process teardown, by which point Shutdown() runs next anyway.
    if (!handle.IsValid() || !LoadBufferGLFunctions()) return;

    if (!UboRingAvailable())
    {
        LegacyDestroyUniformBlock(handle);
        return;
    }

    g_UboHandleSlot.erase(handle.id);
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

void BindIndexBuffer(BufferHandle handle)
{
    // GL_UNSIGNED_INT only, per the header contract -- the one indexed-draw caller in the tree
    // (GLP-16's terrain tile bucketing) uses no other type. Binding GL_ELEMENT_ARRAY_BUFFER here
    // is captured into whichever VAO is currently bound (core-profile VAO state includes the
    // element buffer binding) -- callers issuing this mid-pass are intentionally repointing that
    // VAO's index source, not just setting transient state.
    if (!handle.IsValid() || !LoadBufferGLFunctions()) return;
    fn_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle.id);
}

// ---- Draw ----
void Draw(Topology topology, uint32_t vertexCount, uint32_t firstVertex)
{
    // GLP-19: bound the pending batch's LIFETIME instead of trying to enumerate every piece of
    // state that could move under it. Enumerating failed three times (matrices in GlobalUBO, binds
    // in BindState, texture content in UpdateTexture) because the surface is genuinely open-ended.
    // This is the closed form: a deferred IR batch may never survive another draw. Re-entrancy is
    // free -- IR::Flush() calls straight back into here, but it clears its pending flag before
    // submitting, so the inner call is a no-op. Consecutive IR quads (particles, text runs) still
    // merge, because nothing else draws between them -- which is exactly the hot path.
    IR::Flush(IR::FlushCause::Draw);

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

void DrawIndexed(Topology topology, uint32_t indexCount, uint32_t firstIndex)
{
    IR::Flush(IR::FlushCause::Draw); // GLP-19 -- see Draw()

    GLenum mode;
    switch (topology)
    {
    case Topology::TriangleList: mode = GL_TRIANGLES;  break;
    case Topology::LineList:     mode = GL_LINES;      break;
    case Topology::LineStrip:    mode = GL_LINE_STRIP; break;
    default: return;
    }
    // firstIndex is an ELEMENT offset (matches Draw()'s firstVertex convention), converted to
    // the byte offset glDrawElements wants. Relies on the currently-bound GL_ELEMENT_ARRAY_BUFFER
    // (set via a prior BindIndexBuffer call), same two-call shape as BindVertexBuffer + Draw.
    const uintptr_t byteOffset = (uintptr_t)firstIndex * sizeof(GLuint);
    glDrawElements(mode, (GLsizei)indexCount, GL_UNSIGNED_INT, (const void*)byteOffset);
    FrameProfiler::CountGLCall(FrameProfiler::Counter::DrawCalls);
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
    // GLP-19: texture CONTENT changing is batch state just as much as texture identity, and
    // BindState's hook cannot see it -- the bind below is a no-op whenever the same texture is
    // re-uploaded, which is exactly what the text renderer does (one dynamic texture, new pixels
    // per string). A pending IR batch still references the old content, so it must be drawn first;
    // otherwise every merged quad samples whatever was uploaded last, which showed up as UI text
    // where each line displayed a later line's glyphs.
    IR::Flush(IR::FlushCause::Texture);
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
