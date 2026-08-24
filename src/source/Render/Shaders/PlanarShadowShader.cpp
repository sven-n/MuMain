#include "stdafx.h"
#include "Render/Shaders/EsShaderShim.h"
#include "Render/Shaders/PlanarShadowShader.h"
#include "Render/Core/BindState.h"
#include "Render/Core/RenderConfig.h"
#include "Core/Utilities/FrameProfiler.h"
#include "Core/Utilities/Log/ErrorReport.h"
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
    fn_glShaderSource       = (PFNGLSHADERSOURCEPROC)      MU_GLSHADERSOURCE_PROC;
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
    return InitGL();
}

void CPlanarShadowShader::Shutdown()
{
    ShutdownGL();
}

bool CPlanarShadowShader::Begin(const float* bodyOrigin, const float mvp[16], float sx, float sy, float alpha)
{
    return BeginGL(bodyOrigin, mvp, sx, sy, alpha);
}

void CPlanarShadowShader::End()
{
    EndGL();
}

void CPlanarShadowShader::Draw(const float* vertices, int vertexCount)
{
    DrawGL(vertices, vertexCount);
}

void CPlanarShadowShader::DrawGPUSkinned(GLuint vao, int baseCorner, int vertexCount, const float* skinOrigin, float skinScale)
{
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

