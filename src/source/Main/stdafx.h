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
#ifdef _WIN32
#include <windows.h>
#else
#include "Platform/PlatformTypes.h"
#include "Platform/PlatformCompat.h"
#endif
#else
// For non-Windows, platform types come after system headers but before project headers
#include "Platform/PlatformTypes.h"
#include "Platform/PlatformCompat.h"
#endif

// client - base definitions
#include "Core/mu_base_types.h"
#include "Core/mu_define.h"
#include "Core/mu_enum.h"
#include "Core/ZzzMathLib.h"
#include "Core/mu_types.h"
#include "Core/mu_struct.h"
#include "w_WindowMessageHandler.h"
#include "_GlobalFunctions.h"
#include "_TextureIndex.h"
#include "UIDefaultBase.h"
#include "NewUICommon.h"
#include "Core/ZzzMathLib.h"
#include "ZzzOpenglUtil.h"

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

inline std::wstring g_strSelectedML = L"";
inline float g_fScreenRate_x = 0;
inline float g_fScreenRate_y = 0;
