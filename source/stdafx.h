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
#ifdef _VS2008PORTING
	#pragma warning( disable : 4819 )
	#pragma warning( disable : 4505 )		// 참조 되지 않은 지역 함수를 제거
	#pragma warning( disable : 4100 )		// 참조 되지 않은 정식 매개 변수
	#pragma warning( disable : 4127 )		// 조건식이 상수임
	#pragma warning( disable : 4702 )		// 접근할 수 없는 코드
	//#pragma warning( disable : 4018 )
	//#pragma warning( disable : 4482 )
	//#pragma warning( disable : 4700 )
	//#pragma warning( disable : 4748 )
	//#pragma warning( disable : 4786 )	// GlobalBitmap.h 에서 옮겨옴
#endif // _VS2008PORTING

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN	

#ifdef _VS2008PORTING

	#ifndef POINTER_64
		#define POINTER_64
	#endif // POINTER_64
	
	#ifndef _USE_32BIT_TIME_T
		#define _USE_32BIT_TIME_T
	#endif //_USE_32BIT_TIME_T

	// _VS2008PORTING에서 안전성이 강화된 CRT함수 사용 권고 warning 해제
	// (언젠가는 새로운 함수로 필히 변경하여야 한다.!!!)
	#define _CRT_SECURE_NO_DEPRECATE
	#define _CRT_NONSTDC_NO_DEPRECATE

	// Debug Iterator 끄는 디파인 - Debug에서만 돌아감.
// 	#undef _HAS_ITERATOR_DEBUGGING
// 	#define _HAS_ITERATOR_DEBUGGING 0

// #ifdef _DEBUG
// 	// Checked Iterator 끄는 디파인 - Debug/Release 모두돌아감(Release에서는 off)
// 	#undef _SECURE_SCL
// 	#define _SECURE_SCL 0
// #endif // _DEBUG
#endif // _VS2008PORTING

#ifdef _VS2008PORTING
//===================================================================
// 불러올 파일들은 경고 수준 3으로 한다
//===================================================================
#pragma warning( push, 3 )
#endif // _VS2008PORTING

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

//zzzai file cpp 에서 가져 왔음..안 사용해도 됨..
//#include <iostream.h>

// 라이브러리 꼬이는거 방지.
//stl
#ifdef _DEBUG		// Debug모드
#include <string>
#include <list>
#include <map>
#include <deque>
#include <algorithm>
#include <vector>
#include <queue>
#else // _DEBUG		// Release모드
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

#ifdef _VS2008PORTING
//===================================================================
// 이 후 파일들은 프로젝트 옵션에 따른다
//===================================================================
#pragma warning( pop )
#endif // _VS2008PORTING

#include <boost/smart_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/format.hpp>
#include <boost/any.hpp>
#include <boost/array.hpp>
// vc 버그로 쓸때가 있으면 그 파일 위에다가 걸어주도록 하자.
//#include <boost/tokenizer.hpp>
#include <boost/pool/object_pool.hpp>

//opengl
#include <gl/gl.h>
#include <gl/glu.h>
#include <GL/glaux.h>
//#ifdef KJH_MOD_LIBRARY_LINK_EACH_NATION
#include <glext.h>
#include <wglext.h>
#include <glprocs.h>
// #else // KJH_MOD_LIBRARY_LINK_EACH_NATION
// #include "glext.h"
// #include "wglext.h"
// #include "glprocs.h"
// #endif // KJH_MOD_LIBRARY_LINK_EACH_NATION

//util
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p)  { if(p) { delete [] (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#define DIRECTINPUT_VERSION	0x0800

// namespace
using namespace std;//iosteam이랑 겹쳐서...
using namespace boost;

//patch
//winmain 함수들은 전부 옮겨가자
#include "Winmain.h"
#include "Winmain_New.h"
#include "Winmain_New_Live.h"

#ifdef KJH_MOD_LIBRARY_LINK_EACH_NATION
//----------------------------------------------------------------------------- Eng
// Eng
#if _LANGUAGE_ENG
	#include "Defined_Global.h"
//----------------------------------------------------------------------------- Tai
// Tai
#elif _LANGUAGE_TAI
	#include "Winmain_New_Foreign.h"
//----------------------------------------------------------------------------- Chs
// Chs
#elif _LANGUAGE_CHS
	#include "Winmain_New_Foreign.h"
//----------------------------------------------------------------------------- Jpn
// Jpn
#elif _LANGUAGE_JPN
	#include "Winmain_New_Foreign.h"
//----------------------------------------------------------------------------- Tha
// Tha
#elif _LANGUAGE_THA
	#include "Winmain_New_Foreign.h"
//----------------------------------------------------------------------------- Phi
// Phi
#elif _LANGUAGE_PHI
	#include "Winmain_New_Foreign.h"
//----------------------------------------------------------------------------- Vie
// Vie
#elif _LANGUAGE_VIE
	#include "Winmain_New_Foreign.h"
#endif
//-----------------------------------------------------------------------------
#else // KJH_MOD_LIBRARY_LINK_EACH_NATION
#if SELECTED_LANGUAGE != LANGUAGE_KOREAN
	#include "Winmain_New_Foreign.h"
// mix 파일 유동적으로 바꾸면 옮길것...
#endif //SELECTED_LANGUAGE != LANGUAGE_KOREAN
#endif // KJH_MOD_LIBRARY_LINK_EACH_NATION

#include "Winmain_New_Event.h"

//client
#include "_define.h"
#include "_enum.h"
#include "_types.h"
#include "_struct.h"

#ifdef NEW_USER_INTERFACE

	#include "w_structs.h"
	//util
	#include "w_Coord.h"
	#include "w_Rect.h"
	#include "w_Color.h"
	#include "w_ObjectPool.h"
	#include "w_EventPublisher.h"
	#include "w_build.h"
	#include "stlutl.h"  //이현..
	//input
	#include "w_InputTypes.h"
	#include "w_InputMessageHandler.h"
	//namespace
	using namespace util;
	using namespace input;
	//ui
	#include "w_UIType.h"
	#include "w_UIdefines.h"
	//client
	#include "w_Task.h"
	#include "w_Module.h"

#endif //NEW_USER_INTERFACE
	
#include "w_WindowMessageHandler.h"
#include "_GlobalFunctions.h"
#include "_TextureIndex.h"	
#include "InfoHelperFunctions.h"
#include "UIDefaultBase.h"
#include "NewUICommon.h"
#include "./Math/ZzzMathLib.h"
#include "ZzzOpenglUtil.h"

#endif // KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE		// #ifndef

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__B9DB83DB_A9ZD_12D0_BFJ1_447553540000__INCLUDED_)
