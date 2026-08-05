#include "stdafx.h"
#include "BindState.h"
#include <SDL3/SDL.h>

#ifndef APIENTRY
#define APIENTRY
#endif

typedef void (APIENTRY* PFNGLUSEPROGRAMPROC)(GLuint);
typedef void (APIENTRY* PFNGLBINDVERTEXARRAYPROC)(GLuint);
typedef void (APIENTRY* PFNGLACTIVETEXTUREPROC)(GLenum);

static PFNGLUSEPROGRAMPROC      fn_glUseProgram      = nullptr;
static PFNGLBINDVERTEXARRAYPROC fn_glBindVertexArray = nullptr;
static PFNGLACTIVETEXTUREPROC   fn_glActiveTexture   = nullptr;

// glBindTexture itself is GL 1.1 core (exported directly by opengl32.dll on Windows), so unlike the
// three above it needs no SDL_GL_GetProcAddress load -- same convention every existing call site
// (shader classes and standalone files alike) already relies on.
static bool LoadBindStateFunctions()
{
    static bool loaded = false;
    if (loaded) return true;

    fn_glUseProgram      = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");
    fn_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)SDL_GL_GetProcAddress("glBindVertexArray");
    fn_glActiveTexture   = (PFNGLACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glActiveTexture");

    loaded = (fn_glUseProgram != nullptr && fn_glBindVertexArray != nullptr && fn_glActiveTexture != nullptr);
    return loaded;
}

// DXP-22 step 5: last-bound-id cache. Safe now that step 3 converted every real call site in the
// tree (grep-verified zero raw glUseProgram/glBindTexture(GL_TEXTURE_2D)/glBindVertexArray calls
// remain outside this file) -- nothing can change GL's actual bind state behind this cache's back
// for these three targets. Initial cache values of 0 match GL's real post-context-creation default
// (program 0, VAO 0, texture 0 on every unit), so the very first real call from any caller still
// goes through correctly.
static GLuint s_lastProgram = 0;
static GLuint s_lastVAO     = 0;

// Texture id cache is keyed per slot -- BMDMeshShader alone uses slots 0-3 (base/chrome1/metal/
// chrome2); sized with headroom.
static const int kMaxCachedTextureSlots = 8;
static GLuint    s_lastTexture[kMaxCachedTextureSlots] = { 0 };

void BindProgram(GLuint program)
{
    if (program == s_lastProgram) return;
    if (!LoadBindStateFunctions()) return;
    fn_glUseProgram(program);
    s_lastProgram = program;
}

void UnbindAllShaders()
{
    BindProgram(0);
}

void BindTexture2D(int slot, GLuint texture)
{
    if (!LoadBindStateFunctions()) return;

    // glActiveTexture is deliberately NEVER cached/skipped here, only the per-slot bound texture id
    // is. TerrainShader::SetOverlayTexture and PassthroughShader::SetTexture both still call a raw
    // (uncached) fn_glActiveTexture(GL_TEXTURE0) restore after binding a non-zero slot -- if this
    // function also cached "current active unit" and skipped re-issuing it on a match, those raw
    // restores would silently desync the cache from real GL state, causing a later same-slot bind to
    // wrongly skip the unit switch and bind the wrong texture to the wrong unit. glActiveTexture
    // itself is a cheap state-pointer flip (unlike glBindTexture, which is the call actually measured
    // as redundant in the DXP-22 RenderDoc capture), so unconditionally reissuing it costs little and
    // avoids this whole class of bug.
    fn_glActiveTexture(GL_TEXTURE0 + slot);

    if (slot >= 0 && slot < kMaxCachedTextureSlots && s_lastTexture[slot] == texture) return;
    glBindTexture(GL_TEXTURE_2D, texture);
    if (slot >= 0 && slot < kMaxCachedTextureSlots) s_lastTexture[slot] = texture;
}

void BindVAO(GLuint vao)
{
    if (vao == s_lastVAO) return;
    if (!LoadBindStateFunctions()) return;
    fn_glBindVertexArray(vao);
    s_lastVAO = vao;
}

// Sentinel that can never be a real GL object name (0 is reserved for "no object"; real generated
// names start at 1) -- forces the next Bind* call, whatever id it requests, to always issue the real
// GL call and refresh the cache from scratch.
static const GLuint kInvalidCacheSentinel = 0xFFFFFFFFu;

void InvalidateProgramCache()
{
    s_lastProgram = kInvalidCacheSentinel;
}

void InvalidateTextureCache()
{
    for (int i = 0; i < kMaxCachedTextureSlots; ++i) s_lastTexture[i] = kInvalidCacheSentinel;
}

void InvalidateVAOCache()
{
    s_lastVAO = kInvalidCacheSentinel;
}
