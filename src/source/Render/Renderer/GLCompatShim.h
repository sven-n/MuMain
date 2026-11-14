#pragma once

#include <cstdint>

using MUCompatGLenum = unsigned int;
using MUCompatGLbitfield = unsigned int;
using MUCompatGLboolean = unsigned char;
using MUCompatGLsizei = int;
using MUCompatGLint = int;
using MUCompatGLuint = unsigned int;
using MUCompatGLfloat = float;
using MUCompatGLdouble = double;
using MUCompatGLubyte = unsigned char;

// The GL type and constant names the game code, the bitmap loaders and this shim use, so no desktop GL header
// is needed.
// Spelled as GL.h spells them: a file that also sees GL.h gets identical redefinitions.
using GLuint = MUCompatGLuint;
using GLfloat = MUCompatGLfloat;

#define GL_ADD 0x0104
#define GL_ALPHA_TEST 0x0BC0
#define GL_ALWAYS 0x0207
#define GL_BLEND 0x0BE2
#define GL_CCW 0x0901
#define GL_CLAMP 0x2900
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_CULL_FACE 0x0B44
#define GL_CW 0x0900
#define GL_DECR 0x1E03
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_DEPTH_TEST 0x0B71
#define GL_ENABLE_BIT 0x00002000
#define GL_EXP 0x0800
#define GL_EXP2 0x0801
#define GL_FALSE 0
#define GL_FILL 0x1B02
#define GL_FOG 0x0B60
#define GL_FOG_COLOR 0x0B66
#define GL_FRONT 0x0404
#define GL_GREATER 0x0204
#define GL_INCR 0x1E02
#define GL_KEEP 0x1E00
#define GL_LEQUAL 0x0203
#define GL_LESS 0x0201
#define GL_LIGHTING 0x0B50
#define GL_LINE 0x1B01
#define GL_LINEAR 0x2601
#define GL_LINES 0x0001
#define GL_LINE_BIT 0x00000004
#define GL_LINE_LOOP 0x0002
#define GL_MODELVIEW 0x1700
#define GL_MODELVIEW_MATRIX 0x0BA6
#define GL_MODULATE 0x2100
#define GL_NEAREST 0x2600
#define GL_ONE 1
#define GL_ONE_MINUS_DST_COLOR 0x0307
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_PROJECTION 0x1701
#define GL_PROJECTION_MATRIX 0x0BA7
#define GL_QUADS 0x0007
#define GL_QUAD_STRIP 0x0008
#define GL_REPEAT 0x2901
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_SRC_ALPHA 0x0302
#define GL_SRC_COLOR 0x0300
#define GL_STENCIL_TEST 0x0B90
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_ENV 0x2300
#define GL_TEXTURE_ENV_MODE 0x2200
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_FAN 0x0006
#define GL_TRUE 1
#define GL_UNSIGNED_BYTE 0x1401
#define GL_ZERO 0

void mu_glBegin(MUCompatGLenum mode);
void mu_glEnd();
void mu_glVertex2f(MUCompatGLfloat x, MUCompatGLfloat y);
void mu_glVertex3f(MUCompatGLfloat x, MUCompatGLfloat y, MUCompatGLfloat z);
void mu_glVertex3fv(const MUCompatGLfloat* v);
void mu_glTexCoord2f(MUCompatGLfloat u, MUCompatGLfloat v);
void mu_glColor3f(MUCompatGLfloat r, MUCompatGLfloat g, MUCompatGLfloat b);
void mu_glColor3fv(const MUCompatGLfloat* c);
void mu_glColor3ub(MUCompatGLubyte r, MUCompatGLubyte g, MUCompatGLubyte b);
void mu_glColor4f(MUCompatGLfloat r, MUCompatGLfloat g, MUCompatGLfloat b, MUCompatGLfloat a);
void mu_glColor4ub(MUCompatGLubyte r, MUCompatGLubyte g, MUCompatGLubyte b, MUCompatGLubyte a);
void mu_glEnable(MUCompatGLenum cap);
void mu_glDisable(MUCompatGLenum cap);
MUCompatGLboolean mu_glIsEnabled(MUCompatGLenum cap);
void mu_glBlendFunc(MUCompatGLenum sfactor, MUCompatGLenum dfactor);
void mu_glClear(MUCompatGLbitfield mask);
void mu_glClearColor(MUCompatGLfloat r, MUCompatGLfloat g, MUCompatGLfloat b, MUCompatGLfloat a);
void mu_glDepthFunc(MUCompatGLenum func);
void mu_glDepthMask(MUCompatGLboolean flag);
void mu_glMatrixMode(MUCompatGLenum mode);
void mu_glPushMatrix();
void mu_glPopMatrix();
void mu_glLoadIdentity();
void mu_glTranslatef(MUCompatGLfloat x, MUCompatGLfloat y, MUCompatGLfloat z);
void mu_glRotatef(MUCompatGLfloat angle, MUCompatGLfloat x, MUCompatGLfloat y, MUCompatGLfloat z);
void mu_glScalef(MUCompatGLfloat x, MUCompatGLfloat y, MUCompatGLfloat z);
void mu_glGetFloatv(MUCompatGLenum pname, MUCompatGLfloat* data);
void mu_glGetIntegerv(MUCompatGLenum pname, MUCompatGLint* data);
void mu_glViewport(MUCompatGLint x, MUCompatGLint y, MUCompatGLsizei width, MUCompatGLsizei height);
void mu_glLineWidth(MUCompatGLfloat width);
void mu_glBindTexture(MUCompatGLenum target, MUCompatGLuint texture);
void mu_glGenTextures(MUCompatGLsizei n, MUCompatGLuint* textures);
void mu_glDeleteTextures(MUCompatGLsizei n, const MUCompatGLuint* textures);
void mu_glTexImage2D(MUCompatGLenum target, MUCompatGLint level, MUCompatGLint internalFormat, MUCompatGLsizei width,
                     MUCompatGLsizei height, MUCompatGLint border, MUCompatGLenum format, MUCompatGLenum type,
                     const void* pixels);
void mu_glTexSubImage2D(MUCompatGLenum target, MUCompatGLint level, MUCompatGLint xoffset, MUCompatGLint yoffset,
                        MUCompatGLsizei width, MUCompatGLsizei height, MUCompatGLenum format, MUCompatGLenum type,
                        const void* pixels);
void mu_glTexParameteri(MUCompatGLenum target, MUCompatGLenum pname, MUCompatGLint param);
void mu_glTexEnvi(MUCompatGLenum target, MUCompatGLenum pname, MUCompatGLint param);
void mu_glTexEnvf(MUCompatGLenum target, MUCompatGLenum pname, MUCompatGLfloat param);
void mu_glAlphaFunc(MUCompatGLenum func, MUCompatGLfloat ref);
void mu_glStencilFunc(MUCompatGLenum func, MUCompatGLint ref, MUCompatGLuint mask);
void mu_glStencilOp(MUCompatGLenum sfail, MUCompatGLenum dpfail, MUCompatGLenum dppass);
void mu_glColorMask(MUCompatGLboolean r, MUCompatGLboolean g, MUCompatGLboolean b, MUCompatGLboolean a);
void mu_glPolygonMode(MUCompatGLenum face, MUCompatGLenum mode);
void mu_glFrontFace(MUCompatGLenum mode);
void mu_glFogf(MUCompatGLenum pname, MUCompatGLfloat param);
void mu_glFogi(MUCompatGLenum pname, MUCompatGLint param);
void mu_glFogfv(MUCompatGLenum pname, const MUCompatGLfloat* params);
void mu_glReadPixels(MUCompatGLint x, MUCompatGLint y, MUCompatGLsizei width, MUCompatGLsizei height,
                     MUCompatGLenum format, MUCompatGLenum type, void* pixels);
const MUCompatGLubyte* mu_glGetString(MUCompatGLenum name);
void mu_gluPerspective(MUCompatGLdouble fovy, MUCompatGLdouble aspect, MUCompatGLdouble zNear, MUCompatGLdouble zFar);
void mu_gluOrtho2D(MUCompatGLdouble left, MUCompatGLdouble right, MUCompatGLdouble bottom, MUCompatGLdouble top);
void* mu_gluNewQuadric();
void mu_gluSphere(void* quadric, MUCompatGLdouble radius, MUCompatGLint slices, MUCompatGLint stacks);

void mu_glEnableClientState(MUCompatGLenum array);
void mu_glDisableClientState(MUCompatGLenum array);
void mu_glVertexPointer(MUCompatGLint size, MUCompatGLenum type, MUCompatGLsizei stride, const void* pointer);
void mu_glTexCoordPointer(MUCompatGLint size, MUCompatGLenum type, MUCompatGLsizei stride, const void* pointer);
void mu_glColorPointer(MUCompatGLint size, MUCompatGLenum type, MUCompatGLsizei stride, const void* pointer);
void mu_glDrawArrays(MUCompatGLenum mode, MUCompatGLint first, MUCompatGLsizei count);
void mu_glReadBuffer(MUCompatGLenum mode);
void mu_glFlush();
void mu_glPixelStorei(MUCompatGLenum pname, MUCompatGLint param);
void mu_glPushAttrib(MUCompatGLbitfield mask);
void mu_glPopAttrib();
void mu_glNormal3f(MUCompatGLfloat x, MUCompatGLfloat y, MUCompatGLfloat z);
void mu_glCopyTexImage2D(MUCompatGLenum target, MUCompatGLint level, MUCompatGLenum internalFormat, MUCompatGLint x,
                         MUCompatGLint y, MUCompatGLsizei width, MUCompatGLsizei height, MUCompatGLint border);

#define glBegin mu_glBegin
#define glEnd mu_glEnd
#define glVertex2f mu_glVertex2f
#define glVertex3f mu_glVertex3f
#define glVertex3fv mu_glVertex3fv
#define glTexCoord2f mu_glTexCoord2f
#define glColor3f mu_glColor3f
#define glColor3fv mu_glColor3fv
#define glColor3ub mu_glColor3ub
#define glColor4f mu_glColor4f
#define glColor4ub mu_glColor4ub
#define glEnable mu_glEnable
#define glDisable mu_glDisable
#define glIsEnabled mu_glIsEnabled
#define glBlendFunc mu_glBlendFunc
#define glClear mu_glClear
#define glClearColor mu_glClearColor
#define glDepthFunc mu_glDepthFunc
#define glDepthMask mu_glDepthMask
#define glMatrixMode mu_glMatrixMode
#define glPushMatrix mu_glPushMatrix
#define glPopMatrix mu_glPopMatrix
#define glLoadIdentity mu_glLoadIdentity
#define glTranslatef mu_glTranslatef
#define glRotatef mu_glRotatef
#define glScalef mu_glScalef
#define glGetFloatv mu_glGetFloatv
#define glGetIntegerv mu_glGetIntegerv
#define glViewport mu_glViewport
#define glLineWidth mu_glLineWidth
#define glBindTexture mu_glBindTexture
#define glGenTextures mu_glGenTextures
#define glDeleteTextures mu_glDeleteTextures
#define glTexImage2D mu_glTexImage2D
#define glTexSubImage2D mu_glTexSubImage2D
#define glTexParameteri mu_glTexParameteri
#define glTexEnvi mu_glTexEnvi
#define glTexEnvf mu_glTexEnvf
#define glAlphaFunc mu_glAlphaFunc
#define glStencilFunc mu_glStencilFunc
#define glStencilOp mu_glStencilOp
#define glColorMask mu_glColorMask
#define glPolygonMode mu_glPolygonMode
#define glFrontFace mu_glFrontFace
#define glFogf mu_glFogf
#define glFogi mu_glFogi
#define glFogfv mu_glFogfv
#define glReadPixels mu_glReadPixels
#define glGetString mu_glGetString
#define gluPerspective mu_gluPerspective
#define gluOrtho2D mu_gluOrtho2D
#define gluNewQuadric mu_gluNewQuadric
#define gluSphere mu_gluSphere
#define glEnableClientState mu_glEnableClientState
#define glDisableClientState mu_glDisableClientState
#define glVertexPointer mu_glVertexPointer
#define glTexCoordPointer mu_glTexCoordPointer
#define glColorPointer mu_glColorPointer
#define glDrawArrays mu_glDrawArrays
#define glReadBuffer mu_glReadBuffer
#define glFlush mu_glFlush
#define glPixelStorei mu_glPixelStorei
#define glPushAttrib mu_glPushAttrib
#define glPopAttrib mu_glPopAttrib
#define glNormal3f mu_glNormal3f
#define glCopyTexImage2D mu_glCopyTexImage2D
