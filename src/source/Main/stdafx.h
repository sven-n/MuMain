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

#include "Platform/PlatformCompat.h"

// GL type definitions — unconditional (SDL_gpu is the only backend).
// Story 7.9.3: MU_USE_OPENGL_BACKEND flag and GLEW includes removed.
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
typedef float GLclampf;
typedef unsigned int GLbitfield;

// OpenGL constants for SDL3 GPU backend.
// SDL3 uses SDL_gpu (Metal/Vulkan/D3D12), not OpenGL.
// All GL function calls are routed through MuRenderer (story 7-9-6).
// Constants are retained for MuRenderer API parameters.

// OpenGL Constants
#define GL_FALSE 0
#define GL_TRUE 1
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
#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_LINE 0x1B01
#define GL_FILL 0x1B02
#define GL_STENCIL_TEST 0x0B90
#define GL_ALWAYS 0x0207
#define GL_CCW 0x0901
#define GL_LEQUAL 0x0203
#define GL_KEEP 0x1E00
#define GL_TRIANGLE_FAN 0x0006
#define GL_LESS 0x0201
#define GL_INCR 0x1E01
#define GL_CW 0x0900
#define GL_DECR 0x1E03
#define GL_MODULATE 0x2100
#define GL_REPLACE 0x1E01
#define GL_BLEND_DST 0x0BE1
#define GL_BLEND_SRC 0x0BE0
#define GL_ONE 1
#define GL_ZERO 0
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_EQUAL 0x0202
#define GL_NOTEQUAL 0x0205
#define GL_GREATER 0x0204
#define GL_GEQUAL 0x0206
#define GL_SCISSOR_TEST 0x0C11
#define GL_CULL_FACE_MODE 0x0B45
#define GL_VIEWPORT 0x0BA2
#define GL_TEXTURE_GEN_S 0x0C60
#define GL_TEXTURE_GEN_T 0x0C61
#define GL_TEXTURE_GEN_MODE 0x2500
#define GL_REFLECTION_MAP 0x8512
#define GL_NORMAL_ARRAY 0x8075
#define GL_COLOR_ARRAY 0x8076
#define GL_TEXTURE_COORD_ARRAY 0x8078
#define GL_VERTEX_ARRAY 0x8074
#define GL_FRONT_AND_BACK 0x0408
#define GL_AMBIENT 0x1200
#define GL_DIFFUSE 0x1201
#define GL_SPECULAR 0x1202
#define GL_EMISSION 0x1200
#define GL_SHININESS 0x1601
#define GL_AMBIENT_AND_DIFFUSE 0x1602
#define GL_SMOOTH 0x1D01
#define GL_FLAT 0x1D00
#define GL_PERSPECTIVE_CORRECTION_HINT 0x0C50
#define GL_NICEST 0x1102
#define GL_PACK_ALIGNMENT 0x0D05
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_TEXTURE_ENV 0x2300
#define GL_TEXTURE_ENV_MODE 0x2200
#define GL_BACK_LEFT 0x0402
#define GL_FOG 0x0B60
#define GL_FOG_COLOR 0x0B66
#define GL_FOG_MODE 0x0B65
#define GL_FOG_DENSITY 0x0B62
#define GL_FOG_START 0x0B63
#define GL_FOG_END 0x0B64
#define GL_EXP 0x0800
#define GL_EXP2 0x0801
#define GL_QUAD_STRIP 0x0008
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_ONE_MINUS_DST_COLOR 0x0306
#define GL_VENDOR 0x1F00
#define GL_RENDERER 0x1F01
#define GL_VERSION 0x1F02
#define GL_EXTENSIONS 0x1F03
#define GL_MAX_TEXTURE_SIZE 0x0D33
#define GL_MAX_VIEWPORT_DIMS 0x0D3A
#define GL_REPEAT 0x2901
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_LINES 0x0001
#define GL_LIGHTING 0x0B50
#define GL_LIGHT0 0x4000
#define GL_LIGHT1 0x4001
#define GL_POSITION 0x1203
#define GL_LIGHT_MODEL_AMBIENT 0x0B53
#define GL_POLYGON 0x0009
#define GL_NORMALIZE 0x0BA1
#define GL_COLOR_MATERIAL 0x0B57
#define GL_COMPILE 0x1300
#define GL_MODELVIEW 0x1700
#define GL_PROJECTION 0x1701
#define GL_TEXTURE 0x1702
#define GL_MODELVIEW_MATRIX 0x0BA6
#define GL_PROJECTION_MATRIX 0x0BA7
#define GL_DEPTH_COMPONENT 0x1902
#define GL_FLOAT 0x1406
#define GL_ADD 0x0104
#define GL_DECAL 0x2101
#define GL_COMBINE 0x8570
#define GL_COMBINE_RGB 0x8571
#define GL_COMBINE_ALPHA 0x8572
#define GL_SOURCE0_RGB 0x8580
#define GL_SOURCE1_RGB 0x8581
#define GL_OPERAND0_RGB 0x8590
#define GL_OPERAND1_RGB 0x8591
#define GL_SRC_COLOR 0x0300
#define GL_PREVIOUS 0x8578
#define GL_INTERPOLATE 0x8575
#define GL_SOURCE0_ALPHA 0x8588
#define GL_SOURCE1_ALPHA 0x8589
#define GL_OPERAND0_ALPHA 0x8598
#define GL_OPERAND1_ALPHA 0x8599
#define GL_SRC_ALPHA_SATURATE 0x0308
#define GL_CONSTANT 0x8576
#define GL_TEXTURE_ENV_COLOR 0x2201
#define GL_LIGHT_MODEL_TWO_SIDE 0x0B52
#define GL_COLOR_MATERIAL_FACE 0x0B55
#define GL_INT 0x1404
#define GL_DOUBLE 0x140A
#define GL_BYTE 0x1400
#define GL_UNSIGNED_INT 0x1405
#define GL_CLAMP 0x2900

// client - base definitions
#include "Core/Defined_Global.h"
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

#include "Core/MuLogger.h"
#include "Core/WindowsConsole.h"

// Redirect console output to ImGui when editor is enabled
#ifdef _EDITOR
#include "../MuEditor/UI/Console/MuEditorConsoleRedirectUI.h"
#endif

#include "w_MapHeaders.h"

#include "_crypt.h"

// Story 7.6.1: Game headers that many TUs depend on via transitive includes.
// On Windows, these are pulled in through include chains that differ on macOS.
// Adding them to the PCH ensures all symbols are available on all platforms.
// [VS0-QUAL-BUILDCOMP-MACOS]
#include "Main/MuMain.h"
#include "Data/ZzzInfomation.h"
#include "Scenes/SceneCore.h"
#include "Core/_GlobalFunctions.h"

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
// Guard prevents redefinition when PlatformCompat.h already defined these
#ifndef MU_SWPRINTF_DEFINED
#define MU_SWPRINTF_DEFINED
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
#endif // MU_SWPRINTF_DEFINED
#endif

inline std::wstring g_strSelectedML = L"";
inline float g_fScreenRate_x = 0;
inline float g_fScreenRate_y = 0;
