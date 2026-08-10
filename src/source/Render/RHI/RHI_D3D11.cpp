// DXP-13 -- RHI_D3D11: the Direct3D 11 backend for Render/RHI/RHI.h. First slice: device +
// swapchain + clear + present only (Init/Shutdown/BeginFrame/EndFrame/SetViewport/Clear/
// OnResize) -- see dxp/DXP-13-d3d11-device-swapchain.md.
//
// DXP-14 increment 1 adds the buffer/uniform-block/vertex-layout/draw subset needed for
// PassthroughShader's HLSL port to compile and draw a smoke triangle (see
// dxp/DXP-14-hlsl-shader-port.md): vertex/index buffers, uniform blocks (cbuffers),
// PosUvColor input layout + bind, and Draw. Textures, pipeline/blend state, indexed draw,
// and readback are still fail-loud stubs -- they aren't exercised by any DXP-14 caller and
// land with DXP-15/16 (real UI/world rendering) instead of being guessed at here.
//
// DXP-15 increment 1 adds Textures (CreateTexture/UpdateTexture/DestroyTexture/BindTexture) --
// see dxp/DXP-15-ir-ui-d3d11.md.
//
// DXP-15 increment 2 adds pipeline/blend state (SetBlendMode's 8 combos + SetDepthTestEnabled)
// -- see dxp/DXP-15-ir-ui-d3d11.md. Indexed draw, readback, and fog remain fail-loud stubs.
//
// D3D11 only exists on Windows -- the #else branch below keeps every RHI_D3D11_Impl::
// symbol linkable on non-Windows builds (RHI.cpp's dispatch shim references them
// unconditionally, since Backend=D3D11 is a runtime config value parsed on every platform),
// but Init() fails loud immediately there: Backend=D3D11 is not a supported combination
// off Windows.

#include "stdafx.h"
#include "Render/RHI/RHI.h"
#include "Render/Core/RenderConfig.h"
#include "Core/Utilities/Log/ErrorReport.h"
#include <cassert>

// Must sit OUTSIDE namespace RHI_D3D11_Impl: #include'ing standard headers inside a namespace
// block nests their own `namespace std { ... }` into `RHI_D3D11_Impl::std`, a completely
// different namespace than the real global std:: -- breaks every std:: template that depends
// on something declared by an earlier, correctly-global include (surfaces as a wall of
// unrelated-looking errors inside <xhash>, not anything pointing at this file).
#ifdef RHI_D3D11_AVAILABLE
#include <d3d11.h>
#include <dxgi.h>
#include <vector>
#include <unordered_map>
#endif

namespace RHI_D3D11_Impl {

namespace {
    void NotImplemented(const wchar_t* fn)
    {
        g_ErrorReport.Write(L"[RHI_D3D11] %s: not yet implemented\r\n", fn);
        assert(false && "RHI_D3D11: function not yet implemented");
    }
}

#ifdef RHI_D3D11_AVAILABLE

namespace {
    ID3D11Device*           g_Device      = nullptr;
    ID3D11DeviceContext*    g_Context     = nullptr;
    IDXGISwapChain*         g_SwapChain   = nullptr;
    ID3D11RenderTargetView* g_RTV         = nullptr;
    ID3D11Texture2D*        g_DepthBuffer = nullptr;
    ID3D11DepthStencilView* g_DSV         = nullptr;
    ID3D11InfoQueue*        g_InfoQueue   = nullptr; // _DEBUG only

    // Drains the debug-layer message queue into g_ErrorReport, once per EndFrame() (and once
    // more at Shutdown()) -- the D3D twin of the KHR_debug callback Winmain.cpp registers for
    // the GL backend (Winmain.cpp:1840-1873). ID3D11InfoQueue has no push-callback mechanism
    // pre-D3D11.3's ID3D11InfoQueue1, so polling is the standard, simplest approach.
    void DrainInfoQueue()
    {
        if (!g_InfoQueue) return;
        const UINT64 count = g_InfoQueue->GetNumStoredMessages();
        for (UINT64 i = 0; i < count; ++i)
        {
            SIZE_T len = 0;
            g_InfoQueue->GetMessage(i, nullptr, &len);
            if (len == 0) continue;
            std::vector<char> buf(len);
            D3D11_MESSAGE* msg = reinterpret_cast<D3D11_MESSAGE*>(buf.data());
            g_InfoQueue->GetMessage(i, msg, &len);
            g_ErrorReport.Write(L"[RHI_D3D11] %hs\r\n", msg->pDescription);
        }
        g_InfoQueue->ClearStoredMessages();
    }

    bool CreateRenderTargetAndDepth(int width, int height)
    {
        ID3D11Texture2D* backBuffer = nullptr;
        if (FAILED(g_SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) return false;
        const HRESULT rtvHr = g_Device->CreateRenderTargetView(backBuffer, nullptr, &g_RTV);
        backBuffer->Release();
        if (FAILED(rtvHr)) return false;

        D3D11_TEXTURE2D_DESC depthDesc = {};
        depthDesc.Width          = static_cast<UINT>(width);
        depthDesc.Height         = static_cast<UINT>(height);
        depthDesc.MipLevels      = 1;
        depthDesc.ArraySize      = 1;
        // Deeper than GL's current 16-bit depth buffer (SDL_GL_DEPTH_SIZE 16,
        // Winmain.cpp:1808) -- DXP-17 audits depth behavior across both backends later.
        depthDesc.Format         = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthDesc.SampleDesc.Count = 1;
        depthDesc.Usage          = D3D11_USAGE_DEFAULT;
        depthDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
        if (FAILED(g_Device->CreateTexture2D(&depthDesc, nullptr, &g_DepthBuffer))) return false;
        if (FAILED(g_Device->CreateDepthStencilView(g_DepthBuffer, nullptr, &g_DSV))) return false;

        g_Context->OMSetRenderTargets(1, &g_RTV, g_DSV);

        D3D11_VIEWPORT vp = {};
        vp.Width    = static_cast<float>(width);
        vp.Height   = static_cast<float>(height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        g_Context->RSSetViewports(1, &vp);
        g_ErrorReport.Write(L"[RHI_D3D11] Viewport set to %dx%d (TopLeft 0,0)\r\n", width, height);
        return true;
    }

    void ReleaseRenderTargetAndDepth()
    {
        if (g_RTV)         { g_RTV->Release();         g_RTV = nullptr; }
        if (g_DSV)         { g_DSV->Release();         g_DSV = nullptr; }
        if (g_DepthBuffer) { g_DepthBuffer->Release();  g_DepthBuffer = nullptr; }
    }

    // ---- Buffers + uniform blocks (DXP-14 increment 1) ----
    // One record per RHI::BufferHandle, covering vertex/index/constant buffers alike --
    // only bindFlag and (for uniform blocks) uniformSlot differ.
    struct BufferRec {
        ID3D11Buffer* buffer     = nullptr;
        UINT          capacity   = 0;  // bytes
        UINT          writeOffset = 0; // AppendBuffer ring-buffer bookkeeping only
        UINT          bindFlag   = 0;  // D3D11_BIND_VERTEX_BUFFER / INDEX_BUFFER / CONSTANT_BUFFER
        int           uniformSlot = -1; // >=0 for uniform blocks; bound to both VS and PS stages
    };
    std::unordered_map<uint32_t, BufferRec> g_Buffers;
    uint32_t g_NextBufferId = 1;

    ID3D11Buffer* CreateD3DBuffer(UINT byteWidth, UINT bindFlag, const void* initialData, bool dynamic)
    {
        if (byteWidth == 0) return nullptr;
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth      = byteWidth;
        desc.BindFlags      = bindFlag;
        desc.Usage          = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
        desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
        D3D11_SUBRESOURCE_DATA sub = {};
        sub.pSysMem = initialData;
        ID3D11Buffer* buf = nullptr;
        const HRESULT hr = g_Device->CreateBuffer(&desc, initialData ? &sub : nullptr, &buf);
        if (FAILED(hr))
        {
            g_ErrorReport.Write(L"[RHI_D3D11] CreateBuffer(bind=0x%X, bytes=%u) failed (hr=0x%08lX)\r\n",
                bindFlag, byteWidth, static_cast<unsigned long>(hr));
            return nullptr;
        }
        return buf;
    }

    // uniformSlot >= 0 marks this a uniform block: bound to both VS and PS constant-buffer
    // slots immediately (mirrors GL's glBindBufferBase -- one binding point shared by every
    // stage that declares the block, whereas D3D11's VSSetConstantBuffers/PSSetConstantBuffers
    // are independent per-stage slot spaces; binding both keeps the same-slot-number contract
    // RHI::CreateUniformBlock's header comment describes regardless of which stage(s) a given
    // shader actually references the block from).
    RHI::BufferHandle CreateBufferGeneric(UINT bindFlag, const void* initialData, size_t sizeBytes, bool dynamic, int uniformSlot)
    {
        ID3D11Buffer* buf = CreateD3DBuffer(static_cast<UINT>(sizeBytes), bindFlag, initialData, dynamic);
        if (!buf) return {};
        BufferRec rec;
        rec.buffer      = buf;
        rec.capacity    = static_cast<UINT>(sizeBytes);
        rec.bindFlag    = bindFlag;
        rec.uniformSlot = uniformSlot;
        const uint32_t id = g_NextBufferId++;
        g_Buffers[id] = rec;
        if (uniformSlot >= 0 && g_Context)
        {
            g_Context->VSSetConstantBuffers(static_cast<UINT>(uniformSlot), 1, &buf);
            g_Context->PSSetConstantBuffers(static_cast<UINT>(uniformSlot), 1, &buf);
        }
        return RHI::BufferHandle{ id };
    }

    // ---- Textures (DXP-15 increment 1) ----
    // One record per RHI::TextureHandle, same id-map shape as g_Buffers above.
    struct TextureRec {
        ID3D11Texture2D*          texture = nullptr;
        ID3D11ShaderResourceView* srv     = nullptr;
        RHI::TexFilter            filter  = RHI::TexFilter::Nearest;
        RHI::TexWrap              wrap    = RHI::TexWrap::Clamp;
    };
    std::unordered_map<uint32_t, TextureRec> g_Textures;
    uint32_t g_NextTextureId = 1;

    // Per-PS-slot dirty-check for BindTexture, same shape as g_CurrentSlot0BufferId further down
    // for IASetVertexBuffers. Root cause of the D3D11-Debug-only login/tour-scene slowdown
    // (measured: Terrain pass ~64ms/frame vs GL's ~1ms equivalent): ZzzLodTerrain.cpp's per-tile
    // loop calls TerrainShader::SetBaseTexture/SetOverlayTexture unconditionally for every one of
    // the thousands of 6-index tile draws in a frustum pass -- the GL side already dirty-checks
    // this via BindTexture2D's CachTexture (the TASK-24 "per-tile GL calls are the enemy" lesson),
    // but RHI::BindTexture never carried the same discipline, so every tile paid a full
    // PSSetShaderResources+PSSetSamplers even when back-to-back tiles share a texture (the
    // overwhelmingly common case). Cheap in Release (the driver's own state cache absorbs
    // redundant sets), but the D3D11 debug layer fully validates every call regardless of whether
    // it's a no-op, so the redundant-call volume alone accounts for the gap.
    constexpr int kMaxTextureSlots = 8;
    uint32_t g_CurrentTextureId[kMaxTextureSlots] = {};
    bool     g_CurrentTextureBound[kMaxTextureSlots] = {}; // distinguishes "never bound" from "bound to id 0"

    // DXP-16 increment 3: per-(filter,wrap) shared sampler cache, lazily created, replacing the
    // one-fixed-sampler-per-shader-class design (TerrainShader/BMDMeshShader/PassthroughShader's
    // old m_D3DSampler members). Root cause of a reported D3D11-only "rice terrace" banding
    // artifact on ordinary (non-water) grass terrain: TextureDesc::wrap was computed correctly by
    // every caller (GlobalBitmap.cpp/MapManager.cpp mirror each texture's real GL wrap mode) but
    // silently discarded by CreateTexture (see its own comment, pre-this-fix) -- so ALL textures
    // bound through a shader with a WRAP sampler (TerrainShader, fixed for the ORIGINAL
    // washed-out-terrain bug) wrapped uniformly, including non-power-of-two source textures
    // padded up to their storage size by GlobalBitmap.cpp's NextPowerOfTwo() -- WRAP addressing
    // repeats right into that unused padding, visible as periodic bands. Binding the correct
    // per-texture sampler at BindTexture() time (this texture's own recorded wrap/filter, exactly
    // matching what GL already does per-texture) fixes this at the root instead of picking one
    // wrap mode per shader class. Also resolves the previously-flagged "Hellas water2.jpg wraps
    // when it should clamp" gap the same way, for the same reason.
    ID3D11SamplerState* g_Samplers[2][2] = {}; // [TexFilter][TexWrap]

    ID3D11SamplerState* GetOrCreateSampler(RHI::TexFilter filter, RHI::TexWrap wrap)
    {
        const int fi = static_cast<int>(filter);
        const int wi = static_cast<int>(wrap);
        if (g_Samplers[fi][wi]) return g_Samplers[fi][wi];
        if (!g_Device) return nullptr;

        D3D11_SAMPLER_DESC desc = {};
        desc.Filter   = (filter == RHI::TexFilter::Linear) ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT;
        desc.AddressU = desc.AddressV = desc.AddressW =
            (wrap == RHI::TexWrap::Repeat) ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        desc.MaxLOD   = D3D11_FLOAT32_MAX;

        ID3D11SamplerState* sampler = nullptr;
        g_Device->CreateSamplerState(&desc, &sampler);
        g_Samplers[fi][wi] = sampler;
        return sampler;
    }

    void ReleaseAllSamplers()
    {
        for (auto& row : g_Samplers)
            for (auto& s : row)
                if (s) { s->Release(); s = nullptr; }
    }

    void ReleaseAllTextures()
    {
        for (auto& kv : g_Textures)
        {
            if (kv.second.srv)     kv.second.srv->Release();
            if (kv.second.texture) kv.second.texture->Release();
        }
        g_Textures.clear();
        for (int i = 0; i < kMaxTextureSlots; ++i)
        {
            g_CurrentTextureId[i]    = 0;
            g_CurrentTextureBound[i] = false;
        }
    }

    // ---- Pipeline / blend-mode state (DXP-15 increment 2) ----
    // 8 precomputed ID3D11BlendState (one per RHI::BlendMode), 2 ID3D11RasterizerState (cull
    // on/off), 4 ID3D11DepthStencilState (depthTestEnabled x depthWriteEnabled) -- built once
    // here, swapped by pointer per SetBlendMode/SetDepthTestEnabled call, zero per-draw cost
    // (RHI.h's design comment). Depth test (SetDepthTestEnabled) and depth write (part of the
    // blend combo below, mirroring EnableDepthMask's GL side effect) are independent axes in
    // this codebase's design -- GL toggles them via two entirely separate globals
    // (DepthTestEnable vs DepthMaskEnable) -- so the DepthStencilState needs both dimensions,
    // not just one.
    constexpr int kBlendModeCount = 8;
    struct BlendCombo { bool blendEnable; D3D11_BLEND src; D3D11_BLEND dst; bool cullOn; bool depthWriteOn; };
    // Translated literally from RHI.h's BlendMode table / ZzzOpenglUtil.cpp's wrapper bodies --
    // straight alpha (no premultiply), ADD blend op, RGB and alpha channels share one factor
    // pair (every wrapper calls plain glBlendFunc, never glBlendFuncSeparate).
    const BlendCombo kBlendCombos[kBlendModeCount] = {
        /* Opaque    */ { false, D3D11_BLEND_ONE,           D3D11_BLEND_ZERO,           true,  true  },
        /* LightMap  */ { true,  D3D11_BLEND_ZERO,          D3D11_BLEND_SRC_COLOR,      true,  true  },
        /* AlphaTest */ { true,  D3D11_BLEND_SRC_ALPHA,     D3D11_BLEND_INV_SRC_ALPHA,  false, true  },
        /* Additive  */ { true,  D3D11_BLEND_ONE,           D3D11_BLEND_ONE,            false, false },
        /* Minus     */ { true,  D3D11_BLEND_ZERO,          D3D11_BLEND_INV_SRC_COLOR,  false, false },
        /* Blend2    */ { true,  D3D11_BLEND_INV_SRC_COLOR, D3D11_BLEND_ONE,            false, false },
        /* Blend3    */ { true,  D3D11_BLEND_SRC_ALPHA,     D3D11_BLEND_INV_SRC_ALPHA,  false, false },
        /* Blend4    */ { true,  D3D11_BLEND_ONE,           D3D11_BLEND_INV_SRC_COLOR,  false, false },
    };

    ID3D11BlendState*        g_BlendState[kBlendModeCount] = {};
    ID3D11RasterizerState*   g_RasterizerCullOn  = nullptr;
    ID3D11RasterizerState*   g_RasterizerCullOff = nullptr;
    ID3D11DepthStencilState* g_DepthStencilState[2][2] = {}; // [depthTestEnabled][depthWriteEnabled]
    bool g_DepthTestEnabled  = true; // mirrors ZzzOpenglUtil.cpp's per-frame DepthTestEnable default
    bool g_DepthWriteEnabled = true; // mirrors its DepthMaskEnable default

    // DXP-16 increment 4: depth-biased rasterizer state pair for RHI::SetPolygonOffset
    // (PlanarShadowShader's one caller -- shadows drawn coplanar with the ground need a slight
    // depth pull-toward-camera or they z-fight against it). D3D11 bakes depth bias into the
    // rasterizer state object at creation time (no per-draw glPolygonOffset equivalent), so these
    // are built lazily on first real SetPolygonOffset(true, ...) call using that call's
    // factor/units -- there's only one caller in the tree with one fixed value, so this doesn't
    // need a full cache keyed by value. D3D11_RASTERIZER_DESC::SlopeScaledDepthBias/DepthBias map
    // to GL's glPolygonOffset(factor, units) 1:1 (both APIs define the applied bias as
    // factor*maxSlope + units*r, where r is the implementation's smallest resolvable depth step).
    ID3D11RasterizerState*   g_RasterizerCullOnBias  = nullptr;
    ID3D11RasterizerState*   g_RasterizerCullOffBias = nullptr;
    bool  g_PolyOffsetEnabled = false;
    bool  g_PolyOffsetStatesBuilt = false;
    bool  g_CurrentCullOn = true; // mirrors whichever RSSetState the last SetBlendMode/SetCullEnabled picked

    void EnsurePolyOffsetRasterizerStates(float factor, float units)
    {
        if (g_PolyOffsetStatesBuilt || !g_Device) return;
        D3D11_RASTERIZER_DESC rd = {};
        rd.FillMode = D3D11_FILL_SOLID;
        rd.DepthClipEnable = TRUE;
        rd.SlopeScaledDepthBias = factor;
        rd.DepthBias = static_cast<INT>(units);
        rd.CullMode = D3D11_CULL_NONE;
        if (FAILED(g_Device->CreateRasterizerState(&rd, &g_RasterizerCullOffBias)))
        {
            g_ErrorReport.Write(L"[RHI_D3D11] CreateRasterizerState(bias, CullMode=NONE) failed\r\n");
            return;
        }
        rd.CullMode = D3D11_CULL_BACK;
        if (FAILED(g_Device->CreateRasterizerState(&rd, &g_RasterizerCullOnBias)))
        {
            g_ErrorReport.Write(L"[RHI_D3D11] CreateRasterizerState(bias, CullMode=BACK) failed\r\n");
            return;
        }
        g_PolyOffsetStatesBuilt = true;
    }

    void ApplyRasterizerState()
    {
        if (!g_Context) return;
        ID3D11RasterizerState* rs;
        if (g_PolyOffsetEnabled && g_RasterizerCullOnBias && g_RasterizerCullOffBias)
            rs = g_CurrentCullOn ? g_RasterizerCullOnBias : g_RasterizerCullOffBias;
        else
            rs = g_CurrentCullOn ? g_RasterizerCullOn : g_RasterizerCullOff;
        g_Context->RSSetState(rs);
    }

    // D3D11's alpha-blend-factor slots (SrcBlendAlpha/DestBlendAlpha) reject the *_COLOR family
    // outright (debug-layer ERROR, "invalid alpha blend") -- GL has no such split, a single
    // glBlendFunc factor applies component-wise to all 4 channels, so GL_SRC_COLOR restricted to
    // just the alpha channel is mathematically identical to GL_SRC_ALPHA (both mean "multiply by
    // source alpha"). This maps each RGB-space D3D11_BLEND factor to its alpha-space equivalent
    // for those two fields; ZERO/ONE/SRC_ALPHA/INV_SRC_ALPHA are already valid for alpha as-is.
    D3D11_BLEND ToAlphaBlendFactor(D3D11_BLEND colorFactor)
    {
        switch (colorFactor)
        {
        case D3D11_BLEND_SRC_COLOR:      return D3D11_BLEND_SRC_ALPHA;
        case D3D11_BLEND_INV_SRC_COLOR:  return D3D11_BLEND_INV_SRC_ALPHA;
        case D3D11_BLEND_DEST_COLOR:     return D3D11_BLEND_DEST_ALPHA;
        case D3D11_BLEND_INV_DEST_COLOR: return D3D11_BLEND_INV_DEST_ALPHA;
        default:                         return colorFactor;
        }
    }

    void ApplyDepthStencilState()
    {
        if (!g_Context) return;
        ID3D11DepthStencilState* dss = g_DepthStencilState[g_DepthTestEnabled ? 1 : 0][g_DepthWriteEnabled ? 1 : 0];
        if (dss) g_Context->OMSetDepthStencilState(dss, 0);
    }

    bool CreatePipelineStateObjects()
    {
        for (int i = 0; i < kBlendModeCount; ++i)
        {
            const BlendCombo& c = kBlendCombos[i];
            D3D11_BLEND_DESC desc = {};
            desc.RenderTarget[0].BlendEnable           = c.blendEnable;
            desc.RenderTarget[0].SrcBlend               = c.src;
            desc.RenderTarget[0].DestBlend              = c.dst;
            desc.RenderTarget[0].BlendOp                = D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0].SrcBlendAlpha          = ToAlphaBlendFactor(c.src);
            desc.RenderTarget[0].DestBlendAlpha         = ToAlphaBlendFactor(c.dst);
            desc.RenderTarget[0].BlendOpAlpha           = D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0].RenderTargetWriteMask  = D3D11_COLOR_WRITE_ENABLE_ALL;
            if (FAILED(g_Device->CreateBlendState(&desc, &g_BlendState[i])))
            {
                g_ErrorReport.Write(L"[RHI_D3D11] CreateBlendState(mode=%d) failed\r\n", i);
                return false;
            }
        }

        D3D11_RASTERIZER_DESC rastDesc = {};
        rastDesc.FillMode = D3D11_FILL_SOLID;
        // GL_CULL_FACE's front face is never overridden anywhere in the tree (no glFrontFace
        // call exists) -- GL's default, CCW, is what "front" means throughout this codebase.
        rastDesc.FrontCounterClockwise = TRUE;
        rastDesc.DepthClipEnable = TRUE;
        rastDesc.CullMode = D3D11_CULL_NONE;
        if (FAILED(g_Device->CreateRasterizerState(&rastDesc, &g_RasterizerCullOff)))
        {
            g_ErrorReport.Write(L"[RHI_D3D11] CreateRasterizerState(CullMode=NONE) failed\r\n");
            return false;
        }
        rastDesc.CullMode = D3D11_CULL_BACK; // GL's default glCullFace(GL_BACK), never overridden in the tree
        if (FAILED(g_Device->CreateRasterizerState(&rastDesc, &g_RasterizerCullOn)))
        {
            g_ErrorReport.Write(L"[RHI_D3D11] CreateRasterizerState(CullMode=BACK) failed\r\n");
            return false;
        }

        for (int test = 0; test < 2; ++test)
        {
            for (int write = 0; write < 2; ++write)
            {
                D3D11_DEPTH_STENCIL_DESC dsDesc = {};
                dsDesc.DepthEnable    = test != 0;
                dsDesc.DepthWriteMask = write ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
                dsDesc.DepthFunc      = D3D11_COMPARISON_LESS_EQUAL; // GL_LEQUAL -- the only glDepthFunc value in the tree (RHI.h)
                dsDesc.StencilEnable  = FALSE;
                // FrontFace/BackFace are still validated even with StencilEnable=FALSE -- a
                // zero-initialized D3D11_STENCIL_OP/D3D11_COMPARISON_FUNC (both enums start at 1)
                // is an invalid value, which the debug layer flags as an ERROR-severity message.
                // Init() sets SetBreakOnSeverity(ERROR, TRUE) on the debug info queue, so that
                // ERROR triggered a synchronous DebugBreak() inside this very call with no
                // debugger attached -- the actual crash, not a graceful CreateDepthStencilState
                // failure (which would have hit the FAILED() branch below and logged instead).
                const D3D11_DEPTH_STENCILOP_DESC kNoStencilOp = {
                    D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS
                };
                dsDesc.FrontFace = kNoStencilOp;
                dsDesc.BackFace  = kNoStencilOp;
                if (FAILED(g_Device->CreateDepthStencilState(&dsDesc, &g_DepthStencilState[test][write])))
                {
                    g_ErrorReport.Write(L"[RHI_D3D11] CreateDepthStencilState(test=%d, write=%d) failed\r\n", test, write);
                    return false;
                }
            }
        }
        return true;
    }

    void ReleasePipelineStateObjects()
    {
        for (int i = 0; i < kBlendModeCount; ++i)
        {
            if (g_BlendState[i]) { g_BlendState[i]->Release(); g_BlendState[i] = nullptr; }
        }
        if (g_RasterizerCullOn)  { g_RasterizerCullOn->Release();  g_RasterizerCullOn = nullptr; }
        if (g_RasterizerCullOff) { g_RasterizerCullOff->Release(); g_RasterizerCullOff = nullptr; }
        if (g_RasterizerCullOnBias)  { g_RasterizerCullOnBias->Release();  g_RasterizerCullOnBias = nullptr; }
        if (g_RasterizerCullOffBias) { g_RasterizerCullOffBias->Release(); g_RasterizerCullOffBias = nullptr; }
        g_PolyOffsetStatesBuilt = false;
        g_PolyOffsetEnabled = false;
        for (int test = 0; test < 2; ++test)
        {
            for (int write = 0; write < 2; ++write)
            {
                if (g_DepthStencilState[test][write]) { g_DepthStencilState[test][write]->Release(); g_DepthStencilState[test][write] = nullptr; }
            }
        }
    }

    // ---- Vertex layouts (all 4 implemented as of DXP-14 increment 4) ----
    constexpr int kVertexLayoutCount = 4; // PosUvColor, BMDMesh, Terrain, PosOnly
    ID3D11InputLayout*        g_InputLayout[kVertexLayoutCount]       = {};
    std::vector<unsigned char> g_LayoutBytecode[kVertexLayoutCount];

    // Dirty-check for IA vertex-buffer slot 0. Deliberately ONE shared variable, not one per
    // VertexLayout: there is only one physical slot-0 binding, used by every layout in turn, so
    // a per-layout cache lies the instant two layouts alternate draws within the same frame --
    // each layout's own cache still says "my buffer is bound" even after the OTHER layout's
    // bind call silently changed slot 0 out from under it. (Found via the D3D11 debug layer:
    // "Input vertex slot 0 has stride N which is less than the minimum stride logically
    // expected from the current Input Layout" -- the smoking gun for exactly this bug, DXP-14
    // increment 2.) The input layout itself has no equivalent bug: IASetInputLayout is called
    // unconditionally every BindVertexBuffer, never cached.
    uint32_t g_CurrentSlot0BufferId = 0;
    // DXP-16: same dirty-check shape as g_CurrentSlot0BufferId above, for IASetIndexBuffer.
    uint32_t g_CurrentIndexBufferId = 0;

    // Dirty-check for IASetPrimitiveTopology, same shape/reasoning as g_CurrentSlot0BufferId above
    // -- terrain's per-tile draw loop calls DrawIndexed with the same TriangleList topology
    // thousands of times a frame; skipping the redundant re-set halves the per-tile
    // D3D11-debug-layer-validated call count together with the BindTexture dirty-check above.
    D3D11_PRIMITIVE_TOPOLOGY g_CurrentTopology = static_cast<D3D11_PRIMITIVE_TOPOLOGY>(-1); // D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED


    // pos3+uv2+rgba4, 36B stride -- mirrors ImmediateRenderer.cpp's IRVertex / RHI_GL's
    // ConfigurePosUvColorVAO layout exactly (same source data, same offsets).
    const D3D11_INPUT_ELEMENT_DESC kPosUvColorLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    constexpr UINT kPosUvColorStride = sizeof(float) * 9;

    // pos3+light3+alpha1, 28B stride -- mirrors TerrainShader's `a_Pos`/`a_Light`/`a_Alpha`
    // (locations 0/1/2). DXP-16 increment 1: resolved the interleave-vs-multi-stream gap this
    // comment used to flag by interleaving -- ZzzLodTerrain.cpp's D3D11 path now maintains one
    // CPU-side interleaved scratch array (g_TerrainVertexD3D11, pos+alpha filled once at map
    // load, light refreshed per frame) and re-uploads it whole via RHI::UpdateBuffer's discard
    // semantics each frame (the D3D11-native equivalent of GL's double-buffered light-only
    // STREAM_DRAW VBO -- WRITE_DISCARD already gives the driver a fresh backing store, no
    // hand-rolled double buffering needed). GL's real upload path is untouched, still 3
    // separate VBOs -- this interleaved layout is D3D11-only.
    const D3D11_INPUT_ELEMENT_DESC kTerrainLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32_FLOAT,       0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    constexpr UINT kTerrainStride = sizeof(float) * 7;

    // pos3+uv2+color4+normal3+boneIndex1(int), 52B stride -- mirrors BMDMeshShader's a_Pos/
    // a_UV/a_Color/a_Normal/a_BoneIndex (locations 0-4). DXP-14 increment 3 note: same gap as
    // Terrain's (increment 2) -- the real GPU-skinned static geometry (ZzzBMD.cpp's
    // m_VAO_StaticGPU) uploads pos+uv+normal+boneIndex as ONE interleaved 36-byte VBO plus a
    // SEPARATE per-frame color VBO (m_VBO_Color), not one 52-byte interleaved buffer. This
    // single-buffer layout matches BMDMeshShader's vertex *shader* input exactly (what
    // ID3D11InputLayout needs) but not yet the real 2-VBO upload scheme -- flagged for
    // whoever wires real BMD rendering through D3D11 (DXP-15/16), not solved here.
    const D3D11_INPUT_ELEMENT_DESC kBMDMeshLayout[] = {
        { "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",        0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R32_SINT,           0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    constexpr UINT kBMDMeshStride = sizeof(float) * 13; // 12 float fields + 1 int field, same 4B width

    // pos3 only, 12B stride -- PlanarShadowShader's CPU-path draw (DXP-14 increment 4). Matches
    // RHI.h's own "PlanarShadowShader -- position-only" design intent for this layout exactly.
    const D3D11_INPUT_ELEMENT_DESC kPosOnlyLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    constexpr UINT kPosOnlyStride = sizeof(float) * 3;

    UINT StrideForLayout(int idx)
    {
        switch (static_cast<RHI::VertexLayout>(idx))
        {
        case RHI::VertexLayout::PosUvColor: return kPosUvColorStride;
        case RHI::VertexLayout::Terrain:    return kTerrainStride;
        case RHI::VertexLayout::BMDMesh:    return kBMDMeshStride;
        case RHI::VertexLayout::PosOnly:    return kPosOnlyStride;
        default: return 0;
        }
    }

    bool EnsureInputLayout(int idx)
    {
        if (g_InputLayout[idx]) return true;
        if (g_LayoutBytecode[idx].empty()) return false; // shader hasn't registered its bytecode yet

        const D3D11_INPUT_ELEMENT_DESC* elements = nullptr;
        UINT elementCount = 0;
        switch (static_cast<RHI::VertexLayout>(idx))
        {
        case RHI::VertexLayout::PosUvColor: elements = kPosUvColorLayout; elementCount = ARRAYSIZE(kPosUvColorLayout); break;
        case RHI::VertexLayout::Terrain:    elements = kTerrainLayout;    elementCount = ARRAYSIZE(kTerrainLayout);    break;
        case RHI::VertexLayout::BMDMesh:    elements = kBMDMeshLayout;    elementCount = ARRAYSIZE(kBMDMeshLayout);    break;
        case RHI::VertexLayout::PosOnly:    elements = kPosOnlyLayout;    elementCount = ARRAYSIZE(kPosOnlyLayout);    break;
        default: return false;
        }

        const HRESULT hr = g_Device->CreateInputLayout(elements, elementCount,
            g_LayoutBytecode[idx].data(), g_LayoutBytecode[idx].size(), &g_InputLayout[idx]);
        if (FAILED(hr))
        {
            g_ErrorReport.Write(L"[RHI_D3D11] CreateInputLayout(layout=%d) failed (hr=0x%08lX)\r\n", idx, static_cast<unsigned long>(hr));
            return false;
        }
        return true;
    }

    void ReleaseAllBuffersAndLayouts()
    {
        for (auto& kv : g_Buffers)
        {
            if (kv.second.buffer) kv.second.buffer->Release();
        }
        g_Buffers.clear();
        for (int i = 0; i < kVertexLayoutCount; ++i)
        {
            if (g_InputLayout[i]) { g_InputLayout[i]->Release(); g_InputLayout[i] = nullptr; }
            g_LayoutBytecode[i].clear();
        }
        g_CurrentSlot0BufferId = 0;
        g_CurrentIndexBufferId = 0;
        g_CurrentTopology = static_cast<D3D11_PRIMITIVE_TOPOLOGY>(-1);
    }
}

bool Init(void* nativeWindowHandle, int width, int height)
{
    const HWND hwnd = static_cast<HWND>(nativeWindowHandle);

    DXGI_SWAP_CHAIN_DESC scd = {};
    // DXP-16 fix: was BufferCount=1 + DXGI_SWAP_EFFECT_DISCARD (the legacy "BitBlt" swap model).
    // DXP-16: tried flip-model (FLIP_DISCARD + BufferCount=2) here to chase a reported periodic
    // black-frame flicker on the login screen, on the theory that legacy BitBlt-model DISCARD
    // swapchains are known to flicker under DWM composition in windowed mode. REVERTED -- it made
    // things strictly worse (permanently black window after the initial loading frames, not just
    // an intermittent blink), so whatever the real cause is, it isn't what flip-model fixes, and
    // flip-model itself broke something else about this app's render loop (candidate: a redraw
    // that isn't repeated every single frame, which BitBlt/DISCARD's single-buffer-content
    // tolerance papered over but flip-model's two-buffers-both-need-content behavior does not).
    // Back to the original, still-blinking-but-at-least-visible behavior pending further
    // investigation.
    scd.BufferCount                   = 1;
    scd.BufferDesc.Width              = static_cast<UINT>(width);
    scd.BufferDesc.Height             = static_cast<UINT>(height);
    scd.BufferDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM; // matches RHI's "RGBA8 always" texture contract
    scd.BufferDesc.RefreshRate.Numerator   = 0;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage                   = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow                  = hwnd;
    scd.SampleDesc.Count               = 1;
    scd.SampleDesc.Quality              = 0;
    scd.Windowed                      = TRUE;
    scd.SwapEffect                    = DXGI_SWAP_EFFECT_DISCARD;

    // Opt-in only (config.ini [Render] D3D11DebugLayer=1, RenderConfig.h) -- see g_D3D11DebugLayerEnabled's
    // comment for why this isn't unconditional under _DEBUG anymore. g_InfoQueue's QueryInterface
    // below simply fails (SUCCEEDED check) and stays null when this is off, so DrainInfoQueue()
    // no-ops without needing its own gate.
    UINT deviceFlags = 0;
    if (g_D3D11DebugLayerEnabled)
        deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;

    const D3D_FEATURE_LEVEL requestedLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    D3D_FEATURE_LEVEL obtainedLevel{};

    const HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags,
        requestedLevels, 1, D3D11_SDK_VERSION,
        &scd, &g_SwapChain, &g_Device, &obtainedLevel, &g_Context);

    if (FAILED(hr))
    {
        g_ErrorReport.Write(L"[RHI_D3D11] D3D11CreateDeviceAndSwapChain failed (hr=0x%08lX)\r\n", static_cast<unsigned long>(hr));
        return false;
    }

#ifdef _DEBUG
    if (SUCCEEDED(g_Device->QueryInterface(IID_PPV_ARGS(&g_InfoQueue))))
    {
        g_InfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        g_InfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
    }
#endif

    if (!CreateRenderTargetAndDepth(width, height))
    {
        g_ErrorReport.Write(L"[RHI_D3D11] Render target / depth-stencil creation failed\r\n");
        return false;
    }

    // DXP-15 increment 2: blend/cull/depth pipeline state objects, superseding the interim
    // single always-CullMode=NONE rasterizer state DXP-14 put here (see git history) -- that
    // was only ever a default-state parity fix for GL's real "cull off by default" behavior,
    // documented there as temporary pending SetBlendMode's real implementation. Apply the
    // Opaque combo (cull on, depth write on, blend off) + depth test on as the D3D11 startup
    // default, mirroring ZzzOpenglUtil.cpp's per-frame GL reset
    // (DepthTestEnable=CullFaceEnable=DepthMaskEnable=true).
    if (!CreatePipelineStateObjects())
    {
        g_ErrorReport.Write(L"[RHI_D3D11] Pipeline state object creation failed\r\n");
        return false;
    }
    g_Context->OMSetBlendState(g_BlendState[static_cast<int>(RHI::BlendMode::Opaque)], nullptr, 0xFFFFFFFF);
    g_Context->RSSetState(g_RasterizerCullOn);
    g_DepthTestEnabled  = true;
    g_DepthWriteEnabled = true;
    ApplyDepthStencilState();

    return true;
}

void Shutdown()
{
    ReleaseAllTextures();
    ReleaseAllSamplers();
    ReleaseAllBuffersAndLayouts();
    ReleasePipelineStateObjects();
    ReleaseRenderTargetAndDepth();
    if (g_InfoQueue) { DrainInfoQueue(); g_InfoQueue->Release(); g_InfoQueue = nullptr; }
    if (g_SwapChain) { g_SwapChain->Release(); g_SwapChain = nullptr; }
    if (g_Context)   { g_Context->Release();   g_Context = nullptr; }
    if (g_Device)    { g_Device->Release();    g_Device = nullptr; }
}

void BeginFrame()
{
}

void EndFrame()
{
    DrainInfoQueue();
    // g_VSyncEnabled (RenderConfig.h) is the cross-backend mirror EnableVSync/DisableVSync
    // (ZzzOpenglUtil.cpp) write to -- D3D11 has no GL context/SDL swap interval to toggle, so
    // this is the only lever for it. SyncInterval=0 with a non-flip-model swap chain (this one
    // is BitBlt-model, see Init()) still tears/uncaps correctly; no DXGI_PRESENT_ALLOW_TEARING
    // flag needed for that case.
    g_SwapChain->Present(g_VSyncEnabled ? 1 : 0, 0);
}

void SetViewport(int x, int y, int w, int h)
{
    D3D11_VIEWPORT vp = {};
    vp.TopLeftX = static_cast<float>(x);
    vp.TopLeftY = static_cast<float>(y);
    vp.Width    = static_cast<float>(w);
    vp.Height   = static_cast<float>(h);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    g_Context->RSSetViewports(1, &vp);
}

void Clear(bool color, bool depth, float r, float g, float b, float a)
{
    if (color && g_RTV)
    {
        const float rgba[4] = { r, g, b, a };
        g_Context->ClearRenderTargetView(g_RTV, rgba);
    }
    if (depth && g_DSV)
    {
        g_Context->ClearDepthStencilView(g_DSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }
}

void OnResize(int width, int height)
{
    if (!g_SwapChain) return;
    ReleaseRenderTargetAndDepth();
    g_Context->OMSetRenderTargets(0, nullptr, nullptr);
    g_SwapChain->ResizeBuffers(0, static_cast<UINT>(width), static_cast<UINT>(height), DXGI_FORMAT_UNKNOWN, 0);
    CreateRenderTargetAndDepth(width, height);
}

// ---- Buffers (DXP-14 increment 1) ----

RHI::BufferHandle CreateVertexBuffer(const void* initialData, size_t sizeBytes, RHI::BufferUsage usage)
{
    return CreateBufferGeneric(D3D11_BIND_VERTEX_BUFFER, initialData, sizeBytes, usage == RHI::BufferUsage::Dynamic, -1);
}

RHI::BufferHandle CreateIndexBuffer(const void* initialData, size_t sizeBytes, RHI::BufferUsage usage)
{
    return CreateBufferGeneric(D3D11_BIND_INDEX_BUFFER, initialData, sizeBytes, usage == RHI::BufferUsage::Dynamic, -1);
}

void UpdateBuffer(RHI::BufferHandle handle, const void* data, size_t sizeBytes)
{
    auto it = g_Buffers.find(handle.id);
    if (it == g_Buffers.end() || !g_Context) return;
    BufferRec& rec = it->second;

    if (static_cast<UINT>(sizeBytes) > rec.capacity)
    {
        // Grow: release and recreate at 2x needed size (mirrors RHI_GL's UpdateBuffer grow policy).
        rec.buffer->Release();
        rec.capacity = static_cast<UINT>(sizeBytes) * 2;
        rec.buffer   = CreateD3DBuffer(rec.capacity, rec.bindFlag, nullptr, true);
        if (!rec.buffer) return;
        if (rec.uniformSlot >= 0)
        {
            g_Context->VSSetConstantBuffers(static_cast<UINT>(rec.uniformSlot), 1, &rec.buffer);
            g_Context->PSSetConstantBuffers(static_cast<UINT>(rec.uniformSlot), 1, &rec.buffer);
        }
    }

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (SUCCEEDED(g_Context->Map(rec.buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        memcpy(mapped.pData, data, sizeBytes);
        g_Context->Unmap(rec.buffer, 0);
    }
    rec.writeOffset = 0; // discard resets AppendBuffer's ring position too, per the header's "do not mix" contract
}

size_t AppendBuffer(RHI::BufferHandle handle, const void* data, size_t sizeBytes)
{
    auto it = g_Buffers.find(handle.id);
    if (it == g_Buffers.end() || !g_Context) return 0;
    BufferRec& rec = it->second;
    const UINT needed = static_cast<UINT>(sizeBytes);

    D3D11_MAP mapType = D3D11_MAP_WRITE_NO_OVERWRITE;
    if (needed > rec.capacity)
    {
        rec.buffer->Release();
        rec.capacity = needed * 2;
        rec.buffer   = CreateD3DBuffer(rec.capacity, rec.bindFlag, nullptr, true);
        if (!rec.buffer) return 0;
        rec.writeOffset = 0;
        mapType = D3D11_MAP_WRITE_DISCARD;
    }
    else if (rec.writeOffset + needed > rec.capacity)
    {
        rec.writeOffset = 0;
        mapType = D3D11_MAP_WRITE_DISCARD;
    }

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (SUCCEEDED(g_Context->Map(rec.buffer, 0, mapType, 0, &mapped)))
    {
        memcpy(static_cast<uint8_t*>(mapped.pData) + rec.writeOffset, data, sizeBytes);
        g_Context->Unmap(rec.buffer, 0);
    }

    const UINT offsetWritten = rec.writeOffset;
    rec.writeOffset += needed;
    return static_cast<size_t>(offsetWritten);
}

void DestroyBuffer(RHI::BufferHandle handle)
{
    auto it = g_Buffers.find(handle.id);
    if (it == g_Buffers.end()) return;
    if (it->second.buffer) it->second.buffer->Release();
    g_Buffers.erase(it);
}

RHI::BufferHandle CreateUniformBlock(size_t sizeBytes, int bindingSlot)
{
    // D3D11 constant buffers must be a multiple of 16 bytes.
    const UINT rounded = (static_cast<UINT>(sizeBytes) + 15u) & ~15u;
    return CreateBufferGeneric(D3D11_BIND_CONSTANT_BUFFER, nullptr, rounded, true, bindingSlot);
}

void UpdateUniformBlock(RHI::BufferHandle handle, const void* data, size_t sizeBytes)
{
    auto it = g_Buffers.find(handle.id);
    if (it == g_Buffers.end() || !g_Context) return;
    BufferRec& rec = it->second;
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (SUCCEEDED(g_Context->Map(rec.buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        const size_t copySize = sizeBytes < rec.capacity ? sizeBytes : rec.capacity;
        memcpy(mapped.pData, data, copySize);
        g_Context->Unmap(rec.buffer, 0);
    }
}

void DestroyUniformBlock(RHI::BufferHandle handle)
{
    // Explicitly qualified: ADL would otherwise find RHI::DestroyBuffer too (its parameter
    // type, RHI::BufferHandle, lives in namespace RHI), making an unqualified call ambiguous
    // against this namespace's own DestroyBuffer.
    RHI_D3D11_Impl::DestroyBuffer(handle);
}

// ---- Vertex layout + binding (all 4 layouts implemented as of DXP-14 increment 4) ----

void RegisterVertexShaderBytecode(RHI::VertexLayout layout, const void* bytecode, size_t bytecodeSize)
{
    const int idx = static_cast<int>(layout);
    if (idx < 0 || idx >= kVertexLayoutCount || !bytecode || bytecodeSize == 0) return;
    g_LayoutBytecode[idx].assign(static_cast<const unsigned char*>(bytecode), static_cast<const unsigned char*>(bytecode) + bytecodeSize);
    if (g_InputLayout[idx]) { g_InputLayout[idx]->Release(); g_InputLayout[idx] = nullptr; } // re-registration invalidates a stale layout
}

void BindVertexBuffer(RHI::BufferHandle handle, RHI::VertexLayout layout)
{
    if (!handle.IsValid() || !g_Context) return;
    const int idx = static_cast<int>(layout);
    if (idx < 0 || idx >= kVertexLayoutCount) return;

    if (layout != RHI::VertexLayout::PosUvColor && layout != RHI::VertexLayout::Terrain &&
        layout != RHI::VertexLayout::BMDMesh && layout != RHI::VertexLayout::PosOnly)
    {
        static bool warned[kVertexLayoutCount] = {};
        if (!warned[idx])
        {
            g_ErrorReport.Write(L"[RHI_D3D11] BindVertexBuffer: VertexLayout %d not yet implemented\r\n", idx);
            warned[idx] = true;
        }
        return;
    }

    if (!EnsureInputLayout(idx)) return;

    auto it = g_Buffers.find(handle.id);
    if (it == g_Buffers.end()) return;

    g_Context->IASetInputLayout(g_InputLayout[idx]);
    if (g_CurrentSlot0BufferId != handle.id)
    {
        ID3D11Buffer* vb = it->second.buffer;
        const UINT stride = StrideForLayout(idx);
        const UINT offset = 0;
        g_Context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        g_CurrentSlot0BufferId = handle.id;
    }
}

void BindIndexBuffer(RHI::BufferHandle handle)
{
    if (!handle.IsValid() || !g_Context) return;
    if (g_CurrentIndexBufferId == handle.id) return;

    auto it = g_Buffers.find(handle.id);
    if (it == g_Buffers.end()) return;

    // RHI.h's CreateIndexBuffer contract is GL_UNSIGNED_INT only -- matches R32_UINT here.
    g_Context->IASetIndexBuffer(it->second.buffer, DXGI_FORMAT_R32_UINT, 0);
    g_CurrentIndexBufferId = handle.id;
}

// ---- Draw ----

bool ResolveTopology(RHI::Topology topology, D3D11_PRIMITIVE_TOPOLOGY& out)
{
    switch (topology)
    {
    case RHI::Topology::TriangleList: out = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; return true;
    case RHI::Topology::LineList:     out = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;     return true;
    case RHI::Topology::LineStrip:    out = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;    return true;
    default: return false;
    }
}

void SetTopologyIfChanged(D3D11_PRIMITIVE_TOPOLOGY topo)
{
    if (g_CurrentTopology == topo) return;
    g_CurrentTopology = topo;
    g_Context->IASetPrimitiveTopology(topo);
}

void Draw(RHI::Topology topology, uint32_t vertexCount, uint32_t firstVertex)
{
    if (!g_Context) return;
    D3D11_PRIMITIVE_TOPOLOGY topo;
    if (!ResolveTopology(topology, topo)) return;
    SetTopologyIfChanged(topo);
    g_Context->Draw(vertexCount, firstVertex);
}

void DrawIndexed(RHI::Topology topology, uint32_t indexCount, uint32_t firstIndex)
{
    if (!g_Context) return;
    D3D11_PRIMITIVE_TOPOLOGY topo;
    if (!ResolveTopology(topology, topo)) return;
    SetTopologyIfChanged(topo);
    g_Context->DrawIndexed(indexCount, firstIndex, 0);
}

// ---- Textures (DXP-15 increment 1) ----

RHI::TextureHandle CreateTexture(const RHI::TextureDesc& desc, const void* initialPixelsRGBA)
{
    if (!g_Device || desc.width <= 0 || desc.height <= 0) return {};

    // DXP-16 increment 3: full auto-generated mip chain instead of a single MipLevels=1 level.
    // Root cause of a reported D3D11-only banding/moire artifact on tiled ground textures (terrain
    // viewed at distance/oblique angle -- GL screenshot showed smooth detailed grass, D3D11 showed
    // regular horizontal bands, textbook non-mipmapped-minification aliasing). RHI::CreateTexture
    // is the single unified entry point for every texture on this backend (DXP-12), so this fixes
    // it for all callers at once rather than special-casing terrain.
    // Auto mip-gen requires BIND_RENDER_TARGET + MISC_GENERATE_MIPS + MipLevels=0 (full chain) --
    // and D3D11 disallows initial subresource data on such a texture, so the base level is
    // uploaded via UpdateSubresource + GenerateMips after creation instead of at CreateTexture2D
    // time.
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width            = static_cast<UINT>(desc.width);
    texDesc.Height           = static_cast<UINT>(desc.height);
    texDesc.MipLevels        = 0;
    texDesc.ArraySize        = 1;
    texDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM; // matches RHI's "RGBA8 always" contract
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage            = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags        = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    texDesc.MiscFlags        = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    ID3D11Texture2D* tex = nullptr;
    const HRESULT hr = g_Device->CreateTexture2D(&texDesc, nullptr, &tex);
    if (FAILED(hr))
    {
        g_ErrorReport.Write(L"[RHI_D3D11] CreateTexture(%dx%d) failed (hr=0x%08lX)\r\n",
            desc.width, desc.height, static_cast<unsigned long>(hr));
        return {};
    }

    ID3D11ShaderResourceView* srv = nullptr;
    if (FAILED(g_Device->CreateShaderResourceView(tex, nullptr, &srv)))
    {
        g_ErrorReport.Write(L"[RHI_D3D11] CreateShaderResourceView failed for a %dx%d texture\r\n", desc.width, desc.height);
        tex->Release();
        return {};
    }

    if (initialPixelsRGBA && g_Context)
    {
        const UINT rowPitch = static_cast<UINT>(desc.width) * 4;
        g_Context->UpdateSubresource(tex, 0, nullptr, initialPixelsRGBA, rowPitch, 0);
        g_Context->GenerateMips(srv);
    }

    // desc.filter/desc.wrap are recorded per-texture and applied at BindTexture() time via
    // GetOrCreateSampler() above -- see that helper's comment for why (DXP-16 increment 3, was
    // previously "deliberately not applied here").
    TextureRec rec;
    rec.texture = tex;
    rec.srv     = srv;
    rec.filter  = desc.filter;
    rec.wrap    = desc.wrap;
    const uint32_t id = g_NextTextureId++;
    g_Textures[id] = rec;
    return RHI::TextureHandle{ id };
}

void UpdateTexture(RHI::TextureHandle handle, int x, int y, int w, int h, const void* pixelsRGBA)
{
    auto it = g_Textures.find(handle.id);
    if (it == g_Textures.end() || !g_Context || !pixelsRGBA || w <= 0 || h <= 0) return;

    D3D11_BOX box = {};
    box.left   = static_cast<UINT>(x);
    box.top    = static_cast<UINT>(y);
    box.front  = 0;
    box.right  = static_cast<UINT>(x + w);
    box.bottom = static_cast<UINT>(y + h);
    box.back   = 1;

    const UINT srcRowPitch = static_cast<UINT>(w) * 4;
    g_Context->UpdateSubresource(it->second.texture, 0, &box, pixelsRGBA, srcRowPitch, 0);

    // DXP-16 fix: used to call GenerateMips(it->second.srv) here to keep the auto-generated mip
    // chain (CreateTexture) in sync with partial base-level updates. Every real caller of
    // UpdateTexture (grep-confirmed: UIControls.cpp's font-atlas repaint, ZzzInventory.cpp's item
    // icon repaints) is a UI/2D bitmap that's always drawn near 1:1 pixel scale -- it never needs
    // mips, and the font atlas in particular gets UpdateTexture'd many times per frame (once per
    // RenderText call). GenerateMips is documented (D3D11 remarks) to have driver-internal side
    // effects on pipeline state (bound shaders/SRVs/blend/depth-stencil/viewport) -- calling it
    // this often was unnecessary overhead in the best case and a state-corruption risk in the
    // worst. Terrain, the one caller that actually needs mips for minification, never calls
    // UpdateTexture -- its mip chain is built once from CreateTexture's initial upload and never
    // partially repainted at runtime.
}

void DestroyTexture(RHI::TextureHandle handle)
{
    auto it = g_Textures.find(handle.id);
    if (it == g_Textures.end()) return;
    if (it->second.srv)     it->second.srv->Release();
    if (it->second.texture) it->second.texture->Release();
    g_Textures.erase(it);
}

void BindTexture(RHI::TextureHandle handle, int slot)
{
    if (!g_Context) return;
    if (slot < 0 || slot >= kMaxTextureSlots) return;

    // Dirty-check: skip the rebind entirely when this slot already holds this exact handle (same
    // shape as g_CurrentSlot0BufferId above). See its declaration comment for why this matters --
    // terrain's per-tile draw loop calls this twice per tile (base+overlay) for potentially
    // thousands of tiles a frame, almost always with the same texture as the previous tile.
    if (g_CurrentTextureBound[slot] && g_CurrentTextureId[slot] == handle.id) return;
    g_CurrentTextureBound[slot] = true;
    g_CurrentTextureId[slot]    = handle.id;

    auto it = g_Textures.find(handle.id);
    ID3D11ShaderResourceView* srv = (it != g_Textures.end()) ? it->second.srv : nullptr;
    g_Context->PSSetShaderResources(static_cast<UINT>(slot), 1, &srv);

    // DXP-16 increment 3: bind this texture's own recorded filter/wrap sampler, not whatever the
    // calling shader's Bind() left in that slot -- see GetOrCreateSampler()'s comment for why.
    //
    // Invalid/empty handle (mesh has no texture for this slot, e.g. an item with no specular/
    // chrome map -- the common case, BMDMeshShader calls this with an invalid handle for slots
    // 2/3 whenever those optional maps are absent) still gets a default sampler here rather than
    // leaving the slot unbound: the SRV is null either way so which sampler is bound is
    // functionally irrelevant, but an unbound sampler at a slot the pixel shader declares makes
    // the D3D11 debug layer log a "Sampler not set" warning on every single Draw call -- in
    // Debug builds (where the info-queue reader mirrors these into MuError.log) this was flooding
    // the log to tens of megabytes per session and was the actual cause of "Debug is unusably
    // slow," not GPU or CPU work.
    ID3D11SamplerState* sampler = (it != g_Textures.end())
        ? GetOrCreateSampler(it->second.filter, it->second.wrap)
        : GetOrCreateSampler(RHI::TexFilter::Linear, RHI::TexWrap::Clamp);
    if (sampler)
        g_Context->PSSetSamplers(static_cast<UINT>(slot), 1, &sampler);
}

// ---- Pipeline / blend-mode state (DXP-15 increment 2) ----

void SetBlendMode(RHI::BlendMode mode)
{
    if (!g_Context) return;
    const int idx = static_cast<int>(mode);
    if (idx < 0 || idx >= kBlendModeCount) return;
    const BlendCombo& c = kBlendCombos[idx];

    g_Context->OMSetBlendState(g_BlendState[idx], nullptr, 0xFFFFFFFF);
    g_CurrentCullOn = c.cullOn;
    ApplyRasterizerState();

    g_DepthWriteEnabled = c.depthWriteOn;
    ApplyDepthStencilState();
}

void SetDepthTestEnabled(bool enabled)
{
    g_DepthTestEnabled = enabled;
    ApplyDepthStencilState();
}

// DXP-15 increment 3: standalone toggles for the direct EnableCullFace/DisableCullFace/
// EnableDepthMask/DisableDepthMask call sites outside the blend-mode wrapper family. Reuse the
// same precomputed rasterizer/depth-stencil objects SetBlendMode swaps between -- a standalone
// toggle just needs to pick the matching object for the OTHER axis without touching blend state.
void SetCullEnabled(bool enabled)
{
    if (!g_Context) return;
    g_CurrentCullOn = enabled;
    ApplyRasterizerState();
}

// DXP-16 increment 4: real implementation, was a fail-loud stub. See EnsurePolyOffsetRasterizerStates'
// comment (near g_RasterizerCullOnBias) for the D3D11-vs-GL depth-bias mapping.
void SetPolygonOffset(bool enabled, float factor, float units)
{
    if (!g_Context) return;
    g_PolyOffsetEnabled = enabled;
    if (enabled) EnsurePolyOffsetRasterizerStates(factor, units);
    ApplyRasterizerState();
}

void SetDepthWriteEnabled(bool enabled)
{
    g_DepthWriteEnabled = enabled;
    ApplyDepthStencilState();
}

// ---- Backend-native device access (shader classes' own D3D11 compile/bind work) ----

void* GetD3D11Device()        { return g_Device; }
void* GetD3D11DeviceContext() { return g_Context; }

// DXP-21 phase 3b: see RHI.h's comment. AddRefs so the returned handle's eventual
// RHI::DestroyBuffer and the caller's own Release() are independent, order-safe releases --
// caller (ClothComputeShader) keeps its own reference and destroys the buffer on its own
// schedule regardless of when/whether RHI::DestroyBuffer is called on this handle.
RHI::BufferHandle RegisterExternalD3D11VertexBuffer(void* d3d11Buffer, size_t capacityBytes)
{
    if (!d3d11Buffer) return {};
    ID3D11Buffer* buf = static_cast<ID3D11Buffer*>(d3d11Buffer);
    buf->AddRef();

    BufferRec rec;
    rec.buffer      = buf;
    rec.capacity     = static_cast<UINT>(capacityBytes);
    rec.bindFlag     = D3D11_BIND_VERTEX_BUFFER;
    rec.uniformSlot  = -1;
    const uint32_t id = g_NextBufferId++;
    g_Buffers[id] = rec;
    return RHI::BufferHandle{ id };
}

#else // !RHI_D3D11_AVAILABLE

bool Init(void*, int, int) { NotImplemented(L"Init (D3D11 is Windows-only)"); return false; }
void Shutdown() {}
void BeginFrame() {}
void EndFrame() {}
void SetViewport(int, int, int, int) {}
void Clear(bool, bool, float, float, float, float) {}
void OnResize(int, int) {}

RHI::BufferHandle CreateVertexBuffer(const void*, size_t, RHI::BufferUsage) { return {}; }
RHI::BufferHandle CreateIndexBuffer(const void*, size_t, RHI::BufferUsage)  { return {}; }
void UpdateBuffer(RHI::BufferHandle, const void*, size_t)                   {}
size_t AppendBuffer(RHI::BufferHandle, const void*, size_t)                 { return 0; }
void DestroyBuffer(RHI::BufferHandle)                                       {}

RHI::BufferHandle CreateUniformBlock(size_t, int)                          { return {}; }
void UpdateUniformBlock(RHI::BufferHandle, const void*, size_t)            {}
void DestroyUniformBlock(RHI::BufferHandle)                                 {}

void RegisterVertexShaderBytecode(RHI::VertexLayout, const void*, size_t)  {}
void BindVertexBuffer(RHI::BufferHandle, RHI::VertexLayout)               {}
void BindIndexBuffer(RHI::BufferHandle)                                    {}

void Draw(RHI::Topology, uint32_t, uint32_t)                               {}
void DrawIndexed(RHI::Topology, uint32_t, uint32_t)                        {}

RHI::TextureHandle CreateTexture(const RHI::TextureDesc&, const void*)     { return {}; }
void UpdateTexture(RHI::TextureHandle, int, int, int, int, const void*)   {}
void DestroyTexture(RHI::TextureHandle)                                    {}
void BindTexture(RHI::TextureHandle, int)                                  {}

void SetBlendMode(RHI::BlendMode)                                          {}
void SetDepthTestEnabled(bool)                                             {}
void SetCullEnabled(bool)                                                  {}
void SetDepthWriteEnabled(bool)                                            {}
void SetPolygonOffset(bool, float, float)                                  {}

void* GetD3D11Device()        { return nullptr; }
void* GetD3D11DeviceContext() { return nullptr; }
RHI::BufferHandle RegisterExternalD3D11VertexBuffer(void*, size_t) { return {}; }

#endif // RHI_D3D11_AVAILABLE

// ---- Everything else: fail-loud stubs. Nothing calls these yet -- no game subsystem
// renders through RHI:: under D3D11 this milestone (see "What could go wrong" in
// dxp/DXP-13-d3d11-device-swapchain.md) -- so in practice these should never fire.
// Indexed-draw/readback land with later DXP-15/16 increments. Fog stays a fail-loud stub
// here too -- SetFogEnabled has no caller yet on either backend (fog toggling in the tree
// today is the raw glEnable/glDisable(GL_FOG) calls inside ZzzOpenglUtil.cpp's blend
// wrappers, not yet routed through RHI -- SceneUBO's fogEnabled field is what shaders
// actually read). SetPolygonOffset has a real implementation now (DXP-16 increment 4,
// inside the _WIN32 block above) -- PlanarShadowShader's D3D11 path needs it. ----

void SetFogEnabled(bool)                                                   { NotImplemented(L"SetFogEnabled"); }

bool ReadDepthPixel(int, int, float*)                                      { NotImplemented(L"ReadDepthPixel"); return false; }

#ifdef RHI_D3D11_AVAILABLE
// Implemented (was a fail-loud stub -- SceneManager.cpp/ReconnectDialog.cpp's scene-transition
// background capture calls this unconditionally, so under D3D11 it hit the assert above on every
// close/disconnect/scene-change). CopySubresourceRegion's source box lets the GPU crop directly
// to (x,y,w,h) instead of reading the full backbuffer and cropping on the CPU.
bool ReadColorFramebuffer(int x, int y, int w, int h, void* outRGB)
{
    if (!outRGB || w <= 0 || h <= 0 || !g_SwapChain || !g_Device || !g_Context) return false;

    ID3D11Texture2D* backBuffer = nullptr;
    if (FAILED(g_SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) return false;

    D3D11_TEXTURE2D_DESC stagingDesc = {};
    stagingDesc.Width            = static_cast<UINT>(w);
    stagingDesc.Height           = static_cast<UINT>(h);
    stagingDesc.MipLevels        = 1;
    stagingDesc.ArraySize        = 1;
    stagingDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM; // matches Init()'s swapchain format
    stagingDesc.SampleDesc.Count = 1;
    stagingDesc.Usage            = D3D11_USAGE_STAGING;
    stagingDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_READ;

    ID3D11Texture2D* staging = nullptr;
    const HRESULT hr = g_Device->CreateTexture2D(&stagingDesc, nullptr, &staging);
    if (FAILED(hr)) { backBuffer->Release(); return false; }

    D3D11_BOX box;
    box.left = static_cast<UINT>(x);
    box.top = static_cast<UINT>(y);
    box.front = 0;
    box.right = static_cast<UINT>(x + w);
    box.bottom = static_cast<UINT>(y + h);
    box.back = 1;
    g_Context->CopySubresourceRegion(staging, 0, 0, 0, 0, backBuffer, 0, &box);
    backBuffer->Release();

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (FAILED(g_Context->Map(staging, 0, D3D11_MAP_READ, 0, &mapped)))
    {
        staging->Release();
        return false;
    }

    // Backbuffer is RGBA8 -- drop alpha to match RHI's documented "3 bytes/pixel" contract.
    // D3D11's mapped rows are already top-down (unlike GL's bottom-up glReadPixels), so unlike
    // RHI_GL's ReadColorFramebuffer, no row-flip is needed here to satisfy RHI.h's top-down
    // contract.
    unsigned char* dst = static_cast<unsigned char*>(outRGB);
    const unsigned char* srcBase = static_cast<const unsigned char*>(mapped.pData);
    for (int row = 0; row < h; ++row)
    {
        const unsigned char* srcRow = srcBase + static_cast<size_t>(row) * mapped.RowPitch;
        unsigned char* dstRow = dst + static_cast<size_t>(row) * w * 3;
        for (int col = 0; col < w; ++col)
        {
            dstRow[col * 3 + 0] = srcRow[col * 4 + 0];
            dstRow[col * 3 + 1] = srcRow[col * 4 + 1];
            dstRow[col * 3 + 2] = srcRow[col * 4 + 2];
        }
    }

    g_Context->Unmap(staging, 0);
    staging->Release();
    return true;
}
#else
bool ReadColorFramebuffer(int, int, int, int, void*)                       { NotImplemented(L"ReadColorFramebuffer"); return false; }
#endif

} // namespace RHI_D3D11_Impl
