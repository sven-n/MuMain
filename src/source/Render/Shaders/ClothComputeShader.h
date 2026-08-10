#ifndef CLOTH_COMPUTE_SHADER_H
#define CLOTH_COMPUTE_SHADER_H

#pragma once

#include "stdafx.h"
#include "Render/RHI/RHI.h"
#include <cstdint>

// DXP-21 part 1 phase 1: GPU cloth compute infra, D3D11 only -- device/context obtained via
// RHI::GetD3D11Device()/GetD3D11DeviceContext(), the same escape hatch every shader class uses for
// backend-specific object creation. Never implemented under GL -- not stubbed, simply omitted;
// there is no GL branch anywhere in this class, unlike PlanarShadowShader/BMDMeshShader/etc.
// Phase 3b needed one small RHI.h addition (RegisterExternalD3D11VertexBuffer) to bridge Pass E's
// compute-owned buffer into RHI's draw-time buffer table -- see that function's RHI.h comment; the
// original "no RHI.h changes" framing from Phase 0-2 didn't anticipate needing a draw-time handle.
//
// Scope today (through Phase 3b): Pass A (pin-skin), Pass B (CSR-gather spring force +
// gravity/wind + semi-implicit-Euler integrate), Pass C (CSR-gather LOOSEDISTANCE soft
// constraint), Pass D (STRICTDISTANCE hard clamp via N Jacobi-relaxation sub-iterations), Pass E
// (draw-buffer scatter -- GpuVertex.pos + a static per-vertex UV + a constant color -> a
// PosUvColor-shaped [RHI.h VertexLayout::PosUvColor / ImmediateRenderer.cpp's IRVertex] output
// buffer, GPU-copied into a plain vertex-buffer-flagged buffer and registered with RHI via
// Instance_t::drawVertexBufferHandle). NOT wired to CPhysicsCloth or the render loop yet --
// CreateInstance/Dispatch* take plain arrays, not a CPhysicsCloth*, and nothing calls
// RHI::BindVertexBuffer/DrawIndexed with drawVertexBufferHandle yet -- no index buffer exists
// (per-cloth, topology-dependent, deliberately left to the eventual live-wiring caller, see below)
// and no draw call reads this buffer. See DXP-21-gpu-cloth-d3d11-compute.md's "Correction to the
// Phase 3 draw-wiring plan" and "Phase 3b" sections -- Phase 3a/3b (this) is the self-tested
// compute+buffer-plumbing half only; Phase 3c (per-cloth index buffer + live ZzzCharacter.cpp
// wiring) is the deliberately-deferred, higher-blast-radius remainder. RunSelfTestAndLog() is the
// only caller today, gated behind config.ini [Render] ClothComputeSelfTest=1 (off by default).
//
// Phase 3c correction: collision (Pass F, below) DOES exist on GPU now -- the "no collision"
// framing below is Phase 0-2 history, kept for context, not current scope. Discovered while
// live-wiring that ADD_COLLISION is actually #define'd (PhysicsManager.cpp:16, easy to miss) and
// dozens of real capes/cloaks attach 1-4 CPhysicsColSphere volumes each -- collision was the
// dominant case for real cloth, not a rare edge case, so it needed a real pass, not just a
// documented gap.
//
// Phase 2 scope gaps (deliberate, documented -- not bugs):
// - Pass C's LOOSEDISTANCE average divides by the count of incident LOOSEDISTANCE links only,
//   where CPU's iCountOneTimeMove also folds in collision-triggered one-time-moves into the SAME
//   average (CPU: ProcessCollision() and AddOneTimeMoveToKeepLength() both feed one shared
//   m_vOneTimeMove/m_iCountOneTimeMove accumulator per vertex, applied once via DoOneTimeMove()).
//   Pass F (below) runs collision as its own separate sequential dispatch instead of merging into
//   Pass C's average -- an approximation in the same spirit as Pass D's Jacobi-vs-Gauss-Seidel
//   note, not bit-identical to CPU's simultaneous-average semantics. Visual-tuning/correctness
//   revisit candidate for Phase 4, not wrong enough to block Phase 3c on.
// - InitForces' iKey/fRand spatial-attenuation math (UpdateForce's wind gating) is the
//   CPhysicsCloth (grid-topology, dominant case) formula only, not CPhysicsClothMesh's
//   mesh-topology iKey variant (Magic Skeleton cloak, the one non-grid case, see the task
//   memory's topology note) -- same dominant-case-first scope the CSR adjacency builder itself
//   already established in Phase 0.
// - Pass D is N fixed Jacobi-relaxation sub-iterations, standing in for the CPU's single
//   sequential (Gauss-Seidel, top-down row order) sweep -- will not be bit-identical to CPU,
//   iteration count is a visual-tuning parameter for Phase 4's A/B soak, not a numerical-
//   equivalence target. See DispatchStrictDistance's comment.
//
// Race-hazard note (why the vertex buffer is ping-ponged, not single-buffered): Pass B is a
// gather -- each thread reads its spring-linked neighbors' CURRENT positions. If neighbors lived
// in the same buffer a thread is also writing to, threads racing within one Dispatch could read a
// neighbor's value before or after that neighbor's own thread updates it (undefined, order-
// dependent). Pass A has no such hazard (each thread only ever touches its own vertex, driven by
// a shared constant, not neighbor state) and runs safely in-place on a single buffer.
class CClothComputeShader
{
public:
    static CClothComputeShader& Instance();

    bool Init();
    void Shutdown();
    bool IsSupported() const { return m_bInitialized && m_bSupported; }

    // Opaque per-cloth-instance GPU resource set. Caller owns the lifetime via
    // CreateInstance/DestroyInstance. All pointers are ID3D11* under the hood (void* here so this
    // header stays platform-generic, matching PlanarShadowShader.h's m_D3DVertexShader pattern).
    struct Instance_t
    {
        void* pRestLocalPosSRV = nullptr;  // StructuredBuffer<float3>, per-vertex rest-local position (Pass A input)
        void* pRestLocalPosBuf = nullptr;
        void* pPinnedSRV       = nullptr;  // StructuredBuffer<uint>, 1 = pinned (Pass A writes it, Pass B skips it)
        void* pPinnedBuf       = nullptr;
        void* pLinksSRV        = nullptr;  // StructuredBuffer<GpuLink>
        void* pLinksBuf        = nullptr;
        void* pAdjOffsetsSRV   = nullptr;  // StructuredBuffer<uint>, size vertexCount+1 (CSR, DXP-21 phase 0's shape)
        void* pAdjOffsetsBuf   = nullptr;
        void* pAdjLinksSRV     = nullptr;  // StructuredBuffer<uint>, size adjacencyOffsets[vertexCount]
        void* pAdjLinksBuf     = nullptr;

        // Ping-pong GpuVertex{pos,vel,pinned} pair. m_FrontIndex names the buffer holding the most
        // recently finalized tick's data (readable any time); the other is scratch, only ever
        // written mid-dispatch, never read until it becomes front after a swap.
        void* pVerticesBuf[2] = { nullptr, nullptr };
        void* pVerticesUAV[2] = { nullptr, nullptr };
        void* pVerticesSRV[2] = { nullptr, nullptr };
        void* pStagingBuf     = nullptr;   // CPU-readable CopyResource target for ReadBackVertices
        int   FrontIndex = 0;

        // Pass E (draw-buffer scatter) resources -- created only when CreateInstance is given a
        // non-null uv2 array. DrawVertexGPU's layout is bit-identical to RHI.h
        // VertexLayout::PosUvColor / ImmediateRenderer.cpp's IRVertex (pos3+uv2+rgba4, 36 bytes).
        // pDrawVertexBuf is the compute-writable structured UAV Pass E scatters into; D3D11
        // forbids D3D11_RESOURCE_MISC_BUFFER_STRUCTURED combined with D3D11_BIND_VERTEX_BUFFER on
        // the same resource, so it can never be bound as a vertex buffer directly. DispatchScatterDraw
        // therefore CopyResource's it into pDrawVertexVBOBuf (plain, no UAV/structured flag, just
        // D3D11_BIND_VERTEX_BUFFER) each call -- a GPU-side copy, no CPU readback. drawVertexBufferHandle
        // is pDrawVertexVBOBuf registered into RHI via RHI::RegisterExternalD3D11VertexBuffer at
        // CreateInstance time -- the handle a live draw call would pass to RHI::BindVertexBuffer.
        // pDrawStagingBuf exists purely for ReadBackDrawVertices's self-test readback.
        void* pUVSRV              = nullptr;   // StructuredBuffer<float2>, per-vertex UV (Pass E input, static)
        void* pUVBuf              = nullptr;
        void* pDrawVertexBuf      = nullptr;   // RWStructuredBuffer<DrawVertex>, Pass E compute output
        void* pDrawVertexUAV      = nullptr;
        void* pDrawVertexSRV      = nullptr;
        void* pDrawVertexVBOBuf   = nullptr;   // plain copy target, vertex-buffer-bindable
        RHI::BufferHandle drawVertexBufferHandle; // pDrawVertexVBOBuf registered with RHI (invalid if uv2 was null)
        void* pDrawStagingBuf     = nullptr;

        int vertexCount = 0;
        int linkCount   = 0;
    };

    // Creates all GPU resources for one cloth instance from plain CPU-side arrays (shape mirrors
    // CPhysicsCloth's m_pVertices/m_pLink/m_pAdjacencyOffsets/m_pAdjacencyLinks -- not the class
    // itself, wiring to real cloth data is Phase 3).
    bool CreateInstance(Instance_t& inst, int vertexCount,
        const float* restLocalPos3,        // vertexCount*3 floats, bone-local rest position
        const uint32_t* pinned,            // vertexCount, 1 = pinned
        const float* initialPos3,          // vertexCount*3 floats, seeds the front buffer
        int linkCount,
        const int32_t* linkV0, const int32_t* linkV1,
        const float* linkDistMax,          // St_PhysicsLink::m_fDistance[1] -- spring engage threshold / LOOSEDISTANCE target / STRICTDISTANCE upper clamp
        const float* linkDistMin,          // St_PhysicsLink::m_fDistance[0] -- STRICTDISTANCE lower clamp only (Pass C/D). Unused by Pass B/spring.
        const uint32_t* linkStyle,         // PLS_* bits (PhysicsManager.h): SPRING=0x02 (Pass B), LOOSEDISTANCE=0x01 (Pass C), STRICTDISTANCE=0x04 (Pass D)
        const int* adjacencyOffsets,       // vertexCount+1 (DXP-21 phase 0's CSR shape)
        const int* adjacencyLinks,         // adjacencyOffsets[vertexCount]
        const float* uv2 = nullptr);       // vertexCount*2 floats, static per-vertex UV (Pass E only). Null skips creating Pass E resources entirely -- Phase 0-2 callers are unaffected.
    void DestroyInstance(Instance_t& inst);

    // Pass A: pos = boneMatrix * restLocalPos for pinned vertices only, in-place on the front
    // buffer. boneMatrix4x4 is column_major (RHI.h/BMDMeshShader HLSL convention), 16 floats.
    void DispatchPinSkin(Instance_t& inst, const float boneMatrix4x4[16]);

    // Per-tick scalars Pass B needs beyond the per-vertex/per-link static topology -- mirrors what
    // CPhysicsCloth::Move()/InitForces computes CPU-side once per tick before MoveVertices(): wind
    // RNG stays CPU-side (CPhysicsManager::s_vWind/m_fWind), only the result crosses to GPU, per
    // the task memory's "GPU data layout" plan. numHor must be >= 1 (grid width; also feeds
    // DispatchStrictDistance's row-0 gate) -- CPhysicsClothMesh's non-grid case isn't covered, see
    // this header's top-of-file scope-gap note.
    struct TickParams
    {
        uint32_t dwType = 0;         // CPhysicsCloth::m_dwType (PCT_* bits, PhysicsManager.h)
        float    windVec[3] = { 0, 0, 0 }; // CPhysicsManager::s_vWind, already *FPS_ANIMATION_FACTOR
        float    fWind = 0.0f;       // CPhysicsCloth::m_fWind, already *FPS_ANIMATION_FACTOR
        float    worldTime = 0.0f;   // WorldTime (global) -- only PCT_ELASTIC_HALLOWEEN's sin() term reads this
        int32_t  numHor = 1;         // CPhysicsCloth::m_iNumHor
        int32_t  seed = 0;           // InitForces' iSeed, already reduced mod vertexCount by the caller
    };

    // Pass B: CSR-gather spring force + CPhysicsVertex::UpdateForce's gravity/wind/elastic-type
    // force terms + semi-implicit-Euler integrate, front -> back, then swaps FrontIndex. Skips
    // pinned vertices (pass-through: back[v] = front[v], vel unchanged) -- matches CPU
    // SetFixedVertices/Move()'s own fixed-vertex bypass.
    void DispatchSpringIntegrate(Instance_t& inst, float deltaTime, const TickParams& tick);

    // Pass C: CSR-gather LOOSEDISTANCE soft constraint, front -> back, then swaps FrontIndex.
    // Direct translation of CPhysicsVertex::AddOneTimeMoveToKeepLength + DoOneTimeMove's averaging
    // -- see this header's top-of-file scope-gap note for the collision-count divergence caveat.
    // One dispatch per call; CPU's PreventFromStretching also calls this only once per tick.
    void DispatchLooseDistance(Instance_t& inst);

    // Pass D: STRICTDISTANCE hard clamp, `iterations` Jacobi-relaxation sub-iterations (each one a
    // full front->back dispatch + swap), standing in for CPU's single sequential (Gauss-Seidel,
    // link-declaration/top-down-row order) sweep -- see this header's top-of-file scope-gap note.
    // numHor gates which vertices can ever be corrected (CPU: `pLink->m_nVertices[1] >= m_iNumHor`
    // -- first grid row never plays the corrected role of a vertical STRICTDISTANCE link).
    void DispatchStrictDistance(Instance_t& inst, int numHor, int iterations);

    // Pass F: sphere collision push-out, front -> back, then swaps FrontIndex. Direct translation
    // of CPhysicsColSphere::ProcessCollision (push a vertex radially out of any sphere it's
    // inside), applied sequentially per overlapping sphere within one thread -- see this header's
    // top-of-file scope-gap note for how this differs from CPU's simultaneous-average semantics.
    // Sphere centers are already fully world-transformed by the caller (CPU-side, reusing the same
    // per-bone TransformPosition every sphere already needs -- cheap, at most a handful of spheres
    // per cloth) -- this pass does no bone math itself, just the push-out. sphereCount is clamped
    // to kMaxCollisionSpheres (8; real usage tops out at 4, grepped every AddCollisionSphere call
    // site). No-op if sphereCount <= 0 -- skips the dispatch entirely for cloths with no collision
    // volumes at all, rather than running an empty-loop pass every tick for nothing.
    static constexpr int kMaxCollisionSpheres = 8;
    void DispatchCollision(Instance_t& inst, const float* sphereCenters3, const float* sphereRadii, int sphereCount);

    // Pass E: scatters the front buffer's positions + the static per-vertex UV (from
    // CreateInstance's uv2) + a constant color into inst.pDrawVertexBuf, in a layout bit-identical
    // to RHI.h VertexLayout::PosUvColor. No-op if CreateInstance wasn't given a uv2 array. Not
    // bound to any draw call -- see this header's top-of-file Phase 3a/3b split note.
    void DispatchScatterDraw(Instance_t& inst, const float color4[4]);

    // Synchronous GPU->CPU readback (CopyResource to a staging buffer + Map(READ)) of the front
    // buffer, for diagnostics/validation only -- never called from the real render loop (Phase 3's
    // planned low-frequency shadow readback is a separate, purpose-built staging copy).
    bool ReadBackVertices(Instance_t& inst, float* outPos3, float* outVel3);

    // Synchronous GPU->CPU readback of Pass E's output buffer, self-test only. outVertexData must
    // hold vertexCount*9 floats (pos3, uv2, rgba4 per vertex, matching DrawVertex/IRVertex order).
    // fromVBO=true reads the post-CopyResource plain vertex-buffer copy instead of the structured
    // UAV source -- self-test-only, to verify the copy bridge itself landed the right bytes.
    bool ReadBackDrawVertices(Instance_t& inst, float* outVertexData, bool fromVBO = false);

    // One-shot GPU-vs-CPU correctness check on a small fixed synthetic 3-vertex chain (2 links,
    // asymmetric stretch so the two links' forces don't cancel), logged to MuError.log. See the
    // .cpp for the hand-derived expected values this compares against. Call once at startup,
    // D3D11 only, behind config.ini [Render] ClothComputeSelfTest=1.
    void RunSelfTestAndLog();

private:
    CClothComputeShader();
    ~CClothComputeShader();

    bool CompileShaders();

    bool  m_bInitialized = false;
    bool  m_bSupported   = false;

    void* m_PinSkinCS          = nullptr;  // ID3D11ComputeShader*
    void* m_SpringIntegrateCS  = nullptr;  // ID3D11ComputeShader*
    void* m_LooseDistanceCS    = nullptr;  // ID3D11ComputeShader*
    void* m_StrictDistanceCS   = nullptr;  // ID3D11ComputeShader*
    void* m_CollisionCS        = nullptr;  // ID3D11ComputeShader* (Pass F)
    void* m_ScatterDrawCS      = nullptr;  // ID3D11ComputeShader* (Pass E)
    void* m_PinCBuffer         = nullptr;  // ID3D11Buffer* (Pass A constants)
    void* m_IntegrateCBuffer   = nullptr;  // ID3D11Buffer* (Pass B constants)
    void* m_ConstraintCBuffer  = nullptr;  // ID3D11Buffer* (Pass C/D constants, shared shape)
    void* m_CollisionCBuffer   = nullptr;  // ID3D11Buffer* (Pass F constants)
    void* m_ScatterCBuffer     = nullptr;  // ID3D11Buffer* (Pass E constants)
};

#endif // CLOTH_COMPUTE_SHADER_H
