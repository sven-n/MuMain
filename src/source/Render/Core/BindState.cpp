#include "stdafx.h"
#include "BindState.h"
#include "Core/Utilities/FrameProfiler.h"
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

// GLP-05: matches GL's post-context-creation default (GL_TEXTURE0, i.e. slot 0).
static GLint s_lastActiveSlot = 0;

void BindProgram(GLuint program)
{
    if (program == s_lastProgram) return;
    if (!LoadBindStateFunctions()) return;
    fn_glUseProgram(program);
    s_lastProgram = program;
    FrameProfiler::CountGLCall(FrameProfiler::Counter::ProgramBinds);
}

void UnbindAllShaders()
{
    BindProgram(0);
}

void BindTexture2D(int slot, GLuint texture)
{
    if (!LoadBindStateFunctions()) return;

    // GLP-05: BindState is now the complete monopoly on the active texture unit, not just
    // program/VAO/per-slot-texture-id. This is only sound because TerrainShader::SetOverlayTexture
    // and PassthroughShader::SetTexture's raw (uncached) fn_glActiveTexture(GL_TEXTURE0) restores
    // were deleted in the same commit that added this cache -- a raw glActiveTexture anywhere in
    // the tree desyncs s_lastActiveSlot from real GL state, causing a later same-slot bind to
    // wrongly skip the unit switch and bind the wrong texture to the wrong unit (see DXP-22's
    // "infinity shadow" for what that class of bug looks like in practice). If you are adding a
    // texture bind anywhere, route it through BindTexture2D -- do not call glActiveTexture directly.
    if (slot != s_lastActiveSlot) {
        fn_glActiveTexture(GL_TEXTURE0 + slot);
        s_lastActiveSlot = slot;
    }

    if (slot >= 0 && slot < kMaxCachedTextureSlots && s_lastTexture[slot] == texture) return;
    glBindTexture(GL_TEXTURE_2D, texture);
    if (slot >= 0 && slot < kMaxCachedTextureSlots) s_lastTexture[slot] = texture;
    FrameProfiler::CountGLCall(FrameProfiler::Counter::TextureBinds);
}

void BindVAO(GLuint vao)
{
    if (vao == s_lastVAO) return;
    if (!LoadBindStateFunctions()) return;
    fn_glBindVertexArray(vao);
    s_lastVAO = vao;
    FrameProfiler::CountGLCall();
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

void InvalidateActiveTextureUnitCache()
{
    // -1 rather than kInvalidCacheSentinel (that constant is sized for GLuint object names, not
    // a small non-negative slot index) -- no real slot argument is ever negative, so this can
    // never accidentally match and skip the next real glActiveTexture call.
    s_lastActiveSlot = -1;
}
