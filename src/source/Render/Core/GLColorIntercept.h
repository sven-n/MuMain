#pragma once

#include "Render/Core/RenderConfig.h"

// DXP-08a Category 1: FFP "current color" bridge. Under Core Profile, glColor* is a no-op
// deprecated call and glGetFloatv(GL_CURRENT_COLOR, ...) is itself illegal, so nothing can
// read back "whatever was last set" the way ~340 RenderColor()/RenderBitmap()-family call
// sites throughout the legacy UI code rely on (a per-site sweep of all of them turned out
// to be impractical -- see DXP-08a's task file). These wrappers mirror every glColor* call
// into g_CurrentColor, which those functions read instead of the illegal GL query, while
// leaving every existing call site's source text untouched via macro substitution. Under
// Compat profile the real GL call still fires too, so behavior there is unchanged --
// immediate-mode glBegin/glEnd blocks that consume the real FFP color state keep working
// exactly as before.
//
// 3-component variants (glColor3f/3fv/3ub) only touch RGB, matching real GL semantics --
// alpha is left as whatever it already was.
//
// Ordering matters: the real ::glColor*() calls below must be written before the #define
// lines at the bottom of this file, or the preprocessor would recursively expand them too.
//
inline bool DXP_GLColorShouldSkipRealCall()
{
    return g_CoreProfile;
}

inline void DXP_GLColor3f(GLfloat r, GLfloat g, GLfloat b)
{
    g_CurrentColor[0] = r; g_CurrentColor[1] = g; g_CurrentColor[2] = b;
    if (!DXP_GLColorShouldSkipRealCall()) ::glColor3f(r, g, b);
}

inline void DXP_GLColor3fv(const GLfloat* v)
{
    g_CurrentColor[0] = v[0]; g_CurrentColor[1] = v[1]; g_CurrentColor[2] = v[2];
    if (!DXP_GLColorShouldSkipRealCall()) ::glColor3fv(v);
}

inline void DXP_GLColor3ub(GLubyte r, GLubyte g, GLubyte b)
{
    g_CurrentColor[0] = r / 255.f; g_CurrentColor[1] = g / 255.f; g_CurrentColor[2] = b / 255.f;
    if (!DXP_GLColorShouldSkipRealCall()) ::glColor3ub(r, g, b);
}

inline void DXP_GLColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    g_CurrentColor[0] = r; g_CurrentColor[1] = g; g_CurrentColor[2] = b; g_CurrentColor[3] = a;
    if (!DXP_GLColorShouldSkipRealCall()) ::glColor4f(r, g, b, a);
}

inline void DXP_GLColor4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    g_CurrentColor[0] = r / 255.f; g_CurrentColor[1] = g / 255.f; g_CurrentColor[2] = b / 255.f; g_CurrentColor[3] = a / 255.f;
    if (!DXP_GLColorShouldSkipRealCall()) ::glColor4ub(r, g, b, a);
}

#define glColor3f(r, g, b)     DXP_GLColor3f(r, g, b)
#define glColor3fv(v)          DXP_GLColor3fv(v)
#define glColor3ub(r, g, b)    DXP_GLColor3ub(r, g, b)
#define glColor4f(r, g, b, a)  DXP_GLColor4f(r, g, b, a)
#define glColor4ub(r, g, b, a) DXP_GLColor4ub(r, g, b, a)
