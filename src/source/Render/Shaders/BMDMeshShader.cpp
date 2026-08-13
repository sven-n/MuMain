#include "stdafx.h"
#include "BMDMeshShader.h"
#include "Render/Core/RenderConfig.h"
#include "Render/Core/GlobalUBO.h"
#include "Render/Core/BindState.h"
#include "Core/Utilities/FrameProfiler.h"
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
typedef void   (APIENTRY* PFNGLUNIFORM2FPROC)(GLint, GLfloat, GLfloat);
typedef void   (APIENTRY* PFNGLUNIFORM3FVPROC)(GLint, GLsizei, const GLfloat*);
typedef void   (APIENTRY* PFNGLUNIFORMMATRIX4FVPROC)(GLint, GLsizei, GLboolean, const GLfloat*);

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
static PFNGLUNIFORM2FPROC          fn_glUniform2f           = nullptr;
static PFNGLUNIFORM3FVPROC         fn_glUniform3fv          = nullptr;
static PFNGLUNIFORMMATRIX4FVPROC   fn_glUniformMatrix4fv    = nullptr;

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
    fn_glUniform2f           = (PFNGLUNIFORM2FPROC)SDL_GL_GetProcAddress("glUniform2f");
    fn_glUniform3fv          = (PFNGLUNIFORM3FVPROC)SDL_GL_GetProcAddress("glUniform3fv");
    fn_glUniformMatrix4fv    = (PFNGLUNIFORMMATRIX4FVPROC)SDL_GL_GetProcAddress("glUniformMatrix4fv");

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

static const char* g_szBMDMeshVert = R"(
#version 330 core

layout(std140) uniform GlobalMatrices {
    mat4 u_View;
    mat4 u_Proj;
    mat4 u_Model;
    mat4 u_MVP;
    vec4 u_Time;
};

// Bone matrix palette UBO at binding slot 2. GLP-11: 3x vec4 affine rows per bone, not mat4 --
// std140 pads a vec4 array to 16-byte stride, which vec4 already is, so this array is tight.
// Row i of bone b is u_Bones[b*3+i]; reconstructed via dot products in main(), not a mat3/mat4
// constructor (see the comment at the reconstruction site for why).
layout(std140) uniform BoneMatrices {
    vec4 u_Bones[600]; // 3 rows per bone, 200 bones
};

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec2 a_UV;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in vec3 a_Normal;
layout(location = 4) in int  a_BoneIndex;

// GLS-09: all non-sampler uniforms consolidated into one std140 UBO (binding slot 6).
// Field order/offsets match BMDMeshShader.h's BMDFlagsCB byte-for-byte (176 bytes) —
// don't reorder without updating that struct too.
layout(std140) uniform BMDFlags {
    mat4  u_MVPDraw;     // per-draw MVP from current GL matrix state
    int   u_UseGPUSkin;  // 1 = skin via u_Bones[a_BoneIndex], 0 = pass-through (dynamic VBO)
    int   u_RenderMode;
    float u_WaveOffsetU;
    float u_WaveOffsetV;
    vec3  u_BodyOrigin;  // world-space placement of model root (applied after bone transform)
    float u_BodyScale;   // uniform body scale (applied after bone transform)
    float u_ChromeWave;  // RenderMode 3/4/5: CPU-computed wave term (see BMD::RenderMesh's `wave`)
    float u_ChromeWave2; // RenderMode 3 (variant 1/4) / 6: CPU-computed wave term for the RENDER_CHROME2/6 UV formulas (Wave2s)
    vec2  u_ChromeLightVec; // RenderMode 3 (variant 3) / 7: CPU-computed light vector xy for the RENDER_CHROME5/CHROME4 UV formulas (Lp; z is always 1.0)
    // DXP-20 increment 5: RenderMode 3 covers plain RENDER_CHROME *and* CHROME2/3/5/6/7/METAL — ZzzBMD.cpp's
    // `finalRenderFlags` collapses all of them to RENDER_CHROME, so BMD::RenderMesh's chromeGpuEligible
    // GPU-skinned draw path was already silently taking these variants before this uniform existed, always
    // using the plain-chrome UV formula. u_ChromeVariant selects the CPU-verified per-variant formula
    // (ZzzBMD.cpp's g_chrome[] loop) to match: 0=plain CHROME, 1=CHROME2, 2=CHROME3, 3=CHROME5, 4=CHROME6,
    // 5=CHROME7, 6=METAL/unmatched (same "else" formula the CPU loop falls back to).
    int   u_ChromeVariant;
    float u_ChromeTimeTerm; // RenderMode 3, variant 5 (CHROME7): CPU-computed WorldTime*0.00006 term
    float u_Alpha;
    float u_AlphaRef; // fragment-only; occupies the slot between u_Alpha and u_LightDir per BMDFlagsCB
    // DXP-20 increment 1: per-vertex lighting moved from BMD::Transform()'s CPU IntensityTransform loop
    // into the u_UseGPUSkin==1 path here. u_LightDir is Transform()'s per-body LightPosition vector;
    // u_BodyLight/u_LightEnable replicate RenderMesh()'s useLight/BodyLight exactly.
    vec3  u_LightDir;
    int   u_LightEnable;
    vec3  u_BodyLight;
    float _pad0;
    vec3  u_SpecularTint; // RenderMode 4+ only
    float _pad1;
};

out vec2 v_UV;
out vec4 v_Color;
out vec2 v_ChromeUV;
// RenderMode 5+ (item specular tier 2+, CHROME_METAL/FULL_SPECULAR): static MatCap UV, ported 1:1
// from ZzzBMD.cpp's CPU g_chrome_static[] formula (n.xy*0.5+0.5 — no wave/time term, unlike
// v_ChromeUV). Default value covers the u_UseGPUSkin==0 branch, which these item-specular modes
// never take in practice.
out vec2 v_ChromeUVStatic;
// RenderMode 6 (item specular tier 3, FULL_SPECULAR_V1): animated UV, ported 1:1 from ZzzBMD.cpp's
// CPU g_chrome[] RENDER_CHROME2 formula — a different animated-UV formula from v_ChromeUV's default
// RENDER_CHROME one, driven by u_ChromeWave2 (Wave2s) instead of u_ChromeWave.
out vec2 v_ChromeUV2;

void main() {
    vec3 skinnedNormal = a_Normal;
    vec2 chromeUV = a_UV;
    vec4 gpuSkinColor = a_Color;
    v_ChromeUVStatic = vec2(0.5);
    v_ChromeUV2 = vec2(0.0);
    if (u_UseGPUSkin == 1 && a_BoneIndex >= 0 && a_BoneIndex < 200) {
        // GLP-11: reconstruct bonePos/skinnedNormal from the 3 affine rows via dot products,
        // NOT mat3(...)/mat4(...) constructors -- those take COLUMNS, and r0/r1/r2 are ROWS, so
        // a naive mat3(r0.xyz, r1.xyz, r2.xyz) would silently be the transpose of the rotation
        // wanted. Dot products sidestep the trap entirely and match the old column-major mat4
        // math exactly (verified by hand: old M*p had M[col][row], so (M*p).x/y/z were
        // row0/1/2 dot p under GLSL's column-major multiply -- the same rows this palette now
        // stores directly).
        // u_Bones[a_BoneIndex*3 + i] transforms rest-pose position to skeleton-local space.
        // u_BodyOrigin + u_BodyScale * bonePos replicates BMD::Transform VectorMA:
        //   VectorMA(BodyOrigin, BodyScale, bonePos, VertexTransform)
        // This produces the correct world-space position that u_MVPDraw (camera VP) expects.
        vec4 r0 = u_Bones[a_BoneIndex * 3 + 0];
        vec4 r1 = u_Bones[a_BoneIndex * 3 + 1];
        vec4 r2 = u_Bones[a_BoneIndex * 3 + 2];
        vec4 p = vec4(a_Pos, 1.0);
        vec3 bonePos = vec3(dot(r0, p), dot(r1, p), dot(r2, p));
        vec3 worldPos = u_BodyOrigin + u_BodyScale * bonePos;
        gl_Position = u_MVPDraw * vec4(worldPos, 1.0);
        // Rotation-only skin of the rest normal, matching BMD::Transform's CPU
        // VectorRotate(sn->Normal, BoneMatrix[sn->Node], tn) — same rows as position, dropping
        // the translation component (row.xyz only, no homogeneous term).
        skinnedNormal = normalize(vec3(dot(r0.xyz, a_Normal), dot(r1.xyz, a_Normal), dot(r2.xyz, a_Normal)));
        // Per-vertex lighting, ported 1:1 from BMD::Transform's CPU loop:
        //   Luminosity = DotProduct(tn, LightPosition) * 0.8f + 0.4f; clamp to >= 0.2f (no upper bound)
        //   LightTransform = BodyLight * Luminosity
        // u_LightEnable mirrors RenderMesh's `useLight` (false for RENDER_BRIGHT / !LightEnable /
        // StreamMesh) — those cases use flat u_BodyLight, same as the CPU fallback did.
        if (u_LightEnable == 1) {
            float lum = max(dot(skinnedNormal, u_LightDir) * 0.8 + 0.4, 0.2);
            gpuSkinColor = vec4(u_BodyLight * lum, u_Alpha);
        } else {
            gpuSkinColor = vec4(u_BodyLight, u_Alpha);
        }
        // Plain RENDER_CHROME UV, ported 1:1 from BMD::RenderMesh's CPU g_chrome[] formula:
        //   g_chrome[j][0] = normal[2]*0.5f + wave; g_chrome[j][1] = normal[1]*0.5f + wave*2.f;
        chromeUV = vec2(skinnedNormal.z * 0.5 + u_ChromeWave, skinnedNormal.y * 0.5 + u_ChromeWave * 2.0);
        // Static MatCap UV, ported 1:1 from the CPU g_chrome_static[] formula (ZzzBMD.cpp):
        //   g_chrome_static[j][0] = n[0]*0.5f + 0.5f; g_chrome_static[j][1] = n[1]*0.5f + 0.5f;
        v_ChromeUVStatic = skinnedNormal.xy * 0.5 + 0.5;
        if (u_RenderMode == 7) {
            // RENDER_CHROME4 animated UV, ported 1:1 from the CPU g_chrome[] formula (ZzzBMD.cpp):
            //   g_chrome[j][0] = DotProduct(n,Lp);     g_chrome[j][1] = 1.f - DotProduct(n,Lp);
            //   g_chrome[j][1] -= n[2]*0.5f + waveLoc*3.f; g_chrome[j][0] += n[1]*0.5f + Lp[1]*3.f;
            // (waveLoc is the same CPU value as u_ChromeWave — confirmed identical formula.)
            vec3 Lp = vec3(u_ChromeLightVec, 1.0);
            float d = dot(skinnedNormal, Lp);
            v_ChromeUV2 = vec2(d + skinnedNormal.y * 0.5 + u_ChromeLightVec.y * 3.0,
                                (1.0 - d) - skinnedNormal.z * 0.5 - u_ChromeWave * 3.0);
        } else {
            // RENDER_CHROME2 animated UV, ported 1:1 from the CPU g_chrome[] formula (ZzzBMD.cpp):
            //   g_chrome[j][0] = (n[2]+n[0])*0.8f + Wave2s*2.f; g_chrome[j][1] = (n[1]+n[0])*1.0f + Wave2s*3.f;
            v_ChromeUV2 = vec2((skinnedNormal.z + skinnedNormal.x) * 0.8 + u_ChromeWave2 * 2.0,
                                (skinnedNormal.y + skinnedNormal.x) * 1.0 + u_ChromeWave2 * 3.0);
        }
    } else {
        // CPU pre-transformed position (dynamic VBO fallback / stream mesh)
        // VertexTransform[] already has BodyOrigin+BodyScale applied on CPU side.
        gl_Position = u_MVPDraw * vec4(a_Pos, 1.0);
    }
    v_Color = gpuSkinColor;
    v_ChromeUV = chromeUV;
    if (u_RenderMode == 0) {
        v_UV = a_UV + vec2(u_WaveOffsetU, u_WaveOffsetV);
    } else if (u_RenderMode == 3) {
        // Chrome-family meshes: chrome UV replaces the base UV entirely. u_ChromeVariant selects
        // which of the 7 CPU g_chrome[] formulas this draw corresponds to (see uniform comment above).
        if (u_ChromeVariant == 1) {
            // RENDER_CHROME2: g_chrome[0]=(n.z+n.x)*0.8+Wave2*2; g_chrome[1]=(n.y+n.x)*1.0+Wave2*3
            v_UV = vec2((skinnedNormal.z + skinnedNormal.x) * 0.8 + u_ChromeWave2 * 2.0,
                        (skinnedNormal.y + skinnedNormal.x) * 1.0 + u_ChromeWave2 * 3.0);
        } else if (u_ChromeVariant == 2) {
            // RENDER_CHROME3: g_chrome[0]=dot(n,LightVector); g_chrome[1]=1-dot(n,LightVector)
            // LightVector is ZzzBMD.cpp's file-static vec3_t{0,-0.1,-0.8} — never modified anywhere,
            // safe to inline as a constant.
            float d = dot(skinnedNormal, vec3(0.0, -0.1, -0.8));
            v_UV = vec2(d, 1.0 - d);
        } else if (u_ChromeVariant == 3) {
            // RENDER_CHROME5: g_chrome[0]=dot(n,L)+n.y*3+L.y*5; g_chrome[1]=(1-dot(n,L))-n.z*2.5-wave*1
            vec3 L = vec3(u_ChromeLightVec, 1.0);
            float d = dot(skinnedNormal, L);
            v_UV = vec2(d + skinnedNormal.y * 3.0 + u_ChromeLightVec.y * 5.0,
                        (1.0 - d) - skinnedNormal.z * 2.5 - u_ChromeWave * 1.0);
        } else if (u_ChromeVariant == 4) {
            // RENDER_CHROME6: g_chrome[0]=g_chrome[1]=(n.z+n.x)*0.8+Wave2*2
            float t = (skinnedNormal.z + skinnedNormal.x) * 0.8 + u_ChromeWave2 * 2.0;
            v_UV = vec2(t, t);
        } else if (u_ChromeVariant == 5) {
            // RENDER_CHROME7: g_chrome[0]=g_chrome[1]=(n.z+n.x)*0.8+WorldTime*0.00006
            float t = (skinnedNormal.z + skinnedNormal.x) * 0.8 + u_ChromeTimeTerm;
            v_UV = vec2(t, t);
        } else if (u_ChromeVariant == 6) {
            // RENDER_METAL / unmatched: g_chrome[0]=n.z*0.5+0.2; g_chrome[1]=n.y*0.5+0.5
            v_UV = vec2(skinnedNormal.z * 0.5 + 0.2, skinnedNormal.y * 0.5 + 0.5);
        } else {
            // Plain RENDER_CHROME.
            v_UV = chromeUV;
        }
    } else {
        // RenderMode 4 (item specular tier 1) keeps the base UV in v_UV and uses v_ChromeUV
        // separately for the second (chrome1) texture sample.
        v_UV = a_UV;
    }
}
)";

static const char* g_szBMDMeshFrag = R"(
#version 330 core

uniform sampler2D u_Tex;
uniform sampler2D u_ChromeTex1;   // RenderMode 4+ only
uniform sampler2D u_MetalTex;     // RenderMode 5+ only
uniform sampler2D u_ChromeTex2;   // RenderMode 6 only

// GLS-09: same BMDFlags block as the vertex shader (identical layout required in every
// stage that uses it). Only u_RenderMode, u_AlphaRef, and u_SpecularTint are read here;
// the rest are simply unused in this stage.
layout(std140) uniform BMDFlags {
    mat4  u_MVPDraw;
    int   u_UseGPUSkin;
    int   u_RenderMode;
    float u_WaveOffsetU;
    float u_WaveOffsetV;
    vec3  u_BodyOrigin;
    float u_BodyScale;
    float u_ChromeWave;
    float u_ChromeWave2;
    vec2  u_ChromeLightVec;
    int   u_ChromeVariant;
    float u_ChromeTimeTerm;
    float u_Alpha;
    // Fixed-function alpha test threshold (DXP-01), mirrored from glAlphaFunc/GL_ALPHA_TEST state.
    // -1.0 means alpha test is disabled (no discard); otherwise the GL_GREATER reference value.
    float u_AlphaRef;
    vec3  u_LightDir;
    int   u_LightEnable;
    vec3  u_BodyLight;
    float _pad0;
    vec3  u_SpecularTint; // RenderMode 4+ only
    float _pad1;
};

in vec2 v_UV;
in vec4 v_Color;
in vec2 v_ChromeUV;
in vec2 v_ChromeUVStatic;
in vec2 v_ChromeUV2;

out vec4 fragColor;

void main() {
    // Clamp v_Color to [0, 1] to match FFP vertex color clamping.
    // In fixed-function, glColor3fv(val) is clamped per-vertex BEFORE GL_MODULATE
    // texture multiply. Without this clamp, LightTransform values > 1.0 (which occur
    // near dynamic lights like power slash) cause over-saturation: texture(0.5)*color(2.0)=1.0
    // (white) instead of the correct texture(0.5)*clamped(1.0)=0.5 (grey).
    vec4 c = clamp(v_Color, 0.0, 1.0);

    if (u_RenderMode == 2) {
        // RENDER_BRIGHT: solid vertex color, no texture (matches FFP DisableTexture path)
        fragColor = c;
    } else if (u_RenderMode == 4) {
        // Item specular tier 1 (SHADER_VARIANT_CHROME_1), ported from ItemSpecularShader's V1:
        //   gl_FragColor = base.rgb * u_BodyLight + chrome1.rgb * u_SpecularTint
        // `c` (clamped vertex color) stands in for u_BodyLight — the color VBO already carries
        // the correct BodyLight-scaled per-vertex lighting, same as every other render mode here.
        vec4 base    = texture(u_Tex, v_UV);
        vec4 chrome1 = texture(u_ChromeTex1, v_ChromeUV);
        fragColor = vec4(base.rgb * c.rgb + chrome1.rgb * u_SpecularTint, base.a);
    } else if (u_RenderMode == 5) {
        // Item specular tier 2 (SHADER_VARIANT_CHROME_METAL), ported from ItemSpecularShader's V2:
        //   gl_FragColor = base.rgb * u_BodyLight + (chrome1.rgb + metal.rgb * 1.8) * u_SpecularTint
        // Chrome1 keeps the animated v_ChromeUV (same "water ripple" UV as mode 4); Metal uses the
        // static v_ChromeUVStatic MatCap projection — matches CItemSpecularShader::Begin()'s texture/
        // texcoord pairing for SHADER_VARIANT_CHROME_METAL exactly.
        vec4 base    = texture(u_Tex, v_UV);
        vec4 chrome1 = texture(u_ChromeTex1, v_ChromeUV);
        vec4 metal   = texture(u_MetalTex, v_ChromeUVStatic);
        fragColor = vec4(base.rgb * c.rgb + (chrome1.rgb + metal.rgb * 1.8) * u_SpecularTint, base.a);
    } else if (u_RenderMode == 6) {
        // Item specular tier 3 (SHADER_VARIANT_FULL_SPECULAR_V1), ported from ItemSpecularShader's V3:
        //   gl_FragColor = base.rgb*u_BodyLight + chrome2.rgb*u_BodyLight + (metal.rgb+chrome1.rgb)*u_SpecularTint
        // Chrome2 (animated sweep) is scene-light tinted like the base; Metal + Chrome1 (both static
        // MatCap UV here, unlike mode 4/5's animated Chrome1) are per-item tinted.
        vec4 base    = texture(u_Tex, v_UV);
        vec4 chrome2 = texture(u_ChromeTex2, v_ChromeUV2);
        vec4 metal   = texture(u_MetalTex, v_ChromeUVStatic);
        vec4 chrome1 = texture(u_ChromeTex1, v_ChromeUVStatic);
        fragColor = vec4((base.rgb + chrome2.rgb) * c.rgb + (metal.rgb + chrome1.rgb) * u_SpecularTint, base.a);
    } else if (u_RenderMode == 7) {
        // Item specular tier 4 (SHADER_VARIANT_FULL_SPECULAR_V2), ported from ItemSpecularShader's V4.
        // Textually identical blend formula to mode 6 (FULL_SPECULAR_V1) — only the animated Chrome2
        // UV formula differs (RENDER_CHROME4 vs RENDER_CHROME2), already selected into v_ChromeUV2 by
        // the vertex shader based on u_RenderMode.
        vec4 base    = texture(u_Tex, v_UV);
        vec4 chrome2 = texture(u_ChromeTex2, v_ChromeUV2);
        vec4 metal   = texture(u_MetalTex, v_ChromeUVStatic);
        vec4 chrome1 = texture(u_ChromeTex1, v_ChromeUVStatic);
        fragColor = vec4((base.rgb + chrome2.rgb) * c.rgb + (metal.rgb + chrome1.rgb) * u_SpecularTint, base.a);
    } else {
        // RENDER_TEXTURE / RENDER_CHROME: texture modulated by clamped vertex color
        fragColor = texture(u_Tex, v_UV) * c;
    }
    if (fragColor.a < 0.01) discard;
    // <= matches GL_GREATER exactly (fixed-function alpha test passes when a > ref). Kept as an
    // additional, separate test from the 0.01 line above: that one also fires in blend modes
    // (where alpha test is off, u_AlphaRef < 0) and must keep firing there unchanged.
    if (u_AlphaRef >= 0.0 && fragColor.a <= u_AlphaRef) discard;
}
)";

// DXP-14 increment 3 -- HLSL port, kept semantically line-for-line with the GLSL above (same
// convention as Passthrough/Terrain: column_major matrices + mul(M,v), one shared BMDFlags
// cbuffer for every loose GLSL uniform since HLSL has no non-block uniform storage).
static const char* g_szBMDMeshVertHLSL = R"(
cbuffer GlobalMatrices : register(b0)
{
    column_major matrix u_View;
    column_major matrix u_Proj;
    column_major matrix u_Model;
    column_major matrix u_MVP;
    float4 u_Time;
};

// GLP-11: 3x float4 affine rows per bone, not a full matrix -- matches the GLSL twin's
// std140 vec4[600] layout and BoneUBO's actual upload (200 bones * 12 floats). Row i of
// bone b is u_Bones[b*3+i], reconstructed via dot products in main() below, same as GLSL.
cbuffer BoneMatrices : register(b2)
{
    float4 u_Bones[600]; // 3 rows per bone, 200 bones
};

// The GLSL vertex+fragment shaders' ~20 loose uniforms have no HLSL "non-block uniform"
// equivalent -- packed into one cbuffer instead (RHI slot 6: 0=GlobalMatrices, 1=SceneData,
// 2=BoneUBO, 3=DynamicLights reserved, 4=Passthrough's PassFlags, 5=Terrain's TerrainFlags).
// Field order matches BMDMeshShader.cpp's BMDFlagsCB C++ struct byte-for-byte (both hand-laid
// out to the same 16-byte-row boundaries HLSL's automatic cbuffer packing would produce anyway
// -- explicit here so the two sides never drift out of sync silently).
cbuffer BMDFlags : register(b6)
{
    column_major matrix u_MVPDraw;
    int    u_UseGPUSkin;
    int    u_RenderMode;
    float  u_WaveOffsetU;
    float  u_WaveOffsetV;
    float3 u_BodyOrigin;
    float  u_BodyScale;
    float  u_ChromeWave;
    float  u_ChromeWave2;
    float2 u_ChromeLightVec;
    int    u_ChromeVariant;
    float  u_ChromeTimeTerm;
    float  u_Alpha;
    float  u_AlphaRef;
    float3 u_LightDir;
    int    u_LightEnable;
    float3 u_BodyLight;
    float  _bmdPad0;
    float3 u_SpecularTint;
    float  _bmdPad1;
};

struct VSInput
{
    float3 a_Pos       : POSITION;
    float2 a_UV        : TEXCOORD0;
    float4 a_Color     : COLOR0;
    float3 a_Normal    : NORMAL;
    int    a_BoneIndex : BLENDINDICES0;
};

struct VSOutput
{
    float4 pos           : SV_POSITION;
    float2 uv            : TEXCOORD0;
    float4 color         : COLOR0;
    float2 chromeUV      : TEXCOORD1;
    float2 chromeUVStatic: TEXCOORD2;
    float2 chromeUV2     : TEXCOORD3;
};

VSOutput main(VSInput input)
{
    VSOutput o;
    float3 skinnedNormal = input.a_Normal;
    float2 chromeUV = input.a_UV;
    float4 gpuSkinColor = input.a_Color;
    o.chromeUVStatic = float2(0.5, 0.5);
    o.chromeUV2 = float2(0.0, 0.0);

    if (u_UseGPUSkin == 1 && input.a_BoneIndex >= 0 && input.a_BoneIndex < 200)
    {
        // GLP-11 row layout (matches the GLSL twin exactly, see its comment for the full
        // derivation): u_Bones[a_BoneIndex*3 + i] transforms rest-pose position to
        // skeleton-local space. u_BodyOrigin + u_BodyScale*bonePos replicates BMD::Transform's
        // VectorMA. dot() reconstruction (not a matrix cast) because r0/r1/r2 are rows, not
        // columns -- same trap the GLSL comment calls out.
        float4 r0 = u_Bones[input.a_BoneIndex * 3 + 0];
        float4 r1 = u_Bones[input.a_BoneIndex * 3 + 1];
        float4 r2 = u_Bones[input.a_BoneIndex * 3 + 2];
        float4 p = float4(input.a_Pos, 1.0);
        float3 bonePos = float3(dot(r0, p), dot(r1, p), dot(r2, p));
        float3 worldPos = u_BodyOrigin + u_BodyScale * bonePos;
        o.pos = mul(u_MVPDraw, float4(worldPos, 1.0));

        // Rotation-only skin of the rest normal -- .xyz drops the translation term, same as
        // GLSL's row.xyz-only dot products.
        skinnedNormal = normalize(float3(dot(r0.xyz, input.a_Normal), dot(r1.xyz, input.a_Normal), dot(r2.xyz, input.a_Normal)));

        // Per-vertex lighting, ported 1:1 from BMD::Transform's CPU loop (see the GLSL twin's
        // comment for the exact formula reference).
        if (u_LightEnable == 1)
        {
            float lum = max(dot(skinnedNormal, u_LightDir) * 0.8 + 0.4, 0.2);
            gpuSkinColor = float4(u_BodyLight * lum, u_Alpha);
        }
        else
        {
            gpuSkinColor = float4(u_BodyLight, u_Alpha);
        }

        // Plain RENDER_CHROME UV + static MatCap UV, ported 1:1 (see GLSL twin).
        chromeUV = float2(skinnedNormal.z * 0.5 + u_ChromeWave, skinnedNormal.y * 0.5 + u_ChromeWave * 2.0);
        o.chromeUVStatic = skinnedNormal.xy * 0.5 + 0.5;

        if (u_RenderMode == 7)
        {
            float3 Lp = float3(u_ChromeLightVec, 1.0);
            float d = dot(skinnedNormal, Lp);
            o.chromeUV2 = float2(d + skinnedNormal.y * 0.5 + u_ChromeLightVec.y * 3.0,
                                  (1.0 - d) - skinnedNormal.z * 0.5 - u_ChromeWave * 3.0);
        }
        else
        {
            o.chromeUV2 = float2((skinnedNormal.z + skinnedNormal.x) * 0.8 + u_ChromeWave2 * 2.0,
                                  (skinnedNormal.y + skinnedNormal.x) * 1.0 + u_ChromeWave2 * 3.0);
        }
    }
    else
    {
        // CPU pre-transformed position (dynamic VBO fallback / stream mesh)
        o.pos = mul(u_MVPDraw, float4(input.a_Pos, 1.0));
    }

    o.color = gpuSkinColor;
    o.chromeUV = chromeUV;

    if (u_RenderMode == 0)
    {
        o.uv = input.a_UV + float2(u_WaveOffsetU, u_WaveOffsetV);
    }
    else if (u_RenderMode == 3)
    {
        // Chrome-family meshes: chrome UV replaces the base UV entirely, selected per
        // u_ChromeVariant (see the GLSL twin's per-branch formula references).
        if (u_ChromeVariant == 1)
        {
            o.uv = float2((skinnedNormal.z + skinnedNormal.x) * 0.8 + u_ChromeWave2 * 2.0,
                          (skinnedNormal.y + skinnedNormal.x) * 1.0 + u_ChromeWave2 * 3.0);
        }
        else if (u_ChromeVariant == 2)
        {
            float d = dot(skinnedNormal, float3(0.0, -0.1, -0.8));
            o.uv = float2(d, 1.0 - d);
        }
        else if (u_ChromeVariant == 3)
        {
            float3 L = float3(u_ChromeLightVec, 1.0);
            float d = dot(skinnedNormal, L);
            o.uv = float2(d + skinnedNormal.y * 3.0 + u_ChromeLightVec.y * 5.0,
                          (1.0 - d) - skinnedNormal.z * 2.5 - u_ChromeWave * 1.0);
        }
        else if (u_ChromeVariant == 4)
        {
            float t = (skinnedNormal.z + skinnedNormal.x) * 0.8 + u_ChromeWave2 * 2.0;
            o.uv = float2(t, t);
        }
        else if (u_ChromeVariant == 5)
        {
            float t = (skinnedNormal.z + skinnedNormal.x) * 0.8 + u_ChromeTimeTerm;
            o.uv = float2(t, t);
        }
        else if (u_ChromeVariant == 6)
        {
            o.uv = float2(skinnedNormal.z * 0.5 + 0.2, skinnedNormal.y * 0.5 + 0.5);
        }
        else
        {
            o.uv = chromeUV;
        }
    }
    else
    {
        o.uv = input.a_UV;
    }

    return o;
}
)";

static const char* g_szBMDMeshFragHLSL = R"(
cbuffer BMDFlags : register(b6)
{
    column_major matrix u_MVPDraw;
    int    u_UseGPUSkin;
    int    u_RenderMode;
    float  u_WaveOffsetU;
    float  u_WaveOffsetV;
    float3 u_BodyOrigin;
    float  u_BodyScale;
    float  u_ChromeWave;
    float  u_ChromeWave2;
    float2 u_ChromeLightVec;
    int    u_ChromeVariant;
    float  u_ChromeTimeTerm;
    float  u_Alpha;
    float  u_AlphaRef;
    float3 u_LightDir;
    int    u_LightEnable;
    float3 u_BodyLight;
    float  _bmdPad0;
    float3 u_SpecularTint;
    float  _bmdPad1;
};

Texture2D    u_Tex        : register(t0);
Texture2D    u_ChromeTex1 : register(t1); // RenderMode 4+ only
Texture2D    u_MetalTex   : register(t2); // RenderMode 5+ only
Texture2D    u_ChromeTex2 : register(t3); // RenderMode 6/7 only
// One sampler register per slot, not one shared s0 -- RHI_D3D11::BindTexture() already binds each
// slot's OWN recorded filter/wrap sampler via PSSetSamplers(slot, ...) (each texture's wrap mode
// was set correctly at load time, e.g. BITMAP_CHROME2 is GL_CLAMP_TO_EDGE in GL), but a single
// shared SamplerState register only ever reads whatever sampler is bound at s0 regardless of which
// texture's .Sample() call uses it -- so u_ChromeTex1/u_MetalTex/u_ChromeTex2 were all silently
// sampled with slot 0's (the base armor texture's, often GL_REPEAT for tiling cloth) sampler
// instead of their own. Mode 6/7's chromeUV2 goes well outside [0,1] (chromeWave2 term), so this
// showed up as u_ChromeTex2 tiling/wrapping across a wide UV range instead of clamping -- the
// reported "+11 and up swipe is too fast and too bright" regression (GL is unaffected: each GL
// texture unit carries its own wrap state, no shared-sampler concept).
SamplerState u_Samp0 : register(s0);
SamplerState u_Samp1 : register(s1);
SamplerState u_Samp2 : register(s2);
SamplerState u_Samp3 : register(s3);

struct PSInput
{
    float4 pos           : SV_POSITION;
    float2 uv            : TEXCOORD0;
    float4 color         : COLOR0;
    float2 chromeUV      : TEXCOORD1;
    float2 chromeUVStatic: TEXCOORD2;
    float2 chromeUV2     : TEXCOORD3;
};

float4 main(PSInput input) : SV_TARGET
{
    // Clamp to [0,1] to match FFP vertex color clamping (see the GLSL twin's comment).
    float4 c = saturate(input.color);

    float4 fragColor;
    if (u_RenderMode == 2)
    {
        // RENDER_BRIGHT: solid vertex color, no texture.
        fragColor = c;
    }
    else if (u_RenderMode == 4)
    {
        float4 base    = u_Tex.Sample(u_Samp0, input.uv);
        float4 chrome1 = u_ChromeTex1.Sample(u_Samp1, input.chromeUV);
        fragColor = float4(base.rgb * c.rgb + chrome1.rgb * u_SpecularTint, base.a);
    }
    else if (u_RenderMode == 5)
    {
        float4 base    = u_Tex.Sample(u_Samp0, input.uv);
        float4 chrome1 = u_ChromeTex1.Sample(u_Samp1, input.chromeUV);
        float4 metal   = u_MetalTex.Sample(u_Samp2, input.chromeUVStatic);
        fragColor = float4(base.rgb * c.rgb + (chrome1.rgb + metal.rgb * 1.8) * u_SpecularTint, base.a);
    }
    else if (u_RenderMode == 6)
    {
        float4 base    = u_Tex.Sample(u_Samp0, input.uv);
        float4 chrome2 = u_ChromeTex2.Sample(u_Samp3, input.chromeUV2);
        float4 metal   = u_MetalTex.Sample(u_Samp2, input.chromeUVStatic);
        float4 chrome1 = u_ChromeTex1.Sample(u_Samp1, input.chromeUVStatic);
        fragColor = float4((base.rgb + chrome2.rgb) * c.rgb + (metal.rgb + chrome1.rgb) * u_SpecularTint, base.a);
    }
    else if (u_RenderMode == 7)
    {
        // Textually identical blend formula to mode 6 -- only the animated chromeUV2 formula
        // differs, already selected by the vertex shader based on u_RenderMode.
        float4 base    = u_Tex.Sample(u_Samp0, input.uv);
        float4 chrome2 = u_ChromeTex2.Sample(u_Samp3, input.chromeUV2);
        float4 metal   = u_MetalTex.Sample(u_Samp2, input.chromeUVStatic);
        float4 chrome1 = u_ChromeTex1.Sample(u_Samp1, input.chromeUVStatic);
        fragColor = float4((base.rgb + chrome2.rgb) * c.rgb + (metal.rgb + chrome1.rgb) * u_SpecularTint, base.a);
    }
    else
    {
        // RENDER_TEXTURE / RENDER_CHROME: texture modulated by clamped vertex color.
        fragColor = u_Tex.Sample(u_Samp0, input.uv) * c;
    }

    if (fragColor.a < 0.01) discard;
    // <= matches GL_GREATER exactly (fixed-function alpha test passes when a > ref).
    if (u_AlphaRef >= 0.0 && fragColor.a <= u_AlphaRef) discard;

    return fragColor;
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
                g_ErrorReport.Write(L"[BMDMeshShader][D3D11] %hs compile error: %hs\r\n",
                    entryPoint, static_cast<const char*>(errorBlob->GetBufferPointer()));
                errorBlob->Release();
            }
            else
            {
                g_ErrorReport.Write(L"[BMDMeshShader][D3D11] %hs compile failed, hr=0x%08lX\r\n",
                    entryPoint, static_cast<unsigned long>(hr));
            }
            return false;
        }
        if (errorBlob) errorBlob->Release(); // warnings only
        return true;
    }

    // Field order matches the BMDFlags cbuffer declared above byte-for-byte -- both hand-laid
    // out to the same 16-byte-row boundaries, verified against HLSL's automatic cbuffer packing
    // rules at design time (see the cbuffer's own comment). All members are 4-byte-aligned
    // scalars/arrays so this struct has no compiler-inserted padding; the static_assert below
    // catches any future edit that breaks that assumption.
    struct BMDFlagsCB
    {
        float   mvpDraw[16];
        int32_t useGPUSkin;
        int32_t renderMode;
        float   waveOffsetU;
        float   waveOffsetV;
        float   bodyOrigin[3];
        float   bodyScale;
        float   chromeWave;
        float   chromeWave2;
        float   chromeLightVec[2];
        int32_t chromeVariant;
        float   chromeTimeTerm;
        float   alpha;
        float   alphaRef;
        float   lightDir[3];
        int32_t lightEnable;
        float   bodyLight[3];
        float   _pad0;
        float   specularTint[3];
        float   _pad1;
    };
    static_assert(sizeof(BMDFlagsCB) == 176, "BMDFlagsCB must match the HLSL BMDFlags cbuffer layout byte-for-byte");
}
#endif // RHI_D3D11_AVAILABLE -- ID3DBlob/D3DCompile aren't declared without <d3dcompiler.h>
       // (guarded above with the same macro); this block would fail to compile on non-Windows
       // otherwise.

BMDMeshShader& BMDMeshShader::Instance()
{
    static BMDMeshShader instance;
    return instance;
}

BMDMeshShader::~BMDMeshShader()
{
    Destroy();
}

void BMDMeshShader::Create()
{
    if (g_RenderBackend == RenderBackend::D3D11) { CreateD3D11(); return; }
    CreateGL();
}

void BMDMeshShader::Destroy()
{
    if (g_RenderBackend == RenderBackend::D3D11) { DestroyD3D11(); return; }
    DestroyGL();
}

void BMDMeshShader::Bind(int renderMode, float waveOffsetU, float waveOffsetV, GLuint texID, const float mvp[16], int useGPUSkin, const float bodyOrigin[3], float bodyScale, float chromeWave, GLuint chromeTex1ID, const float specularTint[3], GLuint metalTexID, GLuint chromeTex2ID, float chromeWave2, float chromeLightVecX, float chromeLightVecY, const float lightDir[3], const float bodyLight[3], int lightEnable, float alpha, int chromeVariant, float chromeTimeTerm)
{
    if (g_RenderBackend == RenderBackend::D3D11)
    {
        if (!m_D3DVertexShader) Create();
        BindD3D11(renderMode, waveOffsetU, waveOffsetV, texID, useGPUSkin, bodyOrigin, bodyScale, chromeWave,
            chromeTex1ID, specularTint, metalTexID, chromeTex2ID, chromeWave2, chromeLightVecX, chromeLightVecY,
            lightDir, bodyLight, lightEnable, alpha, chromeVariant, chromeTimeTerm, mvp);
        return;
    }
    BindGL(renderMode, waveOffsetU, waveOffsetV, texID, mvp, useGPUSkin, bodyOrigin, bodyScale, chromeWave,
        chromeTex1ID, specularTint, metalTexID, chromeTex2ID, chromeWave2, chromeLightVecX, chromeLightVecY,
        lightDir, bodyLight, lightEnable, alpha, chromeVariant, chromeTimeTerm);
}

void BMDMeshShader::Unbind()
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

void BMDMeshShader::CreateGL()
{
    if (m_Program != 0) return;
    if (!LoadGLShaderFunctions()) return;

    // Compile Vertex Shader
    m_VertShader = fn_glCreateShader(GL_VERTEX_SHADER);
    fn_glShaderSource(m_VertShader, 1, &g_szBMDMeshVert, nullptr);
    fn_glCompileShader(m_VertShader);

    GLint compiled = 0;
    fn_glGetShaderiv(m_VertShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char infoLog[512];
        if (fn_glGetShaderInfoLog) fn_glGetShaderInfoLog(m_VertShader, 512, nullptr, infoLog);
        SDL_Log("[BMDMeshShader] Vertex compilation error: %s", infoLog);
        fn_glDeleteShader(m_VertShader);
        m_VertShader = 0;
        return;
    }

    // Compile Fragment Shader
    m_FragShader = fn_glCreateShader(GL_FRAGMENT_SHADER);
    fn_glShaderSource(m_FragShader, 1, &g_szBMDMeshFrag, nullptr);
    fn_glCompileShader(m_FragShader);

    fn_glGetShaderiv(m_FragShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char infoLog[512];
        if (fn_glGetShaderInfoLog) fn_glGetShaderInfoLog(m_FragShader, 512, nullptr, infoLog);
        SDL_Log("[BMDMeshShader] Fragment compilation error: %s", infoLog);
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
        SDL_Log("[BMDMeshShader] Program link error: %s", infoLog);
        fn_glDeleteProgram(m_Program);
        InvalidateProgramCache();
        fn_glDeleteShader(m_VertShader);
        fn_glDeleteShader(m_FragShader);
        m_Program = m_VertShader = m_FragShader = 0;
        return;
    }

    // Bind GlobalMatrices uniform block to binding point 0, BoneMatrices to binding point 2,
    // and BMDFlags (GLS-09) to binding point 6.
    if (fn_glGetUniformBlockIndex != nullptr && fn_glUniformBlockBinding != nullptr) {
        GLuint globalBlockIdx = fn_glGetUniformBlockIndex(m_Program, "GlobalMatrices");
        if (globalBlockIdx != GL_INVALID_INDEX) {
            fn_glUniformBlockBinding(m_Program, globalBlockIdx, 0);
        }
        GLuint boneBlockIdx = fn_glGetUniformBlockIndex(m_Program, "BoneMatrices");
        if (boneBlockIdx != GL_INVALID_INDEX) {
            fn_glUniformBlockBinding(m_Program, boneBlockIdx, 2);
        }
        GLuint bmdFlagsBlockIdx = fn_glGetUniformBlockIndex(m_Program, "BMDFlags");
        if (bmdFlagsBlockIdx != GL_INVALID_INDEX) {
            fn_glUniformBlockBinding(m_Program, bmdFlagsBlockIdx, 6);
        }
    }
    if (!m_BMDFlagsUBO.IsValid()) {
        m_BMDFlagsUBO = RHI::CreateUniformBlock(sizeof(BMDFlagsCB), 6);
    }

    // Resolve sampler uniform locations (the only uniforms left outside the BMDFlags UBO —
    // GLSL forbids samplers inside uniform blocks).
    m_LocTex        = fn_glGetUniformLocation(m_Program, "u_Tex");
    m_LocChromeTex1 = fn_glGetUniformLocation(m_Program, "u_ChromeTex1");
    m_LocMetalTex   = fn_glGetUniformLocation(m_Program, "u_MetalTex");
    m_LocChromeTex2 = fn_glGetUniformLocation(m_Program, "u_ChromeTex2");

    BindProgram(m_Program);
    if (m_LocTex != -1) { fn_glUniform1i(m_LocTex, 0); FrameProfiler::CountGLCall(FrameProfiler::Counter::UniformWrites); }
    if (m_LocChromeTex1 != -1) { fn_glUniform1i(m_LocChromeTex1, 1); FrameProfiler::CountGLCall(FrameProfiler::Counter::UniformWrites); }
    if (m_LocMetalTex != -1) { fn_glUniform1i(m_LocMetalTex, 2); FrameProfiler::CountGLCall(FrameProfiler::Counter::UniformWrites); }
    if (m_LocChromeTex2 != -1) { fn_glUniform1i(m_LocChromeTex2, 3); FrameProfiler::CountGLCall(FrameProfiler::Counter::UniformWrites); }
    BindProgram(0);

    g_ErrorReport.Write(L"[BMDMeshShader] Created program ID %d\r\n", m_Program);
}

void BMDMeshShader::DestroyGL()
{
    if (m_Program != 0) {
        BindProgram(0);
        if (m_VertShader) fn_glDeleteShader(m_VertShader);
        if (m_FragShader) fn_glDeleteShader(m_FragShader);
        if (fn_glDeleteProgram) { fn_glDeleteProgram(m_Program); InvalidateProgramCache(); }
        m_Program = m_VertShader = m_FragShader = 0;
    }
    if (m_BMDFlagsUBO.IsValid()) {
        RHI::DestroyUniformBlock(m_BMDFlagsUBO);
        m_BMDFlagsUBO = {};
    }
}

void BMDMeshShader::BindGL(int renderMode, float waveOffsetU, float waveOffsetV, GLuint texID, const float mvp[16], int useGPUSkin, const float bodyOrigin[3], float bodyScale, float chromeWave, GLuint chromeTex1ID, const float specularTint[3], GLuint metalTexID, GLuint chromeTex2ID, float chromeWave2, float chromeLightVecX, float chromeLightVecY, const float lightDir[3], const float bodyLight[3], int lightEnable, float alpha, int chromeVariant, float chromeTimeTerm)
{
    if (m_Program == 0) {
        CreateGL();
    }
    if (m_Program == 0) {
        return;
    }

    BindProgram(m_Program);

    // GLS-09: populate the whole BMDFlags UBO unconditionally every call, mirroring
    // dx-only-port's proven BindD3D11() rather than this function's old per-renderMode/
    // per-useGPUSkin conditional glUniform* calls. Safe because the shader only ever reads
    // the fields relevant to the active u_RenderMode/u_UseGPUSkin branch (confirmed against
    // g_szBMDMeshVert/g_szBMDMeshFrag above) -- whatever an irrelevant field holds doesn't
    // matter, so there's no need to preserve "previous call's leftover value" semantics.
    BMDFlagsCB cb = {};
    if (mvp != nullptr) {
        memcpy(cb.mvpDraw, mvp, sizeof(cb.mvpDraw));
    } else {
        // DXP-07d increment 0, stage 2: no caller currently passes mvp==nullptr (both
        // ZzzBMD.cpp call sites always supply GetCurrentMVP()'s result), so this branch is
        // presently unreachable -- kept correct and consistent with the primary path's source
        // (GlobalUBO) rather than left on the old glGetFloatv fallback it can no longer share
        // a soak history with.
        memcpy(cb.mvpDraw, GlobalUBO::Instance().GetMVP(), sizeof(cb.mvpDraw));
    }
    cb.useGPUSkin = useGPUSkin;
    cb.renderMode = renderMode;
    cb.waveOffsetU = waveOffsetU;
    cb.waveOffsetV = waveOffsetV;

    static const float zeroOrigin[3] = { 0.f, 0.f, 0.f };
    static const float whiteLight[3] = { 1.f, 1.f, 1.f };
    static const float whiteTint[3]  = { 1.f, 1.f, 1.f };

    // GPU skinning: BodyOrigin/BodyScale so vertices land at world position (mirrors
    // BMD::Transform VectorMA(BodyOrigin, BodyScale, bonePos, vertexTransform)), plus
    // DXP-20's in-shader per-vertex lighting (BMD::Transform's IntensityTransform /
    // RenderMesh's useLight, ported to GLSL -- see the vertex shader above).
    memcpy(cb.bodyOrigin, bodyOrigin ? bodyOrigin : zeroOrigin, sizeof(cb.bodyOrigin));
    cb.bodyScale = bodyScale;
    cb.chromeWave = chromeWave;
    cb.chromeWave2 = chromeWave2;
    cb.chromeLightVec[0] = chromeLightVecX;
    cb.chromeLightVec[1] = chromeLightVecY;
    cb.chromeVariant = chromeVariant;
    cb.chromeTimeTerm = chromeTimeTerm;
    cb.alpha = alpha;
    cb.alphaRef = g_AlphaRef; // was always set unconditionally in the old code too
    memcpy(cb.lightDir, lightDir ? lightDir : zeroOrigin, sizeof(cb.lightDir));
    cb.lightEnable = lightEnable;
    memcpy(cb.bodyLight, bodyLight ? bodyLight : whiteLight, sizeof(cb.bodyLight));
    memcpy(cb.specularTint, specularTint ? specularTint : whiteTint, sizeof(cb.specularTint));

    if (m_BMDFlagsUBO.IsValid()) {
        RHI::UpdateUniformBlock(m_BMDFlagsUBO, &cb, sizeof(cb));
    }

    // Sampler binds -- unchanged, samplers can't live in a UBO. GLP-03: u_Tex/u_ChromeTex1/
    // u_MetalTex/u_ChromeTex2 are all assigned once, permanently, at CreateGL() time (:494-499)
    // and SetTexture() no longer rewrites u_Tex per call -- so unlike before, these four calls
    // can run in any order.
    if (renderMode == 4 || renderMode == 5 || renderMode == 6 || renderMode == 7) {
        SetTexture(chromeTex1ID, 1);
    }
    if (renderMode == 5 || renderMode == 6 || renderMode == 7) {
        SetTexture(metalTexID, 2);
    }
    if (renderMode == 6 || renderMode == 7) {
        SetTexture(chromeTex2ID, 3);
    }
    if (texID != 0) {
        SetTexture(texID, 0);
    }
}

void BMDMeshShader::SetTexture(GLuint texID, int slot)
{
    if (g_RenderBackend == RenderBackend::D3D11) return; // DXP-15/16: D3D11 texture binding, not yet wired

    BindTexture2D(slot, texID);
}

#ifdef RHI_D3D11_AVAILABLE
void BMDMeshShader::CreateD3D11()
{
    if (m_D3DVertexShader) return;
    ID3D11Device* device = static_cast<ID3D11Device*>(RHI::GetD3D11Device());
    if (!device) return;

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    if (!CompileHLSL(g_szBMDMeshVertHLSL, "main", "vs_5_0", &vsBlob)) return;
    if (!CompileHLSL(g_szBMDMeshFragHLSL, "main", "ps_5_0", &psBlob)) { vsBlob->Release(); return; }

    ID3D11VertexShader* vs = nullptr;
    if (FAILED(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vs)))
    {
        g_ErrorReport.Write(L"[BMDMeshShader][D3D11] CreateVertexShader failed\r\n");
        vsBlob->Release(); psBlob->Release();
        return;
    }
    ID3D11PixelShader* ps = nullptr;
    if (FAILED(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &ps)))
    {
        g_ErrorReport.Write(L"[BMDMeshShader][D3D11] CreatePixelShader failed\r\n");
        vs->Release(); vsBlob->Release(); psBlob->Release();
        return;
    }

    // Registers this shader's bytecode as the BMDMesh layout's signature source -- RHI_D3D11
    // needs real VS bytecode to build the matching ID3D11InputLayout (D3D11 requirement, no GL
    // equivalent). BMDMeshShader is BMDMesh's only producer today.
    RHI::RegisterVertexShaderBytecode(RHI::VertexLayout::BMDMesh, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize());
    vsBlob->Release();
    psBlob->Release();

    m_D3DVertexShader = vs;
    m_D3DPixelShader = ps;

    m_D3DFlagsCBuffer = RHI::CreateUniformBlock(sizeof(BMDFlagsCB), 6);

    // DXP-16 increment 2 (superseded by increment 3, see below): u_Samp (s0) used to be one fixed
    // linear/clamp sampler shared by all 4 texture slots (t0-t3).
    // DXP-16 increment 3: replaced by RHI::BindTexture() binding each texture's own recorded
    // filter/wrap sampler at bind time (RHI_D3D11.cpp's GetOrCreateSampler) -- see
    // TerrainShader.cpp's CreateD3D11 comment for the bug a fixed shared sampler caused elsewhere.
    // This shader's textures are all CLAMP-wrapped model UVs so it never hit that bug directly,
    // but there's no longer a per-shader sampler to create here either way.

    g_ErrorReport.Write(L"[BMDMeshShader][D3D11] Compiled + linked, BMDFlags cbuffer at slot 6\r\n");
}

void BMDMeshShader::DestroyD3D11()
{
    if (m_D3DFlagsCBuffer.IsValid()) { RHI::DestroyUniformBlock(m_D3DFlagsCBuffer); m_D3DFlagsCBuffer = {}; }
    if (m_D3DPixelShader)  { static_cast<ID3D11PixelShader*>(m_D3DPixelShader)->Release();   m_D3DPixelShader = nullptr; }
    if (m_D3DVertexShader) { static_cast<ID3D11VertexShader*>(m_D3DVertexShader)->Release(); m_D3DVertexShader = nullptr; }
}

extern int CachTexture;

void BMDMeshShader::BindD3D11(int renderMode, float waveOffsetU, float waveOffsetV, GLuint texID, int useGPUSkin,
    const float bodyOrigin[3], float bodyScale, float chromeWave, GLuint chromeTex1ID, const float specularTint[3],
    GLuint metalTexID, GLuint chromeTex2ID, float chromeWave2, float chromeLightVecX, float chromeLightVecY,
    const float lightDir[3], const float bodyLight[3], int lightEnable, float alpha, int chromeVariant,
    float chromeTimeTerm, const float mvp[16])
{
    ID3D11DeviceContext* ctx = static_cast<ID3D11DeviceContext*>(RHI::GetD3D11DeviceContext());
    if (!ctx || !m_D3DVertexShader || !m_D3DPixelShader || !m_D3DFlagsCBuffer.IsValid()) return;

    ctx->VSSetShader(static_cast<ID3D11VertexShader*>(m_D3DVertexShader), nullptr, 0);
    ctx->PSSetShader(static_cast<ID3D11PixelShader*>(m_D3DPixelShader), nullptr, 0);

    // Mirrors BindGL's renderMode-gated texture binds (t0=base, t1=chrome1, t2=metal, t3=chrome2).
    // Always call BindTexture for slots 1-3, not just when the renderMode condition is met: an
    // invalid handle still clears that slot's SRV (previously left stale from whatever draw last
    // used it) and binds a default sampler (RHI_D3D11.cpp's BindTexture) -- skipping the call
    // entirely left both the SRV and sampler wholly unset for the common case (plain-textured
    // items, renderMode outside 4-7), which is what the D3D11 debug layer's per-draw "Sampler not
    // set at Slot 2/3" warning was actually flagging.
    RHI::BindTexture(RHI::TextureHandle{ (renderMode == 4 || renderMode == 5 || renderMode == 6 || renderMode == 7) ? chromeTex1ID : 0 }, 1);
    RHI::BindTexture(RHI::TextureHandle{ (renderMode == 5 || renderMode == 6 || renderMode == 7) ? metalTexID : 0 }, 2);
    RHI::BindTexture(RHI::TextureHandle{ (renderMode == 6 || renderMode == 7) ? chromeTex2ID : 0 }, 3);
    if (texID != 0)
    {
        RHI::BindTexture(RHI::TextureHandle{ texID }, 0);
        // DXP-16 fix: same class of bug as TerrainShader::SetBaseTexture/PassthroughShader::SetTexture's
        // D3D11 branches -- this direct RHI::BindTexture(slot 0) bypasses the CachTexture cache the
        // UI's BindTexture(int) wrapper (ZzzOpenglUtil.cpp) trusts, so it must invalidate it too, or a
        // later UI sprite bind for the same texture ID gets silently skipped while slot 0 still holds
        // whichever character/model texture this draw call just bound (the reported login-window
        // background flicker: RenderCharactersClient()/RenderMount() run before BeginBitmap() every
        // login-scene frame).
        CachTexture = -1;
    }

    BMDFlagsCB cb = {};
    if (mvp != nullptr)
    {
        memcpy(cb.mvpDraw, mvp, sizeof(cb.mvpDraw));
    }
    else
    {
        // Mirrors BindGL's mvp==nullptr fallback (presently unreachable -- both ZzzBMD.cpp call
        // sites always supply GetCurrentMVP()'s result -- kept for parity with the GL path).
        memcpy(cb.mvpDraw, GlobalUBO::Instance().GetMVP(), sizeof(cb.mvpDraw));
    }
    cb.useGPUSkin = useGPUSkin;
    cb.renderMode = renderMode;
    cb.waveOffsetU = waveOffsetU;
    cb.waveOffsetV = waveOffsetV;

    const float zeroOrigin[3] = { 0.f, 0.f, 0.f };
    const float whiteLight[3] = { 1.f, 1.f, 1.f };
    const float whiteTint[3]  = { 1.f, 1.f, 1.f };

    const float* origin = bodyOrigin ? bodyOrigin : zeroOrigin;
    memcpy(cb.bodyOrigin, origin, sizeof(cb.bodyOrigin));
    cb.bodyScale = bodyScale;
    cb.chromeWave = chromeWave;
    cb.chromeWave2 = chromeWave2;
    cb.chromeLightVec[0] = chromeLightVecX;
    cb.chromeLightVec[1] = chromeLightVecY;
    cb.chromeVariant = chromeVariant;
    cb.chromeTimeTerm = chromeTimeTerm;
    cb.alpha = alpha;
    cb.alphaRef = g_AlphaRef;

    const float* dir = lightDir ? lightDir : zeroOrigin;
    memcpy(cb.lightDir, dir, sizeof(cb.lightDir));
    cb.lightEnable = lightEnable;

    const float* light = bodyLight ? bodyLight : whiteLight;
    memcpy(cb.bodyLight, light, sizeof(cb.bodyLight));

    const float* tint = specularTint ? specularTint : whiteTint;
    memcpy(cb.specularTint, tint, sizeof(cb.specularTint));

    RHI::UpdateUniformBlock(m_D3DFlagsCBuffer, &cb, sizeof(cb));
}
#else // !RHI_D3D11_AVAILABLE -- keep these symbols linkable as no-ops since Create()/Destroy()/
      // Bind() call them behind a runtime g_RenderBackend check, which itself compiles on every
      // platform (see RHI_D3D11.cpp's own #else for the same rationale).
void BMDMeshShader::CreateD3D11() {}
void BMDMeshShader::DestroyD3D11() {}
void BMDMeshShader::BindD3D11(int, float, float, GLuint, int, const float[3], float, float, GLuint,
    const float[3], GLuint, GLuint, float, float, float, const float[3], const float[3], int, float,
    int, float, const float[16]) {}
#endif // RHI_D3D11_AVAILABLE
