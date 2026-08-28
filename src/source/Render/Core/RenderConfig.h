#pragma once

// DXP-08: Core Profile GL context flip, read once at startup (Winmain creates the
// GL context immediately after InitRenderConfig() runs, before anything else can
// observe this). true = core 3.3 (default since DXP-08 Stage G), false = compatibility
// profile (rollback path). Set CoreProfile=0 in config.ini [Render] to opt back into
// compatibility.
extern bool g_CoreProfile;

// GLP-08: optional ceiling on the requested core-profile GL context version, read from
// config.ini [Render] MaxGLVersion (e.g. "4.3"). 0 = no cap (try the highest core context
// available -- Winmain.cpp's descending {4,5}->{4,3}->{3,3} attempt loop starts at the top).
// Rollback path for a driver that mishandles that loop. Ignored when g_CoreProfile is false.
extern int g_MaxGLVersionMajor;
extern int g_MaxGLVersionMinor;

// Fixed-function alpha test (GL_ALPHA_TEST/glAlphaFunc) state, mirrored for shader-side
// `discard` (DXP-01). g_AlphaRef is what shaders should test against: -1.0f means alpha
// test is currently disabled (no discard), otherwise it's the active GL_GREATER threshold.
// g_AlphaFuncRef is the last threshold passed to SetAlphaFuncRef(), independent of whether
// alpha test is currently enabled — reapplied to g_AlphaRef the next time it's enabled.
extern float g_AlphaRef;
extern float g_AlphaFuncRef;

// Groups particle draws by texture before submitting them, read from config.ini [Render]
// SortParticleDraws. Particles are walked in slot order, which interleaves textures and so
// breaks the IR batch on nearly every sprite; grouping them lets consecutive sprites merge
// into one draw. Only runs whose result cannot depend on draw order are reordered (see
// ZzzEffectParticle.cpp), so this is behaviour-preserving in principle -- it stays opt-in
// until that has been confirmed by eye against real effects.
extern bool g_SortParticleDraws;

// Vsync flag mirroring SDL's actual swap interval (ZzzOpenglUtil.cpp's EnableVSync/
// DisableVSync call SDL_GL_SetSwapInterval). Default true matches EnableVSync()'s
// unconditional call at boot (Winmain.cpp).
extern bool g_VSyncEnabled;

// DXP-08a Category 1: CPU-tracked mirror of the FFP "current color" any glColor* call used
// to set (RGBA, 0-1 range). Under Core Profile glColor* no-ops and glGetFloatv(GL_CURRENT_
// COLOR, ...) is itself an illegal call, so callers that used to rely on reading back the
// ambient FFP color state now write here instead. Default {1,1,1,1} matches GL's own default.
extern float g_CurrentColor[4];

void InitRenderConfig();

// Shared closed-form perspective projection builder (GL's post-divide NDC z convention,
// [-1,1]). `f` is the caller's precomputed `1/tan(fovY/2)`; `out` is row-major matching this
// codebase's existing GlobalUBO layout (same layout the old glGetFloatv-derived matrices used).
void BuildPerspectiveProjection(float f, float aspect, float zNear, float zFar, float out[16]);
