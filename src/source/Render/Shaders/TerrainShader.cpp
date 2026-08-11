#include "stdafx.h"
#include "TerrainShader.h"
#include "Render/Core/RenderConfig.h"
#include "Render/Core/BindState.h"
#include "Core/Utilities/Log/ErrorReport.h"
#include <SDL3/SDL.h>
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
typedef void   (APIENTRY* PFNGLUNIFORM2FPROC)(GLint, GLfloat, GLfloat);
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
static PFNGLUNIFORM2FPROC          fn_glUniform2f           = nullptr;
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
    fn_glUniform2f           = (PFNGLUNIFORM2FPROC)SDL_GL_GetProcAddress("glUniform2f");
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
              fn_glUniform2f != nullptr &&
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

// Per-tile UV scale mirroring legacy FaceTexture()'s `Width = 64.f/b->Width` (pre-divided by
// TERRAIN_SCALE on the CPU side, see TerrainShader::SetUVScale / RenderTerrainFace's caller) --
// different terrain tile bitmaps are packed at different pixel resolutions (64/128/256px seen in
// this tree's assets), so a single fixed constant here can only ever match one of them. Base and
// overlay are independent because tex1/tex2 can be differently-sized bitmaps on the same tile.
uniform vec2 u_BaseUVScale;
uniform vec2 u_OverlayUVScale;

out vec2 v_UVBase;
out vec2 v_UVOverlay;
out vec3 v_Light;
out float v_Alpha;

void main() {
    vec2 uvBase    = a_Pos.xy * u_BaseUVScale;
    vec2 uvOverlay = a_Pos.xy * u_OverlayUVScale;
    v_UVBase    = (u_BaseIsWater    == 1) ? uvBase    + vec2(u_WaterMove, 0.0) : uvBase;
    v_UVOverlay = (u_OverlayIsWater == 1) ? uvOverlay + vec2(u_WaterMove, 0.0) : uvOverlay;
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
    CreateGL();
}

void TerrainShader::Destroy()
{
    DestroyGL();
}

void TerrainShader::Bind()
{
    BindGL();
}

void TerrainShader::Unbind()
{
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
    m_LocBaseUVScale    = fn_glGetUniformLocation(m_Program, "u_BaseUVScale");
    m_LocOverlayUVScale = fn_glGetUniformLocation(m_Program, "u_OverlayUVScale");

    // Assign default texture unit slots (u_BaseTex = 0, u_OverlayTex = 1)
    BindProgram(m_Program);
    if (m_LocBaseTex != -1) fn_glUniform1i(m_LocBaseTex, 0);
    if (m_LocOverlayTex != -1) fn_glUniform1i(m_LocOverlayTex, 1);
    if (m_LocWaterMove != -1) fn_glUniform1f(m_LocWaterMove, 0.0f);
    if (m_LocBaseIsWater != -1) fn_glUniform1i(m_LocBaseIsWater, 0);
    if (m_LocOverlayIsWater != -1) fn_glUniform1i(m_LocOverlayIsWater, 0);
    if (m_LocAlphaRef != -1) fn_glUniform1f(m_LocAlphaRef, -1.0f);
    m_LastAlphaRef = -1.0f;
    // Standard 64px-tile scale (64/64/TERRAIN_SCALE = 0.01) as a sane pre-first-draw default;
    // every real tile draw overwrites this via SetUVScale() before use.
    if (m_LocBaseUVScale != -1) fn_glUniform2f(m_LocBaseUVScale, 0.01f, 0.01f);
    if (m_LocOverlayUVScale != -1) fn_glUniform2f(m_LocOverlayUVScale, 0.01f, 0.01f);
    m_LastBaseUVScale[0] = m_LastBaseUVScale[1] = 0.01f;
    m_LastOverlayUVScale[0] = m_LastOverlayUVScale[1] = 0.01f;
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

extern int CachTexture;

void TerrainShader::SetBaseTexture(GLuint texID)
{
    BindTexture2D(0, texID);
    CachTexture = -1; // Invalidate engine texture cache
}

void TerrainShader::SetOverlayTexture(GLuint texID)
{
    BindTexture2D(1, texID);
    if (fn_glActiveTexture) fn_glActiveTexture(GL_TEXTURE0); // Restore default active texture
}

void TerrainShader::SetUVScale(float baseScaleX, float baseScaleY, float overlayScaleX, float overlayScaleY)
{
    // Dirty-checked like SyncAlphaRef -- adjacent tiles on the same map very often share both
    // textures, so this is usually a no-op glUniform-wise.
    if (m_LocBaseUVScale != -1 && fn_glUniform2f &&
        (baseScaleX != m_LastBaseUVScale[0] || baseScaleY != m_LastBaseUVScale[1])) {
        fn_glUniform2f(m_LocBaseUVScale, baseScaleX, baseScaleY);
        m_LastBaseUVScale[0] = baseScaleX;
        m_LastBaseUVScale[1] = baseScaleY;
    }
    if (m_LocOverlayUVScale != -1 && fn_glUniform2f &&
        (overlayScaleX != m_LastOverlayUVScale[0] || overlayScaleY != m_LastOverlayUVScale[1])) {
        fn_glUniform2f(m_LocOverlayUVScale, overlayScaleX, overlayScaleY);
        m_LastOverlayUVScale[0] = overlayScaleX;
        m_LastOverlayUVScale[1] = overlayScaleY;
    }
}

void TerrainShader::SetWaterMove(float waterMove)
{
    if (m_LocWaterMove != -1 && fn_glUniform1f) {
        fn_glUniform1f(m_LocWaterMove, waterMove);
    }
}

void TerrainShader::SetWaterFlags(bool baseIsWater, bool overlayIsWater)
{
    if (m_LocBaseIsWater != -1 && fn_glUniform1i) {
        fn_glUniform1i(m_LocBaseIsWater, baseIsWater ? 1 : 0);
    }
    if (m_LocOverlayIsWater != -1 && fn_glUniform1i) {
        fn_glUniform1i(m_LocOverlayIsWater, overlayIsWater ? 1 : 0);
    }
}

void TerrainShader::SyncAlphaRef()
{
    // Dirty-checked plain float compare — TerrainShader stays bound across many tile draws per
    // frustrum pass, so this must not cost a glUniform1f per tile (per the TASK-24 Lorencia FPS
    // lesson: per-tile GL calls are the enemy).
    if (m_LocAlphaRef != -1 && fn_glUniform1f && g_AlphaRef != m_LastAlphaRef) {
        fn_glUniform1f(m_LocAlphaRef, g_AlphaRef);
        m_LastAlphaRef = g_AlphaRef;
    }
}
