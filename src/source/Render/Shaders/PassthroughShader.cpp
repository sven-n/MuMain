#include "stdafx.h"
#include "PassthroughShader.h"
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
#include <iostream>
#include <vector>

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
              fn_glUniform1f != nullptr);
    return loaded;
}

static const char* g_szPassthroughVert = R"(
#version 330 core

layout(std140) uniform GlobalMatrices {
    mat4 u_View;
    mat4 u_Proj;
    mat4 u_Model;
    mat4 u_MVP;
    vec4 u_Time;
};

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec2 a_UV;
layout(location = 2) in vec4 a_Color;

out vec2 v_UV;
out vec4 v_Color;

void main() {
    gl_Position = u_MVP * vec4(a_Pos, 1.0);
    v_UV    = a_UV;
    v_Color = a_Color;
}
)";

static const char* g_szPassthroughFrag = R"(
#version 330 core

uniform sampler2D u_Tex;
uniform int u_UseTexture;
uniform int u_UseFog;
// Fixed-function alpha test threshold (DXP-01), mirrored from glAlphaFunc/GL_ALPHA_TEST state.
// -1.0 means alpha test is disabled (no discard); otherwise the GL_GREATER reference value.
uniform float u_AlphaRef;
// FFP GL_TEXTURE_ENV_MODE port (DXP-08a): 0 = GL_MODULATE (default, texture * vertexColor),
// 1 = GL_ADD (texture.rgb + vertexColor.rgb, alpha stays multiplicative) -- matches the real
// GL_ADD texture-combine spec (Cv = Cf + Ct, Av = Af * At).
uniform int u_TexCombineAdd;

in vec2 v_UV;
in vec4 v_Color;

out vec4 fragColor;

layout(std140) uniform SceneData {
    vec4  u_FogColor;
    float u_FogStart;
    float u_FogEnd;
    float u_FogEnabled;
    float u_ScenePadding;
};

void main() {
    // Clamp to [0,1] to match FFP vertex color clamping (see BMDMeshShader's twin comment) --
    // callers occasionally feed deliberately-over-1.0 values for a "ramp to fully blown out"
    // effect (e.g. LoginScene.cpp's MU-logo intro, glColor4f up to 10.0) that FFP silently
    // clamped away; without this, the shader path lets it through raw, badly over-brightening
    // anything additive-blended (u_TexCombineAdd) far beyond the intended fade-to-white look.
    vec4 c = clamp(v_Color, 0.0, 1.0);
    vec4 texSample = (u_UseTexture != 0) ? texture(u_Tex, v_UV) : vec4(1.0);
    if (u_TexCombineAdd != 0) {
        fragColor = vec4(texSample.rgb + c.rgb, texSample.a * c.a);
    } else {
        fragColor = texSample * c;
    }

    // <= matches GL_GREATER exactly (fixed-function alpha test passes when a > ref).
    if (u_AlphaRef >= 0.0 && fragColor.a <= u_AlphaRef) discard;

    if (u_UseFog != 0 && u_FogEnabled > 0.5) {
        float depth = gl_FragCoord.z / gl_FragCoord.w;
        float fogFactor = clamp((u_FogEnd - depth) / (u_FogEnd - u_FogStart), 0.0, 1.0);
        fragColor.rgb = mix(u_FogColor.rgb, fragColor.rgb, fogFactor);
    }
}
)";

// DXP-14 -- HLSL port, kept semantically line-for-line with the GLSL above (per the task's
// "Rules" section) so the two are easy to diff by eye. Two structural differences forced by
// HLSL, both noted at the site: (1) the four loose GLSL uniforms become one PassFlags cbuffer
// (HLSL has no non-block uniform storage); (2) matrices are declared column_major explicitly
// and multiplied via mul(M, v), matching GLSL's M*v convention (see the task doc's "What could
// go wrong" section) instead of HLSL's other common mul(v, M) style.
static const char* g_szPassthroughVertHLSL = R"(
cbuffer GlobalMatrices : register(b0)
{
    column_major matrix u_View;
    column_major matrix u_Proj;
    column_major matrix u_Model;
    column_major matrix u_MVP;
    float4 u_Time;
};

struct VSInput
{
    float3 a_Pos   : POSITION;
    float2 a_UV    : TEXCOORD0;
    float4 a_Color : COLOR0;
};

struct VSOutput
{
    float4 pos   : SV_POSITION;
    float2 uv    : TEXCOORD0;
    float4 color : COLOR0;
};

VSOutput main(VSInput input)
{
    VSOutput o;
    o.pos   = mul(u_MVP, float4(input.a_Pos, 1.0));
    o.uv    = input.a_UV;
    o.color = input.a_Color;
    return o;
}
)";

static const char* g_szPassthroughFragHLSL = R"(
cbuffer SceneData : register(b1)
{
    float4 u_FogColor;
    float  u_FogStart;
    float  u_FogEnd;
    float  u_FogEnabled;
    float  u_ScenePadding;
};

// The GLSL fragment shader's four loose uniforms (u_UseTexture/u_UseFog/u_AlphaRef/
// u_TexCombineAdd) have no HLSL "non-block uniform" equivalent -- packed into one cbuffer
// instead (RHI slot 4: 0=GlobalMatrices, 1=SceneData, 2=BoneUBO, 3=DynamicLights reserved).
cbuffer PassFlags : register(b4)
{
    int   u_UseTexture;
    int   u_UseFog;
    float u_AlphaRef;
    int   u_TexCombineAdd;
};

Texture2D    u_Tex     : register(t0);
SamplerState u_Sampler : register(s0);

struct PSInput
{
    float4 pos   : SV_POSITION;
    float2 uv    : TEXCOORD0;
    float4 color : COLOR0;
};

float4 main(PSInput input) : SV_TARGET
{
    // See the GLSL twin's comment -- clamp to [0,1] to match FFP vertex color clamping.
    float4 c = saturate(input.color);
    float4 texSample = (u_UseTexture != 0) ? u_Tex.Sample(u_Sampler, input.uv) : float4(1.0, 1.0, 1.0, 1.0);

    float4 fragColor;
    if (u_TexCombineAdd != 0)
        fragColor = float4(texSample.rgb + c.rgb, texSample.a * c.a);
    else
        fragColor = texSample * c;

    // <= matches GL_GREATER exactly (fixed-function alpha test passes when a > ref).
    if (u_AlphaRef >= 0.0 && fragColor.a <= u_AlphaRef)
        discard;

    if (u_UseFog != 0 && u_FogEnabled > 0.5)
    {
        // input.pos (SV_POSITION) carries the same z/w semantics as GLSL's gl_FragCoord.
        float depth = input.pos.z / input.pos.w;
        float fogFactor = saturate((u_FogEnd - depth) / (u_FogEnd - u_FogStart));
        fragColor.rgb = lerp(u_FogColor.rgb, fragColor.rgb, fogFactor);
    }

    return fragColor;
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
                g_ErrorReport.Write(L"[PassthroughShader][D3D11] %hs compile error: %hs\r\n",
                    entryPoint, static_cast<const char*>(errorBlob->GetBufferPointer()));
                errorBlob->Release();
            }
            else
            {
                g_ErrorReport.Write(L"[PassthroughShader][D3D11] %hs compile failed, hr=0x%08lX\r\n",
                    entryPoint, static_cast<unsigned long>(hr));
            }
            return false;
        }
        if (errorBlob) errorBlob->Release(); // warnings only
        return true;
    }

    struct PassFlagsCB { int32_t useTexture; int32_t useFog; float alphaRef; int32_t texCombineAdd; };
}

PassthroughShader& PassthroughShader::Instance()
{
    static PassthroughShader instance;
    return instance;
}

PassthroughShader::~PassthroughShader()
{
    Destroy();
}

void PassthroughShader::Create()
{
    if (g_RenderBackend == RenderBackend::D3D11) { CreateD3D11(); return; }
    CreateGL();
}

void PassthroughShader::Destroy()
{
    if (g_RenderBackend == RenderBackend::D3D11) { DestroyD3D11(); return; }
    DestroyGL();
}

void PassthroughShader::Bind()
{
    if (g_RenderBackend == RenderBackend::D3D11) { BindD3D11(); return; }
    BindGL();
}

void PassthroughShader::Unbind()
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

void PassthroughShader::CreateGL()
{
    if (m_Program != 0) return;
    if (!LoadGLShaderFunctions()) return;

    // Compile Vertex Shader
    m_VertShader = fn_glCreateShader(GL_VERTEX_SHADER);
    fn_glShaderSource(m_VertShader, 1, &g_szPassthroughVert, nullptr);
    fn_glCompileShader(m_VertShader);

    GLint compiled = 0;
    fn_glGetShaderiv(m_VertShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char infoLog[512];
        if (fn_glGetShaderInfoLog) fn_glGetShaderInfoLog(m_VertShader, 512, nullptr, infoLog);
        SDL_Log("[PassthroughShader] Vertex compilation error: %s", infoLog);
        fn_glDeleteShader(m_VertShader);
        m_VertShader = 0;
        return;
    }

    // Compile Fragment Shader
    m_FragShader = fn_glCreateShader(GL_FRAGMENT_SHADER);
    fn_glShaderSource(m_FragShader, 1, &g_szPassthroughFrag, nullptr);
    fn_glCompileShader(m_FragShader);

    fn_glGetShaderiv(m_FragShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char infoLog[512];
        if (fn_glGetShaderInfoLog) fn_glGetShaderInfoLog(m_FragShader, 512, nullptr, infoLog);
        SDL_Log("[PassthroughShader] Fragment compilation error: %s", infoLog);
        fn_glDeleteShader(m_VertShader);
        fn_glDeleteShader(m_FragShader);
        m_VertShader = m_FragShader = 0;
        return;
    }

    // Link Program
    m_Program = fn_glCreateProgram();
    fn_glAttachShader(m_Program, m_VertShader);
    fn_glAttachShader(m_Program, m_FragShader);
    fn_glLinkProgram(m_Program);

    GLint linked = 0;
    fn_glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char infoLog[512];
        if (fn_glGetProgramInfoLog) fn_glGetProgramInfoLog(m_Program, 512, nullptr, infoLog);
        SDL_Log("[PassthroughShader] Program link error: %s", infoLog);
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

    // Bind SceneData uniform block to binding point 1
    if (fn_glGetUniformBlockIndex != nullptr && fn_glUniformBlockBinding != nullptr) {
        GLuint sceneBlockIdx = fn_glGetUniformBlockIndex(m_Program, "SceneData");
        if (sceneBlockIdx != GL_INVALID_INDEX) {
            fn_glUniformBlockBinding(m_Program, sceneBlockIdx, 1);
        }
    }

    // Resolve uniform locations
    m_LocTex        = fn_glGetUniformLocation(m_Program, "u_Tex");
    m_LocUseTexture = fn_glGetUniformLocation(m_Program, "u_UseTexture");
    m_LocUseFog     = fn_glGetUniformLocation(m_Program, "u_UseFog");
    m_LocAlphaRef   = fn_glGetUniformLocation(m_Program, "u_AlphaRef");
    m_LocTexCombineAdd = fn_glGetUniformLocation(m_Program, "u_TexCombineAdd");

    // Default sampler to texture unit 0, texture enabled, fog disabled, alpha test disabled for 2D UI
    BindProgram(m_Program);
    if (m_LocTex != -1) fn_glUniform1i(m_LocTex, 0);
    if (m_LocUseTexture != -1) fn_glUniform1i(m_LocUseTexture, 1);
    if (m_LocUseFog != -1) fn_glUniform1i(m_LocUseFog, 0);
    if (m_LocAlphaRef != -1) fn_glUniform1f(m_LocAlphaRef, -1.0f);
    m_LastAlphaRef = -1.0f;
    if (m_LocTexCombineAdd != -1) fn_glUniform1i(m_LocTexCombineAdd, 0);
    // Tri-state sentinels reset to "unset" (not matched to the values just uploaded above) so the
    // first SetUseTexture/SetUseFog/SetTexCombineAdd call after (re)creation always re-uploads,
    // regardless of what the caller happens to request first.
    m_LastUseTexture = -1;
    m_LastUseFog = -1;
    m_LastTexCombineAdd = -1;
    BindProgram(0);

    g_ErrorReport.Write(L"[PassthroughShader] Created program ID %d, u_Tex loc %d, u_UseTexture loc %d\r\n", m_Program, m_LocTex, m_LocUseTexture);
}

void PassthroughShader::DestroyGL()
{
    if (m_Program != 0) {
        BindProgram(0);
        if (m_VertShader) fn_glDeleteShader(m_VertShader);
        if (m_FragShader) fn_glDeleteShader(m_FragShader);
        if (fn_glDeleteProgram) { fn_glDeleteProgram(m_Program); InvalidateProgramCache(); }
        m_Program = m_VertShader = m_FragShader = 0;
    }
}

void PassthroughShader::BindGL()
{
    if (m_Program != 0) {
        BindProgram(m_Program);

        // Every IR::Begin() calls Bind(), so this is per-draw granularity for free. Dirty-checked
        // so unchanged alpha-test state (the common case) costs one float compare, not a glUniform1f.
        if (m_LocAlphaRef != -1 && fn_glUniform1f != nullptr && g_AlphaRef != m_LastAlphaRef) {
            fn_glUniform1f(m_LocAlphaRef, g_AlphaRef);
            m_LastAlphaRef = g_AlphaRef;
        }
    }
}

#ifdef RHI_D3D11_AVAILABLE
void PassthroughShader::CreateD3D11()
{
    if (m_D3DVertexShader) return;
    ID3D11Device* device = static_cast<ID3D11Device*>(RHI::GetD3D11Device());
    if (!device) return;

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    if (!CompileHLSL(g_szPassthroughVertHLSL, "main", "vs_5_0", &vsBlob)) return;
    if (!CompileHLSL(g_szPassthroughFragHLSL, "main", "ps_5_0", &psBlob)) { vsBlob->Release(); return; }

    ID3D11VertexShader* vs = nullptr;
    if (FAILED(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vs)))
    {
        g_ErrorReport.Write(L"[PassthroughShader][D3D11] CreateVertexShader failed\r\n");
        vsBlob->Release(); psBlob->Release();
        return;
    }
    ID3D11PixelShader* ps = nullptr;
    if (FAILED(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &ps)))
    {
        g_ErrorReport.Write(L"[PassthroughShader][D3D11] CreatePixelShader failed\r\n");
        vs->Release(); vsBlob->Release(); psBlob->Release();
        return;
    }

    // Registers this shader's bytecode as the PosUvColor layout's signature source -- RHI_D3D11
    // needs real VS bytecode to build the matching ID3D11InputLayout (D3D11 requirement, no GL
    // equivalent). PassthroughShader is PosUvColor's only producer today.
    RHI::RegisterVertexShaderBytecode(RHI::VertexLayout::PosUvColor, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize());
    vsBlob->Release();
    psBlob->Release();

    m_D3DVertexShader = vs;
    m_D3DPixelShader = ps;

    // DXP-16 increment 3: no longer creates its own sampler here -- SetTexture()'s
    // RHI::BindTexture() call binds each texture's own recorded filter/wrap sampler at bind time
    // (RHI_D3D11.cpp's GetOrCreateSampler). See TerrainShader.cpp's CreateD3D11 comment for the
    // bug a fixed shared-per-shader sampler caused elsewhere.

    m_D3DFlagsCBuffer = RHI::CreateUniformBlock(sizeof(PassFlagsCB), 4);
    m_D3DUseTexture = 1;
    m_D3DUseFog = 0;
    m_D3DTexCombineAdd = 0;
    m_LastAlphaRef = -1.0f;
    UploadD3D11Flags();
    // Tri-state sentinels reset to "unset" for parity with the GL path (see CreateGL's
    // comment) -- the D3D11 branch itself doesn't consult them, m_D3DUseTexture/m_D3DUseFog/
    // m_D3DTexCombineAdd are the real current values it packs into the cbuffer.
    m_LastUseTexture = -1;
    m_LastUseFog = -1;
    m_LastTexCombineAdd = -1;

    g_ErrorReport.Write(L"[PassthroughShader][D3D11] Compiled + linked, PassFlags cbuffer at slot 4\r\n");
}

void PassthroughShader::DestroyD3D11()
{
    if (m_D3DFlagsCBuffer.IsValid()) { RHI::DestroyUniformBlock(m_D3DFlagsCBuffer); m_D3DFlagsCBuffer = {}; }
    if (m_D3DPixelShader)  { static_cast<ID3D11PixelShader*>(m_D3DPixelShader)->Release();   m_D3DPixelShader = nullptr; }
    if (m_D3DVertexShader) { static_cast<ID3D11VertexShader*>(m_D3DVertexShader)->Release(); m_D3DVertexShader = nullptr; }
}

void PassthroughShader::BindD3D11()
{
    ID3D11DeviceContext* ctx = static_cast<ID3D11DeviceContext*>(RHI::GetD3D11DeviceContext());
    if (!ctx || !m_D3DVertexShader || !m_D3DPixelShader) return;

    ctx->VSSetShader(static_cast<ID3D11VertexShader*>(m_D3DVertexShader), nullptr, 0);
    ctx->PSSetShader(static_cast<ID3D11PixelShader*>(m_D3DPixelShader), nullptr, 0);

    // Mirrors BindGL()'s per-draw alpha-ref dirty check (IR::Begin() calls Bind() every draw).
    if (g_AlphaRef != m_LastAlphaRef)
    {
        m_LastAlphaRef = g_AlphaRef;
        UploadD3D11Flags();
    }
}

void PassthroughShader::UploadD3D11Flags()
{
    if (!m_D3DFlagsCBuffer.IsValid()) return;
    PassFlagsCB cb;
    cb.useTexture    = m_D3DUseTexture;
    cb.useFog        = m_D3DUseFog;
    cb.alphaRef      = m_LastAlphaRef;
    cb.texCombineAdd = m_D3DTexCombineAdd;
    RHI::UpdateUniformBlock(m_D3DFlagsCBuffer, &cb, sizeof(cb));
}
#else // !RHI_D3D11_AVAILABLE -- keep these symbols linkable as no-ops since Create()/Destroy()/
      // Bind() call them behind a runtime g_RenderBackend check, which itself compiles on every
      // platform (see RHI_D3D11.cpp's own #else for the same rationale).
void PassthroughShader::CreateD3D11() {}
void PassthroughShader::DestroyD3D11() {}
void PassthroughShader::BindD3D11() {}
void PassthroughShader::UploadD3D11Flags() {}
#endif // RHI_D3D11_AVAILABLE

extern int CachTexture;

void PassthroughShader::SetTexture(GLuint texID, int slot)
{
    if (g_RenderBackend == RenderBackend::D3D11)
    {
        // texID is the same RHI::TextureHandle::id both backends already share -- DXP-12
        // migrated texture creation (GlobalBitmap.cpp/ZzzInventory.cpp/UIControls.cpp) behind
        // RHI::CreateTexture on both backends, and BITMAP_t::TextureNumber just stores
        // handle.id regardless of which backend produced it. No GLuint<->TextureHandle bridge
        // needed.
        RHI::BindTexture(RHI::TextureHandle{ static_cast<uint32_t>(texID) }, slot);
        // DXP-16 fix: mirrors TerrainShader::SetBaseTexture's D3D11-branch fix -- this direct
        // RHI::BindTexture bypasses the CachTexture cache the UI's BindTexture(int) wrapper trusts
        // for slot 0, so it must be invalidated here too (the GL branch below already does it).
        CachTexture = -1;
        return;
    }

    BindTexture2D(slot, texID);
    CachTexture = -1; // Invalidate engine texture cache
    if (slot != 0 && fn_glActiveTexture != nullptr) {
        fn_glActiveTexture(GL_TEXTURE0);
    }
    if (m_LocTex != -1 && fn_glUniform1i != nullptr) {
        fn_glUniform1i(m_LocTex, slot);
    }
}

void PassthroughShader::SetUseTexture(bool use)
{
    Bind();
    const int v = use ? 1 : 0;
    if (g_RenderBackend == RenderBackend::D3D11)
    {
        m_D3DUseTexture = v;
        UploadD3D11Flags();
        return;
    }
    if (m_LocUseTexture != -1 && fn_glUniform1i != nullptr && v != m_LastUseTexture) {
        fn_glUniform1i(m_LocUseTexture, v);
        m_LastUseTexture = v;
    }
}

void PassthroughShader::SetUseFog(bool use)
{
    Bind();
    const int v = use ? 1 : 0;
    if (g_RenderBackend == RenderBackend::D3D11)
    {
        m_D3DUseFog = v;
        UploadD3D11Flags();
        return;
    }
    if (m_LocUseFog != -1 && fn_glUniform1i != nullptr && v != m_LastUseFog) {
        fn_glUniform1i(m_LocUseFog, v);
        m_LastUseFog = v;
    }
}

void PassthroughShader::SetTexCombineAdd(bool add)
{
    Bind();
    const int v = add ? 1 : 0;
    if (g_RenderBackend == RenderBackend::D3D11)
    {
        m_D3DTexCombineAdd = v;
        UploadD3D11Flags();
        return;
    }
    if (m_LocTexCombineAdd != -1 && fn_glUniform1i != nullptr && v != m_LastTexCombineAdd) {
        fn_glUniform1i(m_LocTexCombineAdd, v);
        m_LastTexCombineAdd = v;
    }
}
