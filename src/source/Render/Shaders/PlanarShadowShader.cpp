#include "stdafx.h"
#include "Render/Shaders/PlanarShadowShader.h"
#include "Render/Core/BindState.h"
#include "Render/Core/RenderConfig.h"
#include "Core/Utilities/FrameProfiler.h"
#include "Core/Utilities/Log/ErrorReport.h"
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
typedef GLuint (APIENTRY* PFNGLCREATEPROGRAMPROC)(void);
typedef void   (APIENTRY* PFNGLATTACHSHADERPROC)(GLuint, GLuint);
typedef void   (APIENTRY* PFNGLLINKPROGRAMPROC)(GLuint);
typedef void   (APIENTRY* PFNGLGETPROGRAMIVPROC)(GLuint, GLenum, GLint*);
typedef void   (APIENTRY* PFNGLDELETESHADERPROC)(GLuint);
typedef void   (APIENTRY* PFNGLDELETEPROGRAMPROC)(GLuint);
typedef GLint  (APIENTRY* PFNGLGETUNIFORMLOCATIONPROC)(GLuint, const GLchar*);
typedef GLuint (APIENTRY* PFNGLGETUNIFORMBLOCKINDEXPROC)(GLuint, const GLchar*);
typedef void   (APIENTRY* PFNGLUNIFORMBLOCKBINDINGPROC)(GLuint, GLuint, GLuint);
typedef void   (APIENTRY* PFNGLUNIFORM1IPROC)(GLint, GLint);
typedef void   (APIENTRY* PFNGLUNIFORM1FPROC)(GLint, GLfloat);
typedef void   (APIENTRY* PFNGLUNIFORM3FVPROC)(GLint, GLsizei, const GLfloat*);
typedef void   (APIENTRY* PFNGLUNIFORMMATRIX4FVPROC)(GLint, GLsizei, GLboolean, const GLfloat*);
typedef void   (APIENTRY* PFNGLGENVERTEXARRAYSPROC)(GLsizei, GLuint*);
typedef void   (APIENTRY* PFNGLDELETEVERTEXARRAYSPROC)(GLsizei, const GLuint*);
typedef void   (APIENTRY* PFNGLVERTEXATTRIBPOINTERPROC)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
typedef void   (APIENTRY* PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint);
typedef void   (APIENTRY* PFNGLGENBUFFERSPROC)(GLsizei, GLuint*);
typedef void   (APIENTRY* PFNGLBINDBUFFERPROC)(GLenum, GLuint);
typedef void   (APIENTRY* PFNGLBUFFERDATAPROC)(GLenum, GLsizeiptr, const void*, GLenum);
typedef void   (APIENTRY* PFNGLBUFFERSUBDATAPROC)(GLenum, GLintptr, GLsizeiptr, const void*);
typedef void   (APIENTRY* PFNGLDELETEBUFFERSPROC)(GLsizei, const GLuint*);

static PFNGLCREATESHADERPROC       fn_glCreateShader       = nullptr;
static PFNGLSHADERSOURCEPROC       fn_glShaderSource        = nullptr;
static PFNGLCOMPILESHADERPROC      fn_glCompileShader       = nullptr;
static PFNGLGETSHADERIVPROC        fn_glGetShaderiv         = nullptr;
static PFNGLCREATEPROGRAMPROC      fn_glCreateProgram       = nullptr;
static PFNGLATTACHSHADERPROC       fn_glAttachShader        = nullptr;
static PFNGLLINKPROGRAMPROC        fn_glLinkProgram         = nullptr;
static PFNGLGETPROGRAMIVPROC       fn_glGetProgramiv        = nullptr;
static PFNGLDELETESHADERPROC       fn_glDeleteShader        = nullptr;
static PFNGLDELETEPROGRAMPROC      fn_glDeleteProgram       = nullptr;
static PFNGLGETUNIFORMLOCATIONPROC fn_glGetUniformLocation  = nullptr;
static PFNGLGETUNIFORMBLOCKINDEXPROC fn_glGetUniformBlockIndex = nullptr;
static PFNGLUNIFORMBLOCKBINDINGPROC  fn_glUniformBlockBinding  = nullptr;
static PFNGLUNIFORM1IPROC          fn_glUniform1i           = nullptr;
static PFNGLUNIFORM1FPROC          fn_glUniform1f           = nullptr;
static PFNGLUNIFORM3FVPROC         fn_glUniform3fv          = nullptr;
static PFNGLUNIFORMMATRIX4FVPROC   fn_glUniformMatrix4fv    = nullptr;
static PFNGLGENVERTEXARRAYSPROC          fn_glGenVertexArrays          = nullptr;
static PFNGLDELETEVERTEXARRAYSPROC       fn_glDeleteVertexArrays       = nullptr;
static PFNGLVERTEXATTRIBPOINTERPROC      fn_glVertexAttribPointer      = nullptr;
static PFNGLENABLEVERTEXATTRIBARRAYPROC  fn_glEnableVertexAttribArray  = nullptr;
static PFNGLGENBUFFERSPROC               fn_glGenBuffers               = nullptr;
static PFNGLBINDBUFFERPROC               fn_glBindBuffer               = nullptr;
static PFNGLBUFFERDATAPROC               fn_glBufferData               = nullptr;
static PFNGLBUFFERSUBDATAPROC            fn_glBufferSubData            = nullptr;
static PFNGLDELETEBUFFERSPROC            fn_glDeleteBuffers            = nullptr;

// ===========================================================================
// GLSL Planar Shadow Vertex Shader (DXP-06: 330 core, explicit attribute + u_MVP,
// no FFP matrix stack / gl_Vertex dependency)
// Exact MU Online shadow perspective projection:
//   rel = v.xyz - u_BodyOrigin;
//   projX_rel = rel.x + (rel.z * (rel.x + u_Sx) / (rel.z - u_Sy));
// ===========================================================================
static const char* g_szPlanarVert = R"(
#version 330 core
layout(location = 0) in vec3 a_Pos;
layout(location = 4) in int  a_BoneIndex; // DXP-20: only used when u_UseGPUSkin==1; matches
                                           // BMD::m_VAO_StaticGPU's attribute layout (BMDMeshShader.cpp)

// Bone matrix palette UBO at binding slot 2 — same binding BMDMeshShader/BoneUBO use, so
// BoneUBO::UploadBones() (called once per body by the CPU caller) feeds both shaders.
// GLP-11: 3x vec4 affine rows per bone, not mat4 -- must match BMDMeshShader.cpp's declaration
// exactly (a std140 mismatch across programs sharing one binding point is undefined behavior,
// not a compile error).
layout(std140) uniform BoneMatrices {
    vec4 u_Bones[600]; // 3 rows per bone, 200 bones
};

uniform mat4  u_MVP;
uniform vec3  u_BodyOrigin;  // shadow skew pivot -- always the body's actual BodyOrigin (CalcShadowPosition parity)
uniform float u_Sx;
uniform float u_Sy;
uniform int   u_UseGPUSkin;  // 1 = skin a_Pos via u_Bones[a_BoneIndex] before the skew, 0 = a_Pos is already world-space
uniform vec3  u_SkinOrigin;  // DXP-20: mirrors RenderMesh's gpuBodyOrigin (zero when bones already encode world pos)
uniform float u_SkinScale;   // DXP-20: mirrors RenderMesh's gpuBodyScale

void main()
{
    vec3 worldPos;
    if (u_UseGPUSkin == 1 && a_BoneIndex >= 0 && a_BoneIndex < 200) {
        // Ported 1:1 from BMDMeshShader's GPU skinning branch, which itself mirrors
        // BMD::Transform's VectorMA(BodyOrigin, BodyScale, bonePos, VertexTransform).
        // GLP-11: dot-product reconstruction from the 3 affine rows -- see BMDMeshShader.cpp's
        // equivalent site for the full row/column transpose reasoning.
        vec4 r0 = u_Bones[a_BoneIndex * 3 + 0];
        vec4 r1 = u_Bones[a_BoneIndex * 3 + 1];
        vec4 r2 = u_Bones[a_BoneIndex * 3 + 2];
        vec4 p = vec4(a_Pos, 1.0);
        vec3 bonePos = vec3(dot(r0, p), dot(r1, p), dot(r2, p));
        worldPos = u_SkinOrigin + u_SkinScale * bonePos;
    } else {
        // CPU path: a_Pos is already a fully world-space VertexTransform position.
        worldPos = a_Pos;
    }

    vec3 rel = worldPos - u_BodyOrigin;

    float denom = rel.z - u_Sy;
    float skewX = (abs(denom) > 0.001) ? (rel.z * (rel.x + u_Sx) / denom) : 0.0;

    vec4 projPos;
    projPos.x = u_BodyOrigin.x + rel.x + skewX;
    projPos.y = u_BodyOrigin.y + rel.y;
    projPos.z = u_BodyOrigin.z + 5.0;
    projPos.w = 1.0;

    gl_Position = u_MVP * projPos;
}
)";

// ===========================================================================
// GLSL Planar Shadow Fragment Shader (DXP-06: 330 core, explicit fragColor)
// ===========================================================================
static const char* g_szPlanarFrag = R"(
#version 330 core
uniform float u_ShadowAlpha;
out vec4 fragColor;

void main()
{
    fragColor = vec4(0.0, 0.0, 0.0, u_ShadowAlpha);
}
)";

// DXP-14 increment 4 -- HLSL port, same column_major + mul(M,v) convention as the other 3
// shaders. Only the CPU-path (a_Pos already world-space) is ported -- see this file's header
// comment for why the GLSL's a_BoneIndex/bone-skinning branch isn't part of the D3D11 twin yet.
static const char* g_szPlanarVertHLSL = R"(
// The GLSL shader's loose uniforms have no HLSL non-block-uniform equivalent -- packed into one
// cbuffer instead (RHI slot 7: 0=GlobalMatrices, 1=SceneData, 2=BoneUBO, 3=DynamicLights
// reserved, 4=Passthrough's PassFlags, 5=Terrain's TerrainFlags, 6=BMDMesh's BMDFlags).
// u_UseGPUSkin/u_SkinOrigin/u_SkinScale are carried for cbuffer-layout parity with the GLSL twin
// (and so a future DrawGPUSkinned D3D11 path can reuse this same cbuffer) but always read as
// 0/unused this increment -- DrawGPUSkinned stays GL-only, see the .cpp's D3D11 Draw().
cbuffer ShadowFlags : register(b7)
{
    column_major matrix u_MVP;
    float3 u_BodyOrigin;
    float  u_Sx;
    float  u_Sy;
    float  u_ShadowAlpha;
    int    u_UseGPUSkin;
    float  _shadowPad0;
    float3 u_SkinOrigin;
    float  u_SkinScale;
};

struct VSInput
{
    float3 a_Pos : POSITION;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
};

VSOutput main(VSInput input)
{
    VSOutput o;

    // D3D11 CPU-path only this increment: a_Pos is already a fully world-space
    // VertexTransform position (see the GLSL twin's else branch).
    float3 worldPos = input.a_Pos;

    float3 rel = worldPos - u_BodyOrigin;

    float denom = rel.z - u_Sy;
    float skewX = (abs(denom) > 0.001) ? (rel.z * (rel.x + u_Sx) / denom) : 0.0;

    float4 projPos;
    projPos.x = u_BodyOrigin.x + rel.x + skewX;
    projPos.y = u_BodyOrigin.y + rel.y;
    projPos.z = u_BodyOrigin.z + 5.0;
    projPos.w = 1.0;

    o.pos = mul(u_MVP, projPos);
    return o;
}
)";

// DXP-21 part 2: GPU-skinned twin of g_szPlanarVertHLSL, a separate VS entry point rather than a
// branch inside the CPU-path one (see PlanarShadowShader.h class comment for why: this VS's
// input signature declares a_BoneIndex, which has no matching element in the CPU path's
// PosOnly-layout descriptor array). Bound with RHI::VertexLayout::BMDMesh -- BMDMeshShader is
// that layout's bytecode producer, this VS's inputs are a strict subset of what BMDMesh already
// provides, so no RegisterVertexShaderBytecode call is needed here. Reads the same slot-7
// ShadowFlags cbuffer and slot-2 BoneMatrices cbuffer as the CPU-path VS / BMDMeshShader.
static const char* g_szPlanarVertGPUSkinHLSL = R"(
cbuffer ShadowFlags : register(b7)
{
    column_major matrix u_MVP;
    float3 u_BodyOrigin;
    float  u_Sx;
    float  u_Sy;
    float  u_ShadowAlpha;
    int    u_UseGPUSkin;
    float  _shadowPad0;
    float3 u_SkinOrigin;
    float  u_SkinScale;
};

// GLP-11: 3x float4 affine rows per bone, not a full matrix -- matches BMDMeshShader's HLSL
// twin and BoneUBO's actual upload layout (both shaders read the same slot-2 cbuffer).
cbuffer BoneMatrices : register(b2)
{
    float4 u_Bones[600]; // 3 rows per bone, 200 bones
};

// DXP-21 part 2 fix (2026-08-05): must declare the SAME fields in the SAME order as
// BMDMeshShader's VSInput (POSITION, TEXCOORD0, COLOR0, NORMAL, BLENDINDICES0), even though this
// VS only reads a_Pos/a_BoneIndex -- CreateInputLayout (RHI_D3D11.cpp's EnsureInputLayout) builds
// VertexLayout::BMDMesh's ID3D11InputLayout from BMDMeshShader's bytecode, which locks in ITS
// compiler-assigned hardware input registers per semantic (POSITION->v0, TEXCOORD0->v1,
// COLOR0->v2, NORMAL->v3, BLENDINDICES0->v4). A VS declaring only a subset in a different order
// gets its OWN independent register assignment from the HLSL compiler (here: POSITION->v0,
// BLENDINDICES0->v1) -- IASetInputLayout doesn't re-validate against whichever VS is bound, so at
// Draw() time the layout keeps feeding BLENDINDICES data into v4 while this shader reads v1,
// silently pulling garbage. Root cause of both the original "shadow to infinity" corruption and
// the heap-corruption crash this task hit previously (confirmed via the D3D11 debug layer:
// "Semantic 'BLENDINDICES' is defined for mismatched hardware registers between the output stage
// and input stage" -- a graceful, well-defined validation error on real hardware, not something
// that should have crashed at all with the debug layer off; the unused fields below cost nothing
// at runtime, they just need to exist so the compiler's register numbering lines up).
struct VSInput
{
    float3 a_Pos       : POSITION;
    float2 a_UV        : TEXCOORD0;   // unused, present only to match BMDMeshShader's register layout
    float4 a_Color     : COLOR0;      // unused, present only to match BMDMeshShader's register layout
    float3 a_Normal    : NORMAL;      // unused, present only to match BMDMeshShader's register layout
    int    a_BoneIndex : BLENDINDICES0;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
};

VSOutput main(VSInput input)
{
    VSOutput o;

    // Ported 1:1 from the GLSL twin's u_UseGPUSkin branch -- mirrors BMDMeshShader's own
    // GPU-skin math. GLP-11 row layout: u_Bones[a_BoneIndex*3 + i], reconstructed via dot
    // products (r0/r1/r2 are rows, not columns -- see BMDMeshShader.cpp's HLSL twin for the
    // full derivation this is copied from).
    float3 worldPos;
    if (input.a_BoneIndex >= 0 && input.a_BoneIndex < 200)
    {
        float4 r0 = u_Bones[input.a_BoneIndex * 3 + 0];
        float4 r1 = u_Bones[input.a_BoneIndex * 3 + 1];
        float4 r2 = u_Bones[input.a_BoneIndex * 3 + 2];
        float4 p = float4(input.a_Pos, 1.0);
        float3 bonePos = float3(dot(r0, p), dot(r1, p), dot(r2, p));
        worldPos = u_SkinOrigin + u_SkinScale * bonePos;
    }
    else
    {
        worldPos = input.a_Pos;
    }

    float3 rel = worldPos - u_BodyOrigin;

    float denom = rel.z - u_Sy;
    float skewX = (abs(denom) > 0.001) ? (rel.z * (rel.x + u_Sx) / denom) : 0.0;

    float4 projPos;
    projPos.x = u_BodyOrigin.x + rel.x + skewX;
    projPos.y = u_BodyOrigin.y + rel.y;
    projPos.z = u_BodyOrigin.z + 5.0;
    projPos.w = 1.0;

    o.pos = mul(u_MVP, projPos);
    return o;
}
)";

static const char* g_szPlanarFragHLSL = R"(
cbuffer ShadowFlags : register(b7)
{
    column_major matrix u_MVP;
    float3 u_BodyOrigin;
    float  u_Sx;
    float  u_Sy;
    float  u_ShadowAlpha;
    int    u_UseGPUSkin;
    float  _shadowPad0;
    float3 u_SkinOrigin;
    float  u_SkinScale;
};

float4 main() : SV_TARGET
{
    return float4(0.0, 0.0, 0.0, u_ShadowAlpha);
}
)";

#ifdef RHI_D3D11_AVAILABLE
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
                g_ErrorReport.Write(L"[PlanarShadowShader][D3D11] %hs compile error: %hs\r\n",
                    entryPoint, static_cast<const char*>(errorBlob->GetBufferPointer()));
                errorBlob->Release();
            }
            else
            {
                g_ErrorReport.Write(L"[PlanarShadowShader][D3D11] %hs compile failed, hr=0x%08lX\r\n",
                    entryPoint, static_cast<unsigned long>(hr));
            }
            return false;
        }
        if (errorBlob) errorBlob->Release(); // warnings only
        return true;
    }

    // Field order matches the ShadowFlags cbuffer above byte-for-byte (same hand-laid-out
    // 16-byte-row convention as BMDFlagsCB in BMDMeshShader.cpp).
    struct ShadowFlagsCB
    {
        float   mvp[16];
        float   bodyOrigin[3];
        float   sx;
        float   sy;
        float   shadowAlpha;
        int32_t useGPUSkin;
        float   _pad0;
        float   skinOrigin[3];
        float   skinScale;
    };
    static_assert(sizeof(ShadowFlagsCB) == 112, "ShadowFlagsCB must match the HLSL ShadowFlags cbuffer layout byte-for-byte");
}
#endif // RHI_D3D11_AVAILABLE -- ID3DBlob/D3DCompile aren't declared without <d3dcompiler.h>
       // (guarded above with the same macro); this block would fail to compile on non-Windows
       // otherwise. (Upstream's own retrofit missed this spot -- fixed here instead of copied.)

CPlanarShadowShader& CPlanarShadowShader::Instance()
{
    static CPlanarShadowShader s_instance;
    return s_instance;
}

CPlanarShadowShader::CPlanarShadowShader()  {}
CPlanarShadowShader::~CPlanarShadowShader() { Shutdown(); }

bool CPlanarShadowShader::LoadGLFunctions()
{
    fn_glCreateShader       = (PFNGLCREATESHADERPROC)      SDL_GL_GetProcAddress("glCreateShader");
    fn_glShaderSource       = (PFNGLSHADERSOURCEPROC)      SDL_GL_GetProcAddress("glShaderSource");
    fn_glCompileShader      = (PFNGLCOMPILESHADERPROC)     SDL_GL_GetProcAddress("glCompileShader");
    fn_glGetShaderiv        = (PFNGLGETSHADERIVPROC)       SDL_GL_GetProcAddress("glGetShaderiv");
    fn_glCreateProgram      = (PFNGLCREATEPROGRAMPROC)     SDL_GL_GetProcAddress("glCreateProgram");
    fn_glAttachShader       = (PFNGLATTACHSHADERPROC)      SDL_GL_GetProcAddress("glAttachShader");
    fn_glLinkProgram        = (PFNGLLINKPROGRAMPROC)       SDL_GL_GetProcAddress("glLinkProgram");
    fn_glGetProgramiv       = (PFNGLGETPROGRAMIVPROC)      SDL_GL_GetProcAddress("glGetProgramiv");
    fn_glDeleteShader       = (PFNGLDELETESHADERPROC)      SDL_GL_GetProcAddress("glDeleteShader");
    fn_glDeleteProgram      = (PFNGLDELETEPROGRAMPROC)     SDL_GL_GetProcAddress("glDeleteProgram");
    fn_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)SDL_GL_GetProcAddress("glGetUniformLocation");
    fn_glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)SDL_GL_GetProcAddress("glGetUniformBlockIndex");
    fn_glUniformBlockBinding  = (PFNGLUNIFORMBLOCKBINDINGPROC) SDL_GL_GetProcAddress("glUniformBlockBinding");
    fn_glUniform1i          = (PFNGLUNIFORM1IPROC)         SDL_GL_GetProcAddress("glUniform1i");
    fn_glUniform1f          = (PFNGLUNIFORM1FPROC)         SDL_GL_GetProcAddress("glUniform1f");
    fn_glUniform3fv         = (PFNGLUNIFORM3FVPROC)        SDL_GL_GetProcAddress("glUniform3fv");
    fn_glUniformMatrix4fv   = (PFNGLUNIFORMMATRIX4FVPROC)  SDL_GL_GetProcAddress("glUniformMatrix4fv");
    fn_glGenVertexArrays          = (PFNGLGENVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glGenVertexArrays");
    fn_glDeleteVertexArrays       = (PFNGLDELETEVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glDeleteVertexArrays");
    fn_glVertexAttribPointer      = (PFNGLVERTEXATTRIBPOINTERPROC)SDL_GL_GetProcAddress("glVertexAttribPointer");
    fn_glEnableVertexAttribArray  = (PFNGLENABLEVERTEXATTRIBARRAYPROC)SDL_GL_GetProcAddress("glEnableVertexAttribArray");
    fn_glGenBuffers               = (PFNGLGENBUFFERSPROC)SDL_GL_GetProcAddress("glGenBuffers");
    fn_glBindBuffer               = (PFNGLBINDBUFFERPROC)SDL_GL_GetProcAddress("glBindBuffer");
    fn_glBufferData               = (PFNGLBUFFERDATAPROC)SDL_GL_GetProcAddress("glBufferData");
    fn_glBufferSubData            = (PFNGLBUFFERSUBDATAPROC)SDL_GL_GetProcAddress("glBufferSubData");
    fn_glDeleteBuffers            = (PFNGLDELETEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteBuffers");

    return fn_glCreateShader && fn_glShaderSource && fn_glCompileShader && fn_glGetShaderiv &&
           fn_glCreateProgram && fn_glAttachShader && fn_glLinkProgram && fn_glGetProgramiv &&
           fn_glGetUniformLocation && fn_glGetUniformBlockIndex &&
           fn_glUniformBlockBinding && fn_glUniform1i && fn_glUniform1f && fn_glUniform3fv &&
           fn_glUniformMatrix4fv && fn_glGenVertexArrays &&
           fn_glDeleteVertexArrays && fn_glVertexAttribPointer && fn_glEnableVertexAttribArray &&
           fn_glGenBuffers && fn_glBindBuffer && fn_glBufferData && fn_glBufferSubData &&
           fn_glDeleteBuffers;
}

bool CPlanarShadowShader::CompileShaders()
{
    GLint ok = 0;

    m_hVertexShader = fn_glCreateShader(GL_VERTEX_SHADER);
    fn_glShaderSource(m_hVertexShader, 1, &g_szPlanarVert, nullptr);
    fn_glCompileShader(m_hVertexShader);
    fn_glGetShaderiv(m_hVertexShader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        // GLP-11: this class previously failed silently here -- every other shader class
        // (BMDMeshShader/TerrainShader/PassthroughShader) logs a compile failure via SDL_Log;
        // this one had neither that nor a success log, making "did the shader actually
        // compile" unverifiable short of a GPU debugger. Matches this task's own Verification
        // requirement, not just a GLP-11-specific concern.
        g_ErrorReport.Write(L"[PlanarShadowShader] Vertex shader compilation failed\r\n");
        return false;
    }

    m_hFragmentShader = fn_glCreateShader(GL_FRAGMENT_SHADER);
    fn_glShaderSource(m_hFragmentShader, 1, &g_szPlanarFrag, nullptr);
    fn_glCompileShader(m_hFragmentShader);
    fn_glGetShaderiv(m_hFragmentShader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        g_ErrorReport.Write(L"[PlanarShadowShader] Fragment shader compilation failed\r\n");
        return false;
    }

    m_hProgram = fn_glCreateProgram();
    fn_glAttachShader(m_hProgram, m_hVertexShader);
    fn_glAttachShader(m_hProgram, m_hFragmentShader);
    fn_glLinkProgram(m_hProgram);
    fn_glGetProgramiv(m_hProgram, GL_LINK_STATUS, &ok);
    if (!ok) {
        g_ErrorReport.Write(L"[PlanarShadowShader] Program link failed\r\n");
        return false;
    }

    m_locBodyOrigin  = fn_glGetUniformLocation(m_hProgram, "u_BodyOrigin");
    m_locSx          = fn_glGetUniformLocation(m_hProgram, "u_Sx");
    m_locSy          = fn_glGetUniformLocation(m_hProgram, "u_Sy");
    m_locShadowAlpha = fn_glGetUniformLocation(m_hProgram, "u_ShadowAlpha");
    m_locMVP         = fn_glGetUniformLocation(m_hProgram, "u_MVP");
    m_locUseGPUSkin  = fn_glGetUniformLocation(m_hProgram, "u_UseGPUSkin");
    m_locSkinOrigin  = fn_glGetUniformLocation(m_hProgram, "u_SkinOrigin");
    m_locSkinScale   = fn_glGetUniformLocation(m_hProgram, "u_SkinScale");

    // Bind BoneMatrices to binding point 2 -- same slot BMDMeshShader/BoneUBO use, so the caller's
    // one BoneUBO::UploadBones() call per body feeds this program too.
    if (fn_glGetUniformBlockIndex != nullptr && fn_glUniformBlockBinding != nullptr) {
        GLuint boneBlockIdx = fn_glGetUniformBlockIndex(m_hProgram, "BoneMatrices");
        if (boneBlockIdx != GL_INVALID_INDEX) {
            fn_glUniformBlockBinding(m_hProgram, boneBlockIdx, 2);
        }
    }

    g_ErrorReport.Write(L"[PlanarShadowShader] Created program ID %d\r\n", m_hProgram);
    return true;
}

void CPlanarShadowShader::CreateBuffers()
{
    if (m_VAO != 0) return;

    fn_glGenVertexArrays(1, &m_VAO);
    BindVAO(m_VAO);

    fn_glGenBuffers(1, &m_VBO);
    fn_glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    // Sized for a reasonably busy shadow batch; Draw() grows it on demand.
    m_VBOCapacity = 4096 * 3 * sizeof(float);
    fn_glBufferData(GL_ARRAY_BUFFER, m_VBOCapacity, nullptr, GL_STREAM_DRAW);

    fn_glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    fn_glEnableVertexAttribArray(0);

    fn_glBindBuffer(GL_ARRAY_BUFFER, 0);
    BindVAO(0);
}

void CPlanarShadowShader::DestroyBuffers()
{
    if (m_VBO != 0 && fn_glDeleteBuffers) {
        fn_glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }
    if (m_VAO != 0 && fn_glDeleteVertexArrays) {
        fn_glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
        InvalidateVAOCache();
    }
    m_VBOCapacity = 0;
}

bool CPlanarShadowShader::Init()
{
    if (g_RenderBackend == RenderBackend::D3D11) return InitD3D11();
    return InitGL();
}

void CPlanarShadowShader::Shutdown()
{
    if (g_RenderBackend == RenderBackend::D3D11) { ShutdownD3D11(); return; }
    ShutdownGL();
}

bool CPlanarShadowShader::Begin(const float* bodyOrigin, const float mvp[16], float sx, float sy, float alpha)
{
    if (g_RenderBackend == RenderBackend::D3D11) return BeginD3D11(bodyOrigin, mvp, sx, sy, alpha);
    return BeginGL(bodyOrigin, mvp, sx, sy, alpha);
}

void CPlanarShadowShader::End()
{
    if (g_RenderBackend == RenderBackend::D3D11) { EndD3D11(); return; }
    EndGL();
}

void CPlanarShadowShader::Draw(const float* vertices, int vertexCount)
{
    if (g_RenderBackend == RenderBackend::D3D11) { DrawD3D11(vertices, vertexCount); return; }
    DrawGL(vertices, vertexCount);
}

void CPlanarShadowShader::DrawGPUSkinned(GLuint vao, RHI::BufferHandle d3dVB, int baseCorner, int vertexCount, const float* skinOrigin, float skinScale)
{
    // DXP-21 part 2 retry (2026-08-05): real D3D11 draw. See ZzzBMD.cpp's call-site comment for
    // the pattern this mirrors. Swaps VSSetShader to the GPU-skin VS for this one draw, then
    // restores the CPU-path VS so any subsequent Draw() call in the same Begin/End session (mixed
    // GPU-skin/CPU-fallback meshes within one shadow batch) still uses the right shader.
    if (g_RenderBackend == RenderBackend::D3D11)
    {
        DrawGPUSkinnedD3D11(d3dVB, baseCorner, vertexCount, skinOrigin, skinScale);
        return;
    }

    if (!m_bActive || vao == 0 || vertexCount <= 0) return;

    const float zeroOrigin[3] = { 0.f, 0.f, 0.f };
    const float* origin = skinOrigin ? skinOrigin : zeroOrigin;

    if (m_locUseGPUSkin != -1) { fn_glUniform1i(m_locUseGPUSkin, 1); FrameProfiler::CountGLCall(FrameProfiler::Counter::UniformWrites); }
    if (m_locSkinOrigin != -1) { fn_glUniform3fv(m_locSkinOrigin, 1, origin); FrameProfiler::CountGLCall(FrameProfiler::Counter::UniformWrites); }
    if (m_locSkinScale  != -1) { fn_glUniform1f(m_locSkinScale, skinScale); FrameProfiler::CountGLCall(FrameProfiler::Counter::UniformWrites); }

    BindVAO(vao);
    glDrawArrays(GL_TRIANGLES, baseCorner, vertexCount);
    FrameProfiler::CountGLCall(FrameProfiler::Counter::DrawCalls);
    BindVAO(0);
}

bool CPlanarShadowShader::InitGL()
{
    if (m_bInitialized) return m_bSupported;
    m_bInitialized = true;
    m_bSupported   = false;
    if (!LoadGLFunctions() || !CompileShaders()) { ShutdownGL(); return false; }
    CreateBuffers();
    m_bSupported = true;
    return true;
}

void CPlanarShadowShader::ShutdownGL()
{
    if (m_hProgram       && fn_glDeleteProgram) { fn_glDeleteProgram(m_hProgram); InvalidateProgramCache(); }
    if (m_hVertexShader  && fn_glDeleteShader)  fn_glDeleteShader(m_hVertexShader);
    if (m_hFragmentShader&& fn_glDeleteShader)  fn_glDeleteShader(m_hFragmentShader);
    DestroyBuffers();

    m_hProgram        = 0;
    m_hVertexShader   = 0;
    m_hFragmentShader = 0;
    m_bSupported      = false;
    m_bActive         = false;
}

bool CPlanarShadowShader::BeginGL(const float* bodyOrigin, const float mvp[16], float sx, float sy, float alpha)
{
    if (!IsSupported() || !m_hProgram) return false;

    m_bActive = true;
    BindProgram(m_hProgram);

    if (m_locBodyOrigin  != -1) { fn_glUniform3fv(m_locBodyOrigin, 1, bodyOrigin); FrameProfiler::CountGLCall(FrameProfiler::Counter::UniformWrites); }
    if (m_locSx          != -1) { fn_glUniform1f(m_locSx, sx); FrameProfiler::CountGLCall(FrameProfiler::Counter::UniformWrites); }
    if (m_locSy          != -1) { fn_glUniform1f(m_locSy, sy); FrameProfiler::CountGLCall(FrameProfiler::Counter::UniformWrites); }
    if (m_locShadowAlpha != -1) { fn_glUniform1f(m_locShadowAlpha, alpha); FrameProfiler::CountGLCall(FrameProfiler::Counter::UniformWrites); }
    if (m_locMVP         != -1) { fn_glUniformMatrix4fv(m_locMVP, 1, GL_FALSE, mvp); FrameProfiler::CountGLCall(FrameProfiler::Counter::UniformWrites); }
    if (m_locUseGPUSkin  != -1) { fn_glUniform1i(m_locUseGPUSkin, 0); FrameProfiler::CountGLCall(FrameProfiler::Counter::UniformWrites); }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);

    return true;
}

void CPlanarShadowShader::EndGL()
{
    if (!IsSupported() || !m_bActive) return;

    glDisable(GL_POLYGON_OFFSET_FILL);
    BindProgram(0);
    m_bActive = false;
}

void CPlanarShadowShader::DrawGL(const float* vertices, int vertexCount)
{
    if (!m_bActive || m_VAO == 0 || m_VBO == 0 || vertexCount <= 0) return;

    if (m_locUseGPUSkin != -1) { fn_glUniform1i(m_locUseGPUSkin, 0); FrameProfiler::CountGLCall(FrameProfiler::Counter::UniformWrites); }

    const size_t neededSize = (size_t)vertexCount * 3 * sizeof(float);

    fn_glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    // Auto-grow buffer if vertex data exceeds current VBO capacity (same orphan-or-grow pattern
    // as ImmediateRenderer.cpp).
    if (neededSize > m_VBOCapacity) {
        m_VBOCapacity = neededSize * 2;
        fn_glBufferData(GL_ARRAY_BUFFER, m_VBOCapacity, nullptr, GL_STREAM_DRAW);
        FrameProfiler::CountGLCall(FrameProfiler::Counter::BufferUpdates);
        FrameProfiler::TagBufferOrphan();
    }

    fn_glBufferSubData(GL_ARRAY_BUFFER, 0, neededSize, vertices);
    FrameProfiler::CountGLCall(FrameProfiler::Counter::BufferUpdates);
    fn_glBindBuffer(GL_ARRAY_BUFFER, 0);

    BindVAO(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    FrameProfiler::CountGLCall(FrameProfiler::Counter::DrawCalls);
    BindVAO(0);
}

#ifdef RHI_D3D11_AVAILABLE
bool CPlanarShadowShader::InitD3D11()
{
    if (m_bInitialized) return m_bSupported;
    m_bInitialized = true;
    m_bSupported   = false;

    ID3D11Device* device = static_cast<ID3D11Device*>(RHI::GetD3D11Device());
    if (!device) return false;

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    if (!CompileHLSL(g_szPlanarVertHLSL, "main", "vs_5_0", &vsBlob)) return false;
    if (!CompileHLSL(g_szPlanarFragHLSL, "main", "ps_5_0", &psBlob)) { vsBlob->Release(); return false; }

    ID3D11VertexShader* vs = nullptr;
    if (FAILED(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vs)))
    {
        g_ErrorReport.Write(L"[PlanarShadowShader][D3D11] CreateVertexShader failed\r\n");
        vsBlob->Release(); psBlob->Release();
        return false;
    }
    ID3D11PixelShader* ps = nullptr;
    if (FAILED(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &ps)))
    {
        g_ErrorReport.Write(L"[PlanarShadowShader][D3D11] CreatePixelShader failed\r\n");
        vs->Release(); vsBlob->Release(); psBlob->Release();
        return false;
    }

    // Registers this shader's bytecode as the PosOnly layout's signature source -- RHI_D3D11
    // needs real VS bytecode to build the matching ID3D11InputLayout. PlanarShadowShader is
    // PosOnly's only producer today (matches RHI.h's design intent for this layout).
    RHI::RegisterVertexShaderBytecode(RHI::VertexLayout::PosOnly, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize());
    vsBlob->Release();
    psBlob->Release();

    // DXP-21 part 2: DrawGPUSkinned's D3D11 VS. Bound with VertexLayout::BMDMesh, whose input
    // layout is registered by BMDMeshShader -- deliberately NOT re-registered here (see class
    // comment): this VS's a_Pos/a_BoneIndex inputs are a strict subset of what that layout
    // already provides, and re-registering would just churn RHI_D3D11's cached ID3D11InputLayout
    // for no behavior change.
    ID3DBlob* vsGpuSkinBlob = nullptr;
    if (!CompileHLSL(g_szPlanarVertGPUSkinHLSL, "main", "vs_5_0", &vsGpuSkinBlob))
    {
        vs->Release(); ps->Release();
        return false;
    }
    ID3D11VertexShader* vsGpuSkin = nullptr;
    if (FAILED(device->CreateVertexShader(vsGpuSkinBlob->GetBufferPointer(), vsGpuSkinBlob->GetBufferSize(), nullptr, &vsGpuSkin)))
    {
        g_ErrorReport.Write(L"[PlanarShadowShader][D3D11] CreateVertexShader (GPU-skin) failed\r\n");
        vsGpuSkinBlob->Release();
        vs->Release(); ps->Release();
        return false;
    }
    vsGpuSkinBlob->Release();

    m_D3DVertexShader = vs;
    m_D3DPixelShader = ps;
    m_D3DVertexShaderGPUSkin = vsGpuSkin;
    m_D3DFlagsCBuffer = RHI::CreateUniformBlock(sizeof(ShadowFlagsCB), 7);

    // Sized for a reasonably busy shadow batch, same capacity as the GL path's m_VBOCapacity;
    // DrawD3D11 grows it on demand via RHI::UpdateBuffer's own grow-and-recreate policy.
    m_D3DVBOCapacity = 4096 * 3 * sizeof(float);
    m_D3DVBO = RHI::CreateVertexBuffer(nullptr, m_D3DVBOCapacity, RHI::BufferUsage::Dynamic);

    m_bSupported = true;
    g_ErrorReport.Write(L"[PlanarShadowShader][D3D11] Compiled + linked, ShadowFlags cbuffer at slot 7\r\n");
    return true;
}

void CPlanarShadowShader::ShutdownD3D11()
{
    if (m_D3DVBO.IsValid()) { RHI::DestroyBuffer(m_D3DVBO); m_D3DVBO = {}; }
    if (m_D3DFlagsCBuffer.IsValid()) { RHI::DestroyUniformBlock(m_D3DFlagsCBuffer); m_D3DFlagsCBuffer = {}; }
    if (m_D3DPixelShader)  { static_cast<ID3D11PixelShader*>(m_D3DPixelShader)->Release();   m_D3DPixelShader = nullptr; }
    if (m_D3DVertexShader) { static_cast<ID3D11VertexShader*>(m_D3DVertexShader)->Release(); m_D3DVertexShader = nullptr; }
    if (m_D3DVertexShaderGPUSkin) { static_cast<ID3D11VertexShader*>(m_D3DVertexShaderGPUSkin)->Release(); m_D3DVertexShaderGPUSkin = nullptr; }
    m_D3DVBOCapacity = 0;
    m_bSupported = false;
    m_bActive = false;
}

bool CPlanarShadowShader::BeginD3D11(const float* bodyOrigin, const float mvp[16], float sx, float sy, float alpha)
{
    if (!IsSupported() || !m_D3DVertexShader || !m_D3DPixelShader) return false;

    ID3D11DeviceContext* ctx = static_cast<ID3D11DeviceContext*>(RHI::GetD3D11DeviceContext());
    if (!ctx) return false;

    m_bActive = true;
    ctx->VSSetShader(static_cast<ID3D11VertexShader*>(m_D3DVertexShader), nullptr, 0);
    ctx->PSSetShader(static_cast<ID3D11PixelShader*>(m_D3DPixelShader), nullptr, 0);

    ShadowFlagsCB cb = {};
    memcpy(cb.mvp, mvp, sizeof(cb.mvp));
    const float zeroOrigin[3] = { 0.f, 0.f, 0.f };
    memcpy(cb.bodyOrigin, bodyOrigin ? bodyOrigin : zeroOrigin, sizeof(cb.bodyOrigin));
    cb.sx = sx;
    cb.sy = sy;
    cb.shadowAlpha = alpha;
    cb.useGPUSkin = 0; // CPU-path VS ignores this field entirely -- see g_szPlanarVertHLSL
    memcpy(cb.skinOrigin, zeroOrigin, sizeof(cb.skinOrigin));
    cb.skinScale = 1.0f;
    RHI::UpdateUniformBlock(m_D3DFlagsCBuffer, &cb, sizeof(cb));

    // DXP-21 part 2: cached so DrawGPUSkinned can rebuild the full ShadowFlagsCB (mvp/bodyOrigin/
    // sx/sy/alpha stay constant for this whole Begin/End session) without a full-cbuffer thread
    // through its own signature.
    memcpy(m_D3DCachedMVP, mvp, sizeof(m_D3DCachedMVP));
    memcpy(m_D3DCachedBodyOrigin, bodyOrigin ? bodyOrigin : zeroOrigin, sizeof(m_D3DCachedBodyOrigin));
    m_D3DCachedSx = sx;
    m_D3DCachedSy = sy;
    m_D3DCachedAlpha = alpha;

    // DXP-16 increment 4: RHI::SetBlendMode/SetPolygonOffset both have real D3D11
    // implementations now (SetBlendMode since DXP-15 increment 2; SetPolygonOffset just gained
    // one this increment, was a fail-loud stub) -- mirrors BeginGL's glEnable(GL_BLEND) +
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) + glPolygonOffset(-1,-1) exactly.
    // BlendMode::Blend3 is the RHI table entry matching those exact blend factors without
    // AlphaTest's unwanted alpha-test-discard side effect.
    RHI::SetBlendMode(RHI::BlendMode::Blend3);
    RHI::SetPolygonOffset(true, -1.0f, -1.0f);
    return true;
}

void CPlanarShadowShader::EndD3D11()
{
    if (!IsSupported() || !m_bActive) return;
    RHI::SetPolygonOffset(false, 0.0f, 0.0f); // mirrors EndGL's glDisable(GL_POLYGON_OFFSET_FILL)
    m_bActive = false;
}

void CPlanarShadowShader::DrawD3D11(const float* vertices, int vertexCount)
{
    if (!m_bActive || !m_D3DVBO.IsValid() || vertexCount <= 0) return;

    // RHI::UpdateBuffer grows-and-recreates internally when sizeBytes exceeds the buffer's
    // current capacity (RHI_D3D11.cpp's own policy) -- no manual capacity bookkeeping needed
    // here, unlike the GL path's hand-rolled m_VBOCapacity (RHI_GL wasn't in the picture when
    // that code was written).
    const size_t neededSize = static_cast<size_t>(vertexCount) * 3 * sizeof(float);
    RHI::UpdateBuffer(m_D3DVBO, vertices, neededSize);

    RHI::BindVertexBuffer(m_D3DVBO, RHI::VertexLayout::PosOnly);
    RHI::Draw(RHI::Topology::TriangleList, static_cast<uint32_t>(vertexCount), 0);
}

// DXP-21 part 2 retry (2026-08-05): swaps VSSetShader to the GPU-skin VS for this one draw, then
// restores the CPU-path VS so any subsequent Draw() call in the same Begin/End session (mixed
// GPU-skin/CPU-fallback meshes within one shadow batch) still uses the right shader.
void CPlanarShadowShader::DrawGPUSkinnedD3D11(RHI::BufferHandle d3dVB, int baseCorner, int vertexCount, const float* skinOrigin, float skinScale)
{
    if (!m_bActive || !d3dVB.IsValid() || vertexCount <= 0 || !m_D3DVertexShaderGPUSkin) return;

    ID3D11DeviceContext* ctx = static_cast<ID3D11DeviceContext*>(RHI::GetD3D11DeviceContext());
    if (!ctx) return;

    const float zeroOrigin[3] = { 0.f, 0.f, 0.f };
    const float* origin = skinOrigin ? skinOrigin : zeroOrigin;

    ShadowFlagsCB cb = {};
    memcpy(cb.mvp, m_D3DCachedMVP, sizeof(cb.mvp));
    memcpy(cb.bodyOrigin, m_D3DCachedBodyOrigin, sizeof(cb.bodyOrigin));
    cb.sx = m_D3DCachedSx;
    cb.sy = m_D3DCachedSy;
    cb.shadowAlpha = m_D3DCachedAlpha;
    cb.useGPUSkin = 1;
    memcpy(cb.skinOrigin, origin, sizeof(cb.skinOrigin));
    cb.skinScale = skinScale;
    RHI::UpdateUniformBlock(m_D3DFlagsCBuffer, &cb, sizeof(cb));

    ctx->VSSetShader(static_cast<ID3D11VertexShader*>(m_D3DVertexShaderGPUSkin), nullptr, 0);

    RHI::BindVertexBuffer(d3dVB, RHI::VertexLayout::BMDMesh);
    RHI::Draw(RHI::Topology::TriangleList, static_cast<uint32_t>(vertexCount), static_cast<uint32_t>(baseCorner));

    ctx->VSSetShader(static_cast<ID3D11VertexShader*>(m_D3DVertexShader), nullptr, 0);
}
#else // !RHI_D3D11_AVAILABLE -- keep these symbols linkable as no-ops since the public Init()/
      // Shutdown()/Begin()/End()/Draw() call them behind a runtime g_RenderBackend check, which
      // itself compiles on every platform (see RHI_D3D11.cpp's own #else for the same rationale).
bool CPlanarShadowShader::InitD3D11() { return false; }
void CPlanarShadowShader::ShutdownD3D11() {}
bool CPlanarShadowShader::BeginD3D11(const float*, const float[16], float, float, float) { return false; }
void CPlanarShadowShader::EndD3D11() {}
void CPlanarShadowShader::DrawD3D11(const float*, int) {}
void CPlanarShadowShader::DrawGPUSkinnedD3D11(RHI::BufferHandle, int, int, const float*, float) {}
#endif // RHI_D3D11_AVAILABLE
