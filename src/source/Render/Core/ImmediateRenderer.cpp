#include "stdafx.h"
#include "ImmediateRenderer.h"
#include "Render/Shaders/PassthroughShader.h"
#include "Render/Textures/ZzzOpenglUtil.h" // GLP-19 -- GetRenderStateSnapshot()
#include "Render/RHI/RHI.h"
#include "Core/Utilities/Log/ErrorReport.h"
#include <vector>
#include <cstring>

// DXP-11: IR:: is the first real RHI:: caller. It no longer owns any raw GL -- buffer
// creation/append, VAO/attrib setup, and the draw call all live in RHI_GL now (Render/RHI/RHI_GL.cpp).
// This file's job is CPU-side vertex accumulation (Begin/Color/TexCoord/Vertex*/quad-and-fan
// decomposition) plus handing the finished vertex list to RHI:: in End().

struct IRVertex {
    float x, y, z;
    float u, v;
    float r, g, b, a;
};

static RHI::BufferHandle g_VBO;

static IRVertex g_CurrentVertex = { 0.f, 0.f, 0.f,  0.f, 0.f,  1.f, 1.f, 1.f, 1.f };
static GLenum   g_CurrentMode   = 0;
static std::vector<IRVertex> g_VertexBuffer;

// GLP-29 2a: a fixed array, not a std::vector. This is a 4-element scratch buffer that never grows,
// yet it was doing a push_back per vertex and a clear per quad -- ~11,900 vector operations a frame
// at 2,975 particles, each carrying a capacity check (and, in Debug, checked-iterator bookkeeping).
static IRVertex g_QuadTemp[4];
static int      g_QuadCount = 0;

// GL_TRIANGLE_FAN state (DXP-09 9.2): D3D11 has no fan topology, so fans are decomposed to a
// triangle list here exactly like GL_QUADS is above. Fan vertex i>=2 emits [first, prev, current].
static IRVertex g_FanFirst;
static IRVertex g_FanPrev;
static int      g_FanCount = 0;

// ---- GLP-19: cross-Begin/End batching ----
// DXP-26 removed the redundant GL calls *around* each per-quad draw but left the draw count alone:
// one AppendBuffer + one RHI::Draw per Begin/End pair. On the dev box that was 2,901 draws in a
// single skill-cast frame, against a frame where TexBind moved by 22 and ProgBind by 3 -- i.e. the
// state was already near-constant across the burst and only the draw boundary was artificial.
// (Measured on RTX 5070 Ti, not Intel HD 530 -- an earlier revision of this comment mis-attributed
// it. Target-hardware verification for the whole GLP series is still outstanding.)
//
// So: End() defers, and Begin() merges into the pending batch when nothing that affects the pixels
// has changed. Compatibility is decided by reading the CURRENT cached render state, not by
// intercepting every writer of it -- the wrapper family in ZzzOpenglUtil.cpp mutates four or five
// independently-cached sub-states per call and is invoked per particle, so hooking its entry points
// would flush on every quad and win nothing.
namespace {
    struct IRBatchKey
    {
        GLRenderStateSnapshot            gl{};
        PassthroughShader::StateSnapshot ps{};

        bool operator==(const IRBatchKey& o) const
        {
            return gl.alphaBlendType  == o.gl.alphaBlendType
                && gl.alphaTestEnable == o.gl.alphaTestEnable
                && gl.alphaRef        == o.gl.alphaRef
                && gl.depthTestEnable == o.gl.depthTestEnable
                && gl.depthMaskEnable == o.gl.depthMaskEnable
                && gl.cullFaceEnable  == o.gl.cullFaceEnable
                && gl.textureEnable   == o.gl.textureEnable
                && gl.fogEnable       == o.gl.fogEnable
                && gl.cachTexture     == o.gl.cachTexture
                && ps.useTexture      == o.ps.useTexture
                && ps.useFog          == o.ps.useFog
                && ps.texCombineAdd   == o.ps.texCombineAdd;
        }
        bool operator!=(const IRBatchKey& o) const { return !(*this == o); }
    };

    // Field-wise, never hashed: a hash collision here draws a quad with the wrong texture or blend
    // mode, scene-dependent and near-impossible to reproduce. The struct is small; compare it.
    IRBatchKey CaptureKey()
    {
        IRBatchKey k;
        k.gl = GetRenderStateSnapshot();
        k.ps = PassthroughShader::Instance().GetStateSnapshot();
        return k;
    }

    RHI::Topology TopologyFor(GLenum mode)
    {
        if (mode == GL_LINES)      return RHI::Topology::LineList;
        if (mode == GL_LINE_STRIP) return RHI::Topology::LineStrip;
        // GL_TRIANGLES, plus GL_QUADS and GL_TRIANGLE_FAN, both decomposed to a triangle list in
        // Vertex3f() -- which is what makes most consecutive batches mergeable at all.
        return RHI::Topology::TriangleList;
    }
}

static bool          g_BatchPending    = false;
static IRBatchKey    g_PendingKey;
static RHI::Topology g_PendingTopology = RHI::Topology::TriangleList;

namespace IR {

void Create()
{
    if (g_VBO.IsValid()) return;

    // GLP-25: 4096 vertices (~144 KB) was far too small for a frame's streaming volume -- one
    // decomposed quad is 6 vertices, so it held ~680 quads, while an effect-dense frame on Intel
    // HD 530 submitted ~4,400 IR appends and wrapped the ring ~23 times. RHI::AppendBuffer only
    // ever grew when a SINGLE append exceeded the whole buffer, so it never grew out of that.
    // 65536 vertices (~2.25 MB) covers ~10,900 quads per wrap -- comfortably a busy cast frame,
    // and a rounding error against this client's texture budget.
    const size_t initialCapacity = 65536 * sizeof(IRVertex);
    g_VBO = RHI::CreateVertexBuffer(nullptr, initialCapacity, RHI::BufferUsage::Dynamic);

    // GLP-29 2b: reserve the CPU-side accumulation buffer once, to the same high-water mark as the
    // GPU ring above. Flush() clears it every batch, so without this it regrows from nothing and
    // reallocates repeatedly through a busy frame -- 2,975 particles is ~17,850 vertices.
    // clear() keeps capacity, so this reserve survives for the process lifetime.
    g_VertexBuffer.reserve(65536);

    g_ErrorReport.Write(L"[ImmediateRenderer] Created RHI streaming vertex buffer, Initial Capacity %zu bytes\r\n", initialCapacity);
}

void Destroy()
{
    if (g_VBO.IsValid()) {
        RHI::DestroyBuffer(g_VBO);
        g_VBO = {};
    }
    // Note: PosUvColor's VAO is owned internally by RHI_GL (one VAO per VertexLayout, not per
    // buffer -- see RHI_GL.cpp) and isn't torn down by DestroyBuffer. Matches this function's
    // pre-RHI behavior of having no caller anywhere in the tree (grepped), so not a behavior change.
}

void Begin(GLenum mode)
{
    // Bind FIRST, then read the key: it has to describe the state this batch will actually draw
    // with, not whatever preceded it. Both calls can themselves flush a pending batch via their
    // own dirty-check hooks (Bind() on a changed u_AlphaRef, SetUseTexture on a changed value) --
    // which is correct, that batch was submitted under the old value.
    // Neither call may be removed: DXP-26 step 5 documents 17 sites that override SetUseTexture
    // right after Begin(), and that pattern is load-bearing.
    PassthroughShader::Instance().Bind();
    PassthroughShader::Instance().SetUseTexture(true);

    // GLP-29: build the comparison key only when there is actually a pending batch to compare it
    // against, and only after the cheap topology check has passed. In the particle path a
    // blend-mode change flushes on nearly every quad, so g_BatchPending is usually false by the
    // time we get here and the snapshot was pure waste -- two CaptureKey() calls per quad (this
    // one and End()'s) of which this one is never read.
    bool mergeable = false;
    if (g_BatchPending && TopologyFor(mode) == g_PendingTopology)
        mergeable = (CaptureKey() == g_PendingKey);

    if (!mergeable)
    {
        Flush();
        g_VertexBuffer.clear();
    }
    // Mergeable: keep the accumulated vertices and append this batch onto them. Deliberately does
    // NOT clear g_VertexBuffer -- that is the whole optimisation.

    g_CurrentMode = mode;
    g_QuadCount = 0; // GLP-29 2a
    g_FanCount = 0;
    // g_CurrentVertex is deliberately NOT reset, unchanged from before: RenderJoints() sets colors
    // BEFORE Begin() and relies on them surviving (ZzzEffectJoint.cpp:7025-7061 -> :7066).
}

void Color4f(float r, float g, float b, float a)
{
    g_CurrentVertex.r = r;
    g_CurrentVertex.g = g;
    g_CurrentVertex.b = b;
    g_CurrentVertex.a = a;
}

void Color4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    g_CurrentVertex.r = r / 255.0f;
    g_CurrentVertex.g = g / 255.0f;
    g_CurrentVertex.b = b / 255.0f;
    g_CurrentVertex.a = a / 255.0f;
}

void Color3f(float r, float g, float b)
{
    g_CurrentVertex.r = r;
    g_CurrentVertex.g = g;
    g_CurrentVertex.b = b;
    g_CurrentVertex.a = 1.0f;
}

void Color3fv(const float* c)
{
    if (!c) return;
    g_CurrentVertex.r = c[0];
    g_CurrentVertex.g = c[1];
    g_CurrentVertex.b = c[2];
    g_CurrentVertex.a = 1.0f;
}

void TexCoord2f(float u, float v)
{
    g_CurrentVertex.u = u;
    g_CurrentVertex.v = v;
}

void Vertex3f(float x, float y, float z)
{
    g_CurrentVertex.x = x;
    g_CurrentVertex.y = y;
    g_CurrentVertex.z = z;

    if (g_CurrentMode == GL_QUADS) {
        g_QuadTemp[g_QuadCount++] = g_CurrentVertex;
        if (g_QuadCount == 4) {
            // Decompose quad to 2 triangles: 0,1,2 and 0,2,3
            g_VertexBuffer.push_back(g_QuadTemp[0]);
            g_VertexBuffer.push_back(g_QuadTemp[1]);
            g_VertexBuffer.push_back(g_QuadTemp[2]);

            g_VertexBuffer.push_back(g_QuadTemp[0]);
            g_VertexBuffer.push_back(g_QuadTemp[2]);
            g_VertexBuffer.push_back(g_QuadTemp[3]);

            g_QuadCount = 0; // GLP-29 2a -- index reset, was g_QuadTemp.clear()
        }
    } else if (g_CurrentMode == GL_TRIANGLE_FAN) {
        // Decompose fan to a triangle list: vertex i>=2 emits [first, prev, current] -- the
        // same winding a real GL_TRIANGLE_FAN produces.
        if (g_FanCount == 0) {
            g_FanFirst = g_CurrentVertex;
        } else if (g_FanCount == 1) {
            g_FanPrev = g_CurrentVertex;
        } else {
            g_VertexBuffer.push_back(g_FanFirst);
            g_VertexBuffer.push_back(g_FanPrev);
            g_VertexBuffer.push_back(g_CurrentVertex);
            g_FanPrev = g_CurrentVertex;
        }
        g_FanCount++;
    } else {
        g_VertexBuffer.push_back(g_CurrentVertex);
    }
}

void Vertex3fv(const float* p)
{
    if (!p) return;
    Vertex3f(p[0], p[1], p[2]);
}

void Vertex2f(float x, float y)
{
    Vertex3f(x, y, 0.0f);
}

void End()
{
    if (!g_VBO.IsValid() || g_VertexBuffer.empty()) return;

    // GLP-19: mark pending instead of drawing. The key is captured HERE rather than at Begin() so
    // it reflects any state the caller set BETWEEN Begin() and End() -- the SetUseTexture(false)
    // pattern at 17 sites in the tree. The next Begin() compares live state against this snapshot.
    g_BatchPending    = true;
    g_PendingTopology = TopologyFor(g_CurrentMode);
    g_PendingKey      = CaptureKey();

    // DXP-26 step 3: don't force-unbind the program here (mirrors ZzzBMD.cpp's BeginRender/
    // EndRender reflexive-unbind removal, DXP-22). Begin() re-binds PassthroughShader every call
    // regardless, so leaving it bound just lets BindProgram's cache skip the glUseProgram
    // round-trip on consecutive IR draws -- the overwhelming common case. The one fixed-function
    // fallback that needs a real unbind (RenderTerrainFrustrum's grass-tile glBegin path) already
    // calls UnbindAllShaders() once per pass.
}

void Flush()
{
    if (!g_BatchPending) return;
    // Cleared before the draw, not after, so a re-entrant call can never see a batch that is
    // already being submitted.
    g_BatchPending = false;

    if (g_VBO.IsValid() && !g_VertexBuffer.empty())
    {
        // Topology comes from the pending batch, NOT from g_CurrentMode -- by the time a flush
        // fires (a state hook, or end of frame) g_CurrentMode may belong to a later Begin().
        const RHI::Topology topology = g_PendingTopology;

        const size_t neededSize = g_VertexBuffer.size() * sizeof(IRVertex);

        // RHI::AppendBuffer owns the DXP-26 ring-buffer logic now (write at a growing offset,
        // grow/wrap only when it doesn't fit) -- see RHI.h's Buffers section for why this can't
        // be RHI::UpdateBuffer (discard semantics would reintroduce the GPU stall DXP-26 fixed).
        const size_t byteOffset = RHI::AppendBuffer(g_VBO, g_VertexBuffer.data(), neededSize);

        // BindVertexBuffer's VAO bind is already dirty-checked (RHI_GL -> BindState.cpp), so
        // consecutive IR draws skip the real glBindVertexArray call for free, same as the
        // pre-RHI DXP-26 step 2 behavior this replaces.
        RHI::BindVertexBuffer(g_VBO, RHI::VertexLayout::PosUvColor);
        RHI::Draw(topology, (uint32_t)g_VertexBuffer.size(), (uint32_t)(byteOffset / sizeof(IRVertex)));
    }

    g_VertexBuffer.clear();
}

} // namespace IR
