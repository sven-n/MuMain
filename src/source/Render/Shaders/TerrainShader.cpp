#include "stdafx.h"
#include "TerrainShader.h"
#include "Render/Core/RenderConfig.h"
#include "Render/Core/BindState.h"
#include "Render/RHI/RHI.h"
#include "Core/Utilities/Log/ErrorReport.h"
#include <SDL3/SDL.h>
#ifdef RHI_D3D11_AVAILABLE
#include <d3d11.h>
#include <d3dcompiler.h>
#endif
#include <cstdint>
#include <cstring>

#ifndef APIENTRY
#define APIENTRY
#endif

typedef GLuint (APIENTRY* PFNGLCREATESHADERPROC)(GLenum);
typedef void   (APIENTRY* PFNGLSHADERSOURCEPROC)(GLuint, GLsizei, const GLchar* const*, const GLint*);
typedef void   (APIENTRY* PFNGLCOMPILESHADERPROC)(GLuint);
typedef void   (APIENTRY* PFNGLGETSHADERIVPROC)(GLuint, GLenum, GLint*);
typedef void   (APIENTRY* PFNGLGETSHADERINFOLOGPROC)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef GLuint (APIENTRY* PFNGLCREATEPROGRAMPROC)(void);
typedef void   (APIENTRY* PFNGLATTACHSHADERPROC)(GLuint, GLuint);
typedef void   (APIENTRY* PFNGLLINKPROGRAMPROC)(GLuint);
typedef void   (APIENTRY* PFNGLGETPROGRAMIVPROC)(GLuint, GLenum, GLint*);
typedef void   (APIENTRY* PFNGLGETPROGRAMINFOLOGPROC)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef void   (APIENTRY* PFNGLDELETESHADERPROC)(GLuint);
typedef void   (APIENTRY* PFNGLDELETEPROGRAMPROC)(GLuint);
typedef GLint  (APIENTRY* PFNGLGETUNIFORMLOCATIONPROC)(GLuint, const GLchar*);
typedef GLuint (APIENTRY* PFNGLGETUNIFORMBLOCKINDEXPROC)(GLuint, const GLchar*);
typedef void   (APIENTRY* PFNGLUNIFORMBLOCKBINDINGPROC)(GLuint, GLuint, GLuint);
typedef void   (APIENTRY* PFNGLUNIFORM1IPROC)(GLint, GLint);
typedef void   (APIENTRY* PFNGLUNIFORM1FPROC)(GLint, GLfloat);
typedef void   (APIENTRY* PFNGLACTIVETEXTUREPROC)(GLenum);

static PFNGLCREATESHADERPROC       fn_glCreateShader       = nullptr;
static PFNGLSHADERSOURCEPROC       fn_glShaderSource        = nullptr;
static PFNGLCOMPILESHADERPROC      fn_glCompileShader       = nullptr;
static PFNGLGETSHADERIVPROC        fn_glGetShaderiv         = nullptr;
static PFNGLGETSHADERINFOLOGPROC   fn_glGetShaderInfoLog    = nullptr;
static PFNGLCREATEPROGRAMPROC      fn_glCreateProgram       = nullptr;
static PFNGLATTACHSHADERPROC       fn_glAttachShader        = nullptr;
static PFNGLLINKPROGRAMPROC        fn_glLinkProgram         = nullptr;
static PFNGLGETPROGRAMIVPROC       fn_glGetProgramiv        = nullptr;
static PFNGLGETPROGRAMINFOLOGPROC  fn_glGetProgramInfoLog   = nullptr;
static PFNGLDELETESHADERPROC       fn_glDeleteShader        = nullptr;
static PFNGLDELETEPROGRAMPROC      fn_glDeleteProgram       = nullptr;
static PFNGLGETUNIFORMLOCATIONPROC fn_glGetUniformLocation  = nullptr;
static PFNGLGETUNIFORMBLOCKINDEXPROC fn_glGetUniformBlockIndex = nullptr;
static PFNGLUNIFORMBLOCKBINDINGPROC fn_glUniformBlockBinding = nullptr;
static PFNGLUNIFORM1IPROC          fn_glUniform1i           = nullptr;
static PFNGLUNIFORM1FPROC          fn_glUniform1f           = nullptr;
static PFNGLACTIVETEXTUREPROC      fn_glActiveTexture       = nullptr;

static bool LoadGLShaderFunctions()
{
    static bool loaded = false;
    if (loaded) return true;

    fn_glCreateShader       = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
    fn_glShaderSource        = (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
    fn_glCompileShader       = (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
    fn_glGetShaderiv         = (PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv");
    fn_glGetShaderInfoLog    = (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog");
    fn_glCreateProgram       = (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
    fn_glAttachShader        = (PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader");
    fn_glLinkProgram         = (PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram");
    fn_glGetProgramiv        = (PFNGLGETPROGRAMIVPROC)SDL_GL_GetProcAddress("glGetProgramiv");
    fn_glGetProgramInfoLog   = (PFNGLGETPROGRAMINFOLOGPROC)SDL_GL_GetProcAddress("glGetProgramInfoLog");
    fn_glDeleteShader        = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");
    fn_glDeleteProgram       = (PFNGLDELETEPROGRAMPROC)SDL_GL_GetProcAddress("glDeleteProgram");
    fn_glGetUniformLocation  = (PFNGLGETUNIFORMLOCATIONPROC)SDL_GL_GetProcAddress("glGetUniformLocation");
    fn_glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)SDL_GL_GetProcAddress("glGetUniformBlockIndex");
    fn_glUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC)SDL_GL_GetProcAddress("glUniformBlockBinding");
    fn_glUniform1i           = (PFNGLUNIFORM1IPROC)SDL_GL_GetProcAddress("glUniform1i");
    fn_glUniform1f           = (PFNGLUNIFORM1FPROC)SDL_GL_GetProcAddress("glUniform1f");
    fn_glActiveTexture       = (PFNGLACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glActiveTexture");

    loaded = (fn_glCreateShader != nullptr &&
              fn_glShaderSource != nullptr &&
              fn_glCompileShader != nullptr &&
              fn_glGetShaderiv != nullptr &&
              fn_glCreateProgram != nullptr &&
              fn_glAttachShader != nullptr &&
              fn_glLinkProgram != nullptr &&
              fn_glGetProgramiv != nullptr &&
              fn_glDeleteShader != nullptr &&
              fn_glDeleteProgram != nullptr &&
              fn_glGetUniformLocation != nullptr &&
              fn_glUniform1i != nullptr &&
              fn_glUniform1f != nullptr &&
              fn_glActiveTexture != nullptr);
    return loaded;
}

static const char* g_szTerrainVert = R"(
#version 330 core

layout(std140) uniform GlobalMatrices {
    mat4 u_View;
    mat4 u_Proj;
    mat4 u_Model;
    mat4 u_MVP;
    vec4 u_Time;
};

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Light;
layout(location = 2) in float a_Alpha;

// TASK-30 phase 2: water UV scroll, ported from FaceTexture()'s CPU `suf += WaterMove` (U-axis only,
// no V-axis scroll — matches legacy, which only ever offsets Water1/Water2 (both always 0) on U).
// u_WaterMove is set once per frame; u_BaseIsWater/u_OverlayIsWater once per tile draw.
uniform float u_WaterMove;
uniform int   u_BaseIsWater;
uniform int   u_OverlayIsWater;

out vec2 v_UVBase;
out vec2 v_UVOverlay;
out vec3 v_Light;
out float v_Alpha;

void main() {
    vec2 baseUV = a_Pos.xy * 0.0025;
    v_UVBase    = (u_BaseIsWater    == 1) ? baseUV + vec2(u_WaterMove, 0.0) : baseUV;
    v_UVOverlay = (u_OverlayIsWater == 1) ? baseUV + vec2(u_WaterMove, 0.0) : baseUV;
    v_Light = a_Light;
    v_Alpha = a_Alpha;
    gl_Position = u_MVP * vec4(a_Pos, 1.0);
}
)";

static const char* g_szTerrainFrag = R"(
#version 330 core

in vec2 v_UVBase;
in vec2 v_UVOverlay;
in vec3 v_Light;
in float v_Alpha;

uniform sampler2D u_BaseTex;
uniform sampler2D u_OverlayTex;
// Fixed-function alpha test threshold (DXP-01), mirrored from glAlphaFunc/GL_ALPHA_TEST state.
// -1.0 means alpha test is disabled (no discard); otherwise the GL_GREATER reference value.
uniform float u_AlphaRef;

out vec4 FragColor;

void main() {
    vec4 texBase = texture(u_BaseTex, v_UVBase);
    vec4 texOverlay = texture(u_OverlayTex, v_UVOverlay);
    vec4 blendedTex = mix(texBase, texOverlay, clamp(v_Alpha, 0.0, 1.0));

    // <= matches GL_GREATER exactly (fixed-function alpha test passes when a > ref).
    if (u_AlphaRef >= 0.0 && blendedTex.a <= u_AlphaRef) discard;

    FragColor = vec4(blendedTex.rgb * v_Light, blendedTex.a);
}
)";

// DXP-14 -- HLSL twin, kept semantically line-for-line with the GLSL above. The three loose
// GLSL uniforms (u_WaterMove/u_BaseIsWater/u_OverlayIsWater, referenced from the vertex stage,
// and u_AlphaRef, referenced from the pixel stage) pack into one TerrainFlags cbuffer at slot 5
// (see TerrainShader.h's field comment for why each shader gets its own slot number).
static const char* g_szTerrainVertHLSL = R"(
cbuffer GlobalMatrices : register(b0)
{
    column_major matrix u_View;
    column_major matrix u_Proj;
    column_major matrix u_Model;
    column_major matrix u_MVP;
    float4 u_Time;
};

cbuffer TerrainFlags : register(b5)
{
    float u_WaterMove;
    int   u_BaseIsWater;
    int   u_OverlayIsWater;
    float u_AlphaRef;
};

struct VSInput
{
    float3 a_Pos   : POSITION;
    float3 a_Light : COLOR0;
    float  a_Alpha : TEXCOORD0;
};

struct VSOutput
{
    float4 pos       : SV_POSITION;
    float2 uvBase    : TEXCOORD0;
    float2 uvOverlay : TEXCOORD1;
    float3 light     : COLOR0;
    float  alpha     : TEXCOORD2;
};

VSOutput main(VSInput input)
{
    VSOutput o;
    float2 baseUV = input.a_Pos.xy * 0.0025;
    o.uvBase    = (u_BaseIsWater    != 0) ? baseUV + float2(u_WaterMove, 0.0) : baseUV;
    o.uvOverlay = (u_OverlayIsWater != 0) ? baseUV + float2(u_WaterMove, 0.0) : baseUV;
    o.light = input.a_Light;
    o.alpha = input.a_Alpha;
    o.pos   = mul(u_MVP, float4(input.a_Pos, 1.0));
    return o;
}
)";

static const char* g_szTerrainFragHLSL = R"(
cbuffer TerrainFlags : register(b5)
{
    float u_WaterMove;
    int   u_BaseIsWater;
    int   u_OverlayIsWater;
    float u_AlphaRef;
};

Texture2D    u_BaseTex     : register(t0);
SamplerState u_BaseSamp    : register(s0);
Texture2D    u_OverlayTex  : register(t1);
SamplerState u_OverlaySamp : register(s1);

struct PSInput
{
    float4 pos       : SV_POSITION;
    float2 uvBase    : TEXCOORD0;
    float2 uvOverlay : TEXCOORD1;
    float3 light     : COLOR0;
    float  alpha     : TEXCOORD2;
};

float4 main(PSInput input) : SV_TARGET
{
    float4 texBase    = u_BaseTex.Sample(u_BaseSamp, input.uvBase);
    float4 texOverlay = u_OverlayTex.Sample(u_OverlaySamp, input.uvOverlay);
    float4 blendedTex = lerp(texBase, texOverlay, saturate(input.alpha));

    // <= matches GL_GREATER exactly (fixed-function alpha test passes when a > ref).
    if (u_AlphaRef >= 0.0 && blendedTex.a <= u_AlphaRef)
        discard;

    return float4(blendedTex.rgb * input.light, blendedTex.a);
}
)";

namespace {
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
                g_ErrorReport.Write(L"[TerrainShader][D3D11] %hs compile error: %hs\r\n",
                    entryPoint, static_cast<const char*>(errorBlob->GetBufferPointer()));
                errorBlob->Release();
            }
            else
            {
                g_ErrorReport.Write(L"[TerrainShader][D3D11] %hs compile failed, hr=0x%08lX\r\n",
                    entryPoint, static_cast<unsigned long>(hr));
            }
            return false;
        }
        if (errorBlob) errorBlob->Release(); // warnings only
        return true;
    }

    struct TerrainFlagsCB { float waterMove; int32_t baseIsWater; int32_t overlayIsWater; float alphaRef; };
}

TerrainShader& TerrainShader::Instance()
{
    static TerrainShader instance;
    return instance;
}

TerrainShader::~TerrainShader()
{
    Destroy();
}

void TerrainShader::Create()
{
    if (g_RenderBackend == RenderBackend::D3D11) { CreateD3D11(); return; }
    CreateGL();
}

void TerrainShader::Destroy()
{
    if (g_RenderBackend == RenderBackend::D3D11) { DestroyD3D11(); return; }
    DestroyGL();
}

void TerrainShader::Bind()
{
    if (g_RenderBackend == RenderBackend::D3D11) { BindD3D11(); return; }
    BindGL();
}

void TerrainShader::Unbind()
{
    if (g_RenderBackend == RenderBackend::D3D11)
    {
        ID3D11DeviceContext* ctx = static_cast<ID3D11DeviceContext*>(RHI::GetD3D11DeviceContext());
        if (ctx)
        {
            ctx->VSSetShader(nullptr, nullptr, 0);
            ctx->PSSetShader(nullptr, nullptr, 0);
        }
        return;
    }
    BindProgram(0);
}

void TerrainShader::CreateGL()
{
    if (m_Program != 0) return;
    if (!LoadGLShaderFunctions()) return;

    GLint success = 0;
    char infoLog[512];

    m_VertShader = fn_glCreateShader(GL_VERTEX_SHADER);
    fn_glShaderSource(m_VertShader, 1, &g_szTerrainVert, nullptr);
    fn_glCompileShader(m_VertShader);
    fn_glGetShaderiv(m_VertShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        if (fn_glGetShaderInfoLog) fn_glGetShaderInfoLog(m_VertShader, 512, nullptr, infoLog);
        SDL_Log("[TerrainShader] Vert compile error: %s", infoLog);
        fn_glDeleteShader(m_VertShader);
        m_VertShader = 0;
        return;
    }

    m_FragShader = fn_glCreateShader(GL_FRAGMENT_SHADER);
    fn_glShaderSource(m_FragShader, 1, &g_szTerrainFrag, nullptr);
    fn_glCompileShader(m_FragShader);
    fn_glGetShaderiv(m_FragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        if (fn_glGetShaderInfoLog) fn_glGetShaderInfoLog(m_FragShader, 512, nullptr, infoLog);
        SDL_Log("[TerrainShader] Frag compile error: %s", infoLog);
        fn_glDeleteShader(m_VertShader);
        fn_glDeleteShader(m_FragShader);
        m_VertShader = m_FragShader = 0;
        return;
    }

    m_Program = fn_glCreateProgram();
    fn_glAttachShader(m_Program, m_VertShader);
    fn_glAttachShader(m_Program, m_FragShader);
    fn_glLinkProgram(m_Program);
    fn_glGetProgramiv(m_Program, GL_LINK_STATUS, &success);
    if (!success) {
        if (fn_glGetProgramInfoLog) fn_glGetProgramInfoLog(m_Program, 512, nullptr, infoLog);
        SDL_Log("[TerrainShader] Program link error: %s", infoLog);
        fn_glDeleteProgram(m_Program);
        InvalidateProgramCache();
        fn_glDeleteShader(m_VertShader);
        fn_glDeleteShader(m_FragShader);
        m_Program = m_VertShader = m_FragShader = 0;
        return;
    }

    // Bind GlobalMatrices uniform block to binding point 0
    if (fn_glGetUniformBlockIndex != nullptr && fn_glUniformBlockBinding != nullptr) {
        GLuint blockIdx = fn_glGetUniformBlockIndex(m_Program, "GlobalMatrices");
        if (blockIdx != GL_INVALID_INDEX) {
            fn_glUniformBlockBinding(m_Program, blockIdx, 0);
        }
    }

    // Resolve uniform locations
    m_LocBaseTex    = fn_glGetUniformLocation(m_Program, "u_BaseTex");
    m_LocOverlayTex = fn_glGetUniformLocation(m_Program, "u_OverlayTex");
    m_LocWaterMove      = fn_glGetUniformLocation(m_Program, "u_WaterMove");
    m_LocBaseIsWater    = fn_glGetUniformLocation(m_Program, "u_BaseIsWater");
    m_LocOverlayIsWater = fn_glGetUniformLocation(m_Program, "u_OverlayIsWater");
    m_LocAlphaRef       = fn_glGetUniformLocation(m_Program, "u_AlphaRef");

    // Assign default texture unit slots (u_BaseTex = 0, u_OverlayTex = 1)
    BindProgram(m_Program);
    if (m_LocBaseTex != -1) fn_glUniform1i(m_LocBaseTex, 0);
    if (m_LocOverlayTex != -1) fn_glUniform1i(m_LocOverlayTex, 1);
    if (m_LocWaterMove != -1) fn_glUniform1f(m_LocWaterMove, 0.0f);
    if (m_LocBaseIsWater != -1) fn_glUniform1i(m_LocBaseIsWater, 0);
    if (m_LocOverlayIsWater != -1) fn_glUniform1i(m_LocOverlayIsWater, 0);
    if (m_LocAlphaRef != -1) fn_glUniform1f(m_LocAlphaRef, -1.0f);
    m_LastAlphaRef = -1.0f;
    BindProgram(0);

    g_ErrorReport.Write(L"[TerrainShader] Created program ID %d, u_BaseTex loc %d, u_OverlayTex loc %d\r\n", m_Program, m_LocBaseTex, m_LocOverlayTex);
}

void TerrainShader::DestroyGL()
{
    if (!m_Program) return;

    BindProgram(0);
    if (fn_glDeleteProgram) { fn_glDeleteProgram(m_Program); InvalidateProgramCache(); }
    if (fn_glDeleteShader) {
        if (m_VertShader) fn_glDeleteShader(m_VertShader);
        if (m_FragShader) fn_glDeleteShader(m_FragShader);
    }
    m_Program = m_VertShader = m_FragShader = 0;
}

void TerrainShader::BindGL()
{
    if (m_Program != 0) {
        BindProgram(m_Program);
    }
}

#ifdef RHI_D3D11_AVAILABLE
void TerrainShader::CreateD3D11()
{
    if (m_D3DVertexShader) return;
    ID3D11Device* device = static_cast<ID3D11Device*>(RHI::GetD3D11Device());
    if (!device) return;

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    if (!CompileHLSL(g_szTerrainVertHLSL, "main", "vs_5_0", &vsBlob)) return;
    if (!CompileHLSL(g_szTerrainFragHLSL, "main", "ps_5_0", &psBlob)) { vsBlob->Release(); return; }

    ID3D11VertexShader* vs = nullptr;
    if (FAILED(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vs)))
    {
        g_ErrorReport.Write(L"[TerrainShader][D3D11] CreateVertexShader failed\r\n");
        vsBlob->Release(); psBlob->Release();
        return;
    }
    ID3D11PixelShader* ps = nullptr;
    if (FAILED(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &ps)))
    {
        g_ErrorReport.Write(L"[TerrainShader][D3D11] CreatePixelShader failed\r\n");
        vs->Release(); vsBlob->Release(); psBlob->Release();
        return;
    }

    // Registers this shader's bytecode as the Terrain layout's signature source (D3D11
    // requirement -- see RHI.h's RegisterVertexShaderBytecode comment).
    RHI::RegisterVertexShaderBytecode(RHI::VertexLayout::Terrain, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize());
    vsBlob->Release();
    psBlob->Release();

    m_D3DVertexShader = vs;
    m_D3DPixelShader = ps;

    m_D3DFlagsCBuffer = RHI::CreateUniformBlock(sizeof(TerrainFlagsCB), 5);
    m_D3DWaterMove = 0.0f;
    m_D3DBaseIsWater = 0;
    m_D3DOverlayIsWater = 0;
    m_LastAlphaRef = -1.0f;
    UploadD3D11Flags();

    // DXP-16 increment 2 fix (superseded by increment 3, see below): this shader used to own a
    // single fixed sampler (WRAP, for the `a_Pos.xy * 0.0025` tiling UV) bound to both s0/s1.
    // DXP-16 increment 3: replaced by RHI::BindTexture() binding each texture's OWN recorded
    // filter/wrap sampler at bind time (RHI_D3D11.cpp's GetOrCreateSampler) -- a fixed WRAP
    // sampler for every texture bound through this shader was itself the bug behind a reported
    // "rice terrace" banding artifact on ordinary grass terrain (non-power-of-two source textures
    // padded up to storage size by GlobalBitmap.cpp wrap right into their own padding under WRAP,
    // where GL would have used that texture's own correct wrap mode instead). This shader no
    // longer creates/binds a sampler of its own; SetBaseTexture()/SetOverlayTexture()'s
    // RHI::BindTexture() calls (below) now do it correctly, per texture, every draw.

    g_ErrorReport.Write(L"[TerrainShader][D3D11] Compiled + linked, TerrainFlags cbuffer at slot 5\r\n");
}

void TerrainShader::DestroyD3D11()
{
    if (m_D3DFlagsCBuffer.IsValid()) { RHI::DestroyUniformBlock(m_D3DFlagsCBuffer); m_D3DFlagsCBuffer = {}; }
    if (m_D3DPixelShader)  { static_cast<ID3D11PixelShader*>(m_D3DPixelShader)->Release();   m_D3DPixelShader = nullptr; }
    if (m_D3DVertexShader) { static_cast<ID3D11VertexShader*>(m_D3DVertexShader)->Release(); m_D3DVertexShader = nullptr; }
}

void TerrainShader::BindD3D11()
{
    ID3D11DeviceContext* ctx = static_cast<ID3D11DeviceContext*>(RHI::GetD3D11DeviceContext());
    if (!ctx || !m_D3DVertexShader || !m_D3DPixelShader) return;

    ctx->VSSetShader(static_cast<ID3D11VertexShader*>(m_D3DVertexShader), nullptr, 0);
    ctx->PSSetShader(static_cast<ID3D11PixelShader*>(m_D3DPixelShader), nullptr, 0);
}

void TerrainShader::UploadD3D11Flags()
{
    if (!m_D3DFlagsCBuffer.IsValid()) return;
    TerrainFlagsCB cb;
    cb.waterMove      = m_D3DWaterMove;
    cb.baseIsWater    = m_D3DBaseIsWater;
    cb.overlayIsWater = m_D3DOverlayIsWater;
    cb.alphaRef       = m_LastAlphaRef;
    RHI::UpdateUniformBlock(m_D3DFlagsCBuffer, &cb, sizeof(cb));
}
#else // !RHI_D3D11_AVAILABLE -- keep these symbols linkable as no-ops since Create()/Destroy()/
      // Bind() call them behind a runtime g_RenderBackend check, which itself compiles on every
      // platform (see RHI_D3D11.cpp's own #else for the same rationale).
void TerrainShader::CreateD3D11() {}
void TerrainShader::DestroyD3D11() {}
void TerrainShader::BindD3D11() {}
void TerrainShader::UploadD3D11Flags() {}
#endif // RHI_D3D11_AVAILABLE

extern int CachTexture;

void TerrainShader::SetBaseTexture(GLuint texID)
{
    // DXP-16 increment 1: texID is already an RHI::TextureHandle id here regardless of backend
    // (BITMAP_t::TextureNumber is set from RHI::CreateTexture(...).id since DXP-12's texture
    // unification) -- same "reuse the GLuint param as the RHI handle id" pattern already used by
    // ZzzOpenglUtil.cpp's BindTexture(int).
    if (g_RenderBackend == RenderBackend::D3D11)
    {
        RHI::BindTexture(RHI::TextureHandle{ texID }, 0);
        // DXP-16 fix: this direct RHI::BindTexture bypasses ZzzOpenglUtil.cpp's BindTexture(int)
        // wrapper, which UI 2D sprite rendering (Sprite::Render) relies on via a global CachTexture
        // cache to skip redundant slot-0 rebinds. Without invalidating it here, the UI's next
        // BindTexture(uiTexID) call sees CachTexture still equal to uiTexID (from a previous UI
        // frame) and skips the real rebind, leaving slot 0 bound to whatever this terrain draw just
        // set -- causing UI bitmaps (e.g. the login window background) to intermittently render with
        // the wrong texture. The GL branch above already does this; D3D11 needs the same.
        CachTexture = -1;
        return;
    }
    BindTexture2D(0, texID);
    CachTexture = -1; // Invalidate engine texture cache
}

void TerrainShader::SetOverlayTexture(GLuint texID)
{
    if (g_RenderBackend == RenderBackend::D3D11)
    {
        RHI::BindTexture(RHI::TextureHandle{ texID }, 1);
        return;
    }
    BindTexture2D(1, texID);
    if (fn_glActiveTexture) fn_glActiveTexture(GL_TEXTURE0); // Restore default active texture
}

void TerrainShader::SetWaterMove(float waterMove)
{
    if (g_RenderBackend == RenderBackend::D3D11)
    {
        m_D3DWaterMove = waterMove;
        UploadD3D11Flags();
        return;
    }
    if (m_LocWaterMove != -1 && fn_glUniform1f) {
        fn_glUniform1f(m_LocWaterMove, waterMove);
    }
}

void TerrainShader::SetWaterFlags(bool baseIsWater, bool overlayIsWater)
{
    if (g_RenderBackend == RenderBackend::D3D11)
    {
        m_D3DBaseIsWater = baseIsWater ? 1 : 0;
        m_D3DOverlayIsWater = overlayIsWater ? 1 : 0;
        UploadD3D11Flags();
        return;
    }
    if (m_LocBaseIsWater != -1 && fn_glUniform1i) {
        fn_glUniform1i(m_LocBaseIsWater, baseIsWater ? 1 : 0);
    }
    if (m_LocOverlayIsWater != -1 && fn_glUniform1i) {
        fn_glUniform1i(m_LocOverlayIsWater, overlayIsWater ? 1 : 0);
    }
}

void TerrainShader::SyncAlphaRef()
{
    if (g_RenderBackend == RenderBackend::D3D11)
    {
        if (g_AlphaRef != m_LastAlphaRef)
        {
            m_LastAlphaRef = g_AlphaRef;
            UploadD3D11Flags();
        }
        return;
    }
    // Dirty-checked plain float compare — TerrainShader stays bound across many tile draws per
    // frustrum pass, so this must not cost a glUniform1f per tile (per the TASK-24 Lorencia FPS
    // lesson: per-tile GL calls are the enemy).
    if (m_LocAlphaRef != -1 && fn_glUniform1f && g_AlphaRef != m_LastAlphaRef) {
        fn_glUniform1f(m_LocAlphaRef, g_AlphaRef);
        m_LastAlphaRef = g_AlphaRef;
    }
}
