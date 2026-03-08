// stdafx.h : include file for standard system include files,
#pragma once

// warining
#pragma warning(disable : 4067)
#pragma warning(disable : 4786)
#pragma warning(disable : 4800)
#pragma warning(disable : 4996)
#pragma warning(disable : 4244)
#pragma warning(disable : 4237)
#pragma warning(disable : 4305)
#pragma warning(disable : 4503)
#pragma warning(disable : 4267)
#pragma warning(disable : 4091)
#pragma warning(disable : 4819)
#pragma warning(disable : 4505)
#pragma warning(disable : 4100)
#pragma warning(disable : 4127)
#pragma warning(disable : 4702)
#pragma warning(disable : 4838)
#pragma warning(disable : 5208)
// #pragma warning( disable : 4482 )
// #pragma warning( disable : 4700 )
// #pragma warning( disable : 4748 )
// #pragma warning( disable : 4786 )
#pragma warning(disable : 28159)
#pragma warning(disable : 26812)

#define NOMINMAX

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN

// Use 32-bit time_t only for 32-bit MSVC builds. This avoids conflicts
// with 64-bit toolchains such as x86_64-w64-mingw32, which require
// 64-bit time_t.
#if defined(_MSC_VER) && !defined(_WIN64)
#ifndef _USE_32BIT_TIME_T
#define _USE_32BIT_TIME_T
#endif //_USE_32BIT_TIME_T
#endif

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE

#include <cstdint>
#include <climits>
#include <cstring>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <algorithm>
#include <memory>
#include <iostream>
#include <deque>
#include <queue>
#include <filesystem>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <cwctype>
#include <cmath>
#include <cstdarg>
#include <ctime>
#include <cassert>

#ifdef _WIN32
#include <windows.h>
#else
#include "Platform/PlatformTypes.h"
#include "Platform/PlatformCompat.h"
#endif

#ifdef _WIN32
#include <gl/glew.h>
#include <gl/GL.h>
#else
#ifdef MU_ENABLE_SDL3
#include <SDL3/SDL_opengl.h>
#else
// Safe OpenGL fallback when SDL3 is not available
typedef unsigned int GLuint;
typedef int GLint;
typedef float GLfloat;
typedef unsigned char GLubyte;
typedef double GLdouble;
typedef unsigned int GLenum;
typedef int GLsizei;
typedef unsigned char GLboolean;
typedef signed char GLbyte;
typedef short GLshort;
typedef unsigned short GLushort;
typedef int GLfixed;
typedef int64_t GLint64;
typedef uint64_t GLuint64;
typedef void GLvoid;

// OpenGL Constants fallback
#define GL_DEPTH_TEST 0x0B71
#define GL_ALPHA_TEST 0x0BC0
#define GL_TEXTURE_2D 0x0DE1
#define GL_QUADS 0x0007
#define GL_TRIANGLES 0x0004
#define GL_LINE_STRIP 0x0003
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_RGBA 0x1908
#define GL_RGB 0x1907
#define GL_UNSIGNED_BYTE 0x1401
#define GL_BLEND 0x0BE2
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_CULL_FACE 0x0B44

// OpenGL Function stubs
inline void glEnable(GLenum) {}
inline void glDisable(GLenum) {}
inline void glBegin(GLenum) {}
inline void glEnd() {}
inline void glColor4f(GLfloat, GLfloat, GLfloat, GLfloat) {}
inline void glColor3f(GLfloat, GLfloat, GLfloat) {}
inline void glVertex3f(GLfloat, GLfloat, GLfloat) {}
inline void glTexCoord2f(GLfloat, GLfloat) {}
inline void glNormal3f(GLfloat, GLfloat, GLfloat) {}
inline void glFlush() {}
inline void glBindTexture(GLenum, GLuint) {}
inline void glGetIntegerv(GLenum, GLint*) {}
#endif // MU_ENABLE_SDL3
#endif // _WIN32

// client - base definitions
#define PBG_ADD_INGAMESHOPMSGBOX
#define ASG_ADD_GENS_MARK
#define PBG_ADD_GENSRANKING

#include "Core/mu_base_types.h"
#include "Core/mu_define.h"
#include "Core/mu_enum.h"
#include "Data/_TextureIndex.h"
#include "Core/mu_types.h"
#include "Core/mu_struct.h"
#include "w_WindowMessageHandler.h"
#include "ZzzOpenglUtil.h"
#include "Core/UsefulDef.h"
#include "NewUICommon.h"

#include "MultiLanguage.h"

#include "Core/muConsoleDebug.h"
#include "Core/ErrorReport.h"
#include "Core/WindowsConsole.h"

// Redirect console output to ImGui when editor is enabled
#ifdef _EDITOR
#include "../MuEditor/UI/Console/MuEditorConsoleRedirectUI.h"
#endif

#include "w_MapHeaders.h"

#include "_crypt.h"

#ifdef _MSC_VER
#define mu_wcstok wcstok_s
#else
#define mu_wcstok(str, delim, ctx) wcstok(str, delim, ctx)
#endif

#ifdef _MSC_VER
// MSVC: swprintf doesn't need buffer size
template <typename... Args> inline int mu_swprintf(wchar_t* buffer, const wchar_t* format, Args... args)
{
    return swprintf(buffer, format, args...);
}
// mu_swprintf_s with explicit size
template <typename... Args> inline int mu_swprintf_s(wchar_t* buffer, size_t size, const wchar_t* format, Args... args)
{
    return swprintf_s(buffer, size, format, args...);
}
// mu_swprintf_s with array - auto-deduce size (like MSVC's swprintf_s)
template <size_t N, typename... Args>
inline int mu_swprintf_s(wchar_t (&buffer)[N], const wchar_t* format, Args... args)
{
    return swprintf_s(buffer, N, format, args...);
}
#else
// GCC/MinGW/Clang: use std::swprintf with explicit buffer size
template <typename... Args> inline int mu_swprintf(wchar_t* buffer, const wchar_t* format, Args... args)
{
    return std::swprintf(buffer, 1024, format, args...);
}
// mu_swprintf_s with explicit size
template <typename... Args> inline int mu_swprintf_s(wchar_t* buffer, size_t size, const wchar_t* format, Args... args)
{
    return std::swprintf(buffer, size, format, args...);
}
// mu_swprintf_s with array - auto-deduce size (compatible with MSVC swprintf_s)
template <size_t N, typename... Args>
inline int mu_swprintf_s(wchar_t (&buffer)[N], const wchar_t* format, Args... args)
{
    return std::swprintf(buffer, N, format, args...);
}
#endif

#ifndef _WIN32
extern HWND g_hWnd;
#endif

inline std::wstring g_strSelectedML = L"";
inline float g_fScreenRate_x = 0;
inline float g_fScreenRate_y = 0;
