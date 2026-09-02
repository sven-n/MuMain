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
