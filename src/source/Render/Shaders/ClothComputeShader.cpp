#include "stdafx.h"
#include "Render/Shaders/ClothComputeShader.h"
#include "Render/RHI/RHI.h"
#include "Core/Utilities/Log/ErrorReport.h"
#include "Engine/Physics/PhysicsManager.h" // PLS_SPRING
#ifdef RHI_D3D11_AVAILABLE
#include <d3d11.h>
#include <d3dcompiler.h>
#endif
#include <cstring>
#include <cmath>
#include <vector>

#ifdef RHI_D3D11_AVAILABLE
namespace {
    // Same helper shape as PlanarShadowShader.cpp's CompileHLSL -- duplicated rather than shared
    // because it's a 20-line static function with no shared header today (every shader class
    // that needs it defines its own copy; not worth a new shared translation unit for one helper).
    bool CompileHLSL(const char* source, const char* entryPoint, const char* target, ID3DBlob** outBlob)
    {
        ID3DBlob* errorBlob = nullptr;
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        const HRESULT hr = D3DCompile(source, strlen(source), nullptr, nullptr, nullptr,
            entryPoint, target, flags, 0, outBlob, &errorBlob);
        if (FAILED(hr))
        {
            if (errorBlob)
            {
                g_ErrorReport.Write(L"[ClothComputeShader][D3D11] %hs compile error: %hs\r\n",
                    entryPoint, static_cast<const char*>(errorBlob->GetBufferPointer()));
                errorBlob->Release();
            }
            else
            {
                g_ErrorReport.Write(L"[ClothComputeShader][D3D11] %hs compile failed, hr=0x%08lX\r\n",
                    entryPoint, static_cast<unsigned long>(hr));
            }
            return false;
        }
        if (errorBlob) errorBlob->Release(); // warnings only
        return true;
    }

    // Mirrors St_PhysicsLink's full shape (m_nVertices[0]/[1], m_fDistance[0]/[1], m_byStyle) --
    // distMin only matters to Pass D's STRICTDISTANCE clamp, carried through Pass B/C too since
    // structured-buffer stride has no alignment requirement to exploit by omitting it.
    struct GpuLinkGPU
    {
        int32_t  v0;
        int32_t  v1;
        float    distMax;
        float    distMin;
        uint32_t style;
    };
    static_assert(sizeof(GpuLinkGPU) == 20, "must match HLSL GpuLink stride");

    // Mirrors CPhysicsVertex's m_vPos/m_vVel + the pin flag PhysicsManager.h computes at Create()
    // time (v->Node == m_iBone). `pad` exists only so the struct is a clean 32 bytes (HLSL
    // structured-buffer stride has no alignment requirement, but a round stride avoids any
    // ambiguity between the C++ and HLSL layouts).
    struct GpuVertexGPU
    {
        float    pos[3];
        float    vel[3];
        uint32_t pinned;
        float    pad;
    };
    static_assert(sizeof(GpuVertexGPU) == 32, "must match HLSL GpuVertex stride");

    struct PinCB
    {
        float    boneMatrix[16]; // column_major, matches BMDMeshShader's u_Bones convention
        uint32_t vertexCount;
        float    pad[3];
    };
    static_assert(sizeof(PinCB) == 80, "cbuffer size must be a multiple of 16 bytes");

    // Mirrors TickParams (ClothComputeShader.h) plus the fixed integrate constants. Row-packed for
    // HLSL cbuffer rules: row1 = {vertexCount,deltaTime,invMass,springFactor}, row2 =
    // {windVec.xyz,fWind}, row3 = {worldTime,numHor,seed,dwType} -- 48 bytes, 3 full float4 rows.
    struct IntegrateCB
    {
        uint32_t vertexCount;
        float    deltaTime;
        float    invMass;
        float    springFactor; // FPS_ANIMATION_FACTOR-equivalent, caller-supplied

        float    windVec[3];
        float    fWind;

        float    worldTime;
        int32_t  numHor;
        int32_t  seed;
        uint32_t dwType;
    };
    static_assert(sizeof(IntegrateCB) == 48, "cbuffer size must be a multiple of 16 bytes");

    // Shared by Pass C (LOOSEDISTANCE) and Pass D (STRICTDISTANCE) -- Pass C ignores NumHor.
    struct ConstraintCB
    {
        uint32_t vertexCount;
        uint32_t numHor;
        float    pad[2];
    };
    static_assert(sizeof(ConstraintCB) == 16, "cbuffer size must be a multiple of 16 bytes");

    // Pass F constants: fixed-size sphere array (kMaxCollisionSpheres) rather than a separate
    // StructuredBuffer -- sphere count per cloth is tiny (grepped every AddCollisionSphere call
    // site: max 4 in the real tree), so a cbuffer array avoids needing per-instance dynamic
    // structured-buffer plumbing just for this. sphereCenterRadius[i] = {center.xyz, radius}.
    struct CollisionCB
    {
        uint32_t vertexCount;
        uint32_t sphereCount;
        float    pad[2];
        float    sphereCenterRadius[CClothComputeShader::kMaxCollisionSpheres][4];
    };
    static_assert(sizeof(CollisionCB) == 16 + CClothComputeShader::kMaxCollisionSpheres * 16, "cbuffer size must be a multiple of 16 bytes");
    static_assert(CClothComputeShader::kMaxCollisionSpheres == 8, "keep in sync with g_szCollisionCS's SphereCenterRadius[8] literal below");

    // Pass E constants: the color RenderFace threads through IR::Color3fv (constant for every
    // vertex in one draw call, see PhysicsManager.cpp's RenderFace/EmitVertex), plus vertexCount.
    struct ScatterCB
    {
        float    color[4];
        uint32_t vertexCount;
        float    pad[3];
    };
    static_assert(sizeof(ScatterCB) == 32, "cbuffer size must be a multiple of 16 bytes");

    // Bit-identical layout to RHI.h VertexLayout::PosUvColor / ImmediateRenderer.cpp's IRVertex
    // (pos3+uv2+rgba4). Deliberate -- see ClothComputeShader.h's Instance_t comment.
    struct DrawVertexGPU
    {
        float pos[3];
        float uv[2];
        float color[4];
    };
    static_assert(sizeof(DrawVertexGPU) == 36, "must match RHI::VertexLayout::PosUvColor stride");

    // ---- Pass A: pin-skin ----
    // pos = BoneMatrix * restLocalPos for pinned vertices only, in-place -- no cross-thread reads,
    // so no ping-pong needed here (see ClothComputeShader.h's race-hazard note).
    const char* g_szPinSkinCS = R"(
struct GpuVertex { float3 pos; float3 vel; uint pinned; float pad; };

cbuffer PinCB : register(b0)
{
    column_major matrix BoneMatrix;
    uint VertexCount;
};

StructuredBuffer<float3> RestLocalPos : register(t0);
RWStructuredBuffer<GpuVertex> Vertices : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    uint i = id.x;
    if (i >= VertexCount) return;
    if (Vertices[i].pinned == 0) return;

    float3 rp = RestLocalPos[i];
    Vertices[i].pos = mul(BoneMatrix, float4(rp, 1.0)).xyz;
}
)";

    // ---- Pass B: CSR-gather spring force + gravity/wind + semi-implicit-Euler integrate ----
    // Reads the front buffer (SRV), writes the back buffer (UAV) -- see the header's race-hazard
    // note for why. Direct translation of CPhysicsVertex::UpdateForce (base wind+damping, the
    // elastic-type wind terms, gravity) + CPhysicsCloth::MoveVertices' PLS_SPRING branch
    // (including PCT_OPT_CORRECTEDFORCE and the elastic-type force-scale switches) +
    // CPhysicsVertex::Move's velocity/position integrate. See ClothComputeShader.h's top-of-file
    // scope-gap note for what's still not covered (collision, mesh-topology iKey variant).
    const char* g_szSpringIntegrateCS = R"(
struct GpuVertex { float3 pos; float3 vel; uint pinned; float pad; };
struct GpuLink { int v0; int v1; float distMax; float distMin; uint style; };

cbuffer IntegrateCB : register(b0)
{
    uint VertexCount;
    float DeltaTime;
    float InvMass;
    float SpringFactor;

    float3 WindVec;
    float Wind;

    float WorldTime;
    int NumHor;
    int Seed;
    uint DwType;
};

StructuredBuffer<GpuLink> Links : register(t0);
StructuredBuffer<uint> AdjOffsets : register(t1);   // size VertexCount+1
StructuredBuffer<uint> AdjLinks : register(t2);     // link indices into Links
StructuredBuffer<GpuVertex> Front : register(t3);
RWStructuredBuffer<GpuVertex> Back : register(u0);

static const uint PLS_SPRING = 0x02;
static const uint PCT_MASK_ELASTIC = 0x300;
static const uint PCT_RUBBER = 0x100;
static const uint PCT_RUBBER2 = 0x200;
static const uint PCT_MASK_ELASTIC_EXT = 0xE000;
static const uint PCT_ELASTIC_HALLOWEEN = 0x4000;
static const uint PCT_ELASTIC_RAGE_L = 0x8000;
static const uint PCT_ELASTIC_RAGE_R = 0xC000;
static const uint PCT_MASK_WEIGHT = 0xC00;
static const uint PCT_HEAVY = 0x400;
static const uint PCT_OPT_CORRECTEDFORCE = 0x20000000;
static const float GRAVITY = 9.8;
static const float MASS = 0.0025;

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    uint v = id.x;
    if (v >= VertexCount) return;

    GpuVertex self = Front[v];

    if (self.pinned != 0)
    {
        // Matches CPU: SetFixedVertices owns pinned positions, Move() never touches them
        // (UpdateForce also early-outs on PVS_FIXEDPOS, so no force is ever computed for these).
        Back[v] = self;
        return;
    }

    // ---- CPhysicsVertex::UpdateForce equivalent ----
    uint row = v / (uint)NumHor;
    uint col = v % (uint)NumHor;
    int seedRow = Seed / NumHor;
    int seedCol = Seed % NumHor;
    int iKey = abs(seedCol - (int)col) + abs(seedRow - (int)row);
    int iTemp = clamp(5 - iKey, 0, 4);
    float fRand = (iTemp == 0) ? 0.0 : (float)(iTemp + 2);

    float3 force = fRand * WindVec - self.vel * 0.01;

    uint elastic = PCT_MASK_ELASTIC & DwType;
    if (elastic == PCT_RUBBER)
        force.z += fRand * (Wind + 0.1) * 1.0 * SpringFactor;
    else if (elastic == PCT_RUBBER2)
        force.z += fRand * Wind * SpringFactor;

    uint elasticExt = PCT_MASK_ELASTIC_EXT & DwType;
    if (elasticExt == PCT_ELASTIC_HALLOWEEN)
    {
        force.x += -(fRand * Wind * 0.5 * SpringFactor);
        force.z += fRand * (Wind + 0.1) * 0.5 * sin(WorldTime * 0.003) * 5.0 * SpringFactor;
        force.z -= GRAVITY * MASS * 50.0 * SpringFactor;
    }
    else if (elasticExt == PCT_ELASTIC_RAGE_L)
    {
        force.x += -(fRand * Wind * 0.8 * SpringFactor);
    }
    else if (elasticExt == PCT_ELASTIC_RAGE_R)
    {
        force.x -= -(fRand * Wind * 0.8 * SpringFactor);
    }

    uint weight = PCT_MASK_WEIGHT & DwType;
    if (weight == PCT_HEAVY)
        force.z -= GRAVITY * MASS * 180.0 * SpringFactor;
    else
        force.z -= GRAVITY * MASS * 100.0 * SpringFactor;

    // ---- CPhysicsCloth::MoveVertices' PLS_SPRING gather ----
    uint begin = AdjOffsets[v];
    uint end = AdjOffsets[v + 1];
    for (uint k = begin; k < end; ++k)
    {
        GpuLink link = Links[AdjLinks[k]];
        if ((link.style & PLS_SPRING) == 0) continue;

        uint other = (uint)((v == (uint)link.v0) ? link.v1 : link.v0);
        float3 otherPos = Front[other].pos;

        // CPU convention: vDistance = pos(v0) - pos(v1) always, regardless of which side is
        // querying -- reproduced here via a sign flip depending on which role `v` plays.
        float3 dirV0MinusV1 = (v == (uint)link.v0) ? (self.pos - otherPos) : (otherPos - self.pos);
        float dist = max(0.001, length(dirV0MinusV1));
        if (dist <= link.distMax + 0.01) continue;

        float3 vForce = (dist - link.distMax) * dirV0MinusV1 / dist * SpringFactor;
        if ((DwType & PCT_OPT_CORRECTEDFORCE) != 0)
            vForce *= (link.distMax / 32.0);
        if (elastic == PCT_RUBBER)
            vForce *= 3.0;
        if (elasticExt == PCT_ELASTIC_HALLOWEEN)
            vForce *= 2.0;
        // RAGE_L/RAGE_R multiply by 1.0 on CPU (no-op), nothing to reproduce.

        // CPU: v0 (link's first vertex) gets -vForce, v1 (second) gets +vForce.
        force += (v == (uint)link.v0) ? -vForce : vForce;
    }

    GpuVertex result = self;
    result.vel = self.vel + force * InvMass * DeltaTime;
    result.pos = self.pos + result.vel * DeltaTime;
    Back[v] = result;
}
)";

    // ---- Pass C: CSR-gather LOOSEDISTANCE soft constraint ----
    // Direct translation of CPhysicsVertex::AddOneTimeMoveToKeepLength (per-link, symmetric,
    // targets exactly m_fDistance[1]) + DoOneTimeMove (average by contribution count, applied to
    // position only -- no velocity change). See ClothComputeShader.h's scope-gap note: the count
    // divisor here is LOOSEDISTANCE-incident-link count only, CPU's also includes collision
    // one-time-moves (not ported).
    const char* g_szLooseDistanceCS = R"(
struct GpuVertex { float3 pos; float3 vel; uint pinned; float pad; };
struct GpuLink { int v0; int v1; float distMax; float distMin; uint style; };

cbuffer ConstraintCB : register(b0)
{
    uint VertexCount;
    uint NumHor; // unused by this pass
    float pad0;
    float pad1;
};

StructuredBuffer<GpuLink> Links : register(t0);
StructuredBuffer<uint> AdjOffsets : register(t1);
StructuredBuffer<uint> AdjLinks : register(t2);
StructuredBuffer<GpuVertex> Front : register(t3);
RWStructuredBuffer<GpuVertex> Back : register(u0);

static const uint PLS_LOOSEDISTANCE = 0x01;

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    uint v = id.x;
    if (v >= VertexCount) return;

    GpuVertex self = Front[v];
    if (self.pinned != 0)
    {
        // Matches CPU: DoOneTimeMove skips PVS_FIXEDPOS vertices entirely.
        Back[v] = self;
        return;
    }

    float3 sumMove = float3(0, 0, 0);
    uint count = 0;
    uint begin = AdjOffsets[v];
    uint end = AdjOffsets[v + 1];
    for (uint k = begin; k < end; ++k)
    {
        GpuLink link = Links[AdjLinks[k]];
        if ((link.style & PLS_LOOSEDISTANCE) == 0) continue;

        uint other = (uint)((v == (uint)link.v0) ? link.v1 : link.v0);
        float3 otherPos = Front[other].pos;
        float3 dirV0MinusV1 = (v == (uint)link.v0) ? (self.pos - otherPos) : (otherPos - self.pos);
        float dist = max(0.001, length(dirV0MinusV1));

        float3 scaled = dirV0MinusV1 * ((dist - link.distMax) * 0.5 / dist);
        sumMove += (v == (uint)link.v0) ? -scaled : scaled;
        count++;
    }

    GpuVertex result = self;
    if (count > 0)
    {
        result.pos = self.pos + sumMove / (float)count;
    }
    Back[v] = result;
}
)";

    // ---- Pass D: STRICTDISTANCE hard clamp, one Jacobi sub-iteration per dispatch ----
    // Direct translation of CPhysicsVertex::KeepLength as called by
    // CPhysicsCloth::PreventFromStretching (`pVertex2->KeepLength(pVertex1, ...)`, i.e. only the
    // link's v1-role vertex is ever moved, clamped relative to the v0-role "anchor"). CPU applies
    // this as one sequential sweep across all links in declaration order (top-down by row for the
    // dominant grid topology), so a correction to row N can see row N-1's *already-corrected*
    // position within the same tick. A single GPU dispatch cannot reproduce that intra-dispatch
    // ordering (every thread reads the same Front snapshot) -- DispatchStrictDistance instead
    // issues `iterations` of these dispatches back-to-back, each one propagating corrections one
    // more row/link-hop through the chain (a standard Jacobi-relaxation substitution for
    // Gauss-Seidel). Not bit-identical to CPU; see ClothComputeShader.h's scope-gap note.
    const char* g_szStrictDistanceCS = R"(
struct GpuVertex { float3 pos; float3 vel; uint pinned; float pad; };
struct GpuLink { int v0; int v1; float distMax; float distMin; uint style; };

cbuffer ConstraintCB : register(b0)
{
    uint VertexCount;
    uint NumHor;
    float pad0;
    float pad1;
};

StructuredBuffer<GpuLink> Links : register(t0);
StructuredBuffer<uint> AdjOffsets : register(t1);
StructuredBuffer<uint> AdjLinks : register(t2);
StructuredBuffer<GpuVertex> Front : register(t3);
RWStructuredBuffer<GpuVertex> Back : register(u0);

static const uint PLS_STRICTDISTANCE = 0x04;

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    uint v = id.x;
    if (v >= VertexCount) return;

    GpuVertex self = Front[v];
    if (self.pinned != 0 || v < NumHor)
    {
        // CPU gate: pLink->m_nVertices[1] >= m_iNumHor -- the first grid row never plays the
        // corrected (v1) role of a vertical STRICTDISTANCE link. PVS_FIXEDPOS is KeepLength's own
        // early-out.
        Back[v] = self;
        return;
    }

    uint begin = AdjOffsets[v];
    uint end = AdjOffsets[v + 1];
    for (uint k = begin; k < end; ++k)
    {
        GpuLink link = Links[AdjLinks[k]];
        if ((link.style & PLS_STRICTDISTANCE) == 0) continue;
        if ((uint)link.v1 != v) continue; // only the v1 role is ever corrected, matches CPU

        float3 neighborPos = Front[(uint)link.v0].pos;
        float3 dist3 = self.pos - neighborPos;
        float dist = max(0.001, length(dist3));

        if (dist > link.distMax)
            self.pos -= dist3 * ((dist - link.distMax) / dist);
        else if (dist < link.distMin)
            self.pos -= dist3 * ((dist - link.distMin) / dist);
    }

    Back[v] = self;
}
)";

    // ---- Pass F: sphere collision push-out ----
    // Direct translation of CPhysicsColSphere::ProcessCollision -- push a vertex radially out of
    // any sphere it's found inside. Applied sequentially per overlapping sphere within one thread
    // (no cross-vertex race -- each thread only ever touches its own vertex), unlike CPU's
    // simultaneous average with LOOSEDISTANCE moves via m_vOneTimeMove -- see this header's
    // top-of-file scope-gap note.
    const char* g_szCollisionCS = R"(
struct GpuVertex { float3 pos; float3 vel; uint pinned; float pad; };

cbuffer CollisionCB : register(b0)
{
    uint VertexCount;
    uint SphereCount;
    float pad0;
    float pad1;
    float4 SphereCenterRadius[8]; // xyz = center, w = radius
};

StructuredBuffer<GpuVertex> Front : register(t0);
RWStructuredBuffer<GpuVertex> Back : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    uint v = id.x;
    if (v >= VertexCount) return;

    GpuVertex self = Front[v];
    if (self.pinned == 0)
    {
        for (uint s = 0; s < SphereCount; ++s)
        {
            float3 center = SphereCenterRadius[s].xyz;
            float radius = SphereCenterRadius[s].w;
            float3 dir = self.pos - center;
            float dist = length(dir);
            if (dist < 0.01)
            {
                dist = 0.01;
                dir = float3(dist, 0.0, 0.0);
            }
            if (dist < radius)
            {
                self.pos += dir * ((radius - dist) / dist);
            }
        }
    }
    Back[v] = self;
}
)";

    // ---- Pass E: draw-buffer scatter ----
    // Direct translation of RenderFace's EmitVertex: position comes from the current front buffer,
    // UV is static per-vertex (grid-derived, precomputed once at CreateInstance -- CPU's
    // (float)xVertex/(m_iNumHor-1) formula never changes tick to tick), color is the single
    // constant RenderFace threads via IR::Color3fv for the whole draw call. Output layout matches
    // RHI.h VertexLayout::PosUvColor exactly (see DrawVertexGPU).
    const char* g_szScatterDrawCS = R"(
struct GpuVertex { float3 pos; float3 vel; uint pinned; float pad; };
struct DrawVertex { float3 pos; float2 uv; float4 color; };

cbuffer ScatterCB : register(b0)
{
    float4 Color;
    uint VertexCount;
};

StructuredBuffer<GpuVertex> Front : register(t0);
StructuredBuffer<float2> UV : register(t1);
RWStructuredBuffer<DrawVertex> Out : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    uint v = id.x;
    if (v >= VertexCount) return;

    DrawVertex dv;
    dv.pos = Front[v].pos;
    dv.uv = UV[v];
    dv.color = Color;
    Out[v] = dv;
}
)";
} // namespace
#endif // RHI_D3D11_AVAILABLE

CClothComputeShader& CClothComputeShader::Instance()
{
    static CClothComputeShader s_instance;
    return s_instance;
}

CClothComputeShader::CClothComputeShader() = default;
CClothComputeShader::~CClothComputeShader() { Shutdown(); }

bool CClothComputeShader::Init()
{
    if (m_bInitialized) return m_bSupported;
    m_bInitialized = true;
    m_bSupported = CompileShaders();
    return m_bSupported;
}

bool CClothComputeShader::CompileShaders()
{
#ifdef RHI_D3D11_AVAILABLE
    ID3D11Device* device = static_cast<ID3D11Device*>(RHI::GetD3D11Device());
    if (!device) return false;

    ID3DBlob* pinBlob = nullptr;
    if (!CompileHLSL(g_szPinSkinCS, "main", "cs_5_0", &pinBlob)) return false;
    ID3D11ComputeShader* pinCS = nullptr;
    HRESULT hr = device->CreateComputeShader(pinBlob->GetBufferPointer(), pinBlob->GetBufferSize(), nullptr, &pinCS);
    pinBlob->Release();
    if (FAILED(hr))
    {
        g_ErrorReport.Write(L"[ClothComputeShader] CreateComputeShader(PinSkin) failed (hr=0x%08lX)\r\n", static_cast<unsigned long>(hr));
        return false;
    }
    m_PinSkinCS = pinCS;

    ID3DBlob* springBlob = nullptr;
    if (!CompileHLSL(g_szSpringIntegrateCS, "main", "cs_5_0", &springBlob)) return false;
    ID3D11ComputeShader* springCS = nullptr;
    hr = device->CreateComputeShader(springBlob->GetBufferPointer(), springBlob->GetBufferSize(), nullptr, &springCS);
    springBlob->Release();
    if (FAILED(hr))
    {
        g_ErrorReport.Write(L"[ClothComputeShader] CreateComputeShader(SpringIntegrate) failed (hr=0x%08lX)\r\n", static_cast<unsigned long>(hr));
        return false;
    }
    m_SpringIntegrateCS = springCS;

    ID3DBlob* looseBlob = nullptr;
    if (!CompileHLSL(g_szLooseDistanceCS, "main", "cs_5_0", &looseBlob)) return false;
    ID3D11ComputeShader* looseCS = nullptr;
    hr = device->CreateComputeShader(looseBlob->GetBufferPointer(), looseBlob->GetBufferSize(), nullptr, &looseCS);
    looseBlob->Release();
    if (FAILED(hr))
    {
        g_ErrorReport.Write(L"[ClothComputeShader] CreateComputeShader(LooseDistance) failed (hr=0x%08lX)\r\n", static_cast<unsigned long>(hr));
        return false;
    }
    m_LooseDistanceCS = looseCS;

    ID3DBlob* strictBlob = nullptr;
    if (!CompileHLSL(g_szStrictDistanceCS, "main", "cs_5_0", &strictBlob)) return false;
    ID3D11ComputeShader* strictCS = nullptr;
    hr = device->CreateComputeShader(strictBlob->GetBufferPointer(), strictBlob->GetBufferSize(), nullptr, &strictCS);
    strictBlob->Release();
    if (FAILED(hr))
    {
        g_ErrorReport.Write(L"[ClothComputeShader] CreateComputeShader(StrictDistance) failed (hr=0x%08lX)\r\n", static_cast<unsigned long>(hr));
        return false;
    }
    m_StrictDistanceCS = strictCS;

    ID3DBlob* collisionBlob = nullptr;
    if (!CompileHLSL(g_szCollisionCS, "main", "cs_5_0", &collisionBlob)) return false;
    ID3D11ComputeShader* collisionCS = nullptr;
    hr = device->CreateComputeShader(collisionBlob->GetBufferPointer(), collisionBlob->GetBufferSize(), nullptr, &collisionCS);
    collisionBlob->Release();
    if (FAILED(hr))
    {
        g_ErrorReport.Write(L"[ClothComputeShader] CreateComputeShader(Collision) failed (hr=0x%08lX)\r\n", static_cast<unsigned long>(hr));
        return false;
    }
    m_CollisionCS = collisionCS;

    ID3DBlob* scatterBlob = nullptr;
    if (!CompileHLSL(g_szScatterDrawCS, "main", "cs_5_0", &scatterBlob)) return false;
    ID3D11ComputeShader* scatterCS = nullptr;
    hr = device->CreateComputeShader(scatterBlob->GetBufferPointer(), scatterBlob->GetBufferSize(), nullptr, &scatterCS);
    scatterBlob->Release();
    if (FAILED(hr))
    {
        g_ErrorReport.Write(L"[ClothComputeShader] CreateComputeShader(ScatterDraw) failed (hr=0x%08lX)\r\n", static_cast<unsigned long>(hr));
        return false;
    }
    m_ScatterDrawCS = scatterCS;

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    cbDesc.ByteWidth = sizeof(PinCB);
    ID3D11Buffer* pinCB = nullptr;
    if (FAILED(device->CreateBuffer(&cbDesc, nullptr, &pinCB))) return false;
    m_PinCBuffer = pinCB;

    cbDesc.ByteWidth = sizeof(IntegrateCB);
    ID3D11Buffer* integrateCB = nullptr;
    if (FAILED(device->CreateBuffer(&cbDesc, nullptr, &integrateCB))) return false;
    m_IntegrateCBuffer = integrateCB;

    cbDesc.ByteWidth = sizeof(ConstraintCB);
    ID3D11Buffer* constraintCB = nullptr;
    if (FAILED(device->CreateBuffer(&cbDesc, nullptr, &constraintCB))) return false;
    m_ConstraintCBuffer = constraintCB;

    cbDesc.ByteWidth = sizeof(CollisionCB);
    ID3D11Buffer* collisionCB = nullptr;
    if (FAILED(device->CreateBuffer(&cbDesc, nullptr, &collisionCB))) return false;
    m_CollisionCBuffer = collisionCB;

    cbDesc.ByteWidth = sizeof(ScatterCB);
    ID3D11Buffer* scatterCB = nullptr;
    if (FAILED(device->CreateBuffer(&cbDesc, nullptr, &scatterCB))) return false;
    m_ScatterCBuffer = scatterCB;

    return true;
#else
    return false;
#endif
}

void CClothComputeShader::Shutdown()
{
#ifdef RHI_D3D11_AVAILABLE
    if (m_PinSkinCS)         { static_cast<ID3D11ComputeShader*>(m_PinSkinCS)->Release();         m_PinSkinCS = nullptr; }
    if (m_SpringIntegrateCS) { static_cast<ID3D11ComputeShader*>(m_SpringIntegrateCS)->Release();  m_SpringIntegrateCS = nullptr; }
    if (m_LooseDistanceCS)   { static_cast<ID3D11ComputeShader*>(m_LooseDistanceCS)->Release();    m_LooseDistanceCS = nullptr; }
    if (m_StrictDistanceCS)  { static_cast<ID3D11ComputeShader*>(m_StrictDistanceCS)->Release();   m_StrictDistanceCS = nullptr; }
    if (m_CollisionCS)       { static_cast<ID3D11ComputeShader*>(m_CollisionCS)->Release();        m_CollisionCS = nullptr; }
    if (m_ScatterDrawCS)     { static_cast<ID3D11ComputeShader*>(m_ScatterDrawCS)->Release();      m_ScatterDrawCS = nullptr; }
    if (m_PinCBuffer)        { static_cast<ID3D11Buffer*>(m_PinCBuffer)->Release();               m_PinCBuffer = nullptr; }
    if (m_IntegrateCBuffer)  { static_cast<ID3D11Buffer*>(m_IntegrateCBuffer)->Release();          m_IntegrateCBuffer = nullptr; }
    if (m_ConstraintCBuffer) { static_cast<ID3D11Buffer*>(m_ConstraintCBuffer)->Release();          m_ConstraintCBuffer = nullptr; }
    if (m_CollisionCBuffer)  { static_cast<ID3D11Buffer*>(m_CollisionCBuffer)->Release();           m_CollisionCBuffer = nullptr; }
    if (m_ScatterCBuffer)    { static_cast<ID3D11Buffer*>(m_ScatterCBuffer)->Release();             m_ScatterCBuffer = nullptr; }
#endif
    m_bInitialized = false;
    m_bSupported = false;
}

#ifdef RHI_D3D11_AVAILABLE
namespace {
    bool CreateStructuredSRVBuffer(ID3D11Device* device, const void* data, UINT elementCount, UINT stride,
        ID3D11Buffer** outBuf, ID3D11ShaderResourceView** outSRV)
    {
        *outBuf = nullptr;
        *outSRV = nullptr;
        if (elementCount == 0) return true; // legal empty buffer (e.g. no links) -- caller just won't dispatch anything reading it

        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = elementCount * stride;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        desc.StructureByteStride = stride;
        D3D11_SUBRESOURCE_DATA sub = {};
        sub.pSysMem = data;
        if (FAILED(device->CreateBuffer(&desc, &sub, outBuf))) return false;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.NumElements = elementCount;
        if (FAILED(device->CreateShaderResourceView(*outBuf, &srvDesc, outSRV))) return false;
        return true;
    }

    bool CreateStructuredUAVBuffer(ID3D11Device* device, const void* data, UINT elementCount, UINT stride,
        ID3D11Buffer** outBuf, ID3D11UnorderedAccessView** outUAV, ID3D11ShaderResourceView** outSRV)
    {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = elementCount * stride;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
        desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        desc.StructureByteStride = stride;
        D3D11_SUBRESOURCE_DATA sub = {};
        sub.pSysMem = data;
        if (FAILED(device->CreateBuffer(&desc, data ? &sub : nullptr, outBuf))) return false;

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Buffer.NumElements = elementCount;
        if (FAILED(device->CreateUnorderedAccessView(*outBuf, &uavDesc, outUAV))) return false;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.NumElements = elementCount;
        if (FAILED(device->CreateShaderResourceView(*outBuf, &srvDesc, outSRV))) return false;
        return true;
    }
}
#endif

bool CClothComputeShader::CreateInstance(Instance_t& inst, int vertexCount,
    const float* restLocalPos3, const uint32_t* pinned, const float* initialPos3,
    int linkCount, const int32_t* linkV0, const int32_t* linkV1,
    const float* linkDistMax, const float* linkDistMin, const uint32_t* linkStyle,
    const int* adjacencyOffsets, const int* adjacencyLinks, const float* uv2)
{
#ifdef RHI_D3D11_AVAILABLE
    if (!IsSupported() || vertexCount <= 0) return false;
    ID3D11Device* device = static_cast<ID3D11Device*>(RHI::GetD3D11Device());
    if (!device) return false;

    inst = Instance_t{};
    inst.vertexCount = vertexCount;
    inst.linkCount = linkCount;

    if (!CreateStructuredSRVBuffer(device, restLocalPos3, static_cast<UINT>(vertexCount), sizeof(float) * 3,
        reinterpret_cast<ID3D11Buffer**>(&inst.pRestLocalPosBuf), reinterpret_cast<ID3D11ShaderResourceView**>(&inst.pRestLocalPosSRV)))
        return false;

    if (!CreateStructuredSRVBuffer(device, pinned, static_cast<UINT>(vertexCount), sizeof(uint32_t),
        reinterpret_cast<ID3D11Buffer**>(&inst.pPinnedBuf), reinterpret_cast<ID3D11ShaderResourceView**>(&inst.pPinnedSRV)))
        return false;

    std::vector<GpuLinkGPU> links(linkCount > 0 ? linkCount : 0);
    for (int i = 0; i < linkCount; ++i)
    {
        links[i].v0 = linkV0[i];
        links[i].v1 = linkV1[i];
        links[i].distMax = linkDistMax[i];
        links[i].distMin = linkDistMin[i];
        links[i].style = linkStyle[i];
    }
    if (!CreateStructuredSRVBuffer(device, links.empty() ? nullptr : links.data(), static_cast<UINT>(linkCount), sizeof(GpuLinkGPU),
        reinterpret_cast<ID3D11Buffer**>(&inst.pLinksBuf), reinterpret_cast<ID3D11ShaderResourceView**>(&inst.pLinksSRV)))
        return false;

    if (!CreateStructuredSRVBuffer(device, adjacencyOffsets, static_cast<UINT>(vertexCount + 1), sizeof(int),
        reinterpret_cast<ID3D11Buffer**>(&inst.pAdjOffsetsBuf), reinterpret_cast<ID3D11ShaderResourceView**>(&inst.pAdjOffsetsSRV)))
        return false;

    const int adjLinkCount = adjacencyOffsets[vertexCount];
    if (!CreateStructuredSRVBuffer(device, adjacencyLinks, static_cast<UINT>(adjLinkCount), sizeof(int),
        reinterpret_cast<ID3D11Buffer**>(&inst.pAdjLinksBuf), reinterpret_cast<ID3D11ShaderResourceView**>(&inst.pAdjLinksSRV)))
        return false;

    std::vector<GpuVertexGPU> initial(vertexCount);
    for (int i = 0; i < vertexCount; ++i)
    {
        initial[i].pos[0] = initialPos3[i * 3 + 0];
        initial[i].pos[1] = initialPos3[i * 3 + 1];
        initial[i].pos[2] = initialPos3[i * 3 + 2];
        initial[i].vel[0] = initial[i].vel[1] = initial[i].vel[2] = 0.0f;
        initial[i].pinned = pinned[i];
        initial[i].pad = 0.0f;
    }
    for (int buf = 0; buf < 2; ++buf)
    {
        if (!CreateStructuredUAVBuffer(device, initial.data(), static_cast<UINT>(vertexCount), sizeof(GpuVertexGPU),
            reinterpret_cast<ID3D11Buffer**>(&inst.pVerticesBuf[buf]),
            reinterpret_cast<ID3D11UnorderedAccessView**>(&inst.pVerticesUAV[buf]),
            reinterpret_cast<ID3D11ShaderResourceView**>(&inst.pVerticesSRV[buf])))
            return false;
    }
    inst.FrontIndex = 0;

    D3D11_BUFFER_DESC stagingDesc = {};
    stagingDesc.ByteWidth = static_cast<UINT>(vertexCount) * sizeof(GpuVertexGPU);
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    ID3D11Buffer* stagingBuf = nullptr;
    if (FAILED(device->CreateBuffer(&stagingDesc, nullptr, &stagingBuf))) return false;
    inst.pStagingBuf = stagingBuf;

    if (uv2)
    {
        if (!CreateStructuredSRVBuffer(device, uv2, static_cast<UINT>(vertexCount), sizeof(float) * 2,
            reinterpret_cast<ID3D11Buffer**>(&inst.pUVBuf), reinterpret_cast<ID3D11ShaderResourceView**>(&inst.pUVSRV)))
            return false;

        if (!CreateStructuredUAVBuffer(device, nullptr, static_cast<UINT>(vertexCount), sizeof(DrawVertexGPU),
            reinterpret_cast<ID3D11Buffer**>(&inst.pDrawVertexBuf),
            reinterpret_cast<ID3D11UnorderedAccessView**>(&inst.pDrawVertexUAV),
            reinterpret_cast<ID3D11ShaderResourceView**>(&inst.pDrawVertexSRV)))
            return false;

        D3D11_BUFFER_DESC drawStagingDesc = {};
        drawStagingDesc.ByteWidth = static_cast<UINT>(vertexCount) * sizeof(DrawVertexGPU);
        drawStagingDesc.Usage = D3D11_USAGE_STAGING;
        drawStagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        ID3D11Buffer* drawStagingBuf = nullptr;
        if (FAILED(device->CreateBuffer(&drawStagingDesc, nullptr, &drawStagingBuf))) return false;
        inst.pDrawStagingBuf = drawStagingBuf;

        // Plain vertex-buffer-flagged copy target -- D3D11 forbids
        // D3D11_RESOURCE_MISC_BUFFER_STRUCTURED (pDrawVertexBuf, above) combined with
        // D3D11_BIND_VERTEX_BUFFER on the same resource, so this separate buffer exists purely to
        // be the CopyResource destination DispatchScatterDraw writes into every call, and the
        // thing actually registered with RHI below.
        D3D11_BUFFER_DESC vboDesc = {};
        vboDesc.ByteWidth = static_cast<UINT>(vertexCount) * sizeof(DrawVertexGPU);
        vboDesc.Usage = D3D11_USAGE_DEFAULT;
        vboDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        ID3D11Buffer* vboBuf = nullptr;
        if (FAILED(device->CreateBuffer(&vboDesc, nullptr, &vboBuf))) return false;
        inst.pDrawVertexVBOBuf = vboBuf;

        inst.drawVertexBufferHandle = RHI::RegisterExternalD3D11VertexBuffer(vboBuf, vboDesc.ByteWidth);
    }

    return true;
#else
    return false;
#endif
}

void CClothComputeShader::DestroyInstance(Instance_t& inst)
{
#ifdef RHI_D3D11_AVAILABLE
    auto release = [](void*& p) { if (p) { static_cast<IUnknown*>(p)->Release(); p = nullptr; } };
    release(inst.pRestLocalPosSRV); release(inst.pRestLocalPosBuf);
    release(inst.pPinnedSRV);       release(inst.pPinnedBuf);
    release(inst.pLinksSRV);        release(inst.pLinksBuf);
    release(inst.pAdjOffsetsSRV);   release(inst.pAdjOffsetsBuf);
    release(inst.pAdjLinksSRV);     release(inst.pAdjLinksBuf);
    for (int i = 0; i < 2; ++i)
    {
        release(inst.pVerticesUAV[i]);
        release(inst.pVerticesSRV[i]);
        release(inst.pVerticesBuf[i]);
    }
    release(inst.pStagingBuf);
    release(inst.pUVSRV);        release(inst.pUVBuf);
    release(inst.pDrawVertexUAV); release(inst.pDrawVertexSRV); release(inst.pDrawVertexBuf);
    release(inst.pDrawStagingBuf);
    if (inst.drawVertexBufferHandle.IsValid())
    {
        RHI::DestroyBuffer(inst.drawVertexBufferHandle); // drops RHI's AddRef; this Release() below drops ours
        inst.drawVertexBufferHandle = {};
    }
    release(inst.pDrawVertexVBOBuf);
#endif
    inst = Instance_t{};
}

void CClothComputeShader::DispatchPinSkin(Instance_t& inst, const float boneMatrix4x4[16])
{
#ifdef RHI_D3D11_AVAILABLE
    if (!IsSupported() || inst.vertexCount <= 0) return;
    ID3D11DeviceContext* ctx = static_cast<ID3D11DeviceContext*>(RHI::GetD3D11DeviceContext());
    if (!ctx) return;

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (SUCCEEDED(ctx->Map(static_cast<ID3D11Buffer*>(m_PinCBuffer), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        PinCB cb = {};
        memcpy(cb.boneMatrix, boneMatrix4x4, sizeof(cb.boneMatrix));
        cb.vertexCount = static_cast<uint32_t>(inst.vertexCount);
        memcpy(mapped.pData, &cb, sizeof(cb));
        ctx->Unmap(static_cast<ID3D11Buffer*>(m_PinCBuffer), 0);
    }

    ID3D11Buffer* cbuf = static_cast<ID3D11Buffer*>(m_PinCBuffer);
    ID3D11ShaderResourceView* srvs[] = { static_cast<ID3D11ShaderResourceView*>(inst.pRestLocalPosSRV) };
    ID3D11UnorderedAccessView* uav = static_cast<ID3D11UnorderedAccessView*>(inst.pVerticesUAV[inst.FrontIndex]);

    ctx->CSSetShader(static_cast<ID3D11ComputeShader*>(m_PinSkinCS), nullptr, 0);
    ctx->CSSetConstantBuffers(0, 1, &cbuf);
    ctx->CSSetShaderResources(0, 1, srvs);
    ctx->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

    const UINT groups = (static_cast<UINT>(inst.vertexCount) + 63) / 64;
    ctx->Dispatch(groups, 1, 1);

    // Unbind: D3D11 forbids a resource remaining bound as both UAV and SRV in a later call
    // (Pass B binds this same buffer as an SRV once it becomes the front buffer for reading).
    ID3D11UnorderedAccessView* nullUAV = nullptr;
    ctx->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
    ID3D11ShaderResourceView* nullSRV = nullptr;
    ctx->CSSetShaderResources(0, 1, &nullSRV);
    ctx->CSSetShader(nullptr, nullptr, 0);
#endif
}

void CClothComputeShader::DispatchSpringIntegrate(Instance_t& inst, float deltaTime, const TickParams& tick)
{
#ifdef RHI_D3D11_AVAILABLE
    if (!IsSupported() || inst.vertexCount <= 0) return;
    ID3D11DeviceContext* ctx = static_cast<ID3D11DeviceContext*>(RHI::GetD3D11DeviceContext());
    if (!ctx) return;

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (SUCCEEDED(ctx->Map(static_cast<ID3D11Buffer*>(m_IntegrateCBuffer), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        IntegrateCB cb = {};
        cb.vertexCount = static_cast<uint32_t>(inst.vertexCount);
        cb.deltaTime = deltaTime;
        cb.invMass = 400.0f;             // CPhysicsVertex::s_fInvOfMass, PhysicsManager.cpp
        cb.springFactor = 25.0f / 60.0f; // FPS_ANIMATION_FACTOR, ZzzAI.cpp
        memcpy(cb.windVec, tick.windVec, sizeof(cb.windVec));
        cb.fWind = tick.fWind;
        cb.worldTime = tick.worldTime;
        cb.numHor = tick.numHor > 0 ? tick.numHor : 1;
        cb.seed = tick.seed;
        cb.dwType = tick.dwType;
        memcpy(mapped.pData, &cb, sizeof(cb));
        ctx->Unmap(static_cast<ID3D11Buffer*>(m_IntegrateCBuffer), 0);
    }

    const int frontIdx = inst.FrontIndex;
    const int backIdx = 1 - frontIdx;

    ID3D11Buffer* cbuf = static_cast<ID3D11Buffer*>(m_IntegrateCBuffer);
    ID3D11ShaderResourceView* srvs[4] = {
        static_cast<ID3D11ShaderResourceView*>(inst.pLinksSRV),
        static_cast<ID3D11ShaderResourceView*>(inst.pAdjOffsetsSRV),
        static_cast<ID3D11ShaderResourceView*>(inst.pAdjLinksSRV),
        static_cast<ID3D11ShaderResourceView*>(inst.pVerticesSRV[frontIdx]),
    };
    ID3D11UnorderedAccessView* uav = static_cast<ID3D11UnorderedAccessView*>(inst.pVerticesUAV[backIdx]);

    ctx->CSSetShader(static_cast<ID3D11ComputeShader*>(m_SpringIntegrateCS), nullptr, 0);
    ctx->CSSetConstantBuffers(0, 1, &cbuf);
    ctx->CSSetShaderResources(0, 4, srvs);
    ctx->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

    const UINT groups = (static_cast<UINT>(inst.vertexCount) + 63) / 64;
    ctx->Dispatch(groups, 1, 1);

    ID3D11UnorderedAccessView* nullUAV = nullptr;
    ctx->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
    ID3D11ShaderResourceView* nullSRVs[4] = {};
    ctx->CSSetShaderResources(0, 4, nullSRVs);
    ctx->CSSetShader(nullptr, nullptr, 0);

    inst.FrontIndex = backIdx;
#endif
}

void CClothComputeShader::DispatchLooseDistance(Instance_t& inst)
{
#ifdef RHI_D3D11_AVAILABLE
    if (!IsSupported() || inst.vertexCount <= 0) return;
    ID3D11DeviceContext* ctx = static_cast<ID3D11DeviceContext*>(RHI::GetD3D11DeviceContext());
    if (!ctx) return;

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (SUCCEEDED(ctx->Map(static_cast<ID3D11Buffer*>(m_ConstraintCBuffer), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        ConstraintCB cb = {};
        cb.vertexCount = static_cast<uint32_t>(inst.vertexCount);
        cb.numHor = 0; // unused by Pass C
        memcpy(mapped.pData, &cb, sizeof(cb));
        ctx->Unmap(static_cast<ID3D11Buffer*>(m_ConstraintCBuffer), 0);
    }

    const int frontIdx = inst.FrontIndex;
    const int backIdx = 1 - frontIdx;

    ID3D11Buffer* cbuf = static_cast<ID3D11Buffer*>(m_ConstraintCBuffer);
    ID3D11ShaderResourceView* srvs[4] = {
        static_cast<ID3D11ShaderResourceView*>(inst.pLinksSRV),
        static_cast<ID3D11ShaderResourceView*>(inst.pAdjOffsetsSRV),
        static_cast<ID3D11ShaderResourceView*>(inst.pAdjLinksSRV),
        static_cast<ID3D11ShaderResourceView*>(inst.pVerticesSRV[frontIdx]),
    };
    ID3D11UnorderedAccessView* uav = static_cast<ID3D11UnorderedAccessView*>(inst.pVerticesUAV[backIdx]);

    ctx->CSSetShader(static_cast<ID3D11ComputeShader*>(m_LooseDistanceCS), nullptr, 0);
    ctx->CSSetConstantBuffers(0, 1, &cbuf);
    ctx->CSSetShaderResources(0, 4, srvs);
    ctx->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

    const UINT groups = (static_cast<UINT>(inst.vertexCount) + 63) / 64;
    ctx->Dispatch(groups, 1, 1);

    ID3D11UnorderedAccessView* nullUAV = nullptr;
    ctx->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
    ID3D11ShaderResourceView* nullSRVs[4] = {};
    ctx->CSSetShaderResources(0, 4, nullSRVs);
    ctx->CSSetShader(nullptr, nullptr, 0);

    inst.FrontIndex = backIdx;
#endif
}

void CClothComputeShader::DispatchStrictDistance(Instance_t& inst, int numHor, int iterations)
{
#ifdef RHI_D3D11_AVAILABLE
    if (!IsSupported() || inst.vertexCount <= 0 || iterations <= 0) return;
    ID3D11DeviceContext* ctx = static_cast<ID3D11DeviceContext*>(RHI::GetD3D11DeviceContext());
    if (!ctx) return;

    const UINT groups = (static_cast<UINT>(inst.vertexCount) + 63) / 64;

    for (int iter = 0; iter < iterations; ++iter)
    {
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        if (SUCCEEDED(ctx->Map(static_cast<ID3D11Buffer*>(m_ConstraintCBuffer), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
        {
            ConstraintCB cb = {};
            cb.vertexCount = static_cast<uint32_t>(inst.vertexCount);
            cb.numHor = static_cast<uint32_t>(numHor > 0 ? numHor : 1);
            memcpy(mapped.pData, &cb, sizeof(cb));
            ctx->Unmap(static_cast<ID3D11Buffer*>(m_ConstraintCBuffer), 0);
        }

        const int frontIdx = inst.FrontIndex;
        const int backIdx = 1 - frontIdx;

        ID3D11Buffer* cbuf = static_cast<ID3D11Buffer*>(m_ConstraintCBuffer);
        ID3D11ShaderResourceView* srvs[4] = {
            static_cast<ID3D11ShaderResourceView*>(inst.pLinksSRV),
            static_cast<ID3D11ShaderResourceView*>(inst.pAdjOffsetsSRV),
            static_cast<ID3D11ShaderResourceView*>(inst.pAdjLinksSRV),
            static_cast<ID3D11ShaderResourceView*>(inst.pVerticesSRV[frontIdx]),
        };
        ID3D11UnorderedAccessView* uav = static_cast<ID3D11UnorderedAccessView*>(inst.pVerticesUAV[backIdx]);

        ctx->CSSetShader(static_cast<ID3D11ComputeShader*>(m_StrictDistanceCS), nullptr, 0);
        ctx->CSSetConstantBuffers(0, 1, &cbuf);
        ctx->CSSetShaderResources(0, 4, srvs);
        ctx->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

        ctx->Dispatch(groups, 1, 1);

        ID3D11UnorderedAccessView* nullUAV = nullptr;
        ctx->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
        ID3D11ShaderResourceView* nullSRVs[4] = {};
        ctx->CSSetShaderResources(0, 4, nullSRVs);
        ctx->CSSetShader(nullptr, nullptr, 0);

        inst.FrontIndex = backIdx;
    }
#endif
}

void CClothComputeShader::DispatchCollision(Instance_t& inst, const float* sphereCenters3, const float* sphereRadii, int sphereCount)
{
#ifdef RHI_D3D11_AVAILABLE
    if (!IsSupported() || inst.vertexCount <= 0 || sphereCount <= 0) return;
    ID3D11DeviceContext* ctx = static_cast<ID3D11DeviceContext*>(RHI::GetD3D11DeviceContext());
    if (!ctx) return;

    if (sphereCount > kMaxCollisionSpheres) sphereCount = kMaxCollisionSpheres;

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (SUCCEEDED(ctx->Map(static_cast<ID3D11Buffer*>(m_CollisionCBuffer), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        CollisionCB cb = {};
        cb.vertexCount = static_cast<uint32_t>(inst.vertexCount);
        cb.sphereCount = static_cast<uint32_t>(sphereCount);
        for (int s = 0; s < sphereCount; ++s)
        {
            cb.sphereCenterRadius[s][0] = sphereCenters3[s * 3 + 0];
            cb.sphereCenterRadius[s][1] = sphereCenters3[s * 3 + 1];
            cb.sphereCenterRadius[s][2] = sphereCenters3[s * 3 + 2];
            cb.sphereCenterRadius[s][3] = sphereRadii[s];
        }
        memcpy(mapped.pData, &cb, sizeof(cb));
        ctx->Unmap(static_cast<ID3D11Buffer*>(m_CollisionCBuffer), 0);
    }

    const int frontIdx = inst.FrontIndex;
    const int backIdx = 1 - frontIdx;

    ID3D11Buffer* cbuf = static_cast<ID3D11Buffer*>(m_CollisionCBuffer);
    ID3D11ShaderResourceView* srv = static_cast<ID3D11ShaderResourceView*>(inst.pVerticesSRV[frontIdx]);
    ID3D11UnorderedAccessView* uav = static_cast<ID3D11UnorderedAccessView*>(inst.pVerticesUAV[backIdx]);

    ctx->CSSetShader(static_cast<ID3D11ComputeShader*>(m_CollisionCS), nullptr, 0);
    ctx->CSSetConstantBuffers(0, 1, &cbuf);
    ctx->CSSetShaderResources(0, 1, &srv);
    ctx->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

    const UINT groups = (static_cast<UINT>(inst.vertexCount) + 63) / 64;
    ctx->Dispatch(groups, 1, 1);

    ID3D11UnorderedAccessView* nullUAV = nullptr;
    ctx->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
    ID3D11ShaderResourceView* nullSRV = nullptr;
    ctx->CSSetShaderResources(0, 1, &nullSRV);
    ctx->CSSetShader(nullptr, nullptr, 0);

    inst.FrontIndex = backIdx;
#endif
}

bool CClothComputeShader::ReadBackVertices(Instance_t& inst, float* outPos3, float* outVel3)
{
#ifdef RHI_D3D11_AVAILABLE
    if (!IsSupported() || inst.vertexCount <= 0) return false;
    ID3D11DeviceContext* ctx = static_cast<ID3D11DeviceContext*>(RHI::GetD3D11DeviceContext());
    if (!ctx) return false;

    ctx->CopyResource(static_cast<ID3D11Buffer*>(inst.pStagingBuf), static_cast<ID3D11Buffer*>(inst.pVerticesBuf[inst.FrontIndex]));

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (FAILED(ctx->Map(static_cast<ID3D11Buffer*>(inst.pStagingBuf), 0, D3D11_MAP_READ, 0, &mapped))) return false;

    const GpuVertexGPU* verts = static_cast<const GpuVertexGPU*>(mapped.pData);
    for (int i = 0; i < inst.vertexCount; ++i)
    {
        if (outPos3) memcpy(&outPos3[i * 3], verts[i].pos, sizeof(float) * 3);
        if (outVel3) memcpy(&outVel3[i * 3], verts[i].vel, sizeof(float) * 3);
    }
    ctx->Unmap(static_cast<ID3D11Buffer*>(inst.pStagingBuf), 0);
    return true;
#else
    return false;
#endif
}

void CClothComputeShader::DispatchScatterDraw(Instance_t& inst, const float color4[4])
{
#ifdef RHI_D3D11_AVAILABLE
    if (!IsSupported() || inst.vertexCount <= 0 || !inst.pDrawVertexUAV) return;
    ID3D11DeviceContext* ctx = static_cast<ID3D11DeviceContext*>(RHI::GetD3D11DeviceContext());
    if (!ctx) return;

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (SUCCEEDED(ctx->Map(static_cast<ID3D11Buffer*>(m_ScatterCBuffer), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        ScatterCB cb = {};
        memcpy(cb.color, color4, sizeof(cb.color));
        cb.vertexCount = static_cast<uint32_t>(inst.vertexCount);
        memcpy(mapped.pData, &cb, sizeof(cb));
        ctx->Unmap(static_cast<ID3D11Buffer*>(m_ScatterCBuffer), 0);
    }

    ID3D11Buffer* cbuf = static_cast<ID3D11Buffer*>(m_ScatterCBuffer);
    ID3D11ShaderResourceView* srvs[2] = {
        static_cast<ID3D11ShaderResourceView*>(inst.pVerticesSRV[inst.FrontIndex]),
        static_cast<ID3D11ShaderResourceView*>(inst.pUVSRV),
    };
    ID3D11UnorderedAccessView* uav = static_cast<ID3D11UnorderedAccessView*>(inst.pDrawVertexUAV);

    ctx->CSSetShader(static_cast<ID3D11ComputeShader*>(m_ScatterDrawCS), nullptr, 0);
    ctx->CSSetConstantBuffers(0, 1, &cbuf);
    ctx->CSSetShaderResources(0, 2, srvs);
    ctx->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

    const UINT groups = (static_cast<UINT>(inst.vertexCount) + 63) / 64;
    ctx->Dispatch(groups, 1, 1);

    ID3D11UnorderedAccessView* nullUAV = nullptr;
    ctx->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
    ID3D11ShaderResourceView* nullSRVs[2] = {};
    ctx->CSSetShaderResources(0, 2, nullSRVs);
    ctx->CSSetShader(nullptr, nullptr, 0);

    // GPU-side bridge into the vertex-buffer-bindable copy (see Instance_t's comment: D3D11
    // forbids BUFFER_STRUCTURED + BIND_VERTEX_BUFFER on one resource). No CPU involvement.
    if (inst.pDrawVertexVBOBuf)
    {
        ctx->CopyResource(static_cast<ID3D11Buffer*>(inst.pDrawVertexVBOBuf), static_cast<ID3D11Buffer*>(inst.pDrawVertexBuf));
    }
#endif
}

bool CClothComputeShader::ReadBackDrawVertices(Instance_t& inst, float* outVertexData, bool fromVBO)
{
#ifdef RHI_D3D11_AVAILABLE
    if (!IsSupported() || inst.vertexCount <= 0 || !inst.pDrawVertexBuf || !inst.pDrawStagingBuf) return false;
    ID3D11DeviceContext* ctx = static_cast<ID3D11DeviceContext*>(RHI::GetD3D11DeviceContext());
    if (!ctx) return false;

    // fromVBO=true reads inst.pDrawVertexVBOBuf instead -- self-test-only path to verify
    // DispatchScatterDraw's CopyResource bridge (structured UAV -> plain vertex-buffer-flagged
    // buffer) actually landed the right bytes, not just that the structured source is correct.
    ID3D11Buffer* source = (fromVBO && inst.pDrawVertexVBOBuf)
        ? static_cast<ID3D11Buffer*>(inst.pDrawVertexVBOBuf)
        : static_cast<ID3D11Buffer*>(inst.pDrawVertexBuf);
    ctx->CopyResource(static_cast<ID3D11Buffer*>(inst.pDrawStagingBuf), source);

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (FAILED(ctx->Map(static_cast<ID3D11Buffer*>(inst.pDrawStagingBuf), 0, D3D11_MAP_READ, 0, &mapped))) return false;

    memcpy(outVertexData, mapped.pData, static_cast<size_t>(inst.vertexCount) * sizeof(DrawVertexGPU));
    ctx->Unmap(static_cast<ID3D11Buffer*>(inst.pDrawStagingBuf), 0);
    return true;
#else
    return false;
#endif
}

#ifdef RHI_D3D11_AVAILABLE
namespace {
    // Identity bone matrix, column_major (matches BMDMeshShader's convention).
    const float kIdentityMatrix[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };

    // Scenario 1 (Pass A+B): fixed 3-vertex chain, v0 pinned at origin, v1/v2 free, both spring
    // links pre-stretched by DIFFERENT amounts so their forces don't cancel at v1 (a symmetric
    // stretch would let a sign error hide behind a coincidental zero net force). dwType=0
    // (PCT_COTTON) and windVec/fWind=0 so gravity's unconditional default-weight term is the only
    // new-vs-Phase-1 contribution, isolating what Phase 2 actually changed in this pass.
    bool RunSpringIntegrateScenario()
    {
        constexpr int kVertexCount = 3;
        const float restLocalPos[kVertexCount * 3] = { 0,0,0,  0,0,0,  0,0,0 }; // only v0 (pinned) reads this
        const uint32_t pinned[kVertexCount] = { 1, 0, 0 };
        const float initialPos[kVertexCount * 3] = {
            0.0f, 0.0f, 0.0f,   // v0
            1.5f, 0.0f, 0.0f,   // v1 -- link0 rest length 1.0, stretched to 1.5
            3.5f, 0.0f, 0.0f,   // v2 -- link1 rest length 1.0, stretched to 2.0 (from v1)
        };
        constexpr int kLinkCount = 2;
        const int32_t linkV0[kLinkCount] = { 0, 1 };
        const int32_t linkV1[kLinkCount] = { 1, 2 };
        const float linkDistMax[kLinkCount] = { 1.0f, 1.0f };
        const float linkDistMin[kLinkCount] = { 1.0f, 1.0f }; // unused by Pass B
        const uint32_t linkStyle[kLinkCount] = { 0x02u /*PLS_SPRING*/, 0x02u };

        // CSR adjacency built the same way Phase 0's BuildAdjacencyCSR would for this link set:
        // v0 -> [link0], v1 -> [link0, link1], v2 -> [link1].
        const int adjOffsets[kVertexCount + 1] = { 0, 1, 3, 4 };
        const int adjLinks[4] = { 0, 0, 1, 1 };

        CClothComputeShader::Instance_t inst;
        if (!CClothComputeShader::Instance().CreateInstance(inst, kVertexCount, restLocalPos, pinned, initialPos,
            kLinkCount, linkV0, linkV1, linkDistMax, linkDistMin, linkStyle, adjOffsets, adjLinks))
        {
            g_ErrorReport.Write(L"[ClothComputeShader] Self-test FAILED: CreateInstance (spring/integrate scenario) failed.\r\n");
            return false;
        }

        constexpr float kDeltaTime = 0.05f;
        CClothComputeShader::TickParams tick; // defaults: dwType=0, windVec=0, fWind=0, worldTime=0, numHor=3, seed=0

        CClothComputeShader::Instance().DispatchPinSkin(inst, kIdentityMatrix);
        CClothComputeShader::Instance().DispatchSpringIntegrate(inst, kDeltaTime, tick);

        float gpuPos[kVertexCount * 3] = {};
        float gpuVel[kVertexCount * 3] = {};
        const bool readOk = CClothComputeShader::Instance().ReadBackVertices(inst, gpuPos, gpuVel);
        CClothComputeShader::Instance().DestroyInstance(inst);

        if (!readOk)
        {
            g_ErrorReport.Write(L"[ClothComputeShader] Self-test FAILED: ReadBackVertices (spring/integrate scenario) failed.\r\n");
            return false;
        }

        // Hand-derived CPU reference: x-axis unchanged from Phase 1's derivation (spring-only
        // math didn't change). z-axis is new -- Phase 1 had no gravity term at all; Phase 2's
        // UpdateForce equivalent adds an unconditional default-weight gravity term, identical for
        // both non-pinned vertices since it depends only on dwType (constant here), not position:
        // force.z = -(9.8 * 0.0025 * 100 * 25/60) = -1.020833
        // vel.z   += force.z * invMass(400) * dt(0.05) = -20.416667
        // pos.z   += vel.z * dt                        = -1.020833
        const float expectedPos[kVertexCount * 3] = {
            0.0f, 0.0f, 0.0f,
            1.708333f, 0.0f, -1.020833f,
            3.083333f, 0.0f, -1.020833f,
        };
        const float expectedVel[kVertexCount * 3] = {
            0.0f, 0.0f, 0.0f,
            4.166667f, 0.0f, -20.416667f,
            -8.333333f, 0.0f, -20.416667f,
        };

        constexpr float kEpsilon = 0.002f;
        bool pass = true;
        for (int i = 0; i < kVertexCount * 3; ++i)
        {
            if (fabsf(gpuPos[i] - expectedPos[i]) > kEpsilon) pass = false;
            if (fabsf(gpuVel[i] - expectedVel[i]) > kEpsilon) pass = false;
        }

        g_ErrorReport.Write(L"[ClothComputeShader] Self-test (Pass A/B, spring+gravity) %hs\r\n", pass ? "PASSED" : "FAILED");
        for (int i = 0; i < kVertexCount; ++i)
        {
            g_ErrorReport.Write(L"  v%d pos=(%.6f,%.6f,%.6f) expected=(%.6f,%.6f,%.6f) vel=(%.6f,%.6f,%.6f) expected=(%.6f,%.6f,%.6f)\r\n",
                i,
                gpuPos[i*3+0], gpuPos[i*3+1], gpuPos[i*3+2],
                expectedPos[i*3+0], expectedPos[i*3+1], expectedPos[i*3+2],
                gpuVel[i*3+0], gpuVel[i*3+1], gpuVel[i*3+2],
                expectedVel[i*3+0], expectedVel[i*3+1], expectedVel[i*3+2]);
        }
        return pass;
    }

    // Scenario 2 (Pass C+D): fixed 4-vertex, 1-wide "grid" (numHor=1) column, v0 pinned at
    // origin. Link A (v0-v1) and Link B (v1-v2) are STRICTDISTANCE only (distMin=0.8,
    // distMax=1.0); Link Cx (v2-v3) is LOOSEDISTANCE only (distMax=1.0) -- disjoint link styles so
    // Pass C and Pass D exercise independent parts of this topology in one dispatch sequence,
    // still through the shared CSR-gather machinery. No Pass B run here (this is a position-only
    // constraint-solve test, not a tick) -- initialPos stands in for "already post-integration"
    // positions.
    bool RunConstraintScenario()
    {
        constexpr int kVertexCount = 4;
        const float restLocalPos[kVertexCount * 3] = { 0,0,0,  0,0,0,  0,0,0,  0,0,0 }; // only v0 (pinned) reads this
        const uint32_t pinned[kVertexCount] = { 1, 0, 0, 0 };
        const float initialPos[kVertexCount * 3] = {
            0.0f, 0.0f,  0.0f,   // v0
            0.0f, 0.0f, -1.5f,   // v1 -- 1.5 from v0, beyond STRICTDISTANCE max 1.0
            0.0f, 0.0f, -3.5f,   // v2 -- 2.0 from v1, beyond STRICTDISTANCE max 1.0
            0.0f, 0.0f, -4.6f,   // v3 -- 1.1 from v2, beyond LOOSEDISTANCE target 1.0
        };
        constexpr int kLinkCount = 3;
        const int32_t linkV0[kLinkCount] = { 0, 1, 2 };
        const int32_t linkV1[kLinkCount] = { 1, 2, 3 };
        const float linkDistMax[kLinkCount] = { 1.0f, 1.0f, 1.0f };
        const float linkDistMin[kLinkCount] = { 0.8f, 0.8f, 1.0f }; // Cx's distMin unused by Pass C
        const uint32_t linkStyle[kLinkCount] = { 0x04u /*STRICTDISTANCE*/, 0x04u, 0x01u /*LOOSEDISTANCE*/ };

        // v0->[A], v1->[A,B], v2->[B,Cx], v3->[Cx]
        const int adjOffsets[kVertexCount + 1] = { 0, 1, 3, 5, 6 };
        const int adjLinks[6] = { 0, 0, 1, 1, 2, 2 };

        CClothComputeShader::Instance_t inst;
        if (!CClothComputeShader::Instance().CreateInstance(inst, kVertexCount, restLocalPos, pinned, initialPos,
            kLinkCount, linkV0, linkV1, linkDistMax, linkDistMin, linkStyle, adjOffsets, adjLinks))
        {
            g_ErrorReport.Write(L"[ClothComputeShader] Self-test FAILED: CreateInstance (constraint scenario) failed.\r\n");
            return false;
        }

        constexpr int kNumHor = 1;
        constexpr int kStrictIterations = 4; // see DispatchStrictDistance's comment -- tuning parameter, not a proof of convergence

        CClothComputeShader::Instance().DispatchPinSkin(inst, kIdentityMatrix);
        CClothComputeShader::Instance().DispatchLooseDistance(inst);
        CClothComputeShader::Instance().DispatchStrictDistance(inst, kNumHor, kStrictIterations);

        float gpuPos[kVertexCount * 3] = {};
        const bool readOk = CClothComputeShader::Instance().ReadBackVertices(inst, gpuPos, nullptr);
        CClothComputeShader::Instance().DestroyInstance(inst);

        if (!readOk)
        {
            g_ErrorReport.Write(L"[ClothComputeShader] Self-test FAILED: ReadBackVertices (constraint scenario) failed.\r\n");
            return false;
        }

        // Hand-derived reference, traced through Pass C then 4 Pass-D Jacobi sub-iterations (full
        // derivation in the DXP-21 task memory's Phase 2 result section):
        // Pass C (LOOSEDISTANCE, v2-v3 only, single non-iterative gather): dist=1.1, target=1.0,
        //   symmetric half-correction -> v2 -= 0.05 (to -3.55), v3 += 0.05 (to -4.55).
        // Pass D iter1: Link A sees v0=0 (pinned, anchor) -> v1 corrected 1.5->1.0 exactly (single
        //   hop from a fixed anchor converges in one iteration). Link B sees v1 STALE (still
        //   -1.5, this iteration's Front) vs v2=-3.55 -> corrected toward max 1.0 -> v2 = -2.5.
        // Pass D iter2: Link A: v1 already at distance 1.0 from pinned v0, no further change.
        //   Link B: v1 now -1.0 (this iteration's Front) vs v2=-2.5, dist=1.5 -> corrected -> v2 = -2.0.
        // Pass D iter3: Link B: v1=-1.0 vs v2=-2.0, dist=1.0 exactly -> no further change. Converged.
        // Pass D iter4: no-op (already converged). v3 untouched by Pass D (no STRICTDISTANCE link).
        const float expectedPos[kVertexCount * 3] = {
            0.0f, 0.0f,  0.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -2.0f,
            0.0f, 0.0f, -4.55f,
        };

        constexpr float kEpsilon = 0.002f;
        bool pass = true;
        for (int i = 0; i < kVertexCount * 3; ++i)
        {
            if (fabsf(gpuPos[i] - expectedPos[i]) > kEpsilon) pass = false;
        }

        g_ErrorReport.Write(L"[ClothComputeShader] Self-test (Pass C/D, loose+strict distance) %hs\r\n", pass ? "PASSED" : "FAILED");
        for (int i = 0; i < kVertexCount; ++i)
        {
            g_ErrorReport.Write(L"  v%d pos=(%.6f,%.6f,%.6f) expected=(%.6f,%.6f,%.6f)\r\n",
                i,
                gpuPos[i*3+0], gpuPos[i*3+1], gpuPos[i*3+2],
                expectedPos[i*3+0], expectedPos[i*3+1], expectedPos[i*3+2]);
        }
        return pass;
    }

    // Scenario 2b (Pass F): 2-vertex, no links (collision push-out is link-independent). v0
    // pinned at origin (unused by collision -- pinned vertices are skipped, same as every other
    // pass), v1 free, placed 0.5 units inside a radius-1.0 sphere centered 2 units along -Z from
    // v0. Direct translation of CPhysicsColSphere::ProcessCollision's own math, hand-traced:
    // dir = pos-center = (0,0,-0.5), dist = 0.5 < radius(1.0) -> pos += dir*(radius-dist)/dist =
    // dir*1.0 -> new pos (0,0,-3.0), exactly on the sphere surface (distance from center = 1.0).
    bool RunCollisionScenario()
    {
        constexpr int kVertexCount = 2;
        const float restLocalPos[kVertexCount * 3] = { 0,0,0,  0,0,0 };
        const uint32_t pinned[kVertexCount] = { 1, 0 };
        const float initialPos[kVertexCount * 3] = {
            0.0f, 0.0f, 0.0f,    // v0
            0.0f, 0.0f, -2.5f,   // v1 -- 0.5 inside the sphere below
        };
        const int adjOffsets[kVertexCount + 1] = { 0, 0, 0 }; // no links

        CClothComputeShader::Instance_t inst;
        if (!CClothComputeShader::Instance().CreateInstance(inst, kVertexCount, restLocalPos, pinned, initialPos,
            0, nullptr, nullptr, nullptr, nullptr, nullptr, adjOffsets, nullptr))
        {
            g_ErrorReport.Write(L"[ClothComputeShader] Self-test FAILED: CreateInstance (collision scenario) failed.\r\n");
            return false;
        }

        const float sphereCenters[3] = { 0.0f, 0.0f, -2.0f };
        const float sphereRadii[1] = { 1.0f };
        CClothComputeShader::Instance().DispatchCollision(inst, sphereCenters, sphereRadii, 1);

        float gpuPos[kVertexCount * 3] = {};
        const bool readOk = CClothComputeShader::Instance().ReadBackVertices(inst, gpuPos, nullptr);
        CClothComputeShader::Instance().DestroyInstance(inst);

        if (!readOk)
        {
            g_ErrorReport.Write(L"[ClothComputeShader] Self-test FAILED: ReadBackVertices (collision scenario) failed.\r\n");
            return false;
        }

        const float expectedPos[kVertexCount * 3] = {
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, -3.0f,
        };

        constexpr float kEpsilon = 0.002f;
        bool pass = true;
        for (int i = 0; i < kVertexCount * 3; ++i)
        {
            if (fabsf(gpuPos[i] - expectedPos[i]) > kEpsilon) pass = false;
        }

        g_ErrorReport.Write(L"[ClothComputeShader] Self-test (Pass F, sphere collision) %hs\r\n", pass ? "PASSED" : "FAILED");
        for (int i = 0; i < kVertexCount; ++i)
        {
            g_ErrorReport.Write(L"  v%d pos=(%.6f,%.6f,%.6f) expected=(%.6f,%.6f,%.6f)\r\n",
                i, gpuPos[i*3+0], gpuPos[i*3+1], gpuPos[i*3+2],
                expectedPos[i*3+0], expectedPos[i*3+1], expectedPos[i*3+2]);
        }
        return pass;
    }

    // Scenario 3 (Pass E): reuses scenario 1's 3-vertex chain post Pass A+B, then scatters into
    // the draw buffer with a fixed UV/color. Pass E is a pure gather-and-repack (no math to get
    // wrong beyond wiring), so the check is just "output equals input, per vertex, in the right
    // slot order" -- pos must match the GPU positions ReadBackVertices already validated in
    // scenario 1, uv/color must match what was fed in.
    bool RunScatterDrawScenario()
    {
        constexpr int kVertexCount = 3;
        const float restLocalPos[kVertexCount * 3] = { 0,0,0,  0,0,0,  0,0,0 };
        const uint32_t pinned[kVertexCount] = { 1, 0, 0 };
        const float initialPos[kVertexCount * 3] = {
            0.0f, 0.0f, 0.0f,
            1.5f, 0.0f, 0.0f,
            3.5f, 0.0f, 0.0f,
        };
        constexpr int kLinkCount = 2;
        const int32_t linkV0[kLinkCount] = { 0, 1 };
        const int32_t linkV1[kLinkCount] = { 1, 2 };
        const float linkDistMax[kLinkCount] = { 1.0f, 1.0f };
        const float linkDistMin[kLinkCount] = { 1.0f, 1.0f };
        const uint32_t linkStyle[kLinkCount] = { 0x02u, 0x02u };
        const int adjOffsets[kVertexCount + 1] = { 0, 1, 3, 4 };
        const int adjLinks[4] = { 0, 0, 1, 1 };
        const float uv[kVertexCount * 2] = { 0.0f, 0.0f,  0.5f, 0.0f,  1.0f, 0.0f };
        const float color[4] = { 1.0f, 0.5f, 0.25f, 1.0f };

        CClothComputeShader::Instance_t inst;
        if (!CClothComputeShader::Instance().CreateInstance(inst, kVertexCount, restLocalPos, pinned, initialPos,
            kLinkCount, linkV0, linkV1, linkDistMax, linkDistMin, linkStyle, adjOffsets, adjLinks, uv))
        {
            g_ErrorReport.Write(L"[ClothComputeShader] Self-test FAILED: CreateInstance (scatter-draw scenario) failed.\r\n");
            return false;
        }

        constexpr float kDeltaTime = 0.05f;
        CClothComputeShader::TickParams tick;

        CClothComputeShader::Instance().DispatchPinSkin(inst, kIdentityMatrix);
        CClothComputeShader::Instance().DispatchSpringIntegrate(inst, kDeltaTime, tick);

        float expectedPos[kVertexCount * 3] = {};
        const bool posReadOk = CClothComputeShader::Instance().ReadBackVertices(inst, expectedPos, nullptr);

        CClothComputeShader::Instance().DispatchScatterDraw(inst, color);

        float drawVerts[kVertexCount * 9] = {};
        const bool drawReadOk = CClothComputeShader::Instance().ReadBackDrawVertices(inst, drawVerts);

        // Phase 3b: also verify the CopyResource bridge into the plain vertex-buffer-flagged
        // buffer (the one actually registered with RHI) landed identical bytes, and that the
        // registration itself produced a usable handle.
        float vboVerts[kVertexCount * 9] = {};
        const bool vboReadOk = CClothComputeShader::Instance().ReadBackDrawVertices(inst, vboVerts, /*fromVBO=*/true);
        const bool handleValid = inst.drawVertexBufferHandle.IsValid();

        CClothComputeShader::Instance().DestroyInstance(inst);

        if (!posReadOk || !drawReadOk || !vboReadOk)
        {
            g_ErrorReport.Write(L"[ClothComputeShader] Self-test FAILED: readback (scatter-draw scenario) failed.\r\n");
            return false;
        }
        if (!handleValid)
        {
            g_ErrorReport.Write(L"[ClothComputeShader] Self-test FAILED: drawVertexBufferHandle invalid after CreateInstance (scatter-draw scenario).\r\n");
            return false;
        }

        constexpr float kEpsilon = 0.002f;
        bool pass = true;
        for (int i = 0; i < kVertexCount; ++i)
        {
            const float* dv = &drawVerts[i * 9];
            const float* vv = &vboVerts[i * 9];
            for (int c = 0; c < 3; ++c) if (fabsf(dv[c] - expectedPos[i * 3 + c]) > kEpsilon) pass = false;
            if (fabsf(dv[3] - uv[i * 2 + 0]) > kEpsilon) pass = false;
            if (fabsf(dv[4] - uv[i * 2 + 1]) > kEpsilon) pass = false;
            for (int c = 0; c < 4; ++c) if (fabsf(dv[5 + c] - color[c]) > kEpsilon) pass = false;
            for (int c = 0; c < 9; ++c) if (fabsf(dv[c] - vv[c]) > kEpsilon) pass = false; // VBO copy must match structured source exactly
        }

        g_ErrorReport.Write(L"[ClothComputeShader] Self-test (Pass E, scatter-draw + VBO copy + RHI registration) %hs\r\n", pass ? "PASSED" : "FAILED");
        for (int i = 0; i < kVertexCount; ++i)
        {
            const float* dv = &drawVerts[i * 9];
            const float* vv = &vboVerts[i * 9];
            g_ErrorReport.Write(L"  v%d pos=(%.6f,%.6f,%.6f) expected=(%.6f,%.6f,%.6f) uv=(%.6f,%.6f) color=(%.4f,%.4f,%.4f,%.4f) vbo_pos=(%.6f,%.6f,%.6f)\r\n",
                i, dv[0], dv[1], dv[2], expectedPos[i*3+0], expectedPos[i*3+1], expectedPos[i*3+2],
                dv[3], dv[4], dv[5], dv[6], dv[7], dv[8], vv[0], vv[1], vv[2]);
        }
        return pass;
    }
} // namespace
#endif // RHI_D3D11_AVAILABLE

void CClothComputeShader::RunSelfTestAndLog()
{
#ifdef RHI_D3D11_AVAILABLE
    if (!Init())
    {
        g_ErrorReport.Write(L"[ClothComputeShader] Self-test skipped: shader compile/init failed.\r\n");
        return;
    }

    const bool pass1 = RunSpringIntegrateScenario();
    const bool pass2 = RunConstraintScenario();
    const bool pass2b = RunCollisionScenario();
    const bool pass3 = RunScatterDrawScenario();

    g_ErrorReport.Write(L"[ClothComputeShader] Self-test overall %hs\r\n", (pass1 && pass2 && pass2b && pass3) ? "PASSED" : "FAILED");
#else
    g_ErrorReport.Write(L"[ClothComputeShader] Self-test skipped: not built with RHI_D3D11_AVAILABLE.\r\n");
#endif
}
