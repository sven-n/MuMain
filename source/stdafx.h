// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__B9DB83DB_A9ZD_12D0_BFJ1_447553540000__INCLUDED_)
#define AFX_STDAFX_H__B9DB83DB_A9ZD_12D0_BFJ1_447553540000__INCLUDED_

//#define KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE

//warining
#pragma warning( disable : 4786 ) 
#pragma warning( disable : 4800 ) 
#pragma warning( disable : 4244 )
#pragma warning( disable : 4237 )
#pragma warning( disable : 4305 )
#pragma warning( disable : 4503 ) 

#pragma warning( disable : 4819 )
#pragma warning( disable : 4505 )
#pragma warning( disable : 4100 )
#pragma warning( disable : 4127 )
#pragma warning( disable : 4702 )
#pragma warning( disable : 5208 )
//#pragma warning( disable : 4018 )
//#pragma warning( disable : 4482 )
//#pragma warning( disable : 4700 )
//#pragma warning( disable : 4748 )
//#pragma warning( disable : 4786 )
#pragma warning( disable : 28159 )
#pragma warning( disable : 26812 )

#pragma once

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN	
	
#ifndef _USE_32BIT_TIME_T
	#define _USE_32BIT_TIME_T
#endif //_USE_32BIT_TIME_T

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE

#pragma warning( push, 3 )

#include <windows.h>
#ifndef KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE
//windows
#include <WinSock2.h>
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

#ifdef _DEBUG
#include <string>
#include <list>
#include <map>
#include <deque>
#include <algorithm>
#include <vector>
#include <queue>
#else // _DEBUG		// Release
#ifdef _MT			
#undef _MT
#include <string>
#include <list>
#include <map>
#include <deque>
#include <algorithm>
#include <vector>
#include <queue>
#define _MT
#else //_MT
#include <string>
#include <list>
#include <map>
#include <deque>
#include <algorithm>
#include <vector>
#include <queue>
#endif // _MT
#endif // _DEBUG

#pragma warning( pop )

//opengl
#include <gl/glew.h>
#include <gl/gl.h>

//util
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p)  { if(p) { delete [] (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#define DIRECTINPUT_VERSION	0x0800
#define BYTECAST(T,X) static_cast<T>(X & 0xFF)

//patch
//winmain
#include "Winmain.h"
#include "Winmain_New_Live.h"
#include "Defined_Global.h"
#include "Winmain_New_Event.h"

//client
#include "_define.h"
#include "_enum.h"
#include "_types.h"
#include "_struct.h"	
#include "w_WindowMessageHandler.h"
#include "_GlobalFunctions.h"
#include "_TextureIndex.h"	
#include "InfoHelperFunctions.h"
#include "UIDefaultBase.h"
#include "NewUICommon.h"
#include "./Math/ZzzMathLib.h"
#include "ZzzOpenglUtil.h"

#endif // KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE		// #ifndef
#endif // !defined(AFX_STDAFX_H__B9DB83DB_A9ZD_12D0_BFJ1_447553540000__INCLUDED_)
