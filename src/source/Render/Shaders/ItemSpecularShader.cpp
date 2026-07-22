#include "stdafx.h"
#include "Render/Shaders/ItemSpecularShader.h"

// ===========================================================================
// OpenGL 2.0 function pointers (loaded at runtime via SDL_GL_GetProcAddress)
// ===========================================================================
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
typedef void   (APIENTRY* PFNGLUSEPROGRAMPROC)(GLuint);
typedef void   (APIENTRY* PFNGLDELETESHADERPROC)(GLuint);
typedef void   (APIENTRY* PFNGLDELETEPROGRAMPROC)(GLuint);
typedef GLint  (APIENTRY* PFNGLGETUNIFORMLOCATIONPROC)(GLuint, const GLchar*);
typedef void   (APIENTRY* PFNGLUNIFORM1IPROC)(GLint, GLint);
typedef void   (APIENTRY* PFNGLUNIFORM3FVPROC)(GLint, GLsizei, const GLfloat*);
typedef void   (APIENTRY* PFNGLACTIVETEXTUREPROC)(GLenum);
typedef void   (APIENTRY* PFNGLCLIENTACTIVETEXTUREPROC)(GLenum);

static PFNGLCREATESHADERPROC       fn_glCreateShader       = nullptr;
static PFNGLSHADERSOURCEPROC       fn_glShaderSource        = nullptr;
static PFNGLCOMPILESHADERPROC      fn_glCompileShader       = nullptr;
static PFNGLGETSHADERIVPROC        fn_glGetShaderiv         = nullptr;
static PFNGLCREATEPROGRAMPROC      fn_glCreateProgram       = nullptr;
static PFNGLATTACHSHADERPROC       fn_glAttachShader        = nullptr;
static PFNGLLINKPROGRAMPROC        fn_glLinkProgram         = nullptr;
static PFNGLGETPROGRAMIVPROC       fn_glGetProgramiv        = nullptr;
static PFNGLUSEPROGRAMPROC         fn_glUseProgram          = nullptr;
static PFNGLDELETESHADERPROC       fn_glDeleteShader        = nullptr;
static PFNGLDELETEPROGRAMPROC      fn_glDeleteProgram       = nullptr;
static PFNGLGETUNIFORMLOCATIONPROC fn_glGetUniformLocation  = nullptr;
static PFNGLUNIFORM1IPROC          fn_glUniform1i           = nullptr;
static PFNGLUNIFORM3FVPROC         fn_glUniform3fv          = nullptr;
static PFNGLACTIVETEXTUREPROC      fn_glActiveTexture       = nullptr;
static PFNGLCLIENTACTIVETEXTUREPROC fn_glClientActiveTexture = nullptr;

// ===========================================================================
// Fast lightweight pass-through vertex shader
// ===========================================================================
static const char* g_szVert = R"(
#version 120
varying vec2 v_TexCoord0;
varying vec2 v_TexCoord1;
varying vec2 v_TexCoord2;
void main()
{
    v_TexCoord0 = gl_MultiTexCoord0.st;
    v_TexCoord1 = gl_MultiTexCoord1.st;
    v_TexCoord2 = gl_MultiTexCoord2.st;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
)";

// ===========================================================================
// Variant 1 (+7/+8): Base*light  +  Chrome1(static)
// ===========================================================================
static const char* g_szFrag_V1 = R"(
#version 120
uniform sampler2D u_BaseTex;
uniform sampler2D u_Chrome1Tex;
uniform vec3 u_BodyLight;
varying vec2 v_TexCoord0;
varying vec2 v_TexCoord2;
void main()
{
    vec4 base    = texture2D(u_BaseTex,    v_TexCoord0);
    vec4 chrome1 = texture2D(u_Chrome1Tex, v_TexCoord2);
    gl_FragColor = vec4(base.rgb * u_BodyLight + chrome1.rgb, base.a);
}
)";

// ===========================================================================
// Variant 2 (+9/+10): Base*light  +  Chrome1(static)  +  Metal(static)
// ===========================================================================
static const char* g_szFrag_V2 = R"(
#version 120
uniform sampler2D u_BaseTex;
uniform sampler2D u_Chrome1Tex;
uniform sampler2D u_MetalTex;
uniform vec3 u_BodyLight;
varying vec2 v_TexCoord0;
varying vec2 v_TexCoord2;
void main()
{
    vec4 base    = texture2D(u_BaseTex,    v_TexCoord0);
    vec4 chrome1 = texture2D(u_Chrome1Tex, v_TexCoord2);
    vec4 metal   = texture2D(u_MetalTex,   v_TexCoord2);
    gl_FragColor = vec4(base.rgb * u_BodyLight + chrome1.rgb + metal.rgb, base.a);
}
)";

// ===========================================================================
// Variant 3 (+11/+12): Base*light  +  Chrome2(CHROME2-mode UV)  +  Metal(static)  +  Chrome1(static)
// ===========================================================================
static const char* g_szFrag_V3 = R"(
#version 120
uniform sampler2D u_BaseTex;
uniform sampler2D u_Chrome2Tex;
uniform sampler2D u_MetalTex;
uniform sampler2D u_Chrome1Tex;
uniform vec3 u_BodyLight;
varying vec2 v_TexCoord0;
varying vec2 v_TexCoord1;
varying vec2 v_TexCoord2;
void main()
{
    vec4 base    = texture2D(u_BaseTex,    v_TexCoord0);
    vec4 chrome2 = texture2D(u_Chrome2Tex, v_TexCoord1);
    vec4 metal   = texture2D(u_MetalTex,   v_TexCoord2);
    vec4 chrome1 = texture2D(u_Chrome1Tex, v_TexCoord2);
    gl_FragColor = vec4(base.rgb * u_BodyLight + chrome2.rgb + metal.rgb + chrome1.rgb, base.a);
}
)";

// ===========================================================================
// Variant 4 (+13/+15): Base*light  +  Chrome2(CHROME4-mode UV)  +  Metal(static)  +  Chrome1(static)
// ===========================================================================
static const char* g_szFrag_V4 = R"(
#version 120
uniform sampler2D u_BaseTex;
uniform sampler2D u_Chrome2Tex;
uniform sampler2D u_MetalTex;
uniform sampler2D u_Chrome1Tex;
uniform vec3 u_BodyLight;
varying vec2 v_TexCoord0;
varying vec2 v_TexCoord1;
varying vec2 v_TexCoord2;
void main()
{
    vec4 base    = texture2D(u_BaseTex,    v_TexCoord0);
    vec4 chrome2 = texture2D(u_Chrome2Tex, v_TexCoord1);
    vec4 metal   = texture2D(u_MetalTex,   v_TexCoord2);
    vec4 chrome1 = texture2D(u_Chrome1Tex, v_TexCoord2);
    gl_FragColor = vec4(base.rgb * u_BodyLight + chrome2.rgb + metal.rgb + chrome1.rgb, base.a);
}
)";

// ===========================================================================
// Singleton
// ===========================================================================
CItemSpecularShader& CItemSpecularShader::Instance()
{
    static CItemSpecularShader s_instance;
    return s_instance;
}

CItemSpecularShader::CItemSpecularShader()  {}
CItemSpecularShader::~CItemSpecularShader() { Shutdown(); }

bool CItemSpecularShader::LoadGLFunctions()
{
    fn_glCreateShader        = (PFNGLCREATESHADERPROC)      SDL_GL_GetProcAddress("glCreateShader");
    fn_glShaderSource        = (PFNGLSHADERSOURCEPROC)      SDL_GL_GetProcAddress("glShaderSource");
    fn_glCompileShader       = (PFNGLCOMPILESHADERPROC)     SDL_GL_GetProcAddress("glCompileShader");
    fn_glGetShaderiv         = (PFNGLGETSHADERIVPROC)       SDL_GL_GetProcAddress("glGetShaderiv");
    fn_glCreateProgram       = (PFNGLCREATEPROGRAMPROC)     SDL_GL_GetProcAddress("glCreateProgram");
    fn_glAttachShader        = (PFNGLATTACHSHADERPROC)      SDL_GL_GetProcAddress("glAttachShader");
    fn_glLinkProgram         = (PFNGLLINKPROGRAMPROC)       SDL_GL_GetProcAddress("glLinkProgram");
    fn_glGetProgramiv        = (PFNGLGETPROGRAMIVPROC)      SDL_GL_GetProcAddress("glGetProgramiv");
    fn_glUseProgram          = (PFNGLUSEPROGRAMPROC)        SDL_GL_GetProcAddress("glUseProgram");
    fn_glDeleteShader        = (PFNGLDELETESHADERPROC)      SDL_GL_GetProcAddress("glDeleteShader");
    fn_glDeleteProgram       = (PFNGLDELETEPROGRAMPROC)     SDL_GL_GetProcAddress("glDeleteProgram");
    fn_glGetUniformLocation  = (PFNGLGETUNIFORMLOCATIONPROC)SDL_GL_GetProcAddress("glGetUniformLocation");
    fn_glUniform1i           = (PFNGLUNIFORM1IPROC)         SDL_GL_GetProcAddress("glUniform1i");
    fn_glUniform3fv          = (PFNGLUNIFORM3FVPROC)        SDL_GL_GetProcAddress("glUniform3fv");
    fn_glActiveTexture       = (PFNGLACTIVETEXTUREPROC)     SDL_GL_GetProcAddress("glActiveTexture");
    fn_glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glClientActiveTexture");

    return fn_glCreateShader && fn_glShaderSource && fn_glCompileShader && fn_glGetShaderiv &&
           fn_glCreateProgram && fn_glAttachShader && fn_glLinkProgram && fn_glGetProgramiv &&
           fn_glUseProgram && fn_glGetUniformLocation && fn_glUniform1i &&
           fn_glUniform3fv && fn_glActiveTexture && fn_glClientActiveTexture;
}

bool CItemSpecularShader::Init()
{
    if (m_bInitialized) return m_bSupported;
    m_bInitialized = true;
    m_bSupported   = false;
    if (!LoadGLFunctions() || !CompileShaders()) { Shutdown(); return false; }
    m_bSupported = true;
    return true;
}

void CItemSpecularShader::Shutdown()
{
    for (int i = 1; i < SHADER_VARIANT_COUNT; ++i)
    {
        if (m_programs[i].hProgram       && fn_glDeleteProgram) fn_glDeleteProgram(m_programs[i].hProgram);
        if (m_programs[i].hVertexShader  && fn_glDeleteShader)  fn_glDeleteShader(m_programs[i].hVertexShader);
        if (m_programs[i].hFragmentShader&& fn_glDeleteShader)  fn_glDeleteShader(m_programs[i].hFragmentShader);
        m_programs[i] = ShaderProgram{};
    }
    m_bSupported   = false;
    m_activeVariant = SHADER_VARIANT_NONE;
}

// ---------------------------------------------------------------------------
static bool BuildProgram(const char* vertSrc, const char* fragSrc, CItemSpecularShader::ShaderProgram& p)
{
    GLint ok = 0;

    p.hVertexShader = fn_glCreateShader(GL_VERTEX_SHADER);
    fn_glShaderSource(p.hVertexShader, 1, &vertSrc, nullptr);
    fn_glCompileShader(p.hVertexShader);
    fn_glGetShaderiv(p.hVertexShader, GL_COMPILE_STATUS, &ok);
    if (!ok) return false;

    p.hFragmentShader = fn_glCreateShader(GL_FRAGMENT_SHADER);
    fn_glShaderSource(p.hFragmentShader, 1, &fragSrc, nullptr);
    fn_glCompileShader(p.hFragmentShader);
    fn_glGetShaderiv(p.hFragmentShader, GL_COMPILE_STATUS, &ok);
    if (!ok) return false;

    p.hProgram = fn_glCreateProgram();
    fn_glAttachShader(p.hProgram, p.hVertexShader);
    fn_glAttachShader(p.hProgram, p.hFragmentShader);
    fn_glLinkProgram(p.hProgram);
    fn_glGetProgramiv(p.hProgram, GL_LINK_STATUS, &ok);
    if (!ok) return false;

    p.locBaseTex       = fn_glGetUniformLocation(p.hProgram, "u_BaseTex");
    p.locAnimChromeTex = fn_glGetUniformLocation(p.hProgram, "u_Chrome2Tex");
    p.locMetalTex      = fn_glGetUniformLocation(p.hProgram, "u_MetalTex");
    p.locChrome1Tex    = fn_glGetUniformLocation(p.hProgram, "u_Chrome1Tex");
    p.locBodyLight     = fn_glGetUniformLocation(p.hProgram, "u_BodyLight");

    // Bind texture unit locations once at initialization
    fn_glUseProgram(p.hProgram);
    if (p.locBaseTex != -1)       fn_glUniform1i(p.locBaseTex, 0);
    if (p.locAnimChromeTex != -1) fn_glUniform1i(p.locAnimChromeTex, 1);
    if (p.locMetalTex != -1)      fn_glUniform1i(p.locMetalTex, 2);
    if (p.locChrome1Tex != -1)    fn_glUniform1i(p.locChrome1Tex, 3);
    fn_glUseProgram(0);

    return true;
}

bool CItemSpecularShader::CompileShaders()
{
    return BuildProgram(g_szVert, g_szFrag_V1, m_programs[SHADER_VARIANT_CHROME_1])       &&
           BuildProgram(g_szVert, g_szFrag_V2, m_programs[SHADER_VARIANT_CHROME_METAL])   &&
           BuildProgram(g_szVert, g_szFrag_V3, m_programs[SHADER_VARIANT_FULL_SPECULAR_V1]) &&
           BuildProgram(g_szVert, g_szFrag_V4, m_programs[SHADER_VARIANT_FULL_SPECULAR_V2]);
}

bool CItemSpecularShader::Begin(EShaderVariant variant,
                                GLuint baseTex, GLuint animChromeTex,
                                GLuint metalTex, GLuint chrome1Tex,
                                const float* bodyLight)
{
    if (!IsSupported() || variant <= SHADER_VARIANT_NONE || variant >= SHADER_VARIANT_COUNT)
        return false;

    const ShaderProgram& p = m_programs[variant];
    if (!p.hProgram) return false;

    m_activeVariant = variant;
    fn_glUseProgram(p.hProgram);

    fn_glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, baseTex);

    if (variant == SHADER_VARIANT_CHROME_1)
    {
        fn_glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, chrome1Tex);
    }
    else if (variant == SHADER_VARIANT_CHROME_METAL)
    {
        fn_glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, chrome1Tex);

        fn_glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, metalTex);
    }
    else
    {
        fn_glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, animChromeTex);

        fn_glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, metalTex);

        fn_glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, chrome1Tex);
    }

    if (p.locBodyLight != -1) fn_glUniform3fv(p.locBodyLight, 1, bodyLight);

    fn_glActiveTexture(GL_TEXTURE0);
    return true;
}

void CItemSpecularShader::SubmitTexCoords(const float texBase[][2],
                                          const float texAnim[][2],
                                          const float texStatic[][2])
{
    if (!fn_glClientActiveTexture) return;

    fn_glClientActiveTexture(GL_TEXTURE0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, texBase);

    fn_glClientActiveTexture(GL_TEXTURE1);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, texAnim);

    fn_glClientActiveTexture(GL_TEXTURE2);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, texStatic);

    fn_glClientActiveTexture(GL_TEXTURE3);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, texStatic);

    fn_glClientActiveTexture(GL_TEXTURE0);
}

void CItemSpecularShader::DisableTexCoords()
{
    if (!fn_glClientActiveTexture) return;

    fn_glClientActiveTexture(GL_TEXTURE3);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    fn_glClientActiveTexture(GL_TEXTURE2);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    fn_glClientActiveTexture(GL_TEXTURE1);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    fn_glClientActiveTexture(GL_TEXTURE0);
}

void CItemSpecularShader::End()
{
    if (!IsSupported() || m_activeVariant == SHADER_VARIANT_NONE) return;

    DisableTexCoords();
    fn_glUseProgram(0);
    m_activeVariant = SHADER_VARIANT_NONE;
}
