// stdafx.h : include file for standard system include files,
#pragma once

//warining
#pragma warning( disable : 4067 )
#pragma warning( disable : 4786 )
#pragma warning( disable : 4800 )
#pragma warning( disable : 4996 )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4237 )
#pragma warning( disable : 4305 )
#pragma warning( disable : 4503 )
#pragma warning( disable : 4267 )
#pragma warning( disable : 4091 )
#pragma warning( disable : 4819 )
#pragma warning( disable : 4505 )
#pragma warning( disable : 4100 )
#pragma warning( disable : 4127 )
#pragma warning( disable : 4702 )
#pragma warning( disable : 4838 )
#pragma warning( disable : 5208 )
//#pragma warning( disable : 4482 )
//#pragma warning( disable : 4700 )
//#pragma warning( disable : 4748 )
//#pragma warning( disable : 4786 )
#pragma warning( disable : 28159 )
#pragma warning( disable : 26812 )

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

#pragma warning( push, 3 )

#include <windows.h>

// MinGW workaround: Allow swprintf usage
#if defined(__MINGW32__) || defined(__MINGW64__)
#undef swprintf
#endif

//windows
#include <winsock2.h>
#include <mmsystem.h>
#include <shellapi.h>

//c runtime
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <assert.h>
#include <mbstring.h>
#include <time.h>
#include <math.h>
#include <stdarg.h>
#include <conio.h>

#include <string>
#include <list>
#include <map>
#include <deque>
#include <algorithm>
#include <vector>
#include <queue>

#pragma warning( pop )

// Cross-platform swprintf replacement
// MinGW blocks swprintf, so we provide mu_swprintf as a safe cross-platform alternative

// Undefine MinGW's blocking macro first
#undef swprintf
#undef swprintf_instead_use_StringCbPrintfW_or_StringCchPrintfW

// Use inline template functions instead of macros to avoid issues with #ifdef in arguments
#ifdef _MSC_VER
  // MSVC: swprintf doesn't need buffer size
  template<typename... Args>
  inline int mu_swprintf(wchar_t* buffer, const wchar_t* format, Args... args) {
      return swprintf(buffer, format, args...);
  }
  // mu_swprintf_s with explicit size
  template<typename... Args>
  inline int mu_swprintf_s(wchar_t* buffer, size_t size, const wchar_t* format, Args... args) {
      return swprintf_s(buffer, size, format, args...);
  }
  // mu_swprintf_s with array - auto-deduce size (like MSVC's swprintf_s)
  template<size_t N, typename... Args>
  inline int mu_swprintf_s(wchar_t (&buffer)[N], const wchar_t* format, Args... args) {
      return swprintf_s(buffer, N, format, args...);
  }
#else
  // GCC/MinGW/Clang: use std::swprintf with explicit buffer size
  template<typename... Args>
  inline int mu_swprintf(wchar_t* buffer, const wchar_t* format, Args... args) {
      return std::swprintf(buffer, 1024, format, args...);
  }
  // mu_swprintf_s with explicit size
  template<typename... Args>
  inline int mu_swprintf_s(wchar_t* buffer, size_t size, const wchar_t* format, Args... args) {
      return std::swprintf(buffer, size, format, args...);
  }
  // mu_swprintf_s with array - auto-deduce size (compatible with MSVC swprintf_s)
  template<size_t N, typename... Args>
  inline int mu_swprintf_s(wchar_t (&buffer)[N], const wchar_t* format, Args... args) {
      return std::swprintf(buffer, N, format, args...);
  }
#endif

//opengl
#include <gl/glew.h>
#include <gl/GL.h>

//patch
//winmain
#include "Winmain.h"
#include "Defined_Global.h"

//client
#include "_define.h"
#include "_enum.h"
#include "_types.h"
#include "_struct.h"
#include "w_WindowMessageHandler.h"
#include "_GlobalFunctions.h"
#include "_TextureIndex.h"
#include "UIDefaultBase.h"
#include "NewUICommon.h"
#include "./Math/ZzzMathLib.h"
#include "ZzzOpenglUtil.h"

#include "MultiLanguage.h"


#include "./Utilities/Log/muConsoleDebug.h"
#include "./Utilities/Log/ErrorReport.h"
#include "./Utilities/Log/WindowsConsole.h"

// Redirect console output to ImGui when editor is enabled
#ifdef _EDITOR
#include "../MuEditor/MuEditorConsoleRedirect.h"
#endif

#include "w_MapHeaders.h"

#include "_crypt.h"

inline std::wstring g_strSelectedML = L"";
inline float g_fScreenRate_x = 0;
inline float g_fScreenRate_y = 0;