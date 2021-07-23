// DebugCameraDefine.h: interface for the DebugCameraDefine class.
//
//////////////////////////////////////////////////////////////////////

#ifdef DO_PROCESS_DEBUGCAMERA

#if !defined(AFX_DEBUGCAMERADEFINE_H__9ED660D0_C1D3_47B1_B739_B22EEA0D9EFB__INCLUDED_)
#define AFX_DEBUGCAMERADEFINE_H__9ED660D0_C1D3_47B1_B739_B22EEA0D9EFB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum EDEBUGCAMERA_TYPE {
			EDEBUGCAMERA_NULL = -1,
			EDEBUGCAMERA_NONE,	
			EDEBUGCAMERA_FREECONTROL,
			EDEBUGCAMERA_TOPVIEW,
			EDEBUGCAMERA_END,
};

#define _RADIAN (Q_PI / 180.0f)

#define _CAMERAVELOCITY_PERTICK (5.0f)

#define _DEFAULT_CAMERADISTANCE_NEAR_ (20.0f)

#define _DEFAULT_CAMERADISTANCE_FAR_ (3200.f)

#define _DEFAULT_RATE_ANGULARVELOCITY (0.5f)

#define _DEFAULT_VELOCITY_PERTICK (5.0f)

#define _DEFAULT_MULTIPLE_TO_VEOLOCITY (1.0f)

#define _DEFAULT_MULTIPLE_TO_ANGULAR_VEOLOCITY (1.0f)


#define COLOR_ARGB(a,r,g,b) \
((DWORD)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#define COLOR_RGBA(r,g,b,a) COLOR_ARGB(a,r,g,b)

// Color  // 0xAARRGGBB
#define PickA(CO) ((CO>>24)&0xff)
#define PickR(CO) ((CO>>16)&0xff)
#define PickG(CO) ((CO>>8)&0xff)
#define PickB(CO) ((CO)&0xff)


#endif // !defined(AFX_DEBUGCAMERADEFINE_H__9ED660D0_C1D3_47B1_B739_B22EEA0D9EFB__INCLUDED_)

#endif // DO_PROCESS_DEBUGCAMERA
